#include "jabber.h"

using namespace QXMPP;

Jabber::Jabber(Jid user_jid, QObject *parent) : QObject(parent), jid(user_jid) {
  transport = new Transport(jid.host, jid.port, jid.keep_alive, this);
  connect(transport, &Transport::sigOnConnectStatus, this, &Jabber::slotTransportStatus);
  connect(transport, &Transport::sigOnData, this, &Jabber::slotTransportData);
}

Jabber::~Jabber() {
  if (auth_sasl) {
    delete auth_sasl;
    auth_sasl = nullptr;
  }
  if (transport) {
    delete transport;
    transport = nullptr;
  }
}

void Jabber::Connect() {
  transport->Connect();
}

void Jabber::Disconnect() {
  writeData(Package::getClose());
  transport->Disconnect();
  current_jabber_status = JabberStatus::Disconnect;
  emit sigOnJabberDisconnect();
}

void Jabber::Reconnect() {
  Disconnect();
  Connect();
}

void Jabber::sendMessage(const QString to, const QString body) {
    writeData(Package::getMessage(to, jid.getFullJid(), body));
}

QHash<QString, Jid> Jabber::getRoster() {
  return roster;
}


void Jabber::slotTransportStatus(ConnectionStatus status) {
  switch (status) {
    case  ConnectionStatus::Connect: {
      current_jabber_status = JabberStatus::Connecting;
      writeData(Package::getPackageHello(jid.user, jid.host));
      break;
    }
    case  ConnectionStatus::ConnectEncryption: {
      current_jabber_status = JabberStatus::ConnectingTLS;
      writeData(Package::getPackageHello(jid.user, jid.host));
      break;
    }
    case  ConnectionStatus::Disconnect: {
      if (JabberStatus::Disconnect != current_jabber_status) {
        emit sigOnJabberDisconnect();
        current_jabber_status = JabberStatus::Disconnect;
      };
      break;
    }
    case  ConnectionStatus::Error: {
      errorAndDisconnect(transport->getLastError());
      break;
    }
  }
}

void Jabber::slotTransportData(QString data) { 
  if (JabberStatus::Disconnect == current_jabber_status) return;
  for (const auto &pack_data : Package::divPackageData(data)) {
    if      (pack_data.contains("<stream:stream")) continue;
    else if (pack_data.contains("</stream:features>")) parsePackageHello(pack_data);
    else if (pack_data.contains("urn:ietf:params:xml:ns:xmpp-tls")) parseTLS(pack_data);
    else if (pack_data.contains("urn:ietf:params:xml:ns:xmpp-sasl")) return;
    else if (pack_data.contains("<iq")) parseIqData(pack_data);
    else if (pack_data.contains("<presence")) parsePresence(pack_data);
    else if (pack_data.contains("<message")) parseMessage(data);
    else if (pack_data.contains("</stream:stream>")) Disconnect();
    if (jid.debug) {
      qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
      qDebug() << data;
      qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
    }
  }
}

void Jabber::slotSASLError(QString error) {
  errorAndDisconnect(error);
}

void Jabber::slotSASLOK() {
  QObject::disconnect(auth_sasl, &Sasl_Base::OnSaslError, this, &Jabber::slotSASLError);
  QObject::disconnect(auth_sasl, &Sasl_Base::OnSaslOK, this, &Jabber::slotSASLOK);
  delete auth_sasl;
  auth_sasl = nullptr;
  current_jabber_status = JabberStatus::Auth;
  writeData(Package::getPackageHello(jid.user, jid.host));
}

void Jabber::parsePackageHello(QString data) {
  if (current_jabber_status == JabberStatus::Auth) writeData(Package::getBindResource(jid.resource));
  else {
    if (current_jabber_status != JabberStatus::ConnectingTLS) {
      bool check_tls = data.contains("<starttls");
      switch (jid.tls) {
        case QXMPP::TLSType::AUTO: {
          if (check_tls)  {
            writeData(Package::getPackageRunTLS());
            return;
          }
          else break;
        };
        case QXMPP::TLSType::TLS_ONLY: {
          if (check_tls) {
            writeData(Package::getPackageRunTLS());
            return;
          }
          else {
            errorAndDisconnect("ERROR: SERVER NOT SUPPORT TLS");
            return;
          }
        }
        case QXMPP::TLSType::NO_TLS: break;
      }
    }
    QDomDocument doc;
    doc.setContent(data, false);
    bool suppor_SASL_Plain     = false;
    bool suppor_SASL_DigestMD5 = false;
    bool suppor_SASL_SCRAM_SHA1 = false;
    QDomNodeList node_list = doc.elementsByTagName("mechanism");
    for (int i = 0; i < node_list.size(); ++i) {
      QDomElement domElement = node_list.at(i).toElement();
      if      ("PLAIN" == domElement.text()) suppor_SASL_Plain = true;
      else if ("DIGEST-MD5" == domElement.text()) suppor_SASL_DigestMD5 = true;
      else if ("SCRAM-SHA-1" == domElement.text()) suppor_SASL_SCRAM_SHA1 = true;
    }
    if (QXMPP::SASLType::AUTO == jid.sasl) {
      if      (suppor_SASL_DigestMD5) jid.sasl = QXMPP::SASLType::DigestMD5;
      else if (suppor_SASL_SCRAM_SHA1) jid.sasl = QXMPP::SASLType::SCRAM_SHA1;
      else if (suppor_SASL_Plain) jid.sasl = QXMPP::SASLType::Plain;
      else {
        errorAndDisconnect("ERROR: Server not support sasl mechanism");
        return;
      }
    }
    if (((QXMPP::SASLType::Plain == jid.sasl) && (suppor_SASL_Plain)) ||
       ((QXMPP::SASLType::SCRAM_SHA1 == jid.sasl) && (suppor_SASL_SCRAM_SHA1)) ||
       ((QXMPP::SASLType::DigestMD5== jid.sasl) && (suppor_SASL_DigestMD5))) {
      runSaslAuth();
    }
    else {
      errorAndDisconnect("ERROR: Server not support sasl mechanism");
    }
  }
}

void Jabber::parseIqData(QString data) {
  QDomDocument doc;
  doc.setContent(data, false);
  QDomElement root = doc.documentElement();
  QString id_atr = root.attribute("id");
  QString res_atr = root.attribute("type");

  if (("bindresource" == id_atr) && ("result" == res_atr)) {
    session_pid = Package::getRandomHexBytes(5);
    writeData(Package::getNewSession(session_pid));
  }
  else if ((session_pid == id_atr) && ("result" == res_atr)) {
    if (data.contains("jabber:iq:roster")) {
      QDomDocument doc;
      doc.setContent(data, false);
      QDomElement root = doc.documentElement();
      QDomNodeList node_list= doc.elementsByTagName("item");
      for (int i = 0; i < node_list.count(); i++) {
        QDomElement domElement = node_list.at(i).toElement();
        Jid r_jid(domElement.attribute("jid"));
        r_jid.user_info = domElement.attribute("name");
        r_jid.user_group = domElement.childNodes().at(0).toElement().text();
        r_jid.show_status = ShowStatus::unknown;
        roster[r_jid.getJid()] = r_jid;
      }
      if (JabberStatus::Auth == current_jabber_status) {
        writeData(Package::getShow("online"));
        writeData(Package::getPriority(jid.priority));
        writeData(Package::getStatus(jid.msg_status));
        current_jabber_status = JabberStatus::Connected;
        emit sigOnJabberConnect();
      }
    }
    else {
      session_pid = Package::getRandomHexBytes(5);
      writeData(Package::getRoster(session_pid));
    };
  }
}

void Jabber::parsePresence(QString data) {
  QDomDocument doc;
  doc.setContent(data, false);
  QDomElement root = doc.documentElement();
  QString from = root.attributeNode("from").value();
  QString to = root.attributeNode("to").value();
  if (from == to) return;
  Jid t_j;
  QString lite_from = Jid::getJid(from);
  if (roster.contains(lite_from)) t_j = roster[lite_from];
  else return;

  if (root.attributeNode("type").value() == "unavailable") {
    t_j.show_status = QXMPP::ShowStatus::unknown;
    roster[lite_from] = t_j;
    emit sigOnJabberUpdateContact();
    return;
  }

  t_j.show_status  = QXMPP::ShowStatus::online;

  QDomNodeList show = root.elementsByTagName("show");
  if( !show.isEmpty()) {
    QDomElement domElement = show.at(0).toElement();
    t_j.setShowStatus(domElement.text());
  }

  QDomNodeList status = root.elementsByTagName("status");
  if( !status.isEmpty()) {
    QDomElement domElement = status.at(0).toElement();
    t_j.msg_status = domElement.text();
  }
  roster[lite_from] = t_j;
  emit sigOnJabberUpdateContact();
}

void Jabber::parseMessage(QString data) {
  QDomDocument doc;
  doc.setContent(data, false);
  QDomElement root = doc.documentElement();
  QString from = root.attributeNode("from").value();
  QString to = root.attributeNode("to").value();
  if (from == to) {
    return;
  }

  QString lite_from = Jid::getJid(from);
  if (!roster.contains(lite_from)) return;
  QDomNodeList body = root.elementsByTagName( "body" );
  if( !body.isEmpty()) {
    QDomElement domElement = body.at(0).toElement();
    QString messsage = domElement.text();
    emit sigOnJabberMessage(lite_from, messsage);
  }
}

void Jabber::parseTLS(QString data) {
  if (data.contains("proceed")) transport->runTLSConnect();
  else errorAndDisconnect("ERROR: SERVER CANCEL TLS");
}

void Jabber::runSaslAuth() {
  switch (jid.sasl) {
    case QXMPP::SASLType::Plain: auth_sasl = new Sasl_PLAIN(jid, transport); break;
    case QXMPP::SASLType::DigestMD5: auth_sasl = new SASL_DIGEST_MD5(jid, transport); break;
    case QXMPP::SASLType::SCRAM_SHA1: auth_sasl = new Sasl_SCRAM_SHA1(jid, transport); break;
    case QXMPP::SASLType::AUTO: break;
  }
  QObject::connect(auth_sasl, &Sasl_Base::OnSaslError, this, &Jabber::slotSASLError);
  QObject::connect(auth_sasl, &Sasl_Base::OnSaslOK, this, &Jabber::slotSASLOK);
  auth_sasl->runAuth();
}

void Jabber::writeData(QString data) {
  transport->sendData(data);
  if (jid.debug) {
    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    qDebug() << data;
    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
  }
}

void Jabber::errorAndDisconnect(QString error) {
  current_jabber_status = JabberStatus::Disconnect;
  emit sigOnJabberError(error);
  transport->Disconnect();
}

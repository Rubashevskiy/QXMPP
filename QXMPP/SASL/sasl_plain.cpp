#include "sasl_plain.h"

using namespace QXMPP;

Sasl_PLAIN::Sasl_PLAIN(const Jid user_jid, Transport *jabber_transport) : jid(user_jid) {
  transport = jabber_transport;
  QObject::connect(transport, &Transport::sigOnData, this, &Sasl_PLAIN::slotTransportData);
}

void Sasl_PLAIN::runAuth() {
    std::string user = jid.user.toStdString();
    std::string password = jid.password.toStdString();
    byte data[1024];
    int offset = 0;
    data[offset++] = 0;

    memcpy(data+offset, user.c_str(), user.length());
    offset+=user.length();
    data[offset++] = 0;

    memcpy(data+offset, password.c_str(), password.length());
    offset += password.length();

    std::string encoded_response;
    CryptoPP::ArraySource css(data, offset, true,
         new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded_response), false));

    QString buffer{};
    QTextStream stream(&buffer, QIODevice::ReadWrite);
    stream << "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='PLAIN'>"
           << QString::fromUtf8(encoded_response)
           << "</auth>";
    writeData(stream.readAll());
}

void Sasl_PLAIN::slotTransportData(QString data) {
  if (jid.debug) {
    qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
    qDebug() << data;
    qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
  }
  QDomDocument doc;
  doc.setContent(data, false);
  QDomElement root = doc.documentElement();
  if (root.tagName() == "success") emit OnSaslOK();
  else emit OnSaslError("ERROR: user or password incorrect");
  return;
}

void Sasl_PLAIN::writeData(const QString &data) {
  transport->sendData(data);
  if (jid.debug) {
    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    qDebug() << data;
    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
  }
}


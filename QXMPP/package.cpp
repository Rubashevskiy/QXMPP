#include "package.h"

using namespace QXMPP;

QString Package::getPackageHello(const QString &user, const QString &host) {
  QString buffer{};
  QTextStream stream(&buffer, QIODevice::ReadWrite);
  stream << "<?xml version='1.0' encoding='UTF-8'?>"
         << "<stream:stream "
         << "from='" << user << "' "
         << "to ='"  << host <<  "' "
         << "version='1.0' "
         << "xml:lang='en' "
         << "xmlns='jabber:client' "
         << "xmlns:stream='http://etherx.jabber.org/streams'>";
  return stream.readAll();
}

QString Package::getPackageRunTLS() {
  QString buffer{};
  QTextStream stream(&buffer, QIODevice::ReadWrite);
  stream << "<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>";
  return stream.readAll();
}

QString Package::getBindResource(const QString resource) {
  QString buffer{};
  QTextStream stream(&buffer, QIODevice::ReadWrite);
  stream  << "<iq type='set' id='bindresource'>"
          << "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'>"
          << "<resource>" << resource << "</resource>"
          << "</bind>"
          << "</iq>";
  return stream.readAll();
}

QString Package::getNewSession(const QString hex) {
  QString buffer{};
  QTextStream stream(&buffer, QIODevice::ReadWrite);
  stream << "<iq type='set' id='" << hex << "'>"
         << "<session xmlns='urn:ietf:params:xml:ns:xmpp-session'/></iq>";
  return stream.readAll();
}

QString Package::getRoster(const QString hex) {
  QString buffer{};
  QTextStream stream(&buffer, QIODevice::ReadWrite);
  stream << "<iq type='get' id='" << hex << "'>"
         << "<query xmlns='jabber:iq:roster'/></iq>";
  return stream.readAll();
}

QString Package::getShow(const QString show) {
  QString buffer{};
  QTextStream stream(&buffer, QIODevice::ReadWrite);
  stream << "<presence><show>" << show << "</show></presence>";
  return stream.readAll();
}

QString Package::getPriority(const int priority) {
  QString buffer{};
  QTextStream stream(&buffer, QIODevice::ReadWrite);
  stream << "<presence><priority>" << priority << "</priority></presence>";
  return stream.readAll();
}

QString Package::getStatus(const QString status) {
  QString buffer{};
  QTextStream stream(&buffer, QIODevice::ReadWrite);
  stream << "<presence><status>" << status << "</status></presence>";
  return stream.readAll();
}

QString Package::getClose() {
  QString buffer{};
  QTextStream stream(&buffer, QIODevice::ReadWrite );
  stream << "<presence type='unavailable'><status>unavailable</status></presence>";
  return stream.readAll();
}

QString Package::getMessage(const QString &to, const QString &from, const QString &body) {
  QString buffer("");
  QTextStream stream(&buffer, QIODevice::ReadWrite );
  stream << "<message type='chat' "
         << " to = '" << to << "'"
         << " id='" << getRandomHexBytes(5) << "'"
         << " from = '" << from << "'>"
         << "<body>" << body << "</body>"
         << "</message>";
  return stream.readAll();
}

QString Package::getRandomHexBytes(int count_data) {
  QString result{};
  QString src_str = "0123456789abcdef";
  for(int i = 0; i < count_data; i++) {
    result.append(src_str[QRandomGenerator::global()->bounded(16)]);
  }
  return result;
}



QVector<QString> QXMPP::Package::divPackageData(const QString &buffer) {
  QList<QString> dv{"iq", "presence", "message"};
  QVector<QString> result{};
  QString data{buffer};
  for (const auto &check_dv : dv) {
    QString st_tg = "<" + check_dv;
    QString fn_tg = "</" + check_dv +">";
    int i = -1;
    int j = -1;
    do {
      QString tmp_pack{};
      int i = data.indexOf(st_tg);
      int j = data.indexOf(fn_tg);
      if ((i != -1) && (j != -1)) {
        tmp_pack = data.mid(i, j + fn_tg.size());
        data = data.remove(i, j + fn_tg.size());
        result.append(tmp_pack);
      }
    }
    while ((i != -1) && (j != -1));
  }
  if ((!data.trimmed().isEmpty()) && (data.trimmed() != " ")) {
    result.append(data);
  }
  return result;
}

#include <QCoreApplication>
#include <QObject>
#include <QDebug>

#include "jabber.h"
#include "jid.h"

QXMPP::Jabber *jabber = nullptr;

void jabberRosterUpdate() {
  QHash<QString, QXMPP::Jid> roster = jabber->getRoster();
  QHash<QString, QXMPP::Jid>::iterator itr = roster.begin();
  qDebug() << "ROSTER:";
  while (itr != roster.end()) {
    if ((*itr).user_info.isEmpty()) {
      qDebug() << QString("%1 >> %2").arg((*itr).getJid(), (*itr).getShowStatus());
    }
    else {
      qDebug() << QString("%1(%2) >> %3").arg((*itr).getJid(), (*itr).user_info, (*itr).getShowStatus());
    }
    itr++;
  }
}

void JabberMessage(QString to, QString msg) {
  qDebug() << QString("JID: %1 SEND MESSAGE: %2").arg(to, msg);
  jabber->sendMessage(to, "HI I`m lite jabber bot");
}

void jabberConnect() {
  qDebug() << "Jabber CONNECT";
  jabberRosterUpdate();
}

void jabberDisconnect() {
  qDebug() << "Jabber DISCONNECT";
}

void jabberError(QString error) {
  qDebug() << "Jabber ERROR: " << error;
}

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);
  QXMPP::Jid user_jid{};
  user_jid.user = "user_name";
  user_jid.host = "jabber.ru";
  user_jid.password = "user_password";
  user_jid.sasl = QXMPP::SASLType::AUTO;
  user_jid.tls = QXMPP::TLSType::AUTO;
  jabber = new QXMPP::Jabber(user_jid, &a);
  QObject::connect(jabber, &QXMPP::Jabber::sigOnJabberConnect, &a, &jabberConnect);
  QObject::connect(jabber, &QXMPP::Jabber::sigOnJabberDisconnect, &a, &jabberDisconnect);
  QObject::connect(jabber, &QXMPP::Jabber::sigOnJabberError, &a, &jabberError);
  QObject::connect(jabber, &QXMPP::Jabber::sigOnJabberMessage, &a, &JabberMessage);
  QObject::connect(jabber, &QXMPP::Jabber::sigOnJabberUpdateContact, &a, &jabberRosterUpdate);
  jabber->Connect();

  return a.exec();
}

#ifndef JABBER_H
#define JABBER_H

#include <QObject>
#include <QTextStream>
#include <QDomDocument>

#include "jid.h"
#include "package.h"
#include "transport.h"
#include "SASL/sasl_base.h"
#include "SASL/sasl_plain.h"
#include "SASL/sasl_digest_md5.h"
#include "SASL/sasl_scram_sha1.h"

namespace QXMPP {

  enum class JabberStatus {
    Connecting,
    ConnectingTLS,
    Auth,
    Connected,
    Disconnect
  };

  class Jabber : public QObject {
    Q_OBJECT
  public:
    explicit Jabber(Jid user_jid, QObject *parent = nullptr);
    ~Jabber();
    void Connect();
    void Disconnect();
    void Reconnect();
    void sendMessage(const QString to, const QString body);
    QHash<QString, QXMPP::Jid> getRoster();
  signals:
    void sigOnJabberConnect();
    void sigOnJabberMessage(QString, QString);
    void sigOnJabberError(QString);
    void sigOnJabberDisconnect();
    void sigOnJabberUpdateContact();
  private:
    void parsePackageHello(QString data);
    void parseIqData(QString data);
    void parsePresence(QString data);
    void parseMessage(QString data);
    void runSaslAuth();
    void errorAndDisconnect(QString error);
  private slots:
    void slotTransportStatus(QXMPP::ConnectionStatus status);
    void slotTransportData(QString data);
    void slotSASLError(QString error);
    void slotSASLOK();
  private:
    QXMPP::Transport *transport = nullptr;
    QXMPP::Sasl_Base *auth_sasl = nullptr;
    QXMPP::JabberStatus current_jabber_status = QXMPP::JabberStatus::Disconnect;
    QXMPP::Jid jid;
    QHash<QString, QXMPP::Jid> roster;
    QString session_pid{};
  };

} //namespace QXMPP



#endif // JABBER_H

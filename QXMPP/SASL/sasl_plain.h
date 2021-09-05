#ifndef SASL_PLAIN_H
#define SASL_PLAIN_H

#include <QObject>
#include "sasl_base.h"


namespace QXMPP {

  class Sasl_PLAIN  : public Sasl_Base {
    Q_OBJECT
    public:
      explicit Sasl_PLAIN(const Jid user_jid, Transport *jabber_transport);
      void runAuth();
    private slots:
      void slotTransportData(QString data);
    private:
      void writeData(const QString &data);
    private:
      Transport *transport = nullptr;
      Jid jid;
  };

} // namespace QXMPP

#endif // SASL_PLAIN_H

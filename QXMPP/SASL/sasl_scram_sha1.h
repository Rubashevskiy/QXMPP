#ifndef SASL_SCRAM_SHA1_H
#define SASL_SCRAM_SHA1_H

#include <QObject>
#include "sasl_base.h"

namespace QXMPP {

  class Sasl_SCRAM_SHA1 : public Sasl_Base {
    Q_OBJECT
    public:
      explicit Sasl_SCRAM_SHA1(const Jid user_jid, Transport *jabber_transport);
      void runAuth();
    private slots:
      void slotTransportData(QString data);
    private:
      Transport *transport = nullptr;
      Jid jid;
      std::string selected_nounce;
      std::string server_proof;

  };

} // namespace QXMPP

#endif // SASL_SCRAM_SHA1_H

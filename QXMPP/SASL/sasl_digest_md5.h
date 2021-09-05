#ifndef SASL_DIGEST_MD5_H
#define SASL_DIGEST_MD5_H

#include <QObject>
#include "sasl_base.h"


namespace QXMPP {

  class SASL_DIGEST_MD5  : public Sasl_Base {
    Q_OBJECT
    public:
      explicit SASL_DIGEST_MD5(const Jid user_jid, Transport *jabber_transport);
      void runAuth();
    private slots:
      void slotTransportData(QString data);
    private:
      std::string getHA1(std::string x, std::string nonce, std::string cnonce);
      std::string getMD5Hex(std::string input);
      void writeData(const QString &data);
    private:
      Transport *transport = nullptr;
      Jid jid;
  };

} // namespace QXMPP

#endif // SASL_DIGEST_MD5_H

#ifndef SASL_BASE_H
#define SASL_BASE_H

#include <QObject>
#include "transport.h"
#include "jid.h"
#include <QDomDocument>
#include <QRandomGenerator>
#include <QHash>


#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <cryptopp/cryptlib.h>
#include <cryptopp/hex.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>
#include <cryptopp/base64.h>
#include <cryptopp/queue.h>
#include <cryptopp/md5.h>


namespace QXMPP {

  class Sasl_Base : public QObject {
    Q_OBJECT
    public:
      explicit Sasl_Base(QObject *parent = nullptr);
      virtual void runAuth();
      static QHash<QByteArray, QByteArray> challengeTokenize(QByteArray data);
      static std::string encodeBase64(std::string input);
      static std::string stringToLower(std::string s);
      static std::string getRandomHexBytes(int count_data);
    signals:
      void OnSaslOK();
      void OnSaslError(QString);

  };

} // namespace QXMPP


#endif // SASL_BASE_H

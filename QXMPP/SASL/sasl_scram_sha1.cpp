#include "sasl_scram_sha1.h"

using namespace QXMPP;

Sasl_SCRAM_SHA1::Sasl_SCRAM_SHA1(const Jid user_jid, Transport *jabber_transport) : jid(user_jid) {
  transport = jabber_transport;
  QObject::connect(transport, &Transport::sigOnData, this, &Sasl_SCRAM_SHA1::slotTransportData);
}

void Sasl_SCRAM_SHA1::runAuth() {
  selected_nounce = getRandomHexBytes(16);
  std::stringstream ss;
  ss<< "n,,n=" << jid.user.toStdString() << ",r=" << selected_nounce;
  std::string request = ss.str();
  request = encodeBase64(request);
  ss.str("");
  ss <<  "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='SCRAM-SHA-1'>"
     << request
     << "</auth>";
  writeData(QString::fromUtf8(ss.str()));
}

void Sasl_SCRAM_SHA1::slotTransportData(QString data) {
  if (jid.debug) {
    qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
    qDebug() << data;
    qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
  }
  QDomDocument doc;
  doc.setContent(data, false);
  QDomElement root = doc.documentElement();
  QByteArray challenge = QByteArray::fromBase64(root.text().toUtf8(), QByteArray::Base64Encoding);
  QHash<QByteArray, QByteArray> challengeMap = challengeTokenize(challenge);
  if (challengeMap.contains("v")) {
    if (challengeMap["v"] == QString::fromUtf8(server_proof)) {
      emit OnSaslOK();
      return;
    }
    else {
      emit OnSaslError("ERROR: user or password incorrect");
      return;
    }
  }
  std::string r = challengeMap["r"].toStdString();
  std::string s = challengeMap["s"].toStdString();
  int NrIterations = challengeMap["i"].toInt();
  std::string n = jid.user.toStdString();

  byte SaltBytes[1024];

  CryptoPP::Base64Decoder decoder;
  decoder.Put((byte*)s.c_str(), s.length());
  decoder.MessageEnd();

  int SaltLength =decoder.Get(SaltBytes, 1024-4);
  SaltBytes[SaltLength++] = 0;
  SaltBytes[SaltLength++] = 0;
  SaltBytes[SaltLength++] = 0;
  SaltBytes[SaltLength++] = 1;

  std::string c = "biws";
  std::string p= ""; // proof!! calcualte!!


  byte Result[CryptoPP::SHA1::DIGESTSIZE ];
  byte tmp[CryptoPP::SHA1::DIGESTSIZE ];
  byte ClientKey[CryptoPP::SHA1::DIGESTSIZE ];
  byte StoredKey[CryptoPP::SHA1::DIGESTSIZE ];
  byte Previous[CryptoPP::SHA1::DIGESTSIZE ];
  byte ClientSignature[CryptoPP::SHA1::DIGESTSIZE ];
  byte ClientProof[CryptoPP::SHA1::DIGESTSIZE ];
  byte ServerKey[CryptoPP::SHA1::DIGESTSIZE ];
  byte ServerSignature[CryptoPP::SHA1::DIGESTSIZE ];

  int digestlength =CryptoPP::SHA1::DIGESTSIZE;

  // Calculate Result
  CryptoPP::HMAC<CryptoPP::SHA1> hmacFromPassword((byte*)jid.password.toStdString().c_str(),
                                                         jid.password.toStdString().length());
  hmacFromPassword.CalculateDigest( Result, SaltBytes, SaltLength);

  memcpy(Previous, Result, digestlength);
  for(int i = 1; i < NrIterations; i++) {
    hmacFromPassword.CalculateDigest( tmp, Previous, digestlength);
    for( int j = 0; j < digestlength; j++) {
      Result[j] = Result[j]^tmp[j];
      Previous[j] = tmp[j];
    }
  }

  CryptoPP::HMAC<CryptoPP::SHA1> hmacFromSaltedPassword(Result, digestlength);
  hmacFromSaltedPassword.CalculateDigest( ClientKey, (byte*)"Client Key", strlen("Client Key"));

  CryptoPP::SHA1 hash;
  hash.CalculateDigest( StoredKey, ClientKey, digestlength);

  std::stringstream ss;
  ss << "n=" << n << ",r=" << selected_nounce;
  ss << "," << challenge.toStdString();
  ss << ",c=" << c;
  ss << ",r=" << r;

  std::string AuthMessage = ss.str();

  CryptoPP::HMAC<CryptoPP::SHA1> hmacFromStoredKey(StoredKey, digestlength);
  hmacFromStoredKey.CalculateDigest( ClientSignature, (byte*)AuthMessage.c_str(), AuthMessage.length());

  for(int i = 0; i < digestlength; i++) {
    ClientProof[i] = ClientKey[i] ^ ClientSignature[i];
  }

  hmacFromSaltedPassword.CalculateDigest( ServerKey,(byte*)"Server Key", strlen("Server Key"));

  CryptoPP::HMAC< CryptoPP::SHA1> hmacFromServerKey(ServerKey, digestlength);
  hmacFromServerKey.CalculateDigest( ServerSignature, (byte*)AuthMessage.c_str(), AuthMessage.length());
              CryptoPP::ArraySource(ServerSignature, digestlength, true,
                                    new CryptoPP::Base64Encoder(
                                    new CryptoPP::StringSink(server_proof), false));

  CryptoPP::ArraySource(ClientProof, digestlength, true,
                        new CryptoPP::Base64Encoder(
                        new CryptoPP::StringSink(p), false));
  ss.str("");
  ss << "c=" << c << ",r=" << r << ",p=" << p;
  std::string Response = ss.str();

  std::string Base64EncodedResponse = encodeBase64(Response);

  ss.str("");
  ss << "<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
     << Base64EncodedResponse
     << "</response>";
  writeData(QString::fromUtf8(ss.str()));
}

void Sasl_SCRAM_SHA1::writeData(const QString &data) {
  transport->sendData(data);
  if (jid.debug) {
    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    qDebug() << data;
    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
  }
}



#include "sasl_digest_md5.h"

using namespace QXMPP;

SASL_DIGEST_MD5::SASL_DIGEST_MD5(const Jid user_jid, Transport *jabber_transport) : jid(user_jid) {
  transport = jabber_transport;
  QObject::connect(transport, &Transport::sigOnData, this, &SASL_DIGEST_MD5::slotTransportData);
}

void SASL_DIGEST_MD5::runAuth() {
  transport->sendData("<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='DIGEST-MD5'/>");
}

void SASL_DIGEST_MD5::slotTransportData(QString data) {
  QDomDocument doc;
  doc.setContent(data, false);
  QDomElement root = doc.documentElement();
  if (root.tagName() == "success") {
    emit OnSaslOK();
    return;
  }
  else if (root.tagName() == "failure") {
    emit OnSaslError("ERROR: user or password incorrect");
    return;
  }
  QByteArray challenge = QByteArray::fromBase64(root.text().toUtf8(), QByteArray::Base64Encoding);
  QHash<QByteArray, QByteArray> challengeMap = challengeTokenize(challenge);
  if (challengeMap.contains("rspauth")) {
    transport->sendData("<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>");
    return;
  }

  std::string user = jid.user.toStdString();
  std::string host = jid.host.toStdString();
  std::string password = jid.password.toStdString();
  std::string nonce = challengeMap["nonce"].toStdString();
  std::string qop = challengeMap["qop"].toStdString();

  std::string cnonce = getRandomHexBytes(16);
  std::string nc = "00000001";

  std::stringstream stream;
  stream << user << ':' << host << ':' << password;
  std::string X = stream.str();

  stream.str("");
  stream << "AUTHENTICATE:xmpp/" << host;
  std::string A2 = stream.str();
  std::string HA1 = getHA1(X, nonce, cnonce);
  std::string HA2 = getMD5Hex(A2);

  stream.str("");
  stream << HA1 << ':' << nonce << ':' << nc << ':' << cnonce << ':' << qop << ':' << HA2;
  std::string KD = stream.str();
  std::string Z = getMD5Hex(KD);
  stream.str("");
  stream << "username=\"" << user << "\""
         << ",realm=\"" << host << "\""
         << ",nonce=\"" << nonce << "\""
         << ",cnonce=\"" << cnonce << "\""
         << ",nc=" << nc
         << ",qop=" << qop
         << ",digest-uri=\"xmpp/"<< host << "\""
         << ",response="<< Z;
  std::string response = encodeBase64(stream.str());

  QString buffer{};
  QTextStream qstream(&buffer, QIODevice::ReadWrite);
  qstream << "<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
         << QString::fromUtf8(response)
         << "</response>";
  transport->sendData(qstream.readAll());
  return;
}

std::string SASL_DIGEST_MD5::getMD5Hex(std::string input) {
  CryptoPP::Weak::MD5 hash;
  byte digest[CryptoPP::Weak::MD5::DIGESTSIZE];
  int length =CryptoPP::Weak::MD5::DIGESTSIZE;

  CryptoPP::HexEncoder Hexit;
  std::string tmp_output;
  Hexit.Attach( new CryptoPP::StringSink(tmp_output));

  hash.CalculateDigest(digest, reinterpret_cast<const unsigned char *>(input.c_str()), input.length());
  Hexit.Put(digest, length);
  Hexit.MessageEnd();

  return stringToLower(tmp_output);
}

std::string SASL_DIGEST_MD5::getHA1(std::string x, std::string nonce, std::string cnonce) {
  CryptoPP::Weak::MD5 hash;
  byte digest[CryptoPP::Weak::MD5::DIGESTSIZE];
  int digestlength =CryptoPP::Weak::MD5::DIGESTSIZE;
  hash.CalculateDigest( digest, reinterpret_cast<const unsigned char *>(x.c_str()), x.length());

  std::stringstream stream;
  stream << ":" << nonce << ":" << cnonce;
  std::string AuthentiationDetails = stream.str();
  int TLen = (int)digestlength + (int)AuthentiationDetails.length();
  byte *T = new byte[TLen];
  memcpy(T, digest, digestlength);
  memcpy(T+digestlength, AuthentiationDetails.c_str(), AuthentiationDetails.length());
  hash.CalculateDigest( digest, reinterpret_cast<const unsigned char *>( T ), TLen );

  CryptoPP::HexEncoder Hexit;
  std::string TOutput;
  Hexit.Attach( new CryptoPP::StringSink( TOutput ) );

  Hexit.Put( digest, digestlength );
  Hexit.MessageEnd();

  return stringToLower(TOutput);
}

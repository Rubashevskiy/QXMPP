#include "sasl_base.h"

using namespace QXMPP;

Sasl_Base::Sasl_Base(QObject *parent) : QObject(parent) {}

void Sasl_Base::runAuth() {
  //virtual
}


QHash<QByteArray, QByteArray> Sasl_Base::challengeTokenize(QByteArray data) {
 QHash<QByteArray, QByteArray> result;
 data.replace('"', "");
 QList<QByteArray> list = data.split(',');
 for (const auto &token : list) {
   int pos = token.indexOf('=');
   if (-1 != pos) {
     result[token.mid(0, pos)] = token.mid(pos+1);
   }
 }
 return result;
}

std::string Sasl_Base::encodeBase64(std::string input) {
  byte data[1024];
  int offset = 0;
  memcpy(data + offset, input.c_str(), input.length());
  offset += input.length();

  std::string encoded_data;
  CryptoPP::ArraySource ss(data, offset, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded_data), false));
  return encoded_data;
}

std::string Sasl_Base::stringToLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){return std::tolower(c);});
  return s;
}

std::string Sasl_Base::getRandomHexBytes(int count_data) {
  QString result{};
  QString src_str = "0123456789abcdef";
  for(int i = 0; i < count_data; i++) {
    result.append(src_str[QRandomGenerator::global()->bounded(16)]);
  }
  return result.toStdString();
}

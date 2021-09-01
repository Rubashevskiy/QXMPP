#ifndef PACKAGE_H
#define PACKAGE_H

#include <QString>
#include <QVector>
#include <QTextStream>
#include <QIODevice>
#include <QByteArray>
#include <QRandomGenerator>



namespace QXMPP {


  class Package {
    public:
      static QString getPackageHello(const QString &user, const QString &host);
      static QString getPackageRunTLS();
      static QString getBindResource(const QString resource);
      static QString getNewSession(const QString hex);
      static QString getRoster(const QString hex);
      static QString getShow(const QString show);
      static QString getPriority(const int priority);
      static QString getStatus(const QString status);
      static QString getClose();
      static QString getMessage(const QString &to, const QString &from, const QString &body);
      static QString getRandomHexBytes(int count_data);
      static QVector<QString> divPackageData(const QString &buffer);



  private:
      Package() {}
  };

}
#endif // PACKAGE_H

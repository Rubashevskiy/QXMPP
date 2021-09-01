#ifndef TRANSPORT_H
#define TRANSPORT_H


#include <QtNetwork>
#include <QObject>
#include <QString>
#include <QTimer>

QT_REQUIRE_CONFIG(ssl);

namespace QXMPP {

enum class ConnectionStatus {
  Connect,
  ConnectEncryption,
  Disconnect,
  Error
};

class Transport : public QObject {
    Q_OBJECT
  public:
    explicit Transport(const QString server_host, const qint16 server_port, bool used_keep_alive, QObject *parent = nullptr);
    ~Transport();
    void Connect();
    void Disconnect();
    void sendData(const QString data);
    void runTLSConnect();
    QString getLastError();
    signals:
      void sigOnConnectStatus(ConnectionStatus);
      void sigOnData(QString);
    private slots:
      void slotSocketStateChanged(QAbstractSocket::SocketState state);
      void slotSocketEncrypted();
      void slotSocketError(QAbstractSocket::SocketError);
      void slotSslErrors(const QList<QSslError> &errors);
      void slotSocketReadyRead();
      void slotKeepAlive();
    private:
      QString host{};
      qint16 port{0};
      bool keep_alive = true;
      QSslSocket *socket = nullptr;
      QMutex m_write;
      QDateTime last_write;
      QTimer *timer = nullptr;
      QString last_error;
  };

} //namespace QXMPP

#endif // TRANSPORT_H

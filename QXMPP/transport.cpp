#include "transport.h"

using namespace QXMPP;

Transport::Transport(const QString server_host, const qint16 server_port, bool used_keep_alive, QObject *parent)
                    : QObject(parent), host(server_host), port(server_port), keep_alive(used_keep_alive) {
  socket = new QSslSocket(this);
  QObject::connect(socket, &QSslSocket::stateChanged, this, &Transport::slotSocketStateChanged);
  QObject::connect(socket, &QSslSocket::encrypted, this, &Transport::slotSocketEncrypted);
  QObject::connect(socket, &QSslSocket::errorOccurred, this, &Transport::slotSocketError);
  QObject::connect(socket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &Transport::slotSslErrors);
  QObject::connect(socket, &QSslSocket::readyRead, this, &Transport::slotSocketReadyRead);
  if (keep_alive) {
    timer = new QTimer(this);
    timer->setInterval(60000);
    QObject::connect(timer, &QTimer::timeout, this, &Transport::slotKeepAlive);
    last_write = QDateTime::currentDateTime();
  }
}

Transport::~Transport() {
  if (timer) {
    timer->stop();
    delete timer;
    timer = nullptr;
  }
  if (socket) {
    socket->disconnectFromHost();
    delete socket;
    socket = nullptr;
  }
}

void Transport::Connect() {
  socket->setPeerVerifyMode(QSslSocket::PeerVerifyMode::VerifyPeer);
  socket->setProtocol(QSsl::SslProtocol::TlsV1_0OrLater);
  socket->connectToHost(host, port);
  if (keep_alive) timer->start();
}

void Transport::Disconnect() {
  if (keep_alive) timer->stop();
  socket->disconnectFromHost();
}

void Transport::sendData(const QString data) {
  if (socket->state() != QAbstractSocket::ConnectedState) return;
  m_write.lock();
  socket->write(data.toUtf8());
  last_write = QDateTime::currentDateTime();
  m_write.unlock();
}

void Transport::runTLSConnect() {
  if (socket->state() != QAbstractSocket::ConnectedState) return;
  socket->startClientEncryption();
}

QString Transport::getLastError() {
  return last_error;
}

void Transport::slotSocketStateChanged(QAbstractSocket::SocketState state) {
  switch (state) {
    case QAbstractSocket::SocketState::ConnectedState: emit sigOnConnectStatus(ConnectionStatus::Connect); break;
    case QAbstractSocket::SocketState::ClosingState: emit sigOnConnectStatus(ConnectionStatus::Disconnect); break;
    case QAbstractSocket::SocketState::ConnectingState: break;
    case QAbstractSocket::SocketState::UnconnectedState: break;
    case QAbstractSocket::SocketState::HostLookupState: break;
    case QAbstractSocket::SocketState::BoundState: break;
    case QAbstractSocket::SocketState::ListeningState: break;
  }
}

void Transport::slotSocketEncrypted() {
  if (socket->isEncrypted()) emit sigOnConnectStatus(ConnectionStatus::ConnectEncryption);
  else {
    last_error = "ERROR: ENCRYPTED";
    emit sigOnConnectStatus(ConnectionStatus::Error);
  }
}

void Transport::slotSocketError(QAbstractSocket::SocketError) {
  last_error = socket->errorString();
  emit sigOnConnectStatus(ConnectionStatus::Error);
  if (socket->state() != QAbstractSocket::ConnectedState)
    slotSocketStateChanged(socket->state());
  if (keep_alive) timer->stop();
}

void Transport::slotSslErrors(const QList<QSslError> &errors) {
  for (const auto &error : errors) {
    last_error = error.errorString();
    emit sigOnConnectStatus(ConnectionStatus::Error);
  }

  if (socket->state() != QAbstractSocket::ConnectedState)
    slotSocketStateChanged(socket->state());
  if (keep_alive) timer->stop();
}

void Transport::slotSocketReadyRead() {
  QByteArray data = socket->readAll();
  QString msg = QString(data);
  emit sigOnData(msg);
}

void Transport::slotKeepAlive() {
  QDateTime now = QDateTime::currentDateTime();
  if (last_write.secsTo(now) > 90) sendData(" ");
}

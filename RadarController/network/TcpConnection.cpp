#include "TcpConnection.h"
#include "Codec.h"
#include <QDebug>

using namespace radar::net;

TcpConnection::TcpConnection(const QString& name, QObject *parent)
    : Connection(parent)
    , name_(name)
    // , codec_(nullptr)
    , socket_(nullptr)
    , socketThread_(nullptr)
{
    timer_ = new QTimer();
    connect(timer_, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
    // connect(this, &TcpConnection::socketReadData, this, &TcpConnection::onSocketReadData);
    // setCodec<Codec>();
    // timer_->start(3000);
}

TcpConnection::~TcpConnection()
{
    disconnectFromServer();
    // qDebug() << "TcpConnection::~TcpConnection()";
}

void TcpConnection::connectToServer(const QHostAddress& address, quint16 port)
{
    // qDebug() << "TcpConnection::connectToServer: " << address.toString() << ":" << port;
    QMetaObject::invokeMethod(this, "connectToServerPrivate",
                              Q_ARG(const QHostAddress&, address),
                              Q_ARG(quint16, port));
}

void TcpConnection::connectToServerPrivate(const QHostAddress& address, quint16 port)
{
    // qDebug() << "TcpConnection::connectToServerPrivate: " << address.toString() << ":" << port;
    address_ = address;
    port_ = port;
    disconnectFromServer();

    socket_ = new TcpAsyncSocket(address_, port_);
    connect(socket_, &TcpAsyncSocket::stateChanged, this, &TcpConnection::onSocketStateChanged);
    connect(socket_, &TcpAsyncSocket::dataHasBeenRead, this, &TcpConnection::onSocketDataHasBeenRead);

    socketThread_ = new QThread();
    socket_->moveToThread(socketThread_);

    connect(socketThread_, &QThread::started, socket_, &TcpAsyncSocket::asyncConnectToHost);
    connect(socketThread_, &QThread::finished, socketThread_, &QThread::deleteLater);
    connect(socketThread_, &QThread::finished, socket_, &TcpAsyncSocket::deleteLater);

    socketThread_->start();

    timer_->start(5000);
}

void TcpConnection::disconnectFromServer()
{
    if (socketThread_)
    {
        socketThread_->quit();
        socketThread_->wait();
        socket_ = nullptr;
        socketThread_ = nullptr;
    }
    timer_->stop();
    // emit disconnected();
}

QTcpSocket::SocketState TcpConnection::state() const
{
    if (socket_)
        return socket_->state();
    else
        return QTcpSocket::UnconnectedState;
}

bool TcpConnection::isConnected() const
{
    return socket_->state() == QTcpSocket::ConnectedState;
}


void TcpConnection::onTimerTimeout()
{
//    if (socket_ && socket_->state() != QTcpSocket::ConnectedState)
//    {
//        socket_->connectToHost(address_, port_);
//    }
}

void TcpConnection::onSocketStateChanged(QTcpSocket::SocketState socketState)
{
    // qDebug() << "onSocketStateChanged: " << socketState;
    switch (socketState)
    {
    case QTcpSocket::ConnectedState:
        emit connected();
        break;
    case QTcpSocket::UnconnectedState:
        emit disconnected();
    default:
        break;
    }
}

void TcpConnection::onSocketDataHasBeenRead(QByteArray data)
{
    inputBuffer()->append(data.data(), data.size());
    messageCallback_(shared_from_this(), inputBuffer(), Timestamp::currentDateTime());
}

//void TcpConnection::onCodecMessage(QByteArray& message)
//{
//    emit receiveMessage(message);
//}

//void TcpConnection::onCodecError(Codec::ErrorCode err)
//{
//    qDebug() << "onCodecError: " << err;
//}

//using std::placeholders::_1;

//void TcpConnection::setCodecPrivate()
//{
//    if (codec_)
//    {
//        codec_->setMessageCallback(std::bind(&TcpConnection::onCodecMessage, this, _1));
//        codec_->setErrorCallback(std::bind(&TcpConnection::onCodecError, this, _1));
//    }
//}

//void TcpConnection::sendMessage(const QByteArray& message)
//{
//    send(codec_->packMessage(message));
//}

void TcpConnection::send(const QByteArray& message)
{
    QMetaObject::invokeMethod(this, "sendInThread",
                              Q_ARG(const QByteArray&, message));
}

void TcpConnection::sendInThread(const QByteArray& message)
{
    if (socket_)
    {
        qint64 result = socket_->write(message);
        //QLOG_INFO()<<"TcpConnection::sendInThread write number:"<<result;
    }
}

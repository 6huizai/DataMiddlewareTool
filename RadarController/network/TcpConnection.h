#ifndef RADAR_NET_TCP_CONNECTION_H
#define RADAR_NET_TCP_CONNECTION_H

#include "Connection.h"
#include "Codec.h"
#include "Message.h"

#include <QObject>
#include <QHostAddress>
#include <QTcpSocket>
#include <QEnableSharedFromThis>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <QNetworkProxy>

namespace radar {
namespace net {

class TcpAsyncSocket : public QObject
{
    Q_OBJECT
public:
    explicit TcpAsyncSocket(QHostAddress &address, quint16 port, QObject *parent = nullptr)
        : QObject(parent)
        , address_(address)
        , port_(port)
    {
        qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
    }
    ~TcpAsyncSocket()
    {
        if (socket_)
        {
            delete socket_;
            socket_ = nullptr;
        }
    }

    QAbstractSocket::SocketState state() const
    {
        if (socket_)
            return socket_->state();
        else
            return QAbstractSocket::SocketState::UnconnectedState;
    }

    qint64 write(const char *data, qint64 len)
    {
        qint64 rc;
        QMetaObject::invokeMethod(this, "writePrivate", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(qint64, rc),
                                  Q_ARG(const char *, data),
                                  Q_ARG(qint64, len));
        return rc;
    }
    qint64 write(const char *data)
    {
        qint64 rc;
        QMetaObject::invokeMethod(this, "writePrivate", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(qint64, rc),
                                  Q_ARG(const char *, data));
        return rc;
    }
    inline qint64 write(const QByteArray &data)
    { return write(data.constData(), data.size()); }

public slots:
    void asyncConnectToHost()
    {
        // QLOG_INFO() << "asyncConnectToHost: " << address_.toString() << ":" << port_;
        socket_ = new QTcpSocket;
        socket_->setProxy(QNetworkProxy::NoProxy);
        socket_->connectToHost(address_, port_);
        connect(socket_, &QTcpSocket::readyRead, this, &TcpAsyncSocket::onReadyRead);
        connect(socket_, &QTcpSocket::stateChanged, this, &TcpAsyncSocket::onStateChanged);
    }

private:
    Q_INVOKABLE qint64 writePrivate(const char *data, qint64 len)
    {
        if (socket_)
            return socket_->write(data, len);
        else
            return 0;
    }

    Q_INVOKABLE qint64 writePrivate(const char *data)
    {
        if (socket_)
            return socket_->write(data);
        else
            return 0;
    }

signals:
    void dataHasBeenRead(QByteArray);
    void stateChanged(QAbstractSocket::SocketState);

private slots:
    void onReadyRead()
    {
        QByteArray bytes = socket_->readAll();
        // qDebug() << "onReadyRead: " << bytes.toHex(' ');
        emit dataHasBeenRead(bytes);
    }

    void onStateChanged(QAbstractSocket::SocketState state)
    {
        //qDebug()<<"onStateChanged"<<(int)state;
        emit stateChanged(state);
    }

private:
    QHostAddress address_;
    quint16 port_;
    QTcpSocket* socket_;
};

//class TcpSocketThread : QThread
//{
//    Q_OBJECT
//public:
//    explicit TcpSocketThread(QObject* parent = nullptr);

//    virtual void run() override;


//signals:
//    void dataHasBeenRead(QByteArray);
//    void socketStateChanged(QTcpSocket::SocketState socketState);

//private:
//    void onSocketStateChanged(QTcpSocket::SocketState socketState);

//private:
//    QTcpSocket *socket_;
//};

class TcpConnection : public radar::net::Connection
{
    Q_OBJECT
public:
    explicit TcpConnection(const QString& name = "", QObject *parent = nullptr);
    ~TcpConnection();

//    template <typename T>
//    void setCodec();

    void connectToServer(const QHostAddress& address, quint16 port);

    void disconnectFromServer();
    QTcpSocket::SocketState state() const;
    bool isConnected() const;

    // void sendMessage(const QByteArray& message);
    // Buffer* inputBuffer() { return &inputBuffer_; }

    virtual void send(const QByteArray& message) override;

signals:
    void connected();
    void disconnected();
    // void receiveMessage(QByteArray&);

private slots:
    void onTimerTimeout();
    // void onSocketReadyRead();
    void onSocketStateChanged(QTcpSocket::SocketState socketState);
    void onSocketDataHasBeenRead(QByteArray data);
//    void onCodecMessage(QByteArray& mesage);
//    void onCodecError(Codec::ErrorCode err);
//    void setCodecPrivate();

private:
    Q_INVOKABLE void sendInThread(const QByteArray& message);
    Q_INVOKABLE void connectToServerPrivate(const QHostAddress& address, quint16 port);
private:
    const QString name_;
    // Buffer inputBuffer_;
    // QMutex inputBufferLock_;
    // Codec *codec_;
    QTimer *timer_;
    TcpAsyncSocket *socket_;
    QThread *socketThread_;
    QHostAddress address_;
    quint16 port_;
};

//template <typename T>
//void TcpConnection::setCodec()
//{
//    static_assert(std::is_base_of<Codec, T>::value,
//                  "T must be derived from radar::net::Codec.");
//    if (codec_)
//    {
//        delete codec_;
//        codec_ = nullptr;
//    }
//    codec_ = new T();
//    setCodecPrivate();
//}

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}
}




#endif // TcpConnection_H

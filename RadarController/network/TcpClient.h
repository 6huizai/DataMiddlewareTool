#ifndef RADAR_NET_TCP_CLIENT_H
#define RADAR_NET_TCP_CLIENT_H

#include <QObject>
#include <QHostAddress>
#include <QElapsedTimer>

#include "Typedefs.h"
#include "TcpConnection.h"
#include "NodeInfo.h"
#include "Codec.h"
#include "Dispatcher.h"
#include "LinkedHashMap.h"
#include "Message.h"

namespace radar {
namespace net {

class TcpClient : public QObject
{
    Q_OBJECT
public:
    enum ClientType {
        ClientUnKnownType = -1,
        ClientTmpType,
        ClientLoginType,
        ClientUserType,
        ClientAdminType,
        ClientDebugType,
        ClientDataType,
    };
    Q_ENUM(ClientType)

    enum ClientState {
        DisconnectedState = 0,
        ConnectingState,
        ConnectedState,
        LoginState,
        LogoutingState,
        LogoutState,
    };
    Q_ENUM(ClientState)

    enum ClientError {
        NullError = 0,
        CodecError,
        MessageError,
    };
    Q_ENUM(ClientError)

public:
    explicit TcpClient(const QHostAddress& address, quint16 port,
                       int32_t clientType = ClientLoginType,
                       QObject *parent = nullptr);
    ~TcpClient();
    

    bool isLogin() const;

    ClientState state() const;
    void sendMessage(const QByteArray& message);
    void sendPacket(const QByteArray& packet);

    const TcpConnectionPtr& conn();
    Codec* codec();
    radar::net::Dispatcher* dispatcher();

    template <typename T>
    void setCodec();
    
    virtual void registerMessages();

signals:
    void sendResponse(QByteArray response);

public slots:
    void login();
    void logout();
    void heartbeat();

protected slots:
    virtual void onMessage(const ConnectionPtr&, Buffer* buffer, Timestamp);

private:
    ClientState changeState(ClientState state);

private slots:
    void onConnected();
    void onDisconnected();
    void onWaitTimerTimeout();


    void onCodecMessage(QByteArray &message);
    void onCodecErrorMessage(int32_t error);
    void onDispatcherErrorMessage(const radar::net::NodeInfoPtr&,
                                  QByteArray& message,
                                  radar::Timestamp);
    void onDispatcherBeforeMessage(const radar::net::NodeInfoPtr&,
                                   radar::net::MessagePtr& ack, radar::Timestamp);
    bool onLoginAcknowledge(const radar::net::NodeInfoPtr&,
                            std::shared_ptr<radar::net::LoginAcknowledge>& ack,
                            radar::Timestamp);
    bool onHearbeatAcknowledge(const radar::net::NodeInfoPtr&,
                               std::shared_ptr<radar::net::HeartbeatAcknowledge>& ack,
                               radar::Timestamp);

protected:
    void removeWaitMessage(uint32_t uniqueId);
    void appendWaitMessage(radar::net::MessagePtr message);

signals:
    void stateChanged(ClientState clientState);
    void error(int32_t errCode, QString errMsg);
    void message(QString msg);

private:
    typedef moss::LinkedHashMap<uint32_t, QPair<QDateTime, radar::net::MessagePtr>> TcpWaitMap;

private:
    const QHostAddress address_;
    const quint16 port_;
    NodeInfoPtr node_;;
    TcpConnectionPtr conn_;
    Codec* codec_;
    Dispatcher* dispatcher_;
    TcpWaitMap waitMap_;
    QTimer *waitTimer_;
    int waitMsecs_;
    QMutex waitMutex_;
    ClientState state_;

    QElapsedTimer heartbeatTimer_;

};

template <typename T>
void TcpClient::setCodec()
{
    static_assert(std::is_base_of<Codec, T>::value,
                  "T must be derived from radar::net::Codec.");
    if (codec_)
    {
        delete codec_;
        codec_ = nullptr;
    }
    codec_ = new T();
    codec_->setMessageCallback(std::bind(&TcpClient::onCodecMessage, this, _1));
    codec_->setErrorCallback(std::bind(&TcpClient::onCodecErrorMessage, this, _1));
}

}
}


#endif // TCPCLIENT_H

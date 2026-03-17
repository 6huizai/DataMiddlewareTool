#include "TcpClient.h"
#include <QMetaType>

using namespace radar;
using namespace radar::net;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

TcpClient::TcpClient(const QHostAddress& address, quint16 port,
                     int32_t clientType,
                     QObject *parent)
    : QObject(parent)
    , address_(address)
    , port_(port)
    , node_(new NodeInfo(clientType))
    , codec_(nullptr)
    , waitMsecs_(3000)
    , state_(DisconnectedState)
{
    qRegisterMetaType<radar::net::TcpClient::ClientType>("radar::net::TcpClient::ClientType");
    qRegisterMetaType<ClientType>("ClientType");
    qRegisterMetaType<radar::net::TcpClient::ClientState>("radar::net::TcpClient::ClientState");
    qRegisterMetaType<ClientState>("ClientState");
    qRegisterMetaType<radar::net::TcpClient::ClientError>("radar::net::TcpClient::ClientError");
    qRegisterMetaType<ClientError>("ClientError");
    qRegisterMetaType<int32_t>("int32_t");

    // 引用需要单独注册
    // qRegisterMetaType<radar::net::TcpClient::ClientType>("radar::net::TcpClient::ClientType&");

    setCodec<Codec>();

    dispatcher_ = new Dispatcher(std::bind(
                                     &TcpClient::onDispatcherErrorMessage,
                                     this, _1, _2, _3));
    dispatcher_->setBeforeCallback(std::bind(
                                       &TcpClient::onDispatcherBeforeMessage,
                                       this, _1, _2, _3));

    waitTimer_ = new QTimer();
    connect(waitTimer_, &QTimer::timeout, this, &TcpClient::onWaitTimerTimeout);
    waitTimer_->start(1000);

    heartbeatTimer_.start();
}

TcpClient::~TcpClient()
{
    waitTimer_->stop();
    delete waitTimer_;
    if (isLogin()) {
        logout();
    }
    if (dispatcher_) {
        delete dispatcher_;
        dispatcher_ = nullptr;
    }
    if (conn_) {
        conn_.reset();
    }
}

void TcpClient::registerMessages()
{
    dispatcher_->registerMessageCallback<LoginAcknowledge>(
                MSG_TYPE_LOGIN,
                std::bind(&TcpClient::onLoginAcknowledge,
                          this, _1, _2, _3));
    dispatcher_->registerMessageCallback<HeartbeatAcknowledge>(
                MSG_TYPE_HEARTBEAT,
                std::bind(&TcpClient::onHearbeatAcknowledge,
                          this, _1, _2, _3));
}


void TcpClient::login()
{
    if (isLogin()) {
        return;
    }
    logout();
    conn_.reset(new TcpConnection);
    conn_->setMessageCallback(std::bind(&TcpClient::onMessage, this, _1, _2, _3));
    connect(conn_.get(), &TcpConnection::connected, this, &TcpClient::onConnected);
    connect(conn_.get(), &TcpConnection::disconnected, this, &TcpClient::onDisconnected);
    // connect(conn_.get(), &TcpConnection::receiveMessage, this, &TcpClient::onMessage);
    // QLOG_INFO() << "TCP Client connect to: " << address_ << ":" << port_;
    conn_->connectToServer(address_, port_);
    changeState(ConnectingState);
}

void TcpClient::logout()
{
    // changeState(LogoutingState);
    if (conn_ && conn_->isConnected())
    {
        conn_->disconnectFromServer();
    }
    if (conn_) {
        disconnect(conn_.get(), 0, this, 0);
        disconnect(this, 0, conn_.get(), 0);
    }
    conn_.reset();
    changeState(DisconnectedState);
}

void TcpClient::heartbeat()
{
    std::shared_ptr<HeartbeatMessage> message = std::make_shared<HeartbeatMessage>();
    message->type = MSG_TYPE_HEARTBEAT;
    codec_->send(conn_, message->dumpToArray());
    appendWaitMessage(message);
}

bool TcpClient::isLogin() const
{
    return (state_ == LoginState);
}

TcpClient::ClientState TcpClient::changeState(ClientState state)
{
    state_ = state;
    emit stateChanged(state_);
    return state_;
}

TcpClient::ClientState TcpClient::state() const
{
    return state_;
}

void TcpClient::sendMessage(const QByteArray& message)
{
    codec_->send(conn_, message);
    //QLOG_INFO()<<"send data:"<<message.toHex(' ');
}

void TcpClient::sendPacket(const QByteArray& packet)
{
    conn_->send(packet);
}

const TcpConnectionPtr& TcpClient::conn()
{
    return conn_;
}

Codec* TcpClient::codec()
{
    return codec_;
}

Dispatcher* TcpClient::dispatcher()
{
    return dispatcher_;
}

void TcpClient::onConnected()
{
    changeState(ConnectedState);
    // 登录
    std::shared_ptr<LoginMessage> message = std::make_shared<LoginMessage>();
    message->type = MSG_TYPE_LOGIN;
    message->role = node_->type();
    message->passwd = 0x0;
    codec_->send(conn_, message->dumpToArray());
    appendWaitMessage(message);
}

void TcpClient::onDisconnected()
{
    // 默认自动重连，收到Socket断开状态更新当前状态为正在连接。
    // qDebug() << "TcpClient::onDisconnected: " << state();
//    if (LogoutingState == state())
//        changeState(DisconnectedState);
//    else if (DisconnectedState != state())
//        changeState(ConnectingState);

    changeState(DisconnectedState);
}

void TcpClient::onMessage(const ConnectionPtr&,
                          Buffer* buffer,
                          Timestamp)
{
    // qDebug() << "TcpClient::onMessage: " << message.toHex(' ');
    codec_->onMessage(buffer);
}

void TcpClient::onCodecMessage(QByteArray &message)
{
    //qDebug() << "TcpClient::onCodecMessage: " << message.toHex(' ');
    dispatcher_->onByteArryMessage(
                node_, message, Timestamp::currentDateTime());
}

void TcpClient::onCodecErrorMessage(int32_t err)
{
    emit error(CodecError, "Codec Error Code: " + QString::number(err));
}

void TcpClient::onDispatcherErrorMessage(const NodeInfoPtr&,
                                         QByteArray& message, Timestamp)
{
    //qDebug() << "TcpClient unknown message: " << message.toHex(' ');
}

void TcpClient::onDispatcherBeforeMessage(const NodeInfoPtr&,
                                          MessagePtr& ack, Timestamp)
{
    removeWaitMessage(ack->uniqueId);
}

void TcpClient::onWaitTimerTimeout()
{
    {
        QMutexLocker locker(&waitMutex_);
        while (waitMap_.size() > 0) {
            QPair<QDateTime, MessagePtr> pair = waitMap_.back();
            if (pair.first.msecsTo(QDateTime::currentDateTime()) > waitMsecs_)
            {
                //record the overtime ids
                emit error(MessageError, QString("%1 timeout.").arg(pair.second->toString()));
                waitMap_.pop_back();
                if (pair.second->type == MSG_TYPE_LOGIN)
                {
                    // 登录命令特殊处理，登录失败则断开连接
                    logout();
                }
                else if (pair.second->type == MSG_TYPE_HEARTBEAT)
                {
                    // 心跳命令特殊处理，登录失败则断开连接
                    logout();
                }
            } else {
                //the last don't overtime,so all is ok
                break;
            }
        }
    } //this is end of tcpWaitMutex_
    // 判断心跳指令是否超时
    {
        // if (isLogin() && heartbeatTimer_.elapsed() > 10000)
        // {
        //     // 10秒间隔发送心跳指令包
        //     heartbeat();
        //     heartbeatTimer_.restart();
        // }
    } //this is end of tcpWaitMutex_
}

bool TcpClient::onLoginAcknowledge(const NodeInfoPtr&,
                                   std::shared_ptr<LoginAcknowledge>& ack,
                                   Timestamp)
{
    if (ack->role == node_->type() && ack->state == 0)
    {
        changeState(LoginState);
        heartbeatTimer_.restart();
    }
    else
    {
        emit error(MessageError, QString("Login radar error(type = %1, state = %2)").arg(ack->role).arg(ack->state));
        logout();
    }
    return true;
}

bool TcpClient::onHearbeatAcknowledge(const radar::net::NodeInfoPtr&,
                           std::shared_ptr<radar::net::HeartbeatAcknowledge>& ack,
                           radar::Timestamp)
{
    Q_UNUSED(ack);
    // QLOG_DEBUG() << "TcpClient::onHearbeatAcknowledge";
    return true;
}

void TcpClient::removeWaitMessage(uint32_t uniqueId)
{
    QMutexLocker locker(&waitMutex_);
    QPair<QDateTime, radar::net::MessagePtr> *pair = waitMap_.find(uniqueId);
    if (pair == 0) {
        return;
    }
    waitMap_.remove(uniqueId);
    // QLOG_DEBUG() << "removeWaitMessage: " << pair->second->toString();
}
void TcpClient::appendWaitMessage(radar::net::MessagePtr message)
{
    QPair<QDateTime, MessagePtr> pair(QDateTime::currentDateTime(), message);
    QMutexLocker locker(&waitMutex_);
    waitMap_.push_front(message->uniqueId, pair);
    // QLOG_DEBUG() << "appendWaitMessage: " << message->toString();
}





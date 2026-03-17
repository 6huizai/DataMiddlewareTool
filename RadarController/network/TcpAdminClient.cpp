#include "TcpAdminClient.h"
#include "Message.h"
#include <QDebug>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

using namespace radar;
using namespace radar::net;

TcpAdminClient::TcpAdminClient(const QHostAddress& address, quint16 port,
                               QObject *parent)
//#if defined(QRADAR_RELEASE_VERSION)
    : TcpClient(address, port, ClientAdminType, parent)
//#else
//    : TcpClient(address, port, ClientDebugType, parent)
//#endif
{

}

void TcpAdminClient::registerMessages()
{
    TcpClient::registerMessages();
    dispatcher()->registerMessageCallback<SetRegisterAcknowledge>(
                MSG_TYPE_SETREG,
                std::bind(&TcpAdminClient::onSetRegisterAcknowledge,
                          this, _1, _2, _3));
    dispatcher()->registerMessageCallback<GetRegisterAcknowledge>(
                MSG_TYPE_GETREG,
                std::bind(&TcpAdminClient::onGetRegisterAcknowledge,
                          this, _1, _2, _3));
    dispatcher()->registerMessageCallback<SetMuxRegisterAcknowledge>(
                MSG_TYPE_SETMUXREG,
                std::bind(&TcpAdminClient::onSetMuxRegisterAcknowledge,
                          this, _1, _2, _3));
    dispatcher()->registerMessageCallback<GetMuxRegisterAcknowledge>(
                MSG_TYPE_GETMUXREG,
                std::bind(&TcpAdminClient::onGetMuxRegisterAcknowledge,
                          this, _1, _2, _3));
    dispatcher()->registerMessageCallback<GetJsonMessageAcknowledge>(
                MSG_TYPE_GETJSON,
                std::bind(&TcpAdminClient::onGetJsonAcknowledge,
                          this, _1, _2, _3));
}

void TcpAdminClient::setRegisters(RegisterList& registers)
{
    if (!isLogin())
    {
        emit error(MessageError, "The radar is not logged in, please log in and try again.");
        return;
    }
    std::shared_ptr<SetRegisterMessage> message = std::make_shared<SetRegisterMessage>();
    message->type = MSG_TYPE_SETREG; //0x00000040设置寄存器
    message->registers = registers;
    sendMessage(message->dumpToArray());
    appendWaitMessage(message);
}

void TcpAdminClient::getRegisters(QList<uint32_t>& addresses)
{
    if (!isLogin())
    {
        emit error(MessageError, "The radar is not logged in, please log in and try again.");
        return;
    }
    std::shared_ptr<GetRegisterMessage> message = std::make_shared<GetRegisterMessage>();
    message->type = MSG_TYPE_GETREG; //0x00000041获取寄存器
    message->addresses = addresses;
    sendMessage(message->dumpToArray());
    appendWaitMessage(message);
}

void TcpAdminClient::setMuxRegisters(uint32_t address,
                                     MuxRegisterList& muxRegisters)
{
    if (!isLogin())
    {
        emit error(MessageError, "The radar is not logged in, please log in and try again.");
        return;
    }
    std::shared_ptr<SetMuxRegisterMessage> message = std::make_shared<SetMuxRegisterMessage>();
    message->type = MSG_TYPE_SETMUXREG;
    message->address = address;
    message->muxRegisters = muxRegisters;
    sendMessage(message->dumpToArray());
    appendWaitMessage(message);
}

void TcpAdminClient::getMuxRegisters(uint32_t address,
                                     QList<uint16_t> muxAddresses)
{
    if (!isLogin())
    {
        emit error(MessageError, "The radar is not logged in, please log in and try again.");
        return;
    }
    std::shared_ptr<GetMuxRegisterMessage> message = std::make_shared<GetMuxRegisterMessage>();
    message->type = MSG_TYPE_GETMUXREG;
    message->address = address;
    message->muxAddresses = muxAddresses;
    sendMessage(message->dumpToArray());
    appendWaitMessage(message);
}

void TcpAdminClient::setJsonMessage(uint32_t address, QString jsonStr)
{
    if (!isLogin())
    {
        emit error(MessageError, "The radar is not logged in, please log in and try again.");
        return;
    }
    std::shared_ptr<SetJsonMessage> message = std::make_shared<SetJsonMessage>();
    message->type = MSG_TYPE_SETJSON;

    message->address = address;
    message->payloadLen = jsonStr.length()+1;
    message->payload = jsonStr.toUtf8();

    sendMessage(message->dumpToArray());
    appendWaitMessage(message);
}

void TcpAdminClient::getJsonMessage(uint32_t address)
{
    if (!isLogin())
    {
        emit error(MessageError, "The radar is not logged in, please log in and try again.");
        return;
    }
    std::shared_ptr<GetJsonMessage> message = std::make_shared<GetJsonMessage>();
    message->type = MSG_TYPE_GETJSON;

    message->address = address;
    sendMessage(message->dumpToArray());
    appendWaitMessage(message);
}

void TcpAdminClient::setSelectedTrackMessage(QSet<uint32_t> trackIds)
{
    if (!isLogin())
    {
        emit error(MessageError, "The radar is not logged in, please log in and try again.");
        return;
    }
    std::shared_ptr<SetSelectedTrackMessage> message = std::make_shared<SetSelectedTrackMessage>();
    message->type = MSG_TYPE_SETSELECTEDTRACK;

    message->size = trackIds.size();
    message->trackIds = trackIds;

    sendMessage(message->dumpToArray());
    appendWaitMessage(message);
}

void TcpAdminClient::getSelectedTrackMessage()
{
    if (!isLogin())
    {
        emit error(MessageError, "The radar is not logged in, please log in and try again.");
        return;
    }
    std::shared_ptr<GetSelectedTrackMessage> message = std::make_shared<GetSelectedTrackMessage>();
    message->type = MSG_TYPE_GETSELECTEDTRACK;
    sendMessage(message->dumpToArray());
    appendWaitMessage(message);
}

bool TcpAdminClient::onSetRegisterAcknowledge(const NodeInfoPtr&,
                              std::shared_ptr<SetRegisterAcknowledge>& ack,
                              Timestamp)
{
    if (ack->state == 0)
    {
        // qDebug() << "设置寄存器成功";
        emit message("Register set successfully.");
    }
    else
    {
        // qDebug() << "设置寄存器失败";
        emit error(MessageError, "Failed to set register.");
    }
    return true;
}

bool TcpAdminClient::onGetRegisterAcknowledge(const NodeInfoPtr&,
                              std::shared_ptr<GetRegisterAcknowledge>& ack,
                              Timestamp)
{
    emit sendResponse(ack->dumpToArray());

    emit receiveRegisters(ack->registers);
    // qDebug() << "获取寄存器数量： " << ack->registers.size();
    // qDebug() << ack->registers;
    emit message(QString("Get the number of registers：%1").arg(ack->registers.size()));
    return true;
}

bool TcpAdminClient::onSetMuxRegisterAcknowledge(const NodeInfoPtr&,
                                 std::shared_ptr<SetMuxRegisterAcknowledge>& ack,
                                 Timestamp)
{
    if (ack->state == 0)
    {
        //qDebug() << "设置复用寄存器成功";
        emit message("Set the multiplexing register successfully.");
    }
    else
    {
        //qDebug() << "设置复用寄存器失败";
        emit error(MessageError, "Failed to set multiplexing register.");
    }
    return true;
}

bool TcpAdminClient::onGetMuxRegisterAcknowledge(const NodeInfoPtr&,
                                 std::shared_ptr<GetMuxRegisterAcknowledge>& ack,
                                 Timestamp)
{
    emit receiveMuxRegisters(ack->address, ack->muxRegisters);
    //qDebug() << "获取复用寄存器数量： " << ack->muxRegisters.size();
    // qDebug() << ack->muxRegisters;
    emit message(QString("Get the number of multiplexing registers：%1").arg(ack->muxRegisters.size()));
    return true;
}

bool TcpAdminClient::onGetJsonAcknowledge(const NodeInfoPtr&,
                                 std::shared_ptr<GetJsonMessageAcknowledge>& ack,
                                 Timestamp)
{
    emit sendResponse(ack->dumpToArray());
    emit receiveJsonConfig(ack->address, ack->payload);
    return true;
}

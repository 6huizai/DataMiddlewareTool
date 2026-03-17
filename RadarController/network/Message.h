#ifndef RADAR_NET_MESSAGE_H
#define RADAR_NET_MESSAGE_H

#include "Timestamp.h"
#include "Typedefs.h"
#include "NodeInfo.h"

#include <QByteArray>
#include <QDataStream>
#include <QSet>

#include <memory>
#include <functional>
#include <stdint.h>
#include <atomic>


namespace radar {
namespace net {

enum MessageTypeEnum
{
    MSG_TYPE_DEVINFO = 0,
    MSG_TYPE_HEARTBEAT,    //0x00000001心跳指令
    MSG_TYPE_LOGIN,        //0x00000002登陆指令
    MSG_TYPE_ISLOGIN,

    MSG_TYPE_SETREG = 64,  //0x00000040设置寄存器
    MSG_TYPE_GETREG,       //0x00000041获取寄存器
    MSG_TYPE_SETMUXREG,
    MSG_TYPE_GETMUXREG,

    MSG_TYPE_SETJSON = 128,
    MSG_TYPE_GETJSON,

    MSG_TYPE_SETSELECTEDTRACK,
    MSG_TYPE_GETSELECTEDTRACK,

    MSG_TYPE_USER = 0x10000,
    MSG_TYPE_UPLOAD_DATA = MSG_TYPE_USER,
};

typedef int32_t MessageType;

class Message
{
    friend class MessageFactory;
public:
    uint32_t type;
    uint32_t uniqueId;

    // explicit Message(uint32_t t) { type = t; }
    virtual ~Message() = default;
    virtual QByteArray dumpToArray();
    virtual bool parseFromArray(QByteArray& array);
    virtual QString toString() const { return QString("Message[type:%1]").arg(type); }

protected:
    static std::atomic<uint32_t> localUniqueId_;
    virtual void dumpBody(QDataStream&) {}
    virtual bool parseBody(QByteArray&) { return true; }

private:
    const static int kHeaderLen = 2 * sizeof(uint32_t);
};

typedef std::shared_ptr<Message> MessagePtr;

class MessageFactory
{
public:
    virtual ~MessageFactory() = default;
    virtual Message* createMessage() = 0;
    virtual bool onMessage(const NodeInfoPtr&,
                           MessagePtr& message,
                           Timestamp) const = 0;
};

template <typename T>
class MessageFactoryT : public MessageFactory
{
    static_assert(std::is_base_of<Message, T>::value,
            "T must be derived from radar::net::Message.");
public:
    typedef std::function<bool (const NodeInfoPtr&, std::shared_ptr<T>&, Timestamp)> MessageTCallback;

    explicit MessageFactoryT(const MessageTCallback& callback)
        : callback_(callback)
    {
    }

    virtual Message* createMessage() override
    {
        return new T;
    }

    virtual bool onMessage(const NodeInfoPtr& conn,
                   MessagePtr& message,
                   Timestamp receiveTime) const override
    {
        std::shared_ptr<T> concrete = std::static_pointer_cast<T>(message);
        assert(concrete != NULL);
        return callback_(conn, concrete, receiveTime);
    }

private:
    MessageTCallback callback_;
};

class DevInfoMessage : public Message
{
protected:
    virtual bool parseBody(QByteArray& array) override { (void)array; return true; }
};

class LoginMessage : public Message
{
public:
    int32_t role;
    int32_t passwd;

public:
    virtual QString toString() const override;

protected:
    virtual bool parseBody(QByteArray& array) override;
    virtual void dumpBody(QDataStream& stream) override;
};

class HeartbeatMessage : public Message
{
public:
    qint64 timestamp;
protected:
    bool parseBody(QByteArray& array) override;
    virtual void dumpBody(QDataStream& stream) override;
};

typedef HeartbeatMessage HeartbeatAcknowledge;

class Acknowledge : public Message
{
public:
    int16_t state;

protected:
    virtual bool parseBody(QByteArray& array) override;
    virtual void dumpBody(QDataStream& stream) override;
};

class LoginAcknowledge : public Message
{
public:
    int32_t role;
    int16_t state;

protected:
    virtual bool parseBody(QByteArray& array) override;
    virtual void dumpBody(QDataStream& stream) override;
};

class SetRegisterMessage : public Message
{
public:
    RegisterList registers;

protected:
    virtual bool parseBody(QByteArray& array) override;
    virtual void dumpBody(QDataStream& stream) override;
};

typedef Acknowledge SetRegisterAcknowledge;

class GetRegisterMessage : public Message
{
public:
    QList<uint32_t> addresses;

protected:
    virtual bool parseBody(QByteArray& array) override;
    virtual void dumpBody(QDataStream& stream) override;
};

typedef SetRegisterMessage GetRegisterAcknowledge;

class SetMuxRegisterMessage : public Message
{
public:
    uint32_t address;
    MuxRegisterList muxRegisters;

protected:
    virtual bool parseBody(QByteArray& array) override;
    virtual void dumpBody(QDataStream& stream) override;
};

typedef Acknowledge SetMuxRegisterAcknowledge;

class GetMuxRegisterMessage : public Message
{
public:
    uint32_t address;
    QList<uint16_t> muxAddresses;

protected:
    virtual bool parseBody(QByteArray& array) override;
    virtual void dumpBody(QDataStream& stream) override;
};

typedef SetMuxRegisterMessage GetMuxRegisterAcknowledge;


class SetJsonMessage : public Message
{
public:
    uint16_t address;
    uint16_t payloadLen;
    uint16_t dump0;
    uint16_t dump1;
    QByteArray payload;

protected:
    virtual bool parseBody(QByteArray& array) override;
    virtual void dumpBody(QDataStream& stream) override;
};

class GetJsonMessage : public Message
{
public:
    uint16_t address;
    uint16_t payloadLen;
    uint16_t dump0;
    uint16_t dump1;

protected:
    virtual void dumpBody(QDataStream& stream) override;
};
typedef SetJsonMessage GetJsonMessageAcknowledge;

class SetSelectedTrackMessage : public Message
{
public:
    uint16_t size;
    uint16_t dump0;
    QSet<uint32_t> trackIds;

protected:
    virtual bool parseBody(QByteArray& array) override;
    virtual void dumpBody(QDataStream& stream) override;
};

class GetSelectedTrackMessage : public Message
{
public:
    uint32_t dump0;
    uint32_t dump1;

protected:
    virtual void dumpBody(QDataStream& stream) override;
};
typedef SetSelectedTrackMessage GetSelectedTrackMessageAcknowledge;


class UploadSampleDataMessage : public Message
{
public:
    typedef struct {
        qint64 timestamp;
        uint32_t frameNumber;
        uint16_t kindA;  // 原始数据类型，ADC、脉压等
        uint16_t kindB;  // 传输数据类型，Beam、CPI等
        uint16_t sequenceNumber;  // 序号，Beam为波位号、CPI为CPI序号
        uint16_t distanceBegin;  // 起始距离单元号
        uint16_t distanceNum;  // 距离单元数量
    }SampleDataInfo;

public:
    SampleDataInfo info;
    QByteArray data;

protected:
    virtual bool parseBody(QByteArray& array) override;
};

}
}

#endif

#ifndef RADAR_NET_TCPCODEC_H
#define RADAR_NET_TCPCODEC_H

#include "Connection.h"
#include <functional>
#include <QByteArray>
#include <QSharedPointer>

namespace radar {
namespace net {

class Codec
{
public:
    enum ErrorCode
    {
        kNoError = 0,
        kInvalidLength,
        kCheckSumError,
        kInvalidNameLen,
        kUnknownMessageType,
        kParseError,
    };

    typedef std::function<void (QByteArray &)> MessageCallback;
    typedef std::function<void (int32_t /* ErrorCode */)> ErrorCallback;

public:
    explicit Codec();
    virtual ~Codec() = default;

    virtual void send(const ConnectionPtr& conn, const QByteArray& message);
    virtual QByteArray packMessage(const QByteArray& message);
    virtual void onMessage(Buffer* buffer, bool doChecksum = true);

    void setMessageCallback(const MessageCallback& cb);
    void setErrorCallback(const ErrorCallback& cb);

    static void defaultMessageCallback(QByteArray &) {}
    static void defaultErrorCallback(int32_t) {}

protected:
    MessageCallback messageCallback_;
    ErrorCallback errorCallback_;
    const static int kHeaderLen = 2*sizeof(int32_t);
    const static int kCrc16Len = sizeof(uint16_t);
    const static int kMinMessageLen = kHeaderLen + kCrc16Len; // nameLen + typeName + checkSum
    const static int kMaxMessageLen = 128*1024*1024; // same as codec_stream.h kDefaultTotalBytesLimit
    const static uint32_t kHeaderMagic = 0x55AA55AA;
};

}
}





#endif // TCPCODEC_H

#include "Codec.h"
#include "TcpConnection.h"
#include <QDataStream>
#include <QtEndian>
#include "crc16modbus.h"

using std::placeholders::_1;

namespace radar {
namespace net {

Codec::Codec()
    : messageCallback_(std::bind(&Codec::defaultMessageCallback, _1))
    , errorCallback_(std::bind(&Codec::defaultErrorCallback, _1))

{

}

void Codec::send(const ConnectionPtr& conn, const QByteArray& message)
{
    conn->send(packMessage(message));
    //QLOG_INFO()<<"send packet:"<<packMessage(message).toHex(' ');
}

QByteArray Codec::packMessage(const QByteArray& message)
{
    // uint16_t checksum = 0xCCCC;
    uint16_t checksum = CRC16_MODBUS(message.data(), message.size());
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << kHeaderMagic;
    stream << static_cast<uint32_t>(message.size() + kCrc16Len);
    stream.writeRawData(message.data(), message.size());
    stream << (uint8_t)((checksum >> 8) & 0xFF);
    stream << (uint8_t)(checksum & 0xFF);
    return buffer;
}

static int32_t asInt32(const char* buf)
{
    qint32 be32;
    memcpy(&be32, buf, sizeof(be32)); // 解决潜在的内存对齐问题
    return qFromBigEndian(be32);      // Qt 内置的字节序转换
}


void Codec::onMessage(Buffer* buf, bool doChecksum)
{
    while (buf->readableBytes() >= kMinMessageLen + kHeaderLen)
    {
        const char* p = buf->peek();
        int32_t magic = asInt32(p);
        int32_t len = asInt32(p+4);
        if (magic != kHeaderMagic || len > kMaxMessageLen || len < kMinMessageLen)
        {
            errorCallback_(kInvalidLength);
            buf->retrieveAll();
            break;
        }
        else if (buf->readableBytes() >= implicit_cast<size_t>(len + kHeaderLen))
        {
            ErrorCode errorCode = kNoError;
            if (doChecksum)
            {
                const uint8_t *pcrc = (uint8_t*)(buf->peek())+kHeaderLen+len-kCrc16Len;
                uint16_t crc = (pcrc[0] << 8) | pcrc[1];
                if ((crc == 0xCCCC) || (0 == CRC16_MODBUS((buf->peek()+kHeaderLen), len))) {
                    errorCode = kNoError;
                } else {
                    QByteArray ba(buf->peek()+kHeaderLen, len);
                    //QLOG_ERROR() << "CRC CODE: " << QString::number(crc, 16);
                    //QLOG_ERROR() << "CRC ERROR: " << ba.toHex(' ');
                    errorCode = kCheckSumError;
                }
            }
            if (errorCode == kNoError)
            {
                QByteArray message;
                message.append(buf->peek()+kHeaderLen, len-kCrc16Len);
                buf->retrieve(kHeaderLen+len);
                messageCallback_(message);
            }
            else
            {
                errorCallback_(errorCode);
                buf->retrieveAll();
                break;
            }
        }
        else
        {
            break;
        }
    }
}

void Codec::setMessageCallback(const MessageCallback& cb)
{
    messageCallback_ = cb;
}

void Codec::setErrorCallback(const ErrorCallback& cb)
{
    errorCallback_ = cb;
}

}
}



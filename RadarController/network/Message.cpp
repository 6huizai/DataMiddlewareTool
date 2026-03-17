#include "Message.h"
#include <QDataStream>

namespace radar {
namespace net {

std::atomic<uint32_t> Message::localUniqueId_(0);

QByteArray Message::dumpToArray()
{
    uniqueId = localUniqueId_.fetch_add(1);
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << type;
    stream << uniqueId;
    dumpBody(stream);
    return array;
}

bool Message::parseFromArray(QByteArray& array)
{
    if (array.size() < kHeaderLen)
    {
        return false;
    }
    else
    {
        QDataStream stream(&array, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream >> type;
        stream >> uniqueId;
        auto body = array.mid(kHeaderLen);
        return parseBody(body);
    }
}

QString LoginMessage::toString() const
{
    return QString("[登录命令：type=%1]").arg(role);
}

bool LoginMessage::parseBody(QByteArray& array)
{
    if (array.size() == 2*sizeof(uint32_t))
    {
        QDataStream stream(&array, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream >> role;
        stream >> passwd;
        return true;
    }
    else
    {
        return false;
    }
}

bool HeartbeatMessage::parseBody(QByteArray& array)
{
    if (array.size() == sizeof(int64_t))
    {
        QDataStream stream(&array, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream >> timestamp;
        return true;
    }
    else
    {
        return false;
    }
}

void HeartbeatMessage::dumpBody(QDataStream& stream)
{
    stream << timestamp;
}


void LoginMessage::dumpBody(QDataStream& stream)
{
    stream << role;
    stream << passwd;
}

bool Acknowledge::parseBody(QByteArray& array)
{
    if (array.size() == sizeof(uint16_t))
    {
        QDataStream stream(&array, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream >> state;
        return true;
    }
    else
    {
        return false;
    }
}

void Acknowledge::dumpBody(QDataStream& stream)
{
    stream << state;
}

bool LoginAcknowledge::parseBody(QByteArray& array)
{
    if (array.size() == sizeof(uint32_t) + sizeof(uint16_t))
    {
        QDataStream stream(&array, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream >> role;
        stream >> state;
        return true;
    }
    else
    {
        return false;
    }
}

void LoginAcknowledge::dumpBody(QDataStream& stream)
{
    stream << role;
    stream << state;
}

bool SetRegisterMessage::parseBody(QByteArray& array)
{
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    int32_t regNum = 0;
    registers.clear();
    if (static_cast<size_t>(array.size()) < sizeof(uint32_t))
    {
        return false;
    }
    stream >> regNum;
    if (static_cast<size_t>(array.size()) != (regNum * 2 * sizeof(uint32_t) + sizeof(uint32_t)))
    {
        return false;
    }
    for (int i=0; i<regNum; ++i)
    {
        uint32_t addr, value;
        stream >> addr;
        stream >> value;
        registers.push_back(QPair<uint32_t, uint32_t>(addr, value));
    }
    return true;
}

void SetRegisterMessage::dumpBody(QDataStream& stream)
{
    int32_t regNum = registers.size();
    stream << regNum;
    for (int i=0; i<regNum; ++i)
    {
        stream << registers.at(i).first;
        stream << registers.at(i).second;
    }
}

bool GetRegisterMessage::parseBody(QByteArray& array)
{
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    int32_t regNum = 0;
    addresses.clear();
    if (static_cast<size_t>(array.size()) < sizeof(uint32_t))
    {
        return false;
    }
    stream >> regNum;
    if (static_cast<size_t>(array.size()) != (regNum * sizeof(uint32_t) + sizeof(uint32_t)))
    {
        return false;
    }
    for (int i=0; i<regNum; ++i)
    {
        uint32_t address;
        stream >> address;
        addresses.push_back(address);
    }
    return true;
}

void GetRegisterMessage::dumpBody(QDataStream& stream)
{
    int32_t regNum = addresses.size();
    stream << regNum;
    for (int i=0; i<regNum; ++i)
    {
        stream << addresses.at(i);
    }
}

bool SetMuxRegisterMessage::parseBody(QByteArray& array)
{
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    if (static_cast<size_t>(array.size()) < 2 * sizeof(int32_t))
    {
        return false;
    }
    int32_t regNum = 0;
    stream >> address;
    stream >> regNum;
    if (static_cast<size_t>(array.size()) != (regNum * 2 * sizeof(uint16_t) + 2 * sizeof(uint32_t)))
    {
        return false;
    }
    for (int i=0; i<regNum; ++i)
    {
        uint16_t muxAddress, muxValue;
        stream >> muxAddress;
        stream >> muxValue;
        muxRegisters.push_back(QPair<uint16_t, uint16_t>(muxAddress, muxValue));
    }
    return true;
}

void SetMuxRegisterMessage::dumpBody(QDataStream& stream)
{
    int32_t regNum = muxRegisters.size();
    stream << address;
    stream << regNum;
    for (int i=0; i<regNum; ++i)
    {
        stream << muxRegisters.at(i).first;
        stream << muxRegisters.at(i).second;
    }
}

bool GetMuxRegisterMessage::parseBody(QByteArray& array)
{
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    if (static_cast<size_t>(array.size()) < 2 * sizeof(int32_t))
    {
        return false;
    }
    int32_t regNum = 0;
    stream >> address;
    stream >> regNum;
    if (static_cast<size_t>(array.size()) != (regNum * sizeof(uint16_t) + 2 * sizeof(uint32_t)))
    {
        return false;
    }
    for (int i=0; i<regNum; ++i)
    {
        uint16_t muxAddress;
        stream >> muxAddress;
        muxAddresses.push_back(muxAddress);
    }
    return true;
}

void GetMuxRegisterMessage::dumpBody(QDataStream& stream)
{
    int32_t regNum = muxAddresses.size();
    stream << address;
    stream << regNum;
    for (int i=0; i<regNum; ++i)
    {
        stream << muxAddresses.at(i);
    }
}

bool SetJsonMessage::parseBody(QByteArray& array)
{
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    if (static_cast<size_t>(array.size()) < 2 * sizeof(int32_t)) {
        return false;
    }

    stream >> address;
    stream >> payloadLen;
    stream >> dump0;
    stream >> dump1;
    payload.resize(payloadLen);
    stream.readRawData(payload.data(), payloadLen);
    return true;
}

void SetJsonMessage::dumpBody(QDataStream& stream)
{
    stream << address;
    payloadLen = payload.size();
    stream << payloadLen;
    stream << dump0;
    stream << dump1;
    //stream << payload;
    stream.writeRawData(payload.data(), payload.size());

}

void GetJsonMessage::dumpBody(QDataStream& stream)
{
    stream << address;
    stream << payloadLen;
    stream << dump0;
    stream << dump1;
}

bool SetSelectedTrackMessage::parseBody(QByteArray& array)
{
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    if (static_cast<size_t>(array.size()) < 2 * sizeof(int32_t)) {
        return false;
    }

    stream >> size;
    stream >> dump0;
    uint32_t tid;
    trackIds.clear();
    for (int i = 0; i < size; i++)
    {
        stream >> tid;
        trackIds.insert(tid);
    }
    return true;
}

void SetSelectedTrackMessage::dumpBody(QDataStream& stream)
{
    stream << size;
    stream << dump0;
    foreach (auto tid, trackIds) {
        stream << uint32_t(tid);
    }
}

void GetSelectedTrackMessage::dumpBody(QDataStream& stream)
{
    stream << dump0;
    stream << dump1;
}

bool UploadSampleDataMessage::parseBody(QByteArray& array)
{
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> info.timestamp;
    stream >> info.frameNumber;
    stream >> info.kindA;
    stream >> info.kindB;
    stream >> info.sequenceNumber;
    stream >> info.distanceBegin;
    stream >> info.distanceNum;
    data.swap(array);
    return true;
}

}
}

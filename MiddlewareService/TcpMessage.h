#ifndef TCPMESSAGE_H
#define TCPMESSAGE_H
#include <cstdint>
#include <vector>
#include <QByteArray>
#include <QObject>
#include <QDataStream>
#include <QSet>
#include <QDebug>

enum class MessageType1
{
    HeartBeat = 0x01,
    Login = 0x02,
    SetReg = 0x40,
    GetReg = 0x41,
    SetJson = 0x80,
    GetJson = 0x81,
    SetSelect = 0x82,
    TargetV2 = 0x00010002,
    TrackV2 = 0x00020002,
    TargetV3 = 0x00030001,
    TrackV3 = 0x00030002,
    RequestRTK = 0x00010005,
    UploadRTK = 0x00010006
};

class Acknowledge
{
public:
    int16_t state;

    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << state;
        return data;
    }
};

class LoginMessage
{
public:
    uint32_t role = 0x00000003; //权限类型
    uint32_t code = 0x00000000; //权限识别码
    uint16_t status = 0x0000;

    void parseBody(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream >> role;
        stream >> code;
    }
    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << role;
        stream << status;

        return data;
    }
};

class HeartbeatMessage
{
public:
    int64_t timestamp;

    void parseBody(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream >> timestamp;
    }
    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << timestamp;

        return data;
    }
};

typedef QPair<uint32_t, uint32_t> Register;
typedef QList<Register> RegisterList;

class SetRegisterMessage1
{
public:
    RegisterList registers;

    bool parseBody(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        int32_t regNum = 0;
        registers.clear();
        if (static_cast<size_t>(data.size()) < sizeof(uint32_t))
        {
            return false;
        }
        stream >> regNum;
        if (static_cast<size_t>(data.size()) != (regNum * 2 * sizeof(uint32_t) + sizeof(uint32_t)))
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

    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);

        int32_t regNum = registers.size();
        stream << regNum;
        for (int i=0; i<regNum; ++i)
        {
            stream << registers.at(i).first;
            stream << registers.at(i).second;
        }

        return data;
    }
};

typedef SetRegisterMessage1 GetRegResponseMessage1;

class GetRegisterMessage1
{
public:
    QList<uint32_t> addresses;

    void parseBody(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        uint32_t num;
        uint32_t addr;
        stream >> num;
        addresses.clear();
        for (int i = 0; i < num; ++i) {
            stream >> addr;
            addresses.push_back(addr);
        }
    }
    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << (uint32_t)0;

        return data;
    }
};

typedef GetRegisterMessage1 SetRegResponseMessage1;

class GetJsonMessage
{
public:
    uint16_t address;
    uint16_t payloadLen;
    uint16_t dump0;
    uint16_t dump1;

    void parseBody(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream >> address;
        stream >> payloadLen;
        stream >> dump0;
        stream >> dump1;
    }
    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << address;
        stream << uint16_t(0);
        stream << uint32_t(0);

        return data;
    }
};

class SetSelectedTrackMessage
{
public:
    uint16_t size;
    uint16_t dump0;
    QSet<uint32_t> trackIds;

    void parseBody(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream >> size;
        stream >> dump0;
        uint32_t trackId;
        trackIds.clear();
        for (int i = 0; i < size; ++i) {
            stream >> trackId;
            trackIds.insert(trackId);
        }

    }
    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << uint16_t(0);

        return data;
    }
};

class CFarMessageV2
{
public:
    struct Point {
        int32_t distance;  ///< 距离,分辨率：0.01米
        int32_t angle; ///< 角度，分辨率：0.0001度
        uint16_t doppler;  ///< 多普勒
        uint16_t snr; ///< 信噪比, 分辨率：0.01dB
        uint16_t beam;  ///< 波位
        uint16_t pluse;  ///< 脉冲索引，指示点迹所在的脉冲
    };
public:
    int64_t timestamp;
    uint32_t frame;
    std::vector<Point> points;

    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << timestamp;
        stream << frame;
        stream << points.size();
        for (int i=0; i<points.size(); i++)
        {
            Point p = points[i];
            stream << p.distance;
            stream << p.angle;
            stream << p.doppler;
            stream << p.snr;
            stream << p.beam;
            stream << p.pluse;
        }

        return data;
    }

};

class CFarMessageV3
{
public:
    struct Point {
        int32_t x;      ///< x轴坐标, 分辨率：0.01米
        int32_t y;      ///< y轴坐标, 分辨率：0.01米
        int32_t z;      ///< z轴坐标, 分辨率：0.01米
        uint16_t di;    ///< 多普勒单元号
        uint16_t ri;    ///< 距离单元号
        uint16_t snr;   ///< 信噪比, 分辨率：0.01dB
        uint16_t beam;   ///< 波位
        int32_t dump0;
    };
public:
    int64_t timestamp;
    int64_t frame;
    int32_t scanBoundaryA; ///< 扫描左边界，分辨率：0.0001度
    int32_t scanBoundaryB; ///< 扫描右边界，分辨率：0.0001度
    int32_t scanningDirection; ///< 扫描方向，0: 顺时针扫描, 2: 逆时针扫描
    std::vector<Point> points;


    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << timestamp;
        stream << frame;
        stream << scanBoundaryA;
        stream << scanBoundaryB;
        stream << scanningDirection;
        stream << points.size();
        for (int i=0; i<points.size(); i++)
        {
            Point p = points[i];
            stream << p.x;
            stream << p.y;
            stream << p.z;
            stream << p.di;
            stream << p.ri;
            stream << p.snr;
            stream << p.beam;
            stream << p.dump0;
        }

        return data;
    }
};

class TrackMessageV2
{
public:
    struct Point {
        // 航迹信息  7*sizeof(int32_t) = 32字节
        int32_t distance;  ///< 距离,分辨率：0.01米
        int32_t angle; ///< 角度，分辨率：0.0001度
        int32_t speed; ///< 速度，分辨率：0.01米/秒
        int32_t speedDirection; ///< 速度方向，分辨率：0.0001度
        int16_t snr; ///< 信噪比，分辨率：0.01dB
        uint16_t id;
        int8_t object;
        int8_t pluse;
        int16_t beam;
        int16_t doppler;
        int16_t dopplerWidth;
        // 预测值
        int32_t privDistance; ///< 预测距离,分辨率：0.01米
        int32_t privAngle; ///< 预测角度，分辨率：0.0001度
        int32_t privSpeed; ///< 预测速度，分辨率：0.01米/秒
        int32_t privSpeedDirection; ///< 预测速度方向，分辨率：0.0001度

        int64_t endTimestamp;
    };
public:
    int64_t timestamp;
    uint32_t frame;
    std::vector<Point> points;

    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << timestamp;
        stream << frame;
        stream << points.size();
        for (int i=0; i<points.size(); i++)
        {
            Point p = points[i];
            stream << p.distance;
            stream << p.angle;
            stream << p.speed;
            stream << p.speedDirection;
            stream << p.snr;
            stream << p.id;
            stream << p.object;
            stream << p.pluse;
            stream << p.beam;
            stream << p.doppler;
            stream << p.dopplerWidth;
            stream << p.privDistance;
            stream << p.privAngle;
            stream << p.privSpeed;
            stream << p.privSpeedDirection;
        }

        return data;
    }
};

class TrackMessageV3
{
public:
    struct Point {
        /**
         * 每个目标数据的长度为64字节，最后的dump数据不需要解析，
         * 预留部分字节空间用于后续其他信息的添加
         * 目标角度及其他信息的计算方式如下：
         * 		目标水平距离 horizontalRange = sqrt(x*x + y*y);
         * 		目标轴向距离 range = sqrt(x*x + y*y + z*z)
         * 		目标方位角 azimuth = atan2(x, y) / M_PI * 180.0;
         * 		目标俯仰角 pitch = asin(z / range) / M_PI * 180.0;
         * 		目标速度 speed = sqrt(vx*vx + vy*vy + vz*vz);
         * 预测值使用同样的计算计算方式
         */
        int32_t x; 		///< x轴坐标, 分辨率：0.01米
        int32_t y; 		///< y轴坐标, 分辨率：0.01米
        int32_t z; 		///< z轴坐标, 分辨率：0.01米
        int32_t vx; 	///< x轴速度, 分辨率：0.01米/秒
        int32_t vy; 	///< y轴速度, 分辨率：0.01米/秒
        int32_t vz; 	///< z轴速度, 分辨率：0.01米/秒
        uint32_t id;	///< 航迹ID，用于区分不同目标
        int16_t snr; 	///< 信噪比，分辨率：0.01dB，调试版本上位机在航迹标签中显示该信息
        int16_t rcs;    ///< 雷达散射截面积，分辨率0.01m2（标签显示）
        int16_t beam; 	///< 波位索引，调试版本上位机在航迹标签中显示该信息
        uint16_t di; 	///< 多普勒单元索引，调试版本上位机在航迹标签中显示该信息
        uint16_t ri; 	///< 距离单元索引，调试版本上位机在航迹标签中显示该信息
        int8_t object;  ///< 目标类型
        int8_t pluse;
        // 预测值
        int64_t timestamp; ///<航迹更新时间戳，单位：ms，用于计算预测值
        int32_t dump0; ///< 预留 不需要解析
        int32_t dump1; ///< 预留 不需要解析
        int32_t dump2; ///< 预留 不需要解析
        int32_t selected; ///< 选中标志

        // 非协议字段
        int32_t duration;        // 持续时间
        int64_t createTimestamp; // 创建时间
    };
public:
    int64_t timestamp;
    int64_t frame;
    int64_t frameTimestamp; ///< 当前帧时间戳，单位ms，用于计算预测值
    int32_t scanBoundaryA; ///< 扫描左边界，分辨率：0.0001度
    int32_t scanBoundaryB; ///< 扫描右边界，分辨率：0.0001度
    int8_t reserve;
    int8_t dataType; ///< 数据类型，0：TWS航迹数据，1：TAS航迹数据
    int8_t orientation; ///< 北向标志，0：相对模式，1：正北模式
    int8_t scanningDirection; ///< 扫描方向，0: 顺时针扫描, 2: 逆时针扫描
    std::vector<Point> points;

    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << timestamp;
        stream << frame;
        stream << frameTimestamp;
        stream << scanBoundaryA;
        stream << scanBoundaryB;
        stream << reserve;
        stream << dataType;
        stream << orientation;
        stream << scanningDirection;
        stream << (int32_t)points.size();

        for (int i=0; i<points.size(); i++)
        {
            Point p = points[i];
            stream << p.x;
            stream << p.y;
            stream << p.z;
            stream << p.vx;
            stream << p.vy;
            stream << p.vz;
            stream << p.id;
            stream << p.snr;
            stream << p.rcs;
            stream << p.beam;
            stream << p.di;
            stream << p.ri;
            stream << p.object;
            stream << p.pluse;
            stream << p.timestamp;
            stream << p.dump0;
            stream << p.dump1;
            stream << p.dump2;
            stream << p.selected;
        }
        return data;
    }
};

class RequestRtkMessage
{
public:
    enum ErrorEnum
    {
        NoError = 0,
        DisconnectError,
    };
public:
    int8_t enable;
    int8_t kind;

    void parseBody(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream >> enable;
        stream >> kind;

    }
    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << (uint16_t)0;

        return data;
    }

};

class UploadRtkMessage
{
public:
    int64_t lat;  // 纬度，1e-9度
    int64_t lon;  // 经度，1e-9度
    int64_t heading;  // 行向角，1e-9度
    int32_t stars;  // 可用卫星数量
    int32_t alt;  // 高程

    QByteArray dumpBody()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << lat;
        stream << lon;
        stream << heading;
        stream << stars;
        stream << alt;

        return data;
    }

};

class RadarProtocol
{
public:
    static void makePacket(MessageType1 type, const QByteArray &data, QByteArray &packet, quint32 id);
    static void makePacket2(const QByteArray &data, QByteArray &packet);
    static MessageType1 parsePacket(QByteArray &packet, QByteArray &data, quint32 &id);

    static uint16_t CRC16 (const uint8_t *updata, int len)
    {
        uint8_t uchCRCHi = 0xff;
        uint8_t uchCRCLo = 0xff;
        uint16_t  uindex;

        while(len--)
        {
            uindex = uchCRCHi ^ *updata++;
            uchCRCHi = uchCRCLo ^ auchCRCHi[uindex];
            uchCRCLo = auchCRCLo[uindex];
        }
        return ((uchCRCHi<<8)|uchCRCLo);
    }

public:
    static uint32_t id;

    static uint8_t auchCRCHi[];

    static uint8_t auchCRCLo[];
};



#endif // TCPMESSAGE_H

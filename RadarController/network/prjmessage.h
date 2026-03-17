#ifndef NETWORK_PRJ_MESSAGE_H
#define NETWORK_PRJ_MESSAGE_H

#include "Message.h"
#include <vector>

namespace radar {
namespace net {

enum PrjMessageTypeEnum
{
    /* DEBUG Message */
    MSG_TYPE_UPLOAD_CFAR = MSG_TYPE_UPLOAD_DATA + 1,
    MSG_TYPE_UPLOAD_TARGET,
    MSG_TYPE_UPLOAD_TRACK,
    MSG_TYPE_UPLOAD_CLUTTER,
    MSG_TYPE_REQUEST_RTK,
    MSG_TYPE_UPLOAD_RTK,
    MSG_TYPE_UPLOAD_STATUS, // 上报雷达状态
    MSG_TYPE_V2 = MSG_TYPE_USER + 0x10000,
    MSG_TYPE_UPLOAD_TARGETV2,
    MSG_TYPE_UPLOAD_TRACKV2,
    MSG_TYPE_UPLOAD_CLUTTERV2,
    MSG_TYPE_V3 = MSG_TYPE_USER + 0x20000,
    MSG_TYPE_UPLOAD_TARGETV3,
    MSG_TYPE_UPLOAD_TRACKV3,
    MSG_TYPE_UPLOAD_CLUTTERV3,
    /* ADMIN Message */
    /* USER Message */
};


class UploadTargetPointsMessage : public Message
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
    qint64 timestamp;
    uint32_t frame;
    std::vector<Point> points;

protected:
    virtual bool parseBody(QByteArray& array) override;

};

class UploadTargetPointsMessageV3 : public Message
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
    qint64 timestamp;
    qint64 frame;
    int32_t scanBoundaryA; ///< 扫描左边界，分辨率：0.0001度
    int32_t scanBoundaryB; ///< 扫描右边界，分辨率：0.0001度
    int16_t reserve;
    int8_t orientation;
    int8_t scanningDirection; ///< 扫描方向，0: 顺时针扫描, 2: 逆时针扫描
    std::vector<Point> points;

protected:
    virtual bool parseBody(QByteArray& array) override;
};

class UploadTrackPointsMessage : public Message
{
public:
    struct Point {
        int32_t distance;  ///< 距离,分辨率：0.01米
        int32_t angle; ///< 角度，分辨率：0.0001度
        int32_t speed; ///< 速度，分辨率：0.01米/秒
        int16_t snr; ///< 信噪比，分辨率：0.01dB
        uint16_t id;
        int8_t object;
        int8_t pluse;
        int16_t beam;
        int16_t doppler;
        int16_t dopplerWidth;
    };
public:
    qint64 timestamp;
    uint32_t frame;
    std::vector<Point> points;

protected:
    virtual bool parseBody(QByteArray& array) override;
};

class UploadTrackPointsMessageV2 : public Message
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
    };
public:
    qint64 timestamp;
    uint32_t frame;
    std::vector<Point> points;

protected:
    virtual bool parseBody(QByteArray& array) override;
};

class UploadTrackPointsMessageV3 : public Message
{
public:
    struct Point {
        /**
         * 每个目标数据的长度为64字节，最后的dump数据不需要解析，
         * 预留部分字节空间用于后续其他信息的添加
         * 目标角度及其他信息的计算方式如下：
         * 		目标水平距离 horizontalRange = sqrt(x*x + y*y);
         * 		目标轴向距离 range = sqrt(x*x + y*y + z*z);
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
        int16_t rcs2;    ///< 雷达散射截面积，分辨率0.01㎡
        int16_t beam; 	///< 波位索引，调试版本上位机在航迹标签中显示该信息
        uint16_t di; 	///< 多普勒单元索引，调试版本上位机在航迹标签中显示该信息
        uint16_t ri; 	///< 距离单元索引，调试版本上位机在航迹标签中显示该信息
        int8_t object;  ///< 目标类型
        int8_t pluse;
        // 预测值
        qint64 timestamp; ///<航迹更新时间戳，单位：ms，用于计算预测值
        int32_t dump0; ///< 预留 不需要解析
        int32_t dump1; ///< 预留 不需要解析
        int32_t rcs6; ///< 雷达散射截面积，分辨率0.000001㎡
        int32_t selected; ///< 选中标志
    };
public:
    qint64 timestamp;
    qint64 frame;
    qint64 frameTimestamp; ///< 当前帧时间戳，单位ms，用于计算预测值
    int32_t scanBoundaryA; ///< 扫描左边界，分辨率：0.0001度
    int32_t scanBoundaryB; ///< 扫描右边界，分辨率：0.0001度
    int16_t reserve;
    int8_t orientation;
    int8_t scanningDirection; ///< 扫描方向，0: 顺时针扫描, 2: 逆时针扫描
    std::vector<Point> points;

public:
    virtual bool parseBody(QByteArray& array) override;
    QByteArray dumpBody();

private:
    QByteArray m_data;
};

class UploadClutterGridMessage : public Message
{
public:
    struct Grid {
        int32_t distance;  ///< 距离,分辨率：1米
        int32_t distanceRange;  ///< 距离,分辨率：1米
        int16_t angle; ///< 角度，分辨率：0.01度
        int16_t angleRange; ///< 角度，分辨率：0.01度
        int16_t mean;
        int16_t level;
    };
public:
    qint64 timestamp;
    uint32_t frame;
    std::vector<Grid> grids;

protected:
    virtual bool parseBody(QByteArray& array) override;

};

class UploadClutterGridMessageV3 : public Message
{
public:
    struct Grid {
        int32_t range; 			// 起始距离，0.01米，雷达轴向距离，地图显示时需要变换为xy平面的投影距离
        int32_t rangeSpan; 		// 距离跨度，0.01米，雷达轴向距离跨度，地图显示时需要变换为xy平面的投影跨度
        int16_t azimuth;		// 起始方位角，0.1度
        int16_t azimuthSpan;	// 方位角跨度，0.1度
        int16_t pitch;			// 起始俯仰角，0.1度，备用，暂时不需要
        int16_t pitchSpan;		// 俯仰角跨度，0.1度，备用，暂时不需要
        int32_t density;		// 杂波密度，显示在地图杂波区内，无量刚，分辨率0.01
        int16_t threshold;		// 门限等级，地图上仅显示门限等级大于0的杂波区
        int16_t beam;			// 俯仰波位，显示在地图杂波区内，用于标识不同的俯仰角
    };
public:
    qint64 timestamp;
    qint64 frame;
    int32_t scanningBoundaryA; ///< 扫描左边界，分辨率：0.0001度
    int32_t scanningBoundaryB; ///< 扫描右边界，分辨率：0.0001度
    std::vector<Grid> grids;

protected:
    virtual bool parseBody(QByteArray& array) override;
};


class RequestRtkMessage : public Message
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

protected:
    virtual void dumpBody(QDataStream& stream) override;
};

typedef Acknowledge RequestRtkAcknowledge;

class UploadRtkMessage : public Message
{
public:
    qint64 lat;  // 纬度，1e-9度
    qint64 lon;  // 经度，1e-9度
    qint64 heading;  // 行向角，1e-9度
    int32_t stars;  // 可用卫星数量
    int32_t alt;  // 高程

protected:
    virtual bool parseBody(QByteArray& array) override;
    virtual void dumpBody(QDataStream& stream) override;
};

/**
 * 当power.soc == 65535时代表未获取到电池状态
 */
class UploadStatusMessage : public Message
{
public:
    struct {
        int16_t voltage; // 电压：0.1V
        int16_t current; // 电流：0.1A
        uint16_t soc; // 剩余点亮，单位：1%
        uint16_t remainTime; // 剩余时间，单位：分钟
    }power;

protected:
    virtual bool parseBody(QByteArray& array) override;
};

}
}


#endif

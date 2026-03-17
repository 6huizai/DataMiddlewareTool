#ifndef NETWORK_TCPCLIENT_H
#define NETWORK_TCPCLIENT_H

#include "TcpAdminClient.h"
#include "prjmessage.h"

namespace radar {
namespace net {

class PrjTcpClient : public TcpAdminClient
{
    Q_OBJECT
public:
    enum RegisterAddress //寄存器地址
    {
        REG_ADDR_VERSION = 1024,
        REG_ADDR_USER_CONFIG_2, //工作模式寄存器WORKING_MODE (0x00000401)
        REG_ADDR_USER_CONFIG_3,
        REG_ADDR_SAMPLE_KIND = 1024+32,
        REG_ADDR_SAMPLE_SEQUENCE,
        REG_ADDR_SAMPLE_DISTANCE,
        REG_ADDR_USER_CONFIG_0 = 1024+64, //配置寄存器USER_CFG0 (0x00000440)
        REG_ADDR_USER_CONFIG_1,
        REG_ADDR_ACCEL_X = 1024+128,
        REG_ADDR_ACCEL_Y,
        REG_ADDR_ACCEL_Z,
        REG_ADDR_PITCH,
        REG_ADDR_ROLL,
        REG_ADDR_YAW,
        REG_ADDR_MPU_TEMP,
        REG_ADDR_INA3221_0_INPUT_0 = 1024+192,
        REG_ADDR_INA3221_0_INPUT_1,
        REG_ADDR_INA3221_0_INPUT_2,
        REG_ADDR_INA3221_1_INPUT_0,
        REG_ADDR_INA3221_1_INPUT_1,
        REG_ADDR_INA3221_1_INPUT_2,
        REG_ADDR_INA3221_2_INPUT_0,
        REG_ADDR_INA3221_2_INPUT_1,
        REG_ADDR_INA3221_2_INPUT_2,
        REG_ADDR_AD7414_TEMP,
        REG_ADDR_NMEA_HDT_H = 1024+256,
        REG_ADDR_NMEA_HDT_L,
        REG_ADDR_NMEA_LAT_H,
        REG_ADDR_NMEA_LAT_L,
        REG_ADDR_NMEA_LON_H,
        REG_ADDR_NMEA_LON_L,
        REG_ADDR_NMEA_STARS,
        REG_ADDR_NUM,
    };
    struct TargetPoint
    {
        double distance;
        double azimuth;
        double height;//version 3
        double pitch;//version 3
        int doppler;//version 1
        int di;//version 3
        int ri;//version 3
        double snr;
        int beam;
        int pluse;
    };

public:
    explicit PrjTcpClient(const QHostAddress& address, quint16 port,
                          QObject *parent = nullptr);

    virtual void registerMessages() override;
    void requestRtkMessage(int8_t enable, int8_t kind = 0);

signals:
    void receiveTargetPoints(int64_t stamp, int32_t frame, const QList<TargetPoint>& points);
    void receiveTrackPoints(int64_t stamp, int32_t frame, const QMap<uint32_t, QMap<QString, QVariant>>& points, int version);

    void receiveRtkMessage(int stars, double lat, double lon, double heading, double alt);
    void receiveTargetPointsV3(int64_t stamp, int64_t frame, int32_t scanBoundaryA, int32_t scanBoundaryB, int32_t direction, const QList<TargetPoint>& points);
    //void receiveTrackPointsV3(int64_t stamp, int32_t frame, int32_t scanBoundaryA, int32_t scanBoundaryB, int32_t direction, const QMap<uint32_t, QMap<QString, QVariant>>& points);

    void receiveTrackPointsV3(UploadTrackPointsMessageV3 data);


private slots:
    bool onUploadTargetPointsMessage(const radar::net::NodeInfoPtr&,
                                     std::shared_ptr<UploadTargetPointsMessage>& message,
                                     radar::Timestamp);
    bool onUploadTrackPointsMessage(const radar::net::NodeInfoPtr&,
                                    std::shared_ptr<UploadTrackPointsMessage>& message,
                                    radar::Timestamp);
    bool onRequestRtkAcknowledge(const radar::net::NodeInfoPtr&,
                                 std::shared_ptr<RequestRtkAcknowledge>& ack,
                                 radar::Timestamp);
    bool onUploadRtkMessage(const radar::net::NodeInfoPtr&,
                            std::shared_ptr<UploadRtkMessage>& ack,
                            radar::Timestamp);

    bool onUploadTrackPointsMessageV2(const radar::net::NodeInfoPtr&,
                                      std::shared_ptr<UploadTrackPointsMessageV2>& message,
                                      radar::Timestamp);

    bool onUploadTargetPointsMessageV3(const radar::net::NodeInfoPtr&,
                                       std::shared_ptr<UploadTargetPointsMessageV3>& message,
                                       radar::Timestamp);
    bool onUploadTrackPointsMessageV3(const radar::net::NodeInfoPtr&,
                                      std::shared_ptr<UploadTrackPointsMessageV3>& message,
                                      radar::Timestamp);

};

}
}

#endif // TCPCLIENT_H

#ifndef RADARCONTROLLER_H
#define RADARCONTROLLER_H

#include "network/prjtcpclient.h"
#include <QObject>
#include <QPointer>
#include <cmath>

using namespace radar::net;

#define kRtkMeanNum_ 30 // RTK获取的

struct RtkInfomation
{
    bool isFirst;
    int stars;  //卫星数量
    double lat; //纬度
    double lon; //经度
    double heading; //转台零位与正北的夹角
    double alt; //海拔高，目前不支持
    int count;  //采样数
    double baseHeading;
    double diffHeading;
};
Q_DECLARE_METATYPE(RtkInfomation)

enum WorkingMode
{
    CirSweep1s, //周扫1s
    CirSweep2s, //周扫2s
    CirSweep4s, //周扫4s
    StandBy     //待机
};
Q_DECLARE_METATYPE(WorkingMode)

struct HardwareConfig {
    int freq;   //频点 0-3
    int cfar;   //检测门限 0-2（0:低门限 1:中门限 2:高门限）一般用低门限
    int speed;  //速度门限 0-3（0:0.25m/s 1:0.5m/s 2:0.75m/s 3:1m/s）
    int rcs;    //rcs门限 0-3（0:关 1:0.001㎡ 2:0.01㎡ 3:0.05㎡）
};
Q_DECLARE_METATYPE(HardwareConfig)


class RadarController : public QObject
{
    Q_OBJECT

public:
    enum RadarState {
        UnknownState = -1,
        DisconnectState,    //连接断开
        ConnectingState,    //正在连接
        ConnectedState,     //连接成功
        LoginState,         //登录成功
    };

public:
    RadarController(QHostAddress ip, int port, QObject *parent = nullptr);
    RadarController(QObject *parent = nullptr);
    ~RadarController();

    // 雷达连接
    void createTcpClient(QHostAddress ip, int port);
    void destroyTcpClient();
    void updateConnection(QHostAddress ip, int port);
    bool online() const;

    // 雷达交互
    void requestRtkInfomation(bool enable);   //雷达RTK开关，开启后onRecvRtkMessage回调会持续收到RTK数据，1s一帧数据。
                                              //至少获取10帧数据后再次调用这个接口关闭开关，之后调用getRtkMean获取RTK均值信息。
    void getWorkStatus(); //获取雷达当前工作模式，工作状态见WorkingMode枚举类型
    void setWorkStatus(WorkingMode mode); //设置雷达工作状态（雷达启停）
    void getDeviceConfig(); //获取雷达的配置参数
    void getJsonConfig(int addr);
public slots:
    void setDeviceConfig(HardwareConfig config); //设置雷达的配置参数
    void setJsonConfig(int addr, QByteArray jsonData);
    // 数据透传
    void sendPacket(QByteArray packet);

private slots:
    void onTimerTimeout(); //定时触发，用于自动重连
    void onTcpClientStateChanged(radar::net::TcpClient::ClientState radarState);
    void onTcpClientError(int32_t errCode, QString errMsg);
    void onTcpClientMessage(QString msg);
    void onTcpClientReceiveRegisters(radar::net::RegisterList registers);
    void onTcpClientReceiveJsonConfig(int addr, QByteArray &jsonStr);

    // 雷达航迹数据回调
    // void onRecvTrackPointsV3(int64_t stamp, int32_t frame,int32_t scanBoundaryA, int32_t scanBoundaryB,
    //                          int32_t direction,const QMap<uint32_t, QMap<QString, QVariant>>& points);
    // RTK数据回调
    void onRecvRtkMessage(int stars, double lat, double lon, double heading, double alt);

signals:
    void sendResponse(QByteArray response);
    void rtkProgressUpdate(int progress);
    void rtkCalculationCompleted(RtkInfomation info);
    void recvWorkStatus(WorkingMode mode);
    void recvDeviceConfig(HardwareConfig config);
    //void recvTrackData(const QMap<uint32_t, QMap<QString, QVariant>>& points);
    void receiveTrackPointsV3(UploadTrackPointsMessageV3 data);

    void receiveReadTrackV3(UploadTrackPointsMessageV3 data);

private: //RTK计算相关
    bool rtkMeanCompleted();
    int getPercentageOfRtkMean();
    RtkInfomation getRtkMean();
    static __inline__ float calcAngleDelta(float base, float other);

private:
    QHostAddress m_ip;
    int m_port = 0;
    QPointer<radar::net::PrjTcpClient> m_tcpClient_ = NULL;
    QTimer *m_reLoginTimer;
    qint64 m_loginms;

private:
    WorkingMode m_workingMode; //雷达工作模式
    HardwareConfig m_config; //雷达配置参数
    RtkInfomation m_rtkInfoMean_; //用于获取并计算RTK坐标的均值
    RtkInfomation m_rtkInfo;
    bool m_rtkFirstCompleted = false;
};

#endif // RADARCONTROLLER_H

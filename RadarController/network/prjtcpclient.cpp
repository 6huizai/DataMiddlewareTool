#include "prjtcpclient.h"
#include <stdint.h>
#include <cmath>

namespace radar {
namespace net {

PrjTcpClient::PrjTcpClient(const QHostAddress& address, quint16 port,
                           QObject *parent)
    : TcpAdminClient(address, port, parent)
{
    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<int64_t>("int64_t");
}

void PrjTcpClient::registerMessages()
{
    TcpAdminClient::registerMessages();
    // V1版本协议
    dispatcher()->registerMessageCallback<UploadTargetPointsMessage>(
                MSG_TYPE_UPLOAD_TARGET,
                std::bind(&PrjTcpClient::onUploadTargetPointsMessage,
                          this, _1, _2, _3));
    dispatcher()->registerMessageCallback<UploadTrackPointsMessage>(
                MSG_TYPE_UPLOAD_TRACK,
                std::bind(&PrjTcpClient::onUploadTrackPointsMessage,
                          this, _1, _2, _3));
    dispatcher()->registerMessageCallback<RequestRtkAcknowledge>(
                MSG_TYPE_REQUEST_RTK,
                std::bind(&PrjTcpClient::onRequestRtkAcknowledge,
                          this, _1, _2, _3));
    dispatcher()->registerMessageCallback<UploadRtkMessage>(
                MSG_TYPE_UPLOAD_RTK,
                std::bind(&PrjTcpClient::onUploadRtkMessage,
                          this, _1, _2, _3));
    // V2版本协议
    dispatcher()->registerMessageCallback<UploadTrackPointsMessageV2>(
                MSG_TYPE_UPLOAD_TRACKV2,
                std::bind(&PrjTcpClient::onUploadTrackPointsMessageV2,
                          this, _1, _2, _3));
    //V3版本协议
    dispatcher()->registerMessageCallback<UploadTargetPointsMessageV3>(
        MSG_TYPE_UPLOAD_TARGETV3,
        std::bind(&PrjTcpClient::onUploadTargetPointsMessageV3,
                  this, _1, _2, _3));
    dispatcher()->registerMessageCallback<UploadTrackPointsMessageV3>(
        MSG_TYPE_UPLOAD_TRACKV3,
        std::bind(&PrjTcpClient::onUploadTrackPointsMessageV3,
                  this, _1, _2, _3));

}

void PrjTcpClient::requestRtkMessage(int8_t enable, int8_t kind)
{
    if (!isLogin())
    {
        emit error(MessageError, "The radar is not logged in, please log in and try again.");
        return;
    }
    std::shared_ptr<RequestRtkMessage> message = std::make_shared<RequestRtkMessage>();
    message->type = MSG_TYPE_REQUEST_RTK;
    message->enable = enable;
    message->kind = kind;
    sendMessage(message->dumpToArray());
    appendWaitMessage(message);
}



bool PrjTcpClient::onUploadTargetPointsMessage(const NodeInfoPtr&,
                              std::shared_ptr<UploadTargetPointsMessage>& message,
                              Timestamp)
{
    QList<TargetPoint> points;
    for (const UploadTargetPointsMessage::Point& p : message->points)
    {
        double fangle, fdistance;
        fangle = double(p.angle) / 10000.0;
        fdistance = double(p.distance) / 100.0;
        //points.append({fangle, fdistance});

        TargetPoint point = {0};
        point.distance = fdistance;
        point.azimuth = fangle;
        point.doppler = p.doppler;
        point.snr = double(p.snr)/100;
        point.beam = p.beam;
        point.pluse = p.pluse;
        points.append(point);
    }
    emit receiveTargetPoints(message->timestamp, message->frame, points);
    return true;
}

/*
         * 		目标水平距离 horizontalRange = sqrt(x*x + y*y);
         * 		目标轴向距离 range = sqrt(x*x + y*y + z*z)
         * 		目标方位角 azimuth = atan2(x, y) / M_PI * 180.0;
         * 		目标俯仰角 pitch = asin(z / range) / M_PI * 180.0;
         * 		目标速度 speed = sqrt(vx*vx + vy*vy + vz*vz);
 * */

bool PrjTcpClient::onUploadTargetPointsMessageV3(const NodeInfoPtr&,
                                                 std::shared_ptr<UploadTargetPointsMessageV3>& message,
                                                 Timestamp)
{
    //QLOG_INFO()<<"cfar点数："<<message->points.size();
    QList<TargetPoint> points;
    for (const UploadTargetPointsMessageV3::Point& p : message->points)
    {
        double x = (double)(p.x) / 100;
        double y = (double)(p.y) / 100;
        double z = (double)(p.z) / 100;
        double horizontalRange = sqrt(x*x + y*y);
        double range = sqrt(x*x + y*y + z*z);
        double azimuth = atan2(x, y) / M_PI * 180.0;
        if(azimuth<0)azimuth+=360;
        double pitch = asin(z / range) / M_PI * 180.0;
        //double speed = sqrt(p.vx*p.vx + p.vy*p.vy + p.vz*p.vz);
        TargetPoint point = {0};
        point.azimuth = azimuth;
        point.pitch = pitch;
        point.distance = horizontalRange;
        point.height = z;
        point.di = p.di;
        point.ri = p.ri;
        point.snr = double(p.snr)/100;
        point.beam = p.beam;
        points.append(point);

        //if(horizontalRange>=10000)QLOG_INFO()<<"超出距离"<<horizontalRange<<range;
        //QLOG_INFO()<<"cfar信息："<<p.x<<p.y<<p.z;
        //QLOG_INFO()<<"cfar信息："<<horizontalRange<<range<<azimuth<<pitch;
        //QLOG_INFO()<<"[cfar]"<<"data:"<<p.x<<p.y<<p.z<<"boundary:"<<(double)message->scanBoundaryA/10000<<(double)message->scanBoundaryB/10000;
    }

    emit receiveTargetPointsV3(message->timestamp, message->frame, message->scanBoundaryA, message->scanBoundaryB, message->scanningDirection, points);
    return true;
}

bool PrjTcpClient::onUploadTrackPointsMessage(const NodeInfoPtr&,
                              std::shared_ptr<UploadTrackPointsMessage>& message,
                              Timestamp)
{
    QMap<uint32_t, QMap<QString, QVariant>> points;
    for (const UploadTrackPointsMessage::Point& p : message->points)
    {
        auto point = QMap<QString, QVariant>({{tr("Version"), 1},
                                              {tr("Velocity"), double(p.speed) / 100.0},
                                              {tr("Distance"), double(p.distance) / 100.0},
                                              {tr("Azimuth"), double(p.angle) / 10000.0},
                                              {tr("SNR"), double(p.snr) / 100.0},
                                              {tr("Type"), int(p.object)},
                                              {tr("Beam"), int(p.beam)},
                                              {tr("Doppler"), int(p.doppler)},
                                              {tr("DW"), int(p.dopplerWidth)},
                                              {tr("Pulse"), int(p.pluse)}});
        points.insert(p.id, point);
    }
    emit receiveTrackPoints(message->timestamp, message->frame, points, 1);
    return true;
}

bool PrjTcpClient::onUploadTrackPointsMessageV2(const NodeInfoPtr&,
                                                std::shared_ptr<UploadTrackPointsMessageV2>& message,
                                                Timestamp)
{
    QMap<uint32_t, QMap<QString, QVariant>> points;
    for (const UploadTrackPointsMessageV2::Point& p : message->points)
    {
        auto point = QMap<QString, QVariant>({{tr("Version"), 2},
                                              {tr("Velocity"), double(p.speed) / 100.0},
                                              {tr("VelocityDirection"), double(p.speedDirection) / 10000.0},
                                              {tr("Distance"), double(p.distance) / 100.0},
                                              {tr("Azimuth"), double(p.angle) / 10000.0},
                                              {tr("SNR"), double(p.snr) / 100.0},
                                              {tr("Type"), int(p.object)},
                                              {tr("Beam"), int(p.beam)},
                                              {tr("Doppler"), int(p.doppler)},
                                              {tr("DW"), int(p.dopplerWidth)},
                                              {tr("Pulse"), int(p.pluse)},
                                              {tr("PrivDistance"), double(p.privDistance) / 100.0},
                                              {tr("PrivAzimuth"), double(p.privAngle) / 10000.0},
                                              {tr("PrivVelocity"), double(p.privSpeed) / 100.0},
                                              {tr("PrivVelocityDirection"), double(p.privSpeedDirection) / 10000.0}});
        points.insert(p.id, point);
    }
    emit receiveTrackPoints(message->timestamp, message->frame, points, 2);
    return true;
}



/*
 *       * 		目标水平距离 horizontalRange = sqrt(x*x + y*y);
         * 		目标轴向距离 range = sqrt(x*x + y*y + z*z)
         * 		目标方位角   azimuth = atan2(y, x) / M_PI * 180.0;
         * 		目标俯仰角   pitch = asin(z / range) / M_PI * 180.0;
         * 		目标速度    speed = sqrt(vx*vx + vy*vy + vz*vz);
 */

bool PrjTcpClient::onUploadTrackPointsMessageV3(const NodeInfoPtr&,
                                                std::shared_ptr<UploadTrackPointsMessageV3>& message,
                                                Timestamp time)
{
    //QLOG_INFO()<<"轨迹点数："<<message->points.size()<<message->scanBoundaryA<<message->scanBoundaryB<<message->scanningDirection;

    /*
    QMap<uint32_t, QMap<QString, QVariant>> points;
    for (const UploadTrackPointsMessageV3::Point& p : message->points)
    {
        double x = (double)(p.x) / 100;
        double y = (double)(p.y) / 100;
        double z = (double)(p.z) / 100;
        double vx = (double)(p.vx) / 100;
        double vy = (double)(p.vy) / 100;
        double vz = (double)(p.vz) / 100;

        double horizontalRange = sqrt(x*x + y*y);
        double range = sqrt(x*x + y*y + z*z);
        double azimuth = atan2(x, y) / M_PI * 180.0;
        if(azimuth<0)azimuth+=360; // 从[-180,180]转换到[0,360]
        double pitch = asin(z / range) / M_PI * 180.0;
        double speed = sqrt(vx*vx + vy*vy + vz*vz);
        double speedDir = atan2(vx, vy) / M_PI * 180.0;
        if(speedDir<0)speedDir+=360;

        double t = (double)(message->frameTimestamp - p.timestamp)/1000.0;
        double xk = x + vx*t;
        double yk = y + vy*t;
        double zk = z + vz*t;
        double privDistance = sqrt(xk*xk + yk*yk);
        double privAzimuth = atan2(xk, yk) / M_PI * 180.0;
        if(privAzimuth<0) privAzimuth+=360;

        double cosValue = (x*vx+y*vy)/sqrt(x*x+y*y)*sqrt(vx*vx+vy*vy);
        QString menace = cosValue > 0 ? tr("No") : tr("Yes");

        auto point = QMap<QString, QVariant>({{tr("Version"), int(3)},
                                              {tr("Velocity"), speed},
                                              {tr("VelocityDir"), speedDir},
                                              {tr("Distance"), horizontalRange},
                                              {tr("Range"), range},
                                              {tr("Azimuth"), azimuth},
                                              {tr("Pitch"), pitch},
                                              {tr("Height"), z},
                                              {tr("Type"), int(p.object)},
                                              {tr("SNR"), double(p.snr) / 100.0},
                                              {tr("RCS2"), double(p.rcs2) / 100.0},
                                              {tr("RCS6"), double(p.rcs6) / 1000000.0},
                                              {tr("RI"), int(p.ri)},
                                              {tr("DI"), int(p.di)},
                                              {tr("Beam"), int(p.beam)},
                                              {tr("PrivDistance"), privDistance},
                                              {tr("PrivAzimuth"),  privAzimuth},
                                              {tr("PrivHeight"),   zk},
                                              {tr("Selected"),   int(p.selected)},
                                              {tr("Menace"), menace}});
        points.insert(p.id, point);
    }

    emit receiveTrackPointsV3(message->timestamp, message->frame, message->scanBoundaryA, message->scanBoundaryB, message->scanningDirection, points);
*/
    emit receiveTrackPointsV3(*message);

    return true;
}


bool PrjTcpClient::onRequestRtkAcknowledge(const NodeInfoPtr&,
                             std::shared_ptr<RequestRtkAcknowledge>& ack,
                             Timestamp)
{
    //QLOG_INFO() << "请求RTK状态： " << ack->state;
    // emit message(QString("请求RTK状态：%1").arg(ack->state));
    return true;
}

bool PrjTcpClient::onUploadRtkMessage(const NodeInfoPtr&,
                             std::shared_ptr<UploadRtkMessage>& msg,
                             Timestamp)
{
    emit sendResponse(msg->dumpToArray());

    QString info = QString::asprintf("RTK: stats=%d, lat=%.9f, lon=%.9f, heading=%.9f",
                                     msg->stars, (double)msg->lat / 1e9, (double)msg->lon / 1e9,
                                     (double)msg->heading / 1e9);
    //QLOG_INFO() << info;
    // emit message(info);
    emit receiveRtkMessage(msg->stars, (double)msg->lat / 1e9, (double)msg->lon / 1e9,
                           (double)msg->heading / 1e9, 0);
    return true;
}

}
}

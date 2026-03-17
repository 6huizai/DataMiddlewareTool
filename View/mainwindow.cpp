#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLayout>
#include "projectionutils.h"
#include <QGeoCoordinate>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 雷达客户端
    m_radarController = new RadarController();

    m_randomForest = new RandomForest("RandomForest.bin");

    m_rtkInfo = new RtkInfomation;
    m_rtkInfo->lat = 34;
    m_rtkInfo->lon = 108;
    m_rtkInfo->heading = 248.18;

    initUI();
    initConnection();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUI()
{
    // 设置窗口大小
    resize(500, 400);
    setWindowTitle(tr("Radar data forwarding middleware"));
}

void MainWindow::initConnection()
{
    // 更新航迹数据
    connect(m_radarController, &RadarController::receiveTrackPointsV3, this, [=](UploadTrackPointsMessageV3 data)
    {
        // qDebug()<<"=============原始数据==============";
        // foreach (auto p, data.points) {
        //     qDebug()<<"id:"<<p.id<<"type:"<<p.object;
        // }

        // 对数据进行处理（目标重新识别），处理完成后进行转发
        UploadTrackPointsMessageV3 newData = m_randomForest->updateData(data);

        // qDebug()<<"=============处理后数据==============";
        // foreach (auto p, newData.points) {
        //     qDebug()<<"id:"<<p.id<<"type:"<<p.object;
        // }

        // 判断界面上是否对目标类型进行筛选
        if(ui->widget_servicePanel->selectObject()  ==  0)
        {
            ui->widget_servicePanel->tcpServer()->sendData(newData.dumpBody());

            QByteArray hpData = formatHPMessage(newData);
            ui->widget_servicePanel->sendUdpData(hpData);
        }
        else
        {
            UploadTrackPointsMessageV3 selsectData;
            selsectData.frame = newData.frame;
            selsectData.frameTimestamp = newData.frameTimestamp;
            selsectData.timestamp = newData.timestamp;
            selsectData.orientation = newData.orientation;
            selsectData.reserve = newData.reserve;
            selsectData.scanBoundaryA = newData.scanBoundaryA;
            selsectData.scanBoundaryB = newData.scanBoundaryB;
            selsectData.scanningDirection = newData.scanningDirection;

            // 通过目标类型进行筛选，只保留关注的类型的目标
            for(int ii=0; ii<newData.points.size(); ii++)
            {
                switch(newData.points.at(ii).object)
                {
                 case 1:
                     {
                      if(ui->widget_servicePanel->typeSelect[1] > 0)
                      {
                       selsectData.points.push_back(newData.points.at(ii));
                        }
                        break;
                    }
                case 2:
                {
                    if(ui->widget_servicePanel->typeSelect[2] > 0)
                    {
                        selsectData.points.push_back(newData.points.at(ii));
                    }
                    break;
                }
                case 3:
                {
                    if(ui->widget_servicePanel->typeSelect[3] > 0)
                    {
                        selsectData.points.push_back(newData.points.at(ii));
                    }
                    break;
                }
                case 4:
                {
                    if(ui->widget_servicePanel->typeSelect[4] > 0)
                    {
                        selsectData.points.push_back(newData.points.at(ii));
                    }
                    break;
                }
                case 6:
                {
                    if(ui->widget_servicePanel->typeSelect[5] > 0)
                    {
                        selsectData.points.push_back(newData.points.at(ii));
                    }
                    break;
                }
                default:
                {
                    break;
                }
                }

            }

            ui->widget_servicePanel->tcpServer()->sendData(selsectData.dumpBody());

            QByteArray hpData = formatHPMessage(selsectData);
            ui->widget_servicePanel->sendUdpData(hpData);
        }

    });


    // 处理登录/登出操作
    connect(ui->widget_controlPanel, &ControlPanel::login, this, [=](QHostAddress ip, int port, bool isLogin){
        if (isLogin){
            m_radarController->updateConnection(ip, port);
        }
        else{
            m_radarController->destroyTcpClient();
        }
    });

    // 处理RTK操作
    connect(ui->widget_controlPanel, &ControlPanel::requestRTK, m_radarController, &RadarController::requestRtkInfomation);
    connect(m_radarController, &RadarController::rtkProgressUpdate, ui->widget_controlPanel, &ControlPanel::onRtkProgressUpdate);
    connect(m_radarController, &RadarController::rtkCalculationCompleted, ui->widget_controlPanel, &ControlPanel::onRtkCalculationCompleted);
    connect(m_radarController, &RadarController::rtkCalculationCompleted, this, [=](RtkInfomation info){
        if (!m_rtkInfo)
        {
            m_rtkInfo = new RtkInfomation;
        }
        *m_rtkInfo = info;
    });

    // 处理启动/停止操作
    connect(ui->widget_controlPanel, &ControlPanel::setWorkMode, this, [=](WorkingMode mode){
        m_radarController->setWorkStatus(mode);
    });

    // 处理设备参数配置
    connect(ui->widget_controlPanel, &ControlPanel::setDeviceConfig, m_radarController, &RadarController::setDeviceConfig);
    connect(m_radarController, &RadarController::recvDeviceConfig, ui->widget_controlPanel, &ControlPanel::onRecvDeviceConfig);
    connect(m_radarController, &RadarController::recvWorkStatus, ui->widget_controlPanel, &ControlPanel::onRecvWorkStatus);

    // 指令透传
    connect(ui->widget_servicePanel->tcpServer(), &TcpServer::sendPacket, m_radarController, &RadarController::sendPacket);
    connect(m_radarController, &RadarController::sendResponse, ui->widget_servicePanel->tcpServer(), &TcpServer::sendResponse);
}


void MainWindow::onRecvTrackData(const QMap<uint32_t, QMap<QString, QVariant>>& targets)
{
    // if (!m_rtkInfo || !m_platformClient->isLogined()) return;

    // QList<uint32_t> currTids = targets.keys();
    // foreach (uint32_t tid, currTids) {
    //     // int type = targets[tid].value(tr("Type")).toInt();
    //     // if (type != 3) continue;

    //     DroneInfo drone;
    //     drone.id = tid;
    //     drone.speed = targets[tid].value(tr("Velocity")).toDouble();
    //     //drone.heading =
    //     drone.height = targets[tid].value(tr("Height")).toDouble();
    //     drone.distance = targets[tid].value(tr("Distance")).toDouble();

    //     double azimuth = targets[tid].value(tr("Azimuth")).toDouble();

    //     QGeoCoordinate coord1(m_rtkInfo->lat, m_rtkInfo->lon);
    //     QGeoCoordinate coord2 = coord1.atDistanceAndAzimuth(drone.distance, azimuth,0);

    //     drone.lat = coord2.latitude();
    //     drone.lon = coord2.longitude();
    //     drone.heading = targets[tid].value(tr("VelocityDir")).toDouble();

    //     m_platformClient->sendDrone(drone);
    // }
}

// 格式化为和普的数据格式
QByteArray MainWindow::formatHPMessage(UploadTrackPointsMessageV3 &data)
{
    HPTrackMessage message;

    quint16 head1 = 0xFF77;
    quint16 head2 = 0xCCCC;
    quint16 head3 = 0x77FF;

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << head1;
    stream << head2;
    stream << head3;
    stream << (int32_t)data.points.size();

    foreach (auto &point, data.points)
    {
        double x = (double)(point.x) / 100;
        double y = (double)(point.y) / 100;
        double z = (double)(point.z) / 100;
        double vx = (double)(point.vx) / 100;
        double vy = (double)(point.vy) / 100;
        double vz = (double)(point.vz) / 100;

        double horizontalRange = sqrt(x*x + y*y);
        double range = sqrt(x*x + y*y + z*z);
        double azimuth = atan2(x, y) / M_PI * 180.0;
        if(azimuth<0)azimuth+=360; // 从[-180,180]转换到[0,360]
        double pitch = asin(z / range) / M_PI * 180.0;
        double speed = sqrt(vx*vx + vy*vy + vz*vz);
        double speedDir = atan2(vx, vy) / M_PI * 180.0;
        if(speedDir<0)speedDir+=360;

        QGeoCoordinate coord1(m_rtkInfo->lat, m_rtkInfo->lon);
        QGeoCoordinate coord2 = coord1.atDistanceAndAzimuth(horizontalRange, azimuth,0);

        HPTrackMessage::Point p;
        p.id = point.id;
        p.timeStamp = point.timestamp;
        p.distance = horizontalRange;
        p.azimuth = azimuth;
        p.pitch = pitch;
        p.speed = speed;
        p.heading = speedDir;
        p.lon = coord2.longitude();
        p.lat = coord2.latitude();
        p.height = z;
        p.targetStrength = float(point.snr) / 100.0;
        p.trackingCount = 0;
        p.lossCount = 0;
        p.deviceId = 0;
        switch (point.object) {
        case 0:{ //未识别对应未知
            p.type = 0;
            break;
        }
        case 1:{ //行人
            p.type = 2;
            break;
        }
        case 2:{ //车辆
            p.type = 1;
            break;
        }
        case 3:{ //无人机
            p.type = 3;
            break;
        }
        case 4:{ //飞鸟
            p.type = 6;
            break;
        }
        case 5:{ //鸟群对应到飞鸟
            p.type = 6;
            break;
        }
        case 6:{ //飞机（民航）
            p.type = 5;
            break;
        }
        default:{ //其他是未识别
            p.type = 0;
            break;
        }
        }
        p.rcs = float(point.rcs6) / 1e6;
        memset(p.reserve, 0, 10);

        QByteArray pointArray;
        QDataStream pointStream(&pointArray, QIODevice::WriteOnly);
        pointStream.setByteOrder(QDataStream::LittleEndian);

        pointStream << p.id;
        pointStream << p.timeStamp;
        pointStream.writeRawData((char*)&(p.distance), 4);
        pointStream.writeRawData((char*)&(p.azimuth), 4);
        pointStream.writeRawData((char*)&(p.pitch), 4);
        pointStream.writeRawData((char*)&(p.speed), 4);
        pointStream.writeRawData((char*)&(p.heading), 4);
        // pointStream << p.distance;
        // pointStream << p.azimuth;
        // pointStream << p.pitch;
        // pointStream << p.speed;
        // pointStream << p.heading;
        pointStream.writeRawData((char*)&(p.lon), 8);
        pointStream.writeRawData((char*)&(p.lat), 8);
        pointStream.writeRawData((char*)&(p.height), 8);
        // pointStream << p.lon;
        // pointStream << p.lat;
        // pointStream << p.height;
        pointStream.writeRawData((char*)&(p.targetStrength), 4);
        // pointStream << p.targetStrength;
        pointStream << p.trackingCount;
        pointStream << p.lossCount;
        pointStream << p.deviceId;
        pointStream << p.type;
        pointStream.writeRawData((char*)&(p.rcs), 4);
        // pointStream << p.rcs;
        pointStream.writeRawData(p.reserve, 10);

        quint8 check = 0;
        for (int i = 0; i < pointArray.size(); ++i) {
            check += static_cast<quint8>(pointArray.at(i));
        }
        pointStream << check;

        qDebug()<<"point array:"<<pointArray.size();

        stream.writeRawData(pointArray.data(), pointArray.size());
    }

    return array;

}

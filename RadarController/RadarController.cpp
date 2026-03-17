#include "radarcontroller.h"

#include <QTimer>
#include <QDebug>
#include <QTableView>
#include <QScrollBar>
#include <QHeaderView>
#include <QMetaMethod>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

RadarController::RadarController(QHostAddress ip, int port, QObject *parent)
    : QObject(parent)
    , m_ip(ip)
    , m_port(port)
    , m_workingMode(StandBy)
{
    qRegisterMetaType<RtkInfomation>("RtkInfomation");
    qRegisterMetaType<WorkingMode>("WorkingMode");
    qRegisterMetaType<HardwareConfig>("HardwareConfig");

    createTcpClient(ip, port);

    m_reLoginTimer = new QTimer(this);
    connect(m_reLoginTimer, &QTimer::timeout, this, &RadarController::onTimerTimeout);

    m_reLoginTimer->start(1000);
}

RadarController::RadarController(QObject *parent)
    : QObject(parent)
    , m_workingMode(StandBy)
{
    qRegisterMetaType<RtkInfomation>("RtkInfomation");
    qRegisterMetaType<WorkingMode>("WorkingMode");
    qRegisterMetaType<HardwareConfig>("HardwareConfig");

    m_reLoginTimer = new QTimer(this);
    connect(m_reLoginTimer, &QTimer::timeout, this, &RadarController::onTimerTimeout);

    m_reLoginTimer->start(1000);
}

RadarController::~RadarController()
{
    if (m_tcpClient_)
    {
        m_tcpClient_->logout();
        delete m_tcpClient_;
        m_tcpClient_ = nullptr;
    }
}


void RadarController::sendPacket(QByteArray packet)
{
    if (m_tcpClient_)
    {
        m_tcpClient_->sendPacket(packet);
    }
}

/**
 * @brief 更新TCP连接
 * 当雷达IP和当前TCP连接的IP不一致时，调用此接口重置TCP连接，雷达的TCP端口默认5001
 *
 * @param ip 新连接的IP
 * @param port 新连接的端口
 * @return void
 */

void RadarController::updateConnection(QHostAddress ip, int port)
{
    if (m_ip != ip || m_port != port)
    {
        destroyTcpClient();
        createTcpClient(ip, port);
    }
}

/**
 * @brief 创建TCP客户端
 * 创建PrjTcpClient实例，用于和雷达建立连接，以及后续交互
 *
 * @param ip 雷达IP地址
 * @param port 雷达端口
 * @return void
 */
void RadarController::createTcpClient(QHostAddress ip, int port)
{
    m_tcpClient_ = new PrjTcpClient(ip, port);
    m_tcpClient_->registerMessages();
    connect(m_tcpClient_, &PrjTcpClient::stateChanged, this, &RadarController::onTcpClientStateChanged);
    connect(m_tcpClient_, &PrjTcpClient::error, this, &RadarController::onTcpClientError);
    connect(m_tcpClient_, &PrjTcpClient::message, this, &RadarController::onTcpClientMessage);
    connect(m_tcpClient_, &PrjTcpClient::receiveRegisters, this, &RadarController::onTcpClientReceiveRegisters);
    connect(m_tcpClient_, &PrjTcpClient::receiveJsonConfig, this, &RadarController::onTcpClientReceiveJsonConfig);

#if !defined(QRADAR_VIRTUAL_TEST)
    connect(m_tcpClient_, &PrjTcpClient::receiveTrackPointsV3, this, &RadarController::receiveTrackPointsV3);

#endif
    connect(m_tcpClient_, &PrjTcpClient::receiveRtkMessage, this, &RadarController::onRecvRtkMessage);

    connect(m_tcpClient_, &PrjTcpClient::sendResponse, this, &RadarController::sendResponse);

    m_loginms = 0;
}

/**
 * @brief 销毁TCP客户端
 * 销毁PrjTcpClient实例，用于和雷达断开连接。
 * 在和同一个雷达重新建立连接时，先销毁之前的连接。
 *
 * @return void
 */
void RadarController::destroyTcpClient()
{
    if (m_tcpClient_) {
        disconnect(m_tcpClient_, &PrjTcpClient::stateChanged, this, &RadarController::onTcpClientStateChanged);
        disconnect(m_tcpClient_, &PrjTcpClient::error, this, &RadarController::onTcpClientError);
        disconnect(m_tcpClient_, &PrjTcpClient::message, this, &RadarController::onTcpClientMessage);
        disconnect(m_tcpClient_, &PrjTcpClient::receiveRegisters, this, &RadarController::onTcpClientReceiveRegisters);
        disconnect(m_tcpClient_, &PrjTcpClient::receiveJsonConfig, this, &RadarController::onTcpClientReceiveJsonConfig);

#if !defined(QRADAR_VIRTUAL_TEST)
        disconnect(m_tcpClient_, &PrjTcpClient::receiveTrackPointsV3, this, &RadarController::receiveTrackPointsV3);
#endif
        disconnect(m_tcpClient_, &PrjTcpClient::receiveRtkMessage, this, &RadarController::onRecvRtkMessage);

        m_tcpClient_->logout();
        delete m_tcpClient_;
        m_tcpClient_ = nullptr;
    }
}

/**
 * @brief 检查当前雷达是否在线
 *
 * @return true即雷达在线，false即雷达不在线
 */
bool RadarController::online() const
{
    if (m_tcpClient_)
    {
        return m_tcpClient_->isLogin();
    }
    else
    {
        return false;
    }
}

/**
 * @brief 设置雷达工作模式
 * 可设置的工作模式包括：周扫1s、周扫2s、周扫4s（现实际改为3s）
 *
 * @param mode 工作模式参考WorkingMode枚举的注释
 * @return void
 */
void RadarController::setWorkStatus(WorkingMode mode)
{
    if (!m_tcpClient_ || !m_tcpClient_->isLogin()) return;

    m_workingMode = mode;

    // 设置寄存器：雷达工作模式
    RegisterList registers;
    int rotationalSpeed = 0;
    int workingMode = 0;


    if (m_workingMode == CirSweep1s)
    {
        rotationalSpeed = 4;
        workingMode = 1;
    }
    else if (m_workingMode == CirSweep2s)
    {
        rotationalSpeed = 1;
        workingMode = 1;
    }
    else if (m_workingMode == CirSweep4s)
    {
        rotationalSpeed = 2;
        workingMode = 1;
    }
    else if(m_workingMode == StandBy)
    {
        workingMode = 0;
    }

    uint32_t value = (((uint32_t)0 & 0xFF) << 24)
                     | (((uint32_t)0 & 0xFF) << 16)
                     | (((uint32_t)rotationalSpeed & 0xFF) << 8)
                     | ((uint32_t)workingMode & 0xFF);
    registers.append(QPair<uint32_t, uint32_t>(PrjTcpClient::REG_ADDR_USER_CONFIG_2, value));

    if (m_tcpClient_ && m_tcpClient_->isLogin())
    {
        m_tcpClient_->setRegisters(registers);
    }
}

/**
 * @brief 获取雷达当前的工作模式
 * 此方法只是发送一个获取请求，雷达发送的响应会在onTcpClientReceiveRegisters回调中接收
 *
 * @return void
 */
void RadarController::getWorkStatus()
{
    QList<uint32_t> addrs;
    addrs.append(PrjTcpClient::REG_ADDR_USER_CONFIG_2);

    if (m_tcpClient_ && m_tcpClient_->isLogin())
    {
        m_tcpClient_->getRegisters(addrs);
    }
}

/**
 * @brief 请求RTK信息
 * 用于开启/关闭RTK
 * 开启RTK后，onRecvRtkMessage回调会持续接收到RTK信息，数据率1Hz
 *
 * @note 请求RTK时，必须保证雷达工作模式为待机状态，即转台静止，否则获取的零度指向不正确
 *
 * @param enable 1:开启RTK  0:关闭RTK
 * @return void
 */
void RadarController::requestRtkInfomation(bool enable)
{
    if (enable) {
        m_rtkInfoMean_ = {true, 0, 0, 0, 0, 0, 0, 0, 0};
        m_rtkFirstCompleted = true;
    }
    if (m_tcpClient_ && m_tcpClient_->isLogin())
    {
        m_tcpClient_->requestRtkMessage(enable);
    }
}

/**
 * @brief 判断RTK读取是否结束
 * kRtkMeanNum_是预设值30，即取30次数据的平均值
 * 若觉得获取时间太长，可适度调整kRtkMeanNum_值，但最好保证接收数据15次以上
 *
 * @note 结束后调用requestRtkInfomation(0)关闭RTK
 *
 * @return bool true:结束  false:未结束
 */
bool RadarController::rtkMeanCompleted()
{
    return m_rtkInfoMean_.count >= kRtkMeanNum_;
}

/**
 * @brief 实时查看当前RTK信息获取进度
 * 一般用于界面进度条显示
 *
 * @return int 0-100，100即获取完成
 */
int RadarController::getPercentageOfRtkMean()
{
    return (double)m_rtkInfoMean_.count / (double)kRtkMeanNum_ * 100 + 0.5;
}

/**
 * @brief 获取RTK结果均值
 * 在RTK读取完成后（rtkMeanCompleted返回true），可调用此方法获取最终的结果均值，即可用的经纬度和零度指向
 *
 * @return int 0-100，100即获取完成
 */
RtkInfomation RadarController::getRtkMean()
{
    RtkInfomation info;
    info.stars = (double)m_rtkInfoMean_.stars / (double)m_rtkInfoMean_.count + 0.5;
    info.lat = m_rtkInfoMean_.lat / (double)m_rtkInfoMean_.count;
    info.lon = m_rtkInfoMean_.lon / (double)m_rtkInfoMean_.count;
    info.heading = m_rtkInfoMean_.baseHeading + m_rtkInfoMean_.diffHeading / (double)m_rtkInfoMean_.count;
    info.alt = m_rtkInfoMean_.heading / (double)m_rtkInfoMean_.count;
    info.count = m_rtkInfoMean_.count;

    return info;
}

/**
 * @brief 计算other与base的角度差
 * @return other在base右侧返回正数，在左侧返回负数
*/
__inline__ float RadarController::calcAngleDelta(float base, float other)
{
    float result;
    float phi = fmodf(other - base, 360.0f);
    phi += (phi < 0) ? 360 : 0;
    float sign = -1;
    if ((phi >= 0 && phi <= 180)
        || (phi <= -180 && phi >= -360)) {
        sign = 1;
    }
    if (phi > 180) {
        result = 360 - phi;
    } else {
        result = phi;
    }
    return result * sign;
}

/**
 * @brief 接收RTK信息的回调
 * 在使用requestRtkInfomation方法开启RTK后，此回调每隔1s会接收到RTK信息，关闭RTK后此回调不再触发
 *
 * @param stars 卫星数量
 * @param lat 纬度
 * @param lon 经度
 * @param heading 零度指向
 * @param alt 海拔高度（预留，暂不支持）
 * @return void
 */
void RadarController::onRecvRtkMessage(int stars, double lat, double lon, double heading, double alt)
{
    if (heading < 0) heading += 360;

    //qDebug()<<"onRecvRtkMessage stars:"<<stars<<" lat:"<<lat<<" lon:"<<lon<<" heading:"<<heading;

    if (m_rtkInfoMean_.isFirst)
    {
        m_rtkInfoMean_.baseHeading = heading;
        m_rtkInfoMean_.isFirst = false;
    }

    if (!rtkMeanCompleted())
    {
        m_rtkInfoMean_.stars += stars;
        m_rtkInfoMean_.lat += lat;
        m_rtkInfoMean_.lon += lon;
        m_rtkInfoMean_.heading += heading;
        m_rtkInfoMean_.diffHeading += calcAngleDelta(m_rtkInfoMean_.baseHeading, heading);
        m_rtkInfoMean_.alt += alt;
        m_rtkInfoMean_.count ++;
        emit rtkProgressUpdate(getPercentageOfRtkMean());
    }

    if(rtkMeanCompleted() && m_rtkFirstCompleted)
    {
        m_rtkFirstCompleted = false;
        requestRtkInfomation(false);
        RtkInfomation info = getRtkMean();
        emit rtkCalculationCompleted(info);
        //qDebug()<<"onSendRtkMessage2"<<m_rtkFirstCompleted;
    }
}

/**
 * @brief 请求设备配置信息
 * 用于获取雷达基础配置，仅发送请求
 * 雷达响应后，会在onTcpClientReceiveRegisters回调中收到设备配置信息
 *
 * @return void
 */
void RadarController::getDeviceConfig()
{
    QList<uint32_t> addrs;
    addrs.append(PrjTcpClient::REG_ADDR_USER_CONFIG_0);
    if (m_tcpClient_ && m_tcpClient_->isLogin())
    {
        m_tcpClient_->getRegisters(addrs);
    }

}

/**
 * @brief 设置设备配置信息
 * 用于设置雷达基础配置
 * 调用此方法设置寄存器成功后，会在onTcpClientMessage回调中收到调用成功的消息，失败在onTcpClientError回调收到消息
 *
 * @return void
 */
void RadarController::setDeviceConfig(HardwareConfig config)
{
    RegisterList registers;
    uint32_t value1 = (((uint32_t)config.freq & 0xFF) << 24)
            | (((uint32_t)config.speed & 0xFF) << 16)
            | (((uint32_t)config.cfar & 0xFF) << 8)
            | ((uint32_t)config.rcs & 0xFF);
    registers.append(QPair<uint32_t, uint32_t>(PrjTcpClient::REG_ADDR_USER_CONFIG_0, value1));

    if (m_tcpClient_ && m_tcpClient_->isLogin())
    {
        m_tcpClient_->setRegisters(registers);
    }
}

void RadarController::setJsonConfig(int addr, QByteArray jsonData)
{
    if (m_tcpClient_ && m_tcpClient_->isLogin())
    {
        m_tcpClient_->setJsonMessage(addr, QString::fromUtf8(jsonData));
    }

}

void RadarController::getJsonConfig(int addr)
{
    if (m_tcpClient_ && m_tcpClient_->isLogin())
    {
        m_tcpClient_->getJsonMessage(addr);
    }
}


/**
 * @brief 断线重连定时器回调
 * 用于断线重连，每10s判断是否雷达掉线，若掉线则重新连接
 *
 * @return void
 */
void RadarController::onTimerTimeout()
{
    quint64 nowms = QDateTime::currentMSecsSinceEpoch();
    if (m_tcpClient_ && !m_tcpClient_->isLogin() && ((nowms - m_loginms) > 10000))
    {
        m_tcpClient_->login();
        m_loginms = nowms;
    }
}

/**
 * @brief TCP连接状态改变回调
 * 用于在Tcp连接状态改变后，做一些处理（如登录成功后获取设备配置和工作状态）
 *
 * @return void
 */
void RadarController::onTcpClientStateChanged(radar::net::TcpClient::ClientState state)
{
    // QString fmt = "[%1] tcp state changed:%2";
    switch (state) {
    case TcpClient::DisconnectedState:
        // QLOG_INFO() << fmt.arg(m_config.name).arg("DisconnectedState");
        break;
    case TcpClient::ConnectingState:
        // QLOG_INFO() << fmt.arg(m_config.name).arg("ConnectingState");
        break;
    case TcpClient::ConnectedState:
        // QLOG_INFO() << fmt.arg(m_config.name).arg("ConnectedState");
        break;
    case TcpClient::LoginState:
        // QLOG_INFO() << fmt.arg(m_config.name).arg("LoginState");
        getDeviceConfig();
        getWorkStatus();
        getJsonConfig(0);
        break;
    case TcpClient:: LogoutingState:
        // QLOG_INFO() << fmt.arg(m_config.name).arg("LogoutingState");
        break;
    case TcpClient::LogoutState:
        // QLOG_INFO() << fmt.arg(m_config.name).arg("LogoutState");
        break;
    default:
        break;
    }
}

/**
 * @brief TCP客户端错误回调
 * 用于接收TCP客户端的错误信息，如设置寄存器失败
 *
 * @return void
 */
void RadarController::onTcpClientError(int32_t errCode, QString errMsg)
{
    QString fmt = "[%1] tcp error: (%2)%3";
    //QLOG_INFO() << fmt.arg(m_config.name.c_str()).arg(errCode).arg(errMsg);
}

/**
 * @brief TCP客户端消息回调
 *
 * 用于接收TCP客户端的消息，如设置寄存器成功
 *
 * @return void
 */
void RadarController::onTcpClientMessage(QString msg)
{
    QString fmt = "[%1] tcp message: %2";
    //QLOG_INFO() << fmt.arg(m_config.name.c_str(), msg);
}

/**
 * @brief 获取寄存器回调
 * 用于在调用getWorkStatus、getDeviceConfig等方法后，接收工作状态、设备配置等信息
 *
 * @note 雷达工作状态和设备基础配置的设置和获取都是通过操作设备寄存器实现的
 *
 * @return void
 */
void RadarController::onTcpClientReceiveRegisters(RegisterList registers)
{
    for (Register reg : registers)
    {
        // 设备配置信息
        if (reg.first == (PrjTcpClient::REG_ADDR_USER_CONFIG_0))
        {
            m_config.freq = (reg.second >> 24) & 0xFF;
            m_config.speed = (reg.second >> 16) & 0xFF;
            m_config.cfar = (reg.second >> 8) & 0xFF;
            m_config.rcs = reg.second & 0xFF;
            emit recvDeviceConfig(m_config);
        }
        // 工作状态信息
        else if (reg.first == (PrjTcpClient::REG_ADDR_USER_CONFIG_2))
        {
            uint8_t workingMode = reg.second & 0xFF;
            uint8_t rotationalSpeed = (reg.second >> 8) & 0xFF;

            if(workingMode == 0x00) //待机
            {
                m_workingMode = StandBy;
            }
            else if (workingMode == 0x01 && rotationalSpeed == 0x04) //周扫1s
            {
                m_workingMode = CirSweep1s;
            }
            else if (workingMode == 0x01 && rotationalSpeed == 0x01) //周扫2s
            {
                m_workingMode = CirSweep2s;
            }
            else if (workingMode == 0x01 && rotationalSpeed == 0x02) //周扫4s
            {
                m_workingMode = CirSweep4s;
            }
            emit recvWorkStatus(m_workingMode);
        }
    }
}

void RadarController::onTcpClientReceiveJsonConfig(int addr, QByteArray &jsonStr)
{
    if (addr == 0) //坐标配置
    {
        // 1. 初始化返回值
        double lat = 0.0;
        double lon = 0.0;
        double altitude = 0.0;
        double yaw = 0.0;

        // 2. 解析JSON字符串为QJsonDocument（捕获解析错误）
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qCritical() << "JSON解析失败：" << parseError.errorString()
                << "位置：" << parseError.offset;
            return;
        }

        // 3. 检查根节点是否为对象
        if (!jsonDoc.isObject()) {
            qCritical() << "JSON根节点不是对象";
            return;
        }
        QJsonObject rootObj = jsonDoc.object();

        // 4. 提取position子对象（检查字段是否存在）
        if (!rootObj.contains("position")) {
            qCritical() << "JSON中不存在position字段";
            return;
        }
        QJsonValue positionValue = rootObj.value("position");
        if (!positionValue.isObject()) {
            qCritical() << "position字段不是对象类型";
            return;
        }
        QJsonObject positionObj = positionValue.toObject();

        // 5. 提取position中的四个字段（检查字段存在性+类型）
        // 纬度lat
        if (!positionObj.contains("lat") || !positionObj["lat"].isDouble()) {
            qCritical() << "position中lat字段缺失或类型错误";
            return;
        }
        lat = positionObj["lat"].toDouble();

        // 经度lon
        if (!positionObj.contains("lon") || !positionObj["lon"].isDouble()) {
            qCritical() << "position中lon字段缺失或类型错误";
            return;
        }
        lon = positionObj["lon"].toDouble();

        // 高度altitude（兼容int/double类型）
        if (!positionObj.contains("altitude")) {
            qCritical() << "position中altitude字段缺失";
            return;
        }
        altitude = positionObj["altitude"].isDouble()
                       ? positionObj["altitude"].toDouble()
                       : positionObj["altitude"].toInt();

        // 偏航角yaw
        if (!positionObj.contains("yaw") || !positionObj["yaw"].isDouble()) {
            qCritical() << "position中yaw字段缺失或类型错误";
            return;
        }
        yaw = positionObj["yaw"].toDouble();

        RtkInfomation info;
        info.lon = lon;
        info.lat = lat;
        info.alt = altitude;
        info.heading = yaw;
        emit rtkCalculationCompleted(info);
    }
}

/**
 * @brief 航迹数据回调
 * 用于接收雷达上传的航迹数据
 * 当雷达工作模式处于周扫状态时，雷达会定时上传航迹数据
 *
 * @param stamp 当前数据帧的时间戳，使用此字段需先对雷达进行校时
 * @param frame 当前数据帧号
 * @param scanBoundaryA 当前帧所扫描的扇区的起始角度，该值除10000为真实角度值，如1253560/10000=125.356
 * @param scanBoundaryB 当前帧所扫描的扇区的终止角度，该值除10000为真实角度值
 * @param direction 转台的扫描方向，预留字段不使用
 * @param points 航迹数据
 * @return void
 */

/*
void RadarController::onRecvTrackPointsV3(int64_t stamp, int32_t frame,
                                        int32_t scanBoundaryA, int32_t scanBoundaryB, int32_t direction,
                                        const QMap<uint32_t, QMap<QString, QVariant>>& points)
{
    if (points.size() == 0) return;

    // qDebug()<<"recv track data:";
    // 遍历多条航迹数据，并输出
    QList<uint32_t> currTids = points.keys();
    foreach (uint32_t tid, currTids) {
        // qDebug()<<"Azimuth:"<<points[tid].value(tr("Azimuth")).toDouble()   //目标方位
        //     <<" Pitch:"<<points[tid].value(tr("Pitch")).toDouble()          //目标俯仰
        //     <<" Distance:"<<points[tid].value(tr("Distance")).toDouble()    //目标距离
        //     <<" Height:"<<points[tid].value(tr("Height")).toDouble()        //目标高度
        //     <<" Velocity:"<<points[tid].value(tr("Velocity")).toDouble()    //目标真实速度
        //     <<" Type:"<<points[tid].value(tr("Type")).toInt()               //目标类型，0:未识别，1:行人，2:车辆，3:无人机，4:鸟类，5:鸟群，6:大飞机
        //     <<" SNR:"<<points[tid].value(tr("SNR")).toDouble()              //信噪比
        //     <<" RCS2:"<<points[tid].value(tr("RCS2")).toDouble()            //RCS，精度为0.01，小于0.01一律为0
        //     <<" RCS6:"<<points[tid].value(tr("RCS6")).toDouble()            //RCS，精度为0.000001
        //     <<" PrivDistance:"<<points[tid].value(tr("PrivDistance")).toDouble()//预测距离，雷达传出数据有延迟，如需引导光电等设备需要使用预测位置
        //     <<" PrivAzimuth:"<<points[tid].value(tr("PrivAzimuth")).toDouble()  //预测方位
        //     <<" PrivHeight:"<<points[tid].value(tr("PrivHeight")).toDouble();   //预测高度
    }

    emit recvTrackData(points);
}
*/


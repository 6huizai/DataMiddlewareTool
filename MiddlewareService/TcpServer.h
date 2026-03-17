#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QVector>
#include <QTimer>
#include "TcpMessage.h"
#include "Buffer.h"

using namespace radar;
using namespace radar::net;

enum class WorkMode
{
    StandBy = 0,    // 待机模式
    CirSweep1s, // 周扫一秒一圈
    CirSweep2s, // 周扫两秒一圈
    FanSweep    // 扇扫（暂不支持）
};

struct RadarConfigParm
{
    uint32_t freq = 0;  //
    uint32_t speed = 0; //
    uint32_t cfar = 0;  //
    uint32_t rcs = 0;   //
};

struct RadarStatus
{
    WorkMode mode;
    RadarConfigParm param;
};

class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    TcpServer(QObject *parent = nullptr);
    ~TcpServer();

public:
    void sendData(QByteArray data);
    void closeAllSocket();

signals:
    void sendPacket(QByteArray packet);

public slots:
    void sendResponse(QByteArray packet);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    void subcontract(QTcpSocket *socket);
    void parsePacket(QByteArray packet, QTcpSocket *socket);

private:
    QVector<QTcpSocket*> clients;
    QMap<QTcpSocket*, Buffer*> m_buffers;
    RadarConfigParm m_config;
    WorkMode m_mode = WorkMode::StandBy;
    //Buffer m_recvBuffer;

    const static int kHeaderLen = 2*sizeof(int32_t);
    const static int kCrc16Len = sizeof(uint16_t);
    const static int kMinMessageLen = kHeaderLen + kCrc16Len; // nameLen + typeName + checkSum
    const static int kMaxMessageLen = 128*1024*1024; // same as codec_stream.h kDefaultTotalBytesLimit
    const static uint32_t kHeaderMagic = 0x55AA55AA;

private:
    QTimer *m_timer = nullptr;
};

#endif // TCPSERVER_H

#ifndef SERVICEPANEL_H
#define SERVICEPANEL_H

#include <QWidget>
#include <QFileDialog>
#include <QString>
#include <QDateTime>
#include <cmath>
#include <QTimer>
#include "TcpServer.h"
#include <QUdpSocket>
#include "prjmessage.h"

namespace Ui {
class ServicePanel;
}

class ServicePanel : public QWidget
{
    Q_OBJECT

public:
    explicit ServicePanel(QWidget *parent = nullptr);
    ~ServicePanel();

    int selectObject();
    TcpServer *tcpServer();
    void sendUdpData(QByteArray &data);

private slots:
    void on_pushButton_start_clicked();
    void on_pushButton_resetNet_clicked();

public:
    QVector<radar::net::UploadTrackPointsMessageV3> all_Message_read;
    QVector<QByteArray> all_Message_dumpbody;
    bool m_Is_Read_All_Data = false;
    int db_frame = 0;
    int typeSelect[6] = {0};

    // 创建定时器
    QTimer *timer = new QTimer();

private:
    Ui::ServicePanel *ui;
    TcpServer *m_tcpServer = nullptr;
    QUdpSocket *m_udpSocket = nullptr;
    QHostAddress m_udpIP;
    quint32 m_udpPort;

    bool timerEnabled;

};

#endif // SERVICEPANEL_H

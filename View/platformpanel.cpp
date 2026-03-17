#include "platformpanel.h"
#include "ui_platformpanel.h"
#include <QDebug>

PlatformPanel::PlatformPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlatformPanel)
{
    ui->setupUi(this);

    ui->pushButton_login->hide();
    ui->pushButton_heartbeat->hide();
    ui->pushButton_sendData->hide();
}

PlatformPanel::~PlatformPanel()
{
    delete ui;
}

void PlatformPanel::onConnected()
{
    ui->pushButton_connect->setText("断开");
}

void PlatformPanel::onDisconnected()
{
    ui->pushButton_connect->setText("登录");
}

void PlatformPanel::onLogined()
{
    ui->pushButton_connect->setText("登出");
}

void PlatformPanel::on_pushButton_connect_clicked()
{
    if (ui->pushButton_connect->text() == "登录")
    {
        QHostAddress ip = QHostAddress(ui->lineEdit_ip->text());
        int port = ui->lineEdit_port->text().toInt();
        QString deviceSN = ui->lineEdit_sn->text();

        emit connectToServer(ip, port, deviceSN);
    }
    else if (ui->pushButton_connect->text() == "断开" || ui->pushButton_connect->text() == "登出")
    {
        emit disconnectFromServer();
    }
}

void PlatformPanel::on_pushButton_login_clicked()
{
    //m_platformClient->login();
    emit login();
}

void PlatformPanel::on_pushButton_heartbeat_clicked()
{
    //m_platformClient->heartbeat();
    emit heartbeat();
}

void PlatformPanel::on_pushButton_sendData_clicked()
{
    // DroneInfo drone;
    // drone.id = 12345;
    // drone.lat = 34.123456;
    // drone.lon = 108.123456;
    // drone.speed = 10;
    // drone.heading = 45;
    // drone.height = 101;
    // drone.distance = 3487;

    // m_platformClient->sendDrone(drone);
}

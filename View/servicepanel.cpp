#include "servicepanel.h"
#include "ui_servicepanel.h"
#include <QHostAddress>

ServicePanel::ServicePanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServicePanel)
{
    ui->setupUi(this);

    m_tcpServer = new TcpServer(this);
    m_udpSocket = new QUdpSocket(this);

    m_udpIP = QHostAddress("127.0.0.1");
    m_udpPort = 6666;
}

ServicePanel::~ServicePanel()
{
    delete ui;
}

int ServicePanel::selectObject() {
    if(ui->checkBox_all->isChecked())
    {
        typeSelect[0] = 1;
        return 0;
    }
    else
    {
        typeSelect[0] = 0;

        if(ui->checkBox_person->isChecked())
        {
            typeSelect[1] = 1;
        }
        else
        {
            typeSelect[1] = 0;
        }

        if(ui->checkBox_verhicle->isChecked())
        {
            typeSelect[2] = 1;
        }
        else
        {
            typeSelect[2] = 0;
        }

        if(ui->checkBox_UAV->isChecked())
        {
            typeSelect[3] = 1;
        }
        else
        {
            typeSelect[3] = 0;
        }

        if(ui->checkBox_brid->isChecked())
        {
            typeSelect[4] = 1;
        }
        else
        {
            typeSelect[4] = 0;
        }

        if(ui->checkBox_airplane->isChecked())
        {
            typeSelect[5] = 1;
        }
        else
        {
            typeSelect[5] = 0;
        }

        return -1;
    }
}

TcpServer *ServicePanel::tcpServer()
{
    return m_tcpServer;
}

void ServicePanel::sendUdpData(QByteArray &data)
{
    m_udpSocket->writeDatagram(data, m_udpIP, m_udpPort);
}

void ServicePanel::on_pushButton_start_clicked()
{
    QString text = ui->pushButton_start->text();
    QString ip = ui->lineEdit_ip->text();
    QString port = ui->lineEdit_port->text();

    if (text == tr("Start service"))
    {
        if (m_tcpServer->listen(QHostAddress(ip), port.toInt()))
        {
            ui->pushButton_start->setText(tr("Shut down service"));
            // QString msg = QString("初始化网络服务成功 (IP:%1, Port:%2)").arg(ip).arg(port);

            //add by hs
            // m_tcpServer->sendData();
        }
        else
        {
            // QString msg = QString("初始化网络服务失败 (IP:%1, Port:%2)").arg(ip).arg(port);
            // onLog(msg);
        }
    }
    else if (text == tr("Shut down service"))
    {
        ui->pushButton_start->setText(tr("Start service"));

        m_tcpServer->closeAllSocket(); // 关闭现有连接
        m_tcpServer->close(); // 退出监听模式

        //ui->textEdit_log->append(QString("关闭网络服务 (IP:%1, Port:%2)").arg(ip).arg(port));
        // QString msg = QString("关闭网络服务 (IP:%1, Port:%2)").arg(ip).arg(port);
        // onLog(msg);
    }
}


void ServicePanel::on_pushButton_resetNet_clicked()
{
    QString ip = ui->lineEdit_udpIP->text();
    QString port = ui->lineEdit_udpPort->text();
    m_udpIP = QHostAddress(ip);
    m_udpPort = port.toInt();
}


#include "controlpanel.h"
#include "ui_controlpanel.h"
#include "radarcontroller.h"
#include <QMessageBox>

ControlPanel::ControlPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ControlPanel)
{
    ui->setupUi(this);
    // ui->progressBar_rtk->hide();
    ui->lineEdit_lat->setEnabled(false);
    ui->lineEdit_lon->setEnabled(false);
    ui->lineEdit_yaw->setEnabled(false);
    ui->pushButton_rtk->setEnabled(false);
    ui->comboBox_mode->setEnabled(false);
    ui->pushButton_run->setEnabled(false);
    ui->comboBox_freq->setEnabled(false);
    ui->comboBox_detect->setEnabled(false);
    ui->comboBox_rcs->setEnabled(false);

    // ui->groupBox_2->hide();
}

ControlPanel::~ControlPanel()
{
    delete ui;
}

QHostAddress ControlPanel::ip()
{
    return QHostAddress(ui->lineEdit_ip->text());
}

int ControlPanel::port()
{
    return ui->lineEdit_port->text().toInt();
}

void ControlPanel::on_pushButton_login_clicked()
{
    if (ui->pushButton_login->text() == tr("Login"))
    {
        emit login(QHostAddress(ui->lineEdit_ip->text()), ui->lineEdit_port->text().toInt(), true);
        ui->pushButton_login->setText(tr("Logout"));
        ui->lineEdit_ip->setEnabled(false);
        ui->lineEdit_port->setEnabled(false);
        ui->lineEdit_lat->setEnabled(true);
        ui->lineEdit_lon->setEnabled(true);
        ui->lineEdit_yaw->setEnabled(true);
        ui->pushButton_rtk->setEnabled(true);
        ui->comboBox_mode->setEnabled(true);
        ui->pushButton_run->setEnabled(true);
        ui->comboBox_freq->setEnabled(true);
        ui->comboBox_detect->setEnabled(true);
        ui->comboBox_rcs->setEnabled(true);
    }
    else if (ui->pushButton_login->text() == tr("Logout"))
    {
        emit login(QHostAddress(ui->lineEdit_ip->text()), ui->lineEdit_port->text().toInt(), false);
        ui->pushButton_login->setText(tr("Login"));
        ui->lineEdit_ip->setEnabled(true);
        ui->lineEdit_port->setEnabled(true);
        ui->lineEdit_lat->setEnabled(false);
        ui->lineEdit_lon->setEnabled(false);
        ui->lineEdit_yaw->setEnabled(false);
        ui->pushButton_rtk->setEnabled(false);
        ui->comboBox_mode->setEnabled(false);
        ui->pushButton_run->setEnabled(false);
        ui->comboBox_freq->setEnabled(false);
        ui->comboBox_detect->setEnabled(false);
        ui->comboBox_rcs->setEnabled(false);
    }
}

void ControlPanel::on_pushButton_run_clicked()
{
    if (ui->pushButton_run->text() == tr("Run"))
    {
        emit setWorkMode(WorkingMode(ui->comboBox_mode->currentIndex()));
        ui->pushButton_run->setText(tr("Stop"));
        ui->comboBox_mode->setEnabled(false);
    }
    else if (ui->pushButton_run->text() == tr("Stop"))
    {
        emit setWorkMode(StandBy);
        ui->pushButton_run->setText(tr("Run"));
        ui->comboBox_mode->setEnabled(true);
    }
}

void ControlPanel::on_pushButton_rtk_clicked()
{
    if (ui->pushButton_run->text() == tr("Stop"))
    {
        QMessageBox::information(this, tr("Prompt"), tr("Please stop the radar first."));
        return;
    }

    emit requestRTK(true);
    ui->pushButton_rtk->setEnabled(false);
    ui->pushButton_rtk->setText(tr("RTK acquisition in progress..."));
    ui->progressBar_rtk->show();
    ui->comboBox_mode->setEnabled(false);
    ui->pushButton_run->setEnabled(false);
}

void ControlPanel::onRecvWorkStatus(WorkingMode mode)
{
    qDebug()<<"onRecvWorkStatus"<<mode;
    if (mode == StandBy)
    {
        ui->pushButton_run->setText(tr("Run"));
        ui->comboBox_mode->setEnabled(true);
    }
    else
    {
        ui->pushButton_run->setText(tr("Stop"));
        ui->comboBox_mode->setCurrentIndex(mode);
        ui->comboBox_mode->setEnabled(false);
    }
}

void ControlPanel::onRecvDeviceConfig(HardwareConfig config)
{
    //qDebug()<<"onRecvDeviceConfig"<<config.freq<<config.cfar<<config.rcs;
    ui->comboBox_freq->setCurrentIndex(config.freq);
    ui->comboBox_detect->setCurrentIndex(config.cfar);
    ui->comboBox_rcs->setCurrentIndex(config.rcs);
}

void ControlPanel::onRtkProgressUpdate(int progress)
{
    ui->progressBar_rtk->setValue(progress);
}

void ControlPanel::onRtkCalculationCompleted(RtkInfomation info)
{
    ui->lineEdit_lat->setText(QString::number(info.lat, 'f', 6));
    ui->lineEdit_lon->setText(QString::number(info.lon, 'f', 6));
    ui->lineEdit_yaw->setText(QString::number(info.heading, 'f', 2));

    ui->pushButton_rtk->setEnabled(true);
    ui->pushButton_rtk->setText(tr("Request RTK"));
    ui->progressBar_rtk->hide();
    ui->progressBar_rtk->setValue(0);
    ui->comboBox_mode->setEnabled(true);
    ui->pushButton_run->setEnabled(true);

    QMessageBox::information(this, tr("Prompt"), tr("RTK obtained successfully."));
}

void ControlPanel::on_comboBox_freq_currentIndexChanged(int index)
{
    HardwareConfig config = getHardwareConfig();
    emit setDeviceConfig(config);
}


void ControlPanel::on_comboBox_detect_currentIndexChanged(int index)
{
    HardwareConfig config = getHardwareConfig();
    emit setDeviceConfig(config);
}


void ControlPanel::on_comboBox_rcs_currentIndexChanged(int index)
{
    HardwareConfig config = getHardwareConfig();
    emit setDeviceConfig(config);
}

HardwareConfig ControlPanel::getHardwareConfig()
{
    HardwareConfig config;
    config.freq = ui->comboBox_freq->currentIndex();
    config.cfar = ui->comboBox_detect->currentIndex();
    config.rcs = ui->comboBox_rcs->currentIndex();
    config.speed = 0;

    return config;
}

#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QHostAddress>
#include "radarcontroller.h"

namespace Ui {
class ControlPanel;
}

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel();

    QHostAddress ip();
    int port();

signals:
    void login(QHostAddress ip, int port, bool isLogin);
    void requestRTK(bool open);
    void setWorkMode(WorkingMode mode);
    void setDeviceConfig(HardwareConfig config);
    void setDisplayRange(int range);

public slots:
    void onRecvWorkStatus(WorkingMode mode);
    void onRecvDeviceConfig(HardwareConfig config);
    void onRtkProgressUpdate(int progress);
    void onRtkCalculationCompleted(RtkInfomation info);

private slots:
    void on_pushButton_login_clicked();
    void on_pushButton_run_clicked();
    void on_pushButton_rtk_clicked();
    void on_comboBox_freq_currentIndexChanged(int index);
    void on_comboBox_detect_currentIndexChanged(int index);
    void on_comboBox_rcs_currentIndexChanged(int index);

private:
    HardwareConfig getHardwareConfig();

private:
    Ui::ControlPanel *ui;
};

#endif // CONTROLPANEL_H

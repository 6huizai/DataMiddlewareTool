#ifndef PLATFORMPANEL_H
#define PLATFORMPANEL_H

#include <QWidget>
#include "PlatformClient.h"

namespace Ui {
class PlatformPanel;
}

class PlatformPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PlatformPanel(QWidget *parent = nullptr);
    ~PlatformPanel();

public slots:
    void onConnected();
    void onDisconnected();
    void onLogined();

signals:
    void connectToServer(QHostAddress ip, int port, QString deviceSN);
    void disconnectFromServer();
    void login();
    void heartbeat();

private slots:
    void on_pushButton_connect_clicked();
    void on_pushButton_login_clicked();
    void on_pushButton_heartbeat_clicked();
    void on_pushButton_sendData_clicked();

private:
    Ui::PlatformPanel *ui;
};

#endif // PLATFORMPANEL_H

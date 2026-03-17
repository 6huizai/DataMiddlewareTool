#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include "radarcontroller.h"
#include "controlpanel.h"
#include "RandomForest.h"
#include "HPMessage.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initUI();
    void initConnection();

private slots:
    void onRecvTrackData(const QMap<uint32_t, QMap<QString, QVariant>>& points);

private:
    QByteArray formatHPMessage(UploadTrackPointsMessageV3 &data);

private:
    Ui::MainWindow *ui;
    ControlPanel *m_controlPanel = nullptr;
private:
    RadarController *m_radarController = nullptr;
    //PlatformClient *m_platformClient = nullptr;
    RandomForest *m_randomForest = nullptr;

    RtkInfomation *m_rtkInfo = nullptr;
};
#endif // MAINWINDOW_H

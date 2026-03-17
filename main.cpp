#include "mainwindow.h"
#include <QApplication>
#include "CConfigHolder.h"
#include <QTranslator>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);

    CConfigHolder *config = new CConfigHolder("config.json");
    config->load();

    qDebug()<<"qConfigData.system.translation"<<qConfigData.system.translation;
    QTranslator translator;
    if (qConfigData.system.translation == 1 && translator.load("translations/RadarDataMiddlewareTool_zh_CN.qm")) {
        // qDebug()<<"qConfigData.system.translation"<<qConfigData.system.translation;
        a.installTranslator(&translator);
    }
    // else if (qConfigData.system.translation == 2 && translator.load("translations/DeviceSearch_ru_RU.qm"))
    // {
    //     qDebug()<<"qConfigData.system.translation"<<qConfigData.system.translation;
    //     a.installTranslator(&translator);
    // }
    // else if (qConfigData.system.translation == 3 && translator.load("translations/DeviceSearch_ko_CN.qm"))
    // {
    //     qDebug()<<"qConfigData.system.translation"<<qConfigData.system.translation;
    //     a.installTranslator(&translator);
    // }

    MainWindow w;
    w.show();

    config->save();
    return a.exec();
}

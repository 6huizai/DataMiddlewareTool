QT += core gui network positioning
QT += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    View \
    RadarController \
    RadarController/network \
    Tools \
    MiddlewareService \
    DataProcessing \
    ConfigManage \

SOURCES += \
    ConfigManage/CConfigData.cpp \
    ConfigManage/CConfigHolder.cpp \
    ConfigManage/FileHolder.cpp \
    ConfigManage/JsonHolder.cpp \
    ConfigManage/Macros.cpp \
    MiddlewareService/HPMessage.cpp \
    DataProcessing/RandomForest.cpp \
    MiddlewareService/TcpMessage.cpp \
    MiddlewareService/TcpServer.cpp \
    RadarController/RadarController.cpp \
    RadarController/network/Codec.cpp \
    RadarController/network/Message.cpp \
    RadarController/network/TcpAdminClient.cpp \
    RadarController/network/TcpClient.cpp \
    RadarController/network/TcpConnection.cpp \
    RadarController/network/TcpDataClient.cpp \
    RadarController/network/crc16modbus.cpp \
    RadarController/network/prjmessage.cpp \
    RadarController/network/prjtcpclient.cpp \
    Tools/projectionutils.cpp \
    View/controlpanel.cpp \
    View/mainwindow.cpp \
    View/servicepanel.cpp \
    main.cpp \


HEADERS += \
    ConfigManage/CConfigData.h \
    ConfigManage/CConfigHolder.h \
    ConfigManage/FileHolder.h \
    ConfigManage/JsonHolder.h \
    ConfigManage/Macros.h \
    MiddlewareService/HPMessage.h \
    DataProcessing/RandomForest.h \
    MiddlewareService/TcpMessage.h \
    MiddlewareService/TcpServer.h \
    RadarController/network/Buffer.h \
    RadarController/network/Callbacks.h \
    RadarController/network/Codec.h \
    RadarController/network/Connection.h \
    RadarController/network/Dispatcher.h \
    RadarController/network/Endian.h \
    RadarController/network/LinkedHashMap.h \
    RadarController/network/Message.h \
    RadarController/network/NodeInfo.h \
    RadarController/network/TcpAdminClient.h \
    RadarController/network/TcpClient.h \
    RadarController/network/TcpConnection.h \
    RadarController/network/TcpDataClient.h \
    RadarController/network/Timestamp.h \
    RadarController/network/Typedefs.h \
    RadarController/network/copyable.h \
    RadarController/network/crc16modbus.h \
    RadarController/network/prjmessage.h \
    RadarController/network/prjtcpclient.h \
    RadarController/network/prjtypedefs.h \
    RadarController/radarcontroller.h \
    Tools/projectionutils.h \
    View/controlpanel.h \
    View/mainwindow.h \
    View/servicepanel.h


FORMS += \
    View/controlpanel.ui \
    View/mainwindow.ui \
    View/servicepanel.ui

win32 {
    # for ntohl
    LIBS += -lWs2_32
}

INCLUDEPATH += $$PWD/3rd/AIGCJson/include
DEPENDPATH += $$PWD/3rd/AIGCJson/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

TRANSLATIONS += \
    RadarDataMiddlewareTool_zh_CN.ts

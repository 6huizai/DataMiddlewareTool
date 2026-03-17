#include "TcpDataClient.h"
#include "Message.h"
#include <QDir>
#include <QDataStream>
#include <QDebug>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

using namespace radar;
using namespace radar::net;

TcpDataClient::TcpDataClient(const QHostAddress& address, quint16 port,
                               QObject *parent)
    : TcpClient(address, port, ClientDataType, parent)
    , path_(QDir::currentPath())
{

}

void TcpDataClient::registerMessages()
{
    TcpClient::registerMessages();
    dispatcher()->registerMessageCallback<UploadSampleDataMessage>(
                MSG_TYPE_UPLOAD_DATA,
                std::bind(&TcpDataClient::onUploadSampleDataMessage,
                          this, _1, _2, _3));
}

void TcpDataClient::onMessage(const ConnectionPtr&, Buffer* buf, Timestamp)
{
 //    emit receiveData(message);
    // qDebug() << "TcpDataClient::onMessage: " << message.size();
//    QMutexLocker locker(&mutex_);
    codec()->onMessage(buf, false);
}

bool TcpDataClient::onUploadSampleDataMessage(const radar::net::NodeInfoPtr&,
                               std::shared_ptr<radar::net::UploadSampleDataMessage>& message,
                               radar::Timestamp)
{
    QString filePath = makeMessagePath(message);
    writeFile(filePath, message->data);
    return true;
}


void TcpDataClient::setPath(const QString& path)
{
    if (isLogin())
    {
        emit error(NullError, "The dataport is logged in and the save path cannot be set. Please log out and try again.");
        return;
    }
    if (path.isEmpty())
    {
        return;
    }
    path_ = path;
    makePath(path_);
}

void TcpDataClient::makePath(const QString& path)
{
    QDir dir(path);
    if(!dir.exists())
    {
        bool retMk = dir.mkpath(path);//创建多级目录
        if(!retMk)
        {
            emit error(NullError, "Failed to create directory:" + path);
            return;
        }
        emit message("Directory created successfully:" + path);
    }
}

QString TcpDataClient::makeTimestampPath(int64_t timestamp)
{
    // qDebug() << "stamp: " << stamp;
    QDateTime time = QDateTime::fromMSecsSinceEpoch(timestamp / 1000);
    // QDateTime time = QDateTime::fromSecsSinceEpoch(stamp);
    QDir dir(path_);
    QString timestampPath = dir.absoluteFilePath(time.toString("yyyyMMddhhmmss"));
    makePath(timestampPath);
    return timestampPath;
}

QString TcpDataClient::makeMessagePath(const std::shared_ptr<radar::net::UploadSampleDataMessage>& message)
{
    QString fileName = QString::asprintf("%d_%d_%d_%d_%d_%08d.bin", message->info.kindA, message->info.kindB,
                                        message->info.sequenceNumber, message->info.distanceBegin,
                                        message->info.distanceNum, message->info.frameNumber);
    QDir dir(makeTimestampPath(message->info.timestamp));
    return dir.absoluteFilePath(fileName);
}

int TcpDataClient::writeFile(const QString& fileName, QByteArray& data)
{
    QFile file(fileName);
    QFileInfo fileInfo(file);

    if (fileInfo.isFile())
    {
        emit message(QString("The file is overwritten: %1").arg(fileInfo.fileName()));
    }
    QIODevice::OpenModeFlag openFlag = QIODevice::Truncate; // QIODevice::Append
    if (!file.open(QIODevice::WriteOnly | openFlag))
    {
        emit error(NullError, "Failed to open file: " + fileName);
        return -1;
    }
    QDataStream ds(&file);
    int wBytes = ds.writeRawData(data.data(), data.size());
    file.close();

    if(data.size() != wBytes)
    {
        emit error(NullError, QString("Error writing to file: Data length(%1)，Write length(%2)").arg(data.size()).arg(wBytes));
        return -2;
    }
    emit message("Write to a file: " + fileInfo.fileName());
    return 0;
}

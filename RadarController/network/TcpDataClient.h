#ifndef RADAR_NET_TCP_DATA_CLIENT_H
#define RADAR_NET_TCP_DATA_CLIENT_H

#include "TcpClient.h"
#include <QMutex>

namespace radar {
namespace net {

class TcpDataClient : public TcpClient
{
    Q_OBJECT
public:
    explicit TcpDataClient(const QHostAddress& address, quint16 port,
                  QObject *parent = nullptr);

    void setPath(const QString& path);

    virtual void registerMessages() override;

signals:
    void receiveData(QByteArray);

protected slots:
    virtual void onMessage(const ConnectionPtr&, Buffer* buf, Timestamp) override;

private slots:
    bool onUploadSampleDataMessage(const radar::net::NodeInfoPtr&,
                                   std::shared_ptr<radar::net::UploadSampleDataMessage>& message,
                                   radar::Timestamp);

private:
    void makePath(const QString& path);
    QString makeTimestampPath(int64_t timestamp);
    QString makeMessagePath(const std::shared_ptr<radar::net::UploadSampleDataMessage>& message);
    int writeFile(const QString& filename, QByteArray& data);

private:
    QString path_;
    QMutex mutex_;
};

}
}


#endif // TCPDATACLIENT_H

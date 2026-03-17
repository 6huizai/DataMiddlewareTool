#ifndef RADAR_NET_CONNECTION_H
#define RADAR_NET_CONNECTION_H

#include "Callbacks.h"
#include "Buffer.h"

#include <QObject>
// #include <QMutex>

namespace radar {
namespace net {

class Connection : public QObject, public std::enable_shared_from_this<Connection>
{
    Q_OBJECT
public:
    Connection(QObject *parent = nullptr)
        : QObject(parent)
        , messageCallback_(std::bind(Connection::defaultMessageCallback, _1, _2, _3))
    {

    }

    void setMessageCallback(const ConnectionMessageCallback& cb)
    {
        messageCallback_ = cb;
    }

    virtual void send(const QByteArray& message) = 0;

protected:
    Buffer* inputBuffer() { return &inputBuffer_; }

private:
    static void defaultMessageCallback(const ConnectionPtr&,
                                       Buffer*, Timestamp) { }

protected:
    ConnectionMessageCallback messageCallback_;

private:
    // QByteArray inputBuffer_;
    Buffer inputBuffer_;
    // QMutex inputBufferLock_;

};

}
}

#endif // CONNECTION_H

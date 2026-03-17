#include "TcpServer.h"

#include <QDebug>
#include <QRandomGenerator>
#include <QTime>
#include "crc16modbus.h"
#include "Codec.h"

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{

}

TcpServer::~TcpServer()
{
    for(QTcpSocket* client : clients) {
        client->disconnectFromHost();
        client->close();
        client->deleteLater();
    }
}


void TcpServer::closeAllSocket()
{
    foreach (auto socket, clients) {
        socket->close();
    }
    clients.clear();
}

void TcpServer::sendData(QByteArray data)
{
    QByteArray sPacket;
    RadarProtocol::makePacket(MessageType1::TrackV3, data, sPacket, 1);
    for(QTcpSocket* client : clients)
    {
        client->write(sPacket);
    }
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *clientSocket = new QTcpSocket(this);
    clientSocket->setSocketDescriptor(socketDescriptor);
    clients.append(clientSocket);

    Buffer *buffer = new Buffer;
    m_buffers.insert(clientSocket, buffer);

    connect(clientSocket, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &TcpServer::onDisconnected);

    QList<QPair<QString, int>> ips;
    foreach (auto socket, clients) {
        ips.append(QPair<QString, int>(socket->peerAddress().toString(), socket->peerPort()));
    }
}

void TcpServer::onReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if(clientSocket) {
        QByteArray packet = clientSocket->readAll();
        m_buffers[clientSocket]->append(packet.data(), packet.size());

        subcontract(clientSocket);
    }
}

void TcpServer::onDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if(clientSocket) {
        if (m_buffers.contains(clientSocket))
        {
            delete m_buffers[clientSocket];
            m_buffers.remove(clientSocket);
        }

        clients.removeAll(clientSocket);
        clientSocket->deleteLater();
    }
}

static int32_t asInt32(const char* buf)
{
    int32_t be32 = 0;
    ::memcpy(&be32, buf, sizeof(be32));
    return be32toh(be32);
}

void TcpServer::subcontract(QTcpSocket *socket)
{
    while (m_buffers[socket]->readableBytes() >= kMinMessageLen + kHeaderLen)
    {
        const char* p = m_buffers[socket]->peek();
        int32_t magic = asInt32(p);
        int32_t len = asInt32(p+4);
        if (magic != kHeaderMagic || len > kMaxMessageLen || len < kMinMessageLen)
        {
            //errorCallback_(kInvalidLength);
            m_buffers[socket]->retrieveAll();
            break;
        }
        else if (m_buffers[socket]->readableBytes() >= implicit_cast<size_t>(len + kHeaderLen))
        {
            Codec::ErrorCode errorCode = Codec::kNoError;
            if (true)
            {
                const uint8_t *pcrc = (uint8_t*)(m_buffers[socket]->peek())+kHeaderLen+len-kCrc16Len;
                uint16_t crc = (pcrc[0] << 8) | pcrc[1];
                if ((crc == 0xCCCC) || (0 == CRC16_MODBUS((m_buffers[socket]->peek()+kHeaderLen), len))) {
                    errorCode = Codec::kNoError;
                } else {
                    QByteArray ba(m_buffers[socket]->peek()+kHeaderLen, len);
                    // QLOG_ERROR() << "CRC CODE: " << QString::number(crc, 16);
                    // QLOG_ERROR() << "CRC ERROR: " << ba.toHex(' ');
                    errorCode = Codec::kCheckSumError;
                }
            }
            if (errorCode == Codec::kNoError)
            {
                QByteArray message;
                message.append(m_buffers[socket]->peek(), kHeaderLen+len);
                m_buffers[socket]->retrieve(kHeaderLen+len);

                parsePacket(message, socket);

                //messageCallback_(message);
                // dispatcher_->onByteArryMessage(
                //     node_, message, Timestamp::currentDateTime());
            }
            else
            {
                //errorCallback_(errorCode);
                m_buffers[socket]->retrieveAll();
                break;
            }
        }
        else
        {
            break;
        }
    }
}

void TcpServer::parsePacket(QByteArray packet, QTcpSocket *socket)
{
    QByteArray data;

    quint32 id;
    MessageType1 type = RadarProtocol::parsePacket(packet, data, id);
    if (type == (MessageType1)-1) return;
    switch (type) {
    case MessageType1::HeartBeat:
    {
        qDebug()<<"TcpServer::parsePacket  HeartBeat";
        HeartbeatMessage message;
        message.parseBody(data);

        QByteArray sPacket;
        RadarProtocol::makePacket(MessageType1::HeartBeat, message.dumpBody(), sPacket, id);
        socket->write(sPacket);

        break;
    }
    case MessageType1::Login:
    {
        qDebug()<<"TcpServer::parsePacket  Login";
        LoginMessage message;
        message.parseBody(data);

        QByteArray sPacket;
        RadarProtocol::makePacket(MessageType1::Login, message.dumpBody(), sPacket, id);
        socket->write(sPacket);

        break;
    }
    case MessageType1::GetReg:
    {
        emit sendPacket(packet);
        break;
    }
    case MessageType1::SetReg:
    {
        emit sendPacket(packet);
        break;
    }
    case MessageType1::SetJson:
    {
        emit sendPacket(packet);
        break;
    }
    case MessageType1::GetJson:
    {
        emit sendPacket(packet);
        break;
    }
    case MessageType1::SetSelect:
    {
        emit sendPacket(packet);
        break;
    }
    case MessageType1::RequestRTK:
    {
        emit sendPacket(packet);
        break;
    }

    default:
        break;
    }
}


void TcpServer::sendResponse(QByteArray packet)
{
    qDebug()<<"TcpServer::sendResponse"<<packet.toHex(' ');

    QByteArray sPacket;
    RadarProtocol::makePacket2(packet, sPacket);

    qDebug()<<"TcpServer::sendResponse222"<<sPacket.toHex(' ');

    for(QTcpSocket* client : clients)
    {
        client->write(sPacket);
    }
}

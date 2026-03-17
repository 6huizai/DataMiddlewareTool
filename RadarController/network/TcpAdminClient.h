#ifndef RADAR_NET_TCP_ADMIN_CLIENT_H
#define RADAR_NET_TCP_ADMIN_CLIENT_H

#include "TcpClient.h"

namespace radar {
namespace net {

class TcpAdminClient : public TcpClient
{
    Q_OBJECT
public:
    explicit TcpAdminClient(const QHostAddress& address, quint16 port,
                            QObject *parent = nullptr);

    void setRegisters(RegisterList& registers);
    void getRegisters(QList<uint32_t>& addresses);
    void setMuxRegisters(uint32_t address, MuxRegisterList& muxRegisters);
    void getMuxRegisters(uint32_t address, QList<uint16_t> muxAddresses);
    void setJsonMessage(uint32_t address, QString jsonStr);
    void getJsonMessage(uint32_t address);
    void setSelectedTrackMessage(QSet<uint32_t> trackIds);
    void getSelectedTrackMessage();

    virtual void registerMessages() override;

private slots:
    bool onSetRegisterAcknowledge(const radar::net::NodeInfoPtr&,
                                  std::shared_ptr<SetRegisterAcknowledge>& ack,
                                  radar::Timestamp);
    bool onGetRegisterAcknowledge(const radar::net::NodeInfoPtr&,
                                  std::shared_ptr<GetRegisterAcknowledge>& ack,
                                  radar::Timestamp);
    bool onSetMuxRegisterAcknowledge(const radar::net::NodeInfoPtr&,
                                     std::shared_ptr<SetMuxRegisterAcknowledge>& ack,
                                     radar::Timestamp);
    bool onGetMuxRegisterAcknowledge(const radar::net::NodeInfoPtr&,
                                     std::shared_ptr<GetMuxRegisterAcknowledge>& ack,
                                     radar::Timestamp);
    bool onGetJsonAcknowledge(const NodeInfoPtr&,
                              std::shared_ptr<GetJsonMessageAcknowledge>& ack,
                              Timestamp);

signals:
    void receiveRegisters(radar::net::RegisterList& registers);
    void receiveMuxRegisters(uint32_t address, radar::net::MuxRegisterList& muxRegisters);
    void receiveJsonConfig(int addr, QByteArray &jsonStr);
};

}
}



#endif // TCPADMINCLIENT_H

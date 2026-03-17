#ifndef RADAR_NET_NODEINFO_H
#define RADAR_NET_NODEINFO_H

#include <memory>

namespace radar {
namespace net {

class NodeInfo
{
public:
    enum NodeType
    {
        NodeNnknwonType = -1,
        NodeTmpType = 0,
        NodeLoginType,
        NodeUserType = 0x02, // 0x02
        NodeAdminType,  // 0x03
        NodeDebugType,  // 0x04
        NodeDataType,  // 0x05
    };

    explicit NodeInfo(int32_t type = NodeNnknwonType)
        : type_(type) {}

    int32_t type() const { return type_; }
    void setType(NodeType type) { type_ = type; }

private:
    int32_t type_;

};

typedef std::shared_ptr<NodeInfo> NodeInfoPtr;

}
}

#endif // NODEINFO_H

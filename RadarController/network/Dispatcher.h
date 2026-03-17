#ifndef RADAR_NET_DISPATCHER_H
#define RADAR_NET_DISPATCHER_H

#include "Timestamp.h"
#include "Typedefs.h"
#include "Message.h"
#include "NodeInfo.h"
//#include <boost/noncopyable.hpp>
#include <stdint.h>
#include <map>

#include <QDataStream>
#include <QDebug>

namespace radar {
namespace net {


class MessageCallback //: boost::noncopyable
{
 public:
  virtual ~MessageCallback() = default;
  virtual void onMessage(const NodeInfoPtr&,
                         const MessagePtr& message,
                         Timestamp) const = 0;
};

template <typename T>
class MessageCallbackT : public MessageCallback
{
  static_assert(std::is_base_of<Message, T>::value,
                "T must be derived from radar::net::Message.");
 public:
  typedef std::function<void (NodeInfoPtr&,
                              const std::shared_ptr<T>& message,
                              Timestamp)> MessageTCallback;

  MessageCallbackT(const MessageTCallback& callback)
    : callback_(callback)
  {
  }

  virtual void onMessage(const NodeInfoPtr& conn,
                 const MessagePtr& message,
                 Timestamp receiveTime) const override
  {
    std::shared_ptr<T> concrete = std::static_pointer_cast<T>(message);
    assert(concrete != NULL);
    callback_(conn, concrete, receiveTime);
  }

 private:
  MessageTCallback callback_;
};

class Dispatcher
{
public:
    typedef std::function<void (const NodeInfoPtr&,
                                QByteArray& message,
                                Timestamp)> DispatcherMessageCallback;

    typedef std::function<void (const NodeInfoPtr&,
                                MessagePtr& message,
                                Timestamp)> BeforeMessageCallback;

    explicit Dispatcher(const DispatcherMessageCallback &cb)
      : defaultCallback_(cb)
      , beforeCallback_(std::bind(&Dispatcher::defaultBeforeCallback,
                                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
    {
    }

    bool onByteArryMessage(NodeInfoPtr& conn,
                         QByteArray& array,
                         Timestamp receiveTime) const
    {
        //qDebug()<<"onByteArryMessage:"<<array.toHex(' ');
        MessagePtr message;
        MessageType type;
        QDataStream stream(&array, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream >> type;
        MessageFactoryMap::const_iterator it = factorys_.find(type);
        if (it != factorys_.end())
        {
            message.reset(it->second->createMessage());
            if (message && message->parseFromArray(array))
            {
                beforeCallback_(conn, message, receiveTime);
                return it->second->onMessage(conn, message, receiveTime);
            }
        }
//		else
//		{
//			defaultCallback_(nodeInfo, stringMessage, receiveTime);
//		}
        return false;
    }

    template<typename T>
    void registerMessageCallback(MessageType type, const typename MessageFactoryT<T>::MessageTCallback& callback)
    {
//		if (type == LOGIN_TYPE || type == LOGOUT_TYPE) {
//			MsgHandlerMap::iterator it = msgHandlerMap_.find(type);
//			if (it != msgHandlerMap_.end()) {
//				return false;
//			}
//		}
        std::shared_ptr<MessageFactoryT<T> > pd(new MessageFactoryT<T>(callback));
        factorys_[type] = pd;
    }

    void setBeforeCallback(const BeforeMessageCallback &cb)
    {
        beforeCallback_ = cb;
    }

private:
    static void defaultBeforeCallback(const NodeInfoPtr&, MessagePtr&, Timestamp)
    {
    }

private:
    typedef std::map<MessageType, std::shared_ptr<MessageFactory>> MessageFactoryMap;
    MessageFactoryMap factorys_;
    DispatcherMessageCallback defaultCallback_;
    BeforeMessageCallback beforeCallback_;
};

}
}

#endif


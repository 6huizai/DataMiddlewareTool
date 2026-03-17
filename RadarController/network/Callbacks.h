#ifndef RADAR_NET_CALLBACKS_H
#define RADAR_NET_CALLBACKS_H

#include "Typedefs.h"
#include "Timestamp.h"
#include "Buffer.h"
#include <functional>
#include <memory>

#include <QByteArray>

namespace radar {

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

// should really belong to base/Types.h, but <memory> is not included there.

template<typename T>
inline T* get_pointer(const std::shared_ptr<T>& ptr)
{
  return ptr.get();
}

template<typename T>
inline T* get_pointer(const std::unique_ptr<T>& ptr)
{
  return ptr.get();
}

// Adapted from google-protobuf stubs/common.h
// see License in muduo/base/Types.h
template<typename To, typename From>
inline ::std::shared_ptr<To> down_pointer_cast(const ::std::shared_ptr<From>& f) {
  if (false)
  {
    net::implicit_cast<From*, To*>(0);
  }

#ifndef NDEBUG
  assert(f == NULL || dynamic_cast<To*>(get_pointer(f)) != NULL);
#endif
  return ::std::static_pointer_cast<To>(f);
}

namespace net {

class Connection;
typedef std::shared_ptr<Connection> ConnectionPtr;

// the data has been read to (buf, len)
typedef std::function<void (const ConnectionPtr&,
                            Buffer*, Timestamp)> ConnectionMessageCallback;

}
}

#endif // CALLBACKS_H

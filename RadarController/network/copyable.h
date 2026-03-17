#ifndef RADAR_BASE_COPYABLE_H
#define RADAR_BASE_COPYABLE_H

namespace radar
{

/// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type.
class copyable
{
 protected:
  copyable() = default;
  ~copyable() = default;
};

}  // namespace rmcom

#endif  // RMCOM_BASE_COPYABLE_H


#include <system_error>

#include "map_errno.h"

namespace map_errno {

std::system_error map_errno(int errno_) {
  return std::system_error(errno_, std::generic_category());
}

}  // namespace map_errno

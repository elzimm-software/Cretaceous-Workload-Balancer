#include "map_errno.h"

#include <system_error>

namespace map_errno {

std::system_error map_errno(int errno_) {
  return {errno_, std::generic_category()};
}

}  // namespace map_errno

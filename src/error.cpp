#include "error.h"
#include <system_error>

static std::system_error map_errno(int errno_) {
  return std::system_error(errno_, std::generic_category());
}

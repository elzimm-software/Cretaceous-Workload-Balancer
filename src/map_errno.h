#ifndef ERROR_H_
#define ERROR_H_

#include <system_error>

namespace map_errno {

std::system_error map_errno(int errno_);

}  // namespace map_errno

#endif  // ERROR_H_

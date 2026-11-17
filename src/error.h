#ifndef ERROR_H_
#define ERROR_H_

#include <system_error>

std::system_error map_errno(int errno_);

#endif // ERROR_H_

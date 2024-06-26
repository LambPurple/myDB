

#ifndef STORAGE_MYDB_PORT_PORT_H_
#define STORAGE_MYDB_PORT_PORT_H_

#include <string.h>

// Include the appropriate platform specific file below.  If you are
// porting to a new platform, see "port_example.h" for documentation
// of what the new port_<platform>.h file must provide.
#if defined(MYDB_PLATFORM_POSIX) || defined(MYDB_PLATFORM_WINDOWS)
#include "port/port_stdcxx.h"
#elif defined(MYDB_PLATFORM_CHROMIUM)
#include "port/port_chromium.h"
#endif

#endif // STORAGE_MYDB_PORT_PORT_H_

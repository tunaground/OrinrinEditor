```cpp
#pragma once

#define NTDDI_VERSION	0x05000000

#ifndef WINVER
#define WINVER	0x0500
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT	WINVER
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS	_WIN32_WINNT
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#endif

```
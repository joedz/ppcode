#pragma once
#ifdef __LINUX__          //简单封装  屏蔽linux和window的差异

#include "os/os_linux.h"

#else

#include "os/os_window.h"

#endif
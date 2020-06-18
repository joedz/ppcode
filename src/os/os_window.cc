#include "os_window.h"
#include <string.h>

#ifdef __WINDOW__

int strcasecmp(const char* s1, const char* s2) {
	return _stricmp(s1, s2);
}

#endif
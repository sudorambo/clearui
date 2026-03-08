/* Runtime version string from CUI_VERSION_* macros. */
#include "../../include/clearui.h"
#include <stdio.h>

const char *cui_version_string(void) {
	static char buf[32];
	snprintf(buf, sizeof(buf), "%d.%d.%d",
	         CUI_VERSION_MAJOR, CUI_VERSION_MINOR, CUI_VERSION_PATCH);
	return buf;
}

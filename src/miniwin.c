#include "miniwin.h"

#if defined(MWIN_BACKEND_X11) || defined(__linux__)

int mwin_init(struct MiniWin* win) {
	return 1;
}

void mwin_destroy(struct MiniWin* win) {
}

#elif defined(MWIN_BACKEND_WIN32) || defined(_WIN32)
#elif defined(MWIN_BACKEND_COCOA) || defined(__APPLE__)
#else
	#error "No Backend could be determined for miniwin"
#endif


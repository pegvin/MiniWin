#include <stdio.h>
#include "miniwin.h"

int main(void) {
	uint32_t fb[320 * 240] = {};
	struct MiniWin win = {
		.title = "HEllo",
		.width = 320,
		.height = 240,
		.frameBuf = fb
	};

	if (!mwin_init(&win)) {
		printf("[ERROR] mwin_init(...) - Failed\n");
		return 1;
	}

	while (mwin_poll(&win)) {
		printf("%d,%d\n", win.mX, win.mY);
	}

	mwin_destroy(&win);

	return 0;
}


#include <stdio.h>
#include "miniwin.h"

int main(void) {
	uint32_t fb[320 * 240] = {};
	struct MiniWin win = {
		.width = 320,
		.height = 240,
		.frameBuf = fb
	};

	if (!mwin_init(&win)) {
		printf("[ERROR] mwin_init(...) - Failed\n");
		return 1;
	}

	// while (1) {}

	mwin_destroy(&win);

	return 0;
}


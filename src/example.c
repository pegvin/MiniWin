#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "miniwin.h"

int main(void) {
	struct MiniWin win = {
		.title = "Hello",
		.width = 320,
		.height = 240
	};

	if (!mwin_init(&win)) {
		printf("[ERROR] mwin_init(...) - Failed\n");
		return 1;
	}

	bool isRunning = true;
	while (isRunning) {
		MW_Event evt = {0};
		while (mwin_poll(&win, &evt)) {
			switch (evt.type) {
				case MW_EVENT_KEYBOARD_KEY: {
					printf(
						"[%s] '%c'\n",
						evt.key.verb == MW_PRESS ? "KEY DOWN" : "KEY UP",
						evt.key.key
					);
					break;
				}
				case MW_EVENT_MOUSE_MOTION: {
					printf(
						"[MOUSE MOVE] %d,%d\n",
						evt.motion.x,
						evt.motion.y
					);
					break;
				}
				case MW_EVENT_MOUSE_BUTTON: {
					printf(
						"[%s] %s\n",
						evt.button.verb == MW_PRESS ? "MOUSE DOWN" : "MOUSE UP",
						evt.button.btn == MW_MOUSE_LEFT ? "Left" : (evt.button.btn == MW_MOUSE_RIGHT ? "Right" : "Middle")
					);
					break;
				}
				case MW_EVENT_WINDOW_RESIZE: {
					printf("[WINDOW RESIZE] %dx%d\n", evt.resize.width, evt.resize.height);
					break;
				}
				case MW_EVENT_WINDOW_CLOSE: {
					isRunning = false;
					break;
				}
				case MW_EVENT_NONE: break;
			}
		}

		for (uint32_t i = 0; i < win.width * win.height; i++) {
			win.pixels[i] = rand();
		}
		mwin_swap(&win);
	}

	mwin_destroy(&win);

	return 0;
}


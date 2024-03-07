#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "miniwin.h"

static void _Rect(struct MiniWin* win, unsigned int off_x, unsigned int off_y, int w, int h, uint32_t c) {
	for (unsigned int y = 0; y < h && (y + off_y) < win->height; y++) {
		for (unsigned int x = 0; x < w && (x + off_x) < win->width; x++) {
			unsigned int newX = x + off_x;
			unsigned int newY = y + off_y;
			win->pixels[(newY * win->width) + newX] = c;
		}
	}
}

static uint16_t font5x3[] = {0x0000,0x2092,0x002d,0x5f7d,0x279e,0x52a5,0x7ad6,0x0012,0x4494,0x1491,0x017a,0x05d0,0x1400,0x01c0,0x0400,0x12a4,0x2b6a,0x749a,0x752a,0x38a3,0x4f4a,0x38cf,0x3bce,0x12a7,0x3aae,0x49ae,0x0410,0x1410,0x4454,0x0e38,0x1511,0x10e3,0x73ee,0x5f7a,0x3beb,0x624e,0x3b6b,0x73cf,0x13cf,0x6b4e,0x5bed,0x7497,0x2b27,0x5add,0x7249,0x5b7d,0x5b6b,0x3b6e,0x12eb,0x4f6b,0x5aeb,0x388e,0x2497,0x6b6d,0x256d,0x5f6d,0x5aad,0x24ad,0x72a7,0x6496,0x4889,0x3493,0x002a,0xf000,0x0011,0x6b98,0x3b79,0x7270,0x7b74,0x6750,0x95d6,0xb9ee,0x5b59,0x6410,0xb482,0x56e8,0x6492,0x5be8,0x5b58,0x3b70,0x976a,0xcd6a,0x1370,0x38f0,0x64ba,0x3b68,0x2568,0x5f68,0x54a8,0xb9ad,0x73b8,0x64d6,0x2492,0x3593,0x03e0};

static void _Text(struct MiniWin* win, unsigned int x, unsigned int y, char* s, int scale, uint32_t c) {
	while (*s) {
		char chr = *s++;
		if (chr > 32) {
			uint16_t bmp = font5x3[chr - 32];
			for (int dy = 0; dy < 5; dy++) {
				for (int dx = 0; dx < 3; dx++) {
					if (bmp >> (dy * 3 + dx) & 1) {
						_Rect(win, x + dx * scale, y + dy * scale, scale, scale, c);
					}
				}
			}
		}
		x = x + 4 * scale;
	}
}

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

	char text[128] = "INITIALIZING...";

	bool isRunning = true;
	bool doReDraw = true;
	while (isRunning) {
		MW_Event evt = {0};
		while (mwin_poll(&win, &evt)) {
			switch (evt.type) {
				case MW_EVENT_KEYBOARD_KEY: {
					sprintf(
						text,
						"[%s] '%c'\n",
						evt.key.verb == MW_PRESS ? "KEY DOWN" : "KEY UP",
						evt.key.key
					);
					doReDraw = true;
					break;
				}
				case MW_EVENT_MOUSE_MOTION: {
					sprintf(
						text,
						"[MOUSE MOVE] %d,%d\n",
						evt.motion.x,
						evt.motion.y
					);
					doReDraw = true;
					break;
				}
				case MW_EVENT_MOUSE_BUTTON: {
					sprintf(
						text,
						"[%s] %s\n",
						evt.button.verb == MW_PRESS ? "MOUSE DOWN" : "MOUSE UP",
						evt.button.btn == MW_MOUSE_LEFT ? "Left" : (evt.button.btn == MW_MOUSE_RIGHT ? "Right" : "Middle")
					);
					doReDraw = true;
					break;
				}
				case MW_EVENT_WINDOW_RESIZE: {
					sprintf(text, "[WINDOW RESIZE] %dx%d\n", evt.resize.width, evt.resize.height);
					doReDraw = true;
					break;
				}
				case MW_EVENT_WINDOW_CLOSE: {
					isRunning = false;
					break;
				}
				case MW_EVENT_NONE: break;
			}
		}

		if (doReDraw) {
			int scale = 4;
			memset(win.pixels, 0, win.width * win.height * sizeof(*win.pixels));
			_Text(&win, 10, 10, text, scale, 0xFFFFFFFF);
			mwin_swap(&win);
			doReDraw = false;
		}
	}

	mwin_destroy(&win);

	return 0;
}


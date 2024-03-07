#include "miniwin.h"

#if defined(MWIN_BACKEND_X11) || defined(__linux__)

#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

struct BackEndData {
	Display* display;
	Window   xWin;
	GC       gCtx;
	XImage*  img;
};

int mwin_init(struct MiniWin* win) {
	struct BackEndData* data = malloc(sizeof(struct BackEndData));
	data->display = XOpenDisplay(NULL);
	int screen = DefaultScreen(data->display);
	data->xWin = XCreateSimpleWindow(
		data->display, RootWindow(data->display, screen),
		0, 0, win->width, win->height,
		0, BlackPixel(data->display, screen), WhitePixel(data->display, screen)
	);
	data->gCtx = XCreateGC(data->display, data->xWin, 0, 0);

	XSelectInput(
		data->display, data->xWin,
		ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask
	);
	XStoreName(data->display, data->xWin, win->title);
	XMapWindow(data->display, data->xWin);
	XSync(data->display, data->xWin);

	data->img = XCreateImage(
		data->display, DefaultVisual(data->display, 0),
		24, ZPixmap, 0, (char *)win->frameBuf, win->width, win->height, 32, 0
	);

	win->backendData = data;

	return 1;
}

void mwin_destroy(struct MiniWin* win) {
	struct BackEndData* data = (struct BackEndData*)win->backendData;
	XCloseDisplay(data->display);

	free(win->backendData);
	win->backendData = NULL;
}

static int KEYCODES_G[124] = {XK_BackSpace,8,XK_Delete,127,XK_Down,18,XK_End,5,XK_Escape,27,XK_Home,2,XK_Insert,26,XK_Left,20,XK_Page_Down,4,XK_Page_Up,3,XK_Return,10,XK_Right,19,XK_Tab,9,XK_Up,17,XK_apostrophe,39,XK_backslash,92,XK_bracketleft,91,XK_bracketright,93,XK_comma,44,XK_equal,61,XK_grave,96,XK_minus,45,XK_period,46,XK_semicolon,59,XK_slash,47,XK_space,32,XK_a,65,XK_b,66,XK_c,67,XK_d,68,XK_e,69,XK_f,70,XK_g,71,XK_h,72,XK_i,73,XK_j,74,XK_k,75,XK_l,76,XK_m,77,XK_n,78,XK_o,79,XK_p,80,XK_q,81,XK_r,82,XK_s,83,XK_t,84,XK_u,85,XK_v,86,XK_w,87,XK_x,88,XK_y,89,XK_z,90,XK_0,48,XK_1,49,XK_2,50,XK_3,51,XK_4,52,XK_5,53,XK_6,54,XK_7,55,XK_8,56,XK_9,57};

void mwin_swap(const struct MiniWin* win) {
	struct BackEndData* data = (struct BackEndData*)win->backendData;

	XPutImage(data->display, data->xWin, data->gCtx, data->img, 0, 0, 0, 0, win->width, win->height);
	XFlush(data->display);
}

int mwin_poll(struct MiniWin* win) {
	struct BackEndData* data = (struct BackEndData*)win->backendData;

	XEvent ev;
	while (XPending(data->display)) {
		XNextEvent(data->display, &ev);
		switch (ev.type) {
			case ButtonPress:
			case ButtonRelease: {
				win->isLmbDown = (ev.type == ButtonPress);
				break;
			}
			case MotionNotify: {
				win->mX = ev.xmotion.x;
				win->mY = ev.xmotion.y;
				break;
			}
			case KeyPress:
			case KeyRelease: {
				int m = ev.xkey.state;
				int k = XkbKeycodeToKeysym(data->display, ev.xkey.keycode, 0, 0);
				for (unsigned int i = 0; i < 124; i += 2) {
					if (KEYCODES_G[i] == k) {
						win->keys[KEYCODES_G[i + 1]] = (ev.type == KeyPress);
						break;
					}
				}
				win->mod = (!!(m & ControlMask)) | (!!(m & ShiftMask) << 1) | (!!(m & Mod1Mask) << 2) | (!!(m & Mod4Mask) << 3);
				break;
			}
		}
	}

	return 1;
}

#else
	#error "No Backend could be determined for miniwin"
#endif


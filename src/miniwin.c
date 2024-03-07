#include "miniwin.h"

#if defined(MWIN_BACKEND_X11) || defined(__linux__)

#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

struct BackEndData {
	Display* display;
	Window   xWin;
	GC       gCtx;
	XImage*  img;
	Atom     wmDelete;
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
		ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
		StructureNotifyMask
	);
	XStoreName(data->display, data->xWin, win->title);
	XMapWindow(data->display, data->xWin);
	XSync(data->display, data->xWin);

	win->pixels = calloc(win->width * win->height * sizeof(*win->pixels), 1);
	data->img = XCreateImage(
		data->display, DefaultVisual(data->display, 0),
		24, ZPixmap, 0, (char *)win->pixels, win->width, win->height, 32, 0
	);

	data->wmDelete = XInternAtom(data->display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(data->display, data->xWin, &data->wmDelete, 1);

	win->backendData = data;

	return 1;
}

void mwin_destroy(struct MiniWin* win) {
	struct BackEndData* data = (struct BackEndData*)win->backendData;
	XCloseDisplay(data->display);

	free(win->backendData);
	win->backendData = NULL;

	free(win->pixels);
	win->pixels = NULL;
}

static int KEYCODES_G[124] = {XK_BackSpace,8,XK_Delete,127,XK_Down,18,XK_End,5,XK_Escape,27,XK_Home,2,XK_Insert,26,XK_Left,20,XK_Page_Down,4,XK_Page_Up,3,XK_Return,10,XK_Right,19,XK_Tab,9,XK_Up,17,XK_apostrophe,39,XK_backslash,92,XK_bracketleft,91,XK_bracketright,93,XK_comma,44,XK_equal,61,XK_grave,96,XK_minus,45,XK_period,46,XK_semicolon,59,XK_slash,47,XK_space,32,XK_a,65,XK_b,66,XK_c,67,XK_d,68,XK_e,69,XK_f,70,XK_g,71,XK_h,72,XK_i,73,XK_j,74,XK_k,75,XK_l,76,XK_m,77,XK_n,78,XK_o,79,XK_p,80,XK_q,81,XK_r,82,XK_s,83,XK_t,84,XK_u,85,XK_v,86,XK_w,87,XK_x,88,XK_y,89,XK_z,90,XK_0,48,XK_1,49,XK_2,50,XK_3,51,XK_4,52,XK_5,53,XK_6,54,XK_7,55,XK_8,56,XK_9,57};

void mwin_swap(const struct MiniWin* win) {
	struct BackEndData* data = (struct BackEndData*)win->backendData;

	XPutImage(data->display, data->xWin, data->gCtx, data->img, 0, 0, 0, 0, win->width, win->height);
	XFlush(data->display);
}

int mwin_poll(struct MiniWin* win, MW_Event* evt) {
	struct BackEndData* data = (struct BackEndData*)win->backendData;

	int pending = XPending(data->display);
	if (pending < 1) {
		return 0;
	}

	evt->type = MW_EVENT_NONE;

	XEvent ev;
	XNextEvent(data->display, &ev);
	switch (ev.type) {
		case ButtonPress:
		case ButtonRelease: {
			if (ev.xbutton.button > Button3) {
				break;
			}

			evt->type =	MW_EVENT_MOUSE_BUTTON;
			// http://xahlee.info/linux/linux_x11_mouse_button_number.html
			switch (ev.xbutton.button) {
				case Button1: {
					evt->button.btn = MW_MOUSE_LEFT;
					break;
				}
				case Button2: {
					evt->button.btn = MW_MOUSE_MIDDLE;
					break;
				}
				case Button3: {
					evt->button.btn = MW_MOUSE_RIGHT;
					break;
				}
			}
			evt->button.verb = ev.type == ButtonPress ? MW_PRESS : MW_RELEASE;
			break;
		}
		case MotionNotify: {
			evt->type = MW_EVENT_MOUSE_MOTION;
			evt->motion.x = ev.xmotion.x;
			evt->motion.y = ev.xmotion.y;
			break;
		}
		case KeyPress:
		case KeyRelease: {
			evt->type = MW_EVENT_KEYBOARD_KEY;
			evt->key.verb = ev.type == KeyPress ? MW_PRESS : MW_RELEASE;
			int m = ev.xkey.state;
			int k = XkbKeycodeToKeysym(data->display, ev.xkey.keycode, 0, 0);
			for (unsigned int i = 0; i < 124; i += 2) {
				if (KEYCODES_G[i] == k) {
					evt->key.key = KEYCODES_G[i + 1];
					break;
				}
			}
			evt->key.mod = (!!(m & ControlMask)) | (!!(m & ShiftMask) << 1) | (!!(m & Mod1Mask) << 2) | (!!(m & Mod4Mask) << 3);
			break;
		}
		case ConfigureNotify: {
			if (ev.xconfigure.width == win->width && ev.xconfigure.height == win->height) {
				break;
			}

			evt->type = MW_EVENT_WINDOW_RESIZE;
			evt->resize.width = ev.xconfigure.width;
			evt->resize.height = ev.xconfigure.height;
			win->width = evt->resize.width;
			win->height = evt->resize.height;

			XDestroyImage(data->img);
			win->pixels = calloc(win->width * win->height * sizeof(*win->pixels), 1);
			data->img = XCreateImage(
				data->display, DefaultVisual(data->display, 0),
				24, ZPixmap, 0, (char *)win->pixels, win->width, win->height, 32, 0
			);

			break;
		}
		case ClientMessage: {
			if (ev.xclient.data.l[0] == data->wmDelete) {
				evt->type = MW_EVENT_WINDOW_CLOSE;
			}
			break;
		}
	}

	return pending;
}

#else
	#error "No Backend could be determined for miniwin"
#endif


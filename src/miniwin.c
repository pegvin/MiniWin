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

static inline MW_KeyboardKey XKeySym_To_MWKey(KeySym k) {
	switch (k) {
	case XK_a: return MW_KEY_a;
	case XK_b: return MW_KEY_b;
	case XK_c: return MW_KEY_c;
	case XK_d: return MW_KEY_d;
	case XK_e: return MW_KEY_e;
	case XK_f: return MW_KEY_f;
	case XK_g: return MW_KEY_g;
	case XK_h: return MW_KEY_h;
	case XK_i: return MW_KEY_i;
	case XK_j: return MW_KEY_j;
	case XK_k: return MW_KEY_k;
	case XK_l: return MW_KEY_l;
	case XK_m: return MW_KEY_m;
	case XK_n: return MW_KEY_n;
	case XK_o: return MW_KEY_o;
	case XK_p: return MW_KEY_p;
	case XK_q: return MW_KEY_q;
	case XK_r: return MW_KEY_r;
	case XK_s: return MW_KEY_s;
	case XK_t: return MW_KEY_t;
	case XK_u: return MW_KEY_u;
	case XK_v: return MW_KEY_v;
	case XK_w: return MW_KEY_w;
	case XK_x: return MW_KEY_x;
	case XK_y: return MW_KEY_y;
	case XK_z: return MW_KEY_z;

	case XK_0: return MW_KEY_0;
	case XK_1: return MW_KEY_1;
	case XK_2: return MW_KEY_2;
	case XK_3: return MW_KEY_3;
	case XK_4: return MW_KEY_4;
	case XK_5: return MW_KEY_5;
	case XK_6: return MW_KEY_6;
	case XK_7: return MW_KEY_7;
	case XK_8: return MW_KEY_8;
	case XK_9: return MW_KEY_9;

	case XK_space: return MW_KEY_SPACE;
	case XK_apostrophe: return MW_KEY_APOSTROPHE;
	case XK_grave: return MW_KEY_GRAVE_ACCENT;
	case XK_comma: return MW_KEY_COMMA;
	case XK_minus: return MW_KEY_MINUS;
	case XK_period: return MW_KEY_PERIOD;
	case XK_semicolon: return MW_KEY_SEMICOLON;
	case XK_equal: return MW_KEY_EQUAL;

	case XK_bracketleft: return MW_KEY_LEFT_BRACKET;
	case XK_bracketright: return MW_KEY_RIGHT_BRACKET;

	case XK_slash: return MW_KEY_SLASH;
	case XK_backslash: return MW_KEY_BACKSLASH;

	case XK_Escape: return MW_KEY_ESCAPE;
	case XK_Return: return MW_KEY_ENTER;
	case XK_Tab: return MW_KEY_TAB;
	case XK_BackSpace: return MW_KEY_BACKSPACE;
	case XK_Insert: return MW_KEY_INSERT;
	case XK_Delete: return MW_KEY_DELETE;
	case XK_Right: return MW_KEY_RIGHT;
	case XK_Left: return MW_KEY_LEFT;
	case XK_Down: return MW_KEY_DOWN;
	case XK_Up: return MW_KEY_UP;
	case XK_Page_Up: return MW_KEY_PAGE_UP;
	case XK_Page_Down: return MW_KEY_PAGE_DOWN;
	case XK_Home: return MW_KEY_HOME;
	case XK_End: return MW_KEY_END;

	default: return MW_KEY_UNKNOWN;
	}
}

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
			int mod = ev.xkey.state;
			int key = XkbKeycodeToKeysym(data->display, ev.xkey.keycode, 0, 0);
			evt->key.key = XKeySym_To_MWKey(key);

			evt->key.mod = MW_KEYMOD_NONE;
			if (mod & ShiftMask)   evt->key.mod |= MW_KEYMOD_SHIFT;
			if (mod & ControlMask) evt->key.mod |= MW_KEYMOD_CTRL;
			if (mod & LockMask)    evt->key.mod |= MW_KEYMOD_CAPS_LOCK;
			if (mod & Mod1Mask)    evt->key.mod |= MW_KEYMOD_ALT;
			if (mod & Mod2Mask)    evt->key.mod |= MW_KEYMOD_NUM_LOCK;
			if (mod & Mod3Mask)    evt->key.mod |= MW_KEYMOD_SCROLL_LOCK;
			if (mod & Mod4Mask)    evt->key.mod |= MW_KEYMOD_SUPER;
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


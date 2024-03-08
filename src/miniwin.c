#include "miniwin.h"

#if defined(MWIN_BACKEND_X11) || defined(__linux__)

#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

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

	// Synchronize to make sure all commands are processed
	XSync(data->display, data->xWin);

	win->pixels = calloc(win->width * win->height * sizeof(*win->pixels), 1);
	data->img = XCreateImage(
		data->display, DefaultVisual(data->display, 0),
		24, ZPixmap, 0, (char *)win->pixels, win->width, win->height, 32, 0
	);

	data->wmDelete = XInternAtom(data->display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(data->display, data->xWin, &data->wmDelete, 1);

	// Disable Compositor For This App
	Atom NET_WM_BYPASS_COMPOSITOR = XInternAtom(data->display, "_NET_WM_BYPASS_COMPOSITOR", False);
	const unsigned long value = 1;
	XChangeProperty(
		data->display, data->xWin,
		NET_WM_BYPASS_COMPOSITOR,
		XA_CARDINAL, 32, PropModeReplace,
		(unsigned char*) &value, 1
	);

	win->backendData = data;

	return 1;
}

void mwin_destroy(struct MiniWin* win) {
	struct BackEndData* data = (struct BackEndData*)win->backendData;

	XDestroyImage(data->img);
	data->img = NULL;
	win->pixels = NULL;

	XFreeGC(data->display, data->gCtx);
	data->gCtx = NULL;

	XUnmapWindow(data->display, data->xWin);
	XDestroyWindow(data->display, data->xWin);

	XCloseDisplay(data->display);
	data->display = NULL;

	free(win->backendData);
	win->backendData = NULL;
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

#elif defined(MWIN_BACKEND_WIN32) || defined(_WIN32) || defined(_WIN64)

#include <windows.h>

typedef struct {
	HWND hwnd;
} BackendData;

typedef struct BINFO {
	BITMAPINFOHEADER    bmiHeader;
	RGBQUAD             bmiColors[3];
} BINFO;

static LRESULT CALLBACK _MW_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CLOSE: {
			DestroyWindow(hwnd);
			break;
		}
		case WM_SIZE: {
			struct MiniWin* win = (struct MiniWin*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (LOWORD(lParam) == win->width && HIWORD(lParam) == win->height) {
				break;
			}

			win->width = LOWORD(lParam);
			win->height = HIWORD(lParam);

			win->pixels = realloc(win->pixels, win->width * win->height * sizeof(*win->pixels));
			memset(win->pixels, 0, win->width * win->height * sizeof(*win->pixels));
			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		default: {
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}

	return 0;
}

int mwin_init(struct MiniWin* win) {
	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASSEX wc = {0};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = _MW_WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = win->title;
	RegisterClassEx(&wc);

	BackendData* data = malloc(sizeof(BackendData));
	data->hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		win->title, win->title,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		win->width, win->height, NULL, NULL, hInstance, NULL
	);

	if (data->hwnd == NULL)
		return 0;

	win->pixels = calloc(win->width * win->height * sizeof(*win->pixels), 1);
	win->backendData = data;

	SetWindowLongPtr(data->hwnd, GWLP_USERDATA, (LONG_PTR)win);
	ShowWindow(data->hwnd, SW_NORMAL);
	UpdateWindow(data->hwnd);
	return 1;
}

static inline MW_KeyboardKey VK_Key_To_MW_Key(WPARAM k) {
	switch (k) {
	case 0x41: return MW_KEY_a;
	case 0x42: return MW_KEY_b;
	case 0x43: return MW_KEY_c;
	case 0x44: return MW_KEY_d;
	case 0x45: return MW_KEY_e;
	case 0x46: return MW_KEY_f;
	case 0x47: return MW_KEY_g;
	case 0x48: return MW_KEY_h;
	case 0x49: return MW_KEY_i;
	case 0x4A: return MW_KEY_j;
	case 0x4B: return MW_KEY_k;
	case 0x4C: return MW_KEY_l;
	case 0x4D: return MW_KEY_m;
	case 0x4E: return MW_KEY_n;
	case 0x4F: return MW_KEY_o;
	case 0x50: return MW_KEY_p;
	case 0x51: return MW_KEY_q;
	case 0x52: return MW_KEY_r;
	case 0x53: return MW_KEY_s;
	case 0x54: return MW_KEY_t;
	case 0x55: return MW_KEY_u;
	case 0x56: return MW_KEY_v;
	case 0x57: return MW_KEY_w;
	case 0x58: return MW_KEY_x;
	case 0x59: return MW_KEY_y;
	case 0x5A: return MW_KEY_z;

	case 0x30: return MW_KEY_0;
	case 0x31: return MW_KEY_1;
	case 0x32: return MW_KEY_2;
	case 0x33: return MW_KEY_3;
	case 0x34: return MW_KEY_4;
	case 0x35: return MW_KEY_5;
	case 0x36: return MW_KEY_6;
	case 0x37: return MW_KEY_7;
	case 0x38: return MW_KEY_8;
	case 0x39: return MW_KEY_9;

	case VK_SPACE: return MW_KEY_SPACE;
	case VK_OEM_7: return MW_KEY_APOSTROPHE;
	case VK_OEM_3: return MW_KEY_GRAVE_ACCENT;
	case VK_OEM_COMMA: return MW_KEY_COMMA;
	case VK_OEM_MINUS: return MW_KEY_MINUS;
	case VK_OEM_PERIOD: return MW_KEY_PERIOD;
	case VK_OEM_1: return MW_KEY_SEMICOLON;
	case VK_OEM_PLUS: return MW_KEY_EQUAL;

	case VK_OEM_4: return MW_KEY_LEFT_BRACKET;
	case VK_OEM_6: return MW_KEY_RIGHT_BRACKET;

	case VK_OEM_2: return MW_KEY_SLASH;
	case VK_OEM_5: return MW_KEY_BACKSLASH;

	case VK_RETURN: return MW_KEY_ENTER;
	case VK_TAB: return MW_KEY_TAB;

	case VK_ESCAPE: return MW_KEY_ESCAPE;
	case VK_BACK: return MW_KEY_BACKSPACE;
	case VK_INSERT: return MW_KEY_INSERT;
	case VK_DELETE: return MW_KEY_DELETE;
	case VK_RIGHT: return MW_KEY_RIGHT;
	case VK_LEFT: return MW_KEY_LEFT;
	case VK_DOWN: return MW_KEY_DOWN;
	case VK_UP: return MW_KEY_UP;
	case VK_PRIOR: return MW_KEY_PAGE_UP;
	case VK_NEXT: return MW_KEY_PAGE_DOWN;
	case VK_HOME: return MW_KEY_HOME;
	case VK_END: return MW_KEY_END;

	default: return MW_KEY_UNKNOWN;
	}
}

int mwin_poll(struct MiniWin* win, MW_Event* evt) {
	evt->type = MW_EVENT_NONE;

	MSG msg;
	int pending;
	if ((pending = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) == 0) {
		return 0;
	}

	BackendData* data = (BackendData*)win->backendData;

	TranslateMessage(&msg);

	switch (msg.message) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(data->hwnd, &ps);

			HDC memdc = CreateCompatibleDC(hdc);
			HBITMAP hbmp = CreateCompatibleBitmap(hdc, win->width, win->height);
			HBITMAP oldbmp = SelectObject(memdc, hbmp);

			BINFO bi = {0};
			bi.bmiHeader.biSize = sizeof(bi);
			bi.bmiHeader.biWidth = win->width;
			bi.bmiHeader.biHeight = -(win->height); /* For uncompressed RGB bitmaps, if biHeight is positive, the bitmap is a bottom-up DIB with the origin at the lower left corner. If biHeight is negative, the bitmap is a top-down DIB with the origin at the upper left corner. (https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader) */
			bi.bmiHeader.biPlanes = 1;
			bi.bmiHeader.biBitCount = 32;
			bi.bmiHeader.biCompression = BI_BITFIELDS;

			bi.bmiColors[0].rgbRed = 0xFF;
			bi.bmiColors[1].rgbGreen = 0xFF;
			bi.bmiColors[2].rgbBlue = 0xFF;

			SetDIBitsToDevice(
				memdc, 0, 0, win->width, win->height, 0, 0,
				0, win->height, win->pixels, (BITMAPINFO*)&bi, DIB_RGB_COLORS
			);
			BitBlt(hdc, 0, 0, win->width, win->height, memdc, 0, 0, SRCCOPY);
			SelectObject(memdc, oldbmp);
			DeleteObject(hbmp);
			DeleteDC(memdc);

			EndPaint(data->hwnd, &ps);
			break;
		}
		case WM_MOUSEMOVE: {
			evt->type = MW_EVENT_MOUSE_MOTION;
			evt->motion.x = LOWORD(msg.lParam);
			evt->motion.y = HIWORD(msg.lParam);
			break;
		}
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP: {
			evt->type = MW_EVENT_MOUSE_BUTTON;
			evt->button.verb = MW_RELEASE;
			evt->button.btn = msg.message == WM_LBUTTONUP ? MW_MOUSE_LEFT : (msg.message == WM_RBUTTONUP ? MW_MOUSE_RIGHT : MW_MOUSE_MIDDLE);
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN: {
			evt->type = MW_EVENT_MOUSE_BUTTON;
			evt->button.verb = MW_PRESS;
			evt->button.btn = msg.message == WM_LBUTTONDOWN ? MW_MOUSE_LEFT : (msg.message == WM_RBUTTONDOWN ? MW_MOUSE_RIGHT : MW_MOUSE_MIDDLE);
			break;
		}
		case WM_KEYDOWN:
		case WM_KEYUP: {
			WPARAM vkCode = msg.wParam;

			evt->type = MW_EVENT_KEYBOARD_KEY;
			evt->key.verb = msg.message == WM_KEYDOWN ? MW_PRESS : MW_RELEASE;
			evt->key.key = VK_Key_To_MW_Key(vkCode);

			evt->key.mod = MW_KEYMOD_NONE;

			if (GetKeyState(VK_SHIFT) & 0x8000) evt->key.mod |= MW_KEYMOD_SHIFT;
			if (GetKeyState(VK_CONTROL) & 0x8000) evt->key.mod |= MW_KEYMOD_CTRL;
			if (GetKeyState(VK_MENU) & 0x8000) evt->key.mod |= MW_KEYMOD_ALT;
			if (GetKeyState(VK_LWIN) & 0x8000 || GetKeyState(VK_RWIN) & 0x8000) evt->key.mod |= MW_KEYMOD_SUPER;

			if (GetKeyState(VK_CAPITAL) & 0x0001) evt->key.mod |= MW_KEYMOD_CAPS_LOCK;
			if (GetKeyState(VK_NUMLOCK) & 0x0001) evt->key.mod |= MW_KEYMOD_NUM_LOCK;
			if (GetKeyState(VK_SCROLL) & 0x0001) evt->key.mod |= MW_KEYMOD_SCROLL_LOCK;
			break;
		}
		case WM_QUIT: {
			evt->type = MW_EVENT_WINDOW_CLOSE;
			break;
		}
		default: {
			int oldWidth = win->width, oldHeight = win->height;

			DispatchMessage(&msg);

			if (oldWidth != win->width || oldHeight != win->height) {
				evt->type = MW_EVENT_WINDOW_RESIZE;
				evt->resize.width = win->width;
				evt->resize.height = win->height;
			}
			break;
		}
	}


	return pending;
}

void mwin_swap(const struct MiniWin* win) {
	BackendData* data = (BackendData*)win->backendData;
	InvalidateRect(data->hwnd, NULL, TRUE);
}

void mwin_destroy(struct MiniWin* win) {
	free(win->backendData);
	win->backendData = NULL;

	free(win->pixels);
	win->pixels = NULL;
}

#else
	#error "No Backend could be determined for miniwin"
#endif


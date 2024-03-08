#ifndef MINIWIN_H_INCLUDED_
#define MINIWIN_H_INCLUDED_ 1
#pragma once

#include <stdint.h>

struct MiniWin {
	const char* title;
	int width;
	int height;

	uint32_t* pixels;  // MiniWin Handles the allocation/resizing/freeing of the pixel buffer, you just need to write to it
	void* backendData; // Backend Specific Data, To be ONLY touched by MiniWin functions
};

typedef enum {
	MW_PRESS,
	MW_RELEASE
} MW_Verb;

typedef enum {
	MW_EVENT_NONE,
	MW_EVENT_KEYBOARD_KEY,

	MW_EVENT_MOUSE_MOTION,
	MW_EVENT_MOUSE_BUTTON,

	MW_EVENT_WINDOW_RESIZE,
	MW_EVENT_WINDOW_CLOSE
} MW_EventType;

typedef enum {
	MW_MOUSE_LEFT,
	MW_MOUSE_MIDDLE,
	MW_MOUSE_RIGHT
} MW_MouseButton;

typedef enum {
	MW_KEYMOD_NONE        = 0x0000,

	MW_KEYMOD_SHIFT       = 0x0001,
	MW_KEYMOD_CTRL        = 0x0002,
	MW_KEYMOD_ALT         = 0x0004,
	MW_KEYMOD_SUPER       = 0x0008, // "Windows Key"
	MW_KEYMOD_CAPS_LOCK   = 0x0010,
	MW_KEYMOD_NUM_LOCK    = 0x0020,
	MW_KEYMOD_SCROLL_LOCK = 0x0040,

	MW_KEYMOD_COUNT
} MW_KeyMod;

typedef enum {
	MW_KEY_a = 'a',
	MW_KEY_b = 'b',
	MW_KEY_c = 'c',
	MW_KEY_d = 'd',
	MW_KEY_e = 'e',
	MW_KEY_f = 'f',
	MW_KEY_g = 'g',
	MW_KEY_h = 'h',
	MW_KEY_i = 'i',
	MW_KEY_j = 'j',
	MW_KEY_k = 'k',
	MW_KEY_l = 'l',
	MW_KEY_m = 'm',
	MW_KEY_n = 'n',
	MW_KEY_o = 'o',
	MW_KEY_p = 'p',
	MW_KEY_q = 'q',
	MW_KEY_r = 'r',
	MW_KEY_s = 's',
	MW_KEY_t = 't',
	MW_KEY_u = 'u',
	MW_KEY_v = 'v',
	MW_KEY_w = 'w',
	MW_KEY_x = 'x',
	MW_KEY_y = 'y',
	MW_KEY_z = 'z',
	MW_KEY_0 = '0',
	MW_KEY_1 = '1',
	MW_KEY_2 = '2',
	MW_KEY_3 = '3',
	MW_KEY_4 = '4',
	MW_KEY_5 = '5',
	MW_KEY_6 = '6',
	MW_KEY_7 = '7',
	MW_KEY_8 = '8',
	MW_KEY_9 = '9',

	MW_KEY_SPACE = ' ',
	MW_KEY_APOSTROPHE = '\'',
	MW_KEY_GRAVE_ACCENT = '`',
	MW_KEY_COMMA = ',',
	MW_KEY_MINUS = '-',
	MW_KEY_PERIOD = '.',
	MW_KEY_SEMICOLON = ';',
	MW_KEY_EQUAL = '=',

	MW_KEY_LEFT_BRACKET = '[',
	MW_KEY_RIGHT_BRACKET = ']',

	MW_KEY_SLASH = '/',
	MW_KEY_BACKSLASH = '\\',

	MW_KEY_ENTER = '\n',
	MW_KEY_TAB = '\t',

	MW_KEY_ESCAPE = 256,
	MW_KEY_BACKSPACE,
	MW_KEY_INSERT,
	MW_KEY_DELETE,
	MW_KEY_RIGHT,
	MW_KEY_LEFT,
	MW_KEY_DOWN,
	MW_KEY_UP,
	MW_KEY_PAGE_UP,
	MW_KEY_PAGE_DOWN,
	MW_KEY_HOME,
	MW_KEY_END,

	MW_KEY_COUNT,
	MW_KEY_UNKNOWN
} MW_KeyboardKey;

typedef	struct {
	MW_EventType type;
	MW_KeyboardKey key;
	MW_KeyMod mod;
	MW_Verb verb;
} MW_EventKeyboard;

typedef struct {
	MW_EventType type;
	int x, y;
} MW_EventMouseMotion;

typedef struct {
	MW_EventType type;
	MW_Verb verb;
	MW_MouseButton btn;
} MW_EventMouseButton;

typedef struct {
	MW_EventType type;
	int width, height;
} MW_EventWindowResize;

typedef union {
	MW_EventType type;
	MW_EventKeyboard key;
	MW_EventMouseMotion motion;
	MW_EventMouseButton button;
	MW_EventWindowResize resize;
} MW_Event;

// Returns '1' on success
int mwin_init(struct MiniWin* win);

// Returns 1 if more events are available, or 0 is no events are available
int mwin_poll(struct MiniWin* win, MW_Event* evt);

void mwin_swap(const struct MiniWin* win);

// Must be called after MW_EVENT_WINDOW_CLOSE occurs
void mwin_destroy(struct MiniWin* win);

#endif // !MINIWIN_H_INCLUDED_


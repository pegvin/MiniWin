#ifndef MINIWIN_H_INCLUDED_
#define MINIWIN_H_INCLUDED_ 1
#pragma once

#include <stdint.h>

struct MiniWin {
	const char* title;
	int width;
	int height;
	uint32_t* frameBuf;

	void* backendData; // Backend Specific Data, To be ONLY touched by MiniWin functions
};

typedef enum {
	MW_KEYMOD_CTRL = 1,
	MW_KEYMOD_SHIFT = 2,
	MW_KEYMOD_ALT = 4,
	MW_KEYMOD_META = 8
} MW_KeyMod;

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

typedef	struct {
	MW_EventType type;
	int key; // Key in ASCII, but arrows 17..20
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

// Returns number of pending events
int mwin_poll(struct MiniWin* win, MW_Event* evt);

void mwin_swap(const struct MiniWin* win);

void mwin_destroy(struct MiniWin* win);

#endif // !MINIWIN_H_INCLUDED_


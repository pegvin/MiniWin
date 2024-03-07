#ifndef MINIWIN_H_INCLUDED_
#define MINIWIN_H_INCLUDED_ 1
#pragma once

#include <stdint.h>

struct MiniWin {
	const char* title;
	const int width;
	const int height;
	uint32_t* frameBuf;

	int isLmbDown;
	int mX, mY; // Mouse Position x, y
	int keys[256]; // Keys in ASCII but arrows 17..20
	int mod; // mod is 4 bits mask, ctrl=1, shift=2, alt=4, meta=8

	void* backendData; // Backend Specific Data, To be ONLY touched by MiniWin functions
};

// Returns '1' on success
int mwin_init(struct MiniWin* win);
int mwin_poll(struct MiniWin* win);
void mwin_swap(const struct MiniWin* win);

void mwin_destroy(struct MiniWin* win);

#endif // !MINIWIN_H_INCLUDED_

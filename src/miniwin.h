#ifndef MINIWIN_H_INCLUDED_
#define MINIWIN_H_INCLUDED_ 1
#pragma once

#include <stdint.h>

struct MiniWin {
	const int width;
	const int height;
	uint32_t* frameBuf;
};

// Returns '1' on success
int mwin_init(struct MiniWin* win);
void mwin_destroy(struct MiniWin* win);

#endif // !MINIWIN_H_INCLUDED_

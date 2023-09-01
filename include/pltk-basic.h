#pragma once
#include <pltk-core.h>

typedef enum pltkitype {
	PLTK_NONE,
	PLTK_KEYBOARD,
	PLTK_POINTER_ABS,
	PLTK_POINTER_REL
} pltkitype_t;

typedef struct pltkbasicptr {
	struct timespec time;
	unsigned int buttonPressed;
	uint16_t x;
	uint16_t y;
} pltkbasicptr_t;

typedef pltkevent_t pltkbasickb_t;

void plTKBasicInit(pltkitype_t pointerType, uint16_t width, uint16_t height, bool disableCursorDraw);
void plTKBasicStop();
void plTKBasicUpdate(bool updateWindow);

void plTKBasicWindowMove(uint16_t x, uint16_t y);
void plTKBasicWindowPixel(uint16_t x, uint16_t y, pltkcolor_t color);
void plTKBasicWindowLine(uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkcolor_t color);
void plTKBasicWindowFBWrite(uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkdata_t* data);
void plTKBasicWindowRenderFont(uint16_t x, uint16_t y, pltkfont_t font, uint32_t index, pltkcolor_t color);

pltkbasickb_t plTKBasicGetKBInput();
pltkbasicptr_t plTKBasicGetPtrState();

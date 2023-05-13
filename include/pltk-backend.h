#include <pl32-memory.h>
#include <pl32-file.h>

typedef struct pltkwindow pltkwindow_t;
typedef struct pltkcolor {
	uint8_t bytes[4]; // Not all bytes might be used. bytes[0] will be used. but
} pltkcolor_t;
typedef struct pltkdata {
	plfatptr_t dataPtr;
	uint8_t bytesPerPixel;
	bool ignoreZero;
} pltkdata_t;
typedef struct pltkfont {
	pltkdata_t* data;
	uint16_t fontSize[2];
} pltkfont_t;

void plTKInit();
pltkwindow_t* plTKCreateWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void plTKWindowRender(pltkwindow_t* window);
void plTKWindowMove(pltkwindow_t* window, uint16_t x, uint16_t y);
void plTKWindowLine(pltkwindow_t* window, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkcolor_t color);
void plTKWindowPixel(pltkwindow_t* window, uint16_t x, uint16_t y, pltkcolor_t color);
void plTKWindowFBWrite(pltkwindow_t* window, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkdata_t* data);
void plTKWindowRenderFont(pltkwindow_t* window, pltkfont_t font, plstring_t string);
void plTKWindowClose(pltkwindow_t* window);
void plTKStop();

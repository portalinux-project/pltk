#include <pl32-memory.h>
#include <pl32-file.h>

typedef struct pltkwindow pltkwindow_t;
typedef struct pltkcolor {
	uint8_t bytes[4]; // Not all bytes might be used. bytes[0] will be used. but
} pltkcolor_t;
typedef struct pltkdata {
	plfatptr_t dataPtr;
	uint8_t bytesPerPixel;
} pltkdata_t;

void plTKInit();
pltkwindow_t* plTKCreateWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void plTKWindowRender(pltkwindow_t* window);
void plTKWindowMove(pltkwindow_t* window, uint16_t x, uint16_t y);
void plTKWindowLine(pltkwindow_t* window, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkcolor_t color);
void plTKWindowFBWrite(pltkwindow_t* window, pltkdata_t* data, bool ignoreZero);
void plTKWindowRenderFont(pltkwindow_t* window, uint8_t xFontSize, uint8_t yFontSize, pltkdata_t* data);
void plTKWindowClose(pltkwindow_t* window);
void plTKStop();

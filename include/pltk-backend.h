#include <pl32-memory.h>

typedef struct pltkwindow pltkwindow_t;
typedef struct pltkcolor {
	uint8_t red; /* Depending on the backend, this may vary. For the fbdev backend, red could be all of the colors
			or just red. It's all down to how many bytes it uses per pixel. While I will do my best to use
			all of these, some fields may go unused. */
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
} pltkcolor_t;

void plTKInit();
pltkwindow_t* plTKCreateWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void plTKWindowRender(pltkwindow_t* window);
void plTKWindowMove(pltkwindow_t* window, uint16_t x, uint16_t y);
void plTKWindowLine(pltkwindow_t* window, float xStart, float yStart, float xStop, float yStop, pltkcolor_t color);
void plTKWindowClose(pltkwindow_t* window);
void plTKStop();

#include <pl32.h>

typedef struct pltkwindow pltkwindow_t;
typedef struct pltkcolor {
	uint8_t red; /* Depending on the backend, this may vary. For the fbdev backend, red could be all of the colors
			or just red. It's all down to how many bytes it uses per pixel. While I will do my best to include
			all of these*/
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
} pltkcolor_t;

void plTKInit();
pltkwindow_t* plTKCreateWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height, plmt_t* mt);
void plTKWindowRender(pltkwindow_t* window);
void plTKWindowMove(pltkwindow_t* window);
void plTKWindowLine(pltkwindow_t* window, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkcolor_t color);
void plTKWindowClose(pltkwindow_t* window);
void plTKStop();

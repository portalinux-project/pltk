#include <pltk-backend.h>
#include <unistd.h>

#include "pltk-test-font.pft"

int main(){
	plTKInit();

	pltkwindow_t* mainWindow = plTKCreateWindow(20, 20, 320, 240);
	pltkcolor_t lineColor;

	lineColor.bytes[0] = 255;
	lineColor.bytes[1] = 255;
	lineColor.bytes[2] = 255;
	lineColor.bytes[3] = 0;
	plTKWindowLine(mainWindow, 20, 30, 30, 20, lineColor);

	pltkdata_t fontData = {
		.dataPtr = {
			.array = fontface,
			.size = 9 * 855,
			.isMemAlloc = false,
			.mt = NULL
		},
		.bytesPerPixel = 1,
		.ignoreZero = false
	};
	pltkfont_t font = {
		.data = &fontData,
		.fontSize = { 9, 9 },
		.utf8Support = false
	};
	plTKWindowRenderFont(mainWindow, 0, 0, font, 48, lineColor);
	plTKWindowRenderFont(mainWindow, 9, 0, font, 44, lineColor);
	plTKWindowRenderFont(mainWindow, 18, 0, font, 52, lineColor);
	plTKWindowRenderFont(mainWindow, 27, 0, font, 43, lineColor);
	plTKWindowRenderFont(mainWindow, 36, 0, font, 0, lineColor);
	plTKWindowRenderFont(mainWindow, 45, 0, font, 52, lineColor);
	plTKWindowRenderFont(mainWindow, 54, 0, font, 69, lineColor);
	plTKWindowRenderFont(mainWindow, 63, 0, font, 83, lineColor);
	plTKWindowRenderFont(mainWindow, 72, 0, font, 84, lineColor);


	plTKWindowRender(mainWindow);
	sleep(5);

	plTKWindowClose(mainWindow);
	plTKStop();
}

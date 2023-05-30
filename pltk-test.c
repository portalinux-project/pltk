#include <pltk-backend.h>
#include <unistd.h>

int main(){
	plTKInit();

	pltkwindow_t* mainWindow = plTKCreateWindow(500, 200, 640, 480);
	pltkcolor_t lineColor;

	lineColor.bytes[0] = 255;
	lineColor.bytes[1] = 255;
	lineColor.bytes[2] = 255;
	lineColor.bytes[3] = 0;

	plTKWindowLine(mainWindow, 20, 30, 30, 20, lineColor);
	plTKWindowRender(mainWindow);
	sleep(5);

	plTKWindowClose(mainWindow);
	plTKStop();
}

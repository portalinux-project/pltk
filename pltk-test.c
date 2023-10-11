#include <pltk-core.h>

//#include "pltk-test-font.pft"

int main(){
	plTKInit(0);

	uint16_t positionHack[2] = {320, 240};
	pltkwindow_t* mainWindow = plTKCreateWindow(320, 240, 320, 240);
	plmt_t* inputMT = plMTInit(1024 * 1024);
	pltkcolor_t lineColor;

	lineColor.bytes[0] = 255;
	lineColor.bytes[1] = 255;
	lineColor.bytes[2] = 255;
	lineColor.bytes[3] = 0;
	plTKWindowLine(mainWindow, 20, 30, 30, 20, lineColor);

	/*
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
	*/

	plTKWindowRender(mainWindow);

	pltkinput_t* keyboard = plTKInputInit("event5", true, inputMT);
	pltkinput_t* mouse = plTKInputInit("event4", true, inputMT);

	bool endProg = false;
	while(endProg == false){
		pltkevent_t keyboardinput = plTKGetInput(keyboard);
		pltkevent_t mouseinput = plTKGetInput(mouse);

		if(keyboardinput.value == PLTK_KEY_SPACE || (mouseinput.type == PLTK_KEYDOWN && mouseinput.value == PLTK_KEY_LEFTCLICK))
			endProg = true;

		switch(mouseinput.type){
			case PLTK_REL_X:
				positionHack[0] += *((int*)&mouseinput.value);
				break;
			case PLTK_REL_Y:
				positionHack[1] += *((int*)&mouseinput.value);
				break;
			default:;
		}

		if(mouseinput.type == PLTK_REL_X || mouseinput.type == PLTK_REL_Y)
			plTKWindowMove(mainWindow, positionHack[0], positionHack[1]);
	}

	plTKInputClose(mouse);
	plTKInputClose(keyboard);
	plTKWindowClose(mainWindow);
	plTKStop();
}

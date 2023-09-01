#include <pltk-basic.h>
#include "pltk-test-font.pft"

int main(){
	plTKBasicInit(PLTK_POINTER_REL, 640, 480, false);

	pltkcolor_t lineColor;

	lineColor.bytes[0] = 255;
	lineColor.bytes[1] = 255;
	lineColor.bytes[2] = 255;
	lineColor.bytes[3] = 0;
	plTKBasicWindowLine(20, 30, 30, 20, lineColor);

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
	plTKBasicWindowRenderFont(0, 0, font, 48, lineColor);
	plTKBasicWindowRenderFont(9, 0, font, 44, lineColor);
	plTKBasicWindowRenderFont(18, 0, font, 52, lineColor);
	plTKBasicWindowRenderFont(27, 0, font, 43, lineColor);
	plTKBasicWindowRenderFont(36, 0, font, 0, lineColor);
	plTKBasicWindowRenderFont(45, 0, font, 52, lineColor);
	plTKBasicWindowRenderFont(54, 0, font, 69, lineColor);
	plTKBasicWindowRenderFont(63, 0, font, 83, lineColor);
	plTKBasicWindowRenderFont(72, 0, font, 84, lineColor);
	plTKBasicUpdate(true);

	bool endProg = false;
	pltkbasicptr_t mouseInputBuf = {
		.time = {
			.tv_sec = 0,
			.tv_nsec = 0
		},
		.buttonPressed = PLTK_KEY_ERROR,
		.x = 0,
		.y = 0
	};
	while(!endProg){
		pltkbasicptr_t mouseInput = plTKBasicGetPtrState();
		pltkbasickb_t keyboardInput = plTKBasicGetKBInput();

		if(mouseInput.buttonPressed == PLTK_KEY_LEFTCLICK && mouseInputBuf.buttonPressed == PLTK_KEY_ERROR && plTKBasicPointerHoversOverWindow()){
			mouseInputBuf = mouseInput;
		}else if(mouseInput.buttonPressed == PLTK_KEY_ERROR && mouseInputBuf.buttonPressed != mouseInput.buttonPressed){
			pltkwinfo_t windowInfo = plTKBasicWindowGetInfo();
			int relX = (int)mouseInput.x - (int)mouseInputBuf.x;
			int relY = (int)mouseInput.y - (int)mouseInputBuf.y;

			while((int)windowInfo.x + relX < 0)
				relX++;

			while((int)windowInfo.y + relY < 0)
				relY++;

			plTKBasicWindowMove(windowInfo.x + relX, windowInfo.y + relY);
			mouseInputBuf.buttonPressed = PLTK_KEY_ERROR;
		}

		if(keyboardInput.type == PLTK_KEYDOWN && keyboardInput.value == PLTK_KEY_SPACE)
			endProg = true;

		plTKBasicUpdate(false);
	}

	plTKBasicStop();
}

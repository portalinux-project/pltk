#include "../../core/framebuffer/framebuffer-api.h"
#include "cursor.c"
#include "../../../include/pltk-basic.h"

pltkfbinfo_t fbinfo;
pltkwindow_t* window;
pltkinput_t* kb;
pltkinput_t* ptr;
plmt_t* inputMT;
pltkdata_t cursorData;
bool drawCursor = true;
pltkitype_t pointerType;
struct timespec pointerLastInputTime;
unsigned int pointerButton = PLTK_KEY_ERROR;
uint16_t pointerPos[2] = { 0, 0 };

plfatptr_t plTKBasicGenerateCursorGraphic(char* data){
	plfatptr_t retStruct;

	retStruct.array = plMTAllocE(inputMT, 24 * 24 * fbinfo.bytesPerPixel);
	retStruct.size = 24 * 24 * fbinfo.bytesPerPixel;

	for(int i = 0; i < 576; i++){
		int color = 0;
		if(data[i] == 1)
			color = 1;
		else if(data[i] == 2)
			color = 255;

		((uint8_t*)retStruct.array)[i * 4] = color;
		((uint8_t*)retStruct.array)[i * 4 + 1] = color;
		((uint8_t*)retStruct.array)[i * 4 + 2] = color;
		((uint8_t*)retStruct.array)[i * 4 + 3] = 0;
	}

	return retStruct;
}

void plTKBasicDrawScreenFragment(){
	uint8_t dataBuf[24 * 24 * fbinfo.bytesPerPixel];
	pltkdata_t data;

	int startPos[2] = {pointerPos[0] - window->position[0], pointerPos[1] - window->position[1]};
	int stopPos[2] = {startPos[0] + 24, startPos[1] + 24};

	if(pointerPos[0] < window->position[0]){
		stopPos[0] = startPos[0] + 24;
		startPos[0] = 0;
	}

	if(pointerPos[1] < window->position[1]){
		stopPos[1] = startPos[1] + 24;
		startPos[1] = 0;
	}

	if(pointerPos[0] + 24 > window->position[0] + window->dimensions[0])
		stopPos[0] = window->dimensions[0];
	if(pointerPos[1] + 24 > window->position[1] + window->dimensions[1])
		stopPos[1] = window->dimensions[1];

	data.dataPtr.array = dataBuf;
	data.dataPtr.size = (stopPos[0] - startPos[0]) * (stopPos[1] - startPos[1]) * fbinfo.bytesPerPixel;
	data.bytesPerPixel = fbinfo.bytesPerPixel;
	data.ignoreZero = false;

	uint8_t* startData = window->windowBuffer + (startPos[0] * fbinfo.bytesPerPixel) + (window->dimensions[0] * startPos[1] * fbinfo.bytesPerPixel);
	uint8_t* dataArray = data.dataPtr.array;
	for(int i = 0; i < stopPos[1] - startPos[1]; i++){
		memcpy(dataArray, startData, (stopPos[0] - startPos[0]) * fbinfo.bytesPerPixel);
		startData += window->dimensions[0] * fbinfo.bytesPerPixel;
		dataArray += (stopPos[0] - startPos[0]) * fbinfo.bytesPerPixel;
	}

	plTKFBWrite(&data, window->position[0] + startPos[0], window->position[1] + startPos[1], window->position[0] + stopPos[0], window->position[1] + stopPos[1], false);
}

char* plTKBasicGetInputName(pltkitype_t devType){
	DIR* eventDirectory = opendir("/dev/input");
	struct dirent* dirEntry;

	int bitsPerLong = sizeof(long) * 8;
	long evBitfield[EV_MAX / bitsPerLong];
	memset(evBitfield, 0, sizeof(evBitfield));

	static char retName[32];
	while((dirEntry = readdir(eventDirectory)) != NULL){
		int deviceFeatures = 0;

		memset(retName, 0, 32);
		snprintf(retName, 32, "/dev/input/%s", dirEntry->d_name);
		int fd = open(retName, O_RDONLY);
		ioctl(fd, EVIOCGBIT(0, EV_MAX), evBitfield);

		for(int evType = 0; evType < EV_MAX; evType++){
			if(evBitfield[evType / bitsPerLong] & (1 << (evType % bitsPerLong))){
				switch(devType){
					case PLTK_KEYBOARD:
						if(evType == EV_KEY || evType == EV_REP)
							deviceFeatures++;

						if(deviceFeatures == 2){
							snprintf(retName, 32, "%s", dirEntry->d_name);
							return retName;
						}
						break;
					case PLTK_POINTER_REL:
						if(evType == EV_REL)
							deviceFeatures++;

						if(deviceFeatures){
							snprintf(retName, 32, "%s", dirEntry->d_name);
							return retName;
						}
						break;
					case PLTK_POINTER_ABS:
						if(evType == EV_ABS)
							deviceFeatures++;

						if(deviceFeatures){
							snprintf(retName, 32, "%s", dirEntry->d_name);
							return retName;
						}
						break;
					default:;
				}
			}
		}
		close(fd);
	}

	plTKPanic("plTKBasicGetInputPath: Cannot find device", false, false);
	return NULL;
}

void plTKBasicInit(pltkitype_t ptrType, uint16_t width, uint16_t height, bool disableCursorDraw){
	plTKInit(0);
	window = plTKCreateWindow((fbinfo.displaySize[0] / 2) - (width / 2), (fbinfo.displaySize[1] / 2) - (height / 2), width, height);

	fbinfo = plTKFBGetInfo();
	inputMT = plMTInit(1024 * 1024);
	kb = plTKInputInit(plTKBasicGetInputName(PLTK_KEYBOARD), true, inputMT);
	ptr = plTKInputInit(plTKBasicGetInputName(ptrType), true, inputMT);

	pointerPos[0] = fbinfo.displaySize[0] / 2;
	pointerPos[1] = fbinfo.displaySize[1] / 2;
	pointerType = ptrType;
	drawCursor = !disableCursorDraw;

	cursorData.dataPtr = plTKBasicGenerateCursorGraphic(cursor);
	cursorData.bytesPerPixel = fbinfo.bytesPerPixel;
	cursorData.ignoreZero = true;

	plTKWindowRender(window);

	if(drawCursor)
		plTKFBWrite(&cursorData, pointerPos[0], pointerPos[1], pointerPos[0] + 24, pointerPos[1] + 24, false);
}

void plTKBasicStop(){
	plTKInputClose(kb);
	plTKInputClose(ptr);
	plTKWindowClose(window);
	plMTStop(inputMT);
	plTKStop();
}

void plTKBasicUpdate(bool updateWindow){
	pltkevent_t pointerInput = plTKGetInput(ptr);
	pltkwinfo_t windowInfo = plTKWindowGetInfo(window);

	bool updatePointerLocation = false;
	if(pointerInput.type == PLTK_ABS_X || pointerInput.type == PLTK_ABS_Y || pointerInput.type == PLTK_REL_X || pointerInput.type == PLTK_REL_Y)
		updatePointerLocation = true;

	if(drawCursor && updatePointerLocation)
		plTKFBClear(pointerPos[0], pointerPos[1], pointerPos[0] + 24, pointerPos[1] + 24, false);

	if(((pointerPos[0] + 24 > windowInfo.x && pointerPos[0] < windowInfo.x + windowInfo.width) && (pointerPos[1] + 24 > windowInfo.y && pointerPos[1] < windowInfo.y + windowInfo.height)) && updatePointerLocation)
		plTKBasicDrawScreenFragment();

	switch(pointerInput.type){
		case PLTK_ABS_X:
			pointerPos[0] = pointerInput.value;
			pointerLastInputTime = pointerInput.time;
			break;
		case PLTK_ABS_Y:
			pointerPos[1] = pointerInput.value;
			pointerLastInputTime = pointerInput.time;
			break;
		case PLTK_REL_X:
			pointerPos[0] += *((int*)&pointerInput.value);
			pointerLastInputTime = pointerInput.time;
			break;
		case PLTK_REL_Y:
			pointerPos[1] += *((int*)&pointerInput.value);
			pointerLastInputTime = pointerInput.time;
			break;
		case PLTK_KEYDOWN:
			pointerButton = pointerInput.value;
			break;
		case PLTK_KEYUP:
			pointerButton = PLTK_KEY_ERROR;
			break;
		default: ;
	}

	if(updateWindow)
		plTKWindowRender(window);

	if(drawCursor)
		plTKFBWrite(&cursorData, pointerPos[0], pointerPos[1], pointerPos[0] + 24, pointerPos[1] + 24, true);

	return;
}

void plTKBasicWindowMove(uint16_t x, uint16_t y){
	plTKWindowMove(window, x, y);
}

void plTKBasicWindowPixel(uint16_t x, uint16_t y, pltkcolor_t color){
	plTKWindowPixel(window, x, y, color);
}

void plTKBasicWindowLine(uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkcolor_t color){
	plTKWindowLine(window, xStart, yStart, xStop, yStop, color);
}

void plTKBasicWindowFBWrite(uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkdata_t* data){
	plTKWindowFBWrite(window, xStart, yStart, xStop, yStop, data);
}

void plTKBasicWindowRenderFont(uint16_t x, uint16_t y, pltkfont_t font, uint32_t index, pltkcolor_t color){
	plTKWindowRenderFont(window, x, y, font, index, color);
}

pltkbasickb_t plTKBasicGetKBInput(){
	return plTKGetInput(kb);
}

pltkbasicptr_t plTKBasicGetPtrState(){
	pltkbasicptr_t retStruct;
	retStruct.time = pointerLastInputTime;
	retStruct.buttonPressed = pointerButton;
	retStruct.x = pointerPos[0];
	retStruct.y = pointerPos[1];

	return retStruct;
}

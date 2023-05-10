#include <pltk-backend.h>
#include <math.h>
#include <fcntl.h>
#include <sys/mman.h>

int fb;
byte_t* fbmem;
uint16_t displaySize[2];
uint16_t scanlineSize;
uint8_t bytesPerPixel;

bool fullscreenMode = false;
uint16_t controllingWindowId = 0;
uint16_t windowAmnt = 0;

struct pltkwindow {
	uint16_t position[2];
	uint16_t dimensions[2];
	uint8_t* windowBuffer;
	bool inFocus;
	bool disableWindowBar;
	uint16_t windowId;
	plmt_t* mt;
};

void plTKInit(){
	fb = open("/dev/fb0", O_RDWR);
	if(fb == -1)
		plPanic("plTKInit", true, false);

	plmt_t* mt = plMTInit(0);

	plfile_t* dispSize = plFOpen("/sys/class/graphics/fb0/virtual_size", "r", mt);
	plfile_t* strideSize = plFOpen("/sys/class/graphics/fb0/stride", "r", mt);
	plfile_t* bitsPerPixel = plFOpen("/sys/class/graphics/fb0/bits_per_pixel", "r", mt);
	char stringBuf[256] = "";
	char* convertBuf;
	char* strtolBuf;

	plFGets(stringBuf, 256, dispSize);
	convertBuf = strtok(stringBuf, ",\n");
	displaySize[0] = strtol(convertBuf, &strtolBuf, 10) - 1;
	convertBuf = strtok(NULL, ",\n");
	displaySize[1] = strtol(convertBuf, &strtolBuf, 10) - 1;
	plFClose(dispSize);

	plFGets(stringBuf, 256, strideSize);
	scanlineSize = strtol(stringBuf, &strtolBuf, 10);
	plFClose(strideSize);

	plFGets(stringBuf, 256, bitsPerPixel);
	bytesPerPixel = strtol(stringBuf, &strtolBuf, 10) / 8;
	plFClose(bitsPerPixel);

	plMTStop(mt);

	fbmem = mmap(NULL, scanlineSize * displaySize[1] * bytesPerPixel, PROT_WRITE, MAP_SHARED, fb, 0);
	if(fbmem == NULL)
		plPanic("plTKInit: Failed to map framebuffer to memory", false, false);
}

pltkwindow_t* plTKCreateWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height){
	if(x > displaySize[0])
		x = displaySize[0] - 1;
	if(y > displaySize[0])
		y = displaySize[1] - 1;
	if(width > displaySize[0])
		width = displaySize[0];
	if(height > displaySize[1])
		height = displaySize[1];

	plmt_t* windowMT = plMTInit((width * height * bytesPerPixel) + (1024 * 1024));
	pltkwindow_t* retWindow = plMTAllocE(windowMT, sizeof(pltkwindow_t));

	retWindow->position[0] = x;
	retWindow->position[1] = y;
	retWindow->dimensions[0] = width;
	retWindow->dimensions[1] = height;
	retWindow->windowBuffer = plMTAllocE(windowMT, width * height * bytesPerPixel);
	retWindow->inFocus = true;
	retWindow->disableWindowBar = false;
	retWindow->windowId = windowAmnt++;
	retWindow->mt = windowMT;

	return retWindow;
}

void plTKFBWrite(plfatptr_t* data, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop){
	if(data == NULL)
		plPanic("plTKFBWrite: Data buffer was set to NULL", false, true);

	uint8_t* startPtr = fbmem + (xStart * yStart * bytesPerPixel);

	int32_t xOverdraw = displaySize[0] - (xStart + xStop);
	int32_t yOverdraw = displaySize[1] - (yStart + yStop);
	uint16_t drawDim[2] = {xStop, yStop};

	if(xOverdraw > 0)
		drawDim[0] -= xOverdraw;
	if(yOverdraw > 0)
		drawDim[1] -= yOverdraw;

	uint8_t* dataByte = data->array;
	for(int i = 0; i < drawDim[1]; i++){
		for(int j = 0; j < drawDim[0]; j++){
			for(int k = 0; k < bytesPerPixel; k++){
				startPtr[j * bytesPerPixel + k] = *dataByte;
				if(dataByte < (uint8_t*)data->array + data->size - 1)
					dataByte++;
				else
					dataByte = data->array;
			}
		}
		startPtr += scanlineSize * bytesPerPixel;
	}
}

void plTKWindowRender(pltkwindow_t* window){
	plfatptr_t dataPtr;
	dataPtr.array = window->windowBuffer;
	dataPtr.size = window->dimensions[0] * window->dimensions[1] * bytesPerPixel;

	plTKFBWrite(&dataPtr, window->position[0], window->position[1], window->position[0] + window->dimensions[0], window->position[1] + window->dimensions[1]);
}

void plTKWindowMove(pltkwindow_t* window, uint16_t x, uint16_t y){
	uint8_t clear = 0;

	plfatptr_t dataPtr;
	dataPtr.array = &clear;
	dataPtr.size = 1;

	plTKFBWrite(&dataPtr, window->position[0], window->position[1], window->position[0] + window->dimensions[0], window->position[1] + window->dimensions[1]);

	window->position[0] = x;
	window->position[1] = y;
	plTKWindowRender(window);
}

void plTKWindowLine(pltkwindow_t* window, float xStart, uint16_t yStart, float xStop, uint16_t yStop, pltkcolor_t color){
	xStart = round(xStart);
	xStop = round(xStop);

	if(xStart > window->dimensions[0])
		xStart = window->dimensions[0];
	else if (xStart < 0)
		xStart = 0;

	if(yStart > window->dimensions[1])
		yStart = window->dimensions[1];

	if(xStop > window->dimensions[0])
		xStop = window->dimensions[1];
	else if(xStop < 0)
		xStop = 0;

	if(yStop > window->dimensions[1])
		yStop = window->dimensions[1];

	if(yStart > yStop){
		uint16_t holder = yStart;
		yStart = yStop;
		yStop = holder;

		holder = xStart;
		xStart = xStop;
		xStop = holder;
	}

	int32_t xDiff = xStop - xStart;
	uint16_t yDiff = yStop - yStart;
	if(xDiff < 0)
		xDiff = -xDiff;

	float slope = xDiff / yDiff;
}

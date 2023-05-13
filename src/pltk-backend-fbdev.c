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
	fputs("\x1b[?25l", stdout);
	fflush(stdout);

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
	displaySize[0] = strtol(convertBuf, &strtolBuf, 10);
	convertBuf = strtok(NULL, ",\n");
	displaySize[1] = strtol(convertBuf, &strtolBuf, 10);
	plFClose(dispSize);

	plFGets(stringBuf, 256, bitsPerPixel);
	bytesPerPixel = strtol(stringBuf, &strtolBuf, 10) / 8;
	plFClose(bitsPerPixel);

	plFGets(stringBuf, 256, strideSize);
	scanlineSize = strtol(stringBuf, &strtolBuf, 10) / bytesPerPixel;
	plFClose(strideSize);

	plMTStop(mt);

	fbmem = mmap(NULL, scanlineSize * displaySize[1] * bytesPerPixel, PROT_WRITE, MAP_SHARED, fb, 0);
	if(fbmem == MAP_FAILED)
		plPanic("plTKInit: Failed to map framebuffer to memory", false, true);
}

void plTKFBWrite(pltkdata_t* data, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop){
	if(data == NULL || data->dataPtr.array == NULL || data->dataPtr.size == 0)
		plPanic("plTKFBWrite: Given data buffer is either empty or invalid", false, true);

	if(data->bytesPerPixel != bytesPerPixel)
		plPanic("plTKFBWrite: Mismached bitness between data buffer and framebuffer", false, true);

	uint8_t* startPtr = fbmem + (scanlineSize * yStart * bytesPerPixel) + (xStart * bytesPerPixel);

	int32_t xOverdraw = displaySize[0] - (xStart + xStop) - 1;
	int32_t yOverdraw = displaySize[1] - (yStart + yStop) - 1;
	uint16_t drawDim[2] = {xStop, yStop};

	if(xOverdraw > 0)
		drawDim[0] -= xOverdraw;
	if(yOverdraw > 0)
		drawDim[1] -= yOverdraw;

	uint8_t* dataByte = data->dataPtr.array;
	for(int i = 0; i < drawDim[1]; i++){
		for(int j = 0; j < drawDim[0]; j++){
			for(int k = 0; k < bytesPerPixel; k++){
				startPtr[j * bytesPerPixel + k] = *dataByte;

				if(dataByte < (uint8_t*)data->dataPtr.array + data->dataPtr.size - 1)
					dataByte++;
				else
					dataByte = data->dataPtr.array;
			}
		}
		startPtr += scanlineSize * bytesPerPixel;
	}
}

pltkwindow_t* plTKCreateWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height){
	if(x > displaySize[0])
		x = displaySize[0] - 2;
	if(y > displaySize[0])
		y = displaySize[1] - 2;
	if(width > displaySize[0])
		width = displaySize[0] - 1;
	if(height > displaySize[1])
		height = displaySize[1] - 1;

	plmt_t* windowMT = plMTInit((width * height * bytesPerPixel) * 3);
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

	for(int i = 0; i < (width * height * bytesPerPixel); i++)
		retWindow->windowBuffer[i] = 0;

	return retWindow;
}

void plTKWindowRender(pltkwindow_t* window){
	pltkdata_t data;
	data.dataPtr.array = window->windowBuffer;
	data.dataPtr.size = window->dimensions[0] * window->dimensions[1] * bytesPerPixel;
	data.bytesPerPixel = bytesPerPixel;
	data.ignoreZero = false;

	plTKFBWrite(&data, window->position[0], window->position[1], window->position[0] + window->dimensions[0], window->position[1] + window->dimensions[1]);
}

void plTKWindowMove(pltkwindow_t* window, uint16_t x, uint16_t y){
	uint8_t clear = 0;

	pltkdata_t data;
	data.dataPtr.array = &clear;
	data.dataPtr.size = 1;
	data.bytesPerPixel = bytesPerPixel;

	plTKFBWrite(&data, window->position[0], window->position[1], window->position[0] + window->dimensions[0], window->position[1] + window->dimensions[1]);

	window->position[0] = x;
	window->position[1] = y;
	plTKWindowRender(window);
}

void plTKWindowLine(pltkwindow_t* window, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkcolor_t color){
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

	float slope = xDiff / yDiff;
	float xCurrent = xStart;
	uint16_t scanlineStep = (window->dimensions[0] - xStart) * bytesPerPixel;
	uint8_t* startPtr = window->windowBuffer + (xStart * yStart * bytesPerPixel);

	for(int i = 0; i <= yDiff; i++){
		for(int j = 0; j < bytesPerPixel; i++)
			startPtr[j] = color.bytes[j];

		xCurrent += slope;
		startPtr += scanlineStep + ((uint16_t)round(xCurrent) * bytesPerPixel);
	}
}

void plTKConvertBPP(pltkdata_t* data, pltkwindow_t* window){
	uint8_t* originalPtr = data->dataPtr.array;
	uint8_t* newPtr = plMTAllocE(window->mt, data->dataPtr.size);

	//TODO: Add bpp conversion logic

	data->dataPtr.array = newPtr;
	data->dataPtr.mt = window->mt;
}

void plTKWindowPixel(pltkwindow_t* window, uint16_t x, uint16_t y, pltkcolor_t color){
	uint8_t* startPtr = window->windowBuffer + (x * y * bytesPerPixel);
	for(int i = 0; i < bytesPerPixel; i++){
		startPtr[i] = color.bytes[i];
	}
}

void plTKWindowFBWrite(pltkwindow_t* window, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkdata_t* data){
	if(data == NULL || data->dataPtr.array == NULL || data->dataPtr.size == 0)
		plPanic("plTKWindowFBWrite: Given data buffer is either empty or invalid", false, true);

	if(data->bytesPerPixel != bytesPerPixel)
		plTKConvertBPP(data, window);

	int32_t xOverdraw = window->dimensions[0] - (xStart + xStop);
	int32_t yOverdraw = window->dimensions[1] - (yStart + yStop);
	uint16_t drawDim[2] = {xStop, yStop};

	if(xOverdraw > 0)
		drawDim[0] -= xOverdraw;
	if(yOverdraw > 0)
		drawDim[1] -= yOverdraw;

	pltkcolor_t pixel;
	uint8_t* dataByte = data->dataPtr.array;
	for(int i = 0; i < drawDim[1]; i++){
		for(int j = 0; j < drawDim[0]; j++){
			for(int k = 0; k < bytesPerPixel; k++)
				pixel.bytes[k] = dataByte[k];

			bool isZero = (pixel.bytes[0] == 0 && pixel.bytes[1] == 0 && pixel.bytes[2] == 0 && pixel.bytes[3] == 0);
			if(!(isZero && data->ignoreZero))
				plTKWindowPixel(window, xStart + j, yStart + i, pixel);

			if(dataByte < (uint8_t*)data->dataPtr.array + data->dataPtr.size - 1)
				dataByte += bytesPerPixel;
			else
				dataByte = data->dataPtr.array;
		}
	}

	if(data->dataPtr.mt == window->mt){
		plMTFree(window->mt,data->dataPtr.array);
		data->dataPtr.array = NULL;
		data->dataPtr.mt = NULL;
	}
}


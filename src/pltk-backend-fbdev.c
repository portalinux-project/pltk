#include <pltk-backend.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int fb = -2;
byte_t* fbmem = NULL;
uint16_t displaySize[2];
uint16_t scanlineSize;
uint8_t bytesPerPixel;

bool fullscreenMode = false;
uint8_t backgroundColor = 96;
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

void plTKFBWrite(pltkdata_t* data, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop){
	if(data == NULL || data->dataPtr.array == NULL || data->dataPtr.size == 0)
		plPanic("\x1b[?25hplTKFBWrite: Given data buffer is either empty or invalid", false, true);

	if(data->bytesPerPixel != bytesPerPixel)
		plPanic("\x1b[?25hplTKFBWrite: Mismached bitness between data buffer and framebuffer", false, true);

	uint8_t* startPtr = fbmem + (scanlineSize * yStart * bytesPerPixel) + (xStart * bytesPerPixel);

	int32_t xOverdraw = displaySize[0] - (xStart + xStop) - 1;
	int32_t yOverdraw = displaySize[1] - (yStart + yStop) - 1;
	uint16_t drawDim[2] = {xStop - xStart, yStop - yStart};

	if(xOverdraw < 0)
		drawDim[0] += xOverdraw;
	if(yOverdraw < 0)
		drawDim[1] += yOverdraw;

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

void plTKFBClear(uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop){
	pltkdata_t data;
	data.dataPtr.array = &backgroundColor;
	data.dataPtr.size = 1;
	data.bytesPerPixel = bytesPerPixel;

	plTKFBWrite(&data, xStart, yStart, xStop, yStop);
}

void plTKPanic(string_t string, bool usePerror, bool devBug){
	if(fb != -1 && fbmem != MAP_FAILED)
		plTKFBClear(0, 0, displaySize[0] - 1, displaySize[1] - 1);

	fputs("\x1b[2J\x1b[?25h", stdout);
	fflush(stdout);

	plPanic(string, usePerror, devBug);
}

void plTKInit(){
	if(fbmem != NULL)
		return;

	fputs("\x1b[2J\x1b[?25l", stdout);
	fflush(stdout);

	fb = open("/dev/fb0", O_RDWR);
	if(fb == -1)
		plTKPanic("plTKInit", true, false);

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
		plTKPanic("plTKInit: Failed to map framebuffer to memory", false, true);

	plTKFBClear(0, 0, displaySize[0] - 1, displaySize[1] - 1);
}

void plTKStop(){
	if(fbmem == NULL)
		plPanic("plTKStop: PLTK hasn't been initialized yet", false, true);

	plTKFBClear(0, 0, displaySize[0] - 1, displaySize[1] - 1);
	munmap(fbmem, displaySize[0] * displaySize[1] * bytesPerPixel);
	close(fb);

	fb = -2;
	fbmem = NULL;

	fputs("\x1b[2J\x1b[1;1H\x1b[?25h", stdout);
	fflush(stdout);
}

pltkwindow_t* plTKCreateWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height){
	if(fbmem == NULL)
		plPanic("plTKCreateWindow: PLTK hasn't been initialized yet", false, true);

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

void plTKWindowClose(pltkwindow_t* window){
	if(fbmem == NULL)
		plPanic("plTKWindowClose: PLTK hasn't been initialized yet", false, true);

	plTKFBClear(window->position[0], window->position[1], window->position[0] + window->dimensions[0], window->position[1] + window->dimensions[1]);

	plmt_t* mt = window->mt;
	plMTFree(mt, window->windowBuffer);
	plMTFree(mt, window);
	plMTStop(mt);
}

void plTKWindowRender(pltkwindow_t* window){
	if(fbmem == NULL)
		plPanic("plTKWindowRender: PLTK hasn't been initialized yet", false, true);

	if(window == NULL)
		plTKPanic("plTKWindowRender: Window handle was set as NULL", false, true);

	pltkdata_t data;
	data.dataPtr.array = window->windowBuffer;
	data.dataPtr.size = window->dimensions[0] * window->dimensions[1] * bytesPerPixel;
	data.bytesPerPixel = bytesPerPixel;
	data.ignoreZero = false;

	plTKFBWrite(&data, window->position[0], window->position[1], window->position[0] + window->dimensions[0], window->position[1] + window->dimensions[1]);
}

void plTKWindowMove(pltkwindow_t* window, uint16_t x, uint16_t y){
	if(fbmem == NULL)
		plPanic("plTKWindowMove: PLTK hasn't been initialized yet", false, true);

	if(window == NULL)
		plTKPanic("plTKWindowMove: Window handle was set as NULL", false, true);

	plTKFBClear(window->position[0], window->position[1], window->position[0] + window->dimensions[0], window->position[1]);
	window->position[0] = x;
	window->position[1] = y;
	plTKWindowRender(window);
}

void plTKWindowPixel(pltkwindow_t* window, uint16_t x, uint16_t y, pltkcolor_t color){
	if(window == NULL)
		plTKPanic("plTKWindowPixel: Window handle was set as NULL", false, true);

	uint8_t* startPtr = window->windowBuffer + (x * bytesPerPixel) + (y * window->dimensions[0] * bytesPerPixel);
	for(int i = 0; i < bytesPerPixel; i++){
		startPtr[i] = color.bytes[i];
	}
}

void plTKWindowLine(pltkwindow_t* window, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkcolor_t color){
	if(window == NULL)
		plTKPanic("plTKWindowRender: Window handle was set as NULL", false, true);

	if(xStart > window->dimensions[0])
		xStart = window->dimensions[0];

	if(yStart > window->dimensions[1])
		yStart = window->dimensions[1];

	if(xStop > window->dimensions[0])
		xStop = window->dimensions[1];

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

	if(yDiff != 0){
		uint16_t xCurrent = xStart;
		float slope = xDiff / yDiff;
		float xNext = xCurrent + slope;

		for(int i = 0; i <= yDiff; i++){
			uint16_t xLimit = round(xNext);
			while(xCurrent != xLimit){
				plTKWindowPixel(window, xCurrent, yStart + i, color);
				if(xCurrent < xLimit)
					xCurrent++;
				else
					xCurrent--;
			}
			xCurrent = xLimit;
			xNext += slope;
		}
	}else{
		if(xStop < xStart){
			uint8_t holder = xStart;
			xStart = xStop;
			xStop = holder;
		}

		while(xStart < xStop){
			plTKWindowPixel(window, xStart, yStart, color);
			xStart++;
		}
	}
}

void plTKConvertBPP(pltkdata_t* data, pltkwindow_t* window){
	if(data == NULL || data->dataPtr.array == NULL || data->dataPtr.size == 0)
		plTKPanic("plTKConvertBPP: Given data buffer is either empty or invalid", false, true);

	if(window == NULL)
		plTKPanic("plTKConvertBPP: Window handle was set as NULL", false, true);

	//TODO: Add proper bpp downsampling
	if(data->bytesPerPixel > bytesPerPixel){
		data->bytesPerPixel = bytesPerPixel;
		return;
	}

	size_t bufferPixels = data->dataPtr.size / data->bytesPerPixel;
	pltkcolor_t pixelHolder;
	uint8_t* originalPtr = data->dataPtr.array;
	uint8_t* newPtr = plMTAllocE(window->mt, bufferPixels * bytesPerPixel);

	switch(data->bytesPerPixel){
		case 1:
			for(int i = 0; i < bufferPixels; i++){
				if(bytesPerPixel > 2){
					pixelHolder.bytes[0] = (originalPtr[i] >> 5);
					pixelHolder.bytes[1] = ((originalPtr[i] >> 2) & 0x7);
					pixelHolder.bytes[2] = (originalPtr[i] & 0x3);
					pixelHolder.bytes[3] = 0;

					pixelHolder.bytes[0] *= pixelHolder.bytes[0] * pixelHolder.bytes[0];
					pixelHolder.bytes[1] *= pixelHolder.bytes[1] * pixelHolder.bytes[1];
					pixelHolder.bytes[2] *= pixelHolder.bytes[2] * pixelHolder.bytes[2];
				}else{
					pixelHolder.bytes[0] = originalPtr[i] >> 4;
					pixelHolder.bytes[1] = originalPtr[i] & 0xf;

					pixelHolder.bytes[0] *= pixelHolder.bytes[0];
					pixelHolder.bytes[1] *= pixelHolder.bytes[1];
				}

				for(int j = 0; j < bufferPixels; i++){
					newPtr[i * bytesPerPixel + j] = pixelHolder.bytes[j];
				}
			}
			break;
		case 2:
			for(int i = 0; i < bufferPixels; i++){
				pixelHolder.bytes[0] = (originalPtr[i * 2] >> 3) * 4;
				pixelHolder.bytes[1] = (((originalPtr[i * 2] & 0x7) << 3) | (originalPtr[i * 2 + 1] >> 5)) * 4;
				pixelHolder.bytes[2] = (originalPtr[i * 2 + 1] & 0x1f) * 4;
				pixelHolder.bytes[3] = 0;

				for(int j = 0; j < bufferPixels; i++){
					newPtr[i * bytesPerPixel + j] = pixelHolder.bytes[j];
				}
			}
			break;
		case 3:
			for(int i = 0; i < bufferPixels; i++){
				pixelHolder.bytes[0] = originalPtr[i * 3];
				pixelHolder.bytes[1] = originalPtr[i * 3 + 1];
				pixelHolder.bytes[2] = originalPtr[i * 3 + 2];
				pixelHolder.bytes[3] = 0;
			}
			break;
	}

	data->bytesPerPixel = bytesPerPixel;
	data->dataPtr.size = bufferPixels * bytesPerPixel;
	data->dataPtr.array = newPtr;
	data->dataPtr.mt = window->mt;
}

void plTKWindowFBWrite(pltkwindow_t* window, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkdata_t* data){
	if(window == NULL)
		plTKPanic("plTKWindowFBWrite: Window handle was set as NULL", false, true);

	if(data == NULL || data->dataPtr.array == NULL || data->dataPtr.size == 0)
		plTKPanic("plTKWindowFBWrite: Given data buffer is either empty or invalid", false, true);

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

void plTKWindowRenderFont(pltkwindow_t* window, uint16_t x, uint16_t y, pltkfont_t font, uint32_t index, pltkcolor_t color){
	if(window == NULL)
		plTKPanic("plTKWindowRenderFont: Window handle was set as NULL", false, true);

	uint8_t* startPtr = font.data->dataPtr.array + (index * font.fontSize[1] * font.fontSize[0]);

	for(int i = 0; i < font.fontSize[1]; i++){
		for(int j = 0; j < font.fontSize[0]; j++){
			if(startPtr[i + j] != 0)
				plTKWindowPixel(window, x + j, y + i, color);
		}
	}
}

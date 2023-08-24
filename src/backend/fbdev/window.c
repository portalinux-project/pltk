#include "fbdev-api.h"

pltkwindow_t* plTKCreateWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height){
	pltkfbinfo_t fbinfo = plTKFBGetInfo();

	if(plTKIsFBReady() == false)
		plPanic("plTKCreateWindow: PLTK hasn't been initialized yet", false, true);

	if(x > fbinfo.displaySize[0])
		x = fbinfo.displaySize[0] - 2;
	if(y > fbinfo.displaySize[0])
		y = fbinfo.displaySize[1] - 2;
	if(width > fbinfo.displaySize[0])
		width = fbinfo.displaySize[0] - 1;
	if(height > fbinfo.displaySize[1])
		height = fbinfo.displaySize[1] - 1;

	plmt_t* windowMT = plMTInit((width * height * fbinfo.bytesPerPixel) * 3);
	pltkwindow_t* retWindow = plMTAllocE(windowMT, sizeof(pltkwindow_t));

	retWindow->position[0] = x;
	retWindow->position[1] = y;
	retWindow->dimensions[0] = width;
	retWindow->dimensions[1] = height;
	retWindow->windowBuffer = plMTAllocE(windowMT, width * height * fbinfo.bytesPerPixel);
	retWindow->windowId = plTKFBGetNewWindowID();
	retWindow->mt = windowMT;

	for(int i = 0; i < (width * height * fbinfo.bytesPerPixel); i++)
		retWindow->windowBuffer[i] = 0;

	return retWindow;
}

void plTKWindowClose(pltkwindow_t* window){
	if(plTKIsFBReady() == false)
		plPanic("plTKWindowClose: PLTK hasn't been initialized yet", false, true);

	plTKFBClear(window->position[0], window->position[1], window->position[0] + window->dimensions[0], window->position[1] + window->dimensions[1]);

	plmt_t* mt = window->mt;
	plMTFree(mt, window->windowBuffer);
	plMTFree(mt, window);
	plMTStop(mt);
}

void plTKWindowRender(pltkwindow_t* window){
	if(plTKIsFBReady() == false)
		plPanic("plTKWindowRender: PLTK hasn't been initialized yet", false, true);

	if(window == NULL)
		plTKPanic("plTKWindowRender: Window handle was set as NULL", false, true);

	pltkfbinfo_t fbinfo = plTKFBGetInfo();
	pltkdata_t data;
	data.dataPtr.array = window->windowBuffer;
	data.dataPtr.size = window->dimensions[0] * window->dimensions[1] * fbinfo.bytesPerPixel;
	data.bytesPerPixel = fbinfo.bytesPerPixel;
	data.ignoreZero = false;

	plTKFBWrite(&data, window->position[0], window->position[1], window->position[0] + window->dimensions[0], window->position[1] + window->dimensions[1]);
}

void plTKWindowMove(pltkwindow_t* window, uint16_t x, uint16_t y){
	if(plTKIsFBReady() == false)
		plPanic("plTKWindowMove: PLTK hasn't been initialized yet", false, true);

	if(window == NULL)
		plTKPanic("plTKWindowMove: Window handle was set as NULL", false, true);

	plTKFBClear(window->position[0], window->position[1], window->position[0] + window->dimensions[0], window->position[1] + window->dimensions[1]);
	window->position[0] = x;
	window->position[1] = y;
	plTKWindowRender(window);
}

void plTKWindowPixel(pltkwindow_t* window, uint16_t x, uint16_t y, pltkcolor_t color){
	if(window == NULL)
		plTKPanic("plTKWindowPixel: Window handle was set as NULL", false, true);

	pltkfbinfo_t fbinfo = plTKFBGetInfo();

	uint8_t* startPtr = window->windowBuffer + (x * fbinfo.bytesPerPixel) + (y * window->dimensions[0] * fbinfo.bytesPerPixel);
	for(int i = 0; i < fbinfo.bytesPerPixel; i++)
		startPtr[i] = color.bytes[i];
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

	pltkfbinfo_t fbinfo = plTKFBGetInfo();

	//TODO: Add proper bpp downsampling
	if(data->bytesPerPixel > fbinfo.bytesPerPixel){
		data->bytesPerPixel = fbinfo.bytesPerPixel;
		return;
	}

	size_t bufferPixels = data->dataPtr.size / data->bytesPerPixel;
	pltkcolor_t pixelHolder;
	uint8_t* originalPtr = data->dataPtr.array;
	uint8_t* newPtr = plMTAllocE(window->mt, bufferPixels * fbinfo.bytesPerPixel);

	switch(data->bytesPerPixel){
		case 1:
			for(int i = 0; i < bufferPixels; i++){
				if(fbinfo.bytesPerPixel > 2){
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
					newPtr[i * fbinfo.bytesPerPixel + j] = pixelHolder.bytes[j];
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
					newPtr[i * fbinfo.bytesPerPixel + j] = pixelHolder.bytes[j];
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

	data->bytesPerPixel = fbinfo.bytesPerPixel;
	data->dataPtr.size = bufferPixels * fbinfo.bytesPerPixel;
	data->dataPtr.array = newPtr;
	data->dataPtr.mt = window->mt;
}

void plTKWindowFBWrite(pltkwindow_t* window, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkdata_t* data){
	if(window == NULL)
		plTKPanic("plTKWindowFBWrite: Window handle was set as NULL", false, true);

	if(data == NULL || data->dataPtr.array == NULL || data->dataPtr.size == 0)
		plTKPanic("plTKWindowFBWrite: Given data buffer is either empty or invalid", false, true);

	pltkfbinfo_t fbinfo = plTKFBGetInfo();

	if(data->bytesPerPixel != fbinfo.bytesPerPixel)
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
			for(int k = 0; k < fbinfo.bytesPerPixel; k++)
				pixel.bytes[k] = dataByte[k];

			bool isZero = (pixel.bytes[0] == 0 && pixel.bytes[1] == 0 && pixel.bytes[2] == 0 && pixel.bytes[3] == 0);
			if(!(isZero && data->ignoreZero))
				plTKWindowPixel(window, xStart + j, yStart + i, pixel);

			if(dataByte < (uint8_t*)data->dataPtr.array + data->dataPtr.size - 1)
				dataByte += fbinfo.bytesPerPixel;
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

	uint8_t* startPtr = font.data->dataPtr.array + ((font.fontSize[0] * font.fontSize[1]) * index);

	for(int i = 0; i < font.fontSize[1]; i++){
		for(int j = 0; j < font.fontSize[0]; j++){
			if(startPtr[(i * font.fontSize[0]) + j] != 0)
				plTKWindowPixel(window, x + j, y + i, color);
		}
	}
}

#include "fbdev-api.h"

int fb = -2;
byte_t* fbmem = NULL;
pltkfbinfo_t fbinfo;

bool fullscreenMode = false;
uint8_t backgroundColor = 96;
uint16_t windowAmnt = 0;

void plTKFBWrite(pltkdata_t* data, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop){
	if(data == NULL || data->dataPtr.array == NULL || data->dataPtr.size == 0)
		plTKPanic("plTKFBWrite: Given data buffer is either empty or invalid", false, true);

	if(data->bytesPerPixel != fbinfo.bytesPerPixel)
		plTKPanic("plTKFBWrite: Mismached bitness between data buffer and framebuffer", false, true);

	if(xStop < xStart || yStop < yStart)
		return;

	uint8_t* startPtr = fbmem + (fbinfo.scanlineSize * yStart * fbinfo.bytesPerPixel) + (xStart * fbinfo.bytesPerPixel);

	int32_t xOverdraw = fbinfo.displaySize[0] - xStop - 1;
	int32_t yOverdraw = fbinfo.displaySize[1] - yStop - 1;
	uint16_t drawDim[2] = {xStop - xStart, yStop - yStart};

	if(xOverdraw < 0){
		if((xStop - xStart) + xOverdraw < 1)
			return;
		drawDim[0] += xOverdraw;
	}

	if(yOverdraw < 0){
		if((yStop - yStart) + yOverdraw < 1)
			return;
		drawDim[1] += yOverdraw;
	}

	uint8_t* dataByte = data->dataPtr.array;
	for(int i = 0; i < drawDim[1]; i++){
		for(int j = 0; j < drawDim[0]; j++){
			for(int k = 0; k < fbinfo.bytesPerPixel; k++){
				startPtr[j * fbinfo.bytesPerPixel + k] = *dataByte;
				if(dataByte < (uint8_t*)data->dataPtr.array + data->dataPtr.size - 1)
					dataByte++;
				else
					dataByte = data->dataPtr.array;
			}
		}

		if(xOverdraw < 0)
			dataByte += ((xStop - xStart) - drawDim[0]) * fbinfo.bytesPerPixel;

		startPtr += fbinfo.scanlineSize * fbinfo.bytesPerPixel;
	}
}

void plTKFBClear(uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop){
	pltkdata_t data;
	data.dataPtr.array = &backgroundColor;
	data.dataPtr.size = 1;
	data.bytesPerPixel = fbinfo.bytesPerPixel;

	plTKFBWrite(&data, xStart, yStart, xStop, yStop);
}

void plTKInit(uint8_t screen){
	if(fbmem != NULL)
		return;

	char stringBuf[256] = "";
	snprintf(stringBuf, 256, "/dev/fb%d", screen);
	fb = open(stringBuf, O_RDWR);
	if(fb == -1)
		plTKPanic("plTKInit", true, false);

	fputs("\x1b[2J\x1b[?25l", stdout);
	fflush(stdout);

	plmt_t* mt = plMTInit(0);

	snprintf(stringBuf, 256, "/sys/class/graphics/fb%d/virtual_size", screen);
	plfile_t* dispSize = plFOpen(stringBuf, "r", mt);
	snprintf(stringBuf, 256, "/sys/class/graphics/fb%d/stride", screen);
	plfile_t* strideSize = plFOpen(stringBuf, "r", mt);
	snprintf(stringBuf, 256, "/sys/class/graphics/fb%d/bits_per_pixel", screen);
	plfile_t* bitsPerPixel = plFOpen(stringBuf, "r", mt);
	char* convertBuf;
	char* strtolBuf;

	plFGets(stringBuf, 256, dispSize);
	convertBuf = strtok(stringBuf, ",\n");
	fbinfo.displaySize[0] = strtol(convertBuf, &strtolBuf, 10);
	convertBuf = strtok(NULL, ",\n");
	fbinfo.displaySize[1] = strtol(convertBuf, &strtolBuf, 10);
	plFClose(dispSize);

	plFGets(stringBuf, 256, bitsPerPixel);
	fbinfo.bytesPerPixel = strtol(stringBuf, &strtolBuf, 10) / 8;
	plFClose(bitsPerPixel);

	plFGets(stringBuf, 256, strideSize);
	fbinfo.scanlineSize = strtol(stringBuf, &strtolBuf, 10) / fbinfo.bytesPerPixel;
	plFClose(strideSize);

	plMTStop(mt);

	fbmem = mmap(NULL, fbinfo.scanlineSize * fbinfo.displaySize[1] * fbinfo.bytesPerPixel, PROT_WRITE | PROT_READ, MAP_SHARED, fb, 0);
	if(fbmem == MAP_FAILED)
		plTKPanic("plTKInit: Failed to map framebuffer to memory", false, true);

	plTKFBClear(0, 0, fbinfo.displaySize[0] - 1, fbinfo.displaySize[1] - 1);
}

void plTKStop(){
	if(fbmem == NULL)
		plPanic("plTKStop: PLTK hasn't been initialized yet", false, true);

	backgroundColor = 0;
	plTKFBClear(0, 0, fbinfo.displaySize[0] - 1, fbinfo.displaySize[1] - 1);
	munmap(fbmem, fbinfo.displaySize[0] * fbinfo.displaySize[1] * fbinfo.bytesPerPixel);
	close(fb);

	fb = -2;
	fbmem = NULL;

	fputs("\x1b[2J\x1b[1;1H\x1b[?25h", stdout);
	fflush(stdout);
}

bool plTKIsFBReady(){
	if(fb < 0 || fbmem == NULL)
		return false;
	return true;
}

pltkfbinfo_t plTKFBGetInfo(){
	return fbinfo;
}

void plTKFBSetBackground(uint8_t color){
	backgroundColor = color;
}

uint16_t plTKFBGetNewWindowID(){
	return windowAmnt++;
}

#include "fbdev-api.h"

int fb = -2;
byte_t* fbmem = NULL;
pltkfbinfo_t fbinfo;

bool fullscreenMode = false;
uint8_t backgroundColor = 96;
uint16_t controllingWindowId = 0;
uint16_t windowAmnt = 0;

void plTKFBWrite(pltkdata_t* data, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop){
	if(data == NULL || data->dataPtr.array == NULL || data->dataPtr.size == 0)
		plPanic("\x1b[?25hplTKFBWrite: Given data buffer is either empty or invalid", false, true);

	if(data->bytesPerPixel != fbinfo.bytesPerPixel)
		plPanic("\x1b[?25hplTKFBWrite: Mismached bitness between data buffer and framebuffer", false, true);

	uint8_t* startPtr = fbmem + (fbinfo.scanlineSize * yStart * fbinfo.bytesPerPixel) + (xStart * fbinfo.bytesPerPixel);

	int32_t xOverdraw = fbinfo.displaySize[0] - (xStart + xStop) - 1;
	int32_t yOverdraw = fbinfo.displaySize[1] - (yStart + yStop) - 1;
	uint16_t drawDim[2] = {xStop - xStart, yStop - yStart};

	if(xOverdraw < 0)
		drawDim[0] += xOverdraw;
	if(yOverdraw < 0)
		drawDim[1] += yOverdraw;

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

void plTKFBInit(){
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

	fbmem = mmap(NULL, fbinfo.scanlineSize * fbinfo.displaySize[1] * fbinfo.bytesPerPixel, PROT_WRITE, MAP_SHARED, fb, 0);
	if(fbmem == MAP_FAILED)
		plTKPanic("plTKInit: Failed to map framebuffer to memory", false, true);

	plTKFBClear(0, 0, fbinfo.displaySize[0] - 1, fbinfo.displaySize[1] - 1);
}

void plTKFBStop(){
	if(fbmem == NULL)
		plPanic("plTKStop: PLTK hasn't been initialized yet", false, true);

	plTKFBClear(0, 0, fbinfo.displaySize[0] - 1, fbinfo.displaySize[1] - 1);
	munmap(fbmem, fbinfo.displaySize[0] * fbinfo.displaySize[1] * fbinfo.bytesPerPixel);
	close(fb);

	fb = -2;
	fbmem = NULL;

	fputs("\x1b[2J\x1b[1;1H\x1b[?25h", stdout);
	fflush(stdout);
}


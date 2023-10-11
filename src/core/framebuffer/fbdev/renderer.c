#include "../framebuffer-api.h"

int fb = -2;
byte_t* pfbmem = NULL;
byte_t* fbmem = NULL;
plmt_t* mt = NULL;
pltkfbinfo_t fbinfo;

bool fullscreenMode = false;
uint8_t backgroundColor = 96;
uint16_t windowAmnt = 0;
struct timespec renderDelay = {
	.tv_sec = 0,
	.tv_nsec = 0
};

void plTKFBUpdateBuffer(){
	memcpy(pfbmem, fbmem, fbinfo.scanlineSize * fbinfo.displaySize[1] * fbinfo.bytesPerPixel);
}

void plTKFBWrite(pltkdata_t* data, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, bool refreshBuffer){
	if(data == NULL || data->dataPtr.pointer == NULL || data->dataPtr.size == 0)
		plTKPanic("plTKFBWrite", PLTK_INVALID_BUFFER, true);

	if(data->bytesPerPixel != fbinfo.bytesPerPixel)
		plTKPanic("plTKFBWrite", PLTK_FB_MISMATCHED_BITDEPTH, true);

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

	uint8_t* dataByte = data->dataPtr.pointer;
	for(int i = 0; i < drawDim[1]; i++){
		for(int j = 0; j < drawDim[0]; j++){
			for(int k = 0; k < fbinfo.bytesPerPixel; k++){
				if(*dataByte != 0 || data->ignoreZero == false)
					startPtr[j * fbinfo.bytesPerPixel + k] = *dataByte;

				if(dataByte < (uint8_t*)data->dataPtr.pointer + data->dataPtr.size - 1)
					dataByte++;
				else
					dataByte = data->dataPtr.pointer;
			}
		}

		if(xOverdraw < 0)
			dataByte += ((xStop - xStart) - drawDim[0]) * fbinfo.bytesPerPixel;

		startPtr += fbinfo.scanlineSize * fbinfo.bytesPerPixel;
	}

	if(refreshBuffer == true)
		plTKFBUpdateBuffer();

	if(renderDelay.tv_nsec != 0)
		nanosleep(&renderDelay, NULL);
}

void plTKFBClear(uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, bool refreshBuffer){
	pltkdata_t data;
	data.dataPtr.pointer = &backgroundColor;
	data.dataPtr.size = 1;
	data.bytesPerPixel = fbinfo.bytesPerPixel;
	data.ignoreZero = false;

	plTKFBWrite(&data, xStart, yStart, xStop, yStop, refreshBuffer);
}

void plTKInit(uint8_t screen){
	if(fbmem != NULL)
		return;

	char stringBuf[256] = "";
	snprintf(stringBuf, 256, "/dev/fb%d", screen);
	fb = open(stringBuf, O_RDWR);
	if(fb == -1)
		plTKPanic("plTKInit", PLRT_ERROR | PLRT_ERRNO, false);

	fputs("\x1b[2J\x1b[?25l", stdout);
	fflush(stdout);

	mt = plMTInit(0);

	snprintf(stringBuf, 256, "/sys/class/graphics/fb%d/virtual_size", screen);
	plfile_t* dispSize = plFOpen(stringBuf, "r", mt);
	snprintf(stringBuf, 256, "/sys/class/graphics/fb%d/stride", screen);
	plfile_t* strideSize = plFOpen(stringBuf, "r", mt);
	snprintf(stringBuf, 256, "/sys/class/graphics/fb%d/bits_per_pixel", screen);
	plfile_t* bitsPerPixel = plFOpen(stringBuf, "r", mt);
	char* convertBuf;
	char* strtolBuf;

	plstring_t plStringBuf = {
		.data = {
			.pointer = stringBuf,
			.size = 256
		},
		.mt = NULL,
		.isplChar = false
	};

	plFGets(&plStringBuf, dispSize);
	convertBuf = strtok(stringBuf, ",\n");
	fbinfo.displaySize[0] = strtol(convertBuf, &strtolBuf, 10);
	convertBuf = strtok(NULL, ",\n");
	fbinfo.displaySize[1] = strtol(convertBuf, &strtolBuf, 10);
	plFClose(dispSize);

	plFGets(&plStringBuf, bitsPerPixel);
	fbinfo.bytesPerPixel = strtol(stringBuf, &strtolBuf, 10) / 8;
	plFClose(bitsPerPixel);

	plFGets(&plStringBuf, strideSize);
	fbinfo.scanlineSize = strtol(stringBuf, &strtolBuf, 10) / fbinfo.bytesPerPixel;
	plFClose(strideSize);

	pfbmem = mmap(NULL, fbinfo.scanlineSize * fbinfo.displaySize[1] * fbinfo.bytesPerPixel, PROT_WRITE | PROT_READ, MAP_SHARED, fb, 0);
	if(pfbmem == MAP_FAILED)
		plTKPanic("plTKInit", PLTK_FB_FAILED, true);

	fbmem = plMTAlloc(mt, fbinfo.scanlineSize * fbinfo.displaySize[1] * fbinfo.bytesPerPixel);
	plTKFBClear(0, 0, fbinfo.displaySize[0] - 1, fbinfo.displaySize[1] - 1, true);

	tcgetattr(STDIN_FILENO, &fbinfo.termMode);
	struct termios currentMode;
	currentMode.c_iflag &= ~(IGNBRK | BRKINT | ISTRIP);
	currentMode.c_lflag &= ~(ICANON | ECHO | ISIG | ECHONL | IEXTEN);
	currentMode.c_oflag &= ~OPOST;
	currentMode.c_cflag &= ~(CSIZE | PARENB);
	currentMode.c_cflag |= CS8;
	currentMode.c_cc[VMIN] = 1;
	currentMode.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &currentMode);
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void plTKStop(){
	if(pfbmem == NULL || fbmem == NULL)
		plTKPanic("plTKStop", PLTK_NOT_INITIALIZED, true);

	backgroundColor = 0;
	plTKFBClear(0, 0, fbinfo.displaySize[0] - 1, fbinfo.displaySize[1] - 1, true);

	plMTStop(mt);
	munmap(pfbmem, fbinfo.scanlineSize * fbinfo.displaySize[1] * fbinfo.bytesPerPixel);
	close(fb);

	fb = -2;
	pfbmem = NULL;
	fbmem = NULL;

	uint8_t buffer[4096];
	while(read(STDIN_FILENO, buffer, 4096) != -1);

	fcntl(STDIN_FILENO, F_SETFL, 0);
	tcsetattr(STDIN_FILENO, 0, &fbinfo.termMode);

	fputs("\x1b[2J\x1b[1;1H\x1b[?25h", stdout);
	fflush(stdout);
}

void plTKSetRenderDelay(uint32_t microseconds){
	while(microseconds > 1000000){
		renderDelay.tv_sec++;
		microseconds -= 1000000;
	}
	renderDelay.tv_nsec = microseconds * 1000;
}

bool plTKIsFBReady(){
	if(fb < 0 || pfbmem == NULL || fbmem == NULL)
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

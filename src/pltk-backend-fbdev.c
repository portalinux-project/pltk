#include <pltk-backend.h>
#include <fcntl.h>
#include <linux/fbdev.h>

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
	uint16_t dimension[2];
	uint8_t* windowBuffer;
	bool inFocus;
	bool disableWindowBar;
	uint16_t windowId;
};

void plTKInit(){
	fb = open("/dev/fb0", O_RDWR);
	if(fb == -1)
		plPanic("plTKInit", true, false);

	plfile_t* dispSize = plFOpen("/sys/class/graphics/fb0/virtual_size");
	plfile_t* strideSize = plFOpen("/sys/class/graphics/fb0/stride");
	plfile_t* bitsPerPixel = plFOpen("/sys/class/graphics/fb0/bits_per_pixel");
	char stringBuf[256] = "";
	char* convertBuf;
	char* strtolBuf;

	plFGets(stringBuf, 256, dispSize);
	convertBuf = strtok(stringBuf, ",\n");
	displaySize[0] = strtol(convertBuf, &strtolBuf, 10);
	convertBuf = strtok(NULL, ",\n");
	diplaySize[1] = strtol(convertBuf, &strtolBuf, 10);
	plFClose(dispSize);

	plFGets(stringBuf, 256, strideSize);
	scanlineSize = strtol(stringBuf, &strtolBuf, 10);
	plFClose(strideSize);

	plFGets(stringBuf, 256, bitsPerPixel);
	bytesPerPixel = strtol(stringBuf, &strtolBuf, 10) / 8;
	plFClose(bitsPerPixel);

	fbmem = mmap(NULL, scanlineSize * displaySize[1] * bytesPerPixel, PROT_WRITE, MAP_SHARED, fb, 0);
	if(fbmem == NULL)
		plPanic("plTKInit: Failed to map framebuffer to memory");
}

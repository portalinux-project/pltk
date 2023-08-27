#include "../../../include/pltk-core.h"
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/input-event-codes.h>

struct pltkwindow {
	uint16_t position[2];
	uint16_t dimensions[2];
	uint8_t* windowBuffer;
	uint16_t windowId;
	plmt_t* mt;
};

typedef struct pltkfbinfo {
	uint16_t displaySize[2];
	uint16_t scanlineSize;
	uint8_t bytesPerPixel;
} pltkfbinfo_t;

void plTKFBWrite(pltkdata_t* data, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, bool refreshBuffer);
void plTKFBClear(uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, bool refreshBuffer);
bool plTKIsFBReady();
pltkfbinfo_t plTKFBGetInfo();
void plTKFBSetBackground(uint8_t color);
uint16_t plTKFBGetNewWindowID();

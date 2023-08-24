#include "fbdev-api.h"

void plTKPanic(string_t string, bool usePerror, bool devBug){
	pltkfbinfo_t fbinfo = plTKFBGetInfo();
	if(plTKIsFBReady())
		plTKFBClear(0, 0, fbinfo.displaySize[0] - 1, fbinfo.displaySize[1] - 1);

	fputs("\x1b[2J\x1b[?25h", stdout);
	fflush(stdout);

	plPanic(string, usePerror, devBug);
}

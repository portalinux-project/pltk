#include <pltk-core.h>

void plTKPanic(string_t string, bool usePerror, bool devBug){
	if(fb != -1 && fbmem != MAP_FAILED){
		backgroundColor = 0;
		plTKFBClear(0, 0, fbinfo.displaySize[0] - 1, fbinfo.displaySize[1] - 1);
	}

	fputs("\x1b[2J\x1b[?25h", stdout);
	fflush(stdout);

	plPanic(string, usePerror, devBug);
}

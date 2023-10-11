#include "../framebuffer-api.h"

void plTKPanic(char* string, unsigned long errCode, bool isDeveloperBug){
	pltkfbinfo_t fbinfo = plTKFBGetInfo();
	if(plTKIsFBReady()){
		plTKFBSetBackground(0);
		plTKFBClear(0, 0, fbinfo.displaySize[0] - 1, fbinfo.displaySize[1] - 1, true);
	}

	tcsetattr(STDIN_FILENO, TCSANOW, &fbinfo.termMode);
	fcntl(STDIN_FILENO, F_SETFL, 0);
	fputs("\x1b[2J\x1b[?25h", stdout);
	fflush(stdout);

	if((errCode & PLRT_ERROR) == 0){
		printf("* PLTK Runtime Error at %s: ", string);
		switch(errCode){
			case PLTK_NOT_INITIALIZED:
				printf("PLTK has not been initialized");
				break;
			case PLTK_FB_FAILED:
				printf("Mapping framebuffer to memory has failed");
				break;
			case PLTK_FB_MISMATCHED_BITDEPTH:
				printf("Mismached bitness between data buffer and framebuffer");
				break;
			case PLTK_INVALID_BUFFER:
				printf("Given data buffer is either empty or invalid");
				break;
		}
		printf("\n");
		string = NULL;
	}

	plRTPanic(string, errCode, isDeveloperBug);
}

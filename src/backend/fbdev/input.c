#include <pltk-backend.h>
#include <unistd.h>
#include <dirent.h>

struct pltkinput {
	pltkitype_t type;
	int fd;
	plmt_t* mt;
};

pltkinput_t* plTKInputInit(pltkitype_t inputType, char* specificDevice, plmt_t* mt){
	DIR* inputDir = opendir("/sys/class/input")
	struct dirent* dirEntry;
	pltkinput_t* retStruct = plMTAllocE(mt, sizeof(pltkinput_t));
	retStruct->fd = -1;

	while((dirEntry = readdir(inputDir)) != NULL){
		char buffer[255] = "";
		snprintf(buffer, 255, "/sys/class/input/%s/name", dirEntry->d_name);

		if(strstr(buffer, "event") != NULL){
			int fileHandle = open(buffer, O_RDONLY);
			read(fileHandle, buffer, 255);
			close(fileHandle);

			switch(devType){
				case PLTK_KEYBOARD:
					retStruct->type = PLTK_KEYBOARD;
					if(strstr(buffer, "keyboard") != NULL){
						snprintf(buffer, 255, "/dev/input/%s", dirEntry->d_name);
						retStruct->fd = open(buffer, O_RDONLY | O_NONBLOCK);
					}
					break;
				case PLTK_POINTER:
					break;
		}
	}

	if(retStruct->fd == -1){
		plMTFree(mt, retStruct);
		return NULL;
	}

	retStruct->mt = mt;
	return retStruct;
}

void plTKInputClose(pltkinput_t* inputDevice){
	close(inputDevice->fd);
	plMTFree(inputDevice->mt, inputDevice);
}

int32_t plTKGetInput(pltkinput_t* inputDevice){
	switch(inputDevice->type){
		case PLTK_KEYBOARD:
			read()
			break;
	}
}


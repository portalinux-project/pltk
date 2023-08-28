#include "fbdev-api.h"

struct pltkinput {
	pltkitype_t type;
	bool nonblock;
	int fd;
	plmt_t* mt;
};

typedef struct pltklinuxevent {
	struct timeval time;
	unsigned short type;
	unsigned short code;
	unsigned int value;
} pltklinuxevent_t;

pltkievent_t plTKEvdevEventTranslator(pltklinuxevent_t* rawEvent){
	switch(rawEvent->type){
		case EV_SYN:
			return PLTK_SYNC;
		case EV_KEY:
			if(rawEvent->value == 0)
				return PLTK_KEYUP;
			else
				return PLTK_KEYDOWN;
			break;
		case EV_REL:
			switch(rawEvent->code){
				case REL_X:
					return PLTK_REL_X;
				case REL_Y:
					return PLTK_REL_Y;
				case REL_WHEEL:
					return PLTK_REL_WHEEL;
			}
			break;
		case EV_ABS:
			switch(rawEvent->code){
				case ABS_X:
					return PLTK_ABS_X;
				case ABS_Y:
					return PLTK_ABS_Y;
				case ABS_WHEEL:
					return PLTK_ABS_WHEEL;
			}
			break;
	}

	return PLTK_ERROR;
}

pltkkey_t plTKEvdevKeyTranslator(pltklinuxevent_t* rawEvent){
	switch(rawEvent->code){
		case KEY_ESC:
			return PLTK_KEY_ESC;
		case KEY_1:
			return PLTK_KEY_1;
		case KEY_2:
			return PLTK_KEY_2;
		case KEY_3:
			return PLTK_KEY_3;
		case KEY_4:
			return PLTK_KEY_4;
		case KEY_5:
			return PLTK_KEY_5;
		case KEY_6:
			return PLTK_KEY_6;
		case KEY_7:
			return PLTK_KEY_7;
		case KEY_8:
			return PLTK_KEY_8;
		case KEY_9:
			return PLTK_KEY_9;
		case KEY_0:
			return PLTK_KEY_0;
		case KEY_MINUS:
			return PLTK_KEY_MINUS;
		case KEY_EQUAL:
			return PLTK_KEY_EQUAL;
		case KEY_BACKSPACE:
			return PLTK_KEY_BACKSPACE;
		case KEY_TAB:
			return PLTK_KEY_TAB;
		case KEY_Q:
			return PLTK_KEY_Q;
		case KEY_W:
			return PLTK_KEY_W;
		case KEY_E:
			return PLTK_KEY_E;
		case KEY_R:
			return PLTK_KEY_R;
		case KEY_T:
			return PLTK_KEY_T;
		case KEY_Y:
			return PLTK_KEY_Y;
		case KEY_U:
			return PLTK_KEY_U;
		case KEY_I:
			return PLTK_KEY_I;
		case KEY_O:
			return PLTK_KEY_O;
		case KEY_P:
			return PLTK_KEY_P;
		case KEY_LEFTBRACE:
			return PLTK_KEY_LEFTBRACE;
		case KEY_RIGHTBRACE:
			return PLTK_KEY_RIGHTBRACE;
		case KEY_ENTER:
			return PLTK_KEY_ENTER;
		case KEY_LEFTCTRL:
			return PLTK_KEY_LEFTCTRL;
		case KEY_A:
			return PLTK_KEY_A;
		case KEY_S:
			return PLTK_KEY_S;
		case KEY_D:
			return PLTK_KEY_D;
		case KEY_F:
			return PLTK_KEY_F;
		case KEY_G:
			return PLTK_KEY_G;
		case KEY_H:
			return PLTK_KEY_H;
		case KEY_J:
			return PLTK_KEY_J;
		case KEY_K:
			return PLTK_KEY_K;
		case KEY_L:
			return PLTK_KEY_L;
		case KEY_SEMICOLON:
			return PLTK_KEY_SEMICOLON;
		case KEY_APOSTROPHE:
			return PLTK_KEY_APOSTROPHE;
		case KEY_GRAVE:
			return PLTK_KEY_GRAVE;
		case KEY_LEFTSHIFT:
			return PLTK_KEY_LEFTSHIFT;
		case KEY_BACKSLASH:
			return PLTK_KEY_BACKSLASH;
		case KEY_Z:
			return PLTK_KEY_Z;
		case KEY_X:
			return PLTK_KEY_X;
		case KEY_C:
			return PLTK_KEY_C;
		case KEY_V:
			return PLTK_KEY_V;
		case KEY_B:
			return PLTK_KEY_B;
		case KEY_N:
			return PLTK_KEY_N;
		case KEY_M:
			return PLTK_KEY_M;
		case KEY_COMMA:
			return PLTK_KEY_COMMA;
		case KEY_DOT:
			return PLTK_KEY_DOT;
		case KEY_SLASH:
			return PLTK_KEY_SLASH;
		case KEY_RIGHTSHIFT:
			return PLTK_KEY_RIGHTSHIFT;
		case KEY_KPASTERISK:
			return PLTK_KEY_KPASTERISK;
		case KEY_LEFTALT:
			return PLTK_KEY_LEFTALT;
		case KEY_SPACE:
			return PLTK_KEY_SPACE;
		case KEY_CAPSLOCK:
			return PLTK_KEY_CAPSLOCK;
		case KEY_F1:
			return PLTK_KEY_F1;
		case KEY_F2:
			return PLTK_KEY_F2;
		case KEY_F3:
			return PLTK_KEY_F3;
		case KEY_F4:
			return PLTK_KEY_F4;
		case KEY_F5:
			return PLTK_KEY_F5;
		case KEY_F6:
			return PLTK_KEY_F6;
		case KEY_F7:
			return PLTK_KEY_F7;
		case KEY_F8:
			return PLTK_KEY_F8;
		case KEY_F9:
			return PLTK_KEY_F9;
		case KEY_F10:
			return PLTK_KEY_F10;
		case KEY_NUMLOCK:
			return PLTK_KEY_NUMLOCK;
		case KEY_SCROLLLOCK:
			return PLTK_KEY_SCROLLLOCK;
		case KEY_KP7:
			return PLTK_KEY_KP7;
		case KEY_KP8:
			return PLTK_KEY_KP8;
		case KEY_KP9:
			return PLTK_KEY_KP9;
		case KEY_KPMINUS:
			return PLTK_KEY_KPMINUS;
		case KEY_KP4:
			return PLTK_KEY_KP4;
		case KEY_KP5:
			return PLTK_KEY_KP5;
		case KEY_KP6:
			return PLTK_KEY_KP6;
		case KEY_KPPLUS:
			return PLTK_KEY_KPPLUS;
		case KEY_KP1:
			return PLTK_KEY_KP1;
		case KEY_KP2:
			return PLTK_KEY_KP2;
		case KEY_KP3:
			return PLTK_KEY_KP3;
		case KEY_KP0:
			return PLTK_KEY_KP0;
		case KEY_KPDOT:
			return PLTK_KEY_KPDOT;
		case KEY_102ND:
			return PLTK_KEY_102ND;
		case KEY_F11:
			return PLTK_KEY_F11;
		case KEY_F12:
			return PLTK_KEY_F12;
		case KEY_KPENTER:
			return PLTK_KEY_KPENTER;
		case KEY_RIGHTCTRL:
			return PLTK_KEY_RIGHTCTRL;
		case KEY_KPSLASH:
			return PLTK_KEY_KPSLASH;
		case KEY_SYSRQ:
			return PLTK_KEY_SYSRQ;
		case KEY_RIGHTALT:
			return PLTK_KEY_RIGHTALT;
		case KEY_HOME:
			return PLTK_KEY_HOME;
		case KEY_UP:
			return PLTK_KEY_UP;
		case KEY_PAGEUP:
			return PLTK_KEY_PAGEUP;
		case KEY_LEFT:
			return PLTK_KEY_LEFT;
		case KEY_RIGHT:
			return PLTK_KEY_RIGHT;
		case KEY_END:
			return PLTK_KEY_END;
		case KEY_DOWN:
			return PLTK_KEY_DOWN;
		case KEY_PAGEDOWN:
			return PLTK_KEY_PAGEDOWN;
		case KEY_INSERT:
			return PLTK_KEY_INSERT;
		case KEY_DELETE:
			return PLTK_KEY_DELETE;
		case BTN_LEFT:
			return PLTK_KEY_LEFTCLICK;
		case BTN_RIGHT:
			return PLTK_KEY_RIGHTCLICK;
		case BTN_MIDDLE:
			return PLTK_KEY_MIDDLECLICK;
		default:
			return PLTK_KEY_ERROR;
	}
}

pltkinput_t* plTKInputInit(pltkitype_t inputType, char* specificDevice, bool nonblock, plmt_t* mt){
	if(specificDevice == NULL || mt == NULL)
		plTKPanic("plTKInputInit: NULL pointers were given!", false, true);

	pltkinput_t* retStruct = plMTAllocE(mt, sizeof(pltkinput_t));
	char buffer[255] = "";
	int openFlags = O_RDONLY;
	retStruct->fd = -1;
	retStruct->nonblock = nonblock;

	snprintf(buffer, 255, "/dev/input/%s", specificDevice);
	if(nonblock == true)
		openFlags = O_RDONLY | O_NONBLOCK;

	retStruct->fd = open(buffer, openFlags);
	if(retStruct->fd == -1){
		plMTFree(mt, retStruct);
		return NULL;
	}

	retStruct->type = inputType;
	retStruct->mt = mt;

	return retStruct;
}

void plTKInputClose(pltkinput_t* inputDevice){
	pltklinuxevent_t junk;
	if(inputDevice->nonblock)
		while(read(inputDevice->fd, &junk, sizeof(pltklinuxevent_t)) != -1);

	close(inputDevice->fd);
	plMTFree(inputDevice->mt, inputDevice);
}

pltkevent_t plTKGetInput(pltkinput_t* inputDevice){
	pltklinuxevent_t rawEvent;
	int success = read(inputDevice->fd, &rawEvent, sizeof(pltklinuxevent_t));
	pltkevent_t retStruct = {
		.type = PLTK_ERROR,
		.value = 0
	};

	if(success < 0)
		return retStruct;

	retStruct.type = plTKEvdevEventTranslator(&rawEvent);
	switch(retStruct.type){
		case PLTK_KEYUP:
		case PLTK_KEYDOWN:
			retStruct.value = plTKEvdevKeyTranslator(&rawEvent);
			break;
		default:
			retStruct.value = rawEvent.value;
			break;
	}

	retStruct.time.tv_sec = rawEvent.time.tv_sec;
	retStruct.time.tv_nsec = rawEvent.time.tv_usec * 1000;
	return retStruct;
}

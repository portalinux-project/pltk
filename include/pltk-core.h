#include <pl32-memory.h>
#include <pl32-file.h>
#include <time.h>

typedef struct pltkwindow pltkwindow_t;
typedef struct pltkinput pltkinput_t;
typedef plchar_t pltkcolor_t;

typedef struct pltkdata {
	plfatptr_t dataPtr;
	uint8_t bytesPerPixel;
	bool ignoreZero;
} pltkdata_t;

typedef struct pltkfont {
	pltkdata_t* data;
	uint16_t fontSize[2];
	bool utf8Support;
} pltkfont_t;

typedef enum pltkitype {
	PLTK_NONE,
	PLTK_KEYBOARD,
	PLTK_POINTER,
	PLTK_CUSTOM
} pltkitype_t;

typedef enum pltkievent {
	PLTK_ERROR,
	PLTK_SYNC,
	PLTK_KEYDOWN,
	PLTK_KEYUP,
	PLTK_REL_X,
	PLTK_REL_Y,
	PLTK_REL_WHEEL,
	PLTK_ABS_X,
	PLTK_ABS_Y,
	PLTK_ABS_WHEEL
} pltkievent_t;

typedef enum pltkkey {
	PLTK_KEY_ERROR,
	PLTK_KEY_ESC,
	PLTK_KEY_1,
	PLTK_KEY_2,
	PLTK_KEY_3,
	PLTK_KEY_4,
	PLTK_KEY_5,
	PLTK_KEY_6,
	PLTK_KEY_7,
	PLTK_KEY_8,
	PLTK_KEY_9,
	PLTK_KEY_0,
	PLTK_KEY_MINUS,
	PLTK_KEY_EQUAL,
	PLTK_KEY_BACKSPACE,
	PLTK_KEY_TAB,
	PLTK_KEY_Q,
	PLTK_KEY_W,
	PLTK_KEY_E,
	PLTK_KEY_R,
	PLTK_KEY_T,
	PLTK_KEY_Y,
	PLTK_KEY_U,
	PLTK_KEY_I,
	PLTK_KEY_O,
	PLTK_KEY_P,
	PLTK_KEY_LEFTBRACE,
	PLTK_KEY_RIGHTBRACE,
	PLTK_KEY_ENTER,
	PLTK_KEY_LEFTCTRL,
	PLTK_KEY_A,
	PLTK_KEY_S,
	PLTK_KEY_D,
	PLTK_KEY_F,
	PLTK_KEY_G,
	PLTK_KEY_H,
	PLTK_KEY_J,
	PLTK_KEY_K,
	PLTK_KEY_L,
	PLTK_KEY_SEMICOLON,
	PLTK_KEY_APOSTROPHE,
	PLTK_KEY_GRAVE,
	PLTK_KEY_LEFTSHIFT,
	PLTK_KEY_BACKSLASH,
	PLTK_KEY_Z,
	PLTK_KEY_X,
	PLTK_KEY_C,
	PLTK_KEY_V,
	PLTK_KEY_B,
	PLTK_KEY_N,
	PLTK_KEY_M,
	PLTK_KEY_COMMA,
	PLTK_KEY_DOT,
	PLTK_KEY_SLASH,
	PLTK_KEY_RIGHTSHIFT,
	PLTK_KEY_KPASTERISK,
	PLTK_KEY_LEFTALT,
	PLTK_KEY_SPACE,
	PLTK_KEY_CAPSLOCK,
	PLTK_KEY_F1,
	PLTK_KEY_F2,
	PLTK_KEY_F3,
	PLTK_KEY_F4,
	PLTK_KEY_F5,
	PLTK_KEY_F6,
	PLTK_KEY_F7,
	PLTK_KEY_F8,
	PLTK_KEY_F9,
	PLTK_KEY_F10,
	PLTK_KEY_NUMLOCK,
	PLTK_KEY_SCROLLLOCK,
	PLTK_KEY_KP7,
	PLTK_KEY_KP8,
	PLTK_KEY_KP9,
	PLTK_KEY_KPMINUS,
	PLTK_KEY_KP4,
	PLTK_KEY_KP5,
	PLTK_KEY_KP6,
	PLTK_KEY_KPPLUS,
	PLTK_KEY_KP1,
	PLTK_KEY_KP2,
	PLTK_KEY_KP3,
	PLTK_KEY_KP0,
	PLTK_KEY_KPDOT,
	PLTK_KEY_102ND,
	PLTK_KEY_F11,
	PLTK_KEY_F12,
	PLTK_KEY_KPENTER,
	PLTK_KEY_RIGHTCTRL,
	PLTK_KEY_KPSLASH,
	PLTK_KEY_SYSRQ,
	PLTK_KEY_RIGHTALT,
	PLTK_KEY_HOME,
	PLTK_KEY_UP,
	PLTK_KEY_PAGEUP,
	PLTK_KEY_LEFT,
	PLTK_KEY_RIGHT,
	PLTK_KEY_END,
	PLTK_KEY_DOWN,
	PLTK_KEY_PAGEDOWN,
	PLTK_KEY_INSERT,
	PLTK_KEY_DELETE,
	PLTK_KEY_LEFTCLICK,
	PLTK_KEY_RIGHTCLICK,
	PLTK_KEY_MIDDLECLICK,
	PLTK_KEY_FORWARDS,
	PLTK_KEY_BACK
} pltkkey_t;

typedef struct pltkevent {
	struct timespec time;
	pltkievent_t type;
	unsigned int value;
} pltkevent_t;

void plTKPanic(string_t string, bool usePerror, bool devBug);

void plTKInit(uint8_t screen);
void plTKStop();
void plTKSetRenderDelay(uint32_t microseconds);

pltkwindow_t* plTKCreateWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void plTKWindowClose(pltkwindow_t* window);

void plTKWindowRender(pltkwindow_t* window);
void plTKWindowMove(pltkwindow_t* window, uint16_t x, uint16_t y);
void plTKWindowPixel(pltkwindow_t* window, uint16_t x, uint16_t y, pltkcolor_t color);
void plTKWindowLine(pltkwindow_t* window, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkcolor_t color);
void plTKWindowFBWrite(pltkwindow_t* window, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop, pltkdata_t* data);
void plTKWindowRenderFont(pltkwindow_t* window, uint16_t x, uint16_t y, pltkfont_t font, uint32_t index, pltkcolor_t color);

pltkinput_t* plTKInputInit(pltkitype_t inputType, char* specificDevice, bool nonblock, plmt_t* mt);
void plTKInputClose(pltkinput_t* inputDevice);
pltkevent_t plTKGetInput(pltkinput_t* inputDevice);

#include <pltk-backend.h>
#include <wayland-client.h>

struct wl_display* display = NULL;
struct wl_compositor* compositor = NULL;
size_t displaySize[2];

struct pltkwindow {
	uint16_t pos[2];
	uint16_t size[2];
};

void plTKInit(){
	display = wl_display_connect(NULL);
	if(display == NULL)
		plPanic("plTKInit: Failed to get a display", false, false);
}

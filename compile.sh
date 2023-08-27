#!/bin/sh
# Dummy build system so that it can at least build for now :3

set -e
clang -Iinclude -g -shared -fPIC -lpl32 -lm src/backend/fbdev/common.c src/backend/fbdev/renderer.c src/backend/fbdev/window.c src/backend/fbdev/input.c -o libpltk-core.so
clang -Iinclude -g -L. -lpl32 -lpltk-core pltk-test.c -o pltk-test.out
#clang -Iinclude -g -L. -lpl32 -lpltk-core pltk-input-test.c -o pltk-input-test.out

if [ $(id -u) -ne 0 ] || [ "$DISPLAY" != "" ] || [ "$XDG_RUNTIME_DIR" != "" ]; then
	echo "To test this program, run this line as root on a Linux framebuffer terminal:"
	echo "LD_LIBRARY_PATH=. ./pltk-test.out"
else
	LD_LIBRARY_PATH=. ./pltk-test.out
fi

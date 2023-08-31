#!/bin/sh
# Dummy build system so that it can at least build for now :3

set -e
clang -Iinclude -Os -shared -fPIC -lpl32 -lm src/backend/fbdev/common.c src/backend/fbdev/renderer.c src/backend/fbdev/window.c src/backend/fbdev/input.c -o libpltk-core.so
clang -Iinclude -g -L. -shared -fPIC -lpl32 -lpltk-core src/backend/fbdev/basic.c -o libpltk-basic.so
clang -Iinclude -Os -L. -lpl32 -lpltk-core pltk-test.c -o pltk-test.out
clang -Iinclude -g -L. -lpl32 -lpltk-core -lpltk-basic pltk-test-basic.c -o pltk-test-basic.out

if [ $(id -u) -ne 0 ] || [ "$DISPLAY" != "" ] || [ "$XDG_RUNTIME_DIR" != "" ]; then
	echo "To test this program, run this line as root on a Linux framebuffer terminal:"
	echo "LD_LIBRARY_PATH=. ./pltk-test.out"
else
	LD_LIBRARY_PATH=. ./pltk-test.out
fi

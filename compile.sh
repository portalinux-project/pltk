#!/bin/sh
# Dummy build system so that it can at least build for now :3

set -e
clang -Iinclude -g -shared -fPIC -lpl32 -lm src/backend/fbdev/renderer.c src/backend/fbdev/input.c -o libpltk-backend.so
clang -Iinclude -g -L. -lpltk-backend pltk-test.c -o pltk-test.out

if [ $(id -u) -ne 0 ] || [ "$DISPLAY" != "" ] || [ "$XDG_RUNTIME_DIR" != "" ]; then
	echo "To test this program, run this line as root on a Linux framebuffer terminal:"
	echo "LD_LIBRARY_PATH=. ./pltk-test.out"
else
	LD_LIBRARY_PATH=. ./pltk-test.out
fi

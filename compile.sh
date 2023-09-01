#!/bin/sh
# Dummy build system so that it can at least build for now :3

set -e
clang -Iinclude -Os -shared -fPIC -lpl32 -lm src/core/framebuffer/fbdev/common.c src/core/framebuffer/fbdev/renderer.c src/core/framebuffer/fbdev/window.c src/core/framebuffer/fbdev/input.c -o libpltk-core.so
clang -Iinclude -Os -L. -shared -fPIC -lpl32 -lpltk-core src/basic/framebuffer/basic.c -o libpltk-basic.so
clang -Iinclude -Os -L. -lpl32 -lpltk-core pltk-test.c -o pltk-core-test.out
clang -Iinclude -Os -L. -lpl32 -lpltk-core -lpltk-basic pltk-basic-test.c -o pltk-basic-test.out

if [ $(id -u) -ne 0 ] || [ "$DISPLAY" != "" ] || [ "$XDG_RUNTIME_DIR" != "" ]; then
	echo "To test this program, run this line as root on a Linux framebuffer terminal:"
	echo "LD_LIBRARY_PATH=. ./pltk-test.out"
else
	LD_LIBRARY_PATH=. ./pltk-test.out
fi

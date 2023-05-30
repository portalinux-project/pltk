#!/bin/sh
# Dummy build system so that it can at least build for now :3

set -e
clang -Iinclude -g -shared -fPIC -lpl32 -lm src/pltk-backend-fbdev.c -o libpltk-backend.so
clang -Iinclude -g -L. -lpltk-backend pltk-test.c -o pltk-test

if [ $(id -u) -ne 0 ]; then
	echo "To test this program, run this line as root on a Linux framebuffer terminal:"
	echo "LD_LIBRARY_PATH=. ./pltk-test"
else
	LD_LIBRARY_PATH=. ./pltk-test
fi

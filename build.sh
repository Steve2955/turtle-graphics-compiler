#!/bin/bash

if ! command -v sdl2-config &> /dev/null
then
    apt install libsdl2-dev libsdl2-2.0-0 libjpeg-dev libwebp-dev libtiff5-dev libsdl2-image-dev libsdl2-image-2.0-0 -y
fi
mkdir -p build
mkdir -p build/turtle
gcc `sdl2-config --cflags --libs` -lm -o build/turtle/output.out src/sdl/sdlinterf.c src/main.c src/eval.c src/parse.c src/lex.c src/nametab.c -lSDL2main -lSDL2 -Wl,--no-undefined -lm -luuid -static-libgcc
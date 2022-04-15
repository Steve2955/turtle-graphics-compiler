:: Create build-folder if it does not exist
mkdir build

:: Compile project
gcc -I src\sdl\SDL2\include ^
    -L src\sdl\SDL2\lib -static ^
    -o build\turtle ^
    src\sdl\sdlinterf.c src\main.c src\eval.c src\parse.c src\lex.c src\nametab.c ^
    -lmingw32 -lSDL2main -lSDL2 -Wl,--no-undefined ^
    -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 ^
    -lole32 -loleaut32 -lshell32 -lversion -luuid -lsetupapi -lhid -static-libgcc

:: Copy examples to build-folder for testing
copy /Y test\examples\* build\

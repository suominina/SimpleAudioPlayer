CC = gcc
CFLAGS = -pedantic -g `pkg-config --cflags SDL2_mixer`
LIBS = -lSDL2 `pkg-config --libs SDL2_mixer`

final: src/main.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

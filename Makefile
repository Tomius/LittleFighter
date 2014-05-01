all:
	gcc *.c *.h -o LittleFighter `sdl-config --cflags --libs` -lSDL_gfx -lSDL_ttf -lSDL_image -lSDL_mixer -lm

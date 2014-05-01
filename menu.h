#ifndef MENU_H
#define MENU_H
#include <SDL.h>

enum menu_type {None, Quit, Death};
extern enum menu_type menu;

int Draw_menu(SDL_Surface *screen);
void Loading_screen(SDL_Surface *screen);
int Select_Difficulty(SDL_Surface *screen);
int Quit_menu(SDL_Surface *screen, int *mouse_x, int *mouse_y);
int Death_menu(SDL_Surface *screen, int enemies_killed, int *mouse_x, int *mouse_y);
void Credits_menu(SDL_Surface *screen);

#endif //MENU_H

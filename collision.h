#ifndef COLLISION_H
#define COLLISION_H

#include <SDL.h>
#include "character.h"

enum rect_functions {Intersection, Union};
SDL_Rect Rect_Function(enum rect_functions checkfor, SDL_Rect rectA, SDL_Rect rectB);
void Collison_Detection(struct character *chars);
void Animate_collision(struct character *chars);


#endif // COLLISION_H

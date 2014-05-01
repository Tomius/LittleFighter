#ifndef ANIMATE_H
#define ANIMATE_H

#include "character.h"
#include "projectile.h"

void Animate_actions(int Tick, struct character *chars, struct projectile **projs);
void Draw_characters(struct character *players, SDL_Surface *screen);
void Reorder_for_draw_characters (struct character **chars);
SDL_Surface * Current_action(struct character *iter);

#endif // ANIMATE_H

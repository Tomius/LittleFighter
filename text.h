#ifndef TEXT_H
#define TEXT_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "character.h"
#include "projectile.h"

void Init_fonts();
extern TTF_Font *Knig11, *Knig35, *Knig60;
extern TTF_Font *Cour10, *Cour12, *Cour15, *Cour20, *Cour20B, *Cour25, *Cour35B, *Cour45;
SDL_Surface * RenderText(TTF_Font *font, char *c, SDL_Color color);
void Text_bubble(struct character *who, char *c1, char *c2, char *c3, int ms, SDL_Surface *screen, struct character *chars, struct character *player, struct projectile **projs);
void Draw_text_bubble (SDL_Surface *screen, struct character *player, char *c1, char *c2, char *c3);
void Close_Fonts();

#endif //TEXT_H

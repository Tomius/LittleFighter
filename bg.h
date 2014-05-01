#ifndef BG_H
#define BG_H
#include <SDL.h>
#include "character.h"
#include "projectile.h"

extern SDL_Rect TerrainRect;
extern SDL_Surface *shadow;
extern enum maps current_map;
extern int TerrainMax;
enum maps {forest, prison, vallenfyre, colisseum, practise_arena};

void Init_bg();
void Draw_background(SDL_Surface *screen);
void Draw_bg(SDL_Surface *screen);
void Draw_background2(SDL_Surface *screen);
void Animate_background(int Tick, struct character *chars, struct projectile *projs, struct character *player);
void Free_background(void);
void Reset_map(void);

#endif //BG_H

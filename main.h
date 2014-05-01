#ifndef MAIN_H
#define MAIN_H
#include <SDL.h>
#include "character.h"
#include "projectile.h"


enum mapsize {base_kx = 1024 , base_ky = 576};
enum {base_framesize = 80};

extern int kx, ky, kyTop, kyBottom, framesize;
extern double scale;
extern bool practise, top, benchmark;
extern int aNextTick, dNextTick, fps;
extern const int aFPS;
extern Uint32 paused;
extern int screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask;

Uint32 GetTicks(void);

#endif //MAIN_H

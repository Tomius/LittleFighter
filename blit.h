#ifndef BLIT_H
#define BLIT_H
#include <SDL.h>

typedef struct Rect {
    SDL_Rect rect[500];
    int size;
} Rect;
extern Rect rectList_old;

inline void Rect_add(Rect *list, const SDL_Rect *rect);
void BlitSurface (SDL_Surface *src, SDL_Rect *srcRect, SDL_Surface *dest, SDL_Rect *destRect);
void Flip(void *screen);
void Wait_Flip_Thread();

#endif

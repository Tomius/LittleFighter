#ifndef IMGLOAD_H
#define IMGLOAD_H
#include <SDL.h>

SDL_Surface * ImgLoader(char *file);
void Adv_ImgLoader(SDL_Surface **left, SDL_Surface **right, char *charname, char *skillname);
void AnimLoader(SDL_Surface ***left, SDL_Surface ***right, char *charname, int *maxindex, char *skillname);
void AnimLoader2(SDL_Surface ***left, SDL_Surface ***right, char *charname, int *maxindex, char *skillname);
SDL_Surface ** AnimCpy (SDL_Surface **src, int frame_num);
SDL_Surface ** AnimCpy_Alpha (SDL_Surface **src, int frame_num, int alpha);
SDL_Surface ** AnimCpy_AlphaScale (SDL_Surface **src, int frame_num, int alpha_start, int alpha_scale);
SDL_Surface * RotateSurface (SDL_Surface *src, double angle);
void RotateSurface2(SDL_Surface **src, double angle);
SDL_Surface * FlipSurface (SDL_Surface *src);
void SetSize (SDL_Surface **src, int x, int y);
void Scale (SDL_Surface **src);
void Scale_no_smooth (SDL_Surface **src);
SDL_Surface * ImgLoader_no_smooth(char *file);
void DisplayFormat( SDL_Surface **src);

#endif // IMGLOAD_H

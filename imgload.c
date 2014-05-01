#include <string.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include "main.h"
#include "character.h"
#include "imgload.h"

/** Betölt egy képet, úgy, hogy a teljesen fekete pixelek átlátszóak legyenek, átállitja
a kép formátumát az ablak megjelenitési formátumára, illetve felnagyitja a képet az ablak méretétől
függően, majd egy anti-aliasingot is meghív. Szinte az összes többi képbetöltő függvény ezen alapul */
SDL_Surface * ImgLoader(char *file) {
    SDL_Surface *pic;
    pic = IMG_Load(file);
    if(pic==NULL) {
        fprintf(stderr,"Missing image %s : %s\n",file,IMG_GetError());
        return NULL;
    }
    // Beállitja, hogy a milyen szinu pixelek legyenek atlatszoak
    SDL_SetColorKey(pic,SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(pic->format,0,0,0));

    // Felnagyitja a képet fullscreen-hez és átállitja a formatumtot
    Scale(&pic);

    return pic;
}

/** Egy adott karakter adott skilljét tölti be*/
void Adv_ImgLoader(SDL_Surface **left, SDL_Surface **right, char *charname, char *skillname) {
    char c[50];
    sprintf(c, "./sprite/%s/%s.gif", charname, skillname);
    *right = ImgLoader(c);
    if(*right == NULL)
    {
        *left = NULL;
        return;
    }
    // Tükrözi a képet vizszintesen
    *left = FlipSurface(*right);
}

/** Betölt egy 80*80-as frameket tartalmazó képet, majd visszaad
  * egy a frameket tartalmazó dinamikusan foglalt tömböt */
void AnimLoader(SDL_Surface ***left, SDL_Surface ***right, char *charname, int *maxindex, char *skillname) {
    if(!(*maxindex)) {
        *left = NULL;
        *right = NULL;
        return;
    }

    *left =  (SDL_Surface **) malloc( *maxindex * sizeof(SDL_Surface *) );
    *right =  (SDL_Surface **) malloc( *maxindex * sizeof(SDL_Surface *) );
    char c[50];
    sprintf(c, "./sprite/%s/%s.gif", charname, skillname);
    SDL_Surface *src_right = IMG_Load(c);

    // Ha hiányzik a kép
    if(src_right == NULL) {
        fprintf(stderr,"Missing image %s : %s\n",c,IMG_GetError());
        *maxindex = 0;
        *left = NULL;
        *right = NULL;
        return;
    }
    SDL_SetColorKey(src_right,SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(src_right->format,0,0,0));
    DisplayFormat(&src_right);

    // Tükrözi a képet vizszintesen
    SDL_Surface *src_left = FlipSurface(src_right);

    SDL_Rect srcRect_r = {0, 0, base_framesize, base_framesize}, srcRect_l = {src_left->w - base_framesize, 0, base_framesize, base_framesize};

    int i;
    for(i=0; i < *maxindex; i++) {
        // ELőször létrehozza a surface-t
        (*right)[i] = SDL_CreateRGBSurface(SDL_ANYFORMAT, base_framesize, base_framesize, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
        (*left)[i] = SDL_CreateRGBSurface(SDL_ANYFORMAT, base_framesize, base_framesize, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);

        // Majd rámásolja a framere a beolvasott kép megfelelő részét
        SDL_BlitSurface(src_right, &srcRect_r, (*right)[i], NULL);
        SDL_BlitSurface(src_left, &srcRect_l, (*left)[i], NULL);

        // Beállitja hogy a #000000 szinű pixelek feketék legyenek
        SDL_SetColorKey((*right)[i], SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB((*right)[i]->format,0,0,0));
        SDL_SetColorKey((*left)[i], SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB((*left)[i]->format,0,0,0));

        //Majd felnagyítja a képeket
        Scale(*right + i);
        Scale(*left + i);

        srcRect_r.x += base_framesize;
        if(srcRect_r.x == src_right->w) {
            srcRect_r.x = 0;
            srcRect_r.y += base_framesize;
        }

        srcRect_l.x -= base_framesize;
        if(srcRect_l.x < 0) {
            srcRect_l.x = src_left->w - base_framesize;
            srcRect_l.y += base_framesize;
        }
    }
    SDL_FreeSurface(src_left);
    SDL_FreeSurface(src_right);
}

/** U.a mint az előző fv, csak ez 160*160-as framekkel dolgozik */
void AnimLoader2(SDL_Surface ***left, SDL_Surface ***right, char *charname, int *maxindex, char *skillname) {
    if(!(*maxindex)) {
        *left = NULL;
        *right = NULL;
        return;
    }

    *left =  (SDL_Surface **) malloc( *maxindex * sizeof(SDL_Surface *) );
    *right =  (SDL_Surface **) malloc( *maxindex * sizeof(SDL_Surface *) );
    char c[50];
    sprintf(c, "./sprite/%s/%s.gif", charname, skillname);
    SDL_Surface *src_right = IMG_Load(c);

    // Ha hiányzik a kép
    if(src_right == NULL) {
        fprintf(stderr,"Missing image %s : %s\n",c,IMG_GetError());
        *maxindex = 0;
        *left = NULL;
        *right = NULL;
        return;
    }
    SDL_SetColorKey(src_right,SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(src_right->format,0,0,0));
    DisplayFormat(&src_right);

    // Tükrözi a képet vizszintesen
    SDL_Surface *src_left = FlipSurface(src_right);

    SDL_Rect srcRect_r = {0, 0, 2*base_framesize, 2*base_framesize}, srcRect_l = {src_left->w - 2*base_framesize, 0, 2*base_framesize, 2*base_framesize};

    int i;
    for(i=0; i < *maxindex; i++) {
        // ELőször létrehozza a surface-t
        (*right)[i] = SDL_CreateRGBSurface(SDL_ANYFORMAT, 2*base_framesize, 2*base_framesize, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
        (*left)[i] = SDL_CreateRGBSurface(SDL_ANYFORMAT, 2*base_framesize, 2*base_framesize, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);

        // Majd rámásolja a framere a beolvasott kép megfelelő részét
        SDL_BlitSurface(src_right, &srcRect_r, (*right)[i], NULL);
        SDL_BlitSurface(src_left, &srcRect_l, (*left)[i], NULL);

        // Beállitja hogy a #000000 szinű pixelek feketék legyenek
        SDL_SetColorKey((*right)[i], SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB((*right)[i]->format,0,0,0));
        SDL_SetColorKey((*left)[i], SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB((*left)[i]->format,0,0,0));

        //Majd felnagyítja a képeket
        Scale(*right + i);
        Scale(*left + i);

        srcRect_r.x += 2*base_framesize;
        if(srcRect_r.x == src_right->w) {
            srcRect_r.x = 0;
            srcRect_r.y += 2*base_framesize;
        }

        srcRect_l.x -= 2*base_framesize;
        if(srcRect_l.x < 0) {
            srcRect_l.x = src_left->w - 2*base_framesize;
            srcRect_l.y += 2*base_framesize;
        }
    }
    SDL_FreeSurface(src_left);
    SDL_FreeSurface(src_right);
}

/** Sekély másolat, nem kell külön felszabaditani **/
SDL_Surface ** AnimCpy (SDL_Surface **src, int frame_num) {
    SDL_Surface **dest = malloc (frame_num * sizeof(SDL_Surface *));
    int i;
    for(i = 0; i < frame_num; i++) {
        dest[i] = src[i];
    }
    return dest;
}


/** Mély másolat, külön fell kell szabaditani **/
SDL_Surface ** AnimCpy_Alpha (SDL_Surface **src, int frame_num, int alpha) {
    SDL_Surface **dest = malloc (frame_num * sizeof(SDL_Surface *));
    int i;
    for(i = 0; i < frame_num; i++) {
        dest[i] = SDL_CreateRGBSurface(SDL_ANYFORMAT, src[i]->w , src[i]->h,  screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
        SDL_SetColorKey(dest[i], SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(dest[i]->format,0,0,0));
        SDL_BlitSurface(src[i], NULL, dest[i], NULL);
        SDL_SetAlpha(dest[i], SDL_SRCALPHA|SDL_RLEACCEL , 255 - alpha);
    }
    return dest;
}

/** Mély másolat, külön fell kell szabaditani **/
SDL_Surface ** AnimCpy_AlphaScale (SDL_Surface **src, int frame_num, int alpha_start, int alpha_scale) {
    SDL_Surface **dest = malloc (frame_num * sizeof(SDL_Surface *));
    int i;
    for(i = 0; i < frame_num; i++) {
        dest[i] = SDL_CreateRGBSurface(SDL_ANYFORMAT, src[i]->w , src[i]->h, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
        SDL_SetColorKey(dest[i], SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(dest[i]->format,0,0,0));
        SDL_BlitSurface(src[i], NULL, dest[i], NULL);
        SDL_SetAlpha(dest[i], SDL_SRCALPHA|SDL_RLEACCEL , 255 - alpha_start - alpha_scale * i);
    }
    return dest;
}

/** Forgatások, tükrözések **/

#include "SDL_rotozoom.h"

/** Elforgat egy képet, és az elforgatottal tér vissza */
SDL_Surface * RotateSurface (SDL_Surface *src, double angle) {
    if (src == NULL)
        return NULL;
    SDL_Surface *pic = rotozoomSurface(src, angle, 1, 0);
    SDL_Surface *display_pic = SDL_DisplayFormatAlpha(pic);
    SDL_FreeSurface(pic);
    return display_pic;
}

/** Elforgat egy képet és felülírja az eredetit */
void RotateSurface2 (SDL_Surface **src, double angle) {
    if (*src == NULL)
        return;
    SDL_Surface *pic = rotozoomSurface(*src, angle, 1, 0);
    SDL_FreeSurface(*src);
    *src = SDL_DisplayFormat(pic);
    SDL_FreeSurface(pic);
}

/** y tengelyre tükröz egy képet */
SDL_Surface * FlipSurface (SDL_Surface *src) {
    if (src == NULL)
        return NULL;
    SDL_Surface *pic = rotozoomSurfaceXY(src, 0, -1, 1, 0);
    SDL_Surface *display_pic = SDL_DisplayFormatAlpha(pic);
    SDL_FreeSurface(pic);
    return display_pic;
}

/** Adott méretűre nagyít egy képet */
void SetSize (SDL_Surface **src, int x, int y) {
    if (*src == NULL)
        return;
    DisplayFormat(src);
    SDL_Surface *pic =rotozoomSurfaceXY(*src, 0, (double)x / (*src)->w, (double)y / (*src)->h, 0);
    SDL_FreeSurface(*src);
    DisplayFormat(&pic);
    *src = pic;
}

/** Full screen mértetéhez arányosan nagyít egy képet, illetve rak rá anit-aliasingot */
void Scale (SDL_Surface **src) {
    if (*src == NULL)
        return;
    DisplayFormat(src);
    SDL_Surface *pic = rotozoomSurface(*src, 0, scale, 1);
    SDL_FreeSurface(*src);
    DisplayFormat(&pic);
    *src = pic;
}

/** u.a mint előző, csak anti-aliasing nélkül */
void Scale_no_smooth (SDL_Surface **src) {
    if (*src == NULL)
        return;
    DisplayFormat(src);
    SDL_Surface *pic = rotozoomSurface(*src, 0, scale, 0);
    SDL_FreeSurface(*src);
    DisplayFormat(&pic);
    *src = pic;
}

/** Képbetöltő anti-aliasing nélkül */
SDL_Surface * ImgLoader_no_smooth(char *file) {
    SDL_Surface *pic;
    pic = IMG_Load(file);
    if(pic==NULL) {
        fprintf(stderr,"Missing image %s : %s\n",file,IMG_GetError());
        return NULL;
    }
    // Beállitja az átlátszó szint
    SDL_SetColorKey(pic,SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(pic->format,0,0,0));

    // Felnagyitja a képet fullscreen-hez és átállitja a formatumtot
    Scale_no_smooth(&pic);

    return pic;
}

/** Megjelenítés formátuámra konvertálja a képet (felülírja az eredetit) */
void DisplayFormat( SDL_Surface **src) {
    SDL_Surface *pic = SDL_DisplayFormatAlpha(*src);
    SDL_FreeSurface(*src);
    *src = pic;
}


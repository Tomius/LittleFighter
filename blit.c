#include <SDL.h>
#include <SDL_thread.h>
#include "blit.h"
#include "collision.h"
#include "main.h"

/* Részletenként rajzoláshoz kellő globális változók */
Rect rectList = {};
Rect rectList_old = {};
Rect fuse = {};
/* A flip-hez tartozó szál címe */
SDL_Thread *flip_thread = NULL;

/* A modul "privát" függvényei */
static int Flip_Thread(void *screen);
static void Normalize_Bounds(Rect *list);
/* Megírt, de nem használt függvények: */
//static void Fuse_lists(Rect *to, Rect a, Rect b);
//static void Delete_rect(Rect *list, int i);
//static void Optimalize_list(Rect *list);



/** Az SDL_BlitSurfacenek van egy olyan "hasznos" tulajdonsága hogy ha egy negatív x vagy y
  * koordinátjú objektumra hívják meg, akkor visszaír az SDL_Rect-be (ezért kell címszerint
  * átadni azokat), vagyis kinulláza a negatív kooridnáltát. Na ez az én esetemben nem túl hasznos,
  * ezért létrehozok egy segéd SDL_Rect-et (csak a destRect számít nekem), amibe nyugodtan visszaírhat.
  * Ezekívül még optimalizáláshoz is használom, azaz itt tárolom el hogy a képernyő mely részei változtak */
void BlitSurface (SDL_Surface *src, SDL_Rect *srcRect, SDL_Surface *dest, SDL_Rect *destRect) {
    // Sigsegv ellen
    if(src == NULL || dest == NULL)
        return;

    // A rajzolás
    SDL_Rect tempDestRect = *destRect;
    SDL_BlitSurface(src, srcRect, dest, &tempDestRect);

    // Optimalizáshoz
    if(srcRect == NULL) {
        if(destRect == NULL) {
            SDL_Rect temp = {0, 0, dest->w, dest->h};
            Rect_add(&rectList, &temp);
        } else {
            SDL_Rect temp = {destRect->x, destRect->y, src->w, src->h};
            Rect_add(&rectList, &temp);
        }
    } else {
        if(destRect == NULL) {
            SDL_Rect temp = {srcRect->x, srcRect->y, srcRect->w, srcRect->h};
            Rect_add(&rectList, &temp);
        } else {
            SDL_Rect temp = {destRect->x + srcRect->x, destRect->y + srcRect->y, srcRect->w, srcRect->h};
            Rect_add(&rectList, &temp);
        }
    }
}

/** A Flip_Thread-et új szálként meghívó fv. */
void Flip(void *screen)
{
    flip_thread = SDL_CreateThread(Flip_Thread, screen);
}

/** Vár amíg vége nem lesz a flip szálnak */
void Wait_Flip_Thread()
{
    SDL_WaitThread(flip_thread, NULL);
}

/** Hozzáad egy tégalalpot a listához (lsd Blitsurface) */
inline void Rect_add(Rect *list, const SDL_Rect *rect) {
    if(list->size < 499 && (list->rect[0].w != kx || list->rect[0].h != ky) && (rect->w != kx || rect->h != ky))
        list->rect[(list->size)++] = *rect;
    else {
        SDL_Rect temp = {0, 0, kx, ky};
        list->rect[0] = temp;
        list->size = 1;
    }
}

/** Frissíti a képernyő azon részeit, amik megváltoztak. Vagy legalábbis ez volt az eredeti cél.
  * Csak az a gond ezzel, hogy ez kb 10%-al lassabb mint ha csak egy nagy téglalapot frissítnék,
  * ami lefedi az összes változást (az SDL_UpdateRects és az optimalizálás is lassú).
  * Amúgy a képernyő frissítése a játék leglassabb része, ezért ez külön szálon fut,
  * ami kb +50-90%-ot növel a teljesítményen */
static int Flip_Thread(void *screen) {

    // A képernyőről kilógó részek nem kellenek
    // Elég ezt mindig csak a jelenlegi rectListekre meghívni,
    // hiszen a mostani rectList_old is volt valaha rectList.
    Normalize_Bounds(&rectList);

    // A max lefedő téglalap meghatározása
    int i, minx=kx, miny=ky, maxx=0, maxy=0;
    for(i=0; i<rectList.size; i++) {
        if(rectList.rect[i].x < minx)
            minx = rectList.rect[i].x;
        if(rectList.rect[i].y < miny)
            miny = rectList.rect[i].y;
        if(rectList.rect[i].x + rectList.rect[i].w > maxx)
            maxx = rectList.rect[i].x + rectList.rect[i].w;
        if(rectList.rect[i].y + rectList.rect[i].h > maxy)
            maxy = rectList.rect[i].y + rectList.rect[i].h;
    }
    for(i=0; i<rectList_old.size; i++) {
        if(rectList_old.rect[i].x < minx)
            minx = rectList_old.rect[i].x;
        if(rectList_old.rect[i].y < miny)
            miny = rectList_old.rect[i].y;
        if(rectList_old.rect[i].x + rectList_old.rect[i].w > maxx)
            maxx = rectList_old.rect[i].x + rectList_old.rect[i].w;
        if(rectList_old.rect[i].y + rectList_old.rect[i].h > maxy)
            maxy = rectList_old.rect[i].y + rectList_old.rect[i].h;
    }
    SDL_UpdateRect((SDL_Surface *)screen, minx, miny, maxx-minx, maxy-miny);
    //Fuse_lists(&fuse, rectList, rectList_old);
    //Optimalize_list(&fuse);
    //SDL_UpdateRects(screen, fuse.size, fuse.rect);
    rectList_old = rectList;
    rectList.size = 0;

    // Csak azért kell visszatérési érték, hogy lehessen belőle külön szálat csinálni
    return 0;
}

static void Normalize_Bounds(Rect *list) {
    int i;
    for(i=0; i<list->size; i++) {
        if(list->rect[i].x < 0) {
            list->rect[i].w += list->rect[i].x;
            if(list->rect[i].w < 0)
                list->rect[i].w = 0;
            list->rect[i].x = 0;
        }
        if(list->rect[i].x + list->rect[i].w  > kx) {
            if(list->rect[i].x >= kx) {
                SDL_Rect temp = {0,0,0,0};
                list->rect[i] = temp;
            } else
                list->rect[i].w = kx - list->rect[i].x;
        }
        if(list->rect[i].y < 0) {
            list->rect[i].h += list->rect[i].y;
            if(list->rect[i].h < 0)
                list->rect[i].h = 0;
            list->rect[i].y = 0;
        }
        if(list->rect[i].y + list->rect[i].h > ky) {
            if(list->rect[i].y >= ky) {
                SDL_Rect temp = {0,0,0,0};
                list->rect[i] = temp;
            } else
                list->rect[i].h = ky - list->rect[i].y;
        }
    }
}

//static void Fuse_lists(Rect *to, Rect a, Rect b) {
//    if(a.size + b.size > 499 || (a.rect[0].w == kx && a.rect[0].h != ky) || (b.rect[0].w == kx && b.rect[0].h != ky)) {
//        SDL_Rect screensize = {0, 0, kx, ky};
//        to->rect[0] = screensize;
//        to->size = 1;
//        return;
//    }
//    int i,j;
//    for(i=0; i<a.size; i++)
//        to->rect[i] = a.rect[i];
//    for(j=0; j<a.size; j++)
//        to->rect[i+j] = b.rect[j];
//    to->size = i+j;
//}
//
//static void Delete_rect(Rect *list, int i)
//{
//    int j;
//    for(j=i; j+1<list->size; j++)
//    {
//        list->rect[j]= list->rect[j+1];
//    }
//    list->size--;
//}
//
//static void Optimalize_list(Rect *list)
//{
//    int i,j;
//    for(i=0; i+1<list->size; i++)
//    {
//        for(j=i+1; j<list->size; j++)
//        {
//            SDL_Rect unionRect = Rect_Function(Union, list->rect[i], list->rect[j]);
//            // Ha megéri összevonni a két téglalapot, azaz ha a lefedett összterület
//            // kevesebb mint 500 pixel^2-el változik.
//            if( (unionRect.w*unionRect.h) - (list->rect[i].w*list->rect[i].h) + (list->rect[i].w*list->rect[i].h) < 500 )
//            {
//                list->rect[i] = unionRect;
//                Delete_rect(list, j);
//
//                i=0;
//                j=0;
//                break;
//            }
//        }
//    }
//}


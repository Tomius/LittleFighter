#include <SDL.h>
#include <math.h>
#include "collision.h"
#include "character.h"
#include "animate.h"
#include "main.h"
#include "bg.h"

/* Saját függvények: */
static int TransparentPixel(SDL_Surface *pic, int x , int y);
static int Collide(SDL_Surface* A, SDL_Rect rectA, SDL_Surface* B, SDL_Rect rectB);


/** Visszaadja két SDL_Rect metszetét vagy únióját (A Collision detection-t csak a metszetben kell nézni,
  * az únió pedig az optimalizáláshoz kell) */
SDL_Rect Rect_Function(enum rect_functions checkfor, SDL_Rect rectA, SDL_Rect rectB) {

    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;

    if(checkfor == Intersection)
    {
        /* Egy ábra a könnyebb megértéshez:
                 ________________
                |                |
                |                |
                |                |
                |      x1,y1_____|______
                |        |       |      |
                |        |       |      |
                |________|_______|      |
                         |     x2,y2    |
                         |              |
                         |              |
                         |______________| */

        // Az x1,y1 pont az két téglalap bal, és felső oldalának maximumaként jön ki
        x1 = rectA.x > rectB.x ? rectA.x : rectB.x; // Max
        y1 = rectA.y > rectB.y ? rectA.y : rectB.y; // Max
        // Az x2,y2 pont pedig a jobb, és alsó koordinálták minimumából
        x2 = rectA.x + rectA.w < rectB.x + rectB.w ? rectA.x + rectA.w : rectB.x + rectB.w; // Min
        y2 = rectA.y + rectA.h < rectB.y + rectB.h ? rectA.y + rectA.h : rectB.y + rectB.h; // Min
    }
    else if(checkfor == Union)
    {
        /*
            x1,y1________________ _ _ _
                |                |      |
                |                |
                |                |      |
                |         _______|______
                |        |       |      |
                |        |       |      |
                |________|_______|      |
                |        |              |
                         |              |
                |        |              |
                 _ _ _ _ |______________|
                                        x2,y2*/

        // Az x1,y1 pont az két téglalap bal, és felső oldalának minimumaként jön ki
        x1 = rectA.x < rectB.x ? rectA.x : rectB.x; // Min
        y1 = rectA.y < rectB.y ? rectA.y : rectB.y; // Min
        // Az x2,y2 pont pedig a jobb, és alsó koordinálták maximumából
        x2 = rectA.x + rectA.w > rectB.x + rectB.w ? rectA.x + rectA.w : rectB.x + rectB.w; // Max
        y2 = rectA.y + rectA.h > rectB.y + rectB.h ? rectA.y + rectA.h : rectB.y + rectB.h; // Max
    }


    // Ekkora lesz a visszaadott téglalap
    int width = x2 - x1;
    int height = y2 - y1;

    // Ha a két téglalapnak messze van egymástól, akkor nyilván nem lesz metszetük.
    // Ekkor a metszet szélessége vagy magaságga nulla, vagy negativ, ekkor a fv
    // egy csupa nulla SDL_Rectet ad vissza. Únió számolásánál ilyen gond nem lép
    // fel
    if(width > 0 && height > 0) {
        SDL_Rect intersect = {x1, y1, width, height};
        return intersect;
    } else {
        SDL_Rect intersect = {0, 0, 0, 0};
        return intersect;
    }
}

/** Ez végzi a main főciklusában az ütközések vizsgálatát. Egyrészt nézi hogy a karakterek ütköztek-e
  * egymással (ha igen akkor egy torzított rugalmas ütközést szimulál), illetve hogy a karakterek ütköztek-e
  * a pálya szélével, mert onnan meg vissza kell pattanniuk */
void Collison_Detection(struct character *chars) {
    struct character *iter, *iter2;
    // Azért működnek ilyen "furán" a for ciklusok, hogy minden elempárra csak egyszer hivodjon meg a fv. (mert ez elég lassú)
    for(iter = chars; iter->next != NULL; iter = iter->next)
        for(iter2 = iter->next; iter2 != NULL; iter2 = iter2->next) {
            if((iter->act == def || iter->act == counter || iter->act == s_melee || iter->act == dead || iter->act == ground)
                    || (iter2->act == def || iter2->act == counter || iter2->act == s_melee || iter2->act == dead || iter2->act == ground)
                    || fabs(iter->shadowRect.y - iter2->shadowRect.y) > framesize/5)
                continue;
            else if( Collide(Current_action(iter), iter->loc, Current_action(iter2), iter2->loc) ) {
                // Nagyjából egy rugalmas ütközést modellez két karakter között, de egy picit torzitva van
                // (kisebb a sebességvesztés), mert igy jobban néz ki.

                // A jelenlegi sebességek kiszámitása (figyelembe kell venni a sétáló karaktereket, akiknek
                // nincs explicit eltárolva a sebességük, csak a sebességüknek az iránya)
                int tempx = iter->vx, tempy = iter->vy, tempz = iter->vz;
                int temp2x = iter2->vx, temp2y = iter2->vy, temp2z = iter2->vz;
                if(iter->act == a_walk || (iter->act == jump && iter->vx == 0))
                    tempx = iter->walk.vx*3;
                if(iter2->act == a_walk || (iter2->act == jump && iter2->vx == 0))
                    temp2x = iter2->walk.vx*3;
                if(iter->act == a_walk || (iter->act == jump && iter->vx == 0))
                    tempz = iter->walk.vz*2;
                if(iter2->act == a_walk || (iter2->act == jump && iter2->vx == 0))
                    temp2z = iter2->walk.vz*2;
                // Illetve a karakterre jellemző sebesség állandó is számit
                tempx *= iter->speed;
                tempy *= iter->speed;
                tempz *= iter->speed;
                temp2x *= iter2->speed;
                temp2y *= iter2->speed;
                temp2z *= iter2->speed;


                const double modifier = 0.65;
                // Az új sebességek kiszámitása
                if(iter->loc.x < iter2->loc.x) {
                    if(temp2x < 0) {
                        iter->impulse_x = temp2x;
                        iter->collision_modifier = modifier;
                        iter2->collision_modifier = modifier;
                    }
                    if(tempx > 0) {
                        iter2->impulse_x = tempx;
                        iter->collision_modifier = modifier;
                        iter2->collision_modifier = modifier;
                    }
                } else {
                    if(temp2x > 0) {
                        iter->impulse_x = temp2x;
                        iter->collision_modifier = modifier;
                        iter2->collision_modifier = modifier;
                    }

                    if(tempx < 0) {
                        iter2->impulse_x = tempx;
                        iter->collision_modifier = modifier;
                        iter2->collision_modifier = modifier;
                    }
                }

                if(iter->loc.y < iter2->loc.y) {
                    if(temp2y + temp2z < 0) {
                        iter->impulse_z = temp2y + temp2z;
                        iter->collision_modifier = modifier;
                        iter2->collision_modifier = modifier;
                    }

                    if(tempy + tempz > 0) {
                        iter2->impulse_z = tempy + tempz;
                        iter->collision_modifier = modifier;
                        iter2->collision_modifier = modifier;
                    }
                } else {
                    if(temp2y + temp2z > 0) {
                        iter->impulse_z = temp2y + temp2z;
                        iter->collision_modifier = modifier;
                        iter2->collision_modifier = modifier;
                    }

                    if(tempy + tempz < 0) {
                        iter2->impulse_z = tempy + tempz;
                        iter->collision_modifier = modifier;
                        iter2->collision_modifier = modifier;
                    }
                } // else

                // Ennek csak Woody lökdösése miatt van szerepe (hogy lehessen tudni hogy melyik irányba kell félre állnia)
                // Ez egy fontos feature, na, megér 80 sort :)
                if(iter->impulse_x == 0) {
                    if(iter2->loc.x > iter->loc.x) {
                        if(15 < iter->loc.x)
                            iter->impulse_x = -0.0001;
                        else
                            iter->impulse_x = 0.0001;
                    } else {
                        if(iter->loc.x + framesize + 10 < kx)
                            iter->impulse_x = 0.0001;
                        else
                            iter->impulse_x = -0.0001;
                    }
                }
                if(iter2->impulse_x == 0) {
                    if(iter->loc.x > iter2->loc.x) {
                        if(15 < iter2->loc.x)
                            iter2->impulse_x = -0.0001;
                        else
                            iter2->impulse_x = 0.0001;
                    } else {
                        if(iter2->loc.x + framesize + 10 < kx)
                            iter2->impulse_x = 0.0001;
                        else
                            iter2->impulse_x = -0.0001;
                    }
                }

                if(iter->impulse_z == 0) {
                    if(iter2->shadowRect.y > iter->shadowRect.y) {
                        if(kyTop < iter->loc.y)
                            iter->impulse_z = -0.0001;
                        else
                            iter->impulse_z = 0.0001;
                    } else {
                        if(iter->shadowRect.y < kyBottom)
                            iter->impulse_z = 0.0001;
                        else
                            iter->impulse_z = -0.0001;
                    }
                }
                if(iter2->impulse_z == 0) {
                    if(iter->shadowRect.y > iter2->shadowRect.y) {
                        if(kyTop < iter2->loc.y)
                            iter2->impulse_z = -0.0001;
                        else
                            iter2->impulse_z = 0.0001;
                    } else {
                        if(iter2->shadowRect.y < kyBottom)
                            iter2->impulse_z = 0.0001;
                        else
                            iter2->impulse_z = -0.0001;
                    }
                }
            } // else if(collide)
        } // for

    // A karakterek pattanjanak vissza a pálya széléről és tetejéről
    for(iter = chars; iter != NULL; iter = iter->next) {
        if(TerrainRect.x + iter->loc.x < 0) {
            if(iter->act != s_walk) {
                iter->dir = right;
                iter->vx = fabs(iter->vx);
                if(iter->act == jump && iter->vx == 0)
                    iter->vx = 3;
                iter->walk.vx = 1;
            } else {
                iter->dir = right;
                iter->act = jump;
                iter->vx = 5;
            }

        } else if(iter->loc.x + framesize + TerrainRect.x > TerrainMax + kx) {
            if(iter->act != s_walk) {
                iter->dir = left;
                iter->vx = - fabs(iter->vx);
                if(iter->act == jump && iter->vx == 0)
                    iter->vx = -3;
                iter->walk.vx = - 1;
            } else {
                iter->dir = left;
                iter->act = jump;
                iter->vx = -5;
            }

        }
        if(iter->loc.y < -10*scale)
            iter->vy = fabs(iter->vy) / 4;
    }
}

/** A Collison_detection által kiszámolt impulzus megváltozásokat
  * ez a függvény hozza létre, a karakterek sebességeinek megváltoz-
  * tatásával. */
void Animate_collision(struct character *chars) {
    struct character *iter;
    for(iter = chars; iter != NULL; iter = iter->next) {
        if(iter->act != ground) {
            iter->loc.x += iter->impulse_x * (int)round(scale) * iter->collision_modifier;
            if(iter->act == a_walk && iter->impulse_x != 0) {
                if(iter->impulse_x > 0)
                    iter->walk.vx = 1;
                if(iter->impulse_x < 0)
                    iter->walk.vx = -1;
            }
            if((kyTop + framesize < iter->shadowRect.y && iter->impulse_z < 0) || (iter->shadowRect.y < kyBottom && 0 < iter->impulse_z)) {
                //iter->loc.y += iter->impulse_z * (int)round(scale) * iter->collision_modifier;
                //iter->shadowRect.y += iter->impulse_z * (int)round(scale) * iter->collision_modifier;
                if(iter->act == a_walk && iter->impulse_z != 0) {
                    if(iter->impulse_z > 0)
                        iter->walk.vz = 1;
                    if(iter->impulse_z < 0)
                        iter->walk.vz = -1;
                }
            }
        }
        iter->collision_modifier = 1;
        iter->impulse_x = 0;
        iter->impulse_z = 0;
    }
}

/** Megnézi egy SDL_surface-ről hogy annak egy adott pixele átlátszó-e */
static int TransparentPixel(SDL_Surface *pic, int x , int y) {
    // Ha pixel nem a kép része, akkor átlátszónak tekinthető
    if(x >= pic->w || y >= pic->h)
        return 1;

    // A pixelünkre mutató pointer
    Uint8 *p = (Uint8 *)pic->pixels + (y * pic->pitch) + (x * pic->format->BytesPerPixel);
    // A pixelt a bytesperpixel-től függően kasztolni kell. Bár ránézésre fölösleges a switch,
    // mert a programban csak 32 bites képeket használok (és linux alatt tényleg fölösleges is),
    // de eggyes W-vel kezdődő nevű oprendszereken a rotozoomSurface (impliciten a RotateSurface
    // és a RotateSurface2 hívja meg) ha 32 bites az azt jeleni hogy jó esetben 2 hatvány bpp-jű
    // képet ad vissza, de sajnos elég gyakran 24 bites képeket is dobál. Amit persze ha 32
    // bitesnek kezelnék, akkor kb 100 000 elemmel indexelnék túl egy tömböt egy pixel lekérésekor,
    // ami legjobb esetben is csak seg faulthoz vezet.
    Uint32 pixelcolor = 0;
    switch(pic->format->BytesPerPixel) {
    case 1:
        pixelcolor = *p;
        break;
    case 2:
        pixelcolor = *(Uint16 *)p;
        break;
        // A 24 bites kép egy 16 és egy 8 bites részből áll, amiket bele kell shiftelni a pointer mint tömb megfelelő részeibe,
        // de nem triviális hogy milyen irányba, főleg hogy ez architektúra függő (szerencsére ennek eldöntésére van egy SDL-es makro)
    case 3:
        // Nagyjából annyit jelent mint Digiten a Most Significant Bit first
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            pixelcolor = p[0] << 16 | p[1] << 8 | p[2];
        else
            pixelcolor = p[0] | p[1] << 8 | p[2] << 16;
        break;

    case 4:
        pixelcolor = *(Uint32 *)p;
        break;
    }
    // Ha a pixel szine megegyezik a képhez beállitott átlátszó pixel szinével, akkor igazzal tér vissza
    return (pixelcolor == pic->format->colorkey);
}

/** Megnézi két SDL_Surface-ről hogy azoknak van-e fedésben nem átlátszó pixel-e */
static int Collide(SDL_Surface* A, SDL_Rect rectA, SDL_Surface* B, SDL_Rect rectB) {
    // Nyilván ha az egyik kép nem létezik akkor annak nem lesz metszete a másik képpel
    if(A == NULL || B == NULL)
        return 0;
    // Megnézi hogy mekkora rész van fedésben
    SDL_Rect collisionRect = Rect_Function(Intersection, rectA, rectB);

    if(collisionRect.w == 0 || collisionRect.h == 0)
        return 0;

    // A pixelszintű hozzáféréshez a képeket zárolni kell
    if(SDL_MUSTLOCK(A))
        SDL_LockSurface(A);
    if(SDL_MUSTLOCK(B))
        SDL_LockSurface(B);

    // Minden n-edik pixelre (a pontosság az accuracy nevű változóban van eltárolva) megnézi hogy ott a két pixel
    // ami fedi egymást az átlátszó-e. Ha egyik se átlátszó, akkor a két objektum összeütközött, azaz a fv. logikai igazzal tér vissza.
    int x,y, accuracy = benchmark ? 16*scale : 4*scale;
    for(y = 0; y < collisionRect.h; y += accuracy)
        for(x = 0; x < collisionRect.w; x += accuracy) {
            // Bár ránézésre ez a feltétel sose teljesül, elég sok elvetemült eset van a rotozoomSurfacenek köszönhetően,
            // ahol ez előfordulhat, és ez nyilván seg faulthoz vezetne, ezért a legegyszerűbb ezeket az eseteket egyszerűen
            // átugorni, amúgy sincs értelme egy nem a képhez tartozó pixelről lekérdezni, hogy az a képen milyen szinű.
            if(collisionRect.x - rectA.x + x >= A->w || collisionRect.x - rectB.x + x >= B->w || collisionRect.y - rectA.y + y >= A->h || collisionRect.y - rectB.y + y >= B->h)
                continue;
            if( !TransparentPixel(A, collisionRect.x - rectA.x + x, collisionRect.y - rectA.y + y)
                    && !TransparentPixel(B, collisionRect.x - rectB.x + x, collisionRect.y - rectB.y + y)) {
                // A képek újra kinyitása
                if(SDL_MUSTLOCK(A))
                    SDL_UnlockSurface(A);
                if(SDL_MUSTLOCK(B))
                    SDL_UnlockSurface(B);
                return 1;
            }
        }

    // A képek újra kinyitása
    if(SDL_MUSTLOCK(A))
        SDL_UnlockSurface(A);
    if(SDL_MUSTLOCK(B))
        SDL_UnlockSurface(B);
    return 0;
}


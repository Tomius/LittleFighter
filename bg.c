#include <SDL.h>
#include <math.h>
#include <SDL_image.h>
#include <SDL_gfxPrimitives.h>
#include "character.h"
#include "projectile.h"
#include "imgload.h"
#include "blit.h"
#include "main.h"
#include "bg.h"

SDL_Surface **Terrain = NULL, *Hills = NULL, *Sky = NULL, *shadow;
// A forest nevű pálya volt az első, akkor neveztem
// el igy a pálya részeket, aztán igy maradt
SDL_Rect TerrainRect = {0, 0, 0, 0}, HillsRect = {0, 0, 0, 0};
SDL_Rect Terrain_destRect= {0, 0, 0, 0}, Hills_destRect= {0, 0, 0, 0};
int TerrainMax, Terrain_num, Terrain_state = 0;

enum maps current_map = colisseum;
bool refresh = false;

/** A háttérkép inicializálása (csak a jelenlegit tölti be, nem az összeset) */
void Init_bg(void) {
    switch (current_map) {
    case forest: {
        // A közeli hegyek
        SDL_Surface **hills_src = (SDL_Surface **) malloc(3 * sizeof(SDL_Surface *));
        hills_src[0] = IMG_Load("./bg/Forest/hills1.bmp");
        hills_src[1] = IMG_Load("./bg/Forest/hills2.bmp");
        hills_src[2] = IMG_Load("./bg/Forest/hills3.bmp");
        int i;
        for (i = 0; i < 3; i++)
            SDL_SetColorKey(hills_src[i],SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(hills_src[i]->format,0,0,0));

        Hills = SDL_CreateRGBSurface(SDL_ANYFORMAT, 2*800, hills_src[0]->h, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
        int hillsh = Hills->h;
        SDL_Rect rect = {0,0,0,0};
        rect.x = rect.y = 0;
        while(rect.x < Hills->w) {
            SDL_BlitSurface(hills_src[0], NULL, Hills, &rect);
            rect.x += hills_src[0]->w;
        }
        rect.x = 0;
        rect.y = Hills->h - hills_src[1]->h;
        SDL_BlitSurface(hills_src[1], NULL, Hills, &rect);
        rect.x = Hills->w - hills_src[2]->w;
        rect.y = Hills->h - hills_src[2]->h;
        SDL_BlitSurface(hills_src[2], NULL, Hills, &rect);
        SetSize(&Hills, 2*kx, 2*kx*Hills->h/Hills->w);

        // Felszabaditas
        for (i = 0; i < 3; i++)
            SDL_FreeSurface(hills_src[i]);
        free(hills_src);

        // Az ég
        Sky = IMG_Load("./bg/Forest/sky.bmp");
        int skyh = Sky->h;
        SetSize(&Sky, kx, kx*Sky->h/Sky->w);

        // A Terrain előállitása
        SDL_Surface **terrain_src = (SDL_Surface **) malloc (5 * sizeof(SDL_Surface *));
        terrain_src[0] = IMG_Load("./bg/Forest/land1.bmp");
        terrain_src[1] = IMG_Load("./bg/Forest/land2.bmp");
        terrain_src[2] = IMG_Load("./bg/Forest/land3.bmp");
        terrain_src[3] = IMG_Load("./bg/Forest/land4.bmp");
        terrain_src[4] = IMG_Load("./bg/Forest/forest.bmp");
        SDL_SetColorKey(terrain_src[4], SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(terrain_src[4]->format,0,0,0));
        Terrain_num = 1;
        Terrain = malloc (Terrain_num * sizeof(SDL_Surface *));
        Terrain[0] = SDL_CreateRGBSurface(SDL_ANYFORMAT, 6*800, 450 - skyh/4 - hillsh/2, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
        kyTop = ky - ((double)(Terrain[0]->h - terrain_src[4] ->h) * (ky - Sky->h/4 - Hills->h/2) / Terrain[0]->h) - framesize;

        // Az erdő kirajzolása
        rect.x = rect.y = 0;
        while(rect.x < Terrain[0]->w) {
            SDL_BlitSurface(terrain_src[4], NULL, Terrain[0], &rect);
            rect.x += terrain_src[4]->w;
        }
        // A talaj kirajzolása
        boxRGBA(Terrain[0], 0, terrain_src[4]->h, Terrain[0]->w, Terrain[0]->h, 19, 79, 16, 255);
        rect.x = 40;
        int random = rand()%4;
        while(rect.x + terrain_src[random]->h < Terrain[0]->w) {
            rect.y = rand()%(Terrain[0]->h - terrain_src[4]->h - terrain_src[random]->h) + terrain_src[4]->h;
            SDL_BlitSurface(terrain_src[random], NULL, Terrain[0], &rect);
            rect.x += terrain_src[random]->w + 40 + rand()%500;
            random = rand()%4;
        }
        SetSize(&Terrain[0], 6*kx ,ky - Sky->h/4 - Hills->h/2);

        // A terrain_srchez felhasznált cuccok felszabaditása
        for (i = 0; i < 5; i++)
            SDL_FreeSurface(terrain_src[i]);
        free(terrain_src);

        Hills_destRect.y = Sky->h / 4;
        Terrain_destRect.y = ky - Terrain[0]->h;

        TerrainRect.w = kx;
        TerrainRect.h = Terrain[0]->h;
        HillsRect.w = kx;
        HillsRect.h = Hills->h;

        // TerrainMax kiszámitása
        TerrainMax = Terrain[0]->w - kx;
        // Az árnyék
        shadow = IMG_Load("./bg/Forest/s.bmp");
        Scale_no_smooth(&shadow);
    }
    break;
    case prison: {
        Sky = NULL;
        Hills = NULL;
        Terrain_num = 7;
        Terrain = malloc (Terrain_num * sizeof(SDL_Surface *));
        SDL_Surface *wall = IMG_Load("./bg/Prison/wall.bmp");
        SDL_Rect wallrect = {0, 0, 0, 0};
        int i;
        for(i=0; i<Terrain_num; i++) {
            Terrain[i] = SDL_CreateRGBSurface(SDL_ANYFORMAT, 4*800, 450, 32, 0, 0, 0, 255);
            char c[50];
            sprintf(c, "./bg/Prison/fire%d.bmp",i+1);
            SDL_Surface *fire = IMG_Load(c);
            while(wallrect.x < Terrain[i]->w) {
                SDL_BlitSurface(wall, NULL, Terrain[i], &wallrect);
                SDL_BlitSurface(fire, NULL, Terrain[i], &wallrect);
                wallrect.x += wall->w;
            }
            boxRGBA(Terrain[i], 0, wall->h, Terrain[i]->w, Terrain[i]->h, 89, 78, 74, 255);
            wallrect.x = 0;
            SetSize(&Terrain[i], 4*kx, ky);
            SDL_FreeSurface(fire);
        }
        kyTop = wall->h/2.0 * ky / 450;
        SDL_FreeSurface(wall);
        TerrainRect.w = kx;
        TerrainRect.h = ky;
        TerrainMax = Terrain[0]->w - kx;
        shadow = IMG_Load("./bg/Prison/s.bmp");
        Scale_no_smooth(&shadow);
    }
    break;
    case vallenfyre: {
        top = true;
        Sky = NULL;
        Hills = NULL;
        Terrain_num = 18;
        Terrain = malloc (Terrain_num * sizeof(SDL_Surface *));
        int i;
        // A képek betöltése itt egy picit fura, mivel az átmértetezésben használt DisplayFormat függvény valamiért
        // átállitja a fekete pixeleket átlátszóra. Hogy ezt miért teszi, arról ötletem sincs, mert amig a forditó
        // idáig eljut, még egyszer se lett meghivva az SDL_SetColorKey függvény, de még ha meg is lett volna hivva,
        // valamelyik függvénybe ez előtt, akkor se kérte még senki az SDL-t hogy csinálja meg itt is. A megoldás
        // szerencsére egyszerű: fekete háttére kell ráblittelni ezeket a képeket
        for(i=0; i<Terrain_num; i++) {
            char c[50];
            sprintf(c, "./bg/Vallenfyre/%d.bmp", i);
            SDL_Surface *temp = IMG_Load(c);
            SetSize(&temp, ky*temp->w/temp->h, ky);
            Terrain[i] = SDL_CreateRGBSurface(SDL_ANYFORMAT, temp->w, temp->h, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
            SDL_BlitSurface(temp, NULL, Terrain[i], NULL);
            SDL_FreeSurface(temp);
        }
        kyTop = 4.0*ky/5;
        kyBottom = ky;
        TerrainRect.w = kx;
        TerrainRect.h = ky;
        TerrainMax = Terrain[0]->w - kx;
        shadow = IMG_Load("./bg/Vallenfyre/s.bmp");
        boxRGBA(shadow, 0, 0, shadow->w, shadow->h, 0, 0, 0, 255);
        Scale_no_smooth(&shadow);
    }
    break;
    case colisseum: {
        Sky = NULL;
        Hills = NULL;
        Terrain_num = 2;
        Terrain = malloc (Terrain_num * sizeof(SDL_Surface *));
        SDL_Surface *temp = IMG_Load("./bg/Colisseum/0.bmp");
        SetSize(&temp, kx, ky);
        Terrain[0] = SDL_CreateRGBSurface(SDL_ANYFORMAT, temp->w, temp->h, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
        SDL_BlitSurface(temp, NULL, Terrain[0], NULL);
        SDL_FreeSurface(temp);

        temp = IMG_Load("./bg/Colisseum/1.bmp");
        SetSize(&temp, kx, ky);
        Terrain[1] = SDL_CreateRGBSurface(SDL_ANYFORMAT, temp->w, temp->h, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
        SDL_BlitSurface(temp, NULL, Terrain[1], NULL);
        SDL_FreeSurface(temp);
        kyTop = 5.0*ky/16;
        kyBottom = 7.0*ky/8;
        TerrainRect.w = kx;
        TerrainRect.h = ky;
        TerrainMax = 0;
        shadow = IMG_Load("./bg/Colisseum/s.bmp");
        Scale_no_smooth(&shadow);
    }
    break;
    case practise_arena: {
        Sky = NULL;
        Hills = NULL;
        Terrain_num = 1;
        Terrain = malloc (Terrain_num * sizeof(SDL_Surface *));
        SDL_Surface *temp = IMG_Load("./bg/Practise_Arena/0.bmp");
        SetSize(&temp, kx, ky);
        Terrain[0] = SDL_CreateRGBSurface(SDL_ANYFORMAT, temp->w, temp->h, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
        SDL_BlitSurface(temp, NULL, Terrain[0], NULL);
        SDL_FreeSurface(temp);

        kyTop = 5.0*ky/16;
        kyBottom = 7.0*ky/8;
        TerrainRect.w = kx;
        TerrainRect.h = ky;
        TerrainMax = 0;
        shadow = IMG_Load("./bg/Practise_Arena/s.bmp");
        Scale_no_smooth(&shadow);
    }
    break;
    default:
        break;
    }
}

/** A háttér kirajzolása */
void Draw_background(SDL_Surface *screen) {
    // Fontos a sorrend (föntről lefele)
    if(Sky != NULL)
        SDL_BlitSurface(Sky, NULL, screen, NULL);
    if(Hills != NULL)
        SDL_BlitSurface(Hills, &HillsRect, screen, &Hills_destRect);
    if(Terrain[Terrain_state] != NULL)
        SDL_BlitSurface(Terrain[Terrain_state], &TerrainRect, screen, &Terrain_destRect);
}

/** A háttérnek csak a megváltozott részeinek újrarajzolása volt ennek a fvnek a célja.
  * Mint utólag kiderült a blitsurface-nek elég hosszú az inicializálási ideje
  * colorkey-jes képekre, ezért az "optimalizálástól" csak lényegesen lassabb lett
  * a játék a nagy számú BlitSurface hívás miatt. Emiatt ez az fv jelenleg annyit csinál,
  * hogy nem a teljes háttért rajzolja újra, hanem a legkisebb olyan téglalapot, ami lefedi
  * az összes változást. Legfeljebb 5-10%-ot javít a teljesítményen, de a különbség érezhető. */
void Draw_bg(SDL_Surface *screen) {

    // Ne próbáljon meg a képernyővel dolgozni, amíg egy másik szál dolgozik vele
    Wait_Flip_Thread();

    // Ha a háttér megváltozott, akkor a teljes hátteret rajzolja ki
    if(refresh){
        SDL_Rect whole_screen = {0, 0, kx, ky};
        Rect_add(&rectList_old, &whole_screen);
    }

    // A legkisebb lefedő téglalap meghatározása
    int i, minx=kx, miny=ky, maxx=0, maxy=0;
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
    if(minx < 0)
        minx = 0;
    if(miny < 0)
        miny = 0;
    if(maxx > kx)
        maxx = kx;
    if(maxy > ky)
        maxy = ky;

    SDL_Rect rect = {minx, miny, maxx-minx, maxy-miny};

    // Sky
    if(Sky != NULL) {
        SDL_Rect tempRect = {rect.x, rect.y, rect.w, rect.h};
        if(tempRect.y < 0) {
            // Ha a karakternek van a pálya részletre belógó része
            tempRect.h += tempRect.y;
            tempRect.y = 0;
            if(tempRect.h > 0) {
                SDL_Rect tempdestRect = {rect.x, 0, rect.w, rect.h};
                if(tempdestRect.h > Sky->h)
                    tempdestRect.h = Sky->h;
                SDL_BlitSurface(Sky, &tempRect, screen, &tempdestRect);
            }
        } else if(tempRect.y < Sky->h) {
            if(tempRect.h + tempRect.y > Sky->h)
                tempRect.h = Sky->h - tempRect.y;
            // Csak hogy ne írjon vissza az iter->Rect-be
            SDL_Rect tempdestRect = rect;
            SDL_BlitSurface(Sky, &tempRect, screen, &tempdestRect);
        }
    }
    // Hills
    if(Hills != NULL) {
        SDL_Rect tempRect = {HillsRect.x + rect.x, rect.y - Hills_destRect.y, rect.w, rect.h};
        if(tempRect.y < 0) {
            // Ha a karakternek van a pálya részletre belógó része
            tempRect.h += tempRect.y;
            tempRect.y = 0;
            if(tempRect.h > 0) {
                SDL_Rect tempdestRect = {rect.x, Hills_destRect.y, rect.w, rect.h};
                if(tempdestRect.h > Hills->h)
                    tempdestRect.h = Hills->h;
                SDL_BlitSurface(Hills, &tempRect, screen, &tempdestRect);
            }
        } else if(tempRect.y < Hills->h) {
            if(tempRect.h + tempRect.y > Hills->h)
                tempRect.h = Hills->h - tempRect.y;
            SDL_Rect tempdestRect = rect;
            SDL_BlitSurface(Hills, &tempRect, screen, &tempdestRect);
        }
    }
    // Terrain
    if(Terrain[Terrain_state] != NULL) {
        SDL_Rect tempRect = {TerrainRect.x + rect.x, rect.y - Terrain_destRect.y, rect.w, rect.h};
        if(tempRect.y < 0) {
            // Ha a karakternek van a pálya részletre belógó része
            tempRect.h += tempRect.y;
            tempRect.y = 0;
            if(tempRect.h > 0) {
                SDL_Rect tempdestRect = {rect.x, Terrain_destRect.y, rect.w, rect.h};
                if(tempdestRect.h > Terrain[Terrain_state]->h)
                    tempdestRect.h = Terrain[Terrain_state]->h;
                SDL_BlitSurface(Terrain[Terrain_state], &tempRect, screen, &tempdestRect);
            }

        } else if(tempRect.y < Terrain[Terrain_state]->h) {
            if(tempRect.h + tempRect.y > Terrain[Terrain_state]->h)
                tempRect.h = Terrain[Terrain_state]->h - tempRect.y;
            SDL_Rect tempdestRect = rect;
            SDL_BlitSurface(Terrain[Terrain_state], &tempRect, screen, &tempdestRect);
        }
    }
}

/** A háttér x tengely menti mozgatásáért, ilyenkor a karakterek a képernyőhöz képesti koordináltarendszerben
  * való eltolásáért, illetve a háttér apróbb részleteinek animálásáért felelős függvény */
void Animate_background(int Tick, struct character *chars, struct projectile *projs, struct character *player) {
    int camera_speed = 0;
    if(!benchmark) {
        if(player->act == air || player->act == jump || player->act == flip || player->act == s_walk)
            camera_speed = fabs(player->vx);
        else
            camera_speed = 3;
        camera_speed *= player->speed * (int)round(scale);
    }
    switch (current_map) {
    case forest:
        if(player->loc.x + framesize / 2 > kx / 2 && TerrainRect.x < TerrainMax) {
            if(TerrainMax - TerrainRect.x < camera_speed)
                camera_speed = TerrainMax - TerrainRect.x;
            struct character *iter;
            for(iter=chars; iter!=NULL; iter=iter->next)
                iter->loc.x -= camera_speed;
            struct projectile *iter2;
            for(iter2=projs; iter2!=NULL; iter2=iter2->next) {
                iter2->x -= camera_speed;
                iter2->shadowRect.x -= camera_speed;
                iter2->x3 -= camera_speed;
            }
            TerrainRect.x += camera_speed;
            HillsRect.x += camera_speed / 5;
            // Az egészosztás pontatlansága miatt
            if(HillsRect.x > Hills->w - kx)
                HillsRect.x = Hills->w - kx;
            refresh = true;
        }
        if(player->loc.x + framesize / 2 < kx / 2 && TerrainRect.x > 0) {
            if(TerrainRect.x < camera_speed)
                camera_speed = TerrainRect.x;
            struct character *iter;
            for(iter=chars; iter!=NULL; iter=iter->next)
                iter->loc.x += camera_speed;
            struct projectile *iter2;
            for(iter2=projs; iter2!=NULL; iter2=iter2->next) {
                iter2->x += camera_speed;
                iter2->shadowRect.x += camera_speed;
                iter2->x3 += camera_speed;
            }
            TerrainRect.x -= camera_speed;
            HillsRect.x -= camera_speed / 5;
            if(HillsRect.x < 0)
                HillsRect.x = 0;
            refresh = true;
        }
        break;
    case vallenfyre:
    case prison:
        if(player->loc.x + framesize / 2 > kx / 2 && TerrainRect.x < TerrainMax) {
            if(TerrainMax - TerrainRect.x < camera_speed)
                camera_speed = TerrainMax - TerrainRect.x;
            struct character *iter;
            for(iter=chars; iter!=NULL; iter=iter->next)
                iter->loc.x -= camera_speed;
            struct projectile *iter2;
            for(iter2=projs; iter2!=NULL; iter2=iter2->next) {
                iter2->x -= camera_speed;
                iter2->shadowRect.x -= camera_speed;
                iter2->x3 -= camera_speed;
            }
            TerrainRect.x += camera_speed;
            refresh = true;
        }
        if(player->loc.x + framesize / 2 < kx / 2 && TerrainRect.x > 0) {
            if(TerrainRect.x < camera_speed)
                camera_speed = TerrainRect.x;
            struct character *iter;
            for(iter=chars; iter!=NULL; iter=iter->next)
                iter->loc.x += camera_speed;
            struct projectile *iter2;
            for(iter2=projs; iter2!=NULL; iter2=iter2->next) {
                iter2->x += camera_speed;
                iter2->shadowRect.x += camera_speed;
                iter2->x3 += camera_speed;
            }
            TerrainRect.x -= camera_speed;
            refresh = true;
        }
        if(Tick % 6 == 0) {
            Terrain_state = (Terrain_state + 1) % Terrain_num;
            refresh = true;
        }
        break;
    case colisseum:
    case practise_arena:
        if(Tick % 20 == 0) {
            Terrain_state = (Terrain_state + 1) % Terrain_num;
            refresh = true;
        }
        break;
    }
}

/** A colisseum pályán a box ring alján lévő kötél rijazolása (ennek fednie kell a karaktereket) */
void Draw_background2(SDL_Surface *screen) {
    switch (current_map) {
    case colisseum: {
        int linesize = 1.5 * scale;
        boxRGBA(screen, 0, 320*ky/386 - linesize, kx, 320*ky/386 + linesize, 143, 113, 106, 255);
        boxRGBA(screen, 0, 332*ky/386 - linesize, kx, 332*ky/386 + linesize, 143, 113, 106, 255);
    }
    break;
    default:
        break;
    }

}

/** A jelenleg betöltött háttér felszabadítása, a háttér adatainak resetlése) */
void Free_background(void) {
    SDL_FreeSurface(Hills);
    Hills = NULL;
    SDL_FreeSurface(Sky);
    Sky = NULL;
    int i;
    for(i=0; i<Terrain_num; i++)
        SDL_FreeSurface(Terrain[i]);
    free(Terrain);
    Terrain = NULL;
    SDL_FreeSurface(shadow);
    shadow = NULL;
    TerrainRect.x = HillsRect.x = 0;
    Terrain_destRect.x = Terrain_destRect.y = Hills_destRect.x = Hills_destRect.y = 0;
    TerrainMax = Terrain_state = 0;
    Terrain_num = 1;
}

/** Ha a játékos meghalt de újra akar kezdeni egy pályát akkor ez a fv tekeri azt vissza
  * az elejére */
void Reset_map(void) {
    TerrainRect.x = 0;
    HillsRect.x = 0;
}

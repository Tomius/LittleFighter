#include <SDL.h>
#include <SDL_image.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_ttf.h>
#include <stdlib.h>
#include <time.h>
#include "animate.h"
#include "character.h"
#include "projectile.h"
#include "free.h"
#include "imgload.h"
#include "blit.h"
#include "controls.h"
#include "UI.h"
#include "main.h"
#include "menu.h"
#include "bg.h"
#include "hit.h"
#include "AI.h"
#include "level.h"
#include "text.h"
#include "music.h"
#include "collision.h"

/** Teendők:
 *  - alt + tab működjön menü közben is */

/* Kívülről is elérhető globális változók */
// A fullscreen felbontásához tartozó konstansok
int kx, ky, kyTop, kyBottom, framesize;
double scale;

// Globális logikai változók (a bool egy typedefelt enum)
bool benchmark = false, practise = false, top = false;

// Mennyi ideig volt megállítva a játék (Hogy ne zavarjon be az, hogy az SDL_GetTicks() az alatt is számol)
Uint32 paused = 0;

// A képernyő adatai (SDL_CreateRGBSurface-hez kell, hogy ne a DisplayFormat-nak kelljen ezt megcsinálni)
int screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask;

// FPS változók: két csoportja van, a d - drawFPS és az a - animateFPS.
// A DrawFPS azt számolja hogy hányszor frissül a kép másodpercenként,
// míg az AnimateFPS a játék menetét kontrollálja. A kettő azért van
// különválasztva, mert így a játék sebessége független tud maradni
// a gép teljesítményétől.
int dTick = -1, aTick = 0, aNextTick = 0, dNextTick = 0, fps = 0;
const int aFPS = 50;

/* Saját Globális változók */
// Az fps számitásához segédváltozók
static int dFPS = 50, last_aTick = -1, last_dTick = 0, NextSecond = 0, lag = 0;

/* A modul saját "privát" függvényei: */
static void Screen_init(SDL_Surface **screen);
static void Fps_fn(void);
/* További megírt függvény(ek): */
//static void Display_FPS(SDL_Surface *screen);

int main(int argc, char *argv[]) {

    SDL_Surface *screen;
    Screen_init(&screen);

    bool quit = false, retry = true, initialized = false, initialized2 = false, levelup = false;

    while(!quit) {
        // Inicializál
        if(!initialized) {
            Init_cursor();
            Init_Audio();
            Init_fonts();
            initialized = true;
        }
        // A képernyő közepére mozgatja a kurzort
        SDL_WarpMouse(kx/2, ky/2);

        // Főmenü
        quit = Draw_menu(screen);

        if(!quit) {
            // A kiválasztott játékmód betöltése
            retry = true;
            Init_bg();
            Loading_screen(screen);
            if(!initialized2) {
                Init_characters();
                Hp_bar_init();
                Init_skill_icons();
                initialized2 = true;
            }
        } else
            retry = false;

        while(retry) {
            // Adattárolók inicializálása
            struct character *player = NULL, *Woody = NULL, *chars = NULL, *boss = NULL;
            struct projectile *projs = NULL;

            // Pálya inicializálása
            init_level(dTick, screen, &chars, &player, &boss, &Woody, &projs);

            // Főciklus változói
            bool keydown[keys_num] = {}, justpressed[keys_num] = {};
            bool justreleased[keys_num] = {}, alive = true;
            int mouse_x = kx/2, mouse_y = ky/2, enemies_killed = 0;
            SDL_GetMouseState(&mouse_x, &mouse_y);
            levelup = false;

            // Főciklus
            while(alive && !levelup) {
                dTick++;

                // Rajzolás
                Draw_bg(screen);
                Draw_UI(screen, chars, player, Woody, boss, enemies_killed);
                Reorder_for_draw_characters(&chars);
                Draw_characters(chars, screen);
                Draw_projectiles(projs, screen);
                Draw_background2(screen);
                //Display_FPS(screen);

                // Ez szabályozza hogy hol mikor mennyi ellenség jöjjön ellenünk,
                // illetve azt is nézi hogy él meg a mi karaktererünk (logikai igazzal tér vissza ha él)
                alive = spawn_or_kill(dTick, screen, &chars, player, &Woody, &boss, &enemies_killed, &levelup, &retry, &projs);
                // Kirajzolja a kurzort, vagy valamelyik játék közbeni menü egyikét
                Draw_cursor_or_menu(screen, enemies_killed, &mouse_x, &mouse_y, &alive, &retry, &quit, levelup);

                // A képernyő frissitése (ez lassú, ezért külön szálon fut)
                Flip(screen);

                // Mesterséges Intelligencia
                AI(last_aTick, chars, player, projs);

                // A program ezen részei nem a konkrét frame számtól, hanem csakis időtől függnek.
                // Ez azért jó mert pl. Benchmark módba 500+ fps-el, vagy valgrind futtatása
                // közben kb 5-10 fps-el is ugyan akkora lesz a játék sebessége
                while(last_aTick < aTick) {
                    last_aTick++;

                    // Ha nagyon beszaggatott, akkor adjunk esélyt a gépnek hogy visszahozza a hátrányát
                    if(aTick - last_aTick < 5) {
                        // Animálás, illetve ütközések
                        Collison_Detection(chars);
                        Animate_actions(last_aTick, chars, &projs);
                        Animate_collision(chars);
                        Animate_projectiles(last_aTick, &projs);
                        Animate_background(last_aTick, chars, projs, player);
                    }
                }

                // Sebzések
                Reorder_for_melee_hits(&chars);
                Melee_hits(chars);
                Projectile_hits(chars, projs);

                // Várakozás a következő ciklusig
                Fps_fn();

                // A billenytűlenyomások elmentése
                Key_Presses(keydown, justpressed, justreleased, &mouse_x, &mouse_y, &alive, &retry, &quit);
                // A billentyűlenyomások hatásainak kezelése
                Change_States(keydown, justpressed, justreleased, mouse_x, mouse_y, player);

            } // főhurok vége (amig a karakter életben van, vagy amig a felhasználó ki nem akar lépni)
            if(retry == true && !levelup)
                Reset_map();
            if(retry == false && !levelup) {
                Free_background();
                practise = top = benchmark = false;
            }
            killall(chars);
            deleteall(projs);
        }
    }
    if(initialized) {
        Free_cursor();
        Close_Audio();
        Close_Fonts();
    }
    if(initialized2) {
        spawn_kill_all();
        Free_hp_bar();
        Free_skill_bar();
    }
    SDL_Quit();
    return 0;
}

/** Ez az SDL_GetTicks()-nek egy olyan változata, ami csak azt számolja,
  * hogy a játék ténylegesen mennyi ideig futott, de nem számolja bele
  * azokat az idő intervallumokat, amikor a játék le volt állitva
  * (nagyon primitv, de nagyon fontos fv) */
Uint32 GetTicks(void) {
    return SDL_GetTicks() - paused;
}

/** Inicializálja a képernyőt, megnézi hogy mekkora a fullscreen felbontás,
  * illetve ennek tudatában beállít néhány globális változót (pl kx, ky scale) */
static void Screen_init(SDL_Surface **screen) {
    // Ahogy az SDL2 quit() doksijába is írták, az atexit() gonosz
    // (és nem is mindig működik), de azért jobb mint a semmi.
    atexit(SDL_Quit);

    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);

    //*screen = SDL_SetVideoMode(0, 0, 0, SDL_ASYNCBLIT|SDL_ANYFORMAT|SDL_FULLSCREEN);
    // Debuggoláshoz előnyös ha a progi nem full screenbe fut (Valgrindhoz különösen)
    *screen = SDL_SetVideoMode( 1024 , 576 , 0, SDL_ASYNCBLIT|SDL_ANYFORMAT);

    kx = (*screen)->w;
    ky = (*screen)->h;
    kyTop = 0; // Ezt majd a pálya inicializálása állitja be, de arra az esetre ha egy pályánál lemaradna
    kyBottom = ky;
    framesize = (int)( (double) kx / base_kx * base_framesize);
    scale = framesize / (double) base_framesize;

    // CreateSurface-hez
    screen_bpp = (*screen)->format->BitsPerPixel;
    screen_rmask = (*screen)->format->Rmask;
    screen_gmask = (*screen)->format->Gmask;
    screen_bmask = (*screen)->format->Bmask;
    screen_amask = (*screen)->format->Amask;

    // Titlet csak azért állítok be, mert néhány OS az alt tab hatására elfelejti hogy az ablak full screen volt
    SDL_WM_SetCaption("Little Fighter", NULL);
    // A játékban elég sokszor van meghívva a rand()
    srand(time(NULL));
    // Saját kurzort használok
    SDL_ShowCursor(false);
}

/** Ez a függvény azért felel hogy a CPU ne 100%-on pörögjön, azazhogy egy
  * adott számnál (dFPS) ne rajzoljon gyakrabban a program másodpercenként,
  * hanem inkább minden rajozlás után várjon. A függvénye továbbá számolja
  * hány rajzolás sikerült, és ha az lényegesebben kevesebb mint amit elvárt,
  * akkor lejjebb veszi az elvárásait a géppel szemben, a játékélmény
  * növelése érdekében */
static void Fps_fn(void) {
    // Ha az FPS maximalizálva van, akkor 1000/FPS
    // ms teljen el két kirajzolás között
    if(!benchmark) {
        if( dNextTick > GetTicks() )
            SDL_Delay( dNextTick - GetTicks() );
        dNextTick = GetTicks() + 1000/dFPS;
    }

    // animateTick kiszámitása
    while(aNextTick < GetTicks()) {
        aTick++;
        aNextTick += 1000/aFPS;
    }

    // A konkrét rajzoláshoz tartozó FPS kiszámtiása
    if(NextSecond <= GetTicks()) {
        fps = dTick - last_dTick;
        if(fps< 0.7 * dFPS)
            lag++;
        else
            lag = 0;
        if(lag >= 5 && !benchmark && dFPS > 10) {
            // Ha a gép 5mp-n át nem tudja tartani a jelenlegi FPS-t, akkor fölöslegesen ne fusson 100%-on a proci,
            // rajzoljon ritkábban, amitől csak egy picit néz ki roszabbul a játék, cserébe élvethetőbb
            dFPS = dFPS/2;
        }
        last_dTick = dTick;
        NextSecond = GetTicks() + 1000;
    }
}

///** Az fps-t kiíró függvény, alapvetően fejlesztéshez */
//static void Display_FPS(SDL_Surface * screen) {
//    char c[50];
//    sprintf(c, "FPS: %d", fps);
//    if(level == 1)
//        stringRGBA(screen, kx-80, 10, c, 0, 0, 0, 255);
//    else
//        stringRGBA(screen, kx-80, 10, c, 255, 255, 255, 255);
//}

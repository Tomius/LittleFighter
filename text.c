#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "imgload.h"
#include "main.h"
#include "blit.h"
#include "character.h"
#include "animate.h"
#include "bg.h"

/** Globális fontok */
TTF_Font *Knig11, *Knig35, *Knig60;
TTF_Font *Cour10, *Cour12, *Cour15, *Cour20, *Cour20B, *Cour25, *Cour35B, *Cour45;

/** Inicializálja a fontokat */
void Init_fonts() {
    TTF_Init();
    Knig11 = TTF_OpenFont("./Text/Knigqst.ttf", 11*scale);
    Knig35 = TTF_OpenFont("./Text/Knigqst.ttf", 35*scale);
    Knig60 = TTF_OpenFont("./Text/Knigqst.ttf", 60*scale);

    Cour10 = TTF_OpenFont("./Text/Courier.ttf", 10*scale);
    Cour12 = TTF_OpenFont("./Text/Courier.ttf", 12*scale);
    Cour15 = TTF_OpenFont("./Text/Courier.ttf", 15*scale);
    Cour20 = TTF_OpenFont("./Text/Courier.ttf", 20*scale);
    Cour20B = TTF_OpenFont("./Text/Courier.ttf", 20*scale);
    TTF_SetFontOutline(Cour20B, 1);
    Cour25 = TTF_OpenFont("./Text/Courier.ttf", 25*scale);
    Cour35B = TTF_OpenFont("./Text/Courier.ttf", 35*scale);
    TTF_SetFontOutline(Cour35B, 1);
    Cour45 = TTF_OpenFont("./Text/Courier.ttf", 45*scale);
}

/** Kirajzol egy szöveget (utf8 kódolásan) majd átkonvertálja a megjelenítés formátumára. */
SDL_Surface * RenderText(TTF_Font *font, char *c, SDL_Color color) {
    SDL_Surface *ret = TTF_RenderUTF8_Blended(font, c, color);
    DisplayFormat(&ret);
    return ret;
}

/** Kirajzol egy szövegdobozt egy adott karakterhez, egy paraméterben kapott szöveggel */
void Draw_text_bubble (SDL_Surface *screen, struct character *player, char *c1, char *c2, char *c3) {
    SDL_Surface *bubble = IMG_Load("./Text/text_bubble.gif");
    Scale(&bubble);
    SDL_Color black = {5, 5, 5};
    SDL_Surface *text = RenderText(Knig11, c1, black);
    SDL_Rect Rect = {bubble->w/2 - text->w/2 - 2*scale, bubble->h/6, 0, 0};
    SDL_BlitSurface(text, NULL, bubble, &Rect);
    SDL_FreeSurface(text);

    text = RenderText(Knig11, c2, black);
    Rect.x = bubble->w/2 - text->w/2 - 2*scale;
    Rect.y = 2*bubble->h/6;
    SDL_BlitSurface(text, NULL, bubble, &Rect);
    SDL_FreeSurface(text);

    text = RenderText(Knig11, c3, black);
    Rect.x = bubble->w/2 - text->w/2 - 2*scale;
    Rect.y = 3*bubble->h/6;
    SDL_BlitSurface(text, NULL, bubble, &Rect);
    SDL_FreeSurface(text);

    SDL_Rect destRect = {player->loc.x - bubble->w, player->loc.y - bubble->h, 0, 0};
    BlitSurface(bubble, NULL, screen, &destRect);
    SDL_FreeSurface(bubble);
}

/** Kiirja a paraméterben kapott 3 stringet(egy string egy sor) egy szövegdobozba,
  * közbe animálja a karaktereket (hogy azok lélegezzenek), illetve a háttért.
  * Hagy időt a felhasználónak elolvasni a szöveget (paraméterben kapott ideig),
  * de az "any key" megnyomásával tovább is lehet lépni. */
void Text_bubble(struct character *who, char *c1, char *c2, char *c3, int ms, SDL_Surface *screen, struct character *chars, struct character *player, struct projectile **projs) {
    Uint32 start = GetTicks();
    SDL_Event event;
    // Először kiszedi a várólistán lévő gombnyomásokat
    while(SDL_PollEvent(&event));

    int Tick = -1, NextTick = SDL_GetTicks() + 1000/aFPS;

    while(1) {
        Tick++;

        // Rajzolás, animálás
        Draw_background(screen);
        Draw_text_bubble(screen, who, c1, c2, c3);
        Draw_characters(chars, screen);
        Animate_actions(Tick, chars, projs);
        Animate_background(Tick, chars, *projs, player);
        SDL_Flip(screen);

        // Itt se fusson 100%on a proci
        if( NextTick > SDL_GetTicks() )
            SDL_Delay( NextTick - SDL_GetTicks() );
        NextTick = SDL_GetTicks() + 1000/aFPS;

        // Akkor lép ki, ha lejárt az idő, amit paraméterben kapott hogy ennyit várnia kell
        if(start + ms < GetTicks()) {
            paused += GetTicks() - start;
            aNextTick = GetTicks();
            dNextTick = GetTicks();
            return;
        }
        // Vagy ha a felhasználó "megnyomta az any key gombot"
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_KEYUP || event.type == SDL_MOUSEBUTTONUP) {
                paused += GetTicks() - start;
                aNextTick = GetTicks();
                dNextTick = GetTicks();
                return;
            }
        }
    }
}

/** Fontok felszabadítása */
void Close_Fonts() {
    TTF_CloseFont(Knig11);
    TTF_CloseFont(Knig35);
    TTF_CloseFont(Knig60);
    TTF_CloseFont(Cour10);
    TTF_CloseFont(Cour12);
    TTF_CloseFont(Cour15);
    TTF_CloseFont(Cour20);
    TTF_CloseFont(Cour20B);
    TTF_CloseFont(Cour25);
    TTF_CloseFont(Cour35B);
    TTF_CloseFont(Cour45);

    TTF_Quit();
}

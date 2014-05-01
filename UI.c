#include <SDL.h>
#include <SDL_image.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_ttf.h>
#include "imgload.h"
#include "blit.h"
#include "level.h"
#include "text.h"
#include "main.h"
#include "UI.h"
#include "menu.h"

/* Globális, kívülről is elérhető változók */
SDL_Surface *cursor;

/* Globális, csak a modulon belülről elérhető változók */
// Élet kirajzolásához képek:
static SDL_Surface *hp_bar, *hp_line, *hp_start, *hp_end, *hp_tick;
// Skillek cooldown ikonjait tartalmazó képek, és az azokat megnevező enum
static SDL_Surface ***skill_icons, *skill_bg, *current_bg;
enum skill {skill_a_melee, skill_s_melee, skill_s_walk, skill_def, skill_jump_attack1, skill_jump_attack2, skill_counter, skill_a_ranged, skill_s_ranged, skill_num};

/* Saját függvények: */
static void Draw_hp_bar (SDL_Surface *screen, struct character *player, struct character *Woody, struct character *boss);
static void Draw_hp_bar2 (int percent, SDL_Surface *screen, int x, int y, char *s);
static void Draw_skill_icons (SDL_Surface *screen, struct character *player);
static void Draw_benchmark_UI(SDL_Surface *screen, struct character *chars, int enemies_killed);


/** Inicializálja a hp bar kirajzolásához szükséges, globális
  * változókban eltárolt képeket (csak a képre mutató pointer
  * globális, az meg nem foglal sok helyet). */
void Hp_bar_init (void) {
    hp_bar = IMG_Load("./sprite/etc/hp_bar.gif");
    SDL_SetColorKey(hp_bar,SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(hp_bar->format,0,0,0));
    DisplayFormat(&hp_bar);

    hp_line = IMG_Load("./sprite/etc/hp_line.gif");
    DisplayFormat(&hp_line);

    hp_start = IMG_Load("./sprite/etc/hp_start.gif");
    SDL_SetColorKey(hp_start,SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(hp_start->format,0,0,0));
    DisplayFormat(&hp_start);

    hp_end = FlipSurface(hp_start);

    hp_tick = IMG_Load("./sprite/etc/hp_tick.gif");
    SDL_SetColorKey(hp_tick,SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(hp_tick->format,0,0,0));
    DisplayFormat(&hp_tick);
}

/** Inicializálja a képek skilljeit */
void Init_skill_icons(void) {
    skill_icons = (SDL_Surface***) malloc (2 * sizeof(SDL_Surface**));
    skill_icons[0] = (SDL_Surface**) malloc (skill_num * sizeof(SDL_Surface*));
    skill_icons[1] = (SDL_Surface**) malloc (skill_num * sizeof(SDL_Surface*));
    skill_icons[0][skill_a_melee] = IMG_Load("./sprite/Skill_icons/a_melee.gif");
    skill_icons[1][skill_a_melee] = IMG_Load("./sprite/etc/a_melee_key.gif");
    skill_icons[0][skill_a_ranged] = IMG_Load("./sprite/Skill_icons/a_ranged.gif");
    skill_icons[1][skill_a_ranged] = IMG_Load("./sprite/etc/a_ranged_key.gif");
    skill_icons[0][skill_counter] = IMG_Load("./sprite/Skill_icons/counter.gif");
    skill_icons[1][skill_counter] = IMG_Load("./sprite/etc/counter_key.gif");
    skill_icons[0][skill_def] = IMG_Load("./sprite/Skill_icons/def.gif");
    skill_icons[1][skill_def] = IMG_Load("./sprite/etc/def_key.gif");
    skill_icons[0][skill_jump_attack1] = IMG_Load("./sprite/Skill_icons/jump_attack1.gif");
    skill_icons[1][skill_jump_attack1] = IMG_Load("./sprite/etc/jump_attack1_key.gif");
    skill_icons[0][skill_jump_attack2] = IMG_Load("./sprite/Skill_icons/jump_attack2.gif");
    skill_icons[1][skill_jump_attack2] = IMG_Load("./sprite/etc/jump_attack2_key.gif");
    skill_icons[0][skill_s_melee] = IMG_Load("./sprite/Skill_icons/s_melee.gif");
    skill_icons[1][skill_s_melee] = IMG_Load("./sprite/etc/s_melee_key.gif");
    skill_icons[0][skill_s_ranged] = IMG_Load("./sprite/Skill_icons/s_ranged.gif");
    skill_icons[1][skill_s_ranged] = IMG_Load("./sprite/etc/s_ranged_key.gif");
    skill_icons[0][skill_s_walk] = IMG_Load("./sprite/Skill_icons/s_walk.gif");
    skill_icons[1][skill_s_walk] = IMG_Load("./sprite/etc/s_walk_key.gif");
    int i, j;
    for(j=0; j<2; j++)
        for(i=0; i<skill_num; i++) {
            SDL_SetColorKey(skill_icons[j][i],SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(skill_icons[j][i]->format,0,0,0));
            SDL_SetAlpha(skill_icons[j][i], SDL_SRCALPHA|SDL_RLEACCEL , 200);
            SetSize(&skill_icons[j][i], (double)kx/1920 * skill_icons[j][i]->w, (double)kx/1920 * skill_icons[j][i]->h);
            // Ez úgy néz ki jól ha FULL HD mellett nincs raja nagyitás, azaz a kisebb felbontásokon le kell kicsinyiteni
        }
    skill_bg = IMG_Load("./sprite/etc/skill_bg.gif");
    SetSize(&skill_bg, (double)kx/1920 * skill_bg->w, (double)kx/1920 * skill_bg->h);
    current_bg = IMG_Load("./sprite/etc/current_bg.gif");
    SDL_SetColorKey(current_bg,SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(current_bg->format,0,0,0));
    SetSize(&current_bg, (double)kx/1920 * current_bg->w, (double)kx/1920 * current_bg->h);
}

/** Inicializálja a kurzort (máig nem tudom hogy miért nem volt nekem jó az oprendszer kurzorja) */
void Init_cursor(void) {
    cursor = ImgLoader_no_smooth("./sprite/etc/cursor.gif");
}

/** Kirajzolja a kurzort vagy egy menüt. Első látásra picit nehéz megérteni hogy
  * ez miért kell, ha a menük közbe is van kurzor, de e mögött az áll, hogy a
  * menük alatt nem a főciklus fut (nyilván). De ez azzal is jár, hogy nem tudja
  * mindig újra rajzolni részletenként a képet. Így viszont szüksége van egy olyan
  * képre, amit használhat a kurzor letörlésére, és nyilván ez egy olyan kép amin
  * nincs rajta a kurzor */
void Draw_cursor_or_menu(SDL_Surface *screen, int enemies_killed, int *mouse_x, int *mouse_y, bool *alive, bool *retry, bool *quit, bool levelup) {
    if(*alive == false)
        menu = Death;
    switch (menu) {
    case None:
        if(!levelup && !benchmark)
            Draw_cursor(screen, *mouse_x, *mouse_y);
        break;
    case Quit:
        *retry = Quit_menu(screen, mouse_x, mouse_y);
        if(!(*retry))
            *alive = false;
        break;
    case Death:
        *retry = Death_menu(screen, enemies_killed, mouse_x, mouse_y);
        break;
    }
    menu = None;
}

/** Kirajzolja a kurzort **/
void Draw_cursor(SDL_Surface *screen, int mouse_x, int mouse_y) {
    SDL_Rect destRect = {mouse_x, mouse_y, 0, 0};
    BlitSurface(cursor, NULL, screen, &destRect);
}

/** A legtöbb mód esetén kiírja néhány karakter életét, meg kirajzolja a játékos skilljeit.
  * Benchmark módban viszont az FPS-t, illetve a két ellenséges oldallal kapcsolatos
  * információkat ír ki. */
void Draw_UI(SDL_Surface *screen, struct character *chars, struct character *player, struct character *Woody, struct character *boss, int enemies_killed) {
    if(!benchmark) {
        Draw_hp_bar(screen, player, Woody, boss);
        Draw_skill_icons(screen, player);
    } else
        Draw_benchmark_UI(screen, chars, enemies_killed);
}

/** Eldönti hogy kiknek az életét kell kiírni, hova, és hogy hívják őket */
static void Draw_hp_bar (SDL_Surface *screen, struct character *player, struct character *Woody, struct character *boss) {
    if(player != NULL)
        Draw_hp_bar2(player->hp*100/player->maxhp, screen, 10 * scale, (top ? (double)kx/1920 * 20 : ky - (double)kx/1920 * 65), "You");
    if(Woody != NULL)
        Draw_hp_bar2(Woody->hp*100/Woody->maxhp, screen, 10 * scale, (top ? (double)kx/1920 * 20 + hp_bar->h*scale : ky - (double)kx/1920 * 65 - hp_bar->h*scale), "Woody");
    if(boss != NULL)
        Draw_hp_bar2(boss->hp*100/boss->maxhp, screen, kx - hp_bar->w*scale - 10 * scale, (top ? (double)kx/1920 * 20 : ky - (double)kx/1920 * 65), "Boss");
}

/** Elvégzi a hp bar konkrét kirajzolását */
static void Draw_hp_bar2 (int percent, SDL_Surface *screen, int x, int y, char *s) {
    SDL_Surface *current = SDL_CreateRGBSurface(SDL_ANYFORMAT, hp_bar->w, hp_bar->h, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
    DisplayFormat(&current);

    SDL_Rect rect = {0,0,0,0};

    SDL_BlitSurface(hp_bar, NULL, current, NULL);
    if(percent > 0) {
        rect.x = 16;
        rect.y = 10;
        SDL_BlitSurface(hp_start, NULL, current, &rect);
    }
    int i;
    if(percent > 2) {
        rect.x += 5;
        for(i=0; i < (percent - 2) * 2.4; i++) {
            SDL_BlitSurface(hp_line, NULL, current, &rect);
            rect.x++;
        }
        SDL_BlitSurface(hp_end, NULL, current, &rect);
    }
    rect.x = 13 + 61;
    rect.y = 16;
    for(i=0; i<3; i++) {
        SDL_BlitSurface(hp_tick, NULL, current, &rect);
        rect.x += 61;
    }
    rect.x = x;
    rect.y = y;

    DisplayFormat(&current);
    Scale(&current);
    BlitSurface(current, NULL, screen, &rect);
    SDL_FreeSurface(current);
    SDL_Color white = {255, 255, 255};
    current = RenderText(Cour15, s, white);
    rect.y = y - current->h/2;
    rect.x = x + hp_bar->w*scale/2 - current->w/2;
    BlitSurface(current, NULL, screen, &rect);
}

/** A skilleket, cooldownjaikat, és hotkeyjeiket kirajzoló fv.
  * A jelenlegi állapotban nem használható skilleketre rárajzol
  * egy félig átlátszó fekete négyzetet, míg a cooldownok mutatá-
  * sához pedig egy fekete polygonnal fedi le a négyzet megfelelő
  * %-ának megfelelő területét */
#include "character.h"
static void Draw_skill_icons (SDL_Surface *screen, struct character *player) {
    SDL_Rect rect= {350 * scale, (top ? 0 : (practise ? ky - (double)kx/1920 * 150 : ky - (double)kx/1920 * 90))
                    , skill_icons[0][0]->w, skill_icons[0][0]->h
                   };
    if(!top && kyBottom > rect.y - 10*scale)
        kyBottom = rect.y - 10*scale;
    int i,j;
    for(j=0; j < (practise ? 2 : 1); j++) {
        for(i=0; i<skill_num; i++) {
            if(j == 0)
                BlitSurface(skill_bg, NULL, screen, &rect);
            BlitSurface(skill_icons[j][i], NULL, screen, &rect);
            // A Cooldownok kirajzolása
            if(j == 0) {
                int time = GetTicks();
                int percent = 0;
                bool available = false;
                bool current = false;
                switch (i) {
                case skill_a_melee:
                    if(player->act == a_walk)
                        available = true;
                    if(player->act == a_melee)
                        current = true;
                    if(player->melee.a_cd > time)
                        percent = ((player->melee.a_cd - time) * 100) / player->a_melee_cd;
                    break;
                case skill_s_melee:
                    if(player->act == a_walk || player->act == jump || player->act == air)
                        available = true;
                    if(player->act == s_melee)
                        current = true;
                    if(player->melee.s_cd > time)
                        percent = ((player->melee.s_cd - time) * 100) / player->s_melee_cd;
                    break;
                case skill_s_walk:
                    if( (player->act == a_walk || player->act == jump) && player->jump.second == false)
                        available = true;
                    if(player->act == s_walk)
                        current = true;
                    if(player->walk.cd > time)
                        percent = ((player->walk.cd - time) * 100) / player->s_walk_cd;
                    break;
                case skill_def:
                    if(player->act == a_walk || player->act == a_melee)
                        available = true;
                    if(player->act == def)
                        current = true;
                    if(player->def.cd > time)
                        percent = ((player->def.cd - time) * 100) / player->def_cd;
                    break;
                case skill_jump_attack1:
                    if(player->act == jump && player->jump.second == false)
                        available = true;
                    if(player->act == jump_attack)
                        current = true;
                    if(player->jump.cd1 > time)
                        percent = ((player->jump.cd1 - time) * 100) / player->jump_attack_cd;
                    break;
                case skill_jump_attack2:
                    if(player->act == jump && player->jump.second == false)
                        available = true;
                    if(player->act == jump_attack2)
                        current = true;
                    if(player->jump.cd2 > time)
                        percent = ((player->jump.cd2 - time) * 100) / player->jump_attack_cd;
                    break;
                case skill_counter:
                    if(player->act == a_walk || player->act == hit
                            || player->act == a_melee || player->act == ground)
                        available = true;
                    if(player->act == counter)
                        current = true;
                    if(player->counter.cd > time)
                        percent =  ((player->counter.cd - time) * 100) / player->counter_cd;
                    break;
                case skill_a_ranged:
                    if(player->act == a_walk)
                        available = true;
                    if(player->act == a_ranged)
                        current = true;
                    if(player->ranged.a_cd > time)
                        percent =  ((player->ranged.a_cd - time) * 100) / player->a_cast_cd;
                    break;
                case skill_s_ranged:
                    if(player->act == a_walk)
                        available = true;
                    if(player->act == s_ranged)
                        current = true;
                    if(player->ranged.s_cd > time)
                        percent =  ((player->ranged.s_cd - time) * 100) / player->s_cast_cd;
                    break;
                default:
                    break;
                }
                if(!available && !current)
                    boxRGBA(screen, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, 0, 0, 0, 125);
                if(percent != 0 && !current) { // A hátralévő cooldown kirajzolása
                    int w =skill_icons[j][i]->w, h = skill_icons[j][i]->h;
                    Sint16 x[7] = {w/2 + w*(100-percent)/24, w, w, 0, 0, w/2, w/2};
                    Sint16 y[7] = {0, 0, h, h, 0, 0, h/2};
                    int n;
                    if(percent > 87) {
                        n=7;
                    } else if(percent > 62) {
                        n=6;
                        y[1] = h*(88-percent)/25;
                    } else if(percent > 37) {
                        n=5;
                        x[2] = w*(percent-38)/25;
                    } else if(percent > 12) {
                        n=4;
                        y[3] = h*(percent-13)/25;
                    } else {
                        n=3;
                        x[4] = w*(13-percent)/24;
                    }
                    int k;
                    for(k=0; k<7; k++) {
                        x[k] += rect.x;
                        y[k] += rect.y;
                    }
                    filledPolygonRGBA(screen, (const Sint16*) (x + 7-n), (const Sint16*) (y + 7-n), n, 0, 0, 0, 125);
                }
                if(current)
                    BlitSurface(current_bg, NULL, screen, &rect);
            }
            rect.x += skill_icons[j][i]->w + 1;
        }
        rect.x = 350 * scale;
        rect.y += (int)((double)kx/1920 * 90);
    }
}

/** Benchmark módban kiírja az FPS-t, illetve hogy az egyes oldalaknak hány %-nyi élete van */
static void Draw_benchmark_UI(SDL_Surface *screen, struct character *chars, int enemies_killed) {
    // FPS kiírása
    char c[50];
    sprintf(c, "FPS: %d", fps);
    SDL_Color white = {255,255,255};
    SDL_Surface *text = RenderText(Cour45, c, white);
    SDL_Rect fpsRect = {kx/2 - text->w/2, ky - text->h/2 - 5*(ky-kyBottom)/12, 0, 0};
    BlitSurface(text, NULL, screen, &fpsRect);
    SDL_FreeSurface(text);

    // Bal oldali (LittleFighters' health) szöveg kiírása
    SDL_Color blue = {0, 0, 255};
    text = RenderText(Cour20B, "LittleFighters' health:", blue);
    SDL_Rect allytextRect = {15*scale , kyBottom + 15*scale, text->w, text->h};
    BlitSurface(text, NULL, screen, &allytextRect);
    SDL_FreeSurface(text);

    // Bal oldali (LittleFighters' health) számérték kiszámolása és kiírása
    long hp, maxhp;
    Total_hp(chars, friendly, &hp, &maxhp);
    int percent = hp * 100 / (maxhp * 30);
    sprintf(c, "%d%%", percent);
    text = RenderText(Cour35B, c, blue);
    SDL_Rect allyhpRect = {allytextRect.x + allytextRect.w/2 - text->w/2 , allytextRect.y + allytextRect.h + 10*scale, 0, 0};
    BlitSurface(text, NULL, screen, &allyhpRect);
    SDL_FreeSurface(text);

    // Jobb oldali (Bandits' health) szöveg kiírása
    SDL_Color red = {255, 0, 0};
    text = RenderText(Cour20B, "Bandits' health:", red);
    SDL_Rect enemytextRect = {kx - text->w - 15*scale , kyBottom + 15*scale, text->w, text->h};
    BlitSurface(text, NULL, screen, &enemytextRect);
    SDL_FreeSurface(text);

    // Jobb oldali (Bandits' health) számérték kiszámolása és kiírása
    Total_hp(chars, enemy, &hp, &maxhp);
    if(enemies_killed < 60)
        percent = (hp + (60 - enemies_killed) * maxhp) * 100 / (maxhp * 90);
    else
        percent = hp * 100 / (maxhp * 90);
    sprintf(c, "%d%%", percent);
    text = RenderText(Cour35B, c, red);
    SDL_Rect enemyhpRect = {enemytextRect.x + enemytextRect.w/2 - text->w/2 , enemytextRect.y + enemytextRect.h + 10*scale, 0, 0};
    BlitSurface(text, NULL, screen, &enemyhpRect);
    SDL_FreeSurface(text);
}

/** --- Felszabadítások --- **/
void Free_hp_bar(void) {
    SDL_FreeSurface(hp_bar);
    SDL_FreeSurface(hp_line);
    SDL_FreeSurface(hp_start);
    SDL_FreeSurface(hp_end);
    SDL_FreeSurface(hp_tick);
}

void Free_skill_bar(void) {
    int i,j;
    for(j=0; j < 2; j++)
        for(i=0; i<skill_num; i++)
            SDL_FreeSurface(skill_icons[j][i]);
    for(j=0; j < 2; j++)
        free(skill_icons[j]);
    free(skill_icons);
    SDL_FreeSurface(skill_bg);
    SDL_FreeSurface(current_bg);
}

void Free_cursor(void) {
    SDL_FreeSurface(cursor);
}

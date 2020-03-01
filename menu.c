#include <SDL.h>
#include <SDL_image.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_ttf.h>
#include "main.h"
#include "imgload.h"
#include "UI.h"
#include "bg.h"
#include "AI.h"
#include "level.h"
#include "menu.h"
#include "music.h"
#include "text.h"

enum menu_type menu = None;

/** A játék elindulásakor a főmenü kirajzolása */
int Draw_menu(SDL_Surface *screen) {
    //Háttér zene
    playmusic(mus_main);

    // Háttér kép
    SDL_Surface *menu = IMG_Load("menu.gif");
    SetSize( &menu, kx, ky);
    SDL_BlitSurface(menu, NULL, screen, NULL);
    SDL_FreeSurface(menu);

    // Jobb alsó szöveg
    SDL_Color grey = {140, 140, 140};
    SDL_Surface *text = RenderText(Cour15, "This game was created by", grey);
    int w = text->w;
    SDL_Rect textRect = {kx - text->w - 10, ky - 2 * text->h - 20, 0, 0};
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);
    text = RenderText(Cour15, "Tam\xC3\xA1s Csala", grey);
    textRect.x += (w / 2 - text->w/2);
    textRect.y = ky - text->h - 10;
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);

    SDL_Color white = {255, 255, 255};
    text = RenderText(Knig35, "Story mode", white);
    SDL_Rect storyRect = {kx/2 - text->w/2, 2*ky/5, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &storyRect);
    SDL_FreeSurface(text);
    text = RenderText(Knig35, "Survival mode", white);
    SDL_Rect survivalRect = {kx/2 - text->w/2, storyRect.y + text->h + 10*scale, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &survivalRect);
    SDL_FreeSurface(text);
    text = RenderText(Knig35, "Skill practise arena", white);
    SDL_Rect practiseRect = {kx/2 - text->w/2, survivalRect.y + text->h + 10*scale, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &practiseRect);
    SDL_FreeSurface(text);
    text = RenderText(Knig35, "Benchmark", white);
    SDL_Rect benchmarkRect = {kx/2 - text->w/2, practiseRect.y + text->h + 10*scale, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &benchmarkRect);
    SDL_FreeSurface(text);
    text = RenderText(Knig35, "Quit", white);
    SDL_Rect quitRect = {kx/2 - text->w/2, benchmarkRect.y + text->h + 10*scale, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &quitRect);
    SDL_FreeSurface(text);

    SDL_Flip(screen);
    SDL_Surface *screen_wo_cursor = SDL_CreateRGBSurface(SDL_ANYFORMAT, kx, ky,  screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
    SDL_BlitSurface(screen, NULL, screen_wo_cursor, NULL);

    SDL_Event event;
    bool mouse_movement = false;
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    while(1) {
        if(mouse_movement) {
            SDL_BlitSurface(screen_wo_cursor, NULL, screen, NULL);
            Draw_cursor(screen, mouse_x, mouse_y);
            SDL_Flip(screen);
            mouse_movement = false;
        }
        while( SDL_PollEvent( &event ) ) {
            if(event.type == SDL_KEYDOWN)
                if( event.key.keysym.sym  == SDLK_ESCAPE) {
                    SDL_FreeSurface(screen_wo_cursor);
                    return 1;
                }
            if(event.type == SDL_MOUSEBUTTONUP)
                if(event.button.button == SDL_BUTTON_LEFT) {
                    if(storyRect.x < event.motion.x && event.motion.x < storyRect.x + storyRect.w)
                        if(storyRect.y < event.motion.y && event.motion.y < storyRect.y + storyRect.h) {
                            if( Select_Difficulty(screen) ) {
                                level = 1;
                                current_map = forest;
                                SDL_FreeSurface(screen_wo_cursor);
                                return 0;
                            } else {
                                SDL_BlitSurface(screen_wo_cursor, NULL, screen, NULL);
                                Draw_cursor(screen, mouse_x, mouse_y);
                                SDL_Flip(screen);
                            }

                        }
                    if(survivalRect.x < event.motion.x && event.motion.x < survivalRect.x + survivalRect.w)
                        if(survivalRect.y < event.motion.y && event.motion.y < survivalRect.y + survivalRect.h) {
                            if( Select_Difficulty(screen) ) {
                                level = lvl_Survival;
                                current_map = colisseum;
                                SDL_FreeSurface(screen_wo_cursor);
                                return 0;
                            } else {
                                SDL_BlitSurface(screen_wo_cursor, NULL, screen, NULL);
                                Draw_cursor(screen, mouse_x, mouse_y);
                                SDL_Flip(screen);
                            }
                        }
                    if(practiseRect.x < event.motion.x && event.motion.x < practiseRect.x + practiseRect.w)
                        if(practiseRect.y < event.motion.y && event.motion.y < practiseRect.y + practiseRect.h) {
                            diff = Easy;
                            practise = true;
                            level = lvl_Practise_Arena;
                            current_map = practise_arena;
                            SDL_FreeSurface(screen_wo_cursor);
                            return 0;
                        }
                    if(benchmarkRect.x < event.motion.x && event.motion.x < benchmarkRect.x + benchmarkRect.w)
                        if(benchmarkRect.y < event.motion.y && event.motion.y < benchmarkRect.y + benchmarkRect.h) {
                            diff = Hard;
                            benchmark = true;
                            level = lvl_Benchmark;
                            current_map = colisseum;
                            SDL_FreeSurface(screen_wo_cursor);
                            return 0;
                        }
                    if(quitRect.x < event.motion.x && event.motion.x < quitRect.x + quitRect.w)
                        if(quitRect.y < event.motion.y && event.motion.y < quitRect.y + quitRect.h) {
                            SDL_FreeSurface(screen_wo_cursor);
                            return 1;
                        }
                }
        }
        if(event.type == SDL_MOUSEMOTION) {
            mouse_x = event.motion.x;
            mouse_y = event.motion.y;
            mouse_movement = true;
        }
    }
    return 0;
}

/** Főmenüből megnyíló almenü, amiben a játék nehézségét lehet kiválasztani */
int Select_Difficulty(SDL_Surface *screen) {
    // Háttér kép
    SDL_Surface *menu = IMG_Load("menu.gif");
    SetSize( &menu, kx, ky);
    SDL_BlitSurface(menu, NULL, screen, NULL);
    SDL_FreeSurface(menu);

    // Jobb alsó szöveg
    SDL_Color grey = {140, 140, 140};
    SDL_Surface *text = RenderText(Cour15, "This game was created by", grey);
    int w = text->w;
    SDL_Rect textRect = {kx - text->w - 10, ky - 2 * text->h - 20, 0, 0};
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);
    text = RenderText(Cour15, "Tam\xC3\xA1s Csala", grey);
    textRect.x += (w / 2 - text->w/2);
    textRect.y = ky - text->h - 10;
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);

    SDL_Color white = {255, 255, 255};
    text = RenderText(Knig35, "Select Difficulty", white);
    textRect.x = kx/2 - text->w/2;
    textRect.y = 2*ky/5;
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);
    text = RenderText(Knig35, "Easy", white);
    SDL_Rect easyRect = {kx/2 - text->w/2 - 100*scale, 2*ky/5 + text->h + 10*scale, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &easyRect);
    SDL_FreeSurface(text);
    text = RenderText(Knig35, "Normal", white);
    SDL_Rect normalRect = {kx/2 - text->w/2, 2*ky/5 + text->h + 10*scale, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &normalRect);
    SDL_FreeSurface(text);
    text = RenderText(Knig35, "Hard", white);
    SDL_Rect hardRect = {kx/2 - text->w/2 + 100*scale, 2*ky/5 + text->h + 10*scale, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &hardRect);
    SDL_FreeSurface(text);

    SDL_Flip(screen);
    SDL_Surface *screen_wo_cursor = SDL_CreateRGBSurface(SDL_ANYFORMAT, kx, ky, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
    SDL_BlitSurface(screen, NULL, screen_wo_cursor, NULL);

    SDL_Event event;
    bool mouse_movement = false;
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    while(1) {
        if(mouse_movement) {
            SDL_BlitSurface(screen_wo_cursor, NULL, screen, NULL);
            Draw_cursor(screen, mouse_x, mouse_y);
            SDL_Flip(screen);
            mouse_movement = false;
        }
        while( SDL_PollEvent( &event ) ) {
            if(event.type == SDL_KEYDOWN)
                if( event.key.keysym.sym  == SDLK_ESCAPE) {
                    SDL_FreeSurface(screen_wo_cursor);
                    return 0;
                }
            if(event.type == SDL_MOUSEBUTTONUP)
                if(event.button.button == SDL_BUTTON_LEFT) {
                    if(easyRect.x < event.motion.x && event.motion.x < easyRect.x + easyRect.w)
                        if(easyRect.y < event.motion.y && event.motion.y < easyRect.y + easyRect.h) {
                            diff = Easy;
                            practise = true;
                            SDL_FreeSurface(screen_wo_cursor);
                            return 1;
                        }
                    if(normalRect.x < event.motion.x && event.motion.x < normalRect.x + normalRect.w)
                        if(normalRect.y < event.motion.y && event.motion.y < normalRect.y + normalRect.h) {

                            diff = Normal;
                            SDL_FreeSurface(screen_wo_cursor);
                            return 1;
                        }
                    if(hardRect.x < event.motion.x && event.motion.x < hardRect.x + hardRect.w)
                        if(hardRect.y < event.motion.y && event.motion.y < hardRect.y + hardRect.h) {
                            diff = Hard;
                            SDL_FreeSurface(screen_wo_cursor);
                            return 1;
                        }
                }
        }
        if(event.type == SDL_MOUSEMOTION) {
            mouse_x = event.motion.x;
            mouse_y = event.motion.y;
            mouse_movement = true;
        }
    }
    return 0;
}

/** A játék töltése alatt kirajzolt képet létrehozó fv. */
void Loading_screen(SDL_Surface *screen) {
    SDL_Surface *menu = IMG_Load("menu.gif");
    SetSize( &menu, kx, ky);
    SDL_BlitSurface(menu, NULL, screen, NULL);
    SDL_FreeSurface(menu);

    SDL_Color white = {255, 255, 255};
    SDL_Surface *text = RenderText(Knig60, "Loading", white);
    SDL_Rect textRect = {kx/2 - text->w/2, ky/2, 0, 0};
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);

    SDL_Color grey = {140, 140, 140};
    text = RenderText(Cour15, "This game was created by", grey);
    int w = text->w;
    textRect.x = kx - text->w - 10;
    textRect.y = ky - 2 * text->h - 20;
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);
    text = RenderText(Cour15, "Tam\xC3\xA1s Csala", grey);
    textRect.x += (w / 2 - text->w/2);
    textRect.y = ky - text->h - 10;
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);

    SDL_Flip(screen);
}

/** Játék közbe az Escape megnyomásával (vagy alt+tabbal) behozható menü */
int Quit_menu(SDL_Surface *screen, int *mouse_x, int *mouse_y) {
    Uint32 pause_start = GetTicks();

    SDL_Surface *menu = SDL_CreateRGBSurface(SDL_ANYFORMAT, kx/8, kx/16, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
    // A menünek csak egyes részei lesznek átlátszóak, ezért először rámásolom a menüre a háttér megfelelő részét
    SDL_Rect menuRect = {kx/2 - menu->w/2, ky/2 - menu->h/2, menu->w, menu->h};

    // A félig átlátszó fekete négyzet
    boxRGBA(menu, 0, 0, menu->w, menu->h, 0, 0, 0, 200);
    // Szegély
    lineRGBA(menu, 0, 0, 0, menu->h - 1, 8, 8, 8, 255);
    rectangleRGBA(menu, 1, 0, menu->w - 1, menu->h - 1, 34, 34, 34, 255);
    rectangleRGBA(menu, 2, 1, menu->w - 2, menu->h - 2, 15, 15, 15, 255);
    SDL_Surface *text;

    // A gombok
    SDL_Surface *button = IMG_Load("./sprite/etc/button.gif");
    SetSize(&button, 5 * menu->w / 6, (5 * menu->w / 6) * button->h / button->w);
    SDL_SetAlpha(button, SDL_SRCALPHA|SDL_RLEACCEL , 230);

    SDL_Color white = {255, 255, 255};

    SDL_Rect resumeButton = {menu->w/2 - button->w/2, menu->h/2 - button->h/2 - menu->h/5, button->w, button->h};
    SDL_BlitSurface(button, NULL, menu, &resumeButton);

    text = RenderText(Cour15, "Resume", white);
    SDL_Rect resumeRect = {menu->w/2 - text-> w/2, menu->h/2 - text->h/2 - menu->h/5, text->w, text->h};
    SDL_BlitSurface(text, NULL, menu, &resumeRect);
    SDL_FreeSurface(text);

    SDL_Rect quitButton = {menu->w/2 - button-> w/2, menu->h/2 - button->h/2 + menu->h/5, button->w, button->h};
    SDL_BlitSurface(button, NULL, menu, &quitButton);

    text = RenderText(Cour15, "Quit", white);
    SDL_Rect quitRect = {menu->w/2 - text-> w/2, menu->h/2 - text->h/2 + menu->h/5, text->w, text->h};
    SDL_BlitSurface(text, NULL, menu, &quitRect);
    SDL_FreeSurface(text);
    SDL_FreeSurface(button);

    // A fejléc a menühöz
    SDL_Surface *title = IMG_Load("./sprite/etc/button.gif");
    SetSize(&title, menu->w, menu->w * title->h / title->w);
    text = RenderText(Cour12, "Little Fighter", white);
    SDL_Rect titletextRect = {title->w/2 - text->w/2, title->h/2 - text->h/2, text->w, text->h};
    SDL_BlitSurface(text, NULL, title, &titletextRect);
    SDL_FreeSurface(text);

    SDL_Rect titleRect = {menuRect.x, menuRect.y - title->h, menuRect.w, title->h};
    SDL_BlitSurface(title, NULL, screen, &titleRect);
    SDL_FreeSurface(title);

    SDL_BlitSurface(menu, NULL, screen, &menuRect);
    SDL_FreeSurface(menu);

    SDL_Surface *screen_wo_cursor = SDL_CreateRGBSurface(SDL_ANYFORMAT, screen->w, screen->h, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
    SDL_BlitSurface(screen, NULL, screen_wo_cursor, NULL);

    Draw_cursor(screen, *mouse_x, *mouse_y);

    SDL_Flip(screen);

    SDL_Event event;
    bool mouse_movement = false;
    while(1) {
        if(mouse_movement) {
            SDL_BlitSurface(screen_wo_cursor, NULL, screen, NULL);
            Draw_cursor(screen, *mouse_x, *mouse_y);
            SDL_Flip(screen);
            mouse_movement = false;
        }
        while( SDL_PollEvent( &event ) ) {
            if(event.type == SDL_KEYDOWN)
                if( event.key.keysym.sym  == SDLK_ESCAPE) {
                    paused += GetTicks() - pause_start;
                    aNextTick = dNextTick = GetTicks();
                    SDL_FreeSurface(screen_wo_cursor);
                    return 1;
                }
            if(event.type == SDL_MOUSEBUTTONUP)
                if(event.button.button == SDL_BUTTON_LEFT) {
                    if(menuRect.x + resumeButton.x < event.motion.x && event.motion.x < menuRect.x + resumeButton.x + resumeButton.w)
                        if(menuRect.y + resumeButton.y < event.motion.y && event.motion.y < menuRect.y + resumeButton.y + resumeButton.h) {
                            paused += GetTicks() - pause_start;
                            aNextTick = dNextTick = GetTicks();
                            SDL_FreeSurface(screen_wo_cursor);
                            return 1;
                        }
                    if(menuRect.x + quitButton.x < event.motion.x && event.motion.x < menuRect.x + quitButton.x + quitButton.w)
                        if(menuRect.y + quitButton.y < event.motion.y && event.motion.y < menuRect.y + quitButton.y + quitButton.h) {
                            paused += GetTicks() - pause_start;
                            aNextTick = dNextTick = GetTicks();
                            SDL_FreeSurface(screen_wo_cursor);
                            return 0;
                        }
                }
        }
        if(event.type == SDL_MOUSEMOTION) {
            *mouse_x = event.motion.x;
            *mouse_y = event.motion.y;
            mouse_movement = true;
        }
    }
    paused += GetTicks() - pause_start;
    aNextTick = dNextTick = GetTicks();
    SDL_FreeSurface(screen_wo_cursor);
    return 0;

}

/** A játékos halálakor megjelenő menü */
int Death_menu(SDL_Surface *screen, int enemies_killed, int *mouse_x, int *mouse_y) {
    Uint32 pause_start = GetTicks();
    SDL_Surface *menu = SDL_CreateRGBSurface(SDL_ANYFORMAT, kx/8, kx/8, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
    // A menünek csak egyes részei lesznek átlátszóak, ezért először rámásolom a menüre a háttér megfelelő részét
    SDL_Rect menuRect = {kx/2 - menu->w/2, ky/2 - menu->h/2, menu->w, menu->h};

    // A félig átlátszó fekete négyzet
    boxRGBA(menu, 0, 0, menu->w, menu->h, 0, 0, 0, 200);
    // Szegély
    lineRGBA(menu, 0, 0, 0, menu->h - 1, 8, 8, 8, 255);
    rectangleRGBA(menu, 1, 0, menu->w - 1, menu->h - 1, 34, 34, 34, 255);
    rectangleRGBA(menu, 2, 1, menu->w - 2, menu->h - 2, 15, 15, 15, 255);
    SDL_Surface *text;


    SDL_Color white = {255, 255, 255};

    if(level == lvl_Survival || level == lvl_Benchmark) {
        text = RenderText(Cour10, benchmark ? "Benchmark points:" : "Enemies Killed:", white);
        SDL_Rect Rect = {menu->w/2 - text-> w/2, menu->h/8, text->w, text->h};
        SDL_BlitSurface(text, NULL, menu, &Rect);
        SDL_FreeSurface(text);
        char c[50];
        sprintf(c, "%d", enemies_killed);
        text = RenderText(Cour20, c, white);
        SDL_Rect Rect2 = {menu->w/2 - text-> w/2, menu->h/6 + text->h / 2, text->w, text->h};
        SDL_BlitSurface(text, NULL, menu, &Rect2);
        SDL_FreeSurface(text);
    } else {
        text = RenderText(Cour25, "Defeat!", white);
        SDL_Rect Rect = {menu->w/2 - text-> w/2, menu->h/8, text->w, text->h};
        SDL_BlitSurface(text, NULL, menu, &Rect);
        SDL_FreeSurface(text);
    }


    // A gombok
    SDL_Surface *button = IMG_Load("./sprite/etc/button.gif");
    SetSize(&button, 5 * menu->w / 6, (5 * menu->w / 6) * button->h / button->w);
    SDL_SetAlpha(button, SDL_SRCALPHA|SDL_RLEACCEL , 230);

    SDL_Rect resumeButton = {menu->w/2 - button-> w/2, menu->h/2 - button->h/2, button->w, button->h};
    SDL_BlitSurface(button, NULL, menu, &resumeButton);

    text = RenderText(Cour20, "Retry", white);
    SDL_Rect resumeRect = {menu->w/2 - text-> w/2, menu->h/2 - text->h/2, text->w, text->h};
    SDL_BlitSurface(text, NULL, menu, &resumeRect);
    SDL_FreeSurface(text);

    SDL_Rect quitButton = {menu->w/2 - button-> w/2, menu->h/2 - button->h/2 + menu->h/4, button->w, button->h};
    SDL_BlitSurface(button, NULL, menu, &quitButton);

    text = RenderText(Cour20, "Quit", white);
    SDL_Rect quitRect = {menu->w/2 - text-> w/2, menu->h/2 - text->h/2 + menu->h/4, text->w, text->h};
    SDL_BlitSurface(text, NULL, menu, &quitRect);
    SDL_FreeSurface(text);
    SDL_FreeSurface(button);

    // A fejléc a menühöz
    SDL_Surface *title = IMG_Load("./sprite/etc/button.gif");
    SetSize(&title, menu->w, menu->w * title->h / title->w);
    text = RenderText(Cour12, "Little Fighter", white);
    SDL_Rect titletextRect = {title->w/2 - text->w/2, title->h/2 - text->h/2, text->w, text->h};
    SDL_BlitSurface(text, NULL, title, &titletextRect);
    SDL_FreeSurface(text);

    SDL_Rect titleRect = {menuRect.x, menuRect.y - title->h, menuRect.w, title->h};
    SDL_BlitSurface(title, NULL, screen, &titleRect);
    SDL_FreeSurface(title);

    SDL_BlitSurface(menu, NULL, screen, &menuRect);
    SDL_FreeSurface(menu);

    SDL_Surface *screen_wo_cursor = SDL_CreateRGBSurface(SDL_ANYFORMAT, screen->w, screen->h, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
    SDL_BlitSurface(screen, NULL, screen_wo_cursor, NULL);
    Draw_cursor(screen, *mouse_x, *mouse_y);

    SDL_Flip(screen);

    SDL_Event event;
    bool mouse_movement = false;
    while(1) {
        if(mouse_movement) {
            SDL_BlitSurface(screen_wo_cursor, NULL, screen, NULL);
            Draw_cursor(screen, *mouse_x, *mouse_y);
            SDL_Flip(screen);
            mouse_movement = false;
        }
        while( SDL_PollEvent( &event ) ) {
            if(event.type == SDL_KEYDOWN)
                if( event.key.keysym.sym  == SDLK_ESCAPE) {
                    paused += GetTicks() - pause_start;
                    aNextTick = dNextTick = GetTicks();
                    SDL_FreeSurface(screen_wo_cursor);
                    return 0;
                }
            if(event.type == SDL_MOUSEBUTTONUP)
                // Fontos hogy ez ne mousebuttondown legyen, mert akkor a kilépésnél a főmenübe
                // is érzékelné még kattintást azaz a kilépés után egyből be is lépne a practise arenába
                if(event.button.button == SDL_BUTTON_LEFT) {
                    if(menuRect.x + resumeButton.x < event.motion.x && event.motion.x < menuRect.x + resumeButton.x + resumeButton.w)
                        if(menuRect.y + resumeButton.y < event.motion.y && event.motion.y < menuRect.y + resumeButton.y + resumeButton.h) {
                            paused += GetTicks() - pause_start;
                            aNextTick = dNextTick = GetTicks();
                            SDL_FreeSurface(screen_wo_cursor);
                            return 1;
                        }
                    if(menuRect.x + quitButton.x < event.motion.x && event.motion.x < menuRect.x + quitButton.x + quitButton.w)
                        if(menuRect.y + quitButton.y < event.motion.y && event.motion.y < menuRect.y + quitButton.y + quitButton.h) {
                            paused += GetTicks() - pause_start;
                            aNextTick = dNextTick = GetTicks();
                            SDL_FreeSurface(screen_wo_cursor);
                            return 0;
                        }
                }
            if(event.type == SDL_MOUSEMOTION) {
                *mouse_x = event.motion.x;
                *mouse_y = event.motion.y;
                mouse_movement = true;
            }
        }
    }
    paused += GetTicks() - pause_start;
    aNextTick = dNextTick = GetTicks();
    SDL_FreeSurface(screen_wo_cursor);
    return 0;
}

#include "music.h"
void Credits_menu(SDL_Surface *screen) {
    playmusic(mus_num);
    boxRGBA(screen, 0, 0, kx, ky, 0, 0, 0, 255);
    SDL_Color white = {255, 255, 255};

    SDL_Surface *text = RenderText(Knig60, "LittleFighter", white);
    SDL_Rect titlerect = {kx/2 - text->w/2, 20*scale, 0, 0};
    SDL_BlitSurface(text, NULL, screen, &titlerect);
    SDL_FreeSurface(text);

    text = RenderText(Cour25, "This game was created by Tam\xC3\xA1s Csala.", white);
    SDL_Rect creditsrect = {kx/2 - text->w/2, ky/2 - text->h/2 - 20*scale, 0, 0};
    SDL_BlitSurface(text, NULL, screen, &creditsrect);
    SDL_FreeSurface(text);

    text = RenderText(Cour25, "Thanks for playing! :)", white);
    SDL_Rect thanksrect = {kx/2 - text->w/2, ky/2 - text->h/2 + 60*scale, 0, 0};
    SDL_BlitSurface(text, NULL, screen, &thanksrect);
    SDL_FreeSurface(text);

    SDL_Flip(screen);
}

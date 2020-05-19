#include "main.h"
#include "character.h"
#include "controls.h"
#include "imgload.h"
#include "bg.h"
#include "menu.h"

/** Rögzti a billentyűlenyomásokat */
void Key_Presses(bool keydown[keys_num], bool justpressed[keys_num], bool justreleased[keys_num], int *mouse_x, int *mouse_y, bool *alive, bool *retry, bool *quit) {
    int mod;
    SDL_Event event;

    int i;
    for(i=0; i<keys_num; i++) {
        // Az előző ciklusban lenyomott gombok már nem érdekelnek
        justpressed[i] = false;
        justreleased[i] = false;
    }

    while( SDL_PollEvent( &event ) ) {
        switch( event.type ) {
        case SDL_KEYDOWN:
            switch( event.key.keysym.sym ) {
            case SDLK_SPACE:
                keydown[space] = true;
                justpressed[space] = true;
                break;
            case SDLK_e:
                keydown[e] = true;
                justpressed[e] = true;
                break;
            case SDLK_w:
                keydown[w] = true;
                break;
            case SDLK_a:
                keydown[a] = true;
                break;
            case SDLK_s:
                keydown[s] = true;
                break;
            case SDLK_d:
                keydown[d] = true;
                break;
            case SDLK_ESCAPE:
                menu = Quit;
                break;
            case SDLK_TAB:
                keydown[tab] = true;
            default:
                mod = SDL_GetModState();
                if (mod & KMOD_SHIFT)
                    keydown[left_shift] = true;
                if (mod & KMOD_ALT)
                    keydown[alt] = true;
                break;
            }
            break;
        case SDL_KEYUP:
            switch( event.key.keysym.sym ) {
            case SDLK_e:
                keydown[e] = false;
                justreleased[e] = true;
                break;
            case SDLK_w:
                keydown[w] = false;
                break;
            case SDLK_a:
                keydown[a] = false;
                break;
            case SDLK_s:
                keydown[s] = false;
                break;
            case SDLK_d:
                keydown[d] = false;
                break;
            default:
                mod = SDL_GetModState();
                if ((mod & KMOD_SHIFT) == 0)
                    keydown[left_shift] = false;
                if ((mod & KMOD_ALT) == 0)
                    keydown[alt] = true;
                break;
            }
            break;
        case SDL_QUIT:
            *quit = true;
            *retry = false;
            *alive = false;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_RIGHT) {
                keydown[right_click] = true;
                justpressed[right_click] = true;
                *mouse_x = event.motion.x;
                *mouse_y = event.motion.y;
            }
            if (event.button.button == SDL_BUTTON_LEFT) {
                keydown[left_click] = true;
                justpressed[left_click] = true;
                *mouse_x = event.motion.x;
                *mouse_y = event.motion.y;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_RIGHT) {
                keydown[right_click] = false;
                justreleased[right_click] = true;
            }
            if (event.button.button == SDL_BUTTON_LEFT) {
                keydown[left_click] = false;
                justreleased[left_click] = true;
            }
            break;
        case SDL_MOUSEMOTION: {
            *mouse_x = event.motion.x;
            *mouse_y = event.motion.y;
        }
        break;
        default:
            break;
        } // switch
    }
}

/** A billentyűlenyomások hatásainak kezelése */
void Change_States(bool *keydown, bool *justpressed, bool *justreleased, int mouse_x, int mouse_y, struct character *player) {
    // Benchmark módban nincs player, ezért ekkor ne próbálja meg a player adattagjait módostani a program
    if(!benchmark) {
        /*
        // W
        if(keydown[w] && !keydown[s]) {
            player->walk.vz = -1;
            if(player->act == a_ranged && player->state > 2) {
                player->act = a_walk;
                player->ranged.a_cd = GetTicks() + player->state * player->a_cast_cd / player->a_cast_No;
                player->state = 0;
            } else if (player->act == def) {
                player->act = a_walk;
                player->state = 0;
            }
        }
        // S
        else if(!keydown[w] && keydown[s]) {
            player->walk.vz = 1;
            if(player->act == a_ranged && player->state > 2) {
                player->act = a_walk;
                player->ranged.a_cd = GetTicks() + player->state * player->a_cast_cd / player->a_cast_No;
                player->state = 0;
            } else if (player->act == def) {
                player->act = a_walk;
                player->state = 0;
            }
        }
        // alap
        else if(!keydown[w] && !keydown[s])
            player->walk.vz = 0;
        */

        // A
        if(keydown[a] && !keydown[d]) {
            if(player->act == a_walk || player->act == jump)
                player->dir = left;
            player->walk.vx = -1;
            if(player->act == a_ranged && player->state > 2) {
                player->act = a_walk;
                player->dir = left;
                player->ranged.a_cd = GetTicks() + player->state * player->a_cast_cd / player->a_cast_No;
                player->state = 0;
            } else if (player->act == def) {
                player->act = a_walk;
                player->dir = left;
                player->state = 0;
            }
        }
        // D
        else if(!keydown[a] && keydown[d]) {
            if(player->act == a_walk || player->act == jump)
                player->dir = right;
            player->walk.vx = 1;
            if(player->act == a_ranged && player->state > 2) {
                player->act = a_walk;
                player->dir = right;
                player->ranged.a_cd = GetTicks() + player->state * player->a_cast_cd / player->a_cast_No;
                player->state = 0;
            } else if (player->act == def) {
                player->act = a_walk;
                player->dir = right;
                player->state = 0;
            }
        }
        // alap
        else if(!keydown[a] && !keydown[d]) {
            player->walk.vx = 0;
        }

        // E lenyomva
        if(keydown[e])
            if( (player->act == a_walk || player->act == a_melee) && player->def.cd < GetTicks())
                if ((player->dir == right && player->def.ra != NULL)
                        || (player->dir == left && player->def.la != NULL)) {
                    player->act = def;
                    player->hit.cd = GetTicks() + 600;
                    player->state = 0;
                }

        // Dash (s_walk)
        if( (keydown[a] || keydown[d]) && keydown[left_shift]) {
            if( (player->act == a_walk || player->act == jump || player->act == flip) && player->s_walk_No != 0) {
                if(player->walk.cd < GetTicks()) {
                    player->act = s_walk;
                    player->vx = player->dir == right ? 7 : -7;
                    player->state = 0;
                    player->walk.state = 0;
                    player->walk.cd = GetTicks() + player->s_walk_cd;
                }
            }
        }
        // Space
        if(justpressed[space]) {
            if((player->act == a_walk || player->act == jump) && !player->jump.second)
                if ((player->dir == right && player->jump.ra != NULL)
                        || (player->dir == left && player->jump.la != NULL)) {
                    if(player->act == jump) {
                        player->act = flip;
                        player->flip.type = forward;
                        if(player->dir == right) {
                            player->flip.angle = -5;
                            player->flip.current = RotateSurface (player->jump.ra, -5);
                            player->flip.base = player->jump.ra;
                            player->vx = 4;
                            player->loc.x -= player->flip.current->w/2 - player->jump.ra->w/2;
                            player->loc.y -= player->flip.current->h/2 - player->jump.ra->h/2;
                            player->flip.x = player->flip.current->w/2;
                            player->flip.y = player->flip.current->h/2;
                        } else {
                            player->flip.angle = 5;
                            player->flip.current = RotateSurface (player->jump.la, 5);
                            player->flip.base = player->jump.la;
                            player->vx = -4;
                            player->loc.x -= player->flip.current->w/2 - player->jump.la->w/2;
                            player->loc.y -= player->flip.current->h/2 - player->jump.la->h/2;
                            player->flip.x = player->flip.current->w/2;
                            player->flip.y = player->flip.current->h/2;
                        }
                        player->flip.angle_inc = 10;
                        player->vy = -3;
                        player->state = 0;
                        player->jump.second = true;
                    } else if (player->act == a_walk) {
                        player->act = jump;
                        player->vy = -5;
                    }
                }
        }

        // Bal + jobb klikk
        if((justreleased[left_click] && keydown[right_click])
                || (justreleased[right_click] && keydown[left_click])
                || (justreleased[left_click] && justreleased[right_click])) {
            // Counter
            if(player->counter_No != 0 && (player->act == hit || player->act == a_melee || player->act == a_walk || player->act == ground) && player->stunned == false) {
                if ((player->dir == right && player->counter.ra != NULL)
                        || (player->dir == left && player->counter.la != NULL)) {
                    if(player->counter.cd < GetTicks()) {
                        player->act = counter;
                        player->state = 0;
                        player->counter.cd = GetTicks() + player->counter_cd;
                    }
                }
            }
        }

        // Bal klikk
        if(justreleased[left_click]) {
            if(!keydown[left_shift]) {
                // A_melee
                if(player->a_melee_No != 0 && player->act == a_walk && player->melee.a_cd < GetTicks()) {
                    if ((player->dir == right && player->melee.ra != NULL)
                            || (player->dir == left && player->melee.la != NULL)) {
                        player->act = a_melee;
                        player->state = 0;
                        player->melee.next = 4;
                        player->melee.a_cd = GetTicks() + player->a_melee_cd;
                    }
                } else if(player->act == a_melee) {
                    if(player->state <= 3)
                        player->melee.next = 7 - player->state;
                    else
                        player->melee.next = 13 - player->state;
                }
                // Jump attack
                else if( player->jump_attack_No != 0 && (player->act == jump || player->act == s_walk) ) {
                    if ((player->dir == right && player->jump.rattack != NULL)
                            || (player->dir == left && player->jump.lattack != NULL)) {
                        if(player->jump.cd1 < GetTicks()) {
                            player->act = jump_attack;
                            player->state = 0;
                            player->jump.cd1 = GetTicks() + player->jump_attack_cd;
                        }
                    }
                }
            } else {
                // S_Melee
                if(player->s_melee_No != 0 && (player->act == a_walk ||  player->act == jump || player->act == air) && player->melee.s_cd < GetTicks()) {
                    if ((player->dir == right && player->melee.rs != NULL)
                            || (player->dir == left && player->melee.ls != NULL)) {
                        player->act = s_melee;
                        player->state = 0;
                        player->melee.x = mouse_x;
                        player->melee.y = mouse_y;
                        if(player->melee.y < kyTop + framesize)
                            player->melee.y = kyTop + framesize;
                        else if(player->melee.y > kyBottom)
                            player->melee.y = kyBottom;
                        player->melee.s_cd = GetTicks() + player->s_melee_cd;
                    }
                }
            }
        }
        // Jobb klikk
        if(justreleased[right_click]) {
            if(!keydown[left_shift]) {
                // A_Ranged
                if(player->a_cast_No != 0 && player->act == a_walk && player->ranged.a_cd < GetTicks()) {
                    if ((player->dir == right && player->ranged.ra_cast != NULL
                            && player->ranged.ra_proj1 != NULL && player->ranged.ra_exp != NULL)
                            || (player->dir == left && player->ranged.la_cast != NULL
                                && player->ranged.la_proj1 != NULL && player->ranged.la_exp != NULL)) {
                        player->act = a_ranged;
                        player->state = 0;
                    }

                }
                // Jump attack2
                else if(player->jump_attack2_No != 0 && (player->act == jump || player->act == s_walk)) {
                    if ((player->dir == right && player->jump.rattack2 != NULL)
                            || (player->dir == left && player->jump.lattack2 != NULL)) {
                        if(player->jump.cd2 < GetTicks()) {
                            if( (player->dir == right && mouse_x > player->loc.x + 1.5 * framesize)
                                    || (player->dir == left && mouse_x < player->loc.x - 1.5 * framesize) ) {
                                player->jump.x = mouse_x;
                                player->jump.y = mouse_y;
                                if(player->jump.y < kyTop)
                                    player->jump.y = kyTop;
                                else if(player->jump.y > kyBottom - framesize/2)
                                    player->jump.y = kyBottom - framesize/2;
                                player->act = jump_attack2;
                                player->state = 0;
                                player->jump.cd2 = GetTicks() + player->jump_attack_cd;
                            }
                        }
                    }
                }
            }
            // S_ranged
            else if(player->s_cast_No != 0 && player->act == a_walk && player->ranged.s_cd < GetTicks()) {
                if ((player->dir == right && player->ranged.rs_cast != NULL
                        && player->ranged.rs_proj != NULL)
                        || (player->dir == left && player->ranged.ls_cast != NULL
                            && player->ranged.ls_proj != NULL)) {
                    player->act = s_ranged;
                    player->state = 0;
                }
            }
        }
    }
    // És hogy az alt + tab is működjön :)
    if(keydown[alt] && keydown[tab]) {
        keydown[alt] = false;
        keydown[tab] = false;
        menu = Quit;
        SDL_WM_IconifyWindow();
    }
}

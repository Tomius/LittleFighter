#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include "main.h"
#include "character.h"
#include "projectile.h"
#include "blit.h"
#include "imgload.h"
#include "bg.h"
#include "SDL_rotozoom.h"
#include "music.h"

/** Kirajzolja a lövedékeket */
void Draw_projectiles(struct projectile *projs, SDL_Surface *screen) {
    struct projectile *p = projs;
    for(p = projs; p!=NULL; p=p->next) {
        if(p->exp == false && p->vanish == false) {
            switch (p->type) {
            case arrow:
                if(p->dir == right) {
                    SDL_Surface *current = RotateSurface(p->r_proj[p->state], -atan2(p->vy, p->v) * 180 / M_PI);
                    SDL_Rect srcRect = { p->x, p->y, current->w, current->h };
                    BlitSurface (current, 0, screen, &srcRect);
                    SDL_FreeSurface(current);
                    BlitSurface(p->shadow, NULL, screen, &(p->shadowRect));
                } else {
                    SDL_Surface *current = RotateSurface(p->l_proj[p->state], atan2(p->vy, p->v) * 180 / M_PI);
                    SDL_Rect srcRect = { p->x, p->y, current->w, current->h };
                    BlitSurface (current, 0, screen, &srcRect);
                    SDL_FreeSurface(current);
                    BlitSurface(p->shadow, NULL, screen, &(p->shadowRect));
                }
                break;
            default:
                if(p->proj_max != 0) {
                    if(p->dir == right) {
                        SDL_Rect srcRect = { p->x, p->y, p->r_proj[p->state]->w, p->r_proj[p->state]->h };
                        BlitSurface (p->r_proj[p->state], 0, screen, &srcRect);
                        BlitSurface(p->shadow, NULL, screen, &(p->shadowRect));
                    } else {
                        SDL_Rect srcRect = { p->x, p->y, p->l_proj[p->state]->w, p->l_proj[p->state]->h };
                        BlitSurface (p->l_proj[p->state], 0, screen, &srcRect);
                        BlitSurface(p->shadow, NULL, screen, &(p->shadowRect));
                    }
                }
            }
        } else if(p->vanish == true) { // Ha a nyilvessző eltört
            if(p->vanish_max != 0) {
                if(p->dir == right) {
                    SDL_Surface *current = RotateSurface(p->r_vanish[0], -p->state * 18);
                    // Ugyan az a trükközés mint az Animate_Flare -> case flip:-nél
                    p->x -= current->w/2 - p->x2;
                    p->y -= current->h/2 - p->y2;
                    p->x2 = current->w/2;
                    p->y2 = current->h/2;
                    SDL_Rect srcRect = { p->x, p->y, current->w, current->h };
                    BlitSurface(current, NULL, screen, &srcRect);
                    BlitSurface(p->shadow, NULL, screen, &(p->shadowRect));
                    SDL_FreeSurface(current);
                } else {
                    SDL_Surface *current = RotateSurface(p->l_vanish[0], p->state * 18);
                    p->x -= current->w/2 - p->x2;
                    p->y -= current->h/2 - p->y2;
                    p->x2 = current->w/2;
                    p->y2 = current->h/2;
                    SDL_Rect srcRect = { p->x, p->y, current->w, current->h };
                    BlitSurface(current, NULL, screen, &srcRect);
                    BlitSurface(p->shadow, NULL, screen, &(p->shadowRect));
                    SDL_FreeSurface(current);
                }
            }
        } else {
            if(p->exp_max != 0) {
                if(p->dir == right) {
                    SDL_Rect srcRect = { p->x, p->y, p->r_exp[p->state]->w, p->r_exp[p->state]->h };
                    BlitSurface (p->r_exp[p->state], 0, screen, &srcRect);
                } else {
                    SDL_Rect srcRect = { p->x, p->y, p->l_exp[p->state]->w, p->l_exp[p->state]->h };
                    BlitSurface (p->l_exp[p->state], 0, screen, &srcRect);
                }
            }
        }
    }
}

/** Animálja (és mozgatja) a lövedékeket. Ha azok kimentek a
  * képernyőről akkor pedig megszünteti őket */
void Animate_projectiles(int Tick, struct projectile **projs) {
    struct projectile *p = *projs;
    while (p != NULL) {
        if(p->exp == false && p->vanish == false) {
            switch (p->type) {
            case jump_attack_proj:
                if(Tick%5 == 0) {
                    p->state++;
                    if(p->state == p->proj_max)
                        p->state = 0;
                }
                p->y += p->v * (int)round(scale);
                p = p->next;
                break;
            case jump_attack2_proj:
                if(Tick%5 == 0) {
                    p->state++;
                    if(p->state == p->proj_max)
                        p->state = 0;
                }

                // A lövedék a kurzor felé mozog, nagyjából 7 pixel / sec sebességgel, a képlet a pitagorasz-tételből jön ki
                // Muszály 2-2 double értéket használni erre mert int értékekkel baromi nagy lenne a torzitás
                // Fontos még hogy a kurzor világkoordináltáira van szükség, nem a képernyőhöz képestire
                // Ja, és elfajuló háromszögre nem működik (csak 5 helyen oszt nullával)
                double a = p->x2 - (p->x3 + TerrainRect.x), b = p->y2 - p->y3;
                if(fabs(a) > pow(10, -5) && fabs(b) > pow(10, -5)) { //Lebegő pontos érték, az == 0 vizsgálat nem működik
                    if(b > 0) {
                        p->x3 += ( a/b * sqrt( p->v*p->v / ( (a/b)*(a/b) + 1 ) ) ) * (int)round(scale);
                        p->y3 +=  sqrt( p->v*p->v / ( (a/b)*(a/b) + 1 ) ) * (int)round(scale);
                    } else {
                        p->x3 -= ( a/b * sqrt( p->v*p->v / ( (a/b)*(a/b) + 1 ) ) ) * (int)round(scale);
                        p->y3 -= sqrt( p->v*p->v / ( (a/b)*(a/b) + 1 ) ) * (int)round(scale);
                    }
                } else { //Elfajuló háromszög esetén
                    if(fabs(a) < pow(10, -5) && b > 0)
                        p->y3 += p->v * (int)round(scale);
                    else if(fabs(a) < pow(10, -5) && b < 0)
                        p->y3 -= p->v * (int)round(scale);
                    else if(fabs(b) < pow(10, -5) && a > 0)
                        p->x3 += p->v * (int)round(scale);
                    else if(fabs(b) < pow(10, -5) && a < 0)
                        p->x3 -= p->v * (int)round(scale);
                }

                p->x = (int) p->x3;
                p->y = (int) p->y3;

                // Az árnyék más sebességgel és más szögben mozog de az x koordináltája a lövedék x koordináltájához kötött
                // ezért csak az y menti sebességét kellett kiszámolni
                p->shadowy += p->vy;

                p->shadowRect.x = (int) p->x3 + (p->dir == right ? 40 * scale : 20 * scale);
                p->shadowRect.y = (int) p->shadowy;
                p = p->next;
                break;
            case arrow:
                if(Tick%5 == 0) {
                    p->state++;
                    if(p->state == p->proj_max)
                        p->state = 0;
                }
                // Ha a talajba ütközött akar ki kell törölni
                if(p->y > p->y2) {
                    struct projectile *q = p->next;
                    *projs = delete_projectile(*projs, p);
                    p=q;
                    break;
                }
                if(p->dir == right) {
                    p->x += p->v * (int)round(scale);
                    p->shadowRect.x += p->v * (int)round(scale);
                    p->y += (int)p->vy * (int)round(scale);
                    p->vy += 0.09;
                } else {
                    p->x -= p->v * (int)round(scale);
                    p->shadowRect.x -= p->v * (int)round(scale);
                    p->y += (int)p->vy * (int)round(scale);
                    p->vy += 0.09;
                }
                p = p->next;
                break;
            default:
                if(Tick%5 == 0 && p->proj_max != 0) {
                    p->state++;
                    if(p->state == p->proj_max)
                        p->state = 0;
                }
                if(p->dir == right) {
                    if(p->x < kx) {
                        p->x += p->v * (int)round(scale);
                        p->shadowRect.x += p->v * (int)round(scale);
                        p = p->next;
                    } else {
                        struct projectile *q = p->next;
                        *projs = delete_projectile(*projs, p);
                        p=q;
                        break; // break out of switch == contiue for
                    }
                } else {
                    if(-p->l_proj[p->state]->w < p->x) {
                        p->x -= p->v * (int)round(scale);
                        p->shadowRect.x -= p->v * (int)round(scale);
                        p = p->next;
                    } else {
                        struct projectile *q = p->next;
                        *projs = delete_projectile(*projs, p);
                        p=q;
                        break;
                    }
                }
            } // switch
        } // if
        else if(p->vanish == true) {
            switch (p->type) {
            case arrow:
                p->state++;
                if(p->state == p->vanish_max || p->y > p->shadowRect.y - 3*framesize / 4) {
                    struct projectile *q = p->next;
                    *projs = delete_projectile(*projs, p);
                    p=q;
                    break;
                }
                if(p->dir == right) {
                    p->x += p->v * (int)round(scale);
                    p->shadowRect.x += p->v * (int)round(scale);
                    p->y += p->vy * (int)round(scale);
                    p->vy += 0.3;
                } else {
                    p->x -= p->v * (int)round(scale);
                    p->shadowRect.x -= p->v * (int)round(scale);
                    p->y += p->vy * (int)round(scale);
                    p->vy += 0.3;
                }
                p = p->next;
                break;
            default:
                break;
            }
        } else { // Ha a lövedék felrobbant
            if(p->exp_max != 0) {
                switch (p->type) {
                default:
                    if(Tick%5 == 0) {
                        p->state++;
                        if(p->state == p->exp_max) {
                            struct projectile *q = p->next;
                            *projs = delete_projectile(*projs, p);
                            p=q;
                            break;
                        }
                    }
                    p = p->next;
                    break;
                }
            } else {
                struct projectile *q = p->next;
                *projs = delete_projectile(*projs, p);
                p=q;
                break;
            }


        } //else
    } //while
} //fn

/** A levedékek és a karakterek közti ütközésekért felelős,
  * emiatt robbannak fel a tűzgolyók, illetve emiatt törik
  * el a nyílvessző is ha védekező ellenségbe lőtték */
void Projectile_hits(struct character *players, struct projectile *projs) {
    struct projectile *p;
    struct character *c, *c2;
    for(p = projs; p != NULL; p = p->next) {
        if(p->exp == true || p->vanish == true)
            continue;
        for(c = players; c != NULL ; c = c->next) {
            switch (p->type) {
            case normal:
            case normal2:
                if( (p->side != enemy && c->side == enemy) || (p->side == enemy && c->side != enemy) )
                    if(fabs(p->x - c->loc.x)  < p->dmg.range[0] / 2)
                        if(fabs(p->y + p->l_proj[p->state]->h/2 - c->loc.y - framesize / 2) < p->dmg.range[0]) {
                            switch (c->act) {
                            case a_walk:
                            case s_walk:
                            case jump:
                            case a_melee:
                            case a_ranged:
                            case s_ranged:
                            case hit:
                            case air:
                            case jump_attack:
                            case jump_attack2:
                            case flip:
                            case def: {
                                // Ha a karakter arrafele védekezik, amerről lövik
                                if((c->dir == right && c->loc.x < p->x && c->act == def)
                                        || (c->dir == left && c->loc.x > p->x && c->act == def)) {
                                    // Akkor nem sebződik
                                    p->exp = true;
                                    soundeffect(chunk_air2, c->loc.x + framesize/2, c->loc.y + framesize/2);
                                    p->x = c->loc.x;
                                    p->y = c->loc.y;
                                    p->state = 0;
                                } else { // különben igen
                                    p->exp = true;
                                    soundeffect(chunk_air2, c->loc.x + framesize/2, c->loc.y + framesize/2);
                                    p->x = c->loc.x;
                                    p->y = c->loc.y;
                                    p->state = 0;
                                    // A robbanás közelében mindenki sebződik
                                    for(c2 = players; c2 != NULL ; c2 = c2->next) {
                                        if( ((p->side != enemy && c2->side == enemy) || (p->side == enemy && c2->side != enemy)) && c2-> vunerable < GetTicks())
                                            if(fabs(p->x - c2->loc.x)  < p->dmg.range[1])
                                                if(fabs(p->y + p->l_exp[p->state]->h/2 - c2->loc.y - framesize / 2) < p->dmg.range[1]) {
                                                    switch (c2->act) {
                                                    case a_walk:
                                                    case s_walk:
                                                    case jump:
                                                    case a_melee:
                                                    case a_ranged:
                                                    case s_ranged:
                                                    case hit:
                                                    case air:
                                                    case jump_attack:
                                                    case jump_attack2:
                                                    case flip:
                                                    case ground:
                                                        c2->hp -= p->dmg.value;
                                                        c2->act = air;
                                                        c2->state = 0;
                                                        if(p->dir == right)
                                                            c2->vx = p->dmg.impact[0];
                                                        else
                                                            c2->vx = -p->dmg.impact[0];
                                                        c2->vy = -p->dmg.impact[0];
                                                        break;
                                                    case def:
                                                        // Ha a karakter arrafele védekezik, amerről lövik
                                                        if((c2->dir == right && c2->loc.x < p->x)
                                                                || (c2->dir == left && c2->loc.x > p->x))
                                                            break; // akkor ne sebződjön
                                                        else { // különben viszont igen
                                                            c2->hp -= p->dmg.value;
                                                            c2->act = air;
                                                            c2->state = 0;
                                                            if(p->dir == right)
                                                                c2->vx = p->dmg.impact[0];
                                                            else
                                                                c2->vx = -p->dmg.impact[0];
                                                            c2->vy = -p->dmg.impact[0];
                                                        }
                                                        break;
                                                    default:
                                                        break;
                                                    }
                                                }
                                    }
                                }

                            }
                            break;
                            case counter: // A counter act-ben minden karakter sebezhetetlen
                                p->exp = true;
                                soundeffect(chunk_air2, c->loc.x + framesize/2, c->loc.y + framesize/2);
                                p->x = c->loc.x;
                                p->y = c->loc.y;
                                p->state = 0;
                                break;
                            default:
                                break;
                            }
                        }
                break;
            case shift:
            case jump_attack2_proj:
            case arrow:
                if( (p->side != enemy && c->side == enemy) || (p->side == enemy && c->side != enemy) )
                    if(fabs(p->x - c->loc.x)  < p->dmg.range[0])
                        if(fabs(p->y - c->loc.y) < p->dmg.range[0]) {
                            switch (c->act) {
                            case a_walk:
                            case s_walk:
                            case jump:
                            case a_melee:
                            case a_ranged:
                            case s_ranged:
                            case hit:
                            case air:
                            case jump_attack:
                            case jump_attack2:
                            case flip:
                            case def: {
                                // Ha a karakter arrafele védekezik, amerről lövik
                                if((c->dir == right && c->loc.x < p->x && c->act == def)
                                        || (c->dir == left && c->loc.x > p->x && c->act == def)) {
                                    // akkor ne sebződjön, de a lövedék robbanjon fel, vagy a nyilvessző törjön el
                                    if(p->type == arrow) {
                                        p->vanish = true;
                                        int i;
                                        for(i=0; i<3; i++)
                                            soundeffect(chunk_hit, c->loc.x + framesize/2, c->loc.y + framesize/2);
                                        p->v *= -0.5;
                                        p->vy = (rand()%3 - 1) / 5;
                                        p->state = 0;
                                        p->x2 = p->l_vanish[0]->w / 2;
                                        p->y2 = p->l_vanish[0]->h / 2;
                                    } else {
                                        p->exp = true;
                                        soundeffect(chunk_exp, c->loc.x + framesize/2, c->loc.y + framesize/2);
                                        p->x = c->loc.x - framesize/2;
                                        p->y = c->loc.y - framesize/2;
                                        p->state = 0;
                                    }
                                } else { // különben viszont igen
                                    p->exp = true;
                                    if(p->type != arrow)
                                        soundeffect(chunk_exp, c->loc.x + framesize/2, c->loc.y + framesize/2);
                                    else {
                                        int i;
                                        for(i=0; i<5; i++)
                                            soundeffect(chunk_hit, c->loc.x + framesize/2, c->loc.y + framesize/2);
                                    }
                                    p->x = c->loc.x - framesize/2;
                                    p->y = c->loc.y - framesize/2;
                                    p->state = 0;
                                    for(c2 = players; c2 != NULL ; c2 = c2->next) {
                                        if( ((p->side != enemy && c2->side == enemy) || (p->side == enemy && c2->side != enemy)) && c2-> vunerable < GetTicks())
                                            if( fabs(p->x + framesize / 2 - c2->loc.x)  < p->dmg.range[1])
                                                if(fabs(p->y + framesize/2 - c2->loc.y) < p->dmg.range[1]) {
                                                    switch (c2->act) {
                                                    case a_walk:
                                                    case s_walk:
                                                    case jump:
                                                    case a_melee:
                                                    case a_ranged:
                                                    case s_ranged:
                                                    case hit:
                                                    case air:
                                                    case jump_attack:
                                                    case jump_attack2:
                                                    case flip:
                                                    case ground:
                                                        //case def:
                                                        c2->hp -= p->dmg.value;
                                                        c2->act = air;
                                                        c2->state = 0;
                                                        if(p->dir == right)
                                                            c2->vx = p->dmg.impact[0];
                                                        else
                                                            c2->vx = -p->dmg.impact[0];
                                                        c2->vy = -p->dmg.impact[0];
                                                        if(p->type == jump_attack2_proj)
                                                            c2->vz = 0.5 * p->vy;
                                                        break;
                                                    case def:
                                                        // Ha a karakter arrafele védekezik, amerről lövik
                                                        if((c2->dir == right && c2->loc.x < p->x + framesize/2)
                                                                || (c2->dir == left && c2->loc.x > p->x + framesize/2))
                                                            break; // akkor ne sebződjön
                                                        else { // különben viszont igen
                                                            c2->hp -= p->dmg.value;
                                                            c2->act = air;
                                                            c2->state = 0;
                                                            if(p->dir == right)
                                                                c2->vx = p->dmg.impact[0];
                                                            else
                                                                c2->vx = -p->dmg.impact[0];
                                                            c2->vy = -p->dmg.impact[0];
                                                        }
                                                        break;
                                                    default:
                                                        break;
                                                    }

                                                }
                                    }
                                }
                            }
                            break;
                            case counter:
                                if(p->type == arrow) {
                                    p->vanish = true;
                                    int i;
                                    for(i=0; i<3; i++)
                                        soundeffect(chunk_hit, c->loc.x + framesize/2, c->loc.y + framesize/2);
                                    p->v *= -0.5;
                                    p->vy = (rand()%3 - 1) / 5;
                                    p->state = 0;
                                    p->x2 = p->l_vanish[0]->w / 2;
                                    p->y2 = p->l_vanish[0]->h / 2;
                                } else {
                                    p->exp = true;
                                    if(p->type != arrow)
                                        soundeffect(chunk_exp, c->loc.x + framesize/2, c->loc.y + framesize/2);
                                    p->x = c->loc.x;
                                    p->y = c->loc.y;
                                    p->state = 0;
                                }
                                break;
                            default:
                                break;
                            }
                        }
                break;
            default:
                break;
            }
        } // for (players)
        if(p->type == jump_attack_proj) {
            if( p->y + p->l_proj[0]->h > p->y2) {
                p->exp = true;
                soundeffect(chunk_exp, p->x + framesize/2, p->y + framesize/2);
                p->x -= framesize/2;
                p->y -= framesize/2;
                p->state = 0;
                for(c = players; c != NULL ; c = c->next) {
                    if( ((p->side != enemy && c->side == enemy) || (p->side == enemy && c->side != enemy)) && c-> vunerable < GetTicks())
                        if( fabs(p->x + framesize / 2- c->loc.x)  < p->dmg.range[1])
                            if(fabs(p->y + framesize/2 - c->loc.y) < p->dmg.range[1]) {
                                switch (c->act) {
                                case a_walk:
                                case s_walk:
                                case jump:
                                case a_melee:
                                case a_ranged:
                                case s_ranged:
                                case hit:
                                case air:
                                case jump_attack:
                                case jump_attack2:
                                case flip:
                                case ground:
                                    c->hp -= p->dmg.value;
                                    c->act = air;
                                    c->state = 0;
                                    if(c->loc.x > p->x + framesize)
                                        c->vx = p->dmg.impact[0];
                                    else
                                        c->vx = -p->dmg.impact[0];
                                    c->vy = -p->dmg.impact[0];
                                    break;
                                case def: {
                                    // Ha a karakter arrafele védekezik, amerről lövik
                                    if((c->dir == right && c->loc.x < p->x + framesize/2)
                                            || (c->dir == left && c->loc.x > p->x + framesize/2))
                                        break; // akkor ne sebződjön
                                    else { // különben viszont igen
                                        c->hp -= p->dmg.value;
                                        c->act = air;
                                        c->state = 0;
                                        if(p->dir == right)
                                            c->vx = p->dmg.impact[0];
                                        else
                                            c->vx = -p->dmg.impact[0];
                                        c->vy = -p->dmg.impact[0];
                                    }
                                }
                                break;
                                default:
                                    break;
                                }
                            }
                }
            }
        }
        if(p->type == jump_attack2_proj) {
            if (fabs(p->y2 - p->y) < framesize/2 && fabs(p->x2 - (p->x + TerrainRect.x)) < framesize/2 ) {
                p->exp = true;
                soundeffect(chunk_exp, p->x + framesize/2, p->y + framesize/2);
                p->x -= framesize/2;
                p->y -= framesize/2;
                p->state = 0;
                for(c = players; c != NULL; c = c->next) {
                    if( ((p->side != enemy && c->side == enemy) || (p->side == enemy && c->side != enemy)) && c-> vunerable < GetTicks())
                        if( fabs(p->x + framesize/2 - c->loc.x) < p->dmg.range[1])
                            if(fabs(p->y + framesize/2 - c->loc.y) < p->dmg.range[1]) {
                                switch (c->act) {
                                case a_walk:
                                case s_walk:
                                case jump:
                                case a_melee:
                                case a_ranged:
                                case s_ranged:
                                case hit:
                                case air:
                                case jump_attack:
                                case jump_attack2:
                                case flip:
                                case ground:
                                    c->hp -= p->dmg.value;
                                    c->act = air;
                                    c->state = 0;
                                    if(p->dir == right)
                                        c->vx = p->dmg.impact[0];
                                    else
                                        c->vx = -p->dmg.impact[0];
                                    c->vy = -p->dmg.impact[0];
                                    break;
                                case def:
                                    // Ha a karakter arrafele védekezik, amerről lövik
                                    if((c->dir == right && c->loc.x < p->x + framesize/2)
                                            || (c->dir == left && c->loc.x > p->x + framesize/2))
                                        break; // akkor ne sebződjön
                                    else { // különben viszont igen
                                        c->hp -= p->dmg.value;
                                        c->act = air;
                                        c->state = 0;
                                        if(p->dir == right)
                                            c->vx = p->dmg.impact[0];
                                        else
                                            c->vx = -p->dmg.impact[0];
                                        c->vy = -p->dmg.impact[0];
                                    }
                                    break;
                                default:
                                    break;
                                }
                            }
                }
            }
        }

    } //for (projs)
} //fn

/** Felszabadít egy lövedéket */
struct projectile * delete_projectile(struct projectile *projs, struct projectile *todelete) {
    struct projectile *p;
    int i;
    if(projs == todelete) {
        p = todelete->next;
        for(i=0; i < todelete->proj_max; i++) {
            SDL_FreeSurface(todelete->l_proj[i]);
            SDL_FreeSurface(todelete->r_proj[i]);
        }
        free(todelete->l_proj);
        free(todelete->r_proj);
        for(i=0; i < todelete->exp_max; i++) {
            SDL_FreeSurface(todelete->l_exp[i]);
            SDL_FreeSurface(todelete->r_exp[i]);
        }
        free(todelete->l_exp);
        free(todelete->r_exp);
        free(todelete->l_vanish);
        free(todelete->r_vanish);
        SDL_FreeSurface(todelete->shadow);
        free(todelete);
        return p;
    }
    for(p=projs; p!=NULL; p=p->next) {
        if(p->next != todelete)
            continue;
        if(p== NULL || p->next == NULL) {
            return NULL;
        } else {
            p->next = todelete->next;
            for(i=0; i < todelete->proj_max; i++) {
                SDL_FreeSurface(todelete->l_proj[i]);
                SDL_FreeSurface(todelete->r_proj[i]);
            }
            free(todelete->l_proj);
            free(todelete->r_proj);
            for(i=0; i < todelete->exp_max; i++) {
                SDL_FreeSurface(todelete->l_exp[i]);
                SDL_FreeSurface(todelete->r_exp[i]);
            }
            free(todelete->l_exp);
            free(todelete->r_exp);
            free(todelete->l_vanish);
            free(todelete->r_vanish);
            SDL_FreeSurface(todelete->shadow);
            free(todelete);
            return projs;
        }
    }
    return NULL;
}

/** Az összes lövedéket felszabadítja */
void deleteall(struct projectile *projs) {
    struct projectile *p,*q;
    for(p=projs; p!=NULL; ) {
        q = p->next;
        projs = delete_projectile(projs,p);
        p = q;
    }
}

/** Létrehoz egy lövedéket */
struct projectile * summon_projectile(struct projectile *projs, struct character *player, enum proj_type type) {
    struct projectile *p = (struct projectile *) malloc (sizeof (struct projectile));
    p->state = 0;
    p->side = player->side;
    p->dir = player->dir;
    p->next = projs;
    p->type = type;
    p->dmg = player->dmg;
    p->exp = false;
    p->vanish = false;
    p->shadowRect = player->shadowRect;
    p->shadow = SDL_CreateRGBSurface(SDL_ANYFORMAT, shadow->w, shadow->h, screen_bpp, screen_rmask, screen_gmask, screen_bmask, screen_amask);
    SDL_SetColorKey(p->shadow,SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(p->shadow->format,0,0,0));
    SDL_BlitSurface(shadow, NULL, p->shadow, NULL);
    SetSize(&(p->shadow), p->shadow->w, p->shadow->h/2);
    p->l_vanish = NULL;
    p->r_vanish = NULL;

    switch (type) {
    case normal:
        p->proj_max = player->a_proj_No;
        p->exp_max = player->a_exp_No;
        p->v = 8;
        p->x = p->dir == right ? player->loc.x + framesize - 10 * scale: player->loc.x - player->ranged.la_proj1[0]->w;
        p->shadowRect.x = p->dir == right ? player->loc.x + framesize + 15 * scale: player->loc.x - player->ranged.la_proj1[0]->w + 15 * scale;
        p->y = player->loc.y;

        p->l_proj = AnimCpy_Alpha(player->ranged.la_proj1, p->proj_max, 5);
        p->r_proj = AnimCpy_Alpha(player->ranged.ra_proj1, p->proj_max, 5);
        p->l_exp = AnimCpy_AlphaScale(player->ranged.la_exp, p->exp_max, 5, 10);
        p->r_exp = AnimCpy_AlphaScale(player->ranged.ra_exp, p->exp_max, 5, 10);
        break;

    case normal2:
        p->proj_max = player->a_proj_No;
        p->exp_max = player->a_exp_No;
        p->v = 8;
        p->x = p->dir == right ? player->loc.x + framesize - 10 * scale: player->loc.x - player->ranged.la_proj2[0]->w;
        p->shadowRect.x = p->dir == right ? player->loc.x + framesize + 15 * scale : player->loc.x - player->ranged.la_proj1[0]->w + 15 * scale;
        p->y = player->loc.y;

        p->l_proj = AnimCpy_Alpha(player->ranged.la_proj2, p->proj_max, 5);
        p->r_proj = AnimCpy_Alpha(player->ranged.ra_proj2, p->proj_max, 5);
        p->l_exp = AnimCpy_AlphaScale(player->ranged.la_exp, p->exp_max, 5, 10);
        p->r_exp = AnimCpy_AlphaScale(player->ranged.ra_exp, p->exp_max, 5, 10);
        break;
    case shift:
        p->proj_max = player->s_proj_No;
        p->exp_max = player->s_exp_No;
        p->v = 7;
        p->x = p->dir == right ? player->loc.x + framesize - 10 * scale : player->loc.x - player->ranged.ls_proj[0]->w;
        p->shadowRect.x = p->dir == right ? player->loc.x + framesize + 25 * scale : player->loc.x - player->ranged.la_proj1[0]->w + 10 * scale;
        p->y = player->loc.y;

        p->l_proj = AnimCpy_Alpha(player->ranged.ls_proj, p->proj_max, 10);
        p->r_proj = AnimCpy_Alpha(player->ranged.rs_proj, p->proj_max, 10);
        p->l_exp = AnimCpy_AlphaScale(player->ranged.ls_exp, p->exp_max, 45, 12);
        p->r_exp = AnimCpy_AlphaScale(player->ranged.rs_exp, p->exp_max, 45, 12);
        break;
    case jump_attack_proj:
        p->proj_max = player->jump_attack_proj_No;
        p->exp_max = player->jump_attack_exp_No;
        p->v = 4;
        if(player->chrname != julian) {
            p->x = p->dir == right ? player->loc.x + 45 * scale : player->loc.x + 10 * scale;
            p->shadowRect.x = p->dir == right ? player->loc.x + 62 * scale : player->loc.x + 15 * scale;
            p->y = player->loc.y + framesize / 2;
            p->y2 = player->shadowRect.y;
        } else {
            p->x = player->loc.x;
            p->y = player->loc.y - framesize/4;
            p->shadowRect.x = player->shadowRect.x;
            p->y2 = player->shadowRect.y - framesize/4;
        }

        p->l_proj = AnimCpy_Alpha(player->jump.lproj, p->proj_max, 5);
        p->r_proj = AnimCpy_Alpha(player->jump.rproj, p->proj_max, 5);
        p->l_exp = AnimCpy_AlphaScale(player->jump.lexp, p->exp_max, 20, 10);
        p->r_exp = AnimCpy_AlphaScale(player->jump.rexp, p->exp_max, 20, 10);
        break;
    case jump_attack2_proj:
        /** Egy jó tanács: ezt ne is akard megérteni hogy hogyan működik... */
        p->proj_max = player->jump_attack2_proj_No;
        p->exp_max = player->jump_attack_exp_No;
        p->v = 7;
        p->x = p->dir == right ? player->loc.x + framesize - 10 * scale : player->loc.x - player->jump.lproj2[0]->w;
        p->y = player->loc.y;
        p->x2 = player->jump.x - player->jump.lproj2[0]->w/2 + TerrainRect.x;
        p->y2 = player->jump.y - player->jump.lproj2[0]->h/2;
        p->shadowRect.x += p->dir == right ? framesize : - player->jump.lproj2[0]->w;
        p->shadowy = player->shadowRect.y;
        p->x3 = (double) p->x;
        p->y3 = (double) p->y;

        p->l_proj = AnimCpy_Alpha(player->jump.lproj2, p->proj_max, 5);
        p->r_proj = AnimCpy_Alpha(player->jump.rproj2, p->proj_max, 5);
        int i;
        for (i = 0; i < p->proj_max; i++) {
            // Elforgatja a lövedéket, úgy, hogy az a kurzor felé nézzen
            RotateSurface2(&(p->r_proj[i]), -1 * atan2( p->y2 - p->y , p->x2 - (p->x + TerrainRect.x) ) * 180 / M_PI );
            RotateSurface2(&(p->l_proj[i]),  atan2( p->y2 - p->y , (p->x + TerrainRect.x) - p->x2 ) * 180 / M_PI );
        }
        // A bal felső sarok körüli forgatást át kell alakitan középpont körülire
        p->x3 -= p->l_proj[0]->w/2 - player->jump.lproj2[0]->w/2;
        p->y3 -= p->l_proj[0]->h/2 - player->jump.lproj2[0]->h/2;
        p->l_exp = AnimCpy_AlphaScale(player->jump.lexp, p->exp_max, 20, 10);
        p->r_exp = AnimCpy_AlphaScale(player->jump.rexp, p->exp_max, 20, 10);

        // A lövedék árnyéka nem ugyan olyan sebességgel mozog, mint maga a lövedék
        // Az viszont elég nagy segitség hogy az x koordinálta az mindig megeggyezik a lövedékével, igy csak az
        // y-t kell számolni, ami már sokkal egyszerűbb...

        // Először megnézem hogy mennyi ideig repül a lövedék
        double tempx = p->x3, tempy = p->y3, a, b;
        int t = 0;
        while(!(fabs(p->y2 - (int)tempy) < framesize/2 && fabs(p->x2 - ((int)tempx + TerrainRect.x)) < framesize/2 ) && t<1000) {
            a = p->x2 - (tempx + TerrainRect.x);
            b = p->y2 - tempy;
            if(fabs(a) > pow(10, -5) && fabs(b) > pow(10, -5)) { //Lebegő pontos érték, az == 0 vizsgálat nem működik
                if(b > 0) {
                    tempx += ( a/b * sqrt( p->v*p->v / ( (a/b)*(a/b) + 1 ) ) ) * (int)round(scale);
                    tempy +=  sqrt( p->v*p->v / ( (a/b)*(a/b) + 1 ) ) * (int)round(scale);
                } else {
                    tempx -= ( a/b * sqrt( p->v*p->v / ( (a/b)*(a/b) + 1 ) ) ) * (int)round(scale);
                    tempy -=  sqrt( p->v*p->v / ( (a/b)*(a/b) + 1 ) ) * (int)round(scale);
                }
            } else { //Elfajuló háromszög esetén

                if(fabs(a) < pow(10, -5) && b > 0)
                    tempy += p->v * (int)round(scale);
                else if(fabs(a) < pow(10, -5) && b < 0)
                    tempy -= p->v * (int)round(scale);
                else if(fabs(b) < pow(10, -5) && a > 0)
                    tempx += p->v * (int)round(scale);
                else if(fabs(b) < pow(10, -5) && a < 0)
                    tempx -= p->v * (int)round(scale);
            }
            t++;
        }
        // Ha a lövedék már ottvan ahol fel kell robbania (bár erre elég kicsi az esély) akkor az árnyék mozgása lényegtelen,
        // ezért a t akármennyi lehet, de azért inkább ne osszon nullával, és ne lépjen ki a program floating point exeptionnel
        if(t == 0)
            t = 1;

        // Aztán v=s/t
        if(p->y2 < p->y)
            p->vy = (p->y2 + player->jump.lproj2[0]->h - p->shadowRect.y) / t;
        else
            p->vy = (p->y2 + 0.8 * player->jump.lproj2[0]->h - p->shadowRect.y) / t;
        break;
    case arrow:
        p->proj_max = player->s_proj_No;
        p->exp_max = player->s_exp_No;
        p->vanish_max = 20;
        p->v = 9;
        p->vy = -0.5;
        p->x = p->dir == right ? player->loc.x + framesize - 10 * scale : player->loc.x - player->ranged.ls_proj[0]->w;
        p->y = player->loc.y - 20 * scale;
        p->y2 = player->loc.y + framesize / 4;
        p->shadowRect.x += p->dir == right ? framesize - 10 * scale : - player->ranged.ls_proj[0]->w;

        p->l_proj = AnimCpy_Alpha(player->ranged.ls_proj, p->proj_max, 10);
        p->r_proj = AnimCpy_Alpha(player->ranged.rs_proj, p->proj_max, 10);
        p->l_exp = AnimCpy_AlphaScale(player->ranged.la_exp, p->exp_max, 45, 12);
        p->r_exp = AnimCpy_AlphaScale(player->ranged.ra_exp, p->exp_max, 45, 12);
        p->l_vanish = AnimCpy(player->ranged.ls_vanish, player->s_vanish_No);
        p->r_vanish = AnimCpy(player->ranged.rs_vanish, player->s_vanish_No);
        break;
    default:
        break;
    }

    // Ez csak az instant kill módhoz kell, debuggoláshoz
    // Vagyis inkább a debuggolás élvezetesebbé tételéhez
    if(player->speed == 3)
        p->v *= 2;

    return p;
}

#include <stdlib.h>
#include <SDL.h>
#include <math.h>
#include "main.h"
#include "flare.h"
#include "level.h"
#include "character.h"
#include "projectile.h"
#include "imgload.h"
#include "music.h"

/** Flare konstansainak inicializálása */
void Init_Flare(struct character *c) {
    // Speed
    c->speed = 1;

    // Frame No's
    c->a_walk_No = 4;
    c->s_walk_No = 3;
    c->jump_attack_No = 5;
    c->jump_attack2_No = 2;
    c->jump_attack_proj_No = 6;
    c->jump_attack2_proj_No = 6;
    c->jump_attack_exp_No = 11;
    c->a_melee_No = 13;
    c->s_melee_No = 9;
    c->a_cast_No = 10;
    c->s_cast_No = 2;
    c->a_proj_No = 4;
    c->s_proj_No = 6;
    c->a_exp_No = 4;
    c->s_exp_No = 11;
    c->counter_No = 11;
    c->ground_No = 5;
    c->air_No = 4;
    c->dead_No = 4;

    // Cooldowns
    c->s_walk_cd = 1500;
    c->a_melee_cd = 500;
    c->jump_attack_cd = 2500;
    c->s_melee_cd = 2500;
    c->a_cast_cd = 2000;
    c->s_cast_cd = 3000;
    c->counter_cd =  3500;

    // Damages
    Init_Damage(c);
    c->walk.dmg.value = 84 / 21; // Egy state 3 framen át tart, igy 7*3 szor fog 46/21at sebezni, azaz összesen 46ot
    c->jump.dmg1.value = 65;
    c->jump.dmg2.value = 65;
    c->melee.a_dmg.value = 50 / 5;
    c->melee.s_dmg.value = 0;
    c->ranged.a_dmg.value = 30;
    c->ranged.s_dmg.value = 80;
    c->counter.dmg.value = 2; // Ez a skill 50 framen át sebez (10 db 5 frameből álló state)

    // Skill ranges
    c->walk.dmg.range[1] = framesize;
    c->jump.dmg1.range[1] = 1.5 * framesize;
    c->jump.dmg2.range[0] = framesize / 2;
    c->jump.dmg2.range[1] = 1.5 * framesize;
    c->melee.a_dmg.range[1] = framesize;
    c->melee.a_dmg.range[5] = framesize;
    c->melee.a_dmg.range[10] = framesize;
    c->ranged.a_dmg.range[0] = framesize / 2;
    c->ranged.a_dmg.range[1] = framesize / 2;
    c->ranged.s_dmg.range[0] = framesize / 2;
    c->ranged.s_dmg.range[1] = 1.5 * framesize;
    int i;
    for(i = 0; i < 10; i++) {
        if(i%3 == 0)
            c->counter.dmg.range[i] = 1.2 * framesize;
    }

    // Impacts
    c->walk.dmg.impact[1] = 2;
    c->jump.dmg1.impact[0] = 4;
    c->jump.dmg2.impact[0] = 4;
    c->melee.a_dmg.impact[10] = 2;
    c->ranged.a_dmg.impact[0] = 3;
    c->ranged.s_dmg.impact[0] = 5;
    c->counter.dmg.impact[9] = 3;
}

/** Flare skilljeinek működése (a legtöbb skill esetében a többi karakter is ezeket
  * használja, ez a fv adja meg egy új karakter skilljeinek default működését,
  * és csak azokat a skilleket kell megírni, amiknek a működése eltér ezektől) */
void Animate_Flare(int Tick, struct character *player, struct projectile **projs) {
    // Maga a fv egy 10 km hosszú switch, és semmi izgalmas nincs benne :(
    switch (player->act) {
    case a_walk:
        if(Tick%10 == 0) {
            if(player->state == 0)
                player->walk.state = 1;
            else if(player->state ==  player->a_walk_No - 1)
                player->walk.state = -1;
            else if(player->walk.state != 1 && player->walk.state != -1)
                player->walk.state = 1;
            player->state += player->walk.state * 1;
        }
        // A karakter mozgatása
        // Az x tengely mentén
        if((player->loc.x + framesize + 10 < kx &&  0 < player->walk.vx) || (10 < player->loc.x && player->walk.vx < 0))
            player->loc.x += (player->walk.vx * 3 * (int)round(scale) * player->collision_modifier * player->speed);
        // Az z tengely mentén (aminek az iránya egybe esik az y tengelyével a perspektiva miatt
        if((kyTop < player->loc.y && player->walk.vz < 0) || (player->shadowRect.y < kyBottom && 0 < player->walk.vz)) {
            player->loc.y += player->walk.vz * 2 * (int)round(scale) * player->collision_modifier * player->speed;
            player->shadowRect.y += player->walk.vz * 2 * (int)round(scale) * player->collision_modifier * player->speed;
        }
        break;
    case s_walk:
        if(Tick%3 == 0) {
            if(player->state == 0) {
                player->state++;
                if(player->side != enemy)
                    soundeffect(chunk_dash, player->loc.x + framesize/2, player->loc.y + framesize/2);
                else
                    soundeffect(chunk_dash2, player->loc.x + framesize/2, player->loc.y + framesize/2);
            } else if(player->state == 1 && player->walk.state < 7)
                player->walk.state++;
            else
                player->state++;
            if(player->state == player->s_walk_No) {
                player->act = jump;
                player->vy = player->vx = 0;
                player->state = 0;
                player->walk.state = 1;
                player->jump.second = true;
            }

        }
        player->loc.x += player->vx * (int)round(scale) * player->collision_modifier * player->speed;
        player->dmg = player->walk.dmg;
        break;
    case jump:
        // A karakter mozgatása
        player->loc.y += (player->vy + player->walk.vz * 2) * (int)round(scale);
        if((kyTop + framesize < player->shadowRect.y && player->walk.vz < 0) || (player->shadowRect.y < kyBottom && 0 < player->walk.vz))
            player->shadowRect.y += player->walk.vz * 2 * (int)round(scale) * player->collision_modifier * player->speed;
        player->vy += 0.25;
        player->loc.x += (player->vx == 0 ? player->walk.vx * 3 : player->vx) * (int)round(scale) * player->collision_modifier * player->speed;

        if(player->shadowRect.y - framesize + 4 < player->loc.y) {
            player->loc.y = player->shadowRect.y - framesize + 4;
            player->act = a_walk;
            player->state = 0;
            player->jump.second = false;
            player->vx = player->vy = 0;
        }
        break;
    case jump_attack:
        if(Tick%5 == 0) {
            player->state++;

            //summon proj & exit
            if(player->state == player->jump_attack_No) {
                (*projs) = summon_projectile((*projs), player, jump_attack_proj);
                player->act = flip;
                player->flip.type = backward;
                if(player->dir == right) {
                    player->flip.angle = 60;
                    player->flip.current = RotateSurface(player->jump.ra, 60);
                    player->flip.base = player->jump.ra;
                    player->vx = -4;
                    player->loc.x -= player->flip.current->w/2 - player->jump.la->w/2;
                    player->loc.y -= player->flip.current->h/2 - player->jump.la->h/2;
                    player->flip.x = player->flip.current->w/2;
                    player->flip.y = player->flip.current->h/2;
                } else {
                    player->flip.angle = -60;
                    player->flip.current = RotateSurface(player->jump.la, -60);
                    player->flip.base = player->jump.la;
                    player->vx = 4;
                    player->loc.x -= player->flip.current->w/2 - player->jump.la->w/2;
                    player->loc.y -= player->flip.current->h/2 - player->jump.la->h/2;
                    player->flip.x = player->flip.current->w/2;
                    player->flip.y = player->flip.current->h/2;
                }
                player->flip.angle_inc = 5;
                player->vy = -4;
                player->jump.second = true;
            }

        }
        // Moving the char
        if(player->state == 3 || player->state == 4) {
            player->loc.y -= 3;
            player->loc.x += player->dir==right ? -3 : 3;
        }
        player->dmg = player->jump.dmg1;
        break;
    case jump_attack2:
        if(Tick%8 == 0) {
            player->state++;
            if(player->state == player->jump_attack2_No) {
                // Ez csak a debuggolás élvezetesebbé tételéhez kell
                if(superpower) {
                    int i;
                    for(i=0, player->jump.x = player->loc.x + framesize; i<10; i++) {
                        (*projs) = summon_projectile((*projs), player, jump_attack2_proj);
                        if(player->dir == right)
                            player->jump.x += kx/10;
                        else
                            player->jump.x -= kx/10;
                    }
                } else
                    (*projs) = summon_projectile((*projs), player, jump_attack2_proj);

                player->act = flip;
                player->flip.type = backward;
                if(player->dir == right) {
                    player->flip.angle = 30;
                    player->flip.current = RotateSurface(player->jump.ra, 30);
                    player->flip.base = player->jump.ra;
                    player->vx = -4;
                    player->loc.x -= player->flip.current->w/2 - player->jump.ra->w/2;
                    player->loc.y -= player->flip.current->h/2 - player->jump.ra->h/2;
                    player->flip.x = player->flip.current->w/2;
                    player->flip.y = player->flip.current->h/2;
                } else {
                    player->flip.angle = -30;
                    player->flip.current =  RotateSurface(player->jump.la, -30);
                    player->flip.base = player->jump.la;
                    player->vx = 4;
                    player->loc.x -= player->flip.current->w/2 - player->jump.la->w/2;
                    player->loc.y -= player->flip.current->h/2 - player->jump.la->h/2;
                    player->flip.x = player->flip.current->w/2;
                    player->flip.y = player->flip.current->h/2;
                }
                player->flip.angle_inc = 5;
                player->vy = -2;
                player->state = 0;
                player->jump.second = true;
            }
        }
        player->dmg = player->jump.dmg2;
        break;
    case a_melee:
        if(Tick%5 == 0) {
            player->state++;
            if(player->state == player->a_melee_No || player->melee.next == 0) {
                player->act = a_walk;
                player->state = 0;
            }
            player->melee.next--;
        }
        player->dmg = player->melee.a_dmg;
        break;
    case s_melee:
        if(Tick%5 == 0) {
            player->state++;
            if(player->state == 6) {
                player->loc.x = player->melee.x - framesize / 2;
                player->loc.y = player->melee.y - framesize;
                player->shadowRect.y = player->loc.y + framesize - 4;
                soundeffect(chunk_teleport, player->loc.x + framesize/2, player->loc.y +framesize/2);
            }
            if(player->state == player->s_melee_No) {
                player->act = a_walk;
                player->state = 0;
                player->jump.second = false;
                player->vy = player->vx = 0;
            }
        }
        player->dmg = player->melee.s_dmg;
        break;
    case a_ranged:
        if(Tick%5 == 0) {
            player->state++;
            if(player->state == 4)
                (*projs) = summon_projectile((*projs), player, normal);
            if(player->state == 7)
                (*projs) = summon_projectile((*projs), player, normal2);
            if(player->state == player->a_cast_No) {
                player->act = a_walk;
                player->state = 0;
                player->ranged.a_cd = GetTicks() + player->a_cast_cd;
            }
        }
        player->dmg = player->ranged.a_dmg;
        break;
    case s_ranged:
        if(Tick%7 == 0) {
            player->state++;
            if(player->state == player->s_cast_No) {
                (*projs) = summon_projectile((*projs), player, shift);
                player->act = a_walk;
                player->state = 0;
                player->ranged.s_cd = GetTicks() + player->s_cast_cd;
            }
        }
        player->dmg = player->ranged.s_dmg;
        break;
    case def:
        if(player->hit.cd < GetTicks()) {
            player->act = a_walk;
            player->vunerable = GetTicks() + 150;
            player->def.cd = GetTicks() + player->def_cd;
        }
        break;
    case counter:
        if(Tick%5 == 0) {
            player->state++;
            if(player->state == player->counter_No) {
                player->act = a_walk;
                player->state = 0;
            }
        }
        player->dmg = player->counter.dmg;
        break;
    case hit:
        if( player->hit.cd < GetTicks() ) {
            player->act = a_walk;
            player->stunned = false;
            player->state = 0;
        }
        break;
    case air:
        player->jump.second = false;
        if(Tick%8 == 0 && player->state < player->air_No - 1)
            player->state++;
        // A karakter mozgatása
        // X
        player->loc.x += player->vx * (int)round(scale) * (player->speed > 1 ? player->speed : 1);
        // Y
        player->loc.y += player->vy * (int)round(scale);
        player->vy += 0.25;
        // Z
        if(kyTop + framesize - 4 * scale < player->loc.y && player->shadowRect.y < kyBottom) {
            player->loc.y += player->vz * (int)round(scale);
            player->shadowRect.y += player->vz * (int)round(scale);
        }
        if(player->shadowRect.y - framesize + 4 < player->loc.y - 30) {
            player->loc.y = player->shadowRect.y - framesize + 4;
            player->act = ground;
            player->dmg = player->ground.dmg; // Fontos: ezt nem lehet a ground alján meghivni, az segfaulthoz vezet
            player->state = 0;
            player->vx = 0;
            player->vy = 0;
            player->vz = 0;
        }
        break;
    case ground:
        if(Tick%6 == 0) {
            if(player->state == 0 && player->ground.get_up == false) {
                player->ground.cd = GetTicks() + rand()%300 + 200;
                player->ground.get_up = true;
            } else if(GetTicks() > player->ground.cd) {
                player->state++;
                if(player->side == hero)
                    player->vunerable = GetTicks() + 250;
                if(player->state == player->ground_No) {
                    player->act = a_walk;
                    player->state = 0;
                    player->jump.second = false; // Ha éppen a második ugrás közbe ütötték volna meg
                    player->ground.get_up = false;
                }
            }
        }
        break;
    case dead:
        if(Tick%5 == 0) {
            if(player->state < player->dead_No - 1)
                player->state++;
            else if (player->dead.timer == 0)
                player->dead.timer = GetTicks() + 3000;
            if(player->vy < 0)
                player->vy = 0;
        }
        // Ha a levegőben halt volna meg, akkor essen vissza a földre, ne a levegőben lebegjen a hulla
        if(player->shadowRect.y - framesize + 4 > player->loc.y) {
            player->loc.y += 1.1 * player->vy;
            player->vy += 0.25;
            player->loc.x += player->vx;
        }
        break;
    case flip:
        player->loc.x += player->vx * (int)round(scale) * player->collision_modifier * player->speed;
        player->loc.y += player->vy * (int)round(scale);

        // A forgásnak gyorsulnia kell, hogy ne nézzen ki hülyén
        if(player->flip.type == forward)
            player->flip.angle += player->dir == right ? -1 * player->flip.angle_inc++ : player->flip.angle_inc++;
        else
            player->flip.angle += player->dir == right ? player->flip.angle_inc++ : -1 * player->flip.angle_inc++;
        SDL_FreeSurface(player->flip.current);

        // Maga a forgatás
        player->flip.current = RotateSurface(player->flip.base, player->flip.angle);

        // A rotozoomSurface a bal felső sarok körül forgat, nekünk viszont középpon körüli forgatás kell,
        // ezért az új képet el kell tolni a két kép középpontját összekötő vektorral
        player->loc.x -= player->flip.current->w/2 - player->flip.x;
        player->loc.y -= player->flip.current->h/2 - player->flip.y;
        player->flip.x = player->flip.current->w/2;
        player->flip.y = player->flip.current->h/2;
        if(fabs(player->flip.angle) >= 360) {
            player->act = jump;
            SDL_FreeSurface(player->flip.current);
            player->flip.current = NULL;
        }
        break;
    default:
        break;
    } //End of switch
}


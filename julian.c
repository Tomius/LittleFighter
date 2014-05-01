#include <stdlib.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include "flare.h"
#include "character.h"
#include "main.h"
#include "music.h"

/** Julian ( a főgonosz ) konstansainak inicializálása */
void Init_Julian(struct character *c) {
    // Speed
    c->speed = 1.05;

    // Frame No's
    c->a_walk_No = 4;
    c->s_walk_No = 2;
    c->jump_attack_proj_No = 1;
    c->jump_attack_exp_No = 11;
    c->a_melee_No = 4;
    c->s_melee_No = 3;
    c->a_cast_No = 17;
    c->s_cast_No = 6;
    c->a_proj_No = 4;
    c->s_proj_No = 2;
    c->a_exp_No = 4;
    c->s_exp_No = 11;
    c->counter_No = 6;
    c->ground_No = 7;
    c->air_No = 3;
    c->dead_No = 3;

    // Cooldowns
    c->s_walk_cd = 2000;
    c->a_melee_cd = 500;
    c->s_melee_cd = 2000;
    c->a_cast_cd = 2000;
    c->s_cast_cd = 3000;
    c->counter_cd =  3500;

    // Damages (ugyan annyit sebez mint Flare)
    Init_Damage(c);
    c->walk.dmg.value = 84 / 21;
    c->jump.dmg1.value = 65;
    c->melee.a_dmg.value = 50 / 5;
    c->melee.s_dmg.value = 0;
    c->ranged.a_dmg.value = 30;
    c->ranged.s_dmg.value = 80;

    // Skill ranges
    c->walk.dmg.range[0] = framesize;
    c->walk.dmg.range[1] = framesize;
    c->jump.dmg1.range[1] = 1.5 * framesize;
    c->melee.a_dmg.range[1] = framesize;
    c->melee.a_dmg.range[3] = framesize;
    c->ranged.a_dmg.range[0] = framesize / 2;
    c->ranged.a_dmg.range[1] = framesize / 2;
    c->ranged.s_dmg.range[0] = framesize / 2;
    c->ranged.s_dmg.range[1] = 1.5 * framesize;

    // Impacts
    c->walk.dmg.impact[0] = 2;
    c->walk.dmg.impact[1] = 2;
    c->jump.dmg1.impact[0] = 4;
    c->melee.a_dmg.impact[3] = 2;
    c->ranged.a_dmg.impact[0] = 3;
    c->ranged.s_dmg.impact[0] = 5;
}

/** Julian skilljei működésének leírása */
void Animate_Julian(int Tick, struct character *player, struct projectile **projs) {
    switch (player->act) {
    case s_walk:
        if(Tick%3 == 0) {
            if(player->walk.state == 0) {
                if(player->side != enemy)
                    soundeffect(chunk_dash, player->loc.x + framesize/2, player->loc.y + framesize/2);
                else
                    soundeffect(chunk_dash2, player->loc.x + framesize/2, player->loc.y + framesize/2);
            }
            if(player->walk.state < 12) {
                player->state = player->walk.state%2;
                player->walk.state++;
            } else {
                player->act = jump;
                player->vy = 0;
                player->state = 0;
                player->walk.state = 1;
                player->jump.second = true;
            }
        }
        player->loc.x += player->vx * (int)round(scale) * player->collision_modifier * player->speed;
        player->dmg = player->walk.dmg;
        break;
    case a_melee:
        if(Tick%6 == 0) {
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
            if(player->state == 0)
                soundeffect(chunk_teleport2, player->loc.x + framesize/2, player->loc.y + framesize/2);
            player->state++;
            if(player->state == player->s_melee_No) {
                player->loc.x = player->melee.x - framesize / 2;
                player->loc.y = player->melee.y - framesize;
                player->shadowRect.y = player->loc.y + framesize - 4;
                player->act = a_walk;
                player->state = 0;
                player->jump.second = false;
                player->vy = player->vx = 0;
            }
        }
        player->dmg = player->melee.s_dmg;
        break;
    case a_ranged:
        if(Tick%6 == 0) {
            player->state++;
            switch (player->state) {
            case 1:
            case 4:
            case 8:
            case 11:
            case 14:
                (*projs) = summon_projectile((*projs), player, normal);
            default:
                break;
            }
            if(player->state == player->a_cast_No) {
                player->act = a_walk;
                player->state = 0;
                player->ranged.a_cd = GetTicks() + player->a_cast_cd;
            }
        }
        player->dmg = player->ranged.a_dmg;
        break;
    case s_ranged:
        if(Tick%4 == 0) {
            player->state++;
            if(player->state == 3)
                (*projs) = summon_projectile((*projs), player, shift);
            if(player->state == player->s_cast_No) {
                player->act = a_walk;
                player->state = 0;
                player->ranged.s_cd = GetTicks() + player->s_cast_cd;
            }
        }
        player->dmg = player->ranged.s_dmg;
        break;
    case counter:
        if(Tick%5 == 0) {
            player->state++;
            if(player->state == 2)
                (*projs) = summon_projectile((*projs), player, jump_attack_proj);
            if(player->state == player->counter_No) {
                player->act = a_walk;
                player->state = 0;
            }
        }
        player->dmg = player->jump.dmg1;
        break;
    case ground:
        if(Tick%4 == 0) {
            if(player->state == 0 && player->ground.get_up == false) {
                player->ground.cd = GetTicks() + rand()%300 + 200;
                player->ground.get_up = true;
            } else if(GetTicks() > player->ground.cd) {
                player->state++;
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
    default:
        Animate_Flare (Tick, player, projs);
        break;
    } //End of switch
}


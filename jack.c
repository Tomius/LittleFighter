#include <string.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include "flare.h"
#include "bandit.h"
#include "character.h"
#include "imgload.h"
#include "main.h"

/** Jack konstansait inicializálja */
void Init_Jack(struct character *c) {
    // Speed
    c->speed = 0.9;

    // Frame No's
    c->a_walk_No = 4;
    c->a_melee_No = 10;
    c->s_walk_No = 3;
    c->a_cast_No = 8;
    c->a_proj_No = 2;
    c->a_exp_No = 4;
    c->jump_attack_No = 5;
    c->ground_No = 4;
    c->air_No = 4;
    c->dead_No = 4;

    // Cooldowns
    c->a_melee_cd = 1400;
    c->s_walk_cd = 5000;
    c->jump_attack_cd = 2500;
    c->a_cast_cd = 2000;

    // Damages
    Init_Damage(c);
    c->melee.a_dmg.value = 32 / 8;
    c->walk.dmg.value = 42 / 21;
    c->ranged.a_dmg.value = 30;
    c->jump.dmg1.value = 60 / 5;

    c->melee.a_dmg.range[1] = framesize;
    c->melee.a_dmg.range[3] = framesize;
    c->melee.a_dmg.range[7] = framesize;
    c->walk.dmg.range[1] = framesize;
    c->ranged.a_dmg.range[0] = framesize / 2;
    c->ranged.a_dmg.range[1] = framesize / 2;
    c->jump.dmg1.range[1] = 1.5 * framesize;

    c->melee.a_dmg.impact[7] = 3;
    c->walk.dmg.impact[1] = 2;
    c->jump.dmg1.impact[1] = 3;
    c->ranged.a_dmg.impact[0] = 3;

}

/** Animálja Jack-et */
void Animate_Jack(int Tick, struct character *player, struct projectile **projs) {
    switch (player->act) {
    case a_ranged:
        if(Tick%5 == 0) {
            player->state++;
            if(player->state == 3)
                (*projs) = summon_projectile((*projs), player, normal);
            if(player->state == 5)
                (*projs) = summon_projectile((*projs), player, normal2);
            if(player->state == player->a_cast_No) {
                player->act = a_walk;
                player->state = 0;
                player->ranged.a_cd = GetTicks() + player->a_cast_cd;
            }
        }
        player->dmg = player->ranged.a_dmg;
        break;
    case jump_attack:
        if(Tick%5 == 0) {
            player->state++;
            if(player->state == player->jump_attack_No) {
                player->act = flip;
                player->flip.type = backward;
                if(player->dir == right) {
                    player->flip.angle = 270;
                    player->flip.current = RotateSurface(player->jump.ra, 270);
                    player->flip.base = player->jump.ra;
                    player->vx = -3;
                    player->loc.x -= player->flip.current->w/2 - player->jump.la->w/2;
                    player->loc.y -= player->flip.current->h/2 - player->jump.la->h/2;
                    player->flip.x = player->flip.current->w/2;
                    player->flip.y = player->flip.current->h/2;
                } else {
                    player->flip.angle = -270;
                    player->flip.current = RotateSurface(player->jump.la, -270);
                    player->flip.base = player->jump.la;
                    player->vx = 3;
                    player->loc.x -= player->flip.current->w/2 - player->jump.la->w/2;
                    player->loc.y -= player->flip.current->h/2 - player->jump.la->h/2;
                    player->flip.x = player->flip.current->w/2;
                    player->flip.y = player->flip.current->h/2;
                }
                player->flip.angle_inc = 5;
                player->vy = -1;
                player->jump.second = true;
            }

        }
        // A karakter mozgatása
        if(player->state == 3 || player->state == 4) {
            player->loc.y -= 3;
            player->loc.x += player->dir==right ? -3 : 3;
        }
        player->dmg = player->jump.dmg1;
        break;
    default:
        Animate_Flare (Tick, player, projs);
        break;
    } //End of switch
}



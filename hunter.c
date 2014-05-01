#include <SDL_image.h>
#include "flare.h"
#include "hunter.h"
#include "character.h"
#include "main.h"
#include "imgload.h"
#include "music.h"

/** Iniciálja az íjász karakter */
void Init_Hunter(struct character *c) {
    // Speed
    c->speed = 0.7;

    // Frame No's
    c->a_walk_No = 4;
    c->a_melee_No = 3;
    //c->jump_attack2_No = 5; // Ez a skill is működik, csak a kommenteket kell kiszedni (de annyira nem illik bele a játékba)
    //c->jump_attack2_proj_No = 6;
    //c->jump_attack_exp_No = 11;
    c->a_cast_No = 10;
    c->s_cast_No = 5;
    c->a_proj_No = 6;
    c->a_exp_No = 4;
    c->s_proj_No = 1;
    c->s_vanish_No = 1;
    c->ground_No = 6;
    c->air_No = 4;
    c->dead_No = 4;

    // Cooldowns
    c->a_melee_cd = 1500;
    c->a_cast_cd = 1000;
    c->s_cast_cd = 1500;
    c->jump_attack_cd = 5000;

    // Damage
    Init_Damage(c);
    c->ranged.s_dmg.value = 45;
    c->ranged.a_dmg.value = 60;
    //c->jump.dmg2.value = 65;
    c->melee.a_dmg.value = 27 / 9;

    // Range
    c->ranged.s_dmg.range[0] = framesize / 2;
    c->ranged.s_dmg.range[1] = framesize / 2;
    c->ranged.a_dmg.range[0] = framesize / 2;
    c->ranged.a_dmg.range[1] = framesize / 2;
    //c->jump.dmg2.range[0] = framesize / 2;
    //c->jump.dmg2.range[1] = 1.5 * framesize;
    c->melee.a_dmg.range[2] = framesize;

    // Impact
    c->ranged.s_dmg.impact[0] = 4;
    c->ranged.a_dmg.impact[0] = 4;
    //c->jump.dmg2.impact[0] = 4;
    c->melee.a_dmg.impact[2] = 2;
}

/** Animálja az íjász skilljeit */
void Animate_Hunter(int Tick, struct character *player, struct projectile **projs) {
    switch (player->act) {
    case a_ranged:
        if(Tick%7 == 0) {
            player->state++;
            if(player->state == 4) {
                (*projs) = summon_projectile((*projs), player, normal);
                soundeffect(chunk_arrow, player->loc.x + framesize/2, player->loc.y + framesize/2);
            }
            if(player->state == 9) {
                (*projs) = summon_projectile((*projs), player, normal2);
                soundeffect(chunk_arrow, player->loc.x + framesize/2, player->loc.y + framesize/2);
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
        if(Tick%7 == 0) {
            player->state++;
            if(player->state == player->s_cast_No) {
                (*projs) = summon_projectile((*projs), player, arrow);
                soundeffect(chunk_arrow, player->loc.x + framesize/2, player->loc.y + framesize/2);
                player->act = a_walk;
                player->state = 0;
                player->ranged.s_cd = GetTicks() + player->s_cast_cd;
            }
        }
        player->dmg = player->ranged.s_dmg;
        break;
    case a_melee:
        if(Tick%9 == 0) {
            player->state++;
            if(player->state == player->a_melee_No || player->melee.next == 0) {
                player->act = a_walk;
                player->state = 0;
            }
            player->melee.next--;
        }
        player->dmg = player->melee.a_dmg;
        break;
    case jump_attack2:
        if(Tick%5 == 0)
        {
            player->state++;
            if(player->state == player->jump_attack2_No)
            {
                (*projs) = summon_projectile((*projs), player, jump_attack2_proj);
                soundeffect(chunk_arrow, player->loc.x + framesize/2, player->loc.y + framesize/2);
                player->act = flip;
                player->flip.type = backward;
                if(player->dir == right)
                {
                    player->flip.angle = 30;
                    player->flip.current = RotateSurface(player->jump.ra, 30);
                    player->flip.base = player->jump.ra;
                    player->vx = -4;
                    player->loc.x -= player->flip.current->w/2 - player->jump.ra->w/2;
                    player->loc.y -= player->flip.current->h/2 - player->jump.ra->h/2;
                    player->flip.x = player->flip.current->w/2;
                    player->flip.y = player->flip.current->h/2;
                }
                else
                {
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
    case ground:
        if(Tick%7 == 0) {
            if(player->state == 0 && player->ground.get_up == false) {
                player->ground.cd = GetTicks() + rand()%300 + 200;
                player->ground.get_up = true;
            } else if(GetTicks() > player->ground.cd) {
                player->state++;
                if(player->side == hero)
                    player->vunerable = GetTicks() + 250;
                if(player->state == player->ground_No) {
                    player->melee.a_cd = GetTicks() + player->a_melee_cd; // A felkeléskor mindenképpen használ egy melee ütést (a felkelés animációjában is benne van).
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

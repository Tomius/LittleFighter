#include <string.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include "flare.h"
#include "character.h"
#include "main.h"

void Init_Dummy(struct character *c) {
    // Speed
    c->speed = 1;

    // Frame No's
    c->a_walk_No = 4;
    c->a_melee_No = 13;
    c->ground_No = 5;
    c->air_No = 4;
    c->dead_No = 4;

    // Cooldowns
    c->a_melee_cd = 1500;

    // Damages
    Init_Damage(c);
    c->melee.a_dmg.range[1] = framesize;
    c->melee.a_dmg.range[5] = framesize;
    c->melee.a_dmg.range[10] = framesize;
    c->ground.dmg.value = 1;
    c->melee.a_dmg.value = 1;

    c->melee.a_dmg.impact[10] = 3;
}

void Animate_Dummy(int Tick, struct character *player, struct projectile **projs) {
    switch (player->act) {
    default:
        Animate_Flare (Tick, player, projs);
        break;
    } //End of switch
}


#include <stdlib.h>
#include <SDL.h>
#include <math.h>
#include "main.h"
#include "flare.h"
#include "character.h"
#include "projectile.h"
#include "imgload.h"

/** Woody konstansait inicialázja */
void Init_Woody(struct character *c) {
    // Woody realtív sebessége
    c->speed = 1;

    // Frame Nok
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

    // Cooldownok
    c->s_walk_cd = 1500;
    c->a_melee_cd = 500;
    c->jump_attack_cd = 2500;
    c->s_melee_cd = 2500;
    c->a_cast_cd = 2000;
    c->s_cast_cd = 3000;
    c->counter_cd =  3500;

    // Damagek
    Init_Damage(c);
    c->walk.dmg.value = 84 / 21;
    c->jump.dmg1.value = 65;
    c->jump.dmg2.value = 65;
    c->melee.a_dmg.value = 50 / 5;
    c->melee.s_dmg.value = 0;
    c->ranged.a_dmg.value = 30;
    c->ranged.s_dmg.value = 80;
    c->counter.dmg.value = 2;

    // Skill rangek
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
    for(i = 1; i < 10; i++) {
        if(i%3 == 0)
            c->counter.dmg.range[i] = 1.2 * framesize;
    }

    // Erőlökések
    c->walk.dmg.impact[1] = 2;
    c->jump.dmg1.impact[0] = 4;
    c->jump.dmg2.impact[0] = 4;
    c->melee.a_dmg.impact[10] = 2;
    c->ranged.a_dmg.impact[0] = 3;
    c->ranged.s_dmg.impact[0] = 5;
    c->counter.dmg.impact[9] = 3;
}

/** Woody összes skillje ugyan úgy működik mint flarenek */
void Animate_Woody(int Tick, struct character *player, struct projectile **projs) {
    switch (player->act) {
    default:
        Animate_Flare (Tick, player, projs);
        break;
    } //End of switch
}

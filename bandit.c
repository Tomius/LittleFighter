#include <string.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include "flare.h"
#include "bandit.h"
#include "character.h"
#include "main.h"

/** A bandita konstansainak inicializálása */
void Init_Bandit(struct character *c) {
    // Relatív sebesség (a játékosunké 1)
    c->speed = 0.8;

    // Frame Nok
    c->a_walk_No = 4;
    c->a_melee_No = 6;
    c->s_walk_No = 3;
    c->ground_No = 6;
    c->air_No = 4;
    c->dead_No = 4;

    // Cooldownok
    c->a_melee_cd = 1400;
    c->s_walk_cd = 5000;

    // Damagek
    Init_Damage(c);
    c->melee.a_dmg.value = 24 / 8;
    c->walk.dmg.value = 42 / 21;

    c->melee.a_dmg.range[1] = framesize;
    c->melee.a_dmg.range[3] = framesize;
    c->melee.a_dmg.range[5] = framesize;
    c->walk.dmg.range[1] = framesize;

    c->melee.a_dmg.impact[5] = 3;
    c->walk.dmg.impact[1] = 2;

}

/** A bandita skilljei (ami nincs külön leírva a switchbe, az ugyan úgy működik mint Woodynál) */
void Animate_Bandit(int Tick, struct character *player, struct projectile **projs) {
    switch (player->act) {
    case a_melee:
        if(Tick%8 == 0) {
            player->state++;
            if(player->state == player->a_melee_No || player->melee.next == 0) {
                player->act = a_walk;
                player->state = 0;
            }
            player->melee.next--;
        }
        player->dmg = player->melee.a_dmg;
        break;
    default:
        Animate_Flare (Tick, player, projs);
        break;
    }
}

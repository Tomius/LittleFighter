#include "main.h"
#include "character.h"
#include "free.h"

/** Felszabadit egy Init karaktert */
void spawn_kill( struct character *player ) {
    int i;
    for(i=0; i < player->a_walk_No; i++) {
        SDL_FreeSurface(player->walk.la[i]);
        SDL_FreeSurface(player->walk.ra[i]);
    }
    for(i=0; i < player->a_walk_No; i++) {
        SDL_FreeSurface(player->walk.lstand[i]);
        SDL_FreeSurface(player->walk.rstand[i]);
    }
    for(i=0; i < player->s_walk_No; i++) {
        SDL_FreeSurface(player->walk.ls[i]);
        SDL_FreeSurface(player->walk.rs[i]);
    }
    SDL_FreeSurface(player->jump.la);
    SDL_FreeSurface(player->jump.ra);
    for(i=0; i < player->jump_attack_No; i++) {
        SDL_FreeSurface(player->jump.lattack[i]);
        SDL_FreeSurface(player->jump.rattack[i]);
    }
    for(i=0; i < player->jump_attack2_No; i++) {
        SDL_FreeSurface(player->jump.lattack2[i]);
        SDL_FreeSurface(player->jump.rattack2[i]);
    }
    for(i=0; i < player->jump_attack_proj_No; i++) {
        SDL_FreeSurface(player->jump.lproj[i]);
        SDL_FreeSurface(player->jump.rproj[i]);
    }
    for(i=0; i < player->jump_attack2_proj_No; i++) {
        SDL_FreeSurface(player->jump.lproj2[i]);
        SDL_FreeSurface(player->jump.rproj2[i]);
    }
    for(i=0; i < player->jump_attack_exp_No; i++) {
        SDL_FreeSurface(player->jump.lexp[i]);
        SDL_FreeSurface(player->jump.rexp[i]);
    }
    for(i=0; i < player->a_melee_No; i++) {
        SDL_FreeSurface(player->melee.la[i]);
        SDL_FreeSurface(player->melee.ra[i]);
    }
    for(i=0; i < player->s_melee_No; i++) {
        SDL_FreeSurface(player->melee.ls[i]);
        SDL_FreeSurface(player->melee.rs[i]);
    }
    for(i=0; i < player->a_cast_No; i++) {
        SDL_FreeSurface(player->ranged.la_cast[i]);
        SDL_FreeSurface(player->ranged.ra_cast[i]);
    }
    for(i=0; i < player->s_cast_No; i++) {
        SDL_FreeSurface(player->ranged.ls_cast[i]);
        SDL_FreeSurface(player->ranged.rs_cast[i]);
    }
    for(i=0; i < player->a_proj_No; i++) {
        SDL_FreeSurface(player->ranged.la_proj1[i]);
        SDL_FreeSurface(player->ranged.ra_proj1[i]);
    }
    for(i=0; i < player->a_proj_No; i++) {
        SDL_FreeSurface(player->ranged.la_proj2[i]);
        SDL_FreeSurface(player->ranged.ra_proj2[i]);
    }
    for(i=0; i < player->s_proj_No; i++) {
        SDL_FreeSurface(player->ranged.ls_proj[i]);
        SDL_FreeSurface(player->ranged.rs_proj[i]);
    }
    for(i=0; i < player->a_exp_No; i++) {
        SDL_FreeSurface(player->ranged.la_exp[i]);
        SDL_FreeSurface(player->ranged.ra_exp[i]);
    }
    for(i=0; i < player->s_exp_No; i++) {
        SDL_FreeSurface(player->ranged.ls_exp[i]);
        SDL_FreeSurface(player->ranged.rs_exp[i]);
    }
    for(i=0; i < player->s_vanish_No; i++) {
        SDL_FreeSurface(player->ranged.ls_vanish[i]);
        SDL_FreeSurface(player->ranged.rs_vanish[i]);
    }
    SDL_FreeSurface(player->def.la);
    SDL_FreeSurface(player->def.ra);
    for(i=0; i < player->counter_No; i++) {
        SDL_FreeSurface(player->counter.la[i]);
        SDL_FreeSurface(player->counter.ra[i]);
    }
    SDL_FreeSurface(player->hit.la);
    SDL_FreeSurface(player->hit.ra);
    for(i=0; i < player->ground_No; i++) {
        SDL_FreeSurface(player->ground.la[i]);
        SDL_FreeSurface(player->ground.ra[i]);
    }
    for(i=0; i < player->air_No; i++) {
        SDL_FreeSurface(player->air.la[i]);
        SDL_FreeSurface(player->air.ra[i]);
    }
    for(i=0; i < player->dead_No; i++) {
        SDL_FreeSurface(player->dead.la[i]);
        SDL_FreeSurface(player->dead.ra[i]);
    }

    // Ezekről sekély másolat készül a struktúra értékadásnál, ezeket csak az init_karaktereknék kell felszabaditani
    free(player->walk.dmg.range);
    free(player->jump.dmg1.range);
    free(player->jump.dmg2.range);
    free(player->counter.dmg.range);
    free(player->melee.a_dmg.range);
    free(player->melee.s_dmg.range);
    free(player->ranged.a_dmg.range);
    free(player->ranged.s_dmg.range);
    free(player->ground.dmg.range);
    free(player->walk.dmg.impact);
    free(player->jump.dmg1.impact);
    free(player->jump.dmg2.impact);
    free(player->counter.dmg.impact);
    free(player->melee.a_dmg.impact);
    free(player->melee.s_dmg.impact);
    free(player->ranged.a_dmg.impact);
    free(player->ranged.s_dmg.impact);
    free(player->ground.dmg.impact);
    // Ez az fv azokat szabaditja fel, amit a másolatok is külön lefognalnak
    kill_character(player);

}

/** Töröl egy karaktert a listából */
void delete_char(struct character **chars, struct character *todelete) {
    struct character *iter;
    if(todelete == *chars) {
        iter = (*chars)->next;
        kill_character(todelete);
        *chars = iter;
    } else {
        // Előre teker a listában a todelete elé
        for(iter = *chars; iter != NULL && iter->next != todelete; iter = iter->next);
        iter->next = todelete->next;
        kill_character(todelete);
    }
}

/** Egy Init karakterről készült másolatot szabadit fel.
  * A delete_char hívja meg impliciten ezt a függvényt. */
/** Fontos megjegyzés: Ennek a fvnek eredetileg kill volt
  * a neve. Linuxosoknak érdemes kipróbálni, hogy mi történik,
  * ha search and replace-el átnevezitek a "kill_character("-t
  * "kill"-re. A megmagyarázhatatlan, teljesen értelmetlen,
  * legelvetemültebb helyeken is seg faultokat okozó bugnak
  * az az oka hogy az SDL_Init(INIT_AUDIO)-nak is van egy
  * kill nevű függvénye, de ez felül fogja definiálja azt a
  * kill-t így az inicializáláskor még nem létező, értelmetlen
  * paraméteren meghívott felszabító függvény fog lefutni,
  * ami úgy ritkán egészséges. Én erre kb 50 óra debuggolás
  * árán jöttem rá. */
void kill_character( struct character *player ) {
    free(player->walk.la);
    free(player->walk.ra);
    free(player->walk.lstand);
    free(player->walk.rstand);
    free(player->walk.ls);
    free(player->walk.rs);
    free(player->jump.lattack);
    free(player->jump.rattack);
    free(player->jump.lattack2);
    free(player->jump.rattack2);
    free(player->jump.lproj);
    free(player->jump.rproj);
    free(player->jump.lproj2);
    free(player->jump.rproj2);
    free(player->jump.lexp);
    free(player->jump.rexp);
    free(player->melee.la);
    free(player->melee.ra);
    free(player->melee.ls);
    free(player->melee.rs);
    free(player->ranged.la_cast);
    free(player->ranged.ra_cast);
    free(player->ranged.ls_cast);
    free(player->ranged.rs_cast);
    free(player->ranged.la_proj1);
    free(player->ranged.ra_proj1);
    free(player->ranged.la_proj2);
    free(player->ranged.ra_proj2);
    free(player->ranged.ls_proj);
    free(player->ranged.rs_proj);
    free(player->ranged.la_exp);
    free(player->ranged.ra_exp);
    free(player->ranged.ls_exp);
    free(player->ranged.rs_exp);
    free(player->ranged.ls_vanish);
    free(player->ranged.rs_vanish);
    SDL_FreeSurface(player->flip.current);
    free(player->counter.la);
    free(player->counter.ra);
    free(player->ground.la);
    free(player->ground.ra);
    free(player->air.la);
    free(player->air.ra);
    free(player->dead.la);
    free(player->dead.ra);
    free(player);
}

/** Kinyírja az összes inicializáló
  * karakterekről készült sekély másolatot */
void killall(struct character *players) {
    struct character *c, *d;
    for(c=players; c!=NULL;) {
        d = c->next;
        kill_character(c);
        c = d;
    }
}

/** Resetel egy karaktert (hogy ne kelljen felszabadítani,
  * majd újra létrehozni a karaktert, hanem elég csak újra-
  * éleszteni. */
void reset (struct character *c, int x, int y) {
    c->hp = c->maxhp;
    c->loc.x = x;
    c->loc.y = y;
    c->shadowRect.x = x + 25;
    c->shadowRect.y = y + framesize - 4;
    c->state = 0;
    c->act = a_walk;
    c->dead.timer = 0;
    SDL_FreeSurface(c->flip.current);
}

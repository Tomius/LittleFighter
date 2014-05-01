#include <math.h>
#include "character.h"
#include "main.h"
#include "music.h"

/** A közelre ható skilleknek a sebzéseit megcsináló függvény */
void Melee_hits(struct character* chars) {
    struct character *player, *iter;
    // Iterative variable, named like this for easier understanding
    for(player = chars; player != NULL; player = player->next) {
        // Offensive actions take effect
        switch (player->act) {
        case ground:
        case s_walk:
        case a_melee:
        case counter:
        case jump_attack:
            // A kivételek erősitik a szabályt :)
            if((player->act == jump_attack && player->chrname != jack) || (player->act == counter && player->chrname == julian))
                continue;
            for(iter = chars; iter != NULL; iter = iter->next) {
                // Ha sebezhetetlen, akkor, ugorja át
                if(iter->vunerable > GetTicks())
                    continue;
                // Ha ellenség
                if( (iter->side != enemy && player->side == enemy) || (iter->side == enemy && player->side != enemy) ) {
                    // Ha megfelelő távolágon belül van
                    if(fabs(player->loc.y - iter->loc.y) < player->dmg.range[player->state]
                            && fabs(player->loc.x - iter->loc.x) < player->dmg.range[player->state]) {
                        // És megfelelő irányba néz
                        if((player->dir == right && iter->loc.x > player->loc.x)
                                || (player->dir == left && iter->loc.x < player->loc.x)
                                || player->act == counter /* Ez mindkét irányba sebez */) {
                            switch (iter->act) {
                            case a_walk:
                            case s_ranged:
                            case a_melee:
                            case a_ranged:
                            case hit:
                                iter->hp -= player->dmg.value;
                                if(player->dmg.impact[player->state] == 0) {
                                    iter->act = hit;
                                    if(player->act != counter)
                                        soundeffect(chunk_hit, iter->loc.x + framesize/2, iter->loc.y +framesize/2);
                                    else
                                        soundeffect(chunk_hit2, iter->loc.x + framesize/2, iter->loc.y +framesize/2);
                                    iter->state = 0;
                                    iter->hit.cd = GetTicks() + 500;
                                }
                                if(player->dmg.impact[player->state] != 0) {
                                    iter->act = air;
                                    soundeffect(chunk_air, iter->loc.x + framesize/2, iter->loc.y +framesize/2);
                                    iter->state = 0;
                                    if(player->loc.x > iter->loc.x)
                                        iter->vx = -player->dmg.impact[player->state];
                                    else
                                        iter->vx = player->dmg.impact[player->state];
                                    iter->vy = -player->dmg.impact[player->state] * 5 / 4;
                                }
                                break;
                            case s_walk:
                            case jump:
                            case air: {
                                iter->hp -= player->dmg.value;
                                int impact = player->dmg.impact[player->state] < 2 ? 2 : player->dmg.impact[player->state];
                                iter->act = air;
                                soundeffect(chunk_air, iter->loc.x + framesize/2, iter->loc.y +framesize/2);
                                iter->state = 0;
                                if(player->loc.x > iter->loc.x)
                                    iter->vx = -impact;
                                else
                                    iter->vx = impact;
                                iter->vy = -impact;
                            }
                            break;
                            case def:
                                // Ha a karakter arrafele védekezik, amerről ütnek
                                if((iter->dir == right && iter->loc.x < player->loc.x)
                                        || (iter->dir == left && iter->loc.x > player->loc.x)) {
                                    if(player->act != counter)
                                        soundeffect(chunk_hit, iter->loc.x + framesize/2, iter->loc.y +framesize/2);
                                    else
                                        soundeffect(chunk_hit2, iter->loc.x + framesize/2, iter->loc.y +framesize/2);
                                    break; // akkor ne sebződjön
                                }

                                else {
                                    iter->hp -= player->dmg.value;
                                    if(player->dmg.impact[player->state] == 0) {
                                        iter->act = hit;
                                        if(player->act != counter)
                                            soundeffect(chunk_hit, iter->loc.x + framesize/2, iter->loc.y +framesize/2);
                                        else
                                            soundeffect(chunk_hit2, iter->loc.x + framesize/2, iter->loc.y +framesize/2);
                                        iter->state = 0;
                                        iter->hit.cd = GetTicks() + 500;
                                    }
                                    if(player->dmg.impact[player->state] != 0) {
                                        iter->act = air;
                                        soundeffect(chunk_air, iter->loc.x + framesize/2, iter->loc.y +framesize/2);
                                        iter->state = 0;
                                        if(player->loc.x > iter->loc.x)
                                            iter->vx = -player->dmg.impact[player->state];
                                        else
                                            iter->vx = player->dmg.impact[player->state];
                                        iter->vy = -player->dmg.impact[player->state] * 5 / 4;
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
            break;
        default:
            break;
        }
    }
}

/** Új sorrendbe rakja a karaktereket, hogy ha két karakterre egyszerre akarná megütni egymást, akkor azok közül a játékos,
  * vagy a játékos oldalán játszó baráti karakter sebezzen először, és ne a véletlen döntse el, hogy ki nyer */
void Reorder_for_melee_hits (struct character **chars) {
    // Először megszámolja hány karakter van a listában
    struct character *iter;
    int n;
    for(iter = *chars , n = 0; iter != NULL; iter = iter->next, n++);

    // Majd felépit a listából egy tömböt
    struct character *array[n];
    int i;
    for(iter = *chars, i = 0; iter != NULL; iter = iter->next, i++) {
        array[i] = iter;
    }

    // Az ötlet ugyan az mint a gyorsrendezésnél a kékek pirosak cserélgetése, hogy a tömb első felében csak kékek, második felében csak pirosak legyenek
    int left = 0, right = n-1;
    while (left<right) {
        while (left<right && array[left]->side != enemy)  /* ellenséget keresünk */
            left++;
        while (left<right && array[right]->side == enemy) /* baráti játékost keresünk keresünk */
            right--;

        if (left<right) {
            struct character *temp = array[left];   /* csere */
            array[left] = array[right];
            array[right] = temp;
            left++;
            right--;        /* egyből a következőkre */
        }
    }

    *chars = array[0];
    // Újrafelépiti a listát
    for(i = 0; i < n-1; i++) {
        array[i]->next = array[i + 1];
    }
    array[i]->next = NULL;
}

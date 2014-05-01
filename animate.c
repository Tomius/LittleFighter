#include <stdlib.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include "animate.h"
#include "imgload.h"
#include "main.h"
#include "character.h"
#include "blit.h"
#include "flare.h"
#include "woody.h"
#include "dummy.h"
#include "bandit.h"
#include "hunter.h"
#include "mark.h"
#include "jack.h"
#include "julian.h"
#include "free.h"
#include "bg.h"

/** A karaktereknek és árnyékaiknak kirajzolása */
void Draw_characters(struct character *chars, SDL_Surface *screen) {
    struct character *iter;
    for(iter = chars; iter != NULL; iter = iter->next) {
        // Ha a karakter nem látszódik, akkor ne bajlódjon a kirajzolásával
        if (iter->loc.x < -framesize || iter->loc.x > kx)
            continue;

        // A karakter árnyékának a karakterhez igazitása, kirajzolása
        if(iter->dir == right)
            iter->shadowRect.x = iter->loc.x + 5 * framesize / 16;
        else
            iter->shadowRect.x = iter->loc.x + 4 * framesize / 16;

        // Ha a karakter láthatatlan, vagy a földön fekszik akkor ne legyen árnyéka
        if( !((iter->act == s_melee && (iter->state == 4 || iter->state == 5 || iter->state == 6))
                || (iter->act == dead && iter->state == iter->dead_No - 1 && iter->shadowRect.y - framesize + 4 <= iter->loc.y)
                || (iter->act == ground && iter->state == 0)) )
            BlitSurface(shadow , NULL , screen , &iter->shadowRect );

        // Majd a karakter kirajzolása
        BlitSurface(Current_action(iter), NULL, screen, &iter->loc);
    }
}

/** Visszaadja azt a képet, amit ki kell rajzolni a karakterről,
  * a jelenlegi cselekvése (act) és állapota(state) függvényében */
SDL_Surface *Current_action(struct character *iter) {
    if(iter->dir == right) {
        // A karakter kirajzolása a jelenlegi cselekvésétől függően
        switch (iter->act) {
        case a_walk:
            if(iter->walk.vx == 0 && iter->walk.vz == 0)
                return iter->walk.rstand[iter->state];
            else
                return iter->walk.ra[iter->state];
            break;
        case s_walk:
            return iter->walk.rs[iter->state];
            break;
        case jump:
            return iter->jump.ra;
            break;
        case jump_attack:
            return iter->jump.rattack[iter->state];
            break;
        case jump_attack2:
            return iter->jump.rattack2[iter->state];
            break;
        case a_melee:
            return iter->melee.ra[iter->state];
            break;
        case s_melee:
            return iter->melee.rs[iter->state];
            break;
        case a_ranged:
            return iter->ranged.ra_cast[iter->state];
            break;
        case s_ranged:
            return iter->ranged.rs_cast[iter->state];
            break;
        case counter:
            return iter->counter.ra[iter->state];
            break;
        case def:
            return iter->def.ra;
            break;
        case hit:
            return iter->hit.ra;
            break;
        case air:
            return iter->air.ra[iter->state];
            break;
        case ground:
            return iter->ground.ra[iter->state];
            break;
        case dead:
            return iter->dead.ra[iter->state];
            break;
        case flip:
            return iter->flip.current;
            break;
        default:
            break;
        } //switch
    } //if
    else {
        switch (iter->act) {
        case a_walk:
            if(iter->walk.vx == 0 && iter->walk.vz == 0)
                return iter->walk.lstand[iter->state];
            else
                return iter->walk.la[iter->state];
            break;
        case s_walk:
            return iter->walk.ls[iter->state];
            break;
        case jump:
            return iter->jump.la;
            break;
        case jump_attack:
            return iter->jump.lattack[iter->state];
            break;
        case jump_attack2:
            return iter->jump.lattack2[iter->state];
            break;
        case a_melee:
            return iter->melee.la[iter->state];
            break;
        case s_melee:
            return iter->melee.ls[iter->state];
            break;
        case a_ranged:
            return iter->ranged.la_cast[iter->state];
            break;
        case s_ranged:
            return iter->ranged.ls_cast[iter->state];
            break;
        case counter:
            return iter->counter.la[iter->state];
            break;
        case def:
            return iter->def.la;
            break;
        case hit:
            return iter->hit.la;
            break;
        case air:
            return iter->air.la[iter->state];
            break;
        case ground:
            return iter->ground.la[iter->state];
            break;
        case dead:
            return iter->dead.la[iter->state];
            break;
        case flip:
            return iter->flip.current;
            break;
        default:
            break;
        } //switch
    } //else
    return NULL;
}

/** Meghívja a karakterek moduljaiban definiált függvényeket, amik leírják hogy
  * hogyan is viselkednek az adott karakter képességei*/
void Animate_actions(int Tick, struct character *chars, struct projectile **projs) {
    struct character *iter;

    for(iter = chars; iter != NULL; iter = iter->next) {
        switch (iter->chrname) {
        case flare:
            Animate_Flare(Tick, iter, projs);
            break;
        case woody:
            Animate_Woody(Tick, iter, projs);
            break;
        case bandit:
            Animate_Bandit(Tick, iter, projs);
            break;
        case hunter:
            Animate_Hunter(Tick, iter, projs);
            break;
        case mark:
            Animate_Mark(Tick, iter, projs);
            break;
        case jack:
            Animate_Jack(Tick, iter, projs);
            break;
        case dummy:
            Animate_Dummy(Tick, iter, projs);
            break;
        case julian:
            Animate_Julian(Tick, iter, projs);
            break;
        default:
            Animate_Flare(Tick, iter, projs);
            break;
        }
    } // for
} //fn


/** Ez a fv és struktúra a Reorder_for_draw_characters-ben lévő qsorthoz kell*/
struct foo {
    struct character *c;
    int dist; // A kamerától számolt távolság
};
/** Összehasonlitja a két számot a qsortnak megfelelően, azzal az egy aprósággal, hogy sose ad vissza nullát */
int compare_foo (const void * a, const void * b) {
    if(((struct foo*)a)->dist == ((struct foo*)b)->dist) {
        ((struct foo*)a)->c->loc.y += 1;
        ((struct foo*)a)->c->shadowRect.y += 1;
    }
    // Ez azért kell, mert a két szám egyenlősége nem jelenti azt, hogy azok szabadon cserélgethetőek.
    // Sőt, pont hogy ezt akarjuk elkerülni, mert ekkor a két egymásralógó karakter folyamatosan vibrálna.
    // Erre egy megoldás, hogy a két karakter közül az egyiket elmozditom egy pixellel, ami gyakorlatilag
    // észrevehetetlen, szemben azzal a baromi idegesitő 50 hertz-es vibrálással, amit ennek a hiánya okoz.
    return ((struct foo*)a)->dist - ((struct foo*)b)->dist;
}

/** A karakterek kirajzolása előtt újra kell rendezni a karakterek listáját úgy,
  * hogy aki közelebb van a kamerához, az legyen később kirajzolva,
  * mert eléggé illúziórombomó amikor két fedésben lévő karakter közül
  * a távolabbik fedi a közelebb levőt */
void Reorder_for_draw_characters (struct character **chars) {
    // A láncolt listából csinál egy tömböt, rányom egy qsortot, majd jó sorrendben újra felépiti a listát

    // Először megszámolja hány karakter van a listában
    struct character *iter;
    int n;
    for(iter = *chars , n = 0; iter != NULL; iter = iter->next, n++);

    // Majd feltölti az indexelő tömböt
    struct foo array[n];
    int i;
    for(iter = *chars, i = 0; iter != NULL; iter = iter->next, i++) {
        array[i].c = iter;
        array[i].dist = iter->shadowRect.y;
    }

    // Quicksort
    qsort(array, n ,sizeof(struct foo), compare_foo);

    *chars = array[0].c;
    // Újrafelépiti a listát
    for(i = 0; i < n-1; i++) {
        array[i].c->next = array[i + 1].c;
    }
    array[i].c->next = NULL;
}

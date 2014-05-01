#include <string.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include "main.h"
#include "flare.h"
#include "free.h"
#include "woody.h"
#include "dummy.h"
#include "bandit.h"
#include "hunter.h"
#include "mark.h"
#include "jack.h"
#include "julian.h"
#include "character.h"
#include "imgload.h"

/* Globális (modulon belüli) változó: */
static struct character *init_chars[chr_num];

/* Saját függvények: */
static struct character * init_character(enum characters chrname, char *chrstring);
static struct character * ChrCpy(struct character *src);

/** Inicializáló karaktereket létrehozó függvény. Csak ezek a karakterek hozzák létre és tárolják el
  * az SDL_Surface-eket, az összes többi karakter csak az ezek képeire csinál pointereket. Amiért ez
  * jó, az az, hogy ha pl van 30 db ugyan olyan karakter, akkor a karakterhez tartozó képek akkor is
  * mindössze egyszer jönnek létre a memóriába, és egy karakter halálakor nem kell felszabadítani
  * őket. Viszont ezért cserébe kb kétszer annyi inicializáló függvény kell, de ennélkül élvezhetet-
  * lenül lassú lenne a játék */
void Init_characters (void) { // Hogy minden karakterhez csak egyszer kelljen betölteni, forgatni, felnagyitani a képeket
    init_chars[flare] = init_character(flare, "Flare");
    init_chars[woody] = init_character(woody, "Woody");
    init_chars[bandit] = init_character(bandit, "Bandit");
    init_chars[hunter] = init_character(hunter, "Hunter");
    init_chars[mark] = init_character(mark, "Mark");
    init_chars[jack] = init_character(jack, "Jack");
    init_chars[dummy] = init_character(dummy, "Woody");
    init_chars[julian] = init_character(julian, "Julian");
}

/** A karaktereket tároló lista elejére készit egy másolatot a megfelelő init karakterről */
void spawn_character(struct character **chars, enum characters chrname, int side, int x, int y, int hp) {
    struct character *c = ChrCpy(init_chars[chrname]); //Struktúra értékadással természetesen az SDL_Surface* tömbök nem másolódnának át
    c->hp = hp;
    c->maxhp = hp;
    c->loc.x = x;
    c->loc.y = y;
    c->shadowRect.x = x + 5 * framesize / 16;
    c->shadowRect.y = y + framesize - 4;
    c->side = side;
    if(side == hero || side == friendly)
        c->dir = right;
    else
        c->dir = left;
    // A játékos sebessége "definíció szerint" 1 legyen, még akkor is ha amúgy
    // egy 0.6-es sebessébű íjásszal játszik a felhasználó
    if(side == hero)
        c->speed = 1;
    c->vunerable = GetTicks() + 500;
    c->next = *chars;
    *chars = c;
}

/** A sebzésekhez szükséges dinamikus tárolókat inicalizálja.
  * A legtöbb tömböt azért nullázza ki, hogy a karakterek modul-
  * jaiban csak azokat kelljen megadni amik nem nullák. */
void Init_Damage(struct character *c) {
    c->walk.dmg.range = malloc (c->s_walk_No * sizeof(int));
    c->walk.dmg.impact = malloc (c->s_walk_No * sizeof(int));
    c->melee.a_dmg.range = malloc(c->a_melee_No * sizeof(int));
    c->melee.a_dmg.impact = malloc(c->a_melee_No * sizeof(int));
    c->melee.s_dmg.range = malloc(c->s_melee_No * sizeof(int));
    c->melee.s_dmg.impact = malloc(c->s_melee_No * sizeof(int));
    c->counter.dmg.range = malloc (c->counter_No * sizeof(int));
    c->counter.dmg.impact = malloc (c->counter_No * sizeof(int));
    c->ground.dmg.range = malloc (c->ground_No * sizeof(int));
    c->ground.dmg.impact = malloc (c->ground_No * sizeof(int));
    // Ez jacknél melee skill, a többieknél ranged
    if(c->chrname == jack) {
        c->jump.dmg1.range = malloc (c->jump_attack_No * sizeof(int));
        c->jump.dmg1.impact = malloc (c->jump_attack_No * sizeof(int));
        int i;
        for(i = 0 ; i < c->jump_attack_No; i++) {
            c->jump.dmg1.range[i] = 0;
            c->jump.dmg1.impact[i] = 0;
        }
    }

    int i;
    for(i = 0 ; i < c->s_walk_No; i++) {
        c->walk.dmg.range[i] = 0;
        c->walk.dmg.impact[i] = 0;
    }
    for(i = 0 ; i < c->a_melee_No; i++) {
        c->melee.a_dmg.range[i] = 0;
        c->melee.a_dmg.impact[i] = 0;
    }
    for(i = 0 ; i < c->s_melee_No; i++) {
        c->melee.s_dmg.range[i] = 0;
        c->melee.s_dmg.impact[i] = 0;
    }
    for(i = 0 ; i < c->counter_No; i++) {
        c->counter.dmg.range[i] = 0;
        c->counter.dmg.impact[i] = 0;
    }
    for(i = 0 ; i < c->ground_No - 1; i++) {
        c->ground.dmg.range[i] = 0;
        c->ground.dmg.impact[i] = 0;
    }
    // A groundból felkelés minden karakternél sebez
    c->ground.dmg.range[c->ground_No - 1] = framesize;
    c->ground.dmg.impact[c->ground_No - 1] = 2;
    c->ground.dmg.value = 48/6;

    // A távolsági skilleknek két range-e van, az első, hogy milyen közel kell mennie az ellenséghez hoz felrobbanjon,
    // a második, hogy robbanáskor mekkora környezetben sebez
    c->jump.dmg2.range = malloc (2 * sizeof(int));
    c->jump.dmg2.impact = malloc (sizeof(int));
    c->ranged.a_dmg.range = malloc (2 * sizeof(int));
    c->ranged.a_dmg.impact = malloc (sizeof(int));
    c->ranged.s_dmg.range = malloc (2 * sizeof(int));
    c->ranged.s_dmg.impact = malloc (sizeof(int));
    if(c->chrname != jack) {
        c->jump.dmg1.range = malloc (2 * sizeof(int));
        c->jump.dmg1.impact = malloc (sizeof(int));
        c->jump.dmg1.range[0] = c->jump.dmg1.range[1] = 0;
        c->jump.dmg1.impact[0] = 0;
    }

    c->jump.dmg2.range[0] = c->jump.dmg2.range[1] = 0;
    c->jump.dmg2.impact[0] = 0;
    c->ranged.a_dmg.range[0] = c->ranged.a_dmg.range[1] = 0;
    c->ranged.a_dmg.impact[0] = 0;
    c->ranged.s_dmg.range[0] = c->ranged.s_dmg.range[1] = 0;
    c->ranged.s_dmg.impact[0] = 0;
}

/** Felszabadítja az inicializásó karaktereket */
void spawn_kill_all(void) {
    int i;
    for(i=0; i<chr_num; i++)
        spawn_kill(init_chars[i]);
}


/** Inicializáló karakterek létrehozása, változók inicializálása, képek betöltése,
  * tükrözése, forgatása, anti-aliasing meghívása, majd a fullscreen méretéhez
  * felnagyítása. Természetes az itt felsoroltak fizikailag nem mind ebben a
  * függvényben vannak megvalósítva, hanem nagyrészt az imgload modulban. */
static struct character * init_character(enum characters chrname, char *chrstring) {
    struct character *c = (struct character *) malloc(sizeof(struct character));

    // General Init
    c->chrname = chrname;
    c->loc.w = framesize;
    c->loc.h = framesize;
    c->shadowRect.w = 0;
    c->shadowRect.h = 0;
    c->vx = 0;
    c->vy = 0;
    c->vz = 0;
    c->impulse_x = 0;
    c->impulse_z = 0;
    c->speed = 0;
    c->collision_modifier = 1;
    c->act = a_walk;
    c->state = 0;
    c->stunned = false;
    c->spotted = false;
    c->dmg.value = 0;

    // Frame Nok, cooldownok és damagek a karakter moduljába inicializálodnak, de itt kinullázom őket,
    // hogy csak azokat kelljen megadni, amihez tényleg van is skill
    c->a_walk_No = 0;
    c->s_walk_No = 0;
    c->jump_attack_No = 0;
    c->jump_attack2_No = 0;
    c->jump_attack_proj_No = 0;
    c->jump_attack2_proj_No = 0;
    c->a_melee_No = 0;
    c->s_melee_No = 0;
    c->a_cast_No = 0;
    c->s_cast_No = 0;
    c->a_proj_No = 0;
    c->s_proj_No = 0;
    c->a_exp_No = 0;
    c->s_exp_No = 0;
    c->s_vanish_No = 0;
    c->jump_attack_exp_No = 0;
    c->counter_No = 0;
    c->ground_No = 0;
    c->air_No = 0;
    c->dead_No = 0;

    // Cooldowns
    c->s_walk_cd = 0;
    c->a_melee_cd = 0;
    c->jump_attack_cd = 0;
    c->s_melee_cd = 0;
    c->a_cast_cd = 0;
    c->s_cast_cd = 0;
    c->counter_cd =  0;
    c->def_cd = 500;

    // Damages
    c->walk.dmg.value = 0;
    c->jump.dmg1.value = 0;
    c->jump.dmg2.value = 0;
    c->melee.a_dmg.value = 0;
    c->melee.s_dmg.value = 0;
    c->ranged.a_dmg.value = 0;
    c->ranged.s_dmg.value = 0;
    c->counter.dmg.value = 0;

    // A karakterfüggő inicializáció meghivása
    void (*init_constants[chr_num])(struct character *c);
    init_constants[flare] = Init_Flare;
    init_constants[woody] = Init_Woody;
    init_constants[bandit] = Init_Bandit;
    init_constants[hunter] = Init_Hunter;
    init_constants[mark] = Init_Mark;
    init_constants[jack] = Init_Jack;
    init_constants[dummy] = Init_Dummy;
    init_constants[julian] = Init_Julian;
    init_constants[chrname](c);

    // Walk
    c->walk.vx = 0;
    c->walk.vz = 0;
    c->walk.cd = 0;
    c->walk.state = 1;
    AnimLoader(&(c->walk.la), &(c->walk.ra), chrstring, &(c->a_walk_No), "a_walk");
    AnimLoader(&(c->walk.lstand), &(c->walk.rstand), chrstring, &(c->a_walk_No), "stand");
    AnimLoader(&(c->walk.ls), &(c->walk.rs), chrstring, &(c->s_walk_No), "s_walk");

    // Jump
    c->jump.second = false;
    c->jump.cd1 = 0;
    c->jump.cd2 = 0;
    Adv_ImgLoader(&(c->jump.la), &(c->jump.ra), chrstring, "jump");
    AnimLoader(&(c->jump.lattack), &(c->jump.rattack), chrstring, &(c->jump_attack_No), "jump_attack");
    AnimLoader(&(c->jump.lattack2), &(c->jump.rattack2), chrstring, &(c->jump_attack2_No), "jump_attack2");
    AnimLoader(&(c->jump.lproj), &(c->jump.rproj), chrstring, &(c->jump_attack_proj_No), "jump_attack_proj");
    AnimLoader(&(c->jump.lproj2), &(c->jump.rproj2), chrstring, &(c->jump_attack2_proj_No), "jump_attack2_proj");
    AnimLoader2(&(c->jump.lexp), &(c->jump.rexp), chrstring, &(c->jump_attack_exp_No), "jump_attack_exp");

    // Melee
    c->melee.next = 0;
    c->melee.a_cd = 0;
    c->melee.s_cd = 0;
    AnimLoader(&(c->melee.la), &(c->melee.ra), chrstring, &(c->a_melee_No), "a_melee");
    AnimLoader(&(c->melee.ls), &(c->melee.rs), chrstring, &(c->s_melee_No), "s_melee");

    // Ranged attack
    c->ranged.a_cd =  0;
    c->ranged.s_cd =  0;
    AnimLoader(&(c->ranged.la_cast), &(c->ranged.ra_cast), chrstring, &(c->a_cast_No), "a_cast");
    AnimLoader(&(c->ranged.la_proj1), &(c->ranged.ra_proj1), chrstring, &(c->a_proj_No), "a_proj1");
    AnimLoader(&(c->ranged.la_proj2), &(c->ranged.ra_proj2), chrstring, &(c->a_proj_No), "a_proj2");
    AnimLoader(&(c->ranged.la_exp), &(c->ranged.ra_exp), chrstring, &(c->a_exp_No), "a_exp");
    AnimLoader(&(c->ranged.ls_proj), &(c->ranged.rs_proj), chrstring, &(c->s_proj_No), "s_proj");
    AnimLoader2(&(c->ranged.ls_exp), &(c->ranged.rs_exp), chrstring, &(c->s_exp_No), "s_exp");
    AnimLoader(&(c->ranged.ls_cast), &(c->ranged.rs_cast), chrstring, &(c->s_cast_No), "s_cast");
    AnimLoader(&(c->ranged.ls_vanish), &(c->ranged.rs_vanish), chrstring, &(c->s_vanish_No), "s_vanish");

    // Defence
    c->def.cd = 0;
    Adv_ImgLoader(&(c->def.la), &(c->def.ra), chrstring, "def");

    // Counter
    c->counter.cd = 0;
    AnimLoader(&(c->counter.la), &(c->counter.ra), chrstring, &(c->counter_No), "counter");

    // Hit
    c->hit.cd = 0;
    Adv_ImgLoader(&(c->hit.la), &(c->hit.ra), chrstring, "hit");

    // Ground
    c->ground.cd = 0;
    c->ground.get_up = false;
    AnimLoader(&(c->ground.la), &(c->ground.ra), chrstring, &(c->ground_No), "ground");

    // Air
    AnimLoader(&(c->air.la), &(c->air.ra), chrstring, &(c->air_No), "air");

    // Dead
    c->dead.timer = 0;
    AnimLoader(&(c->dead.la), &(c->dead.ra), chrstring, &(c->dead_No), "dead");

    // Flip
    c->flip.x = 0;
    c->flip.y = 0;
    c->flip.angle = 0;
    c->flip.angle_inc = 0;
    c->flip.current = NULL;

    c->next = NULL;
    return c;
}

/** Ez másolja át az inicializáló karakter képeit a konkrét karakterbe.
  * Azért nem működik ez egy egyszerű "bitről bitre" másolással, mert
  * a képek pointerei dinamikus tömbbe vannak tárolva, és azt külön létre
  * kell hozni */
static struct character * ChrCpy(struct character *src) {
    struct character *c = (struct character *) malloc(sizeof(struct character));

    //Struktúra értékadás, ez mindent átmásol, de sajnos az SDL_Surface* tömböket manuálisan kell
    *c = *src;

    //Walk
    c->walk.la = AnimCpy(src->walk.la, src->a_walk_No);
    c->walk.ra = AnimCpy(src->walk.ra, src->a_walk_No);
    c->walk.lstand = AnimCpy(src->walk.lstand, src->a_walk_No);
    c->walk.rstand = AnimCpy(src->walk.rstand, src->a_walk_No);
    c->walk.ls = AnimCpy(src->walk.ls, src->s_walk_No);
    c->walk.rs = AnimCpy(src->walk.rs, src->s_walk_No);

    // Jump
    c->jump.lattack = AnimCpy(src->jump.lattack, src->jump_attack_No);
    c->jump.rattack = AnimCpy(src->jump.rattack, src->jump_attack_No);
    c->jump.lattack2 = AnimCpy(src->jump.lattack2, src->jump_attack2_No);
    c->jump.rattack2 = AnimCpy(src->jump.rattack2, src->jump_attack2_No);
    c->jump.lproj = AnimCpy(src->jump.lproj, src->jump_attack_proj_No);
    c->jump.rproj = AnimCpy(src->jump.rproj, src->jump_attack_proj_No);
    c->jump.lproj2 = AnimCpy(src->jump.lproj2, src->jump_attack2_proj_No);
    c->jump.rproj2 = AnimCpy(src->jump.rproj2, src->jump_attack2_proj_No);
    c->jump.lexp = AnimCpy(src->jump.lexp, src->jump_attack_exp_No);
    c->jump.rexp = AnimCpy(src->jump.rexp, src->jump_attack_exp_No);

    // Melee
    c->melee.la = AnimCpy(src->melee.la, src->a_melee_No);
    c->melee.ra = AnimCpy(src->melee.ra, src->a_melee_No);
    c->melee.ls = AnimCpy(src->melee.ls, src->s_melee_No);
    c->melee.rs = AnimCpy(src->melee.rs, src->s_melee_No);

    // Ranged attack
    c->ranged.la_cast = AnimCpy(src->ranged.la_cast, src->a_cast_No);
    c->ranged.ra_cast = AnimCpy(src->ranged.ra_cast, src->a_cast_No);
    c->ranged.la_proj1 = AnimCpy(src->ranged.la_proj1, src->a_proj_No);
    c->ranged.ra_proj1 = AnimCpy(src->ranged.ra_proj1, src->a_proj_No);
    c->ranged.la_proj2 = AnimCpy(src->ranged.la_proj2, src->a_proj_No);
    c->ranged.ra_proj2 = AnimCpy(src->ranged.ra_proj2, src->a_proj_No);
    c->ranged.la_exp = AnimCpy(src->ranged.la_exp, src->a_exp_No);
    c->ranged.ra_exp = AnimCpy(src->ranged.ra_exp, src->a_exp_No);
    c->ranged.ls_proj = AnimCpy(src->ranged.ls_proj, src->s_proj_No);
    c->ranged.rs_proj = AnimCpy(src->ranged.rs_proj, src->s_proj_No);
    c->ranged.ls_exp = AnimCpy(src->ranged.ls_exp, src->s_exp_No);
    c->ranged.rs_exp = AnimCpy(src->ranged.rs_exp, src->s_exp_No);
    c->ranged.ls_cast = AnimCpy(src->ranged.ls_cast, src->s_cast_No);
    c->ranged.rs_cast = AnimCpy(src->ranged.rs_cast, src->s_cast_No);
    c->ranged.ls_vanish = AnimCpy(src->ranged.ls_vanish, src->s_vanish_No);
    c->ranged.rs_vanish = AnimCpy(src->ranged.rs_vanish, src->s_vanish_No);

    // Counter
    c->counter.la = AnimCpy(src->counter.la, src->counter_No);
    c->counter.ra = AnimCpy(src->counter.ra, src->counter_No);

    // Ground
    c->ground.la = AnimCpy(src->ground.la, src->ground_No);
    c->ground.ra = AnimCpy(src->ground.ra, src->ground_No);

    // Air
    c->air.la = AnimCpy(src->air.la, src->air_No);
    c->air.ra = AnimCpy(src->air.ra, src->air_No);

    // Dead
    c->dead.la = AnimCpy(src->dead.la, src->dead_No);
    c->dead.ra = AnimCpy(src->dead.ra, src->dead_No);

    c->next = NULL;
    return c;
}

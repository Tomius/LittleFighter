#include <math.h>
#include <stdlib.h>
#include "character.h"
#include "projectile.h"
#include "main.h"
#include "bg.h"
#include "imgload.h"
#include "AI.h"

/* Globális változó (kívülről is látható) */
enum difficulty diff;

/* A modul saját függvényei: */
static int Will_i_get_hit(struct character *who, struct projectile *projs);
static void Melee_AI(struct character *iter, struct character *player, struct character *Enemy, struct projectile *projs, double distance);
static void Ranged_AI(struct character *iter, struct character *player, struct character *Enemy, struct projectile *projs, double distance);
static int Reaction_time(enum difficulty Diff) ;

/** A függvény nevéből is könnyű kitalálni, hogy ez a mesterséges intelligenciáért felelős
  * fv, ez mondja meg a friendly és az enemey karaktereknek hogy mit csináljanak. Érdekessége
  * hogy a hunter (az  íjász) kivételével az összes többi karakternek ugyan az az AI-ja,
  * csak a skilljeikből fakadó különbségekből játszanak teljesen másképp. */
void AI(int Tick, struct character *chars, struct character *player, struct projectile *projs) {
    struct character *iter;
    for(iter = chars; iter != NULL; iter = iter->next) {
        if(iter->side == enemy || iter->side == friendly) {

            // Általános AI (Preprocess)
            if(iter->act == dead)
                continue;
            if(iter->side == enemy && iter->loc.x < framesize)
                iter->walk.vx = 1;
            if(iter->side == enemy && iter->loc.x + framesize > kx)
                iter->walk.vx = -1;

            // Az AIhoz segédszámitások

            // Megkeresi a legközelebbi, támadható ellenséget (NULL-t ad vissza ha ilyen nincs)
            struct character *Enemy = NearestEnemy(chars, iter);
            // Megnézi hogy a képernyőn van-e a karakter és az ellenség is(vagy voltak-e már a képernyőn valaha)
            if(Enemy != NULL && Enemy->spotted == false && -2*framesize/3 < Enemy->loc.x && Enemy->loc.x < kx - framesize/3)
                Enemy->spotted = true;
            if(iter != NULL && iter->spotted == false && -2*framesize/3 < iter->loc.x && iter->loc.x < kx - framesize/3)
                iter->spotted = true;

            // Woody kövesse a karakterünket ha nincs kit ölni
            if( (Enemy == NULL || Enemy->spotted == false) && iter->side == friendly && TerrainMax != 0) {
                if(rand()%2 == 0 || iter->walk.vx || iter->walk.vz) {
                    if(player->loc.x > iter->loc.x + framesize)
                        iter->walk.vx = 1;
                    else if(player->loc.x + framesize < iter->loc.x)
                        iter->walk.vx = -1;
                    else if(fabs(player->loc.x - iter->loc.x) > 3*framesize/4 || !((iter->loc.x + framesize + 10 < kx &&  0 < iter->walk.vx) || (10 < iter->loc.x && iter->walk.vx < 0)))
                        iter->walk.vx = 0;

                    if(player->loc.y > iter->loc.y + framesize)
                        iter->walk.vz = 1;
                    else if(player->loc.y + framesize < iter->loc.y)
                        iter->walk.vz = -1;
                    else if(fabs(player->loc.y - iter->loc.y) > 3*framesize/4 || !((kyTop < iter->loc.y && iter->walk.vz < 0) || (iter->shadowRect.y < kyBottom && 0 < iter->walk.vz)))
                        iter->walk.vz = 0;

                    if(fabs(player->loc.x - iter->loc.x) > kx/2 && iter->state == a_walk && iter->s_melee_No != 0 && iter->s_melee_cd < GetTicks()) {
                        iter->act = s_melee;
                        iter->melee.x = (player->loc.x - iter->loc.x > 0) ? (player->loc.x - framesize) : (player->loc.x + framesize);
                        iter->melee.y = player->loc.y + framesize/2;
                        if(iter->melee.y < kyTop + framesize)
                            iter->melee.y = kyTop + framesize;
                        iter->state = 0;
                        iter->melee.s_cd = GetTicks() + iter->s_melee_cd;
                        iter->dmg = iter->melee.s_dmg;
                    }
                }
                if(iter->walk.vx == 1)
                    iter->dir = right;
                else if (iter->walk.vx == -1)
                    iter->dir = left;
                else {
                    if(player->loc.x > iter->loc.x + framesize)
                        iter->dir = right;
                    else if(player->loc.x + framesize < iter->loc.x)
                        iter->dir = left;
                }
                continue;
            }
            // A többi karakter, meg ha nincs ki ellen harcolni(mert minden ellenség halott, vagy nincs még a képernyőn), akkor ne csináljon semmit
            if(Enemy == NULL || (iter->side == friendly && TerrainMax == 0 && (Enemy->loc.x < -framesize || Enemy->loc.x > kx) )) {
                if(player != NULL) {
                    // Ez a feltétel azért van, hogy lehessen lökdösni Woody-t :)
                    if(fabs(player->loc.x - iter->loc.x) > 3*framesize/4 || !((iter->loc.x + framesize + 10 < kx &&  0 < iter->walk.vx) || (10 < iter->loc.x && iter->walk.vx < 0)))
                        iter->walk.vx = 0;
                    if(fabs(player->loc.y - iter->loc.y) > 3*framesize/4 || !((kyTop < iter->loc.y && iter->walk.vz < 0) || (iter->shadowRect.y < kyBottom && 0 < iter->walk.vz)))
                        iter->walk.vz = 0;
                } else if(!benchmark) { // Ez a feltétel annyira nem fontos, csak a játék végén kb 2mp erejéig számit
                    iter->walk.vx = 0;
                    iter->walk.vz = 0;
                }
                continue;
            }
            // Az ellenségek ne támadjanak ha még nem lettek felfedezve
            if(iter->side == enemy && iter->spotted == false)
                continue;

            // A karakter és az ellensége közti távolság
            int distance = sqrt(pow(Enemy->loc.x - iter->loc.x, 2) + pow(Enemy->loc.y - iter->loc.y, 2));
            // A karakter nézzen az ellensége felé
            if(iter->loc.x > Enemy->loc.x && iter->act == a_walk)
                iter->dir = left;
            if(iter->loc.x < Enemy->loc.x && iter->act == a_walk)
                iter->dir = right;

            // Karakterfüggő AI
            switch (iter->chrname) {
            case bandit:
            case woody:
            case flare:
            case mark:
            case jack:
            case dummy:
            case julian:
                Melee_AI(iter, player, Enemy, projs, distance);
                break;
            case hunter:
                Ranged_AI(iter, player, Enemy, projs, distance);
                break;
            default:
                break;
            }
        }
    }
}

/** Ez a fv visszaadja a legközelebbi támadható elenséget a paraméterben kapott karakterhez képest.
  * Ha nincsen ellenség, akkor NULL-t ad vissza */
struct character * NearestEnemy(struct character *chars, struct character *who) {
    struct character *iter;
    int minDist = 10000;
    // Először megkeresem a miniumumot
    for(iter=chars; iter!=NULL; iter=iter->next) {
        if( ((who->side != enemy && iter->side == enemy) || (who->side == enemy && iter->side != enemy)) && iter->act != dead) {
            if( (int)sqrt(pow(who->loc.x - iter->loc.x, 2) + pow(who->loc.y - iter->loc.y, 2)) < minDist)
                minDist = (int)sqrt(pow(who->loc.x - iter->loc.x, 2) + pow(who->loc.y - iter->loc.y, 2));
        }

    }
    // Majd visszadom azt amelyiknek a távolsága egyenlő a minimum távolsággal
    for(iter=chars; iter!=NULL; iter=iter->next) {
        if( ((who->side != enemy && iter->side == enemy) || (who->side == enemy && iter->side != enemy)) && iter->act != dead) {
            if( (int)sqrt(pow(who->loc.x - iter->loc.x, 2) + pow(who->loc.y - iter->loc.y, 2)) == minDist)
                return iter;
        }

    }
    return NULL;
}

/** Megmondja hogy van-e még ellenség a pályán */
int Is_there_enemy(struct character *chars) {
    struct character *iter;
    for(iter=chars; iter!=NULL; iter=iter->next) {
        if(iter->side == enemy)
            return 1;
    }
    return 0;
}

/** Megmondja hogy van-e még szövetséges a pályán */
int Is_there_ally(struct character *chars) {
    struct character *iter;
    for(iter=chars; iter!=NULL; iter=iter->next) {
        if(iter->side != enemy)
            return 1;
    }
    return 0;
}

/** Megnézi hogy az adott karaktert 1-2 körön belül el fogja-e találni egy lövedék.
  * 0-t ad vissza ha nem, 1-t ha bal oldalról, 2-t ha jobb oldalról
  * 3-at ha mindkét oldalról veszély fenyegeti */
static int Will_i_get_hit(struct character *who, struct projectile *projs) {
    int Left = false, Right = false;
    struct projectile *p;
    for(p = projs; p != NULL; p = p->next)
        // Ha a p egy ellenség lövedéke
        if((p->side != enemy && who->side == enemy) || (p->side == enemy && who->side != enemy))
            // És még nem robbant fel
            if(p->vanish == false && p->exp == false)
                // És a közelben van
                if(fabs(p->x - who->loc.x) < 1.5 * framesize && (fabs(p->y - who->loc.y) < p->dmg.range[0] || p->type == jump_attack_proj || p->type == jump_attack2_proj)) {
                    // És a megfelelő irányba néz
                    if(who->loc.x > p->x && (p->dir == right || p->type == jump_attack2_proj))
                        Left = true;
                    if(who->loc.x < p->x && (p->dir == left || p->type == jump_attack2_proj))
                        Right = true;
                }
    return (Left + 2*Right);
}

/** Lényegében ez a fv végzi a karakterek 90%ánál (kivéve az íjászt) a tényleges mesterséges intelligenciát. */
static void Melee_AI(struct character *iter, struct character *player, struct character *Enemy, struct projectile *projs, double distance) {
    enum difficulty Diff = diff;
    // A barátok legyenek okosak
    if(iter->side == friendly)
        Diff = Hard;
    // Ha a játkosunk halott akkor ne csináljanak semmit, csak álljanak egyhelybe (és lehajtott fővel tisztelegjenek) */
    if(!benchmark && player->act == dead) {
        iter->walk.vx = 0;
        iter->walk.vz = 0;
    } else {
        // A karakterek mozogjanak a legközelebbi ellenségünk felé, egy kicsi randommal fűszerezve, hogy reálisabb legyen a mozgás.
        if(iter->act != a_walk && iter->act != jump) {
            iter->walk.vx = 0;
            iter->walk.vz = 0;
        } else if(((iter->walk.vx == 0 && iter->walk.vz == 0) || Reaction_time(Diff)) && iter->impulse_x == 0 && iter->impulse_z == 0) {
            if(Enemy->loc.x + framesize/2  < iter->loc.x) {
                iter->dir = left;
                iter->walk.vx = -1;
            } else if (Enemy->loc.x > iter->loc.x + framesize/2 ) {
                iter->dir = right;
                iter->walk.vx = 1;
            } else
                iter->walk.vx = 0;

            /*if(Enemy->loc.y + framesize / 4 < iter->loc.y)
                iter->walk.vz = -1;
            else if(Enemy->loc.y > iter->loc.y + framesize/4)
                iter->walk.vz = 1;
            else
                iter->walk.vz = 0;
            */
        }
        // Ha az AI szintje nehéz, akkor az karakterek megpróbálnak védekezni is, persze csak ha kell
        if(Diff == Hard) {
            int defence;
            if( (defence = Will_i_get_hit(iter,projs)) && (iter->act == a_walk || iter->act == a_melee)) {
                if(defence == 1) {
                    if(iter->def.la != NULL && iter->def.cd < GetTicks()) {
                        iter->dir = left;
                        iter->act = def;
                        iter->hit.cd = GetTicks() + 600;
                        iter->state = 0;
                    }
                }
                if(defence == 2) {
                    if(iter->def.ra != NULL && iter->def.cd < GetTicks()) {
                        iter->dir = right;
                        iter->act = def;
                        iter->hit.cd = GetTicks() + 600;
                        iter->state = 0;
                    }
                }
                if(defence == 3) {
                    if(iter->counter_No != 0 && iter->counter.cd < GetTicks()) {
                        iter->act = counter;
                        iter->state = 0;
                        iter->counter.cd = GetTicks() + iter->counter.cd;
                        iter->dmg = iter->counter.dmg;
                    } else {
                        iter->act = jump;
                        iter->vy = -5;
                    }
                }
            }
        }
        // Ha nem kell védekeznie, akkor támad
        if(iter->act == a_walk && Enemy->act != ground && Reaction_time(Diff)) {
            // Meleek
            // A_melee
            if(fabs(Enemy->loc.y - iter->loc.y) < 3 * framesize / 4 && iter->melee.a_cd < GetTicks() && iter->a_melee_No != 0) {
                if(distance < framesize) {
                    iter->act = a_melee;
                    iter->state = 0;
                    iter->melee.next = iter->a_melee_No;
                    iter->melee.a_cd = GetTicks() + iter->a_melee_cd;
                    iter->dmg = iter->melee.a_dmg;
                }
            }
            // S_walk
            if(fabs(Enemy->loc.y - iter->loc.y) < framesize/2  && iter->walk.cd < GetTicks() && iter->s_walk_No != 0 && iter->act == a_walk && rand()%(Diff+1) != 0) {
                if(distance > framesize && distance < 3 * framesize) {
                    iter->act = s_walk;
                    iter->vx = iter->dir == right ? 7 : -7;
                    iter->state = 0;
                    iter->walk.state = 0;
                    iter->walk.cd = GetTicks() + iter->s_walk_cd;
                    iter->dmg = iter->walk.dmg;
                }
            }
            // Ranged skillek
            if(fabs(Enemy->loc.y - iter->loc.y) < framesize/2 && distance > 1.5 * framesize && iter->act == a_walk && iter->loc.x > -framesize && iter->loc.x < kx && rand()%(Diff+2) != 0) {
                // S_ranged
                if(iter->s_cast_No != 0 && iter->ranged.s_cd < GetTicks() && rand()%(Diff+2) != 0 && rand()%2 == 0) {
                    iter->act = s_ranged;
                    iter->state = 0;
                    iter->dmg = iter->ranged.s_dmg;
                }
                // A_ranged
                else if(iter->a_cast_No != 0 && iter->ranged.a_cd < GetTicks() && rand()%(Diff+2) != 0 && rand()%2 == 0) {
                    iter->act = a_ranged;
                    iter->state = 0;
                    iter->dmg = iter->ranged.a_dmg;
                }
            }
            // Jump_attack
            if(distance < 1.5 * framesize && fabs(Enemy->loc.y - iter->loc.y) < framesize && iter->jump.cd1 < GetTicks() && iter->jump_attack_No != 0 && iter->act == a_walk && rand()%(Diff+2) != 0) {
                iter->act = jump;
                iter->vy = -4;
            }
            // Jump_attack_2
            if(iter->jump_attack2_No != 0 && iter->jump.cd2 < GetTicks() && iter->act == a_walk && Enemy->loc.x > framesize
                    && Enemy->loc.x + framesize < kx  && iter->loc.x > -framesize && iter->loc.x < kx  && rand()%(Diff+1) != 0) {
                iter->act = jump;
                iter->vy = -4;
            }
            if(iter->act == a_walk) {
                // Teleport
                if(iter->s_melee_No != 0 && iter->melee.s_cd < GetTicks() && distance > 2 * framesize
                        && Enemy->loc.x > framesize && Enemy->loc.x + framesize < kx  && rand()%(Diff+1) != 0) {
                    iter->act = s_melee;
                    iter->state = 0;
                    if(iter->dir == right)
                        iter->melee.x = Enemy->loc.x;
                    else
                        iter->melee.x = Enemy->loc.x + framesize;
                    iter->melee.y = Enemy->loc.y + framesize;
                    if(Enemy->loc.y < kyTop + framesize)
                        iter->melee.y = kyTop + framesize;
                    iter->melee.s_cd = GetTicks() + iter->s_melee_cd;
                    iter->dmg = iter->melee.s_dmg;
                } else if(rand()%25 == 0) { // Flip
                    iter->act = jump;
                    iter->vy = -4;
                }
            }
        }
        //Jump
        if(iter->act == jump && iter->jump.second == false && rand()%2 == 0) {
            // Jump_Attack
            if(distance < 1.5 * framesize && fabs(Enemy->loc.y - iter->loc.y) < 2 * framesize && iter->jump.cd1 < GetTicks() && iter->jump_attack_No != 0) {
                iter->act = jump_attack;
                iter->state = 0;
                iter->jump.cd1 = GetTicks() + iter->jump_attack_cd;
                iter->dmg = iter->jump.dmg1;
            }
            // Jump_Attack_2
            else if(iter->jump_attack2_No != 0 && iter->jump.cd2 < GetTicks() && distance >= 1.5 * framesize
                    && Enemy->loc.x > framesize && Enemy->loc.x + framesize < kx && iter->act == jump) {
                iter->act = jump_attack2;
                iter->state = 0;
                iter->jump.x = Enemy->loc.x;
                iter->jump.y = Enemy->loc.y;
                if(iter->jump.y < kyTop)
                    iter->jump.y = kyTop;
                if(iter->jump.y > kyBottom - framesize/2)
                    iter->jump.y = kyBottom - framesize/2;
                iter->jump.cd2 = GetTicks() + iter->jump_attack_cd;
                iter->dmg = iter->jump.dmg2;
            }
            // Flip
            if(iter->act == jump) {
                iter->act = flip;
                iter->flip.type = forward;
                if(iter->dir == right) {
                    iter->flip.angle = -5;
                    iter->flip.current = RotateSurface (iter->jump.ra, -5);
                    iter->flip.base = iter->jump.ra;
                    iter->vx = 4;
                    iter->loc.x -= iter->flip.current->w/2 - iter->jump.ra->w/2;
                    iter->loc.y -= iter->flip.current->h/2 - iter->jump.ra->h/2;
                    iter->flip.x = iter->flip.current->w/2;
                    iter->flip.y = iter->flip.current->h/2;
                } else {
                    iter->flip.angle = 5;
                    iter->flip.current = RotateSurface (iter->jump.la, 5);
                    iter->flip.base = iter->jump.la;
                    iter->vx = -4;
                    iter->loc.x -= iter->flip.current->w/2 - iter->jump.la->w/2;
                    iter->loc.y -= iter->flip.current->h/2 - iter->jump.la->h/2;
                    iter->flip.x = iter->flip.current->w/2;
                    iter->flip.y = iter->flip.current->h/2;
                }
                iter->flip.angle_inc = 10;
                iter->vy = -3;
                iter->state = 0;
                iter->jump.second = true;
            }
        }
        // Counter
        if((iter->act == hit || iter->act == ground) && iter->counter_No != 0 && iter->counter.cd < GetTicks() && rand()%((Diff*5) + 1) != 0) {
            if(distance < 1.4 * framesize) {
                iter->act = counter;
                iter->state = 0;
                iter->counter.cd = GetTicks() + iter->counter.cd;
                iter->dmg = iter->counter.dmg;
            }
        }
        // Teleport
        if(iter->act == air && iter->s_melee_No != 0 && iter->melee.s_cd < GetTicks()  && rand()%(Diff+1) != 0
                && Enemy->loc.x > framesize && Enemy->loc.x + framesize < kx) {
            iter->act = s_melee;
            iter->state = 0;
            iter->melee.x = rand()%(kx - 2 * framesize) + framesize;
            iter->melee.y = kyBottom - rand()%(kyBottom - kyTop - framesize);
            iter->melee.s_cd = GetTicks() + iter->s_melee_cd;
            iter->dmg = iter->melee.s_dmg;
        }
    }
}

/** A hunternek az AI-ja */
static void Ranged_AI(struct character *iter, struct character *player, struct character *Enemy, struct projectile *projs, double distance) {
    enum difficulty Diff = diff;
    // Bár íjász haver a jelenlegi verzióban nincs, de később még lehet
    if(iter->side == friendly)
        Diff = Hard;
    // Az íjászok is haggyák abba a lövöldözést ha meghalt a karakterünk
    if(!benchmark && player->act == dead) {
        iter->walk.vx = 0;
        iter->walk.vz = 0;
    } else {
        // A mozgásra egy picit összetettebb, mint a közelharcos karaktereknek, ő alapvetően
        // megpróbál távolságot tartani, de ha túl közel került egy ellenség akkor ő is inkább
        // közelharci fegyverként használja az íját, illetve a nehezségtől függően, néha
        // meg is próbál elugrani a közeli ellenségek elől.
        if(iter->act == a_walk && ((iter->walk.vx == 0 && iter->walk.vz == 0)
                                   || Reaction_time(Diff)) && iter->impulse_x == 0 && iter->impulse_z == 0) {
            bool close = false;
            bool melee_range = false;
            if(Enemy->loc.x + 5 * framesize - 5  < iter->loc.x) {
                iter->dir = left;
                iter->walk.vx = -1;
            } else if (Enemy->loc.x > iter->loc.x + 5 * framesize - 5) {
                iter->dir = right;
                iter->walk.vx = 1;
            } else {
                if(Enemy->loc.x > iter->loc.x)
                    iter->dir = right;
                else
                    iter->dir = left;

                if(!(Enemy->loc.x + framesize - 5  < iter->loc.x) && !(Enemy->loc.x > iter->loc.x + framesize - 5))
                    melee_range = true;
                else if(!(Enemy->loc.x + 2 * framesize - 5  < iter->loc.x) && !(Enemy->loc.x > iter->loc.x + 2 * framesize - 5))
                    close = true;

                if(iter->loc.x < 0)
                    iter->walk.vx = 1;
                else if(iter->loc.x > kx)
                    iter->walk.vx = -1;
                else
                    iter->walk.vx = 0;
            }
            if(Enemy->loc.y + framesize / 4 < iter->loc.y)
                iter->walk.vz = -1;
            else if(Enemy->loc.y > iter->loc.y + framesize/4)
                iter->walk.vz = 1;
            else
                iter->walk.vz = 0;
            if(melee_range && iter->walk.vz == 0 && iter->melee.a_cd < GetTicks() && rand()%(Diff+2) != 0) {
                if(Enemy->loc.x < iter->loc.x) {
                    iter->dir = left;
                    iter->act = a_melee;
                    iter->state = 0;
                    iter->melee.next = iter->a_melee_No;
                    iter->melee.a_cd = GetTicks() + iter->a_melee_cd;
                    iter->dmg = iter->melee.a_dmg;
                } else {
                    iter->dir = right;
                    iter->act = a_melee;
                    iter->state = 0;
                    iter->melee.next = iter->a_melee_No;
                    iter->melee.a_cd = GetTicks() + iter->a_melee_cd;
                    iter->dmg = iter->melee.a_dmg;
                }
            } else if((melee_range || close) && iter->jump.cd1 < GetTicks() && rand()%(Diff+1) != 0) {
                iter->vy = -4;
                iter->act = jump;
                iter->walk.vx = 0;
                if(Enemy->loc.y > iter->loc.y + framesize/2)
                    iter->walk.vz = 1;
                else if (Enemy->loc.y + framesize/2 < iter->loc.y)
                    iter->walk.vz = -1;
                else
                    iter->walk.vz = 0;
            } else if(close && iter->walk.vz == 0 && iter->melee.a_cd < GetTicks()) {
                if(Enemy->loc.x < iter->loc.x)
                    iter->walk.vx = -1;
                else
                    iter->walk.vx = 1;
            } else if((melee_range || close) && iter->walk.vz == 0) {
                if(Enemy->loc.x < iter->loc.x)
                    iter->walk.vx = 1;
                else
                    iter->walk.vx = -1;
            }
        }
        // Az ugrás közbeni mozgása
        else if(iter->act == jump) {
            if((TerrainRect.x > 0 || (TerrainRect.x == 0 && iter->loc.x > 3*framesize))
                    && (TerrainRect.x < TerrainMax || (TerrainRect.x == TerrainMax && iter->loc.x < kx - 3*framesize))) {
                if(Enemy->loc.x < iter->loc.x && iter->walk.vx == 0) {
                    iter->walk.vx = 2;
                    iter->dir = left;
                } else if(iter->walk.vx == 0) {
                    iter->walk.vx = -2;
                    iter->dir = right;
                }
            } else {
                if(TerrainRect.x > 0 && iter->walk.vx == 0) {
                    iter->walk.vx = -2;
                    iter->dir = left;
                } else if(TerrainRect.x == 0 && iter->walk.vx == 0) {
                    iter->walk.vx = 2;
                    iter->dir = right;
                }
            }
            iter->jump.cd1 = GetTicks() + iter->jump_attack_cd;
        }
        // Ha az AI nehézségi szintje Hard akkor védekezik is ha kell
        if(Diff == Hard) {
            int defence;
            if( (defence = Will_i_get_hit(iter,projs)) && (iter->act == a_walk || iter->act == a_melee)) {
                if(defence == 1) {
                    if(iter->def.la != NULL && iter->def.cd < GetTicks()) {
                        iter->dir = left;
                        iter->act = def;
                        iter->hit.cd = GetTicks() + 600;
                        iter->state = 0;
                    }
                }
                if(defence == 2) {
                    if(iter->def.ra != NULL && iter->def.cd < GetTicks()) {
                        iter->dir = right;
                        iter->act = def;
                        iter->hit.cd = GetTicks() + 600;
                        iter->state = 0;
                    }
                }
                if(defence == 3) {
                    if(iter->counter_No != 0 && iter->counter.cd < GetTicks()) {
                        iter->act = counter;
                        iter->state = 0;
                        iter->counter.cd = GetTicks() + iter->counter.cd;
                        iter->dmg = iter->counter.dmg;
                    } else {
                        iter->act = jump;
                        iter->vy = -5;
                    }
                }
            }
        }
        // Ha nem kell védekeznie, akkor támad
        // S_ranged
        if(Reaction_time(Diff) && iter->s_cast_No != 0 && iter->loc.x > 0 && iter->loc.x < kx && Enemy->act != ground && fabs(Enemy->loc.x - iter->loc.x) < 6 * framesize
                && ((Enemy->loc.x + 2 * framesize - 5  < iter->loc.x) || (Enemy->loc.x > iter->loc.x + 2 * framesize - 5)) && fabs(Enemy->loc.y - iter->loc.y) < framesize / 2
                && GetTicks() > iter->ranged.s_cd && iter->act == a_walk) {
            if(iter->loc.x > Enemy->loc.x) {
                iter->dir = left;
                iter->act = s_ranged;
                iter->state = 0;
                iter->dmg = iter->ranged.s_dmg;
            }
            if(iter->loc.x < Enemy->loc.x) {
                iter->dir = right;
                iter->act = s_ranged;
                iter->state = 0;
                iter->dmg = iter->ranged.s_dmg;
            }
        } else if(Reaction_time(Diff) && iter->a_cast_No != 0 && iter->loc.x > 0 && iter->loc.x < kx && Enemy->act != ground && fabs(Enemy->loc.x - iter->loc.x) < 6 * framesize
                  && ((Enemy->loc.x + 2 * framesize - 5  < iter->loc.x) || (Enemy->loc.x > iter->loc.x + 2 * framesize - 5)) && fabs(Enemy->loc.y - iter->loc.y) < framesize / 2
                  && GetTicks() > iter->ranged.a_cd && iter->act == a_walk && rand()%((Diff*10)+1) > 8) {
            if(iter->loc.x > Enemy->loc.x) {
                iter->dir = left;
                iter->act = a_ranged;
                iter->state = 0;
                iter->dmg = iter->ranged.a_dmg;
            }
            if(iter->loc.x < Enemy->loc.x) {
                iter->dir = right;
                iter->act = a_ranged;
                iter->state = 0;
                iter->dmg = iter->ranged.a_dmg;
            }
        }
    }
}

/** A Reakcióidő alapvetően a véletlenen alapul, de minden egyes próbálkozásnál
  * egyre nagyobb az esélye hogy igazzal tér vissza (hogy a karakter ne ragadjon be úgy hogy áll és néz).
  * Az átlagos és maximális reakcióidő függ a nehézségtől. */
static int Reaction_time(enum difficulty Diff) {

    // Ez számolja hogy hányszor tért vissza hamissal a fv.
    static int tries = 0;

    int result = rand()%(Hard - Diff + 1) * (2*(Hard - Diff + 3) - tries);
    if(result != 0)
        tries++;
    else
        tries = 0;
    return  result == 0;
}


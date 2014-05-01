#include <stdlib.h>
#include <math.h>
#include "character.h"
#include "blit.h"
#include "animate.h"
#include "main.h"
#include "menu.h"
#include "free.h"
#include "level.h"
#include "AI.h"
#include "text.h"
#include "bg.h"
#include "music.h"

// --------- Globális változók -----------
int level, stage, wave, timer, initTick, tempTerrainMax;
bool wavestart = true;

// Teszteléshez nagyon hasznos lehet ezeket true-ra állitani (minden ellenség egy
// ütésből meghal vagy Flare és Woody 3szor olyan gyors, nincsenek cooldownok)
// Meg amúgy még vicces is :)
bool instantkill = false;
bool superpower = false;

/** Az adott pályán incializálja a karaktereket, a háttérzenét, az adott pálya menetéhez szükséges karaktereket
  * illetve, ha az adott pálya elején van párbeszéd, akkor ahhoz is ez hivja meg a fv-t */
void init_level(int Tick, SDL_Surface *screen, struct character **chars, struct character **player, struct character **boss, struct character **Woody, struct projectile **projs) {
    switch (level) {
    case lvl_Practise_Arena: {
        spawn_character(chars, flare, hero, 200*scale, kyBottom - (kyBottom - kyTop - framesize) / 2 , 1000);
        *player = *chars;
        spawn_character(chars, dummy, enemy, kx - 200*scale, kyBottom - (kyBottom - kyTop -framesize)/2, 999999);
        playmusic(mus_survival);
        Text_bubble(*chars, "I heard you", "wanted to", "practise, bro", 3000, screen, *chars, *player, projs);
        Text_bubble(*chars, "I won't hurt", " you too much,", "I promise", 3000, screen, *chars, *player, projs);
    }
    break;
    case lvl_Survival: {
        spawn_character(chars, flare, hero, kx/2 - framesize/2, kyTop + (kyBottom - kyTop - framesize) / 4 , ( (Hard - diff) + 2)*450);
        *player = *chars;
        spawn_character(chars, woody, friendly, kx/2 - framesize/2, kyTop + 3*(kyBottom - kyTop - framesize) / 4, ((Hard - diff) + 2)*450);
        *Woody = *chars;
        playmusic(mus_survival);
        if(rand()%2 == 0)
            Text_bubble(*Woody, " ", "Let's do this!", " ", 2000, screen, *chars, *player, projs);
        else
            Text_bubble(*Woody, " ", "Kill 'em all!", " ", 2000, screen, *chars, *player, projs);
        wave = chr_num-4;
    }
    break;
    case 1: {
        spawn_character(chars, flare, hero, 200 * scale, kyTop + (kyBottom - kyTop - framesize) / 4 , 1200 );
        *player = *chars;
        spawn_character(chars, woody, friendly, 200 * scale, kyTop + 3*(kyBottom - kyTop - framesize) / 4, 1000);
        *Woody = *chars;
        playmusic(mus_level1);
        Text_bubble(*Woody, "There are a", " lot of bandits", "out there", 3000, screen, *chars, *player, projs);
        Text_bubble(*Woody, "It's dangeorous", "to go alone.", "Let me help you", 3000, screen, *chars, *player, projs);
        Text_bubble(*player, " ", "Thx, bro!", " ", 1000, screen, *chars, *player, projs);
        wave = 0;
    }
    break;
    case 2: {
        spawn_character(chars, flare, hero, 200 * scale, kyTop + (kyBottom - kyTop - framesize) / 4 , 1500 );
        *player = *chars;
        spawn_character(chars, woody, friendly, 200 * scale, kyTop + 3*(kyBottom - kyTop - framesize) / 4, 1250);
        *Woody = *chars;
        playmusic(mus_level2);
        Text_bubble(*Woody, "We were lucky ", "to find that", " secret entrace", 3500, screen, *chars, *player, projs);
        Text_bubble(*Woody, "to the bandits", "base. Let's get to", "their leader", 3000, screen, *chars, *player, projs);
        Text_bubble(*player, "Sure!", "But what a", "creepy place.", 3000, screen, *chars, *player, projs);
        Text_bubble(*Woody, "Did you expect", "different", "from bandits?", 3000, screen, *chars, *player, projs);
        wave = 0;
    }
    break;
    case 3: {
        spawn_character(chars, flare, hero, 200 * scale, kyTop + (kyBottom - kyTop - framesize) / 4 , 1200);
        *player = *chars;
        spawn_character(chars, woody, friendly, 100 * scale, kyTop + 3*(kyBottom - kyTop - framesize) / 4, 850);
        *Woody = *chars;
        spawn_character(chars, julian, enemy, kx - 100 * scale, kyTop + 3*(kyBottom - kyTop - framesize) / 4, instantkill ? 100 : 2500);
        *boss = *chars;
        (*boss)->melee.s_cd = SDL_GetTicks() + 5000;
        (*boss)->walk.cd = SDL_GetTicks() + 3000;
        playmusic(mus_level3);
        Text_bubble(*Woody, "At least", "we have found", "their leader.", 3000, screen, *chars, *player, projs);
        Text_bubble(*player, "Let's", "show him", "what we've got.", 3000, screen, *chars, *player, projs);
        Text_bubble(*Woody, "We will be", "legends if ", "we kill him", 3500, screen, *chars, *player, projs);
        Text_bubble(*player, "They will", "call us the ", "Little Fighters.", 3000, screen, *chars, *player, projs);
        aNextTick = dNextTick = GetTicks();
        wave = 0;
    }
    break;
    case lvl_Benchmark: {
        Uint32 start = GetTicks(); // Itt nem elhanyagolható a karakterek spawnolásával töltött idő
        int i;
        for(i=0; i<15; i++) {
            spawn_character(chars, woody, friendly, rand()%kx, kyTop + rand()%(kyBottom - kyTop - framesize), 1500);
            spawn_character(chars, flare, friendly, rand()%kx, kyTop + rand()%(kyBottom - kyTop - framesize), 1500);
            spawn_character(chars, rand()%(chr_num - 3) + 3, enemy, rand()%kx, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, 1000);
            spawn_character(chars, julian, enemy, rand()%kx, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, 1000);
        }
        timer = GetTicks();
        initTick = Tick;
        playmusic(mus_benchmark);
        paused += GetTicks() - start;
    }
    break;
    default:
        break;
    }
    wavestart = true;

    // Megmonja a blit modulnak hogy a teljes képernyő megvátozott (hiszen új pálya lett)
    SDL_Rect whole_screen = {0, 0, kx, ky};
    Rect_add(&rectList_old, &whole_screen);
}

/** Karaktereket hoz életre, vagy öl meg, illetve a pályák menetét irányítja */
int spawn_or_kill (int Tick, SDL_Surface *screen, struct character **chars, struct character *player, struct character **Woody, struct character **boss, int *enemies_killed, bool *levelup, bool *retry, struct projectile **projs) {
    struct character *iter;
    bool alive = true;

    // Az instant kill módhoz kell
    int difficulty = instantkill ? -2 : diff;
    if(superpower && player != NULL) {
        player->speed = 3;
        player->s_walk_cd = 0;
        player->a_melee_cd = 0;
        player->jump_attack_cd = 0;
        player->s_melee_cd = 0;
        player->a_cast_cd = 0;
        player->s_cast_cd = 0;
        player->counter_cd =  0;
        player->def_cd = 0;
        player->jump.second = false;
    }
    if(superpower && *Woody != NULL) {
        (*Woody)->speed = 3;
        (*Woody)->s_walk_cd = 0;
        (*Woody)->a_melee_cd = 0;
        (*Woody)->jump_attack_cd = 0;
        (*Woody)->s_melee_cd = 0;
        (*Woody)->a_cast_cd = 0;
        (*Woody)->s_cast_cd = 0;
        (*Woody)->counter_cd =  0;
        (*Woody)->def_cd = 0;
        (*Woody)->jump.second = false;
    }



    for (iter = *chars; iter != NULL; iter = iter->next) {
        // Megnézem hogy vannak-e halott karakterek
        if(iter->hp <= 0 && iter->act != dead && iter->act != jump) {
            iter->act = dead;
            iter->state = 0;
            if(iter->side == enemy)
                *enemies_killed += 1;
        }
    }

    switch (level) {
    case lvl_Practise_Arena: {
        // Ha a karakterünk megakarna halni, akkor legyen újra max élete
        if(player->hp < 50)
            player->hp = player->maxhp;
    }
    break;
    case lvl_Survival: {
        // Ha van halott karakter
        for (iter = *chars; iter != NULL; iter = iter->next) {
            if(iter->act == dead && iter->side != hero && iter->dead.timer != 0 && iter->dead.timer < GetTicks()) {
                if(iter == *Woody)
                    *Woody = NULL;
                // Akkor az tűnjön el a pályáról
                delete_char(chars, iter);
                iter = *chars;
            }
        }
        // Ha minden ellenség halott
        if(!Is_there_enemy(*chars)) {
            // Akkor jöjjön a következő hullám
            int temp = wave++;
            while (temp != 0) {
                if(rand()%2 == 0)
                    spawn_character(chars, temp%(chr_num - 4) + 4, enemy, kx + rand()%500*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150 + 1);
                else
                    spawn_character(chars, temp%(chr_num - 4) + 4, enemy, -rand()%500*scale - framesize, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150 + 1);
                temp = temp /  (chr_num - 4);
            }
        }
    }
    break;
    case 1: {
        // Ha van halott karakter
        for (iter = *chars; iter != NULL; iter = iter->next) {
            if(iter->act == dead && iter->side != hero && iter->dead.timer != 0 && iter->dead.timer < GetTicks()) {
                // Akkor az tűnjön el a pályáról
                delete_char(chars, iter);
                iter = *chars;
            }
        }
        // Ha minden ellenség halott
        if(NearestEnemy(*chars, player) == NULL) {
            // Akkor jöjjön a következő hullám
            switch (wave) {
            case 0:
                if(TerrainMax != 0) {
                    tempTerrainMax = TerrainMax;
                    TerrainMax = 0;
                }
                if(player->loc.x >= kx/2 && wavestart == true) {
                    spawn_character(chars, bandit, enemy, kx + rand()%100*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    wavestart = false;

                } else if(wavestart == false) {
                    wave++;
                    TerrainMax = tempTerrainMax/5.0;
                    wavestart = true;
                }
                break;
            case 1:
                if(TerrainRect.x == TerrainMax && wavestart == true) {
                    spawn_character(chars, bandit, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    spawn_character(chars, bandit, enemy, -rand()%120*scale - framesize, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    wavestart = false;
                } else if(wavestart == false) {
                    wave++;
                    TerrainMax = 2*tempTerrainMax/5.0;
                    wavestart = true;
                }
                break;
            case 2:
                if(TerrainRect.x == TerrainMax && wavestart == true) {
                    spawn_character(chars, hunter, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    wavestart = false;
                } else if(wavestart == false) {
                    wave++;
                    TerrainMax = 3*tempTerrainMax/5.0;
                    wavestart = true;
                }
                break;
            case 3:
                if(TerrainRect.x == TerrainMax && wavestart == true) {
                    spawn_character(chars, bandit, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    spawn_character(chars, hunter, enemy, -rand()%120*scale - framesize, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    wavestart = false;
                } else if(wavestart == false) {
                    wave++;
                    TerrainMax = 4*tempTerrainMax/5.0;
                    wavestart = true;
                }
                break;
            case 4:
                if(TerrainRect.x == TerrainMax && wavestart == true) {
                    spawn_character(chars, hunter, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    spawn_character(chars, bandit, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    spawn_character(chars, bandit, enemy, -rand()%120*scale - framesize, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    wavestart = false;
                } else if(wavestart == false) {
                    wave++;
                    TerrainMax = tempTerrainMax;
                    wavestart = true;
                }
                break;
            case 5:
                if(TerrainRect.x == TerrainMax && wavestart == true) {
                    spawn_character(chars, hunter, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    spawn_character(chars, hunter, enemy, -rand()%120*scale - framesize, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    spawn_character(chars, hunter, enemy, -rand()%120*scale - framesize, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    spawn_character(chars, bandit, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    wavestart = false;
                } else if(wavestart == false) {
                    wave++;
                    wavestart = true;
                }
            case 6:
                if(TerrainRect.x == TerrainMax && player->loc.x > kx - 2*framesize && !Is_there_enemy(*chars)) {
                    level++;
                    Loading_screen(screen);
                    Free_background();
                    current_map = prison;
                    Init_bg();
                    *levelup = true;
                }
                break;
            default:
                break;
            }
        }
    }
    break;
    case 2: {
        // Ha van halott karakter
        for (iter = *chars; iter != NULL; iter = iter->next) {
            if(iter->act == dead && iter->side != hero && iter->dead.timer != 0 && iter->dead.timer < GetTicks()) {
                // Akkor az tűnjön el a pályáról
                delete_char(chars, iter);
                iter = *chars;
            }
        }
        // Ha minden ellenség halott
        if(NearestEnemy(*chars, player) == NULL) {
            // Akkor jöjjön a következő hullám
            switch (wave) {
            case 0:
                if(TerrainMax != 0) {
                    tempTerrainMax = TerrainMax;
                    TerrainMax = 0;
                }
                if(player->loc.x >= kx/2 && wavestart == true) {
                    spawn_character(chars, mark, enemy, kx + rand()%100*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    spawn_character(chars, bandit, enemy, kx + rand()%100*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    wavestart = false;

                } else if(wavestart == false) {
                    wave++;
                    TerrainMax = tempTerrainMax/5.0;
                    wavestart = true;
                }
                break;
            case 1:
                if(TerrainRect.x == TerrainMax && wavestart == true) {
                    spawn_character(chars, mark, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*140);
                    spawn_character(chars, bandit, enemy, -rand()%120*scale - framesize, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*140);
                    spawn_character(chars, bandit, enemy, -rand()%120*scale - framesize, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*140);
                    wavestart = false;
                } else if(wavestart == false) {
                    wave++;
                    TerrainMax = 2*tempTerrainMax/5.0;
                    wavestart = true;
                }
                break;
            case 2:
                if(TerrainRect.x == TerrainMax && wavestart == true) {
                    spawn_character(chars, jack, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    spawn_character(chars, bandit, enemy, -rand()%120*scale - framesize, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    wavestart = false;
                } else if(wavestart == false) {
                    wave++;
                    TerrainMax = 3*tempTerrainMax/5.0;
                    wavestart = true;
                }
                break;
            case 3:
                if(TerrainRect.x == TerrainMax && wavestart == true) {
                    spawn_character(chars, mark, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    spawn_character(chars, jack, enemy, -rand()%120*scale - framesize, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*150);
                    wavestart = false;
                } else if(wavestart == false) {
                    wave++;
                    TerrainMax = 4*tempTerrainMax/5.0;
                    wavestart = true;
                }
                break;
            case 4:
                if(TerrainRect.x == TerrainMax && wavestart == true) {
                    spawn_character(chars, mark, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*120);
                    spawn_character(chars, jack, enemy, -rand()%120*scale - framesize, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*120);
                    spawn_character(chars, hunter, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*120);
                    spawn_character(chars, bandit, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*120);
                    wavestart = false;
                } else if(wavestart == false) {
                    wave++;
                    TerrainMax = tempTerrainMax;
                    wavestart = true;
                }
                break;
            case 5:
                if(TerrainRect.x == TerrainMax && wavestart == true) {
                    spawn_character(chars, mark, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*100);
                    spawn_character(chars, jack, enemy, -rand()%120*scale - framesize, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*100);
                    spawn_character(chars, hunter, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*100);
                    spawn_character(chars, hunter, enemy, -rand()%120*scale - framesize, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*100);
                    spawn_character(chars, bandit, enemy, kx + rand()%120*scale, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, (difficulty+2)*100);
                    wavestart = false;
                } else if(wavestart == false) {
                    wave++;
                    wavestart = true;
                }
            case 6:
                if(TerrainRect.x == TerrainMax && player->loc.x > kx - 2*framesize && !Is_there_enemy(*chars)) {
                    level++;
                    Loading_screen(screen);
                    Free_background();
                    current_map = vallenfyre;
                    Init_bg();
                    *levelup = true;
                    aNextTick = dNextTick = GetTicks() + 10000;
                }
                break;
            default:
                break;
            }
        }
    }
    break;
    case 3: {
        // Ha van halott karakter
        for (iter = *chars; iter != NULL; iter = iter->next) {
            if(iter->act == dead && iter->side != hero && iter->dead.timer != 0 && iter->dead.timer < GetTicks()) {
                // Akkor az tűnjön el a pályáról
                delete_char(chars, iter);
                if(iter == *boss)
                    *boss = NULL;
                iter = *chars;
            }
        }
        // Ha Julien halott, és Woody is látszódik, továbbá a karakterünk éppen nem egy szaltó közepén van fejjel lefele
        if(!Is_there_enemy(*chars) && (*Woody)->loc.x > framesize && (*Woody)->loc.x < kx - framesize && player->act == a_walk) {
            // Akkor legyen vége a játéknak
            (*Woody)->walk.vx = 0;
            (*Woody)->walk.vz = 0;
            player->walk.vx = 0;
            player->walk.vz = 0;
            Text_bubble(*Woody, " ", "We did it!", " ", 3000, screen, *chars, player, projs);
            Text_bubble(player, "Never mess ", "with the", "Little Fighters.", 3000, screen, *chars, player, projs);
            Credits_menu(screen);
            *levelup = true;
            *retry = false;
            Free_background();
            practise = top = benchmark = false;
            SDL_Delay(3000);
        }
    }
    break;
    case lvl_Benchmark: {
        // Ha van halott karakter
        for (iter = *chars; iter != NULL; iter = iter->next) {
            if(iter->act == dead && iter->dead.timer != 0 && iter->dead.timer - 2000 < GetTicks()) {
                // Akkor tűnjön el, és szülessen helyette másik, ha kell
                if((*enemies_killed)++ < 60 && iter->side == enemy) {
                    if(rand()%2==0)
                        spawn_character(chars, julian, enemy, rand()%kx, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, 1000);
                    else
                        spawn_character(chars, rand()%(chr_num - 3) + 3, enemy, rand()%kx, kyBottom - (rand()%(kyBottom - kyTop))/2 - (kyBottom - kyTop)/4, 1000);
                }
                delete_char(chars, iter);
                iter = *chars;
            }
        }
        // Ha minden ellenség halott
        if(!Is_there_enemy(*chars) || !Is_there_ally(*chars)) {
            // Nem igazán enemies killed, de ha már itt van egy változó akkor nem hozok létre egy újat az átlag FPS-nek
            *enemies_killed = (Tick - initTick)* 1000 * ((double)kx/base_kx) *((double)ky/base_ky) / (GetTicks() - timer);
            // Kilépéshez kell
            alive = false;
        }
    }
    break;
    default:
        break;
    }

    // Végül visszadja, hogy él-e még a karakter (illetve story mód esetén Woody életben léte is fontos)
    if(benchmark)
        return alive;
    else {
        if(level > 0) // Story mód
            return !(player->act == dead && player->dead.timer != 0 && player->dead.timer < GetTicks()) && !((*Woody)->act == dead && (*Woody)->dead.timer != 0 && (*Woody)->dead.timer < GetTicks());
        else
            return !(player->act == dead && player->dead.timer != 0 && player->dead.timer < GetTicks());
    }
}

/** Kiszámolja hogy a barátoknak vagy az ellenségeknek mennyi az össz hp-ja */
void Total_hp(struct character *chars, enum side side, long *hp, long *maxhp) {
    *hp = *maxhp = 0;
    struct character *iter;
    for(iter=chars; iter!=NULL; iter=iter->next)
        if(iter->side == side) {
            *hp += iter->hp;
            *maxhp = iter->maxhp;
        }
}


#ifndef LEVEL_H
#define LEVEL_H
#include "character.h"
#include "projectile.h"

enum {lvl_Benchmark = -2, lvl_Practise_Arena = -1, lvl_Survival = 0};
extern int level;
extern bool superpower;

void init_level(int Tick, SDL_Surface *screen, struct character **chars, struct character **player, struct character **boss, struct character **Woody, struct projectile **projs);
int spawn_or_kill (int Tick, SDL_Surface *screen, struct character **chars, struct character *player, struct character **Woody, struct character **boss, int *enemies_killed, bool *levelup, bool *retry, struct projectile **projs);
void Total_hp(struct character *chars, enum side side, long *hp, long *maxhp);

#endif //LEVEL_H

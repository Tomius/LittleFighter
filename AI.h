#ifndef AI_H
#define AI_H
#include "character.h"
#include "projectile.h"

enum difficulty {Easy, Normal, Hard};
extern enum difficulty diff;

void AI(int Tick, struct character *chars, struct character *player, struct projectile *projs);
struct character * NearestEnemy(struct character *chars, struct character *who);
int Is_there_enemy(struct character *chars);
int Is_there_ally(struct character *chars);

#endif //AI_H

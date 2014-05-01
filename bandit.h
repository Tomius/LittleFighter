#ifndef BANDIT_H
#define BANDIT_H

#include "character.h"
#include "projectile.h"

void Init_Bandit(struct character *c);
void Animate_Bandit(int Tick, struct character *player, struct projectile **projs);


#endif // BANDIT_H

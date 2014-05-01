#ifndef HUNTER_H
#define HUNTER_H

#include "character.h"
#include "projectile.h"

void Init_Hunter(struct character *c);
void Animate_Hunter(int Tick, struct character *player, struct projectile **projs);

#endif // HUNTER_H

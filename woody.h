#ifndef WOODY_H
#define WOODY_H

#include "character.h"
#include "projectile.h"

void Init_Woody(struct character *c);
void Animate_Woody(int Tick, struct character *player, struct projectile **projs);

#endif // Woody_H

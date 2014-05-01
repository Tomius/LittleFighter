#ifndef FLARE_H
#define FLARE_H

#include "character.h"
#include "projectile.h"

void Init_Flare(struct character *c);
void Animate_Flare(int Tick, struct character *player, struct projectile **projs);

#endif // FLARE_H


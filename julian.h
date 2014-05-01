#ifndef JULIAN_H
#define JULIAN_H

#include "character.h"
#include "projectile.h"

void Init_Julian(struct character *c);
void Animate_Julian(int Tick, struct character *player, struct projectile **projs);


#endif // JULIAN_H


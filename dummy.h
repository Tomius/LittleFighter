#ifndef DUMMY_H
#define DUMMY_H

#include "character.h"
#include "projectile.h"

void Init_Dummy(struct character *c);
void Animate_Dummy(int Tick, struct character *player, struct projectile **projs);


#endif // DUMMY_H

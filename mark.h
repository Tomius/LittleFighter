#ifndef MARK_H
#define MARK_H

#include "character.h"
#include "projectile.h"

void Init_Mark(struct character *c);
void Animate_Mark(int Tick, struct character *player, struct projectile **projs);


#endif // MARK_H


#ifndef JACK_H
#define JACK_H

#include "character.h"
#include "projectile.h"

void Init_Jack(struct character *c);
void Animate_Jack(int Tick, struct character *player, struct projectile **projs);


#endif // JACK_H



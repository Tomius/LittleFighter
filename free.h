#ifndef FREE_H
#define FREE_H

#include "character.h"

void spawn_kill( struct character *player );
void kill_character(struct character *player);
void killall(struct character *players);
void delete_char(struct character **chars, struct character *todelete);
void reset (struct character *c, int x, int y);

#endif //FREE_H

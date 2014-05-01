#ifndef CONTROLS_H
#define CONTROLS_H

#include "character.h"

enum keys {w, a , s, d, e, left_shift, space, left_click, right_click, alt, tab, keys_num}; // Van egy sima shift enum is

void Key_Presses(bool keydown[keys_num], bool justpressed[keys_num], bool justreleased[keys_num], int *mouse_x, int *mouse_y, bool *alive, bool *retry, bool *quit);
void Change_States(bool *keydown, bool *justpressed, bool *justreleased, int mouse_x, int mouse_y, struct character *players);

#endif //CONTROLS_H

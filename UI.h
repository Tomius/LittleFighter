#ifndef UI_H
#define UI_H

#include "character.h"

void Hp_bar_init (void);
void Init_skill_icons(void);
void Init_cursor(void);

extern SDL_Surface *cursor;

void Draw_cursor_or_menu(SDL_Surface *screen, int enemies_killed, int *mouse_x, int *mouse_y, bool *alive, bool *retry, bool *quit, bool levelup);
void Draw_cursor(SDL_Surface *screen, int mouse_x, int mouse_y);
void Draw_UI(SDL_Surface *screen, struct character *chars, struct character *player, struct character *Woody, struct character *boss, int enemies_killed);
void Free_hp_bar (void);
void Free_skill_bar(void);
void Free_cursor(void);
#endif //UI_H

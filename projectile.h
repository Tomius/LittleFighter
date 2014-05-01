#ifndef PROJECTILE_H
#define PROJECTILE_H
#include "character.h"

enum proj_type {normal, normal2, shift, jump_attack_proj, jump_attack2_proj, arrow};

struct projectile {
    struct damage dmg;
    int state, proj_max, exp_max, vanish_max;
    bool exp, vanish;
    int x, y, v;
    double x2, y2, x3, y3, shadowy, vy;
    enum side side;
    enum direction dir;
    enum proj_type type;
    SDL_Surface **l_proj, **r_proj;
    SDL_Surface **l_exp, **r_exp;
    SDL_Surface **l_vanish, **r_vanish;
    SDL_Rect shadowRect;
    SDL_Surface *shadow;
    struct projectile *next;
};

void Draw_projectiles(struct projectile *projs, SDL_Surface *screen);
void Animate_projectiles(int Tick, struct projectile **projectiles);
struct projectile * delete_projectile(struct projectile *projectiles, struct projectile *todelete);
void deleteall(struct projectile *projs);
struct projectile * summon_projectile(struct projectile *projectiles, struct character *player, enum proj_type type);
void Projectile_hits(struct character *players, struct projectile *projs);

#endif // PROJECTILE_H






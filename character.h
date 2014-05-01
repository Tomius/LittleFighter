#ifndef CHAR_H
#define CHAR_H
#include <SDL.h>

enum activity { a_walk, s_walk, jump, jump_attack, jump_attack2, flip, a_melee, s_melee,
                a_ranged, s_ranged, def, counter, hit, air, ground, dead, act_num
              };
enum side {hero, friendly, enemy};
enum direction {left, right};
enum characters {flare, woody, dummy, julian, bandit, hunter, mark, jack, chr_num};
enum flip_type {backward, forward};

typedef enum bool {false = 0,  true = 1} bool;

struct damage {
    int value;
    int *range; // Ezt framenként kell megadni (pl a_melee-ben 13ben 3szor sebez ténylegesen, a többinél 0 a range)
    int *impact; // Framenként, mekkora erőhatással jár az ütés
};


typedef struct character {
    // Általános info a karakterről
    enum characters chrname;
    int hp, maxhp;
    SDL_Rect loc;
    SDL_Rect shadowRect;
    double vx, vy, vz, impulse_x, impulse_z;
    double speed, collision_modifier;
    enum side side;
    enum direction dir;
    enum activity act;
    int state, vunerable;
    bool stunned;
    bool spotted;
    struct damage dmg;

    // Frame számok
    int  a_walk_No, s_walk_No, a_melee_No, jump_No, jump_attack_No, jump_attack2_No,
    jump_attack_proj_No, jump_attack2_proj_No, s_melee_No, a_cast_No, s_cast_No,
    a_proj_No, s_proj_No, a_exp_No, s_exp_No, s_vanish_No, jump_attack_exp_No,
    counter_No, ground_No, air_No, dead_No;

    // Skill cooldownok
    int  s_walk_cd, a_melee_cd, jump_attack_cd, s_melee_cd,
    a_cast_cd, s_cast_cd, counter_cd, def_cd;

    // Cselekvés-alapú információk
    struct walk {
        struct damage dmg;
        int vx, vz;
        int state;
        Uint32 cd;
        SDL_Surface **la, **ra;
        SDL_Surface **lstand, **rstand;
        SDL_Surface **ls, **rs;
    } walk;
    struct jump {
        struct damage dmg1, dmg2;
        int x, y;
        Uint32 cd1, cd2;
        bool second;
        SDL_Surface *la, *ra;
        SDL_Surface **lattack, **rattack;
        SDL_Surface **lattack2, **rattack2;
        SDL_Surface **lproj, **rproj;
        SDL_Surface **lproj2, **rproj2;
        SDL_Surface **lexp, **rexp;
    } jump;
    struct melee {
        struct damage a_dmg, s_dmg;
        int next, x, y;
        Uint32 a_cd, s_cd;
        SDL_Surface **la, **ra;
        SDL_Surface **ls, **rs;
    } melee;
    struct ranged {
        struct damage a_dmg, s_dmg;
        Uint32 a_cd, s_cd;
        SDL_Surface **la_cast, **ra_cast;
        SDL_Surface **la_proj1, **ra_proj1;
        SDL_Surface **la_proj2, **ra_proj2;
        SDL_Surface **la_exp, **ra_exp;
        SDL_Surface **ls_cast, **rs_cast;
        SDL_Surface **ls_proj, **rs_proj;
        SDL_Surface **ls_exp, **rs_exp;
        SDL_Surface **ls_vanish, **rs_vanish;
    } ranged;
    struct def {
        Uint32 cd;
        SDL_Surface *la, *ra;
    } def;
    struct counter {
        struct damage dmg;
        Uint32 cd;
        SDL_Surface **la, **ra;
    } counter;
    struct hit {
        Uint32 cd;
        SDL_Surface *la, *ra;
    } hit;
    struct air {
        SDL_Surface **la, **ra;
    } air;
    struct ground {
        struct damage dmg;
        bool get_up;
        Uint32 cd;
        SDL_Surface **la, **ra;
    } ground;
    struct dead {
        int timer;
        SDL_Surface **la, **ra;
    } dead;
    struct flip {
        enum flip_type type;
        int x, y, angle, angle_inc;
        SDL_Surface *current, *base;
    } flip;
    struct character *next;
} character;


void Init_characters (void);
void Init_Damage(struct character *c);
void spawn_character(struct character **chars, enum characters chrname, int side, int x, int y, int hp);
void spawn_kill_all(void);

#endif // CHAR_H

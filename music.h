#ifndef MUSIC_H
#define MUSIC_H

enum music {mus_benchmark , mus_level1, mus_level2, mus_level3, mus_main, mus_survival, mus_num};
enum chunks {chunk_hit, chunk_hit2, chunk_air, chunk_air2, chunk_dash, chunk_dash2, chunk_exp, chunk_arrow, chunk_teleport, chunk_teleport2, chunk_num};

void Init_Audio();
void playmusic(enum music mus);
void soundeffect(enum chunks current, int x, int y);
void Close_Audio();

#endif

#include <SDL.h>
#include <SDL_mixer.h>
#include "music.h"

Mix_Music *music[mus_num + 1];  // Háttér zenék
Mix_Chunk *chunk[chunk_num];
enum music currently_playing=mus_num;

/** A Zenék betöltése */
void Init_Audio() {
    Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024);
    Mix_Init(MIX_INIT_OGG);
    Mix_VolumeMusic(32);
    Mix_AllocateChannels(128); // Sajna elég sok csatornára van szükség amitől a hang egy picit késik, de még mindig jobb mint ha néha a skilleknek egyáltalán ne lenne hangja
    music[mus_benchmark] = Mix_LoadMUS("./Music/benchmark.ogg");
    music[mus_level1] = Mix_LoadMUS("./Music/level1.ogg");
    music[mus_level2] = Mix_LoadMUS("./Music/level2.ogg");
    music[mus_level3] = Mix_LoadMUS("./Music/level3.ogg");
    music[mus_main] = Mix_LoadMUS("./Music/main.ogg");
    music[mus_survival] = Mix_LoadMUS("./Music/survival.ogg");
    music[mus_num] = NULL;
    chunk[chunk_hit] = Mix_LoadWAV("./Music/hit.wav");
    Mix_VolumeChunk(chunk[chunk_hit], 16);
    chunk[chunk_hit2] = Mix_LoadWAV("./Music/hit2.wav");
    Mix_VolumeChunk(chunk[chunk_hit2], 24);
    chunk[chunk_air] = Mix_LoadWAV("./Music/air.wav");
    Mix_VolumeChunk(chunk[chunk_air], 16);
    chunk[chunk_air2] = Mix_LoadWAV("./Music/air2.wav");
    Mix_VolumeChunk(chunk[chunk_air2], 16);
    chunk[chunk_dash] = Mix_LoadWAV("./Music/dash2.wav");
    Mix_VolumeChunk(chunk[chunk_dash], 20);
    chunk[chunk_dash2] = Mix_LoadWAV("./Music/dash.wav");
    Mix_VolumeChunk(chunk[chunk_dash2], 20);
    chunk[chunk_exp] = Mix_LoadWAV("./Music/exp.wav");
    chunk[chunk_arrow] = Mix_LoadWAV("./Music/arrow.wav");
    chunk[chunk_teleport] = Mix_LoadWAV("./Music/teleport2.wav");
    Mix_VolumeChunk(chunk[chunk_teleport], 25);
    chunk[chunk_teleport2] = Mix_LoadWAV("./Music/teleport.wav");
    Mix_VolumeChunk(chunk[chunk_teleport2], 25);
}

/** Egy háttérzene lejátszása */
void playmusic(enum music mus) {
    // Nem tudom hogyan kéne megoldani hogy a zene újra elinditásánál ne a régi buffert játsza le 2-3 mpig aztán kezdje újra előről,
    // mert az nagyon szarul hangzik.... Ahogy sikerült megoldani az nagyon gyány módszer, de több mint 2 órán át túrtam az SDL_mixer
    // függvényeit, és egy se oldotta meg, ez legalább működik...
    Mix_FreeMusic(music[currently_playing]);
    switch (currently_playing) {
    case mus_benchmark:
        music[mus_benchmark] = Mix_LoadMUS("./Music/benchmark.ogg");
        break;
    case mus_level1:
        music[mus_level1] = Mix_LoadMUS("./Music/level1.ogg");
        break;
    case mus_level2:
        music[mus_level2] = Mix_LoadMUS("./Music/level2.ogg");
        break;
    case mus_level3:
        music[mus_level3] = Mix_LoadMUS("./Music/level3.ogg");
        break;
    case mus_main:
        music[mus_main] = Mix_LoadMUS("./Music/main.ogg");
        break;
    case mus_survival:
        music[mus_survival] = Mix_LoadMUS("./Music/survival.ogg");
        break;
    case mus_num:
        music[mus_num] = NULL;
        break;
    }
    Mix_FadeOutMusic(250);
    Mix_FadeInMusic(music[mus],-1, 250);
    currently_playing = mus;
}

#include <math.h>
#include "main.h"
/**Lejátsza a kért hangot, a képernyő aljának közepétől szemlélt sztereo hatással*/
void soundeffect(enum chunks current, int x, int y) {
    int channel = Mix_PlayChannel(-1,chunk[current],0);
    Sint16 angle = atan2(ky-y, x-kx/2)*180/M_PI;
    Uint8 distance = sqrt(pow(kx-x,2) + pow(ky-y,2)) * 50 / sqrt(pow(kx/2,2) + pow(ky, 2));

    Mix_SetPosition( channel, angle, distance );
}

/** Felszabadítja a zenéket */
void Close_Audio() {
    int i;
    for(i=0; i<mus_num; i++)
        Mix_FreeMusic(music[i]);
    for(i=0; i<chunk_num; i++)
        Mix_FreeChunk(chunk[i]);
    Mix_CloseAudio();
    Mix_Quit();
}



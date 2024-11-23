#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include <SDL.h>
#include <SDL_mixer.h>

void scc(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL_ERROR: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

void *scp(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "SDL_ERROR: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    return ptr;
}

void play_music(char *path)
{
//     char *dropped_file;
//     Mix_Music *music;
//     int frequency = 48000; /* Reasonable default according to SDL_mixer.h */
//     Uint16 format = AUDIO_S16SYS;
//     int channels = 2; /* 1 for mono, 2 for stereo */
//     int chunksize = 2048; /* Reasonable default according to SDL_mixer.h */
// 
//     scc(Mix_OpenAudio(frequency, format, channels, chunksize));
//     music = scp(Mix_LoadMUS(dropped_file));
//     scc(Mix_PlayMusic(music, 0));
// 
//     if (dropped_file) { SDL_free(dropped_file); }
//     if (music) { Mix_FreeMusic(music); }
}

int main(int argc, char **argv)
{
    char *dropped_file;
    Mix_Music *music;
    int frequency = 48000; /* Reasonable default according to SDL_mixer.h */
    Uint16 format = AUDIO_S16SYS;
    int channels = 2; /* 1 for mono, 2 for stereo */
    int chunksize = 2048; /* Reasonable default according to SDL_mixer.h */

    scc(SDL_Init(SDL_INIT_VIDEO));
    scc(Mix_Init(MIX_INIT_MP3 | MIX_INIT_FLAC | MIX_INIT_WAVPACK));

    SDL_Window *window = 
        scp(SDL_CreateWindow("Music Player", 
                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         800, 450, SDL_WINDOW_RESIZABLE));

    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    bool done = false;
    SDL_Event event;
    while (!done) {
        while (!done && SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                done = true;
                break;

            case SDL_DROPFILE:
                dropped_file = event.drop.file;
                if (dropped_file) {
                    scc(Mix_OpenAudio(frequency, format, channels, chunksize));
                    music = scp(Mix_LoadMUS(dropped_file));
                    scc(Mix_PlayMusic(music, 0));

                    const char *now_playing = Mix_GetMusicTitle(music);
                    printf("Now playing... %s\n", now_playing);
                    fflush(stdout);
                } else {
                    fprintf(stderr, "Failed to play music.\n");
                    break;
                };
                break;
            }
        }
    }
    SDL_Delay(0);

    if (dropped_file) { SDL_free(dropped_file); }
    if (music) { Mix_FreeMusic(music); }
    SDL_DestroyWindow(window);
    Mix_Quit();
    SDL_Quit();

    exit(EXIT_SUCCESS);
}



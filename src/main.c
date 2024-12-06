#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include <SDL.h>
#include <SDL_mixer.h>


void check_code(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL_ERROR: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

void *check_ptr(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "SDL_ERROR: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    return ptr;
}

/* Play dropped music, return the music title. */
char *play_music(char *file_path, Mix_Music *music)
{
     int frequency = 48000; /* Reasonable default according to SDL_mixer.h */
     Uint16 format = AUDIO_S16SYS;
     int channels = 2;      /* 1 for mono, 2 for stereo */
     int chunksize = 2048;  /* Reasonable default according to SDL_mixer.h */
 
     check_code(Mix_OpenAudio(frequency, format, channels, chunksize));
     music = check_ptr(Mix_LoadMUS(file_path));
     check_code(Mix_PlayMusic(music, 0));

     return (char *)Mix_GetMusicTitle(music);
}

// \param flag 1 to volume up, 0 to volume down.
void change_music_volume(Mix_Music *music, int volume, int flag)
{
    if (flag == 1) {
        volume = Mix_GetMusicVolume(music);
        volume += 3;
        Mix_VolumeMusic(volume);
    } else if (flag == 0) {
        volume = Mix_GetMusicVolume(music);
        volume -= 3;
        Mix_VolumeMusic(volume);
    } else {
        fprintf(stderr, "bad flag was passed.");
    }
}


int main(int argc, char **argv)
{
    int volume = MIX_MAX_VOLUME;
    char *file_path;
    Mix_Music *music = NULL;

    check_code(SDL_Init(SDL_INIT_EVERYTHING));
    check_code(Mix_Init(MIX_INIT_MP3 | MIX_INIT_FLAC | MIX_INIT_WAVPACK));

    SDL_Window *window = 
        check_ptr(SDL_CreateWindow("Music Player", 
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
                file_path= event.drop.file;
                if (file_path) {
                    char *now_playing = play_music(file_path, music);
                    printf("now playing... %s\n", now_playing);
                    fflush(stdout);
                } else {
                    fprintf(stderr, "Failed to play music.\n");
                    break;
                }
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_SPACE:
                    if (Mix_PausedMusic())
                        Mix_ResumeMusic();
                    else
                        Mix_PauseMusic();
                    break;

                case SDLK_UP:
                    change_music_volume(music, volume, 1);

                    break;

                case SDLK_DOWN:
                    change_music_volume(music, volume, 0);

                    break;
                }

                break;
            }
        }
    }
    SDL_Delay(0);

    // Cleanups
    if (file_path) { SDL_free(file_path); }
    if (music) { Mix_FreeMusic(music); }
    SDL_DestroyWindow(window);
    Mix_Quit();
    SDL_Quit();

    exit(EXIT_SUCCESS);
}



#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#define WINDOW_WIDTH 1000 
#define WINDOW_HEIGHT 450 

#define NOW_PLAYING_MAX 1024


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

/* TTF check code */
void tcc(int code)
{
    if (code < 0) {
        fprintf(stderr, "TTF ERROR: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}

void *tcp(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "TTF ERROR: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    return ptr;
}

/* 
 * \param flag 1 to volume up, 0 to volume down.
 */
void change_music_volume(Mix_Music *music, int volume, int flag)
{
    if (flag == 1) {
        volume = Mix_GetMusicVolume(music);
        volume += 4;
        Mix_VolumeMusic(volume);
    } else if (flag == 0) {
        volume = Mix_GetMusicVolume(music);
        volume -= 4;
        Mix_VolumeMusic(volume);
    } else {
        fprintf(stderr, "bad flag was passed.");
    }
}

/* Play dropped music, return the music title. */
void play_music(char *file_path, Mix_Music *music)
{
    int frequency = 48000; /* Reasonable default according to SDL_mixer.h */
    Uint16 format = AUDIO_S16SYS;
    int channels = 2;      /* 1 for mono, 2 for stereo */
    int chunksize = 2048;  /* Reasonable default according to SDL_mixer.h */

    check_code(Mix_OpenAudio(frequency, format, channels, chunksize));
    music = check_ptr(Mix_LoadMUS(file_path));
    check_code(Mix_PlayMusic(music, 0));

    Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
}


void create_src_rect(SDL_Rect *rect, int width, int height)
{
    rect->x = 0;
    rect->y = 0;
    rect->w = width;
    rect->h = height;
}

void create_dst_rect(SDL_Rect *rect, int width, int height)
{
    rect->x = (WINDOW_WIDTH - width) / 2;
    rect->y = (WINDOW_HEIGHT - height) / 2;
    rect->w = width;
    rect->h = height;
}

void render_text(SDL_Renderer *renderer, 
                 SDL_Texture *texture, 
                 SDL_Rect *src_rect, 
                 SDL_Rect *dst_rect)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, src_rect, dst_rect);
    SDL_RenderPresent(renderer);
}

void cleanups(SDL_Window *window,
              SDL_Renderer *renderer,
              SDL_Texture *texture,
              TTF_Font *font,
              Mix_Music *music)
{
    Mix_FreeMusic(music);
    TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}


int main(int argc, char **argv)
{

    check_code(SDL_Init(SDL_INIT_EVERYTHING));
    check_code(Mix_Init(MIX_INIT_MP3 | MIX_INIT_FLAC | MIX_INIT_WAVPACK));

    SDL_Window *window = 
        check_ptr(SDL_CreateWindow("Music Player", 
                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                  WINDOW_WIDTH, WINDOW_HEIGHT, 
                  SDL_WINDOW_RESIZABLE));

    SDL_Renderer *renderer = 
        check_ptr(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));


    char *init_msg = "Drag & drop a music file";
    tcc(TTF_Init());
    TTF_Font *font =
        tcp(TTF_OpenFont("OpenSans-Regular.ttf", 30));
    SDL_Color string_color = { 255, 255, 255 }; // white

    SDL_Surface* surface = TTF_RenderText_Blended(font, init_msg, string_color);
    SDL_Texture* texture = 
        check_ptr(SDL_CreateTextureFromSurface(renderer, surface));

    SDL_Rect src_rect;
    SDL_Rect dst_rect;
    create_src_rect(&src_rect, surface->w, surface->h);
    create_dst_rect(&dst_rect, surface->w, surface->h);

    SDL_FreeSurface(surface);

    render_text(renderer, texture, &src_rect, &dst_rect);


    Mix_Music *music = NULL;
    char *file_path;
    int volume = MIX_MAX_VOLUME;

    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    bool done = false;
    SDL_Event event;
    while (!done) {
        while (!done && SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                Mix_HaltMusic();
                done = true;
                break;

            case SDL_DROPFILE:
                file_path = event.drop.file;
                if (file_path) {
                    play_music(file_path, music);

                    char *now_playing = check_ptr((char *)malloc(NOW_PLAYING_MAX));
                    snprintf(now_playing, NOW_PLAYING_MAX, "%s%s", 
                             "Now playing... ", basename(file_path));

                    surface = 
                        check_ptr(TTF_RenderUTF8_Solid(font, now_playing, string_color));
                    texture = 
                        check_ptr(SDL_CreateTextureFromSurface(renderer, surface));

                    create_src_rect(&src_rect, surface->w, surface->h);
                    create_dst_rect(&dst_rect, surface->w, surface->h);
                    SDL_FreeSurface(surface);

                    render_text(renderer, texture, &src_rect, &dst_rect);
                    free(now_playing);
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

                case SDLK_q:
                    done = true;
                    break;
                }

                break;
            }
        }
    }
    SDL_Delay(0);

    cleanups(window, renderer, texture, font, music);

    exit(EXIT_SUCCESS);
}

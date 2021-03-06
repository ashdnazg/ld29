#include <stdio.h>
#include <assert.h>
#include <SDL2/SDL.h>

#include "sdl.h"
#include "sdl_audio.h"

#include "core/mem_wrap.h"
#include "core/int_list.h"
#include "core/macros.h"



void exit_on_SDL_sound_error(void *pt) {
    if(pt == NULL) {
        printf("SDL Sound Error: %s\n", SDL_GetError());
        exit(1);
    }
}

sample_playback_t * sample_playback_new(sample_t *sample, int volume, bool loop, void **parent_ptr) {
    sample_playback_t *playback = mem_alloc(sizeof(sample_playback_t));
    link_init(&(playback->played_samples_link));
    playback->pos = 0;
    playback->volume = (volume < 0 || volume > SDL_MIX_MAXVOLUME) ? SDL_MIX_MAXVOLUME : volume;
    playback->sample = sample;
    playback->loop = loop;
    playback->parent_ptr = parent_ptr;
    return playback;
}

void sample_playback_free(sample_playback_t * playback){
    SDL_LockAudio();
    link_remove_from_list(&(playback->played_samples_link));
    SDL_UnlockAudio();
    if (playback->parent_ptr != NULL) {
        *(playback->parent_ptr) = NULL;
    }
    mem_free(playback);
}

void sound_callback(sound_manager_t *s_manager, Uint8 *stream, int len) {
    SDL_memset(stream, 0, len);
    int remaining;
	list_for_each(&(s_manager->played_samples), sample_playback_t *, playback) {
        assert(!(playback->loop && (playback->sample->len < len)));
        remaining = (playback->sample->len - playback->pos);
        SDL_MixAudio(stream, &(playback->sample->data[playback->pos]), MIN(len, remaining), playback->volume);
        playback->pos += len;
        if (playback->pos >= playback->sample->len) {
            if (playback->loop) {
                SDL_MixAudio(&(stream[remaining]), playback->sample->data, playback->pos - playback->sample->len, playback->volume);
                playback->pos %= playback->sample->len;
            } else {
                sample_playback_free(playback);
            }
        }
    }
}

void sound_manager_init(sound_manager_t *s_manager) {
    asset_cache_init(&(s_manager->samples) , MAYBIFY_FUNC(sample_free));
    list_init(&(s_manager->played_samples), sample_playback_t, played_samples_link);
    s_manager->open = FALSE;
    s_manager->spec = mem_alloc(sizeof(SDL_AudioSpec));
}

sound_manager_t * sound_manager_new(void) {
    sound_manager_t *s_manager = mem_alloc(sizeof(sound_manager_t));
    sound_manager_init(s_manager);
    return s_manager;
}
void sound_manager_clean(sound_manager_t *s_manager) {
    if(s_manager->open) {
        SDL_CloseAudio();
    }
    list_for_each(&(s_manager->played_samples), sample_playback_t *, playback) {
        sample_playback_free(playback);
    }
    asset_cache_clean(&(s_manager->samples));
    mem_free(s_manager->spec);
}

void sound_manager_free(sound_manager_t *s_manager) {
    sound_manager_clean(s_manager);
    mem_free(s_manager);
}

sample_playback_t * sound_manager_play_sample(sound_manager_t *s_manager, const char *sample_name, int volume, bool loop, void **parent_ptr) {
    sample_t *sample = sound_manager_get_sample(s_manager, sample_name);
    sample_playback_t *playback = sample_playback_new(sample, volume, loop, parent_ptr);
    if(!(s_manager->open)) {
        memcpy(s_manager->spec, sample->spec, sizeof(SDL_AudioSpec));
        if(SDL_OpenAudio(s_manager->spec, NULL) != 0) {
            exit_on_SDL_sound_error(NULL);
        }
        SDL_PauseAudio(0);
        s_manager->open = TRUE;
        memcpy(s_manager->spec, sample->spec, sizeof(SDL_AudioSpec));
    } else {
        assert(memcmp(s_manager->spec, sample->spec, sizeof(SDL_AudioSpec)) == 0);
    }
    SDL_LockAudio();
    list_insert_tail(&(s_manager->played_samples), playback);
    SDL_UnlockAudio();
    return playback;
}


sample_t * sample_new(Uint8 *data, Uint32 len, SDL_AudioSpec *spec) {
    sample_t *sample = mem_alloc(sizeof(sample_t));
    sample->spec = spec;
    sample->data = data;
    sample->len = len;
    return sample;
}

void sample_free(sample_t * sample) {
    SDL_FreeWAV(sample->data);
    mem_free(sample->spec);
    mem_free(sample);
}


sample_t * sound_manager_get_sample(sound_manager_t *s_manager, const char *name) {
    MAYBE(sample_t *) maybe_sample = asset_cache_get(&(s_manager->samples), name);
    if (UNMAYBE(maybe_sample) != NULL) {
        return (sample_t *) UNMAYBE(maybe_sample);
    }
    return load_sample(s_manager, name);
}

sample_t * load_sample(sound_manager_t *s_manager, const char *name) {
    sample_t *sample;
    Uint8 *data;
    Uint32 len;
    SDL_AudioSpec *spec = mem_alloc(sizeof(SDL_AudioSpec));
    SDL_AudioSpec *ret_spec;
    
    char path_buffer[BUFFER_SIZE];
    sprintf(path_buffer, "%s%s%s", ASSETS_DIR, name, SAMPLE_EXTENSION);
    
    ret_spec = SDL_LoadWAV(path_buffer, spec, &(data), &(len));
    exit_on_SDL_sound_error(ret_spec);
    
    spec->callback = (SDL_AudioCallback) sound_callback;
    spec->userdata = s_manager;
    
    sample = sample_new(data, len, spec); 
    asset_cache_add(&(s_manager->samples), sample, name);
    return sample;
    
}

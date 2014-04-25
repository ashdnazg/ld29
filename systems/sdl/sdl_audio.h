#ifndef __SDL_AUDIO_H__
#define __SDL_AUDIO_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "core/int_list.h"
#include "core/macros.h"
#include "core/asset_cache.h"
#include <SDL2/SDL.h>
#define SAMPLE_EXTENSION ".wav"

typedef struct sample_s {
    SDL_AudioSpec *spec;
    Uint8 *data;
    Uint32 len;
} sample_t;

typedef struct sample_playback_s {
    link_t played_samples_link;
    sample_t *sample;
    Uint32 pos;
    int volume;
    bool loop;
    void **parent_ptr;
} sample_playback_t;

typedef struct sound_manager_s {
    asset_cache_t samples;
    list_t played_samples;
    SDL_AudioSpec *spec;
    bool open;
} sound_manager_t;
sample_t * sound_manager_get_sample(sound_manager_t *s_manager, const char *name);
void sound_manager_init(sound_manager_t *s_manager);
sound_manager_t * sound_manager_new(void);
void sound_manager_clean(sound_manager_t *s_manager);
void sound_manager_free(sound_manager_t *s_manager);
sample_playback_t * sound_manager_play_sample(sound_manager_t *s_manager, const char *sample_name, int volume, bool loop, void **parent_ptr);
sample_t * load_sample(sound_manager_t *s_manager, const char *path);
void sample_free(sample_t * sample);
void sample_playback_free(sample_playback_t * playback);

#ifdef __cplusplus
}
#endif

#endif

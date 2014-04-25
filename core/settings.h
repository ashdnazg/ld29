#ifndef __SETTINGS_H__
#define __SETTINGS_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "macros.h"
#include "asset_cache.h"

#ifndef SETTINGS_FILE
#define SETTINGS_FILE "bugfixes.ini"
#endif


// typedef struct settings_s {
    // asset_cache_t ints;
    // asset_cache_t strings;
// } settings_t;


// void settings_init(settings_t *settings);
// bool settings_load(settings_t *settings);
// void settings_clean(settings_t *settings);


long settings_get_long(const char *key);

MAYBE(char *) settings_get_string(const char *key);


#ifdef __cplusplus
}
#endif

#endif

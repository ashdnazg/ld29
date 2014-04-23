#include "macros.h"
#include "asset_cache.h"
#include "settings.h"
#include "mem_wrap.h"
#include "minIni.h"

// void settings_init(settings_t *settings) {
    // asset_cache_init(&(settings.ints), MAYBIFY(NULL));
    // asset_cache_init(&(settings.strings), MAYBIFY(mem_free));
// }

// bool settings_load(settings_t *settings) {
    // return TRUE;
// }

// void settings_clean(settings_t *settings) {
    // asset_cache_clean(&(settings.ints), MAYBIFY(NULL));
    // asset_cache_clean(&(settings.strings), MAYBIFY(mem_free));
// }


long settings_get_long(const char *key) {
    return ini_getl(NULL, key, -1, SETTINGS_FILE);
}

MAYBE(char *) settings_get_string(const char *key) {
    int len = 0;
    char buffer[BUFFER_SIZE + 1];
    len = ini_gets(NULL, key, "", buffer, BUFFER_SIZE, SETTINGS_FILE);
    if (len == 0) {
        return MAYBIFY(NULL);
    } else {
        buffer[len] = '\0';
        return MAYBIFY(mem_strdup(buffer));
    }
}

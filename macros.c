#include "macros.h"

inline __maybe _MAYBIFY(void *var) {
    __maybe ret = {var};
    return ret;
}

inline __maybe_func _MAYBIFY_FUNC(__func_ptr var) {
    __maybe_func ret = {var};
    return ret;
}
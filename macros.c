#include "macros.h"

inline __maybe _MAYBIFY(void *var) {
    __maybe ret = {var};
    return ret;
}

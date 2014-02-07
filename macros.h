#ifndef __MACROS_H__
#define __MACROS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef int bool;
#define FALSE 0
#define TRUE 1

#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define CONCAT(a,b) a ## b
#define CONCAT3(a,b,c) a ## b ## c
#define STR(a) #a

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __STRLINE__ TOSTRING(__LINE__)

#define _UNIQUE(var, line) CONCAT3(_, var, line)
#define UNIQUE(var) _UNIQUE(var, __LINE__)

#ifndef NO_MAYBE

typedef struct {
    void *ptr;
} ___maybe;
typedef ___maybe __maybe;
#define MAYBE(type_name) __maybe
#define UNMAYBE(var) (var.ptr)
#define MAYBIFY(var) _MAYBIFY((void *) var)

inline __maybe _MAYBIFY(void *var);

#else

#define MAYBE(type_name) type_name
#define UNMAYBE(var) (var)
#define MAYBIFY(var) (var)

#endif

#ifdef __cplusplus
}
#endif

#endif

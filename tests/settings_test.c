#include "core/settings.h"
#include "core/mem_wrap.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    char buff[255];
    getcwd(buff, sizeof (buff));
    printf("path=%s\n", buff);
    long a = settings_get_long("A");
    printf("a: %ld\n", a);
    MAYBE(char *) b = settings_get_string("B");
    if (UNMAYBE(b) != NULL) {
        printf("b: %s\n", (char *) UNMAYBE(b));
        mem_free(UNMAYBE(b));
    }
    mem_wrap_print_mallocs();
    return 0;
}

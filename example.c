#include <stdio.h>
#include <stdlib.h>

#include "sarge.h"

int main(int argc, char **argv) {
    int verbosity = 0;
    const char *name = NULL;

    sarge_t sarge = {0};
    sarge.name = "example";
    sarge.options = (sarge_option_t []){
        SARGE_GROUP("Options"),
        SARGE_FLAG('v', "verbose", &verbosity, "increase verbosity"),
        SARGE_WARG('n', "name", &name, "set name"),
        SARGE_END,
    };

    char **remaining_args;
    if ((remaining_args = sarge_parse(sarge, argc, argv)) == NULL) {
        fprintf(stderr, "error: %s\n", sarge_get_error_text());
        sarge_print_help(stderr, sarge);
        return 1;
    }

    for (int i = 0; remaining_args[i] != NULL; i++) {
        printf("%s\n", remaining_args[i]);
    }

    printf("verbosity: %d\n", verbosity);
    printf("name: %s\n", name);

    free(remaining_args);
}

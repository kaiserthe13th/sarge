#ifndef SARGE_H
#define SARGE_H
#include <stdio.h>

typedef struct sarge_option_flag sarge_option_flag_t;
typedef struct sarge_option_with_argument sarge_option_with_argument_t;
typedef struct sarge_option_group sarge_option_group_t;

/// A flag which increments its value every time it's encountered
struct sarge_option_flag {
    const char short_name;
    const char *long_name;
    int *value;
    const char *help;
};

/// An option which sets its value to the next argument or whatever
/// comes after the equals sign (=) every time it's encountered
struct sarge_option_with_argument {
    const char short_name;
    const char *long_name;
    char **value;
    const char *help;
};

/// A configurator for a group of options
///
/// Any option coming after it until the next option group will be counted as part of it
struct sarge_option_group {
    const char *name;
};

typedef enum sarge_option_kind {
    sarge_option_kind_end = 0,
    sarge_option_kind_flag,
    sarge_option_kind_with_argument,
    sarge_option_kind_group,
} sarge_option_kind_t;

union sarge_option_value {
    sarge_option_flag_t flag;
    sarge_option_with_argument_t with_argument;
    sarge_option_group_t group;
};

typedef struct sarge_option {
    sarge_option_kind_t kind;
    union sarge_option_value value;
} sarge_option_t;

#define SARGE_FLAG(_short_name, _long_name, _value, _help) (sarge_option_t){sarge_option_kind_flag, {.flag = {\
    .short_name = _short_name,\
    .long_name = _long_name,\
    .value = _value,\
    .help = _help,\
}}}

#define SARGE_WARG(_short_name, _long_name, _value, _help) (sarge_option_t){sarge_option_kind_with_argument, {.with_argument = {\
    .short_name = _short_name,\
    .long_name = _long_name,\
    .value = _value,\
    .help = _help,\
}}}

#define SARGE_GROUP(_name) (sarge_option_t){sarge_option_kind_group, {.group = {\
    .name = _name,\
}}}

#define SARGE_END (sarge_option_t){0}

typedef struct sarge {
    const char *name;
    sarge_option_t *options;
    size_t argc;
} sarge_t;

/// Parse command line arguments, according to the given sarge config
char **sarge_parse(sarge_t sarge, int argc, char **argv);
/// Print the help message for the program, according to the given sarge config
void sarge_print_help(FILE *file, sarge_t sarge);
/// Get error if any
int sarge_get_error();
/// Get a textual representation of the error if any exists, otherwise NULL
const char *sarge_get_error_text();

/// The kinds of errors that can occur in sarge_parse
enum sarge_error {
    sarge_error_none = 0,
    sarge_error_overflow_argc,
    sarge_error_unknown_option,
};

#define SARGE_MAX_ERROR_MESSAGE_LENGTH 1024

#endif //SARGE_H

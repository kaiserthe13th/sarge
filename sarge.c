#include "sarge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum sarge_error sarge_error_no = sarge_error_none;
char sarge_error_msg[SARGE_MAX_ERROR_MESSAGE_LENGTH] = {0};

char **sarge_parse(sarge_t sarge, int argc, char **argv) {
    // If active, every remaining argument should be copied over to remaining_args
    int no_parse_arguments = 0;
    // The current index of remaining_args
    int arg_index = 0;
    char **remaining_args = calloc(sarge.argc + 1, sizeof(char *));
    // Take arguments form argv, one at a time, starting from index 1, because the index 0 is the program name
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        // Copy argument to remaining_args and continue
        if (no_parse_arguments) {
            if (arg_index < sarge.argc) {
                remaining_args[arg_index++] = arg;
                continue;
            }
            sarge_error_no = sarge_error_overflow_argc;
            sprintf_s(sarge_error_msg, SARGE_MAX_ERROR_MESSAGE_LENGTH, "more arguments than can be taken have been provided");
            free(remaining_args);
            return NULL;
        }
        // If arg is an option
        if (arg[0] == '-') {
            // If arg is "-"
            // Copy argument to remaining_args and continue
            if (arg[1] == '\0') {
                if (arg_index < sarge.argc) {
                    remaining_args[arg_index++] = arg;
                    continue;
                }

                sarge_error_no = sarge_error_overflow_argc;
                sprintf_s(sarge_error_msg, SARGE_MAX_ERROR_MESSAGE_LENGTH, "more arguments than can be taken have been provided");
                free(remaining_args);
                return NULL;
            }
            // Is a long option
            if (arg[1] == '-') {
                // If arg is "--"
                // Enable no_parse_arguments
                //
                // Because without no_parse_arguments, if the user wanted to create an argument
                // that starts with a dash (-) then they wouldn't be able to
                if (arg[2] == '\0') {
                    no_parse_arguments = 1;
                    continue;
                }
                // Go through each known option
                for (int j = 0; sarge.options[j].kind != sarge_option_kind_end; j++) {
                    switch (sarge.options[j].kind) {
                    case sarge_option_kind_flag:
                        // If it's a flag then check if it's entirely equal
                        if (sarge.options[j].value.flag.long_name != NULL && strcmp(sarge.options[j].value.flag.long_name, &arg[2]) == 0) {
                            (*sarge.options[j].value.flag.value)++;
                            goto next_argument;
                        }
                        break;
                    case sarge_option_kind_with_argument:
                        // If it's an option then check if arg starts with it
                        if (sarge.options[j].value.with_argument.long_name == NULL) continue;
                        size_t arg_length = strlen(sarge.options[j].value.with_argument.long_name);
                        if (strncmp(sarge.options[j].value.with_argument.long_name, &arg[2], arg_length) == 0) {
                            // If it ends here, that means the argument for the option is the next argument
                            if (arg[arg_length + 2] == '\0') {
                                *sarge.options[j].value.with_argument.value = argv[++i];
                                goto next_argument;
                            }
                            // If it doesn't end here and the next character is '=', that means
                            // the argument for the option is whatever is after the '='
                            if (arg[arg_length + 2] == '=') {
                                *sarge.options[j].value.with_argument.value = &arg[arg_length + 3];
                                goto next_argument;
                            }
                        }
                        break;
                    default:
                        // If there is something else ignore it, even if it's invalid,
                        // because maybe someone forked this and worked on it,
                        // and the user of the library downloaded the wrong library somehow
                        break;
                    }
                }

                // We found no option that matched
                sarge_error_no = sarge_error_unknown_option;
                sprintf_s(sarge_error_msg, SARGE_MAX_ERROR_MESSAGE_LENGTH, "unknown option: %s", arg);
                free(remaining_args);
                return NULL;
            }
            // For every character of the short option, except the initial '-'
            for (int k = 1; arg[k] != '\0'; k++) {
                // Go through each known option
                for (int j = 0; sarge.options[j].kind != sarge_option_kind_end; j++) {
                    switch (sarge.options[j].kind) {
                    case sarge_option_kind_flag:
                        // If it's a flag then check if the character is equal
                        if (sarge.options[j].value.flag.short_name != 0 && sarge.options[j].value.flag.short_name == arg[k]) {
                            (*sarge.options[j].value.flag.value)++;
                            goto next_short;
                        }
                        break;
                    case sarge_option_kind_with_argument:
                        // If it's an option with an argument then check if the character is equal
                        if (sarge.options[j].value.with_argument.short_name != 0 && sarge.options[j].value.with_argument.short_name == arg[k]) {
                           // If the arg ends here, then the value is the next argument
                            if (arg[k + 1] == '\0') {
                                *sarge.options[j].value.with_argument.value = argv[++i];
                                goto next_argument;
                            }
                           // Otherwise, the value is whatever comes after
                            *sarge.options[j].value.with_argument.value = &arg[k + 1];
                            goto next_argument;
                        }
                        break;
                    default:
                        // If there is something else ignore it, even if it's invalid,
                        // because maybe someone forked this and worked on it,
                        // and the user of the library downloaded the wrong library somehow
                        break;
                    }
                }

                // We found no option that matched
                sarge_error_no = sarge_error_unknown_option;
                sprintf_s(sarge_error_msg, SARGE_MAX_ERROR_MESSAGE_LENGTH, "unknown option: -%c", arg[k]);
                free(remaining_args);
                return NULL;
            next_short:
            }
            continue;
        }
        if (arg_index < sarge.argc) {
            remaining_args[arg_index++] = arg;
            continue;
        }

        sarge_error_no = sarge_error_overflow_argc;
        sprintf_s(sarge_error_msg, SARGE_MAX_ERROR_MESSAGE_LENGTH, "more arguments than can be taken have been provided");
        free(remaining_args);
        return NULL;
    next_argument:
    }

    return remaining_args;
}

void sarge_print_help(FILE *file, sarge_t sarge) {
    fprintf(file, "%s [options] [arguments]\n", sarge.name);
    fprintf(file, "\n");
    // Go through the options
    for (int i = 0; sarge.options[i].kind != sarge_option_kind_end; i++) {
        char short_name;
        const char *long_name;
        const char *help;

        switch (sarge.options[i].kind) {
        case sarge_option_kind_flag:
            fprintf(file, "    ");
            short_name = sarge.options[i].value.flag.short_name;
            long_name = sarge.options[i].value.flag.long_name;
            help = sarge.options[i].value.flag.help;
            if (short_name != '\0')
                fprintf(file, "-%c ", short_name);
            else
                fprintf(file, "   ");
            if (long_name != NULL)
                fprintf(file, "--%s", long_name);
            else
                fprintf(file, "    ");
            if (help != NULL)
                fprintf(file, "\t%s", help);
            fprintf(file, "\n");
            break;
        case sarge_option_kind_with_argument:
            fprintf(file, "    ");
            short_name = sarge.options[i].value.with_argument.short_name;
            long_name = sarge.options[i].value.with_argument.long_name;
            help = sarge.options[i].value.with_argument.help;
            if (short_name != '\0')
                fprintf(file, "-%c ", short_name);
            else
                fprintf(file, "   ");
            if (long_name != NULL)
                fprintf(file, "--%s", long_name);
            else
                fprintf(file, "    ");
            if (help != NULL)
                fprintf(file, "\t%s", help);
            fprintf(file, "\n");
            break;
        case sarge_option_kind_group:
            fprintf(file, "%s:\n", sarge.options[i].value.group.name);
            break;
        default: break;
        }
    }
}

int sarge_get_error() {
    return sarge_error_no;
}

const char *sarge_get_error_text() {
    return sarge_error_msg;
}

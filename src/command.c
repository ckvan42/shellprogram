
#include "../include/command.h"
#include <stdlib.h>
#include <stdio.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>

static void find_regex(const struct dc_posix_env *env, struct dc_error *err,
                       struct state **statePt, struct command **commandPt);

void parse_command(const struct dc_posix_env *env, struct dc_error *err,
                   struct state *state, struct command *command)
{
    //if any out-of memeory errors occur
    // states->fatal_error = true;
    // "./a.out < in.txt >> out.txt 2>>err.txt"

    regmatch_t matches;
    char *command_string;
    int matched;    //0 if matched, REG_NOMATCH, or others for error.
    char* redirection;
    regex_t reg;
//    regcomp(&reg, "[ \t\f\v]2>[>]?.*", REG_EXTENDED);

    reg = *(state->err_redirect_regex);
    command_string = command->line;
    matched = regexec(&reg, command_string, 1 , &matches, REG_EXTENDED);
    if (matched == 0)
    {
        char *str;
        regoff_t length;

        length = matches.rm_eo - matches.rm_so;
        str = dc_malloc(env, err, (size_t) (length + 1));
        if (dc_error_has_error(err))
        {
            state->fatal_error = true;
        }
        dc_strncpy(env, str, &command_string[matches.rm_so], (size_t) length);
        str[length] = '\0';

        //at this point, the str is 2>>err.txt\0
        //if >> exists overwrite true;
        if ((redirection = dc_strstr(env, str, ">>")) != NULL)
        {
            command->stderr_overwrite = true;
            str += 4;           //moving it by 4 to offset 2>>space
        }
        else
        {
            //then there is only 2>
            str += 3;            //mvoing it by 3 to offset 2>space
        }
        command->stderr_file = dc_strdup(env, err, str);
        if (dc_error_has_error(err))
        {
            state->fatal_error = true;
        }

    }

    matched = regexec(state->out_redirect_regex, command_string, 1 , &matches, 0);
    if (matched == 0)
    {
        command->stderr_overwrite = true;

        char *str;
        regoff_t length;

        length = matches.rm_eo - matches.rm_so;
        str = dc_malloc(env, err, (size_t) (length + 1));
        if (dc_error_has_error(err))
        {
            state->fatal_error = true;
        }
        dc_strncpy(env, str, &command_string[matches.rm_so], (size_t) length);
        str[length] = '\0';
    }

}

static void find_regex(const struct dc_posix_env *env, struct dc_error *err,
                       struct state **statePt, struct command **commandPt)
{
    struct state* state;
    struct command* command;

    state = (struct state*)*statePt;
    command = (struct command*)*commandPt;

}

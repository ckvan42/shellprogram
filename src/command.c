
#include "../include/command.h"
#include <stdlib.h>
#include <stdio.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>
#include <dc_util/strings.h>
#include <dc_posix/dc_wordexp.h>


static void free_loops(const struct dc_posix_env *env, size_t *argc, char*** argv);
static void free_char(const struct dc_posix_env *env, char **target);
static void find_reg_match(const struct dc_posix_env *env, struct dc_error *err, struct state* state, regex_t * reg, char** fileNamePt,
                           regmatch_t *matches, char **command_stringPt, bool *overwritePt);

void parse_command(const struct dc_posix_env *env, struct dc_error *err,
                   struct state *state, struct command *command)
{
    //if any out-of memeory errors occur
    // states->fatal_error = true;
    // "./a.out < in.txt >> out.txt 2>>err.txt"

    regmatch_t matches;
    char *command_string;
    wordexp_t exp;

    command_string = dc_strdup(env, err, command->line);
    if (dc_error_has_error(err))
    {
        state->fatal_error = true;
    }
    find_reg_match(env, err, state, state->err_redirect_regex, &command->stderr_file, &matches, &command_string, &command->stderr_overwrite);
    find_reg_match(env, err, state, state->out_redirect_regex, &command->stdout_file, &matches, &command_string, &command->stdout_overwrite);
    find_reg_match(env, err, state, state->in_redirect_regex, &command->stdin_file, &matches, &command_string, NULL);
    dc_wordexp(env, err, command_string, &exp, 0);
    if (dc_error_has_error(err))
    {
        state->fatal_error = true;
    }
    command->argc = exp.we_wordc;

    command->argv = dc_calloc(env, err, exp.we_wordc + 2, sizeof (char *));
    if (dc_error_has_error(err))
    {
        state->fatal_error = true;
    }
    for (size_t i = 1; i < exp.we_wordc; i++)
    {
        command->argv[i] = dc_strdup(env, err, exp.we_wordv[i]);
    }
    command->argv[exp.we_wordc] = NULL;
    command->command = dc_strdup(env, err, exp.we_wordv[0]);
    if (dc_error_has_error(err))
    {
        state->fatal_error = true;
    }
}


static void find_reg_match(const struct dc_posix_env *env, struct dc_error *err, struct state* state, regex_t * reg, char** fileNamePt,
                           regmatch_t *matches, char **command_stringPt, bool *overwritePt)
{
    char *redirection;
    int matched;
    char *command_string;

    command_string = (char *)*command_stringPt;

    matched = regexec(reg, command_string, 1 , matches, REG_EXTENDED);
    if (matched == 0)
    {
        char *str;
        regoff_t length;
        char *temp;
        wordexp_t exp;

        length = (*matches).rm_eo - (*matches).rm_so;
        str = dc_malloc(env, err, (size_t) (length + 1));
        if (dc_error_has_error(err))
        {
            state->fatal_error = true;
        }
        dc_strncpy(env, str, &command_string[(*matches).rm_so], (size_t) length);
        command_string[(*matches).rm_so] = '\0';
        str[length] = '\0';

//        command_string[(*matches).rm_so] = '0';
        //at this point, the str is 2>>err.txt\0
        //if >> exists overwrite true;
//        dc_str_trim(env, str);
        if ((redirection = dc_strstr(env, str, ">>")) != NULL)
        {
            if (overwritePt)
            {
                *overwritePt = true;
                temp = dc_strdup(env, err, &redirection[2]);
                if (dc_error_has_error(err))
                {
                    state->fatal_error = true;
                }
            }
        }
        else if ((redirection = dc_strstr(env, str, ">")) != NULL)
        {
            //then there is only 2>
            temp = dc_strdup(env, err, &redirection[1]);
            if (dc_error_has_error(err))
            {
                state->fatal_error = true;
            }
        }
        else if ((redirection = dc_strstr(env, str, "<")) != NULL)
        {
            temp = dc_strdup(env, err, &redirection[1]);
            if (dc_error_has_error(err))
            {
                state->fatal_error = true;
            }
        }
        else
        {
            temp = dc_strdup(env, err, str);
            if (dc_error_has_error(err))
            {
                state->fatal_error = true;
            }
        }
        dc_str_trim(env, temp);
        dc_wordexp(env, err, temp, &exp, 0);
        *fileNamePt = dc_strdup(env, err, exp.we_wordv[0]);
        if (dc_error_has_error(err)) {
            state->fatal_error = true;
        }
        free(temp);
        free(str);
        dc_wordfree(env, &exp);
    }
}


/**
 * Destroys command structure values and memory.
 *
 * @param env
 * @param command
 */
void destroy_command(const struct dc_posix_env *env, struct command *command)
{

    free_char(env, &command->line);
    free_char(env, &command->command);
    free_loops(env, &command->argc, &command->argv);
    dc_free(env, command->argv, command->argc * sizeof(char*) + 1);
    command->argv = NULL;
    free_char(env, &command->stdin_file);
    free_char(env, &command->stdout_file);
    command->stdout_overwrite = false;
    free_char(env, &command->stderr_file);
    command->stderr_overwrite = false;
    command->exit_code = 0;
}

static void free_loops(const struct dc_posix_env *env, size_t *argc, char*** argv)
{
    char **argPt = (char **)*argv;

    for (size_t i = 0; i < *argc; i++)
    {
        free_char(env, &argPt[i]);
    }
    *argc = 0;
}

static void free_char(const struct dc_posix_env *env, char **target)
{
    char *targetPt = (char *)*target;

    if (targetPt)
    {
        dc_free(env, targetPt, dc_strlen(env, targetPt));
        *target = NULL;
    }
}

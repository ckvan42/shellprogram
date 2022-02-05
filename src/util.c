//
// Created by Giwoun Bae on 2022-01-18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_wordexp.h>
#include <wordexp.h>
#include "../include/state.h"
#include "../include/util.h"
#include "../include/command.h"

/**
 * Free char pointers if not NULL.
 * @param env the posix environment.
 * @param err the error object.
 * @param target the char pointer to be freed.
 */
static void free_char(const struct dc_posix_env *env, const struct dc_error *err, char ** target);

/**
 * Get the prompt to use.
 *
 * @param env the posix environment.
 * @param err the error object
 * @return PS1 env var or "$" if PS1 not set.
 */
char *get_prompt(const struct dc_posix_env *env, struct dc_error *err)
{
    const char* name = "PS1";
    char* value;
    char* prompt;


    value = dc_getenv(env, name);
    if (value)
    {
        prompt = dc_strdup(env, err, value);
    }
    else
    {
        prompt = dc_strdup(env, err, "$ ");
    }

    return prompt;
}

/**
 * Get the PATH env var.
 *
 * @param env the posix environment.
 * @param err the error object
 * @return the PATH env var
 */
char *get_path(const struct dc_posix_env *env, struct dc_error *err)
{
    const char* name = "PATH";
    char* value;
    char* path;
    //this might be in the dc_utils
    //search for it in there.
    value = dc_getenv(env, name);
    if (value == NULL)
    {
        path = NULL;
    }
    else
    {
        path = dc_strdup(env, err, value);
    }
    return path;
}

/**
 * Separate a path (eg. PATH env var) into separate directories.
 * Directories are separated with a ':' character.
 * Any directories with ~ are converted to the users home directory.
 *
 * @param env the posix environment.
 * @param err the error object.
 * @param path_str the string to separate.
 * @return The directories that make up the path.
 */
char **parse_path(const struct dc_posix_env *env, struct dc_error *err,
                  const char *path_str)
{
    char *temp; //the original string gets destroyed. So we need the temp copy.
    char *token; //each token
    char **list; //list
    size_t index;
    char *tempFr;
    char *tempNumFr;
    char *numFr;
    wordexp_t exp;

    temp = strdup(path_str);
    tempFr = temp;
    numFr = strdup(path_str);
    tempNumFr = numFr;
    index = 0;

    if (dc_error_has_error(err))
    {
        dc_exit(env, DC_ERROR_ERRNO);
    }

    while(dc_strtok_r(env, numFr, ":", &numFr))
    {
        index++;
    }
    dc_free(env, tempNumFr, dc_strlen(env, path_str) + 1);

    list = dc_calloc(env, err, index + 1, sizeof (char *));
    index = 0;

    while((token = dc_strtok_r(env, temp, ":", &temp)) != NULL)
    {
        dc_wordexp(env, err, token, &exp, 0);
        if (dc_error_has_error(err))
        {
            dc_exit(env, errno);
        }
        list[index] = dc_strdup(env, err, exp.we_wordv[0]);
        index++;
    }
    dc_free(env, tempFr, dc_strlen(env, path_str) + 1);
    list[index] = NULL;

    return list;
}

/**
 * Reset the state for the next read, freeing any dynamically allocated memory.
 *
 * @param env the posix environment.
 * @param err the error object
 */
void do_reset_state(const struct dc_posix_env *env, struct dc_error *err, struct state *state)
{
    state->current_line_length = 0;
    free_char(env, err, &(state->current_line));
    if (state->command)
    {
        destroy_command(env, state->command);
        state->command = NULL;
    }
    state->fatal_error = false;
    dc_error_reset(err);
}

static void free_char(const struct dc_posix_env *env, const struct dc_error *err, char **target)
{
    if (target != NULL)
    {
        dc_free(env, *target, dc_strlen(env, (const char *) target));
        *target = NULL;
    }
}

/**
 * Display the state values to the given stream.
 *
 * @param env the posix environment.
 * @param state the state to display.
 * @param stream the stream to display the state on,
 */
void display_state(const struct dc_posix_env *env, struct dc_error *err, const struct state *state, FILE *stream)
{
    char *str;

    str = state_to_string(env, err, state);
    fprintf(stream, "%s", str);
    dc_free(env, str, dc_strlen(env, str));
}

/**
 * Display the state values to the given stream.
 *
 * @param env the posix environment.
 * @param state the state to display.
 * @param stream the stream to display the state on,
 */
char *state_to_string(const struct dc_posix_env *env,  struct dc_error *err, const struct state *state)
{
    char *str;
    size_t length;

    if (state->current_line != NULL)
    {
        //need to know how many bytes to allocate

        length = dc_strlen(env, "current_line = ");
        length += dc_strlen(env, ", fatal_error = ");

        //need to know how many bytes from the actual size.
        length += dc_strlen(env, state->current_line);
        length += 1; // for fatal error

        //need to dynamically allocate memory.
        str = dc_malloc(env, err, length + 1); // +1 for null byte.
        sprintf(str, "current_line = \"%s\", fatal_error = %s", state->current_line, state->fatal_error? "1": "0");
    }
    else
    {
        //need to know how many bytes to allocate
        length = dc_strlen(env, "current_line = NULL");
        length += dc_strlen(env, ", fatal_error = ");

        //need to know how many bytes from the actual size.
        length += 1; // for fatal error

        //need to dynamically allocate memory.
        str = dc_malloc(env, err, length + 1); // +1 for null byte.
        sprintf(str, "current_line = NULL, fatal_error = %s", state->fatal_error? "1": "0");
    }

     return str;
}

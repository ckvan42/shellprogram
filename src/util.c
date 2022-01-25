//
// Created by Giwoun Bae on 2022-01-18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdlib.h>
#include "../include/state.h"
#include "../include/util.h"


/**
 * Get the prompt to use.
 *
 * @param env the posix environment.
 * @param err the error object
 * @return PS1 env var or "$" if PS1 not set.
 */
const char *get_prompt(const struct dc_posix_env *env, struct dc_error *err)
{

}

#define MAX_BUF_PATH 1024
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
    //this might be in the dc_utils
    //search for it in there.
    value = getenv(name);
    if (!value)
    {
        return NULL; //ATTENTION: what would be the best way to handle the null pointer in this case should I create an error handling function for null pointer??
    }

    return value;
}

static size_t count(const char* str, const char sep);
#define MAX_NUM_TOKEN 10
#define MAX_LEN_TOKEN 50
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
                  char *path_str)
{
    char *temp; //the original string gets destroyed. So we need the temp copy.
    char *token; //each token
    char **list; //list
    size_t maxLen;
    size_t index;

    temp = strdup(path_str);
    maxLen = strlen(temp); // maximum number of tokens.
    maxLen = dc_strlen(env, temp); // maximum number of tokens.
    list = dc_calloc(env, err, maxLen + 1, sizeof (char *));
    index = 0;

    while((token = strtok_r(temp, ":", &temp)) != NULL)
    {
        list[index] = dc_strdup(env, err, token);
        index++;
    }
    list[index] = NULL;

    return list;
}

static size_t count(const char* str, const char sep)
{
    size_t num;
    char* temp;

    num = 0;
    temp = str;

    while(temp)
    {
        if ((*temp) == sep)
        {
            num++;
        }
        temp++;
    }
    printf("%zu\n", num);
    return num;
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

    if (state->current_line)
    {
        dc_free(env, state->current_line, dc_strlen(env, state->current_line));
        state->current_line = NULL;
    }

    if (state->command)
    {
        dc_free(env, state->command, dc_strlen(env, (const char *) state->command));
        state->command = NULL;
    }
    state->fatal_error = false;
    dc_error_reset(err);
}

/**
 * Display the state values to the given stream.
 *
 * @param env the posix environment.
 * @param state the state to display.
 * @param stream the stream to display the state on,
 */
void display_state(const struct dc_posix_env *env, const struct dc_error *err, const struct state *state, FILE *stream)
{
    //for debugging purposes.
    char *str;

    str = state_to_string(env, err, state);
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
//    length += dc_strlen(env, "in_redirect_regex = ");
//    length += dc_strlen(env, "out_redirect_regex = ");
//    length += dc_strlen(env, "err_redirect_regex = ");
//    length += dc_strlen(env, "path = ");
//    length += dc_strlen(env, "prompt = ");
//    length += dc_strlen(env, "max_line_len = ");
        length = dc_strlen(env, "current_line = ");
//    length += dc_strlen(env, ", current_line_length = ");
//    length += dc_strlen(env, ", command = ");
        length += dc_strlen(env, ", fatal_error = ");

        //need to know how many bytes from the actual size.
//    length += dc_strlen(env, "in_redirect_regex = ");
//    length += dc_strlen(env, "out_redirect_regex = ");
//    length += dc_strlen(env, "err_redirect_regex = ");
//    length += dc_strlen(env, "path = ");
//    length += dc_strlen(env, "prompt = ");
//    length += dc_strlen(env, "max_line_len = ");
        length += dc_strlen(env, state->current_line);
        length += 1; // for fatal error
//    length += dc_strlen(env, "command = ");
//    length += dc_strlen(env, "fatal_error  = ");

        //need to dynamically allocate memory.
        str = dc_malloc(env, err, length + 1); // +1 for null byte.
        sprintf(str, "current_line = \"%s\", fatal_error = %s", state->current_line, state->fatal_error? "1": "0");
    }
    else
    {
        //need to know how many bytes to allocate
//    length += dc_strlen(env, "in_redirect_regex = ");
//    length += dc_strlen(env, "out_redirect_regex = ");
//    length += dc_strlen(env, "err_redirect_regex = ");
//    length += dc_strlen(env, "path = ");
//    length += dc_strlen(env, "prompt = ");
//    length += dc_strlen(env, "max_line_len = ");
        length = dc_strlen(env, "current_line = NULL");
//    length += dc_strlen(env, ", current_line_length = ");
//    length += dc_strlen(env, ", command = ");
        length += dc_strlen(env, ", fatal_error = ");

        //need to know how many bytes from the actual size.
//    length += dc_strlen(env, "in_redirect_regex = ");
//    length += dc_strlen(env, "out_redirect_regex = ");
//    length += dc_strlen(env, "err_redirect_regex = ");
//    length += dc_strlen(env, "path = ");
//    length += dc_strlen(env, "prompt = ");
//    length += dc_strlen(env, "max_line_len = ");
        length += 1; // for fatal error
//    length += dc_strlen(env, "command = ");
//    length += dc_strlen(env, "fatal_error  = ");

        //need to dynamically allocate memory.
        str = dc_malloc(env, err, length + 1); // +1 for null byte.
        sprintf(str, "current_line = NULL, fatal_error = %s", state->fatal_error? "1": "0");
    }

     return str;
}

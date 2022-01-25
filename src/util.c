//
// Created by Giwoun Bae on 2022-01-18.
//

#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdlib.h>
#include "../include/state.h"


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
//    int num_tokens = 0;
//    char *tempArray[MAX_NUM_TOKEN];
//    const char* delimiter = ":";
//    char* context;
//    char *token;
//
//    token = dc_strtok_r(env, path_str, delimiter, &context);
//
//    if (!token)
//    {
//        printf("token is null."); //ATTENTION: best way to do it?
//    }
//
//    while (token != NULL)
//    {
//        dc_strcpy(env, tempArray[num_tokens], token);
//        num_tokens++;
//        token = dc_strtok_r(env, NULL, delimiter, &context);
//    }
//
//    return tempArray;
}

/**
 * Reset the state for the next read, freeing any dynamically allocated memory.
 *
 * @param env the posix environment.
 * @param err the error object
 */
void do_reset_state(const struct dc_posix_env *env, struct dc_error *err, struct state *state)
{

}

/**
 * Display the state values to the given stream.
 *
 * @param env the posix environment.
 * @param state the state to display.
 * @param stream the stream to display the state on,
 */
void display_state(const struct dc_posix_env *env, const struct state *state, FILE *stream)
{
    //for debugging purposes.
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

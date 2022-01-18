//
// Created by Giwoun Bae on 2022-01-18.
//

#include "util.h"
#include <stdio.h>


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

/**
 * Get the PATH env var.
 *
 * @param env the posix environment.
 * @param err the error object
 * @return the PATH env var
 */
char *get_path(const struct dc_posix_env *env, struct dc_error *err)
{
    //this might be in the dc_utils
    //search for it in there.
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
                  char *path_str)
{

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
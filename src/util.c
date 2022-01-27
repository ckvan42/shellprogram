//
// Created by Giwoun Bae on 2022-01-18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdlib.h>
#include <wordexp.h>
#include "../include/state.h"
#include "../include/util.h"
#include "../include/command.h"

/**
 * Free command struct when resetting.
 *
 * @param env the posix environment.
 * @param err the error object.
 * @param command
 */
static void free_command(const struct dc_posix_env *env, const struct dc_error *err, struct command **command);

/**
 * Free char pointers if not NULL.
 * @param env the posix environment.
 * @param err the error object.
 * @param target the char pointer to be freed.
 */
static void free_char(const struct dc_posix_env *env, const struct dc_error *err, char ** target);

/**
 * Loops through the array and frees those pointers.
 *
 * @param env the posix environment.
 * @param err the error object.
 * @param argc size of the array.
 * @param argv the array of pointers.
 */
static void free_loop(const struct dc_posix_env *env, const struct dc_error *err, size_t *argc, char*** argv);

/**
 * Get the prompt to use.
 *
 * @param env the posix environment.
 * @param err the error object
 * @return PS1 env var or "$" if PS1 not set.
 */
const char *get_prompt(const struct dc_posix_env *env, struct dc_error *err)
{
    const char* name = "PS1";
    char* value;
    char* prompt;

    value = dc_getenv(env, name);
    if (value)
    {
//        prompt = dc_strdup(env, err, value);
        prompt = value;
    }
    else
    {
//        prompt = dc_strdup(env, err, "$ ");
        prompt = "$ ";
    }

    return prompt;
    //ATTENTION HOW CAN I FREE THIS????
    //COUPLING???
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
    //this might be in the dc_utils
    //search for it in there.
    value = dc_getenv(env, name);

    //ATTENTION:
    //should I handle NULL here?
    //No, the test case also checks for NULL.

    return value;
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
    char *temp; //the original string gets destroyed. So we need the temp copy.
    char *token; //each token
    char **list; //list
    size_t maxLen;
    size_t index;
    char *tempFr;

    temp = strdup(path_str);
    tempFr = temp;
    maxLen = dc_strlen(env, temp); // maximum number of tokens.
    list = dc_calloc(env, err, maxLen + 1, sizeof (char *));
    index = 0;

    while((token = strtok_r(temp, ":", &temp)) != NULL)
    {
        list[index] = dc_strdup(env, err, token);
        index++;
    }
    free(tempFr);
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
        free_command(env, err, &state->command);
        dc_free(env, state->command, sizeof(struct command));
        state->command = NULL;
    }
    state->fatal_error = false;
    dc_error_reset(err);
}

static void free_command(const struct dc_posix_env *env, const struct dc_error *err, struct command **pCommand)
{

    struct command *command;

    command = (struct command *) *pCommand;

    free_char(env, err, &command->line);
    free_char(env, err, &command->command);
    free_loop(env, err, &command->argc, &command->argv);
    free_char(env, err, &command->stdin_file);
    free_char(env, err, &command->stdout_file);
    command->stdout_overwrite = false;
    free_char(env, err, &command->stderr_file);
    command->stderr_overwrite = false;
    command->exit_code = 0;
}

static void free_loop(const struct dc_posix_env *env, const struct dc_error *err, size_t *argc, char*** argv)
{
    for (size_t i = 0; i < *argc; i++)
    {
        free_char(env, err, argv[i]);
    }
    *argc = 0;
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
char *state_to_string(const struct dc_posix_env *env, const struct dc_error *err, const struct state *state)
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

//
// Created by Giwoun Bae on 2022-01-18.
//


#include <dc_posix/dc_stdlib.h>
#include <unistd.h>
#include <dc_posix/dc_string.h>
#include <stdlib.h>
#include "../include/util.h"
#include "../include/input.h"
#include <dc_posix/dc_posix_env.h>
#include <dc_util/filesystem.h>
#include "../include/shell_impl.h"
#include "../include/state.h"

/**
 * Free all the individual paths.
 *
 * @param env
 * @param err
 * @param arg
 */
static void free_paths(const struct dc_posix_env *env, struct dc_error *err,
                       char ***pPath);

#define IN_REDIRECT_REGEX "[ \t\f\v]<.*"
#define OUT_REDIRECT_REGEX "[ \t\f\v][1^2]?>[>]?.*"
#define ERR_REDIRECT_REGEX "[ \t\f\v]2>[>]?.*"

/**
 * Set up the initial state:
 *  - in_redirect_regex  "[ \t\f\v]<.*"
 *  - out_redirect_regex "[ \t\f\v][1^2]?>[>]?.*"
 *  - err_redirect_regex "[ \t\f\v]2>[>]?.*"
 *  - path the PATH env var seaprated into directories
 *  - prompt the PS1 env var or "$" if PS1 not set
 *  - max_line_length the value of _SC_ARG_MAX (see sysconf)
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return READ_COMMANDS or INIT_ERROR
 */
int init_state(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
    struct state *states;
    int return_value_regex;
    //get the PATH environment variables
    char *path;
    char **path_array;

    states = (struct state*) arg;

    states->in_redirect_regex = (regex_t *)dc_calloc(env, err, 1, sizeof(regex_t));
    if (dc_error_has_no_error(err))
    {
        return_value_regex = regcomp(states->in_redirect_regex, IN_REDIRECT_REGEX, 0);
        if (return_value_regex != 0)
        {
            size_t size;
            char *msg;

            size = regerror(return_value_regex, states->in_redirect_regex, NULL, 0);
            msg = dc_malloc(env, err, size + 1);
            fprintf(stderr, msg);
            free(msg);

            printf("FAILED??\n");
            states->fatal_error = true;
            return ERROR;
        }
    }
    states->out_redirect_regex = (regex_t *)dc_calloc(env, err, 1, sizeof(regex_t));
    if (dc_error_has_no_error(err))
    {
        return_value_regex = regcomp(states->out_redirect_regex, OUT_REDIRECT_REGEX, 0);
        if (return_value_regex != 0)
        {
            states->fatal_error = true;
            return ERROR;
        }
    }
    states->err_redirect_regex = (regex_t *)dc_calloc(env, err, 1, sizeof(regex_t));
    if (dc_error_has_no_error(err))
    {
        return_value_regex = regcomp(states->err_redirect_regex, ERR_REDIRECT_REGEX, 0);
        if (return_value_regex != 0)
        {
            states->fatal_error = true;
            return ERROR;
        }
    }

    path = get_path(env, err);
    path_array = parse_path(env, err, path);
    states->path = path_array; //already dynamically allocated.

    //get the PS1 environment variables
    states->prompt = dc_strdup(env, err, get_prompt(env, err));

    //all other variables to zero
    states->fatal_error = false;
    states->max_line_length = (size_t) sysconf(_SC_ARG_MAX);
    states->current_line = NULL;
    states->current_line_length = 0;
    states->command = NULL;

    return READ_COMMANDS;
}

/**
 * Free any dynamically allocated memory in the state.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return DC_FSM_EXIT
 */
int destroy_state(const struct dc_posix_env *env, struct dc_error *err,
                  void *arg)
{
    struct state *states;

    states = (struct state*) arg;

    if (states->in_redirect_regex != NULL)
    {
        regfree(states->in_redirect_regex);
        states->in_redirect_regex = NULL;
    }

    if (states->out_redirect_regex != NULL)
    {
        regfree(states->out_redirect_regex);
        states->out_redirect_regex = NULL;
    }

    if (states->err_redirect_regex != NULL)
    {
        regfree(states->err_redirect_regex);
        states->err_redirect_regex = NULL;
    }

    dc_free(env, states->prompt, dc_strlen(env, states->prompt) + 1);
    states->prompt = NULL;

    free_paths(env, err, &states->path);

    do_reset_state(env, err, states);
    states->max_line_length = 0;

    return DC_FSM_EXIT;
}

/**
 * Free each char * in the array of paths
 *
 * @param env the posix environment
 * @param err the error object
 * @param paths pointer to the first element of the array of path
 */
static void free_paths(const struct dc_posix_env *env, struct dc_error *err,
                        char ***pPath)
{
    char ** paths;
    //it was strduped and last one has null byte
    //each element has char* strduped.
    size_t i;

    i = 0;
    paths = (char **)*pPath;

    while (!paths[i])
    {
        dc_free(env, paths[i], dc_strlen(env, paths[i]) + 1);
        paths[i] = NULL;

        i++;
    }
    dc_free(env, *pPath, sizeof(char**));
    *pPath = NULL;
}

/**
 * Reset the state for the next read (see do_reset_state).
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return READ_COMMANDS
 */
int reset_state(const struct dc_posix_env *env, struct dc_error *err,
                void *arg)
{

    struct state *states;

    states = (struct state*) arg;
    do_reset_state(env, err, states);
    return READ_COMMANDS;
}

/**
 * Prompt the user and read the command line (see read_command_line).
 * Sets the state->current_line and current_line_length.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return SEPARATE_COMMANDS
 */
int read_commands(const struct dc_posix_env *env, struct dc_error *err,
                  void *arg)
{
    struct state* states;
    char *current_working_dir;
    char *current_prompt;
    size_t line_len;
    char * cur_line;

    states = (struct state*) arg;

    current_working_dir = dc_get_working_dir(env, err);
    if (dc_error_has_error(err))
    {
        states->fatal_error = true;
        return ERROR;
    }

    current_prompt = dc_malloc(env, err, 1 + dc_strlen(env, current_working_dir) + 1 + 2 + dc_strlen(env, states->prompt));
    if (dc_error_has_error(err))
    {
        states->fatal_error = true;
        return ERROR;
    }
    sprintf(current_prompt, "[%s] %s", current_working_dir, states->prompt);
    fprintf(states->stdout, "%s", current_prompt);

    //read input from state.stdin in to state.current_line
    line_len = states->max_line_length;
    cur_line = read_command_line(env, err, states->stdin, &line_len);
    if (dc_error_has_error(err))
    {
        states->fatal_error = true;
        return ERROR;
    }

    states->current_line = dc_strdup(env, err, cur_line);
    states->current_line_length = line_len;
    if (dc_error_has_error(err))
    {
        states->fatal_error = true;
        return ERROR;
    }

    if (line_len == 0)
    {
        return RESET_STATE;
    }

    return SEPARATE_COMMANDS;
}

/**
 * Separate the commands. In the current implementation there is only one command.
 * Sets the state->command.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return PARSE_COMMANDS or SEPARATE_ERROR
 */
int separate_commands(const struct dc_posix_env *env, struct dc_error *err,
                      void *arg)
{
    struct state* states;

    states = (struct state*) arg;


    states->command->stdin_file;
    states->command->stdout_file;
    states->command->stderr_file;
    states->command->stdout_overwrite;
    states->command->stderr_overwrite;
    states->command->argc;
    states->command->argv;
    states->command->line;
    states->command->command;
    states->command->exit_code;
}

/**
 * Parse the commands (see parse_command)
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return EXECUTE_COMMANDS or PARSE_ERROR
 */
int parse_commands(const struct dc_posix_env *env, struct dc_error *err,
                   void *arg)
{

}


/**
 * Run the command (see execute).
 * If the command->command is cd run builtin_cd
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return EXIT (if command->command is exit), RESET_STATE or EXECUTE_ERROR
 */
int execute_commands(const struct dc_posix_env *env, struct dc_error *err,
                     void *arg)
{

}


/**
 * Handle the exit command (see do_reset_state)
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return DESTROY_STATE
 */
int do_exit(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{

}

/**
 * Print the err->message to stderr and reset the err (see dc_err_reset).
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return RESET_STATE or DESTROY_STATE (if state->fatal_error is true)
 */
int handle_error(const struct dc_posix_env *env, struct dc_error *err,
                 void *arg)
{
    struct state* states;
    states = (struct state*)arg;

    if (states->current_line == NULL)
    {
    }

    if(states->fatal_error)
    {
        return DESTROY_STATE;
    }
    dc_error_reset(err);
    return RESET_STATE;
}

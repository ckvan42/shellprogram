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
#include <builtins.h>
#include "../include/shell_impl.h"
#include "../include/state.h"

/**
 * Free all the individual paths.
 *
 * @param env the posix environment.
 * @param arg the current struct state
 */
static void free_paths(const struct dc_posix_env *env, char ***pPath);


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
    regex_t regIn;
    regex_t regOut;
    regex_t regErr;

    states = (struct state*) arg;

    return_value_regex = regcomp(&regIn, IN_REDIRECT_REGEX, REG_EXTENDED);
    states->in_redirect_regex = (regex_t *)dc_calloc(env, err, 1, sizeof(regIn));
    dc_memcpy(env, states->in_redirect_regex, &regIn, sizeof(regIn));
    if (return_value_regex != 0)
    {
        states->fatal_error = true;
        return ERROR;
    }

    return_value_regex = regcomp(&regOut, OUT_REDIRECT_REGEX, REG_EXTENDED);
    states->out_redirect_regex = (regex_t *)dc_calloc(env, err, 1, sizeof(regOut));
    dc_memcpy(env, states->out_redirect_regex, &regOut, sizeof(regOut));
    if (return_value_regex != 0)
    {
        states->fatal_error = true;
        return ERROR;
    }

    return_value_regex = regcomp(&regErr, ERR_REDIRECT_REGEX, REG_EXTENDED);
    states->err_redirect_regex = (regex_t *)dc_calloc(env, err, 1, sizeof(regErr));
    dc_memcpy(env, states->err_redirect_regex, &regErr, sizeof(regErr));
    if (return_value_regex != 0)
    {
        states->fatal_error = true;
        return ERROR;
    }

    path = get_path(env, err);
    path_array = parse_path(env, err, path);
    states->path = path_array; //already dynamically allocated.

    //get the PS1 environment variables
    states->prompt = get_prompt(env, err);

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
    size_t regex_size;
    states = (struct state*) arg;

    if (states->in_redirect_regex != NULL)
    {
        regex_size = sizeof(*states->in_redirect_regex);
        regfree(states->in_redirect_regex);
        dc_free(env, states->in_redirect_regex, regex_size);
        states->in_redirect_regex = NULL;
    }

    if (states->out_redirect_regex != NULL)
    {
        regex_size = sizeof(*states->out_redirect_regex);
        regfree(states->out_redirect_regex);
        dc_free(env, states->out_redirect_regex, regex_size);
        states->out_redirect_regex = NULL;
    }

    if (states->err_redirect_regex != NULL)
    {
        regex_size = sizeof(*states->err_redirect_regex);
        regfree(states->err_redirect_regex);
        dc_free(env, states->err_redirect_regex, regex_size);
        states->err_redirect_regex = NULL;
    }

    dc_free(env, states->prompt, dc_strlen(env, states->prompt) + 1);
    states->prompt = NULL;

    free_paths(env, &states->path);

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
static void free_paths(const struct dc_posix_env *env, char ***pPath)
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
    char * current_working_dir;
    char * current_prompt;
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

    states->command = dc_calloc(env, err, 1, sizeof(struct command));

    if (dc_error_has_error(err))
    {
        states->fatal_error = true;
        return ERROR;
    }

    states->command->line  = dc_strdup(env, err, states->current_line);
    states->command->stdin_file = NULL;
    states->command->stdout_file = NULL;
    states->command->stderr_file = NULL;
    states->command->stdout_overwrite = false;
    states->command->stderr_overwrite = false;
    states->command->argc = 0;
    states->command->argv = NULL;
    states->command->command = NULL;
    states->command->exit_code = EXIT_SUCCESS;

    return PARSE_COMMANDS;
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
    struct state* states;

    states = (struct state*) arg;

    parse_command(env, err, states, states->command);
    if (dc_error_has_error(err))
    {
        return ERROR;
    }
    return EXECUTE_COMMANDS;
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
    struct state *states;
    const char * cd_command;
    const char * exit_command;

    states = (struct state *)arg;

    cd_command = "cd";
    exit_command = "exit";

    if (dc_strcmp(env, states->command->command, cd_command) == 0)
    {
        builtin_cd(env, err, states->command, states->stderr);
    }
    else if (dc_strcmp(env, states->command->command, exit_command) == 0)
    {
        return EXIT;
    }
    else
    {
        execute(env, err, states->command, states->path);
        if (dc_error_has_error(err))
        {
            return ERROR;
        }
    }

    fprintf(states->stdout, "%d\n", states->command->exit_code);

    if (states->fatal_error)
    {
        return ERROR;
    }
    return RESET_STATE;
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
    struct state *states;

    states = (struct state *)arg;
    do_reset_state(env, err, states);

    return DESTROY_STATE;
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
        fprintf(states->stderr, "internal error (%d) %s\n", err->errno_code, err->message);
    }
    else
    {
        fprintf(states->stderr, "internal error (%d) %s: \"%s\"\n", err->errno_code, err->message, states->current_line);
    }

    if(states->fatal_error)
    {
        return DESTROY_STATE;
    }
    dc_error_reset(err);
    return RESET_STATE;
}

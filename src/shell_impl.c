//
// Created by Giwoun Bae on 2022-01-18.
//

#include "shell_impl.h"
#include <stdio.h>
#include <dc_posix/dc_stdlib.h>
#include <unistd.h>
#include <stdlib.h>

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

    states = dc_malloc(env, err, sizeof (struct state));

    if (states == NULL)
    {
        return ERROR;
    }

    states->max_line_length = (size_t) sysconf(_SC_ARG_MAX);
    return_value_regex = regcomp(states->in_redirect_regex, IN_REDIRECT_REGEX, 0);
    if (return_value_regex != 0)
    {
        states->fatal_error = true;
        return ERROR;
    }
    return_value_regex = regcomp(states->out_redirect_regex, OUT_REDIRECT_REGEX, 0);
    if (return_value_regex != 0)
    {
        states->fatal_error = true;
        return ERROR;
    }
    return_value_regex = regcomp(states->err_redirect_regex, ERR_REDIRECT_REGEX, 0);
    if (return_value_regex != 0)
    {
        states->fatal_error = true;
        return ERROR;
    }


    /* get the PATH environment */
    const char* name = "HOME";
    char *value;

    value = getenv(name);



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

    if(states->fatal_error)
    {
        return DESTROY_STATE;
    }
    dc_error_reset(err);
    return RESET_STATE;
}

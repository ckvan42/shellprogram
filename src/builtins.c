//
// Created by Giwoun Bae on 2022-01-18.
//

#include "../include/builtins.h"
#include <stdlib.h>
#include <stdio.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_wordexp.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_unistd.h>


#define COMMAND_ERROR_EXIT_CODE 1
#define COMMAND_SUCCESS_EXIT_CODE 0

/**
 * Outputs the error message to stream.
 *
 * @param env the posix environment
 * @param dir directory name.
 * @param errNum error number
 * @param stream output stream.
 */
static void stream_error(const struct dc_posix_env *env, char* dir, int errNum, FILE *stream);

/**
 * Change the working directory.
 * ~ is converted to the users home directory.
 * - no arguments is converted to the users home directory.
 * The command->exit_code is set to 0 on success or error->errno_code on failure.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param command the command information
 * @param errstream the stream to print error messages to
 */
void builtin_cd(const struct dc_posix_env *env, struct dc_error *err,
                struct command *command, FILE *errstream)
{
    char *temp;
    char *path;
    wordexp_t wd_path;

    if(!command->argv[1])
    {
        //set path to "~/"
        temp = dc_strdup(env, err, "~/");
        if(dc_error_has_error(err))
        {
            command->exit_code = COMMAND_ERROR_EXIT_CODE;
        }
    }
    else
    {
        temp = dc_strdup(env, err, command->argv[1]);
    }

    //expand the path to change ~ to the user's home directory
    dc_wordexp(env, err, temp, &wd_path, 0);
    path = dc_strdup(env, err, wd_path.we_wordv[0]);
    dc_free(env, temp, dc_strlen(env, temp) + 1);

    dc_chdir(env, err, path);

    if(dc_error_has_error(err))
    {
        //Print error message where message is:
        stream_error(env, path, errno, errstream);
        command->exit_code = COMMAND_ERROR_EXIT_CODE;
    }
    else
    {
        command->exit_code = COMMAND_SUCCESS_EXIT_CODE;
    }
}
#define ERR_BUF_LEN 1024

static void stream_error(const struct dc_posix_env *env, char* dir, int errNum, FILE *stream)
{
    char message[ERR_BUF_LEN] = {0};

    switch (errNum)
    {
        case ENOENT:
            dc_strcpy(env, message, "does not exist");
            break;
        case ENOTDIR:
            dc_strcpy(env, message, "is not a directory");
            break;
        default:
            dc_strerror_r(env, errNum, message, ERR_BUF_LEN);
            break;
    }
    fprintf(stream, "%s: %s\n", dir, message);
}

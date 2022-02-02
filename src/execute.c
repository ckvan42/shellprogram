//
// Created by Giwoun Bae on 2022-01-18.
//

#include "../include/execute.h"
#include <stdio.h>
#include <dc_posix/dc_stdio.h>
#include <dc_posix/dc_unistd.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>
#include <unistd.h>

static void redirect(const struct dc_posix_env* env, struct dc_error* err, struct command* command);

static int handle_run_error(struct dc_error* err, struct command* command);

static void run(const struct dc_posix_env* env, struct dc_error* err, const struct command* command, char** path);

void execute(const struct dc_posix_env *env, struct dc_error *err,
             struct command *command, char **path)
{
    pid_t pid;
    int status;

    pid = dc_fork(env, err);
    status = command->exit_code;

    if (dc_error_has_no_error(err))
    {
        if (pid == 0)
        {
            redirect(env, err, command);
            if (dc_error_has_error(err))
            {
                dc_exit(env, 126);
            }
            //call run() -> this only returns if there is an error calling execv.
            run(env, err, command, path);
            //am I calling the execv in this function too? I'm guessing not.
            if  (dc_error_has_error(err))
            {
                status = handle_run_error(err, command);
            }
            dc_exit(env, status);
        }
        else
        {
            waitpid(pid, &status, WNOHANG);
            command->exit_code = status;
        }
    }
}


static void redirect(const struct dc_posix_env* env, struct dc_error* err, struct command* command)
{
    FILE *inFP;
    FILE *outFP;
    FILE *errFP;

    if (command->stdin_file != NULL)
    {
        //open the file
        inFP = dc_fopen(env, err, command->stdin_file, "r");
        if (dc_error_has_error(err))
        {
            if (inFP != NULL)
            {
                dc_fclose(env, err, inFP);
            }
            return;
        }
        //call dup2 for the file and stdin
        dc_dup2(env, err, fileno(inFP), STDIN_FILENO);
    }

    if (command->stdout_file != NULL)
    {
        //open the file for truncation or appending.
        if (command->stdout_overwrite)
        {
            //truncation
            outFP = dc_fopen(env, err, command->stdout_file, "w");
        }
        else
        {
            outFP = dc_fopen(env, err, command->stdout_file, "a");
        }
        //call dup2 for the file and stdout
        dc_dup2(env, err, fileno(outFP), STDIN_FILENO);
    }

    if (command->stderr_file != NULL)
    {
        //open the file for truncation or appending.
        if (command->stderr_overwrite)
        {
            //truncation
            errFP = dc_fopen(env, err, command->stderr_file, "w");
        }
        else
        {
            errFP = dc_fopen(env, err, command->stderr_file, "a");
        }
        //call dup2 for the file and stdout
        dc_dup2(env, err, fileno(errFP), STDIN_FILENO);
    }
}

/**
 * Run a process
 *
 * @param command   the command object to run.
 * @param path      The array of PATH directories to serach for the program.
 */
static void run(const struct dc_posix_env* env, struct dc_error* err, const struct command* command, char** path)
{
    char *cmd;
    char path_buf[1024] = {0};

    if (dc_strchr(env, command->command, '/') == 0)
    {
        //set command.argv[0] to command.command
        command->argv[0] = dc_strdup(env, err, command->command);
        dc_execve(env, err, command->argv[0], command->argv, NULL);
    }
    else
    {
        //loop over the path array
        for (size_t i = 0; path[i]; i++)
        {
            //set cmd to path[i]/command.command
            dc_strcpy(env, path_buf, path[i]);
            dc_strcat(env, path_buf, command->command);
            cmd = dc_strdup(env, err, path_buf);
            //set cmmand.argv[0] to cmd
            command->argv[0] = cmd;
            //call execve for the cmd
            dc_execve(env, err, cmd, command->argv, NULL);
            if (dc_error_has_error(err))
            {
                if (err->errno_code != ENOENT)
                {
                    //exit the loop
                    break;
                }
            }
        }

    }
}

static int handle_run_error(struct dc_error* err, struct command* command)
{
//    fprintf(stderr, "ERROR: COMMAND %s : %s", command->command, err->message);
    return err->err_code;
}

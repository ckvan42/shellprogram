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

/**
 *
 *
 * @param env
 * @param err
 * @param command
 */
static void redirect(const struct dc_posix_env* env, struct dc_error* err, struct command* command);

/**
 *
 *
 * @param err
 * @param commandPt
 * @return
 */
static int handle_run_error(struct dc_error* err);

/**
 *
 * @param env
 * @param err
 * @param command
 * @param path
 */
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
                dc_exit(env, err->err_code);
            }
            //call run() -> this only returns if there is an error calling execv.
            run(env, err, command, path);
            //am I calling the execv in this function too? I'm guessing not.
            if  (dc_error_has_error(err))
            {
                status = handle_run_error(err);
            }
            dc_exit(env, status);
        }
        else
        {
            // Main process
            waitpid(pid, &status, 0);

            if (WIFEXITED(status))
            {
                int es = WEXITSTATUS(status);
                command->exit_code = es;
            }

            if (command->exit_code == 127)
            {
                fprintf(stdout, "command: %s not found.\n", command->command);
            }
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
            if (inFP)
            {
                dc_fclose(env, err, inFP);
            }
            return;
        }
        //call dup2 for the file and stdin
        dc_dup2(env, err, fileno(inFP), STDIN_FILENO);
        if (dc_error_has_error(err))
        {
            if (inFP)
            {
                dc_fclose(env, err, inFP);
            }
            return;
        }
    }

    if (command->stdout_file != NULL)
    {
        //open the file for truncation or appending.
        if (command->stdout_overwrite)
        {
            //truncation
            outFP = dc_fopen(env, err, command->stdout_file, "w+");
        }
        else
        {
            outFP = dc_fopen(env, err, command->stdout_file, "a+");
        }
        //call dup2 for the file and stdout
        if (dc_error_has_error(err))
        {
            if (outFP)
            {
                dc_fclose(env, err, outFP);
            }
            return;
        }
        dc_dup2(env, err, fileno(outFP), STDOUT_FILENO);
        if (dc_error_has_error(err))
        {
            if (outFP)
            {
                dc_fclose(env, err, outFP);
            }
            return;
        }
    }

    if (command->stderr_file != NULL)
    {
        //open the file for truncation or appending.
        if (command->stderr_overwrite)
        {
            //truncation
            errFP = dc_fopen(env, err, command->stderr_file, "w+");
        }
        else
        {
            errFP = dc_fopen(env, err, command->stderr_file, "a+");
        }
        //call dup2 for the file and stdout
        if (dc_error_has_error(err))
        {
            dc_fclose(env, err, errFP);
            return;
        }
        dc_dup2(env, err, fileno(errFP), STDERR_FILENO);
        if (dc_error_has_error(err))
        {
            if(errFP)
            {
                dc_fclose(env, err, errFP);
            }
            return;
        }
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


    if (dc_strchr(env, command->command, '/') != NULL)
    {
        //set command.argv[0] to command.command
        command->argv[0] = dc_strdup(env, err, command->command);
        dc_execv(env, err, command->command, command->argv);
    }
    else
    {
        //loop over the path array
        if (!path[0])
        {
            DC_ERROR_RAISE_ERRNO(err, ENOENT);
        }

        for (size_t i = 0; path[i]; i++)
        {
            //set cmd to path[i]/command.command
            sprintf(path_buf, "%s/%s", path[i], command->command);
            cmd = dc_strdup(env, err, path_buf);
            command->argv[0] = cmd;
            //call execve for the cmd
            dc_execv(env, err, cmd, command->argv);
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

static int handle_run_error(struct dc_error* err)
{
    int ex_code;

    switch(err->errno_code) {
        case E2BIG:
            ex_code = 1;
            break;
        case EACCES:
            ex_code = 2;
            break;
        case EINVAL:
            ex_code = 3;
            break;
        case ELOOP:
            ex_code = 4;
            break;
        case ENAMETOOLONG:
            ex_code = 5;
            break;
        case ENOENT:
            ex_code = 127;
            break;
        case ENOTDIR:
            ex_code = 6;
            break;
        case ENOEXEC:
            ex_code = 7;
            break;
        case ENOMEM:
            ex_code = 8;
            break;
        case ETXTBSY:
            ex_code = 9;
            break;
        default:
            ex_code = 7;
            break;
    }

    return ex_code;
}

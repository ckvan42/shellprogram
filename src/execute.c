//
// Created by Giwoun Bae on 2022-01-18.
//

#include "../include/execute.h"
#include <stdio.h>
#include <dc_posix/dc_unistd.h>

void execute(const struct dc_posix_env *env, struct dc_error *err,
             struct command *command, char **path)
{
    pid_t pid;

    pid = dc_fork(env, err);

    if (pid == 0)
    {
        //call redirect();
    }

}
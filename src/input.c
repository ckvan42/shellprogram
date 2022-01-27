//
// Created by Giwoun Bae on 2022-01-18.
//

#include "../include/input.h"
#include <stdio.h>
#include <dc_posix/dc_string.h>
#include <dc_util/strings.h>

char *read_command_line(const struct dc_posix_env *env, struct dc_error *err,
                        FILE *stream, size_t *line_size)
{
    char * buffer;
    size_t size;
    ssize_t nread;

    size = *line_size;
    buffer = dc_calloc(env, err, size, sizeof (char));
    if (dc_error_has_no_error(err))
    {
        nread = getline(&buffer, line_size, stream);
        if (nread == -1)
        {
            if (!buffer)
            {
                perror("FAILED\n");
                dc_exit(env, DC_ERROR_USER);
            }
        }
    }
    dc_str_trim(env, buffer);
    *line_size = dc_strlen(env, buffer);

    return buffer;
}
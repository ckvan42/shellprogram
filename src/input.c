
#include "../include/input.h"
#include <stdio.h>
#include <dc_posix/dc_string.h>
#include <dc_util/strings.h>
#include <dc_posix/dc_stdio.h>

char *read_command_line(const struct dc_posix_env *env, struct dc_error *err,
                        FILE *stream, size_t *line_size)
{
    char * buffer;
    size_t size;

    size = *line_size;
    buffer = dc_calloc(env, err, size, sizeof (char));
    if (dc_error_has_error(err)) {
        dc_exit(env, errno);
    }
    dc_getline(env, err, &buffer, line_size, stream);
    if (dc_error_has_error(err)) {
        dc_exit(env, errno);
    }
    dc_str_trim(env, buffer);
    *line_size = dc_strlen(env, buffer);

    return buffer;
}

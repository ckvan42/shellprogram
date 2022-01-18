#ifndef DC_SHELL_STATE_H
#define DC_SHELL_STATE_H

/*
 * This file is part of dc_shell.
 *
 *  dc_shell is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Foobar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with dc_shell.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <regex.h>
#include <stdbool.h>

/*! \struct state
    \brief The current FSM state.

    The state passed around to the FSM functions.
*/
struct state
{
  regex_t *in_redirect_regex;   /**< stdin regex */
  regex_t *out_redirect_regex;  /**< stdout regex */
  regex_t *err_redirect_regex;  /**< stderr regex */
  char **path;                  /**< PATH env var broken up */
  const char *prompt;           /**< Prompt to display before a command is entered */
  size_t max_line_length;       /**< the largest possible line */
  char *current_line;           /**< the line the user most recently entered */
  size_t current_line_length;   /**< the length of the most recently line */
  struct command *command;      /**< the commands to execute - currently only one */
  bool fatal_error;             /**< should the error terminate the shell (true = terminate) */
};

#endif // DC_SHELL_STATE_H

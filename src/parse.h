/*
	Anynome - The Generalized Metronome

    Copyright (C) 2007-2008  Nigel D. Stepp

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Nigel Stepp <stepp@atistar.net>

    $Id: parse.h 184 2008-03-03 08:20:16Z stepp $
*/

#ifndef _PARSE_H
#define _PARSE_H

#include <limits.h>
#include "stimulus.h"
#include "plugin.h"

#ifdef WIN32
 #define usleep(t) (Sleep( (t)/1000 ))
 #define PATH_MAX MAX_PATH
#endif

#define CONFIG_MAX_LINE  1024
#define CONFIG_MAX_TOKEN   64

typedef struct {
	int fullscreen;
	int generate_only;
	int verbose;
	int show_config_help;
	int num_stimuli;
	char stimulus_file_base[PATH_MAX];
	char plugin_dir[PATH_MAX];
	stimulus_list_t *stimulus_list;
} config_t;


/*
 * These are standard keywords for all possible
 * stimuli.  Stimulus specfic keywords are defined
 * in the stimulus plugin library.
 */
typedef enum {
	CONF_NUM_EVENTS,
	CONF_DISPLAY,
	CONF_DISPLAY_DURATION,
	CONF_DISPLAY_POSITION,
	CONF_DISPLAY_SIZE,
	CONF_DISPLAY_COLOR,
	CONF_AUDIO,
	CONF_AUDIO_DURATION,
	CONF_AUDIO_FREQUENCY,
	CONF_END
} config_keyword_t;

void config_parse(config_t *config, char *config_path);
void config_set_value(stimulus_t *stim, dict_entry_value_t *value, dict_entry_t *entry);
void config_plugin_add(config_t *config, char *plugin);
char *config_remove_whitespace(char *str);
void config_show_help(void);

#endif /* _PARSE_H */

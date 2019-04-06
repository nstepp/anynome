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

    $Id: plugin_file.c 749 2012-03-11 07:30:08Z stepp $
*/

#include <stdio.h>
#include "dict.h"


#define PLUGIN_LABEL "file"

enum {
	CONF_FILE,
	CONF_END
} config_keywords;

// Define plugin parameters here
dict_entry_t plugin_dict_entries[] = {
	{CONF_FILE,           "file",           DICT_TYPE_STRING, {.string_val=""}}
};

dict_t plugin_dict = {
	CONF_END, // num entries
	plugin_dict_entries
};

typedef struct {
	FILE *delay_file;
} plugin_config_t;

#include "plugin_common.h"

void set_arg(void *config, dict_entry_t *entry, dict_entry_value_t value)
{
	plugin_config_t *plugin_config = (plugin_config_t*)config;

	switch( entry->id ) {
		case CONF_FILE:
			plugin_config->delay_file = fopen(value.string_val,"r");
			if( !plugin_config->delay_file ) {
				plugin_error(PLUGIN_LABEL, "Unable to open delay file.");
				perror(PLUGIN_LABEL);
				exit(1);
			}
			break;
	}
}

void get_delay_series(void *config, uint32_t *delay_data, int num_events)
{
	int i;
	int len=16;
	int32_t delay;
	char line[16];
	plugin_config_t *plugin_config = (plugin_config_t*)config;


	if( !config || !delay_data || num_events <= 0 ) {
		return;
	}


	if( !plugin_config->delay_file ) {
		plugin_error(PLUGIN_LABEL, "Delay file is not open!");
		exit(1);
	}


	for( i=0; i<num_events; i++ ) {
		if( feof(plugin_config->delay_file) ) {
			plugin_error(PLUGIN_LABEL, "Ran out of file before we got to num_events");
			break;
		}

		fgets(line, len-1, plugin_config->delay_file);
		delay = atoi(line);

		if( delay <= 0 ) {
			fprintf(stderr, "%s: Error reading in a number at line %d, bailing.\n", PLUGIN_LABEL, i+1);
			exit(1);
		}

		delay_data[i] = (uint32_t)delay;
	}

	fclose(plugin_config->delay_file);
}


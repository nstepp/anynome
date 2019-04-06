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

    $Id: plugin_constant.c 184 2008-03-03 08:20:16Z stepp $
*/

#ifndef WIN32
 #include <unistd.h>
 #include <stdint.h>
#else
 #include <Windows.h>
#endif
#include "dict.h"

/*
 * Define plugin specific things here
 */

#define PLUGIN_LABEL "constant"

enum {
	CONF_DELAY,
	CONF_END
} config_keywords;

// Define plugin parameters here
dict_entry_t plugin_dict_entries[] = {
	{CONF_DELAY,           "delay",           DICT_TYPE_INT, {.int_val=1000}}
};

dict_t plugin_dict = {
	CONF_END, // num entries
	plugin_dict_entries
};

typedef struct {
	// in the future, possibly put a GUID here to compare against.
	// delay in miliseconds
	uint32_t delay;
} plugin_config_t;

/*
 * Then include common items
 */
#include "plugin_common.h"

/*
 * API Implementations:
 */
void set_arg(void *config, dict_entry_t *entry, dict_entry_value_t value)
{
	plugin_config_t *plugin_config = (plugin_config_t*)config;

	switch( entry->id ) {
		case CONF_DELAY:
			plugin_config->delay = value.int_val;
			if( plugin_config->delay <= 0 ) {
				plugin_error(PLUGIN_LABEL, "Delay must be a strictly positive integer.");
				exit(1);
			}
			break;
	}
}

void get_delay_series(void *config, uint32_t *delay_data, int num_events)
{
	int i;
	plugin_config_t *plugin_config = (plugin_config_t*)config;

	if( !config || !delay_data || num_events <= 0 ) {
		return;
	}

	for( i=0; i<num_events; i++ ) {
		// delay_data is expected to be in microseconds
		delay_data[i] = plugin_config->delay * 1000;
	}
}


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

    $Id: plugin_template.c 186 2008-03-03 22:21:08Z stepp $
*/

/*
 * Add your includes here.
 * You must include dict.h.
 */
#include "dict.h"

/*
 * Define your plugin name here
 */
#define PLUGIN_LABEL "template"

/*
 * Create names for your configuration keywords.
 * If they are duplicates of main keywords, they
 * will be ignored.
 */
enum {
	CONF_EXAMPLE,
	CONF_END
} config_keywords;

/*
 * Create the dictionary of keywords.
 * For each keyword, define its actual string value, type, and default.
 */
dict_entry_t plugin_dict_entries[] = {
	{CONF_EXAMPLE,           "example",           DICT_TYPE_INT, {.int_val=1}}
};

/*
 * This is the actual dictionary.
 * Usually this will not need to be changed.
 *
 *>>>> This *must* be called plugin_dict <<<<
 */
dict_t plugin_dict = {
	CONF_END, // number of entries
	plugin_dict_entries
};

/*
 * Define a structure to hold the values of your configuration items.
 *
 * This will be passed to set_arg and get_delay_series.
 */
typedef struct {
	// delay in miliseconds
	int example;
} plugin_config_t;


/*
 * Include common plugin components here.
 */
#include "plugin_common.h"

/*
 * Use this function to set configuration values using your dictionary.
 */
void set_arg(void *config, dict_entry_t *entry, dict_entry_value_t value)
{
	plugin_config_t *plugin_config = (plugin_config_t*)config;

	switch( entry->id ) {
		case CONF_EXAMPLE:
			plugin_config->example = value.int_val;
			// Probably want to add error checking
			break;
	}
}

/*
 * get_delay_series fills in delay_data with unsigned 32 bit integers.
 * Each number is a delay:
 * stimulus --> delay_data[0] --> stimulus --> delay_data[1] --> ...
 */
void get_delay_series(void *config, uint32_t *delay_data, int num_events)
{
	plugin_config_t *plugin_config = (plugin_config_t*)config;

	if( !config || !delay_data || num_events <= 0 ) {
		return;
	}

	// Fill in delay_data
}



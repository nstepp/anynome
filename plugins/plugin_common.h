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

    $Id: plugin_common.h 184 2008-03-03 08:20:16Z stepp $
*/


#ifndef _PLUGIN_COMMON_H
#define _PLUGIN_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "dict.h"

#ifndef WIN32
 #include <unistd.h>
 #include <stdint.h>
 #define DLLEXPORT
#else
 #include <Windows.h>

 #define DLLEXPORT __declspec(dllexport)
 #define inline __inline

 typedef UINT32 uint32_t;
#endif

const char *api_version = "4";

// The following functions define the API
DLLEXPORT void set_arg(void *config, dict_entry_t *entry, dict_entry_value_t value);
DLLEXPORT void get_delay_series(void *config, uint32_t *delay_data, int num_events);
DLLEXPORT dict_t *get_plugin_dict(void);
DLLEXPORT void *get_new_config(void);

inline void plugin_error(const char *label, const char *msg)
{
	fprintf(stderr, "%s: %s\n", label, msg);
}

dict_t *get_plugin_dict(void)
{
	return &plugin_dict;
}

void *get_new_config(void)
{
	plugin_config_t *config = NULL;

	config = malloc(sizeof(plugin_config_t));

	if( !config ) {
		plugin_error(PLUGIN_LABEL, "Error creating a copy of my config structure.");
		exit(1);
	}

	return (void *)config;
}


DLLEXPORT const char *get_api_version(void)
{
	return api_version;
}

void show_help(void)
{
	int i;
	dict_entry_value_t default_val;

	printf("\nDictionary for '%s':\n", PLUGIN_LABEL);

	for( i=0; i<plugin_dict.num_entries; i++ ) {
		printf("\t%s = ", plugin_dict.entries[i].token);
		default_val = plugin_dict.entries[i].default_value;
		switch( plugin_dict.entries[i].type ) {
			case DICT_TYPE_BOOL:
				printf("<bool> (default: %s)\n", (default_val.bool_val) ? "true" : "false");
				break;
			case DICT_TYPE_INT:
				printf("<int> (default: %d)\n", default_val.int_val);
				break;
			case DICT_TYPE_FLOAT:
				printf("<float> (default: %f)\n", default_val.float_val);
				break;
			case DICT_TYPE_STRING:
				printf("<string> (default: %s)\n", default_val.string_val);
				break;
			case DICT_TYPE_TUPLE:
				printf("(<int>,<int>) (default: (%d,%d))\n", default_val.tuple_val[0], default_val.tuple_val[1]);
				break;
		}
	}
}

#endif


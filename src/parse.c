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

    $Id: parse.c 749 2012-03-11 07:30:08Z stepp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include "parse.h"
#include "dict.h"
#include "stimulus.h"

// This is the dictionary for the main configuration keywords.
// Each plugin may define its own dictionary, which it uses
// internally.
dict_entry_t config_dict_entries[] = {
	{CONF_NUM_EVENTS,       "num_events",       DICT_TYPE_INT,    {.int_val=100}},
	{CONF_DISPLAY,          "display",          DICT_TYPE_BOOL,   {.bool_val=1}},
	{CONF_DISPLAY_DURATION, "display_duration", DICT_TYPE_INT,    {.int_val=500}},
	{CONF_DISPLAY_POSITION, "display_position", DICT_TYPE_TUPLE,  {.tuple_val={390,290}}},
	{CONF_DISPLAY_SIZE,     "display_size",     DICT_TYPE_TUPLE,  {.tuple_val={20,20}}},
	{CONF_DISPLAY_COLOR,    "display_color",    DICT_TYPE_STRING, {.string_val="white"}},
	{CONF_AUDIO,            "audio",            DICT_TYPE_BOOL,   {.bool_val=0}},
	{CONF_AUDIO_DURATION,   "audio_duration",   DICT_TYPE_INT,    {.int_val=500}},
	{CONF_AUDIO_FREQUENCY,  "audio_frequency",  DICT_TYPE_INT,    {.int_val=260}}
};

void config_parse(config_t *config, char *config_path)
{
	int i;
	int linenum = 0;
	int token_for_plugin;
	char raw_line[CONFIG_MAX_LINE];
	char *line = NULL;
	char *c, *token = NULL, *value = NULL;
	FILE *config_file = NULL;
	struct stat file_stat;
	dict_t config_dict;
	dict_entry_value_t config_val;
	dict_entry_t *token_info = NULL;
	stimulus_t *current_stim = NULL;

	if( !config ) {
		fprintf(stderr, "Cannot parse to NULL config");
		return;
	}
	if( !config_path ) {
		fprintf(stderr, "Cannot parse NULL config file");
		return;
	}

	if( stat(config_path, &file_stat) == -1 ) {
		perror("Config file stat");
		exit(1);
	}

	config_file = fopen(config_path, "r");
	if( !config_file ) {
		perror("Config file open");
		exit(1);
	}

	// Initialize dictionary
	config_dict.num_entries = CONF_END;
	config_dict.entries = config_dict_entries;

	linenum = 0;
	memset(raw_line, 0, CONFIG_MAX_LINE);
	while( fgets(raw_line, CONFIG_MAX_LINE, config_file) ) {
		// let me sleep at night...
		raw_line[CONFIG_MAX_LINE-1] = '\0';

		// Clean up newline garbage
		if( (c = strrchr(raw_line, '\n')) ) {
			*c = '\0';
		}
		if( (c = strrchr(raw_line, '\r')) ) {
			*c = '\0';
		}

		linenum++;

		// Remove initial spaces and tabs
		line = raw_line;
		while( line[0] == ' ' || line[0] == '\t' ) {
			line++;
		}

		// Take care of comments and empty lines
		if( line[0] == '#' || line[0] == '\0' ) {
			continue;
		}

		// Check for a plugin heading
		if( line[0] == '[' ) {
			c = value = line+1;
			while( c!= '\0' && isalnum(*c) ) {
				c++;
			}
			*c = '\0';
		
			if( !strlen(value) ) {
				fprintf(stderr, "Bad plugin heading on line %d\n", linenum);
				exit(1);
			}

			config_plugin_add(config, value);
			current_stim = config->stimulus_list->stim;

			// set defaults
			for( i=0; i<config_dict.num_entries; i++ ) {
				config_set_value(current_stim, &(config_dict.entries[i].default_value), &(config_dict.entries[i]));
			}

			//plugin_set_defaults();

			if( config->show_config_help ) {
				plugin_show_help(current_stim->plugin_dl);
			}

			continue;

		}

		// Format is:
		// variable[ ]*=[ ]*value
		// The first word should be a config token.
		token = strtok(line, "=");

		// remove spaces
		token = util_remove_whitespace(token);

		// try to find which token this is
		token_info = dict_lookup_token(&config_dict, token);

		// if this is unknown and we have a plugin,
		// have the plugin look it up in its dictionary.

		token_for_plugin = 0;
		if( !token_info && current_stim ) {
			token_info = plugin_lookup_token(current_stim->plugin_dl, token);
			token_for_plugin = 1;
		}

		if( !token_info ) {
			fprintf(stderr, "Unknown token '%s' on line %d\n", token, linenum);
			exit(1);
		}

		// Get the value for this token
		value = strtok(NULL,"=");
		value = config_remove_whitespace(value);

		if( !strlen(value) ) {
			fprintf(stderr, "Empty value for '%s' on line %d\n", token, linenum);
			exit(1);
		}

		dict_set_value(&config_val, token_info->type, value);

		// If this belongs to a plugin, we need to send it there
		if( token_for_plugin ) {
			plugin_set_arg(current_stim->plugin_dl, token_info, config_val);
			continue;
		}


		// Now we can set stimulus specific stuff.
		if( !config->stimulus_list ) {
			fprintf(stderr, "Plugin option before plugin definition on line %d\n", linenum);
			exit(1);
		}


		config_set_value(current_stim, &config_val, token_info);
	}
	fclose(config_file);
}

void config_set_value(stimulus_t *stim, dict_entry_value_t *value, dict_entry_t *entry)
{
	switch( entry->id ) {
		case CONF_NUM_EVENTS:
			stim->num_events = value->int_val;
			break;
		case CONF_DISPLAY:
			stim->display = value->bool_val;
			break;
		case CONF_DISPLAY_DURATION:
			stim->display_duration = value->int_val;
			break;
		case CONF_DISPLAY_POSITION:
			stim->display_position[0] = value->tuple_val[0];
			stim->display_position[1] = value->tuple_val[1];
			break;
		case CONF_DISPLAY_SIZE:
			stim->display_size[0] = value->tuple_val[0];
			stim->display_size[1] = value->tuple_val[1];
			break;
		case CONF_DISPLAY_COLOR:
			stimulus_set_color(&(stim->display_color), value->string_val);
			break;
		case CONF_AUDIO:
			stim->audio = value->bool_val;
			break;
		case CONF_AUDIO_DURATION:
			stim->audio_duration = value->int_val;
			break;
		case CONF_AUDIO_FREQUENCY:
			stim->audio_frequency = value->int_val;
			break;
		default:
			fprintf(stderr, "Unknown token id? That's unpossible!\n");
			exit(1);
			break;
	}
}


void config_plugin_add(config_t *config, char *plugin)
{
	char stimulus_filename[PATH_MAX];

	stimulus_list_t *new_list_node = malloc(sizeof(stimulus_list_t));
	stimulus_t *new_stim = malloc(sizeof(stimulus_t));

	if( !new_list_node || !new_stim ) {
		fprintf(stderr, "Unable to allocate memory for a plugin\n");
		exit(1);
	}

	strncpy(new_stim->plugin, plugin, PATH_MAX);
	snprintf(stimulus_filename, PATH_MAX, "%s-%d-%s.txt", config->stimulus_file_base, config->num_stimuli, plugin);

	if( !config->show_config_help ) {
		new_stim->stimulus_file = fopen(stimulus_filename, "w");
		if( !new_stim->stimulus_file ) {
			perror("stimulus file open");
			exit(1);
		}
	}

	new_stim->plugin_dl = plugin_load( config->plugin_dir, plugin );
	if( !new_stim->plugin_dl ) {
		fprintf(stderr, "Error loading plugin '%s'.\n", plugin);
		exit(1);
	}

	new_list_node->stim = new_stim;
	new_list_node->next = NULL;

	if( config->stimulus_list ) {
		new_list_node->next = config->stimulus_list;
	}

	config->stimulus_list = new_list_node;
	config->num_stimuli++;
}


char *config_remove_whitespace(char *str)
{
	int i;

	if( !str ) {
		return NULL;
	}

	while( str[0] == ' ' || str[0] == '\t' ) {
		str++;
	}

	i = strlen(str) - 1;
	while( isspace(str[i]) ) {
		str[i--] = '\0';
	}

	return str;
}

void config_show_help(void)
{
	int i;
	dict_entry_value_t default_val;
	stimulus_color_list_t *stimulus_colors;

	printf("A configuration file is made up sections defining properties\n");
	printf("for each stimulus.  Each section begins with:\n");
	printf("\n[plugin_name]\n\n");
	printf("Inside each section place common properties defined in the main\n");
	printf("configuration dictionary, as well as plugin specific properties.\n");

	printf("\nMain Configuration Dictionary:\n");

	for( i=0; i<CONF_END; i++ ) {
		printf("\t%s = ", config_dict_entries[i].token);
		default_val = config_dict_entries[i].default_value;
		switch( config_dict_entries[i].type ) {
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

	printf("\nThe following colors are defined:\n");
	stimulus_colors = stimulus_get_color_list();
	printf("%16s\tRGB\t\tHex\n","Color");
	for( i=0; i<stimulus_colors->num_colors; i++ ) {
		printf("%16s:\t(%d,%d,%d),\t#%02x%02x%02x\n",
			stimulus_colors->colors[i].name,
			stimulus_colors->colors[i].r,
			stimulus_colors->colors[i].g,
			stimulus_colors->colors[i].b,
			stimulus_colors->colors[i].r,
			stimulus_colors->colors[i].g,
			stimulus_colors->colors[i].b
		);
	}


	printf("\nTo see help for plugins, add a [plugin] section to a config file\n");
	printf("and re-run with -H.\n");
}


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

    $Id: dict.c 184 2008-03-03 08:20:16Z stepp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include "dict.h"

dict_entry_t *dict_lookup_token(dict_t *dict, char *token)
{
	int i;

	if( !dict ) {
		return NULL;
	}
	if( !token ) {
		return NULL;
	}
	if( dict->num_entries <= 0 ) {
		return NULL;
	}

	for(i=0; i<dict->num_entries; i++) {
		if( !strcasecmp(dict->entries[i].token, token) ) {
			return &(dict->entries[i]);
		}
	}

	return NULL;
}


void dict_set_value(dict_entry_value_t *dict_val, dict_type_t type, char *value)
{
	char *c;

	if( !dict_val ) {
		fprintf(stderr, "dict_set_value: Null dictionary entry!\n");
		exit(1);
	}
	if( !value ) {
		fprintf(stderr, "dict_set_value: Null value!\n");
		exit(1);
	}

	// Fill in dict_val union
	switch( type ) {
		case DICT_TYPE_BOOL:
			if( !strcasecmp(value,"true") || !strcasecmp(value,"yes") ) {
				dict_val->bool_val = 1;
			} else {
				dict_val->bool_val = 0;
			}
			break;
		case DICT_TYPE_INT:
			dict_val->int_val = atoi(value);
			break;
		case DICT_TYPE_FLOAT:
			dict_val->float_val = atof(value);
			break;
		case DICT_TYPE_STRING:
			strncpy(dict_val->string_val, value, DICT_MAX_TOKEN-1);
			break;
		case DICT_TYPE_TUPLE:
			c = strchr(value, ',');
			
			if( c ) {
				*c = '\0';
			}

			dict_val->tuple_val[0] = atoi(value);
			dict_val->tuple_val[1] = atoi(c+1);

			break;
		default:
			fprintf(stderr, "Something very strange is going on. Unknown type!!\n");
			exit(1);
	}
}


char *util_remove_whitespace(char *str)
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


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

    $Id: dict.h 184 2008-03-03 08:20:16Z stepp $
*/

#ifndef _DICT_H
#define _DICT_H

#define DICT_MAX_TOKEN   64

/*
 * These are standard keywords for all possible
 * stimuli.  Stimulus specfic keywords are defined
 * in the stimulus plugin library.
 */

typedef enum {
	DICT_TYPE_BOOL,
	DICT_TYPE_INT,
	DICT_TYPE_FLOAT,
	DICT_TYPE_STRING,
	DICT_TYPE_TUPLE
} dict_type_t;

typedef union {
		unsigned char bool_val;
		int int_val;
		float float_val;
		char string_val[DICT_MAX_TOKEN];
		int tuple_val[2];
} dict_entry_value_t;

typedef struct {
	int id;
	char token[DICT_MAX_TOKEN];
	dict_type_t type;
	dict_entry_value_t default_value;
} dict_entry_t;

typedef struct {
	int num_entries;
	dict_entry_t *entries;
} dict_t;

dict_entry_t *dict_lookup_token(dict_t *dict, char *token);
void dict_set_value(dict_entry_value_t *val, dict_type_t type, char *value);
char *util_remove_whitespace(char *str);

#endif /* _DICT_H */

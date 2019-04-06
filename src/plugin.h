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

    $Id: plugin.h 184 2008-03-03 08:20:16Z stepp $
*/


#ifndef _PLUGIN_H
#define _PLUGIN_H

#include "dict.h"

#define APIVERSION "4"

typedef struct {
	void *handle;
	char *name;
	void *config;
} plugin_t;

plugin_t *plugin_load(char *plugin_dir, char *plugin_name);
void plugin_set_arg(plugin_t *plugin, dict_entry_t *entry, dict_entry_value_t value);
dict_entry_t *plugin_lookup_token(plugin_t *plugin, char *token);
void plugin_show_help(plugin_t *plugin);
void plugin_call_void_fcn(plugin_t *plugin, const char *fcn);
void *plugin_get_fcn(plugin_t *plugin, const char *fcn);
void plugin_unload(plugin_t *plugin);

#endif

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

    $Id: plugin.c 184 2008-03-03 08:20:16Z stepp $
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

#ifdef WIN32
 #include <windows.h>
 #define dlopen(a,b) (LoadLibrary(a))
 #define dlsym(a,b) (GetProcAddress(a,b))
 #define dlclose(a) (FreeLibrary(a))
 #define dlerror() (NULL)
 #define PATH_MAX MAX_PATH
#else
 #include <dlfcn.h>
#endif

#include "plugin.h"

plugin_t *plugin_load(char *plugin_dir, char *plugin_name)
{
	char plugin_filename[PATH_MAX];
	char api_version[8];
	void *plugin_handle = NULL;
	plugin_t *new_plugin = NULL;
	const char *(*get_api_version)(void) = NULL;
	void *(*get_new_config)(void) = NULL;
	struct stat file_stat;

	memset(plugin_filename, 0, PATH_MAX);

	if( plugin_dir && strlen(plugin_dir) ) {
#ifdef WIN32
		_snprintf(plugin_filename, PATH_MAX-1, "%s\\plugin_%s.dll", plugin_dir, plugin_name);
#else
		snprintf(plugin_filename, PATH_MAX-1, "%s/plugin_%s.so", plugin_dir, plugin_name);
#endif

		if( stat(plugin_filename, &file_stat) != -1 ) {
			perror("Plugin library stat");
			exit(1);
		}
		if( !S_ISREG(file_stat.st_mode) ) {
			fprintf(stderr, "The path specified for plugin '%s' is not a regular file.\n", plugin_name);
			exit(1);
		}
	} else {
#ifdef WIN32
		_snprintf(plugin_filename, PATH_MAX-1, "plugin_%s.dll", plugin_name);
#else
		snprintf(plugin_filename, PATH_MAX-1, "plugin_%s.so", plugin_name);
#endif
	}

	plugin_handle = dlopen(plugin_filename, RTLD_NOW);
	if( !plugin_handle ) {
		fprintf(stderr, "Failed to load plugin library: %s\n", dlerror());
		exit(1);
	}

	new_plugin = malloc(sizeof(plugin_t));
	if( !new_plugin ) {
		fprintf(stderr, "Could not malloc new plugin.\n");
		exit(1);
	}

	// Set the minimum here, then check API version

	new_plugin->handle = plugin_handle;
	new_plugin->name = strdup(plugin_name);

	get_api_version = plugin_get_fcn(new_plugin, "get_api_version");

	strncpy(api_version, get_api_version(), 7);
	api_version[7] = '\0';

	if( strcmp( APIVERSION, api_version ) ) {
		fprintf(stderr, "API versions do not match.  I am %s, plugin is %s.\n", APIVERSION, api_version);
		exit(1);
	}

	// Now we can set everything else

	get_new_config = plugin_get_fcn(new_plugin, "get_new_config");

	new_plugin->config = get_new_config();


	return new_plugin;
}

void plugin_unload(plugin_t *plugin)
{
	free(plugin->name);
	free(plugin->config);
	dlclose(plugin->handle);
	free(plugin);
}

void plugin_set_arg(plugin_t *plugin, dict_entry_t *entry, dict_entry_value_t value)
{
	void (*set_arg)(void *,dict_entry_t *,dict_entry_value_t);

	set_arg = plugin_get_fcn(plugin, "set_arg");

	set_arg(plugin->config, entry, value);
}

dict_entry_t *plugin_lookup_token(plugin_t *plugin, char *token)
{
	dict_t *(*get_plugin_dict)(void);

	get_plugin_dict = plugin_get_fcn(plugin, "get_plugin_dict");

	return dict_lookup_token(get_plugin_dict(), token);
}

void plugin_set_defaults(plugin_t *plugin)
{
	plugin_call_void_fcn(plugin, "set_defaults");
}

void plugin_show_help(plugin_t *plugin)
{
	plugin_call_void_fcn(plugin, "show_help");
}

void plugin_call_void_fcn(plugin_t *plugin, const char *fcn)
{
	void(*plugin_fcn)(void);

	plugin_fcn = plugin_get_fcn(plugin, fcn);

	plugin_fcn();
}

void *plugin_get_fcn(plugin_t *plugin, const char *fcn)
{
	char *error;
	void *func;

	if( !plugin || !fcn ) {
		return NULL;
	}

	// clear errors. save so the Win32 def will work out better
	error = dlerror();
	func = dlsym(plugin->handle, fcn);
	if( (error = dlerror()) != NULL || !func ) {

#ifdef WIN32
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &error,
			0, NULL );
#endif
		fprintf(stderr, "Error getting plugin function '%s': %s\n", fcn, error);
		exit(1);
	}

	return func;
}


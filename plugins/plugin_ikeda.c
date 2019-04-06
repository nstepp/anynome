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

    $Id: plugin_ikeda.c 184 2008-03-03 08:20:16Z stepp $
*/


#include <limits.h>
#include <float.h>
#include <math.h>
#include <sys/time.h>
#include "dict.h"

#define PLUGIN_LABEL "ikeda"

#define TS_TIMESTEP 0.1
#define RAND_WAIT_MIN 500000
#define RAND_WAIT_MAX 1000000

// Define plugin parameters here
typedef struct {
	float alpha;
	float beta;
	int tau;
} plugin_config_t;

enum {
	CONF_ALPHA,
	CONF_BETA,
	CONF_TAU,
	CONF_END
} config_keywords;

// Define plugin parameters here
dict_entry_t plugin_dict_entries[] = {
	{CONF_ALPHA, "alpha", DICT_TYPE_FLOAT, {.float_val=-1}},
	{CONF_BETA,  "beta", DICT_TYPE_FLOAT, {.float_val=20}},
	{CONF_TAU,   "tau", DICT_TYPE_INT, {.int_val=2}}
};

dict_t plugin_dict = {
	CONF_END, // num entries
	plugin_dict_entries
};

#include "plugin_common.h"

void set_arg(void *config, dict_entry_t *entry, dict_entry_value_t value)
{
	plugin_config_t *plugin_config = (plugin_config_t*)config;

	switch( entry->id ) {
		case CONF_ALPHA:
			plugin_config->alpha = value.float_val;
			break;
		case CONF_BETA:
			plugin_config->beta = value.float_val;
			break;
		case CONF_TAU:
			plugin_config->tau = value.int_val;
			if( plugin_config->tau <= 0 ) {
				plugin_error(PLUGIN_LABEL, "tau must be a strictly positive integer.");
				exit(1);
			}
			break;
		}
}



void get_delay_series(void *config, uint32_t *delay_data, int num_events)
{
	int i;
	double max = -DBL_MAX;
	double min = DBL_MAX;
	double *double_delay_data = NULL;
	struct timeval tv;
	plugin_config_t *plugin_config = (plugin_config_t*)config;

	if( !config || !delay_data || num_events <= 0 ) {
		return;
	}
	if( plugin_config->tau >= num_events ) {
		plugin_error(PLUGIN_LABEL, "tau must be less than the number of stimulus events.");
		exit(1);
	}

	double_delay_data = malloc(num_events * sizeof(double));

	// initialize random number generator
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	double_delay_data[0] = (double)(rand()) / (double)INT_MAX;

	//	First do up to tau, since we can't go all the way back yet.
	for( i=1; i<plugin_config->tau; i++ ) {
		double_delay_data[i] = double_delay_data[i-1] + TS_TIMESTEP * ( plugin_config->alpha*double_delay_data[i-1] - plugin_config->beta*sin(double_delay_data[0]) );
		if( double_delay_data[i] > max ) {
			max = double_delay_data[i];
		} else if( double_delay_data[i] < min ) {
			min = double_delay_data[i];
		}
	}

	//	Then do the rest
	for( i=plugin_config->tau; i<num_events; i++ ) {
		double_delay_data[i] = double_delay_data[i-1] + TS_TIMESTEP * ( plugin_config->alpha*double_delay_data[i-1] - plugin_config->beta*sin(double_delay_data[i-1-plugin_config->tau]) );
		if( double_delay_data[i] > max ) {
			max = double_delay_data[i];
		} else if( double_delay_data[i] < min ) {
			min = double_delay_data[i];
		}
	}

	// Remap the data to fit between 1 and 2 seconds
	// and fill in the unsigned int array required by the API
	for( i=0; i<num_events; i++ ) {
		double_delay_data[i] -= min;
		double_delay_data[i] /= (max - min)/1000000;
		double_delay_data[i] += 1000000;
		double_delay_data[i] = rint(double_delay_data[i]);
		delay_data[i] = (uint32_t)(double_delay_data[i]);
	}
}


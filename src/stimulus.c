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

    $Id: stimulus.c 184 2008-03-03 08:20:16Z stepp $
*/


#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
 #include <unistd.h>
#endif

#include <limits.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>

#ifndef WIN32
 #include <SDL/SDL.h>
 #include <SDL/SDL_thread.h>
 #include <SDL/SDL_ttf.h>
#else
 #include <Windows.h>
 #include "SDL.h"
 #include "SDL_thread.h"
 #include "SDL_ttf.h"
#endif

#include "stimulus.h"
#include "plugin.h"
#include "sound.h"


stimulus_color_list_t stimulus_colors = {
	12,
	{
		{ "red",        0xFF, 0x00, 0x00 },
		{ "green",      0x00, 0xFF, 0x00 },
		{ "blue",       0x00, 0x00, 0xFF },
		{ "white",      0xFF, 0xFF, 0xFF },
		{ "cyan",       0x00, 0xFF, 0xFF },
		{ "yellow",     0xFF, 0xFF, 0x00 },
		{ "magenta",    0xFF, 0x00, 0xFF },
		{ "purple",     0x80, 0x00, 0x80 },
		{ "orange",     0xFF, 0xA5, 0x00 },
		{ "dark red",   0x80, 0x00, 0x00 },
		{ "dark green", 0x00, 0x80, 0x00 },
		{ "dark blue",  0x00, 0x00, 0x80 }
	}
};


stimulus_color_list_t *stimulus_get_color_list(void)
{
	return &stimulus_colors;
}

int stimulus_thread(void *data)
{
	int i;
	Uint32 stimDelay = 500000;
	Uint32 delay;
	Uint32 *delay_data = NULL;
	Uint32 *time_data = NULL;
	Uint32 stim_color;
	SDL_Rect stimDest;
	SDL_Event event;
	//TTF_Font *font = TTF_OpenFont( "./VeraMono.ttf", 36 );
	stimulus_t *stimulus = (stimulus_t *)data;

	// Allocate buffer for times
	delay_data = malloc(stimulus->num_events * sizeof(Uint32));
	if( !delay_data ) {
		fprintf(stderr, "Couldn't allocate memory for delay buffer.\n");
	}
	time_data = malloc(stimulus->num_events * sizeof(Uint32));
	if( !time_data ) {
		fprintf(stderr, "Couldn't allocate memory for time buffer.\n");
		exit(1);
	}
	memset(time_data, 0, stimulus->num_events * sizeof(Uint32));

	
	stimulus->get_delay_series(stimulus->plugin_dl->config, delay_data, stimulus->num_events);
	
	// each stimulus has a configurable location
	stimDest.x = stimulus->display_position[0];
	stimDest.y = stimulus->display_position[1];
	stimDest.w = stimulus->display_size[0];
	stimDest.h = stimulus->display_size[1];

	stimDelay = stimulus->display_duration * 1000;

	// Notify that I'm ready to go
	event.type = SDL_USEREVENT;
	event.user.code = USER_EVT_THREADREADY;
	SDL_PushEvent(&event);
	if( stimulus->verbose ) {
		printf("%u: Pushed ready event (thread %u).\n", SDL_GetTicks(), SDL_ThreadID());
	}
	
	// Wait here for signal to start
	SDL_mutexP(stimulus->stim_mutex);
	SDL_CondWait(stimulus->stim_cond, stimulus->stim_mutex);
	SDL_mutexV(stimulus->stim_mutex);

	if( stimulus->verbose ) {
		printf("%u: %u got signal to go.\n", SDL_GetTicks(), SDL_ThreadID());
	}

	// Provide the stimulus
	// only essential things go in this loop since timing is critical
	stim_color = SDL_MapRGB(stimulus->screen->format,
			stimulus->display_color.r,
			stimulus->display_color.g,
			stimulus->display_color.b
	);

	if( stimulus->screen ) {
		for( i=0; i<stimulus->num_events && !stimulus->done; i++ ) {
			// Display stimulus...
			if( stimulus->display ) {
				SDL_mutexP(stimulus->stim_mutex);
				SDL_FillRect(stimulus->screen, &stimDest, stim_color);
				SDL_UpdateRect(stimulus->screen, stimDest.x, stimDest.y, stimDest.w, stimDest.h);
				SDL_mutexV(stimulus->stim_mutex);
			}

			time_data[i] = SDL_GetTicks();

			if( stimulus->audio ) {
				play_sample(stimulus->sound->sound_data,
					stimulus->audio_data,
					stimulus->audio_data_len
				);
			}

			// ...for this long
			usleep(stimDelay);

			// Blank
			if( stimulus->display ) {
				SDL_mutexP(stimulus->stim_mutex);
				SDL_FillRect(stimulus->screen, &stimDest, 0x000000);
				SDL_UpdateRect(stimulus->screen, stimDest.x, stimDest.y, stimDest.w, stimDest.h);
				SDL_mutexV(stimulus->stim_mutex);
			}

			// Adjust delays to include stimulus display time,
			// also clamp to stimDelay so stimuli don't overlap.
			if( delay_data[i] > stimDelay ) {
				delay = delay_data[i] - stimDelay;
			} else {
				delay = 0;
			}

			usleep( delay );
		}
	}

	if( stimulus->verbose ) {
		printf("%u: Thread %u is done.\n", SDL_GetTicks(), SDL_ThreadID());
	}

	// Write data out to the stimulus file
	for( i=0; i<stimulus->num_events; i++ ) {
		fprintf(stimulus->stimulus_file, "%u\t%u\n", time_data[i], delay_data[i]);
	}
	fflush(stimulus->stimulus_file);


	free(time_data);
	free(delay_data);
	//if( font ) {
	//	TTF_CloseFont(font);
	//}

	// Let everyone know we are done
	event.type = SDL_USEREVENT;
	event.user.code = USER_EVT_THREADFINISH;
	SDL_PushEvent(&event);
	if( stimulus->verbose ) {
		printf("%u: Pushed done event (thread %u).\n", SDL_GetTicks(), SDL_ThreadID());
	}

	return 0;
}

void stimulus_set_color(stimulus_color_t *color_val, const char *color_str)
{
	int i;

	for( i=0; i<stimulus_colors.num_colors; i++ ) {
		if( !strcasecmp(color_str, stimulus_colors.colors[i].name) ) {
			strcpy(color_val->name, stimulus_colors.colors[i].name);
			color_val->r = stimulus_colors.colors[i].r;
			color_val->g = stimulus_colors.colors[i].g;
			color_val->b = stimulus_colors.colors[i].b;
			break;
		}
	}
}



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

    $Id: stimulus.h 184 2008-03-03 08:20:16Z stepp $
*/


#ifndef _STIMULUS_H
#define _STIMULUS_H

#include "sound.h"
#include "plugin.h"

#ifndef WIN32
 #include <SDL/SDL.h>
 #include <SDL/SDL_thread.h>
 #include <SDL/SDL_ttf.h>
#else
 #include <Windows.h>
 #include "SDL.h"
 #include "SDL_thread.h"
 #include "SDL_ttf.h"
 #include "getopt.h"
#endif


#ifdef WIN32
 #define usleep(t) (Sleep( (t)/1000 ))
 #define PATH_MAX MAX_PATH
#endif

#define USER_EVT_THREADREADY  0
#define USER_EVT_THREADFINISH 1

#define COLOR_NAME_MAX 16

typedef struct stimulus_color_t {
	char name[COLOR_NAME_MAX];
	int r;
	int g;
	int b;
} stimulus_color_t;

typedef struct {
	int num_colors;
	stimulus_color_t colors[12];
} stimulus_color_list_t;

// XXX This is getting big, might be good
// to make it more hierarchical.
typedef struct stimulus_t {
	   int done;
       int verbose;
       int num_events;
       unsigned char display;
       int display_duration;
       int display_position[2];
       int display_size[2];
	   stimulus_color_t display_color;
       unsigned char audio;
       int audio_duration;
       int audio_frequency;
	   char *audio_data;
	   int audio_data_len;
       FILE *stimulus_file;
       sound_t *sound;
       SDL_Surface *screen;
       SDL_Thread *thread;
       SDL_mutex *stim_mutex;
       SDL_cond *stim_cond;
       plugin_t *plugin_dl;
       void (*get_delay_series)(void *, Uint32 *, int);
       char plugin[PATH_MAX];
} stimulus_t;

typedef struct stimulus_list_t {
       stimulus_t *stim;
       struct stimulus_list_t *next;
} stimulus_list_t;


int stimulus_thread(void *data);

void stimulus_set_color(stimulus_color_t *color_val, const char *color_str);
stimulus_color_list_t *stimulus_get_color_list(void);


#endif /* _STIMULUS_H */


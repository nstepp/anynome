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

    $Id: sound.h 184 2008-03-03 08:20:16Z stepp $
*/


#ifndef _SOUND_H
#define _SOUND_H

#ifndef WIN32
 #include <SDL/SDL.h>
#else
 #include "SDL.h"
#endif

typedef struct sample_list_t {
	char *sample_data;
	int sample_len;
	struct sample_list_t *next;
} sample_list_t;

typedef struct sound_data_t {
	int pending_samples;
	SDL_mutex *mutex;
	sample_list_t *sample_list;
} sound_data_t;

typedef struct {
	SDL_AudioSpec *waveSpec;
	sound_data_t *sound_data;
} sound_t;

char *generate_sine_wave(int frequency, int samplerate, int msec, int *samples_out);
void sound_mix(char *dst, char *src, int len, double volume_factor);
void sound_clean_sample_list(sound_data_t *sound_data);
void play_sample(sound_data_t *sound_data, char *sample, int sample_len);
sound_t *init_sound(void);
void close_sound(sound_t *sound);


#endif

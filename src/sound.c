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

    $Id: sound.c 184 2008-03-03 08:20:16Z stepp $
*/


#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
 #include <unistd.h>
#endif

//#include <malloc.h>
#include <math.h>
#include <time.h>

#ifndef WIN32
 #include <SDL/SDL.h>
#else
 #include <Windows.h>
 #include "SDL.h"
#endif

#include "sound.h"

#define SOUND_SAMPLES 512
#define SOUND_FREQ 44100

static void sample_list_callback(void *udata, Uint8 *stream, int len);

char *generate_sine_wave(int frequency, int samplerate, int msec, int *samples_out)
{
	int i, samples;
	double buf_freq, freq;
	char *buf = NULL;
	
	samples = (int)ceil(((double)samplerate/1000) *  msec);
	
	buf_freq = ((double)frequency/(double)samplerate) * samples;

	buf = malloc(samples);

	freq = buf_freq * 2 * M_PI;
	for( i=0; i<samples; i++ ) {
		buf[i] = (char)(127 * sin( freq * ((double)i/(double)samples) ));
	}

	if( samples_out ) {
		*samples_out = samples;
	}

	return buf;
}


/*
 * This is not a general purpose mixer.
 * It specifically adds a series of waveforms together, assuming we
 * start with silence.
 * Each waveform is scaled by volume_factor before addition.
 *
 * It "works" ok, but a real mixer would be better.
 */
void sound_mix(char *dst, char *src, int len, double volume_factor)
{
	int i;

	if( !dst || !src || len <= 0 ) {
		fprintf(stderr, "bad input to sound_mix\n");
		return;
	}

	if( volume_factor > 1.0 || volume_factor < 0.0 ) {
		fprintf(stderr, "sound_mix got bad volume_factor (%f), not mixing\n", volume_factor);
		return;
	}

	for( i=0; i<len; i++ ) {
		dst[i] = (char)((double)dst[i] + volume_factor * (double)src[i]);
	}
}

static void sample_list_callback(void *udata, Uint8 *stream, int len)
{
	double volume_factor;
	sound_data_t *sound_data = (sound_data_t *)udata;

	if( !sound_data ) {
		return;
	}

	// We don't want sound_data changing while we do this
	SDL_mutexP(sound_data->mutex);

	sound_clean_sample_list(sound_data);

	if( sound_data->pending_samples == 0 ) {
		memset(stream, 0, len);
		SDL_mutexV(sound_data->mutex);
		return;
	}

	volume_factor = 1.0/sound_data->pending_samples;

	sample_list_t *p = sound_data->sample_list;
	while( p && p->sample_len > 0) {
		if( p->sample_len >= len ) {
			sound_mix((char *)stream, p->sample_data, len, volume_factor);
			p->sample_len -= len;
			p->sample_data += len;
		} else {
			sound_mix((char *)stream, p->sample_data, p->sample_len, volume_factor);
			// done with this sample, mark for deletion
			p->sample_len = 0;
			p->sample_data = NULL;
		}

		p = p->next;
	}

	SDL_mutexV(sound_data->mutex);
}

void play_sample(sound_data_t *sound_data, char *sample, int sample_len)
{
	sample_list_t *new_sample = NULL;

	if( !sound_data ) {
		fprintf(stderr, "NULL sound data\n");
		exit(1);
	}

	new_sample = malloc(sizeof(sample_list_t));

	if( !new_sample ) {
		fprintf(stderr, "Can't allocate new sound sample!\n");
		exit(1);
	}

	// Protect the sound_data structure
	SDL_mutexP(sound_data->mutex);

	// Then add new one
	new_sample->sample_data = sample;
	new_sample->sample_len = sample_len;
	new_sample->next = sound_data->sample_list;

	sound_data->sample_list = new_sample;

	sound_data->pending_samples++;


	// Go on our merry way
	SDL_mutexV(sound_data->mutex);
}

void sound_clean_sample_list(sound_data_t *sound_data)
{
	sample_list_t *p, *p_tmp;

	if( !sound_data ) return;

	// Remove initial nodes with length 0
	p = sound_data->sample_list;
	while( p && p->sample_len <= 0 ) {
		sound_data->sample_list = p->next;
		free(p);
		sound_data->pending_samples--;
		p = sound_data->sample_list;
	}

	// Look through the rest
	p = sound_data->sample_list;
	while( p ) {
		if( p->next && p->next->sample_len <= 0 ) {
			p_tmp = p->next;
			p->next = p->next->next;
			free(p_tmp);
			sound_data->pending_samples--;
		}

		p = p->next;
	}
}



sound_t *init_sound(void)
{
	sound_t *sound = NULL;
	SDL_AudioSpec *waveSpec = NULL;
	sound_data_t *sound_data = NULL;

	// Initialize Audio
	sound = malloc(sizeof(sound_t));
	waveSpec = malloc(sizeof(SDL_AudioSpec));
	sound_data = malloc(sizeof(sound_data_t));

	if( !sound ) {
		fprintf(stderr, "Could not malloc sound context.\n");
		exit(1);
	}
	if( !waveSpec ) {
		fprintf(stderr, "Could not malloc audio spec.\n");
		exit(1);
	}
	if( !sound_data ) {
		fprintf(stderr, "Could not malloc callback data.\n");
		exit(1);
	}

	memset(waveSpec, 0, sizeof(SDL_AudioSpec));
	memset(sound_data, 0, sizeof(sound_data_t));

	waveSpec->freq = SOUND_FREQ;
	waveSpec->format = AUDIO_S8;
	waveSpec->channels = 1;
	waveSpec->samples = SOUND_SAMPLES;
	waveSpec->callback = sample_list_callback;
	waveSpec->userdata = sound_data;

	sound_data->mutex = SDL_CreateMutex();

	if( SDL_OpenAudio(waveSpec, NULL) < 0 ) {
		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
		free(sound_data);
		free(waveSpec);
		free(sound);
		return NULL;
	}
	SDL_PauseAudio(0);

	sound->waveSpec = waveSpec;
	sound->sound_data = sound_data;

	return sound;
}

void close_sound(sound_t *sound)
{
	sample_list_t *p;

	if( !sound ) {
		return;
	}

	SDL_CloseAudio();
	SDL_DestroyMutex(sound->sound_data->mutex);
	p = sound->sound_data->sample_list;
	while( p ) {
		p = p->next;
		free(sound->sound_data->sample_list);
		sound->sound_data->sample_list = p;
	}
	free(sound->sound_data);
	free(sound->waveSpec);
	free(sound);
}



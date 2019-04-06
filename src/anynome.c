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

    $Id: anynome.c 749 2012-03-11 07:30:08Z stepp $
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
 #include "getopt.h"
#endif

#include "common.h"
#include "anynome.h"
#include "parse.h"
#include "stimulus.h"
#include "plugin.h"
#include "sound.h"

int main(int argc, char *argv[])
{
	int c;
	int video_flags;
	int threads_running = 0;
	int threads_ready = 0;
	Uint32 event_time;
	char config_filename[PATH_MAX] = {0};
	FILE *response_file = NULL;
	config_t *config = NULL;
	stimulus_list_t *stim_p = NULL;
	stimulus_t *stim = NULL;
	sound_t *sound = NULL;
	SDL_Surface *screen = NULL;
	SDL_Event event;
	SDL_mutex *stim_mutex = NULL;
	SDL_cond *stim_cond = NULL;

	// Command line parsing
	
	if( argc == 1 ) {
		show_help();
		exit(1);
	}

	// set defaults
	config = malloc(sizeof(config_t));
	if( !config ) {
		fprintf(stderr, "Could not malloc config.\n");
		exit(1);
	}
	/*
	config->fullscreen = 0;
	config->generate_only = 0;
	config->verbose = 0;
	config->show_config_help = 0;
	config->num_stimuli = 0;
	*/
	memset(config, 0, sizeof(config_t));

	while( (c = getopt(argc, argv, "fgtvVhHc:s:r:d:")) != -1 ) {
		switch(c) {
			case 'c':
				strncpy(config_filename, optarg, PATH_MAX);
				break;
			case 's':
				strncpy(config->stimulus_file_base, optarg, sizeof(config->stimulus_file_base)-1);
				break;
			case 'r':
				response_file = fopen(optarg, "w");
				if( !response_file ) {
					perror("response file open");
					exit(1);
				}
				break;
			case 'd':
				strncpy(config->plugin_dir, optarg, sizeof(config->plugin_dir)-1);
				break;
			case 'f':
				config->fullscreen = 1;
				break;
			case 'g':
				config->generate_only = 1;
				break;
			case 't':
				timer_test();
				exit(0);
				break;
			case 'v':
				config->verbose = 1;
				break;
			case 'V':
				show_version();
				exit(0);
				break;
			case 'H':
				config->show_config_help = 1;
				config_show_help();
				break;
			case '?':
			case 'h':
			default:
				show_help();
				exit(0);
		}
	}


	// Read in stim info from config file
	config_parse(config,config_filename);

	// After the plugins have shown help
	// we can exit.
	if( config->show_config_help ) {
		exit(0);
	}

	if( !response_file ) {
		fprintf(stderr, "You must specify a response output file.\n");
		exit(1);
	}
	
	// Initialize Graphics
	if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	if( !config->generate_only ) {
		// Initialize Audio
		sound = init_sound();

		// Initialize font library
		TTF_Init();
		atexit(TTF_Quit);

		// Create a screen to draw on
		video_flags = SDL_SWSURFACE;
		if( config->fullscreen ) {
			video_flags |= SDL_FULLSCREEN;
		}
		screen = SDL_SetVideoMode(800, 600, 16, video_flags);
		if ( screen == NULL ) {
			fprintf(stderr, "Unable to set 800x600 video: %s\n", SDL_GetError());
			exit(1);
		}

		SDL_WM_SetCaption("Anynome", "Anynome");
	}

	// Create child processes
	//
	// Stimulus threads
	
	// Stop threads from starting until all have been created.
	stim_mutex = SDL_CreateMutex();
	stim_cond = SDL_CreateCond();

	threads_running = 0;
	threads_ready = 0;

	stim_p = config->stimulus_list;
	while( stim_p ) {
		stim = stim_p->stim;

		if( config->verbose ) {
			printf("Starting '%s' thread...\n", stim->plugin);
		}

		stim->done = 0;
		stim->stim_mutex = stim_mutex;
		stim->stim_cond = stim_cond;
		stim->screen = screen;
		stim->sound = sound;
		stim->verbose = config->verbose;

		if( stim->audio ) {
			stim->audio_data = generate_sine_wave(stim->audio_frequency,
					stim->sound->waveSpec->freq,
					stim->audio_duration,
					&(stim->audio_data_len)
			);
		}

		stim_p->stim->get_delay_series = plugin_get_fcn(stim_p->stim->plugin_dl, "get_delay_series");
		stim_p->stim->thread = SDL_CreateThread( stimulus_thread, (void *)(stim_p->stim) );
		if ( stim_p->stim->thread == NULL ) {
			fprintf(stderr, "Unable to create stimulus thread: %s\n", SDL_GetError());
			exit(1);
		}

		threads_running++;
		stim_p = stim_p->next;
	}

	if( config->verbose ) {
		printf("Started %d threads.\n", threads_running);
	}

	// Event Loop
	while( threads_running ) {
		if( !SDL_PollEvent(&event) ) {
			usleep(1000);
			continue;
		}
		event_time = SDL_GetTicks();
		switch( event.type ) {
			case SDL_KEYDOWN:
				if( event.key.keysym.sym == SDLK_q ) {
					fprintf(response_file, "User hit 'q' at %u\n", event_time);
					if( config->verbose ) {
						printf("%u: User quit, stopping all threads...\n", event_time);
					}
					stop_stimuli(config->stimulus_list);
				} else {
					fprintf(response_file, "%u\n", event_time);
					if( config->verbose ) {
						printf("User keypress at %u.\n", event_time);
					}
				}

				break;
			case SDL_USEREVENT:
				switch( event.user.code ) {
					case USER_EVT_THREADREADY:
						threads_ready++;
						if( config->verbose ) {
							printf("%u: %d threads are ready to begin.\n", event_time, threads_ready);
						}
						if( threads_ready == threads_running ) {
							if( config->verbose ) {
								printf("%u: All threads are ready to start, broadcasting go ahead.\n",event_time);
							}
							// Give threads signal to go ahead...
							if( SDL_CondBroadcast(stim_cond) != 0 ) {
								fprintf(stderr, "Error broadcasting to stimulus threads: %s\n", SDL_GetError());
								exit(1);
							}
						}
						break;
					case USER_EVT_THREADFINISH:
						threads_running--;
						if( config->verbose ) {
							printf("%u: A thread finished, %d remaining.\n", event_time, threads_running);
						}
						break;
					default:
						fprintf(stderr, "Unknown user event code: %d\n", event.user.code);
						break;
				}
				break;
			case SDL_QUIT:
				fprintf(stderr, "%u: Got a QUIT event, stopping all stimuli...\n", event_time);
				stop_stimuli(config->stimulus_list);
				break;
		}
	}

	
	// Wait for the children to close
	stim_p = config->stimulus_list;
	while( stim_p ) {
		if( config->verbose ) {
			printf("Waiting for thread to finish....\n");
		}
		SDL_WaitThread( stim_p->stim->thread, NULL );

		stim_p = stim_p->next;
	}


	// Clean up
	SDL_DestroyMutex(stim_mutex);
	SDL_DestroyCond(stim_cond);

	// XXX this goes to config_free
	stim_p = config->stimulus_list;
	while( stim_p ) {
		plugin_unload(stim_p->stim->plugin_dl);
		fclose(stim_p->stim->stimulus_file);

		if( stim_p->stim->audio_data ) {
			free(stim_p->stim->audio_data);
		}
		
		free(stim_p->stim);

		stim_p = stim_p->next;
		free(config->stimulus_list);
		config->stimulus_list = stim_p;
	}

	fclose(response_file);
	close_sound(sound);
	free(config);

	return 0;
}

void stop_stimuli(stimulus_list_t *stimuli)
{
	while( stimuli ) {
		if( stimuli->stim ) {
			if( stimuli->stim->verbose ) {
				printf("Telling thread it's done.\n");
			}
			stimuli->stim->done = 1;
		}

		stimuli = stimuli->next;
	}
}

void timer_test()
{
	int i;
	Uint32 off;
	Uint32 ticks[100];
	Uint32 *tick;
	SDL_Thread *thread;
	SDL_Event event;
	SDL_Surface *screen;

	if ( SDL_Init(0) < 0 ) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	// We need this for events
	screen = SDL_SetVideoMode(80, 60, 16, SDL_SWSURFACE);
	if ( screen == NULL ) {
		fprintf(stderr, "Unable to set 800x600 video: %s\n", SDL_GetError());
		exit(1);
	}
	/* 
	 * Delay Timing
	 */
	printf("Testing 100ms delay resolution....\n");
	
	tick = ticks;
	for(i=0; i<100; i++) {
		usleep(100000);
		*(tick++) = SDL_GetTicks();
	}
	off = 0;
	for(i=1; i<100; i++) {
		off += ticks[i]-ticks[i-1] - 100;
	}
	printf("  Mean deviation after 100 delays: %fms\n", ((double)off)/99.0);

	printf("Testing 10ms delay resolution....\n");
	
	tick = ticks;
	for(i=0; i<100; i++) {
		usleep(10000);
		*(tick++) = SDL_GetTicks();
	}
	off = 0;
	for(i=1; i<100; i++) {
		off += ticks[i]-ticks[i-1] - 10;
	}
	printf("  Mean deviation after 100 delays: %fms\n", ((double)off)/99.0);

	printf("Testing 1ms delay resolution....\n");

	tick = ticks;
	for(i=0; i<100; i++) {
		usleep(1000);
		*(tick++) = SDL_GetTicks();
	}
	off = 0;
	for(i=1; i<100; i++) {
		off += ticks[i]-ticks[i-1] - 1;
	}
	printf("  Mean deviation after 100 delays: %fms\n", ((double)off)/99.0);

	/*
	 * Event Timing
	 */
	printf("\nTesting event delay....\n");
	thread = SDL_CreateThread(event_thread, NULL);
	tick = ticks;
	for(i=0; i<100; ) {
		if( SDL_PollEvent(&event) && event.type == SDL_USEREVENT ) {
			*(tick++) = SDL_GetTicks()-event.user.code;
			i++;
		}
		usleep(1000);
	}
	if( !i ) {
		printf("i==0! (%s)\n", SDL_GetError());
		exit(1);
	}

	off=0;
	for(i=0; i<100; i++) {
		off += ticks[i];
	}
	printf("  Mean delay for 100 events (handled - pushed): %fms\n", ((double)off)/100.0);
	SDL_WaitThread(thread, NULL);
}

int event_thread(void *data)
{
	int i;
	SDL_Event event;

	event.type = SDL_USEREVENT;

	for(i=0; i<100; i++) {
		usleep(50000);
		event.user.code=SDL_GetTicks();
		SDL_PushEvent(&event);
	}

	return 0;
}

void show_help()
{
	printf("anynome -fgtvVhH -c <file> -s <base> -r <file>\n\n");
	printf("  -f       \tUse a fullscreen video mode (default: off)\n");
	printf("  -g       \tOnly generate time series, do not display (default: off)\n");
	printf("  -t       \tTiming tester\n");
	printf("  -v       \tBe verbose\n");
	printf("  -V       \tShow version numbers\n");
	printf("  -h       \tHelp\n");
	printf("  -H       \tShow stimulus config file help and exit\n");
	printf("  -c file  \tStimulus config file\n");
	printf("  -s base  \tStimulus output file base name (file will be base-#-plugin.txt)\n");
	printf("  -r file  \tWrite keypress (response) times to <file>\n");
	printf("\n");
}

void show_version()
{
	printf("Anynome version: %s\n", VERSION);
	printf("Plugin API version: %s\n", APIVERSION);
}



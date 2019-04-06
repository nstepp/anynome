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

    $Id: anynome.h 184 2008-03-03 08:20:16Z stepp $
*/


#ifndef _ANYNOME_H
#define _ANYNOME_H

#include "stimulus.h"

#define VERSION "0.10.0"

#ifdef WIN32
 #define usleep(t) (Sleep( (t)/1000 ))
 #define PATH_MAX MAX_PATH
#endif

void stop_stimuli(stimulus_list_t *stimuli);
void timer_test(void);
Uint32 timer_callback(Uint32 interval, void *data);
int event_thread(void *data);
void show_help(void);
void show_version(void);


#endif

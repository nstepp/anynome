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

    $Id: win32_compat.h 189 2008-03-05 22:03:02Z stepp $
*/


#ifndef _WIN32_COMPAT_H
#define _WIN32_COMPAT_H

#ifdef WIN32
 #define usleep(t) (Sleep( (t)/1000 ))
 #define PATH_MAX MAX_PATH
 #define snprintf _snprintf
 #define strdup _strdup
 #define strcasecmp(a,b) _stricmp(a,b)
 #define S_ISREG(a) ((a) & _S_IFREG)


 // Silence warnings for things we don't fix
 #define _CRT_SECURE_NO_DEPRECATE
 #define _CRT_SECURE_NO_WARNINGS

 #if __STDC_SECURE_LIB__
  // Define things which don't require real code changes
  #define strncpy(a,b,c) strncpy_s(a,c,b,c)
  #define _snprintf(a,b,c,...) _snprintf_s(a,b,b,c,__VA_ARGS__)

  // Outstanding functions:
  // These require changes to code logic, or require extra
  // variables to be defined.
  // fopen_s - FILE is an argument rather than a return value
  // strtok_s - requires an additional pointer as a context var
 #endif
#endif

#endif /* _WIN32_COMPAT_H */


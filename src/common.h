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

    $Id: common.h 188 2008-03-05 21:59:36Z stepp $
*/


#ifndef _COMMON_H
#define _COMMON_H

#if defined( WIN32 )
 #include "compat/win32_compat.h"
#elif defined( __APPLE__  )
 #include "compat/osx_compat.h"
#endif

#endif /* _COMMON_H */


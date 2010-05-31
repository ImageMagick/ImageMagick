/* lt__alloc.h -- internal memory management interface

   Copyright (C) 2004 Free Software Foundation, Inc.
   Written by Gary V. Vaughan, 2004

   NOTE: The canonical source of this file is maintained with the
   GNU Libtool package.  Report bugs to bug-libtool@gnu.org.

GNU Libltdl is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

As a special exception to the GNU Lesser General Public License,
if you distribute this file as part of a program or library that
is built using GNU Libtool, you may include this file under the
same distribution terms that you use for the rest of that program.

GNU Libltdl is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with GNU Libltdl; see the file COPYING.LIB.  If not, a
copy can be downloaded from http://www.gnu.org/licenses/lgpl.html,
or obtained by writing to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#if !defined(LT__ALLOC_H)
#define LT__ALLOC_H 1

#include "lt_system.h"

LT_BEGIN_C_DECLS

#define MALLOC(tp, n)		(tp*) lt__malloc((n) * sizeof(tp))
#define REALLOC(tp, mem, n)	(tp*) lt__realloc((mem), (n) * sizeof(tp))
#define FREE(mem)					LT_STMT_START {	\
	if (mem) { free ((void *)mem); mem = NULL; }	} LT_STMT_END
#define MEMREASSIGN(p, q)				LT_STMT_START {	\
	if ((p) != (q)) { if (p) free (p); (p) = (q); (q) = 0; }	\
								} LT_STMT_END

/* If set, this function is called when memory allocation has failed.  */
LT_SCOPE void (*lt__alloc_die) (void);

LT_SCOPE void *lt__malloc (size_t n);
LT_SCOPE void *lt__zalloc (size_t n);
LT_SCOPE void *lt__realloc (void *mem, size_t n);
LT_SCOPE void *lt__memdup (void const *mem, size_t n);

LT_SCOPE char *lt__strdup (const char *string);

LT_END_C_DECLS

#endif /*!defined(LT__ALLOC_H)*/

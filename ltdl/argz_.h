/* lt__argz.h -- internal argz interface for non-glibc systems

   Copyright (C) 2004, 2007, 2008 Free Software Foundation, Inc.
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

#if !defined(LT__ARGZ_H)
#define LT__ARGZ_H 1

#include <stdlib.h>
#define __need_error_t
#include <errno.h>
#include <sys/types.h>

#if defined(LTDL)
#  include "lt__glibc.h"
#  include "lt_system.h"
#else
#  define LT_SCOPE
#endif

#if defined(__cplusplus)
extern "C" {
#endif

LT_SCOPE error_t argz_append	(char **pargz, size_t *pargz_len,
				 const char *buf, size_t buf_len);
LT_SCOPE error_t argz_create_sep(const char *str, int delim,
				 char **pargz, size_t *pargz_len);
LT_SCOPE error_t argz_insert	(char **pargz, size_t *pargz_len,
				 char *before, const char *entry);
LT_SCOPE char *	 argz_next	(char *argz, size_t argz_len,
				 const char *entry);
LT_SCOPE void	 argz_stringify	(char *argz, size_t argz_len, int sep);

#if defined(__cplusplus)
}
#endif

#if !defined(LTDL)
#  undef LT_SCOPE
#endif

#endif /*!defined(LT__ARGZ_H)*/

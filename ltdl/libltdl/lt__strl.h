/* lt__strl.h -- size-bounded string copying and concatenation

   Copyright (C) 2004, 2006 Free Software Foundation, Inc.
   Written by Bob Friesenhahn, 2004

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
copy can be downloaded from  http://www.gnu.org/licenses/lgpl.html,
or obtained by writing to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#if !defined(LT__STRL_H)
#define LT__STRL_H 1

#if defined(LT_CONFIG_H)
#  include LT_CONFIG_H
#else
#  include <config.h>
#endif

#include <string.h>
#include "lt_system.h"

#if !defined(HAVE_STRLCAT)
#  define strlcat(dst,src,dstsize) lt_strlcat(dst,src,dstsize)
LT_SCOPE size_t lt_strlcat(char *dst, const char *src, const size_t dstsize);
#endif /* !defined(HAVE_STRLCAT) */

#if !defined(HAVE_STRLCPY)
#  define strlcpy(dst,src,dstsize) lt_strlcpy(dst,src,dstsize)
LT_SCOPE size_t lt_strlcpy(char *dst, const char *src, const size_t dstsize);
#endif /* !defined(HAVE_STRLCPY) */

#endif /*!defined(LT__STRL_H)*/

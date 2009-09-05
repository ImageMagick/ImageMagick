/* lt__glibc.h -- support for non glibc environments

   Copyright (C) 2004, 2006, 2007 Free Software Foundation, Inc.
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

#if !defined(LT__GLIBC_H)
#define LT__GLIBC_H 1

#if defined(LT_CONFIG_H)
#  include LT_CONFIG_H
#else
#  include <config.h>
#endif

#if !defined(HAVE_ARGZ_H) || !defined(HAVE_WORKING_ARGZ)
/* Redefine any glibc symbols we reimplement to import the
   implementations into our lt__ namespace so we don't ever
   clash with the system library if our clients use argz_*
   from there in addition to libltdl.  */
#  undef  argz_append
#  define argz_append		lt__argz_append
#  undef  argz_create_sep
#  define argz_create_sep	lt__argz_create_sep
#  undef  argz_insert
#  define argz_insert		lt__argz_insert
#  undef  argz_next
#  define argz_next		lt__argz_next
#  undef  argz_stringify
#  define argz_stringify	lt__argz_stringify
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <argz.h>

#ifdef __cplusplus
}
#endif

# define slist_concat	lt__slist_concat
# define slist_cons	lt__slist_cons
# define slist_delete	lt__slist_delete
# define slist_remove	lt__slist_remove
# define slist_reverse	lt__slist_reverse
# define slist_sort	lt__slist_sort
# define slist_tail	lt__slist_tail
# define slist_nth	lt__slist_nth
# define slist_find	lt__slist_find
# define slist_length	lt__slist_length
# define slist_foreach	lt__slist_foreach
# define slist_box	lt__slist_box
# define slist_unbox	lt__slist_unbox

#include <slist.h>

#endif /*!defined(LT__GLIBC_H)*/

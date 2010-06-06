/* slist.h -- generalised singly linked lists

   Copyright (C) 2000, 2004, 2009 Free Software Foundation, Inc.
   Written by Gary V. Vaughan, 2000

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

/* A generalised list.  This is deliberately transparent so that you
   can make the NEXT field of all your chained data structures first,
   and then cast them to `(SList *)' so that they can be manipulated
   by this API.

   Alternatively, you can generate raw SList elements using slist_new(),
   and put the element data in the USERDATA field.  Either way you
   get to manage the memory involved by yourself.
*/

#if !defined(SLIST_H)
#define SLIST_H 1

#if defined(LTDL)
#  include <libltdl/lt__glibc.h>
#  include <libltdl/lt_system.h>
#else
#  define LT_SCOPE
#endif

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct slist {
  struct slist *next;		/* chain forward pointer*/
  const void *userdata;		/* for boxed `SList' item */
} SList;

typedef void *	SListCallback	(SList *item, void *userdata);
typedef int	SListCompare	(const SList *item1, const SList *item2,
				 void *userdata);

LT_SCOPE SList *slist_concat	(SList *head, SList *tail);
LT_SCOPE SList *slist_cons	(SList *item, SList *slist);

LT_SCOPE SList *slist_delete	(SList *slist, void (*delete_fct) (void *item));
LT_SCOPE SList *slist_remove	(SList **phead, SListCallback *find,
				 void *matchdata);
LT_SCOPE SList *slist_reverse	(SList *slist);
LT_SCOPE SList *slist_sort	(SList *slist, SListCompare *compare,
				 void *userdata);

LT_SCOPE SList *slist_tail	(SList *slist);
LT_SCOPE SList *slist_nth	(SList *slist, size_t n);
LT_SCOPE void *	slist_find	(SList *slist, SListCallback *find,
				 void *matchdata);
LT_SCOPE size_t slist_length	(SList *slist);

LT_SCOPE void *	slist_foreach   (SList *slist, SListCallback *foreach,
				 void *userdata);

LT_SCOPE SList *slist_box	(const void *userdata);
LT_SCOPE void *	slist_unbox	(SList *item);

#if defined(__cplusplus)
}
#endif

#if !defined(LTDL)
#  undef LT_SCOPE
#endif

#endif /*!defined(SLIST_H)*/

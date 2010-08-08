/* slist.c -- generalised singly linked lists

   Copyright (C) 2000, 2004, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include <assert.h>

#include "slist.h"
#include <stddef.h>
#include <stdlib.h>

static SList *	slist_sort_merge    (SList *left, SList *right,
				     SListCompare *compare, void *userdata);


/* Call DELETE repeatedly on each element of HEAD.

   CAVEAT: If you call this when HEAD is the start of a list of boxed
           items, you must remember that each item passed back to your
	   DELETE function will be a boxed item that must be slist_unbox()ed
	   before operating on its contents.

   e.g. void boxed_delete (void *item) { item_free (slist_unbox (item)); }
        ...
	  slist = slist_delete (slist, boxed_delete);
	...
*/
SList *
slist_delete (SList *head, void (*delete_fct) (void *item))
{
  assert (delete_fct);

  while (head)
    {
      SList *next = head->next;
      (*delete_fct) (head);
      head = next;
    }

  return 0;
}

/* Call FIND repeatedly with MATCHDATA and each item of *PHEAD, until
   FIND returns non-NULL, or the list is exhausted.  If a match is found
   the matching item is destructively removed from *PHEAD, and the value
   returned by the matching call to FIND is returned.

   CAVEAT: To avoid memory leaks, unless you already have the address of
           the stale item, you should probably return that from FIND if
	   it makes a successful match.  Don't forget to slist_unbox()
	   every item in a boxed list before operating on its contents.   */
SList *
slist_remove (SList **phead, SListCallback *find, void *matchdata)
{
  SList *stale = 0;
  void *result = 0;

  assert (find);

  if (!phead || !*phead)
    return 0;

  /* Does the head of the passed list match? */
  result = (*find) (*phead, matchdata);
  if (result)
    {
      stale = *phead;
      *phead = stale->next;
    }
  /* what about the rest of the elements? */
  else
    {
      SList *head;
      for (head = *phead; head->next; head = head->next)
	{
	  result = (*find) (head->next, matchdata);
	  if (result)
	    {
	      stale		= head->next;
	      head->next	= stale->next;
	      break;
	    }
	}
    }

  return (SList *) result;
}

/* Call FIND repeatedly with each element of SLIST and MATCHDATA, until
   FIND returns non-NULL, or the list is exhausted.  If a match is found
   the value returned by the matching call to FIND is returned. */
void *
slist_find (SList *slist, SListCallback *find, void *matchdata)
{
  void *result = 0;

  assert (find);

  for (; slist; slist = slist->next)
    {
      result = (*find) (slist, matchdata);
      if (result)
	break;
    }

  return result;
}

/* Return a single list, composed by destructively concatenating the
   items in HEAD and TAIL.  The values of HEAD and TAIL are undefined
   after calling this function.

   CAVEAT: Don't mix boxed and unboxed items in a single list.

   e.g.  slist1 = slist_concat (slist1, slist2);  */
SList *
slist_concat (SList *head, SList *tail)
{
  SList *last;

  if (!head)
    {
      return tail;
    }

  last = head;
  while (last->next)
    last = last->next;

  last->next = tail;

  return head;
}

/* Return a single list, composed by destructively appending all of
   the items in SLIST to ITEM.  The values of ITEM and SLIST are undefined
   after calling this function.

   CAVEAT:  Don't mix boxed and unboxed items in a single list.

   e.g.  slist1 = slist_cons (slist_box (data), slist1);  */
SList *
slist_cons (SList *item, SList *slist)
{
  if (!item)
    {
      return slist;
    }

  assert (!item->next);

  item->next = slist;
  return item;
}

/* Return a list starting at the second item of SLIST.  */
SList *
slist_tail (SList *slist)
{
  return slist ? slist->next : NULL;
}

/* Return a list starting at the Nth item of SLIST.  If SLIST is less
   than N items long, NULL is returned.  Just to be confusing, list items
   are counted from 1, to get the 2nd element of slist:

   e.g. shared_list = slist_nth (slist, 2);  */
SList *
slist_nth (SList *slist, size_t n)
{
  for (;n > 1 && slist; n--)
    slist = slist->next;

  return slist;
}

/* Return the number of items in SLIST.  We start counting from 1, so
   the length of a list with no items is 0, and so on.  */
size_t
slist_length (SList *slist)
{
  size_t n;

  for (n = 0; slist; ++n)
    slist = slist->next;

  return n;
}

/* Destructively reverse the order of items in SLIST.  The value of SLIST
   is undefined after calling this function.

  CAVEAT: You must store the result of this function, or you might not
          be able to get all the items except the first one back again.

  e.g.    slist = slist_reverse (slist);  */
SList *
slist_reverse (SList *slist)
{
  SList *result = 0;
  SList *next;

  while (slist)
    {
      next		= slist->next;
      slist->next	= result;
      result		= slist;
      slist 		= next;
    }

  return result;
}

/* Call FOREACH once for each item in SLIST, passing both the item and
   USERDATA on each call. */
void *
slist_foreach (SList *slist, SListCallback *foreach, void *userdata)
{
  void *result = 0;

  assert (foreach);

  while (slist)
    {
      SList *next = slist->next;
      result = (*foreach) (slist, userdata);

      if (result)
	break;

      slist = next;
    }

  return result;
}

/* Destructively merge the items of two ordered lists LEFT and RIGHT,
   returning a single sorted list containing the items of both --  Part of
   the quicksort algorithm.  The values of LEFT and RIGHT are undefined
   after calling this function.

   At each iteration, add another item to the merged list by taking the
   lowest valued item from the head of either LEFT or RIGHT, determined
   by passing those items and USERDATA to COMPARE.  COMPARE should return
   less than 0 if the head of LEFT has the lower value, greater than 0 if
   the head of RIGHT has the lower value, otherwise 0.  */
static SList *
slist_sort_merge (SList *left, SList *right, SListCompare *compare,
		  void *userdata)
{
  SList merged, *insert;

  insert = &merged;

  while (left && right)
    {
      if ((*compare) (left, right, userdata) <= 0)
	{
	  insert = insert->next = left;
	  left = left->next;
	}
      else
	{
	  insert = insert->next = right;
	  right = right->next;
	}
    }

  insert->next = left ? left : right;

  return merged.next;
}

/* Perform a destructive quicksort on the items in SLIST, by repeatedly
   calling COMPARE with a pair of items from SLIST along with USERDATA
   at every iteration.  COMPARE is a function as defined above for
   slist_sort_merge().  The value of SLIST is undefined after calling
   this function.

   e.g.  slist = slist_sort (slist, compare, 0);  */
SList *
slist_sort (SList *slist, SListCompare *compare, void *userdata)
{
  SList *left, *right;

  if (!slist)
    return slist;

  /* Be sure that LEFT and RIGHT never contain the same item.  */
  left = slist;
  right = slist->next;

  if (!right)
    return left;

  /* Skip two items with RIGHT and one with SLIST, until RIGHT falls off
     the end.  SLIST must be about half way along.  */
  while (right && (right = right->next))
    {
      if (!right || !(right = right->next))
	break;
      slist = slist->next;
    }
  right = slist->next;
  slist->next = 0;

  /* Sort LEFT and RIGHT, then merge the two.  */
  return slist_sort_merge (slist_sort (left, compare, userdata),
			   slist_sort (right, compare, userdata),
			   compare, userdata);
}


/* Aside from using the functions above to manage chained structures of
   any type that has a NEXT pointer as its first field, SLISTs can
   be comprised of boxed items.  The boxes are chained together in
   that case, so there is no need for a NEXT field in the item proper.
   Some care must be taken to slist_box and slist_unbox each item in
   a boxed list at the appropriate points to avoid leaking the memory
   used for the boxes.  It us usually a very bad idea to mix boxed and
   non-boxed items in a single list.  */

/* Return a `boxed' freshly mallocated 1 element list containing
   USERDATA.  */
SList *
slist_box (const void *userdata)
{
  SList *item = (SList *) malloc (sizeof *item);

  if (item)
    {
      item->next     = 0;
      item->userdata = userdata;
    }

  return item;
}

/* Return the contents of a `boxed' ITEM, recycling the box itself.  */
void *
slist_unbox (SList *item)
{
  void *userdata = 0;

  if (item)
    {
      /* Strip the const, because responsibility for this memory
	 passes to the caller on return.  */
      userdata = (void *) item->userdata;
      free (item);
    }

  return userdata;
}

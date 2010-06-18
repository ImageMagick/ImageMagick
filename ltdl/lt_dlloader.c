/* lt_dlloader.c -- dynamic library loader interface

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
copy can be downloaded from  http://www.gnu.org/licenses/lgpl.html,
or obtained by writing to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "lt__private.h"
#include "lt_dlloader.h"

#define RETURN_SUCCESS 0
#define RETURN_FAILURE 1

static void *	loader_callback (SList *item, void *userdata);

/* A list of all the dlloaders we know about, each stored as a boxed
   SList item:  */
static	SList    *loaders		= 0;


/* Return NULL, unless the loader in this ITEM has a matching name,
   in which case we return the matching item so that its address is
   passed back out (for possible freeing) by slist_remove.  */
static void *
loader_callback (SList *item, void *userdata)
{
  const lt_dlvtable *vtable = (const lt_dlvtable *) item->userdata;
  const char *	    name    = (const char *) userdata;

  assert (vtable);

  return streq (vtable->name, name) ? (void *) item : NULL;
}


/* Hook VTABLE into our global LOADERS list according to its own
   PRIORITY field value.  */
int
lt_dlloader_add (const lt_dlvtable *vtable)
{
  SList *item;

  if ((vtable == 0)	/* diagnose invalid vtable fields */
      || (vtable->module_open == 0)
      || (vtable->module_close == 0)
      || (vtable->find_sym == 0)
      || ((vtable->priority != LT_DLLOADER_PREPEND) &&
	  (vtable->priority != LT_DLLOADER_APPEND)))
    {
      LT__SETERROR (INVALID_LOADER);
      return RETURN_FAILURE;
    }

  item = slist_box (vtable);
  if (!item)
    {
      (*lt__alloc_die) ();

      /* Let the caller know something went wrong if lt__alloc_die
	 doesn't abort.  */
      return RETURN_FAILURE;
    }

  if (vtable->priority == LT_DLLOADER_PREPEND)
    {
      loaders = slist_cons (item, loaders);
    }
  else
    {
      assert (vtable->priority == LT_DLLOADER_APPEND);
      loaders = slist_concat (loaders, item);
    }

  return RETURN_SUCCESS;
}

#ifdef LT_DEBUG_LOADERS
static void *
loader_dump_callback (SList *item, void *userdata)
{
  const lt_dlvtable *vtable = (const lt_dlvtable *) item->userdata;
  fprintf (stderr, ", %s", (vtable && vtable->name) ? vtable->name : "(null)");
  return 0;
}

void
lt_dlloader_dump (void)
{
  fprintf (stderr, "loaders: ");
  if (!loaders)
    {
      fprintf (stderr, "(empty)");
    }
  else
    {
      const lt_dlvtable *head = (const lt_dlvtable *) loaders->userdata;
      fprintf (stderr, "%s", (head && head->name) ? head->name : "(null)");
      if (slist_tail (loaders))
	slist_foreach (slist_tail (loaders), loader_dump_callback, NULL);
    }
  fprintf (stderr, "\n");
}
#endif

/* An iterator for the global loader list: if LOADER is NULL, then
   return the first element, otherwise the following element.  */
lt_dlloader
lt_dlloader_next (lt_dlloader loader)
{
  SList *item = (SList *) loader;
  return (lt_dlloader) (item ? item->next : loaders);
}


/* Non-destructive unboxing of a loader.  */
const lt_dlvtable *
lt_dlloader_get	(lt_dlloader loader)
{
  return (const lt_dlvtable *) (loader ? ((SList *) loader)->userdata : NULL);
}


/* Return the contents of the first item in the global loader list
   with a matching NAME after removing it from that list.  If there
   was no match, return NULL; if there is an error, return NULL and
   set an error for lt_dlerror; do not set an error if only resident
   modules need this loader; in either case, the loader list is not
   changed if NULL is returned.  */
lt_dlvtable *
lt_dlloader_remove (char *name)
{
  const lt_dlvtable *	vtable	= lt_dlloader_find (name);
  static const char	id_string[] = "lt_dlloader_remove";
  lt_dlinterface_id	iface;
  lt_dlhandle		handle = 0;
  int			in_use = 0;
  int			in_use_by_resident = 0;

  if (!vtable)
    {
      LT__SETERROR (INVALID_LOADER);
      return 0;
    }

  /* Fail if there are any open modules which use this loader.  */
  iface = lt_dlinterface_register (id_string, NULL);
  while ((handle = lt_dlhandle_iterate (iface, handle)))
    {
      lt_dlhandle cur = handle;
      if (cur->vtable == vtable)
	{
	  in_use = 1;
	  if (lt_dlisresident (handle))
	    in_use_by_resident = 1;
	}
    }
  lt_dlinterface_free (iface);
  if (in_use)
    {
      if (!in_use_by_resident)
	LT__SETERROR (REMOVE_LOADER);
      return 0;
    }

  /* Call the loader finalisation function.  */
  if (vtable && vtable->dlloader_exit)
    {
      if ((*vtable->dlloader_exit) (vtable->dlloader_data) != 0)
	{
	  /* If there is an exit function, and it returns non-zero
	     then it must set an error, and we will not remove it
	     from the list.  */
	  return 0;
	}
    }

  /* If we got this far, remove the loader from our global list.  */
  return (lt_dlvtable *)
      slist_unbox ((SList *) slist_remove (&loaders, loader_callback, name));
}


const lt_dlvtable *
lt_dlloader_find (char *name)
{
  return lt_dlloader_get (slist_find (loaders, loader_callback, name));
}

/* loader-shl_load.c --  dynamic linking with shl_load (HP-UX)

   Copyright (C) 1998, 1999, 2000, 2004, 2006,
                 2007, 2008 Free Software Foundation, Inc.
   Written by Thomas Tanner, 1998

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

/* Use the preprocessor to rename non-static symbols to avoid namespace
   collisions when the loader code is statically linked into libltdl.
   Use the "<module_name>_LTX_" prefix so that the symbol addresses can
   be fetched from the preloaded symbol list by lt_dlsym():  */
#define get_vtable	shl_load_LTX_get_vtable

LT_BEGIN_C_DECLS
LT_SCOPE lt_dlvtable *get_vtable (lt_user_data loader_data);
LT_END_C_DECLS


/* Boilerplate code to set up the vtable for hooking this loader into
   libltdl's loader list:  */
static int	 vl_exit  (lt_user_data loader_data);
static lt_module vm_open  (lt_user_data loader_data, const char *filename,
                           lt_dladvise advise);
static int	 vm_close (lt_user_data loader_data, lt_module module);
static void *	 vm_sym   (lt_user_data loader_data, lt_module module,
			  const char *symbolname);

static lt_dlvtable *vtable = 0;

/* Return the vtable for this loader, only the name and sym_prefix
   attributes (plus the virtual function implementations, obviously)
   change between loaders.  */
lt_dlvtable *
get_vtable (lt_user_data loader_data)
{
  if (!vtable)
    {
      vtable = lt__zalloc (sizeof *vtable);
    }

  if (vtable && !vtable->name)
    {
      vtable->name		= "lt_shl_load";
      vtable->module_open	= vm_open;
      vtable->module_close	= vm_close;
      vtable->find_sym		= vm_sym;
      vtable->dlloader_exit	= vl_exit;
      vtable->dlloader_data	= loader_data;
      vtable->priority		= LT_DLLOADER_APPEND;
    }

  if (vtable && (vtable->dlloader_data != loader_data))
    {
      LT__SETERROR (INIT_LOADER);
      return 0;
    }

  return vtable;
}



/* --- IMPLEMENTATION --- */


#if defined(HAVE_DL_H)
#  include <dl.h>
#endif

/* some flags are missing on some systems, so we provide
 * harmless defaults.
 *
 * Mandatory:
 * BIND_IMMEDIATE  - Resolve symbol references when the library is loaded.
 * BIND_DEFERRED   - Delay code symbol resolution until actual reference.
 *
 * Optionally:
 * BIND_FIRST	   - Place the library at the head of the symbol search
 * 		     order.
 * BIND_NONFATAL   - The default BIND_IMMEDIATE behavior is to treat all
 * 		     unsatisfied symbols as fatal.  This flag allows
 * 		     binding of unsatisfied code symbols to be deferred
 * 		     until use.
 *		     [Perl: For certain libraries, like DCE, deferred
 *		     binding often causes run time problems. Adding
 *		     BIND_NONFATAL to BIND_IMMEDIATE still allows
 *		     unresolved references in situations like this.]
 * BIND_NOSTART	   - Do not call the initializer for the shared library
 *		     when the library is loaded, nor on a future call to
 *		     shl_unload().
 * BIND_VERBOSE	   - Print verbose messages concerning possible
 *		     unsatisfied symbols.
 *
 * hp9000s700/hp9000s800:
 * BIND_RESTRICTED - Restrict symbols visible by the library to those
 *		     present at library load time.
 * DYNAMIC_PATH	   - Allow the loader to dynamically search for the
 *		     library specified by the path argument.
 */

#if !defined(DYNAMIC_PATH)
#  define DYNAMIC_PATH		0
#endif
#if !defined(BIND_RESTRICTED)
#  define BIND_RESTRICTED	0
#endif

#define	LT_BIND_FLAGS	(BIND_IMMEDIATE | BIND_NONFATAL | DYNAMIC_PATH)


/* A function called through the vtable when this loader is no
   longer needed by the application.  */
static int
vl_exit (lt_user_data LT__UNUSED loader_data)
{
  vtable = NULL;
  return 0;
}

/* A function called through the vtable to open a module with this
   loader.  Returns an opaque representation of the newly opened
   module for processing with this loader's other vtable functions.  */
static lt_module
vm_open (lt_user_data LT__UNUSED loader_data, const char *filename,
         lt_dladvise LT__UNUSED advise)
{
  static shl_t self = (shl_t) 0;
  lt_module module = shl_load (filename, LT_BIND_FLAGS, 0L);

  /* Since searching for a symbol against a NULL module handle will also
     look in everything else that was already loaded and exported with
     the -E compiler flag, we always cache a handle saved before any
     modules are loaded.  */
  if (!self)
    {
      void *address;
      shl_findsym (&self, "main", TYPE_UNDEFINED, &address);
    }

  if (!filename)
    {
      module = self;
    }
  else
    {
      module = shl_load (filename, LT_BIND_FLAGS, 0L);

      if (!module)
	{
	  LT__SETERROR (CANNOT_OPEN);
	}
    }

  return module;
}

/* A function called through the vtable when a particular module
   should be unloaded.  */
static int
vm_close (lt_user_data LT__UNUSED loader_data, lt_module module)
{
  int errors = 0;

  if (module && (shl_unload ((shl_t) (module)) != 0))
    {
      LT__SETERROR (CANNOT_CLOSE);
      ++errors;
    }

  return errors;
}


/* A function called through the vtable to get the address of
   a symbol loaded from a particular module.  */
static void *
vm_sym (lt_user_data LT__UNUSED loader_data, lt_module module, const char *name)
{
  void *address = 0;

  /* sys_shl_open should never return a NULL module handle */
  if (module == (lt_module) 0)
  {
    LT__SETERROR (INVALID_HANDLE);
  }
  else if (!shl_findsym((shl_t*) &module, name, TYPE_UNDEFINED, &address))
    {
      if (!address)
	{
	  LT__SETERROR (SYMBOL_NOT_FOUND);
	}
    }

  return address;
}

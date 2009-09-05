/* loader-dyld.c -- dynamic linking on darwin and OS X

   Copyright (C) 1998, 1999, 2000, 2004, 2006,
                 2007, 2008 Free Software Foundation, Inc.
   Written by Peter O'Gorman, 1998

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
#define get_vtable	dyld_LTX_get_vtable

LT_BEGIN_C_DECLS
LT_SCOPE lt_dlvtable *get_vtable (lt_user_data loader_data);
LT_END_C_DECLS


/* Boilerplate code to set up the vtable for hooking this loader into
   libltdl's loader list:  */
static int	 vl_init  (lt_user_data loader_data);
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
      vtable->name		= "lt_dyld";
      vtable->sym_prefix	= "_";
      vtable->dlloader_init	= vl_init;
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


#if defined(HAVE_MACH_O_DYLD_H)
#  if !defined(__APPLE_CC__) && !defined(__MWERKS__) && !defined(__private_extern__)
  /* Is this correct? Does it still function properly? */
#    define __private_extern__ extern
#  endif
#  include <mach-o/dyld.h>
#endif

#include <mach-o/getsect.h>

/* We have to put some stuff here that isn't in older dyld.h files */
#if !defined(ENUM_DYLD_BOOL)
# define ENUM_DYLD_BOOL
# undef FALSE
# undef TRUE
 enum DYLD_BOOL {
    FALSE,
    TRUE
 };
#endif
#if !defined(LC_REQ_DYLD)
# define LC_REQ_DYLD 0x80000000
#endif
#if !defined(LC_LOAD_WEAK_DYLIB)
# define LC_LOAD_WEAK_DYLIB (0x18 | LC_REQ_DYLD)
#endif

#if !defined(NSADDIMAGE_OPTION_NONE)
#  define NSADDIMAGE_OPTION_NONE                          0x0
#endif
#if !defined(NSADDIMAGE_OPTION_RETURN_ON_ERROR)
#  define NSADDIMAGE_OPTION_RETURN_ON_ERROR               0x1
#endif
#if !defined(NSADDIMAGE_OPTION_WITH_SEARCHING)
#  define NSADDIMAGE_OPTION_WITH_SEARCHING                0x2
#endif
#if !defined(NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED)
#  define NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED         0x4
#endif
#if !defined(NSADDIMAGE_OPTION_MATCH_FILENAME_BY_INSTALLNAME)
#  define NSADDIMAGE_OPTION_MATCH_FILENAME_BY_INSTALLNAME 0x8
#endif

#if !defined(NSLOOKUPSYMBOLINIMAGE_OPTION_BIND)
#  define NSLOOKUPSYMBOLINIMAGE_OPTION_BIND               0x0
#endif
#if !defined(NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW)
#  define NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW           0x1
#endif
#if !defined(NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_FULLY)
#  define NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_FULLY         0x2
#endif
#if !defined(NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR)
#  define NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR    0x4
#endif

#define LT__SYMLOOKUP_OPTS	(NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW \
				| NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR)

#if defined(__BIG_ENDIAN__)
#  define LT__MAGIC	MH_MAGIC
#else
#  define LT__MAGIC	MH_CIGAM
#endif

#define DYLD__SETMYERROR(errmsg)    LT__SETERRORSTR (dylderror (errmsg))
#define DYLD__SETERROR(errcode)	    DYLD__SETMYERROR (LT__STRERROR (errcode))

typedef struct mach_header mach_header;
typedef struct dylib_command dylib_command;

static const char *dylderror (const char *errmsg);
static const mach_header *lt__nsmodule_get_header (NSModule module);
static const char *lt__header_get_instnam (const mach_header *mh);
static const mach_header *lt__match_loadedlib (const char *name);
static NSSymbol lt__linkedlib_symbol (const char *symname, const mach_header *mh);

static const mach_header *(*lt__addimage)	(const char *image_name,
						 unsigned long options) = 0;
static NSSymbol	(*lt__image_symbol)		(const mach_header *image,
						 const char *symbolName,
						 unsigned long options) = 0;
static enum DYLD_BOOL (*lt__image_symbol_p)	(const mach_header *image,
						 const char *symbolName) = 0;
static enum DYLD_BOOL (*lt__module_export)	(NSModule module) = 0;

static int dyld_cannot_close				  = 0;


/* A function called through the vtable when this loader is no
   longer needed by the application.  */
static int
vl_exit (lt_user_data LT__UNUSED loader_data)
{
  vtable = NULL;
  return 0;
}

/* A function called through the vtable to initialise this loader.  */
static int
vl_init (lt_user_data loader_data)
{
  int errors = 0;

  if (! dyld_cannot_close)
    {
      if (!_dyld_present ())
	{
	  ++errors;
	}
      else
	{
	  (void) _dyld_func_lookup ("__dyld_NSAddImage",
				    (unsigned long*) &lt__addimage);
	  (void) _dyld_func_lookup ("__dyld_NSLookupSymbolInImage",
				    (unsigned long*)&lt__image_symbol);
	  (void) _dyld_func_lookup ("__dyld_NSIsSymbolNameDefinedInImage",
				    (unsigned long*) &lt__image_symbol_p);
	  (void) _dyld_func_lookup ("__dyld_NSMakePrivateModulePublic",
				    (unsigned long*) &lt__module_export);
	  dyld_cannot_close = lt_dladderror ("can't close a dylib");
	}
    }

  return errors;
}


/* A function called through the vtable to open a module with this
   loader.  Returns an opaque representation of the newly opened
   module for processing with this loader's other vtable functions.  */
static lt_module
vm_open (lt_user_data loader_data, const char *filename,
         lt_dladvise LT__UNUSED advise)
{
  lt_module module = 0;
  NSObjectFileImage ofi = 0;

  if (!filename)
    {
      return (lt_module) -1;
    }

  switch (NSCreateObjectFileImageFromFile (filename, &ofi))
    {
    case NSObjectFileImageSuccess:
      module = NSLinkModule (ofi, filename, NSLINKMODULE_OPTION_RETURN_ON_ERROR
			     		    | NSLINKMODULE_OPTION_PRIVATE
			     		    | NSLINKMODULE_OPTION_BINDNOW);
      NSDestroyObjectFileImage (ofi);

      if (module)
	{
	  lt__module_export (module);
	}
      break;

    case NSObjectFileImageInappropriateFile:
      if (lt__image_symbol_p && lt__image_symbol)
	{
	  module = (lt_module) lt__addimage(filename,
					    NSADDIMAGE_OPTION_RETURN_ON_ERROR);
	}
      break;

    case NSObjectFileImageFailure:
    case NSObjectFileImageArch:
    case NSObjectFileImageFormat:
    case NSObjectFileImageAccess:
      /*NOWORK*/
      break;
    }

  if (!module)
    {
      DYLD__SETERROR (CANNOT_OPEN);
    }

  return module;
}


/* A function called through the vtable when a particular module
   should be unloaded.  */
static int
vm_close (lt_user_data loader_data, lt_module module)
{
  int errors = 0;

  if (module != (lt_module) -1)
    {
      const mach_header *mh = (const mach_header *) module;
      int flags = 0;
      if (mh->magic == LT__MAGIC)
	{
	  lt_dlseterror (dyld_cannot_close);
	  ++errors;
	}
      else
	{
	  /* Currently, if a module contains c++ static destructors and it
	     is unloaded, we get a segfault in atexit(), due to compiler and
	     dynamic loader differences of opinion, this works around that.  */
	  if ((const struct section *) NULL !=
	      getsectbynamefromheader (lt__nsmodule_get_header (module),
				       "__DATA", "__mod_term_func"))
	    {
	      flags |= NSUNLINKMODULE_OPTION_KEEP_MEMORY_MAPPED;
	    }
#if defined(__ppc__)
	  flags |= NSUNLINKMODULE_OPTION_RESET_LAZY_REFERENCES;
#endif
	  if (!NSUnLinkModule (module, flags))
	    {
	      DYLD__SETERROR (CANNOT_CLOSE);
	      ++errors;
	    }
	}
    }

  return errors;
}

/* A function called through the vtable to get the address of
   a symbol loaded from a particular module.  */
static void *
vm_sym (lt_user_data loader_data, lt_module module, const char *name)
{
  NSSymbol *nssym = 0;
  const mach_header *mh = (const mach_header *) module;
  char saveError[256] = "Symbol not found";

  if (module == (lt_module) -1)
    {
      void *address, *unused;
      _dyld_lookup_and_bind (name, (unsigned long*) &address, &unused);
      return address;
    }

  if (mh->magic == LT__MAGIC)
    {
      if (lt__image_symbol_p && lt__image_symbol)
	{
	  if (lt__image_symbol_p (mh, name))
	    {
	      nssym = lt__image_symbol (mh, name, LT__SYMLOOKUP_OPTS);
	    }
	}

    }
  else
    {
      nssym = NSLookupSymbolInModule (module, name);
    }

  if (!nssym)
    {
      strncpy (saveError, dylderror (LT__STRERROR (SYMBOL_NOT_FOUND)), 255);
      saveError[255] = 0;
      if (!mh)
	{
	  mh = (mach_header *)lt__nsmodule_get_header (module);
	}
      nssym = lt__linkedlib_symbol (name, mh);
    }

  if (!nssym)
    {
      LT__SETERRORSTR (saveError);
    }

  return nssym ? NSAddressOfSymbol (nssym) : 0;
}




/* --- HELPER FUNCTIONS --- */


/* Return the dyld error string, or the passed in error string if none. */
static const char *
dylderror (const char *errmsg)
{
  NSLinkEditErrors ler;
  int lerno;
  const char *file;
  const char *errstr;

  NSLinkEditError (&ler, &lerno, &file, &errstr);

  if (! (errstr && *errstr))
    {
      errstr = errmsg;
    }

  return errstr;
}

/* There should probably be an apple dyld api for this. */
static const mach_header *
lt__nsmodule_get_header (NSModule module)
{
  int i = _dyld_image_count();
  const char *modname = NSNameOfModule (module);
  const mach_header *mh = 0;

  if (!modname)
    return NULL;

  while (i > 0)
    {
      --i;
      if (strneq (_dyld_get_image_name (i), modname))
	{
	  mh = _dyld_get_image_header (i);
	  break;
	}
    }

  return mh;
}

/* NSAddImage is also used to get the loaded image, but it only works if
   the lib is installed, for uninstalled libs we need to check the
   install_names against each other.  Note that this is still broken if
   DYLD_IMAGE_SUFFIX is set and a different lib was loaded as a result.  */
static const char *
lt__header_get_instnam (const mach_header *mh)
{
  unsigned long offset = sizeof(mach_header);
  const char* result   = 0;
  int j;

  for (j = 0; j < mh->ncmds; j++)
    {
      struct load_command *lc;

      lc = (struct load_command*) (((unsigned long) mh) + offset);
      if (LC_ID_DYLIB == lc->cmd)
	{
	  result=(char*)(((dylib_command*) lc)->dylib.name.offset +
			 (unsigned long) lc);
	}
      offset += lc->cmdsize;
    }

  return result;
}

static const mach_header *
lt__match_loadedlib (const char *name)
{
  const mach_header *mh	= 0;
  int i = _dyld_image_count();

  while (i > 0)
    {
      const char *id;

      --i;
      id = lt__header_get_instnam (_dyld_get_image_header (i));
      if (id && strneq (id, name))
	{
	  mh = _dyld_get_image_header (i);
	  break;
	}
    }

  return mh;
}

/* Safe to assume our mh is good. */
static NSSymbol
lt__linkedlib_symbol (const char *symname, const mach_header *mh)
{
  NSSymbol symbol = 0;

  if (lt__image_symbol && NSIsSymbolNameDefined (symname))
    {
      unsigned long offset = sizeof(mach_header);
      struct load_command *lc;
      int j;

      for (j = 0; j < mh->ncmds; j++)
	{
	  lc = (struct load_command*) (((unsigned long) mh) + offset);
	  if ((LC_LOAD_DYLIB == lc->cmd) || (LC_LOAD_WEAK_DYLIB == lc->cmd))
	    {
	      unsigned long base = ((dylib_command *) lc)->dylib.name.offset;
	      char *name = (char *) (base + (unsigned long) lc);
	      const mach_header *mh1 = lt__match_loadedlib (name);

	      if (!mh1)
		{
		  /* Maybe NSAddImage can find it */
		  mh1 = lt__addimage (name,
				      NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED
				      | NSADDIMAGE_OPTION_WITH_SEARCHING
				      | NSADDIMAGE_OPTION_RETURN_ON_ERROR);
		}

	      if (mh1)
		{
		  symbol = lt__image_symbol (mh1, symname, LT__SYMLOOKUP_OPTS);
		  if (symbol)
		    break;
		}
	    }

	  offset += lc->cmdsize;
	}
    }

  return symbol;
}

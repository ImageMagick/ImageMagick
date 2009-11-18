/* ltdl.c -- system independent dlopen wrapper

   Copyright (C) 1998, 1999, 2000, 2004, 2005, 2006,
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
#include "lt_system.h"
#include "lt_dlloader.h"


/* --- MANIFEST CONSTANTS --- */


/* Standard libltdl search path environment variable name  */
#undef  LTDL_SEARCHPATH_VAR
#define LTDL_SEARCHPATH_VAR	"LTDL_LIBRARY_PATH"

/* Standard libtool archive file extension.  */
#undef  LT_ARCHIVE_EXT
#define LT_ARCHIVE_EXT	".la"

/* max. filename length */
#if !defined(LT_FILENAME_MAX)
#  define LT_FILENAME_MAX	1024
#endif

#if !defined(LT_LIBEXT)
#  define LT_LIBEXT "a"
#endif

/* This is the maximum symbol size that won't require malloc/free */
#undef	LT_SYMBOL_LENGTH
#define LT_SYMBOL_LENGTH	128

/* This accounts for the _LTX_ separator */
#undef	LT_SYMBOL_OVERHEAD
#define LT_SYMBOL_OVERHEAD	5

/* Various boolean flags can be stored in the flags field of an
   lt_dlhandle... */
#define LT_DLIS_RESIDENT(handle)  ((handle)->info.is_resident)
#define LT_DLIS_SYMGLOBAL(handle) ((handle)->info.is_symglobal)
#define LT_DLIS_SYMLOCAL(handle)  ((handle)->info.is_symlocal)


static	const char	objdir[]		= LT_OBJDIR;
static	const char	archive_ext[]		= LT_ARCHIVE_EXT;
static  const char	libext[]		= LT_LIBEXT;
#if defined(LT_MODULE_EXT)
static	const char	shlib_ext[]		= LT_MODULE_EXT;
#endif
#if defined(LT_DLSEARCH_PATH)
static	const char	sys_dlsearch_path[]	= LT_DLSEARCH_PATH;
#endif




/* --- DYNAMIC MODULE LOADING --- */


/* The type of a function used at each iteration of  foreach_dirinpath().  */
typedef int	foreach_callback_func (char *filename, void *data1,
				       void *data2);
/* foreachfile_callback itself calls a function of this type: */
typedef int	file_worker_func      (const char *filename, void *data);


static	int	foreach_dirinpath     (const char *search_path,
				       const char *base_name,
				       foreach_callback_func *func,
				       void *data1, void *data2);
static	int	find_file_callback    (char *filename, void *data1,
				       void *data2);
static	int	find_handle_callback  (char *filename, void *data,
				       void *ignored);
static	int	foreachfile_callback  (char *filename, void *data1,
				       void *data2);


static	int     canonicalize_path     (const char *path, char **pcanonical);
static	int	argzize_path	      (const char *path,
				       char **pargz, size_t *pargz_len);
static	FILE   *find_file	      (const char *search_path,
				       const char *base_name, char **pdir);
static	lt_dlhandle *find_handle      (const char *search_path,
				       const char *base_name,
				       lt_dlhandle *handle,
				       lt_dladvise advise);
static	int	find_module	      (lt_dlhandle *handle, const char *dir,
				       const char *libdir, const char *dlname,
				       const char *old_name, int installed,
				       lt_dladvise advise);
static  int     has_library_ext       (const char *filename);
static	int	load_deplibs	      (lt_dlhandle handle,  char *deplibs);
static	int	trim		      (char **dest, const char *str);
static	int	try_dlopen	      (lt_dlhandle *handle,
				       const char *filename, const char *ext,
				       lt_dladvise advise);
static	int	tryall_dlopen	      (lt_dlhandle *handle,
				       const char *filename,
				       lt_dladvise padvise,
				       const lt_dlvtable *vtable);
static	int	unload_deplibs	      (lt_dlhandle handle);
static	int	lt_argz_insert	      (char **pargz, size_t *pargz_len,
				       char *before, const char *entry);
static	int	lt_argz_insertinorder (char **pargz, size_t *pargz_len,
				       const char *entry);
static	int	lt_argz_insertdir     (char **pargz, size_t *pargz_len,
				       const char *dirnam, struct dirent *dp);
static	int	lt_dlpath_insertdir   (char **ppath, char *before,
				       const char *dir);
static	int	list_files_by_dir     (const char *dirnam,
				       char **pargz, size_t *pargz_len);
static	int	file_not_found	      (void);

#ifdef HAVE_LIBDLLOADER
static	int	loader_init_callback  (lt_dlhandle handle);
#endif /* HAVE_LIBDLLOADER */

static	int	loader_init	      (lt_get_vtable *vtable_func,
				       lt_user_data data);

static	char	       *user_search_path= 0;
static	lt_dlhandle	handles	= 0;
static	int		initialized	= 0;

/* Our memory failure callback sets the error message to be passed back
   up to the client, so we must be careful to return from mallocation
   callers if allocation fails (as this callback returns!!).  */
void
lt__alloc_die_callback (void)
{
  LT__SETERROR (NO_MEMORY);
}

#ifdef HAVE_LIBDLLOADER
/* This function is called to initialise each preloaded module loader,
   and hook it into the list of loaders to be used when attempting to
   dlopen an application module.  */
static int
loader_init_callback (lt_dlhandle handle)
{
  lt_get_vtable *vtable_func = (lt_get_vtable *) lt_dlsym (handle, "get_vtable");
  return loader_init (vtable_func, 0);
}
#endif /* HAVE_LIBDLLOADER */

static int
loader_init (lt_get_vtable *vtable_func, lt_user_data data)
{
  const lt_dlvtable *vtable = 0;
  int errors = 0;

  if (vtable_func)
    {
      vtable = (*vtable_func) (data);
    }

  /* lt_dlloader_add will LT__SETERROR if it fails.  */
  errors += lt_dlloader_add (vtable);

  assert (errors || vtable);

  if ((!errors) && vtable->dlloader_init)
    {
      if ((*vtable->dlloader_init) (vtable->dlloader_data))
	{
	  LT__SETERROR (INIT_LOADER);
	  ++errors;
	}
    }

  return errors;
}

/* Bootstrap the loader loading with the preopening loader.  */
#define get_vtable		preopen_LTX_get_vtable
#define preloaded_symbols	LT_CONC3(lt_, LTDLOPEN, _LTX_preloaded_symbols)

LT_BEGIN_C_DECLS
LT_SCOPE const lt_dlvtable *	get_vtable (lt_user_data data);
LT_END_C_DECLS
#ifdef HAVE_LIBDLLOADER
extern lt_dlsymlist		preloaded_symbols;
#endif

/* Initialize libltdl. */
int
lt_dlinit (void)
{
  int	errors	= 0;

  /* Initialize only at first call. */
  if (++initialized == 1)
    {
      lt__alloc_die	= lt__alloc_die_callback;
      handles		= 0;
      user_search_path	= 0; /* empty search path */

      /* First set up the statically loaded preload module loader, so
	 we can use it to preopen the other loaders we linked in at
	 compile time.  */
      errors += loader_init (get_vtable, 0);

      /* Now open all the preloaded module loaders, so the application
	 can use _them_ to lt_dlopen its own modules.  */
#ifdef HAVE_LIBDLLOADER
      if (!errors)
	{
	  errors += lt_dlpreload (&preloaded_symbols);
	}

      if (!errors)
	{
	  errors += lt_dlpreload_open (LT_STR(LTDLOPEN), loader_init_callback);
	}
#endif /* HAVE_LIBDLLOADER */
    }

#ifdef LT_DEBUG_LOADERS
  lt_dlloader_dump();
#endif

  return errors;
}

int
lt_dlexit (void)
{
  /* shut down libltdl */
  lt_dlloader *loader   = 0;
  lt_dlhandle  handle   = handles;
  int	       errors   = 0;

  if (!initialized)
    {
      LT__SETERROR (SHUTDOWN);
      ++errors;
      goto done;
    }

  /* shut down only at last call. */
  if (--initialized == 0)
    {
      int	level;

      while (handles && LT_DLIS_RESIDENT (handles))
	{
	  handles = handles->next;
	}

      /* close all modules */
      for (level = 1; handle; ++level)
	{
	  lt_dlhandle cur = handles;
	  int saw_nonresident = 0;

	  while (cur)
	    {
	      lt_dlhandle tmp = cur;
	      cur = cur->next;
	      if (!LT_DLIS_RESIDENT (tmp))
		{
		  saw_nonresident = 1;
		  if (tmp->info.ref_count <= level)
		    {
		      if (lt_dlclose (tmp))
			{
			  ++errors;
			}
		      /* Make sure that the handle pointed to by 'cur' still exists.
			 lt_dlclose recursively closes dependent libraries which removes
			 them from the linked list.  One of these might be the one
			 pointed to by 'cur'.  */
		      if (cur)
			{
			  for (tmp = handles; tmp; tmp = tmp->next)
			    if (tmp == cur)
			      break;
			  if (! tmp)
			    cur = handles;
			}
		    }
		}
	    }
	  /* done if only resident modules are left */
	  if (!saw_nonresident)
	    break;
	}

      /* When removing loaders, we can only find out failure by testing
	 the error string, so avoid a spurious one from an earlier
	 failed command. */
      if (!errors)
	LT__SETERRORSTR (0);

      /* close all loaders */
      for (loader = (lt_dlloader *) lt_dlloader_next (NULL); loader;)
	{
	  lt_dlloader *next   = (lt_dlloader *) lt_dlloader_next (loader);
	  lt_dlvtable *vtable = (lt_dlvtable *) lt_dlloader_get (loader);

	  if ((vtable = lt_dlloader_remove ((char *) vtable->name)))
	    {
	      FREE (vtable);
	    }
	  else
	    {
	      /* ignore errors due to resident modules */
	      const char *err;
	      LT__GETERROR (err);
	      if (err)
		++errors;
	    }

	  loader = next;
	}

      FREE(user_search_path);
    }

 done:
  return errors;
}


/* Try VTABLE or, if VTABLE is NULL, all available loaders for FILENAME.
   If the library is not successfully loaded, return non-zero.  Otherwise,
   the dlhandle is stored at the address given in PHANDLE.  */
static int
tryall_dlopen (lt_dlhandle *phandle, const char *filename,
	       lt_dladvise advise, const lt_dlvtable *vtable)
{
  lt_dlhandle	handle		= handles;
  const char *	saved_error	= 0;
  int		errors		= 0;

#ifdef LT_DEBUG_LOADERS
  fprintf (stderr, "tryall_dlopen (%s, %s)\n",
	   filename ? filename : "(null)",
	   vtable ? vtable->name : "(ALL)");
#endif

  LT__GETERROR (saved_error);

  /* check whether the module was already opened */
  for (;handle; handle = handle->next)
    {
      if ((handle->info.filename == filename) /* dlopen self: 0 == 0 */
	  || (handle->info.filename && filename
	      && streq (handle->info.filename, filename)))
	{
	  break;
	}
    }

  if (handle)
    {
      ++handle->info.ref_count;
      *phandle = handle;
      goto done;
    }

  handle = *phandle;
  if (filename)
    {
      /* Comment out the check of file permissions using access.
	 This call seems to always return -1 with error EACCES.
      */
      /* We need to catch missing file errors early so that
	 file_not_found() can detect what happened.
      if (access (filename, R_OK) != 0)
	{
	  LT__SETERROR (FILE_NOT_FOUND);
	  ++errors;
	  goto done;
	} */

      handle->info.filename = lt__strdup (filename);
      if (!handle->info.filename)
	{
	  ++errors;
	  goto done;
	}
    }
  else
    {
      handle->info.filename = 0;
    }

  {
    lt_dlloader loader = lt_dlloader_next (0);
    const lt_dlvtable *loader_vtable;

    do
      {
	if (vtable)
	  loader_vtable = vtable;
	else
	  loader_vtable = lt_dlloader_get (loader);

#ifdef LT_DEBUG_LOADERS
	fprintf (stderr, "Calling %s->module_open (%s)\n",
		 (loader_vtable && loader_vtable->name) ? loader_vtable->name : "(null)",
		 filename ? filename : "(null)");
#endif
	handle->module = (*loader_vtable->module_open) (loader_vtable->dlloader_data,
							filename, advise);
#ifdef LT_DEBUG_LOADERS
	fprintf (stderr, "  Result: %s\n",
		 handle->module ? "Success" : "Failed");
#endif

	if (handle->module != 0)
	  {
	    if (advise)
	      {
		handle->info.is_resident  = advise->is_resident;
		handle->info.is_symglobal = advise->is_symglobal;
		handle->info.is_symlocal  = advise->is_symlocal;
	      }
	    break;
	  }
      }
    while (!vtable && (loader = lt_dlloader_next (loader)));

    /* If VTABLE was given but couldn't open the module, or VTABLE wasn't
       given but we exhausted all loaders without opening the module, bail
       out!  */
    if ((vtable && !handle->module)
	|| (!vtable && !loader))
      {
	FREE (handle->info.filename);
	++errors;
	goto done;
      }

    handle->vtable = loader_vtable;
  }

  LT__SETERRORSTR (saved_error);

 done:
  return errors;
}


static int
tryall_dlopen_module (lt_dlhandle *handle, const char *prefix,
		      const char *dirname, const char *dlname,
		      lt_dladvise advise)
{
  int      error	= 0;
  char     *filename	= 0;
  size_t   filename_len	= 0;
  size_t   dirname_len	= LT_STRLEN (dirname);

  assert (handle);
  assert (dirname);
  assert (dlname);
#if defined(LT_DIRSEP_CHAR)
  /* Only canonicalized names (i.e. with DIRSEP chars already converted)
     should make it into this function:  */
  assert (strchr (dirname, LT_DIRSEP_CHAR) == 0);
#endif

  if (dirname_len > 0)
    if (dirname[dirname_len -1] == '/')
      --dirname_len;
  filename_len = dirname_len + 1 + LT_STRLEN (dlname);

  /* Allocate memory, and combine DIRNAME and MODULENAME into it.
     The PREFIX (if any) is handled below.  */
  filename  = MALLOC (char, filename_len + 1);
  if (!filename)
    return 1;

  sprintf (filename, "%.*s/%s", (int) dirname_len, dirname, dlname);

  /* Now that we have combined DIRNAME and MODULENAME, if there is
     also a PREFIX to contend with, simply recurse with the arguments
     shuffled.  Otherwise, attempt to open FILENAME as a module.  */
  if (prefix)
    {
      error += tryall_dlopen_module (handle, (const char *) 0,
				     prefix, filename, advise);
    }
  else if (tryall_dlopen (handle, filename, advise, 0) != 0)
    {
      ++error;
    }

  FREE (filename);
  return error;
}

static int
find_module (lt_dlhandle *handle, const char *dir, const char *libdir,
	     const char *dlname,  const char *old_name, int installed,
	     lt_dladvise advise)
{
  /* Try to open the old library first; if it was dlpreopened,
     we want the preopened version of it, even if a dlopenable
     module is available.  */
  if (old_name && tryall_dlopen (handle, old_name,
			  advise, lt_dlloader_find ("lt_preopen") ) == 0)
    {
      return 0;
    }

  /* Try to open the dynamic library.  */
  if (dlname)
    {
      /* try to open the installed module */
      if (installed && libdir)
	{
	  if (tryall_dlopen_module (handle, (const char *) 0,
				    libdir, dlname, advise) == 0)
	    return 0;
	}

      /* try to open the not-installed module */
      if (!installed)
	{
	  if (tryall_dlopen_module (handle, dir, objdir,
				    dlname, advise) == 0)
	    return 0;
	}

      /* maybe it was moved to another directory */
      {
	  if (dir && (tryall_dlopen_module (handle, (const char *) 0,
					    dir, dlname, advise) == 0))
	    return 0;
      }
    }

  return 1;
}


static int
canonicalize_path (const char *path, char **pcanonical)
{
  char *canonical = 0;

  assert (path && *path);
  assert (pcanonical);

  canonical = MALLOC (char, 1+ LT_STRLEN (path));
  if (!canonical)
    return 1;

  {
    size_t dest = 0;
    size_t src;
    for (src = 0; path[src] != LT_EOS_CHAR; ++src)
      {
	/* Path separators are not copied to the beginning or end of
	   the destination, or if another separator would follow
	   immediately.  */
	if (path[src] == LT_PATHSEP_CHAR)
	  {
	    if ((dest == 0)
		|| (path[1+ src] == LT_PATHSEP_CHAR)
		|| (path[1+ src] == LT_EOS_CHAR))
	      continue;
	  }

	/* Anything other than a directory separator is copied verbatim.  */
	if ((path[src] != '/')
#if defined(LT_DIRSEP_CHAR)
	    && (path[src] != LT_DIRSEP_CHAR)
#endif
	    )
	  {
	    canonical[dest++] = path[src];
	  }
	/* Directory separators are converted and copied only if they are
	   not at the end of a path -- i.e. before a path separator or
	   NULL terminator.  */
	else if ((path[1+ src] != LT_PATHSEP_CHAR)
		 && (path[1+ src] != LT_EOS_CHAR)
#if defined(LT_DIRSEP_CHAR)
		 && (path[1+ src] != LT_DIRSEP_CHAR)
#endif
		 && (path[1+ src] != '/'))
	  {
	    canonical[dest++] = '/';
	  }
      }

    /* Add an end-of-string marker at the end.  */
    canonical[dest] = LT_EOS_CHAR;
  }

  /* Assign new value.  */
  *pcanonical = canonical;

  return 0;
}

static int
argzize_path (const char *path, char **pargz, size_t *pargz_len)
{
  error_t error;

  assert (path);
  assert (pargz);
  assert (pargz_len);

  if ((error = argz_create_sep (path, LT_PATHSEP_CHAR, pargz, pargz_len)))
    {
      switch (error)
	{
	case ENOMEM:
	  LT__SETERROR (NO_MEMORY);
	  break;
	default:
	  LT__SETERROR (UNKNOWN);
	  break;
	}

      return 1;
    }

  return 0;
}

/* Repeatedly call FUNC with each LT_PATHSEP_CHAR delimited element
   of SEARCH_PATH and references to DATA1 and DATA2, until FUNC returns
   non-zero or all elements are exhausted.  If BASE_NAME is non-NULL,
   it is appended to each SEARCH_PATH element before FUNC is called.  */
static int
foreach_dirinpath (const char *search_path, const char *base_name,
		   foreach_callback_func *func, void *data1, void *data2)
{
  int	 result		= 0;
  size_t filenamesize	= 0;
  size_t lenbase	= LT_STRLEN (base_name);
  size_t argz_len	= 0;
  char *argz		= 0;
  char *filename	= 0;
  char *canonical	= 0;

  if (!search_path || !*search_path)
    {
      LT__SETERROR (FILE_NOT_FOUND);
      goto cleanup;
    }

  if (canonicalize_path (search_path, &canonical) != 0)
    goto cleanup;

  if (argzize_path (canonical, &argz, &argz_len) != 0)
    goto cleanup;

  {
    char *dir_name = 0;
    while ((dir_name = argz_next (argz, argz_len, dir_name)))
      {
	size_t lendir = LT_STRLEN (dir_name);

	if (1+ lendir + lenbase >= filenamesize)
	{
	  FREE (filename);
	  filenamesize	= 1+ lendir + 1+ lenbase; /* "/d" + '/' + "f" + '\0' */
	  filename	= MALLOC (char, filenamesize);
	  if (!filename)
	    goto cleanup;
	}

	assert (filenamesize > lendir);
	strcpy (filename, dir_name);

	if (base_name && *base_name)
	  {
	    if (filename[lendir -1] != '/')
	      filename[lendir++] = '/';
	    strcpy (filename +lendir, base_name);
	  }

	if ((result = (*func) (filename, data1, data2)))
	  {
	    break;
	  }
      }
  }

 cleanup:
  FREE (argz);
  FREE (canonical);
  FREE (filename);

  return result;
}

/* If FILEPATH can be opened, store the name of the directory component
   in DATA1, and the opened FILE* structure address in DATA2.  Otherwise
   DATA1 is unchanged, but DATA2 is set to a pointer to NULL.  */
static int
find_file_callback (char *filename, void *data1, void *data2)
{
  char	     **pdir	= (char **) data1;
  FILE	     **pfile	= (FILE **) data2;
  int	     is_done	= 0;

  assert (filename && *filename);
  assert (pdir);
  assert (pfile);

  if ((*pfile = fopen (filename, LT_READTEXT_MODE)))
    {
      char *dirend = strrchr (filename, '/');

      if (dirend > filename)
	*dirend   = LT_EOS_CHAR;

      FREE (*pdir);
      *pdir   = lt__strdup (filename);
      is_done = (*pdir == 0) ? -1 : 1;
    }

  return is_done;
}

static FILE *
find_file (const char *search_path, const char *base_name, char **pdir)
{
  FILE *file = 0;

  foreach_dirinpath (search_path, base_name, find_file_callback, pdir, &file);

  return file;
}

static int
find_handle_callback (char *filename, void *data, void *data2)
{
  lt_dlhandle  *phandle		= (lt_dlhandle *) data;
  int		notfound	= access (filename, R_OK);
  lt_dladvise   advise		= (lt_dladvise) data2;

  /* Bail out if file cannot be read...  */
  if (notfound)
    return 0;

  /* Try to dlopen the file, but do not continue searching in any
     case.  */
  if (tryall_dlopen (phandle, filename, advise, 0) != 0)
    *phandle = 0;

  return 1;
}

/* If HANDLE was found return it, otherwise return 0.  If HANDLE was
   found but could not be opened, *HANDLE will be set to 0.  */
static lt_dlhandle *
find_handle (const char *search_path, const char *base_name,
	     lt_dlhandle *phandle, lt_dladvise advise)
{
  if (!search_path)
    return 0;

  if (!foreach_dirinpath (search_path, base_name, find_handle_callback,
			  phandle, advise))
    return 0;

  return phandle;
}

#if !defined(LTDL_DLOPEN_DEPLIBS)
static int
load_deplibs (lt_dlhandle handle, char * LT__UNUSED deplibs)
{
  handle->depcount = 0;
  return 0;
}

#else /* defined(LTDL_DLOPEN_DEPLIBS) */
static int
load_deplibs (lt_dlhandle handle, char *deplibs)
{
  char	*p, *save_search_path = 0;
  int   depcount = 0;
  int	i;
  char	**names = 0;
  int	errors = 0;

  handle->depcount = 0;

  if (!deplibs)
    {
      return errors;
    }
  ++errors;

  if (user_search_path)
    {
      save_search_path = lt__strdup (user_search_path);
      if (!save_search_path)
	goto cleanup;
    }

  /* extract search paths and count deplibs */
  p = deplibs;
  while (*p)
    {
      if (!isspace ((unsigned char) *p))
	{
	  char *end = p+1;
	  while (*end && !isspace((unsigned char) *end))
	    {
	      ++end;
	    }

	  if (strncmp(p, "-L", 2) == 0 || strncmp(p, "-R", 2) == 0)
	    {
	      char save = *end;
	      *end = 0; /* set a temporary string terminator */
	      if (lt_dladdsearchdir(p+2))
		{
		  goto cleanup;
		}
	      *end = save;
	    }
	  else
	    {
	      ++depcount;
	    }

	  p = end;
	}
      else
	{
	  ++p;
	}
    }


  if (!depcount)
    {
      errors = 0;
      goto cleanup;
    }

  names = MALLOC (char *, depcount);
  if (!names)
    goto cleanup;

  /* now only extract the actual deplibs */
  depcount = 0;
  p = deplibs;
  while (*p)
    {
      if (isspace ((unsigned char) *p))
	{
	  ++p;
	}
      else
	{
	  char *end = p+1;
	  while (*end && !isspace ((unsigned char) *end))
	    {
	      ++end;
	    }

	  if (strncmp(p, "-L", 2) != 0 && strncmp(p, "-R", 2) != 0)
	    {
	      char *name;
	      char save = *end;
	      *end = 0; /* set a temporary string terminator */
	      if (strncmp(p, "-l", 2) == 0)
		{
		  size_t name_len = 3+ /* "lib" */ LT_STRLEN (p + 2);
		  name = MALLOC (char, 1+ name_len);
		  if (name)
		    sprintf (name, "lib%s", p+2);
		}
	      else
		name = lt__strdup(p);

	      if (!name)
		goto cleanup_names;

	      names[depcount++] = name;
	      *end = save;
	    }
	  p = end;
	}
    }

  /* load the deplibs (in reverse order)
     At this stage, don't worry if the deplibs do not load correctly,
     they may already be statically linked into the loading application
     for instance.  There will be a more enlightening error message
     later on if the loaded module cannot resolve all of its symbols.  */
  if (depcount)
    {
      lt_dlhandle cur = handle;
      int	j = 0;

      cur->deplibs = MALLOC (lt_dlhandle, depcount);
      if (!cur->deplibs)
	goto cleanup_names;

      for (i = 0; i < depcount; ++i)
	{
	  cur->deplibs[j] = lt_dlopenext(names[depcount-1-i]);
	  if (cur->deplibs[j])
	    {
	      ++j;
	    }
	}

      cur->depcount	= j;	/* Number of successfully loaded deplibs */
      errors		= 0;
    }

 cleanup_names:
  for (i = 0; i < depcount; ++i)
    {
      FREE (names[i]);
    }

 cleanup:
  FREE (names);
  /* restore the old search path */
  if (save_search_path) {
    MEMREASSIGN (user_search_path, save_search_path);
  }

  return errors;
}
#endif /* defined(LTDL_DLOPEN_DEPLIBS) */

static int
unload_deplibs (lt_dlhandle handle)
{
  int i;
  int errors = 0;
  lt_dlhandle cur = handle;

  if (cur->depcount)
    {
      for (i = 0; i < cur->depcount; ++i)
	{
	  if (!LT_DLIS_RESIDENT (cur->deplibs[i]))
	    {
	      errors += lt_dlclose (cur->deplibs[i]);
	    }
	}
      FREE (cur->deplibs);
    }

  return errors;
}

static int
trim (char **dest, const char *str)
{
  /* remove the leading and trailing "'" from str
     and store the result in dest */
  const char *end   = strrchr (str, '\'');
  size_t len	    = LT_STRLEN (str);
  char *tmp;

  FREE (*dest);

  if (!end)
    return 1;

  if (len > 3 && str[0] == '\'')
    {
      tmp = MALLOC (char, end - str);
      if (!tmp)
	return 1;

      memcpy(tmp, &str[1], (end - str) - 1);
      tmp[(end - str) - 1] = LT_EOS_CHAR;
      *dest = tmp;
    }
  else
    {
      *dest = 0;
    }

  return 0;
}

/* Read the .la file FILE. */
static int
parse_dotla_file(FILE *file, char **dlname, char **libdir, char **deplibs,
    char **old_name, int *installed)
{
  int		errors = 0;
  size_t	line_len = LT_FILENAME_MAX;
  char *	line = MALLOC (char, line_len);

  if (!line)
    {
      LT__SETERROR (FILE_NOT_FOUND);
      return 1;
    }

  while (!feof (file))
    {
      line[line_len-2] = '\0';
      if (!fgets (line, (int) line_len, file))
	{
	  break;
	}

      /* Handle the case where we occasionally need to read a line
	 that is longer than the initial buffer size.
	 Behave even if the file contains NUL bytes due to corruption. */
      while (line[line_len-2] != '\0' && line[line_len-2] != '\n' && !feof (file))
	{
	  line = REALLOC (char, line, line_len *2);
	  if (!line)
	    {
	      ++errors;
	      goto cleanup;
	    }
	  line[line_len * 2 - 2] = '\0';
	  if (!fgets (&line[line_len -1], (int) line_len +1, file))
	    {
	      break;
	    }
	  line_len *= 2;
	}

      if (line[0] == '\n' || line[0] == '#')
	{
	  continue;
	}

#undef  STR_DLNAME
#define STR_DLNAME	"dlname="
      if (strncmp (line, STR_DLNAME, sizeof (STR_DLNAME) - 1) == 0)
	{
	  errors += trim (dlname, &line[sizeof (STR_DLNAME) - 1]);
	}

#undef  STR_OLD_LIBRARY
#define STR_OLD_LIBRARY	"old_library="
      else if (strncmp (line, STR_OLD_LIBRARY,
	    sizeof (STR_OLD_LIBRARY) - 1) == 0)
	{
	  errors += trim (old_name, &line[sizeof (STR_OLD_LIBRARY) - 1]);
	}
#undef  STR_LIBDIR
#define STR_LIBDIR	"libdir="
      else if (strncmp (line, STR_LIBDIR, sizeof (STR_LIBDIR) - 1) == 0)
	{
	  errors += trim (libdir, &line[sizeof(STR_LIBDIR) - 1]);
	}

#undef  STR_DL_DEPLIBS
#define STR_DL_DEPLIBS	"dependency_libs="
      else if (strncmp (line, STR_DL_DEPLIBS,
	    sizeof (STR_DL_DEPLIBS) - 1) == 0)
	{
	  errors += trim (deplibs, &line[sizeof (STR_DL_DEPLIBS) - 1]);
	}
      else if (streq (line, "installed=yes\n"))
	{
	  *installed = 1;
	}
      else if (streq (line, "installed=no\n"))
	{
	  *installed = 0;
	}

#undef  STR_LIBRARY_NAMES
#define STR_LIBRARY_NAMES "library_names="
      else if (!*dlname && strncmp (line, STR_LIBRARY_NAMES,
	    sizeof (STR_LIBRARY_NAMES) - 1) == 0)
	{
	  char *last_libname;
	  errors += trim (dlname, &line[sizeof (STR_LIBRARY_NAMES) - 1]);
	  if (!errors
	      && *dlname
	      && (last_libname = strrchr (*dlname, ' ')) != 0)
	    {
	      last_libname = lt__strdup (last_libname + 1);
	      if (!last_libname)
		{
		  ++errors;
		  goto cleanup;
		}
	      MEMREASSIGN (*dlname, last_libname);
	    }
	}

      if (errors)
	break;
    }
cleanup:
  FREE (line);
  return errors;
}


/* Try to open FILENAME as a module. */
static int
try_dlopen (lt_dlhandle *phandle, const char *filename, const char *ext,
	    lt_dladvise advise)
{
  const char *	saved_error	= 0;
  char *	archive_name	= 0;
  char *	canonical	= 0;
  char *	base_name	= 0;
  char *	dir		= 0;
  char *	name		= 0;
  char *        attempt		= 0;
  int		errors		= 0;
  lt_dlhandle	newhandle;

  assert (phandle);
  assert (*phandle == 0);

#ifdef LT_DEBUG_LOADERS
  fprintf (stderr, "try_dlopen (%s, %s)\n",
	   filename ? filename : "(null)",
	   ext ? ext : "(null)");
#endif

  LT__GETERROR (saved_error);

  /* dlopen self? */
  if (!filename)
    {
      *phandle = (lt_dlhandle) lt__zalloc (sizeof (struct lt__handle));
      if (*phandle == 0)
	return 1;

      newhandle	= *phandle;

      /* lt_dlclose()ing yourself is very bad!  Disallow it.  */
      newhandle->info.is_resident = 1;

      if (tryall_dlopen (&newhandle, 0, advise, 0) != 0)
	{
	  FREE (*phandle);
	  return 1;
	}

      goto register_handle;
    }

  assert (filename && *filename);

  if (ext)
    {
      attempt = MALLOC (char, LT_STRLEN (filename) + LT_STRLEN (ext) + 1);
      if (!attempt)
	return 1;

      sprintf(attempt, "%s%s", filename, ext);
    }
  else
    {
      attempt = lt__strdup (filename);
      if (!attempt)
	return 1;
    }

  /* Doing this immediately allows internal functions to safely
     assume only canonicalized paths are passed.  */
  if (canonicalize_path (attempt, &canonical) != 0)
    {
      ++errors;
      goto cleanup;
    }

  /* If the canonical module name is a path (relative or absolute)
     then split it into a directory part and a name part.  */
  base_name = strrchr (canonical, '/');
  if (base_name)
    {
      size_t dirlen = (1+ base_name) - canonical;

      dir = MALLOC (char, 1+ dirlen);
      if (!dir)
	{
	  ++errors;
	  goto cleanup;
	}

      strncpy (dir, canonical, dirlen);
      dir[dirlen] = LT_EOS_CHAR;

      ++base_name;
    }
  else
    MEMREASSIGN (base_name, canonical);

  assert (base_name && *base_name);

  ext = strrchr (base_name, '.');
  if (!ext)
    {
      ext = base_name + LT_STRLEN (base_name);
    }

  /* extract the module name from the file name */
  name = MALLOC (char, ext - base_name + 1);
  if (!name)
    {
      ++errors;
      goto cleanup;
    }

  /* canonicalize the module name */
  {
    int i;
    for (i = 0; i < ext - base_name; ++i)
      {
	if (isalnum ((unsigned char)(base_name[i])))
	  {
	    name[i] = base_name[i];
	  }
	else
	  {
	    name[i] = '_';
	  }
      }
    name[ext - base_name] = LT_EOS_CHAR;
  }

  /* Before trawling through the filesystem in search of a module,
     check whether we are opening a preloaded module.  */
  if (!dir)
    {
      const lt_dlvtable *vtable	= lt_dlloader_find ("lt_preopen");

      if (vtable)
	{
	  /* name + "." + libext + NULL */
	  archive_name = MALLOC (char, LT_STRLEN (name) + LT_STRLEN (libext) + 2);
	  *phandle = (lt_dlhandle) lt__zalloc (sizeof (struct lt__handle));

	  if ((*phandle == NULL) || (archive_name == NULL))
	    {
	      ++errors;
	      goto cleanup;
	    }
	  newhandle = *phandle;

	  /* Preloaded modules are always named according to their old
	     archive name.  */
	  sprintf (archive_name, "%s.%s", name, libext);

	  if (tryall_dlopen (&newhandle, archive_name, advise, vtable) == 0)
	    {
	      goto register_handle;
	    }

	  /* If we're still here, there was no matching preloaded module,
	     so put things back as we found them, and continue searching.  */
	  FREE (*phandle);
	  newhandle = NULL;
	}
    }

  /* If we are allowing only preloaded modules, and we didn't find
     anything yet, give up on the search here.  */
  if (advise && advise->try_preload_only)
    {
      goto cleanup;
    }

  /* Check whether we are opening a libtool module (.la extension).  */
  if (ext && streq (ext, archive_ext))
    {
      /* this seems to be a libtool module */
      FILE *	file	 = 0;
      char *	dlname	 = 0;
      char *	old_name = 0;
      char *	libdir	 = 0;
      char *	deplibs	 = 0;

      /* if we can't find the installed flag, it is probably an
	 installed libtool archive, produced with an old version
	 of libtool */
      int	installed = 1;

      /* Now try to open the .la file.  If there is no directory name
	 component, try to find it first in user_search_path and then other
	 prescribed paths.  Otherwise (or in any case if the module was not
	 yet found) try opening just the module name as passed.  */
      if (!dir)
	{
	  const char *search_path = user_search_path;

	  if (search_path)
	    file = find_file (user_search_path, base_name, &dir);

	  if (!file)
	    {
	      search_path = getenv (LTDL_SEARCHPATH_VAR);
	      if (search_path)
		file = find_file (search_path, base_name, &dir);
	    }

#if defined(LT_MODULE_PATH_VAR)
	  if (!file)
	    {
	      search_path = getenv (LT_MODULE_PATH_VAR);
	      if (search_path)
		file = find_file (search_path, base_name, &dir);
	    }
#endif
#if defined(LT_DLSEARCH_PATH)
	  if (!file && *sys_dlsearch_path)
	    {
	      file = find_file (sys_dlsearch_path, base_name, &dir);
	    }
#endif
	}
      else
	{
	  file = fopen (attempt, LT_READTEXT_MODE);
	}

      /* If we didn't find the file by now, it really isn't there.  Set
	 the status flag, and bail out.  */
      if (!file)
	{
	  LT__SETERROR (FILE_NOT_FOUND);
	  ++errors;
	  goto cleanup;
	}

      /* read the .la file */
      if (parse_dotla_file(file, &dlname, &libdir, &deplibs,
	    &old_name, &installed) != 0)
	++errors;

      fclose (file);

      /* allocate the handle */
      *phandle = (lt_dlhandle) lt__zalloc (sizeof (struct lt__handle));
      if (*phandle == 0)
	++errors;

      if (errors)
	{
	  FREE (dlname);
	  FREE (old_name);
	  FREE (libdir);
	  FREE (deplibs);
	  FREE (*phandle);
	  goto cleanup;
	}

      assert (*phandle);

      if (load_deplibs (*phandle, deplibs) == 0)
	{
	  newhandle = *phandle;
	  /* find_module may replace newhandle */
	  if (find_module (&newhandle, dir, libdir, dlname, old_name,
			   installed, advise))
	    {
	      unload_deplibs (*phandle);
	      ++errors;
	    }
	}
      else
	{
	  ++errors;
	}

      FREE (dlname);
      FREE (old_name);
      FREE (libdir);
      FREE (deplibs);

      if (errors)
	{
	  FREE (*phandle);
	  goto cleanup;
	}

      if (*phandle != newhandle)
	{
	  unload_deplibs (*phandle);
	}
    }
  else
    {
      /* not a libtool module */
      *phandle = (lt_dlhandle) lt__zalloc (sizeof (struct lt__handle));
      if (*phandle == 0)
	{
	  ++errors;
	  goto cleanup;
	}

      newhandle = *phandle;

      /* If the module has no directory name component, try to find it
	 first in user_search_path and then other prescribed paths.
	 Otherwise (or in any case if the module was not yet found) try
	 opening just the module name as passed.  */
      if ((dir || (!find_handle (user_search_path, base_name,
				 &newhandle, advise)
		   && !find_handle (getenv (LTDL_SEARCHPATH_VAR), base_name,
				    &newhandle, advise)
#if defined(LT_MODULE_PATH_VAR)
		   && !find_handle (getenv (LT_MODULE_PATH_VAR), base_name,
				    &newhandle, advise)
#endif
#if defined(LT_DLSEARCH_PATH)
		   && !find_handle (sys_dlsearch_path, base_name,
				    &newhandle, advise)
#endif
		   )))
	{
	  if (tryall_dlopen (&newhandle, attempt, advise, 0) != 0)
	    {
	      newhandle = NULL;
	    }
	}

      if (!newhandle)
	{
	  FREE (*phandle);
	  ++errors;
	  goto cleanup;
	}
    }

 register_handle:
  MEMREASSIGN (*phandle, newhandle);

  if ((*phandle)->info.ref_count == 0)
    {
      (*phandle)->info.ref_count	= 1;
      MEMREASSIGN ((*phandle)->info.name, name);

      (*phandle)->next	= handles;
      handles		= *phandle;
    }

  LT__SETERRORSTR (saved_error);

 cleanup:
  FREE (dir);
  FREE (attempt);
  FREE (name);
  if (!canonical)		/* was MEMREASSIGNed */
    FREE (base_name);
  FREE (canonical);
  FREE (archive_name);

  return errors;
}


/* If the last error messge store was `FILE_NOT_FOUND', then return
   non-zero.  */
static int
file_not_found (void)
{
  const char *error = 0;

  LT__GETERROR (error);
  if (error == LT__STRERROR (FILE_NOT_FOUND))
    return 1;

  return 0;
}


/* Unless FILENAME already bears a suitable library extension, then
   return 0.  */
static int
has_library_ext (const char *filename)
{
  char *	ext     = 0;

  assert (filename);

  ext = strrchr (filename, '.');

  if (ext && ((streq (ext, archive_ext))
#if defined(LT_MODULE_EXT)
	     || (streq (ext, shlib_ext))
#endif
    ))
    {
      return 1;
    }

  return 0;
}


/* Initialise and configure a user lt_dladvise opaque object.  */

int
lt_dladvise_init (lt_dladvise *padvise)
{
  lt_dladvise advise = (lt_dladvise) lt__zalloc (sizeof (struct lt__advise));
  *padvise = advise;
  return (advise ? 0 : 1);
}

int
lt_dladvise_destroy (lt_dladvise *padvise)
{
  if (padvise)
    FREE(*padvise);
  return 0;
}

int
lt_dladvise_ext (lt_dladvise *padvise)
{
  assert (padvise && *padvise);
  (*padvise)->try_ext = 1;
  return 0;
}

int
lt_dladvise_resident (lt_dladvise *padvise)
{
  assert (padvise && *padvise);
  (*padvise)->is_resident = 1;
  return 0;
}

int
lt_dladvise_local (lt_dladvise *padvise)
{
  assert (padvise && *padvise);
  (*padvise)->is_symlocal = 1;
  return 0;
}

int
lt_dladvise_global (lt_dladvise *padvise)
{
  assert (padvise && *padvise);
  (*padvise)->is_symglobal = 1;
  return 0;
}

int
lt_dladvise_preload (lt_dladvise *padvise)
{
  assert (padvise && *padvise);
  (*padvise)->try_preload_only = 1;
  return 0;
}

/* Libtool-1.5.x interface for loading a new module named FILENAME.  */
lt_dlhandle
lt_dlopen (const char *filename)
{
  return lt_dlopenadvise (filename, NULL);
}


/* If FILENAME has an ARCHIVE_EXT or MODULE_EXT extension, try to
   open the FILENAME as passed.  Otherwise try appending ARCHIVE_EXT,
   and if a file is still not found try again with MODULE_EXT appended
   instead.  */
lt_dlhandle
lt_dlopenext (const char *filename)
{
  lt_dlhandle	handle	= 0;
  lt_dladvise	advise;

  if (!lt_dladvise_init (&advise) && !lt_dladvise_ext (&advise))
    handle = lt_dlopenadvise (filename, advise);

  lt_dladvise_destroy (&advise);
  return handle;
}


lt_dlhandle
lt_dlopenadvise (const char *filename, lt_dladvise advise)
{
  lt_dlhandle	handle	= 0;
  int		errors	= 0;

  /* Can't have symbols hidden and visible at the same time!  */
  if (advise && advise->is_symlocal && advise->is_symglobal)
    {
      LT__SETERROR (CONFLICTING_FLAGS);
      return 0;
    }

  if (!filename
      || !advise
      || !advise->try_ext
      || has_library_ext (filename))
    {
      /* Just incase we missed a code path in try_dlopen() that reports
	 an error, but forgot to reset handle... */
      if (try_dlopen (&handle, filename, NULL, advise) != 0)
	return 0;

      return handle;
    }
  else if (filename && *filename)
    {

      /* First try appending ARCHIVE_EXT.  */
      errors += try_dlopen (&handle, filename, archive_ext, advise);

      /* If we found FILENAME, stop searching -- whether we were able to
	 load the file as a module or not.  If the file exists but loading
	 failed, it is better to return an error message here than to
	 report FILE_NOT_FOUND when the alternatives (foo.so etc) are not
	 in the module search path.  */
      if (handle || ((errors > 0) && !file_not_found ()))
	return handle;

#if defined(LT_MODULE_EXT)
      /* Try appending SHLIB_EXT.   */
      errors = try_dlopen (&handle, filename, shlib_ext, advise);

      /* As before, if the file was found but loading failed, return now
	 with the current error message.  */
      if (handle || ((errors > 0) && !file_not_found ()))
	return handle;
#endif
    }

  /* Still here?  Then we really did fail to locate any of the file
     names we tried.  */
  LT__SETERROR (FILE_NOT_FOUND);
  return 0;
}


static int
lt_argz_insert (char **pargz, size_t *pargz_len, char *before,
		const char *entry)
{
  error_t error;

  /* Prior to Sep 8, 2005, newlib had a bug where argz_insert(pargz,
     pargz_len, NULL, entry) failed with EINVAL.  */
  if (before)
    error = argz_insert (pargz, pargz_len, before, entry);
  else
    error = argz_append (pargz, pargz_len, entry, 1 + strlen (entry));

  if (error)
    {
      switch (error)
	{
	case ENOMEM:
	  LT__SETERROR (NO_MEMORY);
	  break;
	default:
	  LT__SETERROR (UNKNOWN);
	  break;
	}
      return 1;
    }

  return 0;
}

static int
lt_argz_insertinorder (char **pargz, size_t *pargz_len, const char *entry)
{
  char *before = 0;

  assert (pargz);
  assert (pargz_len);
  assert (entry && *entry);

  if (*pargz)
    while ((before = argz_next (*pargz, *pargz_len, before)))
      {
	int cmp = strcmp (entry, before);

	if (cmp < 0)  break;
	if (cmp == 0) return 0;	/* No duplicates! */
      }

  return lt_argz_insert (pargz, pargz_len, before, entry);
}

static int
lt_argz_insertdir (char **pargz, size_t *pargz_len, const char *dirnam,
		   struct dirent *dp)
{
  char   *buf	    = 0;
  size_t buf_len    = 0;
  char   *end	    = 0;
  size_t end_offset = 0;
  size_t dir_len    = 0;
  int    errors	    = 0;

  assert (pargz);
  assert (pargz_len);
  assert (dp);

  dir_len = LT_STRLEN (dirnam);
  end     = dp->d_name + D_NAMLEN(dp);

  /* Ignore version numbers.  */
  {
    char *p;
    for (p = end; p -1 > dp->d_name; --p)
      if (strchr (".0123456789", p[-1]) == 0)
	break;

    if (*p == '.')
      end = p;
  }

  /* Ignore filename extension.  */
  {
    char *p;
    for (p = end -1; p > dp->d_name; --p)
      if (*p == '.')
	{
	  end = p;
	  break;
	}
  }

  /* Prepend the directory name.  */
  end_offset	= end - dp->d_name;
  buf_len	= dir_len + 1+ end_offset;
  buf		= MALLOC (char, 1+ buf_len);
  if (!buf)
    return ++errors;

  assert (buf);

  strcpy  (buf, dirnam);
  strcat  (buf, "/");
  strncat (buf, dp->d_name, end_offset);
  buf[buf_len] = LT_EOS_CHAR;

  /* Try to insert (in order) into ARGZ/ARGZ_LEN.  */
  if (lt_argz_insertinorder (pargz, pargz_len, buf) != 0)
    ++errors;

  FREE (buf);

  return errors;
}

static int
list_files_by_dir (const char *dirnam, char **pargz, size_t *pargz_len)
{
  DIR	*dirp	  = 0;
  int    errors	  = 0;

  assert (dirnam && *dirnam);
  assert (pargz);
  assert (pargz_len);
  assert (dirnam[LT_STRLEN(dirnam) -1] != '/');

  dirp = opendir (dirnam);
  if (dirp)
    {
      struct dirent *dp	= 0;

      while ((dp = readdir (dirp)))
	if (dp->d_name[0] != '.')
	  if (lt_argz_insertdir (pargz, pargz_len, dirnam, dp))
	    {
	      ++errors;
	      break;
	    }

      closedir (dirp);
    }
  else
    ++errors;

  return errors;
}


/* If there are any files in DIRNAME, call the function passed in
   DATA1 (with the name of each file and DATA2 as arguments).  */
static int
foreachfile_callback (char *dirname, void *data1, void *data2)
{
  file_worker_func *func = *(file_worker_func **) data1;

  int	  is_done  = 0;
  char   *argz     = 0;
  size_t  argz_len = 0;

  if (list_files_by_dir (dirname, &argz, &argz_len) != 0)
    goto cleanup;
  if (!argz)
    goto cleanup;

  {
    char *filename = 0;
    while ((filename = argz_next (argz, argz_len, filename)))
      if ((is_done = (*func) (filename, data2)))
	break;
  }

 cleanup:
  FREE (argz);

  return is_done;
}


/* Call FUNC for each unique extensionless file in SEARCH_PATH, along
   with DATA.  The filenames passed to FUNC would be suitable for
   passing to lt_dlopenext.  The extensions are stripped so that
   individual modules do not generate several entries (e.g. libfoo.la,
   libfoo.so, libfoo.so.1, libfoo.so.1.0.0).  If SEARCH_PATH is NULL,
   then the same directories that lt_dlopen would search are examined.  */
int
lt_dlforeachfile (const char *search_path,
		  int (*func) (const char *filename, void *data),
		  void *data)
{
  int is_done = 0;
  file_worker_func **fpptr = &func;

  if (search_path)
    {
      /* If a specific path was passed, search only the directories
	 listed in it.  */
      is_done = foreach_dirinpath (search_path, 0,
				   foreachfile_callback, fpptr, data);
    }
  else
    {
      /* Otherwise search the default paths.  */
      is_done = foreach_dirinpath (user_search_path, 0,
				   foreachfile_callback, fpptr, data);
      if (!is_done)
	{
	  is_done = foreach_dirinpath (getenv(LTDL_SEARCHPATH_VAR), 0,
				       foreachfile_callback, fpptr, data);
	}

#if defined(LT_MODULE_PATH_VAR)
      if (!is_done)
	{
	  is_done = foreach_dirinpath (getenv(LT_MODULE_PATH_VAR), 0,
				       foreachfile_callback, fpptr, data);
	}
#endif
#if defined(LT_DLSEARCH_PATH)
      if (!is_done && *sys_dlsearch_path)
	{
	  is_done = foreach_dirinpath (sys_dlsearch_path, 0,
				       foreachfile_callback, fpptr, data);
	}
#endif
    }

  return is_done;
}

int
lt_dlclose (lt_dlhandle handle)
{
  lt_dlhandle cur, last;
  int errors = 0;

  /* check whether the handle is valid */
  last = cur = handles;
  while (cur && handle != cur)
    {
      last = cur;
      cur = cur->next;
    }

  if (!cur)
    {
      LT__SETERROR (INVALID_HANDLE);
      ++errors;
      goto done;
    }

  cur = handle;
  cur->info.ref_count--;

  /* Note that even with resident modules, we must track the ref_count
     correctly incase the user decides to reset the residency flag
     later (even though the API makes no provision for that at the
     moment).  */
  if (cur->info.ref_count <= 0 && !LT_DLIS_RESIDENT (cur))
    {
      lt_user_data data = cur->vtable->dlloader_data;

      if (cur != handles)
	{
	  last->next = cur->next;
	}
      else
	{
	  handles = cur->next;
	}

      errors += cur->vtable->module_close (data, cur->module);
      errors += unload_deplibs (handle);

      /* It is up to the callers to free the data itself.  */
      FREE (cur->interface_data);

      FREE (cur->info.filename);
      FREE (cur->info.name);
      FREE (cur);

      goto done;
    }

  if (LT_DLIS_RESIDENT (handle))
    {
      LT__SETERROR (CLOSE_RESIDENT_MODULE);
      ++errors;
    }

 done:
  return errors;
}

void *
lt_dlsym (lt_dlhandle place, const char *symbol)
{
  size_t lensym;
  char	lsym[LT_SYMBOL_LENGTH];
  char	*sym;
  void *address;
  lt_user_data data;
  lt_dlhandle handle;

  if (!place)
    {
      LT__SETERROR (INVALID_HANDLE);
      return 0;
    }

  handle = place;

  if (!symbol)
    {
      LT__SETERROR (SYMBOL_NOT_FOUND);
      return 0;
    }

  lensym = LT_STRLEN (symbol) + LT_STRLEN (handle->vtable->sym_prefix)
					+ LT_STRLEN (handle->info.name);

  if (lensym + LT_SYMBOL_OVERHEAD < LT_SYMBOL_LENGTH)
    {
      sym = lsym;
    }
  else
    {
      sym = MALLOC (char, lensym + LT_SYMBOL_OVERHEAD + 1);
      if (!sym)
	{
	  LT__SETERROR (BUFFER_OVERFLOW);
	  return 0;
	}
    }

  data = handle->vtable->dlloader_data;
  if (handle->info.name)
    {
      const char *saved_error;

      LT__GETERROR (saved_error);

      /* this is a libtool module */
      if (handle->vtable->sym_prefix)
	{
	  strcpy(sym, handle->vtable->sym_prefix);
	  strcat(sym, handle->info.name);
	}
      else
	{
	  strcpy(sym, handle->info.name);
	}

      strcat(sym, "_LTX_");
      strcat(sym, symbol);

      /* try "modulename_LTX_symbol" */
      address = handle->vtable->find_sym (data, handle->module, sym);
      if (address)
	{
	  if (sym != lsym)
	    {
	      FREE (sym);
	    }
	  return address;
	}
      LT__SETERRORSTR (saved_error);
    }

  /* otherwise try "symbol" */
  if (handle->vtable->sym_prefix)
    {
      strcpy(sym, handle->vtable->sym_prefix);
      strcat(sym, symbol);
    }
  else
    {
      strcpy(sym, symbol);
    }

  address = handle->vtable->find_sym (data, handle->module, sym);
  if (sym != lsym)
    {
      FREE (sym);
    }

  return address;
}

const char *
lt_dlerror (void)
{
  const char *error;

  LT__GETERROR (error);
  LT__SETERRORSTR (0);

  return error ? error : NULL;
}

static int
lt_dlpath_insertdir (char **ppath, char *before, const char *dir)
{
  int    errors		= 0;
  char  *canonical	= 0;
  char  *argz		= 0;
  size_t argz_len	= 0;

  assert (ppath);
  assert (dir && *dir);

  if (canonicalize_path (dir, &canonical) != 0)
    {
      ++errors;
      goto cleanup;
    }

  assert (canonical && *canonical);

  /* If *PPATH is empty, set it to DIR.  */
  if (*ppath == 0)
    {
      assert (!before);		/* BEFORE cannot be set without PPATH.  */
      assert (dir);		/* Without DIR, don't call this function!  */

      *ppath = lt__strdup (dir);
      if (*ppath == 0)
	++errors;

      goto cleanup;
    }

  assert (ppath && *ppath);

  if (argzize_path (*ppath, &argz, &argz_len) != 0)
    {
      ++errors;
      goto cleanup;
    }

  /* Convert BEFORE into an equivalent offset into ARGZ.  This only works
     if *PPATH is already canonicalized, and hence does not change length
     with respect to ARGZ.  We canonicalize each entry as it is added to
     the search path, and don't call this function with (uncanonicalized)
     user paths, so this is a fair assumption.  */
  if (before)
    {
      assert (*ppath <= before);
      assert ((int) (before - *ppath) <= (int) strlen (*ppath));

      before = before - *ppath + argz;
    }

  if (lt_argz_insert (&argz, &argz_len, before, dir) != 0)
    {
      ++errors;
      goto cleanup;
    }

  argz_stringify (argz, argz_len, LT_PATHSEP_CHAR);
  MEMREASSIGN(*ppath, argz);

 cleanup:
  FREE (argz);
  FREE (canonical);

  return errors;
}

int
lt_dladdsearchdir (const char *search_dir)
{
  int errors = 0;

  if (search_dir && *search_dir)
    {
      if (lt_dlpath_insertdir (&user_search_path, 0, search_dir) != 0)
	++errors;
    }

  return errors;
}

int
lt_dlinsertsearchdir (const char *before, const char *search_dir)
{
  int errors = 0;

  if (before)
    {
      if ((before < user_search_path)
	  || (before >= user_search_path + LT_STRLEN (user_search_path)))
	{
	  LT__SETERROR (INVALID_POSITION);
	  return 1;
	}
    }

  if (search_dir && *search_dir)
    {
      if (lt_dlpath_insertdir (&user_search_path,
			       (char *) before, search_dir) != 0)
	{
	  ++errors;
	}
    }

  return errors;
}

int
lt_dlsetsearchpath (const char *search_path)
{
  int   errors	    = 0;

  FREE (user_search_path);

  if (!search_path || !LT_STRLEN (search_path))
    {
      return errors;
    }

  if (canonicalize_path (search_path, &user_search_path) != 0)
    ++errors;

  return errors;
}

const char *
lt_dlgetsearchpath (void)
{
  const char *saved_path;

  saved_path = user_search_path;

  return saved_path;
}

int
lt_dlmakeresident (lt_dlhandle handle)
{
  int errors = 0;

  if (!handle)
    {
      LT__SETERROR (INVALID_HANDLE);
      ++errors;
    }
  else
    {
      handle->info.is_resident = 1;
    }

  return errors;
}

int
lt_dlisresident	(lt_dlhandle handle)
{
  if (!handle)
    {
      LT__SETERROR (INVALID_HANDLE);
      return -1;
    }

  return LT_DLIS_RESIDENT (handle);
}



/* --- MODULE INFORMATION --- */

typedef struct {
  const char *id_string;
  lt_dlhandle_interface *iface;
} lt__interface_id;

lt_dlinterface_id
lt_dlinterface_register (const char *id_string, lt_dlhandle_interface *iface)
{
  lt__interface_id *interface_id = (lt__interface_id *) lt__malloc (sizeof *interface_id);

  /* If lt__malloc fails, it will LT__SETERROR (NO_MEMORY), which
     can then be detected with lt_dlerror() if we return 0.  */
  if (interface_id)
    {
      interface_id->id_string = lt__strdup (id_string);
      if (!interface_id->id_string)
	FREE (interface_id);
      else
	interface_id->iface = iface;
    }

  return (lt_dlinterface_id) interface_id;
}

void lt_dlinterface_free (lt_dlinterface_id key)
{
  lt__interface_id *interface_id = (lt__interface_id *)key;
  FREE (interface_id->id_string);
  FREE (interface_id);
}

void *
lt_dlcaller_set_data (lt_dlinterface_id key, lt_dlhandle handle, void *data)
{
  int n_elements = 0;
  void *stale = (void *) 0;
  lt_dlhandle cur = handle;
  int i;

  if (cur->interface_data)
    while (cur->interface_data[n_elements].key)
      ++n_elements;

  for (i = 0; i < n_elements; ++i)
    {
      if (cur->interface_data[i].key == key)
	{
	  stale = cur->interface_data[i].data;
	  break;
	}
    }

  /* Ensure that there is enough room in this handle's interface_data
     array to accept a new element (and an empty end marker).  */
  if (i == n_elements)
    {
      lt_interface_data *temp
	= REALLOC (lt_interface_data, cur->interface_data, 2+ n_elements);

      if (!temp)
	{
	  stale = 0;
	  goto done;
	}

      cur->interface_data = temp;

      /* We only need this if we needed to allocate a new interface_data.  */
      cur->interface_data[i].key	= key;
      cur->interface_data[1+ i].key	= 0;
    }

  cur->interface_data[i].data = data;

 done:
  return stale;
}

void *
lt_dlcaller_get_data (lt_dlinterface_id key, lt_dlhandle handle)
{
  void *result = (void *) 0;
  lt_dlhandle cur = handle;

  /* Locate the index of the element with a matching KEY.  */
  if (cur->interface_data)
    {
      int i;
      for (i = 0; cur->interface_data[i].key; ++i)
	{
	  if (cur->interface_data[i].key == key)
	    {
	      result = cur->interface_data[i].data;
	      break;
	    }
	}
    }

  return result;
}

const lt_dlinfo *
lt_dlgetinfo (lt_dlhandle handle)
{
  if (!handle)
    {
      LT__SETERROR (INVALID_HANDLE);
      return 0;
    }

  return &(handle->info);
}


lt_dlhandle
lt_dlhandle_iterate (lt_dlinterface_id iface, lt_dlhandle place)
{
  lt_dlhandle handle = place;
  lt__interface_id *iterator = (lt__interface_id *) iface;

  assert (iface); /* iface is a required argument */

  if (!handle)
    handle = handles;
  else
    handle = handle->next;

  /* advance while the interface check fails */
  while (handle && iterator->iface
	 && ((*iterator->iface) (handle, iterator->id_string) != 0))
    {
      handle = handle->next;
    }

  return handle;
}


lt_dlhandle
lt_dlhandle_fetch (lt_dlinterface_id iface, const char *module_name)
{
  lt_dlhandle handle = 0;

  assert (iface); /* iface is a required argument */

  while ((handle = lt_dlhandle_iterate (iface, handle)))
    {
      lt_dlhandle cur = handle;
      if (cur && cur->info.name && streq (cur->info.name, module_name))
	break;
    }

  return handle;
}


int
lt_dlhandle_map (lt_dlinterface_id iface,
		 int (*func) (lt_dlhandle handle, void *data), void *data)
{
  lt__interface_id *iterator = (lt__interface_id *) iface;
  lt_dlhandle cur = handles;

  assert (iface); /* iface is a required argument */

  while (cur)
    {
      int errorcode = 0;

      /* advance while the interface check fails */
      while (cur && iterator->iface
	     && ((*iterator->iface) (cur, iterator->id_string) != 0))
	{
	  cur = cur->next;
	}

      if ((errorcode = (*func) (cur, data)) != 0)
	return errorcode;
    }

  return 0;
}

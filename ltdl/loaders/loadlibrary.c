/* loader-loadlibrary.c --  dynamic linking for Win32

   Copyright (C) 1998, 1999, 2000, 2004, 2005, 2006,
                 2007, 2008, 2010 Free Software Foundation, Inc.
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

#if defined(__CYGWIN__)
# include <sys/cygwin.h>
#endif

/* Use the preprocessor to rename non-static symbols to avoid namespace
   collisions when the loader code is statically linked into libltdl.
   Use the "<module_name>_LTX_" prefix so that the symbol addresses can
   be fetched from the preloaded symbol list by lt_dlsym():  */
#define get_vtable	loadlibrary_LTX_get_vtable

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

static lt_dlinterface_id iface_id = 0;
static lt_dlvtable *vtable = 0;

/* Return the vtable for this loader, only the name and sym_prefix
   attributes (plus the virtual function implementations, obviously)
   change between loaders.  */
lt_dlvtable *
get_vtable (lt_user_data loader_data)
{
  if (!vtable)
    {
      vtable = (lt_dlvtable *) lt__zalloc (sizeof *vtable);
      iface_id = lt_dlinterface_register ("ltdl loadlibrary", NULL);
    }

  if (vtable && !vtable->name)
    {
      vtable->name		= "lt_loadlibrary";
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


#include <windows.h>

#define LOCALFREE(mem)					     LT_STMT_START { \
	if (mem) { LocalFree ((void *)mem); mem = NULL; }    } LT_STMT_END
#define LOADLIB__SETERROR(errmsg) LT__SETERRORSTR (loadlibraryerror (errmsg))
#define LOADLIB_SETERROR(errcode) LOADLIB__SETERROR (LT__STRERROR (errcode))

static const char *loadlibraryerror (const char *default_errmsg);
static DWORD WINAPI wrap_getthreaderrormode (void);
static DWORD WINAPI fallback_getthreaderrormode (void);
static BOOL WINAPI wrap_setthreaderrormode (DWORD mode, DWORD *oldmode);
static BOOL WINAPI fallback_setthreaderrormode (DWORD mode, DWORD *oldmode);

typedef DWORD (WINAPI getthreaderrormode_type) (void);
typedef BOOL (WINAPI setthreaderrormode_type) (DWORD, DWORD *);

static getthreaderrormode_type *getthreaderrormode = wrap_getthreaderrormode;
static setthreaderrormode_type *setthreaderrormode = wrap_setthreaderrormode;
static char *error_message = 0;


/* A function called through the vtable when this loader is no
   longer needed by the application.  */
static int
vl_exit (lt_user_data LT__UNUSED loader_data)
{
  vtable = NULL;
  LOCALFREE (error_message);
  return 0;
}

/* A function called through the vtable to open a module with this
   loader.  Returns an opaque representation of the newly opened
   module for processing with this loader's other vtable functions.  */
static lt_module
vm_open (lt_user_data LT__UNUSED loader_data, const char *filename,
         lt_dladvise LT__UNUSED advise)
{
  lt_module	module	   = 0;
  char		*ext;
  char		wpath[MAX_PATH];
  size_t	len;

  if (!filename)
    {
      /* Get the name of main module */
      *wpath = 0;
      GetModuleFileName (NULL, wpath, sizeof (wpath));
      filename = wpath;
    }
  else
    {
      len = LT_STRLEN (filename);

      if (len >= MAX_PATH)
        {
	  LT__SETERROR (CANNOT_OPEN);
	  return 0;
	}

#if HAVE_DECL_CYGWIN_CONV_PATH
      if (cygwin_conv_path (CCP_POSIX_TO_WIN_A, filename, wpath, MAX_PATH))
	{
	  LT__SETERROR (CANNOT_OPEN);
	  return 0;
	}
      len = 0;
#elif defined(__CYGWIN__)
      cygwin_conv_to_full_win32_path (filename, wpath);
      len = 0;
#else
      strcpy(wpath, filename);
#endif

      ext = strrchr (wpath, '.');
      if (!ext)
	{
	  /* Append a `.' to stop Windows from adding an
	     implicit `.dll' extension. */
	  if (!len)
	    len = strlen (wpath);

	  if (len + 1 >= MAX_PATH)
	    {
	      LT__SETERROR (CANNOT_OPEN);
	      return 0;
	    }

	  wpath[len] = '.';
	  wpath[len+1] = '\0';
	}
    }

  {
    /* Silence dialog from LoadLibrary on some failures. */
    DWORD errormode = getthreaderrormode ();
    DWORD last_error;

    setthreaderrormode (errormode | SEM_FAILCRITICALERRORS, NULL);

    module = LoadLibrary (wpath);

    /* Restore the error mode. */
    last_error = GetLastError ();
    setthreaderrormode (errormode, NULL);
    SetLastError (last_error);
  }

  /* libltdl expects this function to fail if it is unable
     to physically load the library.  Sadly, LoadLibrary
     will search the loaded libraries for a match and return
     one of them if the path search load fails.

     We check whether LoadLibrary is returning a handle to
     an already loaded module, and simulate failure if we
     find one. */
  {
    lt_dlhandle cur = 0;

    while ((cur = lt_dlhandle_iterate (iface_id, cur)))
      {
        if (!cur->module)
          {
            cur = 0;
            break;
          }

        if (cur->module == module)
          {
            break;
          }
      }

    if (!module)
      LOADLIB_SETERROR (CANNOT_OPEN);
    else if (cur)
      {
        LT__SETERROR (CANNOT_OPEN);
        module = 0;
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

  if (FreeLibrary ((HMODULE) module) == 0)
    {
      LOADLIB_SETERROR (CANNOT_CLOSE);
      ++errors;
    }

  return errors;
}


/* A function called through the vtable to get the address of
   a symbol loaded from a particular module.  */
static void *
vm_sym (lt_user_data LT__UNUSED loader_data, lt_module module, const char *name)
{
  void *address = (void *) GetProcAddress ((HMODULE) module, name);

  if (!address)
    {
      LOADLIB_SETERROR (SYMBOL_NOT_FOUND);
    }

  return address;
}



/* --- HELPER FUNCTIONS --- */


/* Return the windows error message, or the passed in error message on
   failure. */
static const char *
loadlibraryerror (const char *default_errmsg)
{
  size_t len;
  LOCALFREE (error_message);

  FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  GetLastError (),
                  0,
                  (char *) &error_message,
                  0, NULL);

  /* Remove trailing CRNL */
  len = LT_STRLEN (error_message);
  if (len && error_message[len - 1] == '\n')
    error_message[--len] = LT_EOS_CHAR;
  if (len && error_message[len - 1] == '\r')
    error_message[--len] = LT_EOS_CHAR;

  return len ? error_message : default_errmsg;
}

/* A function called through the getthreaderrormode variable which checks
   if the system supports GetThreadErrorMode (or GetErrorMode) and arranges
   for it or a fallback implementation to be called directly in the future.
   The selected version is then called. */
static DWORD WINAPI
wrap_getthreaderrormode (void)
{
  HMODULE kernel32 = GetModuleHandleA ("kernel32.dll");
  getthreaderrormode
    = (getthreaderrormode_type *) GetProcAddress (kernel32,
						  "GetThreadErrorMode");
  if (!getthreaderrormode)
    getthreaderrormode
      = (getthreaderrormode_type *) GetProcAddress (kernel32,
						    "GetErrorMode");
  if (!getthreaderrormode)
    getthreaderrormode = fallback_getthreaderrormode;
  return getthreaderrormode ();
}

/* A function called through the getthreaderrormode variable for cases
   where the system does not support GetThreadErrorMode or GetErrorMode */
static DWORD WINAPI
fallback_getthreaderrormode (void)
{
  /* Prior to Windows Vista, the only way to get the current error
     mode was to set a new one. In our case, we are setting a new
     error mode right after "getting" it while ignoring the error
     mode in effect when setting the new error mode, so that's
     fairly ok. */
  return (DWORD) SetErrorMode (SEM_FAILCRITICALERRORS);
}

/* A function called through the setthreaderrormode variable which checks
   if the system supports SetThreadErrorMode and arranges for it or a
   fallback implementation to be called directly in the future.
   The selected version is then called. */
static BOOL WINAPI
wrap_setthreaderrormode (DWORD mode, DWORD *oldmode)
{
  HMODULE kernel32 = GetModuleHandleA ("kernel32.dll");
  setthreaderrormode
    = (setthreaderrormode_type *) GetProcAddress (kernel32,
						  "SetThreadErrorMode");
  if (!setthreaderrormode)
    setthreaderrormode = fallback_setthreaderrormode;
  return setthreaderrormode (mode, oldmode);
}

/* A function called through the setthreaderrormode variable for cases
   where the system does not support SetThreadErrorMode. */
static BOOL WINAPI
fallback_setthreaderrormode (DWORD mode, DWORD *oldmode)
{
  /* Prior to Windows 7, there was no way to set the thread local error
     mode, so set the process global error mode instead. */
  DWORD old = (DWORD) SetErrorMode (mode);
  if (oldmode)
    *oldmode = old;
  return TRUE;
}

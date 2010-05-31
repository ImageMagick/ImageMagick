/* lt_error.h -- error propogation interface

   Copyright (C) 1999, 2000, 2001, 2004, 2007 Free Software Foundation, Inc.
   Written by Thomas Tanner, 1999

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

/* Only include this header file once. */
#if !defined(LT_ERROR_H)
#define LT_ERROR_H 1

#include <libltdl/lt_system.h>

LT_BEGIN_C_DECLS

/* Defining error strings alongside their symbolic names in a macro in
   this way allows us to expand the macro in different contexts with
   confidence that the enumeration of symbolic names will map correctly
   onto the table of error strings.  \0 is appended to the strings to
   expilicitely initialize the string terminator. */
#define lt_dlerror_table						\
    LT_ERROR(UNKNOWN,		    "unknown error\0")			\
    LT_ERROR(DLOPEN_NOT_SUPPORTED,  "dlopen support not available\0")	\
    LT_ERROR(INVALID_LOADER,	    "invalid loader\0")			\
    LT_ERROR(INIT_LOADER,	    "loader initialization failed\0")	\
    LT_ERROR(REMOVE_LOADER,	    "loader removal failed\0")		\
    LT_ERROR(FILE_NOT_FOUND,	    "file not found\0")			\
    LT_ERROR(DEPLIB_NOT_FOUND,	    "dependency library not found\0")	\
    LT_ERROR(NO_SYMBOLS,	    "no symbols defined\0")		\
    LT_ERROR(CANNOT_OPEN,	    "can't open the module\0")		\
    LT_ERROR(CANNOT_CLOSE,	    "can't close the module\0")		\
    LT_ERROR(SYMBOL_NOT_FOUND,	    "symbol not found\0")		\
    LT_ERROR(NO_MEMORY,		    "not enough memory\0")		\
    LT_ERROR(INVALID_HANDLE,	    "invalid module handle\0")		\
    LT_ERROR(BUFFER_OVERFLOW,	    "internal buffer overflow\0")	\
    LT_ERROR(INVALID_ERRORCODE,	    "invalid errorcode\0")		\
    LT_ERROR(SHUTDOWN,		    "library already shutdown\0")	\
    LT_ERROR(CLOSE_RESIDENT_MODULE, "can't close resident module\0")	\
    LT_ERROR(INVALID_MUTEX_ARGS,    "internal error (code withdrawn)\0")\
    LT_ERROR(INVALID_POSITION,	    "invalid search path insert position\0")\
    LT_ERROR(CONFLICTING_FLAGS,	    "symbol visibility can be global or local\0")

/* Enumerate the symbolic error names. */
enum {
#define LT_ERROR(name, diagnostic)	LT_CONC(LT_ERROR_, name),
	lt_dlerror_table
#undef LT_ERROR

	LT_ERROR_MAX
};

/* Should be max of the error string lengths above (plus one for C++) */
#define LT_ERROR_LEN_MAX (41)

/* These functions are only useful from inside custom module loaders. */
LT_SCOPE int	lt_dladderror	(const char *diagnostic);
LT_SCOPE int	lt_dlseterror	(int errorcode);


LT_END_C_DECLS

#endif /*!defined(LT_ERROR_H)*/

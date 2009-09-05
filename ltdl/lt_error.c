/* lt_error.c -- error propogation interface

   Copyright (C) 1999, 2000, 2001, 2004, 2005, 2007 Free Software Foundation, Inc.
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

#include "lt__private.h"
#include "lt_error.h"

static const char	*last_error	= 0;
static const char	error_strings[LT_ERROR_MAX][LT_ERROR_LEN_MAX + 1] =
  {
#define LT_ERROR(name, diagnostic)	diagnostic,
    lt_dlerror_table
#undef LT_ERROR
  };

static	const char    **user_error_strings	= 0;
static	int		errorcount		= LT_ERROR_MAX;

int
lt_dladderror (const char *diagnostic)
{
  int		errindex = 0;
  int		result	 = -1;
  const char  **temp     = (const char **) 0;

  assert (diagnostic);

  errindex = errorcount - LT_ERROR_MAX;
  temp = REALLOC (const char *, user_error_strings, 1 + errindex);
  if (temp)
    {
      user_error_strings		= temp;
      user_error_strings[errindex]	= diagnostic;
      result				= errorcount++;
    }

  return result;
}

int
lt_dlseterror (int errindex)
{
  int		errors	 = 0;

  if (errindex >= errorcount || errindex < 0)
    {
      /* Ack!  Error setting the error message! */
      LT__SETERROR (INVALID_ERRORCODE);
      ++errors;
    }
  else if (errindex < LT_ERROR_MAX)
    {
      /* No error setting the error message! */
      LT__SETERRORSTR (error_strings[errindex]);
    }
  else
    {
      /* No error setting the error message! */
      LT__SETERRORSTR (user_error_strings[errindex - LT_ERROR_MAX]);
    }

  return errors;
}

const char *
lt__error_string (int errorcode)
{
  assert (errorcode >= 0);
  assert (errorcode < LT_ERROR_MAX);

  return error_strings[errorcode];
}

const char *
lt__get_last_error (void)
{
  return last_error;
}

const char *
lt__set_last_error (const char *errormsg)
{
  return last_error = errormsg;
}

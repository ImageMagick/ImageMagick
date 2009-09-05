/* lt__dirent.c -- internal directory entry scanning interface

   Copyright (C) 2001, 2004 Free Software Foundation, Inc.
   Written by Bob Friesenhahn, 2001

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

#include <assert.h>
#include <stddef.h>

#include "lt__dirent.h"

#if defined(__WINDOWS__)

void
closedir (DIR *entry)
{
  assert (entry != (DIR *) NULL);
  FindClose (entry->hSearch);
  free ((void *) entry);
}


DIR *
opendir (const char *path)
{
  char file_spec[LT_FILENAME_MAX];
  DIR *entry;

  assert (path != (char *) 0);
  if (lt_strlcpy (file_spec, path, sizeof file_spec) >= sizeof file_spec
      || lt_strlcat (file_spec, "\\", sizeof file_spec) >= sizeof file_spec)
    return (DIR *) 0;
  entry = (DIR *) malloc (sizeof(DIR));
  if (entry != (DIR *) 0)
    {
      entry->firsttime = TRUE;
      entry->hSearch = FindFirstFile (file_spec, &entry->Win32FindData);

      if (entry->hSearch == INVALID_HANDLE_VALUE)
	{
	  if (lt_strlcat (file_spec, "\\*.*", sizeof file_spec) < sizeof file_spec)
	    {
	      entry->hSearch = FindFirstFile (file_spec, &entry->Win32FindData);
	    }

	  if (entry->hSearch == INVALID_HANDLE_VALUE)
	    {
	      entry = (free (entry), (DIR *) 0);
	    }
	}
    }

  return entry;
}


struct dirent *
readdir (DIR *entry)
{
  int status;

  if (entry == (DIR *) 0)
    return (struct dirent *) 0;

  if (!entry->firsttime)
    {
      status = FindNextFile (entry->hSearch, &entry->Win32FindData);
      if (status == 0)
        return (struct dirent *) 0;
    }

  entry->firsttime = FALSE;
  if (lt_strlcpy (entry->file_info.d_name, entry->Win32FindData.cFileName,
	sizeof entry->file_info.d_name) >= sizeof entry->file_info.d_name)
    return (struct dirent *) 0;
  entry->file_info.d_namlen = strlen (entry->file_info.d_name);

  return &entry->file_info;
}

#endif /*defined(__WINDOWS__)*/

/* lt__strl.c -- size-bounded string copying and concatenation

   Copyright (C) 2004 Free Software Foundation, Inc.
   Written by Bob Friesenhahn, 2004

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
#include <string.h>

#include "lt__strl.h"

/*
 lt_strlcat appends the NULL-terminated string src to the end of dst.
 It will append at most dstsize - strlen(dst) - 1 bytes,
 NULL-terminating the result. The total length of the string which
 would have been created given sufficient buffer size (may be longer
 than dstsize) is returned.  This function substitutes for strlcat()
 which is available under NetBSD, FreeBSD and Solaris 9.

 Buffer overflow can be checked as follows:

   if (lt_strlcat(dst, src, dstsize) >= dstsize)
     return -1;
*/
#if !defined(HAVE_STRLCAT)
size_t
lt_strlcat(char *dst, const char *src, const size_t dstsize)
{
  size_t length;
  char *p;
  const char *q;

  assert(dst != NULL);
  assert(src != (const char *) NULL);
  assert(dstsize >= 1);

  length=strlen(dst);

  /*
    Copy remaining characters from src while constraining length to
    size - 1.
  */
  for ( p = dst + length, q = src;
        (*q != 0) && (length < dstsize - 1) ;
        length++, p++, q++ )
    *p = *q;

  dst[length]='\0';

  /*
    Add remaining length of src to length.
  */
  while (*q++)
    length++;

  return length;
}
#endif /* !defined(HAVE_STRLCAT) */

/*
  lt_strlcpy copies up to dstsize - 1 characters from the NULL-terminated
  string src to dst, NULL-terminating the result. The total length of
  the string which would have been created given sufficient buffer
  size (may be longer than dstsize) is returned. This function
  substitutes for strlcpy() which is available under OpenBSD, FreeBSD
  and Solaris 9.

  Buffer overflow can be checked as  follows:

    if (lt_strlcpy(dst, src, dstsize) >= dstsize)
      return -1;
*/
#if !defined(HAVE_STRLCPY)
size_t
lt_strlcpy(char *dst, const char *src, const size_t dstsize)
{
  size_t length=0;
  char *p;
  const char *q;

  assert(dst != NULL);
  assert(src != (const char *) NULL);
  assert(dstsize >= 1);

  /*
    Copy src to dst within bounds of size-1.
  */
  for ( p=dst, q=src, length=0 ;
        (*q != 0) && (length < dstsize-1) ;
        length++, p++, q++ )
    *p = *q;

  dst[length]='\0';

  /*
    Add remaining length of src to length.
  */
  while (*q++)
    length++;

  return length;
}
#endif /* !defined(HAVE_STRLCPY) */

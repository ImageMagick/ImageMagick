/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private utility methods.
*/
#ifndef _MAGICKCORE_UTILITY_PRIVATE_H
#define _MAGICKCORE_UTILITY_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickPrivate char
  **GetPathComponents(const char *,size_t *),
  **ListFiles(const char *,const char *,size_t *);

extern MagickPrivate MagickBooleanType
  GetExecutionPath(char *,const size_t);

extern MagickPrivate ssize_t
  GetMagickPageSize(void);

extern MagickPrivate void
  ChopPathComponents(char *,const size_t),
  ExpandFilename(char *),
  MagickDelay(const MagickSizeType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

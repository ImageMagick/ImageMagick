/*
  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image coder methods.
*/
#ifndef _MAGICKCORE_CODER_H
#define _MAGICKCORE_CODER_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _CoderInfo
{
  char
    *path,
    *magick,
    *name;
                                                                                
  MagickBooleanType
    stealth;
                                                                                
  struct _CoderInfo
    *previous,
    *next;  /* deprecated, use GetCoderInfoList() */

  unsigned long
    signature;
} CoderInfo;

extern MagickExport char
  **GetCoderList(const char *,unsigned long *,ExceptionInfo *);

extern MagickExport const CoderInfo
  *GetCoderInfo(const char *,ExceptionInfo *),
  **GetCoderInfoList(const char *,unsigned long *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  ListCoderInfo(FILE *,ExceptionInfo *);

MagickExport void
  DestroyCoderList(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

/*
  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  The ImageMagick mime methods.
*/
#ifndef _MIME_MIME_H
#define _MIME_MIME_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _MimeInfo
  MimeInfo;

extern MagickExport char
  **GetMimeList(const char *,size_t *,ExceptionInfo *),
  *MagickToMime(const char *);

extern MagickExport const char
  *GetMimeDescription(const MimeInfo *),
  *GetMimeType(const MimeInfo *);

extern MagickExport MagickBooleanType
  ListMimeInfo(FILE *,ExceptionInfo *),
  LoadMimeLists(const char *,ExceptionInfo *),
  MimeComponentGenesis(void);

extern MagickExport const MimeInfo
  *GetMimeInfo(const char *,const unsigned char *,const size_t,ExceptionInfo *),
  **GetMimeInfoList(const char *,size_t *,ExceptionInfo *);

extern MagickExport void
  MimeComponentTerminus(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

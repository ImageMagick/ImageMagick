/*
  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore exception private methods.
*/
#ifndef _MAGICKCORE_EXCEPTION_PRIVATE_H
#define _MAGICKCORE_EXCEPTION_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/log.h"

#define ThrowBinaryException(severity,tag,context) \
{ \
  if (image != (Image *) NULL) \
    (void) ThrowMagickException(&image->exception,GetMagickModule(),severity, \
      tag == (const char *) NULL ? "unknown" : tag,"`%s'",context); \
  return(MagickFalse); \
}
#define ThrowFatalException(severity,tag) \
{ \
  ExceptionInfo \
    exception; \
 \
  GetExceptionInfo(&exception); \
  (void) ThrowMagickException(&exception,GetMagickModule(),severity, \
    tag == (const char *) NULL ? "unknown" : tag,"`%s'", \
    GetExceptionMessage(errno)); \
  CatchException(&exception); \
  (void) DestroyExceptionInfo(&exception); \
  _exit(1); \
}
#define ThrowFileException(exception,severity,tag,context) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),severity, \
    tag == (const char *) NULL ? "unknown" : tag,"`%s': %s",context, \
    GetExceptionMessage(errno)); \
}
#define ThrowImageException(severity,tag) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),severity, \
    tag == (const char *) NULL ? "unknown" : tag,"`%s'",image->filename); \
  return((Image *) NULL); \
}
#define ThrowReaderException(severity,tag) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),severity, \
    tag == (const char *) NULL ? "unknown" : tag,"`%s'",image_info->filename); \
  if ((image) != (Image *) NULL) \
    { \
      (void) CloseBlob(image); \
      image=DestroyImageList(image); \
    } \
  return((Image *) NULL); \
}
#define ThrowWriterException(severity,tag) \
{ \
  (void) ThrowMagickException(&image->exception,GetMagickModule(),severity, \
    tag == (const char *) NULL ? "unknown" : tag,"`%s'",image->filename); \
  if (image_info->adjoin != MagickFalse) \
    while (image->previous != (Image *) NULL) \
      image=image->previous; \
  (void) CloseBlob(image); \
  return(MagickFalse); \
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

/*
  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore exception private methods.
*/
#ifndef MAGICKCORE_EXCEPTION_PRIVATE_H
#define MAGICKCORE_EXCEPTION_PRIVATE_H

#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/string_.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define ThrowBinaryException(severity,tag,context) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),severity, \
    tag == (const char *) NULL ? "unknown" : tag,"`%s'",context); \
  return(MagickFalse); \
}
#define ThrowFatalException(severity,tag) \
{ \
  char \
    *fatal_message; \
 \
  ExceptionInfo \
    *fatal_exception; \
 \
  fatal_exception=AcquireExceptionInfo(); \
  fatal_message=GetExceptionMessage(errno); \
  (void) ThrowMagickException(fatal_exception,GetMagickModule(),severity, \
    tag == (const char *) NULL ? "unknown" : tag,"`%s'",fatal_message); \
  fatal_message=DestroyString(fatal_message); \
  CatchException(fatal_exception); \
  (void) DestroyExceptionInfo(fatal_exception); \
  MagickCoreTerminus(); \
  _exit((int) (severity-FatalErrorException)+1); \
}
#define ThrowFileException(exception,severity,tag,context) \
{ \
  char \
    *file_message; \
 \
  file_message=GetExceptionMessage(errno); \
  (void) ThrowMagickException(exception,GetMagickModule(),severity, \
    tag == (const char *) NULL ? "unknown" : tag,"'%s': %s",context, \
    file_message); \
  file_message=DestroyString(file_message); \
}
#define ThrowImageException(severity,tag) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),severity, \
    tag == (const char *) NULL ? "unknown" : tag,"`%s'",image->filename); \
  return((Image *) NULL); \
}
#define ThrowReaderException(severity,tag) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),severity,  \
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
  (void) ThrowMagickException(exception,GetMagickModule(),severity,  \
    tag == (const char *) NULL ? "unknown" : tag,"`%s'",image->filename); \
  if (image_info->adjoin != MagickFalse) \
    while (image->previous != (Image *) NULL) \
      image=image->previous; \
  (void) CloseBlob(image); \
  return(MagickFalse); \
}

extern MagickPrivate void
  ExceptionComponentTerminus(void),
  InitializeExceptionInfo(ExceptionInfo *);

extern MagickPrivate MagickBooleanType
  ExceptionComponentGenesis(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

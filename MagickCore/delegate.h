/*
  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore delegates methods.
*/
#ifndef MAGICKCORE_DELEGATE_H
#define MAGICKCORE_DELEGATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <stdarg.h>
#include "MagickCore/semaphore.h"

typedef struct _DelegateInfo
{
  char
    *path,
    *decode,
    *encode,
    *commands;

  ssize_t
    mode;

  MagickBooleanType
    thread_support,
    spawn,
    stealth;

  SemaphoreInfo
    *semaphore;

  size_t
    signature;
} DelegateInfo;

extern MagickExport char
  *GetDelegateCommand(const ImageInfo *,Image *,const char *,const char *,
    ExceptionInfo *),
  **GetDelegateList(const char *,size_t *,ExceptionInfo *);

extern MagickExport const char
  *GetDelegateCommands(const DelegateInfo *);

extern MagickExport const DelegateInfo
  *GetDelegateInfo(const char *,const char *,ExceptionInfo *exception),
  **GetDelegateInfoList(const char *,size_t *,ExceptionInfo *);

extern MagickExport int
  ExternalDelegateCommand(const MagickBooleanType,const MagickBooleanType,
    const char *,char *,ExceptionInfo *);

extern MagickExport ssize_t
  GetDelegateMode(const DelegateInfo *);

extern MagickExport MagickBooleanType
  GetDelegateThreadSupport(const DelegateInfo *),
  InvokeDelegate(ImageInfo *,Image *,const char *,const char *,ExceptionInfo *),
  ListDelegateInfo(FILE *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

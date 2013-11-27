/*
  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore module methods.
*/
#ifndef _MAGICKCORE_MODULE_H
#define _MAGICKCORE_MODULE_H

#include <time.h>
#include "magick/version.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MagickImageCoderSignature  ((size_t) \
  (((MagickLibVersion) << 8) | MAGICKCORE_QUANTUM_DEPTH))
#define MagickImageFilterSignature  ((size_t) \
  (((MagickLibVersion) << 8) | MAGICKCORE_QUANTUM_DEPTH))

typedef enum
{
  MagickImageCoderModule,
  MagickImageFilterModule
} MagickModuleType;

typedef struct _ModuleInfo
{
  char
    *path,
    *tag;

  void
    *handle,
    (*unregister_module)(void);

  size_t
    (*register_module)(void);

  time_t
    timestamp;

  MagickBooleanType
    stealth;

  struct _ModuleInfo
    *previous,
    *next;  /* deprecated, use GetModuleInfoList() */

  size_t
    signature;
} ModuleInfo;

typedef size_t
  ImageFilterHandler(Image **,const int,const char **,ExceptionInfo *);

extern MagickExport char
  **GetModuleList(const char *,const MagickModuleType,size_t *,ExceptionInfo *);

extern MagickExport const ModuleInfo
  **GetModuleInfoList(const char *,size_t *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  InitializeModuleList(ExceptionInfo *),
  InvokeDynamicImageFilter(const char *,Image **,const int,const char **,
    ExceptionInfo *),
  ListModuleInfo(FILE *,ExceptionInfo *),
  ModuleComponentGenesis(void),
  OpenModule(const char *,ExceptionInfo *),
  OpenModules(ExceptionInfo *);

extern MagickExport ModuleInfo
  *GetModuleInfo(const char *,ExceptionInfo *);

extern MagickExport void
  DestroyModuleList(void),
  ModuleComponentTerminus(void),
  RegisterStaticModules(void),
  UnregisterStaticModules(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

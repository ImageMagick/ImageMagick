/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore module methods.
*/
#ifndef MAGICKCORE_MODULE_H
#define MAGICKCORE_MODULE_H

#include "MagickCore/version.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MagickImageCoderSignature  ((size_t) \
  (((MagickLibInterface) << 8) | MAGICKCORE_QUANTUM_DEPTH))
#define MagickImageFilterSignature  ((size_t) \
  (((MagickLibInterface) << 8) | MAGICKCORE_QUANTUM_DEPTH))

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
  InvokeDynamicImageFilter(const char *,Image **,const int,const char **,
    ExceptionInfo *),
  ListModuleInfo(FILE *,ExceptionInfo *);

extern MagickExport ModuleInfo
  *GetModuleInfo(const char *,ExceptionInfo *);

extern MagickExport void
  DestroyModuleList(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

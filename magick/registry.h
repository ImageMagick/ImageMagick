/*
  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore registry methods.
*/
#ifndef _MAGICKCORE_REGISTRY_H
#define _MAGICKCORE_REGISTRY_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedRegistryType,
  ImageRegistryType,
  ImageInfoRegistryType,
  StringRegistryType
} RegistryType;

extern MagickExport char
  *GetNextImageRegistry(void);

extern MagickExport MagickBooleanType
  DefineImageRegistry(const RegistryType,const char *,ExceptionInfo *),
  DeleteImageRegistry(const char *),
  RegistryComponentGenesis(void),
  SetImageRegistry(const RegistryType,const char *,const void *,
    ExceptionInfo *);

extern MagickExport void
  *GetImageRegistry(const RegistryType,const char *,ExceptionInfo *),
  RegistryComponentTerminus(void),
  *RemoveImageRegistry(const char *),
  ResetImageRegistryIterator(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

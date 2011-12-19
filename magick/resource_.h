/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore resource methods.
*/
#ifndef _MAGICKCORE_RESOURCE_H
#define _MAGICKCORE_RESOURCE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedResource,
  AreaResource,
  DiskResource,
  FileResource,
  MapResource,
  MemoryResource,
  ThreadResource,
  TimeResource,
  ThrottleResource
} ResourceType;

#define MagickResourceInfinity  MagickULLConstant(~0)

extern MagickExport int
  AcquireUniqueFileResource(char *);

extern MagickExport MagickBooleanType
  AcquireMagickResource(const ResourceType,const MagickSizeType),
  ListMagickResourceInfo(FILE *,ExceptionInfo *),
  RelinquishUniqueFileResource(const char *),
  ResourceComponentGenesis(void),
  SetMagickResourceLimit(const ResourceType,const MagickSizeType);

extern MagickExport MagickSizeType
  GetMagickResource(const ResourceType),
  GetMagickResourceLimit(const ResourceType);

extern MagickExport void
  AsynchronousResourceComponentTerminus(void),
  RelinquishMagickResource(const ResourceType,const MagickSizeType),
  ResourceComponentTerminus(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

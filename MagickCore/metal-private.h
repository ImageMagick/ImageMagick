/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/license/

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private Metal methods.
*/
#ifndef MAGICKCORE_METAL_PRIVATE_H
#define MAGICKCORE_METAL_PRIVATE_H

#include "MagickCore/studio.h"
#include "MagickCore/semaphore.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(MAGICKCORE_METAL_SUPPORT)

/*
  Pipeline cache entry — caches compiled MTLComputePipelineState by kernel name.
*/
#define METAL_PIPELINE_CACHE_SIZE 64

typedef struct _MetalPipelineCacheEntry
{
  char
    name[64];               /* kernel function name */

  void
    *pipeline;              /* id<MTLComputePipelineState> */
} MetalPipelineCacheEntry;

typedef struct _MetalLibraryCacheEntry
{
  const char
    *source;                /* pointer to source string literal (used as key) */

  void
    *library;               /* id<MTLLibrary> */

  MetalPipelineCacheEntry
    pipelines[8];           /* cached pipelines for this library */

  size_t
    pipeline_count;
} MetalLibraryCacheEntry;

typedef struct _MagickMetalDevice
{
  void
    *device;  /* id<MTLDevice> */

  void
    *command_queue; /* id<MTLCommandQueue> */

  MagickBooleanType
    profile_kernels;

  MetalLibraryCacheEntry
    library_cache[METAL_PIPELINE_CACHE_SIZE];

  size_t
    library_cache_count;

  SemaphoreInfo
    *lock;
} *MagickMetalDevice;

typedef struct _MagickMetalEnv
{
  MagickMetalDevice
    device;

  MagickBooleanType
    enabled;

  SemaphoreInfo
    *lock;
} *MagickMetalEnv;

/*
  MetalKernelArg — tagged union for passing buffer or scalar arguments to kernels.
*/
typedef enum
{
  MetalArgBuffer,
  MetalArgBytes
} MetalArgType;

typedef struct _MetalKernelArg
{
  MetalArgType
    type;

  union {
    void
      *buffer;              /* id<MTLBuffer> via CFBridgingRetain */

    struct {
      const void
        *data;

      size_t
        size;
    } bytes;
  };
} MetalKernelArg;

#define MetalBufferArg(buf) \
  ((MetalKernelArg){ .type = MetalArgBuffer, .buffer = (buf) })

#define MetalBytesArg(ptr, sz) \
  ((MetalKernelArg){ .type = MetalArgBytes, .bytes = { .data = (ptr), .size = (sz) } })

extern MagickPrivate MagickMetalEnv
  GetMagickMetalEnv(void);

extern MagickPrivate MagickMetalDevice
  AcquireMagickMetalDevice(MagickMetalEnv);

extern MagickPrivate void
  RelinquishMagickMetalDevice(MagickMetalDevice);

extern MagickPrivate void
  *AcquireMetalBuffer(MagickMetalDevice,size_t,const void *);

extern MagickPrivate void
  *AcquireMetalBufferNoCopy(MagickMetalDevice,void *,size_t);

extern MagickPrivate void
  *GetMetalBufferContents(void *);

extern MagickPrivate void
  RelinquishMetalBuffer(void *);

extern MagickPrivate void
  *AcquireMetalKernel(MagickMetalDevice,const char *,const char *);

extern MagickPrivate void
  *AcquireMetalKernelWithHelpers(MagickMetalDevice,const char *,const char *,
    const char *);

extern MagickPrivate void
  RelinquishMetalKernel(void *);

extern MagickPrivate MagickBooleanType
  EnqueueMetalKernel(MagickMetalDevice,void *,const size_t,const size_t,void **);

extern MagickPrivate MagickBooleanType
  EnqueueMetalKernel2D(MagickMetalDevice,void *,const size_t *,const size_t *,const size_t,void **);

extern MagickPrivate MagickBooleanType
  EnqueueMetalKernel2DEx(MagickMetalDevice,void *,const size_t *,const size_t *,const size_t,MetalKernelArg *,size_t);

#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

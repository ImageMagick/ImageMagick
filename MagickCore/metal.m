/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%              M   M  EEEEE  TTTTT   AAA   L                                  %
%              MM MM  E        T    A   A  L                                  %
%              M M M  EEE      T    AAAAA  L                                  %
%              M   M  E        T    A   A  L                                  %
%              M   M  EEEEE    T    A   A  LLLLL                              %
%                                                                             %
%                                                                             %
%                      MagickCore Metal Runtime Methods                       %
%                                                                             %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
  MagickCore Metal methods.
*/

#include "MagickCore/magick-baseconfig.h"

#if defined(MAGICKCORE_METAL_SUPPORT)

#define ExceptionInfo SystemExceptionInfo
#import <Metal/Metal.h>
#undef ExceptionInfo

#endif

#include "MagickCore/studio.h"
#include "MagickCore/metal-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/string_.h"

#if defined(MAGICKCORE_METAL_SUPPORT)

/*
  Global Metal Environment
*/
static MagickMetalEnv
  metal_env = (MagickMetalEnv) NULL;

static SemaphoreInfo
  *metal_lock = (SemaphoreInfo *) NULL;

/*
  Forward declarations
*/
static MagickMetalEnv AcquireMagickMetalEnv(void);

MagickPrivate MagickMetalEnv GetMagickMetalEnv(void)
{
  if (metal_lock == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&metal_lock);

  LockSemaphoreInfo(metal_lock);
  if (metal_env == (MagickMetalEnv) NULL)
    metal_env = AcquireMagickMetalEnv();
  UnlockSemaphoreInfo(metal_lock);

  return metal_env;
}

static MagickMetalEnv AcquireMagickMetalEnv(void)
{
  id<MTLCommandQueue>
    queue;

  id<MTLDevice>
    device;

  MagickMetalEnv
    env;

  env = (MagickMetalEnv) AcquireMagickMemory(sizeof(*env));
  if (env == (MagickMetalEnv) NULL)
    return (MagickMetalEnv) NULL;

  (void) memset(env, 0, sizeof(*env));
  ActivateSemaphoreInfo(&env->lock);

  /* Check if Metal is disabled by environment variable */
  if (getenv("MAGICK_DISABLE_METAL") != NULL)
  {
    if (IsEventLogging() != MagickFalse)
      (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
        "Metal acceleration disabled by "
        "MAGICK_DISABLE_METAL environment variable");
    env->enabled = MagickFalse;
    return env;
  }

  /* Create default device */
  device = MTLCreateSystemDefaultDevice();
  if (device != nil)
  {
    if (IsEventLogging() != MagickFalse)
      (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
        "Metal device created successfully: %s",
        [[device name] UTF8String]);

    env->device = (MagickMetalDevice)
      AcquireMagickMemory(sizeof(*env->device));
    if (env->device != (MagickMetalDevice) NULL)
    {
      (void) memset(env->device, 0, sizeof(*env->device));
      ActivateSemaphoreInfo(&env->device->lock);
      env->device->device = (void *) CFBridgingRetain(device);

      queue = [device newCommandQueue];
      env->device->command_queue =
        (void *) CFBridgingRetain(queue);

      env->enabled = MagickTrue;
    }
  }
  else
  {
    if (IsEventLogging() != MagickFalse)
      (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
        "Failed to create Metal default device");
  }

  return env;
}

MagickPrivate MagickMetalDevice AcquireMagickMetalDevice(MagickMetalEnv env)
{
  if (env == (MagickMetalEnv) NULL)
    return (MagickMetalDevice) NULL;

  return env->device;
}

MagickPrivate void RelinquishMagickMetalDevice(MagickMetalDevice device)
{
  /* Currently we rely on the global env to hold the device */
}

MagickPrivate void *AcquireMetalBuffer(MagickMetalDevice device,
  size_t size, const void *data)
{
  id<MTLBuffer>
    buffer;

  id<MTLDevice>
    mtlDevice;

  if (device == (MagickMetalDevice) NULL)
    return NULL;

  mtlDevice = (__bridge id<MTLDevice>) device->device;
  buffer = nil;

  if (data != NULL)
    buffer = [mtlDevice newBufferWithBytes:data
      length:size
      options:MTLResourceStorageModeShared];
  else
    buffer = [mtlDevice newBufferWithLength:size
      options:MTLResourceStorageModeShared];

  return (void *) CFBridgingRetain(buffer);
}

/*
  AcquireMetalBufferNoCopy — wraps existing host memory as an MTLBuffer
  with zero copy. The caller must ensure the memory stays valid while the
  buffer is in use. The memory must be page-aligned.
*/
MagickPrivate void *AcquireMetalBufferNoCopy(MagickMetalDevice device,
  void *data, size_t size)
{
  id<MTLBuffer>
    buffer;

  id<MTLDevice>
    mtlDevice;

  if (device == (MagickMetalDevice) NULL || data == NULL)
    return NULL;

  mtlDevice = (__bridge id<MTLDevice>) device->device;
  buffer = [mtlDevice newBufferWithBytesNoCopy:data
    length:size options:MTLResourceStorageModeShared
    deallocator:nil];

  if (buffer == nil)
    return NULL;

  return (void *) CFBridgingRetain(buffer);
}

MagickPrivate void RelinquishMetalBuffer(void *buffer)
{
  if (buffer != NULL)
    CFRelease(buffer);
}

MagickPrivate void *GetMetalBufferContents(void *buffer)
{
  id<MTLBuffer>
    mtlBuffer;

  if (buffer == NULL)
    return NULL;
  mtlBuffer = (__bridge id<MTLBuffer>) buffer;
  return [mtlBuffer contents];
}

/*
  Pipeline cache — avoids recompiling Metal shaders on every kernel invocation.
  The cache is keyed by source pointer (all kernel source strings are const char*
  literals at fixed addresses) and kernel name.
*/
static MetalLibraryCacheEntry *FindOrCompileLibrary(
  MagickMetalDevice device, const char *source,
  NSError **error)
{
  id<MTLDevice>
    mtlDevice;

  id<MTLLibrary>
    library;

  MetalLibraryCacheEntry
    *entry;

  MTLCompileOptions
    *options;

  NSString
    *srcString;

  size_t
    i;

  /* Search for cached library by source pointer */
  for (i = 0; i < device->library_cache_count; i++)
  {
    if (device->library_cache[i].source == source)
      return &device->library_cache[i];
  }

  /* Compile new library */
  if (device->library_cache_count >= METAL_PIPELINE_CACHE_SIZE)
  {
    (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
      "Metal pipeline cache full");
    return NULL;
  }

  mtlDevice = (__bridge id<MTLDevice>) device->device;
  srcString = [NSString stringWithUTF8String:source];
  options = [[MTLCompileOptions alloc] init];

  library = [mtlDevice newLibraryWithSource:srcString
    options:options error:error];
  if (library == nil)
    return NULL;

  entry =
    &device->library_cache[device->library_cache_count++];
  entry->source = source;
  entry->library = (void *) CFBridgingRetain(library);
  entry->pipeline_count = 0;

  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
      "Compiled and cached Metal library "
      "(cache size: %zu)",
      device->library_cache_count);

  return entry;
}

static void *FindOrCreatePipeline(MagickMetalDevice device,
  MetalLibraryCacheEntry *entry, const char *kernelName,
  NSError **error)
{
  id<MTLComputePipelineState>
    pipeline;

  id<MTLDevice>
    mtlDevice;

  id<MTLFunction>
    function;

  id<MTLLibrary>
    library;

  MetalPipelineCacheEntry
    *pentry;

  NSString
    *nameString;

  size_t
    i;

  /* Search for cached pipeline by name */
  for (i = 0; i < entry->pipeline_count; i++)
  {
    if (strcmp(entry->pipelines[i].name, kernelName) == 0)
    {
      if (IsEventLogging() != MagickFalse)
        (void) LogMagickEvent(AccelerateEvent,
          GetMagickModule(),
          "Pipeline cache hit for kernel '%s'",
          kernelName);
      return entry->pipelines[i].pipeline;
    }
  }

  /* Create new pipeline */
  if (entry->pipeline_count >= 8)
  {
    (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
      "Pipeline cache full for this library");
    return NULL;
  }

  library = (__bridge id<MTLLibrary>) entry->library;
  nameString =
    [NSString stringWithUTF8String:kernelName];
  function = [library newFunctionWithName:nameString];
  if (function == nil)
  {
    (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
      "Metal kernel '%s' not found in library",
      kernelName);
    return NULL;
  }

  mtlDevice = (__bridge id<MTLDevice>) device->device;
  pipeline =
    [mtlDevice
      newComputePipelineStateWithFunction:function
      error:error];
  if (pipeline == nil)
    return NULL;

  pentry =
    &entry->pipelines[entry->pipeline_count++];
  strlcpy(pentry->name, kernelName, sizeof(pentry->name));
  pentry->pipeline = (void *) CFBridgingRetain(pipeline);

  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
      "Created and cached pipeline for kernel '%s'",
      kernelName);

  return pentry->pipeline;
}

MagickPrivate void *AcquireMetalKernel(MagickMetalDevice device,
  const char *source, const char *kernelName)
{
  const char
    *name;

  MetalLibraryCacheEntry
    *entry;

  NSError
    *error;

  void
    *pipeline;

  if (device == (MagickMetalDevice) NULL || source == NULL)
    return NULL;

  name = (kernelName != NULL) ? kernelName : "main_kernel";

  LockSemaphoreInfo(device->lock);

  error = nil;
  entry = FindOrCompileLibrary(device, source, &error);
  if (entry == NULL)
  {
    if (error != nil)
      (void) LogMagickEvent(AccelerateEvent,
        GetMagickModule(),
        "Metal compilation failed: %s",
        [[error localizedDescription] UTF8String]);
    UnlockSemaphoreInfo(device->lock);
    return NULL;
  }

  pipeline = FindOrCreatePipeline(device, entry, name,
    &error);
  if (pipeline == NULL && error != nil)
    (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
      "Metal pipeline creation failed: %s",
      [[error localizedDescription] UTF8String]);

  UnlockSemaphoreInfo(device->lock);
  return pipeline;
}

MagickPrivate void RelinquishMetalKernel(void *kernel)
{
  /* No-op: pipelines are owned by the cache and released at teardown */
  (void) kernel;
}

/*
  AcquireMetalKernelWithHelpers — concatenates helper source with kernel source,
  then compiles and caches the result. The combined source is allocated once and
  cached alongside the library entry.
*/
MagickPrivate void *AcquireMetalKernelWithHelpers(
  MagickMetalDevice device, const char *helperSource,
  const char *kernelSource, const char *kernelName)
{
  char
    *combined;

  id<MTLDevice>
    mtlDevice;

  id<MTLLibrary>
    library;

  MetalLibraryCacheEntry
    *entry;

  MTLCompileOptions
    *options;

  NSError
    *error;

  NSString
    *srcString;

  size_t
    helperLen,
    i,
    j,
    kernelLen;

  void
    *pipeline;

  if (device == (MagickMetalDevice) NULL ||
      kernelSource == NULL || kernelName == NULL)
    return NULL;

  LockSemaphoreInfo(device->lock);

  /*
    Check if this kernel is already cached (by name
    across all libraries).
  */
  for (i = 0; i < device->library_cache_count; i++)
  {
    for (j = 0;
         j < device->library_cache[i].pipeline_count; j++)
    {
      if (strcmp(
            device->library_cache[i].pipelines[j].name,
            kernelName) == 0)
      {
        if (IsEventLogging() != MagickFalse)
          (void) LogMagickEvent(AccelerateEvent,
            GetMagickModule(),
            "Pipeline cache hit for kernel '%s'",
            kernelName);
        UnlockSemaphoreInfo(device->lock);
        return
          device->library_cache[i].pipelines[j].pipeline;
      }
    }
  }

  /* Build combined source */
  helperLen = (helperSource != NULL) ?
    strlen(helperSource) : 0;
  kernelLen = strlen(kernelSource);
  combined = (char *)
    AcquireMagickMemory(helperLen + kernelLen + 1);
  if (combined == NULL)
  {
    UnlockSemaphoreInfo(device->lock);
    return NULL;
  }
  if (helperLen > 0)
    memcpy(combined, helperSource, helperLen);
  memcpy(combined + helperLen, kernelSource, kernelLen);
  combined[helperLen + kernelLen] = '\0';

  /* Compile the combined source */
  mtlDevice = (__bridge id<MTLDevice>) device->device;
  error = nil;
  srcString = [NSString stringWithUTF8String:combined];
  options = [[MTLCompileOptions alloc] init];

  library = [mtlDevice newLibraryWithSource:srcString
    options:options error:&error];

  if (library == nil)
  {
    if (error != nil)
      (void) LogMagickEvent(AccelerateEvent,
        GetMagickModule(),
        "Metal compilation failed: %s",
        [[error localizedDescription] UTF8String]);
    RelinquishMagickMemory(combined);
    UnlockSemaphoreInfo(device->lock);
    return NULL;
  }

  /* Cache the library */
  if (device->library_cache_count >=
      METAL_PIPELINE_CACHE_SIZE)
  {
    (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
      "Metal pipeline cache full");
    RelinquishMagickMemory(combined);
    UnlockSemaphoreInfo(device->lock);
    return NULL;
  }

  entry =
    &device->library_cache[device->library_cache_count++];
  /* cache takes ownership of the string */
  entry->source = combined;
  entry->library = (void *) CFBridgingRetain(library);
  entry->pipeline_count = 0;

  /* Create the pipeline */
  pipeline = FindOrCreatePipeline(device, entry,
    kernelName, &error);
  if (pipeline == NULL && error != nil)
    (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
      "Metal pipeline creation failed: %s",
      [[error localizedDescription] UTF8String]);

  UnlockSemaphoreInfo(device->lock);
  return pipeline;
}

/*
  EnqueueMetalKernel

  args: array of void* pointers to MTLBuffers or basic types (wrapped/handled carefully).
  For now, let's assume args contains pointers to id<MTLBuffer>.
  Handling scalars requires more complex signaling.
*/
MagickPrivate MagickBooleanType EnqueueMetalKernel(
  MagickMetalDevice device, void *kernel,
  const size_t global_size, const size_t local_size,
  void **args)
{
  id<MTLBuffer>
    buf;

  id<MTLCommandBuffer>
    commandBuffer;

  id<MTLCommandQueue>
    queue;

  id<MTLComputeCommandEncoder>
    encoder;

  id<MTLComputePipelineState>
    pipeline;

  int
    i;

  MTLSize
    gridSize,
    threadGroupSize;

  NSUInteger
    threadGroupW;

  if (device == (MagickMetalDevice) NULL || kernel == NULL)
    return MagickFalse;

  queue = (__bridge id<MTLCommandQueue>)
    device->command_queue;
  pipeline = (__bridge id<MTLComputePipelineState>) kernel;

  commandBuffer = [queue commandBuffer];
  encoder = [commandBuffer computeCommandEncoder];

  [encoder setComputePipelineState:pipeline];

  /*
    Set arguments -- assumes all are buffers with a
    NULL sentinel.
  */
  for (i = 0; args[i] != NULL; i++)
  {
    buf = (__bridge id<MTLBuffer>) args[i];
    [encoder setBuffer:buf offset:0 atIndex:i];
  }

  gridSize = MTLSizeMake(global_size, 1, 1);

  /* Calculate threadgroup size */
  threadGroupW = pipeline.maxTotalThreadsPerThreadgroup;
  if (threadGroupW > local_size)
    threadGroupW = local_size;
  if (threadGroupW == 0)
    threadGroupW = 1;
  threadGroupSize = MTLSizeMake(threadGroupW, 1, 1);

  [encoder dispatchThreads:gridSize
    threadsPerThreadgroup:threadGroupSize];
  [encoder endEncoding];
  [commandBuffer commit];
  [commandBuffer waitUntilCompleted];

  return MagickTrue;
}

MagickPrivate MagickBooleanType EnqueueMetalKernel2D(
  MagickMetalDevice device, void *kernel,
  const size_t *global_work_size,
  const size_t *local_work_size,
  const size_t shared_memory_size, void **args)
{
  id<MTLCommandBuffer>
    commandBuffer;

  id<MTLCommandQueue>
    queue;

  id<MTLComputeCommandEncoder>
    computeEncoder;

  id<MTLComputePipelineState>
    pipelineState;

  id
    object;

  int
    i;

  MTLSize
    gridSize,
    threadgroupSize;

  if (device == (MagickMetalDevice) NULL || kernel == NULL)
    return MagickFalse;

  queue = (__bridge id<MTLCommandQueue>)
    device->command_queue;
  pipelineState =
    (__bridge id<MTLComputePipelineState>) kernel;
  commandBuffer = [queue commandBuffer];
  computeEncoder =
    [commandBuffer computeCommandEncoder];

  [computeEncoder setComputePipelineState:pipelineState];

  i = 0;
  while (args[i] != NULL)
  {
    object = (__bridge id) args[i];
    if ([object
          conformsToProtocol:@protocol(MTLBuffer)])
    {
      [computeEncoder setBuffer:object
        offset:0 atIndex:i];
    }
    i++;
  }

  /*
    Set threadgroup memory for dynamically sized
    shared memory (e.g. local memory in OpenCL blur).
  */
  if (shared_memory_size > 0)
  {
    [computeEncoder
      setThreadgroupMemoryLength:shared_memory_size
      atIndex:0];
  }

  gridSize = MTLSizeMake(global_work_size[0],
    global_work_size[1], 1);
  threadgroupSize = MTLSizeMake(local_work_size[0],
    local_work_size[1], 1);

  /* Verify threadgroup size doesn't exceed limits */
  if (threadgroupSize.width * threadgroupSize.height >
      pipelineState.maxTotalThreadsPerThreadgroup)
  {
    [computeEncoder endEncoding];
    return MagickFalse;
  }

  [computeEncoder dispatchThreads:gridSize
    threadsPerThreadgroup:threadgroupSize];

  [computeEncoder endEncoding];
  [commandBuffer commit];
  [commandBuffer waitUntilCompleted];

  return MagickTrue;
}

/*
  EnqueueMetalKernel2DEx — dispatch with typed arguments (buffer or scalar bytes).
*/
MagickPrivate MagickBooleanType EnqueueMetalKernel2DEx(
  MagickMetalDevice device, void *kernel,
  const size_t *global_work_size,
  const size_t *local_work_size,
  const size_t shared_memory_size,
  MetalKernelArg *args, size_t arg_count)
{
  id<MTLBuffer>
    buf;

  id<MTLCommandBuffer>
    commandBuffer;

  id<MTLCommandQueue>
    queue;

  id<MTLComputeCommandEncoder>
    encoder;

  id<MTLComputePipelineState>
    pipelineState;

  MTLSize
    gridSize,
    threadgroupSize;

  size_t
    i;

  if (device == (MagickMetalDevice) NULL || kernel == NULL)
    return MagickFalse;

  queue = (__bridge id<MTLCommandQueue>)
    device->command_queue;
  pipelineState =
    (__bridge id<MTLComputePipelineState>) kernel;
  commandBuffer = [queue commandBuffer];
  encoder = [commandBuffer computeCommandEncoder];

  [encoder setComputePipelineState:pipelineState];

  for (i = 0; i < arg_count; i++)
  {
    if (args[i].type == MetalArgBuffer)
    {
      buf = (__bridge id<MTLBuffer>) args[i].buffer;
      [encoder setBuffer:buf offset:0 atIndex:i];
    }
    else /* MetalArgBytes */
    {
      [encoder setBytes:args[i].bytes.data
        length:args[i].bytes.size atIndex:i];
    }
  }

  if (shared_memory_size > 0)
    [encoder
      setThreadgroupMemoryLength:shared_memory_size
      atIndex:0];

  gridSize = MTLSizeMake(global_work_size[0],
    global_work_size[1], 1);
  threadgroupSize = MTLSizeMake(local_work_size[0],
    local_work_size[1], 1);

  if (threadgroupSize.width * threadgroupSize.height >
      pipelineState.maxTotalThreadsPerThreadgroup)
  {
    [encoder endEncoding];
    return MagickFalse;
  }

  [encoder dispatchThreads:gridSize
    threadsPerThreadgroup:threadgroupSize];
  [encoder endEncoding];
  [commandBuffer commit];
  [commandBuffer waitUntilCompleted];

  return MagickTrue;
}

#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                      CCCC   AAA    CCCC  H   H  EEEEE                       %
%                     C      A   A  C      H   H  E                           %
%                     C      AAAAA  C      HHHHH  EEE                         %
%                     C      A   A  C      H   H  E                           %
%                      CCCC  A   A   CCCC  H   H  EEEEE                       %
%                                                                             %
%                                                                             %
%                       MagickCore Pixel Cache Methods                        %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1999                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/distribute-cache-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/policy.h"
#include "MagickCore/quantum.h"
#include "MagickCore/random_.h"
#include "MagickCore/registry.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#if defined(MAGICKCORE_ZLIB_DELEGATE)
#include "zlib.h"
#endif

/*
  Define declarations.
*/
#define CacheTick(offset,extent)  QuantumTick((MagickOffsetType) offset,extent)
#define IsFileDescriptorLimitExceeded() (GetMagickResource(FileResource) > \
  GetMagickResourceLimit(FileResource) ? MagickTrue : MagickFalse)

/*
  Typedef declarations.
*/
typedef struct _MagickModulo
{
  ssize_t
    quotient,
    remainder;
} MagickModulo;

/*
  Forward declarations.
*/
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static Cache
  GetImagePixelCache(Image *,const MagickBooleanType,ExceptionInfo *)
    magick_hot_spot;

static const Quantum
  *GetVirtualPixelCache(const Image *,const VirtualPixelMethod,const ssize_t,
    const ssize_t,const size_t,const size_t,ExceptionInfo *),
  *GetVirtualPixelsCache(const Image *);

static const void
  *GetVirtualMetacontentFromCache(const Image *);

static MagickBooleanType
  GetOneAuthenticPixelFromCache(Image *,const ssize_t,const ssize_t,Quantum *,
    ExceptionInfo *),
  GetOneVirtualPixelFromCache(const Image *,const VirtualPixelMethod,
    const ssize_t,const ssize_t,Quantum *,ExceptionInfo *),
  OpenPixelCache(Image *,const MapMode,ExceptionInfo *),
  OpenPixelCacheOnDisk(CacheInfo *,const MapMode),
  ReadPixelCachePixels(CacheInfo *magick_restrict,NexusInfo *magick_restrict,
    ExceptionInfo *),
  ReadPixelCacheMetacontent(CacheInfo *magick_restrict,
    NexusInfo *magick_restrict,ExceptionInfo *),
  SyncAuthenticPixelsCache(Image *,ExceptionInfo *),
  WritePixelCachePixels(CacheInfo *magick_restrict,NexusInfo *magick_restrict,
    ExceptionInfo *),
  WritePixelCacheMetacontent(CacheInfo *,NexusInfo *magick_restrict,
    ExceptionInfo *);

static Quantum
  *GetAuthenticPixelsCache(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,ExceptionInfo *),
  *QueueAuthenticPixelsCache(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,ExceptionInfo *),
  *SetPixelCacheNexusPixels(const CacheInfo *,const MapMode,
    const RectangleInfo *,NexusInfo *,ExceptionInfo *) magick_hot_spot;

#if defined(MAGICKCORE_OPENCL_SUPPORT)
static void
  CopyOpenCLBuffer(CacheInfo *magick_restrict);
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

/*
  Global declarations.
*/
static volatile MagickBooleanType
  instantiate_cache = MagickFalse;

static SemaphoreInfo
  *cache_semaphore = (SemaphoreInfo *) NULL;

static ssize_t
  cache_anonymous_memory = (-1);

static time_t
  cache_epoch = 0;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e P i x e l C a c h e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquirePixelCache() acquires a pixel cache.
%
%  The format of the AcquirePixelCache() method is:
%
%      Cache AcquirePixelCache(const size_t number_threads)
%
%  A description of each parameter follows:
%
%    o number_threads: the number of nexus threads.
%
*/
MagickPrivate Cache AcquirePixelCache(const size_t number_threads)
{
  CacheInfo
    *magick_restrict cache_info;

  char
    *value;

  cache_info=(CacheInfo *) AcquireQuantumMemory(1,sizeof(*cache_info));
  if (cache_info == (CacheInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(cache_info,0,sizeof(*cache_info));
  cache_info->type=UndefinedCache;
  cache_info->mode=IOMode;
  cache_info->colorspace=sRGBColorspace;
  cache_info->file=(-1);
  cache_info->id=GetMagickThreadId();
  cache_info->number_threads=number_threads;
  if (GetOpenMPMaximumThreads() > cache_info->number_threads)
    cache_info->number_threads=GetOpenMPMaximumThreads();
  if (GetMagickResourceLimit(ThreadResource) > cache_info->number_threads)
    cache_info->number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  if (cache_info->number_threads == 0)
    cache_info->number_threads=1;
  cache_info->nexus_info=AcquirePixelCacheNexus(cache_info->number_threads);
  if (cache_info->nexus_info == (NexusInfo **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  value=GetEnvironmentValue("MAGICK_SYNCHRONIZE");
  if (value != (const char *) NULL)
    {
      cache_info->synchronize=IsStringTrue(value);
      value=DestroyString(value);
    }
  value=GetPolicyValue("cache:synchronize");
  if (value != (const char *) NULL)
    {
      cache_info->synchronize=IsStringTrue(value);
      value=DestroyString(value);
    }
  cache_info->semaphore=AcquireSemaphoreInfo();
  cache_info->reference_count=1;
  cache_info->file_semaphore=AcquireSemaphoreInfo();
  cache_info->debug=IsEventLogging();
  cache_info->signature=MagickCoreSignature;
  return((Cache ) cache_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e P i x e l C a c h e N e x u s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquirePixelCacheNexus() allocates the NexusInfo structure.
%
%  The format of the AcquirePixelCacheNexus method is:
%
%      NexusInfo **AcquirePixelCacheNexus(const size_t number_threads)
%
%  A description of each parameter follows:
%
%    o number_threads: the number of nexus threads.
%
*/
MagickPrivate NexusInfo **AcquirePixelCacheNexus(const size_t number_threads)
{
  NexusInfo
    **magick_restrict nexus_info;

  register ssize_t
    i;

  nexus_info=(NexusInfo **) MagickAssumeAligned(AcquireAlignedMemory(
    number_threads,sizeof(*nexus_info)));
  if (nexus_info == (NexusInfo **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  nexus_info[0]=(NexusInfo *) AcquireQuantumMemory(number_threads,
    sizeof(**nexus_info));
  if (nexus_info[0] == (NexusInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(nexus_info[0],0,number_threads*sizeof(**nexus_info));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    nexus_info[i]=(&nexus_info[0][i]);
    nexus_info[i]->signature=MagickCoreSignature;
  }
  return(nexus_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e P i x e l C a c h e P i x e l s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquirePixelCachePixels() returns the pixels associated with the specified
%  image.
%
%  The format of the AcquirePixelCachePixels() method is:
%
%      const void *AcquirePixelCachePixels(const Image *image,
%        MagickSizeType *length,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o length: the pixel cache length.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate const void *AcquirePixelCachePixels(const Image *image,
  MagickSizeType *length,ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  *length=0;
  if ((cache_info->type != MemoryCache) && (cache_info->type != MapCache))
    return((const void *) NULL);
  *length=cache_info->length;
  return((const void *) cache_info->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C a c h e C o m p o n e n t G e n e s i s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CacheComponentGenesis() instantiates the cache component.
%
%  The format of the CacheComponentGenesis method is:
%
%      MagickBooleanType CacheComponentGenesis(void)
%
*/
MagickPrivate MagickBooleanType CacheComponentGenesis(void)
{
  if (cache_semaphore == (SemaphoreInfo *) NULL)
    cache_semaphore=AcquireSemaphoreInfo();
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C a c h e C o m p o n e n t T e r m i n u s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CacheComponentTerminus() destroys the cache component.
%
%  The format of the CacheComponentTerminus() method is:
%
%      CacheComponentTerminus(void)
%
*/
MagickPrivate void CacheComponentTerminus(void)
{
  if (cache_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&cache_semaphore);
  LockSemaphoreInfo(cache_semaphore);
  instantiate_cache=MagickFalse;
  UnlockSemaphoreInfo(cache_semaphore);
  RelinquishSemaphoreInfo(&cache_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l o n e P i x e l C a c h e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClonePixelCache() clones a pixel cache.
%
%  The format of the ClonePixelCache() method is:
%
%      Cache ClonePixelCache(const Cache cache)
%
%  A description of each parameter follows:
%
%    o cache: the pixel cache.
%
*/
MagickPrivate Cache ClonePixelCache(const Cache cache)
{
  CacheInfo
    *magick_restrict clone_info;

  const CacheInfo
    *magick_restrict cache_info;

  assert(cache != NULL);
  cache_info=(const CacheInfo *) cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_info->filename);
  clone_info=(CacheInfo *) AcquirePixelCache(cache_info->number_threads);
  if (clone_info == (Cache) NULL)
    return((Cache) NULL);
  clone_info->virtual_pixel_method=cache_info->virtual_pixel_method;
  return((Cache ) clone_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l o n e P i x e l C a c h e M e t h o d s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClonePixelCacheMethods() clones the pixel cache methods from one cache to
%  another.
%
%  The format of the ClonePixelCacheMethods() method is:
%
%      void ClonePixelCacheMethods(Cache clone,const Cache cache)
%
%  A description of each parameter follows:
%
%    o clone: Specifies a pointer to a Cache structure.
%
%    o cache: the pixel cache.
%
*/
MagickPrivate void ClonePixelCacheMethods(Cache clone,const Cache cache)
{
  CacheInfo
    *magick_restrict cache_info,
    *magick_restrict source_info;

  assert(clone != (Cache) NULL);
  source_info=(CacheInfo *) clone;
  assert(source_info->signature == MagickCoreSignature);
  if (source_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      source_info->filename);
  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickCoreSignature);
  source_info->methods=cache_info->methods;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l o n e P i x e l C a c h e R e p o s i t o r y                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%  ClonePixelCacheRepository() clones the source pixel cache to the destination
%  cache.
%
%  The format of the ClonePixelCacheRepository() method is:
%
%      MagickBooleanType ClonePixelCacheRepository(CacheInfo *cache_info,
%        CacheInfo *source_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_info: the pixel cache.
%
%    o source_info: the source pixel cache.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType ClonePixelCacheOnDisk(
  CacheInfo *magick_restrict cache_info,CacheInfo *magick_restrict clone_info)
{
  MagickSizeType
    extent;

  size_t
    quantum;

  ssize_t
    count;

  struct stat
    file_stats;

  unsigned char
    *buffer;

  /*
    Clone pixel cache on disk with identical morphology.
  */
  if ((OpenPixelCacheOnDisk(cache_info,ReadMode) == MagickFalse) ||
      (OpenPixelCacheOnDisk(clone_info,IOMode) == MagickFalse))
    return(MagickFalse);
  quantum=(size_t) MagickMaxBufferExtent;
  if ((fstat(cache_info->file,&file_stats) == 0) && (file_stats.st_size > 0))
    quantum=(size_t) MagickMin(file_stats.st_size,MagickMaxBufferExtent);
  buffer=(unsigned char *) AcquireQuantumMemory(quantum,sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  extent=0;
  while ((count=read(cache_info->file,buffer,quantum)) > 0)
  {
    ssize_t
      number_bytes;

    number_bytes=write(clone_info->file,buffer,(size_t) count);
    if (number_bytes != count)
      break;
    extent+=number_bytes;
  }
  buffer=(unsigned char *) RelinquishMagickMemory(buffer);
  if (extent != cache_info->length)
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ClonePixelCacheRepository(
  CacheInfo *magick_restrict clone_info,CacheInfo *magick_restrict cache_info,
  ExceptionInfo *exception)
{
#define MaxCacheThreads  2
#define cache_threads(source,destination) \
  num_threads(((source)->type == DiskCache) || \
    ((destination)->type == DiskCache) || (((source)->rows) < \
    (16*GetMagickResourceLimit(ThreadResource))) ? 1 : \
    GetMagickResourceLimit(ThreadResource) < MaxCacheThreads ? \
    GetMagickResourceLimit(ThreadResource) : MaxCacheThreads)

  MagickBooleanType
    optimize,
    status;

  NexusInfo
    **magick_restrict cache_nexus,
    **magick_restrict clone_nexus;

  size_t
    length;

  ssize_t
    y;

  assert(cache_info != (CacheInfo *) NULL);
  assert(clone_info != (CacheInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  if (cache_info->type == PingCache)
    return(MagickTrue);
  length=cache_info->number_channels*sizeof(*cache_info->channel_map);
  if ((cache_info->columns == clone_info->columns) &&
      (cache_info->rows == clone_info->rows) &&
      (cache_info->number_channels == clone_info->number_channels) &&
      (memcmp(cache_info->channel_map,clone_info->channel_map,length) == 0) &&
      (cache_info->metacontent_extent == clone_info->metacontent_extent))
    {
      /*
        Identical pixel cache morphology.
      */
      if (((cache_info->type == MemoryCache) ||
           (cache_info->type == MapCache)) &&
          ((clone_info->type == MemoryCache) ||
           (clone_info->type == MapCache)))
        {
          (void) memcpy(clone_info->pixels,cache_info->pixels,
            cache_info->columns*cache_info->number_channels*cache_info->rows*
            sizeof(*cache_info->pixels));
          if ((cache_info->metacontent_extent != 0) &&
              (clone_info->metacontent_extent != 0))
            (void) memcpy(clone_info->metacontent,cache_info->metacontent,
              cache_info->columns*cache_info->rows*
              clone_info->metacontent_extent*sizeof(unsigned char));
          return(MagickTrue);
        }
      if ((cache_info->type == DiskCache) && (clone_info->type == DiskCache))
        return(ClonePixelCacheOnDisk(cache_info,clone_info));
    }
  /*
    Mismatched pixel cache morphology.
  */
  cache_nexus=AcquirePixelCacheNexus(MaxCacheThreads);
  clone_nexus=AcquirePixelCacheNexus(MaxCacheThreads);
  if ((cache_nexus == (NexusInfo **) NULL) ||
      (clone_nexus == (NexusInfo **) NULL))
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  length=cache_info->number_channels*sizeof(*cache_info->channel_map);
  optimize=(cache_info->number_channels == clone_info->number_channels) &&
    (memcmp(cache_info->channel_map,clone_info->channel_map,length) == 0) ?
    MagickTrue : MagickFalse;
  length=(size_t) MagickMin(cache_info->columns*cache_info->number_channels,
    clone_info->columns*clone_info->number_channels);
  status=MagickTrue;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    cache_threads(cache_info,clone_info)
#endif
  for (y=0; y < (ssize_t) cache_info->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    Quantum
      *pixels;

    RectangleInfo
      region;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    if (y >= (ssize_t) clone_info->rows)
      continue;
    region.width=cache_info->columns;
    region.height=1;
    region.x=0;
    region.y=y;
    pixels=SetPixelCacheNexusPixels(cache_info,ReadMode,&region,
      cache_nexus[id],exception);
    if (pixels == (Quantum *) NULL)
      continue;
    status=ReadPixelCachePixels(cache_info,cache_nexus[id],exception);
    if (status == MagickFalse)
      continue;
    region.width=clone_info->columns;
    pixels=SetPixelCacheNexusPixels(clone_info,WriteMode,&region,
      clone_nexus[id],exception);
    if (pixels == (Quantum *) NULL)
      continue;
    (void) ResetMagickMemory(clone_nexus[id]->pixels,0,(size_t)
      clone_nexus[id]->length);
    if (optimize != MagickFalse)
      (void) memcpy(clone_nexus[id]->pixels,cache_nexus[id]->pixels,length*
        sizeof(Quantum));
    else
      {
        register const Quantum
          *magick_restrict p;

        register Quantum
          *magick_restrict q;

        /*
          Mismatched pixel channel map.
        */
        p=cache_nexus[id]->pixels;
        q=clone_nexus[id]->pixels;
        for (x=0; x < (ssize_t) cache_info->columns; x++)
        {
          register ssize_t
            i;

          if (x == (ssize_t) clone_info->columns)
            break;
          for (i=0; i < (ssize_t) clone_info->number_channels; i++)
          {
            PixelChannel
              channel;

            PixelTrait
              traits;

            channel=clone_info->channel_map[i].channel;
            traits=cache_info->channel_map[channel].traits;
            if (traits != UndefinedPixelTrait)
              *q=*(p+cache_info->channel_map[channel].offset);
            q++;
          }
          p+=cache_info->number_channels;
        }
      }
    status=WritePixelCachePixels(clone_info,clone_nexus[id],exception);
  }
  if ((cache_info->metacontent_extent != 0) &&
      (clone_info->metacontent_extent != 0))
    {
      /*
        Clone metacontent.
      */
      length=(size_t) MagickMin(cache_info->metacontent_extent,
        clone_info->metacontent_extent);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        cache_threads(cache_info,clone_info)
#endif
      for (y=0; y < (ssize_t) cache_info->rows; y++)
      {
        const int
          id = GetOpenMPThreadId();

        Quantum
          *pixels;

        RectangleInfo
          region;

        if (status == MagickFalse)
          continue;
        if (y >= (ssize_t) clone_info->rows)
          continue;
        region.width=cache_info->columns;
        region.height=1;
        region.x=0;
        region.y=y;
        pixels=SetPixelCacheNexusPixels(cache_info,ReadMode,&region,
          cache_nexus[id],exception);
        if (pixels == (Quantum *) NULL)
          continue;
        status=ReadPixelCacheMetacontent(cache_info,cache_nexus[id],exception);
        if (status == MagickFalse)
          continue;
        region.width=clone_info->columns;
        pixels=SetPixelCacheNexusPixels(clone_info,WriteMode,&region,
          clone_nexus[id],exception);
        if (pixels == (Quantum *) NULL)
          continue;
        if ((clone_nexus[id]->metacontent != (void *) NULL) &&
            (cache_nexus[id]->metacontent != (void *) NULL))
          (void) memcpy(clone_nexus[id]->metacontent,
            cache_nexus[id]->metacontent,length*sizeof(unsigned char));
        status=WritePixelCacheMetacontent(clone_info,clone_nexus[id],exception);
      }
    }
  cache_nexus=DestroyPixelCacheNexus(cache_nexus,MaxCacheThreads);
  clone_nexus=DestroyPixelCacheNexus(clone_nexus,MaxCacheThreads);
  if (cache_info->debug != MagickFalse)
    {
      char
        message[MagickPathExtent];

      (void) FormatLocaleString(message,MagickPathExtent,"%s => %s",
        CommandOptionToMnemonic(MagickCacheOptions,(ssize_t) cache_info->type),
        CommandOptionToMnemonic(MagickCacheOptions,(ssize_t) clone_info->type));
      (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",message);
    }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y I m a g e P i x e l C a c h e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImagePixelCache() deallocates memory associated with the pixel cache.
%
%  The format of the DestroyImagePixelCache() method is:
%
%      void DestroyImagePixelCache(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
static void DestroyImagePixelCache(Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->cache == (void *) NULL)
    return;
  image->cache=DestroyPixelCache(image->cache);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y I m a g e P i x e l s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImagePixels() deallocates memory associated with the pixel cache.
%
%  The format of the DestroyImagePixels() method is:
%
%      void DestroyImagePixels(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void DestroyImagePixels(Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->methods.destroy_pixel_handler != (DestroyPixelHandler) NULL)
    {
      cache_info->methods.destroy_pixel_handler(image);
      return;
    }
  image->cache=DestroyPixelCache(image->cache);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y P i x e l C a c h e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyPixelCache() deallocates memory associated with the pixel cache.
%
%  The format of the DestroyPixelCache() method is:
%
%      Cache DestroyPixelCache(Cache cache)
%
%  A description of each parameter follows:
%
%    o cache: the pixel cache.
%
*/

static MagickBooleanType ClosePixelCacheOnDisk(CacheInfo *cache_info)
{
  int
    status;

  status=(-1);
  if (cache_info->file != -1)
    {
      status=close(cache_info->file);
      cache_info->file=(-1);
      RelinquishMagickResource(FileResource,1);
    }
  return(status == -1 ? MagickFalse : MagickTrue);
}

static inline void RelinquishPixelCachePixels(CacheInfo *cache_info)
{
  switch (cache_info->type)
  {
    case MemoryCache:
    {
#if defined(MAGICKCORE_OPENCL_SUPPORT)
      if (cache_info->opencl != (MagickCLCacheInfo) NULL)
        {
          cache_info->opencl=RelinquishMagickCLCacheInfo(cache_info->opencl,
            MagickTrue);
          cache_info->pixels=(Quantum *) NULL;
          break;
        }
#endif
      if (cache_info->mapped == MagickFalse)
        cache_info->pixels=(Quantum *) RelinquishAlignedMemory(
          cache_info->pixels);
      else
        (void) UnmapBlob(cache_info->pixels,(size_t) cache_info->length);
      RelinquishMagickResource(MemoryResource,cache_info->length);
      break;
    }
    case MapCache:
    {
      (void) UnmapBlob(cache_info->pixels,(size_t) cache_info->length);
      cache_info->pixels=(Quantum *) NULL;
      if (cache_info->mode != ReadMode)
        (void) RelinquishUniqueFileResource(cache_info->cache_filename);
      *cache_info->cache_filename='\0';
      RelinquishMagickResource(MapResource,cache_info->length);
    }
    case DiskCache:
    {
      if (cache_info->file != -1)
        (void) ClosePixelCacheOnDisk(cache_info);
      if (cache_info->mode != ReadMode)
        (void) RelinquishUniqueFileResource(cache_info->cache_filename);
      *cache_info->cache_filename='\0';
      RelinquishMagickResource(DiskResource,cache_info->length);
      break;
    }
    case DistributedCache:
    {
      *cache_info->cache_filename='\0';
      (void) RelinquishDistributePixelCache((DistributeCacheInfo *)
        cache_info->server_info);
      break;
    }
    default:
      break;
  }
  cache_info->type=UndefinedCache;
  cache_info->mapped=MagickFalse;
  cache_info->metacontent=(void *) NULL;
}

MagickPrivate Cache DestroyPixelCache(Cache cache)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_info->filename);
  LockSemaphoreInfo(cache_info->semaphore);
  cache_info->reference_count--;
  if (cache_info->reference_count != 0)
    {
      UnlockSemaphoreInfo(cache_info->semaphore);
      return((Cache) NULL);
    }
  UnlockSemaphoreInfo(cache_info->semaphore);
  if (cache_info->debug != MagickFalse)
    {
      char
        message[MagickPathExtent];

      (void) FormatLocaleString(message,MagickPathExtent,"destroy %s",
        cache_info->filename);
      (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",message);
    }
  RelinquishPixelCachePixels(cache_info);
  if (cache_info->server_info != (DistributeCacheInfo *) NULL)
    cache_info->server_info=DestroyDistributeCacheInfo((DistributeCacheInfo *)
      cache_info->server_info);
  if (cache_info->nexus_info != (NexusInfo **) NULL)
    cache_info->nexus_info=DestroyPixelCacheNexus(cache_info->nexus_info,
      cache_info->number_threads);
  if (cache_info->random_info != (RandomInfo *) NULL)
    cache_info->random_info=DestroyRandomInfo(cache_info->random_info);
  if (cache_info->file_semaphore != (SemaphoreInfo *) NULL)
    RelinquishSemaphoreInfo(&cache_info->file_semaphore);
  if (cache_info->semaphore != (SemaphoreInfo *) NULL)
    RelinquishSemaphoreInfo(&cache_info->semaphore);
  cache_info->signature=(~MagickCoreSignature);
  cache_info=(CacheInfo *) RelinquishMagickMemory(cache_info);
  cache=(Cache) NULL;
  return(cache);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y P i x e l C a c h e N e x u s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyPixelCacheNexus() destroys a pixel cache nexus.
%
%  The format of the DestroyPixelCacheNexus() method is:
%
%      NexusInfo **DestroyPixelCacheNexus(NexusInfo *nexus_info,
%        const size_t number_threads)
%
%  A description of each parameter follows:
%
%    o nexus_info: the nexus to destroy.
%
%    o number_threads: the number of nexus threads.
%
*/

static inline void RelinquishCacheNexusPixels(NexusInfo *nexus_info)
{
  if (nexus_info->mapped == MagickFalse)
    (void) RelinquishAlignedMemory(nexus_info->cache);
  else
    (void) UnmapBlob(nexus_info->cache,(size_t) nexus_info->length);
  nexus_info->cache=(Quantum *) NULL;
  nexus_info->pixels=(Quantum *) NULL;
  nexus_info->metacontent=(void *) NULL;
  nexus_info->length=0;
  nexus_info->mapped=MagickFalse;
}

MagickPrivate NexusInfo **DestroyPixelCacheNexus(NexusInfo **nexus_info,
  const size_t number_threads)
{
  register ssize_t
    i;

  assert(nexus_info != (NexusInfo **) NULL);
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    if (nexus_info[i]->cache != (Quantum *) NULL)
      RelinquishCacheNexusPixels(nexus_info[i]);
    nexus_info[i]->signature=(~MagickCoreSignature);
  }
  nexus_info[0]=(NexusInfo *) RelinquishMagickMemory(nexus_info[0]);
  nexus_info=(NexusInfo **) RelinquishAlignedMemory(nexus_info);
  return(nexus_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t A u t h e n t i c M e t a c o n t e n t                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticMetacontent() returns the authentic metacontent corresponding
%  with the last call to QueueAuthenticPixels() or GetVirtualPixels().  NULL is
%  returned if the associated pixels are not available.
%
%  The format of the GetAuthenticMetacontent() method is:
%
%      void *GetAuthenticMetacontent(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void *GetAuthenticMetacontent(const Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->methods.get_authentic_metacontent_from_handler !=
      (GetAuthenticMetacontentFromHandler) NULL)
    {
      void
        *metacontent;

      metacontent=cache_info->methods.
        get_authentic_metacontent_from_handler(image);
      return(metacontent);
    }
  assert(id < (int) cache_info->number_threads);
  return(cache_info->nexus_info[id]->metacontent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t A u t h e n t i c M e t a c o n t e n t F r o m C a c h e           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticMetacontentFromCache() returns the meta-content corresponding
%  with the last call to QueueAuthenticPixelsCache() or
%  GetAuthenticPixelsCache().
%
%  The format of the GetAuthenticMetacontentFromCache() method is:
%
%      void *GetAuthenticMetacontentFromCache(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
static void *GetAuthenticMetacontentFromCache(const Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  assert(id < (int) cache_info->number_threads);
  return(cache_info->nexus_info[id]->metacontent);
}

#if defined(MAGICKCORE_OPENCL_SUPPORT)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t A u t h e n t i c O p e n C L B u f f e r                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticOpenCLBuffer() returns an OpenCL buffer used to execute OpenCL
%  operations.
%
%  The format of the GetAuthenticOpenCLBuffer() method is:
%
%      cl_mem GetAuthenticOpenCLBuffer(const Image *image,
%        MagickCLDevice device,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o device: the device to use.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate cl_mem GetAuthenticOpenCLBuffer(const Image *image,
  MagickCLDevice device,ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  cl_int
    status;

  assert(image != (const Image *) NULL);
  assert(device != (const MagickCLDevice) NULL);
  cache_info=(CacheInfo *) image->cache;
  if (cache_info->type == UndefinedCache)
    SyncImagePixelCache((Image *) image,exception);
  if ((cache_info->type != MemoryCache) || (cache_info->mapped != MagickFalse))
    return((cl_mem) NULL);
  if ((cache_info->opencl != (MagickCLCacheInfo) NULL) &&
      (cache_info->opencl->device->context != device->context))
    cache_info->opencl=CopyMagickCLCacheInfo(cache_info->opencl);
  if (cache_info->opencl == (MagickCLCacheInfo) NULL)
    {
      assert(cache_info->pixels != (Quantum *) NULL);
      cache_info->opencl=AcquireMagickCLCacheInfo(device,cache_info->pixels,
        cache_info->length);
      if (cache_info->opencl == (MagickCLCacheInfo) NULL)
        return((cl_mem) NULL);
    }
  assert(cache_info->opencl->pixels == cache_info->pixels);
  return(cache_info->opencl->buffer);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t A u t h e n t i c P i x e l C a c h e N e x u s                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticPixelCacheNexus() gets authentic pixels from the in-memory or
%  disk pixel cache as defined by the geometry parameters.   A pointer to the
%  pixels is returned if the pixels are transferred, otherwise a NULL is
%  returned.
%
%  The format of the GetAuthenticPixelCacheNexus() method is:
%
%      Quantum *GetAuthenticPixelCacheNexus(Image *image,const ssize_t x,
%        const ssize_t y,const size_t columns,const size_t rows,
%        NexusInfo *nexus_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o nexus_info: the cache nexus to return.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickPrivate Quantum *GetAuthenticPixelCacheNexus(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,NexusInfo *nexus_info,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  Quantum
    *magick_restrict pixels;

  /*
    Transfer pixels from the cache.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  pixels=QueueAuthenticPixelCacheNexus(image,x,y,columns,rows,MagickTrue,
    nexus_info,exception);
  if (pixels == (Quantum *) NULL)
    return((Quantum *) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (nexus_info->authentic_pixel_cache != MagickFalse)
    return(pixels);
  if (ReadPixelCachePixels(cache_info,nexus_info,exception) == MagickFalse)
    return((Quantum *) NULL);
  if (cache_info->metacontent_extent != 0)
    if (ReadPixelCacheMetacontent(cache_info,nexus_info,exception) == MagickFalse)
      return((Quantum *) NULL);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t A u t h e n t i c P i x e l s F r o m C a c h e                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticPixelsFromCache() returns the pixels associated with the last
%  call to the QueueAuthenticPixelsCache() or GetAuthenticPixelsCache() methods.
%
%  The format of the GetAuthenticPixelsFromCache() method is:
%
%      Quantum *GetAuthenticPixelsFromCache(const Image image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
static Quantum *GetAuthenticPixelsFromCache(const Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  assert(id < (int) cache_info->number_threads);
  return(cache_info->nexus_info[id]->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t A u t h e n t i c P i x e l Q u e u e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticPixelQueue() returns the authentic pixels associated
%  corresponding with the last call to QueueAuthenticPixels() or
%  GetAuthenticPixels().
%
%  The format of the GetAuthenticPixelQueue() method is:
%
%      Quantum *GetAuthenticPixelQueue(const Image image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport Quantum *GetAuthenticPixelQueue(const Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->methods.get_authentic_pixels_from_handler !=
       (GetAuthenticPixelsFromHandler) NULL)
    return(cache_info->methods.get_authentic_pixels_from_handler(image));
  assert(id < (int) cache_info->number_threads);
  return(cache_info->nexus_info[id]->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t A u t h e n t i c P i x e l s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticPixels() obtains a pixel region for read/write access. If the
%  region is successfully accessed, a pointer to a Quantum array
%  representing the region is returned, otherwise NULL is returned.
%
%  The returned pointer may point to a temporary working copy of the pixels
%  or it may point to the original pixels in memory. Performance is maximized
%  if the selected region is part of one row, or one or more full rows, since
%  then there is opportunity to access the pixels in-place (without a copy)
%  if the image is in memory, or in a memory-mapped file. The returned pointer
%  must *never* be deallocated by the user.
%
%  Pixels accessed via the returned pointer represent a simple array of type
%  Quantum.  If the image has corresponding metacontent,call
%  GetAuthenticMetacontent() after invoking GetAuthenticPixels() to obtain the
%  meta-content corresponding to the region.  Once the Quantum array has
%  been updated, the changes must be saved back to the underlying image using
%  SyncAuthenticPixels() or they may be lost.
%
%  The format of the GetAuthenticPixels() method is:
%
%      Quantum *GetAuthenticPixels(Image *image,const ssize_t x,
%        const ssize_t y,const size_t columns,const size_t rows,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Quantum *GetAuthenticPixels(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  Quantum
    *pixels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->methods.get_authentic_pixels_handler !=
      (GetAuthenticPixelsHandler) NULL)
    {
      pixels=cache_info->methods.get_authentic_pixels_handler(image,x,y,columns,
        rows,exception);
      return(pixels);
    }
  assert(id < (int) cache_info->number_threads);
  pixels=GetAuthenticPixelCacheNexus(image,x,y,columns,rows,
    cache_info->nexus_info[id],exception);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t A u t h e n t i c P i x e l s C a c h e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticPixelsCache() gets pixels from the in-memory or disk pixel cache
%  as defined by the geometry parameters.   A pointer to the pixels is returned
%  if the pixels are transferred, otherwise a NULL is returned.
%
%  The format of the GetAuthenticPixelsCache() method is:
%
%      Quantum *GetAuthenticPixelsCache(Image *image,const ssize_t x,
%        const ssize_t y,const size_t columns,const size_t rows,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Quantum *GetAuthenticPixelsCache(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  Quantum
    *magick_restrict pixels;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  if (cache_info == (Cache) NULL)
    return((Quantum *) NULL);
  assert(cache_info->signature == MagickCoreSignature);
  assert(id < (int) cache_info->number_threads);
  pixels=GetAuthenticPixelCacheNexus(image,x,y,columns,rows,
    cache_info->nexus_info[id],exception);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t I m a g e E x t e n t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageExtent() returns the extent of the pixels associated corresponding
%  with the last call to QueueAuthenticPixels() or GetAuthenticPixels().
%
%  The format of the GetImageExtent() method is:
%
%      MagickSizeType GetImageExtent(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickSizeType GetImageExtent(const Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  assert(id < (int) cache_info->number_threads);
  return(GetPixelCacheNexusExtent(cache_info,cache_info->nexus_info[id]));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t I m a g e P i x e l C a c h e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImagePixelCache() ensures that there is only a single reference to the
%  pixel cache to be modified, updating the provided cache pointer to point to
%  a clone of the original pixel cache if necessary.
%
%  The format of the GetImagePixelCache method is:
%
%      Cache GetImagePixelCache(Image *image,const MagickBooleanType clone,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o clone: any value other than MagickFalse clones the cache pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline MagickBooleanType ValidatePixelCacheMorphology(
  const Image *magick_restrict image)
{
  const CacheInfo
    *magick_restrict cache_info;

  const PixelChannelMap
    *magick_restrict p,
    *magick_restrict q;

  /*
    Does the image match the pixel cache morphology?
  */
  cache_info=(CacheInfo *) image->cache;
  p=image->channel_map;
  q=cache_info->channel_map;
  if ((image->storage_class != cache_info->storage_class) ||
      (image->colorspace != cache_info->colorspace) ||
      (image->alpha_trait != cache_info->alpha_trait) ||
      (image->read_mask != cache_info->read_mask) ||
      (image->write_mask != cache_info->write_mask) ||
      (image->columns != cache_info->columns) ||
      (image->rows != cache_info->rows) ||
      (image->number_channels != cache_info->number_channels) ||
      (memcmp(p,q,image->number_channels*sizeof(*p)) != 0) ||
      (image->metacontent_extent != cache_info->metacontent_extent) ||
      (cache_info->nexus_info == (NexusInfo **) NULL))
    return(MagickFalse);
  return(MagickTrue);
}

static Cache GetImagePixelCache(Image *image,const MagickBooleanType clone,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  MagickBooleanType
    destroy,
    status;

  static MagickSizeType
    cache_timelimit = MagickResourceInfinity,
    cpu_throttle = MagickResourceInfinity,
    cycles = 0;

  status=MagickTrue;
  if (cpu_throttle == MagickResourceInfinity)
    cpu_throttle=GetMagickResourceLimit(ThrottleResource);
  if ((cpu_throttle != 0) && ((cycles++ % 32) == 0))
    MagickDelay(cpu_throttle);
  if (cache_epoch == 0)
    {
      /*
        Set the expire time in seconds.
      */
      cache_timelimit=GetMagickResourceLimit(TimeResource);
      cache_epoch=time((time_t *) NULL);
    }
  if ((cache_timelimit != MagickResourceInfinity) &&
      ((MagickSizeType) (time((time_t *) NULL)-cache_epoch) >= cache_timelimit))
    {
#if defined(ECANCELED)
      errno=ECANCELED;
#endif
      ThrowFatalException(ResourceLimitFatalError,"TimeLimitExceeded");
    }
  LockSemaphoreInfo(image->semaphore);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  CopyOpenCLBuffer(cache_info);
#endif
  destroy=MagickFalse;
  if ((cache_info->reference_count > 1) || (cache_info->mode == ReadMode))
    {
      LockSemaphoreInfo(cache_info->semaphore);
      if ((cache_info->reference_count > 1) || (cache_info->mode == ReadMode))
        {
          CacheInfo
            *clone_info;

          Image
            clone_image;

          /*
            Clone pixel cache.
          */
          clone_image=(*image);
          clone_image.semaphore=AcquireSemaphoreInfo();
          clone_image.reference_count=1;
          clone_image.cache=ClonePixelCache(cache_info);
          clone_info=(CacheInfo *) clone_image.cache;
          status=OpenPixelCache(&clone_image,IOMode,exception);
          if (status != MagickFalse)
            {
              if (clone != MagickFalse)
                status=ClonePixelCacheRepository(clone_info,cache_info,
                  exception);
              if (status != MagickFalse)
                {
                  if (cache_info->reference_count == 1)
                    cache_info->nexus_info=(NexusInfo **) NULL;
                  destroy=MagickTrue;
                  image->cache=clone_image.cache;
                }
            }
          RelinquishSemaphoreInfo(&clone_image.semaphore);
        }
      UnlockSemaphoreInfo(cache_info->semaphore);
    }
  if (destroy != MagickFalse)
    cache_info=(CacheInfo *) DestroyPixelCache(cache_info);
  if (status != MagickFalse)
    {
      /*
        Ensure the image matches the pixel cache morphology.
      */
      image->type=UndefinedType;
      if (ValidatePixelCacheMorphology(image) == MagickFalse)
        {
          status=OpenPixelCache(image,IOMode,exception);
          cache_info=(CacheInfo *) image->cache;
          if (cache_info->type == DiskCache)
            (void) ClosePixelCacheOnDisk(cache_info);
        }
    }
  UnlockSemaphoreInfo(image->semaphore);
  if (status == MagickFalse)
    return((Cache) NULL);
  return(image->cache);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t I m a g e P i x e l C a c h e T y p e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImagePixelCacheType() returns the pixel cache type: UndefinedCache,
%  DiskCache, MemoryCache, MapCache, or PingCache.
%
%  The format of the GetImagePixelCacheType() method is:
%
%      CacheType GetImagePixelCacheType(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport CacheType GetImagePixelCacheType(const Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  return(cache_info->type);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O n e A u t h e n t i c P i x e l                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneAuthenticPixel() returns a single pixel at the specified (x,y)
%  location.  The image background color is returned if an error occurs.
%
%  The format of the GetOneAuthenticPixel() method is:
%
%      MagickBooleanType GetOneAuthenticPixel(const Image image,const ssize_t x,
%        const ssize_t y,Quantum *pixel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y:  These values define the location of the pixel to return.
%
%    o pixel: return a pixel at the specified (x,y) location.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline MagickBooleanType CopyPixel(const Image *image,
  const Quantum *source,Quantum *destination)
{
  register ssize_t
    i;

  if (source == (const Quantum *) NULL)
    {
      destination[RedPixelChannel]=ClampToQuantum(image->background_color.red);
      destination[GreenPixelChannel]=ClampToQuantum(
        image->background_color.green);
      destination[BluePixelChannel]=ClampToQuantum(
        image->background_color.blue);
      destination[BlackPixelChannel]=ClampToQuantum(
        image->background_color.black);
      destination[AlphaPixelChannel]=ClampToQuantum(
        image->background_color.alpha);
      return(MagickFalse);
    }
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel channel=GetPixelChannelChannel(image,i);
    destination[channel]=source[i];
  }
  return(MagickTrue);
}

MagickExport MagickBooleanType GetOneAuthenticPixel(Image *image,
  const ssize_t x,const ssize_t y,Quantum *pixel,ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  register Quantum
    *magick_restrict q;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  (void) memset(pixel,0,MaxPixelChannels*sizeof(*pixel));
  if (cache_info->methods.get_one_authentic_pixel_from_handler !=
       (GetOneAuthenticPixelFromHandler) NULL)
    return(cache_info->methods.get_one_authentic_pixel_from_handler(image,x,y,
      pixel,exception));
  q=GetAuthenticPixelsCache(image,x,y,1UL,1UL,exception);
  return(CopyPixel(image,q,pixel));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t O n e A u t h e n t i c P i x e l F r o m C a c h e                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneAuthenticPixelFromCache() returns a single pixel at the specified (x,y)
%  location.  The image background color is returned if an error occurs.
%
%  The format of the GetOneAuthenticPixelFromCache() method is:
%
%      MagickBooleanType GetOneAuthenticPixelFromCache(const Image image,
%        const ssize_t x,const ssize_t y,Quantum *pixel,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y:  These values define the location of the pixel to return.
%
%    o pixel: return a pixel at the specified (x,y) location.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType GetOneAuthenticPixelFromCache(Image *image,
  const ssize_t x,const ssize_t y,Quantum *pixel,ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  register Quantum
    *magick_restrict q;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  assert(id < (int) cache_info->number_threads);
  (void) memset(pixel,0,MaxPixelChannels*sizeof(*pixel));
  q=GetAuthenticPixelCacheNexus(image,x,y,1UL,1UL,cache_info->nexus_info[id],
    exception);
  return(CopyPixel(image,q,pixel));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O n e V i r t u a l P i x e l                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneVirtualPixel() returns a single virtual pixel at the specified
%  (x,y) location.  The image background color is returned if an error occurs.
%  If you plan to modify the pixel, use GetOneAuthenticPixel() instead.
%
%  The format of the GetOneVirtualPixel() method is:
%
%      MagickBooleanType GetOneVirtualPixel(const Image image,const ssize_t x,
%        const ssize_t y,Quantum *pixel,ExceptionInfo exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y:  These values define the location of the pixel to return.
%
%    o pixel: return a pixel at the specified (x,y) location.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetOneVirtualPixel(const Image *image,
  const ssize_t x,const ssize_t y,Quantum *pixel,ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  const Quantum
    *p;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  (void) memset(pixel,0,MaxPixelChannels*sizeof(*pixel));
  if (cache_info->methods.get_one_virtual_pixel_from_handler !=
       (GetOneVirtualPixelFromHandler) NULL)
    return(cache_info->methods.get_one_virtual_pixel_from_handler(image,
      GetPixelCacheVirtualMethod(image),x,y,pixel,exception));
  assert(id < (int) cache_info->number_threads);
  p=GetVirtualPixelsFromNexus(image,GetPixelCacheVirtualMethod(image),x,y,
    1UL,1UL,cache_info->nexus_info[id],exception);
  return(CopyPixel(image,p,pixel));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t O n e V i r t u a l P i x e l F r o m C a c h e                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneVirtualPixelFromCache() returns a single virtual pixel at the
%  specified (x,y) location.  The image background color is returned if an
%  error occurs.
%
%  The format of the GetOneVirtualPixelFromCache() method is:
%
%      MagickBooleanType GetOneVirtualPixelFromCache(const Image image,
%        const VirtualPixelMethod method,const ssize_t x,const ssize_t y,
%        Quantum *pixel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o virtual_pixel_method: the virtual pixel method.
%
%    o x,y:  These values define the location of the pixel to return.
%
%    o pixel: return a pixel at the specified (x,y) location.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType GetOneVirtualPixelFromCache(const Image *image,
  const VirtualPixelMethod virtual_pixel_method,const ssize_t x,const ssize_t y,
  Quantum *pixel,ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  const Quantum
    *p;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  assert(id < (int) cache_info->number_threads);
  (void) memset(pixel,0,MaxPixelChannels*sizeof(*pixel));
  p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,x,y,1UL,1UL,
    cache_info->nexus_info[id],exception);
  return(CopyPixel(image,p,pixel));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O n e V i r t u a l P i x e l I n f o                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneVirtualPixelInfo() returns a single pixel at the specified (x,y)
%  location.  The image background color is returned if an error occurs.  If
%  you plan to modify the pixel, use GetOneAuthenticPixel() instead.
%
%  The format of the GetOneVirtualPixelInfo() method is:
%
%      MagickBooleanType GetOneVirtualPixelInfo(const Image image,
%        const VirtualPixelMethod virtual_pixel_method,const ssize_t x,
%        const ssize_t y,PixelInfo *pixel,ExceptionInfo exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o virtual_pixel_method: the virtual pixel method.
%
%    o x,y:  these values define the location of the pixel to return.
%
%    o pixel: return a pixel at the specified (x,y) location.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetOneVirtualPixelInfo(const Image *image,
  const VirtualPixelMethod virtual_pixel_method,const ssize_t x,const ssize_t y,
  PixelInfo *pixel,ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  register const Quantum
    *magick_restrict p;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  assert(id < (int) cache_info->number_threads);
  GetPixelInfo(image,pixel);
  p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,x,y,1UL,1UL,
    cache_info->nexus_info[id],exception);
  if (p == (const Quantum *) NULL)
    return(MagickFalse);
  GetPixelInfoPixel(image,p,pixel);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t P i x e l C a c h e C o l o r s p a c e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelCacheColorspace() returns the class type of the pixel cache.
%
%  The format of the GetPixelCacheColorspace() method is:
%
%      Colorspace GetPixelCacheColorspace(Cache cache)
%
%  A description of each parameter follows:
%
%    o cache: the pixel cache.
%
*/
MagickPrivate ColorspaceType GetPixelCacheColorspace(const Cache cache)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_info->filename);
  return(cache_info->colorspace);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t P i x e l C a c h e M e t h o d s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelCacheMethods() initializes the CacheMethods structure.
%
%  The format of the GetPixelCacheMethods() method is:
%
%      void GetPixelCacheMethods(CacheMethods *cache_methods)
%
%  A description of each parameter follows:
%
%    o cache_methods: Specifies a pointer to a CacheMethods structure.
%
*/
MagickPrivate void GetPixelCacheMethods(CacheMethods *cache_methods)
{
  assert(cache_methods != (CacheMethods *) NULL);
  (void) ResetMagickMemory(cache_methods,0,sizeof(*cache_methods));
  cache_methods->get_virtual_pixel_handler=GetVirtualPixelCache;
  cache_methods->get_virtual_pixels_handler=GetVirtualPixelsCache;
  cache_methods->get_virtual_metacontent_from_handler=
    GetVirtualMetacontentFromCache;
  cache_methods->get_one_virtual_pixel_from_handler=GetOneVirtualPixelFromCache;
  cache_methods->get_authentic_pixels_handler=GetAuthenticPixelsCache;
  cache_methods->get_authentic_metacontent_from_handler=
    GetAuthenticMetacontentFromCache;
  cache_methods->get_authentic_pixels_from_handler=GetAuthenticPixelsFromCache;
  cache_methods->get_one_authentic_pixel_from_handler=
    GetOneAuthenticPixelFromCache;
  cache_methods->queue_authentic_pixels_handler=QueueAuthenticPixelsCache;
  cache_methods->sync_authentic_pixels_handler=SyncAuthenticPixelsCache;
  cache_methods->destroy_pixel_handler=DestroyImagePixelCache;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t P i x e l C a c h e N e x u s E x t e n t                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelCacheNexusExtent() returns the extent of the pixels associated
%  corresponding with the last call to SetPixelCacheNexusPixels() or
%  GetPixelCacheNexusPixels().
%
%  The format of the GetPixelCacheNexusExtent() method is:
%
%      MagickSizeType GetPixelCacheNexusExtent(const Cache cache,
%        NexusInfo *nexus_info)
%
%  A description of each parameter follows:
%
%    o nexus_info: the nexus info.
%
*/
MagickPrivate MagickSizeType GetPixelCacheNexusExtent(const Cache cache,
  NexusInfo *magick_restrict nexus_info)
{
  CacheInfo
    *magick_restrict cache_info;

  MagickSizeType
    extent;

  assert(cache != NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickCoreSignature);
  extent=(MagickSizeType) nexus_info->region.width*nexus_info->region.height;
  if (extent == 0)
    return((MagickSizeType) cache_info->columns*cache_info->rows);
  return(extent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t P i x e l C a c h e P i x e l s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelCachePixels() returns the pixels associated with the specified image.
%
%  The format of the GetPixelCachePixels() method is:
%
%      void *GetPixelCachePixels(Image *image,MagickSizeType *length,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o length: the pixel cache length.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate void *GetPixelCachePixels(Image *image,MagickSizeType *length,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  assert(length != (MagickSizeType *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  *length=0;
  if ((cache_info->type != MemoryCache) && (cache_info->type != MapCache))
    return((void *) NULL);
  *length=cache_info->length;
  return((void *) cache_info->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t P i x e l C a c h e S t o r a g e C l a s s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelCacheStorageClass() returns the class type of the pixel cache.
%
%  The format of the GetPixelCacheStorageClass() method is:
%
%      ClassType GetPixelCacheStorageClass(Cache cache)
%
%  A description of each parameter follows:
%
%    o type: GetPixelCacheStorageClass returns DirectClass or PseudoClass.
%
%    o cache: the pixel cache.
%
*/
MagickPrivate ClassType GetPixelCacheStorageClass(const Cache cache)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_info->filename);
  return(cache_info->storage_class);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t P i x e l C a c h e T i l e S i z e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelCacheTileSize() returns the pixel cache tile size.
%
%  The format of the GetPixelCacheTileSize() method is:
%
%      void GetPixelCacheTileSize(const Image *image,size_t *width,
%        size_t *height)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o width: the optimize cache tile width in pixels.
%
%    o height: the optimize cache tile height in pixels.
%
*/
MagickPrivate void GetPixelCacheTileSize(const Image *image,size_t *width,
  size_t *height)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  *width=2048UL/(cache_info->number_channels*sizeof(Quantum));
  if (GetImagePixelCacheType(image) == DiskCache)
    *width=8192UL/(cache_info->number_channels*sizeof(Quantum));
  *height=(*width);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t P i x e l C a c h e V i r t u a l M e t h o d                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelCacheVirtualMethod() gets the "virtual pixels" method for the
%  pixel cache.  A virtual pixel is any pixel access that is outside the
%  boundaries of the image cache.
%
%  The format of the GetPixelCacheVirtualMethod() method is:
%
%      VirtualPixelMethod GetPixelCacheVirtualMethod(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickPrivate VirtualPixelMethod GetPixelCacheVirtualMethod(const Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  return(cache_info->virtual_pixel_method);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t V i r t u a l M e t a c o n t e n t F r o m C a c h e               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualMetacontentFromCache() returns the meta-content corresponding with
%  the last call to QueueAuthenticPixelsCache() or GetVirtualPixelCache().
%
%  The format of the GetVirtualMetacontentFromCache() method is:
%
%      void *GetVirtualMetacontentFromCache(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
static const void *GetVirtualMetacontentFromCache(const Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  const void
    *magick_restrict metacontent;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  assert(id < (int) cache_info->number_threads);
  metacontent=GetVirtualMetacontentFromNexus(cache_info,
    cache_info->nexus_info[id]);
  return(metacontent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t V i r t u a l M e t a c o n t e n t F r o m N e x u s               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualMetacontentFromNexus() returns the meta-content for the specified
%  cache nexus.
%
%  The format of the GetVirtualMetacontentFromNexus() method is:
%
%      const void *GetVirtualMetacontentFromNexus(const Cache cache,
%        NexusInfo *nexus_info)
%
%  A description of each parameter follows:
%
%    o cache: the pixel cache.
%
%    o nexus_info: the cache nexus to return the meta-content.
%
*/
MagickPrivate const void *GetVirtualMetacontentFromNexus(const Cache cache,
  NexusInfo *magick_restrict nexus_info)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->storage_class == UndefinedClass)
    return((void *) NULL);
  return(nexus_info->metacontent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t V i r t u a l M e t a c o n t e n t                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualMetacontent() returns the virtual metacontent corresponding with
%  the last call to QueueAuthenticPixels() or GetVirtualPixels().  NULL is
%  returned if the meta-content are not available.
%
%  The format of the GetVirtualMetacontent() method is:
%
%      const void *GetVirtualMetacontent(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport const void *GetVirtualMetacontent(const Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  const void
    *magick_restrict metacontent;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  metacontent=cache_info->methods.get_virtual_metacontent_from_handler(image);
  if (metacontent != (void *) NULL)
    return(metacontent);
  assert(id < (int) cache_info->number_threads);
  metacontent=GetVirtualMetacontentFromNexus(cache_info,
    cache_info->nexus_info[id]);
  return(metacontent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t V i r t u a l P i x e l s F r o m N e x u s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualPixelsFromNexus() gets virtual pixels from the in-memory or disk
%  pixel cache as defined by the geometry parameters.   A pointer to the pixels
%  is returned if the pixels are transferred, otherwise a NULL is returned.
%
%  The format of the GetVirtualPixelsFromNexus() method is:
%
%      Quantum *GetVirtualPixelsFromNexus(const Image *image,
%        const VirtualPixelMethod method,const ssize_t x,const ssize_t y,
%        const size_t columns,const size_t rows,NexusInfo *nexus_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o virtual_pixel_method: the virtual pixel method.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o nexus_info: the cache nexus to acquire.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static ssize_t
  DitherMatrix[64] =
  {
     0,  48,  12,  60,   3,  51,  15,  63,
    32,  16,  44,  28,  35,  19,  47,  31,
     8,  56,   4,  52,  11,  59,   7,  55,
    40,  24,  36,  20,  43,  27,  39,  23,
     2,  50,  14,  62,   1,  49,  13,  61,
    34,  18,  46,  30,  33,  17,  45,  29,
    10,  58,   6,  54,   9,  57,   5,  53,
    42,  26,  38,  22,  41,  25,  37,  21
  };

static inline ssize_t DitherX(const ssize_t x,const size_t columns)
{
  ssize_t
    index;

  index=x+DitherMatrix[x & 0x07]-32L;
  if (index < 0L)
    return(0L);
  if (index >= (ssize_t) columns)
    return((ssize_t) columns-1L);
  return(index);
}

static inline ssize_t DitherY(const ssize_t y,const size_t rows)
{
  ssize_t
    index;

  index=y+DitherMatrix[y & 0x07]-32L;
  if (index < 0L)
    return(0L);
  if (index >= (ssize_t) rows)
    return((ssize_t) rows-1L);
  return(index);
}

static inline ssize_t EdgeX(const ssize_t x,const size_t columns)
{
  if (x < 0L)
    return(0L);
  if (x >= (ssize_t) columns)
    return((ssize_t) (columns-1));
  return(x);
}

static inline ssize_t EdgeY(const ssize_t y,const size_t rows)
{
  if (y < 0L)
    return(0L);
  if (y >= (ssize_t) rows)
    return((ssize_t) (rows-1));
  return(y);
}

static inline ssize_t RandomX(RandomInfo *random_info,const size_t columns)
{
  return((ssize_t) (columns*GetPseudoRandomValue(random_info)));
}

static inline ssize_t RandomY(RandomInfo *random_info,const size_t rows)
{
  return((ssize_t) (rows*GetPseudoRandomValue(random_info)));
}

static inline MagickModulo VirtualPixelModulo(const ssize_t offset,
  const size_t extent)
{
  MagickModulo
    modulo;

  /*
    Compute the remainder of dividing offset by extent.  It returns not only
    the quotient (tile the offset falls in) but also the positive remainer
    within that tile such that 0 <= remainder < extent.  This method is
    essentially a ldiv() using a floored modulo division rather than the
    normal default truncated modulo division.
  */
  modulo.quotient=offset/(ssize_t) extent;
  if (offset < 0L)
    modulo.quotient--;
  modulo.remainder=offset-modulo.quotient*(ssize_t) extent;
  return(modulo);
}

MagickPrivate const Quantum *GetVirtualPixelsFromNexus(const Image *image,
  const VirtualPixelMethod virtual_pixel_method,const ssize_t x,const ssize_t y,
  const size_t columns,const size_t rows,NexusInfo *nexus_info,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  MagickOffsetType
    offset;

  MagickSizeType
    length,
    number_pixels;

  NexusInfo
    **magick_restrict virtual_nexus;

  Quantum
    *magick_restrict pixels,
    virtual_pixel[MaxPixelChannels];

  RectangleInfo
    region;

  register const Quantum
    *magick_restrict p;

  register const void
    *magick_restrict r;

  register Quantum
    *magick_restrict q;

  register ssize_t
    i,
    u;

  register unsigned char
    *magick_restrict s;

  ssize_t
    v;

  void
    *magick_restrict virtual_metacontent;

  /*
    Acquire pixels.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->type == UndefinedCache)
    return((const Quantum *) NULL);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  CopyOpenCLBuffer(cache_info);
#endif
  region.x=x;
  region.y=y;
  region.width=columns;
  region.height=rows;
  pixels=SetPixelCacheNexusPixels(cache_info,ReadMode,&region,nexus_info,
    exception);
  if (pixels == (Quantum *) NULL)
    return((const Quantum *) NULL);
  q=pixels;
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) (nexus_info->region.height-1L)*cache_info->columns+
    nexus_info->region.width-1L;
  number_pixels=(MagickSizeType) cache_info->columns*cache_info->rows;
  if ((offset >= 0) && (((MagickSizeType) offset+length) < number_pixels))
    if ((x >= 0) && ((ssize_t) (x+columns) <= (ssize_t) cache_info->columns) &&
        (y >= 0) && ((ssize_t) (y+rows) <= (ssize_t) cache_info->rows))
      {
        MagickBooleanType
          status;

        /*
          Pixel request is inside cache extents.
        */
        if (nexus_info->authentic_pixel_cache != MagickFalse)
          return(q);
        status=ReadPixelCachePixels(cache_info,nexus_info,exception);
        if (status == MagickFalse)
          return((const Quantum *) NULL);
        if (cache_info->metacontent_extent != 0)
          {
            status=ReadPixelCacheMetacontent(cache_info,nexus_info,exception);
            if (status == MagickFalse)
              return((const Quantum *) NULL);
          }
        return(q);
      }
  /*
    Pixel request is outside cache extents.
  */
  s=(unsigned char *) nexus_info->metacontent;
  virtual_nexus=AcquirePixelCacheNexus(1);
  if (virtual_nexus == (NexusInfo **) NULL)
    {
      if (virtual_nexus != (NexusInfo **) NULL)
        virtual_nexus=DestroyPixelCacheNexus(virtual_nexus,1);
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "UnableToGetCacheNexus","`%s'",image->filename);
      return((const Quantum *) NULL);
    }
  (void) ResetMagickMemory(virtual_pixel,0,cache_info->number_channels*
    sizeof(*virtual_pixel));
  virtual_metacontent=(void *) NULL;
  switch (virtual_pixel_method)
  {
    case BackgroundVirtualPixelMethod:
    case BlackVirtualPixelMethod:
    case GrayVirtualPixelMethod:
    case TransparentVirtualPixelMethod:
    case MaskVirtualPixelMethod:
    case WhiteVirtualPixelMethod:
    case EdgeVirtualPixelMethod:
    case CheckerTileVirtualPixelMethod:
    case HorizontalTileVirtualPixelMethod:
    case VerticalTileVirtualPixelMethod:
    {
      if (cache_info->metacontent_extent != 0)
        {
          /*
            Acquire a metacontent buffer.
          */
          virtual_metacontent=(void *) AcquireQuantumMemory(1,
            cache_info->metacontent_extent);
          if (virtual_metacontent == (void *) NULL)
            {
              virtual_nexus=DestroyPixelCacheNexus(virtual_nexus,1);
              (void) ThrowMagickException(exception,GetMagickModule(),
                CacheError,"UnableToGetCacheNexus","`%s'",image->filename);
              return((const Quantum *) NULL);
            }
          (void) ResetMagickMemory(virtual_metacontent,0,
            cache_info->metacontent_extent);
        }
      switch (virtual_pixel_method)
      {
        case BlackVirtualPixelMethod:
        {
          for (i=0; i < (ssize_t) cache_info->number_channels; i++)
            SetPixelChannel(image,(PixelChannel) i,(Quantum) 0,virtual_pixel);
          SetPixelAlpha(image,OpaqueAlpha,virtual_pixel);
          break;
        }
        case GrayVirtualPixelMethod:
        {
          for (i=0; i < (ssize_t) cache_info->number_channels; i++)
            SetPixelChannel(image,(PixelChannel) i,QuantumRange/2,
              virtual_pixel);
          SetPixelAlpha(image,OpaqueAlpha,virtual_pixel);
          break;
        }
        case TransparentVirtualPixelMethod:
        {
          for (i=0; i < (ssize_t) cache_info->number_channels; i++)
            SetPixelChannel(image,(PixelChannel) i,(Quantum) 0,virtual_pixel);
          SetPixelAlpha(image,TransparentAlpha,virtual_pixel);
          break;
        }
        case MaskVirtualPixelMethod:
        case WhiteVirtualPixelMethod:
        {
          for (i=0; i < (ssize_t) cache_info->number_channels; i++)
            SetPixelChannel(image,(PixelChannel) i,QuantumRange,virtual_pixel);
          SetPixelAlpha(image,OpaqueAlpha,virtual_pixel);
          break;
        }
        default:
        {
          SetPixelRed(image,ClampToQuantum(image->background_color.red),
            virtual_pixel);
          SetPixelGreen(image,ClampToQuantum(image->background_color.green),
            virtual_pixel);
          SetPixelBlue(image,ClampToQuantum(image->background_color.blue),
            virtual_pixel);
          SetPixelBlack(image,ClampToQuantum(image->background_color.black),
            virtual_pixel);
          SetPixelAlpha(image,ClampToQuantum(image->background_color.alpha),
            virtual_pixel);
          break;
        }
      }
      break;
    }
    default:
      break;
  }
  for (v=0; v < (ssize_t) rows; v++)
  {
    ssize_t
      y_offset;

    y_offset=y+v;
    if ((virtual_pixel_method == EdgeVirtualPixelMethod) ||
        (virtual_pixel_method == UndefinedVirtualPixelMethod))
      y_offset=EdgeY(y_offset,cache_info->rows);
    for (u=0; u < (ssize_t) columns; u+=length)
    {
      ssize_t
        x_offset;

      x_offset=x+u;
      length=(MagickSizeType) MagickMin(cache_info->columns-x_offset,columns-u);
      if (((x_offset < 0) || (x_offset >= (ssize_t) cache_info->columns)) ||
          ((y_offset < 0) || (y_offset >= (ssize_t) cache_info->rows)) ||
          (length == 0))
        {
          MagickModulo
            x_modulo,
            y_modulo;

          /*
            Transfer a single pixel.
          */
          length=(MagickSizeType) 1;
          switch (virtual_pixel_method)
          {
            case EdgeVirtualPixelMethod:
            default:
            {
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                EdgeX(x_offset,cache_info->columns),
                EdgeY(y_offset,cache_info->rows),1UL,1UL,*virtual_nexus,
                exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
            case RandomVirtualPixelMethod:
            {
              if (cache_info->random_info == (RandomInfo *) NULL)
                cache_info->random_info=AcquireRandomInfo();
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                RandomX(cache_info->random_info,cache_info->columns),
                RandomY(cache_info->random_info,cache_info->rows),1UL,1UL,
                *virtual_nexus,exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
            case DitherVirtualPixelMethod:
            {
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                DitherX(x_offset,cache_info->columns),
                DitherY(y_offset,cache_info->rows),1UL,1UL,*virtual_nexus,
                exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
            case TileVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x_offset,cache_info->columns);
              y_modulo=VirtualPixelModulo(y_offset,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,*virtual_nexus,
                exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
            case MirrorVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x_offset,cache_info->columns);
              if ((x_modulo.quotient & 0x01) == 1L)
                x_modulo.remainder=(ssize_t) cache_info->columns-
                  x_modulo.remainder-1L;
              y_modulo=VirtualPixelModulo(y_offset,cache_info->rows);
              if ((y_modulo.quotient & 0x01) == 1L)
                y_modulo.remainder=(ssize_t) cache_info->rows-
                  y_modulo.remainder-1L;
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,*virtual_nexus,
                exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
            case HorizontalTileEdgeVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x_offset,cache_info->columns);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,EdgeY(y_offset,cache_info->rows),1UL,1UL,
                *virtual_nexus,exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
            case VerticalTileEdgeVirtualPixelMethod:
            {
              y_modulo=VirtualPixelModulo(y_offset,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                EdgeX(x_offset,cache_info->columns),y_modulo.remainder,1UL,1UL,
                *virtual_nexus,exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
            case BackgroundVirtualPixelMethod:
            case BlackVirtualPixelMethod:
            case GrayVirtualPixelMethod:
            case TransparentVirtualPixelMethod:
            case MaskVirtualPixelMethod:
            case WhiteVirtualPixelMethod:
            {
              p=virtual_pixel;
              r=virtual_metacontent;
              break;
            }
            case CheckerTileVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x_offset,cache_info->columns);
              y_modulo=VirtualPixelModulo(y_offset,cache_info->rows);
              if (((x_modulo.quotient ^ y_modulo.quotient) & 0x01) != 0L)
                {
                  p=virtual_pixel;
                  r=virtual_metacontent;
                  break;
                }
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,*virtual_nexus,
                exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
            case HorizontalTileVirtualPixelMethod:
            {
              if ((y_offset < 0) || (y_offset >= (ssize_t) cache_info->rows))
                {
                  p=virtual_pixel;
                  r=virtual_metacontent;
                  break;
                }
              x_modulo=VirtualPixelModulo(x_offset,cache_info->columns);
              y_modulo=VirtualPixelModulo(y_offset,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,*virtual_nexus,
                exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
            case VerticalTileVirtualPixelMethod:
            {
              if ((x_offset < 0) || (x_offset >= (ssize_t) cache_info->columns))
                {
                  p=virtual_pixel;
                  r=virtual_metacontent;
                  break;
                }
              x_modulo=VirtualPixelModulo(x_offset,cache_info->columns);
              y_modulo=VirtualPixelModulo(y_offset,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,*virtual_nexus,
                exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
          }
          if (p == (const Quantum *) NULL)
            break;
          (void) memcpy(q,p,(size_t) length*cache_info->number_channels*
            sizeof(*p));
          q+=cache_info->number_channels;
          if ((s != (void *) NULL) && (r != (const void *) NULL))
            {
              (void) memcpy(s,r,(size_t) cache_info->metacontent_extent);
              s+=cache_info->metacontent_extent;
            }
          continue;
        }
      /*
        Transfer a run of pixels.
      */
      p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,x_offset,y_offset,
        (size_t) length,1UL,*virtual_nexus,exception);
      if (p == (const Quantum *) NULL)
        break;
      r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
      (void) memcpy(q,p,(size_t) length*cache_info->number_channels*sizeof(*p));
      q+=length*cache_info->number_channels;
      if ((r != (void *) NULL) && (s != (const void *) NULL))
        {
          (void) memcpy(s,r,(size_t) length);
          s+=length*cache_info->metacontent_extent;
        }
    }
    if (u < (ssize_t) columns)
      break;
  }
  /*
    Free resources.
  */
  if (virtual_metacontent != (void *) NULL)
    virtual_metacontent=(void *) RelinquishMagickMemory(virtual_metacontent);
  virtual_nexus=DestroyPixelCacheNexus(virtual_nexus,1);
  if (v < (ssize_t) rows)
    return((const Quantum *) NULL);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t V i r t u a l P i x e l C a c h e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualPixelCache() get virtual pixels from the in-memory or disk pixel
%  cache as defined by the geometry parameters.   A pointer to the pixels
%  is returned if the pixels are transferred, otherwise a NULL is returned.
%
%  The format of the GetVirtualPixelCache() method is:
%
%      const Quantum *GetVirtualPixelCache(const Image *image,
%        const VirtualPixelMethod virtual_pixel_method,const ssize_t x,
%        const ssize_t y,const size_t columns,const size_t rows,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o virtual_pixel_method: the virtual pixel method.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static const Quantum *GetVirtualPixelCache(const Image *image,
  const VirtualPixelMethod virtual_pixel_method,const ssize_t x,const ssize_t y,
  const size_t columns,const size_t rows,ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  const Quantum
    *magick_restrict p;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  assert(id < (int) cache_info->number_threads);
  p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,x,y,columns,rows,
    cache_info->nexus_info[id],exception);
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t V i r t u a l P i x e l Q u e u e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualPixelQueue() returns the virtual pixels associated corresponding
%  with the last call to QueueAuthenticPixels() or GetVirtualPixels().
%
%  The format of the GetVirtualPixelQueue() method is:
%
%      const Quantum *GetVirtualPixelQueue(const Image image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport const Quantum *GetVirtualPixelQueue(const Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->methods.get_virtual_pixels_handler !=
       (GetVirtualPixelsHandler) NULL)
    return(cache_info->methods.get_virtual_pixels_handler(image));
  assert(id < (int) cache_info->number_threads);
  return(GetVirtualPixelsNexus(cache_info,cache_info->nexus_info[id]));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t V i r t u a l P i x e l s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualPixels() returns an immutable pixel region. If the
%  region is successfully accessed, a pointer to it is returned, otherwise
%  NULL is returned.  The returned pointer may point to a temporary working
%  copy of the pixels or it may point to the original pixels in memory.
%  Performance is maximized if the selected region is part of one row, or one
%  or more full rows, since there is opportunity to access the pixels in-place
%  (without a copy) if the image is in memory, or in a memory-mapped file.  The
%  returned pointer must *never* be deallocated by the user.
%
%  Pixels accessed via the returned pointer represent a simple array of type
%  Quantum.  If the image type is CMYK or the storage class is PseudoClass,
%  call GetAuthenticMetacontent() after invoking GetAuthenticPixels() to
%  access the meta-content (of type void) corresponding to the the
%  region.
%
%  If you plan to modify the pixels, use GetAuthenticPixels() instead.
%
%  Note, the GetVirtualPixels() and GetAuthenticPixels() methods are not thread-
%  safe.  In a threaded environment, use GetCacheViewVirtualPixels() or
%  GetCacheViewAuthenticPixels() instead.
%
%  The format of the GetVirtualPixels() method is:
%
%      const Quantum *GetVirtualPixels(const Image *image,const ssize_t x,
%        const ssize_t y,const size_t columns,const size_t rows,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const Quantum *GetVirtualPixels(const Image *image,
  const ssize_t x,const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  const Quantum
    *magick_restrict p;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->methods.get_virtual_pixel_handler !=
       (GetVirtualPixelHandler) NULL)
    return(cache_info->methods.get_virtual_pixel_handler(image,
      GetPixelCacheVirtualMethod(image),x,y,columns,rows,exception));
  assert(id < (int) cache_info->number_threads);
  p=GetVirtualPixelsFromNexus(image,GetPixelCacheVirtualMethod(image),x,y,
    columns,rows,cache_info->nexus_info[id],exception);
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t V i r t u a l P i x e l s F r o m C a c h e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualPixelsCache() returns the pixels associated corresponding with the
%  last call to QueueAuthenticPixelsCache() or GetVirtualPixelCache().
%
%  The format of the GetVirtualPixelsCache() method is:
%
%      Quantum *GetVirtualPixelsCache(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
static const Quantum *GetVirtualPixelsCache(const Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  assert(id < (int) cache_info->number_threads);
  return(GetVirtualPixelsNexus(image->cache,cache_info->nexus_info[id]));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t V i r t u a l P i x e l s N e x u s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualPixelsNexus() returns the pixels associated with the specified
%  cache nexus.
%
%  The format of the GetVirtualPixelsNexus() method is:
%
%      const Quantum *GetVirtualPixelsNexus(const Cache cache,
%        NexusInfo *nexus_info)
%
%  A description of each parameter follows:
%
%    o cache: the pixel cache.
%
%    o nexus_info: the cache nexus to return the colormap pixels.
%
*/
MagickPrivate const Quantum *GetVirtualPixelsNexus(const Cache cache,
  NexusInfo *magick_restrict nexus_info)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->storage_class == UndefinedClass)
    return((Quantum *) NULL);
  return((const Quantum *) nexus_info->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   O p e n P i x e l C a c h e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpenPixelCache() allocates the pixel cache.  This includes defining the cache
%  dimensions, allocating space for the image pixels and optionally the
%  metacontent, and memory mapping the cache if it is disk based.  The cache
%  nexus array is initialized as well.
%
%  The format of the OpenPixelCache() method is:
%
%      MagickBooleanType OpenPixelCache(Image *image,const MapMode mode,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o mode: ReadMode, WriteMode, or IOMode.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(SIGBUS)
static void CacheSignalHandler(int status)
{
  ThrowFatalException(CacheFatalError,"UnableToExtendPixelCache");
}
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static MagickBooleanType OpenPixelCacheOnDisk(CacheInfo *cache_info,
  const MapMode mode)
{
  int
    file;

  /*
    Open pixel cache on disk.
  */
  if ((cache_info->file != -1) && (cache_info->mode == mode))
    return(MagickTrue);  /* cache already open and in the proper mode */
  if (*cache_info->cache_filename == '\0')
    file=AcquireUniqueFileResource(cache_info->cache_filename);
  else
    switch (mode)
    {
      case ReadMode:
      {
        file=open_utf8(cache_info->cache_filename,O_RDONLY | O_BINARY,0);
        break;
      }
      case WriteMode:
      {
        file=open_utf8(cache_info->cache_filename,O_WRONLY | O_CREAT |
          O_BINARY | O_EXCL,S_MODE);
        if (file == -1)
          file=open_utf8(cache_info->cache_filename,O_WRONLY | O_BINARY,S_MODE);
        break;
      }
      case IOMode:
      default:
      {
        file=open_utf8(cache_info->cache_filename,O_RDWR | O_CREAT | O_BINARY |
          O_EXCL,S_MODE);
        if (file == -1)
          file=open_utf8(cache_info->cache_filename,O_RDWR | O_BINARY,S_MODE);
        break;
      }
    }
  if (file == -1)
    return(MagickFalse);
  (void) AcquireMagickResource(FileResource,1);
  if (cache_info->file != -1)
    (void) ClosePixelCacheOnDisk(cache_info);
  cache_info->file=file;
  return(MagickTrue);
}

static inline MagickOffsetType WritePixelCacheRegion(
  const CacheInfo *magick_restrict cache_info,const MagickOffsetType offset,
  const MagickSizeType length,const unsigned char *magick_restrict buffer)
{
  register MagickOffsetType
    i;

  ssize_t
    count;

#if !defined(MAGICKCORE_HAVE_PWRITE)
  if (lseek(cache_info->file,offset,SEEK_SET) < 0)
    return((MagickOffsetType) -1);
#endif
  count=0;
  for (i=0; i < (MagickOffsetType) length; i+=count)
  {
#if !defined(MAGICKCORE_HAVE_PWRITE)
    count=write(cache_info->file,buffer+i,(size_t) MagickMin(length-i,(size_t)
      SSIZE_MAX));
#else
    count=pwrite(cache_info->file,buffer+i,(size_t) MagickMin(length-i,(size_t)
      SSIZE_MAX),(off_t) (offset+i));
#endif
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
  }
  return(i);
}

static MagickBooleanType SetPixelCacheExtent(Image *image,MagickSizeType length)
{
  CacheInfo
    *magick_restrict cache_info;

  MagickOffsetType
    count,
    extent,
    offset;

  cache_info=(CacheInfo *) image->cache;
  if (image->debug != MagickFalse)
    {
      char
        format[MagickPathExtent],
        message[MagickPathExtent];

      (void) FormatMagickSize(length,MagickFalse,"B",MagickPathExtent,format);
      (void) FormatLocaleString(message,MagickPathExtent,
        "extend %s (%s[%d], disk, %s)",cache_info->filename,
        cache_info->cache_filename,cache_info->file,format);
      (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",message);
    }
  if (length != (MagickSizeType) ((MagickOffsetType) length))
    return(MagickFalse);
  offset=(MagickOffsetType) lseek(cache_info->file,0,SEEK_END);
  if (offset < 0)
    return(MagickFalse);
  if ((MagickSizeType) offset >= length)
    count=(MagickOffsetType) 1;
  else
    {
      extent=(MagickOffsetType) length-1;
      count=WritePixelCacheRegion(cache_info,extent,1,(const unsigned char *)
        "");
      if (count != 1)
        return(MagickFalse);
#if defined(MAGICKCORE_HAVE_POSIX_FALLOCATE)
      if (cache_info->synchronize != MagickFalse)
        (void) posix_fallocate(cache_info->file,offset+1,extent-offset);
#endif
#if defined(SIGBUS)
      (void) signal(SIGBUS,CacheSignalHandler);
#endif
    }
  offset=(MagickOffsetType) lseek(cache_info->file,0,SEEK_SET);
  if (offset < 0)
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType OpenPixelCache(Image *image,const MapMode mode,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info,
    source_info;

  char
    format[MagickPathExtent],
    message[MagickPathExtent];

  const char
    *type;

  MagickBooleanType
    status;

  MagickSizeType
    length,
    number_pixels;

  size_t
    columns,
    packet_size;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (cache_anonymous_memory < 0)
    {
      char
        *value;

      /*
        Does the security policy require anonymous mapping for pixel cache?
      */
      cache_anonymous_memory=0;
      value=GetPolicyValue("pixel-cache-memory");
      if (value == (char *) NULL)
        value=GetPolicyValue("cache:memory-map");
      if (LocaleCompare(value,"anonymous") == 0)
        {
#if defined(MAGICKCORE_HAVE_MMAP) && defined(MAP_ANONYMOUS)
          cache_anonymous_memory=1;
#else
          (void) ThrowMagickException(exception,GetMagickModule(),
            MissingDelegateError,"DelegateLibrarySupportNotBuiltIn",
            "'%s' (policy requires anonymous memory mapping)",image->filename);
#endif
        }
      value=DestroyString(value);
    }
  if ((image->columns == 0) || (image->rows == 0))
    ThrowBinaryException(CacheError,"NoPixelsDefinedInCache",image->filename);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  if ((AcquireMagickResource(WidthResource,image->columns) == MagickFalse) ||
      (AcquireMagickResource(HeightResource,image->rows) == MagickFalse))
    ThrowBinaryException(ImageError,"WidthOrHeightExceedsLimit",
      image->filename);
  source_info=(*cache_info);
  source_info.file=(-1);
  (void) FormatLocaleString(cache_info->filename,MagickPathExtent,"%s[%.20g]",
    image->filename,(double) GetImageIndexInList(image));
  cache_info->storage_class=image->storage_class;
  cache_info->colorspace=image->colorspace;
  cache_info->alpha_trait=image->alpha_trait;
  cache_info->read_mask=image->read_mask;
  cache_info->write_mask=image->write_mask;
  cache_info->rows=image->rows;
  cache_info->columns=image->columns;
  InitializePixelChannelMap(image);
  cache_info->number_channels=GetPixelChannels(image);
  (void) memcpy(cache_info->channel_map,image->channel_map,MaxPixelChannels*
    sizeof(*image->channel_map));
  cache_info->metacontent_extent=image->metacontent_extent;
  cache_info->mode=mode;
  number_pixels=(MagickSizeType) cache_info->columns*cache_info->rows;
  packet_size=cache_info->number_channels*sizeof(Quantum);
  if (image->metacontent_extent != 0)
    packet_size+=cache_info->metacontent_extent;
  length=number_pixels*packet_size;
  columns=(size_t) (length/cache_info->rows/packet_size);
  if ((cache_info->columns != columns) || ((ssize_t) cache_info->columns < 0) ||
      ((ssize_t) cache_info->rows < 0))
    ThrowBinaryException(ResourceLimitError,"PixelCacheAllocationFailed",
      image->filename);
  cache_info->length=length;
  if (image->ping != MagickFalse)
    {
      cache_info->storage_class=image->storage_class;
      cache_info->colorspace=image->colorspace;
      cache_info->type=PingCache;
      return(MagickTrue);
    }
  status=AcquireMagickResource(AreaResource,cache_info->length);
  length=number_pixels*(cache_info->number_channels*sizeof(Quantum)+
    cache_info->metacontent_extent);
  if ((status != MagickFalse) && (length == (MagickSizeType) ((size_t) length)))
    {
      status=AcquireMagickResource(MemoryResource,cache_info->length);
      if (((cache_info->type == UndefinedCache) && (status != MagickFalse)) ||
          (cache_info->type == MemoryCache))
        {
          status=MagickTrue;
          if (cache_anonymous_memory <= 0)
            {
              cache_info->mapped=MagickFalse;
              cache_info->pixels=(Quantum *) MagickAssumeAligned(
                AcquireAlignedMemory(1,(size_t) cache_info->length));
            }
          else
            {
              cache_info->mapped=MagickTrue;
              cache_info->pixels=(Quantum *) MapBlob(-1,IOMode,0,(size_t)
                cache_info->length);
            }
          if (cache_info->pixels == (Quantum *) NULL)
            cache_info->pixels=source_info.pixels;
          else
            {
              /*
                Create memory pixel cache.
              */
              cache_info->type=MemoryCache;
              cache_info->metacontent=(void *) NULL;
              if (cache_info->metacontent_extent != 0)
                cache_info->metacontent=(void *) (cache_info->pixels+
                  number_pixels*cache_info->number_channels);
              if ((source_info.storage_class != UndefinedClass) &&
                  (mode != ReadMode))
                {
                  status=ClonePixelCacheRepository(cache_info,&source_info,
                    exception);
                  RelinquishPixelCachePixels(&source_info);
                }
              if (image->debug != MagickFalse)
                {
                  (void) FormatMagickSize(cache_info->length,MagickTrue,"B",
                    MagickPathExtent,format);
                  type=CommandOptionToMnemonic(MagickCacheOptions,(ssize_t)
                    cache_info->type);
                  (void) FormatLocaleString(message,MagickPathExtent,
                    "open %s (%s %s, %.20gx%.20gx%.20g %s)",
                    cache_info->filename,cache_info->mapped != MagickFalse ?
                    "Anonymous" : "Heap",type,(double) cache_info->columns,
                    (double) cache_info->rows,(double)
                    cache_info->number_channels,format);
                  (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",
                    message);
                }
              return(status == 0 ? MagickFalse : MagickTrue);
            }
        }
      RelinquishMagickResource(MemoryResource,cache_info->length);
    }
  /*
    Create pixel cache on disk.
  */
  status=AcquireMagickResource(DiskResource,cache_info->length);
  if ((status == MagickFalse) || (cache_info->type == DistributedCache))
    {
      DistributeCacheInfo
        *server_info;

      if (cache_info->type == DistributedCache)
        RelinquishMagickResource(DiskResource,cache_info->length);
      server_info=AcquireDistributeCacheInfo(exception);
      if (server_info != (DistributeCacheInfo *) NULL)
        {
          status=OpenDistributePixelCache(server_info,image);
          if (status == MagickFalse)
            {
              ThrowFileException(exception,CacheError,"UnableToOpenPixelCache",
                GetDistributeCacheHostname(server_info));
              server_info=DestroyDistributeCacheInfo(server_info);
            }
          else
            {
              /*
                Create a distributed pixel cache.
              */
              status=MagickTrue;
              cache_info->type=DistributedCache;
              cache_info->server_info=server_info;
              (void) FormatLocaleString(cache_info->cache_filename,
                MagickPathExtent,"%s:%d",GetDistributeCacheHostname(
                (DistributeCacheInfo *) cache_info->server_info),
                GetDistributeCachePort((DistributeCacheInfo *)
                cache_info->server_info));
              if ((source_info.storage_class != UndefinedClass) &&
                  (mode != ReadMode))
                {
                  status=ClonePixelCacheRepository(cache_info,&source_info,
                    exception);
                  RelinquishPixelCachePixels(&source_info);
                }
              if (image->debug != MagickFalse)
                {
                  (void) FormatMagickSize(cache_info->length,MagickFalse,"B",
                    MagickPathExtent,format);
                  type=CommandOptionToMnemonic(MagickCacheOptions,(ssize_t)
                    cache_info->type);
                  (void) FormatLocaleString(message,MagickPathExtent,
                    "open %s (%s[%d], %s, %.20gx%.20gx%.20g %s)",
                    cache_info->filename,cache_info->cache_filename,
                    GetDistributeCacheFile((DistributeCacheInfo *)
                    cache_info->server_info),type,(double) cache_info->columns,
                    (double) cache_info->rows,(double)
                    cache_info->number_channels,format);
                  (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",
                    message);
                }
              return(status == 0 ? MagickFalse : MagickTrue);
            }
        }
      RelinquishMagickResource(DiskResource,cache_info->length);
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "CacheResourcesExhausted","`%s'",image->filename);
      return(MagickFalse);
    }
  if ((source_info.storage_class != UndefinedClass) && (mode != ReadMode))
    {
      (void) ClosePixelCacheOnDisk(cache_info);
      *cache_info->cache_filename='\0';
    }
  if (OpenPixelCacheOnDisk(cache_info,mode) == MagickFalse)
    {
      RelinquishMagickResource(DiskResource,cache_info->length);
      ThrowFileException(exception,CacheError,"UnableToOpenPixelCache",
        image->filename);
      return(MagickFalse);
    }
  status=SetPixelCacheExtent(image,(MagickSizeType) cache_info->offset+
    cache_info->length);
  if (status == MagickFalse)
    {
      ThrowFileException(exception,CacheError,"UnableToExtendCache",
        image->filename);
      return(MagickFalse);
    }
  length=number_pixels*(cache_info->number_channels*sizeof(Quantum)+
    cache_info->metacontent_extent);
  if (length != (MagickSizeType) ((size_t) length))
    cache_info->type=DiskCache;
  else
    {
      status=AcquireMagickResource(MapResource,cache_info->length);
      if ((status == MagickFalse) && (cache_info->type != MapCache) &&
          (cache_info->type != MemoryCache))
        {
          status=MagickTrue;
          cache_info->type=DiskCache;
        }
      else
        {
          status=MagickTrue;
          cache_info->pixels=(Quantum *) MapBlob(cache_info->file,mode,
            cache_info->offset,(size_t) cache_info->length);
          if (cache_info->pixels == (Quantum *) NULL)
            {
              cache_info->type=DiskCache;
              cache_info->pixels=source_info.pixels;
            }
          else
            {
              /*
                Create file-backed memory-mapped pixel cache.
              */
              (void) ClosePixelCacheOnDisk(cache_info);
              cache_info->type=MapCache;
              cache_info->mapped=MagickTrue;
              cache_info->metacontent=(void *) NULL;
              if (cache_info->metacontent_extent != 0)
                cache_info->metacontent=(void *) (cache_info->pixels+
                  number_pixels*cache_info->number_channels);
              if ((source_info.storage_class != UndefinedClass) &&
                  (mode != ReadMode))
                {
                  status=ClonePixelCacheRepository(cache_info,&source_info,
                    exception);
                  RelinquishPixelCachePixels(&source_info);
                }
              if (image->debug != MagickFalse)
                {
                  (void) FormatMagickSize(cache_info->length,MagickTrue,"B",
                    MagickPathExtent,format);
                  type=CommandOptionToMnemonic(MagickCacheOptions,(ssize_t)
                    cache_info->type);
                  (void) FormatLocaleString(message,MagickPathExtent,
                    "open %s (%s[%d], %s, %.20gx%.20gx%.20g %s)",
                    cache_info->filename,cache_info->cache_filename,
                    cache_info->file,type,(double) cache_info->columns,(double)
                    cache_info->rows,(double) cache_info->number_channels,
                    format);
                  (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",
                    message);
                }
              return(status == 0 ? MagickFalse : MagickTrue);
            }
        }
      RelinquishMagickResource(MapResource,cache_info->length);
    }
  status=MagickTrue;
  if ((source_info.storage_class != UndefinedClass) && (mode != ReadMode))
    {
      status=ClonePixelCacheRepository(cache_info,&source_info,exception);
      RelinquishPixelCachePixels(&source_info);
    }
  if (image->debug != MagickFalse)
    {
      (void) FormatMagickSize(cache_info->length,MagickFalse,"B",
        MagickPathExtent,format);
      type=CommandOptionToMnemonic(MagickCacheOptions,(ssize_t)
        cache_info->type);
      (void) FormatLocaleString(message,MagickPathExtent,
        "open %s (%s[%d], %s, %.20gx%.20gx%.20g %s)",cache_info->filename,
        cache_info->cache_filename,cache_info->file,type,(double)
        cache_info->columns,(double) cache_info->rows,(double)
        cache_info->number_channels,format);
      (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",message);
    }
  return(status == 0 ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   P e r s i s t P i x e l C a c h e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PersistPixelCache() attaches to or initializes a persistent pixel cache.  A
%  persistent pixel cache is one that resides on disk and is not destroyed
%  when the program exits.
%
%  The format of the PersistPixelCache() method is:
%
%      MagickBooleanType PersistPixelCache(Image *image,const char *filename,
%        const MagickBooleanType attach,MagickOffsetType *offset,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o filename: the persistent pixel cache filename.
%
%    o attach: A value other than zero initializes the persistent pixel cache.
%
%    o initialize: A value other than zero initializes the persistent pixel
%      cache.
%
%    o offset: the offset in the persistent cache to store pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType PersistPixelCache(Image *image,
  const char *filename,const MagickBooleanType attach,MagickOffsetType *offset,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info,
    *magick_restrict clone_info;

  Image
    clone_image;

  MagickBooleanType
    status;

  ssize_t
    page_size;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (void *) NULL);
  assert(filename != (const char *) NULL);
  assert(offset != (MagickOffsetType *) NULL);
  page_size=GetMagickPageSize();
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  CopyOpenCLBuffer(cache_info);
#endif
  if (attach != MagickFalse)
    {
      /*
        Attach existing persistent pixel cache.
      */
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CacheEvent,GetMagickModule(),
          "attach persistent cache");
      (void) CopyMagickString(cache_info->cache_filename,filename,
        MagickPathExtent);
      cache_info->type=DiskCache;
      cache_info->offset=(*offset);
      if (OpenPixelCache(image,ReadMode,exception) == MagickFalse)
        return(MagickFalse);
      *offset+=cache_info->length+page_size-(cache_info->length % page_size);
      return(MagickTrue);
    }
  if ((cache_info->mode != ReadMode) &&
      ((cache_info->type == DiskCache) || (cache_info->type == MapCache)) &&
      (cache_info->reference_count == 1))
    {
      LockSemaphoreInfo(cache_info->semaphore);
      if ((cache_info->mode != ReadMode) &&
          ((cache_info->type == DiskCache) || (cache_info->type == MapCache)) &&
          (cache_info->reference_count == 1))
        {
          /*
            Usurp existing persistent pixel cache.
          */
          if (rename_utf8(cache_info->cache_filename, filename) == 0)
            {
              (void) CopyMagickString(cache_info->cache_filename,filename,
                MagickPathExtent);
              *offset+=cache_info->length+page_size-(cache_info->length %
                page_size);
              UnlockSemaphoreInfo(cache_info->semaphore);
              cache_info=(CacheInfo *) ReferencePixelCache(cache_info);
              if (image->debug != MagickFalse)
                (void) LogMagickEvent(CacheEvent,GetMagickModule(),
                  "Usurp resident persistent cache");
              return(MagickTrue);
            }
        }
      UnlockSemaphoreInfo(cache_info->semaphore);
    }
  /*
    Clone persistent pixel cache.
  */
  clone_image=(*image);
  clone_info=(CacheInfo *) clone_image.cache;
  image->cache=ClonePixelCache(cache_info);
  cache_info=(CacheInfo *) ReferencePixelCache(image->cache);
  (void) CopyMagickString(cache_info->cache_filename,filename,MagickPathExtent);
  cache_info->type=DiskCache;
  cache_info->offset=(*offset);
  cache_info=(CacheInfo *) image->cache;
  status=OpenPixelCache(image,IOMode,exception);
  if (status != MagickFalse)
    status=ClonePixelCacheRepository(cache_info,clone_info,exception);
  *offset+=cache_info->length+page_size-(cache_info->length % page_size);
  clone_info=(CacheInfo *) DestroyPixelCache(clone_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   Q u e u e A u t h e n t i c P i x e l C a c h e N e x u s                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QueueAuthenticPixelCacheNexus() allocates an region to store image pixels as
%  defined by the region rectangle and returns a pointer to the region.  This
%  region is subsequently transferred from the pixel cache with
%  SyncAuthenticPixelsCache().  A pointer to the pixels is returned if the
%  pixels are transferred, otherwise a NULL is returned.
%
%  The format of the QueueAuthenticPixelCacheNexus() method is:
%
%      Quantum *QueueAuthenticPixelCacheNexus(Image *image,const ssize_t x,
%        const ssize_t y,const size_t columns,const size_t rows,
%        const MagickBooleanType clone,NexusInfo *nexus_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o nexus_info: the cache nexus to set.
%
%    o clone: clone the pixel cache.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate Quantum *QueueAuthenticPixelCacheNexus(Image *image,
  const ssize_t x,const ssize_t y,const size_t columns,const size_t rows,
  const MagickBooleanType clone,NexusInfo *nexus_info,ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  MagickOffsetType
    offset;

  MagickSizeType
    number_pixels;

  Quantum
    *magick_restrict pixels;

  RectangleInfo
    region;

  /*
    Validate pixel cache geometry.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) GetImagePixelCache(image,clone,exception);
  if (cache_info == (Cache) NULL)
    return((Quantum *) NULL);
  assert(cache_info->signature == MagickCoreSignature);
  if ((cache_info->columns == 0) || (cache_info->rows == 0) || (x < 0) ||
      (y < 0) || (x >= (ssize_t) cache_info->columns) ||
      (y >= (ssize_t) cache_info->rows))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "PixelsAreNotAuthentic","`%s'",image->filename);
      return((Quantum *) NULL);
    }
  offset=(MagickOffsetType) y*cache_info->columns+x;
  if (offset < 0)
    return((Quantum *) NULL);
  number_pixels=(MagickSizeType) cache_info->columns*cache_info->rows;
  offset+=(MagickOffsetType) (rows-1)*cache_info->columns+columns-1;
  if ((MagickSizeType) offset >= number_pixels)
    return((Quantum *) NULL);
  /*
    Return pixel cache.
  */
  region.x=x;
  region.y=y;
  region.width=columns;
  region.height=rows;
  pixels=SetPixelCacheNexusPixels(cache_info,WriteMode,&region,nexus_info,
    exception);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   Q u e u e A u t h e n t i c P i x e l s C a c h e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QueueAuthenticPixelsCache() allocates an region to store image pixels as
%  defined by the region rectangle and returns a pointer to the region.  This
%  region is subsequently transferred from the pixel cache with
%  SyncAuthenticPixelsCache().  A pointer to the pixels is returned if the
%  pixels are transferred, otherwise a NULL is returned.
%
%  The format of the QueueAuthenticPixelsCache() method is:
%
%      Quantum *QueueAuthenticPixelsCache(Image *image,const ssize_t x,
%        const ssize_t y,const size_t columns,const size_t rows,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Quantum *QueueAuthenticPixelsCache(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  Quantum
    *magick_restrict pixels;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  assert(id < (int) cache_info->number_threads);
  pixels=QueueAuthenticPixelCacheNexus(image,x,y,columns,rows,MagickFalse,
    cache_info->nexus_info[id],exception);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   Q u e u e A u t h e n t i c P i x e l s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QueueAuthenticPixels() queues a mutable pixel region.  If the region is
%  successfully initialized a pointer to a Quantum array representing the
%  region is returned, otherwise NULL is returned.  The returned pointer may
%  point to a temporary working buffer for the pixels or it may point to the
%  final location of the pixels in memory.
%
%  Write-only access means that any existing pixel values corresponding to
%  the region are ignored.  This is useful if the initial image is being
%  created from scratch, or if the existing pixel values are to be
%  completely replaced without need to refer to their pre-existing values.
%  The application is free to read and write the pixel buffer returned by
%  QueueAuthenticPixels() any way it pleases. QueueAuthenticPixels() does not
%  initialize the pixel array values. Initializing pixel array values is the
%  application's responsibility.
%
%  Performance is maximized if the selected region is part of one row, or
%  one or more full rows, since then there is opportunity to access the
%  pixels in-place (without a copy) if the image is in memory, or in a
%  memory-mapped file. The returned pointer must *never* be deallocated
%  by the user.
%
%  Pixels accessed via the returned pointer represent a simple array of type
%  Quantum. If the image type is CMYK or the storage class is PseudoClass,
%  call GetAuthenticMetacontent() after invoking GetAuthenticPixels() to
%  obtain the meta-content (of type void) corresponding to the region.
%  Once the Quantum (and/or Quantum) array has been updated, the
%  changes must be saved back to the underlying image using
%  SyncAuthenticPixels() or they may be lost.
%
%  The format of the QueueAuthenticPixels() method is:
%
%      Quantum *QueueAuthenticPixels(Image *image,const ssize_t x,
%        const ssize_t y,const size_t columns,const size_t rows,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Quantum *QueueAuthenticPixels(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  Quantum
    *magick_restrict pixels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->methods.queue_authentic_pixels_handler !=
      (QueueAuthenticPixelsHandler) NULL)
    {
      pixels=cache_info->methods.queue_authentic_pixels_handler(image,x,y,
        columns,rows,exception);
      return(pixels);
    }
  assert(id < (int) cache_info->number_threads);
  pixels=QueueAuthenticPixelCacheNexus(image,x,y,columns,rows,MagickFalse,
    cache_info->nexus_info[id],exception);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e a d P i x e l C a c h e M e t a c o n t e n t                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPixelCacheMetacontent() reads metacontent from the specified region of
%  the pixel cache.
%
%  The format of the ReadPixelCacheMetacontent() method is:
%
%      MagickBooleanType ReadPixelCacheMetacontent(CacheInfo *cache_info,
%        NexusInfo *nexus_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_info: the pixel cache.
%
%    o nexus_info: the cache nexus to read the metacontent.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline MagickOffsetType ReadPixelCacheRegion(
  const CacheInfo *magick_restrict cache_info,const MagickOffsetType offset,
  const MagickSizeType length,unsigned char *magick_restrict buffer)
{
  register MagickOffsetType
    i;

  ssize_t
    count;

#if !defined(MAGICKCORE_HAVE_PREAD)
  if (lseek(cache_info->file,offset,SEEK_SET) < 0)
    return((MagickOffsetType) -1);
#endif
  count=0;
  for (i=0; i < (MagickOffsetType) length; i+=count)
  {
#if !defined(MAGICKCORE_HAVE_PREAD)
    count=read(cache_info->file,buffer+i,(size_t) MagickMin(length-i,(size_t)
      SSIZE_MAX));
#else
    count=pread(cache_info->file,buffer+i,(size_t) MagickMin(length-i,(size_t)
      SSIZE_MAX),(off_t) (offset+i));
#endif
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
  }
  return(i);
}

static MagickBooleanType ReadPixelCacheMetacontent(
  CacheInfo *magick_restrict cache_info,NexusInfo *magick_restrict nexus_info,
  ExceptionInfo *exception)
{
  MagickOffsetType
    count,
    offset;

  MagickSizeType
    extent,
    length;

  register ssize_t
    y;

  register unsigned char
    *magick_restrict q;

  size_t
    rows;

  if (cache_info->metacontent_extent == 0)
    return(MagickFalse);
  if (nexus_info->authentic_pixel_cache != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*
    cache_info->metacontent_extent;
  extent=length*nexus_info->region.height;
  rows=nexus_info->region.height;
  y=0;
  q=(unsigned char *) nexus_info->metacontent;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register unsigned char
        *magick_restrict p;

      /*
        Read meta-content from memory.
      */
      if ((cache_info->columns == nexus_info->region.width) &&
          (extent == (MagickSizeType) ((size_t) extent)))
        {
          length=extent;
          rows=1UL;
        }
      p=(unsigned char *) cache_info->metacontent+offset*
        cache_info->metacontent_extent;
      for (y=0; y < (ssize_t) rows; y++)
      {
        (void) memcpy(q,p,(size_t) length);
        p+=cache_info->metacontent_extent*cache_info->columns;
        q+=cache_info->metacontent_extent*nexus_info->region.width;
      }
      break;
    }
    case DiskCache:
    {
      /*
        Read meta content from disk.
      */
      LockSemaphoreInfo(cache_info->file_semaphore);
      if (OpenPixelCacheOnDisk(cache_info,IOMode) == MagickFalse)
        {
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            cache_info->cache_filename);
          UnlockSemaphoreInfo(cache_info->file_semaphore);
          return(MagickFalse);
        }
      if ((cache_info->columns == nexus_info->region.width) &&
          (extent <= MagickMaxBufferExtent))
        {
          length=extent;
          rows=1UL;
        }
      extent=(MagickSizeType) cache_info->columns*cache_info->rows;
      for (y=0; y < (ssize_t) rows; y++)
      {
        count=ReadPixelCacheRegion(cache_info,cache_info->offset+extent*
          cache_info->number_channels*sizeof(Quantum)+offset*
          cache_info->metacontent_extent,length,(unsigned char *) q);
        if (count != (MagickOffsetType) length)
          break;
        offset+=cache_info->columns;
        q+=cache_info->metacontent_extent*nexus_info->region.width;
      }
      if (IsFileDescriptorLimitExceeded() != MagickFalse)
        (void) ClosePixelCacheOnDisk(cache_info);
      UnlockSemaphoreInfo(cache_info->file_semaphore);
      break;
    }
    case DistributedCache:
    {
      RectangleInfo
        region;

      /*
        Read metacontent from distributed cache.
      */
      LockSemaphoreInfo(cache_info->file_semaphore);
      region=nexus_info->region;
      if ((cache_info->columns != nexus_info->region.width) ||
          (extent > MagickMaxBufferExtent))
        region.height=1UL;
      else
        {
          length=extent;
          rows=1UL;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        count=ReadDistributePixelCacheMetacontent((DistributeCacheInfo *)
          cache_info->server_info,&region,length,(unsigned char *) q);
        if (count != (MagickOffsetType) length)
          break;
        q+=cache_info->metacontent_extent*nexus_info->region.width;
        region.y++;
      }
      UnlockSemaphoreInfo(cache_info->file_semaphore);
      break;
    }
    default:
      break;
  }
  if (y < (ssize_t) rows)
    {
      ThrowFileException(exception,CacheError,"UnableToReadPixelCache",
        cache_info->cache_filename);
      return(MagickFalse);
    }
  if ((cache_info->debug != MagickFalse) &&
      (CacheTick(nexus_info->region.y,cache_info->rows) != MagickFalse))
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),
      "%s[%.20gx%.20g%+.20g%+.20g]",cache_info->filename,(double)
      nexus_info->region.width,(double) nexus_info->region.height,(double)
      nexus_info->region.x,(double) nexus_info->region.y);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e a d P i x e l C a c h e P i x e l s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPixelCachePixels() reads pixels from the specified region of the pixel
%  cache.
%
%  The format of the ReadPixelCachePixels() method is:
%
%      MagickBooleanType ReadPixelCachePixels(CacheInfo *cache_info,
%        NexusInfo *nexus_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_info: the pixel cache.
%
%    o nexus_info: the cache nexus to read the pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType ReadPixelCachePixels(
  CacheInfo *magick_restrict cache_info,NexusInfo *magick_restrict nexus_info,
  ExceptionInfo *exception)
{
  MagickOffsetType
    count,
    offset;

  MagickSizeType
    extent,
    length;

  register Quantum
    *magick_restrict q;

  register ssize_t
    y;

  size_t
    number_channels,
    rows;

  if (nexus_info->authentic_pixel_cache != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns;
  if ((ssize_t) (offset/cache_info->columns) != nexus_info->region.y)
    return(MagickFalse);
  offset+=nexus_info->region.x;
  number_channels=cache_info->number_channels;
  length=(MagickSizeType) number_channels*nexus_info->region.width*
    sizeof(Quantum);
  if ((length/number_channels/sizeof(Quantum)) != nexus_info->region.width)
    return(MagickFalse);
  rows=nexus_info->region.height;
  extent=length*rows;
  if ((extent == 0) || ((extent/length) != rows))
    return(MagickFalse);
  y=0;
  q=nexus_info->pixels;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register Quantum
        *magick_restrict p;

      /*
        Read pixels from memory.
      */
      if ((cache_info->columns == nexus_info->region.width) &&
          (extent == (MagickSizeType) ((size_t) extent)))
        {
          length=extent;
          rows=1UL;
        }
      p=cache_info->pixels+offset*cache_info->number_channels;
      for (y=0; y < (ssize_t) rows; y++)
      {
        (void) memcpy(q,p,(size_t) length);
        p+=cache_info->number_channels*cache_info->columns;
        q+=cache_info->number_channels*nexus_info->region.width;
      }
      break;
    }
    case DiskCache:
    {
      /*
        Read pixels from disk.
      */
      LockSemaphoreInfo(cache_info->file_semaphore);
      if (OpenPixelCacheOnDisk(cache_info,IOMode) == MagickFalse)
        {
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            cache_info->cache_filename);
          UnlockSemaphoreInfo(cache_info->file_semaphore);
          return(MagickFalse);
        }
      if ((cache_info->columns == nexus_info->region.width) &&
          (extent <= MagickMaxBufferExtent))
        {
          length=extent;
          rows=1UL;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        count=ReadPixelCacheRegion(cache_info,cache_info->offset+offset*
          cache_info->number_channels*sizeof(*q),length,(unsigned char *) q);
        if (count != (MagickOffsetType) length)
          break;
        offset+=cache_info->columns;
        q+=cache_info->number_channels*nexus_info->region.width;
      }
      if (IsFileDescriptorLimitExceeded() != MagickFalse)
        (void) ClosePixelCacheOnDisk(cache_info);
      UnlockSemaphoreInfo(cache_info->file_semaphore);
      break;
    }
    case DistributedCache:
    {
      RectangleInfo
        region;

      /*
        Read pixels from distributed cache.
      */
      LockSemaphoreInfo(cache_info->file_semaphore);
      region=nexus_info->region;
      if ((cache_info->columns != nexus_info->region.width) ||
          (extent > MagickMaxBufferExtent))
        region.height=1UL;
      else
        {
          length=extent;
          rows=1UL;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        count=ReadDistributePixelCachePixels((DistributeCacheInfo *)
          cache_info->server_info,&region,length,(unsigned char *) q);
        if (count != (MagickOffsetType) length)
          break;
        q+=cache_info->number_channels*nexus_info->region.width;
        region.y++;
      }
      UnlockSemaphoreInfo(cache_info->file_semaphore);
      break;
    }
    default:
      break;
  }
  if (y < (ssize_t) rows)
    {
      ThrowFileException(exception,CacheError,"UnableToReadPixelCache",
        cache_info->cache_filename);
      return(MagickFalse);
    }
  if ((cache_info->debug != MagickFalse) &&
      (CacheTick(nexus_info->region.y,cache_info->rows) != MagickFalse))
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),
      "%s[%.20gx%.20g%+.20g%+.20g]",cache_info->filename,(double)
      nexus_info->region.width,(double) nexus_info->region.height,(double)
      nexus_info->region.x,(double) nexus_info->region.y);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e f e r e n c e P i x e l C a c h e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReferencePixelCache() increments the reference count associated with the
%  pixel cache returning a pointer to the cache.
%
%  The format of the ReferencePixelCache method is:
%
%      Cache ReferencePixelCache(Cache cache_info)
%
%  A description of each parameter follows:
%
%    o cache_info: the pixel cache.
%
*/
MagickPrivate Cache ReferencePixelCache(Cache cache)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(cache != (Cache *) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickCoreSignature);
  LockSemaphoreInfo(cache_info->semaphore);
  cache_info->reference_count++;
  UnlockSemaphoreInfo(cache_info->semaphore);
  return(cache_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e s e t P i x e l C a c h e C h a n n e l s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetPixelCacheChannels() resets the pixel cache channels.
%
%  The format of the ResetPixelCacheChannels method is:
%
%      void ResetPixelCacheChannels(Image *)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickPrivate void ResetPixelCacheChannels(Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  cache_info->number_channels=GetPixelChannels(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e s e t P i x e l C a c h e E p o c h                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetPixelCacheEpoch() resets the pixel cache epoch.
%
%  The format of the ResetPixelCacheEpoch method is:
%
%      void ResetPixelCacheEpoch(void)
%
*/
MagickPrivate void ResetPixelCacheEpoch(void)
{
  cache_epoch=0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S e t P i x e l C a c h e M e t h o d s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetPixelCacheMethods() sets the image pixel methods to the specified ones.
%
%  The format of the SetPixelCacheMethods() method is:
%
%      SetPixelCacheMethods(Cache *,CacheMethods *cache_methods)
%
%  A description of each parameter follows:
%
%    o cache: the pixel cache.
%
%    o cache_methods: Specifies a pointer to a CacheMethods structure.
%
*/
MagickPrivate void SetPixelCacheMethods(Cache cache,CacheMethods *cache_methods)
{
  CacheInfo
    *magick_restrict cache_info;

  GetOneAuthenticPixelFromHandler
    get_one_authentic_pixel_from_handler;

  GetOneVirtualPixelFromHandler
    get_one_virtual_pixel_from_handler;

  /*
    Set cache pixel methods.
  */
  assert(cache != (Cache) NULL);
  assert(cache_methods != (CacheMethods *) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_info->filename);
  if (cache_methods->get_virtual_pixel_handler != (GetVirtualPixelHandler) NULL)
    cache_info->methods.get_virtual_pixel_handler=
      cache_methods->get_virtual_pixel_handler;
  if (cache_methods->destroy_pixel_handler != (DestroyPixelHandler) NULL)
    cache_info->methods.destroy_pixel_handler=
      cache_methods->destroy_pixel_handler;
  if (cache_methods->get_virtual_metacontent_from_handler !=
      (GetVirtualMetacontentFromHandler) NULL)
    cache_info->methods.get_virtual_metacontent_from_handler=
      cache_methods->get_virtual_metacontent_from_handler;
  if (cache_methods->get_authentic_pixels_handler !=
      (GetAuthenticPixelsHandler) NULL)
    cache_info->methods.get_authentic_pixels_handler=
      cache_methods->get_authentic_pixels_handler;
  if (cache_methods->queue_authentic_pixels_handler !=
      (QueueAuthenticPixelsHandler) NULL)
    cache_info->methods.queue_authentic_pixels_handler=
      cache_methods->queue_authentic_pixels_handler;
  if (cache_methods->sync_authentic_pixels_handler !=
      (SyncAuthenticPixelsHandler) NULL)
    cache_info->methods.sync_authentic_pixels_handler=
      cache_methods->sync_authentic_pixels_handler;
  if (cache_methods->get_authentic_pixels_from_handler !=
      (GetAuthenticPixelsFromHandler) NULL)
    cache_info->methods.get_authentic_pixels_from_handler=
      cache_methods->get_authentic_pixels_from_handler;
  if (cache_methods->get_authentic_metacontent_from_handler !=
      (GetAuthenticMetacontentFromHandler) NULL)
    cache_info->methods.get_authentic_metacontent_from_handler=
      cache_methods->get_authentic_metacontent_from_handler;
  get_one_virtual_pixel_from_handler=
    cache_info->methods.get_one_virtual_pixel_from_handler;
  if (get_one_virtual_pixel_from_handler !=
      (GetOneVirtualPixelFromHandler) NULL)
    cache_info->methods.get_one_virtual_pixel_from_handler=
      cache_methods->get_one_virtual_pixel_from_handler;
  get_one_authentic_pixel_from_handler=
    cache_methods->get_one_authentic_pixel_from_handler;
  if (get_one_authentic_pixel_from_handler !=
      (GetOneAuthenticPixelFromHandler) NULL)
    cache_info->methods.get_one_authentic_pixel_from_handler=
      cache_methods->get_one_authentic_pixel_from_handler;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S e t P i x e l C a c h e N e x u s P i x e l s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetPixelCacheNexusPixels() defines the region of the cache for the
%  specified cache nexus.
%
%  The format of the SetPixelCacheNexusPixels() method is:
%
%      Quantum SetPixelCacheNexusPixels(const CacheInfo *cache_info,
%        const MapMode mode,const RectangleInfo *region,NexusInfo *nexus_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_info: the pixel cache.
%
%    o mode: ReadMode, WriteMode, or IOMode.
%
%    o region: A pointer to the RectangleInfo structure that defines the
%      region of this particular cache nexus.
%
%    o nexus_info: the cache nexus to set.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline MagickBooleanType AcquireCacheNexusPixels(
  const CacheInfo *magick_restrict cache_info,NexusInfo *nexus_info,
  ExceptionInfo *exception)
{
  if (nexus_info->length != (MagickSizeType) ((size_t) nexus_info->length))
    return(MagickFalse);
  if (cache_anonymous_memory <= 0)
    {
      nexus_info->mapped=MagickFalse;
      nexus_info->cache=(Quantum *) MagickAssumeAligned(AcquireAlignedMemory(1,
        (size_t) nexus_info->length));
      if (nexus_info->cache != (Quantum *) NULL)
        (void) ResetMagickMemory(nexus_info->cache,0,(size_t)
          nexus_info->length);
    }
  else
    {
      nexus_info->mapped=MagickTrue;
      nexus_info->cache=(Quantum *) MapBlob(-1,IOMode,0,(size_t)
        nexus_info->length);
    }
  if (nexus_info->cache == (Quantum *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        cache_info->filename);
      return(MagickFalse);
    }
  return(MagickTrue);
}

static inline MagickBooleanType IsPixelCacheAuthentic(
  const CacheInfo *magick_restrict cache_info,
  const NexusInfo *magick_restrict nexus_info)
{
  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  /*
    Does nexus pixels point directly to in-core cache pixels or is it buffered?
  */
  if (cache_info->type == PingCache)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  status=nexus_info->pixels == (cache_info->pixels+offset*
    cache_info->number_channels) ? MagickTrue : MagickFalse;
  return(status);
}

static inline void PrefetchPixelCacheNexusPixels(const NexusInfo *nexus_info,
  const MapMode mode)
{
  if (mode == ReadMode)
    {
      MagickCachePrefetch((unsigned char *) nexus_info->pixels,0,1);
      return;
    }
  MagickCachePrefetch((unsigned char *) nexus_info->pixels,1,1);
}

static Quantum *SetPixelCacheNexusPixels(const CacheInfo *cache_info,
  const MapMode mode,const RectangleInfo *region,NexusInfo *nexus_info,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickSizeType
    length,
    number_pixels;

  assert(cache_info != (const CacheInfo *) NULL);
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->type == UndefinedCache)
    return((Quantum *) NULL);
  nexus_info->region=(*region);
  if ((cache_info->type == MemoryCache) || (cache_info->type == MapCache))
    {
      ssize_t
        x,
        y;

      x=nexus_info->region.x+(ssize_t) nexus_info->region.width-1;
      y=nexus_info->region.y+(ssize_t) nexus_info->region.height-1;
      if (((nexus_info->region.x >= 0) && (x < (ssize_t) cache_info->columns) &&
           (nexus_info->region.y >= 0) && (y < (ssize_t) cache_info->rows)) &&
          ((nexus_info->region.height == 1UL) || ((nexus_info->region.x == 0) &&
           ((nexus_info->region.width == cache_info->columns) ||
            ((nexus_info->region.width % cache_info->columns) == 0)))))
        {
          MagickOffsetType
            offset;

          /*
            Pixels are accessed directly from memory.
          */
          offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
            nexus_info->region.x;
          nexus_info->pixels=cache_info->pixels+cache_info->number_channels*
            offset;
          nexus_info->metacontent=(void *) NULL;
          if (cache_info->metacontent_extent != 0)
            nexus_info->metacontent=(unsigned char *) cache_info->metacontent+
              offset*cache_info->metacontent_extent;
          PrefetchPixelCacheNexusPixels(nexus_info,mode);
          nexus_info->authentic_pixel_cache=IsPixelCacheAuthentic(cache_info,
            nexus_info);
          return(nexus_info->pixels);
        }
    }
  /*
    Pixels are stored in a staging region until they are synced to the cache.
  */
  number_pixels=(MagickSizeType) nexus_info->region.width*
    nexus_info->region.height;
  length=number_pixels*cache_info->number_channels*sizeof(Quantum);
  if (cache_info->metacontent_extent != 0)
    length+=number_pixels*cache_info->metacontent_extent;
  if (nexus_info->cache == (Quantum *) NULL)
    {
      nexus_info->length=length;
      status=AcquireCacheNexusPixels(cache_info,nexus_info,exception);
      if (status == MagickFalse)
        {
          nexus_info->length=0;
          return((Quantum *) NULL);
        }
    }
  else
    if (nexus_info->length < length)
      {
        RelinquishCacheNexusPixels(nexus_info);
        nexus_info->length=length;
        status=AcquireCacheNexusPixels(cache_info,nexus_info,exception);
        if (status == MagickFalse)
          {
            nexus_info->length=0;
            return((Quantum *) NULL);
          }
      }
  nexus_info->pixels=nexus_info->cache;
  nexus_info->metacontent=(void *) NULL;
  if (cache_info->metacontent_extent != 0)
    nexus_info->metacontent=(void *) (nexus_info->pixels+number_pixels*
      cache_info->number_channels);
  PrefetchPixelCacheNexusPixels(nexus_info,mode);
  nexus_info->authentic_pixel_cache=IsPixelCacheAuthentic(cache_info,
    nexus_info);
  return(nexus_info->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t P i x e l C a c h e V i r t u a l M e t h o d                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetPixelCacheVirtualMethod() sets the "virtual pixels" method for the
%  pixel cache and returns the previous setting.  A virtual pixel is any pixel
%  access that is outside the boundaries of the image cache.
%
%  The format of the SetPixelCacheVirtualMethod() method is:
%
%      VirtualPixelMethod SetPixelCacheVirtualMethod(Image *image,
%        const VirtualPixelMethod virtual_pixel_method,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o virtual_pixel_method: choose the type of virtual pixel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType SetCacheAlphaChannel(Image *image,const Quantum alpha,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  CacheView
    *magick_restrict image_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  image->alpha_trait=BlendPixelTrait;
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);  /* must be virtual */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,1,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelAlpha(image,alpha,q);
      q+=GetPixelChannels(image);
    }
    status=SyncCacheViewAuthenticPixels(image_view,exception);
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

MagickPrivate VirtualPixelMethod SetPixelCacheVirtualMethod(Image *image,
  const VirtualPixelMethod virtual_pixel_method,ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  VirtualPixelMethod
    method;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  method=cache_info->virtual_pixel_method;
  cache_info->virtual_pixel_method=virtual_pixel_method;
  if ((image->columns != 0) && (image->rows != 0))
    switch (virtual_pixel_method)
    {
      case BackgroundVirtualPixelMethod:
      {
        if ((image->background_color.alpha_trait != UndefinedPixelTrait) &&
            (image->alpha_trait == UndefinedPixelTrait))
          (void) SetCacheAlphaChannel(image,OpaqueAlpha,exception);
        if ((IsPixelInfoGray(&image->background_color) == MagickFalse) &&
            (IsGrayColorspace(image->colorspace) != MagickFalse))
          (void) SetImageColorspace(image,sRGBColorspace,exception);
        break;
      }
      case TransparentVirtualPixelMethod:
      {
        if (image->alpha_trait == UndefinedPixelTrait)
          (void) SetCacheAlphaChannel(image,OpaqueAlpha,exception);
        break;
      }
      default:
        break;
    }
  return(method);
}

#if defined(MAGICKCORE_OPENCL_SUPPORT)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S y n c A u t h e n t i c O p e n C L B u f f e r                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncAuthenticOpenCLBuffer() makes sure that all the OpenCL operations have
%  been completed and updates the host memory.
%
%  The format of the SyncAuthenticOpenCLBuffer() method is:
%
%      void SyncAuthenticOpenCLBuffer(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/

static void CopyOpenCLBuffer(CacheInfo *magick_restrict cache_info)
{
  assert(cache_info != (CacheInfo *) NULL);
  assert(cache_info->signature == MagickCoreSignature);
  if ((cache_info->type != MemoryCache) ||
      (cache_info->opencl == (MagickCLCacheInfo) NULL))
    return;
  /*
    Ensure single threaded access to OpenCL environment.
  */
  LockSemaphoreInfo(cache_info->semaphore);
  cache_info->opencl=(MagickCLCacheInfo) CopyMagickCLCacheInfo(
    cache_info->opencl);
  UnlockSemaphoreInfo(cache_info->semaphore);
}

MagickPrivate void SyncAuthenticOpenCLBuffer(const Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(image != (const Image *) NULL);
  cache_info=(CacheInfo *) image->cache;
  CopyOpenCLBuffer(cache_info);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S y n c A u t h e n t i c P i x e l C a c h e N e x u s                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncAuthenticPixelCacheNexus() saves the authentic image pixels to the
%  in-memory or disk cache.  The method returns MagickTrue if the pixel region
%  is synced, otherwise MagickFalse.
%
%  The format of the SyncAuthenticPixelCacheNexus() method is:
%
%      MagickBooleanType SyncAuthenticPixelCacheNexus(Image *image,
%        NexusInfo *nexus_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o nexus_info: the cache nexus to sync.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate MagickBooleanType SyncAuthenticPixelCacheNexus(Image *image,
  NexusInfo *magick_restrict nexus_info,ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  MagickBooleanType
    status;

  /*
    Transfer pixels to the cache.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->cache == (Cache) NULL)
    ThrowBinaryException(CacheError,"PixelCacheIsNotOpen",image->filename);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->type == UndefinedCache)
    return(MagickFalse);
  if (nexus_info->authentic_pixel_cache != MagickFalse)
    {
      image->taint=MagickTrue;
      return(MagickTrue);
    }
  assert(cache_info->signature == MagickCoreSignature);
  status=WritePixelCachePixels(cache_info,nexus_info,exception);
  if ((cache_info->metacontent_extent != 0) &&
      (WritePixelCacheMetacontent(cache_info,nexus_info,exception) == MagickFalse))
    return(MagickFalse);
  if (status != MagickFalse)
    image->taint=MagickTrue;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S y n c A u t h e n t i c P i x e l C a c h e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncAuthenticPixelsCache() saves the authentic image pixels to the in-memory
%  or disk cache.  The method returns MagickTrue if the pixel region is synced,
%  otherwise MagickFalse.
%
%  The format of the SyncAuthenticPixelsCache() method is:
%
%      MagickBooleanType SyncAuthenticPixelsCache(Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType SyncAuthenticPixelsCache(Image *image,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  assert(id < (int) cache_info->number_threads);
  status=SyncAuthenticPixelCacheNexus(image,cache_info->nexus_info[id],
    exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S y n c A u t h e n t i c P i x e l s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncAuthenticPixels() saves the image pixels to the in-memory or disk cache.
%  The method returns MagickTrue if the pixel region is flushed, otherwise
%  MagickFalse.
%
%  The format of the SyncAuthenticPixels() method is:
%
%      MagickBooleanType SyncAuthenticPixels(Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SyncAuthenticPixels(Image *image,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickCoreSignature);
  if (cache_info->methods.sync_authentic_pixels_handler !=
       (SyncAuthenticPixelsHandler) NULL)
    {
      status=cache_info->methods.sync_authentic_pixels_handler(image,
        exception);
      return(status);
    }
  assert(id < (int) cache_info->number_threads);
  status=SyncAuthenticPixelCacheNexus(image,cache_info->nexus_info[id],
    exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S y n c I m a g e P i x e l C a c h e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncImagePixelCache() saves the image pixels to the in-memory or disk cache.
%  The method returns MagickTrue if the pixel region is flushed, otherwise
%  MagickFalse.
%
%  The format of the SyncImagePixelCache() method is:
%
%      MagickBooleanType SyncImagePixelCache(Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate MagickBooleanType SyncImagePixelCache(Image *image,
  ExceptionInfo *exception)
{
  CacheInfo
    *magick_restrict cache_info;

  assert(image != (Image *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  cache_info=(CacheInfo *) GetImagePixelCache(image,MagickTrue,exception);
  return(cache_info == (CacheInfo *) NULL ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   W r i t e P i x e l C a c h e M e t a c o n t e n t                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePixelCacheMetacontent() writes the meta-content to the specified region
%  of the pixel cache.
%
%  The format of the WritePixelCacheMetacontent() method is:
%
%      MagickBooleanType WritePixelCacheMetacontent(CacheInfo *cache_info,
%        NexusInfo *nexus_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_info: the pixel cache.
%
%    o nexus_info: the cache nexus to write the meta-content.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType WritePixelCacheMetacontent(CacheInfo *cache_info,
  NexusInfo *magick_restrict nexus_info,ExceptionInfo *exception)
{
  MagickOffsetType
    count,
    offset;

  MagickSizeType
    extent,
    length;

  register const unsigned char
    *magick_restrict p;

  register ssize_t
    y;

  size_t
    rows;

  if (cache_info->metacontent_extent == 0)
    return(MagickFalse);
  if (nexus_info->authentic_pixel_cache != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*
    cache_info->metacontent_extent;
  extent=(MagickSizeType) length*nexus_info->region.height;
  rows=nexus_info->region.height;
  y=0;
  p=(unsigned char *) nexus_info->metacontent;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register unsigned char
        *magick_restrict q;

      /*
        Write associated pixels to memory.
      */
      if ((cache_info->columns == nexus_info->region.width) &&
          (extent == (MagickSizeType) ((size_t) extent)))
        {
          length=extent;
          rows=1UL;
        }
      q=(unsigned char *) cache_info->metacontent+offset*
        cache_info->metacontent_extent;
      for (y=0; y < (ssize_t) rows; y++)
      {
        (void) memcpy(q,p,(size_t) length);
        p+=nexus_info->region.width*cache_info->metacontent_extent;
        q+=cache_info->columns*cache_info->metacontent_extent;
      }
      break;
    }
    case DiskCache:
    {
      /*
        Write associated pixels to disk.
      */
      LockSemaphoreInfo(cache_info->file_semaphore);
      if (OpenPixelCacheOnDisk(cache_info,IOMode) == MagickFalse)
        {
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            cache_info->cache_filename);
          UnlockSemaphoreInfo(cache_info->file_semaphore);
          return(MagickFalse);
        }
      if ((cache_info->columns == nexus_info->region.width) &&
          (extent <= MagickMaxBufferExtent))
        {
          length=extent;
          rows=1UL;
        }
      extent=(MagickSizeType) cache_info->columns*cache_info->rows;
      for (y=0; y < (ssize_t) rows; y++)
      {
        count=WritePixelCacheRegion(cache_info,cache_info->offset+extent*
          cache_info->number_channels*sizeof(Quantum)+offset*
          cache_info->metacontent_extent,length,(const unsigned char *) p);
        if (count != (MagickOffsetType) length)
          break;
        p+=cache_info->metacontent_extent*nexus_info->region.width;
        offset+=cache_info->columns;
      }
      if (IsFileDescriptorLimitExceeded() != MagickFalse)
        (void) ClosePixelCacheOnDisk(cache_info);
      UnlockSemaphoreInfo(cache_info->file_semaphore);
      break;
    }
    case DistributedCache:
    {
      RectangleInfo
        region;

      /*
        Write metacontent to distributed cache.
      */
      LockSemaphoreInfo(cache_info->file_semaphore);
      region=nexus_info->region;
      if ((cache_info->columns != nexus_info->region.width) ||
          (extent > MagickMaxBufferExtent))
        region.height=1UL;
      else
        {
          length=extent;
          rows=1UL;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        count=WriteDistributePixelCacheMetacontent((DistributeCacheInfo *)
          cache_info->server_info,&region,length,(const unsigned char *) p);
        if (count != (MagickOffsetType) length)
          break;
        p+=cache_info->metacontent_extent*nexus_info->region.width;
        region.y++;
      }
      UnlockSemaphoreInfo(cache_info->file_semaphore);
      break;
    }
    default:
      break;
  }
  if (y < (ssize_t) rows)
    {
      ThrowFileException(exception,CacheError,"UnableToWritePixelCache",
        cache_info->cache_filename);
      return(MagickFalse);
    }
  if ((cache_info->debug != MagickFalse) &&
      (CacheTick(nexus_info->region.y,cache_info->rows) != MagickFalse))
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),
      "%s[%.20gx%.20g%+.20g%+.20g]",cache_info->filename,(double)
      nexus_info->region.width,(double) nexus_info->region.height,(double)
      nexus_info->region.x,(double) nexus_info->region.y);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   W r i t e C a c h e P i x e l s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePixelCachePixels() writes image pixels to the specified region of the
%  pixel cache.
%
%  The format of the WritePixelCachePixels() method is:
%
%      MagickBooleanType WritePixelCachePixels(CacheInfo *cache_info,
%        NexusInfo *nexus_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_info: the pixel cache.
%
%    o nexus_info: the cache nexus to write the pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType WritePixelCachePixels(
  CacheInfo *magick_restrict cache_info,NexusInfo *magick_restrict nexus_info,
  ExceptionInfo *exception)
{
  MagickOffsetType
    count,
    offset;

  MagickSizeType
    extent,
    length;

  register const Quantum
    *magick_restrict p;

  register ssize_t
    y;

  size_t
    rows;

  if (nexus_info->authentic_pixel_cache != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) cache_info->number_channels*nexus_info->region.width*
    sizeof(Quantum);
  extent=length*nexus_info->region.height;
  rows=nexus_info->region.height;
  y=0;
  p=nexus_info->pixels;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register Quantum
        *magick_restrict q;

      /*
        Write pixels to memory.
      */
      if ((cache_info->columns == nexus_info->region.width) &&
          (extent == (MagickSizeType) ((size_t) extent)))
        {
          length=extent;
          rows=1UL;
        }
      q=cache_info->pixels+offset*cache_info->number_channels;
      for (y=0; y < (ssize_t) rows; y++)
      {
        (void) memcpy(q,p,(size_t) length);
        p+=cache_info->number_channels*nexus_info->region.width;
        q+=cache_info->columns*cache_info->number_channels;
      }
      break;
    }
    case DiskCache:
    {
      /*
        Write pixels to disk.
      */
      LockSemaphoreInfo(cache_info->file_semaphore);
      if (OpenPixelCacheOnDisk(cache_info,IOMode) == MagickFalse)
        {
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            cache_info->cache_filename);
          UnlockSemaphoreInfo(cache_info->file_semaphore);
          return(MagickFalse);
        }
      if ((cache_info->columns == nexus_info->region.width) &&
          (extent <= MagickMaxBufferExtent))
        {
          length=extent;
          rows=1UL;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        count=WritePixelCacheRegion(cache_info,cache_info->offset+offset*
          cache_info->number_channels*sizeof(*p),length,(const unsigned char *)
          p);
        if (count != (MagickOffsetType) length)
          break;
        p+=cache_info->number_channels*nexus_info->region.width;
        offset+=cache_info->columns;
      }
      if (IsFileDescriptorLimitExceeded() != MagickFalse)
        (void) ClosePixelCacheOnDisk(cache_info);
      UnlockSemaphoreInfo(cache_info->file_semaphore);
      break;
    }
    case DistributedCache:
    {
      RectangleInfo
        region;

      /*
        Write pixels to distributed cache.
      */
      LockSemaphoreInfo(cache_info->file_semaphore);
      region=nexus_info->region;
      if ((cache_info->columns != nexus_info->region.width) ||
          (extent > MagickMaxBufferExtent))
        region.height=1UL;
      else
        {
          length=extent;
          rows=1UL;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        count=WriteDistributePixelCachePixels((DistributeCacheInfo *)
          cache_info->server_info,&region,length,(const unsigned char *) p);
        if (count != (MagickOffsetType) length)
          break;
        p+=cache_info->number_channels*nexus_info->region.width;
        region.y++;
      }
      UnlockSemaphoreInfo(cache_info->file_semaphore);
      break;
    }
    default:
      break;
  }
  if (y < (ssize_t) rows)
    {
      ThrowFileException(exception,CacheError,"UnableToWritePixelCache",
        cache_info->cache_filename);
      return(MagickFalse);
    }
  if ((cache_info->debug != MagickFalse) &&
      (CacheTick(nexus_info->region.y,cache_info->rows) != MagickFalse))
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),
      "%s[%.20gx%.20g%+.20g%+.20g]",cache_info->filename,(double)
      nexus_info->region.width,(double) nexus_info->region.height,(double)
      nexus_info->region.x,(double) nexus_info->region.y);
  return(MagickTrue);
}

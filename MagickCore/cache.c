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
%                                John Cristy                                  %
%                                 July 1999                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
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
#include "MagickCore/composite-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/policy.h"
#include "MagickCore/quantum.h"
#include "MagickCore/random_.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/utility.h"
#if defined(MAGICKCORE_ZLIB_DELEGATE)
#include "zlib.h"
#endif

/*
  Define declarations.
*/
#define CacheTick(offset,extent)  QuantumTick((MagickOffsetType) offset,extent)

/*
  Typedef declarations.
*/
typedef struct _MagickModulo
{
  ssize_t
    quotient,
    remainder;
} MagickModulo;

struct _NexusInfo
{
  MagickBooleanType
    mapped;

  RectangleInfo
    region;

  MagickSizeType
    length;

  Quantum
    *cache,
    *pixels;

  void
    *metacontent;

  size_t
    signature;
};

/*
  Forward declarations.
*/
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static const Quantum
  *GetVirtualPixelCache(const Image *,const VirtualPixelMethod,const ssize_t,
    const ssize_t,const size_t,const size_t,ExceptionInfo *),
  *GetVirtualPixelsCache(const Image *);

static const void
  *GetVirtualMetacontentFromCache(const Image *);

static MagickBooleanType
  GetOneAuthenticPixelFromCache(Image *,const ssize_t,const ssize_t,
    PixelPacket *,ExceptionInfo *),
  GetOneVirtualPixelFromCache(const Image *,const VirtualPixelMethod,
    const ssize_t,const ssize_t,PixelPacket *,ExceptionInfo *),
  OpenPixelCache(Image *,const MapMode,ExceptionInfo *),
  ReadPixelCacheMetacontent(CacheInfo *,NexusInfo *,ExceptionInfo *),
  ReadPixelCachePixels(CacheInfo *,NexusInfo *,ExceptionInfo *),
  SyncAuthenticPixelsCache(Image *,ExceptionInfo *),
  WritePixelCacheMetacontent(CacheInfo *,NexusInfo *,ExceptionInfo *),
  WritePixelCachePixels(CacheInfo *,NexusInfo *,ExceptionInfo *);

static Quantum
  *GetAuthenticPixelsCache(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,ExceptionInfo *),
  *QueueAuthenticPixelsCache(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,ExceptionInfo *),
  *SetPixelCacheNexusPixels(const Image *,const RectangleInfo *,NexusInfo *,
    ExceptionInfo *);

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

static SplayTreeInfo
  *cache_resources = (SplayTreeInfo *) NULL;

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
MagickExport Cache AcquirePixelCache(const size_t number_threads)
{
  CacheInfo
    *cache_info;

  cache_info=(CacheInfo *) AcquireMagickMemory(sizeof(*cache_info));
  if (cache_info == (CacheInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(cache_info,0,sizeof(*cache_info));
  cache_info->type=UndefinedCache;
  cache_info->mode=IOMode;
  cache_info->colorspace=RGBColorspace;
  cache_info->file=(-1);
  cache_info->id=GetMagickThreadId();
  cache_info->number_threads=number_threads;
  if (number_threads == 0)
    cache_info->number_threads=GetOpenMPMaximumThreads();
  cache_info->nexus_info=AcquirePixelCacheNexus(cache_info->number_threads);
  if (cache_info->nexus_info == (NexusInfo **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  cache_info->semaphore=AllocateSemaphoreInfo();
  cache_info->reference_count=1;
  cache_info->disk_semaphore=AllocateSemaphoreInfo();
  cache_info->debug=IsEventLogging();
  cache_info->signature=MagickSignature;
  if ((cache_resources == (SplayTreeInfo *) NULL) &&
      (instantiate_cache == MagickFalse))
    {
      if (cache_semaphore == (SemaphoreInfo *) NULL)
        AcquireSemaphoreInfo(&cache_semaphore);
      LockSemaphoreInfo(cache_semaphore);
      if ((cache_resources == (SplayTreeInfo *) NULL) &&
          (instantiate_cache == MagickFalse))
        {
          cache_resources=NewSplayTree((int (*)(const void *,const void *))
            NULL,(void *(*)(void *)) NULL,(void *(*)(void *)) NULL);
          instantiate_cache=MagickTrue;
        }
      UnlockSemaphoreInfo(cache_semaphore);
    }
  (void) AddValueToSplayTree(cache_resources,cache_info,cache_info);
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
MagickExport NexusInfo **AcquirePixelCacheNexus(const size_t number_threads)
{
  NexusInfo
    **nexus_info;

  register ssize_t
    i;

  nexus_info=(NexusInfo **) AcquireQuantumMemory(number_threads,
    sizeof(*nexus_info));
  if (nexus_info == (NexusInfo **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    nexus_info[i]=(NexusInfo *) AcquireAlignedMemory(1,sizeof(**nexus_info));
    if (nexus_info[i] == (NexusInfo *) NULL)
      ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
    (void) ResetMagickMemory(nexus_info[i],0,sizeof(*nexus_info[i]));
    nexus_info[i]->signature=MagickSignature;
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
MagickExport const void *AcquirePixelCachePixels(const Image *image,
  MagickSizeType *length,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
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
MagickExport MagickBooleanType CacheComponentGenesis(void)
{
  AcquireSemaphoreInfo(&cache_semaphore);
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
MagickExport void CacheComponentTerminus(void)
{
  if (cache_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&cache_semaphore);
  LockSemaphoreInfo(cache_semaphore);
  if (cache_resources != (SplayTreeInfo *) NULL)
    cache_resources=DestroySplayTree(cache_resources);
  instantiate_cache=MagickFalse;
  UnlockSemaphoreInfo(cache_semaphore);
  DestroySemaphoreInfo(&cache_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l i p P i x e l C a c h e N e x u s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClipPixelCacheNexus() clips the cache nexus as defined by the image clip
%  mask.  It returns MagickTrue if the pixel region is clipped, otherwise
%  MagickFalse.
%
%  The format of the ClipPixelCacheNexus() method is:
%
%      MagickBooleanType ClipPixelCacheNexus(Image *image,NexusInfo *nexus_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o nexus_info: the cache nexus to clip.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType ClipPixelCacheNexus(Image *image,
  NexusInfo *nexus_info,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  MagickSizeType
    number_pixels;

  NexusInfo
    **clip_nexus,
    **image_nexus;

  register const Quantum
    *restrict p,
    *restrict r;

  register Quantum
    *restrict q;

  register ssize_t
    i;

  /*
    Apply clip mask.
  */
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->clip_mask == (Image *) NULL)
    return(MagickFalse);
  cache_info=(CacheInfo *) image->cache;
  if (cache_info == (Cache) NULL)
    return(MagickFalse);
  image_nexus=AcquirePixelCacheNexus(1);
  clip_nexus=AcquirePixelCacheNexus(1);
  if ((image_nexus == (NexusInfo **) NULL) ||
      (clip_nexus == (NexusInfo **) NULL))
    ThrowBinaryException(CacheError,"UnableToGetCacheNexus",image->filename);
  p=(const Quantum *) GetAuthenticPixelCacheNexus(image,
    nexus_info->region.x,nexus_info->region.y,nexus_info->region.width,
    nexus_info->region.height,image_nexus[0],exception);
  q=nexus_info->pixels;
  r=GetVirtualPixelsFromNexus(image->clip_mask,MaskVirtualPixelMethod,
    nexus_info->region.x,nexus_info->region.y,nexus_info->region.width,
    nexus_info->region.height,clip_nexus[0],exception);
  number_pixels=(MagickSizeType) nexus_info->region.width*
    nexus_info->region.height;
  for (i=0; i < (ssize_t) number_pixels; i++)
  {
    if ((p == (const Quantum *) NULL) || (r == (const Quantum *) NULL))
      break;
    if (GetPixelIntensity(image,r) > ((Quantum) QuantumRange/2))
      {
        SetPixelRed(image,GetPixelRed(image,p),q);
        SetPixelGreen(image,GetPixelGreen(image,p),q);
        SetPixelBlue(image,GetPixelBlue(image,p),q);
        if (cache_info->colorspace == CMYKColorspace)
          SetPixelBlack(image,GetPixelBlack(image,p),q);
        SetPixelAlpha(image,GetPixelAlpha(image,p),q);
      }
    p+=GetPixelComponents(image);
    q+=GetPixelComponents(image);
    r+=GetPixelComponents(image->clip_mask);
  }
  clip_nexus=DestroyPixelCacheNexus(clip_nexus,1);
  image_nexus=DestroyPixelCacheNexus(image_nexus,1);
  if (i < (ssize_t) number_pixels)
    return(MagickFalse);
  return(MagickTrue);
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
MagickExport Cache ClonePixelCache(const Cache cache)
{
  CacheInfo
    *clone_info;

  const CacheInfo
    *cache_info;

  assert(cache != (const Cache) NULL);
  cache_info=(const CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
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
+   C l o n e P i x e l C a c h e P i x e l s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %
%  ClonePixelCachePixels() clones the source pixel cache to the destination
%  cache.
%
%  The format of the ClonePixelCachePixels() method is:
%
%      MagickBooleanType ClonePixelCachePixels(CacheInfo *cache_info,
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

static MagickBooleanType ClosePixelCacheOnDisk(CacheInfo *cache_info)
{
  int
    status;

  status=(-1);
  LockSemaphoreInfo(cache_info->disk_semaphore);
  if (cache_info->file != -1)
    {
      status=close(cache_info->file);
      cache_info->file=(-1);
      RelinquishMagickResource(FileResource,1);
    }
  UnlockSemaphoreInfo(cache_info->disk_semaphore);
  return(status == -1 ? MagickFalse : MagickTrue);
}

static void LimitPixelCacheDescriptors(void)
{
  register CacheInfo
    *p,
    *q;

  /*
    Limit # of open file descriptors.
  */
  if (GetMagickResource(FileResource) < GetMagickResourceLimit(FileResource))
    return;
  LockSemaphoreInfo(cache_semaphore);
  if (cache_resources == (SplayTreeInfo *) NULL)
    {
      UnlockSemaphoreInfo(cache_semaphore);
      return;
    }
  ResetSplayTreeIterator(cache_resources);
  p=(CacheInfo *) GetNextKeyInSplayTree(cache_resources);
  while (p != (CacheInfo *) NULL)
  {
    if ((p->type == DiskCache) && (p->file != -1))
      break;
    p=(CacheInfo *) GetNextKeyInSplayTree(cache_resources);
  }
  for (q=p; p != (CacheInfo *) NULL; )
  {
    if ((p->type == DiskCache) && (p->file != -1) &&
        (p->timestamp < q->timestamp))
      q=p;
    p=(CacheInfo *) GetNextKeyInSplayTree(cache_resources);
  }
  if (q != (CacheInfo *) NULL)
    {
      /*
        Close least recently used cache.
      */
      (void) close(q->file);
      q->file=(-1);
    }
  UnlockSemaphoreInfo(cache_semaphore);
}

static inline MagickSizeType MagickMax(const MagickSizeType x,
  const MagickSizeType y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline MagickSizeType MagickMin(const MagickSizeType x,
  const MagickSizeType y)
{
  if (x < y)
    return(x);
  return(y);
}

static MagickBooleanType OpenPixelCacheOnDisk(CacheInfo *cache_info,
  const MapMode mode)
{
  int
    file;

  /*
    Open pixel cache on disk.
  */
  LockSemaphoreInfo(cache_info->disk_semaphore);
  if (cache_info->file != -1)
    {
      UnlockSemaphoreInfo(cache_info->disk_semaphore);
      return(MagickTrue);  /* cache already open */
    }
  LimitPixelCacheDescriptors();
  if (*cache_info->cache_filename == '\0')
    file=AcquireUniqueFileResource(cache_info->cache_filename);
  else
    switch (mode)
    {
      case ReadMode:
      {
        file=open(cache_info->cache_filename,O_RDONLY | O_BINARY);
        break;
      }
      case WriteMode:
      {
        file=open(cache_info->cache_filename,O_WRONLY | O_CREAT | O_BINARY |
          O_EXCL,S_MODE);
        if (file == -1)
          file=open(cache_info->cache_filename,O_WRONLY | O_BINARY,S_MODE);
        break;
      }
      case IOMode:
      default:
      {
        file=open(cache_info->cache_filename,O_RDWR | O_CREAT | O_BINARY |
          O_EXCL,S_MODE);
        if (file == -1)
          file=open(cache_info->cache_filename,O_RDWR | O_BINARY,S_MODE);
        break;
      }
    }
  if (file == -1)
    {
      UnlockSemaphoreInfo(cache_info->disk_semaphore);
      return(MagickFalse);
    }
  (void) AcquireMagickResource(FileResource,1);
  cache_info->file=file;
  cache_info->timestamp=time(0);
  UnlockSemaphoreInfo(cache_info->disk_semaphore);
  return(MagickTrue);
}

static inline MagickOffsetType ReadPixelCacheRegion(CacheInfo *cache_info,
  const MagickOffsetType offset,const MagickSizeType length,
  unsigned char *restrict buffer)
{
  register MagickOffsetType
    i;

  ssize_t
    count;

  cache_info->timestamp=time(0);
#if !defined(MAGICKCORE_HAVE_PREAD)
  LockSemaphoreInfo(cache_info->disk_semaphore);
  if (lseek(cache_info->file,offset,SEEK_SET) < 0)
    {
      UnlockSemaphoreInfo(cache_info->disk_semaphore);
      return((MagickOffsetType) -1);
    }
#endif
  count=0;
  for (i=0; i < (MagickOffsetType) length; i+=count)
  {
#if !defined(MAGICKCORE_HAVE_PREAD)
    count=read(cache_info->file,buffer+i,(size_t) MagickMin(length-i,
      (MagickSizeType) SSIZE_MAX));
#else
    count=pread(cache_info->file,buffer+i,(size_t) MagickMin(length-i,
      (MagickSizeType) SSIZE_MAX),(off_t) (offset+i));
#endif
    if (count > 0)
      continue;
    count=0;
    if (errno != EINTR)
      {
        i=(-1);
        break;
      }
  }
#if !defined(MAGICKCORE_HAVE_PREAD)
  UnlockSemaphoreInfo(cache_info->disk_semaphore);
#endif
  return(i);
}

static inline MagickOffsetType WritePixelCacheRegion(CacheInfo *cache_info,
  const MagickOffsetType offset,const MagickSizeType length,
  const unsigned char *restrict buffer)
{
  register MagickOffsetType
    i;

  ssize_t
    count;

  cache_info->timestamp=time(0);
#if !defined(MAGICKCORE_HAVE_PWRITE)
  LockSemaphoreInfo(cache_info->disk_semaphore);
  if (lseek(cache_info->file,offset,SEEK_SET) < 0)
    {
      UnlockSemaphoreInfo(cache_info->disk_semaphore);
      return((MagickOffsetType) -1);
    }
#endif
  count=0;
  for (i=0; i < (MagickOffsetType) length; i+=count)
  {
#if !defined(MAGICKCORE_HAVE_PWRITE)
    count=write(cache_info->file,buffer+i,(size_t) MagickMin(length-i,
      (MagickSizeType) SSIZE_MAX));
#else
    count=pwrite(cache_info->file,buffer+i,(size_t) MagickMin(length-i,
      (MagickSizeType) SSIZE_MAX),(off_t) (offset+i));
#endif
    if (count > 0)
      continue;
    count=0;
    if (errno != EINTR)
      {
        i=(-1);
        break;
      }
  }
#if !defined(MAGICKCORE_HAVE_PWRITE)
  UnlockSemaphoreInfo(cache_info->disk_semaphore);
#endif
  return(i);
}

static MagickBooleanType DiskToDiskPixelCacheClone(CacheInfo *clone_info,
  CacheInfo *cache_info,ExceptionInfo *exception)
{
  MagickOffsetType
    count;

  register MagickOffsetType
    i;

  size_t
    length;

  unsigned char
    *blob;

  /*
    Clone pixel cache (both caches on disk).
  */
  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),"disk => disk");
  blob=(unsigned char *) AcquireQuantumMemory(MagickMaxBufferExtent,
    sizeof(*blob));
  if (blob == (unsigned char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        cache_info->filename);
      return(MagickFalse);
    }
  if (OpenPixelCacheOnDisk(cache_info,ReadMode) == MagickFalse)
    {
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        cache_info->cache_filename);
      return(MagickFalse);
    }
  if (OpenPixelCacheOnDisk(clone_info,WriteMode) == MagickFalse)
    {
      (void) ClosePixelCacheOnDisk(cache_info);
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        clone_info->cache_filename);
      return(MagickFalse);
    }
  count=0;
  for (i=0; i < (MagickOffsetType) cache_info->length; i+=count)
  {
    count=ReadPixelCacheRegion(cache_info,cache_info->offset+i,
      MagickMin(cache_info->length-i,(MagickSizeType) MagickMaxBufferExtent),
      blob);
    if (count <= 0)
      {
        ThrowFileException(exception,CacheError,"UnableToReadPixelCache",
          cache_info->cache_filename);
        break;
      }
    length=(size_t) count;
    count=WritePixelCacheRegion(clone_info,clone_info->offset+i,length,blob);
    if ((MagickSizeType) count != length)
      {
        ThrowFileException(exception,CacheError,"UnableToWritePixelCache",
          clone_info->cache_filename);
        break;
      }
  }
  (void) ClosePixelCacheOnDisk(clone_info);
  (void) ClosePixelCacheOnDisk(cache_info);
  blob=(unsigned char *) RelinquishMagickMemory(blob);
  if (i < (MagickOffsetType) cache_info->length)
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType OptimizedPixelCacheClone(CacheInfo *clone_info,
  CacheInfo *cache_info,ExceptionInfo *exception)
{
  MagickOffsetType
    count;

  if ((cache_info->type != DiskCache) && (clone_info->type != DiskCache))
    {
      /*
        Clone pixel cache (both caches in memory).
      */
      if (cache_info->debug != MagickFalse)
        (void) LogMagickEvent(CacheEvent,GetMagickModule(),"memory => memory");
      (void) memcpy(clone_info->pixels,cache_info->pixels,(size_t)
        cache_info->length);
      return(MagickTrue);
    }
  if ((clone_info->type != DiskCache) && (cache_info->type == DiskCache))
    {
      /*
        Clone pixel cache (one cache on disk, one in memory).
      */
      if (cache_info->debug != MagickFalse)
        (void) LogMagickEvent(CacheEvent,GetMagickModule(),"disk => memory");
      if (OpenPixelCacheOnDisk(cache_info,ReadMode) == MagickFalse)
        {
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            cache_info->cache_filename);
          return(MagickFalse);
        }
      count=ReadPixelCacheRegion(cache_info,cache_info->offset,
        cache_info->length,(unsigned char *) clone_info->pixels);
      (void) ClosePixelCacheOnDisk(cache_info);
      if ((MagickSizeType) count != cache_info->length)
        {
          ThrowFileException(exception,CacheError,"UnableToReadPixelCache",
            cache_info->cache_filename);
          return(MagickFalse);
        }
      return(MagickTrue);
    }
  if ((clone_info->type == DiskCache) && (cache_info->type != DiskCache))
    {
      /*
        Clone pixel cache (one cache on disk, one in memory).
      */
      if (clone_info->debug != MagickFalse)
        (void) LogMagickEvent(CacheEvent,GetMagickModule(),"memory => disk");
      if (OpenPixelCacheOnDisk(clone_info,WriteMode) == MagickFalse)
        {
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            clone_info->cache_filename);
          return(MagickFalse);
        }
      count=WritePixelCacheRegion(clone_info,clone_info->offset,
        clone_info->length,(unsigned char *) cache_info->pixels);
      (void) ClosePixelCacheOnDisk(clone_info);
      if ((MagickSizeType) count != clone_info->length)
        {
          ThrowFileException(exception,CacheError,"UnableToWritePixelCache",
            clone_info->cache_filename);
          return(MagickFalse);
        }
      return(MagickTrue);
    }
  /*
    Clone pixel cache (both caches on disk).
  */
  return(DiskToDiskPixelCacheClone(clone_info,cache_info,exception));
}

static MagickBooleanType UnoptimizedPixelCacheClone(CacheInfo *clone_info,
  CacheInfo *cache_info,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickOffsetType
    cache_offset,
    clone_offset,
    count;

  register ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  unsigned char
    *blob;

  /*
    Clone pixel cache (unoptimized).
  */
  if (cache_info->debug != MagickFalse)
    {
      if ((cache_info->type != DiskCache) && (clone_info->type != DiskCache))
        (void) LogMagickEvent(CacheEvent,GetMagickModule(),"memory => memory");
      else
       if ((clone_info->type != DiskCache) && (cache_info->type == DiskCache))
         (void) LogMagickEvent(CacheEvent,GetMagickModule(),"disk => memory");
       else
         if ((clone_info->type == DiskCache) && (cache_info->type != DiskCache))
           (void) LogMagickEvent(CacheEvent,GetMagickModule(),"memory => disk");
         else
           (void) LogMagickEvent(CacheEvent,GetMagickModule(),"disk => disk");
    }
  length=(size_t) MagickMax(MagickMax(cache_info->pixel_components,
    clone_info->pixel_components)*sizeof(Quantum),MagickMax(
    cache_info->metacontent_extent,clone_info->metacontent_extent));
  blob=(unsigned char *) AcquireQuantumMemory(length,sizeof(*blob));
  if (blob == (unsigned char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        cache_info->filename);
      return(MagickFalse);
    }
  (void) ResetMagickMemory(blob,0,length*sizeof(*blob));
  cache_offset=0;
  clone_offset=0;
  if (cache_info->type == DiskCache)
    {
      if (OpenPixelCacheOnDisk(cache_info,ReadMode) == MagickFalse)
        {
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            cache_info->cache_filename);
          return(MagickFalse);
        }
      cache_offset=cache_info->offset;
    }
  if (clone_info->type == DiskCache)
    {
      if ((cache_info->type == DiskCache) &&
          (strcmp(cache_info->cache_filename,clone_info->cache_filename) == 0))
        {
          (void) ClosePixelCacheOnDisk(clone_info);
          *clone_info->cache_filename='\0';
        }
      if (OpenPixelCacheOnDisk(clone_info,WriteMode) == MagickFalse)
        {
          if (cache_info->type == DiskCache)
            (void) ClosePixelCacheOnDisk(cache_info);
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            clone_info->cache_filename);
          return(MagickFalse);
        }
      clone_offset=clone_info->offset;
    }
  /*
    Clone pixel channels.
  */
  status=MagickTrue;
  for (y=0; y < (ssize_t) cache_info->rows; y++)
  {
    for (x=0; x < (ssize_t) cache_info->columns; x++)
    {
      /*
        Read a set of pixel channels.
      */
      length=cache_info->pixel_components*sizeof(Quantum);
      if (cache_info->type != DiskCache)
        (void) memcpy(blob,(unsigned char *) cache_info->pixels+cache_offset,
          length);
      else
        {
          count=ReadPixelCacheRegion(cache_info,cache_offset,length,blob);
          if ((MagickSizeType) count != length)
            {
              status=MagickFalse;
              break;
            }
        }
      cache_offset+=length;
      if ((y < (ssize_t) clone_info->rows) &&
          (x < (ssize_t) clone_info->columns))
        {
          /*
            Write a set of pixel channels.
          */
          length=clone_info->pixel_components*sizeof(Quantum);
          if (clone_info->type != DiskCache)
            (void) memcpy((unsigned char *) clone_info->pixels+clone_offset,
              blob,length);
          else
            {
              count=WritePixelCacheRegion(clone_info,clone_offset,length,
                blob);
              if ((MagickSizeType) count != length)
                {
                  status=MagickFalse;
                  break;
                }
            }
          clone_offset+=length;
        }
    }
    length=clone_info->pixel_components*sizeof(Quantum);
    (void) ResetMagickMemory(blob,0,length*sizeof(*blob));
    for ( ; x < (ssize_t) clone_info->columns; x++)
    {
      /*
        Set remaining columns with transparent pixel channels.
      */
      if (clone_info->type != DiskCache)
        (void) memcpy((unsigned char *) clone_info->pixels+clone_offset,blob,
          length);
      else
        {
          count=WritePixelCacheRegion(clone_info,clone_offset,length,blob);
          if ((MagickSizeType) count != length)
            {
              status=MagickFalse;
              break;
            }
        }
      clone_offset+=length;
    }
  }
  length=clone_info->pixel_components*sizeof(Quantum);
  (void) ResetMagickMemory(blob,0,length*sizeof(*blob));
  for ( ; y < (ssize_t) clone_info->rows; y++)
  {
    /*
      Set remaining rows with transparent pixels.
    */
    for (x=0; x < (ssize_t) clone_info->columns; x++)
    {
      if (clone_info->type != DiskCache)
        (void) memcpy((unsigned char *) clone_info->pixels+clone_offset,blob,
          length);
      else
        {
          count=WritePixelCacheRegion(clone_info,clone_offset,length,blob);
          if ((MagickSizeType) count != length)
            {
              status=MagickFalse;
              break;
            }
        }
      clone_offset+=length;
    }
  }
  if ((cache_info->metacontent_extent != 0) &&
      (clone_info->metacontent_extent != 0))
    {
      /*
        Clone metacontent.
      */
      for (y=0; y < (ssize_t) cache_info->rows; y++)
      {
        for (x=0; x < (ssize_t) cache_info->columns; x++)
        {
          /*
            Read a set of metacontent.
          */
          length=cache_info->metacontent_extent;
          if (cache_info->type != DiskCache)
            (void) memcpy(blob,(unsigned char *) cache_info->pixels+
              cache_offset,length);
          else
            {
              count=ReadPixelCacheRegion(cache_info,cache_offset,length,blob);
              if ((MagickSizeType) count != length)
                {
                  status=MagickFalse;
                  break;
                }
            }
          cache_offset+=length;
          if ((y < (ssize_t) clone_info->rows) &&
              (x < (ssize_t) clone_info->columns))
            {
              /*
                Write a set of metacontent.
              */
              length=clone_info->metacontent_extent;
              if (clone_info->type != DiskCache)
                (void) memcpy((unsigned char *) clone_info->pixels+clone_offset,
                  blob,length);
              else
                {
                  count=WritePixelCacheRegion(clone_info,clone_offset,length,
                    blob);
                  if ((MagickSizeType) count != length)
                    {
                      status=MagickFalse;
                      break;
                    }
                }
              clone_offset+=length;
            }
        }
        length=clone_info->metacontent_extent;
        (void) ResetMagickMemory(blob,0,length*sizeof(*blob));
        for ( ; x < (ssize_t) clone_info->columns; x++)
        {
          /*
            Set remaining columns with metacontent.
          */
          if (clone_info->type != DiskCache)
            (void) memcpy((unsigned char *) clone_info->pixels+clone_offset,
              blob,length);
          else
            {
              count=WritePixelCacheRegion(clone_info,clone_offset,length,
                blob);
              if ((MagickSizeType) count != length)
                {
                  status=MagickFalse;
                  break;
                }
            }
          clone_offset+=length;
        }
      }
      length=clone_info->metacontent_extent;
      (void) ResetMagickMemory(blob,0,length*sizeof(*blob));
      for ( ; y < (ssize_t) clone_info->rows; y++)
      {
        /*
          Set remaining rows with metacontent.
        */
        for (x=0; x < (ssize_t) clone_info->columns; x++)
        {
          if (clone_info->type != DiskCache)
            (void) memcpy((unsigned char *) clone_info->pixels+clone_offset,
              blob,length);
          else
            {
              count=WritePixelCacheRegion(clone_info,clone_offset,length,blob);
              if ((MagickSizeType) count != length)
                {
                  status=MagickFalse;
                  break;
                }
            }
          clone_offset+=length;
        }
      }
    }
  if (clone_info->type == DiskCache)
    (void) ClosePixelCacheOnDisk(clone_info);
  if (cache_info->type == DiskCache)
    (void) ClosePixelCacheOnDisk(cache_info);
  blob=(unsigned char *) RelinquishMagickMemory(blob);
  return(status);
}

static MagickBooleanType ClonePixelCachePixels(CacheInfo *clone_info,
  CacheInfo *cache_info,ExceptionInfo *exception)
{
  if (cache_info->type == PingCache)
    return(MagickTrue);
  if ((cache_info->columns == clone_info->columns) &&
      (cache_info->rows == clone_info->rows) &&
      (cache_info->pixel_components == clone_info->pixel_components) &&
      (cache_info->metacontent_extent == clone_info->metacontent_extent))
    return(OptimizedPixelCacheClone(clone_info,cache_info,exception));
  return(UnoptimizedPixelCacheClone(clone_info,cache_info,exception));
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
MagickExport void ClonePixelCacheMethods(Cache clone,const Cache cache)
{
  CacheInfo
    *cache_info,
    *source_info;

  assert(clone != (Cache) NULL);
  source_info=(CacheInfo *) clone;
  assert(source_info->signature == MagickSignature);
  if (source_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      source_info->filename);
  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
  source_info->methods=cache_info->methods;
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
  assert(image->signature == MagickSignature);
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
    *cache_info;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
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

static inline void RelinquishPixelCachePixels(CacheInfo *cache_info)
{
  switch (cache_info->type)
  {
    case MemoryCache:
    {
      if (cache_info->mapped == MagickFalse)
        cache_info->pixels=(Quantum *) RelinquishMagickMemory(
          cache_info->pixels);
      else
        cache_info->pixels=(Quantum *) UnmapBlob(cache_info->pixels,
          (size_t) cache_info->length);
      RelinquishMagickResource(MemoryResource,cache_info->length);
      break;
    }
    case MapCache:
    {
      cache_info->pixels=(Quantum *) UnmapBlob(cache_info->pixels,(size_t)
        cache_info->length);
      RelinquishMagickResource(MapResource,cache_info->length);
    }
    case DiskCache:
    {
      if (cache_info->file != -1)
        (void) ClosePixelCacheOnDisk(cache_info);
      RelinquishMagickResource(DiskResource,cache_info->length);
      break;
    }
    default:
      break;
  }
  cache_info->type=UndefinedCache;
  cache_info->mapped=MagickFalse;
  cache_info->metacontent=(void *) NULL;
}

MagickExport Cache DestroyPixelCache(Cache cache)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
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
  if (cache_resources != (SplayTreeInfo *) NULL)
    (void) DeleteNodeByValueFromSplayTree(cache_resources,cache_info);
  if (cache_info->debug != MagickFalse)
    {
      char
        message[MaxTextExtent];

      (void) FormatLocaleString(message,MaxTextExtent,"destroy %s",
        cache_info->filename);
      (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",message);
    }
  if ((cache_info->mode == ReadMode) || ((cache_info->type != MapCache) &&
      (cache_info->type != DiskCache)))
    RelinquishPixelCachePixels(cache_info);
  else
    {
      RelinquishPixelCachePixels(cache_info);
      (void) RelinquishUniqueFileResource(cache_info->cache_filename);
    }
  *cache_info->cache_filename='\0';
  if (cache_info->nexus_info != (NexusInfo **) NULL)
    cache_info->nexus_info=DestroyPixelCacheNexus(cache_info->nexus_info,
      cache_info->number_threads);
  if (cache_info->random_info != (RandomInfo *) NULL)
    cache_info->random_info=DestroyRandomInfo(cache_info->random_info);
  if (cache_info->disk_semaphore != (SemaphoreInfo *) NULL)
    DestroySemaphoreInfo(&cache_info->disk_semaphore);
  if (cache_info->semaphore != (SemaphoreInfo *) NULL)
    DestroySemaphoreInfo(&cache_info->semaphore);
  cache_info->signature=(~MagickSignature);
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
    (void) RelinquishMagickMemory(nexus_info->cache);
  else
    (void) UnmapBlob(nexus_info->cache,(size_t) nexus_info->length);
  nexus_info->cache=(Quantum *) NULL;
  nexus_info->pixels=(Quantum *) NULL;
  nexus_info->metacontent=(void *) NULL;
  nexus_info->length=0;
  nexus_info->mapped=MagickFalse;
}

MagickExport NexusInfo **DestroyPixelCacheNexus(NexusInfo **nexus_info,
  const size_t number_threads)
{
  register ssize_t
    i;

  assert(nexus_info != (NexusInfo **) NULL);
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    if (nexus_info[i]->cache != (Quantum *) NULL)
      RelinquishCacheNexusPixels(nexus_info[i]);
    nexus_info[i]->signature=(~MagickSignature);
    nexus_info[i]=(NexusInfo *) RelinquishAlignedMemory(nexus_info[i]);
  }
  nexus_info=(NexusInfo **) RelinquishMagickMemory(nexus_info);
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  void
    *metacontent;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.get_authentic_metacontent_from_handler !=
      (GetAuthenticMetacontentFromHandler) NULL)
    {
      metacontent=cache_info->methods.
        get_authentic_metacontent_from_handler(image);
      return(metacontent);
    }
  assert(id < (int) cache_info->number_threads);
  metacontent=GetPixelCacheNexusMetacontent(cache_info,
    cache_info->nexus_info[id]);
  return(metacontent);
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  void
    *metacontent;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  assert(id < (int) cache_info->number_threads);
  metacontent=GetPixelCacheNexusMetacontent(image->cache,
    cache_info->nexus_info[id]);
  return(metacontent);
}

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

static inline MagickBooleanType IsPixelAuthentic(const CacheInfo *cache_info,
  NexusInfo *nexus_info)
{
  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  if (cache_info->type == PingCache)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  status=nexus_info->pixels == (cache_info->pixels+offset*
    cache_info->pixel_components) ? MagickTrue : MagickFalse;
  return(status);
}

MagickExport Quantum *GetAuthenticPixelCacheNexus(Image *image,
  const ssize_t x,const ssize_t y,const size_t columns,const size_t rows,
  NexusInfo *nexus_info,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  Quantum
    *pixels;

  /*
    Transfer pixels from the cache.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  pixels=QueueAuthenticNexus(image,x,y,columns,rows,nexus_info,exception);
  if (pixels == (Quantum *) NULL)
    return((Quantum *) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (IsPixelAuthentic(cache_info,nexus_info) != MagickFalse)
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  assert(id < (int) cache_info->number_threads);
  return(GetPixelCacheNexusPixels(image->cache,cache_info->nexus_info[id]));
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.get_authentic_pixels_from_handler !=
       (GetAuthenticPixelsFromHandler) NULL)
    return(cache_info->methods.get_authentic_pixels_from_handler(image));
  assert(id < (int) cache_info->number_threads);
  return(GetPixelCacheNexusPixels(cache_info,cache_info->nexus_info[id]));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t A u t h e n t i c P i x e l s                                       %
%                                                                             %
%                                                                             %
%                                                                             % %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  Quantum
    *pixels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  Quantum
    *pixels;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  if (cache_info == (Cache) NULL)
    return((Quantum *) NULL);
  assert(cache_info->signature == MagickSignature);
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
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
static inline MagickBooleanType ValidatePixelCacheMorphology(const Image *image)
{
  CacheInfo
    *cache_info;

  /*
    Does the image match the pixel cache morphology?
  */
  cache_info=(CacheInfo *) image->cache;
  if ((image->storage_class != cache_info->storage_class) ||
      (image->colorspace != cache_info->colorspace) ||
      (image->columns != cache_info->columns) ||
      (image->rows != cache_info->rows) ||
      (image->pixel_components != cache_info->pixel_components) ||
      (image->metacontent_extent != cache_info->metacontent_extent) ||
      (cache_info->nexus_info == (NexusInfo **) NULL) ||
      (cache_info->number_threads < GetOpenMPMaximumThreads()))
    return(MagickFalse);
  return(MagickTrue);
}

static Cache GetImagePixelCache(Image *image,const MagickBooleanType clone,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  MagickBooleanType
    destroy,
    status;

  static MagickSizeType
    cpu_throttle = 0,
    cycles = 0,
    time_limit = 0;

  static time_t
    cache_genesis = 0;

  status=MagickTrue;
  LockSemaphoreInfo(image->semaphore);
  if (cpu_throttle == 0)
    {
      char
        *limit;

      /*
        Set CPU throttle in milleseconds.
      */
      cpu_throttle=MagickResourceInfinity;
      limit=GetEnvironmentValue("MAGICK_THROTTLE");
      if (limit == (char *) NULL)
        limit=GetPolicyValue("throttle");
      if (limit != (char *) NULL)
        {
          cpu_throttle=(MagickSizeType) StringToInteger(limit);
          limit=DestroyString(limit);
        }
    }
  if ((cpu_throttle != MagickResourceInfinity) && ((cycles++ % 32) == 0))
    MagickDelay(cpu_throttle);
  if (time_limit == 0)
    {
      /*
        Set the exire time in seconds.
      */
      time_limit=GetMagickResourceLimit(TimeResource);
      cache_genesis=time((time_t *) NULL);
    }
  if ((time_limit != MagickResourceInfinity) &&
      ((MagickSizeType) (time((time_t *) NULL)-cache_genesis) >= time_limit))
    ThrowFatalException(ResourceLimitFatalError,"TimeLimitExceeded");
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  destroy=MagickFalse;
  if ((cache_info->reference_count > 1) || (cache_info->mode == ReadMode))
    {
      LockSemaphoreInfo(cache_info->semaphore);
      if ((cache_info->reference_count > 1) || (cache_info->mode == ReadMode))
        {
          Image
            clone_image;

          CacheInfo
            *clone_info;

          /*
            Clone pixel cache.
          */
          clone_image=(*image);
          clone_image.semaphore=AllocateSemaphoreInfo();
          clone_image.reference_count=1;
          clone_image.cache=ClonePixelCache(cache_info);
          clone_info=(CacheInfo *) clone_image.cache;
          status=OpenPixelCache(&clone_image,IOMode,exception);
          if (status != MagickFalse)
            {
              if (clone != MagickFalse)
                status=ClonePixelCachePixels(clone_info,cache_info,exception);
              if (status != MagickFalse)
                {
                  destroy=MagickTrue;
                  image->cache=clone_image.cache;
                }
            }
          DestroySemaphoreInfo(&clone_image.semaphore);
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
      image->taint=MagickTrue;
      image->type=UndefinedType;
      if (image->colorspace == GRAYColorspace)
        image->colorspace=RGBColorspace;
      if (ValidatePixelCacheMorphology(image) == MagickFalse)
        status=OpenPixelCache(image,IOMode,exception);
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
%        const ssize_t y,PixelPacket *pixel,ExceptionInfo *exception)
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
MagickExport MagickBooleanType GetOneAuthenticPixel(Image *image,
  const ssize_t x,const ssize_t y,PixelPacket *pixel,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  register Quantum
    *q;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  *pixel=image->background_color;
  if (cache_info->methods.get_one_authentic_pixel_from_handler !=
       (GetOneAuthenticPixelFromHandler) NULL)
    return(cache_info->methods.get_one_authentic_pixel_from_handler(image,x,y,
      pixel,exception));
  q=GetAuthenticPixelsCache(image,x,y,1UL,1UL,exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  GetPixelPacket(image,q,pixel);
  return(MagickTrue);
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
%        const ssize_t x,const ssize_t y,PixelPacket *pixel,
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
  const ssize_t x,const ssize_t y,PixelPacket *pixel,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  register Quantum
    *q;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  assert(id < (int) cache_info->number_threads);
  *pixel=image->background_color;
  q=GetAuthenticPixelCacheNexus(image,x,y,1UL,1UL,cache_info->nexus_info[id],
    exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  GetPixelPacket(image,q,pixel);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O n e V i r t u a l M a g i c k P i x e l                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneVirtualMagickPixel() returns a single pixel at the specified (x,y)
%  location.  The image background color is returned if an error occurs.  If
%  you plan to modify the pixel, use GetOneAuthenticPixel() instead.
%
%  The format of the GetOneVirtualMagickPixel() method is:
%
%      MagickBooleanType GetOneVirtualMagickPixel(const Image image,
%        const ssize_t x,const ssize_t y,PixelInfo *pixel,
%        ExceptionInfo exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y:  these values define the location of the pixel to return.
%
%    o pixel: return a pixel at the specified (x,y) location.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetOneVirtualMagickPixel(const Image *image,
  const ssize_t x,const ssize_t y,PixelInfo *pixel,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  register const Quantum
    *pixels;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  assert(id < (int) cache_info->number_threads);
  pixels=GetVirtualPixelsFromNexus(image,GetPixelCacheVirtualMethod(image),x,y,
    1UL,1UL,cache_info->nexus_info[id],exception);
  GetPixelInfo(image,pixel);
  if (pixels == (const Quantum *) NULL)
    return(MagickFalse);
  SetPixelInfo(image,pixels,pixel);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O n e V i r t u a l M e t h o d P i x e l                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneVirtualMethodPixel() returns a single pixel at the specified (x,y)
%  location as defined by specified pixel method.  The image background color
%  is returned if an error occurs.  If you plan to modify the pixel, use
%  GetOneAuthenticPixel() instead.
%
%  The format of the GetOneVirtualMethodPixel() method is:
%
%      MagickBooleanType GetOneVirtualMethodPixel(const Image image,
%        const VirtualPixelMethod virtual_pixel_method,const ssize_t x,
%        const ssize_t y,PixelPacket *pixel,ExceptionInfo exception)
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
MagickExport MagickBooleanType GetOneVirtualMethodPixel(const Image *image,
  const VirtualPixelMethod virtual_pixel_method,const ssize_t x,const ssize_t y,
  PixelPacket *pixel,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  const Quantum
    *p;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  *pixel=image->background_color;
  if (cache_info->methods.get_one_virtual_pixel_from_handler !=
       (GetOneVirtualPixelFromHandler) NULL)
    return(cache_info->methods.get_one_virtual_pixel_from_handler(image,
      virtual_pixel_method,x,y,pixel,exception));
  assert(id < (int) cache_info->number_threads);
  p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,x,y,1UL,1UL,
    cache_info->nexus_info[id],exception);
  if (p == (const Quantum *) NULL)
    return(MagickFalse);
  GetPixelPacket(image,p,pixel);
  if (image->colorspace == CMYKColorspace)
    pixel->black=GetPixelBlack(image,p);
  return(MagickTrue);
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
%        const ssize_t y,PixelPacket *pixel,ExceptionInfo exception)
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
  const ssize_t x,const ssize_t y,PixelPacket *pixel,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  const Quantum
    *p;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  *pixel=image->background_color;
  if (cache_info->methods.get_one_virtual_pixel_from_handler !=
       (GetOneVirtualPixelFromHandler) NULL)
    return(cache_info->methods.get_one_virtual_pixel_from_handler(image,
      GetPixelCacheVirtualMethod(image),x,y,pixel,exception));
  assert(id < (int) cache_info->number_threads);
  p=GetVirtualPixelsFromNexus(image,GetPixelCacheVirtualMethod(image),x,y,
    1UL,1UL,cache_info->nexus_info[id],exception);
  if (p == (const Quantum *) NULL)
    return(MagickFalse);
  GetPixelPacket(image,p,pixel);
  if (image->colorspace == CMYKColorspace)
    pixel->black=GetPixelBlack(image,p);
  return(MagickTrue);
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
%        PixelPacket *pixel,ExceptionInfo *exception)
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
  PixelPacket *pixel,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  const Quantum
    *p;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  assert(id < (int) cache_info->number_threads);
  *pixel=image->background_color;
  p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,x,y,1UL,1UL,
    cache_info->nexus_info[id],exception);
  if (p == (const Quantum *) NULL)
    return(MagickFalse);
  GetPixelPacket(image,p,pixel);
  if (image->colorspace == CMYKColorspace)
    pixel->black=GetPixelBlack(image,p);
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
MagickExport ColorspaceType GetPixelCacheColorspace(const Cache cache)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
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
MagickExport void GetPixelCacheMethods(CacheMethods *cache_methods)
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
MagickExport MagickSizeType GetPixelCacheNexusExtent(const Cache cache,
  NexusInfo *nexus_info)
{
  CacheInfo
    *cache_info;

  MagickSizeType
    extent;

  assert(cache != (const Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
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
+   G e t P i x e l C a c h e N e x u s M e t a c o n t e n t                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelCacheNexusMetacontent() returns the meta-content for the specified
%  cache nexus.
%
%  The format of the GetPixelCacheNexusMetacontent() method is:
%
%      void *GetPixelCacheNexusMetacontent(const Cache cache,
%        NexusInfo *nexus_info)
%
%  A description of each parameter follows:
%
%    o cache: the pixel cache.
%
%    o nexus_info: the cache nexus to return the meta-content.
%
*/
MagickExport void *GetPixelCacheNexusMetacontent(const Cache cache,
  NexusInfo *nexus_info)
{
  CacheInfo
    *cache_info;

  assert(cache != (const Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->storage_class == UndefinedClass)
    return((void *) NULL);
  return(nexus_info->metacontent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t P i x e l C a c h e N e x u s P i x e l s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelCacheNexusPixels() returns the pixels associated with the specified
%  cache nexus.
%
%  The format of the GetPixelCacheNexusPixels() method is:
%
%      Quantum *GetPixelCacheNexusPixels(const Cache cache,
%        NexusInfo *nexus_info)
%
%  A description of each parameter follows:
%
%    o cache: the pixel cache.
%
%    o nexus_info: the cache nexus to return the pixels.
%
*/
MagickExport Quantum *GetPixelCacheNexusPixels(const Cache cache,
  NexusInfo *nexus_info)
{
  CacheInfo
    *cache_info;

  assert(cache != (const Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->storage_class == UndefinedClass)
    return((Quantum *) NULL);
  return(nexus_info->pixels);
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
MagickExport void *GetPixelCachePixels(Image *image,MagickSizeType *length,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  assert(length != (MagickSizeType *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
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
MagickExport ClassType GetPixelCacheStorageClass(const Cache cache)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
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
MagickExport void GetPixelCacheTileSize(const Image *image,size_t *width,
  size_t *height)
{
  CacheInfo
    *cache_info;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  *width=2048UL/(cache_info->pixel_components*sizeof(Quantum));
  if (GetPixelCacheType(image) == DiskCache)
    *width=8192UL/(cache_info->pixel_components*sizeof(Quantum));
  *height=(*width);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t P i x e l C a c h e T y p e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelCacheType() returns the pixel cache type (e.g. memory, disk, etc.).
%
%  The format of the GetPixelCacheType() method is:
%
%      CacheType GetPixelCacheType(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport CacheType GetPixelCacheType(const Image *image)
{
  CacheInfo
    *cache_info;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  return(cache_info->type);
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
MagickExport VirtualPixelMethod GetPixelCacheVirtualMethod(const Image *image)
{
  CacheInfo
    *cache_info;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  const void
    *metacontent;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
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
MagickExport const void *GetVirtualMetacontentFromNexus(
  const Cache cache,NexusInfo *nexus_info)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  const void
    *metacontent;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.get_virtual_metacontent_from_handler !=
       (GetVirtualMetacontentFromHandler) NULL)
    {
      metacontent=cache_info->methods.
        get_virtual_metacontent_from_handler(image);
      return(metacontent);
    }
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

/*
  VirtualPixelModulo() computes the remainder of dividing offset by extent.  It
  returns not only the quotient (tile the offset falls in) but also the positive
  remainer within that tile such that 0 <= remainder < extent.  This method is
  essentially a ldiv() using a floored modulo division rather than the normal
  default truncated modulo division.
*/
static inline MagickModulo VirtualPixelModulo(const ssize_t offset,
  const size_t extent)
{
  MagickModulo
    modulo;

  modulo.quotient=offset/(ssize_t) extent;
  if (offset < 0L)
    modulo.quotient--;
  modulo.remainder=offset-modulo.quotient*(ssize_t) extent;
  return(modulo);
}

MagickExport const Quantum *GetVirtualPixelsFromNexus(const Image *image,
  const VirtualPixelMethod virtual_pixel_method,const ssize_t x,const ssize_t y,
  const size_t columns,const size_t rows,NexusInfo *nexus_info,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  MagickOffsetType
    offset;

  MagickSizeType
    length,
    number_pixels;

  NexusInfo
    **virtual_nexus;

  Quantum
    *pixels,
    *virtual_pixel;

  RectangleInfo
    region;

  register const Quantum
    *restrict p;

  register const void
    *restrict r;

  register Quantum
    *restrict q;

  register ssize_t
    u,
    v;

  register unsigned char
    *restrict s;

  void
    *virtual_associated_pixel;

  /*
    Acquire pixels.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->type == UndefinedCache)
    return((const Quantum *) NULL);
  region.x=x;
  region.y=y;
  region.width=columns;
  region.height=rows;
  pixels=SetPixelCacheNexusPixels(image,&region,nexus_info,exception);
  if (pixels == (Quantum *) NULL)
    return((const Quantum *) NULL);
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
        if (IsPixelAuthentic(cache_info,nexus_info) != MagickFalse)
          return(pixels);
        status=ReadPixelCachePixels(cache_info,nexus_info,exception);
        if (status == MagickFalse)
          return((const Quantum *) NULL);
        if (cache_info->metacontent_extent != 0)
          {
            status=ReadPixelCacheMetacontent(cache_info,nexus_info,exception);
            if (status == MagickFalse)
              return((const Quantum *) NULL);
          }
        return(pixels);
      }
  /*
    Pixel request is outside cache extents.
  */
  q=pixels;
  s=(unsigned char *) GetPixelCacheNexusMetacontent(cache_info,nexus_info);
  virtual_nexus=AcquirePixelCacheNexus(1);
  if (virtual_nexus == (NexusInfo **) NULL)
    {
      if (virtual_nexus != (NexusInfo **) NULL)
        virtual_nexus=DestroyPixelCacheNexus(virtual_nexus,1);
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "UnableToGetCacheNexus","`%s'",image->filename);
      return((const Quantum *) NULL);
    }
  virtual_pixel=(Quantum *) NULL;
  virtual_associated_pixel=(void *) NULL;
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
      /*
        Acquire virtual pixel and associated channels.
      */
      virtual_pixel=(Quantum *) AcquireQuantumMemory(
        cache_info->pixel_components,sizeof(*virtual_pixel));
      if (virtual_pixel == (Quantum *) NULL)
        {
          virtual_nexus=DestroyPixelCacheNexus(virtual_nexus,1);
          (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
            "UnableToGetCacheNexus","`%s'",image->filename);
          return((const Quantum *) NULL);
        }
      (void) ResetMagickMemory(virtual_pixel,0,cache_info->pixel_components*
        sizeof(*virtual_pixel));
      if (cache_info->metacontent_extent != 0)
        {
          virtual_associated_pixel=(void *) AcquireMagickMemory(
            cache_info->metacontent_extent);
          if (virtual_associated_pixel == (void *) NULL)
            {
              virtual_pixel=(Quantum *) RelinquishMagickMemory(
                virtual_pixel);
              virtual_nexus=DestroyPixelCacheNexus(virtual_nexus,1);
              (void) ThrowMagickException(exception,GetMagickModule(),
                CacheError,"UnableToGetCacheNexus","`%s'",image->filename);
              return((const Quantum *) NULL);
            }
          (void) ResetMagickMemory(virtual_associated_pixel,0,
            cache_info->metacontent_extent);
        }
      switch (virtual_pixel_method)
      {
        case BlackVirtualPixelMethod:
        {
          SetPixelRed(image,0,virtual_pixel);
          SetPixelGreen(image,0,virtual_pixel);
          SetPixelBlue(image,0,virtual_pixel);
          SetPixelAlpha(image,OpaqueAlpha,virtual_pixel);
          break;
        }
        case GrayVirtualPixelMethod:
        {
          SetPixelRed(image,QuantumRange/2,virtual_pixel);
          SetPixelGreen(image,QuantumRange/2,virtual_pixel);
          SetPixelBlue(image,QuantumRange/2,virtual_pixel);
          SetPixelAlpha(image,OpaqueAlpha,virtual_pixel);
          break;
        }
        case TransparentVirtualPixelMethod:
        {
          SetPixelRed(image,0,virtual_pixel);
          SetPixelGreen(image,0,virtual_pixel);
          SetPixelBlue(image,0,virtual_pixel);
          SetPixelAlpha(image,TransparentAlpha,virtual_pixel);
          break;
        }
        case MaskVirtualPixelMethod:
        case WhiteVirtualPixelMethod:
        {
          SetPixelRed(image,(Quantum) QuantumRange,virtual_pixel);
          SetPixelGreen(image,(Quantum) QuantumRange,virtual_pixel);
          SetPixelBlue(image,(Quantum) QuantumRange,virtual_pixel);
          SetPixelAlpha(image,OpaqueAlpha,virtual_pixel);
          break;
        }
        default:
        {
          SetPixelRed(image,image->background_color.red,virtual_pixel);
          SetPixelGreen(image,image->background_color.green,virtual_pixel);
          SetPixelBlue(image,image->background_color.blue,virtual_pixel);
          SetPixelAlpha(image,image->background_color.alpha,virtual_pixel);
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
    for (u=0; u < (ssize_t) columns; u+=length)
    {
      length=(MagickSizeType) MagickMin(cache_info->columns-(x+u),columns-u);
      if ((((x+u) < 0) || ((x+u) >= (ssize_t) cache_info->columns)) ||
          (((y+v) < 0) || ((y+v) >= (ssize_t) cache_info->rows)) ||
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
            default:
            {
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                EdgeX(x+u,cache_info->columns),EdgeY(y+v,cache_info->rows),
                1UL,1UL,*virtual_nexus,exception);
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
                DitherX(x+u,cache_info->columns),DitherY(y+v,cache_info->rows),
                1UL,1UL,*virtual_nexus,exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
            case TileVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,*virtual_nexus,
                exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
            case MirrorVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              if ((x_modulo.quotient & 0x01) == 1L)
                x_modulo.remainder=(ssize_t) cache_info->columns-
                  x_modulo.remainder-1L;
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
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
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,EdgeY(y+v,cache_info->rows),1UL,1UL,
                *virtual_nexus,exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
            case VerticalTileEdgeVirtualPixelMethod:
            {
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                EdgeX(x+u,cache_info->columns),y_modulo.remainder,1UL,1UL,
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
              r=virtual_associated_pixel;
              break;
            }
            case EdgeVirtualPixelMethod:
            case CheckerTileVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              if (((x_modulo.quotient ^ y_modulo.quotient) & 0x01) != 0L)
                {
                  p=virtual_pixel;
                  r=virtual_associated_pixel;
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
              if (((y+v) < 0) || ((y+v) >= (ssize_t) cache_info->rows))
                {
                  p=virtual_pixel;
                  r=virtual_associated_pixel;
                  break;
                }
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,*virtual_nexus,
                exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
            case VerticalTileVirtualPixelMethod:
            {
              if (((x+u) < 0) || ((x+u) >= (ssize_t) cache_info->columns))
                {
                  p=virtual_pixel;
                  r=virtual_associated_pixel;
                  break;
                }
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,*virtual_nexus,
                exception);
              r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
              break;
            }
          }
          if (p == (const Quantum *) NULL)
            break;
          (void) memcpy(q,p,(size_t) length*cache_info->pixel_components*
            sizeof(*p));
          q+=cache_info->pixel_components;
          if ((s != (void *) NULL) &&
              (r != (const void *) NULL))
            {
              (void) memcpy(s,r,(size_t) cache_info->metacontent_extent);
              s+=cache_info->metacontent_extent;
            }
          continue;
        }
      /*
        Transfer a run of pixels.
      */
      p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,x+u,y+v,(size_t)
        length,1UL,*virtual_nexus,exception);
      if (p == (const Quantum *) NULL)
        break;
      r=GetVirtualMetacontentFromNexus(cache_info,*virtual_nexus);
      (void) memcpy(q,p,(size_t) length*cache_info->pixel_components*sizeof(*p));
      q+=length*cache_info->pixel_components;
      if ((s != (void *) NULL) && (r != (const void *) NULL))
        {
          (void) memcpy(s,r,(size_t) length);
          s+=length*cache_info->metacontent_extent;
        }
    }
  }
  /*
    Free resources.
  */
  if (virtual_associated_pixel != (void *) NULL)
    virtual_associated_pixel=(void *) RelinquishMagickMemory(
      virtual_associated_pixel);
  if (virtual_pixel != (Quantum *) NULL)
    virtual_pixel=(Quantum *) RelinquishMagickMemory(virtual_pixel);
  virtual_nexus=DestroyPixelCacheNexus(virtual_nexus,1);
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  const Quantum
    *pixels;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  assert(id < (int) cache_info->number_threads);
  pixels=GetVirtualPixelsFromNexus(image,virtual_pixel_method,x,y,columns,rows,
    cache_info->nexus_info[id],exception);
  return(pixels);
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  const Quantum
    *pixels;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.get_virtual_pixel_handler !=
       (GetVirtualPixelHandler) NULL)
    return(cache_info->methods.get_virtual_pixel_handler(image,
      GetPixelCacheVirtualMethod(image),x,y,columns,rows,exception));
  assert(id < (int) cache_info->number_threads);
  pixels=GetVirtualPixelsFromNexus(image,GetPixelCacheVirtualMethod(image),x,y,
    columns,rows,cache_info->nexus_info[id],exception);
  return(pixels);
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
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
MagickExport const Quantum *GetVirtualPixelsNexus(const Cache cache,
  NexusInfo *nexus_info)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->storage_class == UndefinedClass)
    return((Quantum *) NULL);
  return((const Quantum *) nexus_info->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a s k P i x e l C a c h e N e x u s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MaskPixelCacheNexus() masks the cache nexus as defined by the image mask.
%  The method returns MagickTrue if the pixel region is masked, otherwise
%  MagickFalse.
%
%  The format of the MaskPixelCacheNexus() method is:
%
%      MagickBooleanType MaskPixelCacheNexus(Image *image,
%        NexusInfo *nexus_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o nexus_info: the cache nexus to clip.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline void MagickPixelCompositeMask(const PixelInfo *p,
  const MagickRealType alpha,const PixelInfo *q,
  const MagickRealType beta,PixelInfo *composite)
{
  MagickRealType
    gamma;

  if (alpha == TransparentAlpha)
    {
      *composite=(*q);
      return;
    }
  gamma=1.0-QuantumScale*QuantumScale*alpha*beta;
  gamma=1.0/(gamma <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*MagickOver_(p->red,alpha,q->red,beta);
  composite->green=gamma*MagickOver_(p->green,alpha,q->green,beta);
  composite->blue=gamma*MagickOver_(p->blue,alpha,q->blue,beta);
  if ((p->colorspace == CMYKColorspace) && (q->colorspace == CMYKColorspace))
    composite->black=gamma*MagickOver_(p->black,alpha,q->black,beta);
}

static MagickBooleanType MaskPixelCacheNexus(Image *image,NexusInfo *nexus_info,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  PixelInfo
    alpha,
    beta;

  MagickSizeType
    number_pixels;

  NexusInfo
    **clip_nexus,
    **image_nexus;

  register const Quantum
    *restrict p,
    *restrict r;

  register Quantum
    *restrict q;

  register ssize_t
    i;

  /*
    Apply clip mask.
  */
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->mask == (Image *) NULL)
    return(MagickFalse);
  cache_info=(CacheInfo *) image->cache;
  if (cache_info == (Cache) NULL)
    return(MagickFalse);
  image_nexus=AcquirePixelCacheNexus(1);
  clip_nexus=AcquirePixelCacheNexus(1);
  if ((image_nexus == (NexusInfo **) NULL) ||
      (clip_nexus == (NexusInfo **) NULL))
    ThrowBinaryException(CacheError,"UnableToGetCacheNexus",image->filename);
  p=(const Quantum *) GetAuthenticPixelCacheNexus(image,
    nexus_info->region.x,nexus_info->region.y,nexus_info->region.width,
    nexus_info->region.height,image_nexus[0],exception);
  q=nexus_info->pixels;
  r=GetVirtualPixelsFromNexus(image->mask,MaskVirtualPixelMethod,
    nexus_info->region.x,nexus_info->region.y,nexus_info->region.width,
    nexus_info->region.height,clip_nexus[0],&image->exception);
  GetPixelInfo(image,&alpha);
  GetPixelInfo(image,&beta);
  number_pixels=(MagickSizeType) nexus_info->region.width*
    nexus_info->region.height;
  for (i=0; i < (ssize_t) number_pixels; i++)
  {
    if ((p == (const Quantum *) NULL) || (r == (const Quantum *) NULL))
      break;
    SetPixelInfo(image,p,&alpha);
    SetPixelInfo(image,q,&beta);
    MagickPixelCompositeMask(&beta,(MagickRealType) GetPixelIntensity(image,r),
      &alpha,alpha.alpha,&beta);
    SetPixelRed(image,ClampToQuantum(beta.red),q);
    SetPixelGreen(image,ClampToQuantum(beta.green),q);
    SetPixelBlue(image,ClampToQuantum(beta.blue),q);
    if (cache_info->colorspace == CMYKColorspace)
      SetPixelBlack(image,ClampToQuantum(beta.black),q);
    SetPixelAlpha(image,ClampToQuantum(beta.alpha),q);
    p++;
    q++;
    r++;
  }
  clip_nexus=DestroyPixelCacheNexus(clip_nexus,1);
  image_nexus=DestroyPixelCacheNexus(image_nexus,1);
  if (i < (ssize_t) number_pixels)
    return(MagickFalse);
  return(MagickTrue);
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

static inline void AllocatePixelCachePixels(CacheInfo *cache_info)
{
  cache_info->mapped=MagickFalse;
  cache_info->pixels=(Quantum *) AcquireMagickMemory((size_t)
    cache_info->length);
  if (cache_info->pixels == (Quantum *) NULL)
    {
      cache_info->mapped=MagickTrue;
      cache_info->pixels=(Quantum *) MapBlob(-1,IOMode,0,(size_t)
        cache_info->length);
    }
}

static MagickBooleanType ExtendCache(Image *image,MagickSizeType length)
{
  CacheInfo
    *cache_info;

  MagickOffsetType
    count,
    extent,
    offset;

  cache_info=(CacheInfo *) image->cache;
  if (image->debug != MagickFalse)
    {
      char
        format[MaxTextExtent],
        message[MaxTextExtent];

      (void) FormatMagickSize(length,MagickFalse,format);
      (void) FormatLocaleString(message,MaxTextExtent,
        "extend %s (%s[%d], disk, %s)",cache_info->filename,
        cache_info->cache_filename,cache_info->file,format);
      (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",message);
    }
  if (length != (MagickSizeType) ((MagickOffsetType) length))
    return(MagickFalse);
  extent=(MagickOffsetType) lseek(cache_info->file,0,SEEK_END);
  if (extent < 0)
    return(MagickFalse);
  if ((MagickSizeType) extent >= length)
    return(MagickTrue);
  offset=(MagickOffsetType) length-1;
  count=WritePixelCacheRegion(cache_info,offset,1,(const unsigned char *) "");
  return(count == (MagickOffsetType) 1 ? MagickTrue : MagickFalse);
}

static MagickBooleanType OpenPixelCache(Image *image,const MapMode mode,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info,
    source_info;

  char
    format[MaxTextExtent],
    message[MaxTextExtent];

  MagickBooleanType
    status;

  MagickSizeType
    length,
    number_pixels;

  size_t
    columns,
    packet_size;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->columns == 0) || (image->rows == 0))
    ThrowBinaryException(CacheError,"NoPixelsDefinedInCache",image->filename);
  StandardPixelComponentMap(image);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  source_info=(*cache_info);
  source_info.file=(-1);
  (void) FormatLocaleString(cache_info->filename,MaxTextExtent,"%s[%.20g]",
    image->filename,(double) GetImageIndexInList(image));
  cache_info->storage_class=image->storage_class;
  cache_info->colorspace=image->colorspace;
  cache_info->mode=mode;
  cache_info->rows=image->rows;
  cache_info->columns=image->columns;
  cache_info->pixel_components=GetPixelComponents(image);
  cache_info->metacontent_extent=image->metacontent_extent;
  if (image->ping != MagickFalse)
    {
      cache_info->type=PingCache;
      cache_info->pixels=(Quantum *) NULL;
      cache_info->metacontent=(void *) NULL;
      cache_info->length=0;
      return(MagickTrue);
    }
  number_pixels=(MagickSizeType) cache_info->columns*cache_info->rows;
  packet_size=cache_info->pixel_components*sizeof(Quantum);
  if (image->metacontent_extent != 0)
    packet_size+=cache_info->metacontent_extent;
  length=number_pixels*packet_size;
  columns=(size_t) (length/cache_info->rows/packet_size);
  if (cache_info->columns != columns)
    ThrowBinaryException(ResourceLimitError,"PixelCacheAllocationFailed",
      image->filename);
  cache_info->length=length;
  if ((cache_info->type != UndefinedCache) &&
      (cache_info->columns <= source_info.columns) &&
      (cache_info->rows <= source_info.rows) &&
      (cache_info->pixel_components <= source_info.pixel_components) &&
      (cache_info->metacontent_extent <= source_info.metacontent_extent))
    {
      /*
        Inline pixel cache clone optimization.
      */
      if ((cache_info->columns == source_info.columns) &&
          (cache_info->rows == source_info.rows) &&
          (cache_info->pixel_components == source_info.pixel_components) &&
          (cache_info->metacontent_extent == source_info.metacontent_extent))
        return(MagickTrue);
      return(ClonePixelCachePixels(cache_info,&source_info,exception));
    }
  status=AcquireMagickResource(AreaResource,cache_info->length);
  length=number_pixels*(cache_info->pixel_components*sizeof(Quantum)+
    cache_info->metacontent_extent);
  if ((status != MagickFalse) && (length == (MagickSizeType) ((size_t) length)))
    {
      status=AcquireMagickResource(MemoryResource,cache_info->length);
      if (((cache_info->type == UndefinedCache) && (status != MagickFalse)) ||
          (cache_info->type == MemoryCache))
        {
          AllocatePixelCachePixels(cache_info);
          if (cache_info->pixels == (Quantum *) NULL)
            cache_info->pixels=source_info.pixels;
          else
            {
              /*
                Create memory pixel cache.
              */
              status=MagickTrue;
              if (image->debug != MagickFalse)
                {
                  (void) FormatMagickSize(cache_info->length,MagickTrue,
                    format);
                  (void) FormatLocaleString(message,MaxTextExtent,
                    "open %s (%s memory, %.20gx%.20gx%.20g %s)",
                    cache_info->filename,cache_info->mapped != MagickFalse ?
                    "anonymous" : "heap",(double) cache_info->columns,(double)
                    cache_info->rows,(double) cache_info->pixel_components,
                    format);
                  (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",
                    message);
                }
              cache_info->type=MemoryCache;
              cache_info->metacontent=(void *) NULL;
              if (cache_info->metacontent_extent != 0)
                cache_info->metacontent=(void *) (cache_info->pixels+
                  number_pixels*cache_info->pixel_components);
              if (source_info.storage_class != UndefinedClass)
                {
                  status=ClonePixelCachePixels(cache_info,&source_info,
                    exception);
                  RelinquishPixelCachePixels(&source_info);
                }
              return(status);
            }
        }
      RelinquishMagickResource(MemoryResource,cache_info->length);
    }
  /*
    Create pixel cache on disk.
  */
  status=AcquireMagickResource(DiskResource,cache_info->length);
  if (status == MagickFalse)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "CacheResourcesExhausted","`%s'",image->filename);
      return(MagickFalse);
    }
  if (OpenPixelCacheOnDisk(cache_info,mode) == MagickFalse)
    {
      RelinquishMagickResource(DiskResource,cache_info->length);
      ThrowFileException(exception,CacheError,"UnableToOpenPixelCache",
        image->filename);
      return(MagickFalse);
    }
  status=ExtendCache(image,(MagickSizeType) cache_info->offset+
    cache_info->length);
  if (status == MagickFalse)
    {
      ThrowFileException(exception,CacheError,"UnableToExtendCache",
        image->filename);
      return(MagickFalse);
    }
  length=number_pixels*(cache_info->pixel_components*sizeof(Quantum)+
    cache_info->metacontent_extent);
  status=AcquireMagickResource(AreaResource,cache_info->length);
  if ((status == MagickFalse) || (length != (MagickSizeType) ((size_t) length)))
    cache_info->type=DiskCache;
  else
    {
      status=AcquireMagickResource(MapResource,cache_info->length);
      if ((status == MagickFalse) && (cache_info->type != MapCache) &&
          (cache_info->type != MemoryCache))
        cache_info->type=DiskCache;
      else
        {
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
              status=MagickTrue;
              (void) ClosePixelCacheOnDisk(cache_info);
              cache_info->type=MapCache;
              cache_info->mapped=MagickTrue;
              cache_info->metacontent=(void *) NULL;
              if (cache_info->metacontent_extent != 0)
                cache_info->metacontent=(void *) (cache_info->pixels+
                  number_pixels*cache_info->pixel_components);
              if (source_info.storage_class != UndefinedClass)
                {
                  status=ClonePixelCachePixels(cache_info,&source_info,
                    exception);
                  RelinquishPixelCachePixels(&source_info);
                }
              if (image->debug != MagickFalse)
                {
                  (void) FormatMagickSize(cache_info->length,MagickTrue,
                    format);
                  (void) FormatLocaleString(message,MaxTextExtent,
                    "open %s (%s[%d], memory-mapped, %.20gx%.20gx%.20g %s)",
                    cache_info->filename,cache_info->cache_filename,
                    cache_info->file,(double) cache_info->columns,(double)
                    cache_info->rows,(double) cache_info->pixel_components,
                    format);
                  (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",
                    message);
                }
              return(status);
            }
        }
      RelinquishMagickResource(MapResource,cache_info->length);
    }
  status=MagickTrue;
  if ((source_info.type != UndefinedCache) && (mode != ReadMode))
    {
      status=ClonePixelCachePixels(cache_info,&source_info,exception);
      RelinquishPixelCachePixels(&source_info);
    }
  if (image->debug != MagickFalse)
    {
      (void) FormatMagickSize(cache_info->length,MagickFalse,format);
      (void) FormatLocaleString(message,MaxTextExtent,
        "open %s (%s[%d], disk, %.20gx%.20gx%.20g %s)",cache_info->filename,
        cache_info->cache_filename,cache_info->file,(double)
        cache_info->columns,(double) cache_info->rows,(double)
        cache_info->pixel_components,format);
      (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",message);
    }
  return(status);
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
    *cache_info,
    *clone_info;

  Image
    clone_image;

  MagickBooleanType
    status;

  ssize_t
    page_size;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (void *) NULL);
  assert(filename != (const char *) NULL);
  assert(offset != (MagickOffsetType *) NULL);
  page_size=GetMagickPageSize();
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (attach != MagickFalse)
    {
      /*
        Attach existing persistent pixel cache.
      */
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CacheEvent,GetMagickModule(),
          "attach persistent cache");
      (void) CopyMagickString(cache_info->cache_filename,filename,
        MaxTextExtent);
      cache_info->type=DiskCache;
      cache_info->offset=(*offset);
      if (OpenPixelCache(image,ReadMode,exception) == MagickFalse)
        return(MagickFalse);
      *offset+=cache_info->length+page_size-(cache_info->length % page_size);
      return(MagickTrue);
    }
  if ((cache_info->mode != ReadMode) && (cache_info->type != MemoryCache) &&
      (cache_info->reference_count == 1))
    {
      LockSemaphoreInfo(cache_info->semaphore);
      if ((cache_info->mode != ReadMode) &&
          (cache_info->type != MemoryCache) &&
          (cache_info->reference_count == 1))
        {
          int
            status;

          /*
            Usurp existing persistent pixel cache.
          */
          status=rename(cache_info->cache_filename,filename);
          if (status == 0)
            {
              (void) CopyMagickString(cache_info->cache_filename,filename,
                MaxTextExtent);
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
  (void) CopyMagickString(cache_info->cache_filename,filename,MaxTextExtent);
  cache_info->type=DiskCache;
  cache_info->offset=(*offset);
  cache_info=(CacheInfo *) image->cache;
  status=OpenPixelCache(image,IOMode,exception);
  if (status != MagickFalse)
    status=ClonePixelCachePixels(cache_info,clone_info,&image->exception);
  *offset+=cache_info->length+page_size-(cache_info->length % page_size);
  clone_info=(CacheInfo *) DestroyPixelCache(clone_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   Q u e u e A u t h e n t i c N e x u s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QueueAuthenticNexus() allocates an region to store image pixels as defined
%  by the region rectangle and returns a pointer to the region.  This region is
%  subsequently transferred from the pixel cache with
%  SyncAuthenticPixelsCache().  A pointer to the pixels is returned if the
%  pixels are transferred, otherwise a NULL is returned.
%
%  The format of the QueueAuthenticNexus() method is:
%
%      Quantum *QueueAuthenticNexus(Image *image,const ssize_t x,
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
%    o nexus_info: the cache nexus to set.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Quantum *QueueAuthenticNexus(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,NexusInfo *nexus_info,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  MagickOffsetType
    offset;

  MagickSizeType
    number_pixels;

  RectangleInfo
    region;

  /*
    Validate pixel cache geometry.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) GetImagePixelCache(image,MagickTrue,exception);
  assert(cache_info->signature == MagickSignature);
  if (cache_info == (Cache) NULL)
    return((Quantum *) NULL);
  if ((cache_info->columns == 0) && (cache_info->rows == 0))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "NoPixelsDefinedInCache","`%s'",image->filename);
      return((Quantum *) NULL);
    }
  if ((x < 0) || (y < 0) || (x >= (ssize_t) cache_info->columns) ||
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
  return(SetPixelCacheNexusPixels(image,&region,nexus_info,exception));
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  Quantum
    *pixels;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  assert(id < (int) cache_info->number_threads);
  pixels=QueueAuthenticNexus(image,x,y,columns,rows,cache_info->nexus_info[id],
    exception);
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  Quantum
    *pixels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.queue_authentic_pixels_handler !=
       (QueueAuthenticPixelsHandler) NULL)
    {
      pixels=cache_info->methods.queue_authentic_pixels_handler(image,x,y,
        columns,rows,exception);
      return(pixels);
    }
  assert(id < (int) cache_info->number_threads);
  pixels=QueueAuthenticNexus(image,x,y,columns,rows,cache_info->nexus_info[id],
    exception);
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
static MagickBooleanType ReadPixelCacheMetacontent(CacheInfo *cache_info,
  NexusInfo *nexus_info,ExceptionInfo *exception)
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
    *restrict q;

  size_t
    rows;

  if (cache_info->metacontent_extent == 0)
    return(MagickFalse);
  if (IsPixelAuthentic(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*
    cache_info->metacontent_extent;
  rows=nexus_info->region.height;
  extent=length*rows;
  q=(unsigned char *) nexus_info->metacontent;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register unsigned char
        *restrict p;

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
      if (OpenPixelCacheOnDisk(cache_info,IOMode) == MagickFalse)
        {
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            cache_info->cache_filename);
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
          cache_info->pixel_components*sizeof(Quantum)+offset*
          cache_info->metacontent_extent,length,(unsigned char *) q);
        if ((MagickSizeType) count != length)
          break;
        offset+=cache_info->columns;
        q+=cache_info->metacontent_extent*nexus_info->region.width;
      }
      if (y < (ssize_t) rows)
        {
          ThrowFileException(exception,CacheError,"UnableToReadPixelCache",
            cache_info->cache_filename);
          return(MagickFalse);
        }
      break;
    }
    default:
      break;
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
static MagickBooleanType ReadPixelCachePixels(CacheInfo *cache_info,
  NexusInfo *nexus_info,ExceptionInfo *exception)
{
  MagickOffsetType
    count,
    offset;

  MagickSizeType
    extent,
    length;

  register Quantum
    *restrict q;

  register ssize_t
    y;

  size_t
    rows;

  if (IsPixelAuthentic(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*cache_info->pixel_components*
    sizeof(Quantum);
  rows=nexus_info->region.height;
  extent=length*rows;
  q=nexus_info->pixels;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register Quantum
        *restrict p;

      /*
        Read pixels from memory.
      */
      if ((cache_info->columns == nexus_info->region.width) &&
          (extent == (MagickSizeType) ((size_t) extent)))
        {
          length=extent;
          rows=1UL;
        }
      p=cache_info->pixels+offset*cache_info->pixel_components;
      for (y=0; y < (ssize_t) rows; y++)
      {
        (void) memcpy(q,p,(size_t) length);
        p+=cache_info->pixel_components*cache_info->columns;
        q+=cache_info->pixel_components*nexus_info->region.width;
      }
      break;
    }
    case DiskCache:
    {
      /*
        Read pixels from disk.
      */
      if (OpenPixelCacheOnDisk(cache_info,IOMode) == MagickFalse)
        {
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            cache_info->cache_filename);
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
          cache_info->pixel_components*sizeof(*q),length,(unsigned char *) q);
        if ((MagickSizeType) count != length)
          break;
        offset+=cache_info->columns;
        q+=cache_info->pixel_components*nexus_info->region.width;
      }
      if (y < (ssize_t) rows)
        {
          ThrowFileException(exception,CacheError,"UnableToReadPixelCache",
            cache_info->cache_filename);
          return(MagickFalse);
        }
      break;
    }
    default:
      break;
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
MagickExport Cache ReferencePixelCache(Cache cache)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache *) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
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
MagickExport void SetPixelCacheMethods(Cache cache,CacheMethods *cache_methods)
{
  CacheInfo
    *cache_info;

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
  assert(cache_info->signature == MagickSignature);
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
%      Quantum SetPixelCacheNexusPixels(const Image *image,
%        const RectangleInfo *region,NexusInfo *nexus_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o region: A pointer to the RectangleInfo structure that defines the
%      region of this particular cache nexus.
%
%    o nexus_info: the cache nexus to set.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline MagickBooleanType AcquireCacheNexusPixels(CacheInfo *cache_info,
  NexusInfo *nexus_info,ExceptionInfo *exception)
{
  if (nexus_info->length != (MagickSizeType) ((size_t) nexus_info->length))
    return(MagickFalse);
  nexus_info->mapped=MagickFalse;
  nexus_info->cache=(Quantum *) AcquireMagickMemory((size_t)
    nexus_info->length);
  if (nexus_info->cache == (Quantum *) NULL)
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

static Quantum *SetPixelCacheNexusPixels(const Image *image,
  const RectangleInfo *region,NexusInfo *nexus_info,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  MagickBooleanType
    status;

  MagickSizeType
    length,
    number_pixels;

  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->type == UndefinedCache)
    return((Quantum *) NULL);
  nexus_info->region=(*region);
  if ((cache_info->type != DiskCache) && (cache_info->type != PingCache) &&
      (image->clip_mask == (Image *) NULL) && (image->mask == (Image *) NULL))
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
          nexus_info->pixels=cache_info->pixels+cache_info->pixel_components*
            offset;
          nexus_info->metacontent=(void *) NULL;
          if (cache_info->metacontent_extent != 0)
            nexus_info->metacontent=(unsigned char *) cache_info->metacontent+
              offset*cache_info->metacontent_extent;
          return(nexus_info->pixels);
        }
    }
  /*
    Pixels are stored in a cache region until they are synced to the cache.
  */
  number_pixels=(MagickSizeType) nexus_info->region.width*
    nexus_info->region.height;
  length=number_pixels*cache_info->pixel_components*sizeof(Quantum);
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
    if (nexus_info->length != length)
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
      cache_info->pixel_components);
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
%      VirtualPixelMethod SetPixelCacheVirtualMethod(const Image *image,
%        const VirtualPixelMethod virtual_pixel_method)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o virtual_pixel_method: choose the type of virtual pixel.
%
*/
MagickExport VirtualPixelMethod SetPixelCacheVirtualMethod(const Image *image,
  const VirtualPixelMethod virtual_pixel_method)
{
  CacheInfo
    *cache_info;

  VirtualPixelMethod
    method;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  method=cache_info->virtual_pixel_method;
  cache_info->virtual_pixel_method=virtual_pixel_method;
  return(method);
}

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
MagickExport MagickBooleanType SyncAuthenticPixelCacheNexus(Image *image,
  NexusInfo *nexus_info,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  MagickBooleanType
    status;

  /*
    Transfer pixels to the cache.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->cache == (Cache) NULL)
    ThrowBinaryException(CacheError,"PixelCacheIsNotOpen",image->filename);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->type == UndefinedCache)
    return(MagickFalse);
  if ((image->clip_mask != (Image *) NULL) &&
      (ClipPixelCacheNexus(image,nexus_info,exception) == MagickFalse))
    return(MagickFalse);
  if ((image->mask != (Image *) NULL) &&
      (MaskPixelCacheNexus(image,nexus_info,exception) == MagickFalse))
    return(MagickFalse);
  if (IsPixelAuthentic(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  assert(cache_info->signature == MagickSignature);
  status=WritePixelCachePixels(cache_info,nexus_info,exception);
  if ((cache_info->metacontent_extent != 0) &&
      (WritePixelCacheMetacontent(cache_info,nexus_info,exception) == MagickFalse))
    return(MagickFalse);
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
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
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
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
  NexusInfo *nexus_info,ExceptionInfo *exception)
{
  MagickOffsetType
    count,
    offset;

  MagickSizeType
    extent,
    length;

  register const unsigned char
    *restrict p;

  register ssize_t
    y;

  size_t
    rows;

  if (cache_info->metacontent_extent == 0)
    return(MagickFalse);
  if (IsPixelAuthentic(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*
    cache_info->metacontent_extent;
  rows=nexus_info->region.height;
  extent=(MagickSizeType) length*rows;
  p=(unsigned char *) nexus_info->metacontent;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register unsigned char
        *restrict q;

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
      if (OpenPixelCacheOnDisk(cache_info,IOMode) == MagickFalse)
        {
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            cache_info->cache_filename);
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
          cache_info->pixel_components*sizeof(Quantum)+offset*
          cache_info->metacontent_extent,length,(const unsigned char *) p);
        if ((MagickSizeType) count != length)
          break;
        p+=nexus_info->region.width*cache_info->metacontent_extent;
        offset+=cache_info->columns;
      }
      if (y < (ssize_t) rows)
        {
          ThrowFileException(exception,CacheError,"UnableToWritePixelCache",
            cache_info->cache_filename);
          return(MagickFalse);
        }
      break;
    }
    default:
      break;
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
static MagickBooleanType WritePixelCachePixels(CacheInfo *cache_info,
  NexusInfo *nexus_info,ExceptionInfo *exception)
{
  MagickOffsetType
    count,
    offset;

  MagickSizeType
    extent,
    length;

  register const Quantum
    *restrict p;

  register ssize_t
    y;

  size_t
    rows;

  if (IsPixelAuthentic(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*cache_info->pixel_components*
    sizeof(Quantum);
  rows=nexus_info->region.height;
  extent=length*rows;
  p=nexus_info->pixels;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register Quantum
        *restrict q;

      /*
        Write pixels to memory.
      */
      if ((cache_info->columns == nexus_info->region.width) &&
          (extent == (MagickSizeType) ((size_t) extent)))
        {
          length=extent;
          rows=1UL;
        }
      q=cache_info->pixels+offset*cache_info->pixel_components;
      for (y=0; y < (ssize_t) rows; y++)
      {
        (void) memcpy(q,p,(size_t) length);
        p+=nexus_info->region.width*cache_info->pixel_components;
        q+=cache_info->columns*cache_info->pixel_components;
      }
      break;
    }
    case DiskCache:
    {
      /*
        Write pixels to disk.
      */
      if (OpenPixelCacheOnDisk(cache_info,IOMode) == MagickFalse)
        {
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            cache_info->cache_filename);
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
          cache_info->pixel_components*sizeof(*p),length,(const unsigned char *)
          p);
        if ((MagickSizeType) count != length)
          break;
        p+=nexus_info->region.width*cache_info->pixel_components;
        offset+=cache_info->columns;
      }
      if (y < (ssize_t) rows)
        {
          ThrowFileException(exception,CacheError,"UnableToWritePixelCache",
            cache_info->cache_filename);
          return(MagickFalse);
        }
      break;
    }
    default:
      break;
  }
  if ((cache_info->debug != MagickFalse) &&
      (CacheTick(nexus_info->region.y,cache_info->rows) != MagickFalse))
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),
      "%s[%.20gx%.20g%+.20g%+.20g]",cache_info->filename,(double)
      nexus_info->region.width,(double) nexus_info->region.height,(double)
      nexus_info->region.x,(double) nexus_info->region.y);
  return(MagickTrue);
}

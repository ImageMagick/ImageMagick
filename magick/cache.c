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
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/cache-private.h"
#include "magick/color-private.h"
#include "magick/composite-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/pixel-private.h"
#include "magick/quantum.h"
#include "magick/random_.h"
#include "magick/resource_.h"
#include "magick/semaphore.h"
#include "magick/splay-tree.h"
#include "magick/string_.h"
#include "magick/thread-private.h"
#include "magick/utility.h"
#if defined(MAGICKCORE_ZLIB_DELEGATE)
#include "zlib.h"
#endif

/*
  Typedef declarations.
*/
typedef struct _MagickModulo
{
  long
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

  PixelPacket
    *cache,
    *pixels;

  IndexPacket
    *indexes;

  unsigned long
    signature;
};

/*
  Forward declarations.
*/
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static const IndexPacket
  *GetVirtualIndexesFromCache(const Image *);

static const PixelPacket
  *GetVirtualPixelCache(const Image *,const VirtualPixelMethod,const long,
    const long,const unsigned long,const unsigned long,ExceptionInfo *),
  *GetVirtualPixelsCache(const Image *);

static MagickBooleanType
  GetOneAuthenticPixelFromCache(Image *,const long,const long,PixelPacket *,
    ExceptionInfo *),
  GetOneVirtualPixelFromCache(const Image *,const VirtualPixelMethod,
    const long,const long,PixelPacket *,ExceptionInfo *),
  OpenPixelCache(Image *,const MapMode,ExceptionInfo *),
  ReadPixelCacheIndexes(CacheInfo *,NexusInfo *,ExceptionInfo *),
  ReadPixelCachePixels(CacheInfo *,NexusInfo *,ExceptionInfo *),
  SyncAuthenticPixelsCache(Image *,ExceptionInfo *),
  WritePixelCacheIndexes(CacheInfo *,NexusInfo *,ExceptionInfo *),
  WritePixelCachePixels(CacheInfo *,NexusInfo *,ExceptionInfo *);

static PixelPacket
  *GetAuthenticPixelsCache(Image *,const long,const long,const unsigned long,
    const unsigned long,ExceptionInfo *),
  *QueueAuthenticPixelsCache(Image *,const long,const long,const unsigned long,
    const unsigned long,ExceptionInfo *),
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

static time_t
  cache_timer = 0;

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
%      Cache AcquirePixelCache(const unsigned long number_threads)
%
%  A description of each parameter follows:
%
%    o number_threads: the number of nexus threads.
%
*/
MagickExport Cache AcquirePixelCache(const unsigned long number_threads)
{
  CacheInfo
    *cache_info;

  cache_info=(CacheInfo *) AcquireAlignedMemory(1,sizeof(*cache_info));
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
  GetPixelCacheMethods(&cache_info->methods);
  cache_info->reference_count=1;
  cache_info->semaphore=AllocateSemaphoreInfo();
  cache_info->disk_semaphore=AllocateSemaphoreInfo();
  cache_info->debug=IsEventLogging();
  cache_info->signature=MagickSignature;
  if ((cache_resources == (SplayTreeInfo *) NULL) &&
      (instantiate_cache == MagickFalse))
    {
      AcquireSemaphoreInfo(&cache_semaphore);
      if ((cache_resources == (SplayTreeInfo *) NULL) &&
          (instantiate_cache == MagickFalse))
        {
          cache_resources=NewSplayTree((int (*)(const void *,const void *))
            NULL,(void *(*)(void *)) NULL,(void *(*)(void *)) NULL);
          instantiate_cache=MagickTrue;
        }
      RelinquishSemaphoreInfo(cache_semaphore);
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
%      NexusInfo **AcquirePixelCacheNexus(const unsigned long number_threads)
%
%  A description of each parameter follows:
%
%    o number_threads: the number of nexus threads.
%
*/
MagickExport NexusInfo **AcquirePixelCacheNexus(
  const unsigned long number_threads)
{
  register long
    i;

  NexusInfo
    **nexus_info;

  nexus_info=(NexusInfo **) AcquireAlignedMemory(number_threads,
    sizeof(*nexus_info));
  if (nexus_info == (NexusInfo **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  for (i=0; i < (long) number_threads; i++)
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
+   C l i p P i x e l C a c h e N e x u s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClipPixelCacheNexus() clips the cache nexus as defined by the image clip
%  mask.  The method returns MagickTrue if the pixel region is clipped,
%  otherwise MagickFalse.
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

  register const PixelPacket
    *__restrict r;

  register IndexPacket
    *__restrict nexus_indexes,
    *__restrict indexes;

  register long
    i;

  register PixelPacket
    *__restrict p,
    *__restrict q;

  /*
    Apply clip mask.
  */
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->clip_mask == (Image *) NULL)
    return(MagickFalse);
  cache_info=(CacheInfo *) GetImagePixelCache(image,MagickTrue,exception);
  if (cache_info == (Cache) NULL)
    return(MagickFalse);
  image_nexus=AcquirePixelCacheNexus(1);
  clip_nexus=AcquirePixelCacheNexus(1);
  if ((image_nexus == (NexusInfo **) NULL) ||
      (clip_nexus == (NexusInfo **) NULL))
    ThrowBinaryException(CacheError,"UnableToGetCacheNexus",image->filename);
  p=GetAuthenticPixelCacheNexus(image,nexus_info->region.x,nexus_info->region.y,
    nexus_info->region.width,nexus_info->region.height,image_nexus[0],
    exception);
  indexes=GetPixelCacheNexusIndexes(image->cache,image_nexus[0]);
  q=nexus_info->pixels;
  nexus_indexes=nexus_info->indexes;
  r=GetVirtualPixelsFromNexus(image->clip_mask,MaskVirtualPixelMethod,
    nexus_info->region.x,nexus_info->region.y,nexus_info->region.width,
    nexus_info->region.height,clip_nexus[0],exception);
  number_pixels=(MagickSizeType) nexus_info->region.width*
    nexus_info->region.height;
  for (i=0; i < (long) number_pixels; i++)
  {
    if ((p == (PixelPacket *) NULL) || (r == (const PixelPacket *) NULL))
      break;
    if (PixelIntensityToQuantum(r) > ((Quantum) QuantumRange/2))
      {
        q->red=p->red;
        q->green=p->green;
        q->blue=p->blue;
        q->opacity=p->opacity;
        if (cache_info->active_index_channel != MagickFalse)
          nexus_indexes[i]=indexes[i];
      }
    p++;
    q++;
    r++;
  }
  clip_nexus=DestroyPixelCacheNexus(clip_nexus,1);
  image_nexus=DestroyPixelCacheNexus(image_nexus,1);
  if (i < (long) number_pixels)
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
+   C l o n e P i x e l C a c h e N e x u s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClonePixelCacheNexus() clones the source cache nexus to the destination
%  nexus.
%
%  The format of the ClonePixelCacheNexus() method is:
%
%      MagickBooleanType ClonePixelCacheNexus(CacheInfo *destination,
%        CacheInfo *source,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o destination: the destination cache nexus.
%
%    o source: the source cache nexus.
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
  nexus_info->cache=(PixelPacket *) AcquireMagickMemory((size_t)
    nexus_info->length);
  if (nexus_info->cache == (PixelPacket *) NULL)
    {
      nexus_info->mapped=MagickTrue;
      nexus_info->cache=(PixelPacket *) MapBlob(-1,IOMode,0,(size_t)
        nexus_info->length);
    }
  if (nexus_info->cache == (PixelPacket *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        cache_info->filename);
      return(MagickFalse);
    }
  return(MagickTrue);
}

static MagickBooleanType ClonePixelCacheNexus(CacheInfo *destination,
  CacheInfo *source,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  register long
    i;

  register const NexusInfo
    *p;

  register NexusInfo
    *q;

  status=MagickTrue;
  for (i=0; i < (long) source->number_threads; i++)
  {
    p=source->nexus_info[i];
    q=destination->nexus_info[i];
    q->mapped=p->mapped;
    q->region=p->region;
    q->length=p->length;
    q->cache=p->cache;
    q->pixels=p->pixels;
    q->indexes=p->indexes;
    if (p->cache != (PixelPacket *) NULL)
      {
        status=AcquireCacheNexusPixels(source,q,exception);
        if (status != MagickFalse)
          {
            (void) CopyMagickMemory(q->cache,p->cache,(size_t) p->length);
            q->pixels=q->cache;
            q->indexes=(IndexPacket *) NULL;
            number_pixels=(MagickSizeType) q->region.width*q->region.height;
            if (p->indexes != (IndexPacket *) NULL)
              q->indexes=(IndexPacket *) (q->pixels+number_pixels);
          }
      }
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l o n e P i x e l C a c h e P i x e l s                                %
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

  AcquireSemaphoreInfo(&cache_info->disk_semaphore);
  status=close(cache_info->file);
  cache_info->file=(-1);
  RelinquishMagickResource(FileResource,1);
  RelinquishSemaphoreInfo(cache_info->disk_semaphore);
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
  AcquireSemaphoreInfo(&cache_semaphore);
  if (cache_resources == (SplayTreeInfo *) NULL)
    {
      RelinquishSemaphoreInfo(cache_semaphore);
      return;
    }
  ResetSplayTreeIterator(cache_resources);
  p=(CacheInfo *) GetNextKeyInSplayTree(cache_resources);
  while (p != (CacheInfo *) NULL)
  {
    if ((p->type == DiskCache) && (p->file != -1))
      {
        if (IsMagickThreadEqual(p->id) != MagickFalse)
          break;
      }
    p=(CacheInfo *) GetNextKeyInSplayTree(cache_resources);
  }
  for (q=p; p != (CacheInfo *) NULL; )
  {
    if ((p->type == DiskCache) && (p->file != -1) &&
        (p->timestamp < q->timestamp))
      {
        if (IsMagickThreadEqual(p->id) != MagickFalse)
          q=p;
      }
    p=(CacheInfo *) GetNextKeyInSplayTree(cache_resources);
  }
  if (q != (CacheInfo *) NULL)
    (void) ClosePixelCacheOnDisk(q);  /* relinquish least recently used cache */
  RelinquishSemaphoreInfo(cache_semaphore);
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
  AcquireSemaphoreInfo(&cache_info->disk_semaphore);
  if (cache_info->file != -1)
    {
      RelinquishSemaphoreInfo(cache_info->disk_semaphore);
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
      RelinquishSemaphoreInfo(cache_info->disk_semaphore);
      return(MagickFalse);
    }
  (void) AcquireMagickResource(FileResource,1);
  cache_info->file=file;
  cache_info->timestamp=time(0);
  RelinquishSemaphoreInfo(cache_info->disk_semaphore);
  return(MagickTrue);
}

static inline MagickOffsetType ReadPixelCacheRegion(CacheInfo *cache_info,
  const MagickOffsetType offset,const MagickSizeType length,
  unsigned char *__restrict buffer)
{
  register MagickOffsetType
    i;

  ssize_t
    count;

#if !defined(MAGICKCORE_HAVE_PREAD)
  (void) LockSemaphoreInfo(cache_info->disk_semaphore);
  cache_info->timestamp=time(0);
  if (MagickSeek(cache_info->file,offset,SEEK_SET) < 0)
    {
      (void) UnlockSemaphoreInfo(cache_info->disk_semaphore);
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
  (void) UnlockSemaphoreInfo(cache_info->disk_semaphore);
#endif
  return(i);
}

static inline MagickOffsetType WritePixelCacheRegion(CacheInfo *cache_info,
  const MagickOffsetType offset,const MagickSizeType length,
  const unsigned char *__restrict buffer)
{
  register MagickOffsetType
    i;

  ssize_t
    count;

#if !defined(MAGICKCORE_HAVE_PWRITE)
  (void) LockSemaphoreInfo(cache_info->disk_semaphore);
  cache_info->timestamp=time(0);
  if (MagickSeek(cache_info->file,offset,SEEK_SET) < 0)
    {
      (void) UnlockSemaphoreInfo(cache_info->disk_semaphore);
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
  (void) UnlockSemaphoreInfo(cache_info->disk_semaphore);
#endif
  return(i);
}

static MagickBooleanType CloneDiskToDiskPixelCache(CacheInfo *clone_info,
  CacheInfo *cache_info,ExceptionInfo *exception)
{
  MagickOffsetType
    count,
    offset,
    source_offset;

  MagickSizeType
    length;

  register long
    y;

  register PixelPacket
    *__restrict pixels;

  unsigned long
    columns,
    rows;

  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),"disk => disk");
  if (OpenPixelCacheOnDisk(clone_info,IOMode) == MagickFalse)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        clone_info->cache_filename);
      return(MagickFalse);
    }
  if (OpenPixelCacheOnDisk(cache_info,IOMode) == MagickFalse)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        cache_info->cache_filename);
      return(MagickFalse);
    }
  columns=(unsigned long) MagickMin(clone_info->columns,cache_info->columns);
  rows=(unsigned long) MagickMin(clone_info->rows,cache_info->rows);
  if ((clone_info->active_index_channel != MagickFalse) &&
      (cache_info->active_index_channel != MagickFalse))
    {
      register IndexPacket
        *indexes;

      /*
        Clone cache indexes.
      */
      length=MagickMax(clone_info->columns,cache_info->columns)*
        sizeof(*indexes);
      indexes=(IndexPacket *) AcquireMagickMemory((size_t) length);
      if (indexes == (IndexPacket *) NULL)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
            "MemoryAllocationFailed","`%s'",cache_info->cache_filename);
          return(MagickFalse);
        }
      (void) ResetMagickMemory(indexes,0,(size_t) length);
      length=columns*sizeof(*indexes);
      source_offset=(MagickOffsetType) cache_info->columns*cache_info->rows*
        sizeof(*pixels)+cache_info->columns*rows*sizeof(*indexes);
      offset=(MagickOffsetType) clone_info->columns*clone_info->rows*
        sizeof(*pixels)+clone_info->columns*rows*sizeof(*indexes);
      for (y=0; y < (long) rows; y++)
      {
        source_offset-=cache_info->columns*sizeof(*indexes);
        count=ReadPixelCacheRegion(cache_info,cache_info->offset+source_offset,
          length,(unsigned char *) indexes);
        if ((MagickSizeType) count != length)
          break;
        offset-=clone_info->columns*sizeof(*indexes);
        count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,length,
          (unsigned char *) indexes);
        if ((MagickSizeType) count != length)
          break;
      }
      if (y < (long) rows)
        {
          indexes=(IndexPacket *) RelinquishMagickMemory(indexes);
          ThrowFileException(exception,CacheError,"UnableToCloneCache",
            cache_info->cache_filename);
          return(MagickFalse);
        }
      if (clone_info->columns > cache_info->columns)
        {
          length=(clone_info->columns-cache_info->columns)*sizeof(*indexes);
          (void) ResetMagickMemory(indexes,0,(size_t) length);
          offset=(MagickOffsetType) clone_info->columns*clone_info->rows*
            sizeof(*pixels)+(clone_info->columns*rows+columns)*sizeof(*indexes);
          for (y=0; y < (long) rows; y++)
          {
            offset-=clone_info->columns*sizeof(*indexes);
            count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,
              length,(unsigned char *) indexes);
            if ((MagickSizeType) count != length)
              break;
          }
          if (y < (long) rows)
            {
              indexes=(IndexPacket *) RelinquishMagickMemory(indexes);
              ThrowFileException(exception,CacheError,"UnableToCloneCache",
                cache_info->cache_filename);
              return(MagickFalse);
            }
        }
      indexes=(IndexPacket *) RelinquishMagickMemory(indexes);
    }
  /*
    Clone cache pixels.
  */
  length=MagickMax(clone_info->columns,cache_info->columns)*sizeof(*pixels);
  pixels=(PixelPacket *) AcquireMagickMemory((size_t) length);
  if (pixels == (PixelPacket *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "MemoryAllocationFailed","`%s'",cache_info->cache_filename);
      return(MagickFalse);
    }
  (void) ResetMagickMemory(pixels,0,(size_t) length);
  length=columns*sizeof(*pixels);
  source_offset=(MagickOffsetType) cache_info->columns*rows*sizeof(*pixels);
  offset=(MagickOffsetType) clone_info->columns*rows*sizeof(*pixels);
  for (y=0; y < (long) rows; y++)
  {
    source_offset-=cache_info->columns*sizeof(*pixels);
    count=ReadPixelCacheRegion(cache_info,cache_info->offset+source_offset,
      length,(unsigned char *) pixels);
    if ((MagickSizeType) count != length)
      break;
    offset-=clone_info->columns*sizeof(*pixels);
    count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,length,
      (unsigned char *) pixels);
    if ((MagickSizeType) count != length)
      break;
  }
  if (y < (long) rows)
    {
      pixels=(PixelPacket *) RelinquishMagickMemory(pixels);
      ThrowFileException(exception,CacheError,"UnableToCloneCache",
        cache_info->cache_filename);
      return(MagickFalse);
    }
  if (clone_info->columns > cache_info->columns)
    {
      offset=(MagickOffsetType) (clone_info->columns*rows+columns)*
        sizeof(*pixels);
      length=(clone_info->columns-cache_info->columns)*sizeof(*pixels);
      (void) ResetMagickMemory(pixels,0,(size_t) length);
      for (y=0; y < (long) rows; y++)
      {
        offset-=clone_info->columns*sizeof(*pixels);
        count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,length,
          (unsigned char *) pixels);
        if ((MagickSizeType) count != length)
          break;
      }
      if (y < (long) rows)
        {
          pixels=(PixelPacket *) RelinquishMagickMemory(pixels);
          ThrowFileException(exception,CacheError,"UnableToCloneCache",
            cache_info->cache_filename);
          return(MagickFalse);
        }
    }
  pixels=(PixelPacket *) RelinquishMagickMemory(pixels);
  return(MagickTrue);
}

static MagickBooleanType CloneDiskToMemoryPixelCache(CacheInfo *clone_info,
  CacheInfo *cache_info,ExceptionInfo *exception)
{
  MagickOffsetType
    count,
    offset;

  MagickSizeType
    length;

  register long
    y;

  register PixelPacket
    *__restrict pixels,
    *__restrict q;

  unsigned long
    columns,
    rows;

  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),"disk => memory");
  if (OpenPixelCacheOnDisk(cache_info,IOMode) == MagickFalse)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        cache_info->cache_filename);
      return(MagickFalse);
    }
  columns=(unsigned long) MagickMin(clone_info->columns,cache_info->columns);
  rows=(unsigned long) MagickMin(clone_info->rows,cache_info->rows);
  if ((clone_info->active_index_channel != MagickFalse) &&
      (cache_info->active_index_channel != MagickFalse))
    {
      register IndexPacket
        *indexes,
        *q;

      /*
        Clone cache indexes.
      */
      length=MagickMax(clone_info->columns,cache_info->columns)*
        sizeof(*indexes);
      indexes=(IndexPacket *) AcquireMagickMemory((size_t) length);
      if (indexes == (IndexPacket *) NULL)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
            "MemoryAllocationFailed","`%s'",cache_info->cache_filename);
          return(MagickFalse);
        }
      (void) ResetMagickMemory(indexes,0,(size_t) length);
      length=columns*sizeof(IndexPacket);
      offset=(MagickOffsetType) cache_info->columns*cache_info->rows*
        sizeof(*pixels)+cache_info->columns*rows*sizeof(*indexes);
      q=clone_info->indexes+clone_info->columns*rows;
      for (y=0; y < (long) rows; y++)
      {
        offset-=cache_info->columns*sizeof(IndexPacket);
        count=ReadPixelCacheRegion(cache_info,cache_info->offset+offset,
          length,(unsigned char *) indexes);
        if ((MagickSizeType) count != length)
          break;
        q-=clone_info->columns;
        (void) CopyMagickMemory(q,indexes,(size_t) length);
        if ((MagickSizeType) count != length)
          break;
      }
      if (y < (long) rows)
        {
          indexes=(IndexPacket *) RelinquishMagickMemory(indexes);
          ThrowFileException(exception,CacheError,"UnableToCloneCache",
            cache_info->cache_filename);
          return(MagickFalse);
        }
      indexes=(IndexPacket *) RelinquishMagickMemory(indexes);
    }
  /*
    Clone cache pixels.
  */
  length=MagickMax(clone_info->columns,cache_info->columns)*sizeof(*pixels);
  pixels=(PixelPacket *) AcquireMagickMemory((size_t) length);
  if (pixels == (PixelPacket *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "MemoryAllocationFailed","`%s'",cache_info->cache_filename);
      return(MagickFalse);
    }
  (void) ResetMagickMemory(pixels,0,(size_t) length);
  length=columns*sizeof(*pixels);
  offset=(MagickOffsetType) cache_info->columns*rows*sizeof(*pixels);
  q=clone_info->pixels+clone_info->columns*rows;
  for (y=0; y < (long) rows; y++)
  {
    offset-=cache_info->columns*sizeof(*pixels);
    count=ReadPixelCacheRegion(cache_info,cache_info->offset+offset,length,
      (unsigned char *) pixels);
    if ((MagickSizeType) count != length)
      break;
    q-=clone_info->columns;
    (void) CopyMagickMemory(q,pixels,(size_t) length);
  }
  if (y < (long) rows)
    {
      pixels=(PixelPacket *) RelinquishMagickMemory(pixels);
      ThrowFileException(exception,CacheError,"UnableToCloneCache",
        cache_info->cache_filename);
      return(MagickFalse);
    }
  pixels=(PixelPacket *) RelinquishMagickMemory(pixels);
  return(MagickTrue);
}

static MagickBooleanType CloneMemoryToDiskPixelCache(CacheInfo *clone_info,
  CacheInfo *cache_info,ExceptionInfo *exception)
{
  MagickOffsetType
    count,
    offset;

  MagickSizeType
    length;

  register long
    y;

  register PixelPacket
    *__restrict p,
    *__restrict pixels;

  unsigned long
    columns,
    rows;

  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),"memory => disk");
  if (OpenPixelCacheOnDisk(clone_info,IOMode) == MagickFalse)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        clone_info->cache_filename);
      return(MagickFalse);
    }
  columns=(unsigned long) MagickMin(clone_info->columns,cache_info->columns);
  rows=(unsigned long) MagickMin(clone_info->rows,cache_info->rows);
  if ((clone_info->active_index_channel != MagickFalse) &&
      (cache_info->active_index_channel != MagickFalse))
    {
      register IndexPacket
        *p,
        *indexes;

      /*
        Clone cache indexes.
      */
      length=MagickMax(clone_info->columns,cache_info->columns)*
        sizeof(*indexes);
      indexes=(IndexPacket *) AcquireMagickMemory((size_t) length);
      if (indexes == (IndexPacket *) NULL)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
            "MemoryAllocationFailed","`%s'",cache_info->cache_filename);
          return(MagickFalse);
        }
      (void) ResetMagickMemory(indexes,0,(size_t) length);
      length=columns*sizeof(*indexes);
      p=cache_info->indexes+cache_info->columns*rows;
      offset=(MagickOffsetType) clone_info->columns*clone_info->rows*
        sizeof(*pixels)+clone_info->columns*rows*sizeof(*indexes);
      for (y=0; y < (long) rows; y++)
      {
        p-=cache_info->columns;
        (void) CopyMagickMemory(indexes,p,(size_t) length);
        offset-=clone_info->columns*sizeof(*indexes);
        count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,length,
          (unsigned char *) indexes);
        if ((MagickSizeType) count != length)
          break;
      }
      if (y < (long) rows)
        {
          indexes=(IndexPacket *) RelinquishMagickMemory(indexes);
          ThrowFileException(exception,CacheError,"UnableToCloneCache",
            cache_info->cache_filename);
          return(MagickFalse);
        }
      if (clone_info->columns > cache_info->columns)
        {
          length=(clone_info->columns-cache_info->columns)*sizeof(*indexes);
          (void) ResetMagickMemory(indexes,0,(size_t) length);
          offset=(MagickOffsetType) clone_info->columns*clone_info->rows*
            sizeof(*pixels)+(clone_info->columns*rows+columns)*sizeof(*indexes);
          for (y=0; y < (long) rows; y++)
          {
            offset-=clone_info->columns*sizeof(*indexes);
            count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,
              length,(unsigned char *) indexes);
            if ((MagickSizeType) count != length)
              break;
          }
          if (y < (long) rows)
            {
              indexes=(IndexPacket *) RelinquishMagickMemory(indexes);
              ThrowFileException(exception,CacheError,"UnableToCloneCache",
                cache_info->cache_filename);
              return(MagickFalse);
            }
        }
      indexes=(IndexPacket *) RelinquishMagickMemory(indexes);
    }
  /*
    Clone cache pixels.
  */
  length=MagickMax(clone_info->columns,cache_info->columns)*sizeof(*pixels);
  pixels=(PixelPacket *) AcquireMagickMemory((size_t) length);
  if (pixels == (PixelPacket *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "MemoryAllocationFailed","`%s'",cache_info->cache_filename);
      return(MagickFalse);
    }
  (void) ResetMagickMemory(pixels,0,(size_t) length);
  length=columns*sizeof(*pixels);
  p=cache_info->pixels+cache_info->columns*rows;
  offset=(MagickOffsetType) clone_info->columns*rows*sizeof(*pixels);
  for (y=0; y < (long) rows; y++)
  {
    p-=cache_info->columns;
    (void) CopyMagickMemory(pixels,p,(size_t) length);
    offset-=clone_info->columns*sizeof(*pixels);
    count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,length,
      (unsigned char *) pixels);
    if ((MagickSizeType) count != length)
      break;
  }
  if (y < (long) rows)
    {
      pixels=(PixelPacket *) RelinquishMagickMemory(pixels);
      ThrowFileException(exception,CacheError,"UnableToCloneCache",
        cache_info->cache_filename);
      return(MagickFalse);
    }
  if (clone_info->columns > cache_info->columns)
    {
      offset=(MagickOffsetType) (clone_info->columns*rows+columns)*
        sizeof(*pixels);
      length=(clone_info->columns-cache_info->columns)*sizeof(*pixels);
      (void) ResetMagickMemory(pixels,0,(size_t) length);
      for (y=0; y < (long) rows; y++)
      {
        offset-=clone_info->columns*sizeof(*pixels);
        count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,length,
          (unsigned char *) pixels);
        if ((MagickSizeType) count != length)
          break;
      }
      if (y < (long) rows)
        {
          pixels=(PixelPacket *) RelinquishMagickMemory(pixels);
          ThrowFileException(exception,CacheError,"UnableToCloneCache",
            cache_info->cache_filename);
          return(MagickFalse);
        }
    }
  pixels=(PixelPacket *) RelinquishMagickMemory(pixels);
  return(MagickTrue);
}

static MagickBooleanType CloneMemoryToMemoryPixelCache(CacheInfo *clone_info,
  CacheInfo *cache_info,ExceptionInfo *magick_unused(exception))
{
  register long
    y;

  register PixelPacket
    *__restrict pixels,
    *__restrict source_pixels;

  size_t
    length;

  unsigned long
    columns,
    rows;

  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),"memory => memory");
  columns=(unsigned long) MagickMin(clone_info->columns,cache_info->columns);
  rows=(unsigned long) MagickMin(clone_info->rows,cache_info->rows);
  if ((clone_info->active_index_channel != MagickFalse) &&
      (cache_info->active_index_channel != MagickFalse))
    {
      register IndexPacket
        *indexes,
        *source_indexes;

      /*
        Clone cache indexes.
      */
      length=columns*sizeof(*indexes);
      if (clone_info->columns == cache_info->columns)
        (void) CopyMagickMemory(clone_info->indexes,cache_info->indexes,
          length*rows);
      else
        {
          source_indexes=cache_info->indexes+cache_info->columns*rows;
          indexes=clone_info->indexes+clone_info->columns*rows;
          for (y=0; y < (long) rows; y++)
          {
            source_indexes-=cache_info->columns;
            indexes-=clone_info->columns;
            (void) CopyMagickMemory(indexes,source_indexes,length);
          }
          if (clone_info->columns > cache_info->columns)
            {
              length=(clone_info->columns-cache_info->columns)*
                sizeof(*indexes);
              indexes=clone_info->indexes+clone_info->columns*rows+
                cache_info->columns;
              for (y=0; y < (long) rows; y++)
              {
                indexes-=clone_info->columns;
                (void) ResetMagickMemory(indexes,0,length);
              }
            }
        }
    }
  /*
    Clone cache pixels.
  */
  length=columns*sizeof(*pixels);
  if (clone_info->columns == cache_info->columns)
    (void) CopyMagickMemory(clone_info->pixels,cache_info->pixels,length*rows);
  else
    {
      source_pixels=cache_info->pixels+cache_info->columns*rows;
      pixels=clone_info->pixels+clone_info->columns*rows;
      for (y=0; y < (long) rows; y++)
      {
        source_pixels-=cache_info->columns;
        pixels-=clone_info->columns;
        (void) CopyMagickMemory(pixels,source_pixels,length);
      }
      if (clone_info->columns > cache_info->columns)
        {
          length=(clone_info->columns-cache_info->columns)*sizeof(*pixels);
          pixels=clone_info->pixels+clone_info->columns*rows+
            cache_info->columns;
          for (y=0; y < (long) rows; y++)
          {
            pixels-=clone_info->columns;
            (void) ResetMagickMemory(pixels,0,length);
          }
        }
    }
  return(MagickTrue);
}

static MagickBooleanType ClonePixelCachePixels(CacheInfo *clone_info,
  CacheInfo *cache_info,ExceptionInfo *exception)
{
  if ((clone_info->type != DiskCache) && (cache_info->type != DiskCache))
    return(CloneMemoryToMemoryPixelCache(clone_info,cache_info,exception));
  if ((clone_info->type == DiskCache) && (cache_info->type == DiskCache))
    return(CloneDiskToDiskPixelCache(clone_info,cache_info,exception));
  if (cache_info->type == DiskCache)
    return(CloneDiskToMemoryPixelCache(clone_info,cache_info,exception));
  return(CloneMemoryToDiskPixelCache(clone_info,cache_info,exception));
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
  if (cache_info->methods.destroy_pixel_handler == (DestroyPixelHandler) NULL)
    return;
  cache_info->methods.destroy_pixel_handler(image);
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
        cache_info->pixels=(PixelPacket *) RelinquishMagickMemory(
          cache_info->pixels);
      else
        cache_info->pixels=(PixelPacket *) UnmapBlob(cache_info->pixels,
          (size_t) cache_info->length);
      RelinquishMagickResource(MemoryResource,cache_info->length);
      break;
    }
    case MapCache:
    {
      cache_info->pixels=(PixelPacket *) UnmapBlob(cache_info->pixels,(size_t)
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
  cache_info->indexes=(IndexPacket *) NULL;
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
  (void) LockSemaphoreInfo(cache_info->semaphore);
  cache_info->reference_count--;
  if (cache_info->reference_count != 0)
    {
      (void) UnlockSemaphoreInfo(cache_info->semaphore);
      return((Cache) NULL);
    }
  (void) UnlockSemaphoreInfo(cache_info->semaphore);
  if (cache_resources != (SplayTreeInfo *) NULL)
    (void) DeleteNodeByValueFromSplayTree(cache_resources,cache_info);
  if (cache_info->debug != MagickFalse)
    {
      char
        message[MaxTextExtent];

      (void) FormatMagickString(message,MaxTextExtent,"destroy %s",
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
  cache_info->signature=(~MagickSignature);
  if (cache_info->disk_semaphore != (SemaphoreInfo *) NULL)
    DestroySemaphoreInfo(&cache_info->disk_semaphore);
  if (cache_info->semaphore != (SemaphoreInfo *) NULL)
    DestroySemaphoreInfo(&cache_info->semaphore);
  cache_info=(CacheInfo *) RelinquishAlignedMemory(cache_info);
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
%        const unsigned long number_threads)
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
  nexus_info->cache=(PixelPacket *) NULL;
  nexus_info->pixels=(PixelPacket *) NULL;
  nexus_info->indexes=(IndexPacket *) NULL;
  nexus_info->length=0;
  nexus_info->mapped=MagickFalse;
}

MagickExport NexusInfo **DestroyPixelCacheNexus(NexusInfo **nexus_info,
  const unsigned long number_threads)
{
  register long
    i;

  assert(nexus_info != (NexusInfo **) NULL);
  for (i=0; i < (long) number_threads; i++)
  {
    if (nexus_info[i]->cache != (PixelPacket *) NULL)
      RelinquishCacheNexusPixels(nexus_info[i]);
    nexus_info[i]->signature=(~MagickSignature);
    nexus_info[i]=(NexusInfo *) RelinquishAlignedMemory(nexus_info[i]);
  }
  nexus_info=(NexusInfo **) RelinquishAlignedMemory(nexus_info);
  return(nexus_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y P i x e l C a c h e R e s o u r c e s                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyPixelCacheResources() destroys the cache resources.
%
%  The format of the DestroyPixelCacheResources() method is:
%
%      DestroyPixelCacheResources(void)
%
*/
MagickExport void DestroyPixelCacheResources(void)
{
  AcquireSemaphoreInfo(&cache_semaphore);
  if (cache_resources != (SplayTreeInfo *) NULL)
    cache_resources=DestroySplayTree(cache_resources);
  instantiate_cache=MagickFalse;
  RelinquishSemaphoreInfo(cache_semaphore);
  DestroySemaphoreInfo(&cache_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t A u t h e n t i c I n d e x e s F r o m C a c h e                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticIndexesFromCache() returns the indexes associated with the last
%  call to QueueAuthenticPixelsCache() or GetAuthenticPixelsCache().
%
%  The format of the GetAuthenticIndexesFromCache() method is:
%
%      IndexPacket *GetAuthenticIndexesFromCache(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
static IndexPacket *GetAuthenticIndexesFromCache(const Image *image)
{
  CacheInfo
    *cache_info;

  IndexPacket
    *indexes;

  long
    id;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  id=GetOpenMPThreadId();
  assert(id < (long) cache_info->number_threads);
  indexes=GetPixelCacheNexusIndexes(image->cache,cache_info->nexus_info[id]);
  return(indexes);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t A u t h e n t i c I n d e x Q u e u e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticIndexQueue() returns the authentic black channel or the colormap
%  indexes associated with the last call to QueueAuthenticPixels() or
%  GetVirtualPixels().  NULL is returned if the black channel or colormap
%  indexes are not available.
%
%  The format of the GetAuthenticIndexQueue() method is:
%
%      IndexPacket *GetAuthenticIndexQueue(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport IndexPacket *GetAuthenticIndexQueue(const Image *image)
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
  if (cache_info->methods.get_authentic_indexes_from_handler ==
       (GetAuthenticIndexesFromHandler) NULL)
    return((IndexPacket *) NULL);
  return(cache_info->methods.get_authentic_indexes_from_handler(image));
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
%      PixelPacket *GetAuthenticPixelCacheNexus(Image *image,const long x,
%        const long y,const unsigned long columns,const unsigned long rows,
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

static inline MagickBooleanType IsNexusInCore(const CacheInfo *cache_info,
  NexusInfo *nexus_info)
{
  MagickOffsetType
    offset;

  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  if (nexus_info->pixels != (cache_info->pixels+offset))
    return(MagickFalse);
  return(MagickTrue);
}

MagickExport PixelPacket *GetAuthenticPixelCacheNexus(Image *image,const long x,
  const long y,const unsigned long columns,const unsigned long rows,
  NexusInfo *nexus_info,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  PixelPacket
    *pixels;

  /*
    Transfer pixels from the cache.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  pixels=QueueAuthenticNexus(image,x,y,columns,rows,nexus_info,exception);
  if (pixels == (PixelPacket *) NULL)
    return((PixelPacket *) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (IsNexusInCore(cache_info,nexus_info) != MagickFalse)
    return(pixels);
  if (ReadPixelCachePixels(cache_info,nexus_info,exception) == MagickFalse)
    return((PixelPacket *) NULL);
  if (cache_info->active_index_channel != MagickFalse)
    if (ReadPixelCacheIndexes(cache_info,nexus_info,exception) == MagickFalse)
      return((PixelPacket *) NULL);
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
%      PixelPacket *GetAuthenticPixelsFromCache(const Image image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
static PixelPacket *GetAuthenticPixelsFromCache(const Image *image)
{
  CacheInfo
    *cache_info;

  long
    id;

  PixelPacket
    *pixels;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  id=GetOpenMPThreadId();
  assert(id < (long) cache_info->number_threads);
  pixels=GetPixelCacheNexusPixels(image->cache,cache_info->nexus_info[id]);
  return(pixels);
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
%  GetAuthenticPixelQueue() returns the authentic pixels associated with the
%  last call to QueueAuthenticPixels() or GetAuthenticPixels().
%
%  The format of the GetAuthenticPixelQueue() method is:
%
%      PixelPacket *GetAuthenticPixelQueue(const Image image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport PixelPacket *GetAuthenticPixelQueue(const Image *image)
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
  if (cache_info->methods.get_authentic_pixels_from_handler ==
      (GetAuthenticPixelsFromHandler) NULL)
    return((PixelPacket *) NULL);
  return(cache_info->methods.get_authentic_pixels_from_handler(image));
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
%  region is successfully accessed, a pointer to a PixelPacket array
%  representing the region is returned, otherwise NULL is returned.
%
%  The returned pointer may point to a temporary working copy of the pixels
%  or it may point to the original pixels in memory. Performance is maximized
%  if the selected region is part of one row, or one or more full rows, since
%  then there is opportunity to access the pixels in-place (without a copy)
%  if the image is in RAM, or in a memory-mapped file. The returned pointer
%  should *never* be deallocated by the user.
%
%  Pixels accessed via the returned pointer represent a simple array of type
%  PixelPacket. If the image type is CMYK or if the storage class is
%  PseduoClass, call GetAuthenticIndexQueue() after invoking
%  GetAuthenticPixels() to obtain the black color component or colormap indexes
%  (of type IndexPacket) corresponding to the region.  Once the PixelPacket
%  (and/or IndexPacket) array has been updated, the changes must be saved back
%  to the underlying image using SyncAuthenticPixels() or they may be lost.
%
%  The format of the GetAuthenticPixels() method is:
%
%      PixelPacket *GetAuthenticPixels(Image *image,const long x,const long y,
%        const unsigned long columns,const unsigned long rows,
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
MagickExport PixelPacket *GetAuthenticPixels(Image *image,const long x,
  const long y,const unsigned long columns,const unsigned long rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  PixelPacket
    *pixels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.get_authentic_pixels_handler ==
      (GetAuthenticPixelsHandler) NULL)
    return((PixelPacket *) NULL);
  pixels=cache_info->methods.get_authentic_pixels_handler(image,x,y,columns,
    rows,exception);
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
%      PixelPacket *GetAuthenticPixelsCache(Image *image,const long x,
%        const long y,const unsigned long columns,const unsigned long rows,
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
static PixelPacket *GetAuthenticPixelsCache(Image *image,const long x,
  const long y,const unsigned long columns,const unsigned long rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  long
    id;

  PixelPacket
    *pixels;

  cache_info=(CacheInfo *) GetImagePixelCache(image,MagickTrue,exception);
  if (cache_info == (Cache) NULL)
    return((PixelPacket *) NULL);
  id=GetOpenMPThreadId();
  assert(id < (long) cache_info->number_threads);
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
%  GetImageExtent() returns the extent of the pixels associated with the
%  last call to QueueAuthenticPixels() or GetAuthenticPixels().
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

  long
    id;

  MagickSizeType
    extent;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  id=GetOpenMPThreadId();
  assert(id < (long) cache_info->number_threads);
  extent=GetPixelCacheNexusExtent(image->cache,cache_info->nexus_info[id]);
  return(extent);
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
      (cache_info->nexus_info == (NexusInfo **) NULL) ||
      (cache_info->number_threads < GetOpenMPMaximumThreads()))
    return(MagickFalse);
  return(MagickTrue);
}

MagickExport Cache GetImagePixelCache(Image *image,
  const MagickBooleanType clone,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  MagickSizeType
    time_limit;

  MagickBooleanType
    destroy,
    status;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=MagickTrue;
  (void) LockSemaphoreInfo(image->semaphore);
  time_limit=GetMagickResourceLimit(TimeResource);
  if (cache_timer == 0)
    cache_timer=time((time_t *) NULL);
  if ((time_limit != MagickResourceInfinity) &&
      ((MagickSizeType) (time((time_t *) NULL)-cache_timer) >= time_limit))
    ThrowFatalException(ResourceLimitFatalError,"TimeLimitExceeded");
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  destroy=MagickFalse;
  (void) LockSemaphoreInfo(cache_info->semaphore);
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
      clone_image.cache=ClonePixelCache(cache_info);
      clone_info=(CacheInfo *) clone_image.cache;
      status=ClonePixelCacheNexus(cache_info,clone_info,exception);
      if (status != MagickFalse)
        {
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
        }
    }
  (void) UnlockSemaphoreInfo(cache_info->semaphore);
  if (destroy != MagickFalse)
    DestroyPixelCache(cache_info);
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
  (void) UnlockSemaphoreInfo(image->semaphore);
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
%      MagickBooleanType GetOneAuthenticPixel(const Image image,const long x,
%        const long y,PixelPacket *pixel,ExceptionInfo *exception)
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
MagickExport MagickBooleanType GetOneAuthenticPixel(Image *image,const long x,
  const long y,PixelPacket *pixel,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  GetOneAuthenticPixelFromHandler
    get_one_authentic_pixel_from_handler;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  *pixel=image->background_color;
  get_one_authentic_pixel_from_handler=
    cache_info->methods.get_one_authentic_pixel_from_handler;
  if (get_one_authentic_pixel_from_handler ==
      (GetOneAuthenticPixelFromHandler) NULL)
    return(MagickFalse);
  status=cache_info->methods.get_one_authentic_pixel_from_handler(image,x,y,
    pixel,exception);
  return(status);
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
%        const long x,const long y,PixelPacket *pixel,ExceptionInfo *exception)
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
  const long x,const long y,PixelPacket *pixel,ExceptionInfo *exception)
{
  PixelPacket
    *pixels;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *pixel=image->background_color;
  pixels=GetAuthenticPixelsCache(image,x,y,1UL,1UL,exception);
  if (pixels == (PixelPacket *) NULL)
    return(MagickFalse);
  *pixel=(*pixels);
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
%        const long x,const long y,MagickPixelPacket *pixel,
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
  const long x,const long y,MagickPixelPacket *pixel,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  GetMagickPixelPacket(image,pixel);
  p=GetVirtualPixelCache(image,GetPixelCacheVirtualMethod(image),x,y,1,1,
    exception);
  if (p == (const PixelPacket *) NULL)
    return(MagickFalse);
  indexes=GetVirtualIndexQueue(image);
  SetMagickPixelPacket(image,p,indexes,pixel);
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
%        const VirtualPixelMethod virtual_pixel_method,const long x,
%        const long y,Pixelpacket *pixel,ExceptionInfo exception)
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
  const VirtualPixelMethod virtual_pixel_method,const long x,const long y,
  PixelPacket *pixel,ExceptionInfo *exception)
{
  GetOneVirtualPixelFromHandler
    get_one_virtual_pixel_from_handler;

  CacheInfo
    *cache_info;

  MagickBooleanType
    status;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  *pixel=image->background_color;
  get_one_virtual_pixel_from_handler=
    cache_info->methods.get_one_virtual_pixel_from_handler;
  if (get_one_virtual_pixel_from_handler ==
      (GetOneVirtualPixelFromHandler) NULL)
    return(MagickFalse);
  status=get_one_virtual_pixel_from_handler(image,virtual_pixel_method,x,y,
    pixel,exception);
  return(status);
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
%      MagickBooleanType GetOneVirtualPixel(const Image image,const long x,
%        const long y,PixelPacket *pixel,ExceptionInfo exception)
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
  const long x,const long y,PixelPacket *pixel,ExceptionInfo *exception)
{
  GetOneVirtualPixelFromHandler
    get_one_virtual_pixel_from_handler;

  CacheInfo
    *cache_info;

  MagickBooleanType
    status;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  *pixel=image->background_color;
  get_one_virtual_pixel_from_handler=
    cache_info->methods.get_one_virtual_pixel_from_handler;
  if (get_one_virtual_pixel_from_handler ==
      (GetOneVirtualPixelFromHandler) NULL)
    return(MagickFalse);
  status=get_one_virtual_pixel_from_handler(image,GetPixelCacheVirtualMethod(
    image),x,y,pixel,exception);
  return(status);
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
%        const VirtualPixelPacket method,const long x,const long y,
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
  const VirtualPixelMethod virtual_pixel_method,const long x,const long y,
  PixelPacket *pixel,ExceptionInfo *exception)
{
  const PixelPacket
    *pixels;

  *pixel=image->background_color;
  pixels=GetVirtualPixelCache(image,virtual_pixel_method,x,y,1UL,1UL,exception);
  if (pixels == (const PixelPacket *) NULL)
    return(MagickFalse);
  *pixel=(*pixels);
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
  cache_methods->get_virtual_indexes_from_handler=GetVirtualIndexesFromCache;
  cache_methods->get_one_virtual_pixel_from_handler=GetOneVirtualPixelFromCache;
  cache_methods->get_authentic_pixels_handler=GetAuthenticPixelsCache;
  cache_methods->get_authentic_indexes_from_handler=
    GetAuthenticIndexesFromCache;
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
%  GetPixelCacheNexusExtent() returns the extent of the pixels associated with
%  the last call to SetPixelCacheNexusPixels() or GetPixelCacheNexusPixels().
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

  if (cache == (Cache) NULL)
    return(0);
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
+   G e t P i x e l C a c h e N e x u s I n d e x e s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelCacheNexusIndexes() returns the indexes associated with the
%  specified cache nexus.
%
%  The format of the GetPixelCacheNexusIndexes() method is:
%
%      IndexPacket *GetPixelCacheNexusIndexes(const Cache cache,
%        NexusInfo *nexus_info)
%
%  A description of each parameter follows:
%
%    o cache: the pixel cache.
%
%    o nexus_info: the cache nexus to return the colormap indexes.
%
*/
MagickExport IndexPacket *GetPixelCacheNexusIndexes(const Cache cache,
  NexusInfo *nexus_info)
{
  CacheInfo
    *cache_info;

  if (cache == (Cache) NULL)
    return((IndexPacket *) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->storage_class == UndefinedClass)
    return((IndexPacket *) NULL);
  return(nexus_info->indexes);
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
%      PixelPacket *GetPixelCacheNexusPixels(const Cache cache,
%        NexusInfo *nexus_info)
%
%  A description of each parameter follows:
%
%    o cache: the pixel cache.
%
%    o nexus_info: the cache nexus to return the pixels.
%
*/
MagickExport PixelPacket *GetPixelCacheNexusPixels(const Cache cache,
  NexusInfo *nexus_info)
{
  CacheInfo
    *cache_info;

  if (cache == (Cache) NULL)
    return((PixelPacket *) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_info->filename);
  if (cache_info->storage_class == UndefinedClass)
    return((PixelPacket *) NULL);
  return(nexus_info->pixels);
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
%      void GetPixelCacheTileSize(const Image *image,unsigned long *width,
%        unsigned long *height)
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
MagickExport void GetPixelCacheTileSize(const Image *image,unsigned long *width,
  unsigned long *height)
{
  CacheInfo
    *cache_info;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  *width=2048UL/sizeof(PixelPacket);
  if (GetPixelCacheType(image) == DiskCache)
    *width=8192UL/sizeof(PixelPacket);
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
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
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
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
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
+   G e t V i r t u a l I n d e x e s F r o m C a c h e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualIndexesFromCache() returns the indexes associated with the last
%  call to QueueAuthenticPixelsCache() or GetVirtualPixelCache().
%
%  The format of the GetVirtualIndexesFromCache() method is:
%
%      IndexPacket *GetVirtualIndexesFromCache(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
static const IndexPacket *GetVirtualIndexesFromCache(const Image *image)
{
  CacheInfo
    *cache_info;

  const IndexPacket
    *indexes;

  long
    id;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  id=GetOpenMPThreadId();
  assert(id < (long) cache_info->number_threads);
  indexes=GetVirtualIndexesFromNexus(image->cache,cache_info->nexus_info[id]);
  return(indexes);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t V i r t u a l I n d e x e s F r o m N e x u s                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualIndexesFromNexus() returns the indexes associated with the
%  specified cache nexus.
%
%  The format of the GetVirtualIndexesFromNexus() method is:
%
%      const IndexPacket *GetVirtualIndexesFromNexus(const Cache cache,
%        NexusInfo *nexus_info)
%
%  A description of each parameter follows:
%
%    o cache: the pixel cache.
%
%    o nexus_info: the cache nexus to return the colormap indexes.
%
*/
MagickExport const IndexPacket *GetVirtualIndexesFromNexus(const Cache cache,
  NexusInfo *nexus_info)
{
  CacheInfo
    *cache_info;

  if (cache == (Cache) NULL)
    return((IndexPacket *) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->storage_class == UndefinedClass)
    return((IndexPacket *) NULL);
  return(nexus_info->indexes);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t V i r t u a l I n d e x Q u e u e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualIndexQueue() returns the virtual black channel or the
%  colormap indexes associated with the last call to QueueAuthenticPixels() or
%  GetVirtualPixels().  NULL is returned if the black channel or colormap
%  indexes are not available.
%
%  The format of the GetVirtualIndexQueue() method is:
%
%      const IndexPacket *GetVirtualIndexQueue(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport const IndexPacket *GetVirtualIndexQueue(const Image *image)
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
  if (cache_info->methods.get_virtual_indexes_from_handler ==
      (GetVirtualIndexesFromHandler) NULL)
    return((IndexPacket *) NULL);
  return(cache_info->methods.get_virtual_indexes_from_handler(image));
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
%      PixelPacket *GetVirtualPixelsFromNexus(const Image *image,
%        const VirtualPixelMethod method,const long x,const long y,
%        const unsigned long columns,const unsigned long rows,
%        NexusInfo *nexus_info,ExceptionInfo *exception)
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

static long
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

static inline long DitherX(const unsigned long columns,const long x)
{
  long
    index;

  index=x+DitherMatrix[x & 0x07]-32L;
  if (index < 0L)
    return(0L);
  if (index >= (long) columns)
    return((long) columns-1L);
  return(index);
}

static inline long DitherY(const unsigned long rows,const long y)
{
  long
    index;

  index=y+DitherMatrix[y & 0x07]-32L;
  if (index < 0L)
    return(0L);
  if (index >= (long) rows)
    return((long) rows-1L);
  return(index);
}

static inline long EdgeX(const unsigned long columns,const long x)
{
  if (x < 0L)
    return(0L);
  if (x >= (long) columns)
    return((long) columns-1L);
  return(x);
}

static inline long EdgeY(const unsigned long rows,const long y)
{
  if (y < 0L)
    return(0L);
  if (y >= (long) rows)
    return((long) rows-1L);
  return(y);
}

static inline long RandomX(const unsigned long columns,RandomInfo *random_info)
{
  return((long) (columns*GetPseudoRandomValue(random_info)));
}

static inline long RandomY(const unsigned long rows,RandomInfo *random_info)
{
  return((long) (rows*GetPseudoRandomValue(random_info)));
}

/*
  VirtualPixelModulo() computes the remainder of dividing offset by extent.  It
  returns not only the quotient (tile the offset falls in) but also the positive
  remainer within that tile such that 0 <= remainder < extent.  This method is
  essentially a ldiv() using a floored modulo division rather than the normal
  default truncated modulo division.
*/
static inline MagickModulo VirtualPixelModulo(const long offset,
  const unsigned long extent)
{
  MagickModulo
    modulo;

  modulo.quotient=offset/(long) extent;
  if (offset < 0L)
    modulo.quotient--;
  modulo.remainder=offset-modulo.quotient*(long) extent;
  return(modulo);
}

MagickExport const PixelPacket *GetVirtualPixelsFromNexus(const Image *image,
  const VirtualPixelMethod virtual_pixel_method,const long x,const long y,
  const unsigned long columns,const unsigned long rows,NexusInfo *nexus_info,
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

  PixelPacket
    *pixels,
    virtual_pixel;

  RectangleInfo
    region;

  register const IndexPacket
    *__restrict nexus_indexes;

  register const PixelPacket
    *__restrict p;

  register IndexPacket
    *__restrict indexes;

  register long
    u,
    v;

  register PixelPacket
    *__restrict q;

  /*
    Acquire pixels.
  */
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  if (cache_info->type == UndefinedCache)
    return((const PixelPacket *) NULL);
  region.x=x;
  region.y=y;
  region.width=columns;
  region.height=rows;
  pixels=SetPixelCacheNexusPixels(image,&region,nexus_info,exception);
  if (pixels == (PixelPacket *) NULL)
    return((const PixelPacket *) NULL);
  offset=(MagickOffsetType) region.y*cache_info->columns+region.x;
  length=(MagickSizeType) (region.height-1)*cache_info->columns+region.width-1;
  number_pixels=(MagickSizeType) cache_info->columns*cache_info->rows;
  if ((offset >= 0) && (((MagickSizeType) offset+length) < number_pixels))
    if ((x >= 0) && ((long) (x+columns) <= (long) cache_info->columns) &&
        (y >= 0) && ((long) (y+rows) <= (long) cache_info->rows))
      {
        MagickBooleanType
          status;

        /*
          Pixel request is inside cache extents.
        */
        if (IsNexusInCore(cache_info,nexus_info) != MagickFalse)
          return(pixels);
        status=ReadPixelCachePixels(cache_info,nexus_info,exception);
        if (status == MagickFalse)
          return((const PixelPacket *) NULL);
        if ((cache_info->storage_class == PseudoClass) ||
            (cache_info->colorspace == CMYKColorspace))
          {
            status=ReadPixelCacheIndexes(cache_info,nexus_info,exception);
            if (status == MagickFalse)
              return((const PixelPacket *) NULL);
          }
        return(pixels);
      }
  /*
    Pixel request is outside cache extents.
  */
  q=pixels;
  indexes=GetPixelCacheNexusIndexes(cache_info,nexus_info);
  virtual_nexus=AcquirePixelCacheNexus(1);
  if (virtual_nexus == (NexusInfo **) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "UnableToGetCacheNexus","`%s'",image->filename);
      return((const PixelPacket *) NULL);
    }
  switch (virtual_pixel_method)
  {
    case BlackVirtualPixelMethod:
    {
      virtual_pixel.red=0;
      virtual_pixel.green=0;
      virtual_pixel.blue=0;
      virtual_pixel.opacity=OpaqueOpacity;
      break;
    }
    case GrayVirtualPixelMethod:
    {
      virtual_pixel.red=(Quantum) QuantumRange/2;
      virtual_pixel.green=(Quantum) QuantumRange/2;
      virtual_pixel.blue=(Quantum) QuantumRange/2;
      virtual_pixel.opacity=(Quantum) OpaqueOpacity;
      break;
    }
    case TransparentVirtualPixelMethod:
    {
      virtual_pixel.red=(Quantum) 0;
      virtual_pixel.green=(Quantum) 0;
      virtual_pixel.blue=(Quantum) 0;
      virtual_pixel.opacity=(Quantum) TransparentOpacity;
      break;
    }
    case MaskVirtualPixelMethod:
    case WhiteVirtualPixelMethod:
    {
      virtual_pixel.red=(Quantum) QuantumRange;
      virtual_pixel.green=(Quantum) QuantumRange;
      virtual_pixel.blue=(Quantum) QuantumRange;
      virtual_pixel.opacity=OpaqueOpacity;
      break;
    }
    default:
    {
      virtual_pixel=image->background_color;
      break;
    }
  }
  for (v=0; v < (long) rows; v++)
  {
    for (u=0; u < (long) columns; u+=length)
    {
      length=(MagickSizeType) MagickMin(cache_info->columns-(x+u),columns-u);
      if ((((x+u) < 0) || ((x+u) >= (long) cache_info->columns)) ||
          (((y+v) < 0) || ((y+v) >= (long) cache_info->rows)) || (length == 0))
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
            case BackgroundVirtualPixelMethod:
            case ConstantVirtualPixelMethod:
            case BlackVirtualPixelMethod:
            case GrayVirtualPixelMethod:
            case TransparentVirtualPixelMethod:
            case MaskVirtualPixelMethod:
            case WhiteVirtualPixelMethod:
            {
              p=(&virtual_pixel);
              break;
            }
            case EdgeVirtualPixelMethod:
            default:
            {
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                EdgeX(cache_info->columns,x+u),EdgeY(cache_info->rows,y+v),
                1UL,1UL,virtual_nexus[0],exception);
              break;
            }
            case RandomVirtualPixelMethod:
            {
              if (cache_info->random_info == (RandomInfo *) NULL)
                cache_info->random_info=AcquireRandomInfo();
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                RandomX(cache_info->columns,cache_info->random_info),
                RandomY(cache_info->rows,cache_info->random_info),1UL,1UL,
                virtual_nexus[0],exception);
              break;
            }
            case DitherVirtualPixelMethod:
            {
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                DitherX(cache_info->columns,x+u),DitherY(cache_info->rows,y+v),
                1UL,1UL,virtual_nexus[0],exception);
              break;
            }
            case TileVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,virtual_nexus[0],
                exception);
              break;
            }
            case MirrorVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              if ((x_modulo.quotient & 0x01) == 1L)
                x_modulo.remainder=(long) cache_info->columns-
                  x_modulo.remainder-1L;
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              if ((y_modulo.quotient & 0x01) == 1L)
                y_modulo.remainder=(long) cache_info->rows-
                  y_modulo.remainder-1L;
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,virtual_nexus[0],
                exception);
              break;
            }
            case CheckerTileVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              if (((x_modulo.quotient ^ y_modulo.quotient) & 0x01) != 0L)
                {
                  p=(&virtual_pixel);
                  break;
                }
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,virtual_nexus[0],
                exception);
              break;
            }
            case HorizontalTileVirtualPixelMethod:
            {
              if (((y+v) < 0) || ((y+v) >= (long) cache_info->rows))
                {
                  p=(&virtual_pixel);
                  break;
                }
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,virtual_nexus[0],
                exception);
              break;
            }
            case VerticalTileVirtualPixelMethod:
            {
              if (((x+u) < 0) || ((x+u) >= (long) cache_info->columns))
                {
                  p=(&virtual_pixel);
                  break;
                }
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,virtual_nexus[0],
                exception);
              break;
            }
            case HorizontalTileEdgeVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,EdgeY(cache_info->rows,y+v),1UL,1UL,
                virtual_nexus[0],exception);
              break;
            }
            case VerticalTileEdgeVirtualPixelMethod:
            {
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                EdgeX(cache_info->columns,x+u),y_modulo.remainder,1UL,1UL,
                virtual_nexus[0],exception);
              break;
            }
          }
          if (p == (const PixelPacket *) NULL)
            break;
          *q++=(*p);
          if (indexes != (IndexPacket *) NULL)
            {
              nexus_indexes=GetVirtualIndexesFromNexus(cache_info,
                virtual_nexus[0]);
              if (nexus_indexes != (const IndexPacket *) NULL)
                *indexes++=(*nexus_indexes);
            }
          continue;
        }
      /*
        Transfer a run of pixels.
      */
      p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,x+u,y+v,
        (unsigned long) length,1UL,virtual_nexus[0],exception);
      if (p == (const PixelPacket *) NULL)
        break;
      (void) CopyMagickMemory(q,p,(size_t) length*sizeof(*p));
      q+=length;
      if (indexes != (IndexPacket *) NULL)
        {
          nexus_indexes=GetVirtualIndexesFromNexus(cache_info,virtual_nexus[0]);
          if (nexus_indexes != (const IndexPacket *) NULL)
            {
              (void) CopyMagickMemory(indexes,nexus_indexes,(size_t) length*
                sizeof(*nexus_indexes));
              indexes+=length;
            }
        }
    }
  }
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
%      const PixelPacket *GetVirtualPixelCache(const Image *image,
%        const VirtualPixelMethod virtual_pixel_method,const long x,
%        const long y,const unsigned long columns,const unsigned long rows,
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
static const PixelPacket *GetVirtualPixelCache(const Image *image,
  const VirtualPixelMethod virtual_pixel_method,const long x,const long y,
  const unsigned long columns,const unsigned long rows,ExceptionInfo *exception)
{
  CacheInfo
   *cache_info;

  const PixelPacket
    *pixels;

  long
    id;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  id=GetOpenMPThreadId();
  assert(id < (long) cache_info->number_threads);
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
%  GetVirtualPixelQueue() returns the virtual pixels associated with the
%  last call to QueueAuthenticPixels() or GetVirtualPixels().
%
%  The format of the GetVirtualPixelQueue() method is:
%
%      const PixelPacket *GetVirtualPixelQueue(const Image image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport const PixelPacket *GetVirtualPixelQueue(const Image *image)
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
  if (cache_info->methods.get_virtual_pixels_handler ==
      (GetVirtualPixelsHandler) NULL)
    return((PixelPacket *) NULL);
  return(cache_info->methods.get_virtual_pixels_handler(image));
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
%  NULL is returned. The returned pointer may point to a temporary working
%  copy of the pixels or it may point to the original pixels in memory.
%  Performance is maximized if the selected region is part of one row, or one
%  or more full rows, since there is opportunity to access the pixels in-place
%  (without a copy) if the image is in RAM, or in a memory-mapped file.  The
%  returned pointer should *never* be deallocated by the user.
%
%  Pixels accessed via the returned pointer represent a simple array of type
%  PixelPacket.  If the image type is CMYK or the storage class is PseudoClass,
%  call GetAuthenticIndexQueue() after invoking GetAuthenticPixels() to access
%  the black color component or to obtain the colormap indexes (of type
%  IndexPacket) corresponding to the region.
%
%  If you plan to modify the pixels, use GetAuthenticPixels() instead.
%
%  Note, the GetVirtualPixels() and GetAuthenticPixels() methods are not thread-
%  safe.  In a threaded environment, use GetCacheViewVirtualPixels() or
%  GetCacheViewAuthenticPixels() instead.
%
%  The format of the GetVirtualPixels() method is:
%
%      const PixelPacket *GetVirtualPixels(const Image *image,const long x,
%        const long y,const unsigned long columns,const unsigned long rows,
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
MagickExport const PixelPacket *GetVirtualPixels(const Image *image,
  const long x,const long y,const unsigned long columns,
  const unsigned long rows,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  const PixelPacket
    *pixels;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.get_virtual_pixel_handler ==
      (GetVirtualPixelHandler) NULL)
    return((const PixelPacket *) NULL);
  pixels=cache_info->methods.get_virtual_pixel_handler(image,
    GetPixelCacheVirtualMethod(image),x,y,columns,rows,exception);
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
%  GetVirtualPixelsCache() returns the pixels associated with the last call
%  to QueueAuthenticPixelsCache() or GetVirtualPixelCache().
%
%  The format of the GetVirtualPixelsCache() method is:
%
%      PixelPacket *GetVirtualPixelsCache(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
static const PixelPacket *GetVirtualPixelsCache(const Image *image)
{
  CacheInfo
    *cache_info;

  const PixelPacket
    *pixels;

  long
    id;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  id=GetOpenMPThreadId();
  assert(id < (long) cache_info->number_threads);
  pixels=GetVirtualPixelsNexus(image->cache,cache_info->nexus_info[id]);
  return(pixels);
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
%      const IndexPacket *GetVirtualPixelsNexus(const Cache cache,
%        NexusInfo *nexus_info)
%
%  A description of each parameter follows:
%
%    o cache: the pixel cache.
%
%    o nexus_info: the cache nexus to return the colormap pixels.
%
*/
MagickExport const PixelPacket *GetVirtualPixelsNexus(const Cache cache,
  NexusInfo *nexus_info)
{
  CacheInfo
    *cache_info;

  if (cache == (Cache) NULL)
    return((PixelPacket *) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->storage_class == UndefinedClass)
    return((PixelPacket *) NULL);
  return((const PixelPacket *) nexus_info->pixels);
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

static inline void MagickPixelCompositeMask(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    gamma;

  if (alpha == TransparentOpacity)
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
    composite->index=gamma*MagickOver_(p->index,alpha,q->index,beta);
}

static MagickBooleanType MaskPixelCacheNexus(Image *image,NexusInfo *nexus_info,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  MagickPixelPacket
    alpha,
    beta;

  MagickSizeType
    number_pixels;

  NexusInfo
    **clip_nexus,
    **image_nexus;

  register const PixelPacket
    *__restrict r;

  register IndexPacket
    *__restrict nexus_indexes,
    *__restrict indexes;

  register long
    i;

  register PixelPacket
    *__restrict p,
    *__restrict q;

  /*
    Apply clip mask.
  */
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->mask == (Image *) NULL)
    return(MagickFalse);
  cache_info=(CacheInfo *) GetImagePixelCache(image,MagickTrue,exception);
  if (cache_info == (Cache) NULL)
    return(MagickFalse);
  image_nexus=AcquirePixelCacheNexus(1);
  clip_nexus=AcquirePixelCacheNexus(1);
  if ((image_nexus == (NexusInfo **) NULL) ||
      (clip_nexus == (NexusInfo **) NULL))
    ThrowBinaryException(CacheError,"UnableToGetCacheNexus",image->filename);
  p=GetAuthenticPixelCacheNexus(image,nexus_info->region.x,nexus_info->region.y,
    nexus_info->region.width,nexus_info->region.height,image_nexus[0],
    exception);
  indexes=GetPixelCacheNexusIndexes(image->cache,image_nexus[0]);
  q=nexus_info->pixels;
  nexus_indexes=nexus_info->indexes;
  r=GetVirtualPixelsFromNexus(image->mask,MaskVirtualPixelMethod,
    nexus_info->region.x,nexus_info->region.y,nexus_info->region.width,
    nexus_info->region.height,clip_nexus[0],&image->exception);
  GetMagickPixelPacket(image,&alpha);
  GetMagickPixelPacket(image,&beta);
  number_pixels=(MagickSizeType) nexus_info->region.width*
    nexus_info->region.height;
  for (i=0; i < (long) number_pixels; i++)
  {
    if ((p == (PixelPacket *) NULL) || (r == (const PixelPacket *) NULL))
      break;
    SetMagickPixelPacket(image,p,indexes+i,&alpha);
    SetMagickPixelPacket(image,q,nexus_indexes+i,&beta);
    MagickPixelCompositeMask(&beta,(MagickRealType) PixelIntensityToQuantum(r),
      &alpha,alpha.opacity,&beta);
    q->red=RoundToQuantum(beta.red);
    q->green=RoundToQuantum(beta.green);
    q->blue=RoundToQuantum(beta.blue);
    q->opacity=RoundToQuantum(beta.opacity);
    if (cache_info->active_index_channel != MagickFalse)
      nexus_indexes[i]=indexes[i];
    p++;
    q++;
    r++;
  }
  clip_nexus=DestroyPixelCacheNexus(clip_nexus,1);
  image_nexus=DestroyPixelCacheNexus(image_nexus,1);
  if (i < (long) number_pixels)
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
%  colormap indexes, and memory mapping the cache if it is disk based.  The
%  cache nexus array is initialized as well.
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

static inline void AcquirePixelCachePixels(CacheInfo *cache_info)
{
  cache_info->mapped=MagickFalse;
  cache_info->pixels=(PixelPacket *) AcquireMagickMemory((size_t)
    cache_info->length);
  if (cache_info->pixels == (PixelPacket *) NULL)
    {
      cache_info->mapped=MagickTrue;
      cache_info->pixels=(PixelPacket *) MapBlob(-1,IOMode,0,(size_t)
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

      (void) FormatMagickSize(length,format);
      (void) FormatMagickString(message,MaxTextExtent,
        "extend %s (%s[%d], disk, %s)",cache_info->filename,
        cache_info->cache_filename,cache_info->file,format);
      (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",message);
    }
  if (length != (MagickSizeType) ((MagickOffsetType) length))
    return(MagickFalse);
  extent=(MagickOffsetType) MagickSeek(cache_info->file,0,SEEK_END);
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
  char
    format[MaxTextExtent],
    message[MaxTextExtent];

  CacheInfo
    *cache_info,
    source_info;

  MagickSizeType
    length,
    number_pixels;

  MagickStatusType
    status;

  size_t
    packet_size;

  unsigned long
    columns;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->columns == 0) || (image->rows == 0))
    ThrowBinaryException(CacheError,"NoPixelsDefinedInCache",image->filename);
  cache_info=(CacheInfo *) image->cache;
  source_info=(*cache_info);
  source_info.file=(-1);
  (void) FormatMagickString(cache_info->filename,MaxTextExtent,"%s[%ld]",
    image->filename,GetImageIndexInList(image));
  cache_info->mode=mode;
  cache_info->rows=image->rows;
  cache_info->columns=image->columns;
  cache_info->active_index_channel=((image->storage_class == PseudoClass) ||
    (image->colorspace == CMYKColorspace)) ? MagickTrue : MagickFalse;
  number_pixels=(MagickSizeType) cache_info->columns*cache_info->rows;
  packet_size=sizeof(PixelPacket);
  if (cache_info->active_index_channel != MagickFalse)
    packet_size+=sizeof(IndexPacket);
  length=number_pixels*packet_size;
  columns=(unsigned long) (length/cache_info->rows/packet_size);
  if (cache_info->columns != columns)
    ThrowBinaryException(ResourceLimitError,"PixelCacheAllocationFailed",
      image->filename);
  cache_info->length=length;
  status=AcquireMagickResource(AreaResource,cache_info->length);
  length=number_pixels*(sizeof(PixelPacket)+sizeof(IndexPacket));
  if ((status != MagickFalse) && (length == (MagickSizeType) ((size_t) length)))
    {
      status=AcquireMagickResource(MemoryResource,cache_info->length);
      if (((cache_info->type == UndefinedCache) && (status != MagickFalse)) ||
          (cache_info->type == MemoryCache))
        {
          AcquirePixelCachePixels(cache_info);
          if (cache_info->pixels == (PixelPacket *) NULL)
            cache_info->pixels=source_info.pixels;
          else
            {
              /*
                Create memory pixel cache.
              */
              if (image->debug != MagickFalse)
                {
                  (void) FormatMagickSize(cache_info->length,format);
                  (void) FormatMagickString(message,MaxTextExtent,
                    "open %s (%s memory, %lux%lu %s)",cache_info->filename,
                    cache_info->mapped != MagickFalse ? "anonymous" : "heap",
                    cache_info->columns,cache_info->rows,format);
                  (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",
                    message);
                }
              cache_info->storage_class=image->storage_class;
              cache_info->colorspace=image->colorspace;
              cache_info->type=MemoryCache;
              cache_info->indexes=(IndexPacket *) NULL;
              if (cache_info->active_index_channel != MagickFalse)
                cache_info->indexes=(IndexPacket *) (cache_info->pixels+
                  number_pixels);
              if (source_info.storage_class != UndefinedClass)
                {
                  status|=ClonePixelCachePixels(cache_info,&source_info,
                    exception);
                  RelinquishPixelCachePixels(&source_info);
                }
              return(MagickTrue);
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
  cache_info->storage_class=image->storage_class;
  cache_info->colorspace=image->colorspace;
  length=number_pixels*(sizeof(PixelPacket)+sizeof(IndexPacket));
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
          cache_info->pixels=(PixelPacket *) MapBlob(cache_info->file,mode,
            cache_info->offset,(size_t) cache_info->length);
          if (cache_info->pixels == (PixelPacket *) NULL)
            {
              cache_info->pixels=source_info.pixels;
              cache_info->type=DiskCache;
            }
          else
            {
              /*
                Create file-backed memory-mapped pixel cache.
              */
              (void) ClosePixelCacheOnDisk(cache_info);
              cache_info->type=MapCache;
              cache_info->mapped=MagickTrue;
              cache_info->indexes=(IndexPacket *) NULL;
              if (cache_info->active_index_channel != MagickFalse)
                cache_info->indexes=(IndexPacket *) (cache_info->pixels+
                  number_pixels);
              if ((source_info.type != UndefinedCache) && (mode != ReadMode))
                {
                  status=ClonePixelCachePixels(cache_info,&source_info,
                    exception);
                  RelinquishPixelCachePixels(&source_info);
                }
              if (image->debug != MagickFalse)
                {
                  (void) FormatMagickSize(cache_info->length,format);
                  (void) FormatMagickString(message,MaxTextExtent,
                    "open %s (%s[%d], memory-mapped, %lux%lu %s)",
                    cache_info->filename,cache_info->cache_filename,
                    cache_info->file,cache_info->columns,cache_info->rows,
                    format);
                  (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",
                    message);
                }
              return(MagickTrue);
            }
        }
      RelinquishMagickResource(MapResource,cache_info->length);
    }
  if ((source_info.type != UndefinedCache) && (mode != ReadMode))
    {
      status=ClonePixelCachePixels(cache_info,&source_info,exception);
      RelinquishPixelCachePixels(&source_info);
    }
  if (image->debug != MagickFalse)
    {
      (void) FormatMagickSize(cache_info->length,format);
      (void) FormatMagickString(message,MaxTextExtent,
        "open %s (%s[%d], disk, %lux%lu %s)",cache_info->filename,
        cache_info->cache_filename,cache_info->file,cache_info->columns,
        cache_info->rows,format);
      (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",message);
    }
  return(MagickTrue);
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
%    o attach: A value other than zero initializes the persistent pixel
%      cache.
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

  long
    pagesize;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (void *) NULL);
  assert(filename != (const char *) NULL);
  assert(offset != (MagickOffsetType *) NULL);
  pagesize=(-1);
#if defined(MAGICKCORE_HAVE_SYSCONF) && defined(_SC_PAGESIZE)
  pagesize=sysconf(_SC_PAGESIZE);
#elif defined(MAGICKCORE_HAVE_GETPAGESIZE) && defined(MAGICKCORE_POSIX_SUPPORT)
  pagesize=getpagesize();
#endif
  if (pagesize <= 0)
    pagesize=4096;
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
      *offset+=cache_info->length+pagesize-(cache_info->length % pagesize);
      return(MagickTrue);
    }
  if ((cache_info->mode != ReadMode) && (cache_info->type != MemoryCache) &&
      (cache_info->reference_count == 1))
    {
      (void) LockSemaphoreInfo(cache_info->semaphore);
      if ((cache_info->mode != ReadMode) && (cache_info->type != MemoryCache) &&
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
              *offset+=cache_info->length+pagesize-(cache_info->length %
                pagesize);
              (void) UnlockSemaphoreInfo(cache_info->semaphore);
              cache_info=(CacheInfo *) ReferencePixelCache(cache_info);
              if (image->debug != MagickFalse)
                (void) LogMagickEvent(CacheEvent,GetMagickModule(),
                  "Usurp resident persistent cache");
              return(MagickTrue);
            }
        }
      (void) UnlockSemaphoreInfo(cache_info->semaphore);
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
  status=ClonePixelCacheNexus(cache_info,clone_info,exception);
  if (status != MagickFalse)
    {
      status=OpenPixelCache(image,IOMode,exception);
      if (status != MagickFalse)
       status=ClonePixelCachePixels(cache_info,clone_info,&image->exception);
    }
  *offset+=cache_info->length+pagesize-(cache_info->length % pagesize);
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
%      PixelPacket *QueueAuthenticNexus(Image *image,const long x,const long y,
%        const unsigned long columns,const unsigned long rows,
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
MagickExport PixelPacket *QueueAuthenticNexus(Image *image,const long x,
  const long y,const unsigned long columns,const unsigned long rows,
  NexusInfo *nexus_info,ExceptionInfo *exception)
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
  cache_info=(CacheInfo *) image->cache;
  if ((cache_info->columns == 0) && (cache_info->rows == 0))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "NoPixelsDefinedInCache","`%s'",image->filename);
      return((PixelPacket *) NULL);
    }
  if ((x < 0) || (y < 0) || (x >= (long) cache_info->columns) ||
      (y >= (long) cache_info->rows))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "PixelsAreNotAuthentic","`%s'",image->filename);
      return((PixelPacket *) NULL);
    }
  offset=(MagickOffsetType) y*cache_info->columns+x;
  if (offset < 0)
    return((PixelPacket *) NULL);
  number_pixels=(MagickSizeType) cache_info->columns*cache_info->rows;
  offset+=(MagickOffsetType) (rows-1)*cache_info->columns+columns-1;
  if ((MagickSizeType) offset >= number_pixels)
    return((PixelPacket *) NULL);
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
%      PixelPacket *QueueAuthenticPixelsCache(Image *image,const long x,
%        const long y,const unsigned long columns,const unsigned long rows,
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
static PixelPacket *QueueAuthenticPixelsCache(Image *image,const long x,
  const long y,const unsigned long columns,const unsigned long rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  long
    id;

  PixelPacket
    *pixels;

  cache_info=(CacheInfo *) GetImagePixelCache(image,MagickFalse,exception);
  if (cache_info == (Cache) NULL)
    return((PixelPacket *) NULL);
  id=GetOpenMPThreadId();
  assert(id < (long) cache_info->number_threads);
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
%  successfully intialized a pointer to a PixelPacket array representing the
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
%  pixels in-place (without a copy) if the image is in RAM, or in a
%  memory-mapped file. The returned pointer should *never* be deallocated
%  by the user.
%
%  Pixels accessed via the returned pointer represent a simple array of type
%  PixelPacket. If the image type is CMYK or the storage class is PseudoClass,
%  call GetAuthenticIndexQueue() after invoking GetAuthenticPixels() to obtain
%  the black color component or the colormap indexes (of type IndexPacket)
%  corresponding to the region.  Once the PixelPacket (and/or IndexPacket)
%  array has been updated, the changes must be saved back to the underlying
%  image using SyncAuthenticPixels() or they may be lost.
%
%  The format of the QueueAuthenticPixels() method is:
%
%      PixelPacket *QueueAuthenticPixels(Image *image,const long x,const long y,
%        const unsigned long columns,const unsigned long rows,
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
MagickExport PixelPacket *QueueAuthenticPixels(Image *image,const long x,
  const long y,const unsigned long columns,const unsigned long rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  PixelPacket
    *pixels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.queue_authentic_pixels_handler ==
      (QueueAuthenticPixelsHandler) NULL)
    return((PixelPacket *) NULL);
  pixels=cache_info->methods.queue_authentic_pixels_handler(image,x,y,columns,
    rows,exception);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e a d P i x e l C a c h e I n d e x e s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPixelCacheIndexes() reads colormap indexes from the specified region of
%  the pixel cache.
%
%  The format of the ReadPixelCacheIndexes() method is:
%
%      MagickBooleanType ReadPixelCacheIndexes(CacheInfo *cache_info,
%        NexusInfo *nexus_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_info: the pixel cache.
%
%    o nexus_info: the cache nexus to read the colormap indexes.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType ReadPixelCacheIndexes(CacheInfo *cache_info,
  NexusInfo *nexus_info,ExceptionInfo *exception)
{
  MagickOffsetType
    count,
    offset;

  MagickSizeType
    length,
    number_pixels;

  register IndexPacket
    *__restrict q;

  register long
    y;

  unsigned long
    rows;

  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_info->filename);
  if (cache_info->active_index_channel == MagickFalse)
    return(MagickFalse);
  if (IsNexusInCore(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*sizeof(IndexPacket);
  rows=nexus_info->region.height;
  number_pixels=length*rows;
  if ((cache_info->columns == nexus_info->region.width) &&
      (number_pixels == (MagickSizeType) ((size_t) number_pixels)))
    {
      length=number_pixels;
      rows=1UL;
    }
  q=nexus_info->indexes;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register IndexPacket
        *__restrict p;

      /*
        Read indexes from memory.
      */
      p=cache_info->indexes+offset;
      for (y=0; y < (long) rows; y++)
      {
        (void) CopyMagickMemory(q,p,(size_t) length);
        p+=cache_info->columns;
        q+=nexus_info->region.width;
      }
      break;
    }
    case DiskCache:
    {
      /*
        Read indexes from disk.
      */
      if (OpenPixelCacheOnDisk(cache_info,IOMode) == MagickFalse)
        {
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            cache_info->cache_filename);
          return(MagickFalse);
        }
      number_pixels=(MagickSizeType) cache_info->columns*cache_info->rows;
      for (y=0; y < (long) rows; y++)
      {
        count=ReadPixelCacheRegion(cache_info,cache_info->offset+number_pixels*
          sizeof(PixelPacket)+offset*sizeof(*q),length,(unsigned char *) q);
        if ((MagickSizeType) count < length)
          break;
        offset+=cache_info->columns;
        q+=nexus_info->region.width;
      }
      if (y < (long) rows)
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
      (QuantumTick(nexus_info->region.y,cache_info->rows) != MagickFalse))
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s[%lux%lu%+ld%+ld]",
      cache_info->filename,nexus_info->region.width,nexus_info->region.height,
      nexus_info->region.x,nexus_info->region.y);
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
    length,
    number_pixels;

  register long
    y;

  register PixelPacket
    *__restrict q;

  unsigned long
    rows;

  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_info->filename);
  if (IsNexusInCore(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*sizeof(PixelPacket);
  rows=nexus_info->region.height;
  number_pixels=length*rows;
  if ((cache_info->columns == nexus_info->region.width) &&
      (number_pixels == (MagickSizeType) ((size_t) number_pixels)))
    {
      length=number_pixels;
      rows=1UL;
    }
  q=nexus_info->pixels;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register PixelPacket
        *__restrict p;

      /*
        Read pixels from memory.
      */
      p=cache_info->pixels+offset;
      for (y=0; y < (long) rows; y++)
      {
        (void) CopyMagickMemory(q,p,(size_t) length);
        p+=cache_info->columns;
        q+=nexus_info->region.width;
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
      for (y=0; y < (long) rows; y++)
      {
        count=ReadPixelCacheRegion(cache_info,cache_info->offset+offset*
          sizeof(*q),length,(unsigned char *) q);
        if ((MagickSizeType) count < length)
          break;
        offset+=cache_info->columns;
        q+=nexus_info->region.width;
      }
      if (y < (long) rows)
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
      (QuantumTick(nexus_info->region.y,cache_info->rows) != MagickFalse))
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s[%lux%lu%+ld%+ld]",
      cache_info->filename,nexus_info->region.width,nexus_info->region.height,
      nexus_info->region.x,nexus_info->region.y);
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
  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_info->filename);
  (void) LockSemaphoreInfo(cache_info->semaphore);
  cache_info->reference_count++;
  (void) UnlockSemaphoreInfo(cache_info->semaphore);
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
  if (cache_methods->get_virtual_indexes_from_handler !=
      (GetVirtualIndexesFromHandler) NULL)
    cache_info->methods.get_virtual_indexes_from_handler=
      cache_methods->get_virtual_indexes_from_handler;
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
  if (cache_methods->get_authentic_indexes_from_handler !=
      (GetAuthenticIndexesFromHandler) NULL)
    cache_info->methods.get_authentic_indexes_from_handler=
      cache_methods->get_authentic_indexes_from_handler;
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
%      PixelPacket SetPixelCacheNexusPixels(const Image *image,
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
static PixelPacket *SetPixelCacheNexusPixels(const Image *image,
  const RectangleInfo *region,NexusInfo *nexus_info,ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  MagickSizeType
    length,
    number_pixels;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->type == UndefinedCache)
    return((PixelPacket *) NULL);
  nexus_info->region.width=region->width == 0UL ? 1UL : region->width;
  nexus_info->region.height=region->height == 0UL ? 1UL : region->height;
  nexus_info->region.x=region->x;
  nexus_info->region.y=region->y;
  if ((cache_info->type != DiskCache) && (image->clip_mask == (Image *) NULL) &&
      (image->mask == (Image *) NULL))
    {
      offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
        nexus_info->region.x;
      length=(MagickSizeType) (nexus_info->region.height-1)*cache_info->columns+
        nexus_info->region.width-1;
      number_pixels=(MagickSizeType) cache_info->columns*cache_info->rows;
      if ((offset >= 0) && (((MagickSizeType) offset+length) < number_pixels))
        {
          long
            x,
            y;

          x=nexus_info->region.x+nexus_info->region.width;
          y=nexus_info->region.y+nexus_info->region.height;
          if ((nexus_info->region.x >= 0) &&
              (x <= (long) cache_info->columns) &&
              (nexus_info->region.y >= 0) && (y <= (long) cache_info->rows))
            if ((nexus_info->region.height == 1UL) ||
                ((nexus_info->region.x == 0) &&
                ((nexus_info->region.width % cache_info->columns) == 0)))
              {
                /*
                  Pixels are accessed directly from memory.
                */
                nexus_info->pixels=cache_info->pixels+offset;
                nexus_info->indexes=(IndexPacket *) NULL;
                if (cache_info->active_index_channel != MagickFalse)
                  nexus_info->indexes=cache_info->indexes+offset;
                return(nexus_info->pixels);
              }
        }
    }
  /*
    Pixels are stored in a cache region until they are synced to the cache.
  */
  number_pixels=(MagickSizeType) nexus_info->region.width*
    nexus_info->region.height;
  length=number_pixels*sizeof(PixelPacket);
  if (cache_info->active_index_channel != MagickFalse)
    length+=number_pixels*sizeof(IndexPacket);
  if (nexus_info->cache == (PixelPacket *) NULL)
    {
      nexus_info->length=length;
      status=AcquireCacheNexusPixels(cache_info,nexus_info,exception);
      if (status == MagickFalse)
        return((PixelPacket *) NULL);
    }
  else
    if (nexus_info->length != length)
      {
        RelinquishCacheNexusPixels(nexus_info);
        nexus_info->length=length;
        status=AcquireCacheNexusPixels(cache_info,nexus_info,exception);
        if (status == MagickFalse)
          return((PixelPacket *) NULL);
      }
  nexus_info->pixels=nexus_info->cache;
  nexus_info->indexes=(IndexPacket *) NULL;
  if (cache_info->active_index_channel != MagickFalse)
    nexus_info->indexes=(IndexPacket *) (nexus_info->pixels+number_pixels);
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
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->cache == (Cache) NULL)
    ThrowBinaryException(CacheError,"PixelCacheIsNotOpen",image->filename);
  cache_info=(CacheInfo *) image->cache;
  if (cache_info->type == UndefinedCache)
    return(MagickFalse);
  if ((image->clip_mask != (Image *) NULL) &&
      (ClipPixelCacheNexus(image,nexus_info,exception) == MagickFalse))
    return(MagickFalse);
  if ((image->mask != (Image *) NULL) &&
      (MaskPixelCacheNexus(image,nexus_info,exception) == MagickFalse))
    return(MagickFalse);
  if (IsNexusInCore(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  assert(cache_info->signature == MagickSignature);
  status=WritePixelCachePixels(cache_info,nexus_info,exception);
  if ((cache_info->active_index_channel != MagickFalse) &&
      (WritePixelCacheIndexes(cache_info,nexus_info,exception) == MagickFalse))
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

  long
    id;

  MagickBooleanType
    status;

  cache_info=(CacheInfo *) image->cache;
  id=GetOpenMPThreadId();
  assert(id < (long) cache_info->number_threads);
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

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.sync_authentic_pixels_handler ==
      (SyncAuthenticPixelsHandler) NULL)
    return(MagickFalse);
  return(cache_info->methods.sync_authentic_pixels_handler(image,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   W r i t e P i x e l C a c h e I n d e x e s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePixelCacheIndexes() writes the colormap indexes to the specified
%  region of the pixel cache.
%
%  The format of the WritePixelCacheIndexes() method is:
%
%      MagickBooleanType WritePixelCacheIndexes(CacheInfo *cache_info,
%        NexusInfo *nexus_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_info: the pixel cache.
%
%    o nexus_info: the cache nexus to write the colormap indexes.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType WritePixelCacheIndexes(CacheInfo *cache_info,
  NexusInfo *nexus_info,ExceptionInfo *exception)
{
  MagickOffsetType
    count,
    offset;

  MagickSizeType
    length,
    number_pixels;

  register const IndexPacket
    *__restrict p;

  register long
    y;

  unsigned long
    rows;

  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_info->filename);
  if (cache_info->active_index_channel == MagickFalse)
    return(MagickFalse);
  if (IsNexusInCore(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*sizeof(IndexPacket);
  rows=nexus_info->region.height;
  number_pixels=(MagickSizeType) length*rows;
  if ((cache_info->columns == nexus_info->region.width) &&
      (number_pixels == (MagickSizeType) ((size_t) number_pixels)))
    {
      length=number_pixels;
      rows=1UL;
    }
  p=nexus_info->indexes;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register IndexPacket
        *__restrict q;

      /*
        Write indexes to memory.
      */
      q=cache_info->indexes+offset;
      for (y=0; y < (long) rows; y++)
      {
        (void) CopyMagickMemory(q,p,(size_t) length);
        p+=nexus_info->region.width;
        q+=cache_info->columns;
      }
      break;
    }
    case DiskCache:
    {
      /*
        Write indexes to disk.
      */
      if (OpenPixelCacheOnDisk(cache_info,IOMode) == MagickFalse)
        {
          ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
            cache_info->cache_filename);
          return(MagickFalse);
        }
      number_pixels=(MagickSizeType) cache_info->columns*cache_info->rows;
      for (y=0; y < (long) rows; y++)
      {
        count=WritePixelCacheRegion(cache_info,cache_info->offset+number_pixels*
          sizeof(PixelPacket)+offset*sizeof(*p),length,
          (const unsigned char *) p);
        if ((MagickSizeType) count < length)
          break;
        p+=nexus_info->region.width;
        offset+=cache_info->columns;
      }
      if (y < (long) rows)
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
      (QuantumTick(nexus_info->region.y,cache_info->rows) != MagickFalse))
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s[%lux%lu%+ld%+ld]",
      cache_info->filename,nexus_info->region.width,nexus_info->region.height,
      nexus_info->region.x,nexus_info->region.y);
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
    length,
    number_pixels;

  register long
    y;

  register const PixelPacket
    *__restrict p;

  unsigned long
    rows;

  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_info->filename);
  if (IsNexusInCore(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*sizeof(PixelPacket);
  rows=nexus_info->region.height;
  number_pixels=length*rows;
  if ((cache_info->columns == nexus_info->region.width) &&
      (number_pixels == (MagickSizeType) ((size_t) number_pixels)))
    {
      length=number_pixels;
      rows=1UL;
    }
  p=nexus_info->pixels;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register PixelPacket
        *__restrict q;

      /*
        Write pixels to memory.
      */
      q=cache_info->pixels+offset;
      for (y=0; y < (long) rows; y++)
      {
        (void) CopyMagickMemory(q,p,(size_t) length);
        p+=nexus_info->region.width;
        q+=cache_info->columns;
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
      for (y=0; y < (long) rows; y++)
      {
        count=WritePixelCacheRegion(cache_info,cache_info->offset+offset*
          sizeof(*p),length,(const unsigned char *) p);
        if ((MagickSizeType) count < length)
          break;
        p+=nexus_info->region.width;
        offset+=cache_info->columns;
      }
      if (y < (long) rows)
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
      (QuantumTick(nexus_info->region.y,cache_info->rows) != MagickFalse))
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s[%lux%lu%+ld%+ld]",
      cache_info->filename,nexus_info->region.width,nexus_info->region.height,
      nexus_info->region.x,nexus_info->region.y);
  return(MagickTrue);
}

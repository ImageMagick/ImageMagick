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
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/composite-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/memory-private.h"
#include "magick/pixel.h"
#include "magick/pixel-accessor.h"
#include "magick/pixel-private.h"
#include "magick/policy.h"
#include "magick/quantum.h"
#include "magick/random_.h"
#include "magick/resource_.h"
#include "magick/semaphore.h"
#include "magick/splay-tree.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/thread-private.h"
#include "magick/utility.h"
#include "magick/utility-private.h"
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

  size_t
    signature;
};

/*
  Forward declarations.
*/
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static Cache
  GetImagePixelCache(Image *,const MagickBooleanType,ExceptionInfo *)
    magick_hot_spot;

static const IndexPacket
  *GetVirtualIndexesFromCache(const Image *);

static const PixelPacket
  *GetVirtualPixelCache(const Image *,const VirtualPixelMethod,const ssize_t,
    const ssize_t,const size_t,const size_t,ExceptionInfo *),
  *GetVirtualPixelsCache(const Image *);

static MagickBooleanType
  GetOneAuthenticPixelFromCache(Image *,const ssize_t,const ssize_t,
    PixelPacket *,ExceptionInfo *),
  GetOneVirtualPixelFromCache(const Image *,const VirtualPixelMethod,
    const ssize_t,const ssize_t,PixelPacket *,ExceptionInfo *),
  OpenPixelCache(Image *,const MapMode,ExceptionInfo *),
  ReadPixelCacheIndexes(CacheInfo *,NexusInfo *,ExceptionInfo *),
  ReadPixelCachePixels(CacheInfo *,NexusInfo *,ExceptionInfo *),
  SyncAuthenticPixelsCache(Image *,ExceptionInfo *),
  WritePixelCacheIndexes(CacheInfo *,NexusInfo *,ExceptionInfo *),
  WritePixelCachePixels(CacheInfo *,NexusInfo *,ExceptionInfo *);

static PixelPacket
  *GetAuthenticPixelsCache(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,ExceptionInfo *),
  *QueueAuthenticPixelsCache(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,ExceptionInfo *),
  *SetPixelCacheNexusPixels(const Image *,const MapMode,const RectangleInfo *,
    NexusInfo *,ExceptionInfo *) magick_hot_spot;

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
  cache_info->colorspace=sRGBColorspace;
  cache_info->channels=4;
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
  cache_info->semaphore=AllocateSemaphoreInfo();
  cache_info->reference_count=1;
  cache_info->file_semaphore=AllocateSemaphoreInfo();
  cache_info->debug=IsEventLogging();
  cache_info->signature=MagickSignature;
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
    *restrict r;

  register IndexPacket
    *restrict nexus_indexes,
    *restrict indexes;

  register PixelPacket
    *restrict p,
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
  p=GetAuthenticPixelCacheNexus(image,nexus_info->region.x,nexus_info->region.y,
    nexus_info->region.width,nexus_info->region.height,image_nexus[0],
    exception);
  indexes=GetPixelCacheNexusIndexes(cache_info,image_nexus[0]);
  q=nexus_info->pixels;
  nexus_indexes=nexus_info->indexes;
  r=GetVirtualPixelsFromNexus(image->clip_mask,MaskVirtualPixelMethod,
    nexus_info->region.x,nexus_info->region.y,nexus_info->region.width,
    nexus_info->region.height,clip_nexus[0],exception);
  number_pixels=(MagickSizeType) nexus_info->region.width*
    nexus_info->region.height;
  for (i=0; i < (ssize_t) number_pixels; i++)
  {
    if ((p == (PixelPacket *) NULL) || (r == (const PixelPacket *) NULL))
      break;
    if (PixelIntensityToQuantum(image,r) > (QuantumRange/2))
      {
        SetPixelRed(q,GetPixelRed(p));
        SetPixelGreen(q,GetPixelGreen(p));
        SetPixelBlue(q,GetPixelBlue(p));
        SetPixelOpacity(q,GetPixelOpacity(p));
        if (cache_info->active_index_channel != MagickFalse)
          SetPixelIndex(nexus_indexes+i,GetPixelIndex(indexes+i));
      }
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
  if (cache_info->file != -1)
    {
      status=close(cache_info->file);
      cache_info->file=(-1);
      RelinquishMagickResource(FileResource,1);
    }
  return(status == -1 ? MagickFalse : MagickTrue);
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
  if (cache_info->file != -1)
    return(MagickTrue);  /* cache already open */
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
  cache_info->file=file;
  cache_info->mode=mode;
  return(MagickTrue);
}

static inline MagickOffsetType ReadPixelCacheRegion(
  const CacheInfo *restrict cache_info,const MagickOffsetType offset,
  const MagickSizeType length,unsigned char *restrict buffer)
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
    count=read(cache_info->file,buffer+i,(size_t) MagickMin(length-i,
      (MagickSizeType) SSIZE_MAX));
#else
    count=pread(cache_info->file,buffer+i,(size_t) MagickMin(length-i,
      (MagickSizeType) SSIZE_MAX),(off_t) (offset+i));
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

static inline MagickOffsetType WritePixelCacheRegion(
  const CacheInfo *restrict cache_info,const MagickOffsetType offset,
  const MagickSizeType length,const unsigned char *restrict buffer)
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
    count=write(cache_info->file,buffer+i,(size_t) MagickMin(length-i,
      (MagickSizeType) SSIZE_MAX));
#else
    count=pwrite(cache_info->file,buffer+i,(size_t) MagickMin(length-i,
      (MagickSizeType) SSIZE_MAX),(off_t) (offset+i));
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

static MagickBooleanType CloneDiskToDiskPixelCache(CacheInfo *clone_info,
  CacheInfo *cache_info,ExceptionInfo *exception)
{
  MagickOffsetType
    count,
    offset,
    source_offset;

  MagickSizeType
    length;

  register PixelPacket
    *restrict pixels;

  register ssize_t
    y;

  size_t
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
  columns=(size_t) MagickMin(clone_info->columns,cache_info->columns);
  rows=(size_t) MagickMin(clone_info->rows,cache_info->rows);
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
        sizeof(*pixels);
      offset=(MagickOffsetType) clone_info->columns*clone_info->rows*
        sizeof(*pixels);
      for (y=0; y < (ssize_t) rows; y++)
      {
        count=ReadPixelCacheRegion(cache_info,cache_info->offset+source_offset,
          length,(unsigned char *) indexes);
        if ((MagickSizeType) count != length)
          break;
        count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,length,
          (unsigned char *) indexes);
        if ((MagickSizeType) count != length)
          break;
        source_offset+=cache_info->columns*sizeof(*indexes);
        offset+=clone_info->columns*sizeof(*indexes);
      }
      if (y < (ssize_t) rows)
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
            sizeof(*pixels);
          for (y=0; y < (ssize_t) rows; y++)
          {
            count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,
              length,(unsigned char *) indexes);
            if ((MagickSizeType) count != length)
              break;
            offset+=clone_info->columns*sizeof(*indexes);
          }
          if (y < (ssize_t) rows)
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
  source_offset=0;
  offset=0;
  for (y=0; y < (ssize_t) rows; y++)
  {
    count=ReadPixelCacheRegion(cache_info,cache_info->offset+source_offset,
      length,(unsigned char *) pixels);
    if ((MagickSizeType) count != length)
      break;
    count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,length,
      (unsigned char *) pixels);
    if ((MagickSizeType) count != length)
      break;
    source_offset+=cache_info->columns*sizeof(*pixels);
    offset+=clone_info->columns*sizeof(*pixels);
  }
  if (y < (ssize_t) rows)
    {
      pixels=(PixelPacket *) RelinquishMagickMemory(pixels);
      ThrowFileException(exception,CacheError,"UnableToCloneCache",
        cache_info->cache_filename);
      return(MagickFalse);
    }
  if (clone_info->columns > cache_info->columns)
    {
      length=(clone_info->columns-cache_info->columns)*sizeof(*pixels);
      (void) ResetMagickMemory(pixels,0,(size_t) length);
      offset=(MagickOffsetType) columns*sizeof(*pixels);
      for (y=0; y < (ssize_t) rows; y++)
      {
        count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,length,
          (unsigned char *) pixels);
        if ((MagickSizeType) count != length)
          break;
        offset+=clone_info->columns*sizeof(*pixels);
      }
      if (y < (ssize_t) rows)
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

  register PixelPacket
    *restrict pixels,
    *restrict q;

  register ssize_t
    y;

  size_t
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
  columns=(size_t) MagickMin(clone_info->columns,cache_info->columns);
  rows=(size_t) MagickMin(clone_info->rows,cache_info->rows);
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
        sizeof(*pixels);
      q=clone_info->indexes;
      for (y=0; y < (ssize_t) rows; y++)
      {
        count=ReadPixelCacheRegion(cache_info,cache_info->offset+offset,
          length,(unsigned char *) indexes);
        if ((MagickSizeType) count != length)
          break;
        (void) memcpy(q,indexes,(size_t) length);
        if ((MagickSizeType) count != length)
          break;
        offset+=cache_info->columns*sizeof(*indexes);
        q+=clone_info->columns;
      }
      if (y < (ssize_t) rows)
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
  offset=0;
  q=clone_info->pixels;
  for (y=0; y < (ssize_t) rows; y++)
  {
    count=ReadPixelCacheRegion(cache_info,cache_info->offset+offset,length,
      (unsigned char *) pixels);
    if ((MagickSizeType) count != length)
      break;
    (void) memcpy(q,pixels,(size_t) length);
    offset+=cache_info->columns*sizeof(*pixels);
    q+=clone_info->columns;
  }
  if (y < (ssize_t) rows)
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

  register PixelPacket
    *restrict p,
    *restrict pixels;

  register ssize_t
    y;

  size_t
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
  columns=(size_t) MagickMin(clone_info->columns,cache_info->columns);
  rows=(size_t) MagickMin(clone_info->rows,cache_info->rows);
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
      p=cache_info->indexes;
      offset=(MagickOffsetType) clone_info->columns*clone_info->rows*
        sizeof(*pixels);
      for (y=0; y < (ssize_t) rows; y++)
      {
        (void) memcpy(indexes,p,(size_t) length);
        count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,length,
          (unsigned char *) indexes);
        if ((MagickSizeType) count != length)
          break;
        p+=cache_info->columns;
        offset+=clone_info->columns*sizeof(*indexes);
      }
      if (y < (ssize_t) rows)
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
            sizeof(*pixels);
          for (y=0; y < (ssize_t) rows; y++)
          {
            count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,
              length,(unsigned char *) indexes);
            if ((MagickSizeType) count != length)
              break;
            offset+=clone_info->columns*sizeof(*indexes);
          }
          if (y < (ssize_t) rows)
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
  p=cache_info->pixels;
  offset=0;
  for (y=0; y < (ssize_t) rows; y++)
  {
    (void) memcpy(pixels,p,(size_t) length);
    count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,length,
      (unsigned char *) pixels);
    if ((MagickSizeType) count != length)
      break;
    p+=cache_info->columns;
    offset+=clone_info->columns*sizeof(*pixels);
  }
  if (y < (ssize_t) rows)
    {
      pixels=(PixelPacket *) RelinquishMagickMemory(pixels);
      ThrowFileException(exception,CacheError,"UnableToCloneCache",
        cache_info->cache_filename);
      return(MagickFalse);
    }
  if (clone_info->columns > cache_info->columns)
    {
      length=(clone_info->columns-cache_info->columns)*sizeof(*pixels);
      (void) ResetMagickMemory(pixels,0,(size_t) length);
      offset=(MagickOffsetType) columns*sizeof(*pixels);
      for (y=0; y < (ssize_t) rows; y++)
      {
        count=WritePixelCacheRegion(clone_info,clone_info->offset+offset,length,
          (unsigned char *) pixels);
        if ((MagickSizeType) count != length)
          break;
        offset+=clone_info->columns*sizeof(*pixels);
      }
      if (y < (ssize_t) rows)
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
  register PixelPacket
    *restrict pixels,
    *restrict source_pixels;

  register ssize_t
    y;

  size_t
    columns,
    length,
    rows;

  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(CacheEvent,GetMagickModule(),"memory => memory");
  columns=(size_t) MagickMin(clone_info->columns,cache_info->columns);
  rows=(size_t) MagickMin(clone_info->rows,cache_info->rows);
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
        (void) memcpy(clone_info->indexes,cache_info->indexes,length*rows);
      else
        {
          source_indexes=cache_info->indexes;
          indexes=clone_info->indexes;
          for (y=0; y < (ssize_t) rows; y++)
          {
            (void) memcpy(indexes,source_indexes,length);
            source_indexes+=cache_info->columns;
            indexes+=clone_info->columns;
          }
          if (clone_info->columns > cache_info->columns)
            {
              length=(clone_info->columns-cache_info->columns)*sizeof(*indexes);
              indexes=clone_info->indexes+cache_info->columns;
              for (y=0; y < (ssize_t) rows; y++)
              {
                (void) ResetMagickMemory(indexes,0,length);
                indexes+=clone_info->columns;
              }
            }
        }
    }
  /*
    Clone cache pixels.
  */
  length=columns*sizeof(*pixels);
  if (clone_info->columns == cache_info->columns)
    (void) memcpy(clone_info->pixels,cache_info->pixels,length*rows);
  else
    {
      source_pixels=cache_info->pixels;
      pixels=clone_info->pixels;
      for (y=0; y < (ssize_t) rows; y++)
      {
        (void) memcpy(pixels,source_pixels,length);
        source_pixels+=cache_info->columns;
        pixels+=clone_info->columns;
      }
      if (clone_info->columns > cache_info->columns)
        {
          length=(clone_info->columns-cache_info->columns)*sizeof(*pixels);
          pixels=clone_info->pixels+cache_info->columns;
          for (y=0; y < (ssize_t) rows; y++)
          {
            (void) ResetMagickMemory(pixels,0,length);
            pixels+=clone_info->columns;
          }
        }
    }
  return(MagickTrue);
}

static MagickBooleanType ClonePixelCachePixels(CacheInfo *clone_info,
  CacheInfo *cache_info,ExceptionInfo *exception)
{
  if (cache_info->type == PingCache)
    return(MagickTrue);
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
        cache_info->pixels=(PixelPacket *) RelinquishAlignedMemory(
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
        message[MaxTextExtent];

      (void) FormatLocaleString(message,MaxTextExtent,"destroy %s",
        cache_info->filename);
      (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",message);
    }
  RelinquishPixelCachePixels(cache_info);
  if (cache_info->nexus_info != (NexusInfo **) NULL)
    cache_info->nexus_info=DestroyPixelCacheNexus(cache_info->nexus_info,
      cache_info->number_threads);
  if (cache_info->random_info != (RandomInfo *) NULL)
    cache_info->random_info=DestroyRandomInfo(cache_info->random_info);
  if (cache_info->file_semaphore != (SemaphoreInfo *) NULL)
    DestroySemaphoreInfo(&cache_info->file_semaphore);
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
    (void) RelinquishAlignedMemory(nexus_info->cache);
  else
    (void) UnmapBlob(nexus_info->cache,(size_t) nexus_info->length);
  nexus_info->cache=(PixelPacket *) NULL;
  nexus_info->pixels=(PixelPacket *) NULL;
  nexus_info->indexes=(IndexPacket *) NULL;
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
    if (nexus_info[i]->cache != (PixelPacket *) NULL)
      RelinquishCacheNexusPixels(nexus_info[i]);
    nexus_info[i]->signature=(~MagickSignature);
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

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  assert(id < (int) cache_info->number_threads);
  return(GetPixelCacheNexusIndexes(image->cache,cache_info->nexus_info[id]));
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

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.get_authentic_indexes_from_handler !=
       (GetAuthenticIndexesFromHandler) NULL)
    return(cache_info->methods.get_authentic_indexes_from_handler(image));
  assert(id < (int) cache_info->number_threads);
  return(GetPixelCacheNexusIndexes(cache_info,cache_info->nexus_info[id]));
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
%      PixelPacket *GetAuthenticPixelCacheNexus(Image *image,const ssize_t x,
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

static inline MagickBooleanType IsPixelAuthentic(
  const CacheInfo *restrict cache_info,const NexusInfo *restrict nexus_info)
{
  MagickOffsetType
    offset;

  if (cache_info->type == PingCache)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  return(nexus_info->pixels == (cache_info->pixels+offset) ? MagickTrue :
    MagickFalse);
}

MagickExport PixelPacket *GetAuthenticPixelCacheNexus(Image *image,
  const ssize_t x,const ssize_t y,const size_t columns,const size_t rows,
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
  pixels=QueueAuthenticPixelCacheNexus(image,x,y,columns,rows,MagickTrue,
    nexus_info,exception);
  if (pixels == (PixelPacket *) NULL)
    return((PixelPacket *) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (IsPixelAuthentic(cache_info,nexus_info) != MagickFalse)
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
%  if the image is in memory, or in a memory-mapped file. The returned pointer
%  must *never* be deallocated by the user.
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
%      PixelPacket *GetAuthenticPixels(Image *image,const ssize_t x,
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
MagickExport PixelPacket *GetAuthenticPixels(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.get_authentic_pixels_handler !=
       (GetAuthenticPixelsHandler) NULL)
    return(cache_info->methods.get_authentic_pixels_handler(image,x,y,columns,
      rows,exception));
  assert(id < (int) cache_info->number_threads);
  return(GetAuthenticPixelCacheNexus(image,x,y,columns,rows,
    cache_info->nexus_info[id],exception));
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
%      PixelPacket *GetAuthenticPixelsCache(Image *image,const ssize_t x,
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
static PixelPacket *GetAuthenticPixelsCache(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  if (cache_info == (Cache) NULL)
    return((PixelPacket *) NULL);
  assert(cache_info->signature == MagickSignature);
  assert(id < (int) cache_info->number_threads);
  return(GetAuthenticPixelCacheNexus(image,x,y,columns,rows,
    cache_info->nexus_info[id],exception));
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
static inline MagickBooleanType ValidatePixelCacheMorphology(
  const Image *restrict image)
{
  CacheInfo
    *cache_info;

  /*
    Does the image match the pixel cache morphology?
  */
  cache_info=(CacheInfo *) image->cache;
  if ((image->storage_class != cache_info->storage_class) ||
      (image->colorspace != cache_info->colorspace) ||
      (image->channels != cache_info->channels) ||
      (image->columns != cache_info->columns) ||
      (image->rows != cache_info->rows) ||
      (cache_info->nexus_info == (NexusInfo **) NULL))
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
        Set the expire time in seconds.
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
          CacheInfo
            *clone_info;

          Image
            clone_image;

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
                  if (cache_info->reference_count == 1) 
                    cache_info->nexus_info=(NexusInfo **) NULL;
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
%  DiskCache, MapCache, MemoryCache, or PingCache.
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

MagickExport CacheType GetPixelCacheType(const Image *image)
{
  return(GetImagePixelCacheType(image));
}

MagickExport CacheType GetImagePixelCacheType(const Image *image)
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

  PixelPacket
    *pixels;

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

  PixelPacket
    *pixels;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  *pixel=image->background_color;
  assert(id < (int) cache_info->number_threads);
  pixels=GetAuthenticPixelCacheNexus(image,x,y,1UL,1UL,
    cache_info->nexus_info[id],exception);
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
%        const ssize_t x,const ssize_t y,MagickPixelPacket *pixel,
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
  const ssize_t x,const ssize_t y,MagickPixelPacket *pixel,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *pixels;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  assert(id < (int) cache_info->number_threads);
  pixels=GetVirtualPixelsFromNexus(image,GetPixelCacheVirtualMethod(image),x,y,
    1UL,1UL,cache_info->nexus_info[id],exception);
  GetMagickPixelPacket(image,pixel);
  if (pixels == (const PixelPacket *) NULL)
    return(MagickFalse);
  indexes=GetVirtualIndexesFromNexus(cache_info,cache_info->nexus_info[id]);
  SetMagickPixelPacket(image,pixels,indexes,pixel);
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
%        const ssize_t y,Pixelpacket *pixel,ExceptionInfo exception)
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

  const PixelPacket
    *pixels;

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
  pixels=GetVirtualPixelsFromNexus(image,virtual_pixel_method,x,y,1UL,1UL,
    cache_info->nexus_info[id],exception);
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

  const PixelPacket
    *pixels;

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
  pixels=GetVirtualPixelsFromNexus(image,GetPixelCacheVirtualMethod(image),x,y,
    1UL,1UL,cache_info->nexus_info[id],exception);
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
%        const VirtualPixelPacket method,const ssize_t x,const ssize_t y,
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

  const PixelPacket
    *pixels;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  assert(id < (int) cache_info->number_threads);
  *pixel=image->background_color;
  pixels=GetVirtualPixelsFromNexus(image,virtual_pixel_method,x,y,1UL,1UL,
    cache_info->nexus_info[id],exception);
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
+   G e t P i x e l C a c h e C h a n n e l s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelCacheChannels() returns the number of pixel channels associated
%  with this instance of the pixel cache.
%
%  The format of the GetPixelCacheChannels() method is:
%
%      size_t GetPixelCacheChannels(Cache cache)
%
%  A description of each parameter follows:
%
%    o type: GetPixelCacheChannels returns DirectClass or PseudoClass.
%
%    o cache: the pixel cache.
%
*/
MagickExport size_t GetPixelCacheChannels(const Cache cache)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_info->filename);
  return(cache_info->channels);
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

  assert(cache != (const Cache) NULL);
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

  assert(cache != (const Cache) NULL);
  cache_info=(CacheInfo *) cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->storage_class == UndefinedClass)
    return((PixelPacket *) NULL);
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
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *width=2048UL/sizeof(PixelPacket);
  if (GetImagePixelCacheType(image) == DiskCache)
    *width=8192UL/sizeof(PixelPacket);
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

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  assert(id < (int) cache_info->number_threads);
  return(GetVirtualIndexesFromNexus(cache_info,cache_info->nexus_info[id]));
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

  assert(cache != (Cache) NULL);
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

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.get_virtual_indexes_from_handler !=
       (GetVirtualIndexesFromHandler) NULL)
    return(cache_info->methods.get_virtual_indexes_from_handler(image));
  assert(id < (int) cache_info->number_threads);
  return(GetVirtualIndexesFromNexus(cache_info,cache_info->nexus_info[id]));
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

MagickExport const PixelPacket *GetVirtualPixelsFromNexus(const Image *image,
  const VirtualPixelMethod virtual_pixel_method,const ssize_t x,const ssize_t y,
  const size_t columns,const size_t rows,NexusInfo *nexus_info,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  IndexPacket
    virtual_index;

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
    *restrict virtual_indexes;

  register const PixelPacket
    *restrict p;

  register IndexPacket
    *restrict indexes;

  register PixelPacket
    *restrict q;

  register ssize_t
    u,
    v;

  /*
    Acquire pixels.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->type == UndefinedCache)
    return((const PixelPacket *) NULL);
  region.x=x;
  region.y=y;
  region.width=columns;
  region.height=rows;
  pixels=SetPixelCacheNexusPixels(image,ReadMode,&region,nexus_info,exception);
  if (pixels == (PixelPacket *) NULL)
    return((const PixelPacket *) NULL);
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
      SetPixelRed(&virtual_pixel,0);
      SetPixelGreen(&virtual_pixel,0);
      SetPixelBlue(&virtual_pixel,0);
      SetPixelOpacity(&virtual_pixel,OpaqueOpacity);
      break;
    }
    case GrayVirtualPixelMethod:
    {
      SetPixelRed(&virtual_pixel,QuantumRange/2);
      SetPixelGreen(&virtual_pixel,QuantumRange/2);
      SetPixelBlue(&virtual_pixel,QuantumRange/2);
      SetPixelOpacity(&virtual_pixel,OpaqueOpacity);
      break;
    }
    case TransparentVirtualPixelMethod:
    {
      SetPixelRed(&virtual_pixel,0);
      SetPixelGreen(&virtual_pixel,0);
      SetPixelBlue(&virtual_pixel,0);
      SetPixelOpacity(&virtual_pixel,TransparentOpacity);
      break;
    }
    case MaskVirtualPixelMethod:
    case WhiteVirtualPixelMethod:
    {
      SetPixelRed(&virtual_pixel,QuantumRange);
      SetPixelGreen(&virtual_pixel,QuantumRange);
      SetPixelBlue(&virtual_pixel,QuantumRange);
      SetPixelOpacity(&virtual_pixel,OpaqueOpacity);
      break;
    }
    default:
    {
      virtual_pixel=image->background_color;
      break;
    }
  }
  virtual_index=0;
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
            case BackgroundVirtualPixelMethod:
            case ConstantVirtualPixelMethod:
            case BlackVirtualPixelMethod:
            case GrayVirtualPixelMethod:
            case TransparentVirtualPixelMethod:
            case MaskVirtualPixelMethod:
            case WhiteVirtualPixelMethod:
            {
              p=(&virtual_pixel);
              virtual_indexes=(&virtual_index);
              break;
            }
            case EdgeVirtualPixelMethod:
            default:
            {
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                EdgeX(x+u,cache_info->columns),EdgeY(y+v,cache_info->rows),
                1UL,1UL,*virtual_nexus,exception);
              virtual_indexes=GetVirtualIndexesFromNexus(cache_info,
                *virtual_nexus);
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
              virtual_indexes=GetVirtualIndexesFromNexus(cache_info,
                *virtual_nexus);
              break;
            }
            case DitherVirtualPixelMethod:
            {
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                DitherX(x+u,cache_info->columns),DitherY(y+v,cache_info->rows),
                1UL,1UL,*virtual_nexus,exception);
              virtual_indexes=GetVirtualIndexesFromNexus(cache_info,
                *virtual_nexus);
              break;
            }
            case TileVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,*virtual_nexus,
                exception);
              virtual_indexes=GetVirtualIndexesFromNexus(cache_info,
                *virtual_nexus);
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
              virtual_indexes=GetVirtualIndexesFromNexus(cache_info,
                *virtual_nexus);
              break;
            }
            case CheckerTileVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              if (((x_modulo.quotient ^ y_modulo.quotient) & 0x01) != 0L)
                {
                  p=(&virtual_pixel);
                  virtual_indexes=(&virtual_index);
                  break;
                }
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,*virtual_nexus,
                exception);
              virtual_indexes=GetVirtualIndexesFromNexus(cache_info,
                *virtual_nexus);
              break;
            }
            case HorizontalTileVirtualPixelMethod:
            {
              if (((y+v) < 0) || ((y+v) >= (ssize_t) cache_info->rows))
                {
                  p=(&virtual_pixel);
                  virtual_indexes=(&virtual_index);
                  break;
                }
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,*virtual_nexus,
                exception);
              virtual_indexes=GetVirtualIndexesFromNexus(cache_info,
                *virtual_nexus);
              break;
            }
            case VerticalTileVirtualPixelMethod:
            {
              if (((x+u) < 0) || ((x+u) >= (ssize_t) cache_info->columns))
                {
                  p=(&virtual_pixel);
                  virtual_indexes=(&virtual_index);
                  break;
                }
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,y_modulo.remainder,1UL,1UL,*virtual_nexus,
                exception);
              virtual_indexes=GetVirtualIndexesFromNexus(cache_info,
                *virtual_nexus);
              break;
            }
            case HorizontalTileEdgeVirtualPixelMethod:
            {
              x_modulo=VirtualPixelModulo(x+u,cache_info->columns);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                x_modulo.remainder,EdgeY(y+v,cache_info->rows),1UL,1UL,
                *virtual_nexus,exception);
              virtual_indexes=GetVirtualIndexesFromNexus(cache_info,
                *virtual_nexus);
              break;
            }
            case VerticalTileEdgeVirtualPixelMethod:
            {
              y_modulo=VirtualPixelModulo(y+v,cache_info->rows);
              p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,
                EdgeX(x+u,cache_info->columns),y_modulo.remainder,1UL,1UL,
                *virtual_nexus,exception);
              virtual_indexes=GetVirtualIndexesFromNexus(cache_info,
                *virtual_nexus);
              break;
            }
          }
          if (p == (const PixelPacket *) NULL)
            break;
          *q++=(*p);
          if ((indexes != (IndexPacket *) NULL) &&
              (virtual_indexes != (const IndexPacket *) NULL))
            *indexes++=(*virtual_indexes);
          continue;
        }
      /*
        Transfer a run of pixels.
      */
      p=GetVirtualPixelsFromNexus(image,virtual_pixel_method,x+u,y+v,
        (size_t) length,1UL,*virtual_nexus,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      virtual_indexes=GetVirtualIndexesFromNexus(cache_info,*virtual_nexus);
      (void) memcpy(q,p,(size_t) length*sizeof(*p));
      q+=length;
      if ((indexes != (IndexPacket *) NULL) &&
          (virtual_indexes != (const IndexPacket *) NULL))
        {
          (void) memcpy(indexes,virtual_indexes,(size_t) length*
            sizeof(*virtual_indexes));
          indexes+=length;
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
static const PixelPacket *GetVirtualPixelCache(const Image *image,
  const VirtualPixelMethod virtual_pixel_method,const ssize_t x,const ssize_t y,
  const size_t columns,const size_t rows,ExceptionInfo *exception)
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
  return(GetVirtualPixelsFromNexus(image,virtual_pixel_method,x,y,columns,rows,
    cache_info->nexus_info[id],exception));
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
%      const PixelPacket *GetVirtualPixels(const Image *image,const ssize_t x,
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
MagickExport const PixelPacket *GetVirtualPixels(const Image *image,
  const ssize_t x,const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
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
  if (cache_info->methods.get_virtual_pixel_handler !=
       (GetVirtualPixelHandler) NULL)
    return(cache_info->methods.get_virtual_pixel_handler(image,
      GetPixelCacheVirtualMethod(image),x,y,columns,rows,exception));
  assert(id < (int) cache_info->number_threads);
  return(GetVirtualPixelsFromNexus(image,GetPixelCacheVirtualMethod(image),x,y,
    columns,rows,cache_info->nexus_info[id],exception));
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

  assert(cache != (Cache) NULL);
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
  gamma=PerceptibleReciprocal(gamma);
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
    *restrict r;

  register IndexPacket
    *restrict nexus_indexes,
    *restrict indexes;

  register PixelPacket
    *restrict p,
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
  p=GetAuthenticPixelCacheNexus(image,nexus_info->region.x,
    nexus_info->region.y,nexus_info->region.width,nexus_info->region.height,
    image_nexus[0],exception);
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
  for (i=0; i < (ssize_t) number_pixels; i++)
  {
    if ((p == (PixelPacket *) NULL) || (r == (const PixelPacket *) NULL))
      break;
    SetMagickPixelPacket(image,p,indexes+i,&alpha);
    SetMagickPixelPacket(image,q,nexus_indexes+i,&beta);
    MagickPixelCompositeMask(&beta,(MagickRealType)
      PixelIntensityToQuantum(image,r),&alpha,alpha.opacity,&beta);
    SetPixelRed(q,ClampToQuantum(beta.red));
    SetPixelGreen(q,ClampToQuantum(beta.green));
    SetPixelBlue(q,ClampToQuantum(beta.blue));
    SetPixelOpacity(q,ClampToQuantum(beta.opacity));
    if (cache_info->active_index_channel != MagickFalse)
      SetPixelIndex(nexus_indexes+i,GetPixelIndex(indexes+i));
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

static inline void AllocatePixelCachePixels(CacheInfo *cache_info)
{
  cache_info->mapped=MagickFalse;
  cache_info->pixels=(PixelPacket *) MagickAssumeAligned(AcquireAlignedMemory(1,
    (size_t) cache_info->length));
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
  offset=(MagickOffsetType) lseek(cache_info->file,0,SEEK_END);
  if (offset < 0)
    return(MagickFalse);
  if ((MagickSizeType) offset >= length)
    return(MagickTrue);
  extent=(MagickOffsetType) length-1;
#if !defined(MAGICKCORE_HAVE_POSIX_FALLOCATE)
  {
    MagickOffsetType
      count;

    count=WritePixelCacheRegion(cache_info,extent,1,(const unsigned char *) "");
    if (count != (MagickOffsetType) 1)
      return(MagickFalse);
  }
#else
  {
    int
      status;

    status=posix_fallocate(cache_info->file,offset+1,extent-offset);
    if (status != 0)
      return(MagickFalse);
  }
#endif
  return(MagickTrue);
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

  MagickSizeType
    length,
    number_pixels;

  MagickStatusType
    status;

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
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  source_info=(*cache_info);
  source_info.file=(-1);
  (void) FormatLocaleString(cache_info->filename,MaxTextExtent,"%s[%.20g]",
    image->filename,(double) GetImageIndexInList(image));
  cache_info->mode=mode;
  cache_info->rows=image->rows;
  cache_info->columns=image->columns;
  cache_info->channels=image->channels;
  cache_info->active_index_channel=((image->storage_class == PseudoClass) ||
    (image->colorspace == CMYKColorspace)) ? MagickTrue : MagickFalse;
  if (image->ping != MagickFalse)
    {
      cache_info->storage_class=image->storage_class;
      cache_info->colorspace=image->colorspace;
      cache_info->type=PingCache;
      cache_info->pixels=(PixelPacket *) NULL;
      cache_info->indexes=(IndexPacket *) NULL;
      cache_info->length=0;
      return(MagickTrue);
    }
  number_pixels=(MagickSizeType) cache_info->columns*cache_info->rows;
  packet_size=sizeof(PixelPacket);
  if (cache_info->active_index_channel != MagickFalse)
    packet_size+=sizeof(IndexPacket);
  length=number_pixels*packet_size;
  columns=(size_t) (length/cache_info->rows/packet_size);
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
          AllocatePixelCachePixels(cache_info);
          if (cache_info->pixels == (PixelPacket *) NULL)
            cache_info->pixels=source_info.pixels;
          else
            {
              /*
                Create memory pixel cache.
              */
              cache_info->colorspace=image->colorspace;
              cache_info->type=MemoryCache;
              cache_info->indexes=(IndexPacket *) NULL;
              if (cache_info->active_index_channel != MagickFalse)
                cache_info->indexes=(IndexPacket *) (cache_info->pixels+
                  number_pixels);
              if ((source_info.storage_class != UndefinedClass) &&
                  (mode != ReadMode))
                {
                  status|=ClonePixelCachePixels(cache_info,&source_info,
                    exception);
                  RelinquishPixelCachePixels(&source_info);
                }
              if (image->debug != MagickFalse)
                {
                  (void) FormatMagickSize(cache_info->length,MagickTrue,format);
                  (void) FormatLocaleString(message,MaxTextExtent,
                    "open %s (%s memory, %.20gx%.20g %s)",cache_info->filename,
                    cache_info->mapped != MagickFalse ? "anonymous" : "heap",
                    (double) cache_info->columns,(double) cache_info->rows,
                    format);
                  (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",
                    message);
                }
              cache_info->storage_class=image->storage_class;
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
  if (length != (MagickSizeType) ((size_t) length))
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
              if ((source_info.storage_class != UndefinedClass) &&
                  (mode != ReadMode))
                {
                  status=ClonePixelCachePixels(cache_info,&source_info,
                    exception);
                  RelinquishPixelCachePixels(&source_info);
                }
              if (image->debug != MagickFalse)
                {
                  (void) FormatMagickSize(cache_info->length,MagickTrue,format);
                  (void) FormatLocaleString(message,MaxTextExtent,
                    "open %s (%s[%d], memory-mapped, %.20gx%.20g %s)",
                    cache_info->filename,cache_info->cache_filename,
                    cache_info->file,(double) cache_info->columns,(double)
                    cache_info->rows,format);
                  (void) LogMagickEvent(CacheEvent,GetMagickModule(),"%s",
                    message);
                }
              return(MagickTrue);
            }
        }
      RelinquishMagickResource(MapResource,cache_info->length);
    }
  if ((source_info.storage_class != UndefinedClass) && (mode != ReadMode))
    {
      status=ClonePixelCachePixels(cache_info,&source_info,exception);
      RelinquishPixelCachePixels(&source_info);
    }
  if (image->debug != MagickFalse)
    {
      (void) FormatMagickSize(cache_info->length,MagickFalse,format);
      (void) FormatLocaleString(message,MaxTextExtent,
        "open %s (%s[%d], disk, %.20gx%.20g %s)",cache_info->filename,
        cache_info->cache_filename,cache_info->file,(double)
        cache_info->columns,(double) cache_info->rows,format);
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
      if ((cache_info->mode != ReadMode) && (cache_info->type != MemoryCache) &&
          (cache_info->reference_count == 1))
        {
          int
            status;

          /*
            Usurp existing persistent pixel cache.
          */
          status=rename_utf8(cache_info->cache_filename,filename);
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
%      PixelPacket *QueueAuthenticPixelCacheNexus(Image *image,const ssize_t x,
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
MagickExport PixelPacket *QueueAuthenticPixel(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
  const MagickBooleanType clone,NexusInfo *nexus_info,
  ExceptionInfo *exception)
{
  return(QueueAuthenticPixelCacheNexus(image,x,y,columns,rows,clone,nexus_info,
    exception));
}

MagickExport PixelPacket *QueueAuthenticPixelCacheNexus(Image *image,
  const ssize_t x,const ssize_t y,const size_t columns,const size_t rows,
  const MagickBooleanType clone,NexusInfo *nexus_info,ExceptionInfo *exception)
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
  cache_info=(CacheInfo *) GetImagePixelCache(image,clone,exception);
  if (cache_info == (Cache) NULL)
    return((PixelPacket *) NULL);
  assert(cache_info->signature == MagickSignature);
  if ((cache_info->columns == 0) && (cache_info->rows == 0))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "NoPixelsDefinedInCache","`%s'",image->filename);
      return((PixelPacket *) NULL);
    }
  if ((x < 0) || (y < 0) || (x >= (ssize_t) cache_info->columns) ||
      (y >= (ssize_t) cache_info->rows))
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
  return(SetPixelCacheNexusPixels(image,WriteMode,&region,nexus_info,exception));
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
%      PixelPacket *QueueAuthenticPixelsCache(Image *image,const ssize_t x,
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
static PixelPacket *QueueAuthenticPixelsCache(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
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
  return(QueueAuthenticPixelCacheNexus(image,x,y,columns,rows,MagickFalse,
    cache_info->nexus_info[id],exception));
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
%  successfully initialized a pointer to a PixelPacket array representing the
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
%  PixelPacket. If the image type is CMYK or the storage class is PseudoClass,
%  call GetAuthenticIndexQueue() after invoking GetAuthenticPixels() to obtain
%  the black color component or the colormap indexes (of type IndexPacket)
%  corresponding to the region.  Once the PixelPacket (and/or IndexPacket)
%  array has been updated, the changes must be saved back to the underlying
%  image using SyncAuthenticPixels() or they may be lost.
%
%  The format of the QueueAuthenticPixels() method is:
%
%      PixelPacket *QueueAuthenticPixels(Image *image,const ssize_t x,
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
MagickExport PixelPacket *QueueAuthenticPixels(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.queue_authentic_pixels_handler !=
       (QueueAuthenticPixelsHandler) NULL)
    return(cache_info->methods.queue_authentic_pixels_handler(image,x,y,columns,
      rows,exception));
  assert(id < (int) cache_info->number_threads);
  return(QueueAuthenticPixelCacheNexus(image,x,y,columns,rows,MagickFalse,
    cache_info->nexus_info[id],exception));
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
    extent,
    length;

  register IndexPacket
    *restrict q;

  register ssize_t
    y;

  size_t
    rows;

  if (cache_info->active_index_channel == MagickFalse)
    return(MagickFalse);
  if (IsPixelAuthentic(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*sizeof(IndexPacket);
  rows=nexus_info->region.height;
  extent=length*rows;
  q=nexus_info->indexes;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register IndexPacket
        *restrict p;

      /*
        Read indexes from memory.
      */
      if ((cache_info->columns == nexus_info->region.width) &&
          (extent == (MagickSizeType) ((size_t) extent)))
        {
          length=extent;
          rows=1UL;
        }
      p=cache_info->indexes+offset;
      for (y=0; y < (ssize_t) rows; y++)
      {
        (void) memcpy(q,p,(size_t) length);
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
          sizeof(PixelPacket)+offset*sizeof(*q),length,(unsigned char *) q);
        if ((MagickSizeType) count < length)
          break;
        offset+=cache_info->columns;
        q+=nexus_info->region.width;
      }
      if (IsFileDescriptorLimitExceeded() != MagickFalse)
        (void) ClosePixelCacheOnDisk(cache_info);
      UnlockSemaphoreInfo(cache_info->file_semaphore);
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

  register PixelPacket
    *restrict q;

  register ssize_t
    y;

  size_t
    rows;

  if (IsPixelAuthentic(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*sizeof(PixelPacket);
  rows=nexus_info->region.height;
  extent=length*rows;
  q=nexus_info->pixels;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register PixelPacket
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
      p=cache_info->pixels+offset;
      for (y=0; y < (ssize_t) rows; y++)
      {
        (void) memcpy(q,p,(size_t) length);
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
          sizeof(*q),length,(unsigned char *) q);
        if ((MagickSizeType) count < length)
          break;
        offset+=cache_info->columns;
        q+=nexus_info->region.width;
      }
      if (IsFileDescriptorLimitExceeded() != MagickFalse)
        (void) ClosePixelCacheOnDisk(cache_info);
      UnlockSemaphoreInfo(cache_info->file_semaphore);
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
%        const MapMode mode,const RectangleInfo *region,NexusInfo *nexus_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
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
  const CacheInfo *restrict cache_info,NexusInfo *nexus_info,
  ExceptionInfo *exception)
{
  if (nexus_info->length != (MagickSizeType) ((size_t) nexus_info->length))
    return(MagickFalse);
  nexus_info->mapped=MagickFalse;
  nexus_info->cache=(PixelPacket *) MagickAssumeAligned(AcquireAlignedMemory(1,
    (size_t) nexus_info->length));
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

static PixelPacket *SetPixelCacheNexusPixels(const Image *image,
  const MapMode mode,const RectangleInfo *region,NexusInfo *nexus_info,
  ExceptionInfo *exception)
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
    return((PixelPacket *) NULL);
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
          nexus_info->pixels=cache_info->pixels+offset;
          nexus_info->indexes=(IndexPacket *) NULL;
          if (cache_info->active_index_channel != MagickFalse)
            nexus_info->indexes=cache_info->indexes+offset;
          PrefetchPixelCacheNexusPixels(nexus_info,mode);
          return(nexus_info->pixels);
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
        {
          nexus_info->length=0;
          return((PixelPacket *) NULL);
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
            return((PixelPacket *) NULL);
          }
      }
  nexus_info->pixels=nexus_info->cache;
  nexus_info->indexes=(IndexPacket *) NULL;
  if (cache_info->active_index_channel != MagickFalse)
    nexus_info->indexes=(IndexPacket *) (nexus_info->pixels+number_pixels);
  PrefetchPixelCacheNexusPixels(nexus_info,mode);
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

static MagickBooleanType SetCacheAlphaChannel(Image *image,
  const Quantum opacity)
{
  CacheInfo
    *cache_info;

  CacheView
    *image_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  image->matte=MagickTrue;
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,&image->exception);  /* must be virtual */
#if defined(MAGICKCORE_OPENMP_SUPPORT) && defined(NoBenefitFromParallelism)
  #pragma omp parallel for schedule(static,4) shared(status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
      &image->exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      q->opacity=opacity;
      q++;
    }
    status=SyncCacheViewAuthenticPixels(image_view,&image->exception);
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

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
  if ((image->columns != 0) && (image->rows != 0))
    switch (virtual_pixel_method)
    {
      case BackgroundVirtualPixelMethod:
      {
        if ((image->background_color.opacity != OpaqueOpacity) &&
            (image->matte == MagickFalse))
          (void) SetCacheAlphaChannel((Image *) image,OpaqueOpacity);
        if ((IsPixelGray(&image->background_color) == MagickFalse) &&
            (IsGrayColorspace(image->colorspace) != MagickFalse))
          (void) TransformImageColorspace((Image *) image,RGBColorspace);
        break;
      }
      case TransparentVirtualPixelMethod:
      {
        if (image->matte == MagickFalse)
          (void) SetCacheAlphaChannel((Image *) image,OpaqueOpacity);
        break;
      }
      default:
        break;
    }
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

  const int
    id = GetOpenMPThreadId();

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  assert(id < (int) cache_info->number_threads);
  return(SyncAuthenticPixelCacheNexus(image,cache_info->nexus_info[id],
    exception));
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

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.sync_authentic_pixels_handler !=
       (SyncAuthenticPixelsHandler) NULL)
    return(cache_info->methods.sync_authentic_pixels_handler(image,exception));
  assert(id < (int) cache_info->number_threads);
  return(SyncAuthenticPixelCacheNexus(image,cache_info->nexus_info[id],
    exception));
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
    *cache_info;

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
    extent,
    length;

  register const IndexPacket
    *restrict p;

  register ssize_t
    y;

  size_t
    rows;

  if (cache_info->active_index_channel == MagickFalse)
    return(MagickFalse);
  if (IsPixelAuthentic(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*sizeof(IndexPacket);
  rows=nexus_info->region.height;
  extent=(MagickSizeType) length*rows;
  p=nexus_info->indexes;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register IndexPacket
        *restrict q;

      /*
        Write indexes to memory.
      */
      if ((cache_info->columns == nexus_info->region.width) &&
          (extent == (MagickSizeType) ((size_t) extent)))
        {
          length=extent;
          rows=1UL;
        }
      q=cache_info->indexes+offset;
      for (y=0; y < (ssize_t) rows; y++)
      {
        (void) memcpy(q,p,(size_t) length);
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
          sizeof(PixelPacket)+offset*sizeof(*p),length,(const unsigned char *)
          p);
        if ((MagickSizeType) count < length)
          break;
        p+=nexus_info->region.width;
        offset+=cache_info->columns;
      }
      if (IsFileDescriptorLimitExceeded() != MagickFalse)
        (void) ClosePixelCacheOnDisk(cache_info);
      UnlockSemaphoreInfo(cache_info->file_semaphore);
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

  register const PixelPacket
    *restrict p;

  register ssize_t
    y;

  size_t
    rows;

  if (IsPixelAuthentic(cache_info,nexus_info) != MagickFalse)
    return(MagickTrue);
  offset=(MagickOffsetType) nexus_info->region.y*cache_info->columns+
    nexus_info->region.x;
  length=(MagickSizeType) nexus_info->region.width*sizeof(PixelPacket);
  rows=nexus_info->region.height;
  extent=length*rows;
  p=nexus_info->pixels;
  switch (cache_info->type)
  {
    case MemoryCache:
    case MapCache:
    {
      register PixelPacket
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
      q=cache_info->pixels+offset;
      for (y=0; y < (ssize_t) rows; y++)
      {
        (void) memcpy(q,p,(size_t) length);
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
          sizeof(*p),length,(const unsigned char *) p);
        if ((MagickSizeType) count < length)
          break;
        p+=nexus_info->region.width;
        offset+=cache_info->columns;
      }
      if (IsFileDescriptorLimitExceeded() != MagickFalse)
        (void) ClosePixelCacheOnDisk(cache_info);
      UnlockSemaphoreInfo(cache_info->file_semaphore);
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

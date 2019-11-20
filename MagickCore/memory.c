/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                    M   M  EEEEE  M   M   OOO   RRRR   Y   Y                 %
%                    MM MM  E      MM MM  O   O  R   R   Y Y                  %
%                    M M M  EEE    M M M  O   O  RRRR     Y                   %
%                    M   M  E      M   M  O   O  R R      Y                   %
%                    M   M  EEEEE  M   M   OOO   R  R     Y                   %
%                                                                             %
%                                                                             %
%                     MagickCore Memory Allocation Methods                    %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1998                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Segregate our memory requirements from any program that calls our API.  This
%  should help reduce the risk of others changing our program state or causing
%  memory corruption.
%
%  Our custom memory allocation manager implements a best-fit allocation policy
%  using segregated free lists.  It uses a linear distribution of size classes
%  for lower sizes and a power of two distribution of size classes at higher
%  sizes.  It is based on the paper, "Fast Memory Allocation using Lazy Fits."
%  written by Yoo C. Chung.
%
%  By default, C's standard library is used (e.g. malloc);  use the
%  custom memory allocator by defining MAGICKCORE_ANONYMOUS_MEMORY_SUPPORT
%  to allocate memory with private anonymous mapping rather than from the
%  heap.
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/policy.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/utility-private.h"

/*
  Define declarations.
*/
#define BlockFooter(block,size) \
  ((size_t *) ((char *) (block)+(size)-2*sizeof(size_t)))
#define BlockHeader(block)  ((size_t *) (block)-1)
#define BlockSize  4096
#define BlockThreshold  1024
#define MaxBlockExponent  16
#define MaxBlocks ((BlockThreshold/(4*sizeof(size_t)))+MaxBlockExponent+1)
#define MaxSegments  1024
#define MemoryGuard  ((0xdeadbeef << 31)+0xdeafdeed)
#define NextBlock(block)  ((char *) (block)+SizeOfBlock(block))
#define NextBlockInList(block)  (*(void **) (block))
#define PreviousBlock(block)  ((char *) (block)-(*((size_t *) (block)-2)))
#define PreviousBlockBit  0x01
#define PreviousBlockInList(block)  (*((void **) (block)+1))
#define SegmentSize  (2*1024*1024)
#define SizeMask  (~0x01)
#define SizeOfBlock(block)  (*BlockHeader(block) & SizeMask)

/*
  Typedef declarations.
*/
typedef enum
{
  UndefinedVirtualMemory,
  AlignedVirtualMemory,
  MapVirtualMemory,
  UnalignedVirtualMemory
} VirtualMemoryType;

typedef struct _DataSegmentInfo
{
  void
    *allocation,
    *bound;

  MagickBooleanType
    mapped;

  size_t
    length;

  struct _DataSegmentInfo
    *previous,
    *next;
} DataSegmentInfo;

typedef struct _MagickMemoryMethods
{
  AcquireMemoryHandler
    acquire_memory_handler;

  ResizeMemoryHandler
    resize_memory_handler;

  DestroyMemoryHandler
    destroy_memory_handler;

  AcquireAlignedMemoryHandler
    acquire_aligned_memory_handler;

  RelinquishAlignedMemoryHandler
    relinquish_aligned_memory_handler;
} MagickMemoryMethods;

struct _MemoryInfo
{
  char
    filename[MagickPathExtent];

  VirtualMemoryType
    type;

  size_t
    length;

  void
    *blob;

  size_t
    signature;
};

typedef struct _MemoryPool
{
  size_t
    allocation;

  void
    *blocks[MaxBlocks+1];

  size_t
    number_segments;

  DataSegmentInfo
    *segments[MaxSegments],
    segment_pool[MaxSegments];
} MemoryPool;

/*
  Global declarations.
*/
static size_t
  max_memory_request = 0,
  virtual_anonymous_memory = 0;

#if defined _MSC_VER
static void* MSCMalloc(size_t size)
{
  return malloc(size);
}
static void* MSCRealloc(void* ptr, size_t size)
{
  return realloc(ptr, size);
}
static void MSCFree(void* ptr)
{
  free(ptr);
}
#endif

static MagickMemoryMethods
  memory_methods =
  {
#if defined _MSC_VER
    (AcquireMemoryHandler) MSCMalloc,
    (ResizeMemoryHandler) MSCRealloc,
    (DestroyMemoryHandler) MSCFree,
#else
    (AcquireMemoryHandler) malloc,
    (ResizeMemoryHandler) realloc,
    (DestroyMemoryHandler) free,
#endif
    (AcquireAlignedMemoryHandler) NULL,
    (RelinquishAlignedMemoryHandler) NULL
  };
#if defined(MAGICKCORE_ANONYMOUS_MEMORY_SUPPORT)
static MemoryPool
  memory_pool;

static SemaphoreInfo
  *memory_semaphore = (SemaphoreInfo *) NULL;

static volatile DataSegmentInfo
  *free_segments = (DataSegmentInfo *) NULL;

/*
  Forward declarations.
*/
static MagickBooleanType
  ExpandHeap(size_t);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e A l i g n e d M e m o r y                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireAlignedMemory() returns a pointer to a block of memory whose size is
%  at least (count*quantum) bytes, and whose address is aligned on a cache line.
%
%  The format of the AcquireAlignedMemory method is:
%
%      void *AcquireAlignedMemory(const size_t count,const size_t quantum)
%
%  A description of each parameter follows:
%
%    o count: the number of objects to allocate contiguously.
%
%    o quantum: the size (in bytes) of each object.
%
*/
MagickExport void *AcquireAlignedMemory(const size_t count,const size_t quantum)
{
  size_t
    extent,
    size;

  void
    *memory;

  if (HeapOverflowSanityCheckGetSize(count,quantum,&size) != MagickFalse)
    return(NULL);
  extent=CACHE_ALIGNED(size);
  if (extent < size)
    {
      errno=ENOMEM;
      return(NULL);
    }
  if (memory_methods.acquire_aligned_memory_handler != (AcquireAlignedMemoryHandler) NULL)
    return(memory_methods.acquire_aligned_memory_handler(extent,CACHE_LINE_SIZE));
#if defined(MAGICKCORE_HAVE_POSIX_MEMALIGN)
  if (posix_memalign(&memory,CACHE_LINE_SIZE,extent) != 0)
    return(NULL);
  return(memory);
#elif defined(MAGICKCORE_HAVE__ALIGNED_MALLOC)
  return(_aligned_malloc(extent,CACHE_LINE_SIZE));
#else
  #define ALIGNMENT_OVERHEAD \
    (MAGICKCORE_MAX_ALIGNMENT_PADDING(CACHE_LINE_SIZE) + MAGICKCORE_SIZEOF_VOID_P)
  {
    void
      *p;

    #if SIZE_MAX < ALIGNMENT_OVERHEAD
      #error "CACHE_LINE_SIZE is way too big."
    #endif
    extent=(size + ALIGNMENT_OVERHEAD);
    if (extent <= size)
      {
        errno=ENOMEM;
        return(NULL);
      }
    p=AcquireMagickMemory(extent);
    if (p != NULL)
      {
        memory=(void *) CACHE_ALIGNED(((MagickAddressType) p) + sizeof(void *));
        *(((void **) memory)-1)=p;
        return(memory)
      }
    return(NULL);
  }
#endif
}

#if defined(MAGICKCORE_ANONYMOUS_MEMORY_SUPPORT)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e B l o c k                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireBlock() returns a pointer to a block of memory at least size bytes
%  suitably aligned for any use.
%
%  The format of the AcquireBlock method is:
%
%      void *AcquireBlock(const size_t size)
%
%  A description of each parameter follows:
%
%    o size: the size of the memory in bytes to allocate.
%
*/

static inline size_t AllocationPolicy(size_t size)
{
  register size_t
    blocksize;

  /*
    The linear distribution.
  */
  assert(size != 0);
  assert(size % (4*sizeof(size_t)) == 0);
  if (size <= BlockThreshold)
    return(size/(4*sizeof(size_t)));
  /*
    Check for the largest block size.
  */
  if (size > (size_t) (BlockThreshold*(1L << (MaxBlockExponent-1L))))
    return(MaxBlocks-1L);
  /*
    Otherwise use a power of two distribution.
  */
  blocksize=BlockThreshold/(4*sizeof(size_t));
  for ( ; size > BlockThreshold; size/=2)
    blocksize++;
  assert(blocksize > (BlockThreshold/(4*sizeof(size_t))));
  assert(blocksize < (MaxBlocks-1L));
  return(blocksize);
}

static inline void InsertFreeBlock(void *block,const size_t i)
{
  register void
    *next,
    *previous;

  size_t
    size;

  size=SizeOfBlock(block);
  previous=(void *) NULL;
  next=memory_pool.blocks[i];
  while ((next != (void *) NULL) && (SizeOfBlock(next) < size))
  {
    previous=next;
    next=NextBlockInList(next);
  }
  PreviousBlockInList(block)=previous;
  NextBlockInList(block)=next;
  if (previous != (void *) NULL)
    NextBlockInList(previous)=block;
  else
    memory_pool.blocks[i]=block;
  if (next != (void *) NULL)
    PreviousBlockInList(next)=block;
}

static inline void RemoveFreeBlock(void *block,const size_t i)
{
  register void
    *next,
    *previous;

  next=NextBlockInList(block);
  previous=PreviousBlockInList(block);
  if (previous == (void *) NULL)
    memory_pool.blocks[i]=next;
  else
    NextBlockInList(previous)=next;
  if (next != (void *) NULL)
    PreviousBlockInList(next)=previous;
}

static void *AcquireBlock(size_t size)
{
  register size_t
    i;

  register void
    *block;

  /*
    Find free block.
  */
  size=(size_t) (size+sizeof(size_t)+6*sizeof(size_t)-1) & -(4U*sizeof(size_t));
  i=AllocationPolicy(size);
  block=memory_pool.blocks[i];
  while ((block != (void *) NULL) && (SizeOfBlock(block) < size))
    block=NextBlockInList(block);
  if (block == (void *) NULL)
    {
      i++;
      while (memory_pool.blocks[i] == (void *) NULL)
        i++;
      block=memory_pool.blocks[i];
      if (i >= MaxBlocks)
        return((void *) NULL);
    }
  assert((*BlockHeader(NextBlock(block)) & PreviousBlockBit) == 0);
  assert(SizeOfBlock(block) >= size);
  RemoveFreeBlock(block,AllocationPolicy(SizeOfBlock(block)));
  if (SizeOfBlock(block) > size)
    {
      size_t
        blocksize;

      void
        *next;

      /*
        Split block.
      */
      next=(char *) block+size;
      blocksize=SizeOfBlock(block)-size;
      *BlockHeader(next)=blocksize;
      *BlockFooter(next,blocksize)=blocksize;
      InsertFreeBlock(next,AllocationPolicy(blocksize));
      *BlockHeader(block)=size | (*BlockHeader(block) & ~SizeMask);
    }
  assert(size == SizeOfBlock(block));
  *BlockHeader(NextBlock(block))|=PreviousBlockBit;
  memory_pool.allocation+=size;
  return(block);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e M a g i c k M e m o r y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireMagickMemory() returns a pointer to a block of memory at least size
%  bytes suitably aligned for any use.
%
%  The format of the AcquireMagickMemory method is:
%
%      void *AcquireMagickMemory(const size_t size)
%
%  A description of each parameter follows:
%
%    o size: the size of the memory in bytes to allocate.
%
*/
MagickExport void *AcquireMagickMemory(const size_t size)
{
  register void
    *memory;

#if !defined(MAGICKCORE_ANONYMOUS_MEMORY_SUPPORT)
  memory=memory_methods.acquire_memory_handler(size == 0 ? 1UL : size);
#else
  if (memory_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&memory_semaphore);
  if (free_segments == (DataSegmentInfo *) NULL)
    {
      LockSemaphoreInfo(memory_semaphore);
      if (free_segments == (DataSegmentInfo *) NULL)
        {
          register ssize_t
            i;

          assert(2*sizeof(size_t) > (size_t) (~SizeMask));
          (void) memset(&memory_pool,0,sizeof(memory_pool));
          memory_pool.allocation=SegmentSize;
          memory_pool.blocks[MaxBlocks]=(void *) (-1);
          for (i=0; i < MaxSegments; i++)
          {
            if (i != 0)
              memory_pool.segment_pool[i].previous=
                (&memory_pool.segment_pool[i-1]);
            if (i != (MaxSegments-1))
              memory_pool.segment_pool[i].next=(&memory_pool.segment_pool[i+1]);
          }
          free_segments=(&memory_pool.segment_pool[0]);
        }
      UnlockSemaphoreInfo(memory_semaphore);
    }
  LockSemaphoreInfo(memory_semaphore);
  memory=AcquireBlock(size == 0 ? 1UL : size);
  if (memory == (void *) NULL)
    {
      if (ExpandHeap(size == 0 ? 1UL : size) != MagickFalse)
        memory=AcquireBlock(size == 0 ? 1UL : size);
    }
  UnlockSemaphoreInfo(memory_semaphore);
#endif
  return(memory);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e C r i t i c a l M e m o r y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCriticalMemory() is just like AcquireMagickMemory(), throws a fatal
%  exception if the memory cannot be acquired.
%
%  That is, AcquireCriticalMemory() returns a pointer to a block of memory that
%  is at least size bytes, and that is suitably aligned for any use; however,
%  if this is not possible, it throws an exception and terminates the program
%  as unceremoniously as possible.
%
%  The format of the AcquireCriticalMemory method is:
%
%      void *AcquireCriticalMemory(const size_t size)
%
%  A description of each parameter follows:
%
%    o size: the size (in bytes) of the memory to allocate.
%
*/
MagickExport void *AcquireCriticalMemory(const size_t size)
{
  register void
    *memory;

  /*
    Fail if memory request cannot be fulfilled.
  */
  memory=AcquireMagickMemory(size);
  if (memory == (void *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  return(memory);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e Q u a n t u m M e m o r y                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireQuantumMemory() returns a pointer to a block of memory at least
%  count * quantum bytes suitably aligned for any use.
%
%  The format of the AcquireQuantumMemory method is:
%
%      void *AcquireQuantumMemory(const size_t count,const size_t quantum)
%
%  A description of each parameter follows:
%
%    o count: the number of objects to allocate contiguously.
%
%    o quantum: the size (in bytes) of each object.
%
*/
MagickExport void *AcquireQuantumMemory(const size_t count,const size_t quantum)
{
  size_t
    size;

  if (HeapOverflowSanityCheckGetSize(count,quantum,&size) != MagickFalse)
    return((void *) NULL);
  return(AcquireMagickMemory(size));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e V i r t u a l M e m o r y                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireVirtualMemory() allocates a pointer to a block of memory at least
%  size bytes suitably aligned for any use. In addition to heap, it also
%  supports memory-mapped and file-based memory-mapped memory requests.
%
%  The format of the AcquireVirtualMemory method is:
%
%      MemoryInfo *AcquireVirtualMemory(const size_t count,const size_t quantum)
%
%  A description of each parameter follows:
%
%    o count: the number of objects to allocate contiguously.
%
%    o quantum: the size (in bytes) of each object.
%
*/
MagickExport MemoryInfo *AcquireVirtualMemory(const size_t count,
  const size_t quantum)
{
  char
    *value;

  MemoryInfo
    *memory_info;

  size_t
    size;

  if (HeapOverflowSanityCheckGetSize(count,quantum,&size) != MagickFalse)
    return((MemoryInfo *) NULL);
  if (virtual_anonymous_memory == 0)
    {
      virtual_anonymous_memory=1;
      value=GetPolicyValue("system:memory-map");
      if (LocaleCompare(value,"anonymous") == 0)
        {
          /*
            The security policy sets anonymous mapping for the memory request.
          */
#if defined(MAGICKCORE_HAVE_MMAP) && defined(MAP_ANONYMOUS)
          virtual_anonymous_memory=2;
#endif
        }
      value=DestroyString(value);
    }
  memory_info=(MemoryInfo *) MagickAssumeAligned(AcquireAlignedMemory(1,
    sizeof(*memory_info)));
  if (memory_info == (MemoryInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(memory_info,0,sizeof(*memory_info));
  memory_info->length=size;
  memory_info->signature=MagickCoreSignature;
  if ((virtual_anonymous_memory == 1) &&
      (size <= GetMaxMemoryRequest()))
    {
      memory_info->blob=AcquireAlignedMemory(1,size);
      if (memory_info->blob != NULL)
        memory_info->type=AlignedVirtualMemory;
    }
  if (memory_info->blob == NULL)
    {
      /*
        Acquire anonymous memory map.
      */
      memory_info->blob=NULL;
      if (size <= GetMaxMemoryRequest())
        memory_info->blob=MapBlob(-1,IOMode,0,size);
      if (memory_info->blob != NULL)
        memory_info->type=MapVirtualMemory;
      else
        {
          int
            file;

          /*
            Anonymous memory mapping failed, try file-backed memory mapping.
          */
          file=AcquireUniqueFileResource(memory_info->filename);
          if (file != -1)
            {
              MagickOffsetType
                offset;

              offset=(MagickOffsetType) lseek(file,size-1,SEEK_SET);
              if ((offset == (MagickOffsetType) (size-1)) &&
                  (write(file,"",1) == 1))
                {
MAGICKCORE_DIAGNOSTIC_PUSH()
MAGICKCORE_DIAGNOSTIC_IGNORE_MAYBE_UNINITIALIZED()
#if !defined(MAGICKCORE_HAVE_POSIX_FALLOCATE)
                  memory_info->blob=MapBlob(file,IOMode,0,size);
#else
                  if (posix_fallocate(file,0,size) == 0)
                    memory_info->blob=MapBlob(file,IOMode,0,size);
#endif
MAGICKCORE_DIAGNOSTIC_POP()
                  if (memory_info->blob != NULL)
                    memory_info->type=MapVirtualMemory;
                  else
                    {
                      (void) RelinquishUniqueFileResource(
                        memory_info->filename);
                      *memory_info->filename='\0';
                    }
                }
              (void) close(file);
            }
        }
    }
  if (memory_info->blob == NULL)
    {
      memory_info->blob=AcquireQuantumMemory(1,size);
      if (memory_info->blob != NULL)
        memory_info->type=UnalignedVirtualMemory;
    }
  if (memory_info->blob == NULL)
    memory_info=RelinquishVirtualMemory(memory_info);
  return(memory_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o p y M a g i c k M e m o r y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CopyMagickMemory() copies size bytes from memory area source to the
%  destination.  Copying between objects that overlap will take place
%  correctly.  It returns destination.
%
%  The format of the CopyMagickMemory method is:
%
%      void *CopyMagickMemory(void *magick_restrict destination,
%        const void *magick_restrict source,const size_t size)
%
%  A description of each parameter follows:
%
%    o destination: the destination.
%
%    o source: the source.
%
%    o size: the size of the memory in bytes to allocate.
%
*/
MagickExport void *CopyMagickMemory(void *magick_restrict destination,
  const void *magick_restrict source,const size_t size)
{
  register const unsigned char
    *p;

  register unsigned char
    *q;

  assert(destination != (void *) NULL);
  assert(source != (const void *) NULL);
  p=(const unsigned char *) source;
  q=(unsigned char *) destination;
  if (((q+size) < p) || (q > (p+size)))
    switch (size)
    {
      default: return(memcpy(destination,source,size));
      case 8: *q++=(*p++);
      case 7: *q++=(*p++);
      case 6: *q++=(*p++);
      case 5: *q++=(*p++);
      case 4: *q++=(*p++);
      case 3: *q++=(*p++);
      case 2: *q++=(*p++);
      case 1: *q++=(*p++);
      case 0: return(destination);
    }
  return(memmove(destination,source,size));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y M a g i c k M e m o r y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyMagickMemory() deallocates memory associated with the memory manager.
%
%  The format of the DestroyMagickMemory method is:
%
%      DestroyMagickMemory(void)
%
*/
MagickExport void DestroyMagickMemory(void)
{
#if defined(MAGICKCORE_ANONYMOUS_MEMORY_SUPPORT)
  register ssize_t
    i;

  if (memory_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&memory_semaphore);
  LockSemaphoreInfo(memory_semaphore);
  for (i=0; i < (ssize_t) memory_pool.number_segments; i++)
    if (memory_pool.segments[i]->mapped == MagickFalse)
      memory_methods.destroy_memory_handler(
        memory_pool.segments[i]->allocation);
    else
      (void) UnmapBlob(memory_pool.segments[i]->allocation,
        memory_pool.segments[i]->length);
  free_segments=(DataSegmentInfo *) NULL;
  (void) memset(&memory_pool,0,sizeof(memory_pool));
  UnlockSemaphoreInfo(memory_semaphore);
  RelinquishSemaphoreInfo(&memory_semaphore);
#endif
}

#if defined(MAGICKCORE_ANONYMOUS_MEMORY_SUPPORT)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E x p a n d H e a p                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExpandHeap() get more memory from the system.  It returns MagickTrue on
%  success otherwise MagickFalse.
%
%  The format of the ExpandHeap method is:
%
%      MagickBooleanType ExpandHeap(size_t size)
%
%  A description of each parameter follows:
%
%    o size: the size of the memory in bytes we require.
%
*/
static MagickBooleanType ExpandHeap(size_t size)
{
  DataSegmentInfo
    *segment_info;

  MagickBooleanType
    mapped;

  register ssize_t
    i;

  register void
    *block;

  size_t
    blocksize;

  void
    *segment;

  blocksize=((size+12*sizeof(size_t))+SegmentSize-1) & -SegmentSize;
  assert(memory_pool.number_segments < MaxSegments);
  segment=MapBlob(-1,IOMode,0,blocksize);
  mapped=segment != (void *) NULL ? MagickTrue : MagickFalse;
  if (segment == (void *) NULL)
    segment=(void *) memory_methods.acquire_memory_handler(blocksize);
  if (segment == (void *) NULL)
    return(MagickFalse);
  segment_info=(DataSegmentInfo *) free_segments;
  free_segments=segment_info->next;
  segment_info->mapped=mapped;
  segment_info->length=blocksize;
  segment_info->allocation=segment;
  segment_info->bound=(char *) segment+blocksize;
  i=(ssize_t) memory_pool.number_segments-1;
  for ( ; (i >= 0) && (memory_pool.segments[i]->allocation > segment); i--)
    memory_pool.segments[i+1]=memory_pool.segments[i];
  memory_pool.segments[i+1]=segment_info;
  memory_pool.number_segments++;
  size=blocksize-12*sizeof(size_t);
  block=(char *) segment_info->allocation+4*sizeof(size_t);
  *BlockHeader(block)=size | PreviousBlockBit;
  *BlockFooter(block,size)=size;
  InsertFreeBlock(block,AllocationPolicy(size));
  block=NextBlock(block);
  assert(block < segment_info->bound);
  *BlockHeader(block)=2*sizeof(size_t);
  *BlockHeader(NextBlock(block))=PreviousBlockBit;
  return(MagickTrue);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k M e m o r y M e t h o d s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickMemoryMethods() gets the methods to acquire, resize, and destroy
%  memory.
%
%  The format of the GetMagickMemoryMethods() method is:
%
%      void GetMagickMemoryMethods(AcquireMemoryHandler *acquire_memory_handler,
%        ResizeMemoryHandler *resize_memory_handler,
%        DestroyMemoryHandler *destroy_memory_handler)
%
%  A description of each parameter follows:
%
%    o acquire_memory_handler: method to acquire memory (e.g. malloc).
%
%    o resize_memory_handler: method to resize memory (e.g. realloc).
%
%    o destroy_memory_handler: method to destroy memory (e.g. free).
%
*/
MagickExport void GetMagickMemoryMethods(
  AcquireMemoryHandler *acquire_memory_handler,
  ResizeMemoryHandler *resize_memory_handler,
  DestroyMemoryHandler *destroy_memory_handler)
{
  assert(acquire_memory_handler != (AcquireMemoryHandler *) NULL);
  assert(resize_memory_handler != (ResizeMemoryHandler *) NULL);
  assert(destroy_memory_handler != (DestroyMemoryHandler *) NULL);
  *acquire_memory_handler=memory_methods.acquire_memory_handler;
  *resize_memory_handler=memory_methods.resize_memory_handler;
  *destroy_memory_handler=memory_methods.destroy_memory_handler;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a x M e m o r y R e q u e s t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMaxMemoryRequest() returns the max_memory_request value.
%
%  The format of the GetMaxMemoryRequest method is:
%
%      size_t GetMaxMemoryRequest(void)
%
*/
MagickExport size_t GetMaxMemoryRequest(void)
{
  if (max_memory_request == 0)
    {
      char
        *value;

      value=GetPolicyValue("system:max-memory-request");
      if (value != (char *) NULL)
        {
          /*
            The security policy sets a max memory request limit.
          */
          max_memory_request=StringToSizeType(value,100.0);
          value=DestroyString(value);
        }
      else
        max_memory_request=(size_t) MagickULLConstant(~0);
    }
  return(max_memory_request);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t V i r t u a l M e m o r y B l o b                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualMemoryBlob() returns the virtual memory blob associated with the
%  specified MemoryInfo structure.
%
%  The format of the GetVirtualMemoryBlob method is:
%
%      void *GetVirtualMemoryBlob(const MemoryInfo *memory_info)
%
%  A description of each parameter follows:
%
%    o memory_info: The MemoryInfo structure.
*/
MagickExport void *GetVirtualMemoryBlob(const MemoryInfo *memory_info)
{
  assert(memory_info != (const MemoryInfo *) NULL);
  assert(memory_info->signature == MagickCoreSignature);
  return(memory_info->blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   H e a p O v e r f l o w S a n i t y C h e c k                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  HeapOverflowSanityCheck() returns MagickFalse if the heap allocation request
%  is not zero and fits within the maximum limits of size_t. Otherwise, it
%  returns a value other than MagickFalse; if the heap allocation request does
%  not fit within the maximum limts of size_t, then errno is set to ENOMEM.
%
%  The format of the HeapOverflowSanityCheck method is:
%
%      MagickBooleanType HeapOverflowSanityCheck(const size_t count,
%        const size_t quantum)
%
%  A description of each parameter follows:
%
%    o count: the number of objects to allocate contiguously.
%
%    o quantum: the size (in bytes) of each object.
%
*/
extern MagickExport MagickBooleanType
  HeapOverflowSanityCheck(const size_t count,const size_t quantum);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   H e a p O v e r f l o w S a n i t y C h e c k G e t S i z e               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  HeapOverflowSanityCheckGetSize() returns MagickFalse if the heap allocation
%  request is not zero and fits within the maximum limits of size_t; in this
%  case, the value (count*quantum) is returned in the size_t object to which
%  points the parameter named "size".
%
%  Otherwise, it returns a value other than MagickFalse; if the heap allocation
%  request does not fit within the maximum limits of size_t, then errno is set
%  to ENOMEM. In this case, the object referenced by "size" is not modified.
%
%
%  The format of the HeapOverflowSanityCheckGetSize method is:
%
%      MagickBooleanType HeapOverflowSanityCheckGetSize
%      (
%        const size_t count,
%        const size_t quantum,
%        size_t *const size
%      )
%
%  A description of each parameter follows:
%
%    o count: the number of objects to allocate contiguously.
%
%    o quantum: the size (in bytes) of each object.
%
%    o size: a pointer to an object to hold (count*quantum) if all goes well.
%
*/
extern MagickExport MagickBooleanType
  HeapOverflowSanityCheckGetSize(const size_t count,const size_t quantum,size_t *const size);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h A l i g n e d M e m o r y                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishAlignedMemory() frees memory acquired with AcquireAlignedMemory()
%  or reuse.
%
%  The format of the RelinquishAlignedMemory method is:
%
%      void *RelinquishAlignedMemory(void *memory)
%
%  A description of each parameter follows:
%
%    o memory: A pointer to a block of memory to free for reuse.
%
*/
MagickExport void *RelinquishAlignedMemory(void *memory)
{
  if (memory == (void *) NULL)
    return((void *) NULL);
  if (memory_methods.relinquish_aligned_memory_handler != (RelinquishAlignedMemoryHandler) NULL)
    {
      memory_methods.relinquish_aligned_memory_handler(memory);
      return(NULL);
    }
#if defined(MAGICKCORE_HAVE_POSIX_MEMALIGN)
  free(memory);
#elif defined(MAGICKCORE_HAVE__ALIGNED_MALLOC)
  _aligned_free(memory);
#else
  RelinquishMagickMemory(*((void **) memory-1));
#endif
  return(NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h M a g i c k M e m o r y                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishMagickMemory() frees memory acquired with AcquireMagickMemory()
%  or AcquireQuantumMemory() for reuse.
%
%  The format of the RelinquishMagickMemory method is:
%
%      void *RelinquishMagickMemory(void *memory)
%
%  A description of each parameter follows:
%
%    o memory: A pointer to a block of memory to free for reuse.
%
*/
MagickExport void *RelinquishMagickMemory(void *memory)
{
  if (memory == (void *) NULL)
    return((void *) NULL);
#if !defined(MAGICKCORE_ANONYMOUS_MEMORY_SUPPORT)
  memory_methods.destroy_memory_handler(memory);
#else
  LockSemaphoreInfo(memory_semaphore);
  assert((SizeOfBlock(memory) % (4*sizeof(size_t))) == 0);
  assert((*BlockHeader(NextBlock(memory)) & PreviousBlockBit) != 0);
  if ((*BlockHeader(memory) & PreviousBlockBit) == 0)
    {
      void
        *previous;

      /*
        Coalesce with previous adjacent block.
      */
      previous=PreviousBlock(memory);
      RemoveFreeBlock(previous,AllocationPolicy(SizeOfBlock(previous)));
      *BlockHeader(previous)=(SizeOfBlock(previous)+SizeOfBlock(memory)) |
        (*BlockHeader(previous) & ~SizeMask);
      memory=previous;
    }
  if ((*BlockHeader(NextBlock(NextBlock(memory))) & PreviousBlockBit) == 0)
    {
      void
        *next;

      /*
        Coalesce with next adjacent block.
      */
      next=NextBlock(memory);
      RemoveFreeBlock(next,AllocationPolicy(SizeOfBlock(next)));
      *BlockHeader(memory)=(SizeOfBlock(memory)+SizeOfBlock(next)) |
        (*BlockHeader(memory) & ~SizeMask);
    }
  *BlockFooter(memory,SizeOfBlock(memory))=SizeOfBlock(memory);
  *BlockHeader(NextBlock(memory))&=(~PreviousBlockBit);
  InsertFreeBlock(memory,AllocationPolicy(SizeOfBlock(memory)));
  UnlockSemaphoreInfo(memory_semaphore);
#endif
  return((void *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h V i r t u a l M e m o r y                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishVirtualMemory() frees memory acquired with AcquireVirtualMemory().
%
%  The format of the RelinquishVirtualMemory method is:
%
%      MemoryInfo *RelinquishVirtualMemory(MemoryInfo *memory_info)
%
%  A description of each parameter follows:
%
%    o memory_info: A pointer to a block of memory to free for reuse.
%
*/
MagickExport MemoryInfo *RelinquishVirtualMemory(MemoryInfo *memory_info)
{
  assert(memory_info != (MemoryInfo *) NULL);
  assert(memory_info->signature == MagickCoreSignature);
  if (memory_info->blob != (void *) NULL)
    switch (memory_info->type)
    {
      case AlignedVirtualMemory:
      {
        memory_info->blob=RelinquishAlignedMemory(memory_info->blob);
        break;
      }
      case MapVirtualMemory:
      {
        (void) UnmapBlob(memory_info->blob,memory_info->length);
        memory_info->blob=NULL;
        if (*memory_info->filename != '\0')
          (void) RelinquishUniqueFileResource(memory_info->filename);
        break;
      }
      case UnalignedVirtualMemory:
      default:
      {
        memory_info->blob=RelinquishMagickMemory(memory_info->blob);
        break;
      }
    }
  memory_info->signature=(~MagickCoreSignature);
  memory_info=(MemoryInfo *) RelinquishAlignedMemory(memory_info);
  return(memory_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t M a g i c k M e m o r y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetMagickMemory() fills the first size bytes of the memory area pointed to
%  by memory with the constant byte c.
%
%  The format of the ResetMagickMemory method is:
%
%      void *ResetMagickMemory(void *memory,int byte,const size_t size)
%
%  A description of each parameter follows:
%
%    o memory: a pointer to a memory allocation.
%
%    o byte: set the memory to this value.
%
%    o size: size of the memory to reset.
%
*/
MagickExport void *ResetMagickMemory(void *memory,int byte,const size_t size)
{
  assert(memory != (void *) NULL);
  return(memset(memory,byte,size));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e s e t M a x M e m o r y R e q u e s t                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetMaxMemoryRequest() resets the max_memory_request value.
%
%  The format of the ResetMaxMemoryRequest method is:
%
%      void ResetMaxMemoryRequest(void)
%
*/
MagickPrivate void ResetMaxMemoryRequest(void)
{
  max_memory_request=0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e s e t V i r t u a l A n o n y m o u s M e m o r y                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetVirtualAnonymousMemory() resets the virtual_anonymous_memory value.
%
%  The format of the ResetVirtualAnonymousMemory method is:
%
%      void ResetVirtualAnonymousMemory(void)
%
*/
MagickPrivate void ResetVirtualAnonymousMemory(void)
{
  virtual_anonymous_memory=0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s i z e M a g i c k M e m o r y                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResizeMagickMemory() changes the size of the memory and returns a pointer to
%  the (possibly moved) block.  The contents will be unchanged up to the
%  lesser of the new and old sizes.
%
%  The format of the ResizeMagickMemory method is:
%
%      void *ResizeMagickMemory(void *memory,const size_t size)
%
%  A description of each parameter follows:
%
%    o memory: A pointer to a memory allocation.
%
%    o size: the new size of the allocated memory.
%
*/

#if defined(MAGICKCORE_ANONYMOUS_MEMORY_SUPPORT)
static inline void *ResizeBlock(void *block,size_t size)
{
  register void
    *memory;

  if (block == (void *) NULL)
    return(AcquireBlock(size));
  memory=AcquireBlock(size);
  if (memory == (void *) NULL)
    return((void *) NULL);
  if (size <= (SizeOfBlock(block)-sizeof(size_t)))
    (void) memcpy(memory,block,size);
  else
    (void) memcpy(memory,block,SizeOfBlock(block)-sizeof(size_t));
  memory_pool.allocation+=size;
  return(memory);
}
#endif

MagickExport void *ResizeMagickMemory(void *memory,const size_t size)
{
  register void
    *block;

  if (memory == (void *) NULL)
    return(AcquireMagickMemory(size));
#if !defined(MAGICKCORE_ANONYMOUS_MEMORY_SUPPORT)
  block=memory_methods.resize_memory_handler(memory,size == 0 ? 1UL : size);
  if (block == (void *) NULL)
    memory=RelinquishMagickMemory(memory);
#else
  LockSemaphoreInfo(memory_semaphore);
  block=ResizeBlock(memory,size == 0 ? 1UL : size);
  if (block == (void *) NULL)
    {
      if (ExpandHeap(size == 0 ? 1UL : size) == MagickFalse)
        {
          UnlockSemaphoreInfo(memory_semaphore);
          memory=RelinquishMagickMemory(memory);
          ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
        }
      block=ResizeBlock(memory,size == 0 ? 1UL : size);
      assert(block != (void *) NULL);
    }
  UnlockSemaphoreInfo(memory_semaphore);
  memory=RelinquishMagickMemory(memory);
#endif
  return(block);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s i z e Q u a n t u m M e m o r y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResizeQuantumMemory() changes the size of the memory and returns a pointer
%  to the (possibly moved) block.  The contents will be unchanged up to the
%  lesser of the new and old sizes.
%
%  The format of the ResizeQuantumMemory method is:
%
%      void *ResizeQuantumMemory(void *memory,const size_t count,
%        const size_t quantum)
%
%  A description of each parameter follows:
%
%    o memory: A pointer to a memory allocation.
%
%    o count: the number of objects to allocate contiguously.
%
%    o quantum: the size (in bytes) of each object.
%
*/
MagickExport void *ResizeQuantumMemory(void *memory,const size_t count,
  const size_t quantum)
{
  size_t
    size;

  if (HeapOverflowSanityCheckGetSize(count,quantum,&size) != MagickFalse)
    {
      memory=RelinquishMagickMemory(memory);
      return((void *) NULL);
    }
  return(ResizeMagickMemory(memory,size));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t M a g i c k A l i g n e d M e m o r y M e t h o d s                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetMagickAlignedMemoryMethods() sets the methods to acquire and relinquish
%  aligned memory.
%
%  The format of the SetMagickAlignedMemoryMethods() method is:
%
%      SetMagickAlignedMemoryMethods(
%        AcquireAlignedMemoryHandler acquire_aligned_memory_handler,
%        RelinquishAlignedMemoryHandler relinquish_aligned_memory_handler)
%
%  A description of each parameter follows:
%
%    o acquire_memory_handler: method to acquire aligned memory.
%
%    o relinquish_aligned_memory_handler: method to relinquish aligned memory.
%
*/
MagickExport void SetMagickAlignedMemoryMethods(
  AcquireAlignedMemoryHandler acquire_aligned_memory_handler,
  RelinquishAlignedMemoryHandler relinquish_aligned_memory_handler)
{
  memory_methods.acquire_aligned_memory_handler=acquire_aligned_memory_handler;
  memory_methods.relinquish_aligned_memory_handler=
      relinquish_aligned_memory_handler;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t M a g i c k M e m o r y M e t h o d s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetMagickMemoryMethods() sets the methods to acquire, resize, and destroy
%  memory. Your custom memory methods must be set prior to the
%  MagickCoreGenesis() method.
%
%  The format of the SetMagickMemoryMethods() method is:
%
%      SetMagickMemoryMethods(AcquireMemoryHandler acquire_memory_handler,
%        ResizeMemoryHandler resize_memory_handler,
%        DestroyMemoryHandler destroy_memory_handler)
%
%  A description of each parameter follows:
%
%    o acquire_memory_handler: method to acquire memory (e.g. malloc).
%
%    o resize_memory_handler: method to resize memory (e.g. realloc).
%
%    o destroy_memory_handler: method to destroy memory (e.g. free).
%
*/
MagickExport void SetMagickMemoryMethods(
  AcquireMemoryHandler acquire_memory_handler,
  ResizeMemoryHandler resize_memory_handler,
  DestroyMemoryHandler destroy_memory_handler)
{
  /*
    Set memory methods.
  */
  if (acquire_memory_handler != (AcquireMemoryHandler) NULL)
    memory_methods.acquire_memory_handler=acquire_memory_handler;
  if (resize_memory_handler != (ResizeMemoryHandler) NULL)
    memory_methods.resize_memory_handler=resize_memory_handler;
  if (destroy_memory_handler != (DestroyMemoryHandler) NULL)
    memory_methods.destroy_memory_handler=destroy_memory_handler;
}

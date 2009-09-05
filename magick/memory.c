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
%                                John Cristy                                  %
%                                 July 1998                                   %
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
%  By default, ANSI memory methods are called (e.g. malloc).  Use the
%  custom memory allocator by defining MAGICKCORE_EMBEDDABLE_SUPPORT
%  to allocate memory with private anonymous mapping rather than from the
%  heap.
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/memory_.h"
#include "magick/semaphore.h"
#include "magick/string_.h"

/*
  Define declarations.
*/
#define BlockFooter(block,size) \
  ((size_t *) ((char *) (block)+(size)-2*sizeof(size_t)))
#define BlockHeader(block)  ((size_t *) (block)-1)
#define BlockSize  4096
#define BlockThreshold  1024
#define AlignedSize  (16*sizeof(void *))
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

typedef struct _MemoryInfo
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
} MemoryInfo;

typedef struct _MagickMemoryMethods
{
  AcquireMemoryHandler
    acquire_memory_handler;

  ResizeMemoryHandler
    resize_memory_handler;

  DestroyMemoryHandler
    destroy_memory_handler;
} MagickMemoryMethods;


/*
  Global declarations.
*/
static MagickMemoryMethods
  memory_methods = { malloc, realloc, free };

#if defined(MAGICKCORE_EMBEDDABLE_SUPPORT)
static MemoryInfo
  memory_info;

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
%  AcquireAlignedMemory() returns a pointer to a block of memory at least size
%  bytes whose address is a multiple of 16*sizeof(void *).
%
%  The format of the AcquireAlignedMemory method is:
%
%      void *AcquireAlignedMemory(const size_t count,const size_t quantum)
%
%  A description of each parameter follows:
%
%    o count: the number of quantum elements to allocate.
%
%    o quantum: the number of bytes in each quantum.
%
*/
MagickExport void *AcquireAlignedMemory(const size_t count,const size_t quantum)
{
  size_t
    size;

  size=count*quantum;
  if ((count == 0) || (quantum != (size/count)))
    {
      errno=ENOMEM;
      return((void *) NULL);
    }
#if defined(MAGICKCORE_HAVE_POSIX_MEMALIGN)
  {
    void
      *memory;

    if (posix_memalign(&memory,AlignedSize,size) == 0)
      return(memory);
  }
#endif
  return(malloc(size));
}

#if defined(MAGICKCORE_EMBEDDABLE_SUPPORT)
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
  next=memory_info.blocks[i];
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
    memory_info.blocks[i]=block;
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
    memory_info.blocks[i]=next;
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
  block=memory_info.blocks[i];
  while ((block != (void *) NULL) && (SizeOfBlock(block) < size))
    block=NextBlockInList(block);
  if (block == (void *) NULL)
    {
      i++;
      while (memory_info.blocks[i] == (void *) NULL)
        i++;
      block=memory_info.blocks[i];
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
  memory_info.allocation+=size;
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

#if !defined(MAGICKCORE_EMBEDDABLE_SUPPORT)
  memory=memory_methods.acquire_memory_handler(size == 0 ? 1UL : size);
#else
  if (free_segments == (DataSegmentInfo *) NULL)
    {
      AcquireSemaphoreInfo(&memory_semaphore);
      if (free_segments == (DataSegmentInfo *) NULL)
        {
          register long
            i;

          assert(2*sizeof(size_t) > (size_t) (~SizeMask));
          (void) ResetMagickMemory(&memory_info,0,sizeof(memory_info));
          memory_info.allocation=SegmentSize;
          memory_info.blocks[MaxBlocks]=(void *) (-1);
          for (i=0; i < MaxSegments; i++)
          {
            if (i != 0)
              memory_info.segment_pool[i].previous=
                (&memory_info.segment_pool[i-1]);
            if (i != (MaxSegments-1))
              memory_info.segment_pool[i].next=(&memory_info.segment_pool[i+1]);
          }
          free_segments=(&memory_info.segment_pool[0]);
        }
      RelinquishSemaphoreInfo(memory_semaphore);
    }
  AcquireSemaphoreInfo(&memory_semaphore);
  memory=AcquireBlock(size == 0 ? 1UL : size);
  if (memory == (void *) NULL)
    {
      if (ExpandHeap(size == 0 ? 1UL : size) != MagickFalse)
        memory=AcquireBlock(size == 0 ? 1UL : size);
    }
  RelinquishSemaphoreInfo(memory_semaphore);
#endif
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
%    o count: the number of quantum elements to allocate.
%
%    o quantum: the number of bytes in each quantum.
%
*/
MagickExport void *AcquireQuantumMemory(const size_t count,const size_t quantum)
{
  size_t
    size;

  size=count*quantum;
  if ((count == 0) || (quantum != (size/count)))
    {
      errno=ENOMEM;
      return((void *) NULL);
    }
  return(AcquireMagickMemory(size));
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
%      void *CopyMagickMemory(void *destination,const void *source,
%        const size_t size)
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
MagickExport void *CopyMagickMemory(void *destination,const void *source,
  const size_t size)
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
#if defined(MAGICKCORE_EMBEDDABLE_SUPPORT)
  register long
    i;

  AcquireSemaphoreInfo(&memory_semaphore);
  RelinquishSemaphoreInfo(memory_semaphore);
  for (i=0; i < (long) memory_info.number_segments; i++)
    if (memory_info.segments[i]->mapped == MagickFalse)
      memory_methods.destroy_memory_handler(
        memory_info.segments[i]->allocation);
    else
      (void) UnmapBlob(memory_info.segments[i]->allocation,
        memory_info.segments[i]->length);
  free_segments=(DataSegmentInfo *) NULL;
  (void) ResetMagickMemory(&memory_info,0,sizeof(memory_info));
  DestroySemaphoreInfo(&memory_semaphore);
#endif
}

#if defined(MAGICKCORE_EMBEDDABLE_SUPPORT)
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

  register long
    i;

  register void
    *block;

  size_t
    blocksize;

  void
    *segment;

  blocksize=((size+12*sizeof(size_t))+SegmentSize-1) & -SegmentSize;
  assert(memory_info.number_segments < MaxSegments);
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
  i=(long) memory_info.number_segments-1;
  for ( ; (i >= 0) && (memory_info.segments[i]->allocation > segment); i--)
    memory_info.segments[i+1]=memory_info.segments[i];
  memory_info.segments[i+1]=segment_info;
  memory_info.number_segments++;
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
  free(memory);
  return((void *) NULL);
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
#if !defined(MAGICKCORE_EMBEDDABLE_SUPPORT)
  memory_methods.destroy_memory_handler(memory);
#else
  assert((SizeOfBlock(memory) % (4*sizeof(size_t))) == 0);
  assert((*BlockHeader(NextBlock(memory)) & PreviousBlockBit) != 0);
  AcquireSemaphoreInfo(&memory_semaphore);
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
  RelinquishSemaphoreInfo(memory_semaphore);
#endif
  return((void *) NULL);
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
%    o memory: A pointer to a memory allocation.
%
%    o byte: Set the memory to this value.
%
%    o size: Size of the memory to reset.
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

#if defined(MAGICKCORE_EMBEDDABLE_SUPPORT)
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
  memory_info.allocation+=size;
  return(memory);
}
#endif

MagickExport void *ResizeMagickMemory(void *memory,const size_t size)
{
  register void
    *block;

  if (memory == (void *) NULL)
    return(AcquireMagickMemory(size));
#if !defined(MAGICKCORE_EMBEDDABLE_SUPPORT)
  block=memory_methods.resize_memory_handler(memory,size == 0 ? 1UL : size);
  if (block == (void *) NULL)
    memory=RelinquishMagickMemory(memory);
#else
  AcquireSemaphoreInfo(&memory_semaphore);
  block=ResizeBlock(memory,size == 0 ? 1UL : size);
  if (block == (void *) NULL)
    {
      if (ExpandHeap(size == 0 ? 1UL : size) == MagickFalse)
        {
          RelinquishSemaphoreInfo(memory_semaphore);
          memory=RelinquishMagickMemory(memory);
          ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
        }
      block=ResizeBlock(memory,size == 0 ? 1UL : size);
      assert(block != (void *) NULL);
    }
  RelinquishSemaphoreInfo(memory_semaphore);
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
%    o count: the number of quantum elements to allocate.
%
%    o quantum: the number of bytes in each quantum.
%
*/
MagickExport void *ResizeQuantumMemory(void *memory,const size_t count,
  const size_t quantum)
{
  size_t
    size;

  size=count*quantum;
  if ((count == 0) || (quantum != (size/count)))
    {
      memory=RelinquishMagickMemory(memory);
      errno=ENOMEM;
      return((void *) NULL);
    }
  return(ResizeMagickMemory(memory,size));
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
%  memory.
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

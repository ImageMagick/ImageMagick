/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                    M   M   AAA    GGGG  IIIII   CCCC                        %
%                    MM MM  A   A  G        I    C                            %
%                    M M M  AAAAA  G GGG    I    C                            %
%                    M   M  A   A  G   G    I    C                            %
%                    M   M  A   A   GGGG  IIIII   CCCC                        %
%                                                                             %
%                                                                             %
%                      MagickCore Image Magic Methods                         %
%                                                                             %
%                              Software Design                                %
%                              Bob Friesenhahn                                %
%                                 July 2000                                   %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/client.h"
#include "MagickCore/configure.h"
#include "MagickCore/configure-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/linked-list.h"
#include "MagickCore/linked-list-private.h"
#include "MagickCore/magic.h"
#include "MagickCore/magic-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "coders/coders.h"

/*
  Define declarations.
*/
#define AddMagickCoder(coder) Magick ## coder ## Headers

/*
  Typedef declarations.
*/
typedef struct _MagicMapInfo
{
  const char
    name[10];

  const MagickOffsetType
    offset;

  const unsigned char
    *const magic;

  const size_t
    length;

  const MagickBooleanType
    skip_spaces;
} MagicMapInfo;

struct _MagicInfo
{
  char
    *name;

  unsigned char
    *magic;

  size_t
    length;

  MagickOffsetType
    offset;

  MagickBooleanType
    skip_spaces;

  size_t
    signature;
};

/*
  Static declarations.
*/
static const MagicMapInfo
  MagicMap[] =
  {
    #include "coders/coders-list.h"
    MagickCoderHeader("CGM", 0, "BEGMF")
    MagickCoderHeader("FIG", 0, "#FIG")
    MagickCoderHeader("HPGL", 0, "IN;")
    MagickCoderHeader("ILBM", 8, "ILBM")
  };

static LinkedListInfo
  *magic_cache = (LinkedListInfo *) NULL,
  *magic_list = (LinkedListInfo *) NULL;

static SemaphoreInfo
  *magic_cache_semaphore = (SemaphoreInfo *) NULL,
  *magic_list_semaphore = (SemaphoreInfo *) NULL;

/*
  Forward declarations.
*/
static MagickBooleanType
  IsMagicListInstantiated(ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A c q u i r e M a g i c L i s t                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireMagicList() caches one or more magic configurations which provides a
%  mapping between magic attributes and a magic name.
%
%  The format of the AcquireMagicList method is:
%
%      LinkedListInfo *AcquireMagicList(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static int CompareMagickInfoExtent(const void *a,const void *b)
{
  MagicInfo
    *ma,
    *mb;

  MagickOffsetType
    delta;

  ma=(MagicInfo *) a;
  mb=(MagicInfo *) b;
  delta=(MagickOffsetType) mb->length-(MagickOffsetType) ma->length;
  if (ma->offset != mb->offset)
    {
      /*
        Offset is near the start? Let's search a bit further in the stream.
      */
      delta=ma->offset-mb->offset;
      if ((ma->offset > mb->offset ? ma->offset : mb->offset) <= 10)
        delta=mb->offset-ma->offset;
    }
  return(delta > INT_MAX ? 0 : (int) delta);
}

static LinkedListInfo *AcquireMagicList(ExceptionInfo *exception)
{
  LinkedListInfo
    *list;

  MagickStatusType
    status;

  ssize_t
    i;

  list=NewLinkedList(0);
  status=MagickTrue;
  /*
    Load built-in magic map.
  */
  for (i=0; i < (ssize_t) (sizeof(MagicMap)/sizeof(*MagicMap)); i++)
  {
    MagicInfo
      *magic_info;

    const MagicMapInfo
      *p;

    p=MagicMap+i;
    magic_info=(MagicInfo *) AcquireMagickMemory(sizeof(*magic_info));
    if (magic_info == (MagicInfo *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",p->name);
        continue;
      }
    (void) memset(magic_info,0,sizeof(*magic_info));
    magic_info->name=(char *) p->name;
    magic_info->offset=p->offset;
    magic_info->magic=(unsigned char *) p->magic;
    magic_info->length=p->length;
    magic_info->skip_spaces=p->skip_spaces;
    magic_info->signature=MagickCoreSignature;
    status&=(MagickStatusType) InsertValueInSortedLinkedList(list,
      CompareMagickInfoExtent,NULL,magic_info);
    if (status == MagickFalse)
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",magic_info->name);
  }
  return(list);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagicInfo() searches the magic list for the specified name and if found
%  returns attributes for that magic.
%
%  The format of the GetMagicInfo method is:
%
%      const MagicInfo *GetMagicInfo(const unsigned char *magic,
%        const size_t length,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o magic: A binary string generally representing the first few characters
%      of the image file or blob.
%
%    o length: the length of the binary signature.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline MagickBooleanType CompareMagic(const unsigned char *magic,
  const size_t length,const MagicInfo *magic_info)
{
  const unsigned char
    *q;

  MagickOffsetType
    remaining;

  assert(magic_info->offset >= 0);
  q=magic+magic_info->offset;
  remaining=(MagickOffsetType) length-magic_info->offset;
  if (magic_info->skip_spaces != MagickFalse)
    while ((remaining > 0) && (isspace(*q) != 0))
    {
      q++;
      remaining--;
    }
  if ((remaining >= (MagickOffsetType) magic_info->length) &&
      (memcmp(q,magic_info->magic,magic_info->length) == 0))
    return(MagickTrue);
  return(MagickFalse);
}

static MagickBooleanType IsMagicCacheInstantiated(void)
{
  if (magic_cache == (LinkedListInfo *) NULL)
    {
      if (magic_cache_semaphore == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&magic_cache_semaphore);
      LockSemaphoreInfo(magic_cache_semaphore);
      if (magic_cache == (LinkedListInfo *) NULL)
        magic_cache=NewLinkedList(0);
      UnlockSemaphoreInfo(magic_cache_semaphore);
    }
  return(magic_cache != (LinkedListInfo *) NULL ? MagickTrue : MagickFalse);
}

MagickExport const MagicInfo *GetMagicInfo(const unsigned char *magic,
  const size_t length,ExceptionInfo *exception)
{
  const MagicInfo *
    magic_info;

  ElementInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  if (IsMagicListInstantiated(exception) == MagickFalse)
    return((const MagicInfo *) NULL);
  if (IsMagicCacheInstantiated() == MagickFalse)
    return((const MagicInfo *) NULL);
  /*
    Search for cached entries.
  */
  magic_info=(const MagicInfo *) NULL;
  if (magic != (const unsigned char *) NULL)
    {
      LockSemaphoreInfo(magic_cache_semaphore);
      p=GetHeadElementInLinkedList(magic_cache);
      while (p != (ElementInfo *) NULL)
      {
        magic_info=(const MagicInfo *) p->value;
        if (CompareMagic(magic,length,magic_info) != MagickFalse)
          break;
        p=p->next;
      }
      UnlockSemaphoreInfo(magic_cache_semaphore);
      if (p != (ElementInfo *) NULL)
        return(magic_info);
    }
  /*
    Search for magic tag.
  */
  LockSemaphoreInfo(magic_list_semaphore);
  p=GetHeadElementInLinkedList(magic_list);
  if (magic == (const unsigned char *) NULL)
    {
      UnlockSemaphoreInfo(magic_list_semaphore);
      if (p != (ElementInfo *) NULL)
        magic_info=(const MagicInfo *) p->value;
      return(magic_info);
    }
  while (p != (ElementInfo *) NULL)
  {
    magic_info=(const MagicInfo *) p->value;
    if (CompareMagic(magic,length,magic_info) != MagickFalse)
      break;
    p=p->next;
  }
  UnlockSemaphoreInfo(magic_list_semaphore);
  if (p == (ElementInfo *) NULL)
    magic_info=(const MagicInfo *) NULL;
  else
    {
      LockSemaphoreInfo(magic_cache_semaphore);
      (void) InsertValueInSortedLinkedList(magic_cache,CompareMagickInfoExtent,
        NULL,magic_info);
      UnlockSemaphoreInfo(magic_cache_semaphore);
    }
  return(magic_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c P a t t e r n E x t e n t                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagicPatternExtent() returns the extent of the buffer that is
%  required to check all the MagickInfos. It returns zero if the list is empty.
%
%  The format of the GetMagicPatternExtent method is:
%
%      size_t GetMagicPatternExtent(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport size_t GetMagicPatternExtent(ExceptionInfo *exception)
{
  ElementInfo
    *p;

  MagickOffsetType
    max_offset,
    offset;

  static size_t
    extent = 0;

  assert(exception != (ExceptionInfo *) NULL);
  if ((extent != 0) || (IsMagicListInstantiated(exception) == MagickFalse))
    return(extent);
  LockSemaphoreInfo(magic_list_semaphore);
  p=GetHeadElementInLinkedList(magic_list);
  for (max_offset=0; p != (ElementInfo *) NULL; )
  {
    const MagicInfo
      *magic_info;

    magic_info=(const MagicInfo *) p->value;
    offset=magic_info->offset+(MagickOffsetType) magic_info->length;
    if (offset > max_offset)
      max_offset=offset;
    p=p->next;
  }
  UnlockSemaphoreInfo(magic_list_semaphore);
  if (max_offset > (MagickOffsetType) (MAGICK_SSIZE_MAX/2))
    return(0);
  extent=(size_t) max_offset;
  return(extent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c I n f o L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagicInfoList() returns any image aliases that match the specified
%  pattern.
%
%  The magic of the GetMagicInfoList function is:
%
%      const MagicInfo **GetMagicInfoList(const char *pattern,
%        size_t *number_aliases,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_aliases:  This integer returns the number of aliases in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int MagicInfoCompare(const void *x,const void *y)
{
  const MagicInfo
    **p,
    **q;

  p=(const MagicInfo **) x,
  q=(const MagicInfo **) y;
  return(LocaleCompare((*p)->name,(*q)->name));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport const MagicInfo **GetMagicInfoList(const char *pattern,
  size_t *number_aliases,ExceptionInfo *magick_unused(exception))
{
  const MagicInfo
    **aliases;

  ElementInfo
    *p;

  ssize_t
    i;

  magick_unreferenced(exception);
  assert(pattern != (char *) NULL);
  assert(number_aliases != (size_t *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  *number_aliases=0;
  if (IsMagicCacheInstantiated() == MagickFalse)
    return((const MagicInfo **) NULL);
  aliases=(const MagicInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(magic_list)+1UL,sizeof(*aliases));
  if (aliases == (const MagicInfo **) NULL)
    return((const MagicInfo **) NULL);
  LockSemaphoreInfo(magic_list_semaphore);
  p=GetHeadElementInLinkedList(magic_list);
  for (i=0; p != (ElementInfo *) NULL; )
  {
    const MagicInfo
      *magic_info;
    
    magic_info=(const MagicInfo *) p->value;
    if (GlobExpression(magic_info->name,pattern,MagickFalse) != MagickFalse)
      aliases[i++]=magic_info;
    p=p->next;
  }
  UnlockSemaphoreInfo(magic_list_semaphore);
  if (i == 0)
    aliases=(const MagicInfo **) RelinquishMagickMemory((void*) aliases);
  else
    {
      qsort((void *) aliases,(size_t) i,sizeof(*aliases),MagicInfoCompare);
      aliases[i]=(MagicInfo *) NULL;
    }
  *number_aliases=(size_t) i;
  return(aliases);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c L i s t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagicList() returns any image format aliases that match the specified
%  pattern.
%
%  The format of the GetMagicList function is:
%
%      char **GetMagicList(const char *pattern,size_t *number_aliases,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_aliases:  This integer returns the number of image format aliases
%      in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int MagicCompare(const void *x,const void *y)
{
  const char
    *p,
    *q;

  p=(const char *) x;
  q=(const char *) y;
  return(LocaleCompare(p,q));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport char **GetMagicList(const char *pattern,size_t *number_aliases,
  ExceptionInfo *magick_unused(exception))
{
  char
    **aliases;

  ElementInfo
    *p;

  ssize_t
    i;
  
  magick_unreferenced(exception);
  assert(pattern != (char *) NULL);
  assert(number_aliases != (size_t *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  *number_aliases=0;
  if (IsMagicCacheInstantiated() == MagickFalse)
    return((char **) NULL);
  aliases=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(magic_list)+1UL,sizeof(*aliases));
  if (aliases == (char **) NULL)
    return((char **) NULL);
  LockSemaphoreInfo(magic_list_semaphore);
  p=GetHeadElementInLinkedList(magic_list);
  for (i=0; p != (ElementInfo *) NULL; )
  {
    const MagicInfo
      *magic_info;

    magic_info=(const MagicInfo *) p->value;
    if (GlobExpression(magic_info->name,pattern,MagickFalse) != MagickFalse)
      aliases[i++]=ConstantString(magic_info->name);
    p=p->next;
  }
  UnlockSemaphoreInfo(magic_list_semaphore);
  if (i == 0)
    aliases=(char **) RelinquishMagickMemory(aliases);
  else
    {
      qsort((void *) aliases,(size_t) i,sizeof(*aliases),MagicCompare);
      aliases[i]=(char *) NULL;
    }
  *number_aliases=(size_t) i;
  return(aliases);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c N a m e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagicName() returns the name associated with the magic.
%
%  The format of the GetMagicName method is:
%
%      const char *GetMagicName(const MagicInfo *magic_info)
%
%  A description of each parameter follows:
%
%    o magic_info:  The magic info.
%
*/
MagickExport const char *GetMagicName(const MagicInfo *magic_info)
{
  assert(magic_info != (MagicInfo *) NULL);
  assert(magic_info->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  return(magic_info->name);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s M a g i c L i s t I n s t a n t i a t e d                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMagicListInstantiated() determines if the magic list is instantiated.
%  If not, it instantiates the list and returns it.
%
%  The format of the IsMagicListInstantiated method is:
%
%      MagickBooleanType IsMagicListInstantiated(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType IsMagicListInstantiated(ExceptionInfo *exception)
{
  if (magic_list == (LinkedListInfo *) NULL)
    {
      if (magic_list_semaphore == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&magic_list_semaphore);
      LockSemaphoreInfo(magic_list_semaphore);
      if (magic_list == (LinkedListInfo *) NULL)
        magic_list=AcquireMagicList(exception);
      UnlockSemaphoreInfo(magic_list_semaphore);
    }
  return(magic_list != (LinkedListInfo *) NULL ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t M a g i c I n f o                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListMagicInfo() lists the magic info to a file.
%
%  The format of the ListMagicInfo method is:
%
%      MagickBooleanType ListMagicInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListMagicInfo(FILE *file,
  ExceptionInfo *exception)
{
  const MagicInfo
    **magic_info;

  ssize_t
    i;

  size_t
    number_aliases;

  ssize_t
    j;

  if (file == (const FILE *) NULL)
    file=stdout;
  magic_info=GetMagicInfoList("*",&number_aliases,exception);
  if (magic_info == (const MagicInfo **) NULL)
    return(MagickFalse);
  (void) FormatLocaleFile(file,"Name      Offset Target\n");
  (void) FormatLocaleFile(file,
    "-------------------------------------------------"
    "------------------------------\n");
  for (i=0; i < (ssize_t) number_aliases; i++)
  {
    (void) FormatLocaleFile(file,"%s",magic_info[i]->name);
    for (j=(ssize_t) strlen(magic_info[i]->name); j <= 9; j++)
      (void) FormatLocaleFile(file," ");
    (void) FormatLocaleFile(file,"%6ld ",(long) magic_info[i]->offset);
    if (magic_info[i]->magic != (unsigned char *) NULL)
      {
        for (j=0; magic_info[i]->magic[j] != '\0'; j++)
          if (isprint((int) (magic_info[i]->magic[j])) != 0)
            (void) FormatLocaleFile(file,"%c",magic_info[i]->magic[j]);
          else
            (void) FormatLocaleFile(file,"\\%03o",(unsigned int)
              ((unsigned char) magic_info[i]->magic[j]));
      }
    (void) FormatLocaleFile(file,"\n");
  }
  (void) fflush(file);
  magic_info=(const MagicInfo **) RelinquishMagickMemory((void *) magic_info);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c C o m p o n e n t G e n e s i s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagicComponentGenesis() instantiates the magic component.
%
%  The format of the MagicComponentGenesis method is:
%
%      MagickBooleanType MagicComponentGenesis(void)
%
*/
MagickPrivate MagickBooleanType MagicComponentGenesis(void)
{
  if (magic_list_semaphore == (SemaphoreInfo *) NULL)
    magic_list_semaphore=AcquireSemaphoreInfo();
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c C o m p o n e n t T e r m i n u s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagicComponentTerminus() destroys the magic component.
%
%  The format of the MagicComponentTerminus method is:
%
%      MagicComponentTerminus(void)
%
*/

static void *DestroyMagicElement(void *magic_info)
{
  (void) RelinquishMagickMemory((MagicInfo *) magic_info);
  return((void *) NULL);
}

MagickPrivate void MagicComponentTerminus(void)
{
  if (magic_list_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&magic_list_semaphore);
  LockSemaphoreInfo(magic_list_semaphore);
  if (magic_list != (LinkedListInfo *) NULL)
    magic_list=DestroyLinkedList(magic_list,DestroyMagicElement);
  UnlockSemaphoreInfo(magic_list_semaphore);
  RelinquishSemaphoreInfo(&magic_list_semaphore);
  if (magic_cache_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&magic_cache_semaphore);
  LockSemaphoreInfo(magic_cache_semaphore);
  if (magic_cache != (LinkedListInfo *) NULL)
    magic_cache=DestroyLinkedList(magic_cache,(void *(*)(void *)) NULL);
  UnlockSemaphoreInfo(magic_cache_semaphore);
  RelinquishSemaphoreInfo(&magic_cache_semaphore);
}

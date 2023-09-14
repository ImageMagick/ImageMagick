/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                        M   M  IIIII  M   M  EEEEE                           %
%                        MM MM    I    MM MM  E                               %
%                        M M M    I    M M M  EEE                             %
%                        M   M    I    M   M  E                               %
%                        M   M  IIIII  M   M  EEEEE                           %
%                                                                             %
%                                                                             %
%                          MagickCore Mime Methods                            %
%                                                                             %
%                              Software Design                                %
%                                 July 2000                                   %
%                                                                             %
%                                                                             %
%  Copyright @ 2000 ImageMagick Studio LLC, a non-profit organization         %
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
#include "MagickCore/memory_.h"
#include "MagickCore/mime.h"
#include "MagickCore/mime-private.h"
#include "MagickCore/option.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/string_.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/xml-tree.h"
#include "MagickCore/xml-tree-private.h"

/*
  Define declarations.
*/
#define MimeFilename  "mime.xml"

/*
  Typedef declaration.
*/
struct _MimeInfo
{
  char
    *path,
    *type,
    *description,
    *pattern;

  ssize_t
    priority;

  MagickOffsetType
    offset;

  size_t
    extent;

  DataType
    data_type;

  ssize_t
    mask,
    value;

  EndianType
    endian;

  size_t
    length;

  unsigned char
    *magic;

  MagickBooleanType
    stealth;

  size_t
    signature;
};

/*
  Static declarations.
*/
static LinkedListInfo
  *mime_cache = (LinkedListInfo *) NULL;

static SemaphoreInfo
  *mime_semaphore = (SemaphoreInfo *) NULL;

/*
  Forward declarations.
*/
static MagickBooleanType
  IsMimeCacheInstantiated(ExceptionInfo *);

#if !MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
static MagickBooleanType
 LoadMimeCache(LinkedListInfo *,const char *,const char *,const size_t,
    ExceptionInfo *);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A c q u i r e M i m e C a c h e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireMimeCache() caches one or more magic configurations which provides
%  a mapping between magic attributes and a magic name.
%
%  The format of the AcquireMimeCache method is:
%
%      LinkedListInfo *AcquireMimeCache(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport LinkedListInfo *AcquireMimeCache(const char *filename,
  ExceptionInfo *exception)
{
  LinkedListInfo
    *cache;

  cache=NewLinkedList(0);
#if !MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
  {
    const StringInfo
      *option;

    LinkedListInfo
      *options;

    options=GetConfigureOptions(filename,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
    while (option != (const StringInfo *) NULL)
    {
      (void) LoadMimeCache(cache,(const char *)
        GetStringInfoDatum(option),GetStringInfoPath(option),0,exception);
      option=(const StringInfo *) GetNextValueInLinkedList(options);
    }
    options=DestroyConfigureOptions(options);
  }
#else
  magick_unreferenced(filename);
  magick_unreferenced(exception);
#endif
  return(cache);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M i m e I n f o                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMimeInfo() attempts to classify the content to identify which mime type
%  is associated with the content, if any.
%
%  The format of the GetMimeInfo method is:
%
%      const MimeInfo *GetMimeInfo(const char *filename,
%        const unsigned char *magic,const size_t length,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename:  If we cannot not classify the string, we attempt to classify
%      based on the filename (e.g. *.pdf returns application/pdf).
%
%    o magic: A binary string generally representing the first few characters
%      of the image file or blob.
%
%    o length: the length of the binary signature.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const MimeInfo *GetMimeInfo(const char *filename,
  const unsigned char *magic,const size_t length,ExceptionInfo *exception)
{
  ElementInfo
    *element_info,
    *p;

  EndianType
    endian;

  const unsigned char
    *r;

  ssize_t
    i;

  ssize_t
    value;

  unsigned long
    lsb_first;

  assert(exception != (ExceptionInfo *) NULL);
  if (IsMimeCacheInstantiated(exception) == MagickFalse)
    return((const MimeInfo *) NULL);
  /*
    Search for mime tag.
  */
  lsb_first=1;
  LockSemaphoreInfo(mime_semaphore);
  p=GetHeadElementInLinkedList(mime_cache);
  if ((magic == (const unsigned char *) NULL) || (length == 0))
    {
      UnlockSemaphoreInfo(mime_semaphore);
      if (p != (ElementInfo *) NULL)
        return((const MimeInfo *) p->value);
      return((const MimeInfo *) NULL);
    }
  element_info=(ElementInfo *) NULL;
  while (p != (ElementInfo *) NULL)
  {
    const MimeInfo
      *q;

    q=(const MimeInfo *) p->value;
    assert(q->offset >= 0);
    if (element_info != (const ElementInfo *) NULL)
      if (q->priority > ((const MimeInfo *) element_info->value)->priority)
        {
          p=p->next;
          continue;
        }
    if ((q->pattern != (char *) NULL) && (filename != (char *) NULL))
      {
        if (GlobExpression(filename,q->pattern,MagickFalse) != MagickFalse)
          element_info=p;
        p=p->next;
        continue;
      }
    switch (q->data_type)
    {
      case ByteData:
      {
        if ((size_t) (q->offset+4) > length)
          break;
        r=magic+q->offset;
        value=(ssize_t) (*r++);
        if (q->mask == 0)
          {
            if (q->value == value)
              element_info=p;
          }
        else
          {
            if ((q->value & q->mask) == value)
              element_info=p;
          }
        break;
      }
      case ShortData:
      {
        if ((size_t) (q->offset+4) > length)
          break;
        r=magic+q->offset;
        endian=q->endian;
        if (q->endian == UndefinedEndian)
          endian=(*(char *) &lsb_first) == 1 ? LSBEndian : MSBEndian;
        if (endian == LSBEndian)
          {
            value=(ssize_t) (*r++);
            value|=(*r++) << 8;
          }
        else
          {
            value=(ssize_t) (*r++) << 8;
            value|=(*r++);
          }
        if (q->mask == 0)
          {
            if (q->value == value)
              element_info=p;
          }
        else
          {
            if ((q->value & q->mask) == value)
              element_info=p;
          }
        break;
      }
      case LongData:
      {
        if ((size_t) (q->offset+4) > length)
          break;
        r=magic+q->offset;
        endian=q->endian;
        if (q->endian == UndefinedEndian)
          endian=(*(char *) &lsb_first) == 1 ? LSBEndian : MSBEndian;
        if (endian == LSBEndian)
          {
            value=(ssize_t) (*r++);
            value|=((ssize_t) *r++) << 8;
            value|=((ssize_t) *r++) << 16;
            value|=((ssize_t) *r++) << 24;
          }
        else
          {
            value=(ssize_t) (*r++) << 24;
            value|=((ssize_t) *r++) << 16;
            value|=((ssize_t) *r++) << 8;
            value|=((ssize_t) *r++);
          }
        if (q->mask == 0)
          {
            if (q->value == value)
              element_info=p;
          }
        else
          {
            if ((q->value & q->mask) == value)
              element_info=p;
          }
        break;
      }
      case StringData:
      default:
      {
        for (i=0; i <= (ssize_t) q->extent; i++)
        {
          if ((size_t) (q->offset+i+(ssize_t) q->length) > length)
            break;
          if (memcmp(magic+q->offset+i,q->magic,q->length) == 0)
            {
              element_info=p;
              break;
            }
        }
        break;
      }
    }
    p=p->next;
  }
  if (element_info != (ElementInfo *) NULL)
    SetHeadElementInLinkedList(mime_cache,element_info);
  UnlockSemaphoreInfo(mime_semaphore);
  if (element_info == (ElementInfo *) NULL)
    return((const MimeInfo *) NULL);
  return((const MimeInfo *) element_info->value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M i m e I n f o L i s t                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMimeInfoList() returns any image aliases that match the specified
%  pattern.
%
%  The magic of the GetMimeInfoList function is:
%
%      const MimeInfo **GetMimeInfoList(const char *pattern,
%        size_t *number_aliases,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_aliases:  This integer returns the number of magics in the
%      list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int MimeInfoCompare(const void *x,const void *y)
{
  const MimeInfo
    **p,
    **q;

  p=(const MimeInfo **) x,
  q=(const MimeInfo **) y;
  if (strcasecmp((*p)->path,(*q)->path) == 0)
    return(strcasecmp((*p)->type,(*q)->type));
  return(strcasecmp((*p)->path,(*q)->path));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport const MimeInfo **GetMimeInfoList(const char *pattern,
  size_t *number_aliases,ExceptionInfo *exception)
{
  const MimeInfo
    **aliases;

  ElementInfo
    *p;

  ssize_t
    i;

  assert(pattern != (char *) NULL);
  assert(number_aliases != (size_t *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  *number_aliases=0;
  if (IsMimeCacheInstantiated(exception) == MagickFalse)
    return((const MimeInfo **) NULL);
  aliases=(const MimeInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(mime_cache)+1UL,sizeof(*aliases));
  if (aliases == (const MimeInfo **) NULL)
    return((const MimeInfo **) NULL);
  LockSemaphoreInfo(mime_semaphore);
  p=GetHeadElementInLinkedList(mime_cache);
  for (i=0; p != (ElementInfo *) NULL; )
  {
    MimeInfo
      *mime_info;

    mime_info=(MimeInfo *) p->value;
    if ((mime_info->stealth == MagickFalse) &&
        (GlobExpression(mime_info->type,pattern,MagickFalse) != MagickFalse))
      aliases[i++]=mime_info;
    p=p->next;
  }
  UnlockSemaphoreInfo(mime_semaphore);
  if (i == 0)
    aliases=(const MimeInfo **) RelinquishMagickMemory((void *) aliases);
  else
    {
      qsort((void *) aliases,(size_t) i,sizeof(*aliases),MimeInfoCompare);
      aliases[i]=(MimeInfo *) NULL;
    }
  *number_aliases=(size_t) i;
  return(aliases);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M i m e L i s t                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMimeList() returns any image format alias that matches the specified
%  pattern.
%
%  The format of the GetMimeList function is:
%
%      char **GetMimeList(const char *pattern,size_t *number_aliases,
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

static int MimeCompare(const void *x,const void *y)
{
  char
    *p,
    *q;

  p=(char *) x;
  q=(char *) y;
  return(strcasecmp(p,q));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport char **GetMimeList(const char *pattern,
  size_t *number_aliases,ExceptionInfo *exception)
{
  char
    **aliases;

  ElementInfo
    *p;

  ssize_t
    i;

  assert(pattern != (char *) NULL);
  assert(number_aliases != (size_t *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  *number_aliases=0;
  if (IsMimeCacheInstantiated(exception) == MagickFalse)
    return((char **) NULL);
  aliases=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(mime_cache)+1UL,sizeof(*aliases));
  if (aliases == (char **) NULL)
    return((char **) NULL);
  LockSemaphoreInfo(mime_semaphore);
  p=GetHeadElementInLinkedList(mime_cache);
  for (i=0; p != (ElementInfo *) NULL; )
  {
    const MimeInfo
      *mime_info;

    mime_info=(const MimeInfo *) p->value;
    if ((mime_info->stealth == MagickFalse) &&
        (GlobExpression(mime_info->type,pattern,MagickFalse) != MagickFalse))
      aliases[i++]=ConstantString(mime_info->type);
    p=p->next;
  }
  UnlockSemaphoreInfo(mime_semaphore);
  if (i == 0)
    aliases=(char **) RelinquishMagickMemory(aliases);
  else
    {
      qsort((void *) aliases,(size_t) i,sizeof(*aliases),MimeCompare);
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
%   G e t M i m e D e s c r i p t i o n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMimeDescription() returns the mime type description.
%
%  The format of the GetMimeDescription method is:
%
%      const char *GetMimeDescription(const MimeInfo *mime_info)
%
%  A description of each parameter follows:
%
%    o mime_info:  The magic info.
%
*/
MagickExport const char *GetMimeDescription(const MimeInfo *mime_info)
{
  assert(mime_info != (MimeInfo *) NULL);
  assert(mime_info->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  return(mime_info->description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M i m e T y p e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMimeType() returns the mime type.
%
%  The format of the GetMimeType method is:
%
%      const char *GetMimeType(const MimeInfo *mime_info)
%
%  A description of each parameter follows:
%
%    o mime_info:  The magic info.
%
*/
MagickExport const char *GetMimeType(const MimeInfo *mime_info)
{
  assert(mime_info != (MimeInfo *) NULL);
  assert(mime_info->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  return(mime_info->type);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s M i m e C a c h e I n s t a n t i a t e d                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMimeCacheInstantiated() determines if the mime list is instantiated.  If
%  not, it instantiates the list and returns it.
%
%  The format of the IsMimeInstantiated method is:
%
%      MagickBooleanType IsMimeCacheInstantiated(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType IsMimeCacheInstantiated(ExceptionInfo *exception)
{
  if (mime_cache == (LinkedListInfo *) NULL)
    {
      if (mime_semaphore == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&mime_semaphore);
      LockSemaphoreInfo(mime_semaphore);
      if (mime_cache == (LinkedListInfo *) NULL)
        mime_cache=AcquireMimeCache(MimeFilename,exception);
      UnlockSemaphoreInfo(mime_semaphore);
    }
  return(mime_cache != (LinkedListInfo *) NULL ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t M i m e I n f o                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListMimeInfo() lists the magic info to a file.
%
%  The format of the ListMimeInfo method is:
%
%      MagickBooleanType ListMimeInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListMimeInfo(FILE *file,ExceptionInfo *exception)
{
  const char
    *path;

  const MimeInfo
    **mime_info;

  ssize_t
    i;

  size_t
    number_aliases;

  ssize_t
    j;

  if (file == (const FILE *) NULL)
    file=stdout;
  mime_info=GetMimeInfoList("*",&number_aliases,exception);
  if (mime_info == (const MimeInfo **) NULL)
    return(MagickFalse);
  j=0;
  path=(const char *) NULL;
  for (i=0; i < (ssize_t) number_aliases; i++)
  {
    if (mime_info[i]->stealth != MagickFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (strcasecmp(path,mime_info[i]->path) != 0))
      {
        if (mime_info[i]->path != (char *) NULL)
          (void) FormatLocaleFile(file,"\nPath: %s\n\n",mime_info[i]->path);
        (void) FormatLocaleFile(file,"Type                   Description\n");
        (void) FormatLocaleFile(file,
          "-------------------------------------------------"
          "------------------------------\n");
      }
    path=mime_info[i]->path;
    (void) FormatLocaleFile(file,"%s",mime_info[i]->type);
    if (strlen(mime_info[i]->type) <= 25)
      {
        for (j=(ssize_t) strlen(mime_info[i]->type); j <= 27; j++)
          (void) FormatLocaleFile(file," ");
      }
    else
      {
        (void) FormatLocaleFile(file,"\n");
        for (j=0; j <= 27; j++)
          (void) FormatLocaleFile(file," ");
      }
    if (mime_info[i]->description != (char *) NULL)
      (void) FormatLocaleFile(file,"%s",mime_info[i]->description);
    (void) FormatLocaleFile(file,"\n");
  }
  (void) fflush(file);
  mime_info=(const MimeInfo **) RelinquishMagickMemory((void *) mime_info);
  return(MagickTrue);
}

#if !MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d M i m e C a c h e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadMimeCache() loads the mime configurations which provides a mapping
%  between mime attributes and a mime name.
%
%  The format of the LoadMimeCache method is:
%
%      MagickBooleanType LoadMimeCache(LinkedListInfo *cache,const char *xml,
%        const char *filename,const size_t depth,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The mime list in XML format.
%
%    o filename:  The mime list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadMimeCache(LinkedListInfo *cache,const char *xml,
  const char *filename,const size_t depth,ExceptionInfo *exception)
{
  const char
    *attribute;

  MimeInfo
    *mime_info = (MimeInfo *) NULL;

  MagickStatusType
    status;

  XMLTreeInfo
    *mime,
    *mime_map,
    *include;

  /*
    Load the mime map file.
  */
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading mime map \"%s\" ...",filename);
  if (xml == (const char *) NULL)
    return(MagickFalse);
  mime_map=NewXMLTree(xml,exception);
  if (mime_map == (XMLTreeInfo *) NULL)
    return(MagickFalse);
  status=MagickTrue;
  include=GetXMLTreeChild(mime_map,"include");
  while (include != (XMLTreeInfo *) NULL)
  {
    /*
      Process include element.
    */
    attribute=GetXMLTreeAttribute(include,"file");
    if (attribute != (const char *) NULL)
      {
        if (depth > MagickMaxRecursionDepth)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ConfigureError,"IncludeElementNestedTooDeeply","`%s'",filename);
        else
          {
            char
              path[MagickPathExtent],
              *file_xml;

            GetPathComponent(filename,HeadPath,path);
            if (*path != '\0')
              (void) ConcatenateMagickString(path,DirectorySeparator,
                MagickPathExtent);
            if (*attribute == *DirectorySeparator)
              (void) CopyMagickString(path,attribute,MagickPathExtent);
            else
              (void) ConcatenateMagickString(path,attribute,MagickPathExtent);
            file_xml=FileToXML(path,~0UL);
            if (file_xml != (char *) NULL)
              {
                status&=(MagickStatusType) LoadMimeCache(cache,file_xml,path,
                  depth+1,exception);
                file_xml=DestroyString(file_xml);
              }
          }
      }
    include=GetNextXMLTreeTag(include);
  }
  mime=GetXMLTreeChild(mime_map,"mime");
  while (mime != (XMLTreeInfo *) NULL)
  {
    /*
      Process mime element.
    */
    mime_info=(MimeInfo *) AcquireCriticalMemory(sizeof(*mime_info));
    (void) memset(mime_info,0,sizeof(*mime_info));
    mime_info->path=ConstantString(filename);
    mime_info->signature=MagickCoreSignature;
    attribute=GetXMLTreeAttribute(mime,"data-type");
    if (attribute != (const char *) NULL)
      mime_info->data_type=(DataType) ParseCommandOption(MagickDataTypeOptions,
        MagickTrue,attribute);
    attribute=GetXMLTreeAttribute(mime,"description");
    if (attribute != (const char *) NULL)
      mime_info->description=ConstantString(attribute);
    attribute=GetXMLTreeAttribute(mime,"endian");
    if (attribute != (const char *) NULL)
      mime_info->endian=(EndianType) ParseCommandOption(MagickEndianOptions,
        MagickTrue,attribute);
    attribute=GetXMLTreeAttribute(mime,"magic");
    if (attribute != (const char *) NULL)
      {
        char
          *token;

        const char
          *p;

        unsigned char
          *q;

        token=AcquireString(attribute);
        (void) SubstituteString((char **) &token,"&lt;","<");
        (void) SubstituteString((char **) &token,"&amp;","&");
        (void) SubstituteString((char **) &token,"&quot;","\"");
        mime_info->magic=(unsigned char *) AcquireString(token);
        q=mime_info->magic;
        for (p=token; *p != '\0'; )
        {
          if (*p == '\\')
            {
              p++;
              if (isdigit((int) ((unsigned char) *p)) != 0)
                {
                  char
                    *end;

                  *q++=(unsigned char) strtol(p,&end,8);
                  p+=(end-p);
                  mime_info->length++;
                  continue;
                }
              switch (*p)
              {
                case 'b': *q='\b'; break;
                case 'f': *q='\f'; break;
                case 'n': *q='\n'; break;
                case 'r': *q='\r'; break;
                case 't': *q='\t'; break;
                case 'v': *q='\v'; break;
                case 'a': *q='a'; break;
                case '?': *q='\?'; break;
                default: *q=(unsigned char) (*p); break;
              }
              p++;
              q++;
              mime_info->length++;
              continue;
            }
          *q++=(unsigned char) (*p++);
          mime_info->length++;
        }
        token=DestroyString(token);
        if (mime_info->data_type != StringData)
          mime_info->value=(ssize_t) strtoul((char *) mime_info->magic,
            (char **) NULL,0);
      }
    attribute=GetXMLTreeAttribute(mime,"mask");
    if (attribute != (const char *) NULL)
      mime_info->mask=(ssize_t) strtoul(attribute,(char **) NULL,0);
    attribute=GetXMLTreeAttribute(mime,"offset");
    if (attribute != (const char *) NULL)
      {
        char
          *c;

        mime_info->offset=(MagickOffsetType) strtol(attribute,&c,0);
        if (*c == ':')
          mime_info->extent=(size_t) strtol(c+1,(char **) NULL,0);
      }
    attribute=GetXMLTreeAttribute(mime,"pattern");
    if (attribute != (const char *) NULL)
      mime_info->pattern=ConstantString(attribute);
    attribute=GetXMLTreeAttribute(mime,"priority");
    if (attribute != (const char *) NULL)
      mime_info->priority=(ssize_t) strtol(attribute,(char **) NULL,0);
    attribute=GetXMLTreeAttribute(mime,"stealth");
    if (attribute != (const char *) NULL)
      mime_info->stealth=IsStringTrue(attribute);
    attribute=GetXMLTreeAttribute(mime,"type");
    if (attribute != (const char *) NULL)
      mime_info->type=ConstantString(attribute);
    status=AppendValueToLinkedList(cache,mime_info);
    if (status == MagickFalse)
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",filename);
    mime=GetNextXMLTreeTag(mime);
  }
  mime_map=DestroyXMLTree(mime_map);
  return(status != 0 ? MagickTrue : MagickFalse);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  M a g i c k T o M i m e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickToMime() returns the officially registered (or de facto) MIME
%  media-type corresponding to a magick string.  If there is no registered
%  media-type, then the string "image/x-magick" (all lower case) is returned.
%  The returned string must be deallocated by the user.
%
%  The format of the MagickToMime method is:
%
%      char *MagickToMime(const char *magick)
%
%  A description of each parameter follows.
%
%   o  magick:  ImageMagick format specification "magick" tag.
%
*/
MagickExport char *MagickToMime(const char *magick)
{
  char
    filename[MagickPathExtent],
    media[MagickPathExtent];

  const MimeInfo
    *mime_info;

  ExceptionInfo
    *exception;

  (void) FormatLocaleString(filename,MagickPathExtent,"file.%s",magick);
  LocaleLower(filename);
  exception=AcquireExceptionInfo();
  mime_info=GetMimeInfo(filename,(unsigned char *) " ",1,exception);
  exception=DestroyExceptionInfo(exception);
  if (mime_info != (const MimeInfo *) NULL)
    return(ConstantString(GetMimeType(mime_info)));
  (void) FormatLocaleString(media,MagickPathExtent,"image/x-%s",magick);
  LocaleLower(media+8);
  return(ConstantString(media));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M i m e C o m p o n e n t G e n e s i s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MimeComponentGenesis() instantiates the mime component.
%
%  The format of the MimeComponentGenesis method is:
%
%      MagickBooleanType MimeComponentGenesis(void)
%
*/
MagickPrivate MagickBooleanType MimeComponentGenesis(void)
{
  if (mime_semaphore == (SemaphoreInfo *) NULL)
    mime_semaphore=AcquireSemaphoreInfo();
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M i m e C o m p o n e n t T e r m i n u s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MimeComponentTerminus() destroys the mime component.
%
%  The format of the MimeComponentTerminus method is:
%
%      MimeComponentTerminus(void)
%
*/

static void *DestroyMimeElement(void *mime_info)
{
  MimeInfo
    *p;

  p=(MimeInfo *) mime_info;
  if (p->magic != (unsigned char *) NULL)
    p->magic=(unsigned char *) RelinquishMagickMemory(p->magic);
  if (p->pattern != (char *) NULL)
    p->pattern=DestroyString(p->pattern);
  if (p->description != (char *) NULL)
    p->description=DestroyString(p->description);
  if (p->type != (char *) NULL)
    p->type=DestroyString(p->type);
  if (p->path != (char *) NULL)
    p->path=DestroyString(p->path);
  p=(MimeInfo *) RelinquishMagickMemory(p);
  return((void *) NULL);
}

MagickPrivate void MimeComponentTerminus(void)
{
  if (mime_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&mime_semaphore);
  LockSemaphoreInfo(mime_semaphore);
  if (mime_cache != (LinkedListInfo *) NULL)
    mime_cache=DestroyLinkedList(mime_cache,DestroyMimeElement);
  UnlockSemaphoreInfo(mime_semaphore);
  RelinquishSemaphoreInfo(&mime_semaphore);
}

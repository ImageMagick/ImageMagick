/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%           RRRR    EEEEE   SSSSS   OOO   U   U  RRRR    CCCC  EEEEE          %
%           R   R   E       SS     O   O  U   U  R   R  C      E              %
%           RRRR    EEE      SSS   O   O  U   U  RRRR   C      EEE            %
%           R R     E          SS  O   O  U   U  R R    C      E              %
%           R  R    EEEEE   SSSSS   OOO    UUU   R  R    CCCC  EEEEE          %
%                                                                             %
%                                                                             %
%                        Get/Set MagickCore Resources                         %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                               September 2002                                %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/cache.h"
#include "magick/configure.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/hashmap.h"
#include "magick/log.h"
#include "magick/image.h"
#include "magick/memory_.h"
#include "magick/option.h"
#include "magick/policy.h"
#include "magick/random_.h"
#include "magick/registry.h"
#include "magick/resource_.h"
#include "magick/semaphore.h"
#include "magick/signature-private.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/splay-tree.h"
#include "magick/thread-private.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/utility-private.h"

/*
  Typedef declarations.
*/
typedef struct _ResourceInfo
{
  MagickOffsetType
    area,
    memory,
    map,
    disk,
    file,
    thread,
    time;

  MagickSizeType
    area_limit,
    memory_limit,
    map_limit,
    disk_limit,
    file_limit,
    thread_limit,
    time_limit;
} ResourceInfo;

/*
  Global declarations.
*/
static RandomInfo
  *random_info = (RandomInfo *) NULL;

static ResourceInfo
  resource_info =
  {
    MagickULLConstant(0),
    MagickULLConstant(0),
    MagickULLConstant(0),
    MagickULLConstant(0),
    MagickULLConstant(0),
    MagickULLConstant(0),
    MagickULLConstant(0),
    MagickULLConstant(3072)*1024*1024,
    MagickULLConstant(1536)*1024*1024,
    MagickULLConstant(3072)*1024*1024,
    MagickResourceInfinity,
    MagickULLConstant(768),
    MagickULLConstant(1),
    MagickResourceInfinity
  };

static SemaphoreInfo
  *resource_semaphore = (SemaphoreInfo *) NULL;

static SplayTreeInfo
  *temporary_resources = (SplayTreeInfo *) NULL;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e M a g i c k R e s o u r c e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireMagickResource() acquires resources of the specified type.
%  MagickFalse is returned if the specified resource is exhausted otherwise
%  MagickTrue.
%
%  The format of the AcquireMagickResource() method is:
%
%      MagickBooleanType AcquireMagickResource(const ResourceType type,
%        const MagickSizeType size)
%
%  A description of each parameter follows:
%
%    o type: the type of resource.
%
%    o size: the number of bytes needed from for this resource.
%
*/
MagickExport MagickBooleanType AcquireMagickResource(const ResourceType type,
  const MagickSizeType size)
{
  char
    resource_current[MaxTextExtent],
    resource_limit[MaxTextExtent],
    resource_request[MaxTextExtent];

  MagickBooleanType
    status;

  MagickSizeType
    limit;

  status=MagickFalse;
  (void) FormatMagickSize(size,MagickFalse,resource_request);
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  switch (type)
  {
    case AreaResource:
    {
      resource_info.area=(MagickOffsetType) size;
      limit=resource_info.area_limit;
      status=(resource_info.area_limit == MagickResourceInfinity) ||
        (size < limit) ? MagickTrue : MagickFalse;
      (void) FormatMagickSize((MagickSizeType) resource_info.area,MagickFalse,
        resource_current);
      (void) FormatMagickSize(resource_info.area_limit,MagickFalse,
        resource_limit);
      break;
    }
    case MemoryResource:
    {
      resource_info.memory+=size;
      limit=resource_info.memory_limit;
      status=(resource_info.memory_limit == MagickResourceInfinity) ||
        ((MagickSizeType) resource_info.memory < limit) ?
        MagickTrue : MagickFalse;
      (void) FormatMagickSize((MagickSizeType) resource_info.memory,MagickTrue,
        resource_current);
      (void) FormatMagickSize(resource_info.memory_limit,MagickTrue,
        resource_limit);
      break;
    }
    case MapResource:
    {
      resource_info.map+=size;
      limit=resource_info.map_limit;
      status=(resource_info.map_limit == MagickResourceInfinity) ||
        ((MagickSizeType) resource_info.map < limit) ?
        MagickTrue : MagickFalse;
      (void) FormatMagickSize((MagickSizeType) resource_info.map,MagickTrue,
        resource_current);
      (void) FormatMagickSize(resource_info.map_limit,MagickTrue,
        resource_limit);
      break;
    }
    case DiskResource:
    {
      resource_info.disk+=size;
      limit=resource_info.disk_limit;
      status=(resource_info.disk_limit == MagickResourceInfinity) ||
        ((MagickSizeType) resource_info.disk < limit) ?
        MagickTrue : MagickFalse;
      (void) FormatMagickSize((MagickSizeType) resource_info.disk,MagickTrue,
        resource_current);
      (void) FormatMagickSize(resource_info.disk_limit,MagickTrue,
        resource_limit);
      break;
    }
    case FileResource:
    {
      resource_info.file+=size;
      limit=resource_info.file_limit;
      status=(resource_info.file_limit == MagickResourceInfinity) ||
        ((MagickSizeType) resource_info.file < limit) ?
        MagickTrue : MagickFalse;
      (void) FormatMagickSize((MagickSizeType) resource_info.file,MagickFalse,
        resource_current);
      (void) FormatMagickSize((MagickSizeType) resource_info.file_limit,
        MagickFalse,resource_limit);
      break;
    }
    case ThreadResource:
    {
      limit=resource_info.thread_limit;
      status=(resource_info.thread_limit == MagickResourceInfinity) ||
        ((MagickSizeType) resource_info.thread < limit) ? MagickTrue :
        MagickFalse;
      (void) FormatMagickSize((MagickSizeType) resource_info.thread,MagickFalse,
        resource_current);
      (void) FormatMagickSize((MagickSizeType) resource_info.thread_limit,
        MagickFalse,resource_limit);
      break;
    }
    case TimeResource:
    {
      resource_info.time+=size;
      limit=resource_info.time_limit;
      status=(resource_info.time_limit == MagickResourceInfinity) ||
        ((MagickSizeType) resource_info.time < limit) ?
        MagickTrue : MagickFalse;
      (void) FormatMagickSize((MagickSizeType) resource_info.time,MagickFalse,
        resource_current);
      (void) FormatMagickSize((MagickSizeType) resource_info.time_limit,
        MagickFalse,resource_limit);
      break;
    }
    default:
      break;
  }
  UnlockSemaphoreInfo(resource_semaphore);
  (void) LogMagickEvent(ResourceEvent,GetMagickModule(),"%s: %s/%s/%s",
    CommandOptionToMnemonic(MagickResourceOptions,(ssize_t) type),
    resource_request,resource_current,resource_limit);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A s y n c h r o n o u s R e s o u r c e C o m p o n e n t T e r m i n u s %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AsynchronousResourceComponentTerminus() destroys the resource environment.
%  It differs from ResourceComponentTerminus() in that it can be called from a
%  asynchronous signal handler.
%
%  The format of the ResourceComponentTerminus() method is:
%
%      ResourceComponentTerminus(void)
%
*/
MagickExport void AsynchronousResourceComponentTerminus(void)
{
  const char
    *path;

  if (temporary_resources == (SplayTreeInfo *) NULL)
    return;
  /*
    Remove any lingering temporary files.
  */
  ResetSplayTreeIterator(temporary_resources);
  path=(const char *) GetNextKeyInSplayTree(temporary_resources);
  while (path != (const char *) NULL)
  {
    (void) remove_utf8(path);
    path=(const char *) GetNextKeyInSplayTree(temporary_resources);
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e U n i q u e F i l e R e s o u r c e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireUniqueFileResource() returns a unique file name, and returns a file
%  descriptor for the file open for reading and writing.
%
%  The format of the AcquireUniqueFileResource() method is:
%
%      int AcquireUniqueFileResource(char *path)
%
%  A description of each parameter follows:
%
%   o  path:  Specifies a pointer to an array of characters.  The unique path
%      name is returned in this array.
%
*/

static void *DestroyTemporaryResources(void *temporary_resource)
{
  (void) remove_utf8((char *) temporary_resource);
  temporary_resource=DestroyString((char *) temporary_resource);
  return((void *) NULL);
}

MagickExport MagickBooleanType GetPathTemplate(char *path)
{
  char
    *directory,
    *value;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  register char
    *p;

  struct stat
    attributes;

  (void) FormatLocaleString(path,MaxTextExtent,"magick-%.20gXXXXXXXXXXXX",
    (double) getpid());
  exception=AcquireExceptionInfo();
  directory=(char *) GetImageRegistry(StringRegistryType,"temporary-path",
    exception);
  exception=DestroyExceptionInfo(exception);
  if (directory == (char *) NULL)
    directory=GetEnvironmentValue("MAGICK_TEMPORARY_PATH");
  if (directory == (char *) NULL)
    directory=GetEnvironmentValue("MAGICK_TMPDIR");
#if defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__OS2__)
  if (directory == (char *) NULL)
    directory=GetEnvironmentValue("TMP");
  if (directory == (char *) NULL)
    directory=GetEnvironmentValue("TEMP");
#endif
#if defined(__VMS)
  if (directory == (char *) NULL)
    directory=GetEnvironmentValue("MTMPDIR");
#endif
#if defined(P_tmpdir)
  if (directory == (char *) NULL)
    directory=ConstantString(P_tmpdir);
#endif
  if (directory == (char *) NULL)
    directory=GetEnvironmentValue("TMPDIR");
  if (directory == (char *) NULL)
    return(MagickTrue);
  value=GetPolicyValue("temporary-path");
  if (value != (char *) NULL)
    (void) CloneString(&directory,value);
  if (strlen(directory) > (MaxTextExtent-25))
    {
      directory=DestroyString(directory);
      return(MagickFalse);
    }
  status=GetPathAttributes(directory,&attributes);
  if ((status == MagickFalse) || !S_ISDIR(attributes.st_mode))
    {
      directory=DestroyString(directory);
      return(MagickFalse);
    }
  if (directory[strlen(directory)-1] == *DirectorySeparator)
    (void) FormatLocaleString(path,MaxTextExtent,"%smagick-%.20gXXXXXXXXXXXX",
      directory,(double) getpid());
  else
    (void) FormatLocaleString(path,MaxTextExtent,"%s%smagick-%.20gXXXXXXXXXXXX",
      directory,DirectorySeparator,(double) getpid());
  directory=DestroyString(directory);
  if (*DirectorySeparator != '/')
    for (p=path; *p != '\0'; p++)
      if (*p == *DirectorySeparator)
        *p='/';
  return(MagickTrue);
}

MagickExport int AcquireUniqueFileResource(char *path)
{
#if !defined(O_NOFOLLOW)
#define O_NOFOLLOW 0
#endif
#if !defined(TMP_MAX)
# define TMP_MAX  238328
#endif

  int
    c,
    file;

  register char
    *p;

  register ssize_t
    i;

  static const char
    portable_filename[65] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

  StringInfo
    *key;

  unsigned char
    *datum;

  assert(path != (char *) NULL);
  (void) LogMagickEvent(ResourceEvent,GetMagickModule(),"Acquire %s",path);
  if (random_info == (RandomInfo *) NULL)
    random_info=AcquireRandomInfo();
  file=(-1);
  for (i=0; i < (ssize_t) TMP_MAX; i++)
  {
    /*
      Get temporary pathname.
    */
    (void) GetPathTemplate(path);
    key=GetRandomKey(random_info,6);
    p=path+strlen(path)-12;
    datum=GetStringInfoDatum(key);
    for (i=0; i < (ssize_t) GetStringInfoLength(key); i++)
    {
      c=(int) (datum[i] & 0x3f);
      *p++=portable_filename[c];
    }
    key=DestroyStringInfo(key);
#if defined(MAGICKCORE_HAVE_MKSTEMP)
    file=mkstemp(path);
#if defined(__OS2__)
    setmode(file,O_BINARY);
#endif
    if (file != -1)
      break;
#endif
    key=GetRandomKey(random_info,12);
    p=path+strlen(path)-12;
    datum=GetStringInfoDatum(key);
    for (i=0; i < (ssize_t) GetStringInfoLength(key); i++)
    {
      c=(int) (datum[i] & 0x3f);
      *p++=portable_filename[c];
    }
    key=DestroyStringInfo(key);
    file=open_utf8(path,O_RDWR | O_CREAT | O_EXCL | O_BINARY | O_NOFOLLOW,
      S_MODE);
    if ((file >= 0) || (errno != EEXIST))
      break;
  }
  (void) LogMagickEvent(ResourceEvent,GetMagickModule(),"Acquire %s",path);
  if (file == -1)
    return(file);
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  if (temporary_resources == (SplayTreeInfo *) NULL)
    temporary_resources=NewSplayTree(CompareSplayTreeString,
      DestroyTemporaryResources,(void *(*)(void *)) NULL);
  UnlockSemaphoreInfo(resource_semaphore);
  (void) AddValueToSplayTree(temporary_resources,ConstantString(path),
    (const void *) NULL);
  return(file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k R e s o u r c e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickResource() returns the specified resource.
%
%  The format of the GetMagickResource() method is:
%
%      MagickSizeType GetMagickResource(const ResourceType type)
%
%  A description of each parameter follows:
%
%    o type: the type of resource.
%
*/
MagickExport MagickSizeType GetMagickResource(const ResourceType type)
{
  MagickSizeType
    resource;

  resource=0;
  LockSemaphoreInfo(resource_semaphore);
  switch (type)
  {
    case AreaResource:
    {
      resource=(MagickSizeType) resource_info.area;
      break;
    }
    case MemoryResource:
    {
      resource=(MagickSizeType) resource_info.memory;
      break;
    }
    case MapResource:
    {
      resource=(MagickSizeType) resource_info.map;
      break;
    }
    case DiskResource:
    {
      resource=(MagickSizeType) resource_info.disk;
      break;
    }
    case FileResource:
    {
      resource=(MagickSizeType) resource_info.file;
      break;
    }
    case ThreadResource:
    {
      resource=(MagickSizeType) resource_info.thread;
      break;
    }
    case TimeResource:
    {
      resource=(MagickSizeType) resource_info.time;
      break;
    }
    default:
      break;
  }
  UnlockSemaphoreInfo(resource_semaphore);
  return(resource);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k R e s o u r c e L i m i t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickResourceLimit() returns the specified resource limit.
%
%  The format of the GetMagickResourceLimit() method is:
%
%      MagickSizeType GetMagickResourceLimit(const ResourceType type)
%
%  A description of each parameter follows:
%
%    o type: the type of resource.
%
*/
MagickExport MagickSizeType GetMagickResourceLimit(const ResourceType type)
{
  MagickSizeType
    resource;

  resource=0;
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  switch (type)
  {
    case AreaResource:
    {
      resource=resource_info.area_limit;
      break;
    }
    case MemoryResource:
    {
      resource=resource_info.memory_limit;
      break;
    }
    case MapResource:
    {
      resource=resource_info.map_limit;
      break;
    }
    case DiskResource:
    {
      resource=resource_info.disk_limit;
      break;
    }
    case FileResource:
    {
      resource=resource_info.file_limit;
      break;
    }
    case ThreadResource:
    {
      resource=resource_info.thread_limit;
      break;
    }
    case TimeResource:
    {
      resource=resource_info.time_limit;
      break;
    }
    default:
      break;
  }
  UnlockSemaphoreInfo(resource_semaphore);
  return(resource);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t M a g i c k R e s o u r c e I n f o                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListMagickResourceInfo() lists the resource info to a file.
%
%  The format of the ListMagickResourceInfo method is:
%
%      MagickBooleanType ListMagickResourceInfo(FILE *file,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListMagickResourceInfo(FILE *file,
  ExceptionInfo *magick_unused(exception))
{
  char
    area_limit[MaxTextExtent],
    disk_limit[MaxTextExtent],
    map_limit[MaxTextExtent],
    memory_limit[MaxTextExtent],
    time_limit[MaxTextExtent];

  if (file == (const FILE *) NULL)
    file=stdout;
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  (void) FormatMagickSize(resource_info.area_limit,MagickFalse,area_limit);
  (void) FormatMagickSize(resource_info.memory_limit,MagickTrue,memory_limit);
  (void) FormatMagickSize(resource_info.map_limit,MagickTrue,map_limit);
  (void) CopyMagickString(disk_limit,"unlimited",MaxTextExtent);
  if (resource_info.disk_limit != MagickResourceInfinity)
    (void) FormatMagickSize(resource_info.disk_limit,MagickTrue,disk_limit);
  (void) CopyMagickString(time_limit,"unlimited",MaxTextExtent);
  if (resource_info.time_limit != MagickResourceInfinity)
    (void) FormatLocaleString(time_limit,MaxTextExtent,"%.20g",(double)
      ((MagickOffsetType) resource_info.time_limit));
  (void) FormatLocaleFile(file,"  File        Area      Memory          Map"
    "         Disk    Thread         Time\n");
  (void) FormatLocaleFile(file,
    "--------------------------------------------------------"
    "-----------------------\n");
  (void) FormatLocaleFile(file,"%6g  %10s  %10s   %10s   %10s    %6g  %11s\n",
    (double) ((MagickOffsetType) resource_info.file_limit),area_limit,
    memory_limit,map_limit,disk_limit,(double) ((MagickOffsetType)
    resource_info.thread_limit),time_limit);
  (void) fflush(file);
  UnlockSemaphoreInfo(resource_semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h M a g i c k R e s o u r c e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishMagickResource() relinquishes resources of the specified type.
%
%  The format of the RelinquishMagickResource() method is:
%
%      void RelinquishMagickResource(const ResourceType type,
%        const MagickSizeType size)
%
%  A description of each parameter follows:
%
%    o type: the type of resource.
%
%    o size: the size of the resource.
%
*/
MagickExport void RelinquishMagickResource(const ResourceType type,
  const MagickSizeType size)
{
  char
    resource_current[MaxTextExtent],
    resource_limit[MaxTextExtent],
    resource_request[MaxTextExtent];

  (void) FormatMagickSize(size,MagickFalse,resource_request);
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  switch (type)
  {
    case AreaResource:
    {
      resource_info.area=(MagickOffsetType) size;
      (void) FormatMagickSize((MagickSizeType) resource_info.area,MagickFalse,
        resource_current);
      (void) FormatMagickSize(resource_info.area_limit,MagickFalse,
        resource_limit);
      break;
    }
    case MemoryResource:
    {
      resource_info.memory-=size;
      (void) FormatMagickSize((MagickSizeType) resource_info.memory,
        MagickTrue,resource_current);
      (void) FormatMagickSize(resource_info.memory_limit,MagickTrue,
        resource_limit);
      break;
    }
    case MapResource:
    {
      resource_info.map-=size;
      (void) FormatMagickSize((MagickSizeType) resource_info.map,MagickTrue,
        resource_current);
      (void) FormatMagickSize(resource_info.map_limit,MagickTrue,
        resource_limit);
      break;
    }
    case DiskResource:
    {
      resource_info.disk-=size;
      (void) FormatMagickSize((MagickSizeType) resource_info.disk,MagickTrue,
        resource_current);
      (void) FormatMagickSize(resource_info.disk_limit,MagickTrue,
        resource_limit);
      break;
    }
    case FileResource:
    {
      resource_info.file-=size;
      (void) FormatMagickSize((MagickSizeType) resource_info.file,MagickFalse,
        resource_current);
      (void) FormatMagickSize((MagickSizeType) resource_info.file_limit,
        MagickFalse,resource_limit);
      break;
    }
    case ThreadResource:
    {
      (void) FormatMagickSize((MagickSizeType) resource_info.thread,MagickFalse,
        resource_current);
      (void) FormatMagickSize((MagickSizeType) resource_info.thread_limit,
        MagickFalse,resource_limit);
      break;
    }
    case TimeResource:
    {
      resource_info.time-=size;
      (void) FormatMagickSize((MagickSizeType) resource_info.time,MagickFalse,
        resource_current);
      (void) FormatMagickSize((MagickSizeType) resource_info.time_limit,
        MagickFalse,resource_limit);
      break;
    }
    default:
      break;
  }
  UnlockSemaphoreInfo(resource_semaphore);
  (void) LogMagickEvent(ResourceEvent,GetMagickModule(),"%s: %s/%s/%s",
    CommandOptionToMnemonic(MagickResourceOptions,(ssize_t) type),
      resource_request,resource_current,resource_limit);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    R e l i n q u i s h U n i q u e F i l e R e s o u r c e                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishUniqueFileResource() relinquishes a unique file resource.
%
%  The format of the RelinquishUniqueFileResource() method is:
%
%      MagickBooleanType RelinquishUniqueFileResource(const char *path)
%
%  A description of each parameter follows:
%
%    o name: the name of the temporary resource.
%
*/
MagickExport MagickBooleanType RelinquishUniqueFileResource(const char *path)
{
  char
    cache_path[MaxTextExtent];

  assert(path != (const char *) NULL);
  (void) LogMagickEvent(ResourceEvent,GetMagickModule(),"Relinquish %s",path);
  if (temporary_resources != (SplayTreeInfo *) NULL)
    {
      register char
        *p;

      ResetSplayTreeIterator(temporary_resources);
      p=(char *) GetNextKeyInSplayTree(temporary_resources);
      while (p != (char *) NULL)
      {
        if (LocaleCompare(p,path) == 0)
          break;
        p=(char *) GetNextKeyInSplayTree(temporary_resources);
      }
      if (p != (char *) NULL)
        (void) DeleteNodeFromSplayTree(temporary_resources,p);
    }
  (void) CopyMagickString(cache_path,path,MaxTextExtent);
  AppendImageFormat("cache",cache_path);
  (void) remove_utf8(cache_path);
  return(remove_utf8(path) == 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e s o u r c e C o m p o n e n t G e n e s i s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResourceComponentGenesis() instantiates the resource component.
%
%  The format of the ResourceComponentGenesis method is:
%
%      MagickBooleanType ResourceComponentGenesis(void)
%
*/

static inline size_t MagickMax(const size_t x,const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline MagickSizeType StringToSizeType(const char *string,
  const double interval)
{
  double
    value;

  value=SiPrefixToDoubleInterval(string,interval);
  if (value >= (double) MagickULLConstant(~0))
    return(MagickULLConstant(~0));
  return((MagickSizeType) value);
}

MagickExport MagickBooleanType ResourceComponentGenesis(void)
{
  char
    *limit;

  MagickSizeType
    memory;

  ssize_t
    files,
    pages,
    pagesize;

  /*
    Set Magick resource limits.
  */
  AcquireSemaphoreInfo(&resource_semaphore);
  pagesize=GetMagickPageSize();
  pages=(-1);
#if defined(MAGICKCORE_HAVE_SYSCONF) && defined(_SC_PHYS_PAGES)
  pages=(ssize_t) sysconf(_SC_PHYS_PAGES);
#endif
  memory=(MagickSizeType) pages*pagesize;
  if ((pagesize <= 0) || (pages <= 0))
    memory=2048UL*1024UL*1024UL;
#if defined(PixelCacheThreshold)
  memory=PixelCacheThreshold;
#endif
  (void) SetMagickResourceLimit(AreaResource,2*memory);
  (void) SetMagickResourceLimit(MemoryResource,memory);
  (void) SetMagickResourceLimit(MapResource,2*memory);
  limit=GetEnvironmentValue("MAGICK_AREA_LIMIT");
  if (limit == (char *) NULL)
    limit=GetPolicyValue("area");
  if (limit != (char *) NULL)
    {
      (void) SetMagickResourceLimit(AreaResource,StringToSizeType(limit,100.0));
      limit=DestroyString(limit);
    }
  limit=GetEnvironmentValue("MAGICK_MEMORY_LIMIT");
  if (limit == (char *) NULL)
    limit=GetPolicyValue("memory");
  if (limit != (char *) NULL)
    {
      (void) SetMagickResourceLimit(MemoryResource,
        StringToSizeType(limit,100.0));
      limit=DestroyString(limit);
    }
  limit=GetEnvironmentValue("MAGICK_MAP_LIMIT");
  if (limit == (char *) NULL)
    limit=GetPolicyValue("map");
  if (limit != (char *) NULL)
    {
      (void) SetMagickResourceLimit(MapResource,StringToSizeType(limit,100.0));
      limit=DestroyString(limit);
    }
  limit=GetEnvironmentValue("MAGICK_DISK_LIMIT");
  if (limit == (char *) NULL)
    limit=GetPolicyValue("disk");
  if (limit != (char *) NULL)
    {
      (void) SetMagickResourceLimit(DiskResource,StringToSizeType(limit,100.0));
      limit=DestroyString(limit);
    }
  files=(-1);
#if defined(MAGICKCORE_HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
  files=(ssize_t) sysconf(_SC_OPEN_MAX);
#endif
#if defined(MAGICKCORE_HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
  if (files < 0)
    {
      struct rlimit
        resources;

      if (getrlimit(RLIMIT_NOFILE,&resources) != -1)
        files=(ssize_t) resources.rlim_cur;
  }
#endif
#if defined(MAGICKCORE_HAVE_GETDTABLESIZE) && defined(MAGICKCORE_POSIX_SUPPORT)
  if (files < 0)
    files=(ssize_t) getdtablesize();
#endif
  if (files < 0)
    files=64;
  (void) SetMagickResourceLimit(FileResource,MagickMax((size_t)
    (3*files/4),64));
  limit=GetEnvironmentValue("MAGICK_FILE_LIMIT");
  if (limit == (char *) NULL)
    limit=GetPolicyValue("file");
  if (limit != (char *) NULL)
    {
      (void) SetMagickResourceLimit(FileResource,StringToSizeType(limit,100.0));
      limit=DestroyString(limit);
    }
  (void) SetMagickResourceLimit(ThreadResource,GetOpenMPMaximumThreads());
  limit=GetEnvironmentValue("MAGICK_THREAD_LIMIT");
  if (limit == (char *) NULL)
    limit=GetPolicyValue("thread");
  if (limit != (char *) NULL)
    {
      (void) SetMagickResourceLimit(ThreadResource,StringToSizeType(limit,
        100.0));
      limit=DestroyString(limit);
    }
  limit=GetEnvironmentValue("MAGICK_TIME_LIMIT");
  if (limit == (char *) NULL)
    limit=GetPolicyValue("time");
  if (limit != (char *) NULL)
    {
      (void) SetMagickResourceLimit(TimeResource,StringToSizeType(limit,100.0));
      limit=DestroyString(limit);
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e s o u r c e C o m p o n e n t T e r m i n u s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResourceComponentTerminus() destroys the resource component.
%
%  The format of the ResourceComponentTerminus() method is:
%
%      ResourceComponentTerminus(void)
%
*/
MagickExport void ResourceComponentTerminus(void)
{
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  if (temporary_resources != (SplayTreeInfo *) NULL)
    temporary_resources=DestroySplayTree(temporary_resources);
  if (random_info != (RandomInfo *) NULL)
    random_info=DestroyRandomInfo(random_info);
  UnlockSemaphoreInfo(resource_semaphore);
  DestroySemaphoreInfo(&resource_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t M a g i c k R e s o u r c e L i m i t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetMagickResourceLimit() sets the limit for a particular resource.
%
%  The format of the SetMagickResourceLimit() method is:
%
%      MagickBooleanType SetMagickResourceLimit(const ResourceType type,
%        const MagickSizeType limit)
%
%  A description of each parameter follows:
%
%    o type: the type of resource.
%
%    o limit: the maximum limit for the resource.
%
*/

static inline MagickSizeType MagickMin(const MagickSizeType x,
  const MagickSizeType y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport MagickBooleanType SetMagickResourceLimit(const ResourceType type,
  const MagickSizeType limit)
{
  char
    *value;

  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  value=(char *) NULL;
  switch (type)
  {
    case AreaResource:
    {
      resource_info.area_limit=limit;
      value=GetPolicyValue("area");
      if (value != (char *) NULL)
        resource_info.area_limit=MagickMin(limit,StringToSizeType(value,100.0));
      break;
    }
    case MemoryResource:
    {
      resource_info.memory_limit=limit;
      value=GetPolicyValue("memory");
      if (value != (char *) NULL)
        resource_info.memory_limit=MagickMin(limit,StringToSizeType(value,
          100.0));
      break;
    }
    case MapResource:
    {
      resource_info.map_limit=limit;
      value=GetPolicyValue("map");
      if (value != (char *) NULL)
        resource_info.map_limit=MagickMin(limit,StringToSizeType(value,100.0));
      break;
    }
    case DiskResource:
    {
      resource_info.disk_limit=limit;
      value=GetPolicyValue("disk");
      if (value != (char *) NULL)
        resource_info.disk_limit=MagickMin(limit,StringToSizeType(value,100.0));
      break;
    }
    case FileResource:
    {
      resource_info.file_limit=limit;
      value=GetPolicyValue("file");
      if (value != (char *) NULL)
        resource_info.file_limit=MagickMin(limit,StringToSizeType(value,100.0));
      break;
    }
    case ThreadResource:
    {
      resource_info.thread_limit=limit;
      value=GetPolicyValue("thread");
      if (value != (char *) NULL)
        resource_info.thread_limit=MagickMin(limit,StringToSizeType(value,
          100.0));
      if (resource_info.thread_limit > GetOpenMPMaximumThreads())
        resource_info.thread_limit=GetOpenMPMaximumThreads();
      break;
    }
    case TimeResource:
    {
      resource_info.time_limit=limit;
      value=GetPolicyValue("time");
      if (value != (char *) NULL)
        resource_info.time_limit=MagickMin(limit,StringToSizeType(value,100.0));
      break;
    }
    default:
      break;
  }
  if (value != (char *) NULL)
    value=DestroyString(value);
  UnlockSemaphoreInfo(resource_semaphore);
  return(MagickTrue);
}

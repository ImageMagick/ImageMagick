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
#include "magick/splay-tree.h"
#include "magick/thread-private.h"
#include "magick/token.h"
#include "magick/utility.h"

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
    MagickULLConstant(2048)*1024*1024,
    MagickULLConstant(1536)*1024*1024,
    MagickULLConstant(8192)*1024*1024,
    MagickResourceInfinity,
    MagickULLConstant(768),
    MagickULLConstant(8),
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
  (void) FormatMagickSize(size,resource_request);
  AcquireSemaphoreInfo(&resource_semaphore);
  switch (type)
  {
    case AreaResource:
    {
      resource_info.area=(MagickOffsetType) size;
      limit=resource_info.area_limit;
      status=(resource_info.area_limit == MagickResourceInfinity) ||
        (size < limit) ? MagickTrue : MagickFalse;
      (void) FormatMagickSize((MagickSizeType) resource_info.area,
        resource_current);
      (void) FormatMagickSize(resource_info.area_limit,resource_limit);
      break;
    }
    case MemoryResource:
    {
      resource_info.memory+=size;
      limit=resource_info.memory_limit;
      status=(resource_info.memory_limit == MagickResourceInfinity) ||
        ((MagickSizeType) resource_info.memory < limit) ?
        MagickTrue : MagickFalse;
      (void) FormatMagickSize((MagickSizeType) resource_info.memory,
        resource_current);
      (void) FormatMagickSize(resource_info.memory_limit,
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
      (void) FormatMagickSize((MagickSizeType) resource_info.map,
        resource_current);
      (void) FormatMagickSize(resource_info.map_limit,
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
      (void) FormatMagickSize((MagickSizeType) resource_info.disk,
        resource_current);
      (void) FormatMagickSize(resource_info.disk_limit,resource_limit);
      break;
    }
    case FileResource:
    {
      resource_info.file+=size;
      limit=resource_info.file_limit;
      status=(resource_info.file_limit == MagickResourceInfinity) ||
        ((MagickSizeType) resource_info.file < limit) ?
        MagickTrue : MagickFalse;
      (void) FormatMagickSize((MagickSizeType) resource_info.file,
        resource_current);
      (void) FormatMagickSize((MagickSizeType) resource_info.file_limit,
        resource_limit);
      break;
    }
    case ThreadResource:
    {
      resource_info.thread+=size;
      limit=resource_info.thread_limit;
      status=(resource_info.thread_limit == MagickResourceInfinity) ||
        ((MagickSizeType) resource_info.thread < limit) ?
        MagickTrue : MagickFalse;
      (void) FormatMagickSize((MagickSizeType) resource_info.thread,
        resource_current);
      (void) FormatMagickSize((MagickSizeType) resource_info.thread_limit,
        resource_limit);
      break;
    }
    case TimeResource:
    {
      resource_info.time+=size;
      limit=resource_info.time_limit;
      status=(resource_info.time_limit == MagickResourceInfinity) ||
        ((MagickSizeType) resource_info.time < limit) ?
        MagickTrue : MagickFalse;
      (void) FormatMagickSize((MagickSizeType) resource_info.time,
        resource_current);
      (void) FormatMagickSize((MagickSizeType) resource_info.time_limit,
        resource_limit);
      break;
    }
    default:
      break;
  }
  RelinquishSemaphoreInfo(resource_semaphore);
  (void) LogMagickEvent(ResourceEvent,GetMagickModule(),"%s: %s/%s/%s",
    MagickOptionToMnemonic(MagickResourceOptions,(long) type),resource_request,
    resource_current,resource_limit);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A s y n c h r o n o u s D e s t r o y M a g i c k R e s o u r c e s       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AsynchronousDestroyMagickResources() destroys the resource environment.
%  It differs from DestroyMagickResources() in that it can be called from a
%  asynchronous signal handler.
%
%  The format of the DestroyMagickResources() method is:
%
%      DestroyMagickResources(void)
%
*/
MagickExport void AsynchronousDestroyMagickResources(void)
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
    (void) remove(path);
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
  (void) remove((char *) temporary_resource);
  return((void *) NULL);
}

static MagickBooleanType GetPathTemplate(char *path)
{
  char
    *directory;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  register char
    *p;

  struct stat
    attributes;

  (void) CopyMagickString(path,"magick-XXXXXXXX",MaxTextExtent);
  exception=AcquireExceptionInfo();
  directory=(char *) GetImageRegistry(StringRegistryType,"temporary-path",
    exception);
  exception=DestroyExceptionInfo(exception);
  if (directory == (char *) NULL)
    directory=GetEnvironmentValue("MAGICK_TEMPORARY_PATH");
  if (directory == (char *) NULL)
    directory=GetEnvironmentValue("MAGICK_TMPDIR");
  if (directory == (char *) NULL)
    directory=GetPolicyValue("temporary-path");
  if (directory == (char *) NULL)
    directory=GetEnvironmentValue("TMPDIR");
#if defined(__WINDOWS__) || defined(__OS2__)
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
    return(MagickTrue);
  if (strlen(directory) > (MaxTextExtent-15))
    {
      directory=DestroyString(directory);
      return(MagickTrue);
    }
  status=GetPathAttributes(directory,&attributes);
  if ((status == MagickFalse) || !S_ISDIR(attributes.st_mode))
    {
      directory=DestroyString(directory);
      return(MagickTrue);
    }
  if (directory[strlen(directory)-1] == *DirectorySeparator)
    (void) FormatMagickString(path,MaxTextExtent,"%smagick-XXXXXXXX",directory);
  else
    (void) FormatMagickString(path,MaxTextExtent,"%s%smagick-XXXXXXXX",
      directory,DirectorySeparator);
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

  char
    *resource;

  int
    c,
    file;

  register char
    *p;

  register long
    i;

  static const char
    portable_filename[65] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

  StringInfo
    *key;

  unsigned char
    *datum;

  assert(path != (char *) NULL);
  (void) LogMagickEvent(ResourceEvent,GetMagickModule(),"%s",path);
  if (random_info == (RandomInfo *) NULL)
    random_info=AcquireRandomInfo();
  file=(-1);
  for (i=0; i < TMP_MAX; i++)
  {
    /*
      Get temporary pathname.
    */
    (void) GetPathTemplate(path);
#if defined(MAGICKCORE_HAVE_MKSTEMP)
    file=mkstemp(path);
#if defined(__OS2__)
    setmode(file,O_BINARY);
#endif
    if (file != -1)
      break;
#endif
    key=GetRandomKey(random_info,8);
    p=path+strlen(path)-8;
    datum=GetStringInfoDatum(key);
    for (i=0; i < 8; i++)
    {
      c=(int) (datum[i] & 0x3f);
      *p++=portable_filename[c];
    }
    key=DestroyStringInfo(key);
    file=open(path,O_RDWR | O_CREAT | O_EXCL | O_BINARY | O_NOFOLLOW,S_MODE);
    if ((file > 0) || (errno != EEXIST))
      break;
  }
  (void) LogMagickEvent(ResourceEvent,GetMagickModule(),"%s",path);
  if (file == -1)
    return(file);
  AcquireSemaphoreInfo(&resource_semaphore);
  if (temporary_resources == (SplayTreeInfo *) NULL)
    temporary_resources=NewSplayTree(CompareSplayTreeString,
      RelinquishMagickMemory,DestroyTemporaryResources);
  RelinquishSemaphoreInfo(resource_semaphore);
  resource=ConstantString(path);
  (void) AddValueToSplayTree(temporary_resources,resource,resource);
  return(file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y M a g i c k R e s o u r c e s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyMagickResources() destroys the resource environment.
%
%  The format of the DestroyMagickResources() method is:
%
%      DestroyMagickResources(void)
%
*/
MagickExport void DestroyMagickResources(void)
{
  AcquireSemaphoreInfo(&resource_semaphore);
  if (temporary_resources != (SplayTreeInfo *) NULL)
    temporary_resources=DestroySplayTree(temporary_resources);
  if (random_info != (RandomInfo *) NULL)
    random_info=DestroyRandomInfo(random_info);
  RelinquishSemaphoreInfo(resource_semaphore);
  DestroySemaphoreInfo(&resource_semaphore);
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
  AcquireSemaphoreInfo(&resource_semaphore);
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
  RelinquishSemaphoreInfo(resource_semaphore);
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
%  GetMagickResource() returns the specified resource limit.
%
%  The format of the GetMagickResourceLimit() method is:
%
%      unsigned long GetMagickResourceLimit(const ResourceType type)
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
  AcquireSemaphoreInfo(&resource_semaphore);
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
  RelinquishSemaphoreInfo(resource_semaphore);
  return(resource);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t i a l i z e M a g i c k R e s o u r c e s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeMagickResources() initializes the resource environment.
%
%  The format of the InitializeMagickResources() method is:
%
%      InitializeMagickResources(void)
%
*/

static inline unsigned long MagickMax(const unsigned long x,
  const unsigned long y)
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

  value=StringToDouble(string,interval);
  if (value >= (double) MagickULLConstant(~0))
    return(MagickULLConstant(~0));
  return((MagickSizeType) value);
}

MagickExport void InitializeMagickResources(void)
{
  char
    *limit;

  long
    files,
    pages,
    pagesize;

  MagickSizeType
    memory;

  /*
    Set Magick resource limits.
  */
  pagesize=(-1);
#if defined(MAGICKCORE_HAVE_SYSCONF) && defined(_SC_PAGESIZE)
  pagesize=sysconf(_SC_PAGESIZE);
#elif defined(MAGICKCORE_HAVE_GETPAGESIZE) && defined(MAGICKCORE_POSIX_SUPPORT)
  pagesize=getpagesize();
#endif
  pages=(-1);
#if defined(MAGICKCORE_HAVE_SYSCONF) && defined(_SC_PHYS_PAGES)
  pages=sysconf(_SC_PHYS_PAGES);
#endif
  memory=(MagickSizeType) pages*pagesize;
  if ((pagesize <= 0) || (pages <= 0))
    memory=2048UL*1024UL*1024UL;
#if defined(PixelCacheThreshold)
  memory=PixelCacheThreshold;
#endif
  (void) SetMagickResourceLimit(AreaResource,2UL*memory);
  (void) SetMagickResourceLimit(MemoryResource,3UL*memory/2UL);
  (void) SetMagickResourceLimit(MapResource,4UL*memory);
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
  files=sysconf(_SC_OPEN_MAX);
#endif
#if defined(MAGICKCORE_HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
  if (files < 0)
    {
      struct rlimit
        resources;

      if (getrlimit(RLIMIT_NOFILE,&resources) != -1)
        files=resources.rlim_cur;
  }
#endif
#if defined(MAGICKCORE_HAVE_GETDTABLESIZE) && defined(MAGICKCORE_POSIX_SUPPORT)
  if (files < 0)
    files=getdtablesize();
#endif
  if (files < 0)
    files=64;
  (void) SetMagickResourceLimit(FileResource,MagickMax((unsigned long)
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
      SetOpenMPMaximumThreads((unsigned long) atol(limit));
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
  AcquireSemaphoreInfo(&resource_semaphore);
  (void) FormatMagickSize(resource_info.area_limit,area_limit);
  (void) FormatMagickSize(resource_info.memory_limit,memory_limit);
  (void) FormatMagickSize(resource_info.map_limit,map_limit);
  (void) FormatMagickSize(resource_info.disk_limit,disk_limit);
  (void) CopyMagickString(time_limit,"unlimited",MaxTextExtent);
  if (resource_info.time_limit != MagickResourceInfinity)
    (void) FormatMagickString(time_limit,MaxTextExtent,"%lu",(unsigned long)
      resource_info.time_limit);
  (void) fprintf(file,"File       Area     Memory        Map"
    "       Disk  Thread       Time\n");
  (void) fprintf(file,"-----------------------------------------------------"
    "--------------\n");
  (void) fprintf(file,"%4lu  %9s  %9s  %9s  %9s  %6lu  %9s\n",(unsigned long)
    resource_info.file_limit,area_limit,memory_limit,map_limit,disk_limit,
    (unsigned long) resource_info.thread_limit,time_limit);
  (void) fflush(file);
  RelinquishSemaphoreInfo(resource_semaphore);
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

  (void) FormatMagickSize(size,resource_request);
  AcquireSemaphoreInfo(&resource_semaphore);
  switch (type)
  {
    case AreaResource:
    {
      resource_info.area=(MagickOffsetType) size;
      (void) FormatMagickSize((MagickSizeType) resource_info.area,
        resource_current);
      (void) FormatMagickSize(resource_info.area_limit,resource_limit);
      break;
    }
    case MemoryResource:
    {
      resource_info.memory-=size;
      (void) FormatMagickSize((MagickSizeType) resource_info.memory,
        resource_current);
      (void) FormatMagickSize(resource_info.memory_limit,resource_limit);
      break;
    }
    case MapResource:
    {
      resource_info.map-=size;
      (void) FormatMagickSize((MagickSizeType) resource_info.map,
        resource_current);
      (void) FormatMagickSize(resource_info.map_limit,resource_limit);
      break;
    }
    case DiskResource:
    {
      resource_info.disk-=size;
      (void) FormatMagickSize((MagickSizeType) resource_info.disk,
        resource_current);
      (void) FormatMagickSize(resource_info.disk_limit,resource_limit);
      break;
    }
    case FileResource:
    {
      resource_info.file-=size;
      (void) FormatMagickSize((MagickSizeType) resource_info.file,
        resource_current);
      (void) FormatMagickSize((MagickSizeType) resource_info.file_limit,
        resource_limit);
      break;
    }
    case ThreadResource:
    {
      resource_info.thread-=size;
      (void) FormatMagickSize((MagickSizeType) resource_info.thread,
        resource_current);
      (void) FormatMagickSize((MagickSizeType) resource_info.thread_limit,
        resource_limit);
      break;
    }
    case TimeResource:
    {
      resource_info.time-=size;
      (void) FormatMagickSize((MagickSizeType) resource_info.time,
        resource_current);
      (void) FormatMagickSize((MagickSizeType) resource_info.time_limit,
        resource_limit);
      break;
    }
    default:
      break;
  }
  RelinquishSemaphoreInfo(resource_semaphore);
  (void) LogMagickEvent(ResourceEvent,GetMagickModule(),"%s: %s/%s/%s",
    MagickOptionToMnemonic(MagickResourceOptions,(long) type),resource_request,
    resource_current,resource_limit);
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
  (void) LogMagickEvent(ResourceEvent,GetMagickModule(),"%s",path);
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
  (void) remove(cache_path);
  return(remove(path) == 0 ? MagickTrue : MagickFalse);
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
MagickExport MagickBooleanType SetMagickResourceLimit(const ResourceType type,
  const MagickSizeType limit)
{
  AcquireSemaphoreInfo(&resource_semaphore);
  switch (type)
  {
    case AreaResource:
    {
      resource_info.area_limit=limit;
      break;
    }
    case MemoryResource:
    {
      resource_info.memory_limit=limit;
      break;
    }
    case MapResource:
    {
      resource_info.map_limit=limit;
      break;
    }
    case DiskResource:
    {
      resource_info.disk_limit=limit;
      break;
    }
    case FileResource:
    {
      resource_info.file_limit=limit;
      break;
    }
    case ThreadResource:
    {
      SetOpenMPMaximumThreads((unsigned long) limit);
      resource_info.thread_limit=GetOpenMPMaximumThreads();
      break;
    }
    case TimeResource:
    {
      resource_info.time_limit=limit;
      break;
    }
    default:
      break;
  }
  RelinquishSemaphoreInfo(resource_semaphore);
  return(MagickTrue);
}

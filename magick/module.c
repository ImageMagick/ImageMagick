/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  M   M   OOO   DDDD   U   U  L      EEEEE                   %
%                  MM MM  O   O  D   D  U   U  L      E                       %
%                  M M M  O   O  D   D  U   U  L      EEE                     %
%                  M   M  O   O  D   D  U   U  L      E                       %
%                  M   M   OOO   DDDD    UUU   LLLLL  EEEEE                   %
%                                                                             %
%                                                                             %
%                          MagickCore Module Methods                          %
%                                                                             %
%                              Software Design                                %
%                              Bob Friesenhahn                                %
%                                March 2000                                   %
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
#include "magick/coder.h"
#include "magick/client.h"
#include "magick/configure.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/log.h"
#include "magick/hashmap.h"
#include "magick/magic.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/policy.h"
#include "magick/semaphore.h"
#include "magick/splay-tree.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/token.h"
#include "magick/utility.h"
#if defined(MAGICKCORE_MODULES_SUPPORT)
#if defined(MAGICKCORE_LTDL_DELEGATE)
#include "ltdl.h"
typedef lt_dlhandle ModuleHandle;
#else
typedef void *ModuleHandle;
#endif

/*
  Define declarations.
*/
#if defined(MAGICKCORE_LTDL_DELEGATE)
#  define ModuleGlobExpression "*.la"
#else
#  if defined(_DEBUG)
#    define ModuleGlobExpression "IM_MOD_DB_*.dll"
#  else
#    define ModuleGlobExpression "IM_MOD_RL_*.dll"
#  endif
#endif

/*
  Global declarations.
*/
static SemaphoreInfo
  *module_semaphore = (SemaphoreInfo *) NULL;

static SplayTreeInfo
  *module_list = (SplayTreeInfo *) NULL;

static volatile MagickBooleanType
  instantiate_module = MagickFalse;

/*
  Forward declarations.
*/
static const ModuleInfo
  *RegisterModule(const ModuleInfo *,ExceptionInfo *);

static MagickBooleanType
  GetMagickModulePath(const char *,MagickModuleType,char *,ExceptionInfo *),
  UnregisterModule(const ModuleInfo *,ExceptionInfo *);

static void
  TagToCoderModuleName(const char *,char *),
  TagToFilterModuleName(const char *,char *),
  TagToModuleName(const char *,const char *,char *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e M o d u l e I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireModuleInfo() allocates the ModuleInfo structure.
%
%  The format of the AcquireModuleInfo method is:
%
%      ModuleInfo *AcquireModuleInfo(const char *path,const char *tag)
%
%  A description of each parameter follows:
%
%    o path: the path associated with the tag.
%
%    o tag: a character string that represents the image format we are
%      looking for.
%
*/
MagickExport ModuleInfo *AcquireModuleInfo(const char *path,const char *tag)
{
  ModuleInfo
    *module_info;

  module_info=(ModuleInfo *) AcquireMagickMemory(sizeof(*module_info));
  if (module_info == (ModuleInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(module_info,0,sizeof(*module_info));
  if (path != (const char *) NULL)
    module_info->path=ConstantString(path);
  if (tag != (const char *) NULL)
    module_info->tag=ConstantString(tag);
  module_info->timestamp=time(0);
  module_info->signature=MagickSignature;
  return(module_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y M o d u l e L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyModuleList() unregisters any previously loaded modules and exits
%  the module loaded environment.
%
%  The format of the DestroyModuleList module is:
%
%      void DestroyModuleList(void)
%
*/
MagickExport void DestroyModuleList(void)
{
  /*
    Destroy magick modules.
  */
  LockSemaphoreInfo(module_semaphore);
#if defined(MAGICKCORE_MODULES_SUPPORT)
  if (module_list != (SplayTreeInfo *) NULL)
    module_list=DestroySplayTree(module_list);
  if (instantiate_module != MagickFalse)
    {
#if !defined(MAGICKCORE_JP2_DELEGATE)
      (void) lt_dlexit();  /* Jasper has an errant atexit() handler */
#endif
      instantiate_module=MagickFalse;
    }
#endif
  UnlockSemaphoreInfo(module_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M o d u l e I n f o                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetModuleInfo() returns a pointer to a ModuleInfo structure that matches the
%  specified tag.  If tag is NULL, the head of the module list is returned. If
%  no modules are loaded, or the requested module is not found, NULL is
%  returned.
%
%  The format of the GetModuleInfo module is:
%
%      ModuleInfo *GetModuleInfo(const char *tag,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o tag: a character string that represents the image format we are
%      looking for.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ModuleInfo *GetModuleInfo(const char *tag,ExceptionInfo *exception)
{
  if ((module_list == (SplayTreeInfo *) NULL) ||
      (instantiate_module == MagickFalse))
    if (InitializeModuleList(exception) == MagickFalse)
      return((ModuleInfo *) NULL);
  if ((module_list == (SplayTreeInfo *) NULL) ||
      (GetNumberOfNodesInSplayTree(module_list) == 0))
    return((ModuleInfo *) NULL);
  if ((tag == (const char *) NULL) || (LocaleCompare(tag,"*") == 0))
    {
      ModuleInfo
        *p;

#if defined(MAGICKCORE_MODULES_SUPPORT)
      if (LocaleCompare(tag,"*") == 0)
        (void) OpenModules(exception);
#endif
      LockSemaphoreInfo(module_semaphore);
      ResetSplayTreeIterator(module_list);
      p=(ModuleInfo *) GetNextValueInSplayTree(module_list);
      UnlockSemaphoreInfo(module_semaphore);
      return(p);
    }
  return((ModuleInfo *) GetValueFromSplayTree(module_list,tag));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M o d u l e I n f o L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetModuleInfoList() returns any modules that match the specified pattern.
%
%  The format of the GetModuleInfoList function is:
%
%      const ModuleInfo **GetModuleInfoList(const char *pattern,
%        size_t *number_modules,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_modules:  This integer returns the number of modules in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int ModuleInfoCompare(const void *x,const void *y)
{
  const ModuleInfo
    **p,
    **q;

  p=(const ModuleInfo **) x,
  q=(const ModuleInfo **) y;
  if (LocaleCompare((*p)->path,(*q)->path) == 0)
    return(LocaleCompare((*p)->tag,(*q)->tag));
  return(LocaleCompare((*p)->path,(*q)->path));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport const ModuleInfo **GetModuleInfoList(const char *pattern,
  size_t *number_modules,ExceptionInfo *exception)
{
  const ModuleInfo
    **modules;

  register const ModuleInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate module list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_modules != (size_t *) NULL);
  *number_modules=0;
  p=GetModuleInfo("*",exception);
  if (p == (const ModuleInfo *) NULL)
    return((const ModuleInfo **) NULL);
  modules=(const ModuleInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfNodesInSplayTree(module_list)+1UL,sizeof(*modules));
  if (modules == (const ModuleInfo **) NULL)
    return((const ModuleInfo **) NULL);
  /*
    Generate module list.
  */
  LockSemaphoreInfo(module_semaphore);
  ResetSplayTreeIterator(module_list);
  p=(const ModuleInfo *) GetNextValueInSplayTree(module_list);
  for (i=0; p != (const ModuleInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->tag,pattern,MagickFalse) != MagickFalse))
      modules[i++]=p;
    p=(const ModuleInfo *) GetNextValueInSplayTree(module_list);
  }
  UnlockSemaphoreInfo(module_semaphore);
  qsort((void *) modules,(size_t) i,sizeof(*modules),ModuleInfoCompare);
  modules[i]=(ModuleInfo *) NULL;
  *number_modules=(size_t) i;
  return(modules);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M o d u l e L i s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetModuleList() returns any image format modules that match the specified
%  pattern.
%
%  The format of the GetModuleList function is:
%
%      char **GetModuleList(const char *pattern,const MagickModuleType type,
%        size_t *number_modules,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o type: choose from MagickImageCoderModule or MagickImageFilterModule.
%
%    o number_modules:  This integer returns the number of modules in the
%      list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int ModuleCompare(const void *x,const void *y)
{
  register const char
    **p,
    **q;

   p=(const char **) x;
  q=(const char **) y;
  return(LocaleCompare(*p,*q));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static inline int MagickReadDirectory(DIR *directory,struct dirent *entry,
  struct dirent **result)
{
#if defined(MAGICKCORE_HAVE_READDIR_R)
  return(readdir_r(directory,entry,result));
#else
  (void) entry;
  errno=0;
  *result=readdir(directory);
  return(errno);
#endif
}

MagickExport char **GetModuleList(const char *pattern,
  const MagickModuleType type,size_t *number_modules,ExceptionInfo *exception)
{
  char
    **modules,
    filename[MaxTextExtent],
    module_path[MaxTextExtent],
    path[MaxTextExtent];

  DIR
    *directory;

  MagickBooleanType
    status;

  register ssize_t
    i;

  size_t
    max_entries;

  struct dirent
    *buffer,
    *entry;

  /*
    Locate all modules in the image coder or filter path.
  */
  switch (type)
  {
    case MagickImageCoderModule:
    default:
    {
      TagToCoderModuleName("magick",filename);
      status=GetMagickModulePath(filename,MagickImageCoderModule,module_path,
        exception);
      break;
    }
    case MagickImageFilterModule:
    {
      TagToFilterModuleName("analyze",filename);
      status=GetMagickModulePath(filename,MagickImageFilterModule,module_path,
        exception);
      break;
    }
  }
  if (status == MagickFalse)
    return((char **) NULL);
  GetPathComponent(module_path,HeadPath,path);
  max_entries=255;
  modules=(char **) AcquireQuantumMemory((size_t) max_entries+1UL,
    sizeof(*modules));
  if (modules == (char **) NULL)
    return((char **) NULL);
  *modules=(char *) NULL;
  directory=opendir(path);
  if (directory == (DIR *) NULL)
    {
      modules=(char **) RelinquishMagickMemory(modules);
      return((char **) NULL);
    }
  buffer=(struct dirent *) AcquireMagickMemory(sizeof(*buffer)+
    FILENAME_MAX+1);
  if (buffer == (struct dirent *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  i=0;
  while ((MagickReadDirectory(directory,buffer,&entry) == 0) &&
         (entry != (struct dirent *) NULL))
  {
    status=GlobExpression(entry->d_name,ModuleGlobExpression,MagickFalse);
    if (status == MagickFalse)
      continue;
    if (GlobExpression(entry->d_name,pattern,MagickFalse) == MagickFalse)
      continue;
    if (i >= (ssize_t) max_entries)
      {
        modules=(char **) NULL;
        if (~max_entries > max_entries)
          modules=(char **) ResizeQuantumMemory(modules,(size_t)
            (max_entries << 1),sizeof(*modules));
        max_entries<<=1;
        if (modules == (char **) NULL)
          break;
      }
    /*
      Add new module name to list.
    */
    modules[i]=AcquireString((char *) NULL);
    GetPathComponent(entry->d_name,BasePath,modules[i]);
    if (LocaleNCompare("IM_MOD_",modules[i],7) == 0)
      {
        (void) CopyMagickString(modules[i],modules[i]+10,MaxTextExtent);
        modules[i][strlen(modules[i])-1]='\0';
      }
    i++;
  }
  buffer=(struct dirent *) RelinquishMagickMemory(buffer);
  (void) closedir(directory);
  if (modules == (char **) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ConfigureError,
        "MemoryAllocationFailed","`%s'",pattern);
      return((char **) NULL);
    }
  qsort((void *) modules,(size_t) i,sizeof(*modules),ModuleCompare);
  modules[i]=(char *) NULL;
  *number_modules=(size_t) i;
  return(modules);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t M a g i c k M o d u l e P a t h                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickModulePath() finds a module with the specified module type and
%  filename.
%
%  The format of the GetMagickModulePath module is:
%
%      MagickBooleanType GetMagickModulePath(const char *filename,
%        MagickModuleType module_type,char *path,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the module file name.
%
%    o module_type: the module type: MagickImageCoderModule or
%      MagickImageFilterModule.
%
%    o path: the path associated with the filename.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType GetMagickModulePath(const char *filename,
  MagickModuleType module_type,char *path,ExceptionInfo *exception)
{
  char
    *module_path;

  assert(filename != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  assert(path != (char *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  (void) CopyMagickString(path,filename,MaxTextExtent);
  module_path=(char *) NULL;
  switch (module_type)
  {
    case MagickImageCoderModule:
    default:
    {
      (void) LogMagickEvent(ModuleEvent,GetMagickModule(),
        "Searching for coder module file \"%s\" ...",filename);
      module_path=GetEnvironmentValue("MAGICK_CODER_MODULE_PATH");
#if defined(MAGICKCORE_CODER_PATH)
      if (module_path == (char *) NULL)
        module_path=AcquireString(MAGICKCORE_CODER_PATH);
#endif
      break;
    }
    case MagickImageFilterModule:
    {
      (void) LogMagickEvent(ModuleEvent,GetMagickModule(),
        "Searching for filter module file \"%s\" ...",filename);
      module_path=GetEnvironmentValue("MAGICK_CODER_FILTER_PATH");
#if defined(MAGICKCORE_FILTER_PATH)
      if (module_path == (char *) NULL)
        module_path=AcquireString(MAGICKCORE_FILTER_PATH);
#endif
      break;
    }
  }
  if (module_path != (char *) NULL)
    {
      register char
        *p,
        *q;

      for (p=module_path-1; p != (char *) NULL; )
      {
        (void) CopyMagickString(path,p+1,MaxTextExtent);
        q=strchr(path,DirectoryListSeparator);
        if (q != (char *) NULL)
          *q='\0';
        q=path+strlen(path)-1;
        if ((q >= path) && (*q != *DirectorySeparator))
          (void) ConcatenateMagickString(path,DirectorySeparator,MaxTextExtent);
        (void) ConcatenateMagickString(path,filename,MaxTextExtent);
        if (IsPathAccessible(path) != MagickFalse)
          {
            module_path=DestroyString(module_path);
            return(MagickTrue);
          }
        p=strchr(p+1,DirectoryListSeparator);
      }
      module_path=DestroyString(module_path);
    }
#if defined(MAGICKCORE_INSTALLED_SUPPORT)
  else
#if defined(MAGICKCORE_CODER_PATH)
    {
      const char
        *directory;

      /*
        Search hard coded paths.
      */
      switch (module_type)
      {
        case MagickImageCoderModule:
        default:
        {
          directory=MAGICKCORE_CODER_PATH;
          break;
        }
        case MagickImageFilterModule:
        {
          directory=MAGICKCORE_FILTER_PATH;
          break;
        }
      }
      (void) FormatLocaleString(path,MaxTextExtent,"%s%s",directory,filename);
      if (IsPathAccessible(path) == MagickFalse)
        {
          ThrowFileException(exception,ConfigureWarning,
            "UnableToOpenModuleFile",path);
          return(MagickFalse);
        }
      return(MagickTrue);
    }
#else
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
    {
      const char
        *registery_key;

      unsigned char
        *key_value;

      /*
        Locate path via registry key.
      */
      switch (module_type)
      {
        case MagickImageCoderModule:
        default:
        {
          registery_key="CoderModulesPath";
          break;
        }
        case MagickImageFilterModule:
        {
          registery_key="FilterModulesPath";
          break;
        }
      }
      key_value=NTRegistryKeyLookup(registery_key);
      if (key_value == (unsigned char *) NULL)
        {
          ThrowMagickException(exception,GetMagickModule(),ConfigureError,
            "RegistryKeyLookupFailed","`%s'",registery_key);
          return(MagickFalse);
        }
      (void) FormatLocaleString(path,MaxTextExtent,"%s%s%s",(char *) key_value,
        DirectorySeparator,filename);
      key_value=(unsigned char *) RelinquishMagickMemory(key_value);
      if (IsPathAccessible(path) == MagickFalse)
        {
          ThrowFileException(exception,ConfigureWarning,
            "UnableToOpenModuleFile",path);
          return(MagickFalse);
        }
      return(MagickTrue);
    }
#endif
#endif
#if !defined(MAGICKCORE_CODER_PATH) && !defined(MAGICKCORE_WINDOWS_SUPPORT)
# error MAGICKCORE_CODER_PATH or MAGICKCORE_WINDOWS_SUPPORT must be defined when MAGICKCORE_INSTALLED_SUPPORT is defined
#endif
#else
  {
    char
      *home;

    home=GetEnvironmentValue("MAGICK_HOME");
    if (home != (char *) NULL)
      {
        /*
          Search MAGICK_HOME.
        */
#if !defined(MAGICKCORE_POSIX_SUPPORT)
        (void) FormatLocaleString(path,MaxTextExtent,"%s%s%s",home,
          DirectorySeparator,filename);
#else
        const char
          *directory;

        switch (module_type)
        {
          case MagickImageCoderModule:
          default:
          {
            directory=MAGICKCORE_CODER_RELATIVE_PATH;
            break;
          }
          case MagickImageFilterModule:
          {
            directory=MAGICKCORE_FILTER_RELATIVE_PATH;
            break;
          }
        }
        (void) FormatLocaleString(path,MaxTextExtent,"%s/lib/%s/%s",home,
          directory,filename);
#endif
        home=DestroyString(home);
        if (IsPathAccessible(path) != MagickFalse)
          return(MagickTrue);
      }
  }
  if (*GetClientPath() != '\0')
    {
      /*
        Search based on executable directory.
      */
#if !defined(MAGICKCORE_POSIX_SUPPORT)
      (void) FormatLocaleString(path,MaxTextExtent,"%s%s%s",GetClientPath(),
        DirectorySeparator,filename);
#else
      char
        prefix[MaxTextExtent];

      const char
        *directory;

      switch (module_type)
      {
        case MagickImageCoderModule:
        default:
        {
          directory="coders";
          break;
        }
        case MagickImageFilterModule:
        {
          directory="filters";
          break;
        }
      }
      (void) CopyMagickString(prefix,GetClientPath(),MaxTextExtent);
      ChopPathComponents(prefix,1);
      (void) FormatLocaleString(path,MaxTextExtent,"%s/lib/%s/%s/%s",prefix,
        MAGICKCORE_MODULES_RELATIVE_PATH,directory,filename);
#endif
      if (IsPathAccessible(path) != MagickFalse)
        return(MagickTrue);
    }
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  {
    /*
      Search module path.
    */
    if ((NTGetModulePath("CORE_RL_magick_.dll",path) != MagickFalse) ||
        (NTGetModulePath("CORE_DB_magick_.dll",path) != MagickFalse) ||
        (NTGetModulePath("Magick.dll",path) != MagickFalse))
      {
        (void) ConcatenateMagickString(path,DirectorySeparator,MaxTextExtent);
        (void) ConcatenateMagickString(path,filename,MaxTextExtent);
        if (IsPathAccessible(path) != MagickFalse)
          return(MagickTrue);
      }
  }
#endif
  {
    char
      *home;

    home=GetEnvironmentValue("HOME");
    if (home == (char *) NULL)
      home=GetEnvironmentValue("USERPROFILE");
    if (home != (char *) NULL)
      {
        /*
          Search $HOME/.magick.
        */
        (void) FormatLocaleString(path,MaxTextExtent,"%s%s.magick%s%s",home,
          DirectorySeparator,DirectorySeparator,filename);
        home=DestroyString(home);
        if (IsPathAccessible(path) != MagickFalse)
          return(MagickTrue);
      }
  }
  /*
    Search current directory.
  */
  if (IsPathAccessible(path) != MagickFalse)
    return(MagickTrue);
  if (exception->severity < ConfigureError)
    ThrowFileException(exception,ConfigureWarning,"UnableToOpenModuleFile",
      path);
  return(MagickFalse);
#endif
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n i t i a l i z e M o d u l e L i s t                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeModuleList() initializes the module loader.
%
%  The format of the InitializeModuleList() method is:
%
%      InitializeModuleList(Exceptioninfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void *DestroyModuleNode(void *module_info)
{
  ExceptionInfo
    *exception;

  register ModuleInfo
    *p;

  exception=AcquireExceptionInfo();
  p=(ModuleInfo *) module_info;
  if (UnregisterModule(p,exception) == MagickFalse)
    CatchException(exception);
  if (p->tag != (char *) NULL)
    p->tag=DestroyString(p->tag);
  if (p->path != (char *) NULL)
    p->path=DestroyString(p->path);
  exception=DestroyExceptionInfo(exception);
  return(RelinquishMagickMemory(p));
}

MagickExport MagickBooleanType InitializeModuleList(
  ExceptionInfo *magick_unused(exception))
{
  if ((module_list == (SplayTreeInfo *) NULL) &&
      (instantiate_module == MagickFalse))
    {
      if (module_semaphore == (SemaphoreInfo *) NULL)
        AcquireSemaphoreInfo(&module_semaphore);
      LockSemaphoreInfo(module_semaphore);
      if ((module_list == (SplayTreeInfo *) NULL) &&
          (instantiate_module == MagickFalse))
        {
          MagickBooleanType
            status;

          ModuleInfo
            *module_info;

          module_list=NewSplayTree(CompareSplayTreeString,
            (void *(*)(void *)) NULL,DestroyModuleNode);
          if (module_list == (SplayTreeInfo *) NULL)
            ThrowFatalException(ResourceLimitFatalError,
              "MemoryAllocationFailed");
          module_info=AcquireModuleInfo((const char *) NULL,"[boot-strap]");
          module_info->stealth=MagickTrue;
          status=AddValueToSplayTree(module_list,module_info->tag,module_info);
          if (status == MagickFalse)
            ThrowFatalException(ResourceLimitFatalError,
              "MemoryAllocationFailed");
          if (lt_dlinit() != 0)
            ThrowFatalException(ModuleFatalError,
              "UnableToInitializeModuleLoader");
          instantiate_module=MagickTrue;
        }
      UnlockSemaphoreInfo(module_semaphore);
    }
  return(module_list != (SplayTreeInfo *) NULL ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n v o k e D y n a m i c I m a g e F i l t e r                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InvokeDynamicImageFilter() invokes a dynamic image filter.
%
%  The format of the InvokeDynamicImageFilter module is:
%
%      MagickBooleanType InvokeDynamicImageFilter(const char *tag,Image **image,
%        const int argc,const char **argv,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o tag: a character string that represents the name of the particular
%      module.
%
%    o image: the image.
%
%    o argc: a pointer to an integer describing the number of elements in the
%      argument vector.
%
%    o argv: a pointer to a text array containing the command line arguments.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType InvokeDynamicImageFilter(const char *tag,
  Image **images,const int argc,const char **argv,ExceptionInfo *exception)
{
  char
    name[MaxTextExtent],
    path[MaxTextExtent];

  ImageFilterHandler
    *image_filter;

  MagickBooleanType
    status;

  ModuleHandle
    handle;

  PolicyRights
    rights;

  /*
    Find the module.
  */
  assert(images != (Image **) NULL);
  assert((*images)->signature == MagickSignature);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      (*images)->filename);
#if !defined(MAGICKCORE_BUILD_MODULES)
  {
    MagickBooleanType
      status;

    status=InvokeStaticImageFilter(tag,images,argc,argv,exception);
    if (status != MagickFalse)
      return(status);
  }
#endif
  rights=ReadPolicyRights;
  if (IsRightsAuthorized(FilterPolicyDomain,rights,tag) == MagickFalse)
    {
      errno=EPERM;
      (void) ThrowMagickException(exception,GetMagickModule(),PolicyError,
        "NotAuthorized","`%s'",tag);
      return(MagickFalse);
    }
  TagToFilterModuleName(tag,name);
  status=GetMagickModulePath(name,MagickImageFilterModule,path,exception);
  if (status == MagickFalse)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ModuleError,
        "UnableToLoadModule","`%s': %s",name,path);
      return(MagickFalse);
    }
  /*
    Open the module.
  */
  handle=(ModuleHandle) lt_dlopen(path);
  if (handle == (ModuleHandle) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ModuleError,
        "UnableToLoadModule","`%s': %s",name,lt_dlerror());
      return(MagickFalse);
    }
  /*
    Locate the module.
  */
#if !defined(MAGICKCORE_NAMESPACE_PREFIX)
  (void) FormatLocaleString(name,MaxTextExtent,"%sImage",tag);
#else
  (void) FormatLocaleString(name,MaxTextExtent,"%s%sImage",
    MAGICKCORE_NAMESPACE_PREFIX,tag);
#endif
  /*
    Execute the module.
  */
  ClearMagickException(exception);
  image_filter=(ImageFilterHandler *) lt_dlsym(handle,name);
  if (image_filter == (ImageFilterHandler *) NULL)
    (void) ThrowMagickException(exception,GetMagickModule(),ModuleError,
      "UnableToLoadModule","`%s': %s",name,lt_dlerror());
  else
    {
      size_t
        signature;

      if ((*images)->debug != MagickFalse)
        (void) LogMagickEvent(ModuleEvent,GetMagickModule(),
          "Invoking \"%s\" dynamic image filter",tag);
      signature=image_filter(images,argc,argv,exception);
      if ((*images)->debug != MagickFalse)
        (void) LogMagickEvent(ModuleEvent,GetMagickModule(),"\"%s\" completes",
          tag);
      if (signature != MagickImageFilterSignature)
        (void) ThrowMagickException(exception,GetMagickModule(),ModuleError,
          "ImageFilterSignatureMismatch","`%s': %8lx != %8lx",tag,
          (unsigned long) signature,(unsigned long) MagickImageFilterSignature);
    }
  /*
    Close the module.
  */
  if (lt_dlclose(handle) != 0)
    (void) ThrowMagickException(exception,GetMagickModule(),ModuleWarning,
      "UnableToCloseModule","`%s': %s",name,lt_dlerror());
  return(exception->severity < ErrorException ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t M o d u l e I n f o                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListModuleInfo() lists the module info to a file.
%
%  The format of the ListModuleInfo module is:
%
%      MagickBooleanType ListModuleInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListModuleInfo(FILE *file,
  ExceptionInfo *exception)
{
  char
    filename[MaxTextExtent],
    module_path[MaxTextExtent],
    **modules,
    path[MaxTextExtent];

  register ssize_t
    i;

  size_t
    number_modules;

  if (file == (const FILE *) NULL)
    file=stdout;
  /*
    List image coders.
  */
  modules=GetModuleList("*",MagickImageCoderModule,&number_modules,exception);
  if (modules == (char **) NULL)
    return(MagickFalse);
  TagToCoderModuleName("magick",filename);
  (void) GetMagickModulePath(filename,MagickImageCoderModule,module_path,
    exception);
  GetPathComponent(module_path,HeadPath,path);
  (void) FormatLocaleFile(file,"\nPath: %s\n\n",path);
  (void) FormatLocaleFile(file,"Image Coder\n");
  (void) FormatLocaleFile(file,
    "-------------------------------------------------"
    "------------------------------\n");
  for (i=0; i < (ssize_t) number_modules; i++)
  {
    (void) FormatLocaleFile(file,"%s",modules[i]);
    (void) FormatLocaleFile(file,"\n");
  }
  (void) fflush(file);
  /*
    Relinquish resources.
  */
  for (i=0; i < (ssize_t) number_modules; i++)
    modules[i]=DestroyString(modules[i]);
  modules=(char **) RelinquishMagickMemory(modules);
  /*
    List image filters.
  */
  modules=GetModuleList("*",MagickImageFilterModule,&number_modules,exception);
  if (modules == (char **) NULL)
    return(MagickFalse);
  TagToFilterModuleName("analyze",filename);
  (void) GetMagickModulePath(filename,MagickImageFilterModule,module_path,
    exception);
  GetPathComponent(module_path,HeadPath,path);
  (void) FormatLocaleFile(file,"\nPath: %s\n\n",path);
  (void) FormatLocaleFile(file,"Image Filter\n");
  (void) FormatLocaleFile(file,
    "-------------------------------------------------"
    "------------------------------\n");
  for (i=0; i < (ssize_t) number_modules; i++)
  {
    (void) FormatLocaleFile(file,"%s",modules[i]);
    (void) FormatLocaleFile(file,"\n");
  }
  (void) fflush(file);
  /*
    Relinquish resources.
  */
  for (i=0; i < (ssize_t) number_modules; i++)
    modules[i]=DestroyString(modules[i]);
  modules=(char **) RelinquishMagickMemory(modules);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M o d u l e C o m p o n e n t G e n e s i s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ModuleComponentGenesis() instantiates the module component.
%
%  The format of the ModuleComponentGenesis method is:
%
%      MagickBooleanType ModuleComponentGenesis(void)
%
*/
MagickExport MagickBooleanType ModuleComponentGenesis(void)
{
  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  AcquireSemaphoreInfo(&module_semaphore);
  exception=AcquireExceptionInfo();
  status=InitializeModuleList(exception);
  exception=DestroyExceptionInfo(exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M o d u l e C o m p o n e n t T e r m i n u s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ModuleComponentTerminus() destroys the module component.
%
%  The format of the ModuleComponentTerminus method is:
%
%      ModuleComponentTerminus(void)
%
*/
MagickExport void ModuleComponentTerminus(void)
{
  if (module_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&module_semaphore);
  DestroyModuleList();
  DestroySemaphoreInfo(&module_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   O p e n M o d u l e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpenModule() loads a module, and invokes its registration module.  It
%  returns MagickTrue on success, and MagickFalse if there is an error.
%
%  The format of the OpenModule module is:
%
%      MagickBooleanType OpenModule(const char *module,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o module: a character string that indicates the module to load.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType OpenModule(const char *module,
  ExceptionInfo *exception)
{
  char
    filename[MaxTextExtent],
    module_name[MaxTextExtent],
    name[MaxTextExtent],
    path[MaxTextExtent];

  MagickBooleanType
    status;

  ModuleHandle
    handle;

  ModuleInfo
    *module_info;

  register const CoderInfo
    *p;

  size_t
    signature;

  /*
    Assign module name from alias.
  */
  assert(module != (const char *) NULL);
  module_info=(ModuleInfo *) GetModuleInfo(module,exception);
  if (module_info != (ModuleInfo *) NULL)
    return(MagickTrue);
  (void) CopyMagickString(module_name,module,MaxTextExtent);
  p=GetCoderInfo(module,exception);
  if (p != (CoderInfo *) NULL)
    (void) CopyMagickString(module_name,p->name,MaxTextExtent);
  if (GetValueFromSplayTree(module_list,module_name) != (void *) NULL)
    return(MagickTrue);  /* module already opened, return */
  /*
    Locate module.
  */
  handle=(ModuleHandle) NULL;
  TagToCoderModuleName(module_name,filename);
  (void) LogMagickEvent(ModuleEvent,GetMagickModule(),
    "Searching for module \"%s\" using filename \"%s\"",module_name,filename);
  *path='\0';
  status=GetMagickModulePath(filename,MagickImageCoderModule,path,exception);
  if (status == MagickFalse)
    return(MagickFalse);
  /*
    Load module
  */
  (void) LogMagickEvent(ModuleEvent,GetMagickModule(),
    "Opening module at path \"%s\"",path);
  handle=(ModuleHandle) lt_dlopen(path);
  if (handle == (ModuleHandle) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ModuleError,
        "UnableToLoadModule","`%s': %s",path,lt_dlerror());
      return(MagickFalse);
    }
  /*
    Register module.
  */
  module_info=AcquireModuleInfo(path,module_name);
  module_info->handle=handle;
  if (RegisterModule(module_info,exception) == (ModuleInfo *) NULL)
    return(MagickFalse);
  /*
    Define RegisterFORMATImage method.
  */
  TagToModuleName(module_name,"Register%sImage",name);
  module_info->register_module=(size_t (*)(void)) lt_dlsym(handle,name);
  if (module_info->register_module == (size_t (*)(void)) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ModuleError,
        "UnableToRegisterImageFormat","`%s': %s",module_name,lt_dlerror());
      return(MagickFalse);
    }
  (void) LogMagickEvent(ModuleEvent,GetMagickModule(),
    "Method \"%s\" in module \"%s\" at address %p",name,module_name,
    (void *) module_info->register_module);
  /*
    Define UnregisterFORMATImage method.
  */
  TagToModuleName(module_name,"Unregister%sImage",name);
  module_info->unregister_module=(void (*)(void)) lt_dlsym(handle,name);
  if (module_info->unregister_module == (void (*)(void)) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ModuleError,
        "UnableToRegisterImageFormat","`%s': %s",module_name,lt_dlerror());
      return(MagickFalse);
    }
  (void) LogMagickEvent(ModuleEvent,GetMagickModule(),
    "Method \"%s\" in module \"%s\" at address %p",name,module_name,
    (void *) module_info->unregister_module);
  signature=module_info->register_module();
  if (signature != MagickImageCoderSignature)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ModuleError,
        "ImageCoderSignatureMismatch","`%s': %8lx != %8lx",module_name,
        (unsigned long) signature,(unsigned long) MagickImageCoderSignature);
      return(MagickFalse);
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   O p e n M o d u l e s                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpenModules() loads all available modules.
%
%  The format of the OpenModules module is:
%
%      MagickBooleanType OpenModules(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType OpenModules(ExceptionInfo *exception)
{
  char
    **modules;

  register ssize_t
    i;

  size_t
    number_modules;

  /*
    Load all modules.
  */
  (void) GetMagickInfo((char *) NULL,exception);
  number_modules=0;
  modules=GetModuleList("*",MagickImageCoderModule,&number_modules,exception);
  if (modules == (char **) NULL)
    return(MagickFalse);
  for (i=0; i < (ssize_t) number_modules; i++)
    (void) OpenModule(modules[i],exception);
  /*
    Relinquish resources.
  */
  for (i=0; i < (ssize_t) number_modules; i++)
    modules[i]=DestroyString(modules[i]);
  modules=(char **) RelinquishMagickMemory(modules);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r M o d u l e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterModule() adds an entry to the module list.  It returns a pointer to
%  the registered entry on success.
%
%  The format of the RegisterModule module is:
%
%      ModuleInfo *RegisterModule(const ModuleInfo *module_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o info: a pointer to the registered entry is returned.
%
%    o module_info: a pointer to the ModuleInfo structure to register.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static const ModuleInfo *RegisterModule(const ModuleInfo *module_info,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  assert(module_info != (ModuleInfo *) NULL);
  assert(module_info->signature == MagickSignature);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",module_info->tag);
  if (module_list == (SplayTreeInfo *) NULL)
    return((const ModuleInfo *) NULL);
  status=AddValueToSplayTree(module_list,module_info->tag,module_info);
  if (status == MagickFalse)
    (void) ThrowMagickException(exception,GetMagickModule(),ResourceLimitError,
      "MemoryAllocationFailed","`%s'",module_info->tag);
  return(module_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  T a g T o C o d e r M o d u l e N a m e                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TagToCoderModuleName() munges a module tag and obtains the filename of the
%  corresponding module.
%
%  The format of the TagToCoderModuleName module is:
%
%      char *TagToCoderModuleName(const char *tag,char *name)
%
%  A description of each parameter follows:
%
%    o tag: a character string representing the module tag.
%
%    o name: return the module name here.
%
*/
static void TagToCoderModuleName(const char *tag,char *name)
{
  assert(tag != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",tag);
  assert(name != (char *) NULL);
#if defined(MAGICKCORE_LTDL_DELEGATE)
  (void) FormatLocaleString(name,MaxTextExtent,"%s.la",tag);
  (void) LocaleLower(name);
#else
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  if (LocaleNCompare("IM_MOD_",tag,7) == 0)
    (void) CopyMagickString(name,tag,MaxTextExtent);
  else
    {
#if defined(_DEBUG)
      (void) FormatLocaleString(name,MaxTextExtent,"IM_MOD_DB_%s_.dll",tag);
#else
      (void) FormatLocaleString(name,MaxTextExtent,"IM_MOD_RL_%s_.dll",tag);
#endif
    }
#endif
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  T a g T o F i l t e r M o d u l e N a m e                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TagToFilterModuleName() munges a module tag and returns the filename of the
%  corresponding filter module.
%
%  The format of the TagToFilterModuleName module is:
%
%      void TagToFilterModuleName(const char *tag,char name)
%
%  A description of each parameter follows:
%
%    o tag: a character string representing the module tag.
%
%    o name: return the filter name here.
%
*/
static void TagToFilterModuleName(const char *tag,char *name)
{
  assert(tag != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",tag);
  assert(name != (char *) NULL);
#if !defined(MAGICKCORE_LTDL_DELEGATE)
  (void) FormatLocaleString(name,MaxTextExtent,"%s.dll",tag);
#else
  (void) FormatLocaleString(name,MaxTextExtent,"%s.la",tag);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T a g T o M o d u l e N a m e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TagToModuleName() munges the module tag name and returns an upper-case tag
%  name as the input string, and a user-provided format.
%
%  The format of the TagToModuleName module is:
%
%      TagToModuleName(const char *tag,const char *format,char *module)
%
%  A description of each parameter follows:
%
%    o tag: the module tag.
%
%    o format: a sprintf-compatible format string containing %s where the
%      upper-case tag name is to be inserted.
%
%    o module: pointer to a destination buffer for the formatted result.
%
*/
static void TagToModuleName(const char *tag,const char *format,char *module)
{
  char
    name[MaxTextExtent];

  assert(tag != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",tag);
  assert(format != (const char *) NULL);
  assert(module != (char *) NULL);
  (void) CopyMagickString(name,tag,MaxTextExtent);
  LocaleUpper(name);
#if !defined(MAGICKCORE_NAMESPACE_PREFIX)
  (void) FormatLocaleString(module,MaxTextExtent,format,name);
#else
  {
    char
      prefix_format[MaxTextExtent];

    (void) FormatLocaleString(prefix_format,MaxTextExtent,"%s%s",
      MAGICKCORE_NAMESPACE_PREFIX,format);
    (void) FormatLocaleString(module,MaxTextExtent,prefix_format,name);
  }
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r M o d u l e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterModule() unloads a module, and invokes its de-registration module.
%  Returns MagickTrue on success, and MagickFalse if there is an error.
%
%  The format of the UnregisterModule module is:
%
%      MagickBooleanType UnregisterModule(const ModuleInfo *module_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o module_info: the module info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType UnregisterModule(const ModuleInfo *module_info,
  ExceptionInfo *exception)
{
  /*
    Locate and execute UnregisterFORMATImage module.
  */
  assert(module_info != (const ModuleInfo *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",module_info->tag);
  assert(exception != (ExceptionInfo *) NULL);
  if (module_info->unregister_module == NULL)
    return(MagickTrue);
  module_info->unregister_module();
  if (lt_dlclose((ModuleHandle) module_info->handle) != 0)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ModuleWarning,
        "UnableToCloseModule","`%s': %s",module_info->tag,lt_dlerror());
      return(MagickFalse);
    }
  return(MagickTrue);
}
#else
MagickExport MagickBooleanType ListModuleInfo(FILE *magick_unused(file),
  ExceptionInfo *magick_unused(exception))
{
  return(MagickTrue);
}

MagickExport MagickBooleanType InvokeDynamicImageFilter(const char *tag,
  Image **image,const int argc,const char **argv,ExceptionInfo *exception)
{
  PolicyRights
    rights;

  assert(image != (Image **) NULL);
  assert((*image)->signature == MagickSignature);
  if ((*image)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*image)->filename);
  rights=ReadPolicyRights;
  if (IsRightsAuthorized(FilterPolicyDomain,rights,tag) == MagickFalse)
    {
      errno=EPERM;
      (void) ThrowMagickException(exception,GetMagickModule(),PolicyError,
        "NotAuthorized","`%s'",tag);
      return(MagickFalse);
    }
#if defined(MAGICKCORE_BUILD_MODULES)
  (void) tag;
  (void) argc;
  (void) argv;
  (void) exception;
#else
  {
    extern size_t
      analyzeImage(Image **,const int,const char **,ExceptionInfo *);

    ImageFilterHandler
      *image_filter;

    image_filter=(ImageFilterHandler *) NULL;
    if (LocaleCompare("analyze",tag) == 0)
      image_filter=(ImageFilterHandler *) analyzeImage;
    if (image_filter == (ImageFilterHandler *) NULL)
      (void) ThrowMagickException(exception,GetMagickModule(),ModuleError,
        "UnableToLoadModule","`%s'",tag);
    else
      {
        size_t
          signature;

        if ((*image)->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Invoking \"%s\" static image filter",tag);
        signature=image_filter(image,argc,argv,exception);
        if ((*image)->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),"\"%s\" completes",
            tag);
        if (signature != MagickImageFilterSignature)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),ModuleError,
              "ImageFilterSignatureMismatch","`%s': %8lx != %8lx",tag,
              (unsigned long) signature,(unsigned long)
              MagickImageFilterSignature);
            return(MagickFalse);
          }
      }
  }
#endif
  return(MagickTrue);
}
#endif

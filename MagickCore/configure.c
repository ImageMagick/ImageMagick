/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%          CCCC   OOO   N   N  FFFFF  IIIII   GGGG  U   U  RRRR   EEEEE       %
%         C      O   O  NN  N  F        I    G      U   U  R   R  E           %
%         C      O   O  N N N  FFF      I    G GG   U   U  RRRR   EEE         %
%         C      O   O  N  NN  F        I    G   G  U   U  R R    E           %
%          CCCC   OOO   N   N  F      IIIII   GGG    UUU   R  R   EEEEE       %
%                                                                             %
%                                                                             %
%                      MagickCore Image Configure Methods                     %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 2003                                   %
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
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/version.h"
#include "MagickCore/xml-tree.h"
#include "MagickCore/xml-tree-private.h"

/*
  Define declarations.
*/
#define ConfigureFilename  "configure.xml"

/*
  Typedef declarations.
*/
typedef struct _ConfigureMapInfo
{
  const char
    *name,
    *value;
} ConfigureMapInfo;

/*
  Static declarations.
*/

static LinkedListInfo
  *configure_cache = (LinkedListInfo *) NULL;

static SemaphoreInfo
  *configure_semaphore = (SemaphoreInfo *) NULL;

/*
  Forward declarations.
*/
static MagickBooleanType
  IsConfigureCacheInstantiated(ExceptionInfo *);

#if !MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
static MagickBooleanType
  LoadConfigureCache(LinkedListInfo *,const char *,const char *,const size_t,
    ExceptionInfo *);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A c q u i r e C o n f i g u r e C a c h e                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireConfigureCache() caches one or more configure configurations which
%  provides a mapping between configure attributes and a configure name.
%
%  The format of the AcquireConfigureCache method is:
%
%      LinkedListInfo *AcquireConfigureCache(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static inline void AddConfigureKey(LinkedListInfo *cache,const char *path,
  const char *name,const char *value,MagickBooleanType exempt)
{
  ConfigureInfo
    *configure_info;

  configure_info=(ConfigureInfo *) AcquireMagickMemory(sizeof(*configure_info));
  if (configure_info == (ConfigureInfo *) NULL)
    return;
  (void) memset(configure_info,0,sizeof(*configure_info));
  if (exempt == MagickTrue)
    {
      configure_info->path=(char *) path;
      configure_info->name=(char *) name;
      configure_info->value=(char *) value;
    }
  else
    {
      configure_info->path=ConstantString(path);
      configure_info->name=ConstantString(name);
      configure_info->value=ConstantString(value);
    }
  configure_info->exempt=exempt;
  configure_info->signature=MagickCoreSignature;
  (void) AppendValueToLinkedList(cache,configure_info);
}

static LinkedListInfo *AcquireConfigureCache(const char *filename,
  ExceptionInfo *exception)
{
  char
    head_path[MagickPathExtent],
    path[MagickPathExtent];

  LinkedListInfo
    *cache;

  /*
    Load external configure map.
  */
  cache=NewLinkedList(0);
#if !MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
  {
    const StringInfo
      *option;

    LinkedListInfo
      *options;

    MagickBooleanType
      status;

    options=GetConfigureOptions(filename,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
    while (option != (const StringInfo *) NULL)
    {
      status=LoadConfigureCache(cache,(const char *)
        GetStringInfoDatum(option),GetStringInfoPath(option),0,exception);
      if (status == MagickTrue)
        break;
      option=(const StringInfo *) GetNextValueInLinkedList(options);
    }
    options=DestroyConfigureOptions(options);
  }
#else
  magick_unreferenced(filename);
  magick_unreferenced(exception);
#endif
  /*
    Load built-in configure.
  */
  AddConfigureKey(cache,"[built-in]","NAME","ImageMagick",MagickTrue);
  /*
    Load runtime configuration.
  */
  AddConfigureKey(cache,"[built-in]","QuantumDepth",GetMagickQuantumDepth(
    (size_t *)NULL),MagickTrue);
  AddConfigureKey(cache,"[built-in]","FEATURES",GetMagickFeatures(),
    MagickTrue);
  AddConfigureKey(cache,"[built-in]","DELEGATES",GetMagickDelegates(),
    MagickTrue);
  (void) AcquireUniqueFilename(path);
  GetPathComponent(path,HeadPath,head_path);
  AddConfigureKey(cache,"[built-in]","MAGICK_TEMPORARY_PATH",head_path,
    MagickFalse);
  return(cache);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o n f i g u r e C o m p o n e n t G e n e s i s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConfigureComponentGenesis() instantiates the configure component.
%
%  The format of the ConfigureComponentGenesis method is:
%
%      MagickBooleanType ConfigureComponentGenesis(void)
%
*/
MagickPrivate MagickBooleanType ConfigureComponentGenesis(void)
{
  if (configure_semaphore == (SemaphoreInfo *) NULL)
    configure_semaphore=AcquireSemaphoreInfo();
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o n f i g u r e C o m p o n e n t T e r m i n u s                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConfigureComponentTerminus() destroys the configure component.
%
%  The format of the ConfigureComponentTerminus method is:
%
%      ConfigureComponentTerminus(void)
%
*/

static void *DestroyConfigureElement(void *configure_info)
{
  ConfigureInfo
    *p;

  p=(ConfigureInfo *) configure_info;
  if (p->exempt == MagickFalse)
    {
      if (p->value != (char *) NULL)
        p->value=DestroyString(p->value);
      if (p->name != (char *) NULL)
        p->name=DestroyString(p->name);
      if (p->path != (char *) NULL)
        p->path=DestroyString(p->path);
    }
  p=(ConfigureInfo *) RelinquishMagickMemory(p);
  return((void *) NULL);
}

MagickPrivate void ConfigureComponentTerminus(void)
{
  if (configure_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&configure_semaphore);
  LockSemaphoreInfo(configure_semaphore);
  if (configure_cache != (LinkedListInfo *) NULL)
    configure_cache=DestroyLinkedList(configure_cache,DestroyConfigureElement);
  configure_cache=(LinkedListInfo *) NULL;
  UnlockSemaphoreInfo(configure_semaphore);
  RelinquishSemaphoreInfo(&configure_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y C o n f i g u r e O p t i o n s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyConfigureOptions() releases memory associated with an configure
%  options.
%
%  The format of the DestroyProfiles method is:
%
%      LinkedListInfo *DestroyConfigureOptions(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/

static void *DestroyOptions(void *option)
{
  return(DestroyStringInfo((StringInfo *) option));
}

MagickExport LinkedListInfo *DestroyConfigureOptions(LinkedListInfo *options)
{
  assert(options != (LinkedListInfo *) NULL);
  return(DestroyLinkedList(options,DestroyOptions));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t C o n f i g u r e I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigureInfo() searches the configure list for the specified name and if
%  found returns attributes for that element.
%
%  The format of the GetConfigureInfo method is:
%
%      const ConfigureInfo *GetConfigureInfo(const char *name,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o configure_info: GetConfigureInfo() searches the configure list for the
%      specified name and if found returns attributes for that element.
%
%    o name: the configure name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const ConfigureInfo *GetConfigureInfo(const char *name,
  ExceptionInfo *exception)
{
  const ConfigureInfo
    *option;

  ElementInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  if (IsConfigureCacheInstantiated(exception) == MagickFalse)
    return((const ConfigureInfo *) NULL);
  /*
    Search for configure tag.
  */
  option=(const ConfigureInfo *) NULL;
  LockSemaphoreInfo(configure_semaphore);
  p=GetHeadElementInLinkedList(configure_cache);
  if ((name == (const char *) NULL) || (LocaleCompare(name,"*") == 0))
    {
      UnlockSemaphoreInfo(configure_semaphore);
      if (p != (ElementInfo *) NULL)
        option=(const ConfigureInfo *) p->value;
      return(option);
    }
  while (p != (ElementInfo *) NULL)
  {
    option=(const ConfigureInfo *) p->value;
    if (LocaleCompare(name,option->name) == 0)
      break;
    p=p->next;
  }
  if (p == (ElementInfo *) NULL)
    option=(const ConfigureInfo *) NULL;
  else
    SetHeadElementInLinkedList(configure_cache,p);
  UnlockSemaphoreInfo(configure_semaphore);
  return(option);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o n f i g u r e I n f o L i s t                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigureInfoList() returns any configure options that match the
%  specified pattern.
%
%  The format of the GetConfigureInfoList function is:
%
%      const ConfigureInfo **GetConfigureInfoList(const char *pattern,
%        size_t *number_options,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_options:  This integer returns the number of configure options in
%    the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int ConfigureInfoCompare(const void *x,const void *y)
{
  const ConfigureInfo
    **p,
    **q;

  p=(const ConfigureInfo **) x,
  q=(const ConfigureInfo **) y;
  if (LocaleCompare((*p)->path,(*q)->path) == 0)
    return(LocaleCompare((*p)->name,(*q)->name));
  return(LocaleCompare((*p)->path,(*q)->path));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport const ConfigureInfo **GetConfigureInfoList(const char *pattern,
  size_t *number_options,ExceptionInfo *exception)
{
  const ConfigureInfo
    **options;

  ElementInfo
    *p;

  ssize_t
    i;

  assert(pattern != (char *) NULL);
  assert(number_options != (size_t *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  *number_options=0;
  if (IsConfigureCacheInstantiated(exception) == MagickFalse)
    return((const ConfigureInfo **) NULL);
  options=(const ConfigureInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(configure_cache)+1UL,sizeof(*options));
  if (options == (const ConfigureInfo **) NULL)
    return((const ConfigureInfo **) NULL);
  LockSemaphoreInfo(configure_semaphore);
  p=GetHeadElementInLinkedList(configure_cache);
  for (i=0; p != (ElementInfo *) NULL; )
  {
    const ConfigureInfo
      *option;

    option=(const ConfigureInfo *) p->value;
    if ((option->stealth == MagickFalse) &&
        (GlobExpression(option->name,pattern,MagickFalse) != MagickFalse))
      options[i++]=option;
    p=p->next;
  }
  UnlockSemaphoreInfo(configure_semaphore);
  if (i == 0)
    options=(const ConfigureInfo **) RelinquishMagickMemory((void*) options);
  else
    {
      qsort((void *) options,(size_t) i,sizeof(*options),ConfigureInfoCompare);
      options[i]=(ConfigureInfo *) NULL;
    }
  *number_options=(size_t) i;
  return(options);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o n f i g u r e L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigureList() returns any configure options that match the specified
%  pattern.
%
%  The format of the GetConfigureList function is:
%
%      char **GetConfigureList(const char *pattern,
%        size_t *number_options,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_options:  This integer returns the number of options in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int ConfigureCompare(const void *x,const void *y)
{
  char
    **p,
    **q;

  p=(char **) x;
  q=(char **) y;
  return(LocaleCompare(*p,*q));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport char **GetConfigureList(const char *pattern,
  size_t *number_options,ExceptionInfo *exception)
{
  char
    **options;

  ElementInfo
    *p;

  ssize_t
    i;

  assert(pattern != (char *) NULL);
  assert(number_options != (size_t *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  *number_options=0;
  if (IsConfigureCacheInstantiated(exception) == MagickFalse)
    return((char **) NULL);
  options=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(configure_cache)+1UL,sizeof(*options));
  if (options == (char **) NULL)
    return((char **) NULL);
  LockSemaphoreInfo(configure_semaphore);
  p=GetHeadElementInLinkedList(configure_cache);
  for (i=0; p != (ElementInfo *) NULL; )
  {
    const ConfigureInfo
      *option;

    option=(const ConfigureInfo *) p->value;
    if ((option->stealth == MagickFalse) &&
        (GlobExpression(option->name,pattern,MagickFalse) != MagickFalse))
      options[i++]=ConstantString(option->name);
    p=p->next;
  }
  UnlockSemaphoreInfo(configure_semaphore);
  if (i == 0)
    options=(char **) RelinquishMagickMemory(options);
  else
    {
      qsort((void *) options,(size_t) i,sizeof(*options),ConfigureCompare);
      options[i]=(char *) NULL;
    }
  *number_options=(size_t) i;
  return(options);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o n f i g u r e O p t i o n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigureOption() returns the value associated with the configure option.
%
%  The format of the GetConfigureOption method is:
%
%      char *GetConfigureOption(const char *option)
%
%  A description of each parameter follows:
%
%    o configure_info:  The configure info.
%
*/
MagickExport char *GetConfigureOption(const char *option)
{
  const char
    *value;

  const ConfigureInfo
    *configure_info;

  ExceptionInfo
    *exception;

  assert(option != (const char *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",option);
  exception=AcquireExceptionInfo();
  configure_info=GetConfigureInfo(option,exception);
  exception=DestroyExceptionInfo(exception);
  if (configure_info == (ConfigureInfo *) NULL)
    return((char *) NULL);
  value=GetConfigureValue(configure_info);
  if ((value == (const char *) NULL) || (*value == '\0'))
    return((char *) NULL);
  return(ConstantString(value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t C o n f i g u r e O p t i o n s                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigureOptions() returns any Magick configuration options associated
%  with the specified filename.
%
%  The format of the GetConfigureOptions method is:
%
%      LinkedListInfo *GetConfigureOptions(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the configure file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport LinkedListInfo *GetConfigureOptions(const char *filename,
  ExceptionInfo *exception)
{
  char
    path[MagickPathExtent];

  const char
    *element;

  LinkedListInfo
    *options,
    *paths;

  StringInfo
    *xml;

  assert(filename != (const char *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  (void) CopyMagickString(path,filename,MagickPathExtent);
  /*
    Load XML from configuration files to linked-list.
  */
  options=NewLinkedList(0);
  paths=GetConfigurePaths(filename,exception);
  if (paths != (LinkedListInfo *) NULL)
    {
      ResetLinkedListIterator(paths);
      element=(const char *) GetNextValueInLinkedList(paths);
      while (element != (const char *) NULL)
      {
        (void) FormatLocaleString(path,MagickPathExtent,"%s%s",element,
          filename);
        (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
          "Searching for configure file: \"%s\"",path);
        xml=ConfigureFileToStringInfo(path);
        if (xml != (StringInfo *) NULL)
          (void) AppendValueToLinkedList(options,xml);
        element=(const char *) GetNextValueInLinkedList(paths);
      }
      paths=DestroyLinkedList(paths,RelinquishMagickMemory);
    }
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  if (GetNumberOfElementsInLinkedList(options) == 0)
    {
      char
        *blob;

      blob=(char *) NTResourceToBlob(filename);
      if (blob != (char *) NULL)
        {
          xml=AcquireStringInfo(0);
          SetStringInfoLength(xml,strlen(blob)+1);
          SetStringInfoDatum(xml,(unsigned char *) blob);
          SetStringInfoPath(xml,filename);
          (void) AppendValueToLinkedList(options,xml);
        }
    }
#endif
  if (GetNumberOfElementsInLinkedList(options) == 0)
    (void) ThrowMagickException(exception,GetMagickModule(),ConfigureWarning,
      "UnableToOpenConfigureFile","`%s'",filename);
  ResetLinkedListIterator(options);
  return(options);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t C o n f i g u r e P a t h s                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigurePaths() returns any Magick configuration paths associated
%  with the specified filename.
%
%  The format of the GetConfigurePaths method is:
%
%      LinkedListInfo *GetConfigurePaths(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the configure file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport LinkedListInfo *GetConfigurePaths(const char *filename,
  ExceptionInfo *exception)
{
  char
    path[MagickPathExtent];

  LinkedListInfo
    *paths;

  assert(filename != (const char *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  (void) CopyMagickString(path,filename,MagickPathExtent);
  paths=NewLinkedList(0);
  {
    char
      *configure_path;

    /*
      Search $MAGICK_CONFIGURE_PATH.
    */
    configure_path=GetEnvironmentValue("MAGICK_CONFIGURE_PATH");
    if (configure_path != (char *) NULL)
      {
        char
          *p,
          *q;

        for (p=configure_path-1; p != (char *) NULL; )
        {
          (void) CopyMagickString(path,p+1,MagickPathExtent);
          q=strchr(path,DirectoryListSeparator);
          if (q != (char *) NULL)
            *q='\0';
          q=path+strlen(path)-1;
          if ((q >= path) && (*q != *DirectorySeparator))
            (void) ConcatenateMagickString(path,DirectorySeparator,
              MagickPathExtent);
          (void) AppendValueToLinkedList(paths,ConstantString(path));
          p=strchr(p+1,DirectoryListSeparator);
        }
        configure_path=DestroyString(configure_path);
      }
  }
#if defined(MAGICKCORE_INSTALLED_SUPPORT)
#if defined(MAGICKCORE_SHARE_PATH)
  (void) AppendValueToLinkedList(paths,ConstantString(MAGICKCORE_SHARE_PATH));
#endif
#if defined(MAGICKCORE_SHAREARCH_PATH)
  (void) AppendValueToLinkedList(paths,ConstantString(
    MAGICKCORE_SHAREARCH_PATH));
#endif
#if defined(MAGICKCORE_CONFIGURE_PATH)
  (void) AppendValueToLinkedList(paths,ConstantString(
    MAGICKCORE_CONFIGURE_PATH));
#endif
#if defined(MAGICKCORE_DOCUMENTATION_PATH)
  (void) AppendValueToLinkedList(paths,ConstantString(
    MAGICKCORE_DOCUMENTATION_PATH));
#endif
#if defined(MAGICKCORE_WINDOWS_SUPPORT) && !(defined(MAGICKCORE_CONFIGURE_PATH) || defined(MAGICKCORE_SHARE_PATH))
  {
    unsigned char
      *key_value;

    /*
      Locate file via registry key.
    */
    key_value=NTRegistryKeyLookup("ConfigurePath");
    if (key_value != (unsigned char *) NULL)
      {
        (void) FormatLocaleString(path,MagickPathExtent,"%s%s",(char *)
          key_value,DirectorySeparator);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
        key_value=(unsigned char *) RelinquishMagickMemory(key_value);
      }
  }
#endif
#else
  {
    char
      *home;

    /*
      Search under MAGICK_HOME.
    */
    home=GetEnvironmentValue("MAGICK_HOME");
    if (home != (char *) NULL)
      {
#if !defined(MAGICKCORE_POSIX_SUPPORT) || defined( __VMS )
        (void) FormatLocaleString(path,MagickPathExtent,"%s%s",home,
          DirectorySeparator);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
#else
        (void) FormatLocaleString(path,MagickPathExtent,"%s/etc/%s/",home,
          MAGICKCORE_CONFIGURE_RELATIVE_PATH);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
        (void) FormatLocaleString(path,MagickPathExtent,"%s/share/%s/",home,
          MAGICKCORE_SHARE_RELATIVE_PATH);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
        (void) FormatLocaleString(path,MagickPathExtent,"%s",
          MAGICKCORE_SHAREARCH_PATH);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
#endif
        home=DestroyString(home);
      }
    }
  if (*GetClientPath() != '\0')
    {
#if !defined(MAGICKCORE_POSIX_SUPPORT) || defined( __VMS )
      (void) FormatLocaleString(path,MagickPathExtent,"%s%s",GetClientPath(),
        DirectorySeparator);
      (void) AppendValueToLinkedList(paths,ConstantString(path));
#else
      char
        prefix[MagickPathExtent];

      /*
        Search based on executable directory if directory is known.
      */
      (void) CopyMagickString(prefix,GetClientPath(),MagickPathExtent);
      ChopPathComponents(prefix,1);
      (void) FormatLocaleString(path,MagickPathExtent,"%s/etc/%s/",prefix,
        MAGICKCORE_CONFIGURE_RELATIVE_PATH);
      (void) AppendValueToLinkedList(paths,ConstantString(path));
      (void) FormatLocaleString(path,MagickPathExtent,"%s/share/%s/",prefix,
        MAGICKCORE_SHARE_RELATIVE_PATH);
      (void) AppendValueToLinkedList(paths,ConstantString(path));
      (void) FormatLocaleString(path,MagickPathExtent,"%s",
        MAGICKCORE_SHAREARCH_PATH);
      (void) AppendValueToLinkedList(paths,ConstantString(path));
#endif
    }
  /*
    Search current directory.
  */
  (void) AppendValueToLinkedList(paths,ConstantString(""));
#endif
  {
    char
      *home;

    home=GetEnvironmentValue("XDG_CONFIG_HOME");
#if defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__MINGW32__)
    if (home == (char *) NULL)
      home=GetEnvironmentValue("LOCALAPPDATA");
    if (home == (char *) NULL)
      home=GetEnvironmentValue("APPDATA");
    if (home == (char *) NULL)
      home=GetEnvironmentValue("USERPROFILE");
#endif
    if (home != (char *) NULL)
      {
        /*
          Search $XDG_CONFIG_HOME/ImageMagick.
        */
        (void) FormatLocaleString(path,MagickPathExtent,"%s%sImageMagick%s",
          home,DirectorySeparator,DirectorySeparator);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
        home=DestroyString(home);
      }
    home=GetEnvironmentValue("HOME");
    if (home != (char *) NULL)
      {
        /*
          Search $HOME/.config/ImageMagick.
        */
        (void) FormatLocaleString(path,MagickPathExtent,
          "%s%s.config%sImageMagick%s",home,DirectorySeparator,
          DirectorySeparator,DirectorySeparator);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
        home=DestroyString(home);
      }
  }
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  {
    char
      module_path[MagickPathExtent];

#if defined(_MAGICKDLL_)
    if ((NTGetModulePath("CORE_RL_MagickCore_.dll",module_path) != MagickFalse) ||
        (NTGetModulePath("CORE_DB_MagickCore_.dll",module_path) != MagickFalse))
      {
        unsigned char
          *key_value;

        /*
          Search module path.
        */
        (void) FormatLocaleString(path,MagickPathExtent,"%s%s",module_path,
          DirectorySeparator);
        key_value=NTRegistryKeyLookup("ConfigurePath");
        if (key_value == (unsigned char *) NULL)
          (void) AppendValueToLinkedList(paths,ConstantString(path));
        else
          key_value=(unsigned char *) RelinquishMagickMemory(key_value);
      }
#endif
    if (NTGetModulePath("Magick.dll",module_path) != MagickFalse)
      {
        /*
          Search PerlMagick module path.
        */
        (void) FormatLocaleString(path,MagickPathExtent,"%s%s",module_path,
          DirectorySeparator);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
        (void) FormatLocaleString(path,MagickPathExtent,"%s%s",module_path,
          "\\inc\\lib\\auto\\Image\\Magick\\");
        (void) AppendValueToLinkedList(paths,ConstantString(path));
      }
  }
#endif
  if (GetNumberOfElementsInLinkedList(paths) == 0)
    (void) ThrowMagickException(exception,GetMagickModule(),ConfigureWarning,
      "no configuration paths found","`%s'",filename);
  return(paths);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o n f i g u r e V a l u e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigureValue() returns the value associated with the configure info.
%
%  The format of the GetConfigureValue method is:
%
%      const char *GetConfigureValue(const ConfigureInfo *configure_info)
%
%  A description of each parameter follows:
%
%    o configure_info:  The configure info.
%
*/
MagickExport const char *GetConfigureValue(const ConfigureInfo *configure_info)
{
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(configure_info != (ConfigureInfo *) NULL);
  assert(configure_info->signature == MagickCoreSignature);
  return(configure_info->value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s C o n f i g u r e C a c h e I n s t a n t i a t e d                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsConfigureCacheInstantiated() determines if the configure list is
%  instantiated.  If not, it instantiates the list and returns it.
%
%  The format of the IsConfigureInstantiated method is:
%
%      MagickBooleanType IsConfigureCacheInstantiated(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType IsConfigureCacheInstantiated(ExceptionInfo *exception)
{
  if (configure_cache == (LinkedListInfo *) NULL)
    {
      if (configure_semaphore == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&configure_semaphore);
      LockSemaphoreInfo(configure_semaphore);
      if (configure_cache == (LinkedListInfo *) NULL)
        configure_cache=AcquireConfigureCache(ConfigureFilename,exception);
      UnlockSemaphoreInfo(configure_semaphore);
    }
  return(configure_cache != (LinkedListInfo *) NULL ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t C o n f i g u r e I n f o                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListConfigureInfo() lists the configure info to a file.
%
%  The format of the ListConfigureInfo method is:
%
%      MagickBooleanType ListConfigureInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListConfigureInfo(FILE *file,
  ExceptionInfo *exception)
{
  const char
    *name,
    *path,
    *value;

  const ConfigureInfo
    **configure_info;

  ssize_t
    i;

  size_t
    number_options;

  ssize_t
    j;

  if (file == (const FILE *) NULL)
    file=stdout;
  configure_info=GetConfigureInfoList("*",&number_options,exception);
  if (configure_info == (const ConfigureInfo **) NULL)
    return(MagickFalse);
  path=(const char *) NULL;
  for (i=0; i < (ssize_t) number_options; i++)
  {
    if (configure_info[i]->stealth != MagickFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (LocaleCompare(path,configure_info[i]->path) != 0))
      {
        if (configure_info[i]->path != (char *) NULL)
          (void) FormatLocaleFile(file,"\nPath: %s\n\n",
            configure_info[i]->path);
        (void) FormatLocaleFile(file,"Name                  Value\n");
        (void) FormatLocaleFile(file,
          "-------------------------------------------------"
          "------------------------------\n");
      }
    path=configure_info[i]->path;
    name="unknown";
    if (configure_info[i]->name != (char *) NULL)
      name=configure_info[i]->name;
    (void) FormatLocaleFile(file,"%s",name);
    for (j=(ssize_t) strlen(name); j <= 20; j++)
      (void) FormatLocaleFile(file," ");
    (void) FormatLocaleFile(file," ");
    value="unknown";
    if (configure_info[i]->value != (char *) NULL)
      value=configure_info[i]->value;
    (void) FormatLocaleFile(file,"%s",value);
    (void) FormatLocaleFile(file,"\n");
  }
  (void) fflush(file);
  configure_info=(const ConfigureInfo **) RelinquishMagickMemory((void *)
    configure_info);
  return(MagickTrue);
}

#if !MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d C o n f i g u r e C a c h e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadConfigureCache() loads the configure configurations which provides a
%  mapping between configure attributes and a configure name.
%
%  The format of the LoadConfigureCache method is:
%
%      MagickBooleanType LoadConfigureCache(LinkedListInfo *cache,
%        const char *xml,const char *filename,const size_t depth,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The configure list in XML format.
%
%    o filename:  The configure list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadConfigureCache(LinkedListInfo *cache,
  const char *xml,const char *filename,const size_t depth,
  ExceptionInfo *exception)
{
  char
    keyword[MagickPathExtent],
    *token;

  ConfigureInfo
    *configure_info;

  const char
    *q;

  MagickStatusType
    status;

  size_t
    extent;

  /*
    Load the configure map file.
  */
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading configure file \"%s\" ...",filename);
  status=MagickTrue;
  configure_info=(ConfigureInfo *) NULL;
  token=AcquireString(xml);
  extent=strlen(token)+MagickPathExtent;
  for (q=(char *) xml; *q != '\0'; )
  {
    /*
      Interpret XML.
    */
    (void) GetNextToken(q,&q,extent,token);
    if (*token == '\0')
      break;
    (void) CopyMagickString(keyword,token,MagickPathExtent);
    if (LocaleNCompare(keyword,"<!DOCTYPE",9) == 0)
      {
        /*
          Doctype element.
        */
        while ((LocaleNCompare(q,"]>",2) != 0) && (*q != '\0'))
          (void) GetNextToken(q,&q,extent,token);
        continue;
      }
    if (LocaleNCompare(keyword,"<!--",4) == 0)
      {
        /*
          Comment element.
        */
        while ((LocaleNCompare(q,"->",2) != 0) && (*q != '\0'))
          (void) GetNextToken(q,&q,extent,token);
        continue;
      }
    if (LocaleCompare(keyword,"<include") == 0)
      {
        /*
          Include element.
        */
        while (((*token != '/') && (*(token+1) != '>')) && (*q != '\0'))
        {
          (void) CopyMagickString(keyword,token,MagickPathExtent);
          (void) GetNextToken(q,&q,extent,token);
          if (*token != '=')
            continue;
          (void) GetNextToken(q,&q,extent,token);
          if (LocaleCompare(keyword,"file") == 0)
            {
              if (depth > MagickMaxRecursionDepth)
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ConfigureError,"IncludeElementNestedTooDeeply","`%s'",token);
              else
                {
                  char
                    path[MagickPathExtent],
                    *file_xml;

                  GetPathComponent(filename,HeadPath,path);
                  if (*path != '\0')
                    (void) ConcatenateMagickString(path,DirectorySeparator,
                      MagickPathExtent);
                  if (*token == *DirectorySeparator)
                    (void) CopyMagickString(path,token,MagickPathExtent);
                  else
                    (void) ConcatenateMagickString(path,token,MagickPathExtent);
                  file_xml=FileToXML(path,~0UL);
                  if (file_xml != (char *) NULL)
                    {
                      status&=(MagickStatusType) LoadConfigureCache(cache,
                        file_xml,path,depth+1,exception);
                      file_xml=DestroyString(file_xml);
                    }
                }
            }
        }
        continue;
      }
    if (LocaleCompare(keyword,"<configure") == 0)
      {
        /*
          Configure element.
        */
        configure_info=(ConfigureInfo *) AcquireCriticalMemory(
          sizeof(*configure_info));
        (void) memset(configure_info,0,sizeof(*configure_info));
        configure_info->path=ConstantString(filename);
        configure_info->exempt=MagickFalse;
        configure_info->signature=MagickCoreSignature;
        continue;
      }
    if (configure_info == (ConfigureInfo *) NULL)
      continue;
    if ((LocaleCompare(keyword,"/>") == 0) ||
        (LocaleCompare(keyword,"</policy>") == 0))
      {
        status=AppendValueToLinkedList(cache,configure_info);
        if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            configure_info->name);
        configure_info=(ConfigureInfo *) NULL;
        continue;
      }
    /*
      Parse configure element.
    */
    (void) GetNextToken(q,(const char **) NULL,extent,token);
    if (*token != '=')
      continue;
    (void) GetNextToken(q,&q,extent,token);
    (void) GetNextToken(q,&q,extent,token);
    switch (*keyword)
    {
      case 'N':
      case 'n':
      {
        if (LocaleCompare((char *) keyword,"name") == 0)
          {
            configure_info->name=ConstantString(token);
            break;
          }
        break;
      }
      case 'S':
      case 's':
      {
        if (LocaleCompare((char *) keyword,"stealth") == 0)
          {
            configure_info->stealth=IsStringTrue(token);
            break;
          }
        break;
      }
      case 'V':
      case 'v':
      {
        if (LocaleCompare((char *) keyword,"value") == 0)
          {
            configure_info->value=ConstantString(token);
            break;
          }
        break;
      }
      default:
        break;
    }
  }
  token=(char *) RelinquishMagickMemory(token);
  return(status != 0 ? MagickTrue : MagickFalse);
}
#endif

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
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/blob.h"
#include "magick/client.h"
#include "magick/configure.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/hashmap.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/semaphore.h"
#include "magick/string_.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/xml-tree.h"

/*
  Define declarations.
*/
#define ConfigureFilename  "configure.xml"

#ifdef _OPENMP
#define MAGICKCORE_FEATURE_OPENMP_STR "OpenMP "
#else
#define MAGICKCORE_FEATURE_OPENMP_STR ""
#endif
#ifdef _OPENCL
#define MAGICKCORE_FEATURE_OPENCL_STR "OpenCL "
#else
#define MAGICKCORE_FEATURE_OPENCL_STR ""
#endif
#ifdef MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
#define MAGICKCORE_FEATURE_ZERO_CONFIGURATION_STR "Zero-Configuration "
#else 
#define MAGICKCORE_FEATURE_ZERO_CONFIGURATION_STR ""
#endif
#ifdef HDRI_SUPPORT
#define MAGICKCORE_FEATURE_HDRI_STR "HDRI"
#else
#define MAGICKCORE_FEATURE_HDRI_STR ""
#endif

#define MAGICKCORE_FEATURES_STR MAGICKCORE_FEATURE_OPENMP_STR MAGICKCORE_FEATURE_OPENCL_STR MAGICKCORE_FEATURE_ZERO_CONFIGURATION_STR MAGICKCORE_FEATURE_HDRI_STR

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
static const ConfigureMapInfo
  ConfigureMap[] =
  {
    { "NAME", "ImageMagick" },
    { "QuantumDepth", MAGICKCORE_STRING_XQUOTE(MAGICKCORE_QUANTUM_DEPTH) },
    { "FEATURES", MAGICKCORE_FEATURES_STR }
  };

static LinkedListInfo
  *configure_list = (LinkedListInfo *) NULL;

static SemaphoreInfo
  *configure_semaphore = (SemaphoreInfo *) NULL;

static volatile MagickBooleanType
  instantiate_configure = MagickFalse;

/*
  Forward declarations.
*/
static MagickBooleanType
  InitializeConfigureList(ExceptionInfo *),
  LoadConfigureLists(const char *,ExceptionInfo *);

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
MagickExport MagickBooleanType ConfigureComponentGenesis(void)
{
  AcquireSemaphoreInfo(&configure_semaphore);
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
  register ConfigureInfo
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

MagickExport void ConfigureComponentTerminus(void)
{
  if (configure_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&configure_semaphore);
  LockSemaphoreInfo(configure_semaphore);
  if (configure_list != (LinkedListInfo *) NULL)
    configure_list=DestroyLinkedList(configure_list,DestroyConfigureElement);
  configure_list=(LinkedListInfo *) NULL;
  instantiate_configure=MagickFalse;
  UnlockSemaphoreInfo(configure_semaphore);
  DestroySemaphoreInfo(&configure_semaphore);
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
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
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
  register const ConfigureInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  if ((configure_list == (LinkedListInfo *) NULL) ||
      (instantiate_configure == MagickFalse))
    if (InitializeConfigureList(exception) == MagickFalse)
      return((const ConfigureInfo *) NULL);
  if ((configure_list == (LinkedListInfo *) NULL) ||
      (IsLinkedListEmpty(configure_list) != MagickFalse))
    return((const ConfigureInfo *) NULL);
  if ((name == (const char *) NULL) || (LocaleCompare(name,"*") == 0))
    return((const ConfigureInfo *) GetValueFromLinkedList(configure_list,0));
  /*
    Search for configure tag.
  */
  LockSemaphoreInfo(configure_semaphore);
  ResetLinkedListIterator(configure_list);
  p=(const ConfigureInfo *) GetNextValueInLinkedList(configure_list);
  while (p != (const ConfigureInfo *) NULL)
  {
    if (LocaleCompare(name,p->name) == 0)
      break;
    p=(const ConfigureInfo *) GetNextValueInLinkedList(configure_list);
  }
  if (p != (ConfigureInfo *) NULL)
    (void) InsertValueInLinkedList(configure_list,0,
      RemoveElementByValueFromLinkedList(configure_list,p));
  UnlockSemaphoreInfo(configure_semaphore);
  return(p);
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

  register const ConfigureInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate configure list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_options != (size_t *) NULL);
  *number_options=0;
  p=GetConfigureInfo("*",exception);
  if (p == (const ConfigureInfo *) NULL)
    return((const ConfigureInfo **) NULL);
  options=(const ConfigureInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(configure_list)+1UL,sizeof(*options));
  if (options == (const ConfigureInfo **) NULL)
    return((const ConfigureInfo **) NULL);
  /*
    Generate configure list.
  */
  LockSemaphoreInfo(configure_semaphore);
  ResetLinkedListIterator(configure_list);
  p=(const ConfigureInfo *) GetNextValueInLinkedList(configure_list);
  for (i=0; p != (const ConfigureInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      options[i++]=p;
    p=(const ConfigureInfo *) GetNextValueInLinkedList(configure_list);
  }
  UnlockSemaphoreInfo(configure_semaphore);
  qsort((void *) options,(size_t) i,sizeof(*options),ConfigureInfoCompare);
  options[i]=(ConfigureInfo *) NULL;
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
  register char
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

  register const ConfigureInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate configure list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_options != (size_t *) NULL);
  *number_options=0;
  p=GetConfigureInfo("*",exception);
  if (p == (const ConfigureInfo *) NULL)
    return((char **) NULL);
  options=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(configure_list)+1UL,sizeof(*options));
  if (options == (char **) NULL)
    return((char **) NULL);
  LockSemaphoreInfo(configure_semaphore);
  ResetLinkedListIterator(configure_list);
  p=(const ConfigureInfo *) GetNextValueInLinkedList(configure_list);
  for (i=0; p != (const ConfigureInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      options[i++]=ConstantString(p->name);
    p=(const ConfigureInfo *) GetNextValueInLinkedList(configure_list);
  }
  UnlockSemaphoreInfo(configure_semaphore);
  qsort((void *) options,(size_t) i,sizeof(*options),ConfigureCompare);
  options[i]=(char *) NULL;
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
    path[MaxTextExtent];

  const char
    *element;

  LinkedListInfo
    *options,
    *paths;

  StringInfo
    *xml;

  assert(filename != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  assert(exception != (ExceptionInfo *) NULL);
  (void) CopyMagickString(path,filename,MaxTextExtent);
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
        (void) FormatLocaleString(path,MaxTextExtent,"%s%s",element,filename);
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
#define RegistryKey  "ConfigurePath"
#define MagickCoreDLL  "CORE_RL_magick_.dll"
#define MagickCoreDebugDLL  "CORE_DB_magick_.dll"

  char
    path[MaxTextExtent];

  LinkedListInfo
    *paths;

  assert(filename != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  assert(exception != (ExceptionInfo *) NULL);
  (void) exception;
  (void) CopyMagickString(path,filename,MaxTextExtent);
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
        register char
          *p,
          *q;

        for (p=configure_path-1; p != (char *) NULL; )
        {
          (void) CopyMagickString(path,p+1,MaxTextExtent);
          q=strchr(path,DirectoryListSeparator);
          if (q != (char *) NULL)
            *q='\0';
          q=path+strlen(path)-1;
          if ((q >= path) && (*q != *DirectorySeparator))
            (void) ConcatenateMagickString(path,DirectorySeparator,
              MaxTextExtent);
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
    key_value=NTRegistryKeyLookup(RegistryKey);
    if (key_value != (unsigned char *) NULL)
      {
        (void) FormatLocaleString(path,MaxTextExtent,"%s%s",(char *) key_value,
          DirectorySeparator);
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
#if !defined(MAGICKCORE_POSIX_SUPPORT)
        (void) FormatLocaleString(path,MaxTextExtent,"%s%s",home,
          DirectorySeparator);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
#else
        (void) FormatLocaleString(path,MaxTextExtent,"%s/etc/%s/",home,
          MAGICKCORE_CONFIGURE_RELATIVE_PATH);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
        (void) FormatLocaleString(path,MaxTextExtent,"%s/share/%s/",home,
          MAGICKCORE_SHARE_RELATIVE_PATH);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
        (void) FormatLocaleString(path,MaxTextExtent,"%s",
          MAGICKCORE_SHAREARCH_PATH);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
#endif
        home=DestroyString(home);
      }
    }
  if (*GetClientPath() != '\0')
    {
#if !defined(MAGICKCORE_POSIX_SUPPORT)
      (void) FormatLocaleString(path,MaxTextExtent,"%s%s",GetClientPath(),
        DirectorySeparator);
      (void) AppendValueToLinkedList(paths,ConstantString(path));
#else
      char
        prefix[MaxTextExtent];

      /*
        Search based on executable directory if directory is known.
      */
      (void) CopyMagickString(prefix,GetClientPath(),MaxTextExtent);
      ChopPathComponents(prefix,1);
      (void) FormatLocaleString(path,MaxTextExtent,"%s/etc/%s/",prefix,
        MAGICKCORE_CONFIGURE_RELATIVE_PATH);
      (void) AppendValueToLinkedList(paths,ConstantString(path));
      (void) FormatLocaleString(path,MaxTextExtent,"%s/share/%s/",prefix,
        MAGICKCORE_SHARE_RELATIVE_PATH);
      (void) AppendValueToLinkedList(paths,ConstantString(path));
      (void) FormatLocaleString(path,MaxTextExtent,"%s",
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

    home=GetEnvironmentValue("HOME");
    if (home == (char *) NULL)
      home=GetEnvironmentValue("USERPROFILE");
    if (home != (char *) NULL)
      {
        /*
          Search $HOME/.magick.
        */
        (void) FormatLocaleString(path,MaxTextExtent,"%s%s.magick%s",home,
          DirectorySeparator,DirectorySeparator);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
        home=DestroyString(home);
      }
  }
#if defined(MAGICKCORE_WINDOWS_SUPPORT)

  {
    char
      module_path[MaxTextExtent];

    if ((NTGetModulePath(MagickCoreDLL,module_path) != MagickFalse) ||
        (NTGetModulePath(MagickCoreDebugDLL,module_path) != MagickFalse))
      {
        unsigned char
          *key_value;

        /*
          Search module path.
        */
        (void) FormatLocaleString(path,MaxTextExtent,"%s%s",module_path,
          DirectorySeparator);
        key_value=NTRegistryKeyLookup(RegistryKey);
        if (key_value == (unsigned char *) NULL)
          (void) AppendValueToLinkedList(paths,ConstantString(path));
        else
          key_value=(unsigned char *) RelinquishMagickMemory(key_value);
      }
    if (NTGetModulePath("Magick.dll",module_path) != MagickFalse)
      {
        /*
          Search PerlMagick module path.
        */
        (void) FormatLocaleString(path,MaxTextExtent,"%s%s",module_path,
          DirectorySeparator);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
        (void) FormatLocaleString(path,MaxTextExtent,"%s%s",module_path,
          "\\inc\\lib\\auto\\Image\\Magick\\");
        (void) AppendValueToLinkedList(paths,ConstantString(path));
      }
  }
#endif
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
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(configure_info != (ConfigureInfo *) NULL);
  assert(configure_info->signature == MagickSignature);
  return(configure_info->value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t i a l i z e C o n f i g u r e L i s t                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeConfigureList() initializes the configure list.
%
%  The format of the InitializeConfigureList method is:
%
%      MagickBooleanType InitializeConfigureList(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType InitializeConfigureList(ExceptionInfo *exception)
{
  if ((configure_list == (LinkedListInfo *) NULL) &&
      (instantiate_configure == MagickFalse))
    {
      if (configure_semaphore == (SemaphoreInfo *) NULL)
        AcquireSemaphoreInfo(&configure_semaphore);
      LockSemaphoreInfo(configure_semaphore);
      if ((configure_list == (LinkedListInfo *) NULL) &&
          (instantiate_configure == MagickFalse))
        {
          (void) LoadConfigureLists(ConfigureFilename,exception);
          instantiate_configure=MagickTrue;
        }
      UnlockSemaphoreInfo(configure_semaphore);
    }
  return(configure_list != (LinkedListInfo *) NULL ? MagickTrue : MagickFalse);
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

  register ssize_t
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
        (void) FormatLocaleFile(file,"Name           Value\n");
        (void) FormatLocaleFile(file,
          "-------------------------------------------------"
          "------------------------------\n");
      }
    path=configure_info[i]->path;
    name="unknown";
    if (configure_info[i]->name != (char *) NULL)
      name=configure_info[i]->name;
    (void) FormatLocaleFile(file,"%s",name);
    for (j=(ssize_t) strlen(name); j <= 13; j++)
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d C o n f i g u r e L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadConfigureList() loads the configure configuration file which provides a
%  mapping between configure attributes and a configure name.
%
%  The format of the LoadConfigureList method is:
%
%      MagickBooleanType LoadConfigureList(const char *xml,const char *filename,
%        const size_t depth,ExceptionInfo *exception)
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
static MagickBooleanType LoadConfigureList(const char *xml,const char *filename,
  const size_t depth,ExceptionInfo *exception)
{
  char
    keyword[MaxTextExtent],
    *token;

  ConfigureInfo
    *configure_info;

  const char
    *q;

  MagickBooleanType
    status;

  /*
    Load the configure map file.
  */
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading configure file \"%s\" ...",filename);
  if (configure_list == (LinkedListInfo *) NULL)
    {
      configure_list=NewLinkedList(0);
      if (configure_list == (LinkedListInfo *) NULL)
        {
          ThrowFileException(exception,ResourceLimitError,
            "MemoryAllocationFailed",filename);
          return(MagickFalse);
        }
    }
  status=MagickTrue;
  configure_info=(ConfigureInfo *) NULL;
  token=AcquireString((char *) xml);
  for (q=(char *) xml; *q != '\0'; )
  {
    /*
      Interpret XML.
    */
    GetMagickToken(q,&q,token);
    if (*token == '\0')
      break;
    (void) CopyMagickString(keyword,token,MaxTextExtent);
    if (LocaleNCompare(keyword,"<!DOCTYPE",9) == 0)
      {
        /*
          Doctype element.
        */
        while ((LocaleNCompare(q,"]>",2) != 0) && (*q != '\0'))
          GetMagickToken(q,&q,token);
        continue;
      }
    if (LocaleNCompare(keyword,"<!--",4) == 0)
      {
        /*
          Comment element.
        */
        while ((LocaleNCompare(q,"->",2) != 0) && (*q != '\0'))
          GetMagickToken(q,&q,token);
        continue;
      }
    if (LocaleCompare(keyword,"<include") == 0)
      {
        /*
          Include element.
        */
        while (((*token != '/') && (*(token+1) != '>')) && (*q != '\0'))
        {
          (void) CopyMagickString(keyword,token,MaxTextExtent);
          GetMagickToken(q,&q,token);
          if (*token != '=')
            continue;
          GetMagickToken(q,&q,token);
          if (LocaleCompare(keyword,"file") == 0)
            {
              if (depth > 200)
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ConfigureError,"IncludeElementNestedTooDeeply","`%s'",token);
              else
                {
                  char
                    path[MaxTextExtent],
                    *xml;

                  GetPathComponent(filename,HeadPath,path);
                  if (*path != '\0')
                    (void) ConcatenateMagickString(path,DirectorySeparator,
                      MaxTextExtent);
                  if (*token == *DirectorySeparator)
                    (void) CopyMagickString(path,token,MaxTextExtent);
                  else
                    (void) ConcatenateMagickString(path,token,MaxTextExtent);
                  xml=FileToString(path,~0UL,exception);
                  if (xml != (char *) NULL)
                    {
                      status=LoadConfigureList(xml,path,depth+1,exception);
                      xml=(char *) RelinquishMagickMemory(xml);
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
        configure_info=(ConfigureInfo *) AcquireMagickMemory(
          sizeof(*configure_info));
        if (configure_info == (ConfigureInfo *) NULL)
          ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
        (void) ResetMagickMemory(configure_info,0,sizeof(*configure_info));
        configure_info->path=ConstantString(filename);
        configure_info->exempt=MagickFalse;
        configure_info->signature=MagickSignature;
        continue;
      }
    if (configure_info == (ConfigureInfo *) NULL)
      continue;
    if (LocaleCompare(keyword,"/>") == 0)
      {
        status=AppendValueToLinkedList(configure_list,configure_info);
        if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            configure_info->name);
        configure_info=(ConfigureInfo *) NULL;
      }
    /*
      Parse configure element.
    */
    GetMagickToken(q,(const char **) NULL,token);
    if (*token != '=')
      continue;
    GetMagickToken(q,&q,token);
    GetMagickToken(q,&q,token);
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
            configure_info->stealth=IsMagickTrue(token);
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
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L o a d C o n f i g u r e L i s t s                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadConfigureList() loads one or more configure configuration files which
%  provides a mapping between configure attributes and a configure name.
%
%  The format of the LoadConfigureLists method is:
%
%      MagickBooleanType LoadConfigureLists(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadConfigureLists(const char *filename,
  ExceptionInfo *exception)
{
  const StringInfo
    *option;

  LinkedListInfo
    *options;

  MagickStatusType
    status;

  register ssize_t
    i;

  /*
    Load external configure map.
  */
  if (configure_list == (LinkedListInfo *) NULL)
    {
      configure_list=NewLinkedList(0);
      if (configure_list == (LinkedListInfo *) NULL)
        {
          ThrowFileException(exception,ResourceLimitError,
            "MemoryAllocationFailed",filename);
          return(MagickFalse);
        }
    }
  status=MagickTrue;
  options=GetConfigureOptions(filename,exception);
  option=(const StringInfo *) GetNextValueInLinkedList(options);
  while (option != (const StringInfo *) NULL)
  {
    status&=LoadConfigureList((const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),0,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
  }
  options=DestroyConfigureOptions(options);
  /*
    Load built-in configure map.
  */
  for (i=0; i < (ssize_t) (sizeof(ConfigureMap)/sizeof(*ConfigureMap)); i++)
  {
    ConfigureInfo
      *configure_info;

    register const ConfigureMapInfo
      *p;

    p=ConfigureMap+i;
    configure_info=(ConfigureInfo *) AcquireMagickMemory(
      sizeof(*configure_info));
    if (configure_info == (ConfigureInfo *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",
          configure_info->name);
        continue;
      }
    (void) ResetMagickMemory(configure_info,0,sizeof(*configure_info));
    configure_info->path=(char *) "[built-in]";
    configure_info->name=(char *) p->name;
    configure_info->value=(char *) p->value;
    configure_info->exempt=MagickTrue;
    configure_info->signature=MagickSignature;
    status&=AppendValueToLinkedList(configure_list,configure_info);
    if (status == MagickFalse)
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        configure_info->name);
  }
  return(status != 0 ? MagickTrue : MagickFalse);
}

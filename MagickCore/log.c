/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             L       OOO    GGGG                             %
%                             L      O   O  G                                 %
%                             L      O   O  G GG                              %
%                             L      O   O  G   G                             %
%                             LLLLL   OOO    GGG                              %
%                                                                             %
%                                                                             %
%                             MagickCore Log Events                           %
%                                                                             %
%                               Software Design                               %
%                                    Cristy                                   %
%                                September 2002                               %
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
#include "MagickCore/log.h"
#include "MagickCore/log-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/option.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/timer.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/timer-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/version.h"
#include "MagickCore/xml-tree.h"
#include "MagickCore/xml-tree-private.h"

/*
  Define declarations.
*/
#define LogFilename  "log.xml"

/*
  Typedef declarations.
*/
typedef enum
{
  UndefinedHandler = 0x0000,
  NoHandler = 0x0000,
  ConsoleHandler = 0x0001,
  StdoutHandler = 0x0002,
  StderrHandler = 0x0004,
  FileHandler = 0x0008,
  DebugHandler = 0x0010,
  EventHandler = 0x0020,
  MethodHandler = 0x0040
} LogHandlerType;

typedef struct _EventInfo
{
  char
    *name;

  LogEventType
    event;
} EventInfo;

typedef struct _HandlerInfo
{
  const char
    name[10];

  LogHandlerType
    handler;
} HandlerInfo;

struct _LogInfo
{
  LogEventType
    event_mask;

  LogHandlerType
    handler_mask;

  char
    *path,
    *name,
    *filename,
    *format;

  size_t
    generations,
    limit;

  FILE
    *file;

  size_t
    generation;

  MagickBooleanType
    append,
    stealth;

  TimerInfo
    timer;

  MagickLogMethod
    method;

  SemaphoreInfo
    *event_semaphore;

  size_t
    signature;
};

typedef struct _LogMapInfo
{
  const LogEventType
    event_mask;

  const LogHandlerType
    handler_mask;

  const char
    *filename,
    *format;
} LogMapInfo;

/*
  Static declarations.
*/
static const HandlerInfo
  LogHandlers[32] =
  {
    { "Console", ConsoleHandler },
    { "Debug", DebugHandler },
    { "Event", EventHandler },
    { "File", FileHandler },
    { "None", NoHandler },
    { "Stderr", StderrHandler },
    { "Stdout", StdoutHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler },
    { "", UndefinedHandler }
  };

static const LogMapInfo
  LogMap[] =
  {
    { NoEvents, ConsoleHandler, "Magick-%g.log",
      "%t %r %u %v %d %c[%p]: %m/%f/%l/%d\\n  %e" }
  };

static char
  log_name[MagickPathExtent] = "Magick";

static LinkedListInfo
  *log_cache = (LinkedListInfo *) NULL;

static MagickBooleanType
  event_logging = MagickFalse;

static SemaphoreInfo
  *log_semaphore = (SemaphoreInfo *) NULL;

/*
  Forward declarations.
*/
#if !MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
static LogHandlerType
  ParseLogHandlers(const char *) magick_attribute((__pure__));
#endif

static LogInfo
  *GetLogInfo(const char *,ExceptionInfo *);

static MagickBooleanType
  IsLogCacheInstantiated(ExceptionInfo *) magick_attribute((__pure__));

#if !MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
static MagickBooleanType
  LoadLogCache(LinkedListInfo *,const char *,const char *,const size_t,
    ExceptionInfo *);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A c q u i r e L o g C a c h e                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireLogCache() caches one or more log configurations which provides a
%  mapping between log attributes and log name.
%
%  The format of the AcquireLogCache method is:
%
%      LinkedListInfo *AcquireLogCache(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the log configuration filename.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static LinkedListInfo *AcquireLogCache(const char *filename,
  ExceptionInfo *exception)
{
  LinkedListInfo
    *cache;

  MagickStatusType
    status;

  register ssize_t
    i;

  /*
    Load external log map.
  */
  cache=NewLinkedList(0);
  status=MagickTrue;
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
      status&=LoadLogCache(cache,(const char *) GetStringInfoDatum(option),
        GetStringInfoPath(option),0,exception);
      option=(const StringInfo *) GetNextValueInLinkedList(options);
    }
    options=DestroyConfigureOptions(options);
  }
#endif
  /*
    Load built-in log map.
  */
  for (i=0; i < (ssize_t) (sizeof(LogMap)/sizeof(*LogMap)); i++)
  {
    LogInfo
      *log_info;

    register const LogMapInfo
      *p;

    p=LogMap+i;
    log_info=(LogInfo *) AcquireMagickMemory(sizeof(*log_info));
    if (log_info == (LogInfo *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",p->filename);
        continue;
      }
    (void) memset(log_info,0,sizeof(*log_info));
    log_info->path=ConstantString("[built-in]");
    GetTimerInfo((TimerInfo *) &log_info->timer);
    log_info->event_mask=p->event_mask;
    log_info->handler_mask=p->handler_mask;
    log_info->filename=ConstantString(p->filename);
    log_info->format=ConstantString(p->format);
    log_info->signature=MagickCoreSignature;
    status&=AppendValueToLinkedList(cache,log_info);
    if (status == MagickFalse)
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",log_info->name);
  }
  return(cache);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o s e M a g i c k L o g                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloseMagickLog() closes the Magick log.
%
%  The format of the CloseMagickLog method is:
%
%      CloseMagickLog(void)
%
*/
MagickExport void CloseMagickLog(void)
{
  ExceptionInfo
    *exception;

  LogInfo
    *log_info;

  if (IsEventLogging() == MagickFalse)
    return;
  exception=AcquireExceptionInfo();
  log_info=GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  LockSemaphoreInfo(log_semaphore);
  if (log_info->file != (FILE *) NULL)
    {
      (void) FormatLocaleFile(log_info->file,"</log>\n");
      (void) fclose(log_info->file);
      log_info->file=(FILE *) NULL;
    }
  UnlockSemaphoreInfo(log_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t L o g I n f o                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLogInfo() searches the log list for the specified name and if found
%  returns attributes for that log.
%
%  The format of the GetLogInfo method is:
%
%      LogInfo *GetLogInfo(const char *name,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o name: the log name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static LogInfo *GetLogInfo(const char *name,ExceptionInfo *exception)
{
  register LogInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  if (IsLogCacheInstantiated(exception) == MagickFalse)
    return((LogInfo *) NULL);
  /*
    Search for log tag.
  */
  LockSemaphoreInfo(log_semaphore);
  ResetLinkedListIterator(log_cache);
  p=(LogInfo *) GetNextValueInLinkedList(log_cache);
  if ((name == (const char *) NULL) || (LocaleCompare(name,"*") == 0))
    {
      UnlockSemaphoreInfo(log_semaphore);
      return(p);
    }
  while (p != (LogInfo *) NULL)
  {
    if (LocaleCompare(name,p->name) == 0)
      break;
    p=(LogInfo *) GetNextValueInLinkedList(log_cache);
  }
  if (p != (LogInfo *) NULL)
    (void) InsertValueInLinkedList(log_cache,0,
      RemoveElementByValueFromLinkedList(log_cache,p));
  UnlockSemaphoreInfo(log_semaphore);
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o g I n f o L i s t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLogInfoList() returns any logs that match the specified pattern.
%
%  The format of the GetLogInfoList function is:
%
%      const LogInfo **GetLogInfoList(const char *pattern,
%        size_t *number_preferences,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_preferences:  This integer returns the number of logs in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int LogInfoCompare(const void *x,const void *y)
{
  const LogInfo
    **p,
    **q;

  p=(const LogInfo **) x,
  q=(const LogInfo **) y;
  if (LocaleCompare((*p)->path,(*q)->path) == 0)
    return(LocaleCompare((*p)->name,(*q)->name));
  return(LocaleCompare((*p)->path,(*q)->path));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport const LogInfo **GetLogInfoList(const char *pattern,
  size_t *number_preferences,ExceptionInfo *exception)
{
  const LogInfo
    **preferences;

  register const LogInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate log list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_preferences != (size_t *) NULL);
  *number_preferences=0;
  p=GetLogInfo("*",exception);
  if (p == (const LogInfo *) NULL)
    return((const LogInfo **) NULL);
  preferences=(const LogInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(log_cache)+1UL,sizeof(*preferences));
  if (preferences == (const LogInfo **) NULL)
    return((const LogInfo **) NULL);
  /*
    Generate log list.
  */
  LockSemaphoreInfo(log_semaphore);
  ResetLinkedListIterator(log_cache);
  p=(const LogInfo *) GetNextValueInLinkedList(log_cache);
  for (i=0; p != (const LogInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      preferences[i++]=p;
    p=(const LogInfo *) GetNextValueInLinkedList(log_cache);
  }
  UnlockSemaphoreInfo(log_semaphore);
  qsort((void *) preferences,(size_t) i,sizeof(*preferences),LogInfoCompare);
  preferences[i]=(LogInfo *) NULL;
  *number_preferences=(size_t) i;
  return(preferences);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o g L i s t                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLogList() returns any logs that match the specified pattern.
%
%  The format of the GetLogList function is:
%
%      char **GetLogList(const char *pattern,size_t *number_preferences,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_preferences:  This integer returns the number of logs in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int LogCompare(const void *x,const void *y)
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

MagickExport char **GetLogList(const char *pattern,size_t *number_preferences,
  ExceptionInfo *exception)
{
  char
    **preferences;

  register const LogInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate log list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_preferences != (size_t *) NULL);
  *number_preferences=0;
  p=GetLogInfo("*",exception);
  if (p == (const LogInfo *) NULL)
    return((char **) NULL);
  preferences=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(log_cache)+1UL,sizeof(*preferences));
  if (preferences == (char **) NULL)
    return((char **) NULL);
  /*
    Generate log list.
  */
  LockSemaphoreInfo(log_semaphore);
  ResetLinkedListIterator(log_cache);
  p=(const LogInfo *) GetNextValueInLinkedList(log_cache);
  for (i=0; p != (const LogInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      preferences[i++]=ConstantString(p->name);
    p=(const LogInfo *) GetNextValueInLinkedList(log_cache);
  }
  UnlockSemaphoreInfo(log_semaphore);
  qsort((void *) preferences,(size_t) i,sizeof(*preferences),LogCompare);
  preferences[i]=(char *) NULL;
  *number_preferences=(size_t) i;
  return(preferences);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o g N a m e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLogName() returns the current log name.
%
%  The format of the GetLogName method is:
%
%      const char *GetLogName(void)
%
*/
MagickExport const char *GetLogName(void)
{
  return(log_name);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s L o g C a c h e I n s t a n t i a t e d                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsLogCacheInstantiated() determines if the log list is instantiated.  If
%  not, it instantiates the list and returns it.
%
%  The format of the IsLogInstantiated method is:
%
%      MagickBooleanType IsLogCacheInstantiated(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline void CheckEventLogging()
{
  /*
    Are we logging events?
  */
  if (IsLinkedListEmpty(log_cache) != MagickFalse)
    event_logging=MagickFalse;
  else
    {
      LogInfo
        *p;

      ResetLinkedListIterator(log_cache);
      p=(LogInfo *) GetNextValueInLinkedList(log_cache);
      event_logging=(p != (LogInfo *) NULL) && (p->event_mask != NoEvents) ?
        MagickTrue: MagickFalse;
    }
}

static MagickBooleanType IsLogCacheInstantiated(ExceptionInfo *exception)
{
  if (log_cache == (LinkedListInfo *) NULL)
    {
      if (log_semaphore == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&log_semaphore);
      LockSemaphoreInfo(log_semaphore);
      if (log_cache == (LinkedListInfo *) NULL)
        {
          log_cache=AcquireLogCache(LogFilename,exception);
          CheckEventLogging();
        }
      UnlockSemaphoreInfo(log_semaphore);
    }
  return(log_cache != (LinkedListInfo *) NULL ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I s E v e n t L o g g i n g                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsEventLogging() returns MagickTrue if debug of events is enabled otherwise
%  MagickFalse.
%
%  The format of the IsEventLogging method is:
%
%      MagickBooleanType IsEventLogging(void)
%
*/
MagickExport MagickBooleanType IsEventLogging(void)
{
  return(event_logging);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t L o g I n f o                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListLogInfo() lists the log info to a file.
%
%  The format of the ListLogInfo method is:
%
%      MagickBooleanType ListLogInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListLogInfo(FILE *file,ExceptionInfo *exception)
{
#define MegabytesToBytes(value) ((MagickSizeType) (value)*1024*1024)

  const char
    *path;

  const LogInfo
    **log_info;

  register ssize_t
    i;

  size_t
    number_aliases;

  ssize_t
    j;

  if (file == (const FILE *) NULL)
    file=stdout;
  log_info=GetLogInfoList("*",&number_aliases,exception);
  if (log_info == (const LogInfo **) NULL)
    return(MagickFalse);
  j=0;
  path=(const char *) NULL;
  for (i=0; i < (ssize_t) number_aliases; i++)
  {
    if (log_info[i]->stealth != MagickFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (LocaleCompare(path,log_info[i]->path) != 0))
      {
        size_t
          length;

        if (log_info[i]->path != (char *) NULL)
          (void) FormatLocaleFile(file,"\nPath: %s\n\n",log_info[i]->path);
        length=0;
        for (j=0; j < (ssize_t) (8*sizeof(LogHandlerType)); j++)
        {
          size_t
            mask;

          if (*LogHandlers[j].name == '\0')
            break;
          mask=1;
          mask<<=j;
          if ((log_info[i]->handler_mask & mask) != 0)
            {
              (void) FormatLocaleFile(file,"%s ",LogHandlers[j].name);
              length+=strlen(LogHandlers[j].name);
            }
        }
        for (j=(ssize_t) length; j <= 12; j++)
          (void) FormatLocaleFile(file," ");
        (void) FormatLocaleFile(file," Generations     Limit  Format\n");
        (void) FormatLocaleFile(file,"-----------------------------------------"
          "--------------------------------------\n");
      }
    path=log_info[i]->path;
    if (log_info[i]->filename != (char *) NULL)
      {
        (void) FormatLocaleFile(file,"%s",log_info[i]->filename);
        for (j=(ssize_t) strlen(log_info[i]->filename); j <= 16; j++)
          (void) FormatLocaleFile(file," ");
      }
    (void) FormatLocaleFile(file,"%9g  ",(double) log_info[i]->generations);
    (void) FormatLocaleFile(file,"%8g   ",(double) log_info[i]->limit);
    if (log_info[i]->format != (char *) NULL)
      (void) FormatLocaleFile(file,"%s",log_info[i]->format);
    (void) FormatLocaleFile(file,"\n");
  }
  (void) fflush(file);
  log_info=(const LogInfo **) RelinquishMagickMemory((void *) log_info);
  return(MagickTrue);
}

#if !MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o a d L o g C a c h e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadLogCache() loads the log configurations which provides a
%  mapping between log attributes and log name.
%
%  The format of the LoadLogCache method is:
%
%      MagickBooleanType LoadLogCache(LinkedListInfo *cache,const char *xml,
%        const char *filename,const size_t depth,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The log list in XML format.
%
%    o filename:  The log list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadLogCache(LinkedListInfo *cache,const char *xml,
  const char *filename,const size_t depth,ExceptionInfo *exception)
{
  char
    keyword[MagickPathExtent],
    *token;

  const char
    *q;

  LogInfo
    *log_info = (LogInfo *) NULL;

  MagickStatusType
    status;

  size_t
    extent;

  /*
    Load the log map file.
  */
  if (xml == (const char *) NULL)
    return(MagickFalse);
  status=MagickTrue;
  token=AcquireString(xml);
  extent=strlen(token)+MagickPathExtent;
  for (q=(const char *) xml; *q != '\0'; )
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
                      status&=LoadLogCache(cache,file_xml,path,depth+1,
                        exception);
                      file_xml=DestroyString(file_xml);
                    }
                }
            }
        }
        continue;
      }
    if (LocaleCompare(keyword,"<logmap>") == 0)
      {
        /*
          Allocate memory for the log list.
        */
        log_info=(LogInfo *) AcquireCriticalMemory(sizeof(*log_info));
        (void) memset(log_info,0,sizeof(*log_info));
        log_info->path=ConstantString(filename);
        GetTimerInfo((TimerInfo *) &log_info->timer);
        log_info->signature=MagickCoreSignature;
        continue;
      }
    if (log_info == (LogInfo *) NULL)
      continue;
    if (LocaleCompare(keyword,"</logmap>") == 0)
      {
        status=AppendValueToLinkedList(cache,log_info);
        if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",filename);
        log_info=(LogInfo *) NULL;
        continue;
      }
    (void) GetNextToken(q,(const char **) NULL,extent,token);
    if (*token != '=')
      continue;
    (void) GetNextToken(q,&q,extent,token);
    (void) GetNextToken(q,&q,extent,token);
    switch (*keyword)
    {
      case 'E':
      case 'e':
      {
        if (LocaleCompare((char *) keyword,"events") == 0)
          {
            log_info->event_mask=(LogEventType) (log_info->event_mask |
              ParseCommandOption(MagickLogEventOptions,MagickTrue,token));
            break;
          }
        break;
      }
      case 'F':
      case 'f':
      {
        if (LocaleCompare((char *) keyword,"filename") == 0)
          {
            if (log_info->filename != (char *) NULL)
              log_info->filename=(char *)
                RelinquishMagickMemory(log_info->filename);
            log_info->filename=ConstantString(token);
            break;
          }
        if (LocaleCompare((char *) keyword,"format") == 0)
          {
            if (log_info->format != (char *) NULL)
              log_info->format=(char *)
                RelinquishMagickMemory(log_info->format);
            log_info->format=ConstantString(token);
            break;
          }
        break;
      }
      case 'G':
      case 'g':
      {
        if (LocaleCompare((char *) keyword,"generations") == 0)
          {
            if (LocaleCompare(token,"unlimited") == 0)
              {
                log_info->generations=(~0UL);
                break;
              }
            log_info->generations=StringToUnsignedLong(token);
            break;
          }
        break;
      }
      case 'L':
      case 'l':
      {
        if (LocaleCompare((char *) keyword,"limit") == 0)
          {
            if (LocaleCompare(token,"unlimited") == 0)
              {
                log_info->limit=(~0UL);
                break;
              }
            log_info->limit=StringToUnsignedLong(token);
            break;
          }
        break;
      }
      case 'O':
      case 'o':
      {
        if (LocaleCompare((char *) keyword,"output") == 0)
          {
            log_info->handler_mask=(LogHandlerType)
              (log_info->handler_mask | ParseLogHandlers(token));
            break;
          }
        break;
      }
      default:
        break;
    }
  }
  token=DestroyString(token);
  if (cache == (LinkedListInfo *) NULL)
    return(MagickFalse);
  return(status != 0 ? MagickTrue : MagickFalse);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o g C o m p o n e n t G e n e s i s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LogComponentGenesis() instantiates the log component.
%
%  The format of the LogComponentGenesis method is:
%
%      MagickBooleanType LogComponentGenesis(void)
%
*/
MagickPrivate MagickBooleanType LogComponentGenesis(void)
{
  ExceptionInfo
    *exception;

  if (log_semaphore == (SemaphoreInfo *) NULL)
    log_semaphore=AcquireSemaphoreInfo();
  exception=AcquireExceptionInfo();
  (void) GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o g C o m p o n e n t T e r m i n u s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LogComponentTerminus() destroys the logging component.
%
%  The format of the LogComponentTerminus method is:
%
%      LogComponentTerminus(void)
%
*/

static void *DestroyLogElement(void *log_info)
{
  register LogInfo
    *p;

  p=(LogInfo *) log_info;
  if (p->file != (FILE *) NULL)
    {
      (void) FormatLocaleFile(p->file,"</log>\n");
      (void) fclose(p->file);
      p->file=(FILE *) NULL;
    }
  if (p->format != (char *) NULL)
    p->format=DestroyString(p->format);
  if (p->path != (char *) NULL)
    p->path=DestroyString(p->path);
  if (p->filename != (char *) NULL)
    p->filename=DestroyString(p->filename);
  if (p->event_semaphore != (SemaphoreInfo *) NULL)
    RelinquishSemaphoreInfo(&p->event_semaphore);
  p=(LogInfo *) RelinquishMagickMemory(p);
  return((void *) NULL);
}

MagickPrivate void LogComponentTerminus(void)
{
  if (log_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&log_semaphore);
  LockSemaphoreInfo(log_semaphore);
  if (log_cache != (LinkedListInfo *) NULL)
    log_cache=DestroyLinkedList(log_cache,DestroyLogElement);
  event_logging=MagickFalse;
  UnlockSemaphoreInfo(log_semaphore);
  RelinquishSemaphoreInfo(&log_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o g M a g i c k E v e n t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LogMagickEvent() logs an event as determined by the log configuration file.
%  If an error occurs, MagickFalse is returned otherwise MagickTrue.
%
%  The format of the LogMagickEvent method is:
%
%      MagickBooleanType LogMagickEvent(const LogEventType type,
%        const char *module,const char *function,const size_t line,
%        const char *format,...)
%
%  A description of each parameter follows:
%
%    o type: the event type.
%
%    o filename: the source module filename.
%
%    o function: the function name.
%
%    o line: the line number of the source module.
%
%    o format: the output format.
%
*/
static char *TranslateEvent(const char *module,const char *function,
  const size_t line,const char *domain,const char *event)
{
  char
    *text;

  double
    elapsed_time,
    user_time;

  ExceptionInfo
    *exception;

  LogInfo
    *log_info;

  register char
    *q;

  register const char
    *p;

  size_t
    extent;

  time_t
    seconds;

  exception=AcquireExceptionInfo();
  log_info=(LogInfo *) GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  seconds=GetMagickTime();
  elapsed_time=GetElapsedTime(&log_info->timer);
  user_time=GetUserTime(&log_info->timer);
  text=AcquireString(event);
  if (log_info->format == (char *) NULL)
    return(text);
  extent=strlen(event)+MagickPathExtent;
  if (LocaleCompare(log_info->format,"xml") == 0)
    {
      char
        timestamp[MagickPathExtent];

      /*
        Translate event in "XML" format.
      */
      (void) FormatMagickTime(seconds,extent,timestamp);
      (void) FormatLocaleString(text,extent,
        "<entry>\n"
        "  <timestamp>%s</timestamp>\n"
        "  <elapsed-time>%lu:%02lu.%06lu</elapsed-time>\n"
        "  <user-time>%0.3f</user-time>\n"
        "  <process-id>%.20g</process-id>\n"
        "  <thread-id>%.20g</thread-id>\n"
        "  <module>%s</module>\n"
        "  <function>%s</function>\n"
        "  <line>%.20g</line>\n"
        "  <domain>%s</domain>\n"
        "  <event>%s</event>\n"
        "</entry>",timestamp,(unsigned long) (elapsed_time/60.0),
        (unsigned long) floor(fmod(elapsed_time,60.0)),(unsigned long)
        (1000000.0*(elapsed_time-floor(elapsed_time))+0.5),user_time,
        (double) getpid(),(double) GetMagickThreadSignature(),module,function,
        (double) line,domain,event);
      return(text);
    }
  /*
    Translate event in "human readable" format.
  */
  q=text;
  for (p=log_info->format; *p != '\0'; p++)
  {
    *q='\0';
    if ((size_t) (q-text+MagickPathExtent) >= extent)
      {
        extent+=MagickPathExtent;
        text=(char *) ResizeQuantumMemory(text,extent+MagickPathExtent,
          sizeof(*text));
        if (text == (char *) NULL)
          return((char *) NULL);
        q=text+strlen(text);
      }
    /*
      The format of the log is defined by embedding special format characters:

        %c   client name
        %d   domain
        %e   event
        %f   function
        %g   generation
        %i   thread id
        %l   line
        %m   module
        %n   log name
        %p   process id
        %r   real CPU time
        %t   wall clock time
        %u   user CPU time
        %v   version
        %%   percent sign
        \n   newline
        \r   carriage return
    */
    if ((*p == '\\') && (*(p+1) == 'r'))
      {
        *q++='\r';
        p++;
        continue;
      }
    if ((*p == '\\') && (*(p+1) == 'n'))
      {
        *q++='\n';
        p++;
        continue;
      }
    if (*p != '%')
      {
        *q++=(*p);
        continue;
      }
    p++;
    if (*p == '\0')
      break;
    switch (*p)
    {
      case 'c':
      {
        q+=CopyMagickString(q,GetClientName(),extent);
        break;
      }
      case 'd':
      {
        q+=CopyMagickString(q,domain,extent);
        break;
      }
      case 'e':
      {
        q+=CopyMagickString(q,event,extent);
        break;
      }
      case 'f':
      {
        q+=CopyMagickString(q,function,extent);
        break;
      }
      case 'g':
      {
        if (log_info->generations == 0)
          {
            (void) CopyMagickString(q,"0",extent);
            q++;
            break;
          }
        q+=FormatLocaleString(q,extent,"%.20g",(double) (log_info->generation %
          log_info->generations));
        break;
      }
      case 'i':
      {
        q+=FormatLocaleString(q,extent,"%.20g",(double)
          GetMagickThreadSignature());
        break;
      }
      case 'l':
      {
        q+=FormatLocaleString(q,extent,"%.20g",(double) line);
        break;
      }
      case 'm':
      {
        register const char
          *r;

        for (r=module+strlen(module)-1; r > module; r--)
          if (*r == *DirectorySeparator)
            {
              r++;
              break;
            }
        q+=CopyMagickString(q,r,extent);
        break;
      }
      case 'n':
      {
        q+=CopyMagickString(q,GetLogName(),extent);
        break;
      }
      case 'p':
      {
        q+=FormatLocaleString(q,extent,"%.20g",(double) getpid());
        break;
      }
      case 'r':
      {
        q+=FormatLocaleString(q,extent,"%lu:%02lu.%03lu",(unsigned long)
          (elapsed_time/60.0),(unsigned long) floor(fmod(elapsed_time,60.0)),
          (unsigned long) (1000.0*(elapsed_time-floor(elapsed_time))+0.5));
        break;
      }
      case 't':
      {
        q+=FormatMagickTime(seconds,extent,q);
        break;
      }
      case 'u':
      {
        q+=FormatLocaleString(q,extent,"%0.3fu",user_time);
        break;
      }
      case 'v':
      {
        q+=CopyMagickString(q,MagickLibVersionText,extent);
        break;
      }
      case '%':
      {
        *q++=(*p);
        break;
      }
      default:
      {
        *q++='%';
        *q++=(*p);
        break;
      }
    }
  }
  *q='\0';
  return(text);
}

static char *TranslateFilename(const LogInfo *log_info)
{
  char
    *filename;

  register char
    *q;

  register const char
    *p;

  size_t
    extent;

  /*
    Translate event in "human readable" format.
  */
  assert(log_info != (LogInfo *) NULL);
  assert(log_info->filename != (char *) NULL);
  filename=AcquireString((char *) NULL);
  extent=MagickPathExtent;
  q=filename;
  for (p=log_info->filename; *p != '\0'; p++)
  {
    *q='\0';
    if ((size_t) (q-filename+MagickPathExtent) >= extent)
      {
        extent+=MagickPathExtent;
        filename=(char *) ResizeQuantumMemory(filename,extent+MagickPathExtent,
          sizeof(*filename));
        if (filename == (char *) NULL)
          return((char *) NULL);
        q=filename+strlen(filename);
      }
    /*
      The format of the filename is defined by embedding special format
      characters:

        %c   client name
        %n   log name
        %p   process id
        %v   version
        %%   percent sign
    */
    if (*p != '%')
      {
        *q++=(*p);
        continue;
      }
    p++;
    if (*p == '\0')
      break;
    switch (*p)
    {
      case '\0':
      {
        p--;
        break;
      }
      case 'c':
      {
        q+=CopyMagickString(q,GetClientName(),extent);
        break;
      }
      case 'g':
      {
        if (log_info->generations == 0)
          {
            (void) CopyMagickString(q,"0",extent);
            q++;
            break;
          }
        q+=FormatLocaleString(q,extent,"%.20g",(double) (log_info->generation %
          log_info->generations));
        break;
      }
      case 'n':
      {
        q+=CopyMagickString(q,GetLogName(),extent);
        break;
      }
      case 'p':
      {
        q+=FormatLocaleString(q,extent,"%.20g",(double) getpid());
        break;
      }
      case 'v':
      {
        q+=CopyMagickString(q,MagickLibVersionText,extent);
        break;
      }
      case '%':
      {
        *q++=(*p);
        break;
      }
      default:
      {
        *q++='%';
        *q++=(*p);
        break;
      }
    }
  }
  *q='\0';
  return(filename);
}

MagickExport MagickBooleanType LogMagickEventList(const LogEventType type,
  const char *module,const char *function,const size_t line,const char *format,
  va_list operands)
{
  char
    event[MagickPathExtent],
    *text;

  const char
    *domain;

  ExceptionInfo
    *exception;

  int
    n;

  LogInfo
    *log_info;

  exception=AcquireExceptionInfo();
  log_info=(LogInfo *) GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  if (log_info->event_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&log_info->event_semaphore);
  LockSemaphoreInfo(log_info->event_semaphore);
  if ((log_info->event_mask & type) == 0)
    {
      UnlockSemaphoreInfo(log_info->event_semaphore);
      return(MagickTrue);
    }
  domain=CommandOptionToMnemonic(MagickLogEventOptions,type);
#if defined(MAGICKCORE_HAVE_VSNPRINTF)
  n=vsnprintf(event,MagickPathExtent,format,operands);
#else
  n=vsprintf(event,format,operands);
#endif
  if (n < 0)
    event[MagickPathExtent-1]='\0';
  text=TranslateEvent(module,function,line,domain,event);
  if (text == (char *) NULL)
    {
      (void) ContinueTimer((TimerInfo *) &log_info->timer);
      UnlockSemaphoreInfo(log_info->event_semaphore);
      return(MagickFalse);
    }
  if ((log_info->handler_mask & ConsoleHandler) != 0)
    {
      (void) FormatLocaleFile(stderr,"%s\n",text);
      (void) fflush(stderr);
    }
  if ((log_info->handler_mask & DebugHandler) != 0)
    {
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
      OutputDebugString(text);
      OutputDebugString("\n");
#endif
    }
  if ((log_info->handler_mask & EventHandler) != 0)
    {
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
      (void) NTReportEvent(text,MagickFalse);
#endif
    }
  if ((log_info->handler_mask & FileHandler) != 0)
    {
      struct stat
        file_info;

      file_info.st_size=0;
      if (log_info->file != (FILE *) NULL)
        (void) fstat(fileno(log_info->file),&file_info);
      if (file_info.st_size > (MagickOffsetType) (1024*1024*log_info->limit))
        {
          (void) FormatLocaleFile(log_info->file,"</log>\n");
          (void) fclose(log_info->file);
          log_info->file=(FILE *) NULL;
        }
      if (log_info->file == (FILE *) NULL)
        {
          char
            *filename;

          filename=TranslateFilename(log_info);
          if (filename == (char *) NULL)
            {
              (void) ContinueTimer((TimerInfo *) &log_info->timer);
              UnlockSemaphoreInfo(log_info->event_semaphore);
              return(MagickFalse);
            }
          log_info->append=IsPathAccessible(filename);
          log_info->file=fopen_utf8(filename,"ab");
          filename=(char  *) RelinquishMagickMemory(filename);
          if (log_info->file == (FILE *) NULL)
            {
              UnlockSemaphoreInfo(log_info->event_semaphore);
              return(MagickFalse);
            }
          log_info->generation++;
          if (log_info->append == MagickFalse)
            (void) FormatLocaleFile(log_info->file,"<?xml version=\"1.0\" "
              "encoding=\"UTF-8\" standalone=\"yes\"?>\n");
          (void) FormatLocaleFile(log_info->file,"<log>\n");
        }
      (void) FormatLocaleFile(log_info->file,"  <event>%s</event>\n",text);
      (void) fflush(log_info->file);
    }
  if ((log_info->handler_mask & MethodHandler) != 0)
    {
      if (log_info->method != (MagickLogMethod) NULL)
        log_info->method(type,text);
    }
  if ((log_info->handler_mask & StdoutHandler) != 0)
    {
      (void) FormatLocaleFile(stdout,"%s\n",text);
      (void) fflush(stdout);
    }
  if ((log_info->handler_mask & StderrHandler) != 0)
    {
      (void) FormatLocaleFile(stderr,"%s\n",text);
      (void) fflush(stderr);
    }
  text=(char  *) RelinquishMagickMemory(text);
  (void) ContinueTimer((TimerInfo *) &log_info->timer);
  UnlockSemaphoreInfo(log_info->event_semaphore);
  return(MagickTrue);
}

MagickExport MagickBooleanType LogMagickEvent(const LogEventType type,
  const char *module,const char *function,const size_t line,
  const char *format,...)
{
  va_list
    operands;

  MagickBooleanType
    status;

  if (IsEventLogging() == MagickFalse)
    return(MagickFalse);
  va_start(operands,format);
  status=LogMagickEventList(type,module,function,line,format,operands);
  va_end(operands);
  return(status);
}

#if !MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e L o g H a n d l e r s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseLogHandlers() parses a string defining which handlers takes a log
%  message and exports them.
%
%  The format of the ParseLogHandlers method is:
%
%      LogHandlerType ParseLogHandlers(const char *handlers)
%
%  A description of each parameter follows:
%
%    o handlers: one or more handlers separated by commas.
%
*/
static LogHandlerType ParseLogHandlers(const char *handlers)
{
  LogHandlerType
    handler_mask;

  register const char
    *p;

  register ssize_t
    i;

  size_t
    length;

  handler_mask=NoHandler;
  for (p=handlers; p != (char *) NULL; p=strchr(p,','))
  {
    while ((*p != '\0') && ((isspace((int) ((unsigned char) *p)) != 0) ||
           (*p == ',')))
      p++;
    for (i=0; *LogHandlers[i].name != '\0'; i++)
    {
      length=strlen(LogHandlers[i].name);
      if (LocaleNCompare(p,LogHandlers[i].name,length) == 0)
        {
          handler_mask=(LogHandlerType) (handler_mask | LogHandlers[i].handler);
          break;
        }
    }
    if (*LogHandlers[i].name == '\0')
      return(UndefinedHandler);
  }
  return(handler_mask);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t L o g E v e n t M a s k                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetLogEventMask() accepts a list that determines which events to log.  All
%  other events are ignored.  By default, no debug is enabled.  This method
%  returns the previous log event mask.
%
%  The format of the SetLogEventMask method is:
%
%      LogEventType SetLogEventMask(const char *events)
%
%  A description of each parameter follows:
%
%    o events: log these events.
%
*/
MagickExport LogEventType SetLogEventMask(const char *events)
{
  ExceptionInfo
    *exception;

  LogInfo
    *log_info;

  ssize_t
    option;

  exception=AcquireExceptionInfo();
  log_info=(LogInfo *) GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  option=ParseCommandOption(MagickLogEventOptions,MagickTrue,events);
  LockSemaphoreInfo(log_semaphore);
  log_info=(LogInfo *) GetValueFromLinkedList(log_cache,0);
  log_info->event_mask=(LogEventType) option;
  if (option == -1)
    log_info->event_mask=UndefinedEvents;
  CheckEventLogging();
  UnlockSemaphoreInfo(log_semaphore);
  return(log_info->event_mask);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t L o g F o r m a t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetLogFormat() sets the format for the "human readable" log record.
%
%  The format of the LogMagickFormat method is:
%
%      SetLogFormat(const char *format)
%
%  A description of each parameter follows:
%
%    o format: the log record format.
%
*/
MagickExport void SetLogFormat(const char *format)
{
  LogInfo
    *log_info;

  ExceptionInfo
    *exception;

  exception=AcquireExceptionInfo();
  log_info=(LogInfo *) GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  LockSemaphoreInfo(log_semaphore);
  if (log_info->format != (char *) NULL)
    log_info->format=DestroyString(log_info->format);
  log_info->format=ConstantString(format);
  UnlockSemaphoreInfo(log_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t L o g M e t h o d                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetLogMethod() sets the method that will be called when an event is logged.
%
%  The format of the SetLogMethod method is:
%
%      void SetLogMethod(MagickLogMethod method)
%
%  A description of each parameter follows:
%
%    o method: pointer to a method that will be called when LogMagickEvent is
%      being called.
%
*/
MagickExport void SetLogMethod(MagickLogMethod method)
{
  ExceptionInfo
    *exception;

  LogInfo
    *log_info;

  exception=AcquireExceptionInfo();
  log_info=(LogInfo *) GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  LockSemaphoreInfo(log_semaphore);
  log_info=(LogInfo *) GetValueFromLinkedList(log_cache,0);
  log_info->handler_mask=(LogHandlerType) (log_info->handler_mask |
    MethodHandler);
  log_info->method=method;
  UnlockSemaphoreInfo(log_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t L o g N a m e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetLogName() sets the log name and returns it.
%
%  The format of the SetLogName method is:
%
%      const char *SetLogName(const char *name)
%
%  A description of each parameter follows:
%
%    o log_name: SetLogName() returns the current client name.
%
%    o name: Specifies the new client name.
%
*/
MagickExport const char *SetLogName(const char *name)
{
  if ((name != (char *) NULL) && (*name != '\0'))
    (void) CopyMagickString(log_name,name,MagickPathExtent);
  return(log_name);
}

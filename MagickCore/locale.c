/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  L       OOO    CCCC   AAA   L      EEEEE                   %
%                  L      O   O  C      A   A  L      E                       %
%                  L      O   O  C      AAAAA  L      EEE                     %
%                  L      O   O  C      A   A  L      E                       %
%                  LLLLL   OOO    CCCC  A   A  LLLLL  EEEEE                   %
%                                                                             %
%                                                                             %
%                      MagickCore Image Locale Methods                        %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image-private.h"
#include "MagickCore/linked-list.h"
#include "MagickCore/locale_.h"
#include "MagickCore/locale-private.h"
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/xml-tree.h"
#include "MagickCore/xml-tree-private.h"

/*
  Define declarations.
*/
#if defined(MAGICKCORE_HAVE_NEWLOCALE) || defined(MAGICKCORE_WINDOWS_SUPPORT)
#  define MAGICKCORE_LOCALE_SUPPORT
#endif
#define LocaleFilename  "locale.xml"

/*
  Static declarations.
*/
static const char
  *LocaleMap =
    "<?xml version=\"1.0\"?>"
    "<localemap>"
    "  <locale name=\"C\">"
    "    <Exception>"
    "     <Message name=\"\">"
    "     </Message>"
    "    </Exception>"
    "  </locale>"
    "</localemap>";

#ifdef __VMS
#define asciimap AsciiMap
#endif
#if !defined(MAGICKCORE_HAVE_STRCASECMP) || !defined(MAGICKCORE_HAVE_STRNCASECMP)
static const unsigned char
  AsciiMap[] =
  {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
    0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73,
    0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
    0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83,
    0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b,
    0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3,
    0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xe1, 0xe2, 0xe3, 0xe4, 0xc5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb,
    0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0xfa, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3,
    0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb,
    0xfc, 0xfd, 0xfe, 0xff,
  };
#endif

static SemaphoreInfo
  *locale_semaphore = (SemaphoreInfo *) NULL;

static SplayTreeInfo
  *locale_cache = (SplayTreeInfo *) NULL;

#if defined(MAGICKCORE_LOCALE_SUPPORT)
static volatile locale_t
  c_locale = (locale_t) NULL;
#endif

/*
  Forward declarations.
*/
static MagickBooleanType
  IsLocaleTreeInstantiated(ExceptionInfo *),
  LoadLocaleCache(SplayTreeInfo *,const char *,const char *,const char *,
    const size_t,ExceptionInfo *);

#if defined(MAGICKCORE_LOCALE_SUPPORT)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e C L o c a l e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCLocale() allocates the C locale object, or (locale_t) 0 with
%  errno set if it cannot be acquired.
%
%  The format of the AcquireCLocale method is:
%
%      locale_t AcquireCLocale(void)
%
*/
static locale_t AcquireCLocale(void)
{
#if defined(MAGICKCORE_HAVE_NEWLOCALE)
  if (c_locale == (locale_t) NULL)
    c_locale=newlocale(LC_ALL_MASK,"C",(locale_t) 0);
#elif defined(MAGICKCORE_WINDOWS_SUPPORT) && !defined(__MINGW32__)
  if (c_locale == (locale_t) NULL)
    c_locale=_create_locale(LC_ALL,"C");
#endif
  return(c_locale);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A c q u i r e L o c a l e S p l a y T r e e                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireLocaleSplayTree() caches one or more locale configurations which
%  provides a mapping between locale attributes and a locale tag.
%
%  The format of the AcquireLocaleSplayTree method is:
%
%      SplayTreeInfo *AcquireLocaleSplayTree(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file tag.
%
%    o locale: the actual locale.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void *DestroyLocaleNode(void *locale_info)
{
  register LocaleInfo
    *p;

  p=(LocaleInfo *) locale_info;
  if (p->path != (char *) NULL)
    p->path=DestroyString(p->path);
  if (p->tag != (char *) NULL)
    p->tag=DestroyString(p->tag);
  if (p->message != (char *) NULL)
    p->message=DestroyString(p->message);
  return(RelinquishMagickMemory(p));
}

static SplayTreeInfo *AcquireLocaleSplayTree(const char *filename,
  const char *locale,ExceptionInfo *exception)
{
  MagickStatusType
    status;

  SplayTreeInfo
    *cache;

  cache=NewSplayTree(CompareSplayTreeString,(void *(*)(void *)) NULL,
    DestroyLocaleNode);
  status=MagickTrue;
#if !defined(MAGICKCORE_ZERO_CONFIGURATION_SUPPORT)
  {
    const StringInfo
      *option;

    LinkedListInfo
      *options;

    options=GetLocaleOptions(filename,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
    while (option != (const StringInfo *) NULL)
    {
      status&=LoadLocaleCache(cache,(const char *)
        GetStringInfoDatum(option),GetStringInfoPath(option),locale,0,
        exception);
      option=(const StringInfo *) GetNextValueInLinkedList(options);
    }
    options=DestroyLocaleOptions(options);
    if (GetNumberOfNodesInSplayTree(cache) == 0)
      {
        options=GetLocaleOptions("english.xml",exception);
        option=(const StringInfo *) GetNextValueInLinkedList(options);
        while (option != (const StringInfo *) NULL)
        {
          status&=LoadLocaleCache(cache,(const char *)
            GetStringInfoDatum(option),GetStringInfoPath(option),locale,0,
            exception);
          option=(const StringInfo *) GetNextValueInLinkedList(options);
        }
        options=DestroyLocaleOptions(options);
      }
  }
#endif
  if (GetNumberOfNodesInSplayTree(cache) == 0)
    status&=LoadLocaleCache(cache,LocaleMap,"built-in",locale,0,
      exception);
  return(cache);
}

#if defined(MAGICKCORE_LOCALE_SUPPORT)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y C L o c a l e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyCLocale() releases the resources allocated for a locale object
%  returned by a call to the AcquireCLocale() method.
%
%  The format of the DestroyCLocale method is:
%
%      void DestroyCLocale(void)
%
*/
static void DestroyCLocale(void)
{
  if (c_locale != (locale_t) NULL)
    freelocale(c_locale);
  c_locale=(locale_t) NULL;
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y L o c a l e O p t i o n s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyLocaleOptions() releases memory associated with an locale
%  messages.
%
%  The format of the DestroyProfiles method is:
%
%      LinkedListInfo *DestroyLocaleOptions(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/

static void *DestroyOptions(void *message)
{
  return(DestroyStringInfo((StringInfo *) message));
}

MagickExport LinkedListInfo *DestroyLocaleOptions(LinkedListInfo *messages)
{
  assert(messages != (LinkedListInfo *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  return(DestroyLinkedList(messages,DestroyOptions));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  F o r m a t L o c a l e F i l e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatLocaleFile() prints formatted output of a variable argument list to a
%  file in the "C" locale.
%
%  The format of the FormatLocaleFile method is:
%
%      ssize_t FormatLocaleFile(FILE *file,const char *format,...)
%
%  A description of each parameter follows.
%
%   o file:  the file.
%
%   o format:  A file describing the format to use to write the remaining
%     arguments.
%
*/

MagickPrivate ssize_t FormatLocaleFileList(FILE *file,
  const char *magick_restrict format,va_list operands)
{
  ssize_t
    n;

#if defined(MAGICKCORE_LOCALE_SUPPORT) && defined(MAGICKCORE_HAVE_VFPRINTF_L)
  {
    locale_t
      locale;

    locale=AcquireCLocale();
    if (locale == (locale_t) NULL)
      n=(ssize_t) vfprintf(file,format,operands);
    else
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
      n=(ssize_t) vfprintf_l(file,format,locale,operands);
#else
      n=(ssize_t) vfprintf_l(file,locale,format,operands);
#endif
  }
#else
#if defined(MAGICKCORE_LOCALE_SUPPORT) && defined(MAGICKCORE_HAVE_USELOCALE)
  {
    locale_t
      locale,
      previous_locale;

    locale=AcquireCLocale();
    if (locale == (locale_t) NULL)
      n=(ssize_t) vfprintf(file,format,operands);
    else
      {
        previous_locale=uselocale(locale);
        n=(ssize_t) vfprintf(file,format,operands);
        uselocale(previous_locale);
      }
  }
#else
  n=(ssize_t) vfprintf(file,format,operands);
#endif
#endif
  return(n);
}

MagickExport ssize_t FormatLocaleFile(FILE *file,
  const char *magick_restrict format,...)
{
  ssize_t
    n;

  va_list
    operands;

  va_start(operands,format);
  n=FormatLocaleFileList(file,format,operands);
  va_end(operands);
  return(n);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  F o r m a t L o c a l e S t r i n g                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatLocaleString() prints formatted output of a variable argument list to
%  a string buffer in the "C" locale.
%
%  The format of the FormatLocaleString method is:
%
%      ssize_t FormatLocaleString(char *string,const size_t length,
%        const char *format,...)
%
%  A description of each parameter follows.
%
%   o string:  FormatLocaleString() returns the formatted string in this
%     character buffer.
%
%   o length: the maximum length of the string.
%
%   o format:  A string describing the format to use to write the remaining
%     arguments.
%
*/

MagickPrivate ssize_t FormatLocaleStringList(char *magick_restrict string,
  const size_t length,const char *magick_restrict format,va_list operands)
{
  ssize_t
    n;

#if defined(MAGICKCORE_LOCALE_SUPPORT) && defined(MAGICKCORE_HAVE_VSNPRINTF_L)
  {
    locale_t
      locale;

    locale=AcquireCLocale();
    if (locale == (locale_t) NULL)
      n=(ssize_t) vsnprintf(string,length,format,operands);
    else
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
      n=(ssize_t) vsnprintf_l(string,length,format,locale,operands);
#else
      n=(ssize_t) vsnprintf_l(string,length,locale,format,operands);
#endif
  }
#elif defined(MAGICKCORE_HAVE_VSNPRINTF)
#if defined(MAGICKCORE_LOCALE_SUPPORT) && defined(MAGICKCORE_HAVE_USELOCALE)
  {
    locale_t
      locale,
      previous_locale;

    locale=AcquireCLocale();
    if (locale == (locale_t) NULL)
      n=(ssize_t) vsnprintf(string,length,format,operands);
    else
      {
        previous_locale=uselocale(locale);
        n=(ssize_t) vsnprintf(string,length,format,operands);
        uselocale(previous_locale);
      }
  }
#else
  n=(ssize_t) vsnprintf(string,length,format,operands);
#endif
#else
  n=(ssize_t) vsprintf(string,format,operands);
#endif
  if (n < 0)
    string[length-1]='\0';
  return(n);
}

MagickExport ssize_t FormatLocaleString(char *magick_restrict string,
  const size_t length,const char *magick_restrict format,...)
{
  ssize_t
    n;

  va_list
    operands;

  va_start(operands,format);
  n=FormatLocaleStringList(string,length,format,operands);
  va_end(operands);
  return(n);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t L o c a l e I n f o _                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleInfo_() searches the locale list for the specified tag and if
%  found returns attributes for that element.
%
%  The format of the GetLocaleInfo method is:
%
%      const LocaleInfo *GetLocaleInfo_(const char *tag,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o tag: the locale tag.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const LocaleInfo *GetLocaleInfo_(const char *tag,
  ExceptionInfo *exception)
{
  const LocaleInfo
    *locale_info;

  assert(exception != (ExceptionInfo *) NULL);
  if (IsLocaleTreeInstantiated(exception) == MagickFalse)
    return((const LocaleInfo *) NULL);
  LockSemaphoreInfo(locale_semaphore);
  if ((tag == (const char *) NULL) || (LocaleCompare(tag,"*") == 0))
    {
      ResetSplayTreeIterator(locale_cache);
      locale_info=(const LocaleInfo *) GetNextValueInSplayTree(locale_cache);
      UnlockSemaphoreInfo(locale_semaphore);
      return(locale_info);
    }
  locale_info=(const LocaleInfo *) GetValueFromSplayTree(locale_cache,tag);
  UnlockSemaphoreInfo(locale_semaphore);
  return(locale_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o c a l e I n f o L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleInfoList() returns any locale messages that match the
%  specified pattern.
%
%  The format of the GetLocaleInfoList function is:
%
%      const LocaleInfo **GetLocaleInfoList(const char *pattern,
%        size_t *number_messages,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_messages:  This integer returns the number of locale messages in
%    the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int LocaleInfoCompare(const void *x,const void *y)
{
  const LocaleInfo
    **p,
    **q;

  p=(const LocaleInfo **) x,
  q=(const LocaleInfo **) y;
  if (LocaleCompare((*p)->path,(*q)->path) == 0)
    return(LocaleCompare((*p)->tag,(*q)->tag));
  return(LocaleCompare((*p)->path,(*q)->path));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport const LocaleInfo **GetLocaleInfoList(const char *pattern,
  size_t *number_messages,ExceptionInfo *exception)
{
  const LocaleInfo
    **messages;

  register const LocaleInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate locale list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_messages != (size_t *) NULL);
  *number_messages=0;
  p=GetLocaleInfo_("*",exception);
  if (p == (const LocaleInfo *) NULL)
    return((const LocaleInfo **) NULL);
  messages=(const LocaleInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfNodesInSplayTree(locale_cache)+1UL,sizeof(*messages));
  if (messages == (const LocaleInfo **) NULL)
    return((const LocaleInfo **) NULL);
  /*
    Generate locale list.
  */
  LockSemaphoreInfo(locale_semaphore);
  ResetSplayTreeIterator(locale_cache);
  p=(const LocaleInfo *) GetNextValueInSplayTree(locale_cache);
  for (i=0; p != (const LocaleInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->tag,pattern,MagickTrue) != MagickFalse))
      messages[i++]=p;
    p=(const LocaleInfo *) GetNextValueInSplayTree(locale_cache);
  }
  UnlockSemaphoreInfo(locale_semaphore);
  qsort((void *) messages,(size_t) i,sizeof(*messages),LocaleInfoCompare);
  messages[i]=(LocaleInfo *) NULL;
  *number_messages=(size_t) i;
  return(messages);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o c a l e L i s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleList() returns any locale messages that match the specified
%  pattern.
%
%  The format of the GetLocaleList function is:
%
%      char **GetLocaleList(const char *pattern,size_t *number_messages,
%        Exceptioninfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_messages:  This integer returns the number of messages in the
%      list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int LocaleTagCompare(const void *x,const void *y)
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

MagickExport char **GetLocaleList(const char *pattern,size_t *number_messages,
  ExceptionInfo *exception)
{
  char
    **messages;

  register const LocaleInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate locale list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_messages != (size_t *) NULL);
  *number_messages=0;
  p=GetLocaleInfo_("*",exception);
  if (p == (const LocaleInfo *) NULL)
    return((char **) NULL);
  messages=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfNodesInSplayTree(locale_cache)+1UL,sizeof(*messages));
  if (messages == (char **) NULL)
    return((char **) NULL);
  LockSemaphoreInfo(locale_semaphore);
  p=(const LocaleInfo *) GetNextValueInSplayTree(locale_cache);
  for (i=0; p != (const LocaleInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->tag,pattern,MagickTrue) != MagickFalse))
      messages[i++]=ConstantString(p->tag);
    p=(const LocaleInfo *) GetNextValueInSplayTree(locale_cache);
  }
  UnlockSemaphoreInfo(locale_semaphore);
  qsort((void *) messages,(size_t) i,sizeof(*messages),LocaleTagCompare);
  messages[i]=(char *) NULL;
  *number_messages=(size_t) i;
  return(messages);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o c a l e M e s s a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleMessage() returns a message in the current locale that matches the
%  supplied tag.
%
%  The format of the GetLocaleMessage method is:
%
%      const char *GetLocaleMessage(const char *tag)
%
%  A description of each parameter follows:
%
%    o tag: Return a message that matches this tag in the current locale.
%
*/
MagickExport const char *GetLocaleMessage(const char *tag)
{
  char
    name[MagickLocaleExtent];

  const LocaleInfo
    *locale_info;

  ExceptionInfo
    *exception;

  if ((tag == (const char *) NULL) || (*tag == '\0'))
    return(tag);
  exception=AcquireExceptionInfo();
  (void) FormatLocaleString(name,MagickLocaleExtent,"%s/",tag);
  locale_info=GetLocaleInfo_(name,exception);
  exception=DestroyExceptionInfo(exception);
  if (locale_info != (const LocaleInfo *) NULL)
    return(locale_info->message);
  return(tag);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t L o c a l e O p t i o n s                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleOptions() returns any Magick configuration messages associated
%  with the specified filename.
%
%  The format of the GetLocaleOptions method is:
%
%      LinkedListInfo *GetLocaleOptions(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the locale file tag.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport LinkedListInfo *GetLocaleOptions(const char *filename,
  ExceptionInfo *exception)
{
  char
    path[MagickPathExtent];

  const char
    *element;

  LinkedListInfo
    *messages,
    *paths;

  StringInfo
    *xml;

  assert(filename != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  assert(exception != (ExceptionInfo *) NULL);
  (void) CopyMagickString(path,filename,MagickPathExtent);
  /*
    Load XML from configuration files to linked-list.
  */
  messages=NewLinkedList(0);
  paths=GetConfigurePaths(filename,exception);
  if (paths != (LinkedListInfo *) NULL)
    {
      ResetLinkedListIterator(paths);
      element=(const char *) GetNextValueInLinkedList(paths);
      while (element != (const char *) NULL)
      {
        (void) FormatLocaleString(path,MagickPathExtent,"%s%s",element,
          filename);
        (void) LogMagickEvent(LocaleEvent,GetMagickModule(),
          "Searching for locale file: \"%s\"",path);
        xml=ConfigureFileToStringInfo(path);
        if (xml != (StringInfo *) NULL)
          (void) AppendValueToLinkedList(messages,xml);
        element=(const char *) GetNextValueInLinkedList(paths);
      }
      paths=DestroyLinkedList(paths,RelinquishMagickMemory);
    }
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  {
    char
      *blob;

    blob=(char *) NTResourceToBlob(filename);
    if (blob != (char *) NULL)
      {
        xml=AcquireStringInfo(0);
        SetStringInfoLength(xml,strlen(blob)+1);
        SetStringInfoDatum(xml,(const unsigned char *) blob);
        blob=(char *) RelinquishMagickMemory(blob);
        SetStringInfoPath(xml,filename);
        (void) AppendValueToLinkedList(messages,xml);
      }
  }
#endif
  ResetLinkedListIterator(messages);
  return(messages);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o c a l e V a l u e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleValue() returns the message associated with the locale info.
%
%  The format of the GetLocaleValue method is:
%
%      const char *GetLocaleValue(const LocaleInfo *locale_info)
%
%  A description of each parameter follows:
%
%    o locale_info:  The locale info.
%
*/
MagickExport const char *GetLocaleValue(const LocaleInfo *locale_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(locale_info != (LocaleInfo *) NULL);
  assert(locale_info->signature == MagickCoreSignature);
  return(locale_info->message);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s L o c a l e T r e e I n s t a n t i a t e d                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsLocaleTreeInstantiated() determines if the locale tree is instantiated.
%  If not, it instantiates the tree and returns it.
%
%  The format of the IsLocaleInstantiated method is:
%
%      MagickBooleanType IsLocaleTreeInstantiated(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType IsLocaleTreeInstantiated(ExceptionInfo *exception)
{
  if (locale_cache == (SplayTreeInfo *) NULL)
    {
      if (locale_semaphore == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&locale_semaphore);
      LockSemaphoreInfo(locale_semaphore);
      if (locale_cache == (SplayTreeInfo *) NULL)
        {
          char
            *locale;

          register const char
            *p;

          locale=(char *) NULL;
          p=setlocale(LC_CTYPE,(const char *) NULL);
          if (p != (const char *) NULL)
            locale=ConstantString(p);
          if (locale == (char *) NULL)
            locale=GetEnvironmentValue("LC_ALL");
          if (locale == (char *) NULL)
            locale=GetEnvironmentValue("LC_MESSAGES");
          if (locale == (char *) NULL)
            locale=GetEnvironmentValue("LC_CTYPE");
          if (locale == (char *) NULL)
            locale=GetEnvironmentValue("LANG");
          if (locale == (char *) NULL)
            locale=ConstantString("C");
          locale_cache=AcquireLocaleSplayTree(LocaleFilename,locale,exception);
          locale=DestroyString(locale);
        }
      UnlockSemaphoreInfo(locale_semaphore);
    }
  return(locale_cache != (SplayTreeInfo *) NULL ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n t e r p r e t L o c a l e V a l u e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpretLocaleValue() interprets the string as a floating point number in
%  the "C" locale and returns its value as a double. If sentinal is not a null
%  pointer, the method also sets the value pointed by sentinal to point to the
%  first character after the number.
%
%  The format of the InterpretLocaleValue method is:
%
%      double InterpretLocaleValue(const char *value,char **sentinal)
%
%  A description of each parameter follows:
%
%    o value: the string value.
%
%    o sentinal:  if sentinal is not NULL, a pointer to the character after the
%      last character used in the conversion is stored in the location
%      referenced by sentinal.
%
*/
MagickExport double InterpretLocaleValue(const char *magick_restrict string,
  char **magick_restrict sentinal)
{
  char
    *q;

  double
    value;

  if ((*string == '0') && ((string[1] | 0x20)=='x'))
    value=(double) strtoul(string,&q,16);
  else
    {
#if defined(MAGICKCORE_LOCALE_SUPPORT) && defined(MAGICKCORE_HAVE_STRTOD_L)
      locale_t
        locale;

      locale=AcquireCLocale();
      if (locale == (locale_t) NULL)
        value=strtod(string,&q);
      else
        value=strtod_l(string,&q,locale);
#else
      value=strtod(string,&q);
#endif
    }
  if (sentinal != (char **) NULL)
    *sentinal=q;
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t L o c a l e I n f o                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListLocaleInfo() lists the locale info to a file.
%
%  The format of the ListLocaleInfo method is:
%
%      MagickBooleanType ListLocaleInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListLocaleInfo(FILE *file,
  ExceptionInfo *exception)
{
  const char
    *path;

  const LocaleInfo
    **locale_info;

  register ssize_t
    i;

  size_t
    number_messages;

  if (file == (const FILE *) NULL)
    file=stdout;
  number_messages=0;
  locale_info=GetLocaleInfoList("*",&number_messages,exception);
  if (locale_info == (const LocaleInfo **) NULL)
    return(MagickFalse);
  path=(const char *) NULL;
  for (i=0; i < (ssize_t) number_messages; i++)
  {
    if (locale_info[i]->stealth != MagickFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (LocaleCompare(path,locale_info[i]->path) != 0))
      {
        if (locale_info[i]->path != (char *) NULL)
          (void) FormatLocaleFile(file,"\nPath: %s\n\n",locale_info[i]->path);
        (void) FormatLocaleFile(file,"Tag/Message\n");
        (void) FormatLocaleFile(file,
          "-------------------------------------------------"
          "------------------------------\n");
      }
    path=locale_info[i]->path;
    (void) FormatLocaleFile(file,"%s\n",locale_info[i]->tag);
    if (locale_info[i]->message != (char *) NULL)
      (void) FormatLocaleFile(file,"  %s",locale_info[i]->message);
    (void) FormatLocaleFile(file,"\n");
  }
  (void) fflush(file);
  locale_info=(const LocaleInfo **)
    RelinquishMagickMemory((void *) locale_info);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d L o c a l e C a c h e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadLocaleCache() loads the locale configurations which provides a mapping
%  between locale attributes and a locale name.
%
%  The format of the LoadLocaleCache method is:
%
%      MagickBooleanType LoadLocaleCache(SplayTreeInfo *cache,const char *xml,
%        const char *filename,const size_t depth,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The locale list in XML format.
%
%    o filename:  The locale list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void ChopLocaleComponents(char *path,const size_t components)
{
  register char
    *p;

  ssize_t
    count;

  if (*path == '\0')
    return;
  p=path+strlen(path)-1;
  if (*p == '/')
    *p='\0';
  for (count=0; (count < (ssize_t) components) && (p > path); p--)
    if (*p == '/')
      {
        *p='\0';
        count++;
      }
  if (count < (ssize_t) components)
    *path='\0';
}

static void LocaleFatalErrorHandler(
  const ExceptionType magick_unused(severity),
  const char *reason,const char *description)
{
  magick_unreferenced(severity);

  if (reason == (char *) NULL)
    return;
  (void) FormatLocaleFile(stderr,"%s: %s",GetClientName(),reason);
  if (description != (char *) NULL)
    (void) FormatLocaleFile(stderr," (%s)",description);
  (void) FormatLocaleFile(stderr,".\n");
  (void) fflush(stderr);
  exit(1);
}

static MagickBooleanType LoadLocaleCache(SplayTreeInfo *cache,const char *xml,
  const char *filename,const char *locale,const size_t depth,ExceptionInfo *exception)
{
  char
    keyword[MagickLocaleExtent],
    message[MagickLocaleExtent],
    tag[MagickLocaleExtent],
    *token;

  const char
    *q;

  FatalErrorHandler
    fatal_handler;

  LocaleInfo
    *locale_info;

  MagickStatusType
    status;

  register char
    *p;

  size_t
    extent;

  /*
    Read the locale configure file.
  */
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading locale configure file \"%s\" ...",filename);
  if (xml == (const char *) NULL)
    return(MagickFalse);
  status=MagickTrue;
  locale_info=(LocaleInfo *) NULL;
  *tag='\0';
  *message='\0';
  *keyword='\0';
  fatal_handler=SetFatalErrorHandler(LocaleFatalErrorHandler);
  token=AcquireString(xml);
  extent=strlen(token)+MagickPathExtent;
  for (q=(char *) xml; *q != '\0'; )
  {
    /*
      Interpret XML.
    */
    GetNextToken(q,&q,extent,token);
    if (*token == '\0')
      break;
    (void) CopyMagickString(keyword,token,MagickLocaleExtent);
    if (LocaleNCompare(keyword,"<!DOCTYPE",9) == 0)
      {
        /*
          Doctype element.
        */
        while ((LocaleNCompare(q,"]>",2) != 0) && (*q != '\0'))
        {
          GetNextToken(q,&q,extent,token);
          while (isspace((int) ((unsigned char) *q)) != 0)
            q++;
        }
        continue;
      }
    if (LocaleNCompare(keyword,"<!--",4) == 0)
      {
        /*
          Comment element.
        */
        while ((LocaleNCompare(q,"->",2) != 0) && (*q != '\0'))
        {
          GetNextToken(q,&q,extent,token);
          while (isspace((int) ((unsigned char) *q)) != 0)
            q++;
        }
        continue;
      }
    if (LocaleCompare(keyword,"<include") == 0)
      {
        /*
          Include element.
        */
        while (((*token != '/') && (*(token+1) != '>')) && (*q != '\0'))
        {
          (void) CopyMagickString(keyword,token,MagickLocaleExtent);
          GetNextToken(q,&q,extent,token);
          if (*token != '=')
            continue;
          GetNextToken(q,&q,extent,token);
          if (LocaleCompare(keyword,"locale") == 0)
            {
              if (LocaleCompare(locale,token) != 0)
                break;
              continue;
            }
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

                  *path='\0';
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
                      status&=LoadLocaleCache(cache,file_xml,path,locale,
                        depth+1,exception);
                      file_xml=DestroyString(file_xml);
                    }
                }
            }
        }
        continue;
      }
    if (LocaleCompare(keyword,"<locale") == 0)
      {
        /*
          Locale element.
        */
        while ((*token != '>') && (*q != '\0'))
        {
          (void) CopyMagickString(keyword,token,MagickLocaleExtent);
          GetNextToken(q,&q,extent,token);
          if (*token != '=')
            continue;
          GetNextToken(q,&q,extent,token);
        }
        continue;
      }
    if (LocaleCompare(keyword,"</locale>") == 0)
      {
        ChopLocaleComponents(tag,1);
        (void) ConcatenateMagickString(tag,"/",MagickLocaleExtent);
        continue;
      }
    if (LocaleCompare(keyword,"<localemap>") == 0)
      continue;
    if (LocaleCompare(keyword,"</localemap>") == 0)
      continue;
    if (LocaleCompare(keyword,"<message") == 0)
      {
        /*
          Message element.
        */
        while ((*token != '>') && (*q != '\0'))
        {
          (void) CopyMagickString(keyword,token,MagickLocaleExtent);
          GetNextToken(q,&q,extent,token);
          if (*token != '=')
            continue;
          GetNextToken(q,&q,extent,token);
          if (LocaleCompare(keyword,"name") == 0)
            {
              (void) ConcatenateMagickString(tag,token,MagickLocaleExtent);
              (void) ConcatenateMagickString(tag,"/",MagickLocaleExtent);
            }
        }
        for (p=(char *) q; (*q != '<') && (*q != '\0'); q++) ;
        while (isspace((int) ((unsigned char) *p)) != 0)
          p++;
        q--;
        while ((isspace((int) ((unsigned char) *q)) != 0) && (q > p))
          q--;
        (void) CopyMagickString(message,p,MagickMin((size_t) (q-p+2),
          MagickLocaleExtent));
        locale_info=(LocaleInfo *) AcquireCriticalMemory(sizeof(*locale_info));
        (void) memset(locale_info,0,sizeof(*locale_info));
        locale_info->path=ConstantString(filename);
        locale_info->tag=ConstantString(tag);
        locale_info->message=ConstantString(message);
        locale_info->signature=MagickCoreSignature;
        status=AddValueToSplayTree(cache,locale_info->tag,locale_info);
        if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            locale_info->tag);
        (void) ConcatenateMagickString(tag,message,MagickLocaleExtent);
        (void) ConcatenateMagickString(tag,"\n",MagickLocaleExtent);
        q++;
        continue;
      }
    if (LocaleCompare(keyword,"</message>") == 0)
      {
        ChopLocaleComponents(tag,2);
        (void) ConcatenateMagickString(tag,"/",MagickLocaleExtent);
        continue;
      }
    if (*keyword == '<')
      {
        /*
          Subpath element.
        */
        if (*(keyword+1) == '?')
          continue;
        if (*(keyword+1) == '/')
          {
            ChopLocaleComponents(tag,1);
            if (*tag != '\0')
              (void) ConcatenateMagickString(tag,"/",MagickLocaleExtent);
            continue;
          }
        token[strlen(token)-1]='\0';
        (void) CopyMagickString(token,token+1,MagickLocaleExtent);
        (void) ConcatenateMagickString(tag,token,MagickLocaleExtent);
        (void) ConcatenateMagickString(tag,"/",MagickLocaleExtent);
        continue;
      }
    GetNextToken(q,(const char **) NULL,extent,token);
    if (*token != '=')
      continue;
  }
  token=(char *) RelinquishMagickMemory(token);
  (void) SetFatalErrorHandler(fatal_handler);
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o c a l e C o m p a r e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleCompare() performs a case-insensitive comparison of two strings
%  byte-by-byte, according to the ordering of the current locale encoding.
%  LocaleCompare returns an integer greater than, equal to, or less than 0,
%  if the string pointed to by p is greater than, equal to, or less than the
%  string pointed to by q respectively.  The sign of a non-zero return value
%  is determined by the sign of the difference between the values of the first
%  pair of bytes that differ in the strings being compared.
%
%  The format of the LocaleCompare method is:
%
%      int LocaleCompare(const char *p,const char *q)
%
%  A description of each parameter follows:
%
%    o p: A pointer to a character string.
%
%    o q: A pointer to a character string to compare to p.
%
*/
MagickExport int LocaleCompare(const char *p,const char *q)
{
  if (p == (char *) NULL)
    {
      if (q == (char *) NULL)
        return(0);
      return(-1);
    }
  if (q == (char *) NULL)
    return(1);
#if defined(MAGICKCORE_HAVE_STRCASECMP)
  return(strcasecmp(p,q));
#else
  {
    register int
      c,
      d;

    for ( ; ; )
    {
      c=(int) *((unsigned char *) p);
      d=(int) *((unsigned char *) q);
      if ((c == 0) || (AsciiMap[c] != AsciiMap[d]))
        break;
      p++;
      q++;
    }
    return(AsciiMap[c]-(int) AsciiMap[d]);
  }
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o c a l e L o w e r                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleLower() transforms all of the characters in the supplied
%  null-terminated string, changing all uppercase letters to lowercase.
%
%  The format of the LocaleLower method is:
%
%      void LocaleLower(char *string)
%
%  A description of each parameter follows:
%
%    o string: A pointer to the string to convert to lower-case Locale.
%
*/
MagickExport void LocaleLower(char *string)
{
  register char
    *q;

  assert(string != (char *) NULL);
  for (q=string; *q != '\0'; q++)
    *q=(char) LocaleLowercase((int) *q);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o c a l e L o w e r c a s e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleLowercase() convert to lowercase.
%
%  The format of the LocaleLowercase method is:
%
%      void LocaleLowercase(const int c)
%
%  A description of each parameter follows:
%
%    o If c is a uppercase letter, return its lowercase equivalent.
%
*/
MagickExport int LocaleLowercase(const int c)
{
  if (c < 0)
    return(c);
#if defined(MAGICKCORE_LOCALE_SUPPORT)
  if (c_locale != (locale_t) NULL)
    return(tolower_l((int) ((unsigned char) c),c_locale));
#endif
  return(tolower((int) ((unsigned char) c)));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o c a l e N C o m p a r e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleNCompare() performs a case-insensitive comparison of two strings
%  byte-by-byte, according to the ordering of the current locale encoding.
%
%  LocaleNCompare returns an integer greater than, equal to, or less than 0,
%  if the string pointed to by p is greater than, equal to, or less than the
%  string pointed to by q respectively.  The sign of a non-zero return value
%  is determined by the sign of the difference between the values of the first
%  pair of bytes that differ in the strings being compared.
%
%  The LocaleNCompare method makes the same comparison as LocaleCompare but
%  looks at a maximum of n bytes.  Bytes following a null byte are not
%  compared.
%
%  The format of the LocaleNCompare method is:
%
%      int LocaleNCompare(const char *p,const char *q,const size_t n)
%
%  A description of each parameter follows:
%
%    o p: A pointer to a character string.
%
%    o q: A pointer to a character string to compare to p.
%
%    o length: the number of characters to compare in strings p and q.
%
*/
MagickExport int LocaleNCompare(const char *p,const char *q,const size_t length)
{
  if (p == (char *) NULL)
    {
      if (q == (char *) NULL)
        return(0);
      return(-1);
    }
  if (q == (char *) NULL)
    return(1);
#if defined(MAGICKCORE_HAVE_STRNCASECMP)
  return(strncasecmp(p,q,length));
#else
  {
    register int
      c,
      d;

    register size_t
      i;

    for (i=length; i != 0; i--)
    {
      c=(int) *((unsigned char *) p);
      d=(int) *((unsigned char *) q);
      if (AsciiMap[c] != AsciiMap[d])
        return(AsciiMap[c]-(int) AsciiMap[d]);
      if (c == 0)
        return(0);
      p++;
      q++;
    }
    return(0);
  }
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o c a l e U p p e r                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleUpper() transforms all of the characters in the supplied
%  null-terminated string, changing all lowercase letters to uppercase.
%
%  The format of the LocaleUpper method is:
%
%      void LocaleUpper(char *string)
%
%  A description of each parameter follows:
%
%    o string: A pointer to the string to convert to upper-case Locale.
%
*/
MagickExport void LocaleUpper(char *string)
{
  register char
    *q;

  assert(string != (char *) NULL);
  for (q=string; *q != '\0'; q++)
    *q=(char) LocaleUppercase((int) *q);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o c a l e U p p e r c a s e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleUppercase() convert to uppercase.
%
%  The format of the LocaleUppercase method is:
%
%      void LocaleUppercase(const int c)
%
%  A description of each parameter follows:
%
%    o If c is a lowercase letter, return its uppercase equivalent.
%
*/
MagickExport int LocaleUppercase(const int c)
{
  if (c < 0)
    return(c);
#if defined(MAGICKCORE_LOCALE_SUPPORT)
  if (c_locale != (locale_t) NULL)
    return(toupper_l((int) ((unsigned char) c),c_locale));
#endif
  return(toupper((int) ((unsigned char) c)));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o c a l e C o m p o n e n t G e n e s i s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleComponentGenesis() instantiates the locale component.
%
%  The format of the LocaleComponentGenesis method is:
%
%      MagickBooleanType LocaleComponentGenesis(void)
%
*/
MagickPrivate MagickBooleanType LocaleComponentGenesis(void)
{
  if (locale_semaphore == (SemaphoreInfo *) NULL)
    locale_semaphore=AcquireSemaphoreInfo();
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o c a l e C o m p o n e n t T e r m i n u s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleComponentTerminus() destroys the locale component.
%
%  The format of the LocaleComponentTerminus method is:
%
%      LocaleComponentTerminus(void)
%
*/
MagickPrivate void LocaleComponentTerminus(void)
{
  if (locale_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&locale_semaphore);
  LockSemaphoreInfo(locale_semaphore);
  if (locale_cache != (SplayTreeInfo *) NULL)
    locale_cache=DestroySplayTree(locale_cache);
#if defined(MAGICKCORE_LOCALE_SUPPORT)
  DestroyCLocale();
#endif
  UnlockSemaphoreInfo(locale_semaphore);
  RelinquishSemaphoreInfo(&locale_semaphore);
}

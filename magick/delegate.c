/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%           DDDD   EEEEE  L      EEEEE   GGGG   AAA   TTTTT  EEEEE            %
%           D   D  E      L      E      G      A   A    T    E                %
%           D   D  EEE    L      EEE    G  GG  AAAAA    T    EEE              %
%           D   D  E      L      E      G   G  A   A    T    E                %
%           DDDD   EEEEE  LLLLL  EEEEE   GGG   A   A    T    EEEEE            %
%                                                                             %
%                                                                             %
%             MagickCore Methods to Read/Write/Invoke Delegates               %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               October 1998                                  %
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
%  The Delegates methods associate a set of commands with a particular
%  image format.  ImageMagick uses delegates for formats it does not handle
%  directly.
%
%  Thanks to Bob Friesenhahn for the initial inspiration and design of the
%  delegates methods.
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/client.h"
#include "magick/configure.h"
#include "magick/constitute.h"
#include "magick/delegate.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/hashmap.h"
#include "magick/list.h"
#include "magick/memory_.h"
#include "magick/policy.h"
#include "magick/resource_.h"
#include "magick/semaphore.h"
#include "magick/string_.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/utility-private.h"
#include "magick/xml-tree.h"

/*
  Define declarations.
*/
#define DelegateFilename  "delegates.xml"

/*
  Declare delegate map.
*/
static const char
  *DelegateMap = (const char *)
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<delegatemap>"
    "  <delegate decode=\"autotrace\" stealth=\"True\" command=\"&quot;autotrace&quot; -output-format svg -output-file &quot;%o&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"avi:decode\" stealth=\"True\" command=\"&quot;mplayer&quot; &quot;%i&quot; -really-quiet -ao null -vo png:z=3\"/>"
    "  <delegate decode=\"browse\" stealth=\"True\" spawn=\"True\" command=\"&quot;xdg-open&quot; http://www.imagemagick.org/; rm &quot;%i&quot;\"/>"
    "  <delegate decode=\"cgm\" thread-support=\"False\" command=\"&quot;ralcgm&quot; -d ps -oC &lt; &quot;%i&quot; &gt; &quot;%o&quot; 2&gt; &quot;%u&quot;\"/>"
    "  <delegate decode=\"dng:decode\" command=\"&quot;/usr/bin/ufraw-batch&quot; --silent --wb=camera --black-point=auto --exposure=auto --create-id=also --out-type=ppm16 &quot;--output=%u.pnm&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"edit\" stealth=\"True\" command=\"&quot;xterm&quot; -title &quot;Edit Image Comment&quot; -e vi &quot;%o&quot;\"/>"
    "  <delegate decode=\"eps\" encode=\"pdf\" mode=\"bi\" command=\"&quot;gs&quot; -q -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pdfwrite&quot; &quot;-sOutputFile=%o&quot; &quot;-f%i&quot;\"/>"
    "  <delegate decode=\"eps\" encode=\"ps\" mode=\"bi\" command=\"&quot;gs&quot; -q -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pswrite&quot; &quot;-sOutputFile=%o&quot; &quot;-f%i&quot;\"/>"
    "  <delegate decode=\"fig\" command=\"&quot;fig2dev&quot; -L ps &quot;%i&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"gplt\" command=\"&quot;echo&quot; &quot;set size 1.25,0.62     set terminal postscript portrait color solid; set output &quot;%o&quot;; load &quot;%i&quot;&quot; &gt; &quot;%u&quot;;&quot;gnuplot&quot; &quot;%u&quot;\"/>"
    "  <delegate decode=\"hpg\" command=\"&quot;hp2xx&quot; -q -m eps -f `basename &quot;%o&quot;` &quot;%i&quot;     mv -f `basename &quot;%o&quot;` &quot;%o&quot;\"/>"
    "  <delegate decode=\"hpgl\" command=\"&quot;hp2xx&quot; -q -m eps -f `basename &quot;%o&quot;` &quot;%i&quot;     mv -f `basename &quot;%o&quot;` &quot;%o&quot;\"/>"
    "  <delegate decode=\"htm\" command=\"&quot;html2ps&quot; -U -o &quot;%o&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"html\" command=\"&quot;html2ps&quot; -U -o &quot;%o&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"https\" command=\"&quot;wget&quot; -q -O &quot;%o&quot; &quot;https:%M&quot;\"/>"
    "  <delegate decode=\"ilbm\" command=\"&quot;ilbmtoppm&quot; &quot;%i&quot; &gt; &quot;%o&quot;\"/>"
    "  <delegate decode=\"man\" command=\"&quot;groff&quot; -man -Tps &quot;%i&quot; &gt; &quot;%o&quot;\"/>"
    "  <delegate decode=\"mpeg:decode\" stealth=\"True\" command=\"&quot;ffmpeg&quot; -v -1 -vframes %S -i &quot;%i&quot; -vcodec pam -an -f rawvideo -y &quot;%u.pam&quot; 2&gt; &quot;%Z&quot;\"/>"
    "  <delegate decode=\"null\" encode=\"mpeg:encode\" stealth=\"True\" command=\"&quot;ffmpeg&quot; -v -1 -mbd rd -trellis 2 -cmp 2 -subcmp 2 -g 300 -i &quot;%M%%d.jpg&quot; &quot;%u.%m&quot; 2&gt; &quot;%Z&quot;\"/>"
    "  <delegate decode=\"pcl:color\" stealth=\"True\" command=\"&quot;pcl6&quot; -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=ppmraw&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;%s&quot;\"/>"
    "  <delegate decode=\"pcl:cmyk\" stealth=\"True\" command=\"&quot;pcl6&quot; -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=bmpsep8&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;%s&quot;\"/>"
    "  <delegate decode=\"pcl:mono\" stealth=\"True\" command=\"&quot;pcl6&quot; -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pbmraw&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;%s&quot;\"/>"
    "  <delegate decode=\"pdf\" encode=\"eps\" mode=\"bi\" command=\"&quot;gs&quot; -q -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=epswrite&quot; &quot;-sOutputFile=%o&quot; &quot;-f%i&quot;\"/>"
    "  <delegate decode=\"pdf\" encode=\"ps\" mode=\"bi\" command=\"&quot;gs&quot; -q -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pswrite&quot; &quot;-sOutputFile=%o&quot; &quot;-f%i&quot;\"/>"
    "  <delegate decode=\"pnm\" encode=\"ilbm\" mode=\"encode\" command=\"&quot;ppmtoilbm&quot; -24if &quot;%i&quot; &gt; &quot;%o&quot;\"/>"
    "  <delegate decode=\"pnm\" encode=\"launch\" mode=\"encode\" command=\"&quot;gimp&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"pov\" command=\"&quot;povray&quot; &quot;+i&quot;%i&quot;&quot; -D0 +o&quot;%o&quot; +fn%q +w%w +h%h +a -q9 -kfi&quot;%s&quot; -kff&quot;%n&quot;     &quot;convert&quot; -concatenate &quot;%o*.png&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"ps\" encode=\"eps\" mode=\"bi\" command=\"&quot;gs&quot; -q -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=epswrite&quot; &quot;-sOutputFile=%o&quot; &quot;-f%i&quot;\"/>"
    "  <delegate decode=\"ps\" encode=\"pdf\" mode=\"bi\" command=\"&quot;gs&quot; -q -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pdfwrite&quot; &quot;-sOutputFile=%o&quot; &quot;-f%i&quot;\"/>"
    "  <delegate decode=\"ps\" encode=\"print\" mode=\"encode\" command=\"lpr &quot;%i&quot;\"/>"
    "  <delegate decode=\"ps:alpha\" stealth=\"True\" command=\"&quot;gs&quot; -q -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pngalpha&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;-f%s&quot; &quot;-f%s&quot;\"/>"
    "  <delegate decode=\"ps:bbox\" stealth=\"True\" command=\"&quot;gs&quot; -q -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=bbox&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;-f%s&quot; &quot;-f%s&quot;\"/>"
    "  <delegate decode=\"ps:cmyk\" stealth=\"True\" command=\"&quot;gs&quot; -q -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=bmpsep8&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;-f%s&quot; &quot;-f%s&quot;\"/>"
    "  <delegate decode=\"ps:color\" stealth=\"True\" command=\"&quot;gs&quot; -q -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pnmraw&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;-f%s&quot; &quot;-f%s&quot;\"/>"
    "  <delegate decode=\"ps:mono\" stealth=\"True\" command=\"&quot;gs&quot; -q -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pnmraw&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;-f%s&quot; &quot;-f%s&quot;\"/>"
    "  <delegate decode=\"rgba\" encode=\"rle\" mode=\"encode\" command=\"&quot;rawtorle&quot; -o &quot;%o&quot; -v &quot;%i&quot;\"/>"
    "  <delegate decode=\"scan\" command=\"&quot;scanimage&quot; -d &quot;%i&quot; &gt; &quot;%o&quot;\"/>"
    "  <delegate encode=\"show\" spawn=\"True\" command=\"&quot;display&quot; -immutable -delay 0 -window-group %g -title &quot;%l of %f&quot; &quot;temporary:%i&quot;\"/>"
    "  <delegate decode=\"shtml\" command=\"&quot;html2ps&quot; -U -o &quot;%o&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"svg\" command=\"&quot;rsvg&quot; &quot;%i&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"txt\" encode=\"ps\" mode=\"bi\" command=\"&quot;enscript&quot; -o &quot;%o&quot; &quot;%i&quot;\"/>"
    "  <delegate encode=\"win\" stealth=\"True\" spawn=\"True\" command=\"&quot;display&quot; -immutable -delay 0 -window-group %g -title &quot;%l of %f&quot; &quot;temporary:%i&quot;\"/>"
    "  <delegate decode=\"wmf\" command=\"&quot;wmf2eps&quot; -o &quot;%o&quot; &quot;%i&quot;\"/>"
    "</delegatemap>";

/*
  Global declaractions.
*/
static LinkedListInfo
  *delegate_list = (LinkedListInfo *) NULL;

static SemaphoreInfo
  *delegate_semaphore = (SemaphoreInfo *) NULL;

static volatile MagickBooleanType
  instantiate_delegate = MagickFalse;

/*
  Forward declaractions.
*/
static MagickBooleanType
  InitializeDelegateList(ExceptionInfo *),
  LoadDelegateLists(const char *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e l e g a t e C o m p o n e n t T e r m i n u s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DelegateComponentGenesis() instantiates the delegate component.
%
%  The format of the DelegateComponentGenesis method is:
%
%      MagickBooleanType DelegateComponentGenesis(void)
%
*/
MagickExport MagickBooleanType DelegateComponentGenesis(void)
{
  AcquireSemaphoreInfo(&delegate_semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e g a t e C o m p o n e n t T e r m i n u s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DelegateComponentTerminus() destroys the delegate component.
%
%  The format of the DelegateComponentTerminus method is:
%
%      DelegateComponentTerminus(void)
%
*/

static void *DestroyDelegate(void *delegate_info)
{
  register DelegateInfo
    *p;

  p=(DelegateInfo *) delegate_info;
  if (p->path != (char *) NULL)
    p->path=DestroyString(p->path);
  if (p->decode != (char *) NULL)
    p->decode=DestroyString(p->decode);
  if (p->encode != (char *) NULL)
    p->encode=DestroyString(p->encode);
  if (p->commands != (char *) NULL)
    p->commands=DestroyString(p->commands);
  p=(DelegateInfo *) RelinquishMagickMemory(p);
  return((void *) NULL);
}


MagickExport void DelegateComponentTerminus(void)
{
  if (delegate_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&delegate_semaphore);
  LockSemaphoreInfo(delegate_semaphore);
  if (delegate_list != (LinkedListInfo *) NULL)
    delegate_list=DestroyLinkedList(delegate_list,DestroyDelegate);
  instantiate_delegate=MagickFalse;
  UnlockSemaphoreInfo(delegate_semaphore);
  DestroySemaphoreInfo(&delegate_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t D e l e g a t e C o m m a n d                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDelegateCommand() replaces any embedded formatting characters with the
%  appropriate image attribute and returns the resulting command.
%
%  The format of the GetDelegateCommand method is:
%
%      char *GetDelegateCommand(const ImageInfo *image_info,Image *image,
%        const char *decode,const char *encode,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o command: Method GetDelegateCommand returns the command associated
%      with specified delegate tag.
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o decode: Specifies the decode delegate we are searching for as a
%      character string.
%
%    o encode: Specifies the encode delegate we are searching for as a
%      character string.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport char *GetDelegateCommand(const ImageInfo *image_info,Image *image,
  const char *decode,const char *encode,ExceptionInfo *exception)
{
  char
    *command,
    **commands;

  const DelegateInfo
    *delegate_info;

  register ssize_t
    i;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  delegate_info=GetDelegateInfo(decode,encode,exception);
  if (delegate_info == (const DelegateInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),DelegateError,
        "NoTagFound","`%s'",decode ? decode : encode);
      return((char *) NULL);
    }
  commands=StringToList(delegate_info->commands);
  if (commands == (char **) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        decode ? decode : encode);
      return((char *) NULL);
    }
  command=InterpretImageProperties(image_info,image,commands[0]);
  if (command == (char *) NULL)
    (void) ThrowMagickException(exception,GetMagickModule(),ResourceLimitError,
      "MemoryAllocationFailed","`%s'",commands[0]);
  /*
    Relinquish resources.
  */
  for (i=0; commands[i] != (char *) NULL; i++)
    commands[i]=DestroyString(commands[i]);
  commands=(char **) RelinquishMagickMemory(commands);
  return(command);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t D e l e g a t e C o m m a n d s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDelegateCommands() returns the commands associated with a delegate.
%
%  The format of the GetDelegateCommands method is:
%
%      const char *GetDelegateCommands(const DelegateInfo *delegate_info)
%
%  A description of each parameter follows:
%
%    o delegate_info:  The delegate info.
%
*/
MagickExport const char *GetDelegateCommands(const DelegateInfo *delegate_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(delegate_info != (DelegateInfo *) NULL);
  assert(delegate_info->signature == MagickSignature);
  return(delegate_info->commands);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t D e l e g a t e I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDelegateInfo() returns any delegates associated with the specified tag.
%
%  The format of the GetDelegateInfo method is:
%
%      const DelegateInfo *GetDelegateInfo(const char *decode,
%        const char *encode,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o decode: Specifies the decode delegate we are searching for as a
%      character string.
%
%    o encode: Specifies the encode delegate we are searching for as a
%      character string.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const DelegateInfo *GetDelegateInfo(const char *decode,
  const char *encode,ExceptionInfo *exception)
{
  register const DelegateInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  if ((delegate_list == (LinkedListInfo *) NULL) ||
      (instantiate_delegate == MagickFalse))
    if (InitializeDelegateList(exception) == MagickFalse)
      return((const DelegateInfo *) NULL);
  if ((delegate_list == (LinkedListInfo *) NULL) ||
      (IsLinkedListEmpty(delegate_list) != MagickFalse))
    return((const DelegateInfo *) NULL);
  if ((LocaleCompare(decode,"*") == 0) && (LocaleCompare(encode,"*") == 0))
    return((const DelegateInfo *) GetValueFromLinkedList(delegate_list,0));
  /*
    Search for named delegate.
  */
  LockSemaphoreInfo(delegate_semaphore);
  ResetLinkedListIterator(delegate_list);
  p=(const DelegateInfo *) GetNextValueInLinkedList(delegate_list);
  while (p != (const DelegateInfo *) NULL)
  {
    if (p->mode > 0)
      {
        if (LocaleCompare(p->decode,decode) == 0)
          break;
        p=(const DelegateInfo *) GetNextValueInLinkedList(delegate_list);
        continue;
      }
    if (p->mode < 0)
      {
        if (LocaleCompare(p->encode,encode) == 0)
          break;
        p=(const DelegateInfo *) GetNextValueInLinkedList(delegate_list);
        continue;
      }
    if (LocaleCompare(decode,p->decode) == 0)
      if (LocaleCompare(encode,p->encode) == 0)
        break;
    if (LocaleCompare(decode,"*") == 0)
      if (LocaleCompare(encode,p->encode) == 0)
        break;
    if (LocaleCompare(decode,p->decode) == 0)
      if (LocaleCompare(encode,"*") == 0)
        break;
    p=(const DelegateInfo *) GetNextValueInLinkedList(delegate_list);
  }
  if (p != (const DelegateInfo *) NULL)
    (void) InsertValueInLinkedList(delegate_list,0,
      RemoveElementByValueFromLinkedList(delegate_list,p));
  UnlockSemaphoreInfo(delegate_semaphore);
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t D e l e g a t e I n f o L i s t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDelegateInfoList() returns any delegates that match the specified pattern.
%
%  The delegate of the GetDelegateInfoList function is:
%
%      const DelegateInfo **GetDelegateInfoList(const char *pattern,
%        size_t *number_delegates,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_delegates:  This integer returns the number of delegates in the
%      list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int DelegateInfoCompare(const void *x,const void *y)
{
  const DelegateInfo
    **p,
    **q;

  p=(const DelegateInfo **) x,
  q=(const DelegateInfo **) y;
  if (LocaleCompare((*p)->path,(*q)->path) == 0)
    {
      if ((*p)->decode == (char *) NULL)
        if (((*p)->encode != (char *) NULL) &&
            ((*q)->encode != (char *) NULL))
          return(strcmp((*p)->encode,(*q)->encode));
      if (((*p)->decode != (char *) NULL) &&
          ((*q)->decode != (char *) NULL))
        return(strcmp((*p)->decode,(*q)->decode));
    }
  return(LocaleCompare((*p)->path,(*q)->path));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport const DelegateInfo **GetDelegateInfoList(const char *pattern,
  size_t *number_delegates,ExceptionInfo *exception)
{
  const DelegateInfo
    **delegates;

  register const DelegateInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate delegate list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_delegates != (size_t *) NULL);
  *number_delegates=0;
  p=GetDelegateInfo("*","*",exception);
  if (p == (const DelegateInfo *) NULL)
    return((const DelegateInfo **) NULL);
  delegates=(const DelegateInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(delegate_list)+1UL,sizeof(*delegates));
  if (delegates == (const DelegateInfo **) NULL)
    return((const DelegateInfo **) NULL);
  /*
    Generate delegate list.
  */
  LockSemaphoreInfo(delegate_semaphore);
  ResetLinkedListIterator(delegate_list);
  p=(const DelegateInfo *) GetNextValueInLinkedList(delegate_list);
  for (i=0; p != (const DelegateInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        ((GlobExpression(p->decode,pattern,MagickFalse) != MagickFalse) ||
         (GlobExpression(p->encode,pattern,MagickFalse) != MagickFalse)))
      delegates[i++]=p;
    p=(const DelegateInfo *) GetNextValueInLinkedList(delegate_list);
  }
  UnlockSemaphoreInfo(delegate_semaphore);
  qsort((void *) delegates,(size_t) i,sizeof(*delegates),DelegateInfoCompare);
  delegates[i]=(DelegateInfo *) NULL;
  *number_delegates=(size_t) i;
  return(delegates);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t D e l e g a t e L i s t                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDelegateList() returns any image format delegates that match the
%  specified  pattern.
%
%  The format of the GetDelegateList function is:
%
%      char **GetDelegateList(const char *pattern,
%        size_t *number_delegates,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_delegates:  This integer returns the number of delegates
%      in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int DelegateCompare(const void *x,const void *y)
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

MagickExport char **GetDelegateList(const char *pattern,
  size_t *number_delegates,ExceptionInfo *exception)
{
  char
    **delegates;

  register const DelegateInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate delegate list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_delegates != (size_t *) NULL);
  *number_delegates=0;
  p=GetDelegateInfo("*","*",exception);
  if (p == (const DelegateInfo *) NULL)
    return((char **) NULL);
  delegates=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(delegate_list)+1UL,sizeof(*delegates));
  if (delegates == (char **) NULL)
    return((char **) NULL);
  LockSemaphoreInfo(delegate_semaphore);
  ResetLinkedListIterator(delegate_list);
  p=(const DelegateInfo *) GetNextValueInLinkedList(delegate_list);
  for (i=0; p != (const DelegateInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->decode,pattern,MagickFalse) != MagickFalse))
      delegates[i++]=ConstantString(p->decode);
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->encode,pattern,MagickFalse) != MagickFalse))
      delegates[i++]=ConstantString(p->encode);
    p=(const DelegateInfo *) GetNextValueInLinkedList(delegate_list);
  }
  UnlockSemaphoreInfo(delegate_semaphore);
  qsort((void *) delegates,(size_t) i,sizeof(*delegates),DelegateCompare);
  delegates[i]=(char *) NULL;
  *number_delegates=(size_t) i;
  return(delegates);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t D e l e g a t e M o d e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDelegateMode() returns the mode of the delegate.
%
%  The format of the GetDelegateMode method is:
%
%      ssize_t GetDelegateMode(const DelegateInfo *delegate_info)
%
%  A description of each parameter follows:
%
%    o delegate_info:  The delegate info.
%
*/
MagickExport ssize_t GetDelegateMode(const DelegateInfo *delegate_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(delegate_info != (DelegateInfo *) NULL);
  assert(delegate_info->signature == MagickSignature);
  return(delegate_info->mode);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t D e l e g a t e T h r e a d S u p p o r t                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDelegateThreadSupport() returns MagickTrue if the delegate supports
%  threads.
%
%  The format of the GetDelegateThreadSupport method is:
%
%      MagickBooleanType GetDelegateThreadSupport(
%        const DelegateInfo *delegate_info)
%
%  A description of each parameter follows:
%
%    o delegate_info:  The delegate info.
%
*/
MagickExport MagickBooleanType GetDelegateThreadSupport(
  const DelegateInfo *delegate_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(delegate_info != (DelegateInfo *) NULL);
  assert(delegate_info->signature == MagickSignature);
  return(delegate_info->thread_support);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t i a l i z e D e l e g a t e L i s t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeDelegateList() initializes the delegate list.
%
%  The format of the InitializeDelegateList method is:
%
%      MagickBooleanType InitializeDelegateList(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType InitializeDelegateList(ExceptionInfo *exception)
{
  if ((delegate_list == (LinkedListInfo *) NULL) &&
      (instantiate_delegate == MagickFalse))
    {
      if (delegate_semaphore == (SemaphoreInfo *) NULL)
        AcquireSemaphoreInfo(&delegate_semaphore);
      LockSemaphoreInfo(delegate_semaphore);
      if ((delegate_list == (LinkedListInfo *) NULL) &&
          (instantiate_delegate == MagickFalse))
        {
          (void) LoadDelegateLists(DelegateFilename,exception);
          instantiate_delegate=MagickTrue;
        }
      UnlockSemaphoreInfo(delegate_semaphore);
    }
  return(delegate_list != (LinkedListInfo *) NULL ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n v o k e D e l e g a t e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InvokeDelegate replaces any embedded formatting characters with the
%  appropriate image attribute and executes the resulting command.  MagickFalse
%  is returned if the commands execute with success otherwise MagickTrue.
%
%  The format of the InvokeDelegate method is:
%
%      MagickBooleanType InvokeDelegate(ImageInfo *image_info,Image *image,
%        const char *decode,const char *encode,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the imageInfo.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static MagickBooleanType CopyDelegateFile(const char *source,
  const char *destination)
{
  int
    destination_file,
    source_file;

  MagickBooleanType
    status;

  register size_t
    i;

  size_t
    length,
    quantum;

  ssize_t
    count;

  struct stat
    attributes;

  unsigned char
    *buffer;

  /*
    Copy source file to destination.
  */
  assert(source != (const char *) NULL);
  assert(destination != (char *) NULL);
  status=GetPathAttributes(destination,&attributes);
  if ((status != MagickFalse) && (attributes.st_size != 0))
    return(MagickTrue);
  destination_file=open_utf8(destination,O_WRONLY | O_BINARY | O_CREAT,S_MODE);
  if (destination_file == -1)
    return(MagickFalse);
  source_file=open_utf8(source,O_RDONLY | O_BINARY,0);
  if (source_file == -1)
    {
      (void) close(destination_file);
      return(MagickFalse);
    }
  quantum=(size_t) MagickMaxBufferExtent;
  if ((fstat(source_file,&attributes) == 0) && (attributes.st_size != 0))
    quantum=MagickMin((size_t) attributes.st_size,MagickMaxBufferExtent);
  buffer=(unsigned char *) AcquireQuantumMemory(quantum,sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    {
      (void) close(source_file);
      (void) close(destination_file);
      return(MagickFalse);
    }
  length=0;
  for (i=0; ; i+=count)
  {
    count=(ssize_t) read(source_file,buffer,quantum);
    if (count <= 0)
      break;
    length=(size_t) count;
    count=(ssize_t) write(destination_file,buffer,length);
    if ((size_t) count != length)
      break;
  }
  (void) close(destination_file);
  (void) close(source_file);
  buffer=(unsigned char *) RelinquishMagickMemory(buffer);
  return(i != 0 ? MagickTrue : MagickFalse);
}

MagickExport MagickBooleanType InvokeDelegate(ImageInfo *image_info,
  Image *image,const char *decode,const char *encode,ExceptionInfo *exception)
{
  char
    *command,
    **commands,
    input_filename[MaxTextExtent],
    output_filename[MaxTextExtent];

  const DelegateInfo
    *delegate_info;

  MagickBooleanType
    status,
    temporary;

  register ssize_t
    i;

  PolicyRights
    rights;

  /*
    Get delegate.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  rights=ExecutePolicyRights;
  if (IsRightsAuthorized(DelegatePolicyDomain,rights,decode) == MagickFalse)
    {
      errno=EPERM;
      (void) ThrowMagickException(exception,GetMagickModule(),PolicyError,
        "NotAuthorized","`%s'",decode);
      return(MagickFalse);
    }
  if (IsRightsAuthorized(DelegatePolicyDomain,rights,encode) == MagickFalse)
    {
      errno=EPERM;
      (void) ThrowMagickException(exception,GetMagickModule(),PolicyError,
        "NotAuthorized","`%s'",encode);
      return(MagickFalse);
    }
  temporary=(*image->filename == '\0') ? MagickTrue : MagickFalse;
  if (temporary != MagickFalse)
    if (AcquireUniqueFilename(image->filename) == MagickFalse)
      {
        ThrowFileException(exception,FileOpenError,
          "UnableToCreateTemporaryFile",image->filename);
        return(MagickFalse);
      }
  delegate_info=GetDelegateInfo(decode,encode,exception);
  if (delegate_info == (DelegateInfo *) NULL)
    {
      if (temporary != MagickFalse)
        (void) RelinquishUniqueFileResource(image->filename);
      (void) ThrowMagickException(exception,GetMagickModule(),DelegateError,
        "NoTagFound","`%s'",decode ? decode : encode);
      return(MagickFalse);
    }
  if (*image_info->filename == '\0')
    {
      if (AcquireUniqueFilename(image_info->filename) == MagickFalse)
        {
          if (temporary != MagickFalse)
            (void) RelinquishUniqueFileResource(image->filename);
          ThrowFileException(exception,FileOpenError,
            "UnableToCreateTemporaryFile",image_info->filename);
          return(MagickFalse);
        }
      image_info->temporary=MagickTrue;
    }
  if ((delegate_info->mode != 0) && (((decode != (const char *) NULL) &&
        (delegate_info->encode != (char *) NULL)) ||
       ((encode != (const char *) NULL) &&
        (delegate_info->decode != (char *) NULL))))
    {
      char
        *magick;

      ImageInfo
        *clone_info;

      register Image
        *p;

      /*
        Delegate requires a particular image format.
      */
      if (AcquireUniqueFilename(image_info->unique) == MagickFalse)
        {
          ThrowFileException(exception,FileOpenError,
            "UnableToCreateTemporaryFile",image_info->unique);
          return(MagickFalse);
        }
      if (AcquireUniqueFilename(image_info->zero) == MagickFalse)
        {
          (void) RelinquishUniqueFileResource(image_info->unique);
          ThrowFileException(exception,FileOpenError,
            "UnableToCreateTemporaryFile",image_info->zero);
          return(MagickFalse);
        }
      magick=InterpretImageProperties(image_info,image,decode != (char *) NULL ?
        delegate_info->encode : delegate_info->decode);
      if (magick == (char *) NULL)
        {
          (void) RelinquishUniqueFileResource(image_info->unique);
          (void) RelinquishUniqueFileResource(image_info->zero);
          if (temporary != MagickFalse)
            (void) RelinquishUniqueFileResource(image->filename);
          (void) ThrowMagickException(exception,GetMagickModule(),
            DelegateError,"DelegateFailed","`%s'",decode ? decode : encode);
          return(MagickFalse);
        }
      LocaleUpper(magick);
      clone_info=CloneImageInfo(image_info);
      (void) CopyMagickString((char *) clone_info->magick,magick,
        MaxTextExtent);
      if (LocaleCompare(magick,"NULL") != 0)
        (void) CopyMagickString(image->magick,magick,MaxTextExtent);
      magick=DestroyString(magick);
      (void) FormatLocaleString(clone_info->filename,MaxTextExtent,"%s:",
        delegate_info->decode);
      (void) SetImageInfo(clone_info,(unsigned int) GetImageListLength(image),
        exception);
      (void) CopyMagickString(clone_info->filename,image_info->filename,
        MaxTextExtent);
      (void) CopyMagickString(image_info->filename,image->filename,
        MaxTextExtent);
      for (p=image; p != (Image *) NULL; p=GetNextImageInList(p))
      {
        (void) FormatLocaleString(p->filename,MaxTextExtent,"%s:%s",
          delegate_info->decode,clone_info->filename);
        status=WriteImage(clone_info,p);
        if (status == MagickFalse)
          {
            (void) RelinquishUniqueFileResource(image_info->unique);
            (void) RelinquishUniqueFileResource(image_info->zero);
            if (temporary != MagickFalse)
              (void) RelinquishUniqueFileResource(image->filename);
            clone_info=DestroyImageInfo(clone_info);
            (void) ThrowMagickException(exception,GetMagickModule(),
              DelegateError,"DelegateFailed","`%s'",decode ? decode : encode);
            return(MagickFalse);
          }
        if (clone_info->adjoin != MagickFalse)
          break;
      }
      (void) RelinquishUniqueFileResource(image_info->unique);
      (void) RelinquishUniqueFileResource(image_info->zero);
      clone_info=DestroyImageInfo(clone_info);
    }
  /*
    Invoke delegate.
  */
  commands=StringToList(delegate_info->commands);
  if (commands == (char **) NULL)
    {
      if (temporary != MagickFalse)
        (void) RelinquishUniqueFileResource(image->filename);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        decode ? decode : encode);
      return(MagickFalse);
    }
  command=(char *) NULL;
  status=MagickFalse;
  (void) CopyMagickString(output_filename,image_info->filename,MaxTextExtent);
  (void) CopyMagickString(input_filename,image->filename,MaxTextExtent);
  for (i=0; commands[i] != (char *) NULL; i++)
  {
    status=AcquireUniqueSymbolicLink(output_filename,image_info->filename);
    if (AcquireUniqueFilename(image_info->unique) == MagickFalse)
      {
        ThrowFileException(exception,FileOpenError,
          "UnableToCreateTemporaryFile",image_info->unique);
        break;
      }
    if (AcquireUniqueFilename(image_info->zero) == MagickFalse)
      {
        (void) RelinquishUniqueFileResource(image_info->unique);
        ThrowFileException(exception,FileOpenError,
          "UnableToCreateTemporaryFile",image_info->zero);
        break;
      }
    if (LocaleCompare(decode,"SCAN") != 0)
      {
        status=AcquireUniqueSymbolicLink(input_filename,image->filename);
        if (status == MagickFalse)
          {
            ThrowFileException(exception,FileOpenError,
              "UnableToCreateTemporaryFile",input_filename);
            break;
          }
      }
    status=MagickFalse;
    command=InterpretImageProperties(image_info,image,commands[i]);
    if (command != (char *) NULL)
      {
        /*
          Execute delegate.
        */
        status=SystemCommand(delegate_info->spawn,image_info->verbose,command,
          exception) != 0 ? MagickTrue : MagickFalse;
        if (delegate_info->spawn != MagickFalse)
          {
            ssize_t
              count;

            /*
              Wait for input file to 'disappear', or maximum 5 secs.
            */
            count=50;
            while ((count-- > 0) && (access_utf8(image->filename,F_OK) == 0))
              (void) MagickDelay(100);  /* sleep 0.1 seconds */
          }
        command=DestroyString(command);
      }
    if (LocaleCompare(decode,"SCAN") != 0)
      {
        if (CopyDelegateFile(image->filename,input_filename) == MagickFalse)
          (void) RelinquishUniqueFileResource(input_filename);
      }
    if (CopyDelegateFile(image_info->filename,output_filename) == MagickFalse)
      (void) RelinquishUniqueFileResource(output_filename);
    if (image_info->temporary != MagickFalse)
      (void) RelinquishUniqueFileResource(image_info->filename);
    (void) RelinquishUniqueFileResource(image_info->unique);
    (void) RelinquishUniqueFileResource(image_info->zero);
    (void) RelinquishUniqueFileResource(image_info->filename);
    (void) RelinquishUniqueFileResource(image->filename);
    if (status != MagickFalse)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),DelegateError,
          "DelegateFailed","`%s'",commands[i]);
        break;
      }
    commands[i]=DestroyString(commands[i]);
  }
  (void) CopyMagickString(image_info->filename,output_filename,MaxTextExtent);
  (void) CopyMagickString(image->filename,input_filename,MaxTextExtent);
  /*
    Relinquish resources.
  */
  for ( ; commands[i] != (char *) NULL; i++)
    commands[i]=DestroyString(commands[i]);
  commands=(char **) RelinquishMagickMemory(commands);
  if (temporary != MagickFalse)
    (void) RelinquishUniqueFileResource(image->filename);
  return(status == MagickFalse ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t D e l e g a t e I n f o                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListDelegateInfo() lists the image formats to a file.
%
%  The format of the ListDelegateInfo method is:
%
%      MagickBooleanType ListDelegateInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListDelegateInfo(FILE *file,
  ExceptionInfo *exception)
{
  const DelegateInfo
    **delegate_info;

  char
    **commands,
    delegate[MaxTextExtent];

  const char
    *path;

  register ssize_t
    i;

  size_t
    number_delegates;

  ssize_t
    j;

  if (file == (const FILE *) NULL)
    file=stdout;
  delegate_info=GetDelegateInfoList("*",&number_delegates,exception);
  if (delegate_info == (const DelegateInfo **) NULL)
    return(MagickFalse);
  path=(const char *) NULL;
  for (i=0; i < (ssize_t) number_delegates; i++)
  {
    if (delegate_info[i]->stealth != MagickFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (LocaleCompare(path,delegate_info[i]->path) != 0))
      {
        if (delegate_info[i]->path != (char *) NULL)
          (void) FormatLocaleFile(file,"\nPath: %s\n\n",delegate_info[i]->path);
        (void) FormatLocaleFile(file,"Delegate                Command\n");
        (void) FormatLocaleFile(file,
          "-------------------------------------------------"
          "------------------------------\n");
      }
    path=delegate_info[i]->path;
    *delegate='\0';
    if (delegate_info[i]->encode != (char *) NULL)
      (void) CopyMagickString(delegate,delegate_info[i]->encode,MaxTextExtent);
    (void) ConcatenateMagickString(delegate,"        ",MaxTextExtent);
    delegate[8]='\0';
    commands=StringToList(delegate_info[i]->commands);
    if (commands == (char **) NULL)
      continue;
    (void) FormatLocaleFile(file,"%11s%c=%c%s  ",delegate_info[i]->decode ?
      delegate_info[i]->decode : "",delegate_info[i]->mode <= 0 ? '<' : ' ',
      delegate_info[i]->mode >= 0 ? '>' : ' ',delegate);
    StripString(commands[0]);
    (void) FormatLocaleFile(file,"\"%s\"\n",commands[0]);
    for (j=1; commands[j] != (char *) NULL; j++)
    {
      StripString(commands[j]);
      (void) FormatLocaleFile(file,"                     \"%s\"\n",commands[j]);
    }
    for (j=0; commands[j] != (char *) NULL; j++)
      commands[j]=DestroyString(commands[j]);
    commands=(char **) RelinquishMagickMemory(commands);
  }
  (void) fflush(file);
  delegate_info=(const DelegateInfo **)
    RelinquishMagickMemory((void *) delegate_info);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d D e l e g a t e L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadDelegateList() loads the delegate configuration file which provides a
%  mapping between delegate attributes and a delegate name.
%
%  The format of the LoadDelegateList method is:
%
%      MagickBooleanType LoadDelegateList(const char *xml,const char *filename,
%        const size_t depth,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The delegate list in XML format.
%
%    o filename:  The delegate list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadDelegateList(const char *xml,const char *filename,
  const size_t depth,ExceptionInfo *exception)
{
  char
    keyword[MaxTextExtent],
    *token;

  const char
    *q;

  DelegateInfo
    *delegate_info;

  MagickBooleanType
    status;

  /*
    Load the delegate map file.
  */
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading delegate configuration file \"%s\" ...",filename);
  if (xml == (const char *) NULL)
    return(MagickFalse);
  if (delegate_list == (LinkedListInfo *) NULL)
    {
      delegate_list=NewLinkedList(0);
      if (delegate_list == (LinkedListInfo *) NULL)
        {
          ThrowFileException(exception,ResourceLimitError,
            "MemoryAllocationFailed",filename);
          return(MagickFalse);
        }
    }
  status=MagickTrue;
  delegate_info=(DelegateInfo *) NULL;
  token=AcquireString(xml);
  for (q=(const char *) xml; *q != '\0'; )
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
                  xml=FileToString(path,~0,exception);
                  if (xml != (char *) NULL)
                    {
                      status=LoadDelegateList(xml,path,depth+1,exception);
                      xml=(char *) RelinquishMagickMemory(xml);
                    }
                }
            }
        }
        continue;
      }
    if (LocaleCompare(keyword,"<delegate") == 0)
      {
        /*
          Delegate element.
        */
        delegate_info=(DelegateInfo *) AcquireMagickMemory(
          sizeof(*delegate_info));
        if (delegate_info == (DelegateInfo *) NULL)
          ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
        (void) ResetMagickMemory(delegate_info,0,sizeof(*delegate_info));
        delegate_info->path=ConstantString(filename);
        delegate_info->signature=MagickSignature;
        continue;
      }
    if (delegate_info == (DelegateInfo *) NULL)
      continue;
    if (LocaleCompare(keyword,"/>") == 0)
      {
        status=AppendValueToLinkedList(delegate_list,delegate_info);
        if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            delegate_info->commands);
        delegate_info=(DelegateInfo *) NULL;
      }
    GetMagickToken(q,(const char **) NULL,token);
    if (*token != '=')
      continue;
    GetMagickToken(q,&q,token);
    GetMagickToken(q,&q,token);
    switch (*keyword)
    {
      case 'C':
      case 'c':
      {
        if (LocaleCompare((char *) keyword,"command") == 0)
          {
            char
              *commands;

            commands=AcquireString(token);
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
            if (strchr(commands,'@') != (char *) NULL)
              {
                char
                  path[MaxTextExtent];

                NTGhostscriptEXE(path,MaxTextExtent);
                (void) SubstituteString((char **) &commands,"@PSDelegate@",
                  path);
                (void) SubstituteString((char **) &commands,"\\","/");
              }
#endif
            (void) SubstituteString((char **) &commands,"&amp;","&");
            (void) SubstituteString((char **) &commands,"&quot;","\"");
            (void) SubstituteString((char **) &commands,"&gt;",">");
            (void) SubstituteString((char **) &commands,"&lt;","<");
            delegate_info->commands=commands;
            break;
          }
        break;
      }
      case 'D':
      case 'd':
      {
        if (LocaleCompare((char *) keyword,"decode") == 0)
          {
            delegate_info->decode=ConstantString(token);
            delegate_info->mode=1;
            break;
          }
        break;
      }
      case 'E':
      case 'e':
      {
        if (LocaleCompare((char *) keyword,"encode") == 0)
          {
            delegate_info->encode=ConstantString(token);
            delegate_info->mode=(-1);
            break;
          }
        break;
      }
      case 'M':
      case 'm':
      {
        if (LocaleCompare((char *) keyword,"mode") == 0)
          {
            delegate_info->mode=1;
            if (LocaleCompare(token,"bi") == 0)
              delegate_info->mode=0;
            else
              if (LocaleCompare(token,"encode") == 0)
                delegate_info->mode=(-1);
            break;
          }
        break;
      }
      case 'S':
      case 's':
      {
        if (LocaleCompare((char *) keyword,"spawn") == 0)
          {
            delegate_info->spawn=IsMagickTrue(token);
            break;
          }
        if (LocaleCompare((char *) keyword,"stealth") == 0)
          {
            delegate_info->stealth=IsMagickTrue(token);
            break;
          }
        break;
      }
      case 'T':
      case 't':
      {
        if (LocaleCompare((char *) keyword,"thread-support") == 0)
          {
            delegate_info->thread_support=IsMagickTrue(token);
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
%  L o a d D e l e g a t e L i s t s                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadDelegateList() loads one or more delegate configuration file which
%  provides a mapping between delegate attributes and a delegate name.
%
%  The format of the LoadDelegateLists method is:
%
%      MagickBooleanType LoadDelegateLists(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadDelegateLists(const char *filename,
  ExceptionInfo *exception)
{
#if defined(MAGICKCORE_ZERO_CONFIGURATION_SUPPORT)
  return(LoadDelegateList(DelegateMap,"built-in",0,exception));
#else
  const StringInfo
    *option;

  LinkedListInfo
    *options;

  MagickStatusType
    status;

  status=MagickFalse;
  options=GetConfigureOptions(filename,exception);
  option=(const StringInfo *) GetNextValueInLinkedList(options);
  while (option != (const StringInfo *) NULL)
  {
    status|=LoadDelegateList((const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),0,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
  }
  options=DestroyConfigureOptions(options);
  if ((delegate_list == (LinkedListInfo *) NULL) ||
      (IsLinkedListEmpty(delegate_list) != MagickFalse))
    status|=LoadDelegateList(DelegateMap,"built-in",0,exception);
  return(status != 0 ? MagickTrue : MagickFalse);
#endif
}

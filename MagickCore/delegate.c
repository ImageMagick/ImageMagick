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
%                                  Cristy                                     %
%                               October 1998                                  %
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
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/client.h"
#include "MagickCore/configure.h"
#include "MagickCore/constitute.h"
#include "MagickCore/delegate.h"
#include "MagickCore/delegate-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/fx-private.h"
#include "MagickCore/image-private.h"
#include "MagickCore/linked-list.h"
#include "MagickCore/linked-list-private.h"
#include "MagickCore/list.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/option.h"
#include "MagickCore/policy.h"
#include "MagickCore/property.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/signature.h"
#include "MagickCore/string_.h"
#include "MagickCore/token.h"
#include "MagickCore/token-private.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/xml-tree.h"
#include "MagickCore/xml-tree-private.h"

/*
  Define declarations.
*/
#if defined(__APPLE__)
  #include "TargetConditionals.h"
  #if TARGET_OS_IOS || TARGET_OS_WATCH || TARGET_OS_TV
    #define system(s) ((s)==NULL ? 0 : -1)
  #endif // end iOS
#elif defined(__ANDROID__)
  #define system(s) ((s)==NULL ? 0 : -1)
#endif
#define DelegateFilename  "delegates.xml"

/*
  Declare delegate map.
*/
static const char
  *DelegateMap = (const char *)
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<delegatemap>"
    "  <delegate decode=\"bpg\" command=\"&quot;bpgdec&quot; -b 16 -o &quot;%o.png&quot; &quot;%i&quot;; mv &quot;%o.png&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"png\" encode=\"bpg\" command=\"&quot;bpgenc&quot; -b 12 -q %~ -o &quot;%o&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"browse\" stealth=\"True\" spawn=\"True\" command=\"&quot;xdg-open&quot; https://imagemagick.org/; rm &quot;%i&quot;\"/>"
    "  <delegate decode=\"cdr\" command=\"&quot;uniconvertor&quot; &quot;%i&quot; &quot;%o.svg&quot;; mv &quot;%o.svg&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"cgm\" command=\"&quot;uniconvertor&quot; &quot;%i&quot; &quot;%o.svg&quot;; mv &quot;%o.svg&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"https\" command=\"&quot;curl&quot; -s -k -L -o &quot;%o&quot; &quot;https:%M&quot;\"/>"
    "  <delegate decode=\"doc\" command=\"&quot;soffice&quot; --convert-to pdf -outdir `dirname &quot;%i&quot;` &quot;%i&quot; 2&gt; &quot;%u&quot;; mv &quot;%i.pdf&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"docx\" command=\"&quot;soffice&quot; --convert-to pdf -outdir `dirname &quot;%i&quot;` &quot;%i&quot; 2&gt; &quot;%u&quot;; mv &quot;%i.pdf&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"dng:decode\" command=\"&quot;ufraw-batch&quot; --silent --create-id=also --out-type=png --out-depth=16 &quot;--output=%u.png&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"dot\" command=\"&quot;dot&quot; -Tsvg &quot;%i&quot; -o &quot;%o&quot;\"/>"
    "  <delegate decode=\"dvi\" command=\"&quot;dvips&quot; -sstdout=%%stderr -o &quot;%o&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"dxf\" command=\"&quot;uniconvertor&quot; &quot;%i&quot; &quot;%o.svg&quot;; mv &quot;%o.svg&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"edit\" stealth=\"True\" command=\"&quot;xterm&quot; -title &quot;Edit Image Comment&quot; -e vi &quot;%o&quot;\"/>"
    "  <delegate decode=\"eps\" encode=\"pdf\" mode=\"bi\" command=\"&quot;gs&quot; -sstdout=%%stderr -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 &quot;-sDEVICE=pdfwrite&quot; &quot;-sOutputFile=%o&quot; &quot;-f%i&quot;\"/>"
    "  <delegate decode=\"eps\" encode=\"ps\" mode=\"bi\" command=\"&quot;gs&quot; -sstdout=%%stderr -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=ps2write&quot; &quot;-sOutputFile=%o&quot; &quot;-f%i&quot;\"/>"
    "  <delegate decode=\"fig\" command=\"&quot;uniconvertor&quot; &quot;%i&quot; &quot;%o.svg&quot;; mv &quot;%o.svg&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"hpg\" command=\"&quot;hp2xx&quot; -sstdout=%%stderr -m eps -f `basename &quot;%o&quot;` &quot;%i&quot;;     mv -f `basename &quot;%o&quot;` &quot;%o&quot;\"/>"
    "  <delegate decode=\"hpgl\" command=\"&quot;hp2xx&quot; -sstdout=%%stderr -m eps -f `basename &quot;%o&quot;` &quot;%i&quot;;     mv -f `basename &quot;%o&quot;` &quot;%o&quot;\"/>"
    "  <delegate decode=\"htm\" command=\"&quot;html2ps&quot; -U -o &quot;%o&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"html\" command=\"&quot;html2ps&quot; -U -o &quot;%o&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"ilbm\" command=\"&quot;ilbmtoppm&quot; &quot;%i&quot; &gt; &quot;%o&quot;\"/>"
    "  <delegate decode=\"jpg\" encode=\"lep\" mode=\"encode\" command=\"&quot;lepton&quot; &quot;%i&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"jxr\" command=\"mv &quot;%i&quot; &quot;%i.jxr&quot;; &quot;JxrDecApp&quot; -i &quot;%i.jxr&quot; -o &quot;%o.tiff&quot;; mv &quot;%i.jxr&quot; &quot;%i&quot;; mv &quot;%o.tiff&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"lep\" mode=\"decode\" command=\"&quot;lepton&quot; &quot;%i&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"odt\" command=\"&quot;soffice&quot; --convert-to pdf -outdir `dirname &quot;%i&quot;` &quot;%i&quot; 2&gt; &quot;%u&quot;; mv &quot;%i.pdf&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"pcl:cmyk\" stealth=\"True\" command=\"&quot;pcl6&quot; -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pamcmyk32&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;%s&quot;\"/>"
    "  <delegate decode=\"pcl:color\" stealth=\"True\" command=\"&quot;pcl6&quot; -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=ppmraw&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;%s&quot;\"/>"
    "  <delegate decode=\"pcl:mono\" stealth=\"True\" command=\"&quot;pcl6&quot; -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pbmraw&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;%s&quot;\"/>"
    "  <delegate decode=\"pdf\" encode=\"eps\" mode=\"bi\" command=\"&quot;gs&quot; -sstdout=%%stderr -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 -sPDFPassword=&quot;%a&quot; &quot;-sDEVICE=eps2write&quot; &quot;-sOutputFile=%o&quot; &quot;-f%i&quot;\"/>"
    "  <delegate decode=\"pdf\" encode=\"ps\" mode=\"bi\" command=\"&quot;gs&quot; -sstdout=%%stderr -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=ps2write&quot; -sPDFPassword=&quot;%a&quot; &quot;-sOutputFile=%o&quot; &quot;-f%i&quot;\"/>"
    "  <delegate decode=\"png\" encode=\"webp\" command=\"&quot;cwebp&quot; -quiet -q %Q &quot;%i&quot; -o &quot;%o&quot;\"/>"
    "  <delegate decode=\"pnm\" encode=\"ilbm\" mode=\"encode\" command=\"&quot;ppmtoilbm&quot; -24if &quot;%i&quot; &gt; &quot;%o&quot;\"/>"
    "  <delegate decode=\"tiff\" encode=\"jxr\" command=\"mv &quot;%i&quot; &quot;%i.tiff&quot;; &quot;JxrEncApp&quot; -i &quot;%i.tiff&quot; -o &quot;%o.jxr&quot;; mv &quot;%i.tiff&quot; &quot;%i&quot;; mv &quot;%o.jxr&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"tiff\" encode=\"wdp\" command=\"mv &quot;%i&quot; &quot;%i.tiff&quot;; &quot;JxrEncApp&quot; -i &quot;%i.tiff&quot; -o &quot;%o.jxr&quot;; mv &quot;%i.tiff&quot; &quot;%i&quot;; mv &quot;%o.jxr&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"ppt\" command=\"&quot;soffice&quot; --convert-to pdf -outdir `dirname &quot;%i&quot;` &quot;%i&quot; 2&gt; &quot;%u&quot;; mv &quot;%i.pdf&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"pptx\" command=\"&quot;soffice&quot; --convert-to pdf -outdir `dirname &quot;%i&quot;` &quot;%i&quot; 2&gt; &quot;%u&quot;; mv &quot;%i.pdf&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"ps\" encode=\"prt\" command=\"&quot;lpr&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"ps:alpha\" stealth=\"True\" command=\"&quot;gs&quot; -sstdout=%%stderr -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pngalpha&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;-f%s&quot; &quot;-f%s&quot;\"/>"
    "  <delegate decode=\"ps:cmyk\" stealth=\"True\" command=\"&quot;gs&quot; -sstdout=%%stderr -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pamcmyk32&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;-f%s&quot; &quot;-f%s&quot;\"/>"
    "  <delegate decode=\"ps:color\" stealth=\"True\" command=\"&quot;gs&quot; -sstdout=%%stderr -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pnmraw&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;-f%s&quot; &quot;-f%s&quot;\"/>"
    "  <delegate decode=\"ps\" encode=\"eps\" mode=\"bi\" command=\"&quot;gs&quot; -sstdout=%%stderr -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=eps2write&quot; &quot;-sOutputFile=%o&quot; &quot;-f%i&quot;\"/>"
    "  <delegate decode=\"ps\" encode=\"pdf\" mode=\"bi\" command=\"&quot;gs&quot; -sstdout=%%stderr -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pdfwrite&quot; &quot;-sOutputFile=%o&quot; &quot;-f%i&quot;\"/>"
    "  <delegate decode=\"ps\" encode=\"print\" mode=\"encode\" command=\"lpr &quot;%i&quot;\"/>"
    "  <delegate decode=\"ps:mono\" stealth=\"True\" command=\"&quot;gs&quot; -sstdout=%%stderr -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pbmraw&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;-f%s&quot; &quot;-f%s&quot;\"/>"
    "  <delegate decode=\"shtml\" command=\"&quot;html2ps&quot; -U -o &quot;%o&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"sid\" command=\"&quot;mrsidgeodecode&quot; -if sid -i &quot;%i&quot; -of tif -o &quot;%o&quot; &gt; &quot;%u&quot;\"/>"
    "  <delegate decode=\"svg\" command=\"&quot;rsvg-convert&quot; --dpi-x %x --dpi-y %y -o &quot;%o&quot; &quot;%i&quot;\"/>"
#ifndef MAGICKCORE_RSVG_DELEGATE
    "  <delegate decode=\"svg:decode\" stealth=\"True\" command=\"&quot;inkscape&quot; &quot;%s&quot; --export-png=&quot;%s&quot; --export-dpi=&quot;%s&quot; --export-background=&quot;%s&quot; --export-background-opacity=&quot;%s&quot; &gt; &quot;%s&quot; 2&gt;&amp;1\"/>"
#endif
    "  <delegate decode=\"tiff\" encode=\"launch\" mode=\"encode\" command=\"&quot;gimp&quot; &quot;%i&quot;\"/>"
    "  <delegate decode=\"wdp\" command=\"mv &quot;%i&quot; &quot;%i.jxr&quot;; &quot;JxrDecApp&quot; -i &quot;%i.jxr&quot; -o &quot;%o.tiff&quot;; mv &quot;%i.jxr&quot; &quot;%i&quot;; mv &quot;%o.tiff&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"webp\" command=\"&quot;dwebp&quot; -pam &quot;%i&quot; -o &quot;%o&quot;\"/>"
    "  <delegate decode=\"xls\" command=\"&quot;soffice&quot; --convert-to pdf -outdir `dirname &quot;%i&quot;` &quot;%i&quot; 2&gt; &quot;%u&quot;; mv &quot;%i.pdf&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"xlsx\" command=\"&quot;soffice&quot; --convert-to pdf -outdir `dirname &quot;%i&quot;` &quot;%i&quot; 2&gt; &quot;%u&quot;; mv &quot;%i.pdf&quot; &quot;%o&quot;\"/>"
    "  <delegate decode=\"xps:cmyk\" stealth=\"True\" command=\"&quot;gxps&quot; -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=bmpsep8&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;%s&quot;\"/>"
    "  <delegate decode=\"xps:color\" stealth=\"True\" command=\"&quot;gxps&quot; -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=ppmraw&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;%s&quot;\"/>"
    "  <delegate decode=\"xps:mono\" stealth=\"True\" command=\"&quot;gxps&quot; -dQUIET -dSAFER -dBATCH -dNOPAUSE -dNOPROMPT -dMaxBitmap=500000000 -dAlignToPixels=0 -dGridFitTT=2 &quot;-sDEVICE=pbmraw&quot; -dTextAlphaBits=%u -dGraphicsAlphaBits=%u &quot;-r%s&quot; %s &quot;-sOutputFile=%s&quot; &quot;%s&quot;\"/>"
    "  <delegate decode=\"video:decode\" command=\"&quot;ffmpeg&quot; -nostdin -loglevel error -i &quot;%s&quot; -an -f rawvideo -y %s &quot;%s&quot;\"/>"
    "  <delegate encode=\"video:encode\" stealth=\"True\" command=\"&quot;ffmpeg&quot; -nostdin -loglevel error -i &quot;%s%%d.%s&quot; %s &quot;%s.%s&quot;\"/>"
    "</delegatemap>";

/*
  Global declarations.
*/
static LinkedListInfo
  *delegate_cache = (LinkedListInfo *) NULL;

static SemaphoreInfo
  *delegate_semaphore = (SemaphoreInfo *) NULL;

/*
  Forward declarations.
*/
static MagickBooleanType
  IsDelegateCacheInstantiated(ExceptionInfo *),
  LoadDelegateCache(LinkedListInfo *,const char *,const char *,const size_t,
    ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A c q u i r e D e l e g a t e C a c h e                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireDelegateCache() caches one or more delegate configurations which
%  provides a mapping between delegate attributes and a delegate name.
%
%  The format of the AcquireDelegateCache method is:
%
%      LinkedListInfo *AcquireDelegateCache(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static LinkedListInfo *AcquireDelegateCache(const char *filename,
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
      (void) LoadDelegateCache(cache,(const char *)
        GetStringInfoDatum(option),GetStringInfoPath(option),0,exception);
      option=(const StringInfo *) GetNextValueInLinkedList(options);
    }
    options=DestroyConfigureOptions(options);
  }
#else
  magick_unreferenced(filename);
#endif
  if (IsLinkedListEmpty(cache) != MagickFalse)
    (void) LoadDelegateCache(cache,DelegateMap,"built-in",0,exception);
  return(cache);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e l e g a t e C o m p o n e n t G e n e s i s                           %
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
MagickPrivate MagickBooleanType DelegateComponentGenesis(void)
{
  if (delegate_semaphore == (SemaphoreInfo *) NULL)
    delegate_semaphore=AcquireSemaphoreInfo();
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
  DelegateInfo
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
  if (p->semaphore != (SemaphoreInfo *) NULL)
    RelinquishSemaphoreInfo(&p->semaphore);
  p=(DelegateInfo *) RelinquishMagickMemory(p);
  return((void *) NULL);
}

MagickPrivate void DelegateComponentTerminus(void)
{
  if (delegate_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&delegate_semaphore);
  LockSemaphoreInfo(delegate_semaphore);
  if (delegate_cache != (LinkedListInfo *) NULL)
    delegate_cache=DestroyLinkedList(delegate_cache,DestroyDelegate);
  UnlockSemaphoreInfo(delegate_semaphore);
  RelinquishSemaphoreInfo(&delegate_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E x t e r n a l D e l e g a t e C o m m a n d                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExternalDelegateCommand() executes the specified command and waits until it
%  terminates.  The returned value is the exit status of the command.
%
%  The format of the ExternalDelegateCommand method is:
%
%      int ExternalDelegateCommand(const MagickBooleanType asynchronous,
%        const MagickBooleanType verbose,const char *command,
%        char *message,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o asynchronous: a value other than 0 executes the parent program
%      concurrently with the new child process.
%
%    o verbose: a value other than 0 prints the executed command before it is
%      invoked.
%
%    o command: this string is the command to execute.
%
%    o message: an option buffer to receive any message posted to stdout or
%      stderr.
%
%    o exception: return any errors here.
%
*/
MagickExport int ExternalDelegateCommand(const MagickBooleanType asynchronous,
  const MagickBooleanType verbose,const char *command,char *message,
  ExceptionInfo *exception)
{
  char
    **arguments,
    *sanitize_command;

  int
    number_arguments,
    status;

  PolicyDomain
    domain;

  PolicyRights
    rights;

  ssize_t
    i;

  status=(-1);
  arguments=StringToArgv(command,&number_arguments);
  if (arguments == (char **) NULL)
    return(status);
  if (*arguments[1] == '\0')
    {
      for (i=0; i < (ssize_t) number_arguments; i++)
        arguments[i]=DestroyString(arguments[i]);
      arguments=(char **) RelinquishMagickMemory(arguments);
      return(-1);
    }
  rights=ExecutePolicyRights;
  domain=DelegatePolicyDomain;
  if (IsRightsAuthorized(domain,rights,arguments[1]) == MagickFalse)
    {
      errno=EPERM;
      (void) ThrowMagickException(exception,GetMagickModule(),PolicyError,
        "NotAuthorized","`%s'",arguments[1]);
      for (i=0; i < (ssize_t) number_arguments; i++)
        arguments[i]=DestroyString(arguments[i]);
      arguments=(char **) RelinquishMagickMemory(arguments);
      return(-1);
    }
  if (verbose != MagickFalse)
    {
      (void) FormatLocaleFile(stderr,"%s\n",command);
      (void) fflush(stderr);
    }
  sanitize_command=SanitizeString(command);
  if (asynchronous != MagickFalse)
    (void) ConcatenateMagickString(sanitize_command,"&",MagickPathExtent);
  if (message != (char *) NULL)
    *message='\0';
#if defined(MAGICKCORE_POSIX_SUPPORT)
#if defined(MAGICKCORE_HAVE_POPEN)
  if ((asynchronous == MagickFalse) && (message !=  (char *) NULL))
    {
      char
        buffer[MagickPathExtent];

      FILE
        *file;

      size_t
        offset;

      offset=0;
      file=popen_utf8(sanitize_command,"r");
      if (file == (FILE *) NULL)
        status=system(sanitize_command);
      else
        {
          while (fgets(buffer,(int) sizeof(buffer),file) != NULL)
          {
            size_t
              length;

            length=MagickMin(MagickPathExtent-offset,strlen(buffer)+1);
            if (length > 0)
              {
                (void) CopyMagickString(message+offset,buffer,length);
                offset+=length-1;
              }
          }
          status=pclose(file);
        }
    }
  else
#endif
    {
#if !defined(MAGICKCORE_HAVE_EXECVP)
      status=system(sanitize_command);
#else
      if ((asynchronous != MagickFalse) ||
          (strpbrk(sanitize_command,"&;<>|") != (char *) NULL))
        status=system(sanitize_command);
      else
        {
          pid_t
            child_pid;

          /*
            Call application directly rather than from a shell.
          */
          child_pid=(pid_t) fork();
          if (child_pid == (pid_t) -1)
            status=system(sanitize_command);
          else
            if (child_pid == 0)
              {
                status=execvp(arguments[1],arguments+1);
                _exit(1);
              }
            else
              {
                int
                  child_status;

                pid_t
                  pid;

                child_status=0;
                pid=(pid_t) waitpid(child_pid,&child_status,0);
                if (pid == -1)
                  status=(-1);
                else
                  {
                    if (WIFEXITED(child_status) != 0)
                      status=WEXITSTATUS(child_status);
                    else
                      if (WIFSIGNALED(child_status))
                        status=(-1);
                  }
              }
        }
#endif
    }
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  {
    char
      *p;

    /*
      If a command shell is executed we need to change the forward slashes in
      files to a backslash. We need to do this to keep Windows happy when we
      want to 'move' a file.

      TODO: This won't work if one of the delegate parameters has a forward
            slash as a parameter.
    */
    p=strstr(sanitize_command,"cmd.exe /c");
    if (p != (char*) NULL)
      {
        p+=10;
        for ( ; *p != '\0'; p++)
          if (*p == '/')
            *p=(*DirectorySeparator);
      }
  }
  status=NTSystemCommand(sanitize_command,message);
#elif defined(vms)
  status=system(sanitize_command);
#else
#  error No suitable system() method.
#endif
  if (status < 0)
    {
      if ((message != (char *) NULL) && (*message != '\0'))
        (void) ThrowMagickException(exception,GetMagickModule(),DelegateError,
          "FailedToExecuteCommand","`%s' (%s)",sanitize_command,message);
      else
        (void) ThrowMagickException(exception,GetMagickModule(),DelegateError,
          "FailedToExecuteCommand","`%s' (%d)",sanitize_command,status);
    }
  sanitize_command=DestroyString(sanitize_command);
  for (i=0; i < (ssize_t) number_arguments; i++)
    arguments[i]=DestroyString(arguments[i]);
  arguments=(char **) RelinquishMagickMemory(arguments);
  return(status);
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

static char *GetMagickPropertyLetter(ImageInfo *image_info,Image *image,
  const char letter,ExceptionInfo *exception)
{
#define WarnNoImageReturn(format,letter) \
  if (image == (Image *) NULL) \
    { \
      (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning, \
        "NoImageForProperty",format,letter); \
      break; \
    }
#define WarnNoImageInfoReturn(format,letter) \
  if (image_info == (ImageInfo *) NULL) \
    { \
      (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning, \
        "NoImageInfoForProperty",format,letter); \
      break; \
    }

  char
    value[MagickPathExtent];

  const char
    *string;

  if ((image != (Image *) NULL) && (IsEventLogging() != MagickFalse))
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  else
    if ((image_info != (ImageInfo *) NULL) && (IsEventLogging() != MagickFalse))
      (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s","no-images");
  /*
    Get properties that are directly defined by images.
  */
  *value='\0';           /* formatted string */
  string=(const char *) value;
  switch (letter)
  {
    case 'a': /* authentication passphrase */
    {
      WarnNoImageInfoReturn("\"%%%c\"",letter);
      string=GetImageOption(image_info,"authenticate");
      break;
    }
    case 'b':  /* image size read in - in bytes */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatMagickSize(image->extent,MagickFalse,"B",MagickPathExtent,
        value);
      if (image->extent == 0)
        (void) FormatMagickSize(GetBlobSize(image),MagickFalse,"B",
          MagickPathExtent,value);
      break;
    }
    case 'd':  /* Directory component of filename */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      GetPathComponent(image->magick_filename,HeadPath,value);
      break;
    }
    case 'e': /* Filename extension (suffix) of image file */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      GetPathComponent(image->magick_filename,ExtensionPath,value);
      break;
    }
    case 'f': /* Filename without directory component */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      GetPathComponent(image->magick_filename,TailPath,value);
      break;
    }
    case 'g': /* Image geometry, canvas and offset  %Wx%H+%X+%Y */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,
        "%.20gx%.20g%+.20g%+.20g",(double) image->page.width,(double)
        image->page.height,(double) image->page.x,(double) image->page.y);
      break;
    }
    case 'h': /* Image height (current) */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",(double)
        (image->rows != 0 ? image->rows : image->magick_rows));
      break;
    }
    case 'i': /* Filename last used for an image (read or write) */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      string=image->filename;
      break;
    }
    case 'm': /* Image format (file magick) */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      string=image->magick;
      break;
    }
    case 'n': /* Number of images in the list.  */
    {
      if (image != (Image *) NULL)
        (void) FormatLocaleString(value,MagickPathExtent,"%.20g",(double)
          GetImageListLength(image));
      break;
    }
    case 'o': /* Output Filename */
    {
      WarnNoImageInfoReturn("\"%%%c\"",letter);
      string=image_info->filename;
      break;
    }
    case 'p': /* Image index in current image list */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",(double)
        GetImageIndexInList(image));
      break;
    }
    case 'q': /* Quantum depth of image in memory */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",(double)
        MAGICKCORE_QUANTUM_DEPTH);
      break;
    }
    case 'r': /* Image storage class, colorspace, and alpha enabled.  */
    {
      ColorspaceType
        colorspace;

      WarnNoImageReturn("\"%%%c\"",letter);
      colorspace=image->colorspace;
      (void) FormatLocaleString(value,MagickPathExtent,"%s %s %s",
        CommandOptionToMnemonic(MagickClassOptions,(ssize_t)
        image->storage_class),CommandOptionToMnemonic(MagickColorspaceOptions,
        (ssize_t) colorspace),image->alpha_trait != UndefinedPixelTrait ?
        "Alpha" : "");
      break;
    }
    case 's': /* Image scene number */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",(double)
        image->scene);
      break;
    }
    case 't': /* Base filename without directory or extension */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      GetPathComponent(image->magick_filename,BasePath,value);
      break;
    }
    case 'u': /* Unique filename */
    {
      WarnNoImageInfoReturn("\"%%%c\"",letter);
      string=image_info->unique;
      break;
    }
    case 'w': /* Image width (current) */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",(double)
        (image->columns != 0 ? image->columns : image->magick_columns));
      break;
    }
    case 'x': /* Image horizontal resolution (with units) */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",
        fabs(image->resolution.x) > MagickEpsilon ? image->resolution.x :
        image->units == PixelsPerCentimeterResolution ? DefaultResolution/2.54 :
        DefaultResolution);
      break;
    }
    case 'y': /* Image vertical resolution (with units) */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",
        fabs(image->resolution.y) > MagickEpsilon ? image->resolution.y :
        image->units == PixelsPerCentimeterResolution ? DefaultResolution/2.54 :
        DefaultResolution);
      break;
    }
    case 'z': /* Image depth as read in */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",
        (double) image->depth);
      break;
    }
    case 'A': /* Image alpha channel  */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      string=CommandOptionToMnemonic(MagickPixelTraitOptions,(ssize_t)
        image->alpha_trait);
      break;
    }
    case 'C': /* Image compression method.  */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      string=CommandOptionToMnemonic(MagickCompressOptions,
        (ssize_t) image->compression);
      break;
    }
    case 'D': /* Image dispose method.  */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      string=CommandOptionToMnemonic(MagickDisposeOptions,
        (ssize_t) image->dispose);
      break;
    }
    case 'F':
    {
      /*
        Magick filename - filename given incl. coder & read mods.
      */
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) CopyMagickString(value,image->magick_filename,MagickPathExtent);
      break;
    }
    case 'G': /* Image size as geometry = "%wx%h" */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20gx%.20g",
        (double) image->magick_columns,(double) image->magick_rows);
      break;
    }
    case 'H': /* layer canvas height */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",
        (double) image->page.height);
      break;
    }
    case 'I': /* image iterations for animations */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",(double)
        image->iterations);
      break;
    }
    case 'M': /* Magick filename - filename given incl. coder & read mods */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      string=image->magick_filename;
      break;
    }
    case 'O': /* layer canvas offset with sign = "+%X+%Y" */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%+ld%+ld",(long)
        image->page.x,(long) image->page.y);
      break;
    }
    case 'P': /* layer canvas page size = "%Wx%H" */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20gx%.20g",
        (double) image->page.width,(double) image->page.height);
      break;
    }
    case '~': /* BPG image compression quality */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",(double)
        (100-(image->quality == 0 ? 42 : image->quality))/2);
      break;
    }
    case 'Q': /* image compression quality */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",(double)
        (image->quality == 0 ? 92 : image->quality));
      break;
    }
    case 'S': /* Number of scenes in image list.  */
    {
      WarnNoImageInfoReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",(double)
        (image_info->number_scenes == 0 ? 2147483647 :
         image_info->number_scenes));
      break;
    }
    case 'T': /* image time delay for animations */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",(double)
        image->delay);
      break;
    }
    case 'U': /* Image resolution units. */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      string=CommandOptionToMnemonic(MagickResolutionOptions,
        (ssize_t) image->units);
      break;
    }
    case 'W': /* layer canvas width */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%.20g",(double)
        image->page.width);
      break;
    }
    case 'X': /* layer canvas X offset */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%+.20g",(double)
        image->page.x);
      break;
    }
    case 'Y': /* layer canvas Y offset */
    {
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) FormatLocaleString(value,MagickPathExtent,"%+.20g",(double)
        image->page.y);
      break;
    }
    case '%': /* percent escaped */
    {
      string="%";
      break;
    }
    case '@': /* Trim bounding box, without actually trimming! */
    {
      RectangleInfo
        page;

      WarnNoImageReturn("\"%%%c\"",letter);
      page=GetImageBoundingBox(image,exception);
      (void) FormatLocaleString(value,MagickPathExtent,
        "%.20gx%.20g%+.20g%+.20g",(double) page.width,(double) page.height,
        (double) page.x,(double) page.y);
      break;
    }
    case '#':
    {
      /*
        Image signature.
      */
      WarnNoImageReturn("\"%%%c\"",letter);
      (void) SignatureImage(image,exception);
      string=GetImageProperty(image,"signature",exception);
      break;
    }
  }
  return(SanitizeDelegateString(string));
}

static char *InterpretDelegateProperties(ImageInfo *image_info,
  Image *image,const char *embed_text,ExceptionInfo *exception)
{
#define ExtendInterpretText(string_length) \
{ \
  size_t length=(string_length); \
  if ((size_t) (q-interpret_text+(ssize_t) length+1) >= extent) \
    { \
      extent+=length; \
      interpret_text=(char *) ResizeQuantumMemory(interpret_text,extent+ \
        MagickPathExtent,sizeof(*interpret_text)); \
      if (interpret_text == (char *) NULL) \
        return((char *) NULL); \
      q=interpret_text+strlen(interpret_text); \
   } \
}

#define AppendKeyValue2Text(key,value)\
{ \
  size_t length=strlen(key)+strlen(value)+2; \
  if ((size_t) (q-interpret_text+length+1) >= extent) \
    { \
      extent+=length; \
      interpret_text=(char *) ResizeQuantumMemory(interpret_text,extent+ \
        MagickPathExtent,sizeof(*interpret_text)); \
      if (interpret_text == (char *) NULL) \
        return((char *) NULL); \
      q=interpret_text+strlen(interpret_text); \
     } \
   q+=FormatLocaleString(q,extent,"%s=%s\n",(key),(value)); \
}

#define AppendString2Text(string) \
{ \
  size_t length=strlen((string)); \
  if ((size_t) (q-interpret_text+(ssize_t) length+1) >= extent) \
    { \
      extent+=length; \
      interpret_text=(char *) ResizeQuantumMemory(interpret_text,extent+ \
        MagickPathExtent,sizeof(*interpret_text)); \
      if (interpret_text == (char *) NULL) \
        return((char *) NULL); \
      q=interpret_text+strlen(interpret_text); \
    } \
  (void) CopyMagickString(q,(string),extent); \
  q+=length; \
}

  char
    *interpret_text,
    *string;

  char
    *q;  /* current position in interpret_text */

  const char
    *p;  /* position in embed_text string being expanded */

  size_t
    extent;  /* allocated length of interpret_text */

  MagickBooleanType
    number;

  assert(image == NULL || image->signature == MagickCoreSignature);
  assert(image_info == NULL || image_info->signature == MagickCoreSignature);
  if ((image != (Image *) NULL) && (IsEventLogging() != MagickFalse))
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  else
   if ((image_info != (ImageInfo *) NULL) && (IsEventLogging() != MagickFalse))
     (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s","no-image");
  if (embed_text == (const char *) NULL)
    return(ConstantString(""));
  p=embed_text;
  while ((isspace((int) ((unsigned char) *p)) != 0) && (*p != '\0'))
    p++;
  if (*p == '\0')
    return(ConstantString(""));
  /*
    Translate any embedded format characters.
  */
  interpret_text=AcquireString(embed_text);  /* new string with extra space */
  extent=MagickPathExtent;  /* allocated space in string */
  number=MagickFalse;  /* is last char a number? */
  for (q=interpret_text; *p!='\0';
    number=isdigit((int) ((unsigned char) *p)) ? MagickTrue : MagickFalse,p++)
  {
    /*
      Interpret escape characters (e.g. Filename: %M).
    */
    *q='\0';
    ExtendInterpretText(MagickPathExtent);
    switch (*p)
    {
      case '\\':
      {
        switch (*(p+1))
        {
          case '\0':
            continue;
          case 'r':  /* convert to RETURN */
          {
            *q++='\r';
            p++;
            continue;
          }
          case 'n':  /* convert to NEWLINE */
          {
            *q++='\n';
            p++;
            continue;
          }
          case '\n':  /* EOL removal UNIX,MacOSX */
          {
            p++;
            continue;
          }
          case '\r':  /* EOL removal DOS,Windows */
          {
            p++;
            if (*p == '\n') /* return-newline EOL */
              p++;
            continue;
          }
          default:
          {
            p++;
            *q++=(*p);
          }
        }
        continue;
      }
      case '&':
      {
        if (LocaleNCompare("&lt;",p,4) == 0)
          {
            *q++='<';
            p+=3;
          }
        else
          if (LocaleNCompare("&gt;",p,4) == 0)
            {
              *q++='>';
              p+=3;
            }
          else
            if (LocaleNCompare("&amp;",p,5) == 0)
              {
                *q++='&';
                p+=4;
              }
            else
              *q++=(*p);
        continue;
      }
      case '%':
        break;  /* continue to next set of handlers */
      default:
      {
        *q++=(*p);  /* any thing else is 'as normal' */
        continue;
      }
    }
    p++; /* advance beyond the percent */
    /*
      Doubled Percent - or percent at end of string.
    */
    if ((*p == '\0') || (*p == '\'') || (*p == '"'))
      p--;
    if (*p == '%')
      {
        *q++='%';
        continue;
      }
    /*
      Single letter escapes %c.
    */
    if (number != MagickFalse)
      {
        /*
          But only if not preceded by a number!
        */
        *q++='%'; /* do NOT substitute the percent */
        p--;      /* back up one */
        continue;
      }
    string=GetMagickPropertyLetter(image_info,image,*p,exception);
    if (string != (char *) NULL)
      {
        AppendString2Text(string);
        string=DestroyString(string);
        continue;
      }
    (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
      "UnknownImageProperty","\"%%%c\"",*p);
  }
  *q='\0';
  return(interpret_text);
}

MagickExport char *GetDelegateCommand(const ImageInfo *image_info,Image *image,
  const char *decode,const char *encode,ExceptionInfo *exception)
{
  char
    *command,
    **commands;

  const DelegateInfo
    *delegate_info;

  ssize_t
    i;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
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
        ResourceLimitError,"MemoryAllocationFailed","`%s'",decode ? decode :
        encode);
      return((char *) NULL);
    }
  command=InterpretDelegateProperties((ImageInfo *) image_info,image,
    commands[0],exception);
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
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(delegate_info != (DelegateInfo *) NULL);
  assert(delegate_info->signature == MagickCoreSignature);
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
  const DelegateInfo
    *delegate_info;

  ElementInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  if (IsDelegateCacheInstantiated(exception) == MagickFalse)
    return((const DelegateInfo *) NULL);
  /*
    Search for named delegate.
  */
  delegate_info=(const DelegateInfo *) NULL;
  LockSemaphoreInfo(delegate_semaphore);
  p=GetHeadElementInLinkedList(delegate_cache);
  if ((LocaleCompare(decode,"*") == 0) && (LocaleCompare(encode,"*") == 0))
    {
      UnlockSemaphoreInfo(delegate_semaphore);
      if (p != (ElementInfo *) NULL)
        delegate_info=(const DelegateInfo* ) p->value;
      return(delegate_info);
    }
  while (p != (ElementInfo *) NULL)
  {
    delegate_info=(const DelegateInfo* ) p->value;
    if (delegate_info->mode > 0)
      {
        if (LocaleCompare(delegate_info->decode,decode) == 0)
          break;
        p=p->next;
        continue;
      }
    if (delegate_info->mode < 0)
      {
        if (LocaleCompare(delegate_info->encode,encode) == 0)
          break;
        p=p->next;
        continue;
      }
    if (LocaleCompare(decode,delegate_info->decode) == 0)
      if (LocaleCompare(encode,delegate_info->encode) == 0)
        break;
    if (LocaleCompare(decode,"*") == 0)
      if (LocaleCompare(encode,delegate_info->encode) == 0)
        break;
    if (LocaleCompare(decode,delegate_info->decode) == 0)
      if (LocaleCompare(encode,"*") == 0)
        break;
    p=p->next;
  }
  if (p == (ElementInfo *) NULL)
    delegate_info=(const DelegateInfo *) NULL;
  else
    SetHeadElementInLinkedList(delegate_cache,p);
  UnlockSemaphoreInfo(delegate_semaphore);
  return(delegate_info);
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

  int
    cmp;

  p=(const DelegateInfo **) x,
  q=(const DelegateInfo **) y;
  cmp=LocaleCompare((*p)->path,(*q)->path);
  if (cmp == 0)
    {
      if ((*p)->decode == (char *) NULL)
        if (((*p)->encode != (char *) NULL) &&
            ((*q)->encode != (char *) NULL))
          return(strcmp((*p)->encode,(*q)->encode));
      if (((*p)->decode != (char *) NULL) &&
          ((*q)->decode != (char *) NULL))
        return(strcmp((*p)->decode,(*q)->decode));
    }
  return(cmp);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport const DelegateInfo **GetDelegateInfoList(const char *pattern,
  size_t *number_delegates,ExceptionInfo *exception)
{
  const DelegateInfo
    **delegates;

  ElementInfo
    *p;

  ssize_t
    i;

  assert(number_delegates != (size_t *) NULL);
  assert(pattern != (char *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  *number_delegates=0;
  if (IsDelegateCacheInstantiated(exception) == MagickFalse)
    return((const DelegateInfo **) NULL);
  delegates=(const DelegateInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(delegate_cache)+1UL,sizeof(*delegates));
  if (delegates == (const DelegateInfo **) NULL)
    return((const DelegateInfo **) NULL);
  LockSemaphoreInfo(delegate_semaphore);
  p=GetHeadElementInLinkedList(delegate_cache);
  for (i=0; p != (ElementInfo *) NULL; )
  {
    const DelegateInfo
      *delegate_info;

    delegate_info=(const DelegateInfo *) p->value;
    if( (delegate_info->stealth == MagickFalse) &&
        (GlobExpression(delegate_info->decode,pattern,MagickFalse) != MagickFalse ||
         GlobExpression(delegate_info->encode,pattern,MagickFalse) != MagickFalse))
      delegates[i++]=delegate_info;
    p=p->next;
  }
  UnlockSemaphoreInfo(delegate_semaphore);
  if (i == 0)
    delegates=(const DelegateInfo **) RelinquishMagickMemory((void*) delegates);
  else
    {
      qsort((void *) delegates,(size_t) i,sizeof(*delegates),DelegateInfoCompare);
      delegates[i]=(DelegateInfo *) NULL;
    }
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
  const char
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

  ElementInfo
    *p;

  ssize_t
    i;

  assert(pattern != (char *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_delegates != (size_t *) NULL);
  *number_delegates=0;
  if (IsDelegateCacheInstantiated(exception) == MagickFalse)
    return((char **) NULL);
  delegates=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(delegate_cache)+1UL,sizeof(*delegates));
  if (delegates == (char **) NULL)
    return((char **) NULL);
  LockSemaphoreInfo(delegate_semaphore);
  p=GetHeadElementInLinkedList(delegate_cache);
  for (i=0; p != (ElementInfo *) NULL; )
  {
    const DelegateInfo
      *delegate_info;

    delegate_info=(const DelegateInfo *) p->value;
    if ((delegate_info->stealth == MagickFalse) &&
       (GlobExpression(delegate_info->decode,pattern,MagickFalse) != MagickFalse))
      delegates[i++]=ConstantString(delegate_info->decode);
    if ((delegate_info->stealth == MagickFalse) &&
        (GlobExpression(delegate_info->encode,pattern,MagickFalse) != MagickFalse))
      delegates[i++]=ConstantString(delegate_info->encode);
    p=p->next;
  }
  UnlockSemaphoreInfo(delegate_semaphore);
  if (i == 0)
    delegates=(char **) RelinquishMagickMemory(delegates);
  else
    {
      qsort((void *) delegates,(size_t) i,sizeof(*delegates),DelegateCompare);
      delegates[i]=(char *) NULL;
    }
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
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(delegate_info != (DelegateInfo *) NULL);
  assert(delegate_info->signature == MagickCoreSignature);
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
  assert(delegate_info != (DelegateInfo *) NULL);
  assert(delegate_info->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  return(delegate_info->thread_support);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s D e l e g a t e C a c h e I n s t a n t i a t e d                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsDelegateCacheInstantiated() determines if the delegate cache is
%  instantiated.  If not, it instantiates the cache and returns it.
%
%  The format of the IsDelegateInstantiated method is:
%
%      MagickBooleanType IsDelegateCacheInstantiated(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType IsDelegateCacheInstantiated(ExceptionInfo *exception)
{
  if (delegate_cache == (LinkedListInfo *) NULL)
    {
      if (delegate_semaphore == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&delegate_semaphore);
      LockSemaphoreInfo(delegate_semaphore);
      if (delegate_cache == (LinkedListInfo *) NULL)
        delegate_cache=AcquireDelegateCache(DelegateFilename,exception);
      UnlockSemaphoreInfo(delegate_semaphore);
    }
  return(delegate_cache != (LinkedListInfo *) NULL ? MagickTrue : MagickFalse);
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

static MagickBooleanType CopyDelegateFile(const char *source,
  const char *destination,const MagickBooleanType overwrite)
{
  int
    destination_file,
    source_file;

  MagickBooleanType
    status;

  ssize_t
    count,
    i;

  size_t
    length,
    quantum;

  struct stat
    attributes;

  unsigned char
    *buffer;

  /*
    Copy source file to destination.
  */
  assert(source != (const char *) NULL);
  assert(destination != (char *) NULL);
  if (overwrite == MagickFalse)
    {
      status=GetPathAttributes(destination,&attributes);
      if (status != MagickFalse)
        return(MagickTrue);
    }
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
  if ((fstat(source_file,&attributes) == 0) && (attributes.st_size > 0))
    quantum=MagickMin((size_t) attributes.st_size,MagickMaxBufferExtent);
  buffer=(unsigned char *) AcquireQuantumMemory(quantum,sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    {
      (void) close(source_file);
      (void) close(destination_file);
      return(MagickFalse);
    }
  length=0;
  for (i=0; ; i+=(ssize_t) count)
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
    input_filename[MagickPathExtent],
    output_filename[MagickPathExtent];

  const DelegateInfo
    *delegate_info;

  MagickBooleanType
    status,
    temporary;

  PolicyRights
    rights;

  ssize_t
    i;

  /*
    Get delegate.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  rights=ExecutePolicyRights;
  if ((decode != (const char *) NULL) &&
      (IsRightsAuthorized(DelegatePolicyDomain,rights,decode) == MagickFalse))
    {
      errno=EPERM;
      (void) ThrowMagickException(exception,GetMagickModule(),PolicyError,
        "NotAuthorized","`%s'",decode);
      return(MagickFalse);
    }
  if ((encode != (const char *) NULL) &&
      (IsRightsAuthorized(DelegatePolicyDomain,rights,encode) == MagickFalse))
    {
      errno=EPERM;
      (void) ThrowMagickException(exception,GetMagickModule(),PolicyError,
        "NotAuthorized","`%s'",encode);
      return(MagickFalse);
    }
  temporary=*image->filename == '\0' ? MagickTrue : MagickFalse;
  if ((temporary != MagickFalse) && (AcquireUniqueFilename(image->filename) ==
      MagickFalse))
    {
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        image->filename);
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

      Image
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
      magick=InterpretImageProperties(image_info,image,decode != (char *) NULL ?
        delegate_info->encode : delegate_info->decode,exception);
      if (magick == (char *) NULL)
        {
          (void) RelinquishUniqueFileResource(image_info->unique);
          if (temporary != MagickFalse)
            (void) RelinquishUniqueFileResource(image->filename);
          (void) ThrowMagickException(exception,GetMagickModule(),
            DelegateError,"DelegateFailed","`%s'",decode ? decode : encode);
          return(MagickFalse);
        }
      LocaleUpper(magick);
      clone_info=CloneImageInfo(image_info);
      (void) CopyMagickString((char *) clone_info->magick,magick,
        MagickPathExtent);
      if (LocaleCompare(magick,"NULL") != 0)
        (void) CopyMagickString(image->magick,magick,MagickPathExtent);
      magick=DestroyString(magick);
      (void) FormatLocaleString(clone_info->filename,MagickPathExtent,"%s:",
        delegate_info->decode);
      (void) SetImageInfo(clone_info,(unsigned int) GetImageListLength(image),
        exception);
      (void) CopyMagickString(clone_info->filename,image_info->filename,
        MagickPathExtent);
      (void) CopyMagickString(image_info->filename,image->filename,
        MagickPathExtent);
      for (p=image; p != (Image *) NULL; p=GetNextImageInList(p))
      {
        (void) FormatLocaleString(p->filename,MagickPathExtent,"%s:%s",
          delegate_info->decode,clone_info->filename);
        (void) SetImageOption(clone_info,"quantum:format","floating-point");
        status=WriteImage(clone_info,p,exception);
        if (status == MagickFalse)
          {
            (void) RelinquishUniqueFileResource(image_info->unique);
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
  status=MagickTrue;
  (void) CopyMagickString(output_filename,image_info->filename,
    MagickPathExtent);
  (void) CopyMagickString(input_filename,image->filename,MagickPathExtent);
  for (i=0; commands[i] != (char *) NULL; i++)
  {
    (void) AcquireUniqueSymbolicLink(output_filename,image_info->filename);
    if (AcquireUniqueFilename(image_info->unique) == MagickFalse)
      {
        ThrowFileException(exception,FileOpenError,
          "UnableToCreateTemporaryFile",image_info->unique);
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
    status=MagickTrue;
    command=InterpretDelegateProperties(image_info,image,commands[i],exception);
    if (command != (char *) NULL)
      {
        /*
          Execute delegate.
        */
        if (ExternalDelegateCommand(delegate_info->spawn,image_info->verbose,
          command,(char *) NULL,exception) != 0)
          status=MagickFalse;
        if (delegate_info->spawn != MagickFalse)
          {
            ssize_t
              count;

            /*
              Wait for input file to 'disappear', or maximum 2 seconds.
            */
            count=20;
            while ((count-- > 0) && (access_utf8(image->filename,F_OK) == 0))
              (void) MagickDelay(100);  /* sleep 0.1 seconds */
          }
        command=DestroyString(command);
      }
    if (LocaleCompare(decode,"SCAN") != 0)
      {
        if (CopyDelegateFile(image->filename,input_filename,MagickFalse) == MagickFalse)
          (void) RelinquishUniqueFileResource(input_filename);
      }
    if ((strcmp(input_filename,output_filename) != 0) &&
        (CopyDelegateFile(image_info->filename,output_filename,MagickTrue) == MagickFalse))
      (void) RelinquishUniqueFileResource(output_filename);
    if (image_info->temporary != MagickFalse)
      (void) RelinquishUniqueFileResource(image_info->filename);
    (void) RelinquishUniqueFileResource(image_info->unique);
    (void) RelinquishUniqueFileResource(image_info->filename);
    (void) RelinquishUniqueFileResource(image->filename);
    if (status == MagickFalse)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),DelegateError,
          "DelegateFailed","`%s'",commands[i]);
        break;
      }
    commands[i]=DestroyString(commands[i]);
  }
  (void) CopyMagickString(image_info->filename,output_filename,
    MagickPathExtent);
  (void) CopyMagickString(image->filename,input_filename,MagickPathExtent);
  /*
    Relinquish resources.
  */
  for ( ; commands[i] != (char *) NULL; i++)
    commands[i]=DestroyString(commands[i]);
  commands=(char **) RelinquishMagickMemory(commands);
  if (temporary != MagickFalse)
    (void) RelinquishUniqueFileResource(image->filename);
  return(status);
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
    delegate[MagickPathExtent];

  const char
    *path;

  ssize_t
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
      (void) CopyMagickString(delegate,delegate_info[i]->encode,
        MagickPathExtent);
    (void) ConcatenateMagickString(delegate,"        ",MagickPathExtent);
    delegate[8]='\0';
    commands=StringToList(delegate_info[i]->commands);
    if (commands == (char **) NULL)
      continue;
    (void) FormatLocaleFile(file,"%11s%c=%c%s  ",delegate_info[i]->decode ?
      delegate_info[i]->decode : "",delegate_info[i]->mode <= 0 ? '<' : ' ',
      delegate_info[i]->mode >= 0 ? '>' : ' ',delegate);
    (void) StripMagickString(commands[0]);
    (void) FormatLocaleFile(file,"\"%s\"\n",commands[0]);
    for (j=1; commands[j] != (char *) NULL; j++)
    {
      (void) StripMagickString(commands[j]);
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
+   L o a d D e l e g a t e C a c h e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadDelegateCache() loads the delegate configurations which provides a
%  mapping between delegate attributes and a delegate name.
%
%  The format of the LoadDelegateCache method is:
%
%      MagickBooleanType LoadDelegateCache(LinkedListInfo *cache,
%        const char *xml,const char *filename,const size_t depth,
%        ExceptionInfo *exception)
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
static MagickBooleanType LoadDelegateCache(LinkedListInfo *cache,
  const char *xml,const char *filename,const size_t depth,
  ExceptionInfo *exception)
{
  char
    keyword[MagickPathExtent],
    *token;

  const char
    *q;

  DelegateInfo
    *delegate_info;

  MagickStatusType
    status;

  size_t
    extent;

  /*
    Load the delegate map file.
  */
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading delegate configuration file \"%s\" ...",filename);
  if (xml == (const char *) NULL)
    return(MagickFalse);
  status=MagickTrue;
  delegate_info=(DelegateInfo *) NULL;
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
                      status&=(MagickStatusType) LoadDelegateCache(cache,
                        file_xml,path,depth+1,exception);
                      file_xml=DestroyString(file_xml);
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
        delegate_info=(DelegateInfo *) AcquireCriticalMemory(
          sizeof(*delegate_info));
        (void) memset(delegate_info,0,sizeof(*delegate_info));
        delegate_info->path=ConstantString(filename);
        delegate_info->thread_support=MagickTrue;
        delegate_info->signature=MagickCoreSignature;
        continue;
      }
    if (delegate_info == (DelegateInfo *) NULL)
      continue;
    if ((LocaleCompare(keyword,"/>") == 0) ||
        (LocaleCompare(keyword,"</policy>") == 0))
      {
        status=AppendValueToLinkedList(cache,delegate_info);
        if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            delegate_info->commands);
        delegate_info=(DelegateInfo *) NULL;
        continue;
      }
    (void) GetNextToken(q,(const char **) NULL,extent,token);
    if (*token != '=')
      continue;
    (void) GetNextToken(q,&q,extent,token);
    (void) GetNextToken(q,&q,extent,token);
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
                  path[MagickPathExtent];

                NTGhostscriptEXE(path,MagickPathExtent);
                (void) SubstituteString((char **) &commands,"@PSDelegate@",
                  path);
                (void) SubstituteString((char **) &commands,"\\","/");
              }
            (void) SubstituteString((char **) &commands,"&quot;","\"");
#else
            (void) SubstituteString((char **) &commands,"&quot;","'");
#endif
            (void) SubstituteString((char **) &commands,"&amp;","&");
            (void) SubstituteString((char **) &commands,"&gt;",">");
            (void) SubstituteString((char **) &commands,"&lt;","<");
            if (delegate_info->commands != (char *) NULL)
              delegate_info->commands=DestroyString(delegate_info->commands);
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
            delegate_info->spawn=IsStringTrue(token);
            break;
          }
        if (LocaleCompare((char *) keyword,"stealth") == 0)
          {
            delegate_info->stealth=IsStringTrue(token);
            break;
          }
        break;
      }
      case 'T':
      case 't':
      {
        if (LocaleCompare((char *) keyword,"thread-support") == 0)
          {
            delegate_info->thread_support=IsStringTrue(token);
            if (delegate_info->thread_support == MagickFalse)
              delegate_info->semaphore=AcquireSemaphoreInfo();
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

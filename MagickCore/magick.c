/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  M   M   AAA    GGGG  IIIII   CCCC  K   K                   %
%                  MM MM  A   A  G        I    C      K  K                    %
%                  M M M  AAAAA  G GGG    I    C      KKK                     %
%                  M   M  A   A  G   G    I    C      K  K                    %
%                  M   M  A   A   GGGG  IIIII   CCCC  K   K                   %
%                                                                             %
%                                                                             %
%               Methods to Read or List ImageMagick Image formats             %
%                                                                             %
%                            Software Design                                  %
%                            Bob Friesenhahn                                  %
%                                 Cristy                                      %
%                             November 1998                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
#include "MagickCore/annotate-private.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/coder-private.h"
#include "MagickCore/client.h"
#include "MagickCore/color-private.h"
#include "MagickCore/configure-private.h"
#include "MagickCore/constitute-private.h"
#include "MagickCore/delegate-private.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/locale-private.h"
#include "MagickCore/log-private.h"
#include "MagickCore/magic-private.h"
#include "MagickCore/magick.h"
#include "MagickCore/magick-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/mime-private.h"
#include "MagickCore/module.h"
#include "MagickCore/module-private.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/nt-feature.h"
#include "MagickCore/opencl-private.h"
#include "MagickCore/option-private.h"
#include "MagickCore/random-private.h"
#include "MagickCore/registry.h"
#include "MagickCore/registry-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/resource-private.h"
#include "MagickCore/policy.h"
#include "MagickCore/policy-private.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/semaphore-private.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/type-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/xwindow-private.h"

/*
  Define declarations.
*/
#if !defined(MAGICKCORE_RETSIGTYPE)
# define MAGICKCORE_RETSIGTYPE  void
#endif
#if !defined(SIG_DFL)
# define SIG_DFL  ((SignalHandler *) 0)
#endif
#if !defined(SIG_ERR)
# define SIG_ERR  ((SignalHandler *) -1)
#endif
#if !defined(SIGMAX)
#define SIGMAX  64
#endif

/*
  Typedef declarations.
*/
typedef MAGICKCORE_RETSIGTYPE
  SignalHandler(int);

/*
  Global declarations.
*/
static SemaphoreInfo
  *magick_semaphore = (SemaphoreInfo *) NULL;

static SignalHandler
  *signal_handlers[SIGMAX] = { (SignalHandler *) NULL };

static SplayTreeInfo
  *magick_list = (SplayTreeInfo *) NULL;

static volatile MagickBooleanType
  instantiate_magickcore = MagickFalse,
  magickcore_signal_in_progress = MagickFalse;

/*
  Forward declarations.
*/
static MagickBooleanType
  IsMagickTreeInstantiated(ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    A c q u i r e M a g i c k I n f o                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireMagickInfo() allocates a MagickInfo structure and initializes the
%  members to default values.
%
%  The format of the AcquireMagickInfo method is:
%
%      MagickInfo *AcquireMagickInfo(const char *module, const char *name,)
%
%  A description of each parameter follows:
%
%    o module: a character string that represents the module associated
%      with the MagickInfo structure.
%
%    o name: a character string that represents the image format associated
%      with the MagickInfo structure.
%
%    o description: a character string that represents the image format
%      associated with the MagickInfo structure.
%
*/
MagickExport MagickInfo *AcquireMagickInfo(const char *module,
  const char *name, const char *description)
{
  MagickInfo
    *magick_info;

  assert(module != (const char *) NULL);
  assert(name != (const char *) NULL);
  assert(description != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",name);
  magick_info=(MagickInfo *) AcquireMagickMemory(sizeof(*magick_info));
  if (magick_info == (MagickInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(magick_info,0,sizeof(*magick_info));
  magick_info->module=ConstantString(module);
  magick_info->name=ConstantString(name);
  magick_info->description=ConstantString(description);
  magick_info->flags=CoderAdjoinFlag | CoderBlobSupportFlag |
    CoderDecoderThreadSupportFlag | CoderEncoderThreadSupportFlag |
    CoderUseExtensionFlag;
  magick_info->signature=MagickCoreSignature;
  return(magick_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t I m a g e D e c o d e r                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageDecoder() returns the image decoder.
%
%  The format of the GetImageDecoder method is:
%
%      DecodeImageHandler *GetImageDecoder(const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport DecodeImageHandler *GetImageDecoder(const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  return(magick_info->decoder);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t I m a g e E n c o d e r                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageEncoder() returns the image encoder.
%
%  The format of the GetImageEncoder method is:
%
%      EncodeImageHandler *GetImageEncoder(const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport EncodeImageHandler *GetImageEncoder(const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  return(magick_info->encoder);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t I m a g e M a g i c k                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageMagick() searches for an image format that matches the specified
%  magick string.  If one is found, MagickTrue is returned otherwise
%  MagickFalse.
%
%  The format of the GetImageMagick method is:
%
%      MagickBooleanType GetImageMagick(const unsigned char *magick,
%        const size_t length,char *format)
%
%  A description of each parameter follows:
%
%    o magick: the image format we are searching for.
%
%    o length: the length of the binary string.
%
%    o format: the image format as determined by the magick bytes.
%
*/
MagickExport MagickBooleanType GetImageMagick(const unsigned char *magick,
  const size_t length,char *format)
{
  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  register const MagickInfo
    *p;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(magick != (const unsigned char *) NULL);
  exception=AcquireExceptionInfo();
  p=GetMagickInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  if (p == (const MagickInfo *) NULL)
    return(MagickFalse);
  status=MagickFalse;
  LockSemaphoreInfo(magick_semaphore);
  ResetSplayTreeIterator(magick_list);
  p=(const MagickInfo *) GetNextValueInSplayTree(magick_list);
  while (p != (const MagickInfo *) NULL)
  {
    if ((p->magick != (IsImageFormatHandler *) NULL) &&
        (p->magick(magick,length) != 0))
      {
        status=MagickTrue;
        (void) CopyMagickString(format,p->name,MagickPathExtent);
        break;
      }
    p=(const MagickInfo *) GetNextValueInSplayTree(magick_list);
  }
  UnlockSemaphoreInfo(magick_semaphore);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k A d j o i n                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickAdjoin() returns MagickTrue if the magick adjoin is MagickTrue.
%
%  The format of the GetMagickAdjoin method is:
%
%      MagickBooleanType GetMagickAdjoin(const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport MagickBooleanType GetMagickAdjoin(const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  return(((magick_info->flags & CoderAdjoinFlag) == 0) ? MagickFalse :
    MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k B l o b S u p p o r t                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickBlobSupport() returns MagickTrue if the magick supports blobs.
%
%  The format of the GetMagickBlobSupport method is:
%
%      MagickBooleanType GetMagickBlobSupport(const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport MagickBooleanType GetMagickBlobSupport(
  const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  return(((magick_info->flags & CoderBlobSupportFlag) == 0) ? MagickFalse :
    MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k D e c o d e r S e e k a b l e S t r e a m               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickDecoderSeekableStream() returns MagickTrue if the magick supports a
%  seekable stream in the decoder.
%
%  The format of the GetMagickDecoderSeekableStream method is:
%
%      MagickBooleanType GetMagickDecoderSeekableStream(
%        const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport MagickBooleanType GetMagickDecoderSeekableStream(
  const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  if ((magick_info->flags & CoderDecoderSeekableStreamFlag) == 0)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k D e c o d e r T h r e a d S u p p o r t                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickDecoderThreadSupport() returns MagickTrue if the decoder supports
%  threads.
%
%  The format of the GetMagickDecoderThreadSupport method is:
%
%      MagickStatusType GetMagickDecoderThreadSupport(
%        const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport MagickBooleanType GetMagickDecoderThreadSupport(
  const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  return(((magick_info->flags & CoderDecoderThreadSupportFlag) == 0) ?
    MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k D e s c r i p t i o n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickDescription() returns the magick description.
%
%  The format of the GetMagickDescription method is:
%
%      const char *GetMagickDescription(const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport const char *GetMagickDescription(const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  return(magick_info->description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k E n c o d e r S e e k a b l e S t r e a m               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickEncoderSeekableStream() returns MagickTrue if the magick supports a
%  seekable stream in the encoder.
%
%  The format of the GetMagickEncoderSeekableStream method is:
%
%      MagickBooleanType GetMagickEncoderSeekableStream(
%        const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport MagickBooleanType GetMagickEncoderSeekableStream(
  const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  if ((magick_info->flags & CoderEncoderSeekableStreamFlag) == 0)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k E n c o d e r T h r e a d S u p p o r t                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickEncoderThreadSupport() returns MagickTrue if the encoder supports
%  threads.
%
%  The format of the GetMagickEncoderThreadSupport method is:
%
%      MagickStatusType GetMagickEncoderThreadSupport(
%        const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport MagickBooleanType GetMagickEncoderThreadSupport(
  const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  return(((magick_info->flags & CoderDecoderThreadSupportFlag) == 0) ?
    MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k E n d i a n S u p p o r t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickEndianSupport() returns the MagickTrue if the coder respects
%  endianness other than MSBEndian.
%
%  The format of the GetMagickEndianSupport method is:
%
%      MagickBooleanType GetMagickEndianSupport(const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport MagickBooleanType GetMagickEndianSupport(
  const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  return(((magick_info->flags & CoderEndianSupportFlag) == 0) ? MagickFalse :
    MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k I n f o                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickInfo() returns a pointer MagickInfo structure that matches
%  the specified name.  If name is NULL, the head of the image format list
%  is returned.
%
%  The format of the GetMagickInfo method is:
%
%      const MagickInfo *GetMagickInfo(const char *name,Exception *exception)
%
%  A description of each parameter follows:
%
%    o name: the image format we are looking for.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const MagickInfo *GetMagickInfo(const char *name,
  ExceptionInfo *exception)
{
  register const MagickInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  if (IsMagickTreeInstantiated(exception) == MagickFalse)
    return((const MagickInfo *) NULL);
#if defined(MAGICKCORE_MODULES_SUPPORT)
  if ((name != (const char *) NULL) && (LocaleCompare(name,"*") == 0))
    (void) OpenModules(exception);
#endif
  /*
    Find name in list.
  */
  LockSemaphoreInfo(magick_semaphore);
  ResetSplayTreeIterator(magick_list);
  p=(const MagickInfo *) GetNextValueInSplayTree(magick_list);
  if ((name == (const char *) NULL) || (LocaleCompare(name,"*") == 0))
    {
      ResetSplayTreeIterator(magick_list);
      p=(const MagickInfo *) GetNextValueInSplayTree(magick_list);
      UnlockSemaphoreInfo(magick_semaphore);
      return(p);
    }
  while (p != (const MagickInfo *) NULL)
  {
    if (LocaleCompare(p->name,name) == 0)
      break;
    p=(const MagickInfo *) GetNextValueInSplayTree(magick_list);
  }
#if defined(MAGICKCORE_MODULES_SUPPORT)
  if (p == (const MagickInfo *) NULL)
    {
      if (*name != '\0')
        (void) OpenModule(name,exception);
      ResetSplayTreeIterator(magick_list);
      p=(const MagickInfo *) GetNextValueInSplayTree(magick_list);
      while (p != (const MagickInfo *) NULL)
      {
        if (LocaleCompare(p->name,name) == 0)
          break;
        p=(const MagickInfo *) GetNextValueInSplayTree(magick_list);
      }
    }
#endif
  UnlockSemaphoreInfo(magick_semaphore);
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k I n f o L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickInfoList() returns any image formats that match the specified
%  pattern.
%
%  The format of the GetMagickInfoList function is:
%
%      const MagickInfo **GetMagickInfoList(const char *pattern,
%        size_t *number_formats,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_formats:  This integer returns the number of formats in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int MagickInfoCompare(const void *x,const void *y)
{
  const MagickInfo
    **p,
    **q;

  p=(const MagickInfo **) x,
  q=(const MagickInfo **) y;
  return(LocaleCompare((*p)->name,(*q)->name));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport const MagickInfo **GetMagickInfoList(const char *pattern,
  size_t *number_formats,ExceptionInfo *exception)
{
  const MagickInfo
    **formats;

  register const MagickInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate magick list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_formats != (size_t *) NULL);
  *number_formats=0;
  p=GetMagickInfo("*",exception);
  if (p == (const MagickInfo *) NULL)
    return((const MagickInfo **) NULL);
  formats=(const MagickInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfNodesInSplayTree(magick_list)+1UL,sizeof(*formats));
  if (formats == (const MagickInfo **) NULL)
    return((const MagickInfo **) NULL);
  /*
    Generate magick list.
  */
  LockSemaphoreInfo(magick_semaphore);
  ResetSplayTreeIterator(magick_list);
  p=(const MagickInfo *) GetNextValueInSplayTree(magick_list);
  for (i=0; p != (const MagickInfo *) NULL; )
  {
    if ((GetMagickStealth(p) == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      formats[i++]=p;
    p=(const MagickInfo *) GetNextValueInSplayTree(magick_list);
  }
  UnlockSemaphoreInfo(magick_semaphore);
  qsort((void *) formats,(size_t) i,sizeof(*formats),MagickInfoCompare);
  formats[i]=(MagickInfo *) NULL;
  *number_formats=(size_t) i;
  return(formats);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k L i s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickList() returns any image formats that match the specified pattern.
%
%  The format of the GetMagickList function is:
%
%      char **GetMagickList(const char *pattern,size_t *number_formats,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_formats:  This integer returns the number of formats in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int MagickCompare(const void *x,const void *y)
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

MagickExport char **GetMagickList(const char *pattern,
  size_t *number_formats,ExceptionInfo *exception)
{
  char
    **formats;

  register const MagickInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate magick list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_formats != (size_t *) NULL);
  *number_formats=0;
  p=GetMagickInfo("*",exception);
  if (p == (const MagickInfo *) NULL)
    return((char **) NULL);
  formats=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfNodesInSplayTree(magick_list)+1UL,sizeof(*formats));
  if (formats == (char **) NULL)
    return((char **) NULL);
  LockSemaphoreInfo(magick_semaphore);
  ResetSplayTreeIterator(magick_list);
  p=(const MagickInfo *) GetNextValueInSplayTree(magick_list);
  for (i=0; p != (const MagickInfo *) NULL; )
  {
    if ((GetMagickStealth(p) == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      formats[i++]=ConstantString(p->name);
    p=(const MagickInfo *) GetNextValueInSplayTree(magick_list);
  }
  UnlockSemaphoreInfo(magick_semaphore);
  qsort((void *) formats,(size_t) i,sizeof(*formats),MagickCompare);
  formats[i]=(char *) NULL;
  *number_formats=(size_t) i;
  return(formats);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k M i m e T y p e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickMimeType() returns the magick mime type.
%
%  The format of the GetMagickMimeType method is:
%
%      const char *GetMagickMimeType(const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport const char *GetMagickMimeType(const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  return(magick_info->mime_type);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k P r e c i s i o n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickPrecision() returns the maximum number of significant digits to be
%  printed.
%
%  The format of the GetMagickPrecision method is:
%
%      int GetMagickPrecision(void)
%
*/
MagickExport int GetMagickPrecision(void)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  return(SetMagickPrecision(0));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k R a w S u p p o r t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickRawSupport() returns the MagickTrue if the coder is a raw format.
%
%  The format of the GetMagickRawSupport method is:
%
%      MagickBooleanType GetMagickRawSupport(const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport MagickBooleanType GetMagickRawSupport(
  const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  return(((magick_info->flags & CoderRawSupportFlag) == 0) ? MagickFalse :
    MagickTrue);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k S t e a l t h                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickStealth() returns MagickTrue if the magick is a stealth coder.
%
%  The format of the GetMagickStealth method is:
%
%      MagickBooleanType GetMagickStealth(const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport MagickBooleanType GetMagickStealth(const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  return(((magick_info->flags & CoderStealthFlag) == 0) ? MagickFalse :
    MagickTrue);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k U s e E x t e n s i o n                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickUseExtension() returns MagickTrue if the magick can use the
%  extension of the format if the format return by IsImageFormatHandler uses
%  the same coder.
%
%  The format of the GetMagickUseExtension method is:
%
%      MagickBooleanType GetMagickUseExtension(const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport MagickBooleanType GetMagickUseExtension(
  const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  return(((magick_info->flags & CoderUseExtensionFlag) == 0) ? MagickFalse :
    MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s M a g i c k T r e e I n s t a n t i a t e d                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMagickTreeInstantiated() determines if the magick tree is instantiated.
%  If not, it instantiates the tree and returns it.
%
%  The format of the IsMagickTreeInstantiated() method is:
%
%      IsMagickTreeInstantiated(Exceptioninfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void *DestroyMagickNode(void *magick_info)
{
  register MagickInfo
    *p;

  p=(MagickInfo *) magick_info;
  if (p->module != (char *) NULL)
    p->module=DestroyString(p->module);
  if (p->note != (char *) NULL)
    p->note=DestroyString(p->note);
  if (p->mime_type != (char *) NULL)
    p->mime_type=DestroyString(p->mime_type);
  if (p->version != (char *) NULL)
    p->version=DestroyString(p->version);
  if (p->description != (char *) NULL)
    p->description=DestroyString(p->description);
  if (p->name != (char *) NULL)
    p->name=DestroyString(p->name);
  if (p->semaphore != (SemaphoreInfo *) NULL)
    RelinquishSemaphoreInfo(&p->semaphore);
  return(RelinquishMagickMemory(p));
}

static MagickBooleanType IsMagickTreeInstantiated(ExceptionInfo *exception)
{
  (void) exception;
  if (magick_list == (SplayTreeInfo *) NULL)
    {
      if (magick_semaphore == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&magick_semaphore);
      LockSemaphoreInfo(magick_semaphore);
      if (magick_list == (SplayTreeInfo *) NULL)
        {
          magick_list=NewSplayTree(CompareSplayTreeString,(void *(*)(void *))
            NULL,DestroyMagickNode);
          if (magick_list == (SplayTreeInfo *) NULL)
            ThrowFatalException(ResourceLimitFatalError,
              "MemoryAllocationFailed");
#if defined(MAGICKCORE_MODULES_SUPPORT)
          (void) GetModuleInfo((char *) NULL,exception);
#endif
#if !defined(MAGICKCORE_BUILD_MODULES)
          RegisterStaticModules();
#endif
        }
      UnlockSemaphoreInfo(magick_semaphore);
    }
  return(magick_list != (SplayTreeInfo *) NULL ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s M a g i c k C o n f l i c t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMagickConflict() returns MagickTrue if the image format conflicts with a
%  logical drive (.e.g. X:).
%
%  The format of the IsMagickConflict method is:
%
%      MagickBooleanType IsMagickConflict(const char *magick)
%
%  A description of each parameter follows:
%
%    o magick: Specifies the image format.
%
*/
MagickPrivate MagickBooleanType IsMagickConflict(const char *magick)
{
  assert(magick != (char *) NULL);
#if defined(macintosh)
  return(MACIsMagickConflict(magick));
#elif defined(vms)
  return(VMSIsMagickConflict(magick));
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  return(NTIsMagickConflict(magick));
#else
  return(MagickFalse);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  L i s t M a g i c k I n f o                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListMagickInfo() lists the image formats to a file.
%
%  The format of the ListMagickInfo method is:
%
%      MagickBooleanType ListMagickInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file: A file handle.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListMagickInfo(FILE *file,
  ExceptionInfo *exception)
{
  const MagickInfo
    **magick_info;

  register ssize_t
    i;

  size_t
    number_formats;

  ssize_t
    j;

  if (file == (FILE *) NULL)
    file=stdout;
  magick_info=GetMagickInfoList("*",&number_formats,exception);
  if (magick_info == (const MagickInfo **) NULL)
    return(MagickFalse);
  ClearMagickException(exception);
#if !defined(MAGICKCORE_MODULES_SUPPORT)
  (void) FormatLocaleFile(file,"   Format  Mode  Description\n");
#else
  (void) FormatLocaleFile(file,"   Format  Module    Mode  Description\n");
#endif
  (void) FormatLocaleFile(file,
    "--------------------------------------------------------"
    "-----------------------\n");
  for (i=0; i < (ssize_t) number_formats; i++)
  {
    if (GetMagickStealth(magick_info[i]) != MagickFalse)
      continue;
    (void) FormatLocaleFile(file,"%9s%c ",
      magick_info[i]->name != (char *) NULL ? magick_info[i]->name : "",
      GetMagickBlobSupport(magick_info[i]) != MagickFalse ? '*' : ' ');
#if defined(MAGICKCORE_MODULES_SUPPORT)
    {
      char
        module[MagickPathExtent];

      *module='\0';
      if (magick_info[i]->module != (char *) NULL)
        (void) CopyMagickString(module,magick_info[i]->module,MagickPathExtent);
      (void) ConcatenateMagickString(module,"          ",MagickPathExtent);
      module[9]='\0';
      (void) FormatLocaleFile(file,"%9s ",module);
    }
#endif
    (void) FormatLocaleFile(file,"%c%c%c ",magick_info[i]->decoder ? 'r' : '-',
      magick_info[i]->encoder ? 'w' : '-',magick_info[i]->encoder != NULL &&
      GetMagickAdjoin(magick_info[i]) != MagickFalse ? '+' : '-');
    if (magick_info[i]->description != (char *) NULL)
      (void) FormatLocaleFile(file,"  %s",magick_info[i]->description);
    if (magick_info[i]->version != (char *) NULL)
      (void) FormatLocaleFile(file," (%s)",magick_info[i]->version);
    (void) FormatLocaleFile(file,"\n");
    if (magick_info[i]->note != (char *) NULL)
      {
        char
          **text;

        text=StringToList(magick_info[i]->note);
        if (text != (char **) NULL)
          {
            for (j=0; text[j] != (char *) NULL; j++)
            {
              (void) FormatLocaleFile(file,"           %s\n",text[j]);
              text[j]=DestroyString(text[j]);
            }
            text=(char **) RelinquishMagickMemory(text);
          }
      }
  }
  (void) FormatLocaleFile(file,"\n* native blob support\n");
  (void) FormatLocaleFile(file,"r read support\n");
  (void) FormatLocaleFile(file,"w write support\n");
  (void) FormatLocaleFile(file,"+ support for multiple images\n");
  (void) fflush(file);
  magick_info=(const MagickInfo **) RelinquishMagickMemory((void *)
    magick_info);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I s M a g i c k C o r e I n s t a n t i a t e d                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMagickCoreInstantiated() returns MagickTrue if the ImageMagick environment
%  is currently instantiated:  MagickCoreGenesis() has been called but
%  MagickDestroy() has not.
%
%  The format of the IsMagickCoreInstantiated method is:
%
%      MagickBooleanType IsMagickCoreInstantiated(void)
%
*/
MagickExport MagickBooleanType IsMagickCoreInstantiated(void)
{
  return(instantiate_magickcore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k C o m p o n e n t G e n e s i s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickComponentGenesis() instantiates the magick component.
%
%  The format of the MagickComponentGenesis method is:
%
%      MagickBooleanType MagickComponentGenesis(void)
%
*/
MagickPrivate MagickBooleanType MagickComponentGenesis(void)
{
  if (magick_semaphore == (SemaphoreInfo *) NULL)
    magick_semaphore=AcquireSemaphoreInfo();
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k C o m p o n e n t T e r m i n u s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickComponentTerminus() destroys the magick component.
%
%  The format of the MagickComponentTerminus method is:
%
%      void MagickComponentTerminus(void)
%
*/
MagickPrivate void MagickComponentTerminus(void)
{
  if (magick_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&magick_semaphore);
  LockSemaphoreInfo(magick_semaphore);
  if (magick_list != (SplayTreeInfo *) NULL)
    magick_list=DestroySplayTree(magick_list);
  UnlockSemaphoreInfo(magick_semaphore);
  RelinquishSemaphoreInfo(&magick_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o r e G e n e s i s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCoreGenesis() initializes the MagickCore environment.
%
%  The format of the MagickCoreGenesis function is:
%
%      MagickCoreGenesis(const char *path,
%        const MagickBooleanType establish_signal_handlers)
%
%  A description of each parameter follows:
%
%    o path: the execution path of the current ImageMagick client.
%
%    o establish_signal_handlers: set to MagickTrue to use MagickCore's own
%      signal handlers for common signals.
%
*/

static SignalHandler *SetMagickSignalHandler(int signal_number,
  SignalHandler *handler)
{
#if defined(MAGICKCORE_HAVE_SIGACTION) && defined(MAGICKCORE_HAVE_SIGEMPTYSET)
  int
    status;

  sigset_t
    mask;

  struct sigaction
    action,
    previous_action;

  sigemptyset(&mask);
  sigaddset(&mask,signal_number);
  sigprocmask(SIG_BLOCK,&mask,NULL);
  action.sa_mask=mask;
  action.sa_handler=handler;
  action.sa_flags=0;
#if defined(SA_INTERRUPT)
  action.sa_flags|=SA_INTERRUPT;
#endif
  status=sigaction(signal_number,&action,&previous_action);
  if (status < 0)
    return(SIG_ERR);
  sigprocmask(SIG_UNBLOCK,&mask,NULL);
  return(previous_action.sa_handler);
#else
  return(signal(signal_number,handler));
#endif
}

static void MagickSignalHandler(int signal_number)
{
  if (magickcore_signal_in_progress != MagickFalse)
    (void) SetMagickSignalHandler(signal_number,signal_handlers[signal_number]);
  magickcore_signal_in_progress=MagickTrue;
  AsynchronousResourceComponentTerminus();
#if defined(SIGQUIT)
  if (signal_number == SIGQUIT)
    abort();
#endif
#if defined(SIGABRT)
  if (signal_number == SIGABRT)
    abort();
#endif
#if defined(SIGFPE)
  if (signal_number == SIGFPE)
    abort();
#endif
#if defined(SIGXCPU)
  if (signal_number == SIGXCPU)
    abort();
#endif
#if defined(SIGXFSZ)
  if (signal_number == SIGXFSZ)
    abort();
#endif
#if defined(SIGSEGV)
  if (signal_number == SIGSEGV)
    abort();
#endif
#if !defined(MAGICKCORE_HAVE__EXIT)
  exit(signal_number);
#else
#if defined(SIGHUP)
  if (signal_number == SIGHUP)
    _exit(signal_number);
#endif
#if defined(SIGINT)
  if (signal_number == SIGINT)
    _exit(signal_number);
#endif
#if defined(SIGTERM)
  if (signal_number == SIGTERM)
    _exit(signal_number);
#endif
#if defined(MAGICKCORE_HAVE_RAISE)
  if (signal_handlers[signal_number] != MagickSignalHandler)
    raise(signal_number);
#endif
  _exit(signal_number);  /* do not invoke registered atexit() methods */
#endif
}

static SignalHandler *RegisterMagickSignalHandler(int signal_number)
{
  SignalHandler
    *handler;

  handler=SetMagickSignalHandler(signal_number,MagickSignalHandler);
  if (handler == SIG_ERR)
    return(handler);
  if (handler != SIG_DFL)
    handler=SetMagickSignalHandler(signal_number,handler);
  else
    (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
      "Register handler for signal: %d",signal_number);
  return(handler);
}

MagickExport void MagickCoreGenesis(const char *path,
  const MagickBooleanType establish_signal_handlers)
{
  char
    *events,
    execution_path[MagickPathExtent],
    filename[MagickPathExtent];

  /*
    Initialize the Magick environment.
  */
  InitializeMagickMutex();
  LockMagickMutex();
  if (instantiate_magickcore != MagickFalse)
    {
      UnlockMagickMutex();
      return;
    }
  (void) SemaphoreComponentGenesis();
  (void) LogComponentGenesis();
  (void) LocaleComponentGenesis();
  (void) RandomComponentGenesis();
  events=GetEnvironmentValue("MAGICK_DEBUG");
  if (events != (char *) NULL)
    {
      (void) SetLogEventMask(events);
      events=DestroyString(events);
    }
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  NTWindowsGenesis();
#endif
  /*
    Set client name and execution path.
  */
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  if ((path != (const char *) NULL) && (IsPathAccessible(path) != MagickFalse))
#else
  if ((path != (const char *) NULL) && (*path == *DirectorySeparator) &&
      (IsPathAccessible(path) != MagickFalse))
#endif
    (void) CopyMagickString(execution_path,path,MagickPathExtent);
  else
    (void) GetExecutionPath(execution_path,MagickPathExtent);
  GetPathComponent(execution_path,TailPath,filename);
  (void) SetClientName(filename);
  GetPathComponent(execution_path,HeadPath,execution_path);
  (void) SetClientPath(execution_path);
  if (establish_signal_handlers != MagickFalse)
    {
      /*
        Set signal handlers.
      */
#if defined(SIGABRT)
      if (signal_handlers[SIGABRT] == (SignalHandler *) NULL)
        signal_handlers[SIGABRT]=RegisterMagickSignalHandler(SIGABRT);
#endif
#if defined(SIGSEGV)
      if (signal_handlers[SIGSEGV] == (SignalHandler *) NULL)
        signal_handlers[SIGSEGV]=RegisterMagickSignalHandler(SIGSEGV);
#endif
#if defined(SIGFPE)
      if (signal_handlers[SIGFPE] == (SignalHandler *) NULL)
        signal_handlers[SIGFPE]=RegisterMagickSignalHandler(SIGFPE);
#endif
#if defined(SIGHUP)
      if (signal_handlers[SIGHUP] == (SignalHandler *) NULL)
        signal_handlers[SIGHUP]=RegisterMagickSignalHandler(SIGHUP);
#endif
#if defined(SIGINT)
      if (signal_handlers[SIGINT] == (SignalHandler *) NULL)
        signal_handlers[SIGINT]=RegisterMagickSignalHandler(SIGINT);
#endif
#if defined(SIGQUIT)
      if (signal_handlers[SIGQUIT] == (SignalHandler *) NULL)
        signal_handlers[SIGQUIT]=RegisterMagickSignalHandler(SIGQUIT);
#endif
#if defined(SIGTERM)
      if (signal_handlers[SIGTERM] == (SignalHandler *) NULL)
        signal_handlers[SIGTERM]=RegisterMagickSignalHandler(SIGTERM);
#endif
#if defined(SIGXCPU)
      if (signal_handlers[SIGXCPU] == (SignalHandler *) NULL)
        signal_handlers[SIGXCPU]=RegisterMagickSignalHandler(SIGXCPU);
#endif
#if defined(SIGXFSZ)
      if (signal_handlers[SIGXFSZ] == (SignalHandler *) NULL)
        signal_handlers[SIGXFSZ]=RegisterMagickSignalHandler(SIGXFSZ);
#endif
    }
  /*
    Instantiate magick resources.
  */
  (void) ConfigureComponentGenesis();
  (void) PolicyComponentGenesis();
  (void) CacheComponentGenesis();
  (void) ResourceComponentGenesis();
  (void) CoderComponentGenesis();
  (void) MagickComponentGenesis();
#if defined(MAGICKCORE_MODULES_SUPPORT)
  (void) ModuleComponentGenesis();
#endif
  (void) DelegateComponentGenesis();
  (void) MagicComponentGenesis();
  (void) ColorComponentGenesis();
  (void) TypeComponentGenesis();
  (void) MimeComponentGenesis();
  (void) AnnotateComponentGenesis();
#if defined(MAGICKCORE_X11_DELEGATE)
  (void) XComponentGenesis();
#endif
  (void) RegistryComponentGenesis();
  instantiate_magickcore=MagickTrue;
  UnlockMagickMutex();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o r e T e r m i n u s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCoreTerminus() destroys the MagickCore environment.
%
%  The format of the MagickCoreTerminus function is:
%
%      MagickCoreTerminus(void)
%
*/
MagickExport void MagickCoreTerminus(void)
{
  InitializeMagickMutex();
  LockMagickMutex();
  if (instantiate_magickcore == MagickFalse)
    {
      UnlockMagickMutex();
      return;
    }
  RegistryComponentTerminus();
#if defined(MAGICKCORE_X11_DELEGATE)
  XComponentTerminus();
#endif
  AnnotateComponentTerminus();
  MimeComponentTerminus();
  TypeComponentTerminus();
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  OpenCLTerminus();
#endif
  ColorComponentTerminus();
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  NTWindowsTerminus();
#endif
  MagicComponentTerminus();
  DelegateComponentTerminus();
  MagickComponentTerminus();
#if !defined(MAGICKCORE_BUILD_MODULES)
  UnregisterStaticModules();
#endif
#if defined(MAGICKCORE_MODULES_SUPPORT)
  ModuleComponentTerminus();
#endif
  CoderComponentTerminus();
  ResourceComponentTerminus();
  CacheComponentTerminus();
  PolicyComponentTerminus();
  ConfigureComponentTerminus();
  RandomComponentTerminus();
  LocaleComponentTerminus();
  LogComponentTerminus();
  instantiate_magickcore=MagickFalse;
  UnlockMagickMutex();
  SemaphoreComponentTerminus();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e g i s t e r M a g i c k I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterMagickInfo() adds attributes for a particular image format to the
%  list of supported formats.  The attributes include the image format name,
%  a method to read and/or write the format, whether the format supports the
%  saving of more than one frame to the same file or blob, whether the format
%  supports native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterMagickInfo method is:
%
%      MagickInfo *RegisterMagickInfo(MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info: the magick info.
%
*/
MagickExport MagickBooleanType RegisterMagickInfo(MagickInfo *magick_info)
{
  MagickBooleanType
    status;

  /*
    Register a new image format.
  */
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",magick_info->name);
  if (magick_list == (SplayTreeInfo *) NULL)
    return(MagickFalse);
  if ((GetMagickDecoderThreadSupport(magick_info) == MagickFalse) ||
      (GetMagickEncoderThreadSupport(magick_info) == MagickFalse))
    magick_info->semaphore=AcquireSemaphoreInfo();
  status=AddValueToSplayTree(magick_list,magick_info->name,magick_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t M a g i c k P r e c i s i o n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetMagickPrecision() sets the maximum number of significant digits to be
%  printed.
%
%  An input argument of 0 returns the current precision setting.
%
%  A negative value forces the precision to reset to a default value according
%  to the environment variable "MAGICK_PRECISION", the current 'policy'
%  configuration setting, or the default value of '6', in that order.
%
%  The format of the SetMagickPrecision method is:
%
%      int SetMagickPrecision(const int precision)
%
%  A description of each parameter follows:
%
%    o precision: set the maximum number of significant digits to be printed.
%
*/
MagickExport int SetMagickPrecision(const int precision)
{
#define MagickPrecision  6

  static int
    magick_precision = 0;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (precision > 0)
    magick_precision=precision;
  if ((precision < 0) || (magick_precision == 0))
    {
      char
        *limit;

      /*
        Precision reset, or it has not been set yet
      */
      magick_precision=MagickPrecision;
      limit=GetEnvironmentValue("MAGICK_PRECISION");
      if (limit == (char *) NULL)
        limit=GetPolicyValue("system:precision");
      if (limit != (char *) NULL)
        {
          magick_precision=StringToInteger(limit);
          limit=DestroyString(limit);
        }
    }
  return(magick_precision);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   U n r e g i s t e r M a g i c k I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterMagickInfo() removes a name from the magick info list.  It returns
%  MagickFalse if the name does not exist in the list otherwise MagickTrue.
%
%  The format of the UnregisterMagickInfo method is:
%
%      MagickBooleanType UnregisterMagickInfo(const char *name)
%
%  A description of each parameter follows:
%
%    o name: a character string that represents the image format we are
%      looking for.
%
*/
MagickExport MagickBooleanType UnregisterMagickInfo(const char *name)
{
  register const MagickInfo
    *p;

  MagickBooleanType
    status;

  assert(name != (const char *) NULL);
  if (magick_list == (SplayTreeInfo *) NULL)
    return(MagickFalse);
  if (GetNumberOfNodesInSplayTree(magick_list) == 0)
    return(MagickFalse);
  LockSemaphoreInfo(magick_semaphore);
  ResetSplayTreeIterator(magick_list);
  p=(const MagickInfo *) GetNextValueInSplayTree(magick_list);
  while (p != (const MagickInfo *) NULL)
  {
    if (LocaleCompare(p->name,name) == 0)
      break;
    p=(const MagickInfo *) GetNextValueInSplayTree(magick_list);
  }
  status=DeleteNodeByValueFromSplayTree(magick_list,p);
  UnlockSemaphoreInfo(magick_semaphore);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                QQQ   U   U   AAA   N   N  TTTTT  U   U  M   M               %
%               Q   Q  U   U  A   A  NN  N    T    U   U  MM MM               %
%               Q   Q  U   U  AAAAA  N N N    T    U   U  M M M               %
%               Q  QQ  U   U  A   A  N  NN    T    U   U  M   M               %
%                QQQQ   UUU   A   A  N   N    T     UUU   M   M               %
%                                                                             %
%             MagicCore Methods to Acquire / Destroy Quantum Pixels           %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               October 1998                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2008 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/color-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/cache.h"
#include "magick/constitute.h"
#include "magick/delegate.h"
#include "magick/geometry.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/option.h"
#include "magick/pixel.h"
#include "magick/pixel-private.h"
#include "magick/quantum.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/semaphore.h"
#include "magick/statistic.h"
#include "magick/stream.h"
#include "magick/string_.h"
#include "magick/thread-private.h"
#include "magick/utility.h"

/*
  Define declarations.
*/
#define QuantumSignature  0xab

/*
  Forward declarations.
*/
static void
  DestroyQuantumPixels(QuantumInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e Q u a n t u m I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireQuantumInfo() allocates the QuantumInfo structure.
%
%  The format of the AcquireQuantumInfo method is:
%
%      QuantumInfo *AcquireQuantumInfo(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
*/

static inline unsigned long MagickMax(const unsigned long x,
  const unsigned long y)
{
  if (x > y)
    return(x);
  return(y);
}

MagickExport QuantumInfo *AcquireQuantumInfo(const ImageInfo *image_info,
  Image *image)
{
  MagickBooleanType
    status;

  QuantumInfo
    *quantum_info;

  quantum_info=(QuantumInfo *) AcquireMagickMemory(sizeof(*quantum_info));
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  quantum_info->signature=MagickSignature;
  GetQuantumInfo(image_info,quantum_info);
  if (image == (const Image *) NULL)
    return(quantum_info);
  status=SetQuantumDepth(image,quantum_info,image->depth);
  if (status == MagickFalse)
    quantum_info=DestroyQuantumInfo(quantum_info);
  return(quantum_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e Q u a n t u m P i x e l s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireQuantumPixels() allocates the unsigned char structure.
%
%  The format of the AcquireQuantumPixels method is:
%
%      MagickBooleanType AcquireQuantumPixels(QuantumInfo *quantum_info,
%        const size_t extent)
%
%  A description of each parameter follows:
%
%    o quantum_info: the quantum info.
%
%    o extent: the quantum info.
%
*/
static MagickBooleanType AcquireQuantumPixels(QuantumInfo *quantum_info,
  const size_t extent)
{
  register long
    i;

  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->number_threads=GetOpenMPMaximumThreads();
  quantum_info->pixels=(unsigned char **) AcquireQuantumMemory(
    quantum_info->number_threads,sizeof(*quantum_info->pixels));
  if (quantum_info->pixels == (unsigned char **) NULL)
    return(MagickFalse);
  quantum_info->extent=extent;
  (void) ResetMagickMemory(quantum_info->pixels,0,
    sizeof(*quantum_info->pixels));
  for (i=0; i < (long) quantum_info->number_threads; i++)
  {
    quantum_info->pixels[i]=(unsigned char *) AcquireQuantumMemory(extent+1,
      sizeof(**quantum_info->pixels));
  	if (quantum_info->pixels[i] == (unsigned char *) NULL)
      return(MagickFalse);
    (void) ResetMagickMemory(quantum_info->pixels[i],0,(extent+1)*
      sizeof(**quantum_info->pixels));
    quantum_info->pixels[i][extent]=QuantumSignature;
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y Q u a n t u m I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyQuantumInfo() deallocates memory associated with the QuantumInfo
%  structure.
%
%  The format of the DestroyQuantumInfo method is:
%
%      QuantumInfo *DestroyQuantumInfo(QuantumInfo *quantum_info)
%
%  A description of each parameter follows:
%
%    o quantum_info: the quantum info.
%
*/
MagickExport QuantumInfo *DestroyQuantumInfo(QuantumInfo *quantum_info)
{
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  if (quantum_info->pixels != (unsigned char **) NULL)
    DestroyQuantumPixels(quantum_info);
  if (quantum_info->semaphore != (SemaphoreInfo *) NULL)
    DestroySemaphoreInfo(&quantum_info->semaphore);
  quantum_info->signature=(~MagickSignature);
  quantum_info=(QuantumInfo *) RelinquishMagickMemory(quantum_info);
  return(quantum_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y Q u a n t u m P i x e l s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyQuantumPixels() destroys the quantum pixels.
%
%  The format of the DestroyQuantumPixels() method is:
%
%      void DestroyQuantumPixels(QuantumInfo *quantum_info)
%
%  A description of each parameter follows:
%
%    o quantum_info: the quantum info.
%
*/
static void DestroyQuantumPixels(QuantumInfo *quantum_info)
{
  register long
    i;

  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  assert(quantum_info->pixels != (unsigned char **) NULL);
  for (i=0; i < (long) quantum_info->number_threads; i++)
  {
    assert(quantum_info->pixels[i][quantum_info->extent] == QuantumSignature);
    quantum_info->pixels[i]=(unsigned char *) RelinquishMagickMemory(
      quantum_info->pixels[i]);
  }
  quantum_info->pixels=(unsigned char **) RelinquishMagickMemory(
    quantum_info->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t Q u a n t u m E x t e n t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetQuantumExtent() returns the quantum pixel buffer extent.
%
%  The format of the GetQuantumExtent method is:
%
%      size_t GetQuantumExtent(Image *image,const QuantumInfo *quantum_info,
%        const QuantumType quantum_type)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o quantum_info: the quantum info.
%
%    o quantum_type: Declare which pixel components to transfer (red, green,
%      blue, opacity, RGB, or RGBA).
%
*/
MagickExport size_t GetQuantumExtent(const Image *image,
  const QuantumInfo *quantum_info,const QuantumType quantum_type)
{
  size_t
    packet_size;

  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  packet_size=1;
  switch (quantum_type)
  {
    case GrayAlphaQuantum: packet_size=2; break;
    case IndexAlphaQuantum: packet_size=2; break;
    case RGBQuantum: packet_size=3; break;
    case RGBAQuantum: packet_size=4; break;
    case RGBOQuantum: packet_size=4; break;
    case CMYKQuantum: packet_size=4; break;
    case CMYKAQuantum: packet_size=5; break;
    default: break;
  }
  if (quantum_info->pack == MagickFalse)
    return((size_t) (packet_size*image->columns*((image->depth+7)/8)));
  return((size_t) ((packet_size*image->columns*image->depth+7)/8));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t Q u a n t u m I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetQuantumInfo() initializes the QuantumInfo structure to default values.
%
%  The format of the GetQuantumInfo method is:
%
%      GetQuantumInfo(const ImageInfo *image_info,QuantumInfo *quantum_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o quantum_info: the quantum info.
%
*/
MagickExport void GetQuantumInfo(const ImageInfo *image_info,
  QuantumInfo *quantum_info)
{
  const char
    *option;

  assert(quantum_info != (QuantumInfo *) NULL);
  (void) ResetMagickMemory(quantum_info,0,sizeof(*quantum_info));
  quantum_info->quantum=8;
  quantum_info->maximum=1.0;
  quantum_info->scale=QuantumRange;
  quantum_info->pack=MagickTrue;
  quantum_info->semaphore=AllocateSemaphoreInfo();
  quantum_info->signature=MagickSignature;
  if (image_info == (const ImageInfo *) NULL)
    return;
  option=GetImageOption(image_info,"quantum:format");
  if (option != (char *) NULL)
    quantum_info->format=(QuantumFormatType) ParseMagickOption(
      MagickQuantumFormatOptions,MagickFalse,option);
  option=GetImageOption(image_info,"quantum:minimum");
  if (option != (char *) NULL)
    quantum_info->minimum=atof(option);
  option=GetImageOption(image_info,"quantum:maximum");
  if (option != (char *) NULL)
    quantum_info->maximum=atof(option);
  if ((quantum_info->minimum == 0.0) && (quantum_info->maximum == 0.0))
    quantum_info->scale=0.0;
  else
    if (quantum_info->minimum == quantum_info->maximum)
      {
        quantum_info->scale=(MagickRealType) QuantumRange/quantum_info->minimum;
        quantum_info->minimum=0.0;
      }
    else
      quantum_info->scale=(MagickRealType) QuantumRange/(quantum_info->maximum-
        quantum_info->minimum);
  option=GetImageOption(image_info,"quantum:scale");
  if (option != (char *) NULL)
    quantum_info->scale=atof(option);
  option=GetImageOption(image_info,"quantum:polarity");
  if (option != (char *) NULL)
    quantum_info->min_is_white=LocaleCompare(option,"min-is-white") == 0 ?
      MagickTrue : MagickFalse;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t Q u a n t u m P i x e l s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetQuantumPixels() returns the quantum pixels.
%
%  The format of the GetQuantumPixels method is:
%
%      unsigned char *QuantumPixels GetQuantumPixels(
%        const QuantumInfo *quantum_info)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport unsigned char *GetQuantumPixels(const QuantumInfo *quantum_info)
{
  long
    id;

  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  id=GetOpenMPThreadId();
  return(quantum_info->pixels[id]);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t Q u a n t u m T y p e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetQuantumType() returns the quantum type of the image.
%
%  The format of the GetQuantumType method is:
%
%      QuantumType GetQuantumType(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport QuantumType GetQuantumType(Image *image,ExceptionInfo *exception)
{
  QuantumType
    quantum_type;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  quantum_type=RGBQuantum;
  if (image->matte != MagickFalse)
    quantum_type=RGBAQuantum;
  if (image->colorspace == CMYKColorspace)
    {
      quantum_type=CMYKQuantum;
      if (image->matte != MagickFalse)
        quantum_type=CMYKAQuantum;
    }
  if (IsGrayImage(image,exception) != MagickFalse)
    {
      quantum_type=GrayQuantum;
      if (image->matte != MagickFalse)
        quantum_type=GrayAlphaQuantum;
    }
  else
    if (image->storage_class == PseudoClass)
      {
        quantum_type=IndexQuantum;
        if (image->matte != MagickFalse)
          quantum_type=IndexAlphaQuantum;
      }
  return(quantum_type);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m F o r m a t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumAlphaType() sets the quantum format.
%
%  The format of the SetQuantumAlphaType method is:
%
%      void SetQuantumAlphaType(QuantumInfo *quantum_info,
%        const QuantumAlphaType type)
%
%  A description of each parameter follows:
%
%    o quantum_info: the quantum info.
%
%    o type: the alpha type (e.g. associate).
%
*/
MagickExport void SetQuantumAlphaType(QuantumInfo *quantum_info,
  const QuantumAlphaType type)
{
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->alpha_type=type;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m D e p t h                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumDepth() sets the quantum depth.
%
%  The format of the SetQuantumDepth method is:
%
%      MagickBooleanType SetQuantumDepth(const Image *image,
%        QuantumInfo *quantum_info,const unsigned long depth)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o quantum_info: the quantum info.
%
%    o depth: the quantum depth.
%
*/
MagickExport MagickBooleanType SetQuantumDepth(const Image *image,
  QuantumInfo *quantum_info,const unsigned long depth)
{
  MagickBooleanType
    status;

  /*
    Allocate the quantum pixel buffer.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->depth=depth;
  if (quantum_info->format == FloatingPointQuantumFormat)
    {
      if (quantum_info->depth > 32)
        quantum_info->depth=64;
      else
        quantum_info->depth=32;
    }
  if (quantum_info->pixels != (unsigned char **) NULL)
    DestroyQuantumPixels(quantum_info);
  status=AcquireQuantumPixels(quantum_info,(quantum_info->pad+5)*image->columns*
    ((quantum_info->depth+7)/8));
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m F o r m a t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumFormat() sets the quantum format.
%
%  The format of the SetQuantumFormat method is:
%
%      MagickBooleanType SetQuantumFormat(const Image *image,
%        QuantumInfo *quantum_info,const QuantumFormatType format)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o quantum_info: the quantum info.
%
%    o format: the quantum format.
%
*/
MagickExport MagickBooleanType SetQuantumFormat(const Image *image,
  QuantumInfo *quantum_info,const QuantumFormatType format)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->format=format;
  return(SetQuantumDepth(image,quantum_info,quantum_info->depth));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m I m a g e T y p e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumImageType() sets the image type based on the quantum type.
%
%  The format of the SetQuantumImageType method is:
%
%      void ImageType SetQuantumImageType(Image *image,
%        const QuantumType quantum_type)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o quantum_type: Declare which pixel components to transfer (red, green,
%      blue, opacity, RGB, or RGBA).
%
*/
MagickExport void SetQuantumImageType(Image *image,
  const QuantumType quantum_type)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  switch (quantum_type)
  {
    case IndexQuantum:
    case IndexAlphaQuantum:
    {
      image->type=PaletteType;
      break;
    }
    case GrayQuantum:
    case GrayAlphaQuantum:
    {
      image->type=GrayscaleType;
      if (image->depth == 1)
        image->type=BilevelType;
      break;
    }
    case CyanQuantum:
    case MagentaQuantum:
    case YellowQuantum:
    case BlackQuantum:
    case CMYKQuantum:
    case CMYKAQuantum:
    {
      image->type=ColorSeparationType;
      break;
    }
    default:
    {
      image->type=TrueColorType;
      break;
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m P a c k                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumPack() sets the quantum pack flag.
%
%  The format of the SetQuantumPack method is:
%
%      void SetQuantumPack(QuantumInfo *quantum_info,
%        const MagickBooleanType pack)
%
%  A description of each parameter follows:
%
%    o quantum_info: the quantum info.
%
%    o pack: the pack flag.
%
*/
MagickExport void SetQuantumPack(QuantumInfo *quantum_info,
  const MagickBooleanType pack)
{
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->pack=pack;
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m P a d                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumPad() sets the quantum pad.
%
%  The format of the SetQuantumPad method is:
%
%      MagickBooleanType SetQuantumPad(const Image *image,
%        QuantumInfo *quantum_info,const unsigned long pad)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o quantum_info: the quantum info.
%
%    o pad: the quantum pad.
%
*/
MagickExport MagickBooleanType SetQuantumPad(const Image *image,
  QuantumInfo *quantum_info,const unsigned long pad)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->pad=pad;
  return(SetQuantumDepth(image,quantum_info,quantum_info->depth));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m M i n I s W h i t e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumMinIsWhite() sets the quantum min-is-white flag.
%
%  The format of the SetQuantumMinIsWhite method is:
%
%      void SetQuantumMinIsWhite(QuantumInfo *quantum_info,
%        const MagickBooleanType min_is_white)
%
%  A description of each parameter follows:
%
%    o quantum_info: the quantum info.
%
%    o min_is_white: the min-is-white flag.
%
*/
MagickExport void SetQuantumMinIsWhite(QuantumInfo *quantum_info,
  const MagickBooleanType min_is_white)
{
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->min_is_white=min_is_white;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m Q u a n t u m                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumQuantum() sets the quantum quantum.
%
%  The format of the SetQuantumQuantum method is:
%
%      void SetQuantumQuantum(QuantumInfo *quantum_info,
%        const unsigned long quantum)
%
%  A description of each parameter follows:
%
%    o quantum_info: the quantum info.
%
%    o quantum: the quantum quantum.
%
*/
MagickExport void SetQuantumQuantum(QuantumInfo *quantum_info,
  const unsigned long quantum)
{
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->quantum=quantum;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m S c a l e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumScale() sets the quantum scale.
%
%  The format of the SetQuantumScale method is:
%
%      void SetQuantumScale(QuantumInfo *quantum_info,const double scale)
%
%  A description of each parameter follows:
%
%    o quantum_info: the quantum info.
%
%    o scale: the quantum scale.
%
*/
MagickExport void SetQuantumScale(QuantumInfo *quantum_info,const double scale)
{
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->scale=scale;
}

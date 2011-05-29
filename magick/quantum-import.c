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
%                   IIIII  M   M  PPPP    OOO   RRRR   TTTTT                  %
%                     I    MM MM  P   P  O   O  R   R    T                    %
%                     I    M M M  PPPP   O   O  RRRR     T                    %
%                     I    M   M  P      O   O  R R      T                    %
%                   IIIII  M   M  P       OOO   R  R     T                    %
%                                                                             %
%                 MagickCore Methods to Import Quantum Pixels                 %
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
#include "magick/utility.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I m p o r t Q u a n t u m P i x e l s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImportQuantumPixels() transfers one or more pixel components from a user
%  supplied buffer into the image pixel cache of an image.  The pixels are
%  expected in network byte order.  It returns MagickTrue if the pixels are
%  successfully transferred, otherwise MagickFalse.
%
%  The format of the ImportQuantumPixels method is:
%
%      size_t ImportQuantumPixels(Image *image,CacheView *image_view,
%        const QuantumInfo *quantum_info,const QuantumType quantum_type,
%        const unsigned char *pixels,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o image_view: the image cache view.
%
%    o quantum_info: the quantum info.
%
%    o quantum_type: Declare which pixel components to transfer (red, green,
%      blue, opacity, RGB, or RGBA).
%
%    o pixels:  The pixel components are transferred from this buffer.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline IndexPacket PushColormapIndex(Image *image,
  const size_t index,MagickBooleanType *range_exception)
{
  if (index < image->colors)
    return((IndexPacket) index);
  *range_exception=MagickTrue;
  return((IndexPacket) 0);
}

static inline const unsigned char *PushDoublePixel(
  const QuantumState *quantum_state,const unsigned char *pixels,double *pixel)
{
  double
    *p;

  unsigned char
    quantum[8];

  if (quantum_state->endian != LSBEndian)
    {
      quantum[7]=(*pixels++);
      quantum[6]=(*pixels++);
      quantum[5]=(*pixels++);
      quantum[5]=(*pixels++);
      quantum[3]=(*pixels++);
      quantum[2]=(*pixels++);
      quantum[1]=(*pixels++);
      quantum[0]=(*pixels++);
      p=(double *) quantum;
      *pixel=(*p);
      *pixel-=quantum_state->minimum;
      *pixel*=quantum_state->scale;
      return(pixels);
    }
  quantum[0]=(*pixels++);
  quantum[1]=(*pixels++);
  quantum[2]=(*pixels++);
  quantum[3]=(*pixels++);
  quantum[4]=(*pixels++);
  quantum[5]=(*pixels++);
  quantum[6]=(*pixels++);
  quantum[7]=(*pixels++);
  p=(double *) quantum;
  *pixel=(*p);
  *pixel-=quantum_state->minimum;
  *pixel*=quantum_state->scale;
  return(pixels);
}

static inline const unsigned char *PushFloatPixel(
  const QuantumState *quantum_state,const unsigned char *pixels,float *pixel)
{
  float
    *p;

  unsigned char
    quantum[4];

  if (quantum_state->endian != LSBEndian)
    {
      quantum[3]=(*pixels++);
      quantum[2]=(*pixels++);
      quantum[1]=(*pixels++);
      quantum[0]=(*pixels++);
      p=(float *) quantum;
      *pixel=(*p);
      *pixel-=quantum_state->minimum;
      *pixel*=quantum_state->scale;
      return(pixels);
    }
  quantum[0]=(*pixels++);
  quantum[1]=(*pixels++);
  quantum[2]=(*pixels++);
  quantum[3]=(*pixels++);
  p=(float *) quantum;
  *pixel=(*p);
  *pixel-=quantum_state->minimum;
  *pixel*=quantum_state->scale;
  return(pixels);
}

static inline const unsigned char *PushQuantumPixel(
  QuantumState *quantum_state,const size_t depth,
  const unsigned char *pixels,unsigned int *quantum)
{
  register ssize_t
    i;

  register size_t
    quantum_bits;

  *quantum=(QuantumAny) 0;
  for (i=(ssize_t) depth; i > 0L; )
  {
    if (quantum_state->bits == 0UL)
      {
        quantum_state->pixel=(*pixels++);
        quantum_state->bits=8UL;
      }
    quantum_bits=(size_t) i;
    if (quantum_bits > quantum_state->bits)
      quantum_bits=quantum_state->bits;
    i-=(ssize_t) quantum_bits;
    quantum_state->bits-=quantum_bits;
    *quantum=(unsigned int) ((*quantum << quantum_bits) |
      ((quantum_state->pixel >> quantum_state->bits) &~ ((~0UL) <<
      quantum_bits)));
  }
  return(pixels);
}

static inline const unsigned char *PushQuantumLongPixel(
  QuantumState *quantum_state,const size_t depth,
  const unsigned char *pixels,unsigned int *quantum)
{
  register ssize_t
    i;

  register size_t
    quantum_bits;

  *quantum=0UL;
  for (i=(ssize_t) depth; i > 0; )
  {
    if (quantum_state->bits == 0)
      {
        pixels=PushLongPixel(quantum_state->endian,pixels,
          &quantum_state->pixel);
        quantum_state->bits=32U;
      }
    quantum_bits=(size_t) i;
    if (quantum_bits > quantum_state->bits)
      quantum_bits=quantum_state->bits;
    *quantum|=(((quantum_state->pixel >> (32U-quantum_state->bits)) &
      quantum_state->mask[quantum_bits]) << (depth-i));
    i-=(ssize_t) quantum_bits;
    quantum_state->bits-=quantum_bits;
  }
  return(pixels);
}

MagickExport size_t ImportQuantumPixels(Image *image,CacheView *image_view,
  const QuantumInfo *quantum_info,const QuantumType quantum_type,
  const unsigned char *pixels,ExceptionInfo *exception)
{
  EndianType
    endian;

  MagickSizeType
    number_pixels;

  QuantumAny
    range;

  QuantumState
    quantum_state;

  register const unsigned char
    *restrict p;

  register IndexPacket
    *restrict indexes;

  register ssize_t
    x;

  register PixelPacket
    *restrict q;

  size_t
    extent;

  ssize_t
    bit;

  unsigned int
    pixel;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  if (pixels == (const unsigned char *) NULL)
    pixels=GetQuantumPixels(quantum_info);
  x=0;
  p=pixels;
  if (image_view == (CacheView *) NULL)
    {
      number_pixels=GetImageExtent(image);
      q=GetAuthenticPixelQueue(image);
      indexes=GetAuthenticIndexQueue(image);
    }
  else
    {
      number_pixels=GetCacheViewExtent(image_view);
      q=GetCacheViewAuthenticPixelQueue(image_view);
      indexes=GetCacheViewAuthenticIndexQueue(image_view);
    }
  InitializeQuantumState(quantum_info,image->endian,&quantum_state);
  extent=GetQuantumExtent(image,quantum_info,quantum_type);
  endian=quantum_state.endian;
  switch (quantum_type)
  {
    case IndexQuantum:
    {
      MagickBooleanType
        range_exception;

      if (image->storage_class != PseudoClass)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
            "ColormappedImageRequired","`%s'",image->filename);
          return(extent);
        }
      range_exception=MagickFalse;
      switch (quantum_info->depth)
      {
        case 1:
        {
          register unsigned char
            pixel;

          for (x=0; x < ((ssize_t) number_pixels-7); x+=8)
          {
            for (bit=0; bit < 8; bit++)
            {
              if (quantum_info->min_is_white == MagickFalse)
                pixel=(unsigned char) (((*p) & (1 << (7-bit))) == 0 ?
                  0x00 : 0x01);
              else
                pixel=(unsigned char) (((*p) & (1 << (7-bit))) != 0 ?
                  0x00 : 0x01);
              SetIndexPixelComponent(indexes+x+bit,PushColormapIndex(image,
                pixel,&range_exception));
              SetRGBOPixelComponents(q,image->colormap+(ssize_t)
                GetIndexPixelComponent(indexes+x+bit));
              q++;
            }
            p++;
          }
          for (bit=0; bit < (ssize_t) (number_pixels % 8); bit++)
          {
            if (quantum_info->min_is_white == MagickFalse)
              pixel=(unsigned char) (((*p) & (1 << (7-bit))) == 0 ?
                0x00 : 0x01);
            else
              pixel=(unsigned char) (((*p) & (1 << (7-bit))) != 0 ?
                0x00 : 0x01);
            SetIndexPixelComponent(indexes+x+bit,PushColormapIndex(image,pixel,
              &range_exception));
            SetRGBOPixelComponents(q,image->colormap+(ssize_t)
              GetIndexPixelComponent(indexes+x+bit));
            q++;
          }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < ((ssize_t) number_pixels-1); x+=2)
          {
            pixel=(unsigned char) ((*p >> 4) & 0xf);
            SetIndexPixelComponent(indexes+x,PushColormapIndex(image,pixel,
              &range_exception));
            SetRGBOPixelComponents(q,image->colormap+(ssize_t)
              GetIndexPixelComponent(indexes+x));
            q++;
            pixel=(unsigned char) ((*p) & 0xf);
            SetIndexPixelComponent(indexes+x+1,PushColormapIndex(image,pixel,
              &range_exception));
            SetRGBOPixelComponents(q,image->colormap+(ssize_t)
              GetIndexPixelComponent(indexes+x+1));
            p++;
            q++;
          }
          for (bit=0; bit < (ssize_t) (number_pixels % 2); bit++)
          {
            pixel=(unsigned char) ((*p++ >> 4) & 0xf);
            SetIndexPixelComponent(indexes+x+bit,PushColormapIndex(image,pixel,
              &range_exception));
            SetRGBOPixelComponents(q,image->colormap+(ssize_t)
              GetIndexPixelComponent(indexes+x+bit));
            q++;
          }
          break;
        }
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetIndexPixelComponent(indexes+x,PushColormapIndex(image,pixel,
              &range_exception));
            SetRGBOPixelComponents(q,image->colormap+(ssize_t)
              GetIndexPixelComponent(indexes+x));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetIndexPixelComponent(indexes+x,PushColormapIndex(image,
                  ClampToQuantum((MagickRealType) QuantumRange*
                  HalfToSinglePrecision(pixel)),&range_exception));
                SetRGBOPixelComponents(q,image->colormap+(ssize_t)
                  GetIndexPixelComponent(indexes+x));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetIndexPixelComponent(indexes+x,PushColormapIndex(image,pixel,
              &range_exception));
            SetRGBOPixelComponents(q,image->colormap+(ssize_t)
              GetIndexPixelComponent(indexes+x));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetIndexPixelComponent(indexes+x,PushColormapIndex(image,
                  ClampToQuantum(pixel),&range_exception));
                SetRGBOPixelComponents(q,image->colormap+(ssize_t)
                  GetIndexPixelComponent(indexes+x));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetIndexPixelComponent(indexes+x,PushColormapIndex(image,pixel,
              &range_exception));
            SetRGBOPixelComponents(q,image->colormap+(ssize_t)
              GetIndexPixelComponent(indexes+x));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetIndexPixelComponent(indexes+x,PushColormapIndex(image,
                  ClampToQuantum(pixel),&range_exception));
                SetRGBOPixelComponents(q,image->colormap+(ssize_t)
                  GetIndexPixelComponent(indexes+x));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetIndexPixelComponent(indexes+x,PushColormapIndex(image,pixel,
              &range_exception));
            SetRGBOPixelComponents(q,image->colormap+(ssize_t)
              GetIndexPixelComponent(indexes+x));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
      }
      if (range_exception != MagickFalse)
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageError,"InvalidColormapIndex","`%s'",image->filename);
      break;
    }
    case IndexAlphaQuantum:
    {
      MagickBooleanType
        range_exception;

      if (image->storage_class != PseudoClass)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            ImageError,"ColormappedImageRequired","`%s'",image->filename);
          return(extent);
        }
      range_exception=MagickFalse;
      switch (quantum_info->depth)
      {
        case 1:
        {
          register unsigned char
            pixel;

          for (x=0; x < ((ssize_t) number_pixels-3); x+=4)
          {
            for (bit=0; bit < 8; bit+=2)
            {
              if (quantum_info->min_is_white == MagickFalse)
                pixel=(unsigned char) (((*p) & (1 << (7-bit))) == 0 ?
                  0x00 : 0x01);
              else
                pixel=(unsigned char) (((*p) & (1 << (7-bit))) != 0 ?
                  0x00 : 0x01);
              SetIndexPixelComponent(indexes+x+bit/2,pixel == 0 ? 0 : 1);
              SetRedPixelComponent(q,pixel == 0 ? 0 : QuantumRange);
              SetGreenPixelComponent(q,GetRedPixelComponent(q));
              SetBluePixelComponent(q,GetRedPixelComponent(q));
              SetOpacityPixelComponent(q,((*p) & (1UL << (unsigned char)
                (6-bit))) == 0 ? TransparentOpacity : OpaqueOpacity);
              q++;
            }
          }
          for (bit=0; bit < (ssize_t) (number_pixels % 4); bit+=2)
          {
            if (quantum_info->min_is_white == MagickFalse)
              pixel=(unsigned char) (((*p) & (1 << (7-bit))) == 0 ?
                0x00 : 0x01);
            else
              pixel=(unsigned char) (((*p) & (1 << (7-bit))) != 0 ?
                0x00 : 0x01);
            SetIndexPixelComponent(indexes+x+bit/2,pixel == 0 ? 0 : 1);
            SetRedPixelComponent(q,pixel == 0 ? 0 : QuantumRange);
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            SetOpacityPixelComponent(q,((*p) & (1UL << (unsigned char)
              (6-bit))) == 0 ? TransparentOpacity : OpaqueOpacity);
            q++;
          }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned char) ((*p >> 4) & 0xf);
            SetIndexPixelComponent(indexes+x,PushColormapIndex(image,pixel,
              &range_exception));
            SetRGBOPixelComponents(q,image->colormap+(ssize_t)
              GetIndexPixelComponent(indexes+x));
            pixel=(unsigned char) ((*p) & 0xf);
            SetAlphaPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p++;
            q++;
          }
          break;
        }
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetIndexPixelComponent(indexes+x,PushColormapIndex(image,pixel,
              &range_exception));
            SetRGBOPixelComponents(q,image->colormap+(ssize_t)
              GetIndexPixelComponent(indexes+x));
            p=PushCharPixel(p,&pixel);
            SetAlphaPixelComponent(q,ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetIndexPixelComponent(indexes+x,PushColormapIndex(image,
                  ClampToQuantum((MagickRealType) QuantumRange*
                  HalfToSinglePrecision(pixel)),&range_exception));
                SetRGBOPixelComponents(q,image->colormap+(ssize_t)
                  GetIndexPixelComponent(indexes+x));
                p=PushShortPixel(endian,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetIndexPixelComponent(indexes+x,PushColormapIndex(image,pixel,
              &range_exception));
            SetRGBOPixelComponents(q,image->colormap+(ssize_t)
              GetIndexPixelComponent(indexes+x));
            p=PushShortPixel(endian,p,&pixel);
            SetAlphaPixelComponent(q,ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetIndexPixelComponent(indexes+x,PushColormapIndex(image,
                  ClampToQuantum(pixel),&range_exception));
                SetRGBOPixelComponents(q,image->colormap+(ssize_t)
                  GetIndexPixelComponent(indexes+x));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetIndexPixelComponent(indexes+x,PushColormapIndex(image,pixel,
              &range_exception));
            SetRGBOPixelComponents(q,image->colormap+(ssize_t)
              GetIndexPixelComponent(indexes+x));
            p=PushLongPixel(endian,p,&pixel);
            SetAlphaPixelComponent(q,ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetIndexPixelComponent(indexes+x,PushColormapIndex(image,
                  ClampToQuantum(pixel),&range_exception));
                SetRGBOPixelComponents(q,image->colormap+(ssize_t)
                  GetIndexPixelComponent(indexes+x));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetIndexPixelComponent(indexes+x,PushColormapIndex(image,pixel,
              &range_exception));
            SetRGBOPixelComponents(q,image->colormap+(ssize_t)
              GetIndexPixelComponent(indexes+x));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetAlphaPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
      }
      if (range_exception != MagickFalse)
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageError,"InvalidColormapIndex","`%s'",image->filename);
      break;
    }
    case BGRQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetBluePixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetGreenPixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetRedPixelComponent(q,ScaleCharToQuantum(pixel));
            SetOpacityPixelComponent(q,OpaqueOpacity);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 10:
        {
          range=GetQuantumRange(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushLongPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ScaleAnyToQuantum((pixel >> 22) & 0x3ff,
                  range));
                SetGreenPixelComponent(q,ScaleAnyToQuantum((pixel >> 12) &
                  0x3ff,range));
                SetBluePixelComponent(q,ScaleAnyToQuantum((pixel >> 2) & 0x3ff,
                  range));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          if (quantum_info->quantum == 32U)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            q++;
          }
          break;
        }
        case 12:
        {
          range=GetQuantumRange(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              unsigned short
                pixel;

              for (x=0; x < (ssize_t) (3*number_pixels-1); x+=2)
              {
                p=PushShortPixel(endian,p,&pixel);
                switch (x % 3)
                {
                  default:
                  case 0:
                  {
                    SetRedPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    break;
                  }
                  case 1:
                  {
                    SetGreenPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    break;
                  }
                  case 2:
                  {
                    SetBluePixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    q++;
                    break;
                  }
                }
                p=PushShortPixel(endian,p,&pixel);
                switch ((x+1) % 3)
                {
                  default:
                  case 0:
                  {
                    SetRedPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    break;
                  }
                  case 1:
                  {
                    SetGreenPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    break;
                  }
                  case 2:
                  {
                    SetBluePixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    q++;
                    break;
                  }
                }
                p+=quantum_info->pad;
              }
              for (bit=0; bit < (ssize_t) (3*number_pixels % 2); bit++)
              {
                p=PushShortPixel(endian,p,&pixel);
                switch ((x+bit) % 3)
                {
                  default:
                  case 0:
                  {
                    SetRedPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    break;
                  }
                  case 1:
                  {
                    SetGreenPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    break;
                  }
                  case 2:
                  {
                    SetBluePixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    q++;
                    break;
                  }
                }
                p+=quantum_info->pad;
              }
              if (bit != 0)
                p++;
              break;
            }
          if (quantum_info->quantum == 32U)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            q++;
          }
          break;
        }
      }
      break;
    }
    case BGRAQuantum:
    case BGROQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetBluePixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetGreenPixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetRedPixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetAlphaPixelComponent(q,ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 10:
        {
          pixel=0;
          if (quantum_info->pack == MagickFalse)
            {
              register ssize_t
                i;

              size_t
                quantum;

              ssize_t
                n;

              n=0;
              quantum=0;
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                for (i=0; i < 4; i++)
                {
                  switch (n % 3)
                  {
                    case 0:
                    {
                      p=PushLongPixel(endian,p,&pixel);
                      quantum=(size_t) (ScaleShortToQuantum(
                        (unsigned short) (((pixel >> 22) & 0x3ff) << 6)));
                      break;
                    }
                    case 1:
                    {
                      quantum=(size_t) (ScaleShortToQuantum(
                        (unsigned short) (((pixel >> 12) & 0x3ff) << 6)));
                      break;
                    }
                    case 2:
                    {
                      quantum=(size_t) (ScaleShortToQuantum(
                        (unsigned short) (((pixel >> 2) & 0x3ff) << 6)));
                      break;
                    }
                  }
                  switch (i)
                  {
                    case 0: SetRedPixelComponent(q,quantum); break;
                    case 1: SetGreenPixelComponent(q,quantum); break;
                    case 2: SetBluePixelComponent(q,quantum); break;
                    case 3: SetAlphaPixelComponent(q,quantum); break;
                  }
                  n++;
                }
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleShortToQuantum((unsigned short)
              (pixel << 6)));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleShortToQuantum((unsigned short)
              (pixel << 6)));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetBluePixelComponent(q,ScaleShortToQuantum((unsigned short)
              (pixel << 6)));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetAlphaPixelComponent(q,ScaleShortToQuantum((unsigned short)
              (pixel << 6)));
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetAlphaPixelComponent(q,ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetAlphaPixelComponent(q,ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetAlphaPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            q++;
          }
          break;
        }
      }
      break;
    }
    case GrayQuantum:
    {
      switch (quantum_info->depth)
      {
        case 1:
        {
          register Quantum
            black,
            white;

          black=0;
          white=(Quantum) QuantumRange;
          if (quantum_info->min_is_white != MagickFalse)
            {
              black=(Quantum) QuantumRange;
              white=0;
            }
          for (x=0; x < ((ssize_t) number_pixels-7); x+=8)
          {
            for (bit=0; bit < 8; bit++)
            {
              SetRedPixelComponent(q,((*p) & (1 << (7-bit))) == 0 ?
                black : white);
              SetGreenPixelComponent(q,GetRedPixelComponent(q));
              SetBluePixelComponent(q,GetRedPixelComponent(q));
              q++;
            }
            p++;
          }
          for (bit=0; bit < (ssize_t) (number_pixels % 8); bit++)
          {
            SetRedPixelComponent(q,((*p) & (0x01 << (7-bit))) == 0 ?
              black : white);
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            q++;
          }
          if (bit != 0)
            p++;
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          range=GetQuantumRange(image->depth);
          for (x=0; x < ((ssize_t) number_pixels-1); x+=2)
          {
            pixel=(unsigned char) ((*p >> 4) & 0xf);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            q++;
            pixel=(unsigned char) ((*p) & 0xf);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            p++;
            q++;
          }
          for (bit=0; bit < (ssize_t) (number_pixels % 2); bit++)
          {
            pixel=(unsigned char) (*p++ >> 4);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            q++;
          }
          break;
        }
        case 8:
        {
          unsigned char
            pixel;

          if (quantum_info->min_is_white != MagickFalse)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushCharPixel(p,&pixel);
                SetRedPixelComponent(q,QuantumRange-ScaleCharToQuantum(pixel));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                SetOpacityPixelComponent(q,OpaqueOpacity);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetRedPixelComponent(q,ScaleCharToQuantum(pixel));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            SetOpacityPixelComponent(q,OpaqueOpacity);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 10:
        {
          range=GetQuantumRange(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              if (image->endian != LSBEndian)
                {
                  for (x=0; x < (ssize_t) (number_pixels-2); x+=3)
                  {
                    p=PushLongPixel(endian,p,&pixel);
                    SetRedPixelComponent(q,ScaleAnyToQuantum((pixel >> 2) &
                      0x3ff,range));
                    SetGreenPixelComponent(q,GetRedPixelComponent(q));
                    SetBluePixelComponent(q,GetRedPixelComponent(q));
                    q++;
                    SetRedPixelComponent(q,ScaleAnyToQuantum((pixel >> 12) &
                      0x3ff,range));
                    SetGreenPixelComponent(q,GetRedPixelComponent(q));
                    SetBluePixelComponent(q,GetRedPixelComponent(q));
                    q++;
                    SetRedPixelComponent(q,ScaleAnyToQuantum((pixel >> 22) &
                      0x3ff,range));
                    SetGreenPixelComponent(q,GetRedPixelComponent(q));
                    SetBluePixelComponent(q,GetRedPixelComponent(q));
                    p+=quantum_info->pad;
                    q++;
                  }
                  p=PushLongPixel(endian,p,&pixel);
                  if (x++ < (ssize_t) (number_pixels-1))
                    {
                      SetRedPixelComponent(q,ScaleAnyToQuantum((pixel >> 2) &
                        0x3ff,range));
                      SetGreenPixelComponent(q,GetRedPixelComponent(q));
                      SetBluePixelComponent(q,GetRedPixelComponent(q));
                      q++;
                    }
                  if (x++ < (ssize_t) number_pixels)
                    {
                      SetRedPixelComponent(q,ScaleAnyToQuantum((pixel >> 12) &
                        0x3ff,range));
                      SetGreenPixelComponent(q,GetRedPixelComponent(q));
                      SetBluePixelComponent(q,GetRedPixelComponent(q));
                      q++;
                    }
                  break;
                }
              for (x=0; x < (ssize_t) (number_pixels-2); x+=3)
              {
                p=PushLongPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ScaleAnyToQuantum((pixel >> 22) &
                  0x3ff,range));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                q++;
                SetRedPixelComponent(q,ScaleAnyToQuantum((pixel >> 12) &
                  0x3ff,range));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                q++;
                SetRedPixelComponent(q,ScaleAnyToQuantum((pixel >> 2) &
                  0x3ff,range));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                p+=quantum_info->pad;
                q++;
              }
              p=PushLongPixel(endian,p,&pixel);
              if (x++ < (ssize_t) (number_pixels-1))
                {
                  SetRedPixelComponent(q,ScaleAnyToQuantum((pixel >> 22) &
                    0x3ff,range));
                  SetGreenPixelComponent(q,GetRedPixelComponent(q));
                  SetBluePixelComponent(q,GetRedPixelComponent(q));
                  q++;
                }
              if (x++ < (ssize_t) number_pixels)
                {
                  SetRedPixelComponent(q,ScaleAnyToQuantum((pixel >> 12) &
                    0x3ff,range));
                  SetGreenPixelComponent(q,GetRedPixelComponent(q));
                  SetBluePixelComponent(q,GetRedPixelComponent(q));
                  q++;
                }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 12:
        {
          range=GetQuantumRange(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              unsigned short
                pixel;

              for (x=0; x < (ssize_t) (number_pixels-1); x+=2)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                  (pixel >> 4),range));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                q++;
                p=PushShortPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                  (pixel >> 4),range));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                p+=quantum_info->pad;
                q++;
              }
              for (bit=0; bit < (ssize_t) (number_pixels % 2); bit++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                  (pixel >> 4),range));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                p+=quantum_info->pad;
                q++;
              }
              if (bit != 0)
                p++;
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->min_is_white != MagickFalse)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetRedPixelComponent(q,QuantumRange-ScaleShortToQuantum(pixel));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleShortToQuantum(pixel));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleLongToQuantum(pixel));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
      }
      break;
    }
    case GrayAlphaQuantum:
    {
      switch (quantum_info->depth)
      {
        case 1:
        {
          register unsigned char
            pixel;

          for (x=0; x < ((ssize_t) number_pixels-3); x+=4)
          {
            for (bit=0; bit < 8; bit+=2)
            {
              pixel=(unsigned char)
                (((*p) & (1 << (7-bit))) != 0 ? 0x00 : 0x01);
              SetRedPixelComponent(q,pixel == 0 ? 0 : QuantumRange);
              SetGreenPixelComponent(q,GetRedPixelComponent(q));
              SetBluePixelComponent(q,GetRedPixelComponent(q));
              SetOpacityPixelComponent(q,((*p) & (1UL << (unsigned char)
                (6-bit))) == 0 ? TransparentOpacity : OpaqueOpacity);
              q++;
            }
            p++;
          }
          for (bit=0; bit <= (ssize_t) (number_pixels % 4); bit+=2)
          {
            pixel=(unsigned char) (((*p) & (1 << (7-bit))) != 0 ? 0x00 : 0x01);
            SetRedPixelComponent(q,pixel != 0 ? 0 : QuantumRange);
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            SetOpacityPixelComponent(q,((*p) & (1UL << (unsigned char)
              (6-bit))) == 0 ? TransparentOpacity : OpaqueOpacity);
            q++;
          }
          if (bit != 0)
            p++;
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned char) ((*p >> 4) & 0xf);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            pixel=(unsigned char) ((*p) & 0xf);
            SetAlphaPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p++;
            q++;
          }
          break;
        }
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetRedPixelComponent(q,ScaleCharToQuantum(pixel));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            p=PushCharPixel(p,&pixel);
            SetAlphaPixelComponent(q,ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 10:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetOpacityPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 12:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetOpacityPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                p=PushShortPixel(endian,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleShortToQuantum(pixel));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            p=PushShortPixel(endian,p,&pixel);
            SetAlphaPixelComponent(q,ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleLongToQuantum(pixel));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            p=PushLongPixel(endian,p,&pixel);
            SetAlphaPixelComponent(q,ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                SetGreenPixelComponent(q,GetRedPixelComponent(q));
                SetBluePixelComponent(q,GetRedPixelComponent(q));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            SetGreenPixelComponent(q,GetRedPixelComponent(q));
            SetBluePixelComponent(q,GetRedPixelComponent(q));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetAlphaPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
      }
      break;
    }
    case RedQuantum:
    case CyanQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetRedPixelComponent(q,ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
      }
      break;
    }
    case GreenQuantum:
    case MagentaQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetGreenPixelComponent(q,ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
      }
      break;
    }
    case BlueQuantum:
    case YellowQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetBluePixelComponent(q,ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
      }
      break;
    }
    case AlphaQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetAlphaPixelComponent(q,ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetAlphaPixelComponent(q,ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetAlphaPixelComponent(q,ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetAlphaPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
      }
      break;
    }
    case BlackQuantum:
    {
      if (image->colorspace != CMYKColorspace)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
            "ColorSeparatedImageRequired","`%s'",image->filename);
          return(extent);
        }
      switch (quantum_info->depth)
      {
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetIndexPixelComponent(indexes+x,ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetIndexPixelComponent(indexes+x,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetIndexPixelComponent(indexes+x,ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetIndexPixelComponent(indexes+x,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetIndexPixelComponent(indexes+x,ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetIndexPixelComponent(indexes+x,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetIndexPixelComponent(indexes+x,ScaleAnyToQuantum(pixel,range));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
      }
      break;
    }
    case RGBQuantum:
    case CbYCrQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetRedPixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetGreenPixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetBluePixelComponent(q,ScaleCharToQuantum(pixel));
            SetOpacityPixelComponent(q,OpaqueOpacity);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 10:
        {
          range=GetQuantumRange(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushLongPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ScaleAnyToQuantum((pixel >> 22) &
                  0x3ff,range));
                SetGreenPixelComponent(q,ScaleAnyToQuantum((pixel >> 12) &
                  0x3ff,range));
                SetBluePixelComponent(q,ScaleAnyToQuantum((pixel >> 2) &
                  0x3ff,range));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          if (quantum_info->quantum == 32U)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
            q++;
          }
          break;
        }
        case 12:
        {
          range=GetQuantumRange(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              unsigned short
                pixel;

              for (x=0; x < (ssize_t) (3*number_pixels-1); x+=2)
              {
                p=PushShortPixel(endian,p,&pixel);
                switch (x % 3)
                {
                  default:
                  case 0:
                  {
                    SetRedPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    break;
                  }
                  case 1:
                  {
                    SetGreenPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    break;
                  }
                  case 2:
                  {
                    SetBluePixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    q++;
                    break;
                  }
                }
                p=PushShortPixel(endian,p,&pixel);
                switch ((x+1) % 3)
                {
                  default:
                  case 0:
                  {
                    SetRedPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    break;
                  }
                  case 1:
                  {
                    SetGreenPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    break;
                  }
                  case 2:
                  {
                    SetBluePixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    q++;
                    break;
                  }
                }
                p+=quantum_info->pad;
              }
              for (bit=0; bit < (ssize_t) (3*number_pixels % 2); bit++)
              {
                p=PushShortPixel(endian,p,&pixel);
                switch ((x+bit) % 3)
                {
                  default:
                  case 0:
                  {
                    SetRedPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    break;
                  }
                  case 1:
                  {
                    SetGreenPixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    break;
                  }
                  case 2:
                  {
                    SetBluePixelComponent(q,ScaleAnyToQuantum((QuantumAny)
                      (pixel >> 4),range));
                    q++;
                    break;
                  }
                }
                p+=quantum_info->pad;
              }
              if (bit != 0)
                p++;
              break;
            }
          if (quantum_info->quantum == 32U)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
            q++;
          }
          break;
        }
      }
      break;
    }
    case RGBAQuantum:
    case RGBOQuantum:
    case CbYCrAQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetRedPixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetGreenPixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetBluePixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetAlphaPixelComponent(q,ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 10:
        {
          pixel=0;
          if (quantum_info->pack == MagickFalse)
            {
              register ssize_t
                i;

              size_t
                quantum;

              ssize_t
                n;

              n=0;
              quantum=0;
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                for (i=0; i < 4; i++)
                {
                  switch (n % 3)
                  {
                    case 0:
                    {
                      p=PushLongPixel(endian,p,&pixel);
                      quantum=(size_t) (ScaleShortToQuantum((unsigned short)
                        (((pixel >> 22) & 0x3ff) << 6)));
                      break;
                    }
                    case 1:
                    {
                      quantum=(size_t) (ScaleShortToQuantum((unsigned short)
                        (((pixel >> 12) & 0x3ff) << 6)));
                      break;
                    }
                    case 2:
                    {
                      quantum=(size_t) (ScaleShortToQuantum((unsigned short)
                        (((pixel >> 2) & 0x3ff) << 6)));
                      break;
                    }
                  }
                  switch (i)
                  {
                    case 0: SetRedPixelComponent(q,quantum); break;
                    case 1: SetGreenPixelComponent(q,quantum); break;
                    case 2: SetBluePixelComponent(q,quantum); break;
                    case 3: SetAlphaPixelComponent(q,quantum); break;
                  }
                  n++;
                }
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleShortToQuantum((unsigned short)
              (pixel << 6)));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleShortToQuantum((unsigned short)
              (pixel << 6)));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetBluePixelComponent(q,ScaleShortToQuantum((unsigned short)
              (pixel << 6)));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetAlphaPixelComponent(q,ScaleShortToQuantum((unsigned short)
              (pixel << 6)));
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetAlphaPixelComponent(q,ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetAlphaPixelComponent(q,ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetAlphaPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            q++;
          }
          break;
        }
      }
      break;
    }
    case CMYKQuantum:
    {
      if (image->colorspace != CMYKColorspace)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
            "ColorSeparatedImageRequired","`%s'",image->filename);
          return(extent);
        }
      switch (quantum_info->depth)
      {
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetRedPixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetGreenPixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetBluePixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetIndexPixelComponent(indexes+x,ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetIndexPixelComponent(indexes+x,ClampToQuantum(
                  (MagickRealType) QuantumRange*HalfToSinglePrecision(pixel)));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetIndexPixelComponent(indexes+x,ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetIndexPixelComponent(indexes+x,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetIndexPixelComponent(indexes+x,ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetIndexPixelComponent(indexes+x,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetIndexPixelComponent(indexes+x,ScaleAnyToQuantum(pixel,range));
            q++;
          }
          break;
        }
      }
      break;
    }
    case CMYKAQuantum:
    case CMYKOQuantum:
    {
      if (image->colorspace != CMYKColorspace)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
            "ColorSeparatedImageRequired","`%s'",image->filename);
          return(extent);
        }
      switch (quantum_info->depth)
      {
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetRedPixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetGreenPixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetBluePixelComponent(q,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetIndexPixelComponent(indexes+x,ScaleCharToQuantum(pixel));
            p=PushCharPixel(p,&pixel);
            SetAlphaPixelComponent(q,ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetIndexPixelComponent(indexes+x,ClampToQuantum(
                  (MagickRealType) QuantumRange*HalfToSinglePrecision(pixel)));
                p=PushShortPixel(endian,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum((MagickRealType)
                  QuantumRange*HalfToSinglePrecision(pixel)));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetIndexPixelComponent(indexes+x,ScaleShortToQuantum(pixel));
            p=PushShortPixel(endian,p,&pixel);
            SetAlphaPixelComponent(q,ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetIndexPixelComponent(indexes+x,ClampToQuantum(pixel));
                p=PushFloatPixel(&quantum_state,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            SetRedPixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetGreenPixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetBluePixelComponent(q,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetIndexPixelComponent(indexes+x,ScaleLongToQuantum(pixel));
            p=PushLongPixel(endian,p,&pixel);
            SetAlphaPixelComponent(q,ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetRedPixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetGreenPixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetBluePixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetIndexPixelComponent(indexes+x,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                SetAlphaPixelComponent(q,ClampToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetBluePixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetIndexPixelComponent(indexes+x,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetAlphaPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            q++;
          }
          break;
        }
      }
      break;
    }
    case CbYCrYQuantum:
    {
      switch (quantum_info->depth)
      {
        case 10:
        {
          Quantum
            cbcr[4];

          pixel=0;
          if (quantum_info->pack == MagickFalse)
            {
              register ssize_t
                i;

              size_t
                quantum;

              ssize_t
                n;

              n=0;
              quantum=0;
              for (x=0; x < (ssize_t) number_pixels; x+=2)
              {
                for (i=0; i < 4; i++)
                {
                  switch (n % 3)
                  {
                    case 0:
                    {
                      p=PushLongPixel(endian,p,&pixel);
                      quantum=(size_t) (ScaleShortToQuantum((unsigned short)
                        (((pixel >> 22) & 0x3ff) << 6)));
                      break;
                    }
                    case 1:
                    {
                      quantum=(size_t) (ScaleShortToQuantum((unsigned short)
                        (((pixel >> 12) & 0x3ff) << 6)));
                      break;
                    }
                    case 2:
                    {
                      quantum=(size_t) (ScaleShortToQuantum((unsigned short)
                        (((pixel >> 2) & 0x3ff) << 6)));
                      break;
                    }
                  }
                  cbcr[i]=(Quantum) (quantum);
                  n++;
                }
                p+=quantum_info->pad;
                SetRedPixelComponent(q,cbcr[1]);
                SetGreenPixelComponent(q,cbcr[0]);
                SetBluePixelComponent(q,cbcr[2]);
                q++;
                SetRedPixelComponent(q,cbcr[3]);
                SetGreenPixelComponent(q,cbcr[0]);
                SetBluePixelComponent(q,cbcr[2]);
                q++;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetRedPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            SetGreenPixelComponent(q,ScaleAnyToQuantum(pixel,range));
            q++;
          }
          break;
        }
      }
      break;
    }
    default:
      break;
  }
  if ((quantum_type == CbYCrQuantum) || (quantum_type == CbYCrAQuantum))
    {
      Quantum
        quantum;

      register PixelPacket
        *restrict q;

      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=GetCacheViewAuthenticPixelQueue(image_view);
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        quantum=GetRedPixelComponent(q);
        SetRedPixelComponent(q,GetGreenPixelComponent(q));
        SetGreenPixelComponent(q,quantum);
        q++;
      }
    }
  if ((quantum_type == RGBOQuantum) || (quantum_type == CMYKOQuantum))
    {
      register PixelPacket
        *restrict q;

      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=GetCacheViewAuthenticPixelQueue(image_view);
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        SetOpacityPixelComponent(q,GetAlphaPixelComponent(q));
        q++;
      }
    }
  if (quantum_info->alpha_type == DisassociatedQuantumAlpha)
    {
      MagickRealType
        alpha;

      register PixelPacket
        *restrict q;

      /*
        Disassociate alpha.
      */
      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=GetCacheViewAuthenticPixelQueue(image_view);
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        alpha=QuantumScale*GetAlphaPixelComponent(q);
        alpha=1.0/(fabs(alpha) <= MagickEpsilon ? 1.0 : alpha);
        SetRedPixelComponent(q,ClampToQuantum(alpha*
          GetRedPixelComponent(q)));
        SetGreenPixelComponent(q,ClampToQuantum(alpha*
          GetGreenPixelComponent(q)));
        SetBluePixelComponent(q,ClampToQuantum(alpha*
          GetBluePixelComponent(q)));
        q++;
      }
    }
  return(extent);
}

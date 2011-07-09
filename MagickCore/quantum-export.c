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
%                   EEEEE  X   X  PPPP    OOO   RRRR   TTTTT                  %
%                   E       X X   P   P  O   O  R   R    T                    %
%                   EEE      X    PPPP   O   O  RRRR     T                    %
%                   E       X X   P      O   O  R R      T                    %
%                   EEEEE  X   X  P       OOO   R  R     T                    %
%                                                                             %
%                 MagickCore Methods to Export Quantum Pixels                 %
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
#include "MagickCore/studio.h"
#include "MagickCore/property.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/color-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/constitute.h"
#include "MagickCore/delegate.h"
#include "MagickCore/geometry.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/statistic.h"
#include "MagickCore/stream.h"
#include "MagickCore/string_.h"
#include "MagickCore/utility.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E x p o r t Q u a n t u m P i x e l s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExportQuantumPixels() transfers one or more pixel components from the image
%  pixel cache to a user supplied buffer.  The pixels are returned in network
%  byte order.  MagickTrue is returned if the pixels are successfully
%  transferred, otherwise MagickFalse.
%
%  The format of the ExportQuantumPixels method is:
%
%      size_t ExportQuantumPixels(const Image *image,
%        const CacheView *image_view,const QuantumInfo *quantum_info,
%        const QuantumType quantum_type,unsigned char *pixels,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o image_view: the image cache view.
%
%    o quantum_info: the quantum info.
%
%    o quantum_type: Declare which pixel components to transfer (RGB, RGBA,
%      etc).
%
%    o pixels:  The components are transferred to this buffer.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline unsigned char *PopDoublePixel(const QuantumState *quantum_state,
  const double pixel,unsigned char *pixels)
{
  double
    *p;

  unsigned char
    quantum[8];

  p=(double *) quantum;
  *p=(double) (pixel*quantum_state->inverse_scale+quantum_state->minimum);
  if (quantum_state->endian != LSBEndian)
    {
      *pixels++=quantum[7];
      *pixels++=quantum[6];
      *pixels++=quantum[5];
      *pixels++=quantum[4];
      *pixels++=quantum[3];
      *pixels++=quantum[2];
      *pixels++=quantum[1];
      *pixels++=quantum[0];
      return(pixels);
    }
  *pixels++=quantum[0];
  *pixels++=quantum[1];
  *pixels++=quantum[2];
  *pixels++=quantum[3];
  *pixels++=quantum[4];
  *pixels++=quantum[5];
  *pixels++=quantum[6];
  *pixels++=quantum[7];
  return(pixels);
}

static inline unsigned char *PopFloatPixel(const QuantumState *quantum_state,
  const float pixel,unsigned char *pixels)
{
  float
    *p;

  unsigned char
    quantum[4];

  p=(float *) quantum;
  *p=(float) ((double) pixel*quantum_state->inverse_scale+
    quantum_state->minimum);
  if (quantum_state->endian != LSBEndian)
    {
      *pixels++=quantum[3];
      *pixels++=quantum[2];
      *pixels++=quantum[1];
      *pixels++=quantum[0];
      return(pixels);
    }
  *pixels++=quantum[0];
  *pixels++=quantum[1];
  *pixels++=quantum[2];
  *pixels++=quantum[3];
  return(pixels);
}

static inline unsigned char *PopQuantumPixel(QuantumState *quantum_state,
  const size_t depth,const QuantumAny pixel,unsigned char *pixels)
{
  register ssize_t
    i;

  size_t
    quantum_bits;

  if (quantum_state->bits == 0UL)
    quantum_state->bits=8U;
  for (i=(ssize_t) depth; i > 0L; )
  {
    quantum_bits=(size_t) i;
    if (quantum_bits > quantum_state->bits)
      quantum_bits=quantum_state->bits;
    i-=(ssize_t) quantum_bits;
    if (quantum_state->bits == 8UL)
      *pixels='\0';
    quantum_state->bits-=quantum_bits;
    *pixels|=(((pixel >> i) &~ ((~0UL) << quantum_bits)) <<
      quantum_state->bits);
    if (quantum_state->bits == 0UL)
      {
        pixels++;
        quantum_state->bits=8UL;
      }
  }
  return(pixels);
}

static inline unsigned char *PopQuantumLongPixel(QuantumState *quantum_state,
  const size_t depth,const size_t pixel,unsigned char *pixels)
{
  register ssize_t
    i;

  size_t
    quantum_bits;

  if (quantum_state->bits == 0U)
    quantum_state->bits=32UL;
  for (i=(ssize_t) depth; i > 0; )
  {
    quantum_bits=(size_t) i;
    if (quantum_bits > quantum_state->bits)
      quantum_bits=quantum_state->bits;
    quantum_state->pixel|=(((pixel >> (depth-i)) &
      quantum_state->mask[quantum_bits]) << (32U-quantum_state->bits));
    i-=(ssize_t) quantum_bits;
    quantum_state->bits-=quantum_bits;
    if (quantum_state->bits == 0U)
      {
        pixels=PopLongPixel(quantum_state->endian,quantum_state->pixel,pixels);
        quantum_state->pixel=0U;
        quantum_state->bits=32U;
      }
  }
  return(pixels);
}

MagickExport size_t ExportQuantumPixels(Image *image,CacheView *image_view,
  const QuantumInfo *quantum_info,const QuantumType quantum_type,
  unsigned char *pixels,ExceptionInfo *exception)
{
  EndianType
    endian;

  MagickRealType
    alpha;

  MagickSizeType
    number_pixels;

  QuantumAny
    range;

  QuantumState
    quantum_state;

  register const Quantum
    *restrict p;

  register ssize_t
    x;

  register unsigned char
    *restrict q;

  size_t
    channels,
    extent;

  ssize_t
    bit;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  if (pixels == (unsigned char *) NULL)
    pixels=GetQuantumPixels(quantum_info);
  if (image_view == (CacheView *) NULL)
    {
      number_pixels=GetImageExtent(image);
      p=GetVirtualPixelQueue(image);
      channels=GetPixelComponents(image);
    }
  else
    {
      number_pixels=GetCacheViewExtent(image_view);
      p=GetCacheViewVirtualPixelQueue(image_view);
      channels=GetPixelComponents(image);
    }
  if (quantum_info->alpha_type == AssociatedQuantumAlpha)
    {
      register Quantum
        *restrict q;

      /*
        Associate alpha.
      */
      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=GetCacheViewAuthenticPixelQueue(image_view);
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        alpha=QuantumScale*GetPixelAlpha(image,q);
        SetPixelRed(image,ClampToQuantum(alpha*GetPixelRed(image,q)),q);
        SetPixelGreen(image,ClampToQuantum(alpha*GetPixelGreen(image,q)),q);
        SetPixelBlue(image,ClampToQuantum(alpha*GetPixelBlue(image,q)),q);
        q++;
      }
    }
  if ((quantum_type == RGBOQuantum) || (quantum_type == CMYKOQuantum) ||
      (quantum_type == BGROQuantum))
    {
      register Quantum
        *restrict q;

      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=GetCacheViewAuthenticPixelQueue(image_view);
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        SetPixelAlpha(image,GetPixelAlpha(image,q),q);
        q++;
      }
    }
  if ((quantum_type == CbYCrQuantum) || (quantum_type == CbYCrAQuantum))
    {
      Quantum
        quantum;

      register Quantum
        *restrict q;

      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=GetAuthenticPixelQueue(image);
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        quantum=GetPixelRed(image,q);
        SetPixelRed(image,GetPixelGreen(image,q),q);
        SetPixelGreen(image,quantum,q);
        q+=channels;
      }
    }
  x=0;
  q=pixels;
  InitializeQuantumState(quantum_info,image->endian,&quantum_state);
  extent=GetQuantumExtent(image,quantum_info,quantum_type);
  endian=quantum_state.endian;
  switch (quantum_type)
  {
    case IndexQuantum:
    {
      if (image->storage_class != PseudoClass)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
            "ColormappedImageRequired","`%s'",image->filename);
          return(extent);
        }
      switch (quantum_info->depth)
      {
        case 1:
        {
          register unsigned char
            pixel;

          for (x=((ssize_t) number_pixels-7); x > 0; x-=8)
          {
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q=((pixel & 0x01) << 7);
            p+=channels;
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q|=((pixel & 0x01) << 6);
            p+=channels;
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q|=((pixel & 0x01) << 5);
            p+=channels;
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q|=((pixel & 0x01) << 4);
            p+=channels;
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q|=((pixel & 0x01) << 3);
            p+=channels;
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q|=((pixel & 0x01) << 2);
            p+=channels;
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q|=((pixel & 0x01) << 1);
            p+=channels;
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q|=((pixel & 0x01) << 0);
            p+=channels;
            q++;
          }
          if ((number_pixels % 8) != 0)
            {
              *q='\0';
              for (bit=7; bit >= (ssize_t) (8-(number_pixels % 8)); bit--)
              {
                pixel=(unsigned char) GetPixelIndex(image,p);
                *q|=((pixel & 0x01) << (unsigned char) bit);
                p+=channels;
              }
              q++;
            }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) (number_pixels-1) ; x+=2)
          {
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q=((pixel & 0xf) << 4);
            p+=channels;
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q|=((pixel & 0xf) << 0);
            p+=channels;
            q++;
          }
          if ((number_pixels % 2) != 0)
            {
              pixel=(unsigned char) GetPixelIndex(image,p);
              *q=((pixel & 0xf) << 4);
              p+=channels;
              q++;
            }
          break;
        }
        case 8:
        {
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopCharPixel((unsigned char) GetPixelIndex(image,p),q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopShortPixel(endian,SinglePrecisionToHalf(QuantumScale*
                  GetPixelIndex(image,p)),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopShortPixel(endian,(unsigned short) GetPixelIndex(image,p),q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelIndex(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopLongPixel(endian,(unsigned int) GetPixelIndex(image,p),q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelIndex(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              GetPixelIndex(image,p),q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case IndexAlphaQuantum:
    {
      if (image->storage_class != PseudoClass)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
            "ColormappedImageRequired","`%s'",image->filename);
          return(extent);
        }
      switch (quantum_info->depth)
      {
        case 1:
        {
          register unsigned char
            pixel;

          for (x=((ssize_t) number_pixels-3); x > 0; x-=4)
          {
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q=((pixel & 0x01) << 7);
            pixel=(unsigned char) (GetPixelAlpha(image,p) == (Quantum)
              TransparentAlpha ? 1 : 0);
            *q|=((pixel & 0x01) << 6);
            p+=channels;
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q|=((pixel & 0x01) << 5);
            pixel=(unsigned char) (GetPixelAlpha(image,p) == (Quantum)
              TransparentAlpha ? 1 : 0);
            *q|=((pixel & 0x01) << 4);
            p+=channels;
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q|=((pixel & 0x01) << 3);
            pixel=(unsigned char) (GetPixelAlpha(image,p) == (Quantum)
              TransparentAlpha ? 1 : 0);
            *q|=((pixel & 0x01) << 2);
            p+=channels;
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q|=((pixel & 0x01) << 1);
            pixel=(unsigned char) (GetPixelAlpha(image,p) == (Quantum)
              TransparentAlpha ? 1 : 0);
            *q|=((pixel & 0x01) << 0);
            p+=channels;
            q++;
          }
          if ((number_pixels % 4) != 0)
            {
              *q='\0';
              for (bit=3; bit >= (ssize_t) (4-(number_pixels % 4)); bit-=2)
              {
                pixel=(unsigned char) GetPixelIndex(image,p);
                *q|=((pixel & 0x01) << (unsigned char) (bit+4));
                pixel=(unsigned char) (GetPixelAlpha(image,p) == (Quantum)
                  TransparentAlpha ? 1 : 0);
                *q|=((pixel & 0x01) << (unsigned char) (bit+4-1));
                p+=channels;
              }
              q++;
            }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels ; x++)
          {
            pixel=(unsigned char) GetPixelIndex(image,p);
            *q=((pixel & 0xf) << 4);
            pixel=(unsigned char) (16*QuantumScale*GetPixelAlpha(image,p)+0.5);
            *q|=((pixel & 0xf) << 0);
            p+=channels;
            q++;
          }
          break;
        }
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopCharPixel((unsigned char) GetPixelIndex(image,p),q);
            pixel=ScaleQuantumToChar(GetPixelAlpha(image,p));
            q=PopCharPixel(pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopShortPixel(endian,(unsigned short)
                  GetPixelIndex(image,p),q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelAlpha(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopShortPixel(endian,(unsigned short) GetPixelIndex(image,p),q);
            pixel=ScaleQuantumToShort(GetPixelAlpha(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                float
                  pixel;

                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelIndex(image,p),q);
                pixel=(float)  GetPixelAlpha(image,p);
                q=PopFloatPixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopLongPixel(endian,(unsigned int) GetPixelIndex(image,p),q);
            pixel=ScaleQuantumToLong(GetPixelAlpha(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                double
                  pixel;

                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelIndex(image,p),q);
                pixel=(double) GetPixelAlpha(image,p);
                q=PopDoublePixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              GetPixelIndex(image,p),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelAlpha(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case BGRQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopCharPixel(ScaleQuantumToChar(GetPixelBlue(image,p)),q);
            q=PopCharPixel(ScaleQuantumToChar(GetPixelGreen(image,p)),q);
            q=PopCharPixel(ScaleQuantumToChar(GetPixelRed(image,p)),q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 10:
        {
          register unsigned int
            pixel;

          range=GetQuantumRange(quantum_info->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=(unsigned int) (
                  ScaleQuantumToAny(GetPixelRed(image,p),range) << 22 |
                  ScaleQuantumToAny(GetPixelGreen(image,p),range) << 12 |
                  ScaleQuantumToAny(GetPixelBlue(image,p),range) << 2);
                q=PopLongPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          if (quantum_info->quantum == 32UL)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 12:
        {
          register unsigned int
            pixel;

          range=GetQuantumRange(quantum_info->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (ssize_t) (3*number_pixels-1); x+=2)
              {
                switch (x % 3)
                {
                  default:
                  case 0:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelRed(image,p),range);
                    break;
                  }
                  case 1:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelGreen(image,p),range);
                    break;
                  }
                  case 2:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelBlue(image,p),range);
                    p+=channels;
                    break;
                  }
                }
                q=PopShortPixel(endian,(unsigned short) (pixel << 4),q);
                switch ((x+1) % 3)
                {
                  default:
                  case 0:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelRed(image,p),range);
                    break;
                  }
                  case 1:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelGreen(image,p),range);
                    break;
                  }
                  case 2:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelBlue(image,p),range);
                    p+=channels;
                    break;
                  }
                }
                q=PopShortPixel(endian,(unsigned short) (pixel << 4),q);
                q+=quantum_info->pad;
              }
              for (bit=0; bit < (ssize_t) (3*number_pixels % 2); bit++)
              {
                switch ((x+bit) % 3)
                {
                  default:
                  case 0:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelRed(image,p),range);
                    break;
                  }
                  case 1:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelGreen(image,p),range);
                    break;
                  }
                  case 2:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelBlue(image,p),range);
                    p+=channels;
                    break;
                  }
                }
                q=PopShortPixel(endian,(unsigned short) (pixel << 4),q);
                q+=quantum_info->pad;
              }
              if (bit != 0)
                p+=channels;
              break;
            }
          if (quantum_info->quantum == 32UL)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelBlue(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelGreen(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelRed(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelBlue(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelGreen(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelRed(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelRed(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelGreen(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelBlue(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelBlue(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelGreen(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelRed(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelRed(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelGreen(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelBlue(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelRed(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelGreen(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelBlue(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
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
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetPixelBlue(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelGreen(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelRed(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelAlpha(image,p));
            q=PopCharPixel(pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 10:
        {
          register unsigned int
            pixel;

          range=GetQuantumRange(quantum_info->depth);
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
              pixel=0;
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                for (i=0; i < 4; i++)
                {
                  switch (i)
                  {
                    case 0: quantum=GetPixelRed(image,p); break;
                    case 1: quantum=GetPixelGreen(image,p); break;
                    case 2: quantum=GetPixelBlue(image,p); break;
                    case 3: quantum=GetPixelAlpha(image,p); break;
                  }
                  switch (n % 3)
                  {
                    case 0:
                    {
                      pixel|=(size_t) (ScaleQuantumToAny((Quantum) quantum,
                        range) << 22);
                      break;
                    }
                    case 1:
                    {
                      pixel|=(size_t) (ScaleQuantumToAny((Quantum) quantum,
                        range) << 12);
                      break;
                    }
                    case 2:
                    {
                      pixel|=(size_t) (ScaleQuantumToAny((Quantum) quantum,
                        range) << 2);
                      q=PopLongPixel(endian,pixel,q);
                      pixel=0;
                      break;
                    }
                  }
                  n++;
                }
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          if (quantum_info->quantum == 32UL)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelAlpha(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelAlpha(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelBlue(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelGreen(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelRed(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelAlpha(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelBlue(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelGreen(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelRed(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelAlpha(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                float
                  pixel;

                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelRed(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelGreen(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelBlue(image,p),q);
                pixel=(float) GetPixelAlpha(image,p);
                q=PopFloatPixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelBlue(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelGreen(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelRed(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelAlpha(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
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
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelRed(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelGreen(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelBlue(image,p),q);
                pixel=(double) GetPixelAlpha(image,p);
                q=PopDoublePixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelBlue(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelGreen(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelRed(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelAlpha(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
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
            threshold;

          register unsigned char
            black,
            white;

          black=0x00;
          white=0x01;
          if (quantum_info->min_is_white != MagickFalse)
            {
              black=0x01;
              white=0x00;
            }
          threshold=(Quantum) (QuantumRange/2);
          for (x=((ssize_t) number_pixels-7); x > 0; x-=8)
          {
            *q='\0';
            *q|=(GetPixelIntensity(image,p) < threshold ? black : white) << 7;
            p+=channels;
            *q|=(GetPixelIntensity(image,p) < threshold ? black : white) << 6;
            p+=channels;
            *q|=(GetPixelIntensity(image,p) < threshold ? black : white) << 5;
            p+=channels;
            *q|=(GetPixelIntensity(image,p) < threshold ? black : white) << 4;
            p+=channels;
            *q|=(GetPixelIntensity(image,p) < threshold ? black : white) << 3;
            p+=channels;
            *q|=(GetPixelIntensity(image,p) < threshold ? black : white) << 2;
            p+=channels;
            *q|=(GetPixelIntensity(image,p) < threshold ? black : white) << 1;
            p+=channels;
            *q|=(GetPixelIntensity(image,p) < threshold ? black : white) << 0;
            p+=channels;
            q++;
          }
          if ((number_pixels % 8) != 0)
            {
              *q='\0';
              for (bit=7; bit >= (ssize_t) (8-(number_pixels % 8)); bit--)
              {
                *q|=(GetPixelIntensity(image,p) < threshold ? black : white) <<
                  bit;
                p+=channels;
              }
              q++;
            }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) (number_pixels-1) ; x+=2)
          {
            pixel=ScaleQuantumToChar(GetPixelIntensity(image,p));
            *q=(((pixel >> 4) & 0xf) << 4);
            p+=channels;
            pixel=ScaleQuantumToChar(GetPixelIntensity(image,p));
            *q|=pixel >> 4;
            p+=channels;
            q++;
          }
          if ((number_pixels % 2) != 0)
            {
              pixel=ScaleQuantumToChar(GetPixelIntensity(image,p));
              *q=(((pixel >> 4) & 0xf) << 4);
              p+=channels;
              q++;
            }
          break;
        }
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetPixelIntensity(image,p));
            q=PopCharPixel(pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 10:
        {
          range=GetQuantumRange(quantum_info->depth);
          if (quantum_info->pack == MagickFalse)
            {
              register unsigned int
                pixel;

              for (x=0; x < (ssize_t) (number_pixels-2); x+=3)
              {
                pixel=(unsigned int) (
                  ScaleQuantumToAny(GetPixelIntensity(image,p+2),range) << 22 |
                  ScaleQuantumToAny(GetPixelIntensity(image,p+1),range) << 12 |
                  ScaleQuantumToAny(GetPixelIntensity(image,p+0),range) << 2);
                q=PopLongPixel(endian,pixel,q);
                p+=3;
                q+=quantum_info->pad;
              }
              pixel=0UL;
              if (x++ < (ssize_t) (number_pixels-1))
                pixel|=ScaleQuantumToAny(GetPixelIntensity(image,p+1),
                  range) << 12;
              if (x++ < (ssize_t) number_pixels)
                pixel|=ScaleQuantumToAny(GetPixelIntensity(image,p+0),
                  range) << 2;
              q=PopLongPixel(endian,pixel,q);
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelIntensity(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 12:
        {
          register unsigned short
            pixel;

          range=GetQuantumRange(quantum_info->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=ScaleQuantumToShort(GetPixelIntensity(image,p));
                q=PopShortPixel(endian,(unsigned short) (pixel >> 4),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelIntensity(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelIntensity(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelIntensity(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                float
                  pixel;

                pixel=(float) GetPixelIntensity(image,p);
                q=PopFloatPixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelIntensity(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                double
                  pixel;

                pixel=(double) GetPixelIntensity(image,p);
                q=PopDoublePixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelIntensity(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
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
          register Quantum
            threshold;

          register unsigned char
            black,
            pixel,
            white;

          black=0x00;
          white=0x01;
          if (quantum_info->min_is_white == MagickFalse)
            {
              black=0x01;
              white=0x00;
            }
          threshold=(Quantum) (QuantumRange/2);
          for (x=((ssize_t) number_pixels-3); x > 0; x-=4)
          {
            *q='\0';
            *q|=(GetPixelIntensity(image,p) > threshold ? black : white) << 7;
            pixel=(unsigned char) (GetPixelAlpha(image,p) == OpaqueAlpha ?
              0x00 : 0x01);
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 6);
            p+=channels;
            *q|=(GetPixelIntensity(image,p) > threshold ? black : white) << 5;
            pixel=(unsigned char) (GetPixelAlpha(image,p) == OpaqueAlpha ?
              0x00 : 0x01);
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 4);
            p+=channels;
            *q|=(GetPixelIntensity(image,p) > threshold ? black : white) << 3;
            pixel=(unsigned char) (GetPixelAlpha(image,p) == OpaqueAlpha ?
              0x00 : 0x01);
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 2);
            p+=channels;
            *q|=(GetPixelIntensity(image,p) > threshold ? black : white) << 1;
            pixel=(unsigned char) (GetPixelAlpha(image,p) == OpaqueAlpha ?
              0x00 : 0x01);
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 0);
            p+=channels;
            q++;
          }
          if ((number_pixels % 4) != 0)
            {
              *q='\0';
              for (bit=0; bit <= (ssize_t) (number_pixels % 4); bit+=2)
              {
                *q|=(GetPixelIntensity(image,p) > threshold ? black : white) <<
                  (7-bit);
                pixel=(unsigned char) (GetPixelAlpha(image,p) == OpaqueAlpha ?
                  0x00 : 0x01);
                *q|=(((int) pixel != 0 ? 0x00 : 0x01) << (unsigned char)
                  (7-bit-1));
                p+=channels;
              }
              q++;
            }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels ; x++)
          {
            pixel=ScaleQuantumToChar(GetPixelIntensity(image,p));
            *q=(((pixel >> 4) & 0xf) << 4);
            pixel=(unsigned char) (16*QuantumScale*GetPixelAlpha(image,p)+0.5);
            *q|=pixel & 0xf;
            p+=channels;
            q++;
          }
          break;
        }
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetPixelIntensity(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelAlpha(image,p));
            q=PopCharPixel(pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelIntensity(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelAlpha(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelIntensity(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelAlpha(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                float
                  pixel;

                pixel=(float) GetPixelIntensity(image,p);
                q=PopFloatPixel(&quantum_state,pixel,q);
                pixel=(float) (GetPixelAlpha(image,p));
                q=PopFloatPixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelIntensity(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelAlpha(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                double
                  pixel;

                pixel=(double) GetPixelIntensity(image,p);
                q=PopDoublePixel(&quantum_state,pixel,q);
                pixel=(double) (GetPixelAlpha(image,p));
                q=PopDoublePixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelIntensity(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelAlpha(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
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
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetPixelRed(image,p));
            q=PopCharPixel(pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelRed(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelRed(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelRed(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelRed(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelRed(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelRed(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
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
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetPixelGreen(image,p));
            q=PopCharPixel(pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelGreen(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelGreen(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelGreen(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelGreen(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelGreen(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelGreen(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
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
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetPixelBlue(image,p));
            q=PopCharPixel(pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelBlue(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelBlue(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelBlue(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelBlue(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelBlue(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelBlue(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
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
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetPixelAlpha(image,p));
            q=PopCharPixel(pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelAlpha(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelAlpha(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                float
                  pixel;

                pixel=(float) GetPixelAlpha(image,p);
                q=PopFloatPixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelAlpha(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                double
                  pixel;

                pixel=(double) (GetPixelAlpha(image,p));
                q=PopDoublePixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelAlpha(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case OpacityQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetPixelAlpha(image,p));
            q=PopCharPixel(pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelAlpha(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelAlpha(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelAlpha(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelAlpha(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelAlpha(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelAlpha(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
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
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetPixelBlack(image,p));
            q=PopCharPixel(pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelBlack(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelBlack(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelBlack(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelBlack(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelBlack(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny((Quantum) GetPixelBlack(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopCharPixel(ScaleQuantumToChar(GetPixelRed(image,p)),q);
            q=PopCharPixel(ScaleQuantumToChar(GetPixelGreen(image,p)),q);
            q=PopCharPixel(ScaleQuantumToChar(GetPixelBlue(image,p)),q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 10:
        {
          register unsigned int
            pixel;

          range=GetQuantumRange(quantum_info->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=(unsigned int) (
                  ScaleQuantumToAny(GetPixelRed(image,p),range) << 22 |
                  ScaleQuantumToAny(GetPixelGreen(image,p),range) << 12 |
                  ScaleQuantumToAny(GetPixelBlue(image,p),range) << 2);
                q=PopLongPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          if (quantum_info->quantum == 32UL)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 12:
        {
          register unsigned int
            pixel;

          range=GetQuantumRange(quantum_info->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (ssize_t) (3*number_pixels-1); x+=2)
              {
                switch (x % 3)
                {
                  default:
                  case 0:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelRed(image,p),range);
                    break;
                  }
                  case 1:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelGreen(image,p),range);
                    break;
                  }
                  case 2:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelBlue(image,p),range);
                    p+=channels;
                    break;
                  }
                }
                q=PopShortPixel(endian,(unsigned short) (pixel << 4),q);
                switch ((x+1) % 3)
                {
                  default:
                  case 0:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelRed(image,p),range);
                    break;
                  }
                  case 1:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelGreen(image,p),range);
                    break;
                  }
                  case 2:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelBlue(image,p),range);
                    p+=channels;
                    break;
                  }
                }
                q=PopShortPixel(endian,(unsigned short) (pixel << 4),q);
                q+=quantum_info->pad;
              }
              for (bit=0; bit < (ssize_t) (3*number_pixels % 2); bit++)
              {
                switch ((x+bit) % 3)
                {
                  default:
                  case 0:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelRed(image,p),range);
                    break;
                  }
                  case 1:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelGreen(image,p),range);
                    break;
                  }
                  case 2:
                  {
                    pixel=(unsigned int) ScaleQuantumToAny(
                      GetPixelBlue(image,p),range);
                    p+=channels;
                    break;
                  }
                }
                q=PopShortPixel(endian,(unsigned short) (pixel << 4),q);
                q+=quantum_info->pad;
              }
              if (bit != 0)
                p+=channels;
              break;
            }
          if (quantum_info->quantum == 32UL)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(image,p),
              range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelRed(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelGreen(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelBlue(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelRed(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelGreen(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelBlue(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelRed(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelGreen(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelBlue(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelRed(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelGreen(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelBlue(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelRed(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelGreen(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelBlue(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelRed(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelGreen(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelBlue(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
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
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetPixelRed(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelGreen(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelBlue(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelAlpha(image,p));
            q=PopCharPixel(pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 10:
        {
          register unsigned int
            pixel;

          range=GetQuantumRange(quantum_info->depth);
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
              pixel=0;
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                for (i=0; i < 4; i++)
                {
                  switch (i)
                  {
                    case 0: quantum=GetPixelRed(image,p); break;
                    case 1: quantum=GetPixelGreen(image,p); break;
                    case 2: quantum=GetPixelBlue(image,p); break;
                    case 3: quantum=GetPixelAlpha(image,p); break;
                  }
                  switch (n % 3)
                  {
                    case 0:
                    {
                      pixel|=(size_t) (ScaleQuantumToAny((Quantum) quantum,
                        range) << 22);
                      break;
                    }
                    case 1:
                    {
                      pixel|=(size_t) (ScaleQuantumToAny((Quantum) quantum,
                        range) << 12);
                      break;
                    }
                    case 2:
                    {
                      pixel|=(size_t) (ScaleQuantumToAny((Quantum) quantum,
                        range) << 2);
                      q=PopLongPixel(endian,pixel,q);
                      pixel=0;
                      break;
                    }
                  }
                  n++;
                }
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          if (quantum_info->quantum == 32UL)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelAlpha(image,p),
                  range);
                q=PopQuantumLongPixel(&quantum_state,quantum_info->depth,pixel,
                  q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned int) ScaleQuantumToAny(
              GetPixelRed(image,p),range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(
              GetPixelGreen(image,p),range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(
              GetPixelBlue(image,p),range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(
              GetPixelAlpha(image,p),range);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelRed(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelGreen(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelBlue(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelAlpha(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelRed(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelGreen(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelBlue(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelAlpha(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                float
                  pixel;

                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelRed(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelGreen(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelBlue(image,p),q);
                pixel=(float) GetPixelAlpha(image,p);
                q=PopFloatPixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelRed(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelGreen(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelBlue(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelAlpha(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
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
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelRed(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelGreen(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelBlue(image,p),q);
                pixel=(double) GetPixelAlpha(image,p);
                q=PopDoublePixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelRed(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelGreen(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelBlue(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelAlpha(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
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
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetPixelRed(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelGreen(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelBlue(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelBlack(image,p));
            q=PopCharPixel(pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelRed(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelGreen(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelBlue(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelBlack(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelRed(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelGreen(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelBlue(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelBlack(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelRed(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelGreen(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelBlue(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelBlack(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelRed(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelGreen(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelBlue(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelBlack(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelRed(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelGreen(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelBlue(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelBlack(image,p),q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelRed(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelGreen(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelBlue(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelBlack(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
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
          register unsigned char
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetPixelRed(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelGreen(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelBlue(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelBlack(image,p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetPixelAlpha(image,p));
            q=PopCharPixel(pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelRed(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelGreen(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelBlue(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelBlack(image,p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetPixelAlpha(image,p));
                q=PopShortPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetPixelRed(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelGreen(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelBlue(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelBlack(image,p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetPixelAlpha(image,p));
            q=PopShortPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned int
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (ssize_t) number_pixels; x++)
              {
                float
                  pixel;

                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelRed(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelGreen(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelBlue(image,p),q);
                q=PopFloatPixel(&quantum_state,(float)
                  GetPixelBlack(image,p),q);
                pixel=(float) (GetPixelAlpha(image,p));
                q=PopFloatPixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetPixelRed(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelGreen(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelBlue(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelBlack(image,p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetPixelAlpha(image,p));
            q=PopLongPixel(endian,pixel,q);
            p+=channels;
            q+=quantum_info->pad;
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
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelRed(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelGreen(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelBlue(image,p),q);
                q=PopDoublePixel(&quantum_state,(double)
                  GetPixelBlack(image,p),q);
                pixel=(double) (GetPixelAlpha(image,p));
                q=PopDoublePixel(&quantum_state,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(quantum_info->depth);
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelRed(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelGreen(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelBlue(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelBlack(image,p),range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(GetPixelAlpha(image,p),range),q);
            p+=channels;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case CbYCrYQuantum:
    {
      Quantum
        cbcr[4];

      register ssize_t
        i;

      register unsigned int
        pixel;

      size_t
        quantum;

     ssize_t
        n;

      n=0;
      quantum=0;
      range=GetQuantumRange(quantum_info->depth);
      switch (quantum_info->depth)
      {
        case 10:
        {
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (ssize_t) number_pixels; x+=2)
              {
                for (i=0; i < 4; i++)
                {
                  switch (n % 3)
                  {
                    case 0:
                    {
                      quantum=GetPixelRed(image,p);
                      break;
                    }
                    case 1:
                    {
                      quantum=GetPixelGreen(image,p);
                      break;
                    }
                    case 2:
                    {
                      quantum=GetPixelBlue(image,p);
                      break;
                    }
                  }
                  cbcr[i]=(Quantum) quantum;
                  n++;
                }
                pixel=(unsigned int) ((size_t) (cbcr[1]) << 22 | (size_t)
                  (cbcr[0]) << 12 | (size_t) (cbcr[2]) << 2);
                q=PopLongPixel(endian,pixel,q);
                p+=channels;
                pixel=(unsigned int) ((size_t) (cbcr[3]) << 22 | (size_t)
                  (cbcr[0]) << 12 | (size_t) (cbcr[2]) << 2);
                q=PopLongPixel(endian,pixel,q);
                p+=channels;
                q+=quantum_info->pad;
              }
              break;
            }
          break;
        }
        default:
        {
          for (x=0; x < (ssize_t) number_pixels; x+=2)
          {
            for (i=0; i < 4; i++)
            {
              switch (n % 3)
              {
                case 0:
                {
                  quantum=GetPixelRed(image,p);
                  break;
                }
                case 1:
                {
                  quantum=GetPixelGreen(image,p);
                  break;
                }
                case 2:
                {
                  quantum=GetPixelBlue(image,p);
                  break;
                }
              }
              cbcr[i]=(Quantum) quantum;
              n++;
            }
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(cbcr[1],range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(cbcr[0],range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(cbcr[2],range),q);
            p+=channels;
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(cbcr[3],range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(cbcr[0],range),q);
            q=PopQuantumPixel(&quantum_state,quantum_info->depth,
              ScaleQuantumToAny(cbcr[2],range),q);
            p+=channels;
            q+=quantum_info->pad;
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

      register Quantum
        *restrict q;

      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=GetCacheViewAuthenticPixelQueue(image_view);
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        quantum=GetPixelRed(image,q);
        SetPixelRed(image,GetPixelGreen(image,q),q);
        SetPixelGreen(image,quantum,q);
        q+=channels;
      }
    }
  if ((quantum_type == RGBOQuantum) || (quantum_type == CMYKOQuantum) ||
      (quantum_type == BGROQuantum))
    {
      register Quantum
        *restrict q;

      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=GetCacheViewAuthenticPixelQueue(image_view);
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        SetPixelAlpha(image,GetPixelAlpha(image,q),q);
        q+=channels;
      }
    }
  return(extent);
}

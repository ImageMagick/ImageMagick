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
  const unsigned long depth,const QuantumAny pixel,unsigned char *pixels)
{
  register long
    i;

  register unsigned long
    quantum_bits;

  if (quantum_state->bits == 0UL)
    quantum_state->bits=8UL;
  for (i=(long) depth; i > 0L; )
  {
    quantum_bits=(unsigned long) i;
    if (quantum_bits > quantum_state->bits)
      quantum_bits=quantum_state->bits;
    i-=quantum_bits;
    if (quantum_state->bits == 8)
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
  const unsigned long depth,const unsigned long pixel,unsigned char *pixels)
{
  register long
    i;

  unsigned long
    quantum_bits;

  if (quantum_state->bits == 0UL)
    quantum_state->bits=32UL;
  for (i=(long) depth; i > 0; )
  {
    quantum_bits=(unsigned long) i;
    if (quantum_bits > quantum_state->bits)
      quantum_bits=quantum_state->bits;
    quantum_state->pixel|=(((pixel >> (depth-i)) &
      quantum_state->mask[quantum_bits]) << (32UL-quantum_state->bits));
    i-=quantum_bits;
    quantum_state->bits-=quantum_bits;
    if (quantum_state->bits == 0U)
      {
        pixels=PopLongPixel(quantum_state->endian,quantum_state->pixel,pixels);
        quantum_state->pixel=0U;
        quantum_state->bits=32UL;
      }
  }
  return(pixels);
}

MagickExport size_t ExportQuantumPixels(const Image *image,
  const CacheView *image_view,const QuantumInfo *quantum_info,
  const QuantumType quantum_type,unsigned char *pixels,ExceptionInfo *exception)
{
  EndianType
    endian;

  long
    bit;

  MagickRealType
    alpha;

  MagickSizeType
    number_pixels;

  QuantumAny
    range;

  QuantumState
    quantum_state;

  register const IndexPacket
    *restrict indexes;

  register const PixelPacket
    *restrict p;

  register long
    x;

  register unsigned char
    *restrict q;

  size_t
    extent;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  if (pixels == (unsigned char *) NULL)
    pixels=GetQuantumPixels(quantum_info);
  number_pixels=GetImageExtent(image);
  p=GetVirtualPixelQueue(image);
  indexes=GetVirtualIndexQueue(image);
  if (image_view != (CacheView *) NULL)
    {
      number_pixels=GetCacheViewExtent(image_view);
      p=GetCacheViewVirtualPixelQueue(image_view);
      indexes=GetCacheViewVirtualIndexQueue(image_view);
    }
  if (quantum_info->alpha_type == AssociatedQuantumAlpha)
    {
      register PixelPacket
        *restrict q;

      /*
        Associate alpha.
      */
      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=(PixelPacket *) GetCacheViewVirtualPixelQueue(image_view);
      for (x=0; x < (long) image->columns; x++)
      {
        alpha=QuantumScale*((double) QuantumRange-q->opacity);
        q->red=ClampToQuantum(alpha*q->red);
        q->green=ClampToQuantum(alpha*q->green);
        q->blue=ClampToQuantum(alpha*q->blue);
        q++;
      }
    }
  if ((quantum_type == RGBOQuantum) || (quantum_type == CMYKOQuantum))
    {
      register PixelPacket
        *restrict q;

      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=(PixelPacket *) GetCacheViewVirtualPixelQueue(image_view);
      for (x=0; x < (long) number_pixels; x++)
      {
        q->opacity=(Quantum) GetAlphaPixelComponent(q);
        q++;
      }
    }
  if ((quantum_type == CbYCrQuantum) || (quantum_type == CbYCrAQuantum))
    {
      Quantum
        quantum;

      register PixelPacket
        *restrict q;

      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=GetAuthenticPixelQueue(image);
      for (x=0; x < (long) number_pixels; x++)
      {
        quantum=q->red;
        q->red=q->green;
        q->green=quantum;
        q++;
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

          for (x=((long) number_pixels-7); x > 0; x-=8)
          {
            pixel=(unsigned char) *indexes++;
            *q=((pixel & 0x01) << 7);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 6);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 5);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 4);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 3);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 2);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 1);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 0);
            q++;
          }
          if ((number_pixels % 8) != 0)
            {
              *q='\0';
              for (bit=7; bit >= (long) (8-(number_pixels % 8)); bit--)
              {
                pixel=(unsigned char) *indexes++;
                *q|=((pixel & 0x01) << (unsigned char) bit);
              }
              q++;
            }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) (number_pixels-1) ; x+=2)
          {
            pixel=(unsigned char) *indexes++;
            *q=((pixel & 0xf) << 4);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0xf) << 0);
            q++;
          }
          if ((number_pixels % 2) != 0)
            {
              pixel=(unsigned char) *indexes++;
              *q=((pixel & 0xf) << 4);
              q++;
            }
          break;
        }
        case 8:
        {
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopCharPixel((unsigned char) indexes[x],q);
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopShortPixel(endian,SinglePrecisionToHalf(QuantumScale*
                  indexes[x]),q);
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopShortPixel(endian,(unsigned short) indexes[x],q);
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) indexes[x],q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopLongPixel(endian,(unsigned long) indexes[x],q);
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) indexes[x],q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,indexes[x],q);
            p++;
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

          for (x=((long) number_pixels-3); x > 0; x-=4)
          {
            pixel=(unsigned char) *indexes++;
            *q=((pixel & 0x01) << 7);
            pixel=(unsigned char) (p->opacity == (Quantum) TransparentOpacity ?
              1 : 0);
            *q|=((pixel & 0x01) << 6);
            p++;
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 5);
            pixel=(unsigned char) (p->opacity == (Quantum) TransparentOpacity ?
              1 : 0);
            *q|=((pixel & 0x01) << 4);
            p++;
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 3);
            pixel=(unsigned char) (p->opacity == (Quantum) TransparentOpacity ?
              1 : 0);
            *q|=((pixel & 0x01) << 2);
            p++;
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 1);
            pixel=(unsigned char) (p->opacity == (Quantum) TransparentOpacity ?
              1 : 0);
            *q|=((pixel & 0x01) << 0);
            p++;
            q++;
          }
          if ((number_pixels % 4) != 0)
            {
              *q='\0';
              for (bit=3; bit >= (long) (4-(number_pixels % 4)); bit-=2)
              {
                pixel=(unsigned char) *indexes++;
                *q|=((pixel & 0x01) << (unsigned char) (bit+4));
                pixel=(unsigned char) (p->opacity == (Quantum)
                  TransparentOpacity ? 1 : 0);
                *q|=((pixel & 0x01) << (unsigned char) (bit+4-1));
                p++;
              }
              q++;
            }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels ; x++)
          {
            pixel=(unsigned char) *indexes++;
            *q=((pixel & 0xf) << 4);
            pixel=(unsigned char) (16*QuantumScale*((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)))+0.5);
            *q|=((pixel & 0xf) << 0);
            p++;
            q++;
          }
          break;
        }
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopCharPixel((unsigned char) indexes[x],q);
            pixel=ScaleQuantumToChar((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)));
            q=PopCharPixel(pixel,q);
            p++;
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
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopShortPixel(endian,(unsigned short) indexes[x],q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetAlphaPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopShortPixel(endian,(unsigned short) indexes[x],q);
            pixel=ScaleQuantumToShort((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                float
                  pixel;

                q=PopFloatPixel(&quantum_state,(float) indexes[x],q);
                pixel=(float)  (GetAlphaPixelComponent(p));
                q=PopFloatPixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopLongPixel(endian,(unsigned long) indexes[x],q);
            pixel=ScaleQuantumToLong((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                double
                  pixel;

                q=PopDoublePixel(&quantum_state,(double) indexes[x],q);
                pixel=(double) (GetAlphaPixelComponent(p));
                q=PopDoublePixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,indexes[x],q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              (Quantum) (GetAlphaPixelComponent(p)),range),q);
            p++;
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
          for (x=((long) number_pixels-7); x > 0; x-=8)
          {
            *q='\0';
            *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) << 7;
            p++;
            *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) << 6;
            p++;
            *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) << 5;
            p++;
            *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) << 4;
            p++;
            *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) << 3;
            p++;
            *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) << 2;
            p++;
            *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) << 1;
            p++;
            *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) << 0;
            p++;
            q++;
          }
          if ((number_pixels % 8) != 0)
            {
              *q='\0';
              for (bit=7; bit >= (long) (8-(number_pixels % 8)); bit--)
              {
                *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) <<
                  bit;
                p++;
              }
              q++;
            }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) (number_pixels-1) ; x+=2)
          {
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            *q=(((pixel >> 4) & 0xf) << 4);
            p++;
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            *q|=pixel >> 4;
            p++;
            q++;
          }
          if ((number_pixels % 2) != 0)
            {
              pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
              *q=(((pixel >> 4) & 0xf) << 4);
              p++;
              q++;
            }
          break;
        }
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 10:
        {
          range=GetQuantumRange(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              register unsigned long
                pixel;

              for (x=0; x < (long) (number_pixels-2); x+=3)
              {
                pixel=(unsigned long) (
                  ScaleQuantumToAny(PixelIntensityToQuantum(p+2),range) << 22 |
                  ScaleQuantumToAny(PixelIntensityToQuantum(p+1),range) << 12 |
                  ScaleQuantumToAny(PixelIntensityToQuantum(p+0),range) << 2);
                q=PopLongPixel(endian,pixel,q);
                p+=3;
                q+=quantum_info->pad;
              }
              pixel=0UL;
              if (x++ < (long) (number_pixels-1))
                pixel|=ScaleQuantumToAny(PixelIntensityToQuantum(p+1),
                  range) << 12;
              if (x++ < (long) number_pixels)
                pixel|=ScaleQuantumToAny(PixelIntensityToQuantum(p+0),
                  range) << 2;
              q=PopLongPixel(endian,pixel,q);
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              PixelIntensityToQuantum(p),range),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 12:
        {
          register unsigned short
            pixel;

          range=GetQuantumRange(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=ScaleQuantumToShort(PixelIntensityToQuantum(p));
                q=PopShortPixel(endian,(unsigned short) (pixel >> 4),q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              PixelIntensityToQuantum(p),range),q);
            p++;
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
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  PixelIntensityToQuantum(p));
                q=PopShortPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(PixelIntensityToQuantum(p));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                float
                  pixel;

                pixel=(float) PixelIntensityToQuantum(p);
                q=PopFloatPixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(PixelIntensityToQuantum(p));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                double
                  pixel;

                pixel=(double) PixelIntensityToQuantum(p);
                q=PopDoublePixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              PixelIntensityToQuantum(p),range),q);
            p++;
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
          for (x=((long) number_pixels-3); x > 0; x-=4)
          {
            *q='\0';
            *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) << 7;
            pixel=(unsigned char) (p->opacity == OpaqueOpacity ? 0x00 : 0x01);
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 6);
            p++;
            *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) << 5;
            pixel=(unsigned char) (p->opacity == OpaqueOpacity ? 0x00 : 0x01);
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 4);
            p++;
            *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) << 3;
            pixel=(unsigned char) (p->opacity == OpaqueOpacity ? 0x00 : 0x01);
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 2);
            p++;
            *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) << 1;
            pixel=(unsigned char) (p->opacity == OpaqueOpacity ? 0x00 : 0x01);
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 0);
            p++;
            q++;
          }
          if ((number_pixels % 4) != 0)
            {
              *q='\0';
              for (bit=3; bit >= (long) (4-(number_pixels % 4)); bit-=2)
              {
                *q|=(PixelIntensityToQuantum(p) < threshold ? black : white) <<
                  (bit+4);
                pixel=(unsigned char) (p->opacity == OpaqueOpacity ? 0x00 :
                  0x01);
                *q|=(((int) pixel != 0 ? 0x00 : 0x01) << (unsigned char)
                  (bit+4-1));
                p++;
              }
              q++;
            }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels ; x++)
          {
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            *q=(((pixel >> 4) & 0xf) << 4);
            pixel=(unsigned char) (16*QuantumScale*((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)))+0.5);
            *q|=pixel & 0xf;
            p++;
            q++;
          }
          break;
        }
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)));
            q=PopCharPixel(pixel,q);
            p++;
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
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  PixelIntensityToQuantum(p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetAlphaPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(PixelIntensityToQuantum(p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                float
                  pixel;

                pixel=(float) PixelIntensityToQuantum(p);
                q=PopFloatPixel(&quantum_state,pixel,q);
                pixel=(float) (GetAlphaPixelComponent(p));
                q=PopFloatPixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(PixelIntensityToQuantum(p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                double
                  pixel;

                pixel=(double) PixelIntensityToQuantum(p);
                q=PopDoublePixel(&quantum_state,pixel,q);
                pixel=(double) (GetAlphaPixelComponent(p));
                q=PopDoublePixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              PixelIntensityToQuantum(p),range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              (Quantum) (GetAlphaPixelComponent(p)),range),q);
            p++;
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

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetRedPixelComponent(p));
            q=PopCharPixel(pixel,q);
            p++;
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
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetRedPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetRedPixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) p->red,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetRedPixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->red,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->red,range),q);
            p++;
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

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetGreenPixelComponent(p));
            q=PopCharPixel(pixel,q);
            p++;
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
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetGreenPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetGreenPixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) p->green,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetGreenPixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->green,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->green,range),q);
            p++;
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

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetBluePixelComponent(p));
            q=PopCharPixel(pixel,q);
            p++;
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
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetBluePixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetBluePixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) p->blue,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetBluePixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->blue,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->blue,range),q);
            p++;
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

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)));
            q=PopCharPixel(pixel,q);
            p++;
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
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetAlphaPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                float
                  pixel;

                pixel=(float) (GetAlphaPixelComponent(p));
                q=PopFloatPixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                double
                  pixel;

                pixel=(double) (GetAlphaPixelComponent(p));
                q=PopDoublePixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              (Quantum) (GetAlphaPixelComponent(p)),range),q);
            p++;
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

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetOpacityPixelComponent(p));
            q=PopCharPixel(pixel,q);
            p++;
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
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetOpacityPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetOpacityPixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) p->opacity,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetOpacityPixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->opacity,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->opacity,range),q);
            p++;
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

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(indexes[x]);
            q=PopCharPixel(pixel,q);
            p++;
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
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*indexes[x]);
                q=PopShortPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(indexes[x]);
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) indexes[x],q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(indexes[x]);
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) indexes[x],q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              (Quantum) indexes[x],range),q);
            p++;
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
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopCharPixel(ScaleQuantumToChar(GetRedPixelComponent(p)),q);
            q=PopCharPixel(ScaleQuantumToChar(GetGreenPixelComponent(p)),q);
            q=PopCharPixel(ScaleQuantumToChar(GetBluePixelComponent(p)),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 10:
        {
          register unsigned long
            pixel;

          range=GetQuantumRange(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=(unsigned long) (ScaleQuantumToAny(p->red,range) << 22 |
                  ScaleQuantumToAny(p->green,range) << 12 |
                  ScaleQuantumToAny(p->blue,range) << 2);
                q=PopLongPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          if (quantum_info->quantum == 32UL)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=(unsigned long) ScaleQuantumToAny(p->red,range);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                pixel=(unsigned long) ScaleQuantumToAny(p->green,range);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                pixel=(unsigned long) ScaleQuantumToAny(p->blue,range);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=(unsigned long) ScaleQuantumToAny(p->red,range);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            pixel=(unsigned long) ScaleQuantumToAny(p->green,range);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            pixel=(unsigned long) ScaleQuantumToAny(p->blue,range);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 12:
        {
          register unsigned long
            pixel;

          range=GetQuantumRange(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (long) (3*number_pixels-1); x+=2)
              {
                switch (x % 3)
                {
                  default:
                  case 0:
                  {
                    pixel=(unsigned long) ScaleQuantumToAny(p->red,range);
                    break;
                  }
                  case 1:
                  {
                    pixel=(unsigned long) ScaleQuantumToAny(p->green,range);
                    break;
                  }
                  case 2:
                  {
                    pixel=(unsigned long) ScaleQuantumToAny(p->blue,range);
                    p++;
                    break;
                  }
                }
                q=PopShortPixel(endian,(unsigned short) (pixel << 4),q);
                switch ((x+1) % 3)
                {
                  default:
                  case 0:
                  {
                    pixel=(unsigned long) ScaleQuantumToAny(p->red,range);
                    break;
                  }
                  case 1:
                  {
                    pixel=(unsigned long) ScaleQuantumToAny(p->green,range);
                    break;
                  }
                  case 2:
                  {
                    pixel=(unsigned long) ScaleQuantumToAny(p->blue,range);
                    p++;
                    break;
                  }
                }
                q=PopShortPixel(endian,(unsigned short) (pixel << 4),q);
                q+=quantum_info->pad;
              }
              for (bit=0; bit < (long) (3*number_pixels % 2); bit++)
              {
                switch ((x+bit) % 3)
                {
                  default:
                  case 0:
                  {
                    pixel=(unsigned long) ScaleQuantumToAny(p->red,range);
                    break;
                  }
                  case 1:
                  {
                    pixel=(unsigned long) ScaleQuantumToAny(p->green,range);
                    break;
                  }
                  case 2:
                  {
                    pixel=(unsigned long) ScaleQuantumToAny(p->blue,range);
                    p++;
                    break;
                  }
                }
                q=PopShortPixel(endian,(unsigned short) (pixel << 4),q);
                q+=quantum_info->pad;
              }
              if (bit != 0)
                p++;
              break;
            }
          if (quantum_info->quantum == 32UL)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=(unsigned long) ScaleQuantumToAny(p->red,range);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                pixel=(unsigned long) ScaleQuantumToAny(p->green,range);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                pixel=(unsigned long) ScaleQuantumToAny(p->blue,range);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=(unsigned long) ScaleQuantumToAny(p->red,range);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            pixel=(unsigned long) ScaleQuantumToAny(p->green,range);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            pixel=(unsigned long) ScaleQuantumToAny(p->blue,range);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            p++;
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
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetRedPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetGreenPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetBluePixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetRedPixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetGreenPixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetBluePixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) p->red,q);
                q=PopFloatPixel(&quantum_state,(float) p->green,q);
                q=PopFloatPixel(&quantum_state,(float) p->blue,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetRedPixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetGreenPixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetBluePixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->red,q);
                q=PopDoublePixel(&quantum_state,(double) p->green,q);
                q=PopDoublePixel(&quantum_state,(double) p->blue,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->red,range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->green,range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->blue,range),q);
            p++;
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

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetRedPixelComponent(p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetGreenPixelComponent(p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetBluePixelComponent(p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetAlphaPixelComponent(p));
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 10:
        {
          register unsigned long
            pixel;

          range=GetQuantumRange(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              long
                n;

              register long
                i;

              unsigned long
                quantum;

              n=0;
              quantum=0;
              pixel=0;
              for (x=0; x < (long) number_pixels; x++)
              {
                for (i=0; i < 4; i++)
                {
                  switch (i)
                  {
                    case 0: quantum=p->red; break;
                    case 1: quantum=p->green; break;
                    case 2: quantum=p->blue; break;
                    case 3: quantum=QuantumRange-p->opacity; break;
                  }
                  switch (n % 3)
                  {
                    case 0:
                    {
                      pixel|=(unsigned long) (ScaleQuantumToAny(quantum,
                        range) << 22);
                      break;
                    }
                    case 1:
                    {
                      pixel|=(unsigned long) (ScaleQuantumToAny(quantum,
                        range) << 12);
                      break;
                    }
                    case 2:
                    {
                      pixel|=(unsigned long) (ScaleQuantumToAny(quantum,
                        range) << 2);
                      q=PopLongPixel(endian,pixel,q);
                      pixel=0;
                      break;
                    }
                  }
                  n++;
                }
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          if (quantum_info->quantum == 32UL)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=(unsigned long) ScaleQuantumToAny(p->red,range);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                pixel=(unsigned long) ScaleQuantumToAny(p->green,range);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                pixel=(unsigned long) ScaleQuantumToAny(p->blue,range);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                pixel=(unsigned long) ScaleQuantumToAny(QuantumRange-p->opacity,
                  range);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=(unsigned long) ScaleQuantumToAny(p->red,range);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            pixel=(unsigned long) ScaleQuantumToAny(p->green,range);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            pixel=(unsigned long) ScaleQuantumToAny(p->blue,range);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            pixel=(unsigned long) ScaleQuantumToAny(QuantumRange-
              p->opacity,range);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            p++;
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
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetRedPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetGreenPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetBluePixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetAlphaPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetRedPixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetGreenPixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetBluePixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetAlphaPixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                float
                  pixel;

                q=PopFloatPixel(&quantum_state,(float) p->red,q);
                q=PopFloatPixel(&quantum_state,(float) p->green,q);
                q=PopFloatPixel(&quantum_state,(float) p->blue,q);
                pixel=GetAlphaPixelComponent(p);
                q=PopFloatPixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetRedPixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetGreenPixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetBluePixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetAlphaPixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            p++;
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

              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->red,q);
                q=PopDoublePixel(&quantum_state,(double) p->green,q);
                q=PopDoublePixel(&quantum_state,(double) p->blue,q);
                pixel=(double) GetAlphaPixelComponent(p);
                q=PopDoublePixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              GetRedPixelComponent(p),range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              GetGreenPixelComponent(p),range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              GetBluePixelComponent(p),range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              GetAlphaPixelComponent(p),range),q);
            p++;
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

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetRedPixelComponent(p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetGreenPixelComponent(p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetBluePixelComponent(p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(indexes[x]);
            q=PopCharPixel(pixel,q);
            p++;
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
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetRedPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetGreenPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetBluePixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*indexes[x]);
                q=PopShortPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetRedPixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetGreenPixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetBluePixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(indexes[x]);
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) p->red,q);
                q=PopFloatPixel(&quantum_state,(float) p->green,q);
                q=PopFloatPixel(&quantum_state,(float) p->blue,q);
                q=PopFloatPixel(&quantum_state,(float) indexes[x],q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetRedPixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetGreenPixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetBluePixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(indexes[x]);
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->red,q);
                q=PopDoublePixel(&quantum_state,(double) p->green,q);
                q=PopDoublePixel(&quantum_state,(double) p->blue,q);
                q=PopDoublePixel(&quantum_state,(double) indexes[x],q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->red,range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->green,range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->blue,range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              indexes[x],range),q);
            p++;
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

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(GetRedPixelComponent(p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetGreenPixelComponent(p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(GetBluePixelComponent(p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(indexes[x]);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)));
            q=PopCharPixel(pixel,q);
            p++;
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
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetRedPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetGreenPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetBluePixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*indexes[x]);
                q=PopShortPixel(endian,pixel,q);
                pixel=SinglePrecisionToHalf(QuantumScale*
                  GetAlphaPixelComponent(p));
                q=PopShortPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(GetRedPixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetGreenPixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(GetBluePixelComponent(p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(indexes[x]);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                float
                  pixel;

                q=PopFloatPixel(&quantum_state,(float) p->red,q);
                q=PopFloatPixel(&quantum_state,(float) p->green,q);
                q=PopFloatPixel(&quantum_state,(float) p->blue,q);
                q=PopFloatPixel(&quantum_state,(float) indexes[x],q);
                pixel=(float) (GetAlphaPixelComponent(p));
                q=PopFloatPixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(GetRedPixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetGreenPixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(GetBluePixelComponent(p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(indexes[x]);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong((Quantum) (QuantumRange-
              GetOpacityPixelComponent(p)));
            q=PopLongPixel(endian,pixel,q);
            p++;
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

              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->red,q);
                q=PopDoublePixel(&quantum_state,(double) p->green,q);
                q=PopDoublePixel(&quantum_state,(double) p->blue,q);
                q=PopDoublePixel(&quantum_state,(double) indexes[x],q);
                pixel=(double) (GetAlphaPixelComponent(p));
                q=PopDoublePixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          range=GetQuantumRange(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->red,range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->green,range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->blue,range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              indexes[x],range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->opacity,range),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case CbYCrYQuantum:
    {
     long
        n;

      Quantum
        cbcr[4];

      register long
        i;

      register unsigned long
        pixel;

      unsigned long
        quantum;

      n=0;
      quantum=0;
      range=GetQuantumRange(image->depth);
      switch (quantum_info->depth)
      {
        case 10:
        {
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (long) number_pixels; x+=2)
              {
                for (i=0; i < 4; i++)
                {
                  switch (n % 3)
                  {
                    case 0:
                    {
                      quantum=GetRedPixelComponent(p);
                      break;
                    }
                    case 1:
                    {
                      quantum=GetGreenPixelComponent(p);
                      break;
                    }
                    case 2:
                    {
                      quantum=GetBluePixelComponent(p);
                      break;
                    }
                  }
                  cbcr[i]=(Quantum) quantum;
                  n++;
                }
                pixel=(unsigned long) ((unsigned long) (cbcr[1]) << 22 |
                  (unsigned long) (cbcr[0]) << 12 |
                  (unsigned long) (cbcr[2]) << 2);
                q=PopLongPixel(endian,pixel,q);
                p++;
                pixel=(unsigned long) ((unsigned long) (cbcr[3]) << 22 |
                  (unsigned long) (cbcr[0]) << 12 |
                  (unsigned long) (cbcr[2]) << 2);
                q=PopLongPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          break;
        }
        default:
        {
          for (x=0; x < (long) number_pixels; x+=2)
          {
            for (i=0; i < 4; i++)
            {
              switch (n % 3)
              {
                case 0:
                {
                  quantum=GetRedPixelComponent(p);
                  break;
                }
                case 1:
                {
                  quantum=GetGreenPixelComponent(p);
                  break;
                }
                case 2:
                {
                  quantum=GetBluePixelComponent(p);
                  break;
                }
              }
              cbcr[i]=(Quantum) quantum;
              n++;
            }
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              cbcr[1],range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              cbcr[0],range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              cbcr[2],range),q);
            p++;
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              cbcr[3],range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              cbcr[0],range),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              cbcr[2],range),q);
            p++;
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

      register PixelPacket
        *restrict q;

      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=(PixelPacket *) GetCacheViewVirtualPixelQueue(image_view);
      for (x=0; x < (long) number_pixels; x++)
      {
        quantum=q->red;
        q->red=q->green;
        q->green=quantum;
        q++;
      }
    }
  if ((quantum_type == RGBOQuantum) || (quantum_type == CMYKOQuantum))
    {
      register PixelPacket
        *restrict q;

      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=(PixelPacket *) GetCacheViewVirtualPixelQueue(image_view);
      for (x=0; x < (long) number_pixels; x++)
      {
        q->opacity=(Quantum) GetAlphaPixelComponent(q);
        q++;
      }
    }
  return(extent);
}

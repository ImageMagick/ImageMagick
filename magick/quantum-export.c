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

static inline unsigned char *PopDoublePixel(QuantumInfo *quantum_info,
  const double pixel,unsigned char *pixels)
{
  double
    *p;

  unsigned char
    quantum[8];

  p=(double *) quantum;
  *p=(double) (pixel*quantum_info->state.inverse_scale+quantum_info->minimum);
  if (quantum_info->endian == LSBEndian)
    {
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

static inline unsigned char *PopFloatPixel(QuantumInfo *quantum_info,
  const float pixel,unsigned char *pixels)
{
  float
    *p;

  unsigned char
    quantum[4];

  p=(float *) quantum;
  *p=(float) ((double) pixel*quantum_info->state.inverse_scale+
    quantum_info->minimum);
  if (quantum_info->endian == LSBEndian)
    {
      *pixels++=quantum[0];
      *pixels++=quantum[1];
      *pixels++=quantum[2];
      *pixels++=quantum[3];
      return(pixels);
    }
  *pixels++=quantum[3];
  *pixels++=quantum[2];
  *pixels++=quantum[1];
  *pixels++=quantum[0];
  return(pixels);
}

static inline unsigned char *PopQuantumPixel(QuantumInfo *quantum_info,
  const QuantumAny pixel,unsigned char *pixels)
{
  register ssize_t
    i;

  size_t
    quantum_bits;

  if (quantum_info->state.bits == 0UL)
    quantum_info->state.bits=8U;
  for (i=(ssize_t) quantum_info->depth; i > 0L; )
  {
    quantum_bits=(size_t) i;
    if (quantum_bits > quantum_info->state.bits)
      quantum_bits=quantum_info->state.bits;
    i-=(ssize_t) quantum_bits;
    if (quantum_info->state.bits == 8UL)
      *pixels='\0';
    quantum_info->state.bits-=quantum_bits;
    *pixels|=(((pixel >> i) &~ ((~0UL) << quantum_bits)) <<
      quantum_info->state.bits);
    if (quantum_info->state.bits == 0UL)
      {
        pixels++;
        quantum_info->state.bits=8UL;
      }
  }
  return(pixels);
}

static inline unsigned char *PopQuantumLongPixel(QuantumInfo *quantum_info,
  const size_t pixel,unsigned char *pixels)
{
  register ssize_t
    i;

  size_t
    quantum_bits;

  if (quantum_info->state.bits == 0U)
    quantum_info->state.bits=32UL;
  for (i=(ssize_t) quantum_info->depth; i > 0; )
  {
    quantum_bits=(size_t) i;
    if (quantum_bits > quantum_info->state.bits)
      quantum_bits=quantum_info->state.bits;
    quantum_info->state.pixel|=(((pixel >> (quantum_info->depth-i)) &
      quantum_info->state.mask[quantum_bits]) <<
      (32U-quantum_info->state.bits));
    i-=(ssize_t) quantum_bits;
    quantum_info->state.bits-=quantum_bits;
    if (quantum_info->state.bits == 0U)
      {
        pixels=PopLongPixel(quantum_info->endian,quantum_info->state.pixel,
          pixels);
        quantum_info->state.pixel=0U;
        quantum_info->state.bits=32U;
      }
  }
  return(pixels);
}

static void ExportAlphaQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  unsigned char *restrict q,ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  switch (quantum_info->depth)
  {
    case 8:
    {
      register unsigned char
        pixel;

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToChar((Quantum) (QuantumRange-GetPixelOpacity(p)));
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelAlpha(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort((Quantum) (QuantumRange-GetPixelOpacity(p)));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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

            pixel=(float) (GetPixelAlpha(p));
            q=PopFloatPixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong((Quantum) (QuantumRange-GetPixelOpacity(p)));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        p++;
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

            pixel=(double) (GetPixelAlpha(p));
            q=PopDoublePixel(quantum_info,pixel,q);
            p++;
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
        q=PopQuantumPixel(quantum_info,ScaleQuantumToAny((Quantum)
          (GetPixelAlpha(p)),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportBGRQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  unsigned char *restrict q,ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  ssize_t
    bit;

  switch (quantum_info->depth)
  {
    case 8:
    {
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q=PopCharPixel(ScaleQuantumToChar(GetPixelBlue(p)),q);
        q=PopCharPixel(ScaleQuantumToChar(GetPixelGreen(p)),q);
        q=PopCharPixel(ScaleQuantumToChar(GetPixelRed(p)),q);
        p++;
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
              ScaleQuantumToAny(GetPixelRed(p),range) << 22 |
              ScaleQuantumToAny(GetPixelGreen(p),range) << 12 |
              ScaleQuantumToAny(GetPixelBlue(p),range) << 2);
            q=PopLongPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      if (quantum_info->quantum == 32UL)
        {
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        p++;
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
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
                break;
              }
              case 1:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
                break;
              }
              case 2:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
                p++;
                break;
              }
            }
            q=PopShortPixel(quantum_info->endian,(unsigned short) (pixel << 4),q);
            switch ((x+1) % 3)
            {
              default:
              case 0:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
                break;
              }
              case 1:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
                break;
              }
              case 2:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
                p++;
                break;
              }
            }
            q=PopShortPixel(quantum_info->endian,(unsigned short) (pixel << 4),q);
            q+=quantum_info->pad;
          }
          for (bit=0; bit < (ssize_t) (3*number_pixels % 2); bit++)
          {
            switch ((x+bit) % 3)
            {
              default:
              case 0:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
                break;
              }
              case 1:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
                break;
              }
              case 2:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
                p++;
                break;
              }
            }
            q=PopShortPixel(quantum_info->endian,(unsigned short) (pixel << 4),q);
            q+=quantum_info->pad;
          }
          if (bit != 0)
            p++;
          break;
        }
      if (quantum_info->quantum == 32UL)
        {
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelBlue(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelGreen(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelRed(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort(GetPixelBlue(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelGreen(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelRed(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopFloatPixel(quantum_info,(float) GetPixelRed(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelGreen(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelBlue(p),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong(GetPixelBlue(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelGreen(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelRed(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopDoublePixel(quantum_info,(double) GetPixelRed(p),q);
            q=PopDoublePixel(quantum_info,(double) GetPixelGreen(p),q);
            q=PopDoublePixel(quantum_info,(double) GetPixelBlue(p),q);
            p++;
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
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelRed(p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelGreen(p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelBlue(p),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportBGRAQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  unsigned char *restrict q,ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  switch (quantum_info->depth)
  {
    case 8:
    {
      register unsigned char
        pixel;

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToChar(GetPixelBlue(p));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar(GetPixelGreen(p));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar(GetPixelRed(p));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar((Quantum) GetPixelAlpha(p));
        q=PopCharPixel(pixel,q);
        p++;
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
                case 0: quantum=GetPixelRed(p); break;
                case 1: quantum=GetPixelGreen(p); break;
                case 2: quantum=GetPixelBlue(p); break;
                case 3: quantum=(Quantum) (QuantumRange-GetPixelOpacity(p)); break;
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
                  q=PopLongPixel(quantum_info->endian,pixel,q);
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny((Quantum) (QuantumRange-
              GetPixelOpacity(p)),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny((Quantum) (QuantumRange-
          GetPixelOpacity(p)),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelBlue(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelGreen(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelRed(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelAlpha(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort(GetPixelBlue(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelGreen(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelRed(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort((Quantum) GetPixelAlpha(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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

            q=PopFloatPixel(quantum_info,(float) GetPixelRed(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelGreen(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelBlue(p),q);
            pixel=(float) GetPixelAlpha(p);
            q=PopFloatPixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong(GetPixelBlue(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelGreen(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelRed(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong((Quantum) GetPixelAlpha(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
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

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopDoublePixel(quantum_info,(double) GetPixelRed(p),q);
            q=PopDoublePixel(quantum_info,(double) GetPixelGreen(p),q);
            q=PopDoublePixel(quantum_info,(double) GetPixelBlue(p),q);
            pixel=(double) GetPixelAlpha(p);
            q=PopDoublePixel(quantum_info,pixel,q);
            p++;
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
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelBlue(p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelGreen(p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelRed(p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny((Quantum) GetPixelAlpha(p),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportBlackQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  const IndexPacket *restrict indexes,unsigned char *restrict q,
  ExceptionInfo *exception)
{
  register ssize_t
    x;

  if (image->colorspace != CMYKColorspace)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "ColorSeparatedImageRequired","`%s'",image->filename);
      return;
    }
  switch (quantum_info->depth)
  {
    case 8:
    {
      register unsigned char
        pixel;

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToChar(GetPixelIndex(indexes+x));
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelIndex(indexes+x));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort(GetPixelIndex(indexes+x));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopFloatPixel(quantum_info,(float) GetPixelIndex(indexes+x),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong(GetPixelIndex(indexes+x));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopDoublePixel(quantum_info,(double) GetPixelIndex(indexes+x),
              q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
    }
    default:
    {
      QuantumAny
        range;

      range=GetQuantumRange(quantum_info->depth);
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny((Quantum) GetPixelIndex(indexes+x),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportBlueQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  unsigned char *restrict q,ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  switch (quantum_info->depth)
  {
    case 8:
    {
      register unsigned char
        pixel;

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToChar(GetPixelBlue(p));
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelBlue(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort(GetPixelBlue(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopFloatPixel(quantum_info,(float) GetPixelBlue(p),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong(GetPixelBlue(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopDoublePixel(quantum_info,(double) GetPixelBlue(p),q);
            p++;
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
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelBlue(p),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportCbYCrYQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  unsigned char *restrict q,ExceptionInfo *exception)
{
  Quantum
    cbcr[4];

  QuantumAny
    range;

  register ssize_t
    i,
    x;

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
                  quantum=GetPixelRed(p);
                  break;
                }
                case 1:
                {
                  quantum=GetPixelGreen(p);
                  break;
                }
                case 2:
                {
                  quantum=GetPixelBlue(p);
                  break;
                }
              }
              cbcr[i]=(Quantum) quantum;
              n++;
            }
            pixel=(unsigned int) ((size_t) (cbcr[1]) << 22 | (size_t)
              (cbcr[0]) << 12 | (size_t) (cbcr[2]) << 2);
            q=PopLongPixel(quantum_info->endian,pixel,q);
            p++;
            pixel=(unsigned int) ((size_t) (cbcr[3]) << 22 | (size_t)
              (cbcr[0]) << 12 | (size_t) (cbcr[2]) << 2);
            q=PopLongPixel(quantum_info->endian,pixel,q);
            p++;
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
              quantum=GetPixelRed(p);
              break;
            }
            case 1:
            {
              quantum=GetPixelGreen(p);
              break;
            }
            case 2:
            {
              quantum=GetPixelBlue(p);
              break;
            }
          }
          cbcr[i]=(Quantum) quantum;
          n++;
        }
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(cbcr[1],range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(cbcr[0],range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(cbcr[2],range),q);
        p++;
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(cbcr[3],range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(cbcr[0],range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(cbcr[2],range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportCMYKQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  const IndexPacket *restrict indexes,unsigned char *restrict q,
  ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  if (image->colorspace != CMYKColorspace)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "ColorSeparatedImageRequired","`%s'",image->filename);
      return;
    }
  switch (quantum_info->depth)
  {
    case 8:
    {
      register unsigned char
        pixel;

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToChar(GetPixelRed(p));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar(GetPixelGreen(p));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar(GetPixelBlue(p));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar(GetPixelIndex(indexes+x));
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelRed(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelGreen(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelBlue(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelIndex(indexes+x));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort(GetPixelRed(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelGreen(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelBlue(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelIndex(indexes+x));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopFloatPixel(quantum_info,(float) GetPixelRed(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelGreen(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelBlue(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelIndex(indexes+x),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong(GetPixelRed(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelGreen(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelBlue(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelIndex(indexes+x));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopDoublePixel(quantum_info,(double) GetPixelRed(p),q);
            q=PopDoublePixel(quantum_info,(double) GetPixelGreen(p),q);
            q=PopDoublePixel(quantum_info,(double) GetPixelBlue(p),q);
            q=PopDoublePixel(quantum_info,(double)
              GetPixelIndex(indexes+x),q);
            p++;
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
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelRed(p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelGreen(p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelBlue(p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelIndex(indexes+x),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportCMYKAQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  const IndexPacket *restrict indexes,unsigned char *restrict q,
  ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  if (image->colorspace != CMYKColorspace)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "ColorSeparatedImageRequired","`%s'",image->filename);
      return;
    }
  switch (quantum_info->depth)
  {
    case 8:
    {
      register unsigned char
        pixel;

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToChar(GetPixelRed(p));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar(GetPixelGreen(p));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar(GetPixelBlue(p));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar(GetPixelIndex(indexes+x));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar((Quantum) (QuantumRange-GetPixelOpacity(p)));
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelRed(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelGreen(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelBlue(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelIndex(indexes+x));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelAlpha(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort(GetPixelRed(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelGreen(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelBlue(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelIndex(indexes+x));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort((Quantum) (QuantumRange-GetPixelOpacity(p)));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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

            q=PopFloatPixel(quantum_info,(float) GetPixelRed(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelGreen(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelBlue(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelIndex(indexes+x),q);
            pixel=(float) (GetPixelAlpha(p));
            q=PopFloatPixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong(GetPixelRed(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelGreen(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelBlue(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelIndex(indexes+x));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong((Quantum) (QuantumRange-GetPixelOpacity(p)));
        q=PopLongPixel(quantum_info->endian,pixel,q);
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

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopDoublePixel(quantum_info,(double) GetPixelRed(p),q);
            q=PopDoublePixel(quantum_info,(double) GetPixelGreen(p),q);
            q=PopDoublePixel(quantum_info,(double) GetPixelBlue(p),q);
            q=PopDoublePixel(quantum_info,(double)
              GetPixelIndex(indexes+x),q);
            pixel=(double) (GetPixelAlpha(p));
            q=PopDoublePixel(quantum_info,pixel,q);
            p++;
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
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelRed(p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelGreen(p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelBlue(p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelIndex(indexes+x),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelOpacity(p),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportGrayQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  unsigned char *restrict q,ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  ssize_t
    bit;

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
        *q|=(PixelIntensityToQuantum(image,p) < threshold ? black : white) << 7;
        p++;
        *q|=(PixelIntensityToQuantum(image,p) < threshold ? black : white) << 6;
        p++;
        *q|=(PixelIntensityToQuantum(image,p) < threshold ? black : white) << 5;
        p++;
        *q|=(PixelIntensityToQuantum(image,p) < threshold ? black : white) << 4;
        p++;
        *q|=(PixelIntensityToQuantum(image,p) < threshold ? black : white) << 3;
        p++;
        *q|=(PixelIntensityToQuantum(image,p) < threshold ? black : white) << 2;
        p++;
        *q|=(PixelIntensityToQuantum(image,p) < threshold ? black : white) << 1;
        p++;
        *q|=(PixelIntensityToQuantum(image,p) < threshold ? black : white) << 0;
        p++;
        q++;
      }
      if ((number_pixels % 8) != 0)
        {
          *q='\0';
          for (bit=7; bit >= (ssize_t) (8-(number_pixels % 8)); bit--)
          {
            *q|=(PixelIntensityToQuantum(image,p) < threshold ? black : white) << bit;
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

      for (x=0; x < (ssize_t) (number_pixels-1) ; x+=2)
      {
        pixel=ScaleQuantumToChar(PixelIntensityToQuantum(image,p));
        *q=(((pixel >> 4) & 0xf) << 4);
        p++;
        pixel=ScaleQuantumToChar(PixelIntensityToQuantum(image,p));
        *q|=pixel >> 4;
        p++;
        q++;
      }
      if ((number_pixels % 2) != 0)
        {
          pixel=ScaleQuantumToChar(PixelIntensityToQuantum(image,p));
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

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToChar(PixelIntensityToQuantum(image,p));
        q=PopCharPixel(pixel,q);
        p++;
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
              ScaleQuantumToAny(PixelIntensityToQuantum(image,p+2),range) << 22 |
              ScaleQuantumToAny(PixelIntensityToQuantum(image,p+1),range) << 12 |
              ScaleQuantumToAny(PixelIntensityToQuantum(image,p+0),range) << 2);
            q=PopLongPixel(quantum_info->endian,pixel,q);
            p+=3;
            q+=quantum_info->pad;
          }
          if (x < (ssize_t) number_pixels)
            {
              pixel=0U;
              if (x++ < (ssize_t) (number_pixels-1))
                pixel|=ScaleQuantumToAny(PixelIntensityToQuantum(image,p+1),
                  range) << 12;
              if (x++ < (ssize_t) number_pixels)
                pixel|=ScaleQuantumToAny(PixelIntensityToQuantum(image,p+0),
                  range) << 2;
              q=PopLongPixel(quantum_info->endian,pixel,q);
            }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(PixelIntensityToQuantum(image,p),range),q);
        p++;
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
            pixel=ScaleQuantumToShort(PixelIntensityToQuantum(image,p));
            q=PopShortPixel(quantum_info->endian,(unsigned short) (pixel >> 4),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(PixelIntensityToQuantum(image,p),range),q);
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*
              PixelIntensityToQuantum(image,p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort(PixelIntensityToQuantum(image,p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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

            pixel=(float) PixelIntensityToQuantum(image,p);
            q=PopFloatPixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong(PixelIntensityToQuantum(image,p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        p++;
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

            pixel=(double) PixelIntensityToQuantum(image,p);
            q=PopDoublePixel(quantum_info,pixel,q);
            p++;
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
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(PixelIntensityToQuantum(image,p),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportGrayAlphaQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  unsigned char *restrict q,ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  ssize_t
    bit;

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
        *q|=(PixelIntensityToQuantum(image,p) > threshold ? black : white) << 7;
        pixel=(unsigned char) (GetPixelOpacity(p) == OpaqueOpacity ?
          0x00 : 0x01);
        *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 6);
        p++;
        *q|=(PixelIntensityToQuantum(image,p) > threshold ? black : white) << 5;
        pixel=(unsigned char) (GetPixelOpacity(p) == OpaqueOpacity ?
          0x00 : 0x01);
        *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 4);
        p++;
        *q|=(PixelIntensityToQuantum(image,p) > threshold ? black : white) << 3;
        pixel=(unsigned char) (GetPixelOpacity(p) == OpaqueOpacity ?
          0x00 : 0x01);
        *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 2);
        p++;
        *q|=(PixelIntensityToQuantum(image,p) > threshold ? black : white) << 1;
        pixel=(unsigned char) (GetPixelOpacity(p) == OpaqueOpacity ?
          0x00 : 0x01);
        *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 0);
        p++;
        q++;
      }
      if ((number_pixels % 4) != 0)
        {
          *q='\0';
          for (bit=0; bit <= (ssize_t) (number_pixels % 4); bit+=2)
          {
            *q|=(PixelIntensityToQuantum(image,p) > threshold ? black : white) <<
              (7-bit);
            pixel=(unsigned char) (GetPixelOpacity(p) == OpaqueOpacity ? 0x00 :
              0x01);
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << (unsigned char) (7-bit-1));
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

      for (x=0; x < (ssize_t) number_pixels ; x++)
      {
        pixel=ScaleQuantumToChar(PixelIntensityToQuantum(image,p));
        *q=(((pixel >> 4) & 0xf) << 4);
        pixel=(unsigned char) (16*QuantumScale*((Quantum) (QuantumRange-
          GetPixelOpacity(p)))+0.5);
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

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToChar(PixelIntensityToQuantum(image,p));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar((Quantum) (QuantumRange-GetPixelOpacity(p)));
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*
              PixelIntensityToQuantum(image,p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelAlpha(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort(PixelIntensityToQuantum(image,p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort((Quantum) (QuantumRange-GetPixelOpacity(p)));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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

            pixel=(float) PixelIntensityToQuantum(image,p);
            q=PopFloatPixel(quantum_info,pixel,q);
            pixel=(float) (GetPixelAlpha(p));
            q=PopFloatPixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong(PixelIntensityToQuantum(image,p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong((Quantum) (QuantumRange-GetPixelOpacity(p)));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        p++;
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

            pixel=(double) PixelIntensityToQuantum(image,p);
            q=PopDoublePixel(quantum_info,pixel,q);
            pixel=(double) (GetPixelAlpha(p));
            q=PopDoublePixel(quantum_info,pixel,q);
            p++;
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
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(PixelIntensityToQuantum(image,p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny((Quantum) (GetPixelAlpha(p)),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportGreenQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  unsigned char *restrict q,ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  switch (quantum_info->depth)
  {
    case 8:
    {
      register unsigned char
        pixel;

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToChar(GetPixelGreen(p));
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelGreen(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort(GetPixelGreen(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopFloatPixel(quantum_info,(float) GetPixelGreen(p),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong(GetPixelGreen(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopDoublePixel(quantum_info,(double) GetPixelGreen(p),q);
            p++;
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
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelGreen(p),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportIndexQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  const IndexPacket *restrict indexes,unsigned char *restrict q,
  ExceptionInfo *exception)
{
  register ssize_t
    x;

  ssize_t
    bit;

  if (image->storage_class != PseudoClass)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "ColormappedImageRequired","`%s'",image->filename);
      return;
    }
  switch (quantum_info->depth)
  {
    case 1:
    {
      register unsigned char
        pixel;

      for (x=((ssize_t) number_pixels-7); x > 0; x-=8)
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
          for (bit=7; bit >= (ssize_t) (8-(number_pixels % 8)); bit--)
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

      for (x=0; x < (ssize_t) (number_pixels-1) ; x+=2)
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
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q=PopCharPixel((unsigned char) GetPixelIndex(indexes+x),q);
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
            q=PopShortPixel(quantum_info->endian,SinglePrecisionToHalf(QuantumScale*
              GetPixelIndex(indexes+x)),q);
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q=PopShortPixel(quantum_info->endian,(unsigned short) GetPixelIndex(indexes+x),q);
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
            q=PopFloatPixel(quantum_info,(float) GetPixelIndex(indexes+x),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q=PopLongPixel(quantum_info->endian,(unsigned int) GetPixelIndex(indexes+x),q);
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
            q=PopDoublePixel(quantum_info,(double) GetPixelIndex(indexes+x),
              q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
    }
    default:
    {
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q=PopQuantumPixel(quantum_info,
          GetPixelIndex(indexes+x),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportIndexAlphaQuantum(const Image *image,
  QuantumInfo *quantum_info,const MagickSizeType number_pixels,
  const PixelPacket *restrict p,const IndexPacket *restrict indexes,
  unsigned char *restrict q,ExceptionInfo *exception)
{
  register ssize_t
    x;

  ssize_t
    bit;

  if (image->storage_class != PseudoClass)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "ColormappedImageRequired","`%s'",image->filename);
      return;
    }
  switch (quantum_info->depth)
  {
    case 1:
    {
      register unsigned char
        pixel;

      for (x=((ssize_t) number_pixels-3); x > 0; x-=4)
      {
        pixel=(unsigned char) *indexes++;
        *q=((pixel & 0x01) << 7);
        pixel=(unsigned char) (GetPixelOpacity(p) == (Quantum)
          TransparentOpacity ? 1 : 0);
        *q|=((pixel & 0x01) << 6);
        p++;
        pixel=(unsigned char) *indexes++;
        *q|=((pixel & 0x01) << 5);
        pixel=(unsigned char) (GetPixelOpacity(p) == (Quantum)
          TransparentOpacity ? 1 : 0);
        *q|=((pixel & 0x01) << 4);
        p++;
        pixel=(unsigned char) *indexes++;
        *q|=((pixel & 0x01) << 3);
        pixel=(unsigned char) (GetPixelOpacity(p) == (Quantum)
          TransparentOpacity ? 1 : 0);
        *q|=((pixel & 0x01) << 2);
        p++;
        pixel=(unsigned char) *indexes++;
        *q|=((pixel & 0x01) << 1);
        pixel=(unsigned char) (GetPixelOpacity(p) == (Quantum)
          TransparentOpacity ? 1 : 0);
        *q|=((pixel & 0x01) << 0);
        p++;
        q++;
      }
      if ((number_pixels % 4) != 0)
        {
          *q='\0';
          for (bit=3; bit >= (ssize_t) (4-(number_pixels % 4)); bit-=2)
          {
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << (unsigned char) (bit+4));
            pixel=(unsigned char) (GetPixelOpacity(p) == (Quantum)
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

      for (x=0; x < (ssize_t) number_pixels ; x++)
      {
        pixel=(unsigned char) *indexes++;
        *q=((pixel & 0xf) << 4);
        pixel=(unsigned char) (16*QuantumScale*((Quantum) (QuantumRange-
          GetPixelOpacity(p)))+0.5);
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

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q=PopCharPixel((unsigned char) GetPixelIndex(indexes+x),q);
        pixel=ScaleQuantumToChar((Quantum) (QuantumRange-GetPixelOpacity(p)));
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopShortPixel(quantum_info->endian,(unsigned short) GetPixelIndex(indexes+x),q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelAlpha(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q=PopShortPixel(quantum_info->endian,(unsigned short) GetPixelIndex(indexes+x),q);
        pixel=ScaleQuantumToShort((Quantum) (QuantumRange-GetPixelOpacity(p)));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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

            q=PopFloatPixel(quantum_info,(float) GetPixelIndex(indexes+x),q);
            pixel=(float)  (GetPixelAlpha(p));
            q=PopFloatPixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q=PopLongPixel(quantum_info->endian,(unsigned int) GetPixelIndex(indexes+x),q);
        pixel=ScaleQuantumToLong((Quantum) (QuantumRange-GetPixelOpacity(p)));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        p++;
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

            q=PopDoublePixel(quantum_info,(double) GetPixelIndex(indexes+x),
              q);
            pixel=(double) (GetPixelAlpha(p));
            q=PopDoublePixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
    }
    default:
    {
      QuantumAny
        range;

      range=GetQuantumRange(quantum_info->depth);
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q=PopQuantumPixel(quantum_info,
          GetPixelIndex(indexes+x),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny((Quantum) (GetPixelAlpha(p)),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportOpacityQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  unsigned char *restrict q,ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  switch (quantum_info->depth)
  {
    case 8:
    {
      register unsigned char
        pixel;

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToChar(GetPixelOpacity(p));
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelOpacity(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort(GetPixelOpacity(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopFloatPixel(quantum_info,(float) GetPixelOpacity(p),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong(GetPixelOpacity(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopDoublePixel(quantum_info,(double) GetPixelOpacity(p),q);
            p++;
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
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelOpacity(p),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportRedQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  unsigned char *restrict q,ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  switch (quantum_info->depth)
  {
    case 8:
    {
      register unsigned char
        pixel;

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToChar(GetPixelRed(p));
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelRed(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort(GetPixelRed(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopFloatPixel(quantum_info,(float) GetPixelRed(p),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong(GetPixelRed(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopDoublePixel(quantum_info,(double) GetPixelRed(p),q);
            p++;
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
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelRed(p),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportRGBQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  unsigned char *restrict q,ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  ssize_t
    bit;

  switch (quantum_info->depth)
  {
    case 8:
    {
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q=PopCharPixel(ScaleQuantumToChar(GetPixelRed(p)),q);
        q=PopCharPixel(ScaleQuantumToChar(GetPixelGreen(p)),q);
        q=PopCharPixel(ScaleQuantumToChar(GetPixelBlue(p)),q);
        p++;
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
              ScaleQuantumToAny(GetPixelRed(p),range) << 22 |
              ScaleQuantumToAny(GetPixelGreen(p),range) << 12 |
              ScaleQuantumToAny(GetPixelBlue(p),range) << 2);
            q=PopLongPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      if (quantum_info->quantum == 32UL)
        {
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        p++;
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
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
                break;
              }
              case 1:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
                break;
              }
              case 2:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
                p++;
                break;
              }
            }
            q=PopShortPixel(quantum_info->endian,(unsigned short) (pixel << 4),q);
            switch ((x+1) % 3)
            {
              default:
              case 0:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
                break;
              }
              case 1:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
                break;
              }
              case 2:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
                p++;
                break;
              }
            }
            q=PopShortPixel(quantum_info->endian,(unsigned short) (pixel << 4),q);
            q+=quantum_info->pad;
          }
          for (bit=0; bit < (ssize_t) (3*number_pixels % 2); bit++)
          {
            switch ((x+bit) % 3)
            {
              default:
              case 0:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
                break;
              }
              case 1:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
                break;
              }
              case 2:
              {
                pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
                p++;
                break;
              }
            }
            q=PopShortPixel(quantum_info->endian,(unsigned short) (pixel << 4),q);
            q+=quantum_info->pad;
          }
          if (bit != 0)
            p++;
          break;
        }
      if (quantum_info->quantum == 32UL)
        {
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelRed(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelGreen(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelBlue(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort(GetPixelRed(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelGreen(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelBlue(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopFloatPixel(quantum_info,(float) GetPixelRed(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelGreen(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelBlue(p),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong(GetPixelRed(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelGreen(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelBlue(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        p++;
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
            q=PopDoublePixel(quantum_info,(double) GetPixelRed(p),q);
            q=PopDoublePixel(quantum_info,(double) GetPixelGreen(p),q);
            q=PopDoublePixel(quantum_info,(double) GetPixelBlue(p),q);
            p++;
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
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelRed(p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelGreen(p),range),q);
        q=PopQuantumPixel(quantum_info,
          ScaleQuantumToAny(GetPixelBlue(p),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

static void ExportRGBAQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const PixelPacket *restrict p,
  unsigned char *restrict q,ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  switch (quantum_info->depth)
  {
    case 8:
    {
      register unsigned char
        pixel;

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToChar(GetPixelRed(p));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar(GetPixelGreen(p));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar(GetPixelBlue(p));
        q=PopCharPixel(pixel,q);
        pixel=ScaleQuantumToChar((Quantum) GetPixelAlpha(p));
        q=PopCharPixel(pixel,q);
        p++;
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
                case 0: quantum=GetPixelRed(p); break;
                case 1: quantum=GetPixelGreen(p); break;
                case 2: quantum=GetPixelBlue(p); break;
                case 3: quantum=(Quantum) (QuantumRange-GetPixelOpacity(p)); break;
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
                  q=PopLongPixel(quantum_info->endian,pixel,q);
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            pixel=(unsigned int) ScaleQuantumToAny((Quantum) (QuantumRange-
              GetPixelOpacity(p)),range);
            q=PopQuantumLongPixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelRed(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelGreen(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny(GetPixelBlue(p),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
        pixel=(unsigned int) ScaleQuantumToAny((Quantum) (QuantumRange-
          GetPixelOpacity(p)),range);
        q=PopQuantumPixel(quantum_info,pixel,q);
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
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelRed(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelGreen(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelBlue(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            pixel=SinglePrecisionToHalf(QuantumScale*GetPixelAlpha(p));
            q=PopShortPixel(quantum_info->endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToShort(GetPixelRed(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelGreen(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort(GetPixelBlue(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToShort((Quantum) GetPixelAlpha(p));
        q=PopShortPixel(quantum_info->endian,pixel,q);
        p++;
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

            q=PopFloatPixel(quantum_info,(float) GetPixelRed(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelGreen(p),q);
            q=PopFloatPixel(quantum_info,(float) GetPixelBlue(p),q);
            pixel=(float) GetPixelAlpha(p);
            q=PopFloatPixel(quantum_info,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        pixel=ScaleQuantumToLong(GetPixelRed(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelGreen(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong(GetPixelBlue(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
        pixel=ScaleQuantumToLong((Quantum) GetPixelAlpha(p));
        q=PopLongPixel(quantum_info->endian,pixel,q);
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

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            q=PopDoublePixel(quantum_info,(double) GetPixelRed(p),q);
            q=PopDoublePixel(quantum_info,(double) GetPixelGreen(p),q);
            q=PopDoublePixel(quantum_info,(double) GetPixelBlue(p),q);
            pixel=(double) GetPixelAlpha(p);
            q=PopDoublePixel(quantum_info,pixel,q);
            p++;
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
        q=PopQuantumPixel(quantum_info,ScaleQuantumToAny(GetPixelRed(p),
          range),q);
        q=PopQuantumPixel(quantum_info,ScaleQuantumToAny(GetPixelGreen(p),
          range),q);
        q=PopQuantumPixel(quantum_info,ScaleQuantumToAny(GetPixelBlue(p),
          range),q);
        q=PopQuantumPixel(quantum_info,ScaleQuantumToAny((Quantum)
          GetPixelAlpha(p),range),q);
        p++;
        q+=quantum_info->pad;
      }
      break;
    }
  }
}

MagickExport size_t ExportQuantumPixels(const Image *image,
  const CacheView *image_view,const QuantumInfo *quantum_info,
  const QuantumType quantum_type,unsigned char *pixels,ExceptionInfo *exception)
{
  MagickRealType
    alpha;

  MagickSizeType
    number_pixels;

  register const IndexPacket
    *restrict indexes;

  register const PixelPacket
    *restrict p;

  register ssize_t
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
  if (image_view == (CacheView *) NULL)
    {
      number_pixels=GetImageExtent(image);
      p=GetVirtualPixelQueue(image);
      indexes=GetVirtualIndexQueue(image);
    }
  else
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
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        alpha=QuantumScale*GetPixelAlpha(q);
        SetPixelRed(q,ClampToQuantum(alpha*GetPixelRed(q)));
        SetPixelGreen(q,ClampToQuantum(alpha*GetPixelGreen(q)));
        SetPixelBlue(q,ClampToQuantum(alpha*GetPixelBlue(q)));
        q++;
      }
    }
  if ((quantum_type == RGBOQuantum) || (quantum_type == CMYKOQuantum) ||
      (quantum_type == BGROQuantum))
    {
      register PixelPacket
        *restrict q;

      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=(PixelPacket *) GetCacheViewVirtualPixelQueue(image_view);
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        q->opacity=(Quantum) GetPixelAlpha(q);
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
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        quantum=GetPixelRed(q);
        SetPixelRed(q,GetPixelGreen(q));
        SetPixelGreen(q,quantum);
        q++;
      }
    }
  x=0;
  q=pixels;
  ResetQuantumState((QuantumInfo *) quantum_info);
  extent=GetQuantumExtent(image,quantum_info,quantum_type);
  switch (quantum_type)
  {
    case AlphaQuantum:
    {
      ExportAlphaQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,q,
        exception);
      break;
    }
    case BGRQuantum:
    {
      ExportBGRQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,q,
        exception);
      break;
    }
    case BGRAQuantum:
    case BGROQuantum:
    {
      ExportBGRAQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,q,
        exception);
      break;
    }
    case BlackQuantum:
    {
      ExportBlackQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,
        indexes,q,exception);
      break;
    }
    case BlueQuantum:
    case YellowQuantum:
    {
      ExportBlueQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,q,
        exception);
      break;
    }
    case CbYCrYQuantum:
    {
      ExportCbYCrYQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,q,
        exception);
      break;
    }
    case CMYKQuantum:
    {
      ExportCMYKQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,
        indexes,q,exception);
      break;
    }
    case CMYKAQuantum:
    case CMYKOQuantum:
    {
      ExportCMYKAQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,
        indexes,q,exception);
      break;
    }
    case GrayQuantum:
    {
      ExportGrayQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,q,
        exception);
      break;
    }
    case GrayAlphaQuantum:
    {
      ExportGrayAlphaQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,q,
        exception);
      break;
    }
    case GreenQuantum:
    case MagentaQuantum:
    {
      ExportGreenQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,q,
        exception);
      break;
    }
    case IndexQuantum:
    {
      ExportIndexQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,
        indexes,q,exception);
      break;
    }
    case IndexAlphaQuantum:
    {
      ExportIndexAlphaQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,
        indexes,q,exception);
      break;
    }
    case OpacityQuantum:
    {
      ExportOpacityQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,q,
        exception);
      break;
    }
    case RedQuantum:
    case CyanQuantum:
    {
      ExportRedQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,q,
        exception);
      break;
    }
    case RGBQuantum:
    case CbYCrQuantum:
    {
      ExportRGBQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,q,
        exception);
      break;
    }
    case RGBAQuantum:
    case RGBOQuantum:
    case CbYCrAQuantum:
    {
      ExportRGBAQuantum(image,(QuantumInfo *) quantum_info,number_pixels,p,q,
        exception);
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
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        quantum=GetPixelRed(q);
        SetPixelRed(q,GetPixelGreen(q));
        SetPixelGreen(q,quantum);
        q++;
      }
    }
  if ((quantum_type == RGBOQuantum) || (quantum_type == CMYKOQuantum) ||
      (quantum_type == BGROQuantum))
    {
      register PixelPacket
        *restrict q;

      q=GetAuthenticPixelQueue(image);
      if (image_view != (CacheView *) NULL)
        q=(PixelPacket *) GetCacheViewVirtualPixelQueue(image_view);
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        SetPixelOpacity(q,(Quantum) GetPixelAlpha(q));
        q++;
      }
    }
  return(extent);
}

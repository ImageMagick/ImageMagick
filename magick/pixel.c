/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                      PPPP   IIIII  X   X  EEEEE  L                          %
%                      P   P    I     X X   E      L                          %
%                      PPPP     I      X    EEE    L                          %
%                      P        I     X X   E      L                          %
%                      P      IIIII  X   X  EEEEE  LLLLL                      %
%                                                                             %
%                  MagickCore Methods to Import/Export Pixels                 %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                               October 1998                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/cache.h"
#include "magick/constitute.h"
#include "magick/delegate.h"
#include "magick/geometry.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/memory-private.h"
#include "magick/monitor.h"
#include "magick/option.h"
#include "magick/pixel.h"
#include "magick/pixel-private.h"
#include "magick/quantum.h"
#include "magick/resource_.h"
#include "magick/semaphore.h"
#include "magick/statistic.h"
#include "magick/stream.h"
#include "magick/string_.h"
#include "magick/transform.h"
#include "magick/utility.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e M a g i c k P i x e l P a c k e t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneMagickPixelPacket() initializes the MagickPixelPacket structure.
%
%  The format of the CloneMagickPixelPacket method is:
%
%      MagickPixelPacket *CloneMagickPixelPacket(MagickPixelPacket *pixel)
%
%  A description of each parameter follows:
%
%    o pixel: Specifies a pointer to a PixelPacket structure.
%
*/
MagickExport MagickPixelPacket *CloneMagickPixelPacket(
  const MagickPixelPacket *pixel)
{
  MagickPixelPacket
    *clone_pixel;

  clone_pixel=(MagickPixelPacket *) MagickAssumeAligned(AcquireAlignedMemory(1,
    sizeof(*clone_pixel)));
  if (clone_pixel == (MagickPixelPacket *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  *clone_pixel=(*pixel);
  return(clone_pixel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e c o d e P i x e l G a m m a                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecodePixelGamma() applies the expansive power-law nonlinearity to the pixel.
%
%  The format of the DecodePixelGamma method is:
%
%      MagickRealType DecodePixelGamma(const MagickRealType pixel)
%
%  A description of each parameter follows:
%
%    o pixel: the pixel.
%
*/

static inline double DecodeGamma(const double x)
{
  div_t
    quotient;

  double
    p,
    term[9];

  int
    exponent;

  static const double coefficient[] =  /* terms for x^(7/5), x=1.5 */
  {
    1.7917488588043277509,
    0.82045614371976854984,
    0.027694100686325412819,
    -0.00094244335181762134018,
    0.000064355540911469709545,
    -5.7224404636060757485e-06,
    5.8767669437311184313e-07,
    -6.6139920053589721168e-08,
    7.9323242696227458163e-09
  };

  static const double powers_of_two[] =  /* (2^x)^(7/5) */
  {
    1.0,
    2.6390158215457883983,
    6.9644045063689921093,
    1.8379173679952558018e+01,
    4.8502930128332728543e+01
  };

  /*
    Compute x^2.4 == x*x^(7/5) == pow(x,2.4).
  */
  term[0]=1.0;
  term[1]=4.0*frexp(x,&exponent)-3.0;
  term[2]=2.0*term[1]*term[1]-term[0];
  term[3]=2.0*term[1]*term[2]-term[1];
  term[4]=2.0*term[1]*term[3]-term[2];
  term[5]=2.0*term[1]*term[4]-term[3];
  term[6]=2.0*term[1]*term[5]-term[4];
  term[7]=2.0*term[1]*term[6]-term[5];
  term[8]=2.0*term[1]*term[7]-term[6];
  p=coefficient[0]*term[0]+coefficient[1]*term[1]+coefficient[2]*term[2]+
    coefficient[3]*term[3]+coefficient[4]*term[4]+coefficient[5]*term[5]+
    coefficient[6]*term[6]+coefficient[7]*term[7]+coefficient[8]*term[8];
  quotient=div(exponent-1,5);
  if (quotient.rem < 0)
    {
      quotient.quot-=1;
      quotient.rem+=5;
    }
  return(x*ldexp(powers_of_two[quotient.rem]*p,7*quotient.quot));
}

MagickExport MagickRealType DecodePixelGamma(const MagickRealType pixel)
{
  if (pixel <= (0.0404482362771076*QuantumRange))
    return(pixel/12.92f);
  return((MagickRealType) (QuantumRange*DecodeGamma((double) (QuantumScale*
    pixel+0.055)/1.055)));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E n c o d e P i x e l G a m m a                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncodePixelGamma() cancels any nonlinearity in the pixel.
%
%  The format of the EncodePixelGamma method is:
%
%      MagickRealType EncodePixelGamma(const MagickRealType pixel)
%
%  A description of each parameter follows:
%
%    o pixel: the pixel.
%
*/

static inline double EncodeGamma(const double x)
{
  div_t
    quotient;

  double
    p,
    term[9];

  int
    exponent;

  static const double coefficient[] =  /* Chebychevi poly: x^(5/12), x=1.5 */
  {
    1.1758200232996901923,
    0.16665763094889061230,
    -0.0083154894939042125035,
    0.00075187976780420279038,
    -0.000083240178519391795367,
    0.000010229209410070008679,
    -1.3400466409860246e-06,
    1.8333422241635376682e-07,
    -2.5878596761348859722e-08
  };

  static const double powers_of_two[] =  /* (2^N)^(5/12) */
  {
    1.0,
    1.3348398541700343678,
    1.7817974362806785482,
    2.3784142300054420538,
    3.1748021039363991669,
    4.2378523774371812394,
    5.6568542494923805819,
    7.5509945014535482244,
    1.0079368399158985525e1,
    1.3454342644059433809e1,
    1.7959392772949968275e1,
    2.3972913230026907883e1
  };

  /*
    Compute x^(1/2.4) == x^(5/12) == pow(x,1.0/2.4).
  */
  term[0]=1.0;
  term[1]=4.0*frexp(x,&exponent)-3.0;
  term[2]=2.0*term[1]*term[1]-term[0];
  term[3]=2.0*term[1]*term[2]-term[1];
  term[4]=2.0*term[1]*term[3]-term[2];
  term[5]=2.0*term[1]*term[4]-term[3];
  term[6]=2.0*term[1]*term[5]-term[4];
  term[7]=2.0*term[1]*term[6]-term[5];
  term[8]=2.0*term[1]*term[7]-term[6];
  p=coefficient[0]*term[0]+coefficient[1]*term[1]+coefficient[2]*term[2]+
    coefficient[3]*term[3]+coefficient[4]*term[4]+coefficient[5]*term[5]+
    coefficient[6]*term[6]+coefficient[7]*term[7]+coefficient[8]*term[8];
  quotient=div(exponent-1,12);
  if (quotient.rem < 0)
    {
      quotient.quot-=1;
      quotient.rem+=12;
    }
  return(ldexp(powers_of_two[quotient.rem]*p,5*quotient.quot));
}

MagickExport MagickRealType EncodePixelGamma(const MagickRealType pixel)
{
  if (pixel <= (0.0031306684425005883*QuantumRange))
    return(12.92f*pixel);
  return((MagickRealType) QuantumRange*(1.055*EncodeGamma((double) QuantumScale*
    pixel)-0.055));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E x p o r t I m a g e P i x e l s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExportImagePixels() extracts pixel data from an image and returns it to you.
%  The method returns MagickTrue on success otherwise MagickFalse if an error is
%  encountered.  The data is returned as char, short int, unsigned int,
%  unsigned long long, float, or double in the order specified by map.
%
%  Suppose you want to extract the first scanline of a 640x480 image as
%  character data in red-green-blue order:
%
%      ExportImagePixels(image,0,0,640,1,"RGB",CharPixel,pixels,exception);
%
%  The format of the ExportImagePixels method is:
%
%      MagickBooleanType ExportImagePixels(const Image *image,const ssize_t x,
%        const ssize_t y,const size_t width,const size_t height,
%        const char *map,const StorageType type,void *pixels,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y,width,height:  These values define the perimeter of a region of
%      pixels you want to extract.
%
%    o map:  This string reflects the expected ordering of the pixel array.
%      It can be any combination or order of R = red, G = green, B = blue,
%      A = alpha (0 is transparent), O = opacity (0 is opaque), C = cyan,
%      Y = yellow, M = magenta, K = black, I = intensity (for grayscale),
%      P = pad.
%
%    o type: Define the data type of the pixels.  Float and double types are
%      normalized to [0..1] otherwise [0..QuantumRange].  Choose from these
%      types: CharPixel (char *), DoublePixel (double *), FloatPixel (float *),
%      LongPixel (unsigned int *), QuantumPixel (Quantum *), or
%      ShortPixel (unsigned short *).
%
%    o pixels: This array of values contain the pixel components as defined by
%      map and type.  You must preallocate this array where the expected
%      length varies depending on the values of width, height, map, and type.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void ExportCharPixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,void *pixels,
  ExceptionInfo *exception)
{
  register const IndexPacket
    *restrict indexes;

  register const PixelPacket
    *restrict p;

  register ssize_t
    x;

  register unsigned char
    *q;

  size_t
    length;

  ssize_t
    y;

  q=(unsigned char *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(GetPixelBlue(p));
          *q++=ScaleQuantumToChar(GetPixelGreen(p));
          *q++=ScaleQuantumToChar(GetPixelRed(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(GetPixelBlue(p));
          *q++=ScaleQuantumToChar(GetPixelGreen(p));
          *q++=ScaleQuantumToChar(GetPixelRed(p));
          *q++=ScaleQuantumToChar((Quantum) GetPixelAlpha(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(GetPixelBlue(p));
          *q++=ScaleQuantumToChar(GetPixelGreen(p));
          *q++=ScaleQuantumToChar(GetPixelRed(p));
          *q++=ScaleQuantumToChar((Quantum) 0);
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(ClampToQuantum(GetPixelIntensity(image,p)));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(GetPixelRed(p));
          *q++=ScaleQuantumToChar(GetPixelGreen(p));
          *q++=ScaleQuantumToChar(GetPixelBlue(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(GetPixelRed(p));
          *q++=ScaleQuantumToChar(GetPixelGreen(p));
          *q++=ScaleQuantumToChar(GetPixelBlue(p));
          *q++=ScaleQuantumToChar((Quantum) GetPixelAlpha(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(GetPixelRed(p));
          *q++=ScaleQuantumToChar(GetPixelGreen(p));
          *q++=ScaleQuantumToChar(GetPixelBlue(p));
          *q++=ScaleQuantumToChar((Quantum) 0);
          p++;
        }
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=ScaleQuantumToChar(GetPixelRed(p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=ScaleQuantumToChar(GetPixelGreen(p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=ScaleQuantumToChar(GetPixelBlue(p));
            break;
          }
          case AlphaQuantum:
          {
            *q=ScaleQuantumToChar((Quantum) GetPixelAlpha(p));
            break;
          }
          case OpacityQuantum:
          {
            *q=ScaleQuantumToChar(GetPixelOpacity(p));
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=ScaleQuantumToChar(GetPixelIndex(indexes+x));
            break;
          }
          case IndexQuantum:
          {
            *q=ScaleQuantumToChar(ClampToQuantum(GetPixelIntensity(image,p)));
            break;
          }
          default:
            break;
        }
        q++;
      }
      p++;
    }
  }
}

static void ExportDoublePixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,void *pixels,
  ExceptionInfo *exception)
{
  register const IndexPacket
    *restrict indexes;

  register const PixelPacket
    *restrict p;

  register double
    *q;

  register ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  q=(double *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(double) (QuantumScale*GetPixelBlue(p));
          *q++=(double) (QuantumScale*GetPixelGreen(p));
          *q++=(double) (QuantumScale*GetPixelRed(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(double) (QuantumScale*GetPixelBlue(p));
          *q++=(double) (QuantumScale*GetPixelGreen(p));
          *q++=(double) (QuantumScale*GetPixelRed(p));
          *q++=(double) (QuantumScale*((Quantum) (QuantumRange-
            GetPixelOpacity(p))));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(double) (QuantumScale*GetPixelBlue(p));
          *q++=(double) (QuantumScale*GetPixelGreen(p));
          *q++=(double) (QuantumScale*GetPixelRed(p));
          *q++=0.0;
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(double) (QuantumScale*GetPixelIntensity(image,p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(double) (QuantumScale*GetPixelRed(p));
          *q++=(double) (QuantumScale*GetPixelGreen(p));
          *q++=(double) (QuantumScale*GetPixelBlue(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(double) (QuantumScale*GetPixelRed(p));
          *q++=(double) (QuantumScale*GetPixelGreen(p));
          *q++=(double) (QuantumScale*GetPixelBlue(p));
          *q++=(double) (QuantumScale*((Quantum) (QuantumRange-
            GetPixelOpacity(p))));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(double) (QuantumScale*GetPixelRed(p));
          *q++=(double) (QuantumScale*GetPixelGreen(p));
          *q++=(double) (QuantumScale*GetPixelBlue(p));
          *q++=0.0;
          p++;
        }
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=(double) (QuantumScale*GetPixelRed(p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=(double) (QuantumScale*GetPixelGreen(p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=(double) (QuantumScale*GetPixelBlue(p));
            break;
          }
          case AlphaQuantum:
          {
            *q=(double) (QuantumScale*((Quantum) (QuantumRange-
              GetPixelOpacity(p))));
            break;
          }
          case OpacityQuantum:
          {
            *q=(double) (QuantumScale*GetPixelOpacity(p));
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=(double) (QuantumScale*GetPixelIndex(indexes+x));
            break;
          }
          case IndexQuantum:
          {
            *q=(double) (QuantumScale*GetPixelIntensity(image,p));
            break;
          }
          default:
            *q=0;
        }
        q++;
      }
      p++;
    }
  }
}

static void ExportFloatPixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,void *pixels,
  ExceptionInfo *exception)
{
  register const IndexPacket
    *restrict indexes;

  register const PixelPacket
    *restrict p;

  register float
    *q;

  register ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  q=(float *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*GetPixelBlue(p));
          *q++=(float) (QuantumScale*GetPixelGreen(p));
          *q++=(float) (QuantumScale*GetPixelRed(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*GetPixelBlue(p));
          *q++=(float) (QuantumScale*GetPixelGreen(p));
          *q++=(float) (QuantumScale*GetPixelRed(p));
          *q++=(float) (QuantumScale*GetPixelAlpha(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*GetPixelBlue(p));
          *q++=(float) (QuantumScale*GetPixelGreen(p));
          *q++=(float) (QuantumScale*GetPixelRed(p));
          *q++=0.0;
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*GetPixelIntensity(image,p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*GetPixelRed(p));
          *q++=(float) (QuantumScale*GetPixelGreen(p));
          *q++=(float) (QuantumScale*GetPixelBlue(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*GetPixelRed(p));
          *q++=(float) (QuantumScale*GetPixelGreen(p));
          *q++=(float) (QuantumScale*GetPixelBlue(p));
          *q++=(float) (QuantumScale*GetPixelAlpha(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*GetPixelRed(p));
          *q++=(float) (QuantumScale*GetPixelGreen(p));
          *q++=(float) (QuantumScale*GetPixelBlue(p));
          *q++=0.0;
          p++;
        }
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=(float) (QuantumScale*GetPixelRed(p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=(float) (QuantumScale*GetPixelGreen(p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=(float) (QuantumScale*GetPixelBlue(p));
            break;
          }
          case AlphaQuantum:
          {
            *q=(float) (QuantumScale*((Quantum) (GetPixelAlpha(p))));
            break;
          }
          case OpacityQuantum:
          {
            *q=(float) (QuantumScale*GetPixelOpacity(p));
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=(float) (QuantumScale*GetPixelIndex(indexes+x));
            break;
          }
          case IndexQuantum:
          {
            *q=(float) (QuantumScale*GetPixelIntensity(image,p));
            break;
          }
          default:
            *q=0;
        }
        q++;
      }
      p++;
    }
  }
}

static void ExportIntegerPixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,void *pixels,
  ExceptionInfo *exception)
{
  register const IndexPacket
    *restrict indexes;

  register const PixelPacket
    *restrict p;

  register ssize_t
    x;

  register unsigned int
    *q;

  size_t
    length;

  ssize_t
    y;

  q=(unsigned int *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
          *q++=(unsigned int) ScaleQuantumToLong((Quantum) (QuantumRange-
            GetPixelOpacity(p)));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
          *q++=0U;
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(unsigned int) ScaleQuantumToLong(ClampToQuantum(
            GetPixelIntensity(image,p)));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
          *q++=(unsigned int) ScaleQuantumToLong((Quantum) GetPixelAlpha(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
          *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
          *q++=0U;
          p++;
        }
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
            break;
          }
          case AlphaQuantum:
          {
            *q=(unsigned int) ScaleQuantumToLong((Quantum) (QuantumRange-
              GetPixelOpacity(p)));
            break;
          }
          case OpacityQuantum:
          {
            *q=(unsigned int) ScaleQuantumToLong(GetPixelOpacity(p));
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=(unsigned int) ScaleQuantumToLong(GetPixelIndex(indexes+x));
            break;
          }
          case IndexQuantum:
          {
            *q=(unsigned int) ScaleQuantumToLong(ClampToQuantum(
              GetPixelIntensity(image,p)));
            break;
          }
          default:
            *q=0;
        }
        q++;
      }
      p++;
    }
  }
}

static void ExportLongPixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,void *pixels,
  ExceptionInfo *exception)
{
  register const IndexPacket
    *restrict indexes;

  register const PixelPacket
    *restrict p;

  register unsigned int
    *q;

  register ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  q=(unsigned int *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(GetPixelBlue(p));
          *q++=ScaleQuantumToLong(GetPixelGreen(p));
          *q++=ScaleQuantumToLong(GetPixelRed(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(GetPixelBlue(p));
          *q++=ScaleQuantumToLong(GetPixelGreen(p));
          *q++=ScaleQuantumToLong(GetPixelRed(p));
          *q++=ScaleQuantumToLong((Quantum) (GetPixelAlpha(p)));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(GetPixelBlue(p));
          *q++=ScaleQuantumToLong(GetPixelGreen(p));
          *q++=ScaleQuantumToLong(GetPixelRed(p));
          *q++=0;
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(ClampToQuantum(GetPixelIntensity(image,p)));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(GetPixelRed(p));
          *q++=ScaleQuantumToLong(GetPixelGreen(p));
          *q++=ScaleQuantumToLong(GetPixelBlue(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(GetPixelRed(p));
          *q++=ScaleQuantumToLong(GetPixelGreen(p));
          *q++=ScaleQuantumToLong(GetPixelBlue(p));
          *q++=ScaleQuantumToLong((Quantum) (GetPixelAlpha(p)));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(GetPixelRed(p));
          *q++=ScaleQuantumToLong(GetPixelGreen(p));
          *q++=ScaleQuantumToLong(GetPixelBlue(p));
          *q++=0;
          p++;
        }
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=ScaleQuantumToLong(GetPixelRed(p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=ScaleQuantumToLong(GetPixelGreen(p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=ScaleQuantumToLong(GetPixelBlue(p));
            break;
          }
          case AlphaQuantum:
          {
            *q=ScaleQuantumToLong((Quantum) GetPixelAlpha(p));
            break;
          }
          case OpacityQuantum:
          {
            *q=ScaleQuantumToLong(GetPixelOpacity(p));
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=ScaleQuantumToLong(GetPixelIndex(indexes+x));
            break;
          }
          case IndexQuantum:
          {
            *q=ScaleQuantumToLong(ClampToQuantum(GetPixelIntensity(image,p)));
            break;
          }
          default:
            break;
        }
        q++;
      }
      p++;
    }
  }
}

static void ExportQuantumPixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,void *pixels,
  ExceptionInfo *exception)
{
  register const IndexPacket
    *restrict indexes;

  register const PixelPacket
    *restrict p;

  register Quantum
    *q;

  register ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  q=(Quantum *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=GetPixelBlue(p);
          *q++=GetPixelGreen(p);
          *q++=GetPixelRed(p);
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=GetPixelBlue(p);
          *q++=GetPixelGreen(p);
          *q++=GetPixelRed(p);
          *q++=(Quantum) GetPixelAlpha(p);
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=GetPixelBlue(p);
          *q++=GetPixelGreen(p);
          *q++=GetPixelRed(p);
          *q++=(Quantum) 0;
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ClampToQuantum(GetPixelIntensity(image,p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=GetPixelRed(p);
          *q++=GetPixelGreen(p);
          *q++=GetPixelBlue(p);
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=GetPixelRed(p);
          *q++=GetPixelGreen(p);
          *q++=GetPixelBlue(p);
          *q++=(Quantum) GetPixelAlpha(p);
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=GetPixelRed(p);
          *q++=GetPixelGreen(p);
          *q++=GetPixelBlue(p);
          *q++=(Quantum) 0;
          p++;
        }
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=(Quantum) 0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=GetPixelRed(p);
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=GetPixelGreen(p);
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=GetPixelBlue(p);
            break;
          }
          case AlphaQuantum:
          {
            *q=(Quantum) (GetPixelAlpha(p));
            break;
          }
          case OpacityQuantum:
          {
            *q=GetPixelOpacity(p);
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=GetPixelIndex(indexes+x);
            break;
          }
          case IndexQuantum:
          {
            *q=(ClampToQuantum(GetPixelIntensity(image,p)));
            break;
          }
          default:
          {
            *q=(Quantum) 0;
            break;
          }
        }
        q++;
      }
      p++;
    }
  }
}

static void ExportShortPixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,void *pixels,
  ExceptionInfo *exception)
{
  register const IndexPacket
    *restrict indexes;

  register const PixelPacket
    *restrict p;

  register ssize_t
    x;

  register unsigned short
    *q;

  size_t
    length;

  ssize_t
    y;

  q=(unsigned short *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(GetPixelBlue(p));
          *q++=ScaleQuantumToShort(GetPixelGreen(p));
          *q++=ScaleQuantumToShort(GetPixelRed(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(GetPixelBlue(p));
          *q++=ScaleQuantumToShort(GetPixelGreen(p));
          *q++=ScaleQuantumToShort(GetPixelRed(p));
          *q++=ScaleQuantumToShort((Quantum) GetPixelAlpha(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(GetPixelBlue(p));
          *q++=ScaleQuantumToShort(GetPixelGreen(p));
          *q++=ScaleQuantumToShort(GetPixelRed(p));
          *q++=0;
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(ClampToQuantum(GetPixelIntensity(image,p)));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(GetPixelRed(p));
          *q++=ScaleQuantumToShort(GetPixelGreen(p));
          *q++=ScaleQuantumToShort(GetPixelBlue(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(GetPixelRed(p));
          *q++=ScaleQuantumToShort(GetPixelGreen(p));
          *q++=ScaleQuantumToShort(GetPixelBlue(p));
          *q++=ScaleQuantumToShort((Quantum) GetPixelAlpha(p));
          p++;
        }
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(GetPixelRed(p));
          *q++=ScaleQuantumToShort(GetPixelGreen(p));
          *q++=ScaleQuantumToShort(GetPixelBlue(p));
          *q++=0;
          p++;
        }
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=ScaleQuantumToShort(GetPixelRed(p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=ScaleQuantumToShort(GetPixelGreen(p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=ScaleQuantumToShort(GetPixelBlue(p));
            break;
          }
          case AlphaQuantum:
          {
            *q=ScaleQuantumToShort((Quantum) (GetPixelAlpha(p)));
            break;
          }
          case OpacityQuantum:
          {
            *q=ScaleQuantumToShort(GetPixelOpacity(p));
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=ScaleQuantumToShort(GetPixelIndex(indexes+x));
            break;
          }
          case IndexQuantum:
          {
            *q=ScaleQuantumToShort(ClampToQuantum(GetPixelIntensity(image,p)));
            break;
          }
          default:
            break;
        }
        q++;
      }
      p++;
    }
  }
}

MagickExport MagickBooleanType ExportImagePixels(const Image *image,
  const ssize_t x,const ssize_t y,const size_t width,const size_t height,
  const char *map,const StorageType type,void *pixels,ExceptionInfo *exception)
{
  QuantumType
    *quantum_map;

  RectangleInfo
    roi;

  register ssize_t
    i;

  size_t
    length;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  length=strlen(map);
  quantum_map=(QuantumType *) AcquireQuantumMemory(length,sizeof(*quantum_map));
  if (quantum_map == (QuantumType *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  for (i=0; i < (ssize_t) length; i++)
  {
    switch (map[i])
    {
      case 'A':
      case 'a':
      {
        quantum_map[i]=AlphaQuantum;
        break;
      }
      case 'B':
      case 'b':
      {
        quantum_map[i]=BlueQuantum;
        break;
      }
      case 'C':
      case 'c':
      {
        quantum_map[i]=CyanQuantum;
        if (image->colorspace == CMYKColorspace)
          break;
        quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
        (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
          "ColorSeparatedImageRequired","`%s'",map);
        return(MagickFalse);
      }
      case 'g':
      case 'G':
      {
        quantum_map[i]=GreenQuantum;
        break;
      }
      case 'I':
      case 'i':
      {
        quantum_map[i]=IndexQuantum;
        break;
      }
      case 'K':
      case 'k':
      {
        quantum_map[i]=BlackQuantum;
        if (image->colorspace == CMYKColorspace)
          break;
        quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
        (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
          "ColorSeparatedImageRequired","`%s'",map);
        return(MagickFalse);
      }
      case 'M':
      case 'm':
      {
        quantum_map[i]=MagentaQuantum;
        if (image->colorspace == CMYKColorspace)
          break;
        quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
        (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
          "ColorSeparatedImageRequired","`%s'",map);
        return(MagickFalse);
      }
      case 'o':
      case 'O':
      {
        quantum_map[i]=OpacityQuantum;
        break;
      }
      case 'P':
      case 'p':
      {
        quantum_map[i]=UndefinedQuantum;
        break;
      }
      case 'R':
      case 'r':
      {
        quantum_map[i]=RedQuantum;
        break;
      }
      case 'Y':
      case 'y':
      {
        quantum_map[i]=YellowQuantum;
        if (image->colorspace == CMYKColorspace)
          break;
        quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
        (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
          "ColorSeparatedImageRequired","`%s'",map);
        return(MagickFalse);
      }
      default:
      {
        quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "UnrecognizedPixelMap","`%s'",map);
        return(MagickFalse);
      }
    }
  }
  roi.width=width;
  roi.height=height;
  roi.x=x;
  roi.y=y;
  switch (type)
  {
    case CharPixel:
    {
      ExportCharPixel((Image *) image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case DoublePixel:
    {
      ExportDoublePixel((Image *) image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case FloatPixel:
    {
      ExportFloatPixel((Image *) image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case IntegerPixel:
    {
      ExportIntegerPixel((Image *) image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case LongPixel:
    {
      ExportLongPixel((Image *) image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case QuantumPixel:
    {
      ExportQuantumPixel((Image *) image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case ShortPixel:
    {
      ExportShortPixel((Image *) image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    default:
    {
      quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "UnrecognizedPixelMap","`%s'",map);
      break;
    }
  }
  quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k P i x e l P a c k e t                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickPixelPacket() initializes the MagickPixelPacket structure.
%
%  The format of the GetMagickPixelPacket method is:
%
%      GetMagickPixelPacket(const Image *image,MagickPixelPacket *pixel)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o pixel: Specifies a pointer to a PixelPacket structure.
%
*/
MagickExport void GetMagickPixelPacket(const Image *image,
  MagickPixelPacket *pixel)
{
  pixel->storage_class=DirectClass;
  pixel->colorspace=sRGBColorspace;
  pixel->matte=MagickFalse;
  pixel->fuzz=0.0;
  pixel->depth=MAGICKCORE_QUANTUM_DEPTH;
  pixel->red=0.0;
  pixel->green=0.0;
  pixel->blue=0.0;
  pixel->opacity=(MagickRealType) OpaqueOpacity;
  pixel->index=0.0;
  if (image == (const Image *) NULL)
    return;
  pixel->storage_class=image->storage_class;
  pixel->colorspace=image->colorspace;
  pixel->matte=image->matte;
  pixel->depth=image->depth;
  pixel->fuzz=image->fuzz;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P i x e l I n t e n s i t y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelIntensity() returns a single sample intensity value from the red,
%  green, and blue components of a pixel based on the selected method:
%
%    Rec601Luma       0.298839R' + 0.586811G' + 0.114350B'
%    Rec601Luminance  0.298839R + 0.586811G + 0.114350B
%    Rec709Luma       0.212656R' + 0.715158G' + 0.072186B'
%    Rec709Luminance  0.212656R + 0.715158G + 0.072186B
%    Brightness       max(R', G', B')
%    Lightness        (min(R', G', B') + max(R', G', B')) / 2.0
%
%    MS               (R^2 + G^2 + B^2) / 3.0
%    RMS              sqrt(R^2 + G^2 + B^2) / 3.0
%    Average          (R + G + B) / 3.0
%
%  The format of the GetPixelIntensity method is:
%
%      MagickRealType GetPixelIntensity(const Image *image,
%        const PixelPacket *pixel)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o pixel: Specifies a pointer to a PixelPacket structure.
%
*/

static inline MagickRealType MagickMax(const MagickRealType x,
  const MagickRealType y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline MagickRealType MagickMin(const MagickRealType x,
  const MagickRealType y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport MagickRealType GetPixelIntensity(const Image *image,
  const PixelPacket *restrict pixel)
{
  MagickRealType
    blue,
    green,
    intensity,
    red;

  if (image->colorspace == GRAYColorspace)
    return((MagickRealType) pixel->red);
  red=(MagickRealType) pixel->red;
  green=(MagickRealType) pixel->green;
  blue=(MagickRealType) pixel->blue;
  switch (image->intensity)
  {
    case AveragePixelIntensityMethod:
    {
      intensity=(red+green+blue)/3.0;
      break;
    }
    case BrightnessPixelIntensityMethod:
    {
      intensity=MagickMax(MagickMax(red,green),blue);
      break;
    }
    case LightnessPixelIntensityMethod:
    {
      intensity=(MagickMin(MagickMin(red,green),blue)+
        MagickMax(MagickMax(red,green),blue))/2.0;
      break;
    }
    case MSPixelIntensityMethod:
    {
      intensity=(MagickRealType) (((double) red*red+green*green+blue*blue)/
        (3.0*QuantumRange));
      break;
    }
    case Rec601LumaPixelIntensityMethod:
    {
      if (image->colorspace == RGBColorspace)
        {
          red=EncodePixelGamma(red);
          green=EncodePixelGamma(green);
          blue=EncodePixelGamma(blue);
        }
      intensity=0.298839*red+0.586811*green+0.114350*blue;
      break;
    }
    case Rec601LuminancePixelIntensityMethod:
    {
      if (image->colorspace == sRGBColorspace)
        {
          red=DecodePixelGamma(red);
          green=DecodePixelGamma(green);
          blue=DecodePixelGamma(blue);
        }
      intensity=0.298839*red+0.586811*green+0.114350*blue;
      break;
    }
    case Rec709LumaPixelIntensityMethod:
    default:
    {
      if (image->colorspace == RGBColorspace)
        {
          red=EncodePixelGamma(red);
          green=EncodePixelGamma(green);
          blue=EncodePixelGamma(blue);
        }
      intensity=0.212656*red+0.715158*green+0.072186*blue;
      break;
    }
    case Rec709LuminancePixelIntensityMethod:
    {
      if (image->colorspace == sRGBColorspace)
        {
          red=DecodePixelGamma(red);
          green=DecodePixelGamma(green);
          blue=DecodePixelGamma(blue);
        }
      intensity=0.212656*red+0.715158*green+0.072186*blue;
      break;
    }
    case RMSPixelIntensityMethod:
    {
      intensity=(MagickRealType) (sqrt((double) red*red+green*green+blue*blue)/
        sqrt(3.0));
      break;
    }
  }
  return(intensity);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I m p o r t I m a g e P i x e l s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImportImagePixels() accepts pixel data and stores in the image at the
%  location you specify.  The method returns MagickTrue on success otherwise
%  MagickFalse if an error is encountered.  The pixel data can be either char,
%  short int, unsigned int, unsigned long long, float, or double in the order
%  specified by map.
%
%  Suppose your want to upload the first scanline of a 640x480 image from
%  character data in red-green-blue order:
%
%      ImportImagePixels(image,0,0,640,1,"RGB",CharPixel,pixels);
%
%  The format of the ImportImagePixels method is:
%
%      MagickBooleanType ImportImagePixels(Image *image,const ssize_t x,
%        const ssize_t y,const size_t width,const size_t height,
%        const char *map,const StorageType type,const void *pixels)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y,width,height:  These values define the perimeter of a region of
%      pixels you want to define.
%
%    o map:  This string reflects the expected ordering of the pixel array.
%      It can be any combination or order of R = red, G = green, B = blue,
%      A = alpha (0 is transparent), O = opacity (0 is opaque), C = cyan,
%      Y = yellow, M = magenta, K = black, I = intensity (for grayscale),
%      P = pad.
%
%    o type: Define the data type of the pixels.  Float and double types are
%      normalized to [0..1] otherwise [0..QuantumRange].  Choose from these
%      types: CharPixel (char *), DoublePixel (double *), FloatPixel (float *),
%      LongPixel (unsigned int *), QuantumPixel (Quantum *), or ShortPixel
%      (unsigned short *).
%
%    o pixels: This array of values contain the pixel components as defined by
%      map and type.  You must preallocate this array where the expected
%      length varies depending on the values of width, height, map, and type.
%
*/

static void ImportCharPixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,const void *pixels,
  ExceptionInfo *exception)
{
  register const unsigned char
    *restrict p;

  register IndexPacket
    *restrict indexes;

  register PixelPacket
    *restrict q;

  register ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  p=(const unsigned char *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ScaleCharToQuantum(*p++));
          SetPixelGreen(q,ScaleCharToQuantum(*p++));
          SetPixelRed(q,ScaleCharToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ScaleCharToQuantum(*p++));
          SetPixelGreen(q,ScaleCharToQuantum(*p++));
          SetPixelRed(q,ScaleCharToQuantum(*p++));
          SetPixelAlpha(q,ScaleCharToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRO") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ScaleCharToQuantum(*p++));
          SetPixelGreen(q,ScaleCharToQuantum(*p++));
          SetPixelRed(q,ScaleCharToQuantum(*p++));
          SetPixelOpacity(q,ScaleCharToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ScaleCharToQuantum(*p++));
          SetPixelGreen(q,ScaleCharToQuantum(*p++));
          SetPixelRed(q,ScaleCharToQuantum(*p++));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleCharToQuantum(*p++));
          SetPixelGreen(q,GetPixelRed(q));
          SetPixelBlue(q,GetPixelRed(q));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleCharToQuantum(*p++));
          SetPixelGreen(q,ScaleCharToQuantum(*p++));
          SetPixelBlue(q,ScaleCharToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleCharToQuantum(*p++));
          SetPixelGreen(q,ScaleCharToQuantum(*p++));
          SetPixelBlue(q,ScaleCharToQuantum(*p++));
          SetPixelAlpha(q,ScaleCharToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBO") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleCharToQuantum(*p++));
          SetPixelGreen(q,ScaleCharToQuantum(*p++));
          SetPixelBlue(q,ScaleCharToQuantum(*p++));
          SetPixelOpacity(q,ScaleCharToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleCharToQuantum(*p++));
          SetPixelGreen(q,ScaleCharToQuantum(*p++));
          SetPixelBlue(q,ScaleCharToQuantum(*p++));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(q,ScaleCharToQuantum(*p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(q,ScaleCharToQuantum(*p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(q,ScaleCharToQuantum(*p));
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(q,ScaleCharToQuantum(*p));
            break;
          }
          case OpacityQuantum:
          {
            SetPixelOpacity(q,ScaleCharToQuantum(*p));
            break;
          }
          case BlackQuantum:
          {
            SetPixelIndex(indexes+x,ScaleCharToQuantum(*p));
            break;
          }
          case IndexQuantum:
          {
            SetPixelRed(q,ScaleCharToQuantum(*p));
            SetPixelGreen(q,GetPixelRed(q));
            SetPixelBlue(q,GetPixelRed(q));
            break;
          }
          default:
            break;
        }
        p++;
      }
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
}

static void ImportDoublePixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,const void *pixels,
  ExceptionInfo *exception)
{
  register const double
    *restrict p;

  register IndexPacket
    *restrict indexes;

  register PixelPacket
    *restrict q;

  register ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  p=(const double *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          q->opacity=QuantumRange-ClampToQuantum((MagickRealType)
            QuantumRange*(*p));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          SetPixelGreen(q,GetPixelRed(q));
          SetPixelBlue(q,GetPixelRed(q));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelAlpha(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(q,ClampToQuantum((MagickRealType)
              QuantumRange*(*p)));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(q,ClampToQuantum((MagickRealType)
              QuantumRange*(*p)));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(q,ClampToQuantum((MagickRealType)
              QuantumRange*(*p)));
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(q,ClampToQuantum((MagickRealType)
              QuantumRange*(*p)));
            break;
          }
          case OpacityQuantum:
          {
            SetPixelOpacity(q,ClampToQuantum((MagickRealType)
              QuantumRange*(*p)));
            break;
          }
          case BlackQuantum:
          {
            SetPixelIndex(indexes+x,ClampToQuantum((MagickRealType)
              QuantumRange*(*p)));
            break;
          }
          case IndexQuantum:
          {
            SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
            SetPixelGreen(q,GetPixelRed(q));
            SetPixelBlue(q,GetPixelRed(q));
            break;
          }
          default:
            break;
        }
        p++;
      }
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
}

static void ImportFloatPixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,const void *pixels,
  ExceptionInfo *exception)
{
  register const float
    *restrict p;

  register IndexPacket
    *restrict indexes;

  register PixelPacket
    *restrict q;

  register ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  p=(const float *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelAlpha(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          SetPixelGreen(q,GetPixelRed(q));
          SetPixelBlue(q,GetPixelRed(q));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelAlpha(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
            break;
          }
          case OpacityQuantum:
          {
            SetPixelOpacity(q,ClampToQuantum((MagickRealType)
              QuantumRange*(*p)));
            break;
          }
          case BlackQuantum:
          {
            SetPixelIndex(indexes+x,ClampToQuantum((MagickRealType)
              QuantumRange*(*p)));
            break;
          }
          case IndexQuantum:
          {
            SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*(*p)));
            SetPixelGreen(q,GetPixelRed(q));
            SetPixelBlue(q,GetPixelRed(q));
            break;
          }
          default:
            break;
        }
        p++;
      }
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
}

static void ImportIntegerPixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,const void *pixels,
  ExceptionInfo *exception)
{
  register const unsigned int
    *restrict p;

  register IndexPacket
    *restrict indexes;

  register PixelPacket
    *restrict q;

  register ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  p=(const unsigned int *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,ScaleLongToQuantum(*p++));
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,ScaleLongToQuantum(*p++));
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          SetPixelAlpha(q,ScaleLongToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,ScaleLongToQuantum(*p++));
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,GetPixelRed(q));
          SetPixelBlue(q,GetPixelRed(q));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,ScaleLongToQuantum(*p++));
          SetPixelBlue(q,ScaleLongToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,ScaleLongToQuantum(*p++));
          SetPixelBlue(q,ScaleLongToQuantum(*p++));
          SetPixelAlpha(q,ScaleLongToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,ScaleLongToQuantum(*p++));
          SetPixelBlue(q,ScaleLongToQuantum(*p++));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(q,ScaleLongToQuantum(*p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(q,ScaleLongToQuantum(*p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(q,ScaleLongToQuantum(*p));
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(q,ScaleLongToQuantum(*p));
            break;
          }
          case OpacityQuantum:
          {
            SetPixelOpacity(q,ScaleLongToQuantum(*p));
            break;
          }
          case BlackQuantum:
          {
            SetPixelIndex(indexes+x,ScaleLongToQuantum(*p));
            break;
          }
          case IndexQuantum:
          {
            SetPixelRed(q,ScaleLongToQuantum(*p));
            SetPixelGreen(q,GetPixelRed(q));
            SetPixelBlue(q,GetPixelRed(q));
            break;
          }
          default:
            break;
        }
        p++;
      }
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
}

static void ImportLongPixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,const void *pixels,
  ExceptionInfo *exception)
{
  register const unsigned int
    *restrict p;

  register IndexPacket
    *restrict indexes;

  register PixelPacket
    *restrict q;

  register ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  p=(const unsigned int *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,ScaleLongToQuantum(*p++));
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,ScaleLongToQuantum(*p++));
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          SetPixelAlpha(q,ScaleLongToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,ScaleLongToQuantum(*p++));
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,GetPixelRed(q));
          SetPixelBlue(q,GetPixelRed(q));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,ScaleLongToQuantum(*p++));
          SetPixelBlue(q,ScaleLongToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,ScaleLongToQuantum(*p++));
          SetPixelBlue(q,ScaleLongToQuantum(*p++));
          SetPixelAlpha(q,ScaleLongToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleLongToQuantum(*p++));
          SetPixelGreen(q,ScaleLongToQuantum(*p++));
          SetPixelBlue(q,ScaleLongToQuantum(*p++));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(q,ScaleLongToQuantum(*p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(q,ScaleLongToQuantum(*p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(q,ScaleLongToQuantum(*p));
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(q,ScaleLongToQuantum(*p));
            break;
          }
          case OpacityQuantum:
          {
          SetPixelOpacity(q,ScaleLongToQuantum(*p));
          break;
          }
          case BlackQuantum:
          {
            SetPixelIndex(indexes+x,ScaleLongToQuantum(*p));
            break;
          }
          case IndexQuantum:
          {
            SetPixelRed(q,ScaleLongToQuantum(*p));
            SetPixelGreen(q,GetPixelRed(q));
            SetPixelBlue(q,GetPixelRed(q));
            break;
          }
          default:
            break;
        }
        p++;
      }
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
}

static void ImportQuantumPixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,const void *pixels,
  ExceptionInfo *exception)
{
  register const Quantum
    *restrict p;

  register IndexPacket
    *restrict indexes;

  register PixelPacket
    *restrict q;

  register ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  p=(const Quantum *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,*p++);
          SetPixelGreen(q,*p++);
          SetPixelRed(q,*p++);
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,*p++);
          SetPixelGreen(q,*p++);
          SetPixelRed(q,*p++);
          SetPixelAlpha(q,*p++);
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,*p++);
          SetPixelGreen(q,*p++);
          SetPixelRed(q,*p++);
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,*p++);
          SetPixelGreen(q,GetPixelRed(q));
          SetPixelBlue(q,GetPixelRed(q));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,*p++);
          SetPixelGreen(q,*p++);
          SetPixelBlue(q,*p++);
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,*p++);
          SetPixelGreen(q,*p++);
          SetPixelBlue(q,*p++);
          SetPixelAlpha(q,*p++);
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,*p++);
          SetPixelGreen(q,*p++);
          SetPixelBlue(q,*p++);
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(q,*p);
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(q,*p);
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(q,*p);
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(q,*p);
            break;
          }
          case OpacityQuantum:
          {
            SetPixelOpacity(q,*p);
            break;
          }
          case BlackQuantum:
          {
            SetPixelIndex(indexes+x,*p);
            break;
          }
          case IndexQuantum:
          {
            SetPixelRed(q,*p);
            SetPixelGreen(q,GetPixelRed(q));
            SetPixelBlue(q,GetPixelRed(q));
            break;
          }
          default:
            break;
        }
        p++;
      }
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
}

static void ImportShortPixel(Image *image,const RectangleInfo *roi,
  const char *restrict map,const QuantumType *quantum_map,const void *pixels,
  ExceptionInfo *exception)
{
  register const unsigned short
    *restrict p;

  register IndexPacket
    *restrict indexes;

  register PixelPacket
    *restrict q;

  register ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  p=(const unsigned short *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ScaleShortToQuantum(*p++));
          SetPixelGreen(q,ScaleShortToQuantum(*p++));
          SetPixelRed(q,ScaleShortToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ScaleShortToQuantum(*p++));
          SetPixelGreen(q,ScaleShortToQuantum(*p++));
          SetPixelRed(q,ScaleShortToQuantum(*p++));
          SetPixelAlpha(q,ScaleShortToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(q,ScaleShortToQuantum(*p++));
          SetPixelGreen(q,ScaleShortToQuantum(*p++));
          SetPixelRed(q,ScaleShortToQuantum(*p++));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleShortToQuantum(*p++));
          SetPixelGreen(q,GetPixelRed(q));
          SetPixelBlue(q,GetPixelRed(q));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleShortToQuantum(*p++));
          SetPixelGreen(q,ScaleShortToQuantum(*p++));
          SetPixelBlue(q,ScaleShortToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleShortToQuantum(*p++));
          SetPixelGreen(q,ScaleShortToQuantum(*p++));
          SetPixelBlue(q,ScaleShortToQuantum(*p++));
          SetPixelAlpha(q,ScaleShortToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(q,ScaleShortToQuantum(*p++));
          SetPixelGreen(q,ScaleShortToQuantum(*p++));
          SetPixelBlue(q,ScaleShortToQuantum(*p++));
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return;
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(q,ScaleShortToQuantum(*p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(q,ScaleShortToQuantum(*p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(q,ScaleShortToQuantum(*p));
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(q,ScaleShortToQuantum(*p));
            break;
          }
          case OpacityQuantum:
          {
            SetPixelOpacity(q,ScaleShortToQuantum(*p));
            break;
          }
          case BlackQuantum:
          {
            SetPixelIndex(indexes+x,ScaleShortToQuantum(*p));
            break;
          }
          case IndexQuantum:
          {
            SetPixelRed(q,ScaleShortToQuantum(*p));
            SetPixelGreen(q,GetPixelRed(q));
            SetPixelBlue(q,GetPixelRed(q));
            break;
          }
          default:
            break;
        }
        p++;
      }
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
}

MagickExport MagickBooleanType ImportImagePixels(Image *image,const ssize_t x,
  const ssize_t y,const size_t width,const size_t height,const char *map,
  const StorageType type,const void *pixels)
{
  ExceptionInfo
    *exception;

  QuantumType
    *quantum_map;

  RectangleInfo
    roi;

  register ssize_t
    i;

  size_t
    length;

  /*
    Allocate image structure.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  length=strlen(map);
  quantum_map=(QuantumType *) AcquireQuantumMemory(length,sizeof(*quantum_map));
  if (quantum_map == (QuantumType *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  for (i=0; i < (ssize_t) length; i++)
  {
    switch (map[i])
    {
      case 'a':
      case 'A':
      {
        quantum_map[i]=AlphaQuantum;
        image->matte=MagickTrue;
        break;
      }
      case 'B':
      case 'b':
      {
        quantum_map[i]=BlueQuantum;
        break;
      }
      case 'C':
      case 'c':
      {
        quantum_map[i]=CyanQuantum;
        (void) SetImageColorspace(image,CMYKColorspace);
        break;
      }
      case 'g':
      case 'G':
      {
        quantum_map[i]=GreenQuantum;
        break;
      }
      case 'K':
      case 'k':
      {
        quantum_map[i]=BlackQuantum;
        (void) SetImageColorspace(image,CMYKColorspace);
        break;
      }
      case 'I':
      case 'i':
      {
        quantum_map[i]=IndexQuantum;
        (void) SetImageColorspace(image,GRAYColorspace);
        break;
      }
      case 'm':
      case 'M':
      {
        quantum_map[i]=MagentaQuantum;
        (void) SetImageColorspace(image,CMYKColorspace);
        break;
      }
      case 'O':
      case 'o':
      {
        quantum_map[i]=OpacityQuantum;
        image->matte=MagickTrue;
        break;
      }
      case 'P':
      case 'p':
      {
        quantum_map[i]=UndefinedQuantum;
        break;
      }
      case 'R':
      case 'r':
      {
        quantum_map[i]=RedQuantum;
        break;
      }
      case 'Y':
      case 'y':
      {
        quantum_map[i]=YellowQuantum;
        (void) SetImageColorspace(image,CMYKColorspace);
        break;
      }
      default:
      {
        quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
        (void) ThrowMagickException(&image->exception,GetMagickModule(),
          OptionError,"UnrecognizedPixelMap","`%s'",map);
        return(MagickFalse);
      }
    }
  }
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  /*
    Transfer the pixels from the pixel datarray to the image.
  */
  exception=(&image->exception);
  roi.width=width;
  roi.height=height;
  roi.x=x;
  roi.y=y;
  switch (type)
  {
    case CharPixel:
    {
      ImportCharPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case DoublePixel:
    {
      ImportDoublePixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case FloatPixel:
    {
      ImportFloatPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case IntegerPixel:
    {
      ImportIntegerPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case LongPixel:
    {
      ImportLongPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case QuantumPixel:
    {
      ImportQuantumPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case ShortPixel:
    {
      ImportShortPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    default:
    {
      quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        OptionError,"UnrecognizedStorageType","`%d'",type);
      break;
    }
  }
  quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n t e r p o l a t e M a g i c k P i x e l P a c k e t                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpolateMagickPixelPacket() applies bi-linear or tri-linear interpolation
%  between a floating point coordinate and the pixels surrounding that
%  coordinate.  No pixel area resampling, or scaling of the result is
%  performed.
%
%  The format of the InterpolateMagickPixelPacket method is:
%
%      MagickBooleanType InterpolateMagickPixelPacket(const Image *image,
%        const CacheView *image_view,const InterpolatePixelMethod method,
%        const double x,const double y,MagickPixelPacket *pixel,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o image_view: the image view.
%
%    o method: the pixel color interpolation method.
%
%    o x,y: A double representing the current (x,y) position of the pixel.
%
%    o pixel: return the interpolated pixel here.
%
%    o exception: return any errors or warnings in this structure.
%
*/

/*
  Prepare pixels for weighted alpha blending.
  Save PixelPacket/IndexPacket 'color'/'index' into 'pixel'/'alpha'
  after multiplying colors by alpha.
*/
static inline void AlphaBlendMagickPixelPacket(const Image *image,
  const PixelPacket *color,const IndexPacket *indexes,MagickPixelPacket *pixel,
  MagickRealType *alpha)
{
  if (image->matte == MagickFalse)
    {
      *alpha=1.0;
      pixel->red=(MagickRealType) GetPixelRed(color);
      pixel->green=(MagickRealType) GetPixelGreen(color);
      pixel->blue=(MagickRealType) GetPixelBlue(color);
      pixel->opacity=(MagickRealType) GetPixelOpacity(color);
      pixel->index=0.0;
      if (((image->colorspace == CMYKColorspace) ||
           (image->storage_class == PseudoClass)) &&
          (indexes != (const IndexPacket *) NULL))
        pixel->index=(MagickRealType) GetPixelIndex(indexes);
      return;
    }
  *alpha=QuantumScale*GetPixelAlpha(color);
  pixel->red=(*alpha*GetPixelRed(color));
  pixel->green=(*alpha*GetPixelGreen(color));
  pixel->blue=(*alpha*GetPixelBlue(color));
  pixel->opacity=(MagickRealType) GetPixelOpacity(color);
  pixel->index=0.0;
  if (((image->colorspace == CMYKColorspace) ||
       (image->storage_class == PseudoClass)) &&
      (indexes != (const IndexPacket *) NULL))
    pixel->index=(*alpha*GetPixelIndex(indexes));
}

static inline void CatromWeights(const MagickRealType x,
  MagickRealType (*weights)[4])
{
  /*
    Nicolas Robidoux' 10 flops (4* + 5- + 1+) refactoring of the
    computation of the standard four 1D Catmull-Rom weights. The
    sampling location is assumed between the second and third input
    pixel locations, and x is the position relative to the second
    input pixel location. Formulas originally derived for the VIPS
    (Virtual Image Processing System) library.
  */
  MagickRealType
    alpha,
    beta,
    gamma;

  alpha=(MagickRealType) 1.0-x;
  beta=(MagickRealType) (-0.5)*x*alpha;
  (*weights)[0]=alpha*beta;
  (*weights)[3]=x*beta;
  /*
    The following computation of the inner weights from the outer ones
    works for all Keys cubics.
  */
  gamma=(*weights)[3]-(*weights)[0];
  (*weights)[1]=alpha-(*weights)[0]+gamma;
  (*weights)[2]=x-(*weights)[3]-gamma;
}

static inline void SplineWeights(const MagickRealType x,
  MagickRealType (*weights)[4])
{
  /*
    Nicolas Robidoux' 12 flops (6* + 5- + 1+) refactoring of the
    computation of the standard four 1D cubic B-spline smoothing
    weights. The sampling location is assumed between the second and
    third input pixel locations, and x is the position relative to the
    second input pixel location.
  */
  MagickRealType
    alpha,
    beta;

  alpha=(MagickRealType) 1.0-x;
  (*weights)[3]=(MagickRealType) (1.0/6.0)*x*x*x;
  (*weights)[0]=(MagickRealType) (1.0/6.0)*alpha*alpha*alpha;
  beta=(*weights)[3]-(*weights)[0];
  (*weights)[1]=alpha-(*weights)[0]+beta;
  (*weights)[2]=x-(*weights)[3]-beta;
}

static inline double MeshInterpolate(const PointInfo *delta,const double p,
  const double x,const double y)
{
  return(delta->x*x+delta->y*y+(1.0-delta->x-delta->y)*p);
}

MagickExport MagickBooleanType InterpolateMagickPixelPacket(const Image *image,
  const CacheView *image_view,const InterpolatePixelMethod method,
  const double x,const double y,MagickPixelPacket *pixel,
  ExceptionInfo *exception)
{
  double
    gamma;

  MagickBooleanType
    status;

  MagickPixelPacket
    pixels[16];

  MagickRealType
    alpha[16];

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register ssize_t
    i;

  ssize_t
    x_offset,
    y_offset;

  InterpolatePixelMethod
    interpolate;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image_view != (CacheView *) NULL);
  status=MagickTrue;
  x_offset=(ssize_t) floor(x);
  y_offset=(ssize_t) floor(y);
  interpolate = method;
  if ( interpolate == UndefinedInterpolatePixel )
    interpolate = image->interpolate;
  switch (interpolate)
  {
    case AverageInterpolatePixel:        /* nearest 4 neighbours */
    case Average9InterpolatePixel:       /* nearest 9 neighbours */
    case Average16InterpolatePixel:      /* nearest 16 neighbours */
    {
      ssize_t
        count;

      count=2;  /* size of the area to average - default nearest 4 */
      if (interpolate == Average9InterpolatePixel)
        {
          count=3;
          x_offset=(ssize_t) (floor(x+0.5)-1);
          y_offset=(ssize_t) (floor(y+0.5)-1);
        }
      else
        if (interpolate == Average16InterpolatePixel)
          {
            count=4;
            x_offset--;
            y_offset--;
          }
      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,(size_t) count,
        (size_t) count,exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      pixel->red=0.0;
      pixel->green=0.0;
      pixel->blue=0.0;
      pixel->opacity=0.0;
      pixel->index=0.0;
      count*=count;  /* number of pixels - square of size */
      for (i=0; i < (ssize_t) count; i++)
      {
        AlphaBlendMagickPixelPacket(image,p+i,indexes+i,pixels,alpha);
        gamma=PerceptibleReciprocal(alpha[0]);
        pixel->red+=gamma*pixels[0].red;
        pixel->green+=gamma*pixels[0].green;
        pixel->blue+=gamma*pixels[0].blue;
        pixel->index+=gamma*pixels[0].index;
        pixel->opacity+=pixels[0].opacity;
      }
      gamma=1.0/count;  /* average weighting of each pixel in area */
      pixel->red*=gamma;
      pixel->green*=gamma;
      pixel->blue*=gamma;
      pixel->index*=gamma;
      pixel->opacity*=gamma;
      break;
    }
    case BackgroundInterpolatePixel:
    {
      IndexPacket
        index;

      index=0;  /* CMYK index -- What should we do?  -- This is a HACK */
      SetMagickPixelPacket(image,&image->background_color,&index,pixel);
      break;
    }
    case BilinearInterpolatePixel:
    default:
    {
      PointInfo
        delta,
        epsilon;

      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,2,2,exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      for (i=0; i < 4L; i++)
        AlphaBlendMagickPixelPacket(image,p+i,indexes+i,pixels+i,alpha+i);
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      epsilon.x=1.0-delta.x;
      epsilon.y=1.0-delta.y;
      gamma=((epsilon.y*(epsilon.x*alpha[0]+delta.x*alpha[1])+delta.y*
        (epsilon.x*alpha[2]+delta.x*alpha[3])));
      gamma=PerceptibleReciprocal(gamma);
      pixel->red=gamma*(epsilon.y*(epsilon.x*pixels[0].red+delta.x*
        pixels[1].red)+delta.y*(epsilon.x*pixels[2].red+delta.x*pixels[3].red));
      pixel->green=gamma*(epsilon.y*(epsilon.x*pixels[0].green+delta.x*
        pixels[1].green)+delta.y*(epsilon.x*pixels[2].green+delta.x*
        pixels[3].green));
      pixel->blue=gamma*(epsilon.y*(epsilon.x*pixels[0].blue+delta.x*
        pixels[1].blue)+delta.y*(epsilon.x*pixels[2].blue+delta.x*
        pixels[3].blue));
      if (image->colorspace == CMYKColorspace)
        pixel->index=gamma*(epsilon.y*(epsilon.x*pixels[0].index+delta.x*
          pixels[1].index)+delta.y*(epsilon.x*pixels[2].index+delta.x*
          pixels[3].index));
      gamma=((epsilon.y*(epsilon.x+delta.x)+delta.y*(epsilon.x+delta.x)));
      gamma=PerceptibleReciprocal(gamma);
      pixel->opacity=(epsilon.y*(epsilon.x*pixels[0].opacity+delta.x*
        pixels[1].opacity)+delta.y*(epsilon.x*pixels[2].opacity+delta.x*
        pixels[3].opacity));
      break;
    }
    case BlendInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,2,2,exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      for (i=0; i < 4L; i++)
        AlphaBlendMagickPixelPacket(image,p+i,indexes+i,pixels+i,alpha+i);
      gamma=1.0;  /* number of pixels blended together */
      for (i=0; i <= 1L; i++)
      {
        if ((y-y_offset) >= 0.75)
          {
            alpha[i]=alpha[i+2];
            pixels[i]=pixels[i+2];
          }
        else
          if ((y-y_offset) > 0.25)
            {
              gamma=2.0;  /* each y pixels have been blended */
              alpha[i]+=alpha[i+2];  /* add up alpha weights */
              pixels[i].red+=pixels[i+2].red;
              pixels[i].green+=pixels[i+2].green;
              pixels[i].blue+=pixels[i+2].blue;
              pixels[i].opacity+=pixels[i+2].opacity;
              pixels[i].index+=pixels[i+2].index;
            }
      }
      if ((x-x_offset) >= 0.75 )
        {
          alpha[0]=alpha[1];
          pixels[0]=pixels[1];
        }
      else
        if ((x-x_offset) > 0.25 )
          {
            gamma*=2.0;  /* double number of pixels blended */
            alpha[0]+=alpha[1];  /* add up alpha weights */
            pixels[0].red+=pixels[1].red;
            pixels[0].green+=pixels[1].green;
            pixels[0].blue+=pixels[1].blue;
            pixels[0].opacity+=pixels[1].opacity;
            pixels[0].index+=pixels[1].index;
          }
      gamma=1.0/gamma;  /* 1/sum(pixels) */
      alpha[0]=PerceptibleReciprocal(alpha[0]);  /* 1/sum(alpha) */
      pixel->red=alpha[0]*pixels[0].red;
      pixel->green=alpha[0]*pixels[0].green;  /* divide by sum of alpha */
      pixel->blue=alpha[0]*pixels[0].blue;
      pixel->index=alpha[0]*pixels[0].index;
      pixel->opacity=gamma*pixels[0].opacity; /* divide by number pixels */
      break;
    }
    case CatromInterpolatePixel:
    case BicubicInterpolatePixel: /* deprecated method */
    {
      MagickRealType
        cx[4],
        cy[4];

      p=GetCacheViewVirtualPixels(image_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      for (i=0; i < 16L; i++)
        AlphaBlendMagickPixelPacket(image,p+i,indexes+i,pixels+i,alpha+i);
      CatromWeights((MagickRealType) (x-x_offset),&cx);
      CatromWeights((MagickRealType) (y-y_offset),&cy);
      pixel->red=(cy[0]*(cx[0]*pixels[0].red+cx[1]*pixels[1].red+cx[2]*
        pixels[2].red+cx[3]*pixels[3].red)+cy[1]*(cx[0]*pixels[4].red+cx[1]*
        pixels[5].red+cx[2]*pixels[6].red+cx[3]*pixels[7].red)+cy[2]*(cx[0]*
        pixels[8].red+cx[1]*pixels[9].red+cx[2]*pixels[10].red+cx[3]*
        pixels[11].red)+cy[3]*(cx[0]*pixels[12].red+cx[1]*pixels[13].red+cx[2]*
        pixels[14].red+cx[3]*pixels[15].red));
      pixel->green=(cy[0]*(cx[0]*pixels[0].green+cx[1]*pixels[1].green+cx[2]*
        pixels[2].green+cx[3]*pixels[3].green)+cy[1]*(cx[0]*pixels[4].green+
        cx[1]*pixels[5].green+cx[2]*pixels[6].green+cx[3]*pixels[7].green)+
        cy[2]*(cx[0]*pixels[8].green+cx[1]*pixels[9].green+cx[2]*
        pixels[10].green+cx[3]*pixels[11].green)+cy[3]*(cx[0]*pixels[12].green+
        cx[1]*pixels[13].green+cx[2]*pixels[14].green+cx[3]*pixels[15].green));
      pixel->blue=(cy[0]*(cx[0]*pixels[0].blue+cx[1]*pixels[1].blue+cx[2]*
        pixels[2].blue+cx[3]*pixels[3].blue)+cy[1]*(cx[0]*pixels[4].blue+cx[1]*
        pixels[5].blue+cx[2]*pixels[6].blue+cx[3]*pixels[7].blue)+cy[2]*(cx[0]*
        pixels[8].blue+cx[1]*pixels[9].blue+cx[2]*pixels[10].blue+cx[3]*
        pixels[11].blue)+cy[3]*(cx[0]*pixels[12].blue+cx[1]*pixels[13].blue+
        cx[2]*pixels[14].blue+cx[3]*pixels[15].blue));
      pixel->opacity=(cy[0]*(cx[0]*pixels[0].opacity+cx[1]*pixels[1].opacity+
        cx[2]*pixels[2].opacity+cx[3]*pixels[3].opacity)+cy[1]*(cx[0]*
        pixels[4].opacity+cx[1]*pixels[5].opacity+cx[2]*pixels[6].opacity+
        cx[3]*pixels[7].opacity)+cy[2]*(cx[0]*pixels[8].opacity+cx[1]*
        pixels[9].opacity+cx[2]*pixels[10].opacity+cx[3]*pixels[11].opacity)+
        cy[3]*(cx[0]*pixels[12].opacity+cx[1]*pixels[13].opacity+cx[2]*
        pixels[14].opacity+cx[3]*pixels[15].opacity));
      break;
    }
    case FilterInterpolatePixel:
    {
      CacheView
        *filter_view;

      Image
        *excerpt_image,
        *filter_image;

      RectangleInfo
        geometry;

      /*
        Use a normal resize filter: warning this is slow due to setup time a
        cache should be initialised on first call, and replaced if 'filter'
        changes.
      */
      geometry.width=4L;
      geometry.height=4L;
      geometry.x=x_offset-1;
      geometry.y=y_offset-1;
      excerpt_image=ExcerptImage(image,&geometry,exception);
      if (excerpt_image == (Image *) NULL)
        {
          status=MagickFalse;
          break;
        }
      filter_image=ResizeImage(excerpt_image,1,1,image->filter,image->blur,
        exception);
      excerpt_image=DestroyImage(excerpt_image);
      if (filter_image == (Image *) NULL)
        break;
      filter_view=AcquireVirtualCacheView(filter_image,exception);
      p=GetCacheViewVirtualPixels(filter_view,0,0,1,1,exception);
      if (p != (const PixelPacket *) NULL)
        {
          indexes=GetVirtualIndexQueue(filter_image);
          SetMagickPixelPacket(image,p,indexes,pixel);
        }
      filter_view=DestroyCacheView(filter_view);
      filter_image=DestroyImage(filter_image);
      break;
    }
    case IntegerInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,1,1,exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      SetMagickPixelPacket(image,p,indexes,pixel);
      break;
    }
    case MeshInterpolatePixel:
    {
      PointInfo
        delta,
        luma;

      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,2,2,
        exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      for (i=0; i < 4L; i++)
        AlphaBlendMagickPixelPacket(image,p+i,indexes+i,pixels+i,alpha+i);
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      luma.x=fabs(MagickPixelLuma(pixels+0)-MagickPixelLuma(pixels+3));
      luma.y=fabs(MagickPixelLuma(pixels+1)-MagickPixelLuma(pixels+2));
      if (luma.x < luma.y)
        {
          /*
            Diagonal 0-3 NW-SE.
          */
          if (delta.x <= delta.y)
            {
              /*
                Bottom-left triangle  (pixel:2, diagonal: 0-3).
              */
              delta.y=1.0-delta.y;
              gamma=MeshInterpolate(&delta,alpha[2],alpha[3],alpha[0]);
              gamma=PerceptibleReciprocal(gamma);
              pixel->red=gamma*MeshInterpolate(&delta,pixels[2].red,
                pixels[3].red,pixels[0].red);
              pixel->green=gamma*MeshInterpolate(&delta,pixels[2].green,
                pixels[3].green,pixels[0].green);
              pixel->blue=gamma*MeshInterpolate(&delta,pixels[2].blue,
                pixels[3].blue,pixels[0].blue);
              if (image->colorspace == CMYKColorspace)
                pixel->index=gamma*MeshInterpolate(&delta,pixels[2].index,
                  pixels[3].index,pixels[0].index);
              gamma=MeshInterpolate(&delta,1.0,1.0,1.0);
              pixel->opacity=gamma*MeshInterpolate(&delta,pixels[2].opacity,
                pixels[3].opacity,pixels[0].opacity);
            }
          else
            {
              /*
                Top-right triangle (pixel:1, diagonal: 0-3).
              */
              delta.x=1.0-delta.x;
              gamma=MeshInterpolate(&delta,alpha[1],alpha[0],alpha[3]);
              gamma=PerceptibleReciprocal(gamma);
              pixel->red=gamma*MeshInterpolate(&delta,pixels[1].red,
                pixels[0].red,pixels[3].red);
              pixel->green=gamma*MeshInterpolate(&delta,pixels[1].green,
                pixels[0].green,pixels[3].green);
              pixel->blue=gamma*MeshInterpolate(&delta,pixels[1].blue,
                pixels[0].blue,pixels[3].blue);
              if (image->colorspace == CMYKColorspace)
                pixel->index=gamma*MeshInterpolate(&delta,pixels[1].index,
                  pixels[0].index,pixels[3].index);
              gamma=MeshInterpolate(&delta,1.0,1.0,1.0);
              pixel->opacity=gamma*MeshInterpolate(&delta,pixels[1].opacity,
                pixels[0].opacity,pixels[3].opacity);
            }
        }
      else
        {
          /*
            Diagonal 1-2 NE-SW.
          */
          if (delta.x <= (1.0-delta.y))
            {
              /*
                Top-left triangle (pixel: 0, diagonal: 1-2).
              */
              gamma=MeshInterpolate(&delta,alpha[0],alpha[1],alpha[2]);
              gamma=PerceptibleReciprocal(gamma);
              pixel->red=gamma*MeshInterpolate(&delta,pixels[0].red,
                pixels[1].red,pixels[2].red);
              pixel->green=gamma*MeshInterpolate(&delta,pixels[0].green,
                pixels[1].green,pixels[2].green);
              pixel->blue=gamma*MeshInterpolate(&delta,pixels[0].blue,
                pixels[1].blue,pixels[2].blue);
              if (image->colorspace == CMYKColorspace)
                pixel->index=gamma*MeshInterpolate(&delta,pixels[0].index,
                  pixels[1].index,pixels[2].index);
              gamma=MeshInterpolate(&delta,1.0,1.0,1.0);
              pixel->opacity=gamma*MeshInterpolate(&delta,pixels[0].opacity,
                pixels[1].opacity,pixels[2].opacity);
            }
          else
            {
              /*
                Bottom-right triangle (pixel: 3, diagonal: 1-2).
              */
              delta.x=1.0-delta.x;
              delta.y=1.0-delta.y;
              gamma=MeshInterpolate(&delta,alpha[3],alpha[2],alpha[1]);
              gamma=PerceptibleReciprocal(gamma);
              pixel->red=gamma*MeshInterpolate(&delta,pixels[3].red,
                pixels[2].red,pixels[1].red);
              pixel->green=gamma*MeshInterpolate(&delta,pixels[3].green,
                pixels[2].green,pixels[1].green);
              pixel->blue=gamma*MeshInterpolate(&delta,pixels[3].blue,
                pixels[2].blue,pixels[1].blue);
              if (image->colorspace == CMYKColorspace)
                pixel->index=gamma*MeshInterpolate(&delta,pixels[3].index,
                  pixels[2].index,pixels[1].index);
              gamma=MeshInterpolate(&delta,1.0,1.0,1.0);
              pixel->opacity=gamma*MeshInterpolate(&delta,pixels[3].opacity,
                pixels[2].opacity,pixels[1].opacity);
            }
        }
      break;
    }
    case NearestNeighborInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(image_view,(ssize_t) floor(x+0.5),
        (ssize_t) floor(y+0.5),1,1,exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      SetMagickPixelPacket(image,p,indexes,pixel);
      break;
    }
    case SplineInterpolatePixel:
    {
      MagickRealType
        cx[4],
        cy[4];

      p=GetCacheViewVirtualPixels(image_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      for (i=0; i < 16L; i++)
        AlphaBlendMagickPixelPacket(image,p+i,indexes+i,pixels+i,alpha+i);
      SplineWeights((MagickRealType) (x-x_offset),&cx);
      SplineWeights((MagickRealType) (y-y_offset),&cy);
      pixel->red=(cy[0]*(cx[0]*pixels[0].red+cx[1]*pixels[1].red+cx[2]*
        pixels[2].red+cx[3]*pixels[3].red)+cy[1]*(cx[0]*pixels[4].red+cx[1]*
        pixels[5].red+cx[2]*pixels[6].red+cx[3]*pixels[7].red)+cy[2]*(cx[0]*
        pixels[8].red+cx[1]*pixels[9].red+cx[2]*pixels[10].red+cx[3]*
        pixels[11].red)+cy[3]*(cx[0]*pixels[12].red+cx[1]*pixels[13].red+cx[2]*
        pixels[14].red+cx[3]*pixels[15].red));
      pixel->green=(cy[0]*(cx[0]*pixels[0].green+cx[1]*pixels[1].green+cx[2]*
        pixels[2].green+cx[3]*pixels[3].green)+cy[1]*(cx[0]*pixels[4].green+
        cx[1]*pixels[5].green+cx[2]*pixels[6].green+cx[3]*pixels[7].green)+
        cy[2]*(cx[0]*pixels[8].green+cx[1]*pixels[9].green+cx[2]*
        pixels[10].green+cx[3]*pixels[11].green)+cy[3]*(cx[0]*pixels[12].green+
        cx[1]*pixels[13].green+cx[2]*pixels[14].green+cx[3]*pixels[15].green));
      pixel->blue=(cy[0]*(cx[0]*pixels[0].blue+cx[1]*pixels[1].blue+cx[2]*
        pixels[2].blue+cx[3]*pixels[3].blue)+cy[1]*(cx[0]*pixels[4].blue+cx[1]*
        pixels[5].blue+cx[2]*pixels[6].blue+cx[3]*pixels[7].blue)+cy[2]*(cx[0]*
        pixels[8].blue+cx[1]*pixels[9].blue+cx[2]*pixels[10].blue+cx[3]*
        pixels[11].blue)+cy[3]*(cx[0]*pixels[12].blue+cx[1]*pixels[13].blue+
        cx[2]*pixels[14].blue+cx[3]*pixels[15].blue));
      pixel->opacity=(cy[0]*(cx[0]*pixels[0].opacity+cx[1]*pixels[1].opacity+
        cx[2]*pixels[2].opacity+cx[3]*pixels[3].opacity)+cy[1]*(cx[0]*
        pixels[4].opacity+cx[1]*pixels[5].opacity+cx[2]*pixels[6].opacity+cx[3]*
        pixels[7].opacity)+cy[2]*(cx[0]*pixels[8].opacity+cx[1]*
        pixels[9].opacity+cx[2]*pixels[10].opacity+cx[3]*pixels[11].opacity)+
        cy[3]*(cx[0]*pixels[12].opacity+cx[1]*pixels[13].opacity+cx[2]*
        pixels[14].opacity+cx[3]*pixels[15].opacity));
      break;
    }
  }
  return(status);
}

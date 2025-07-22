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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/property.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/constitute.h"
#include "MagickCore/delegate.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/pixel-private.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/statistic.h"
#include "MagickCore/stream.h"
#include "MagickCore/string_.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e P i x e l C h a n n e l M a p                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquirePixelChannelMap() acquires a pixel component map.
%
%  The format of the AcquirePixelChannelMap() method is:
%
%      PixelChannelMap *AcquirePixelChannelMap(void)
%
*/
MagickExport PixelChannelMap *AcquirePixelChannelMap(void)
{
  PixelChannelMap
    *channel_map;

  ssize_t
    i;

  channel_map=(PixelChannelMap *) AcquireQuantumMemory(MaxPixelChannels+1,
    sizeof(*channel_map));
  if (channel_map == (PixelChannelMap *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(channel_map,0,(MaxPixelChannels+1)*sizeof(*channel_map));
  for (i=0; i <= MaxPixelChannels; i++)
    channel_map[i].channel=(PixelChannel) i;
  return(channel_map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l o n e P i x e l C h a n n e l M a p                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClonePixelChannelMap() clones a pixel component map.
%
%  The format of the ClonePixelChannelMap() method is:
%
%      PixelChannelMap *ClonePixelChannelMap(PixelChannelMap *channel_map)
%
%  A description of each parameter follows:
%
%    o channel_map: the pixel component map.
%
*/
MagickExport PixelChannelMap *ClonePixelChannelMap(PixelChannelMap *channel_map)
{
  PixelChannelMap
    *clone_map;

  assert(channel_map != (PixelChannelMap *) NULL);
  clone_map=AcquirePixelChannelMap();
  if (clone_map == (PixelChannelMap *) NULL)
    return((PixelChannelMap *) NULL);
  (void) memcpy(clone_map,channel_map,MaxPixelChannels*sizeof(*channel_map));
  return(clone_map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l o n e P i x e l I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClonePixelInfo() makes a duplicate of the given pixel info structure, or if
%  pixel info is NULL, a new one.
%
%  The format of the ClonePixelInfo method is:
%
%      PixelInfo *ClonePixelInfo(const PixelInfo *pixel)
%
%  A description of each parameter follows:
%
%    o pixel: the pixel info.
%
*/
MagickExport PixelInfo *ClonePixelInfo(const PixelInfo *pixel)
{
  PixelInfo
    *pixel_info;

  pixel_info=(PixelInfo *) AcquireMagickMemory(sizeof(*pixel_info));
  if (pixel_info == (PixelInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  *pixel_info=(*pixel);
  return(pixel_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o n f o r m P i x e l I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConformPixelInfo() ensures the pixel conforms with the colorspace and alpha
%  attribute of the image.
%
%  The format of the ConformPixelInfo method is:
%
%      void *ConformPixelInfo((Image *image,const PixelInfo *source,
%        PixelInfo *destination,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o source: the source pixel info.
%
%    o destination: the destination pixel info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void ConformPixelInfo(Image *image,const PixelInfo *source,
  PixelInfo *destination,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(destination != (const PixelInfo *) NULL);
  *destination=(*source);
  if (image->colorspace == CMYKColorspace)
    {
      if (IssRGBCompatibleColorspace(destination->colorspace) != MagickFalse)
        ConvertRGBToCMYK(destination);
    }
  else
    if (destination->colorspace == CMYKColorspace)
      {
        if (IssRGBCompatibleColorspace(image->colorspace) != MagickFalse)
          ConvertCMYKToRGB(destination);
      }
  if ((IsPixelInfoGray(&image->background_color) == MagickFalse) &&
      (IsGrayColorspace(image->colorspace) != MagickFalse))
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
  if ((destination->alpha_trait != UndefinedPixelTrait) &&
      ((image->alpha_trait & BlendPixelTrait) == 0))
    (void) SetImageAlpha(image,OpaqueAlpha,exception);
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
%      double DecodePixelGamma(const MagickRealType pixel)
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
  if (pixel <= (0.0404482362771076*(double) QuantumRange))
    return(pixel/12.92);
  return((MagickRealType) ((double) QuantumRange*DecodeGamma((double)
    (QuantumScale*pixel+0.055)/1.055)));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y P i x e l C h a n n e l M a p                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyPixelChannelMap() deallocates memory associated with the pixel
%  channel map.
%
%  The format of the DestroyPixelChannelMap() method is:
%
%      PixelChannelMap *DestroyPixelChannelMap(PixelChannelMap *channel_map)
%
%  A description of each parameter follows:
%
%    o channel_map: the pixel component map.
%
*/
MagickExport PixelChannelMap *DestroyPixelChannelMap(
  PixelChannelMap *channel_map)
{
  assert(channel_map != (PixelChannelMap *) NULL);
  channel_map=(PixelChannelMap *) RelinquishMagickMemory(channel_map);
  return((PixelChannelMap *) RelinquishMagickMemory(channel_map));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E n c o d e P i x e l G a m m a                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncodePixelGamma() cancels any nonlinearity in the pixel.
%
%  The format of the EncodePixelGamma method is:
%
%      MagickRealType EncodePixelGamma(const double MagickRealType)
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
  if (pixel <= (0.0031306684425005883*(double) QuantumRange))
    return(12.92*pixel);
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
%  encountered.  The data is returned as char, short int, Quantum, unsigned int,
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
%    o x,y,width,height:  These values define the perimeter
%      of a region of pixels you want to extract.
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
%      LongPixel (unsigned int *), LongLongPixel (unsigned long long *),
%      QuantumPixel (Quantum *), or ShortPixel (unsigned short *).
%
%    o pixels: This array of values contain the pixel components as defined by
%      map and type.  You must preallocate this array where the expected
%      length varies depending on the values of width, height, map, and type.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType ExportCharPixel(const Image *image,
  const RectangleInfo *roi,const char *magick_restrict map,
  const QuantumType *quantum_map,void *pixels,ExceptionInfo *exception)
{
  const Quantum
    *magick_restrict p;

  ssize_t
    x;

  unsigned char
    *magick_restrict q;

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
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
          *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
          *q++=ScaleQuantumToChar(GetPixelRed(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
          *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
          *q++=ScaleQuantumToChar(GetPixelRed(image,p));
          *q++=ScaleQuantumToChar(GetPixelAlpha(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
          *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
          *q++=ScaleQuantumToChar(GetPixelRed(image,p));
          *q++=ScaleQuantumToChar((Quantum) 0);
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(ClampToQuantum(GetPixelIntensity(image,p)));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(GetPixelRed(image,p));
          *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
          *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(GetPixelRed(image,p));
          *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
          *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
          *q++=ScaleQuantumToChar(GetPixelAlpha(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToChar(GetPixelRed(image,p));
          *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
          *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
          *q++=ScaleQuantumToChar((Quantum) 0);
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=ScaleQuantumToChar(GetPixelRed(image,p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=ScaleQuantumToChar(GetPixelGreen(image,p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=ScaleQuantumToChar(GetPixelBlue(image,p));
            break;
          }
          case AlphaQuantum:
          {
            *q=ScaleQuantumToChar(GetPixelAlpha(image,p));
            break;
          }
          case OpacityQuantum:
          {
            *q=ScaleQuantumToChar(GetPixelAlpha(image,p));
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=ScaleQuantumToChar(GetPixelBlack(image,p));
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
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

static MagickBooleanType ExportDoublePixel(const Image *image,
  const RectangleInfo *roi,const char *magick_restrict map,
  const QuantumType *quantum_map,void *pixels,ExceptionInfo *exception)
{
  const Quantum
    *magick_restrict p;

  double
    *magick_restrict q;

  ssize_t
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
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=QuantumScale*(double) GetPixelBlue(image,p);
          *q++=QuantumScale*(double) GetPixelGreen(image,p);
          *q++=QuantumScale*(double) GetPixelRed(image,p);
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=QuantumScale*(double) GetPixelBlue(image,p);
          *q++=QuantumScale*(double) GetPixelGreen(image,p);
          *q++=QuantumScale*(double) GetPixelRed(image,p);
          *q++=QuantumScale*(double) GetPixelAlpha(image,p);
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=QuantumScale*(double) GetPixelBlue(image,p);
          *q++=QuantumScale*(double) GetPixelGreen(image,p);
          *q++=QuantumScale*(double) GetPixelRed(image,p);
          *q++=0.0;
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(double) (QuantumScale*(double) GetPixelIntensity(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=QuantumScale*(double) GetPixelRed(image,p);
          *q++=QuantumScale*(double) GetPixelGreen(image,p);
          *q++=QuantumScale*(double) GetPixelBlue(image,p);
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=QuantumScale*(double) GetPixelRed(image,p);
          *q++=QuantumScale*(double) GetPixelGreen(image,p);
          *q++=QuantumScale*(double) GetPixelBlue(image,p);
          *q++=QuantumScale*(double) GetPixelAlpha(image,p);
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=QuantumScale*(double) GetPixelRed(image,p);
          *q++=QuantumScale*(double) GetPixelGreen(image,p);
          *q++=QuantumScale*(double) GetPixelBlue(image,p);
          *q++=0.0;
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=QuantumScale*(double) GetPixelRed(image,p);
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=QuantumScale*(double) GetPixelGreen(image,p);
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=QuantumScale*(double) GetPixelBlue(image,p);
            break;
          }
          case AlphaQuantum:
          {
            *q=QuantumScale*(double) GetPixelAlpha(image,p);
            break;
          }
          case OpacityQuantum:
          {
            *q=QuantumScale*(double) GetPixelAlpha(image,p);
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=QuantumScale*(double) GetPixelBlack(image,p);
            break;
          }
          case IndexQuantum:
          {
            *q=QuantumScale*(double) GetPixelIntensity(image,p);
            break;
          }
          default:
            *q=0;
        }
        q++;
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

static MagickBooleanType ExportFloatPixel(const Image *image,
  const RectangleInfo *roi,const char *magick_restrict map,
  const QuantumType *quantum_map,void *pixels,ExceptionInfo *exception)
{
  const Quantum
    *magick_restrict p;

  float
    *magick_restrict q;

  ssize_t
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
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*(double) GetPixelBlue(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelGreen(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelRed(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*(double) GetPixelBlue(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelGreen(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelRed(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelAlpha(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*(double) GetPixelBlue(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelGreen(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelRed(image,p));
          *q++=0.0;
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*(double) GetPixelIntensity(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*(double) GetPixelRed(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelGreen(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelBlue(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*(double) GetPixelRed(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelGreen(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelBlue(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelAlpha(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=(float) (QuantumScale*(double) GetPixelRed(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelGreen(image,p));
          *q++=(float) (QuantumScale*(double) GetPixelBlue(image,p));
          *q++=0.0;
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=(float) (QuantumScale*(double) GetPixelRed(image,p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=(float) (QuantumScale*(double) GetPixelGreen(image,p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=(float) (QuantumScale*(double) GetPixelBlue(image,p));
            break;
          }
          case AlphaQuantum:
          {
            *q=(float) (QuantumScale*((double) (GetPixelAlpha(image,p))));
            break;
          }
          case OpacityQuantum:
          {
            *q=(float) (QuantumScale*(double) GetPixelAlpha(image,p));
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=(float) (QuantumScale*(double) GetPixelBlack(image,p));
            break;
          }
          case IndexQuantum:
          {
            *q=(float) (QuantumScale*(double) GetPixelIntensity(image,p));
            break;
          }
          default:
            *q=0;
        }
        q++;
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

static MagickBooleanType ExportLongPixel(const Image *image,
  const RectangleInfo *roi,const char *magick_restrict map,
  const QuantumType *quantum_map,void *pixels,ExceptionInfo *exception)
{
  const Quantum
    *magick_restrict p;

  ssize_t
    x;

  unsigned int
    *magick_restrict q;

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
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(GetPixelBlue(image,p));
          *q++=ScaleQuantumToLong(GetPixelGreen(image,p));
          *q++=ScaleQuantumToLong(GetPixelRed(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(GetPixelBlue(image,p));
          *q++=ScaleQuantumToLong(GetPixelGreen(image,p));
          *q++=ScaleQuantumToLong(GetPixelRed(image,p));
          *q++=ScaleQuantumToLong(GetPixelAlpha(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(GetPixelBlue(image,p));
          *q++=ScaleQuantumToLong(GetPixelGreen(image,p));
          *q++=ScaleQuantumToLong(GetPixelRed(image,p));
          *q++=0;
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(ClampToQuantum(GetPixelIntensity(image,p)));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(GetPixelRed(image,p));
          *q++=ScaleQuantumToLong(GetPixelGreen(image,p));
          *q++=ScaleQuantumToLong(GetPixelBlue(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(GetPixelRed(image,p));
          *q++=ScaleQuantumToLong(GetPixelGreen(image,p));
          *q++=ScaleQuantumToLong(GetPixelBlue(image,p));
          *q++=ScaleQuantumToLong(GetPixelAlpha(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLong(GetPixelRed(image,p));
          *q++=ScaleQuantumToLong(GetPixelGreen(image,p));
          *q++=ScaleQuantumToLong(GetPixelBlue(image,p));
          *q++=0;
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=ScaleQuantumToLong(GetPixelRed(image,p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=ScaleQuantumToLong(GetPixelGreen(image,p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=ScaleQuantumToLong(GetPixelBlue(image,p));
            break;
          }
          case AlphaQuantum:
          {
            *q=ScaleQuantumToLong(GetPixelAlpha(image,p));
            break;
          }
          case OpacityQuantum:
          {
            *q=ScaleQuantumToLong(GetPixelAlpha(image,p));
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=ScaleQuantumToLong(GetPixelBlack(image,p));
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
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

static MagickBooleanType ExportLongLongPixel(const Image *image,
  const RectangleInfo *roi,const char *magick_restrict map,
  const QuantumType *quantum_map,void *pixels,ExceptionInfo *exception)
{
  const Quantum
    *magick_restrict p;

  ssize_t
    x;

  MagickSizeType
    *magick_restrict q;

  size_t
    length;

  ssize_t
    y;

  q=(MagickSizeType *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLongLong(GetPixelBlue(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelGreen(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelRed(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLongLong(GetPixelBlue(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelGreen(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelRed(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelAlpha(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLongLong(GetPixelBlue(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelGreen(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelRed(image,p));
          *q++=0;
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLongLong(ClampToQuantum(
            GetPixelIntensity(image,p)));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLongLong(GetPixelRed(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelGreen(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelBlue(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLongLong(GetPixelRed(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelGreen(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelBlue(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelAlpha(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToLongLong(GetPixelRed(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelGreen(image,p));
          *q++=ScaleQuantumToLongLong(GetPixelBlue(image,p));
          *q++=0;
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=ScaleQuantumToLongLong(GetPixelRed(image,p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=ScaleQuantumToLongLong(GetPixelGreen(image,p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=ScaleQuantumToLongLong(GetPixelBlue(image,p));
            break;
          }
          case AlphaQuantum:
          {
            *q=ScaleQuantumToLongLong(GetPixelAlpha(image,p));
            break;
          }
          case OpacityQuantum:
          {
            *q=ScaleQuantumToLongLong(GetPixelAlpha(image,p));
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=ScaleQuantumToLongLong(GetPixelBlack(image,p));
            break;
          }
          case IndexQuantum:
          {
            *q=ScaleQuantumToLongLong(ClampToQuantum(
              GetPixelIntensity(image,p)));
            break;
          }
          default:
            break;
        }
        q++;
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

static MagickBooleanType ExportQuantumPixel(const Image *image,
  const RectangleInfo *roi,const char *magick_restrict map,
  const QuantumType *quantum_map,void *pixels,ExceptionInfo *exception)
{
  const Quantum
    *magick_restrict p;

  Quantum
    *magick_restrict q;

  ssize_t
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
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=GetPixelBlue(image,p);
          *q++=GetPixelGreen(image,p);
          *q++=GetPixelRed(image,p);
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=GetPixelBlue(image,p);
          *q++=GetPixelGreen(image,p);
          *q++=GetPixelRed(image,p);
          *q++=(Quantum) (GetPixelAlpha(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=GetPixelBlue(image,p);
          *q++=GetPixelGreen(image,p);
          *q++=GetPixelRed(image,p);
          *q++=(Quantum) 0;
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ClampToQuantum(GetPixelIntensity(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=GetPixelRed(image,p);
          *q++=GetPixelGreen(image,p);
          *q++=GetPixelBlue(image,p);
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=GetPixelRed(image,p);
          *q++=GetPixelGreen(image,p);
          *q++=GetPixelBlue(image,p);
          *q++=(Quantum) (GetPixelAlpha(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=GetPixelRed(image,p);
          *q++=GetPixelGreen(image,p);
          *q++=GetPixelBlue(image,p);
          *q++=(Quantum) 0;
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=(Quantum) 0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=GetPixelRed(image,p);
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=GetPixelGreen(image,p);
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=GetPixelBlue(image,p);
            break;
          }
          case AlphaQuantum:
          {
            *q=GetPixelAlpha(image,p);
            break;
          }
          case OpacityQuantum:
          {
            *q=GetPixelAlpha(image,p);
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=GetPixelBlack(image,p);
            break;
          }
          case IndexQuantum:
          {
            *q=ClampToQuantum(GetPixelIntensity(image,p));
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
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

static MagickBooleanType ExportShortPixel(const Image *image,
  const RectangleInfo *roi,const char *magick_restrict map,
  const QuantumType *quantum_map,void *pixels,ExceptionInfo *exception)
{
  const Quantum
    *magick_restrict p;

  ssize_t
    x;

  unsigned short
    *magick_restrict q;

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
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(GetPixelBlue(image,p));
          *q++=ScaleQuantumToShort(GetPixelGreen(image,p));
          *q++=ScaleQuantumToShort(GetPixelRed(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(GetPixelBlue(image,p));
          *q++=ScaleQuantumToShort(GetPixelGreen(image,p));
          *q++=ScaleQuantumToShort(GetPixelRed(image,p));
          *q++=ScaleQuantumToShort(GetPixelAlpha(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(GetPixelBlue(image,p));
          *q++=ScaleQuantumToShort(GetPixelGreen(image,p));
          *q++=ScaleQuantumToShort(GetPixelRed(image,p));
          *q++=0;
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(ClampToQuantum(GetPixelIntensity(image,p)));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(GetPixelRed(image,p));
          *q++=ScaleQuantumToShort(GetPixelGreen(image,p));
          *q++=ScaleQuantumToShort(GetPixelBlue(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(GetPixelRed(image,p));
          *q++=ScaleQuantumToShort(GetPixelGreen(image,p));
          *q++=ScaleQuantumToShort(GetPixelBlue(image,p));
          *q++=ScaleQuantumToShort(GetPixelAlpha(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          *q++=ScaleQuantumToShort(GetPixelRed(image,p));
          *q++=ScaleQuantumToShort(GetPixelGreen(image,p));
          *q++=ScaleQuantumToShort(GetPixelBlue(image,p));
          *q++=0;
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    p=GetVirtualPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        *q=0;
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            *q=ScaleQuantumToShort(GetPixelRed(image,p));
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            *q=ScaleQuantumToShort(GetPixelGreen(image,p));
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            *q=ScaleQuantumToShort(GetPixelBlue(image,p));
            break;
          }
          case AlphaQuantum:
          {
            *q=ScaleQuantumToShort(GetPixelAlpha(image,p));
            break;
          }
          case OpacityQuantum:
          {
            *q=ScaleQuantumToShort(GetPixelAlpha(image,p));
            break;
          }
          case BlackQuantum:
          {
            if (image->colorspace == CMYKColorspace)
              *q=ScaleQuantumToShort(GetPixelBlack(image,p));
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
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

MagickExport MagickBooleanType ExportImagePixels(const Image *image,
  const ssize_t x,const ssize_t y,const size_t width,const size_t height,
  const char *map,const StorageType type,void *pixels,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  QuantumType
    *quantum_map;

  RectangleInfo
    roi;

  ssize_t
    i;

  size_t
    length;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
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
      status=ExportCharPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case DoublePixel:
    {
      status=ExportDoublePixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case FloatPixel:
    {
      status=ExportFloatPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case LongPixel:
    {
      status=ExportLongPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case LongLongPixel:
    {
      status=ExportLongLongPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case QuantumPixel:
    {
      status=ExportQuantumPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case ShortPixel:
    {
      status=ExportShortPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    default:
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "UnrecognizedPixelMap","`%s'",map);
      status=MagickFalse;
    }
  }
  quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P i x e l I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelInfo() initializes the PixelInfo structure.
%
%  The format of the GetPixelInfo method is:
%
%      GetPixelInfo(const Image *image,PixelInfo *pixel)
%
%  A description of each parameter follows:
%
%    o image: the image. (optional - may be NULL)
%
%    o pixel: Specifies a pointer to a PixelInfo structure.
%
*/
MagickExport void GetPixelInfo(const Image *image,PixelInfo *pixel)
{
  (void) memset(pixel,0,sizeof(*pixel));
  pixel->storage_class=DirectClass;
  pixel->colorspace=sRGBColorspace;
  pixel->depth=MAGICKCORE_QUANTUM_DEPTH;
  pixel->alpha_trait=UndefinedPixelTrait;
  pixel->alpha=(double) OpaqueAlpha;
  if (image == (const Image *) NULL)
    return;
  pixel->storage_class=image->storage_class;
  pixel->colorspace=image->colorspace;
  pixel->alpha_trait=image->alpha_trait;
  pixel->depth=image->depth;
  pixel->fuzz=image->fuzz;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P i x e l I n d o I n t e n s i t y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelInfoIntensity() returns a single sample intensity value from the red,
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
%    RMS              sqrt((R^2 + G^2 + B^2) / 3.0
%    Average          (R + G + B') / 3.0
%
%  The format of the GetPixelInfoIntensity method is:
%
%      MagickRealType GetPixelInfoIntensity(const Image *image,
%        const Quantum *pixel)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o pixel: Specifies a pointer to a Quantum structure.
%
*/
MagickExport MagickRealType GetPixelInfoIntensity(
  const Image *magick_restrict image,const PixelInfo *magick_restrict pixel)
{
  MagickRealType
    blue,
    green,
    red,
    intensity;

  PixelIntensityMethod
    method;

  method=Rec709LumaPixelIntensityMethod;
  if (image != (const Image *) NULL)
    method=image->intensity;
  red=pixel->red;
  green=pixel->green;
  blue=pixel->blue;
  switch (method)
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
        (3.0*(double) QuantumRange));
      break;
    }
    case Rec601LumaPixelIntensityMethod:
    {
      if (pixel->colorspace == RGBColorspace)
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
      if (pixel->colorspace == sRGBColorspace)
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
      if (pixel->colorspace == RGBColorspace)
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
      if (pixel->colorspace == sRGBColorspace)
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
%    RMS              sqrt((R^2 + G^2 + B^2) / 3.0
%    Average          (R + G + B') / 3.0
%
%  The format of the GetPixelIntensity method is:
%
%      MagickRealType GetPixelIntensity(const Image *image,
%        const Quantum *pixel)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o pixel: Specifies a pointer to a Quantum structure.
%
*/
MagickExport MagickRealType GetPixelIntensity(
  const Image *magick_restrict image,const Quantum *magick_restrict pixel)
{
  MagickRealType
    blue,
    green,
    red,
    intensity;

  red=(MagickRealType) GetPixelRed(image,pixel);
  if (image->number_channels == 1)
    return(red);
  green=(MagickRealType) GetPixelGreen(image,pixel);
  blue=(MagickRealType) GetPixelBlue(image,pixel);
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
        (3.0*(double) QuantumRange));
      break;
    }
    case Rec601LumaPixelIntensityMethod:
    {
      if ((image->colorspace == RGBColorspace) ||
          (image->colorspace == LinearGRAYColorspace))
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
      if ((image->colorspace == sRGBColorspace) ||
          (image->colorspace == GRAYColorspace))
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
      if ((image->colorspace == RGBColorspace) ||
          (image->colorspace == LinearGRAYColorspace))
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
      if ((image->colorspace == sRGBColorspace) ||
          (image->colorspace == GRAYColorspace))
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
%  Quantum, short int, unsigned int, unsigned long long, float, or double in
%  the order specified by map.
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
%        const char *map,const StorageType type,const void *pixels,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y,width,height:  These values define the perimeter
%      of a region of pixels you want to define.
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
%      LongPixel (unsigned int *), LongLongPixel (unsigned long long *),
%      QuantumPixel (Quantum *), or ShortPixel (unsigned short *).
%
%    o pixels: This array of values contain the pixel components as defined by
%      map and type.  You must preallocate this array where the expected
%      length varies depending on the values of width, height, map, and type.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType ImportCharPixel(Image *image,const RectangleInfo *roi,
  const char *magick_restrict map,const QuantumType *quantum_map,
  const void *pixels,ExceptionInfo *exception)
{
  const unsigned char
    *magick_restrict p;

  Quantum
    *magick_restrict q;

  ssize_t
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
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
          SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
          SetPixelRed(image,ScaleCharToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
          SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
          SetPixelRed(image,ScaleCharToQuantum(*p++),q);
          SetPixelAlpha(image,ScaleCharToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRO") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
          SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
          SetPixelRed(image,ScaleCharToQuantum(*p++),q);
          SetPixelAlpha(image,ScaleCharToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
          SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
          SetPixelRed(image,ScaleCharToQuantum(*p++),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelGray(image,ScaleCharToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ScaleCharToQuantum(*p++),q);
          SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
          SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ScaleCharToQuantum(*p++),q);
          SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
          SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
          SetPixelAlpha(image,ScaleCharToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBO") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ScaleCharToQuantum(*p++),q);
          SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
          SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
          SetPixelAlpha(image,ScaleCharToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ScaleCharToQuantum(*p++),q);
          SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
          SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(image,ScaleCharToQuantum(*p),q);
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(image,ScaleCharToQuantum(*p),q);
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(image,ScaleCharToQuantum(*p),q);
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(image,ScaleCharToQuantum(*p),q);
            break;
          }
          case OpacityQuantum:
          {
            SetPixelAlpha(image,ScaleCharToQuantum(*p),q);
            break;
          }
          case BlackQuantum:
          {
            SetPixelBlack(image,ScaleCharToQuantum(*p),q);
            break;
          }
          case IndexQuantum:
          {
            SetPixelGray(image,ScaleCharToQuantum(*p),q);
            break;
          }
          default:
            break;
        }
        p++;
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

static MagickBooleanType ImportDoublePixel(Image *image,
  const RectangleInfo *roi,const char *magick_restrict map,
  const QuantumType *quantum_map,const void *pixels,ExceptionInfo *exception)
{
  const double
    *magick_restrict p;

  Quantum
    *magick_restrict q;

  ssize_t
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
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelRed(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelRed(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelAlpha(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelRed(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelGray(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelAlpha(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(*p)),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
   length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(image,ClampToQuantum((double) QuantumRange*(*p)),q);
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(*p)),q);
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(*p)),q);
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(image,ClampToQuantum((double) QuantumRange*(*p)),q);
            break;
          }
          case OpacityQuantum:
          {
            SetPixelAlpha(image,ClampToQuantum((double) QuantumRange*(*p)),q);
            break;
          }
          case BlackQuantum:
          {
            SetPixelBlack(image,ClampToQuantum((double) QuantumRange*(*p)),q);
            break;
          }
          case IndexQuantum:
          {
            SetPixelGray(image,ClampToQuantum((double) QuantumRange*(*p)),q);
            break;
          }
          default:
            break;
        }
        p++;
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

static MagickBooleanType ImportFloatPixel(Image *image,const RectangleInfo *roi,
  const char *magick_restrict map,const QuantumType *quantum_map,
  const void *pixels,ExceptionInfo *exception)
{
  const float
    *magick_restrict p;

  Quantum
    *magick_restrict q;

  ssize_t
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
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelRed(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelRed(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelAlpha(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelRed(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelGray(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelAlpha(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
            break;
          }
          case OpacityQuantum:
          {
            SetPixelAlpha(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
            break;
          }
          case BlackQuantum:
          {
            SetPixelBlack(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
            break;
          }
          case IndexQuantum:
          {
            SetPixelGray(image,ClampToQuantum((double) QuantumRange*(double)
            (*p)),q);
            break;
          }
          default:
            break;
        }
        p++;
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

static MagickBooleanType ImportLongPixel(Image *image,const RectangleInfo *roi,
  const char *magick_restrict map,const QuantumType *quantum_map,
  const void *pixels,ExceptionInfo *exception)
{
  const unsigned int
    *magick_restrict p;

  Quantum
    *magick_restrict q;

  ssize_t
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
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
          SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
          SetPixelRed(image,ScaleLongToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
          SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
          SetPixelRed(image,ScaleLongToQuantum(*p++),q);
          SetPixelAlpha(image,ScaleLongToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
          SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
          SetPixelRed(image,ScaleLongToQuantum(*p++),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelGray(image,ScaleLongToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ScaleLongToQuantum(*p++),q);
          SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
          SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ScaleLongToQuantum(*p++),q);
          SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
          SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
          SetPixelAlpha(image,ScaleLongToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ScaleLongToQuantum(*p++),q);
          SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
          SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(image,ScaleLongToQuantum(*p),q);
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(image,ScaleLongToQuantum(*p),q);
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(image,ScaleLongToQuantum(*p),q);
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(image,ScaleLongToQuantum(*p),q);
            break;
          }
          case OpacityQuantum:
          {
            SetPixelAlpha(image,ScaleLongToQuantum(*p),q);
            break;
          }
          case BlackQuantum:
          {
            SetPixelBlack(image,ScaleLongToQuantum(*p),q);
            break;
          }
          case IndexQuantum:
          {
            SetPixelGray(image,ScaleLongToQuantum(*p),q);
            break;
          }
          default:
            break;
        }
        p++;
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

static MagickBooleanType ImportLongLongPixel(Image *image,
  const RectangleInfo *roi,const char *magick_restrict map,
  const QuantumType *quantum_map,const void *pixels,ExceptionInfo *exception)
{
  const MagickSizeType
    *magick_restrict p;

  Quantum
    *magick_restrict q;

  ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  p=(const MagickSizeType *) pixels;
  if (LocaleCompare(map,"BGR") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelGreen(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelRed(image,ScaleLongLongToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelGreen(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelRed(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelAlpha(image,ScaleLongLongToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelGreen(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelRed(image,ScaleLongLongToQuantum(*p++),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelGray(image,ScaleLongLongToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelGreen(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelBlue(image,ScaleLongLongToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelGreen(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelBlue(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelAlpha(image,ScaleLongLongToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelGreen(image,ScaleLongLongToQuantum(*p++),q);
          SetPixelBlue(image,ScaleLongLongToQuantum(*p++),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(image,ScaleLongLongToQuantum(*p),q);
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(image,ScaleLongLongToQuantum(*p),q);
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(image,ScaleLongLongToQuantum(*p),q);
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(image,ScaleLongLongToQuantum(*p),q);
            break;
          }
          case OpacityQuantum:
          {
            SetPixelAlpha(image,ScaleLongLongToQuantum(*p),q);
            break;
          }
          case BlackQuantum:
          {
            SetPixelBlack(image,ScaleLongLongToQuantum(*p),q);
            break;
          }
          case IndexQuantum:
          {
            SetPixelGray(image,ScaleLongLongToQuantum(*p),q);
            break;
          }
          default:
            break;
        }
        p++;
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

static MagickBooleanType ImportQuantumPixel(Image *image,
  const RectangleInfo *roi,const char *magick_restrict map,
  const QuantumType *quantum_map,const void *pixels,ExceptionInfo *exception)
{
  const Quantum
    *magick_restrict p;

  Quantum
    *magick_restrict q;

  ssize_t
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
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,*p++,q);
          SetPixelGreen(image,*p++,q);
          SetPixelRed(image,*p++,q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,*p++,q);
          SetPixelGreen(image,*p++,q);
          SetPixelRed(image,*p++,q);
          SetPixelAlpha(image,*p++,q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,*p++,q);
          SetPixelGreen(image,*p++,q);
          SetPixelRed(image,*p++,q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelGray(image,*p++,q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,*p++,q);
          SetPixelGreen(image,*p++,q);
          SetPixelBlue(image,*p++,q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,*p++,q);
          SetPixelGreen(image,*p++,q);
          SetPixelBlue(image,*p++,q);
          SetPixelAlpha(image,*p++,q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,*p++,q);
          SetPixelGreen(image,*p++,q);
          SetPixelBlue(image,*p++,q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(image,*p,q);
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(image,*p,q);
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(image,*p,q);
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(image,*p,q);
            break;
          }
          case OpacityQuantum:
          {
            SetPixelAlpha(image,*p,q);
            break;
          }
          case BlackQuantum:
          {
            SetPixelBlack(image,*p,q);
            break;
          }
          case IndexQuantum:
          {
            SetPixelGray(image,*p,q);
            break;
          }
          default:
            break;
        }
        p++;
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

static MagickBooleanType ImportShortPixel(Image *image,const RectangleInfo *roi,
  const char *magick_restrict map,const QuantumType *quantum_map,
  const void *pixels,ExceptionInfo *exception)
{
  const unsigned short
    *magick_restrict p;

  Quantum
    *magick_restrict q;

  ssize_t
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
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
          SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
          SetPixelRed(image,ScaleShortToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
          SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
          SetPixelRed(image,ScaleShortToQuantum(*p++),q);
          SetPixelAlpha(image,ScaleShortToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"BGRP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
          SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
          SetPixelRed(image,ScaleShortToQuantum(*p++),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"I") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelGray(image,ScaleShortToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGB") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ScaleShortToQuantum(*p++),q);
          SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
          SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBA") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ScaleShortToQuantum(*p++),q);
          SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
          SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
          SetPixelAlpha(image,ScaleShortToQuantum(*p++),q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  if (LocaleCompare(map,"RGBP") == 0)
    {
      for (y=0; y < (ssize_t) roi->height; y++)
      {
        q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) roi->width; x++)
        {
          SetPixelRed(image,ScaleShortToQuantum(*p++),q);
          SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
          SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
          p++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
    }
  length=strlen(map);
  for (y=0; y < (ssize_t) roi->height; y++)
  {
    q=GetAuthenticPixels(image,roi->x,roi->y+y,roi->width,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) roi->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) length; i++)
      {
        switch (quantum_map[i])
        {
          case RedQuantum:
          case CyanQuantum:
          {
            SetPixelRed(image,ScaleShortToQuantum(*p),q);
            break;
          }
          case GreenQuantum:
          case MagentaQuantum:
          {
            SetPixelGreen(image,ScaleShortToQuantum(*p),q);
            break;
          }
          case BlueQuantum:
          case YellowQuantum:
          {
            SetPixelBlue(image,ScaleShortToQuantum(*p),q);
            break;
          }
          case AlphaQuantum:
          {
            SetPixelAlpha(image,ScaleShortToQuantum(*p),q);
            break;
          }
          case OpacityQuantum:
          {
            SetPixelAlpha(image,ScaleShortToQuantum(*p),q);
            break;
          }
          case BlackQuantum:
          {
            SetPixelBlack(image,ScaleShortToQuantum(*p),q);
            break;
          }
          case IndexQuantum:
          {
            SetPixelGray(image,ScaleShortToQuantum(*p),q);
            break;
          }
          default:
            break;
        }
        p++;
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  return(y < (ssize_t) roi->height ? MagickFalse : MagickTrue);
}

MagickExport MagickBooleanType ImportImagePixels(Image *image,const ssize_t x,
  const ssize_t y,const size_t width,const size_t height,const char *map,
  const StorageType type,const void *pixels,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  QuantumType
    *quantum_map;

  RectangleInfo
    roi;

  ssize_t
    i;

  size_t
    length;

  /*
    Allocate image structure.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
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
        image->alpha_trait=BlendPixelTrait;
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
        (void) SetImageColorspace(image,CMYKColorspace,exception);
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
        (void) SetImageColorspace(image,CMYKColorspace,exception);
        break;
      }
      case 'I':
      case 'i':
      {
        quantum_map[i]=IndexQuantum;
        (void) SetImageColorspace(image,GRAYColorspace,exception);
        break;
      }
      case 'm':
      case 'M':
      {
        quantum_map[i]=MagentaQuantum;
        (void) SetImageColorspace(image,CMYKColorspace,exception);
        break;
      }
      case 'O':
      case 'o':
      {
        quantum_map[i]=OpacityQuantum;
        image->alpha_trait=BlendPixelTrait;
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
        (void) SetImageColorspace(image,CMYKColorspace,exception);
        break;
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
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  /*
    Transfer the pixels from the pixel data to the image.
  */
  roi.width=width;
  roi.height=height;
  roi.x=x;
  roi.y=y;
  switch (type)
  {
    case CharPixel:
    {
      status=ImportCharPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case DoublePixel:
    {
      status=ImportDoublePixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case FloatPixel:
    {
      status=ImportFloatPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case LongPixel:
    {
      status=ImportLongPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case LongLongPixel:
    {
      status=ImportLongLongPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case QuantumPixel:
    {
      status=ImportQuantumPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    case ShortPixel:
    {
      status=ImportShortPixel(image,&roi,map,quantum_map,pixels,exception);
      break;
    }
    default:
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "UnrecognizedStorageType","`%d'",type);
      status=MagickFalse;
    }
  }
  quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n t e r p o l a t e P i x e l C h a n n e l                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpolatePixelChannel() applies a pixel interpolation method between a
%  floating point coordinate and the pixels surrounding that coordinate.  No
%  pixel area resampling, or scaling of the result is performed.
%
%  Interpolation is restricted to just the specified channel.
%
%  The format of the InterpolatePixelChannel method is:
%
%      MagickBooleanType InterpolatePixelChannel(
%        const Image *magick_restrict image,const CacheView *image_view,
%        const PixelChannel channel,const PixelInterpolateMethod method,
%        const double x,const double y,double *pixel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o image_view: the image view.
%
%    o channel: the pixel channel to interpolate.
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

static inline void CatromWeights(const double x,double (*weights)[4])
{
  double
    alpha,
    beta,
    gamma;

  /*
    Nicolas Robidoux' 10 flops (4* + 5- + 1+) refactoring of the computation
    of the standard four 1D Catmull-Rom weights. The sampling location is
    assumed between the second and third input pixel locations, and x is the
    position relative to the second input pixel location. Formulas originally
    derived for the VIPS (Virtual Image Processing System) library.
  */
  alpha=(double) 1.0-x;
  beta=(double) (-0.5)*x*alpha;
  (*weights)[0]=alpha*beta;
  (*weights)[3]=x*beta;
  /*
    The following computation of the inner weights from the outer ones work
    for all Keys cubics.
  */
  gamma=(*weights)[3]-(*weights)[0];
  (*weights)[1]=alpha-(*weights)[0]+gamma;
  (*weights)[2]=x-(*weights)[3]-gamma;
}

static inline void SplineWeights(const double x,double (*weights)[4])
{
  double
    alpha,
    beta;

  /*
    Nicolas Robidoux' 12 flops (6* + 5- + 1+) refactoring of the computation
    of the standard four 1D cubic B-spline smoothing weights. The sampling
    location is assumed between the second and third input pixel locations,
    and x is the position relative to the second input pixel location.
  */
  alpha=(double) 1.0-x;
  (*weights)[3]=(double) (1.0/6.0)*x*x*x;
  (*weights)[0]=(double) (1.0/6.0)*alpha*alpha*alpha;
  beta=(*weights)[3]-(*weights)[0];
  (*weights)[1]=alpha-(*weights)[0]+beta;
  (*weights)[2]=x-(*weights)[3]-beta;
}

static inline double MeshInterpolate(const PointInfo *delta,const double p,
  const double x,const double y)
{
  return(delta->x*x+delta->y*y+(1.0-delta->x-delta->y)*p);
}

MagickExport MagickBooleanType InterpolatePixelChannel(
  const Image *magick_restrict image,const CacheView_ *image_view,
  const PixelChannel channel,const PixelInterpolateMethod method,const double x,
  const double y,double *pixel,ExceptionInfo *exception)
{
  const Quantum
    *magick_restrict p;

  double
    alpha[16],
    gamma,
    pixels[16];

  MagickBooleanType
    status;

  PixelInterpolateMethod
    interpolate;

  PixelTrait
    traits;

  ssize_t
    i,
    x_offset,
    y_offset;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image_view != (CacheView *) NULL);
  *pixel=0.0;
  if ((channel < 0) || (channel >= MaxPixelChannels))
    ThrowBinaryException(OptionError,"NoSuchImageChannel",image->filename);
  traits=GetPixelChannelTraits(image,channel);
  x_offset=CastDoubleToSsizeT(floor(x));
  y_offset=CastDoubleToSsizeT(floor(y));
  interpolate=method;
  if (interpolate == UndefinedInterpolatePixel)
    interpolate=image->interpolate;
  status=MagickTrue;
  switch (interpolate)
  {
    case AverageInterpolatePixel:  /* nearest 4 neighbours */
    case Average9InterpolatePixel:  /* nearest 9 neighbours */
    case Average16InterpolatePixel:  /* nearest 16 neighbours */
    {
      ssize_t
        count;

      count=2;  /* size of the area to average - default nearest 4 */
      if (interpolate == Average9InterpolatePixel)
        {
          count=3;
          x_offset=CastDoubleToSsizeT(floor(x+0.5)-1.0);
          y_offset=CastDoubleToSsizeT(floor(y+0.5)-1.0);
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
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      count*=count;  /* Number of pixels to average */
      if ((traits & BlendPixelTrait) == 0)
        for (i=0; i < (ssize_t) count; i++)
        {
          alpha[i]=1.0;
          pixels[i]=(double) p[i*(ssize_t) GetPixelChannels(image)+
            GetPixelChannelOffset(image,channel)];
        }
      else
        for (i=0; i < (ssize_t) count; i++)
        {
          alpha[i]=QuantumScale*(double) GetPixelAlpha(image,p+i*
            (ssize_t) GetPixelChannels(image));
          pixels[i]=alpha[i]*(double) p[i*(ssize_t) GetPixelChannels(image)+
            GetPixelChannelOffset(image,channel)];
        }
      for (i=0; i < (ssize_t) count; i++)
      {
        gamma=MagickSafeReciprocal(alpha[i])/count;
        *pixel+=gamma*pixels[i];
      }
      break;
    }
    case BilinearInterpolatePixel:
    default:
    {
      PointInfo
        delta,
        epsilon;

      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,2,2,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      if ((traits & BlendPixelTrait) == 0)
        for (i=0; i < 4; i++)
        {
          alpha[i]=1.0;
          pixels[i]=(double) p[i*(ssize_t) GetPixelChannels(image)+
            GetPixelChannelOffset(image,channel)];
        }
      else
        for (i=0; i < 4; i++)
        {
          alpha[i]=QuantumScale*(double) GetPixelAlpha(image,p+i*
            (ssize_t) GetPixelChannels(image));
          pixels[i]=alpha[i]*(double) p[i*(ssize_t) GetPixelChannels(image)+
            GetPixelChannelOffset(image,channel)];
        }
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      epsilon.x=1.0-delta.x;
      epsilon.y=1.0-delta.y;
      gamma=((epsilon.y*(epsilon.x*alpha[0]+delta.x*alpha[1])+delta.y*
        (epsilon.x*alpha[2]+delta.x*alpha[3])));
      gamma=MagickSafeReciprocal(gamma);
      *pixel=gamma*(epsilon.y*(epsilon.x*pixels[0]+delta.x*pixels[1])+delta.y*
        (epsilon.x*pixels[2]+delta.x*pixels[3]));
      break;
    }
    case BlendInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,2,2,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      if ((traits & BlendPixelTrait) == 0)
        for (i=0; i < 4; i++)
        {
          alpha[i]=1.0;
          pixels[i]=(MagickRealType) p[i*(ssize_t) GetPixelChannels(image)+
            GetPixelChannelOffset(image,channel)];
        }
      else
        for (i=0; i < 4; i++)
        {
          alpha[i]=QuantumScale*(double) GetPixelAlpha(image,p+i*
            (ssize_t) GetPixelChannels(image));
          pixels[i]=alpha[i]*(double) p[i*(ssize_t) GetPixelChannels(image)+
            GetPixelChannelOffset(image,channel)];
        }
      gamma=1.0;  /* number of pixels blended together (its variable) */
      for (i=0; i <= 1L; i++) {
        if ((y-y_offset) >= 0.75)
          {
            alpha[i]=alpha[i+2];  /* take right pixels */
            pixels[i]=pixels[i+2];
          }
        else
          if ((y-y_offset) > 0.25)
            {
              gamma=2.0;  /* blend both pixels in row */
              alpha[i]+=alpha[i+2];  /* add up alpha weights */
              pixels[i]+=pixels[i+2];
            }
      }
      if ((x-x_offset) >= 0.75)
        {
          alpha[0]=alpha[1];  /* take bottom row blend */
          pixels[0]=pixels[1];
        }
      else
        if ((x-x_offset) > 0.25)
          {
            gamma*=2.0;  /* blend both rows */
            alpha[0]+=alpha[1];  /* add up alpha weights */
            pixels[0]+=pixels[1];
          }
      if (channel != AlphaPixelChannel)
        gamma=MagickSafeReciprocal(alpha[0]);  /* (color) 1/alpha_weights */
      else
        gamma=MagickSafeReciprocal(gamma);  /* (alpha) 1/number_of_pixels */
      *pixel=gamma*pixels[0];
      break;
    }
    case CatromInterpolatePixel:
    {
      double
        cx[4],
        cy[4];

      p=GetCacheViewVirtualPixels(image_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      if ((traits & BlendPixelTrait) == 0)
        for (i=0; i < 16; i++)
        {
          alpha[i]=1.0;
          pixels[i]=(double) p[i*(ssize_t) GetPixelChannels(image)+
            GetPixelChannelOffset(image,channel)];
        }
      else
        for (i=0; i < 16; i++)
        {
          alpha[i]=QuantumScale*(double) GetPixelAlpha(image,p+i*
            (ssize_t) GetPixelChannels(image));
          pixels[i]=alpha[i]*(double) p[i*(ssize_t) GetPixelChannels(image)+
            GetPixelChannelOffset(image,channel)];
        }
      CatromWeights((double) (x-x_offset),&cx);
      CatromWeights((double) (y-y_offset),&cy);
      gamma=(channel == AlphaPixelChannel ? (double) 1.0 :
        MagickSafeReciprocal(cy[0]*(cx[0]*alpha[0]+cx[1]*alpha[1]+cx[2]*
        alpha[2]+cx[3]*alpha[3])+cy[1]*(cx[0]*alpha[4]+cx[1]*alpha[5]+cx[2]*
        alpha[6]+cx[3]*alpha[7])+cy[2]*(cx[0]*alpha[8]+cx[1]*alpha[9]+cx[2]*
        alpha[10]+cx[3]*alpha[11])+cy[3]*(cx[0]*alpha[12]+cx[1]*alpha[13]+
        cx[2]*alpha[14]+cx[3]*alpha[15])));
      *pixel=gamma*(cy[0]*(cx[0]*pixels[0]+cx[1]*pixels[1]+cx[2]*pixels[2]+
        cx[3]*pixels[3])+cy[1]*(cx[0]*pixels[4]+cx[1]*pixels[5]+cx[2]*
        pixels[6]+cx[3]*pixels[7])+cy[2]*(cx[0]*pixels[8]+cx[1]*pixels[9]+
        cx[2]*pixels[10]+cx[3]*pixels[11])+cy[3]*(cx[0]*pixels[12]+cx[1]*
        pixels[13]+cx[2]*pixels[14]+cx[3]*pixels[15]));
      break;
    }
    case IntegerInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,1,1,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      *pixel=(double) GetPixelChannel(image,channel,p);
      break;
    }
    case NearestInterpolatePixel:
    {
      x_offset=CastDoubleToSsizeT(floor(x+0.5));
      y_offset=CastDoubleToSsizeT(floor(y+0.5));
      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,1,1,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      *pixel=(double) GetPixelChannel(image,channel,p);
      break;
    }
    case MeshInterpolatePixel:
    {
      PointInfo
        delta,
        luminance;

      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,2,2,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      if ((traits & BlendPixelTrait) == 0)
        for (i=0; i < 4; i++)
        {
          alpha[i]=1.0;
          pixels[i]=(double) p[i*(ssize_t) GetPixelChannels(image)+
            GetPixelChannelOffset(image,channel)];
        }
      else
        for (i=0; i < 4; i++)
        {
          alpha[i]=QuantumScale*(double) GetPixelAlpha(image,p+i*
            (ssize_t) GetPixelChannels(image));
          pixels[i]=alpha[i]*(double) p[i*(ssize_t) GetPixelChannels(image)+
            GetPixelChannelOffset(image,channel)];
        }
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      luminance.x=GetPixelLuma(image,p)-(double)
        GetPixelLuma(image,p+3*GetPixelChannels(image));
      luminance.y=GetPixelLuma(image,p+GetPixelChannels(image))-(double)
        GetPixelLuma(image,p+2*GetPixelChannels(image));
      if (fabs((double) luminance.x) < fabs((double) luminance.y))
        {
          /*
            Diagonal 0-3 NW-SE.
          */
          if (delta.x <= delta.y)
            {
              /*
                Bottom-left triangle (pixel: 2, diagonal: 0-3).
              */
              delta.y=1.0-delta.y;
              gamma=MeshInterpolate(&delta,alpha[2],alpha[3],alpha[0]);
              gamma=MagickSafeReciprocal(gamma);
              *pixel=gamma*MeshInterpolate(&delta,pixels[2],pixels[3],
                pixels[0]);
            }
          else
            {
              /*
                Top-right triangle (pixel: 1, diagonal: 0-3).
              */
              delta.x=1.0-delta.x;
              gamma=MeshInterpolate(&delta,alpha[1],alpha[0],alpha[3]);
              gamma=MagickSafeReciprocal(gamma);
              *pixel=gamma*MeshInterpolate(&delta,pixels[1],pixels[0],
                pixels[3]);
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
              gamma=MagickSafeReciprocal(gamma);
              *pixel=gamma*MeshInterpolate(&delta,pixels[0],pixels[1],
                pixels[2]);
            }
          else
            {
              /*
                Bottom-right triangle (pixel: 3, diagonal: 1-2).
              */
              delta.x=1.0-delta.x;
              delta.y=1.0-delta.y;
              gamma=MeshInterpolate(&delta,alpha[3],alpha[2],alpha[1]);
              gamma=MagickSafeReciprocal(gamma);
              *pixel=gamma*MeshInterpolate(&delta,pixels[3],pixels[2],
                pixels[1]);
            }
        }
      break;
    }
    case SplineInterpolatePixel:
    {
      double
        cx[4],
        cy[4];

      p=GetCacheViewVirtualPixels(image_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      if ((traits & BlendPixelTrait) == 0)
        for (i=0; i < 16; i++)
        {
          alpha[i]=1.0;
          pixels[i]=(double) p[i*(ssize_t) GetPixelChannels(image)+
            GetPixelChannelOffset(image,channel)];
        }
      else
        for (i=0; i < 16; i++)
        {
          alpha[i]=QuantumScale*(double) GetPixelAlpha(image,p+i*
            (ssize_t) GetPixelChannels(image));
          pixels[i]=alpha[i]*(double) p[i*(ssize_t) GetPixelChannels(image)+
            GetPixelChannelOffset(image,channel)];
        }
      SplineWeights((double) (x-x_offset),&cx);
      SplineWeights((double) (y-y_offset),&cy);
      gamma=(channel == AlphaPixelChannel ? (double) 1.0 :
        MagickSafeReciprocal(cy[0]*(cx[0]*alpha[0]+cx[1]*alpha[1]+cx[2]*
        alpha[2]+cx[3]*alpha[3])+cy[1]*(cx[0]*alpha[4]+cx[1]*alpha[5]+cx[2]*
        alpha[6]+cx[3]*alpha[7])+cy[2]*(cx[0]*alpha[8]+cx[1]*alpha[9]+cx[2]*
        alpha[10]+cx[3]*alpha[11])+cy[3]*(cx[0]*alpha[12]+cx[1]*alpha[13]+
        cx[2]*alpha[14]+cx[3]*alpha[15])));
      *pixel=gamma*(cy[0]*(cx[0]*pixels[0]+cx[1]*pixels[1]+cx[2]*pixels[2]+
        cx[3]*pixels[3])+cy[1]*(cx[0]*pixels[4]+cx[1]*pixels[5]+cx[2]*
        pixels[6]+cx[3]*pixels[7])+cy[2]*(cx[0]*pixels[8]+cx[1]*pixels[9]+
        cx[2]*pixels[10]+cx[3]*pixels[11])+cy[3]*(cx[0]*pixels[12]+cx[1]*
        pixels[13]+cx[2]*pixels[14]+cx[3]*pixels[15]));
      break;
    }
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n t e r p o l a t e P i x e l C h a n n e l s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpolatePixelChannels() applies a pixel interpolation method between a
%  floating point coordinate and the pixels surrounding that coordinate.  No
%  pixel area resampling, or scaling of the result is performed.
%
%  Interpolation is restricted to just the current channel setting of the
%  destination image into which the color is to be stored
%
%  The format of the InterpolatePixelChannels method is:
%
%      MagickBooleanType InterpolatePixelChannels(
%        const Image *magick_restrict source,const CacheView *source_view,
%        const Image *magick_restrict destination,
%        const PixelInterpolateMethod method,const double x,const double y,
%        Quantum *pixel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o source: the source.
%
%    o source_view: the source view.
%
%    o destination: the destination image, for the interpolated color
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
MagickExport MagickBooleanType InterpolatePixelChannels(
  const Image *magick_restrict source,const CacheView_ *source_view,
  const Image *magick_restrict destination,const PixelInterpolateMethod method,
  const double x,const double y,Quantum *pixel,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  double
    alpha[16],
    gamma,
    pixels[16];

  const Quantum
    *magick_restrict p;

  ssize_t
    i;

  ssize_t
    x_offset,
    y_offset;

  PixelInterpolateMethod
    interpolate;

  assert(source != (Image *) NULL);
  assert(source->signature == MagickCoreSignature);
  assert(source_view != (CacheView *) NULL);
  status=MagickTrue;
  x_offset=CastDoubleToSsizeT(floor(x));
  y_offset=CastDoubleToSsizeT(floor(y));
  interpolate=method;
  if (interpolate == UndefinedInterpolatePixel)
    interpolate=source->interpolate;
  switch (interpolate)
  {
    case AverageInterpolatePixel:  /* nearest 4 neighbours */
    case Average9InterpolatePixel:  /* nearest 9 neighbours */
    case Average16InterpolatePixel:  /* nearest 16 neighbours */
    {
      ssize_t
        count;

      count=2;  /* size of the area to average - default nearest 4 */
      if (interpolate == Average9InterpolatePixel)
        {
          count=3;
          x_offset=CastDoubleToSsizeT(floor(x+0.5)-1.0);
          y_offset=CastDoubleToSsizeT(floor(y+0.5)-1.0);
        }
      else
        if (interpolate == Average16InterpolatePixel)
          {
            count=4;
            x_offset--;
            y_offset--;
          }
      p=GetCacheViewVirtualPixels(source_view,x_offset,y_offset,(size_t) count,
        (size_t) count,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      count*=count;  /* Number of pixels to average */
      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        double
          sum;

        ssize_t
          j;

        PixelChannel channel = GetPixelChannelChannel(source,i);
        PixelTrait traits = GetPixelChannelTraits(source,channel);
        PixelTrait destination_traits=GetPixelChannelTraits(destination,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        for (j=0; j < (ssize_t) count; j++)
          pixels[j]=(double) p[j*(ssize_t) GetPixelChannels(source)+i];
        sum=0.0;
        if ((traits & BlendPixelTrait) == 0)
          {
            for (j=0; j < (ssize_t) count; j++)
              sum+=pixels[j];
            sum/=count;
            SetPixelChannel(destination,channel,ClampToQuantum(sum),pixel);
            continue;
          }
        for (j=0; j < (ssize_t) count; j++)
        {
          alpha[j]=QuantumScale*(double) GetPixelAlpha(source,p+j*
            (ssize_t) GetPixelChannels(source));
          pixels[j]*=alpha[j];
          gamma=MagickSafeReciprocal(alpha[j]);
          sum+=gamma*pixels[j];
        }
        sum/=count;
        SetPixelChannel(destination,channel,ClampToQuantum(sum),pixel);
      }
      break;
    }
    case BilinearInterpolatePixel:
    default:
    {
      p=GetCacheViewVirtualPixels(source_view,x_offset,y_offset,2,2,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        PointInfo
          delta,
          epsilon;

        PixelChannel channel = GetPixelChannelChannel(source,i);
        PixelTrait traits = GetPixelChannelTraits(source,channel);
        PixelTrait destination_traits=GetPixelChannelTraits(destination,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        delta.x=x-x_offset;
        delta.y=y-y_offset;
        epsilon.x=1.0-delta.x;
        epsilon.y=1.0-delta.y;
        pixels[0]=(double) p[i];
        pixels[1]=(double) p[(ssize_t) GetPixelChannels(source)+i];
        pixels[2]=(double) p[2*(ssize_t) GetPixelChannels(source)+i];
        pixels[3]=(double) p[3*(ssize_t) GetPixelChannels(source)+i];
        if ((traits & BlendPixelTrait) == 0)
          {
            gamma=((epsilon.y*(epsilon.x+delta.x)+delta.y*(epsilon.x+delta.x)));
            gamma=MagickSafeReciprocal(gamma);
            SetPixelChannel(destination,channel,ClampToQuantum(gamma*(epsilon.y*
              (epsilon.x*pixels[0]+delta.x*pixels[1])+delta.y*(epsilon.x*
              pixels[2]+delta.x*pixels[3]))),pixel);
            continue;
          }
        alpha[0]=QuantumScale*(double) GetPixelAlpha(source,p);
        alpha[1]=QuantumScale*(double) GetPixelAlpha(source,p+
          GetPixelChannels(source));
        alpha[2]=QuantumScale*(double) GetPixelAlpha(source,p+2*
          GetPixelChannels(source));
        alpha[3]=QuantumScale*(double) GetPixelAlpha(source,p+3*
          GetPixelChannels(source));
        pixels[0]*=alpha[0];
        pixels[1]*=alpha[1];
        pixels[2]*=alpha[2];
        pixels[3]*=alpha[3];
        gamma=((epsilon.y*(epsilon.x*alpha[0]+delta.x*alpha[1])+delta.y*
          (epsilon.x*alpha[2]+delta.x*alpha[3])));
        gamma=MagickSafeReciprocal(gamma);
        SetPixelChannel(destination,channel,ClampToQuantum(gamma*(epsilon.y*
          (epsilon.x*pixels[0]+delta.x*pixels[1])+delta.y*(epsilon.x*pixels[2]+
          delta.x*pixels[3]))),pixel);
      }
      break;
    }
    case BlendInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(source_view,x_offset,y_offset,2,2,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        ssize_t
          j;

        PixelChannel channel = GetPixelChannelChannel(source,i);
        PixelTrait traits = GetPixelChannelTraits(source,channel);
        PixelTrait destination_traits=GetPixelChannelTraits(destination,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        if (source->alpha_trait != BlendPixelTrait)
          for (j=0; j < 4; j++)
          {
            alpha[j]=1.0;
            pixels[j]=(double) p[j*(ssize_t) GetPixelChannels(source)+i];
          }
        else
          for (j=0; j < 4; j++)
          {
            alpha[j]=QuantumScale*(double) GetPixelAlpha(source,p+j*
              (ssize_t) GetPixelChannels(source));
            pixels[j]=(double) p[j*(ssize_t) GetPixelChannels(source)+i];
            if (channel != AlphaPixelChannel)
              pixels[j]*=alpha[j];
          }
        gamma=1.0;  /* number of pixels blended together (its variable) */
        for (j=0; j <= 1L; j++)
        {
          if ((y-y_offset) >= 0.75)
            {
              alpha[j]=alpha[j+2];  /* take right pixels */
              pixels[j]=pixels[j+2];
            }
          else
            if ((y-y_offset) > 0.25)
              {
                gamma=2.0;  /* blend both pixels in row */
                alpha[j]+=alpha[j+2];  /* add up alpha weights */
                pixels[j]+=pixels[j+2];
              }
        }
        if ((x-x_offset) >= 0.75)
          {
            alpha[0]=alpha[1];  /* take bottom row blend */
            pixels[0]=pixels[1];
          }
        else
           if ((x-x_offset) > 0.25)
             {
               gamma*=2.0;  /* blend both rows */
               alpha[0]+=alpha[1];  /* add up alpha weights */
               pixels[0]+=pixels[1];
             }
        if (channel != AlphaPixelChannel)
          gamma=MagickSafeReciprocal(alpha[0]);  /* (color) 1/alpha_weights */
        else
          gamma=MagickSafeReciprocal(gamma);  /* (alpha) 1/number_of_pixels */
        SetPixelChannel(destination,channel,ClampToQuantum(gamma*pixels[0]),
          pixel);
      }
      break;
    }
    case CatromInterpolatePixel:
    {
      double
        cx[4],
        cy[4];

      p=GetCacheViewVirtualPixels(source_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        ssize_t
          j;

        PixelChannel channel = GetPixelChannelChannel(source,i);
        PixelTrait traits = GetPixelChannelTraits(source,channel);
        PixelTrait destination_traits=GetPixelChannelTraits(destination,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        if ((traits & BlendPixelTrait) == 0)
          for (j=0; j < 16; j++)
          {
            alpha[j]=1.0;
            pixels[j]=(double) p[j*(ssize_t) GetPixelChannels(source)+i];
          }
        else
          for (j=0; j < 16; j++)
          {
            alpha[j]=QuantumScale*(double) GetPixelAlpha(source,p+j*
              (ssize_t) GetPixelChannels(source));
            pixels[j]=alpha[j]*(double)
              p[j*(ssize_t) GetPixelChannels(source)+i];
          }
        CatromWeights((double) (x-x_offset),&cx);
        CatromWeights((double) (y-y_offset),&cy);
        gamma=((traits & BlendPixelTrait) ? (double) (1.0) :
          MagickSafeReciprocal(cy[0]*(cx[0]*alpha[0]+cx[1]*alpha[1]+cx[2]*
          alpha[2]+cx[3]*alpha[3])+cy[1]*(cx[0]*alpha[4]+cx[1]*alpha[5]+cx[2]*
          alpha[6]+cx[3]*alpha[7])+cy[2]*(cx[0]*alpha[8]+cx[1]*alpha[9]+cx[2]*
          alpha[10]+cx[3]*alpha[11])+cy[3]*(cx[0]*alpha[12]+cx[1]*alpha[13]+
          cx[2]*alpha[14]+cx[3]*alpha[15])));
        SetPixelChannel(destination,channel,ClampToQuantum(gamma*(cy[0]*(cx[0]*
          pixels[0]+cx[1]*pixels[1]+cx[2]*pixels[2]+cx[3]*pixels[3])+cy[1]*
          (cx[0]*pixels[4]+cx[1]*pixels[5]+cx[2]*pixels[6]+cx[3]*pixels[7])+
          cy[2]*(cx[0]*pixels[8]+cx[1]*pixels[9]+cx[2]*pixels[10]+cx[3]*
          pixels[11])+cy[3]*(cx[0]*pixels[12]+cx[1]*pixels[13]+cx[2]*
          pixels[14]+cx[3]*pixels[15]))),pixel);
      }
      break;
    }
    case IntegerInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(source_view,x_offset,y_offset,1,1,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(source,i);
        PixelTrait traits = GetPixelChannelTraits(source,channel);
        PixelTrait destination_traits=GetPixelChannelTraits(destination,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(destination,channel,p[i],pixel);
      }
      break;
    }
    case NearestInterpolatePixel:
    {
      x_offset=CastDoubleToSsizeT(floor(x+0.5));
      y_offset=CastDoubleToSsizeT(floor(y+0.5));
      p=GetCacheViewVirtualPixels(source_view,x_offset,y_offset,1,1,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(source,i);
        PixelTrait traits = GetPixelChannelTraits(source,channel);
        PixelTrait destination_traits=GetPixelChannelTraits(destination,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(destination,channel,p[i],pixel);
      }
      break;
    }
    case MeshInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(source_view,x_offset,y_offset,2,2,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        PointInfo
          delta,
          luminance;

        PixelChannel channel = GetPixelChannelChannel(source,i);
        PixelTrait traits = GetPixelChannelTraits(source,channel);
        PixelTrait destination_traits=GetPixelChannelTraits(destination,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        pixels[0]=(double) p[i];
        pixels[1]=(double) p[(ssize_t) GetPixelChannels(source)+i];
        pixels[2]=(double) p[2*(ssize_t) GetPixelChannels(source)+i];
        pixels[3]=(double) p[3*(ssize_t) GetPixelChannels(source)+i];
        if ((traits & BlendPixelTrait) == 0)
          {
            alpha[0]=1.0;
            alpha[1]=1.0;
            alpha[2]=1.0;
            alpha[3]=1.0;
          }
        else
          {
            alpha[0]=QuantumScale*(double) GetPixelAlpha(source,p);
            alpha[1]=QuantumScale*(double) GetPixelAlpha(source,p+
              GetPixelChannels(source));
            alpha[2]=QuantumScale*(double) GetPixelAlpha(source,p+2*
              GetPixelChannels(source));
            alpha[3]=QuantumScale*(double) GetPixelAlpha(source,p+3*
              GetPixelChannels(source));
          }
        delta.x=x-x_offset;
        delta.y=y-y_offset;
        luminance.x=fabs((double) (GetPixelLuma(source,p)-
          GetPixelLuma(source,p+3*GetPixelChannels(source))));
        luminance.y=fabs((double) (GetPixelLuma(source,p+
          GetPixelChannels(source))-GetPixelLuma(source,p+2*
          GetPixelChannels(source))));
        if (luminance.x < luminance.y)
          {
            /*
              Diagonal 0-3 NW-SE.
            */
            if (delta.x <= delta.y)
              {
                /*
                  Bottom-left triangle (pixel: 2, diagonal: 0-3).
                */
                delta.y=1.0-delta.y;
                gamma=MeshInterpolate(&delta,alpha[2],alpha[3],alpha[0]);
                gamma=MagickSafeReciprocal(gamma);
                SetPixelChannel(destination,channel,ClampToQuantum(gamma*
                  MeshInterpolate(&delta,pixels[2],pixels[3],pixels[0])),pixel);
              }
            else
              {
                /*
                  Top-right triangle (pixel: 1, diagonal: 0-3).
                */
                delta.x=1.0-delta.x;
                gamma=MeshInterpolate(&delta,alpha[1],alpha[0],alpha[3]);
                gamma=MagickSafeReciprocal(gamma);
                SetPixelChannel(destination,channel,ClampToQuantum(gamma*
                  MeshInterpolate(&delta,pixels[1],pixels[0],pixels[3])),pixel);
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
                gamma=MagickSafeReciprocal(gamma);
                SetPixelChannel(destination,channel,ClampToQuantum(gamma*
                  MeshInterpolate(&delta,pixels[0],pixels[1],pixels[2])),pixel);
              }
            else
              {
                /*
                  Bottom-right triangle (pixel: 3, diagonal: 1-2).
                */
                delta.x=1.0-delta.x;
                delta.y=1.0-delta.y;
                gamma=MeshInterpolate(&delta,alpha[3],alpha[2],alpha[1]);
                gamma=MagickSafeReciprocal(gamma);
                SetPixelChannel(destination,channel,ClampToQuantum(gamma*
                  MeshInterpolate(&delta,pixels[3],pixels[2],pixels[1])),pixel);
              }
          }
      }
      break;
    }
    case SplineInterpolatePixel:
    {
      double
        cx[4],
        cy[4];

      p=GetCacheViewVirtualPixels(source_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        ssize_t
          j;

        PixelChannel channel = GetPixelChannelChannel(source,i);
        PixelTrait traits = GetPixelChannelTraits(source,channel);
        PixelTrait destination_traits=GetPixelChannelTraits(destination,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        if ((traits & BlendPixelTrait) == 0)
          for (j=0; j < 16; j++)
          {
            alpha[j]=1.0;
            pixels[j]=(double) p[j*(ssize_t) GetPixelChannels(source)+i];
          }
        else
          for (j=0; j < 16; j++)
          {
            alpha[j]=QuantumScale*(double) GetPixelAlpha(source,p+j*
              (ssize_t) GetPixelChannels(source));
            pixels[j]=alpha[j]*(double) p[j*(ssize_t) GetPixelChannels(source)+
              i];
          }
        SplineWeights((double) (x-x_offset),&cx);
        SplineWeights((double) (y-y_offset),&cy);
        gamma=((traits & BlendPixelTrait) ? (double) (1.0) :
          MagickSafeReciprocal(cy[0]*(cx[0]*alpha[0]+cx[1]*alpha[1]+cx[2]*
          alpha[2]+cx[3]*alpha[3])+cy[1]*(cx[0]*alpha[4]+cx[1]*alpha[5]+cx[2]*
          alpha[6]+cx[3]*alpha[7])+cy[2]*(cx[0]*alpha[8]+cx[1]*alpha[9]+cx[2]*
          alpha[10]+cx[3]*alpha[11])+cy[3]*(cx[0]*alpha[12]+cx[1]*alpha[13]+
          cx[2]*alpha[14]+cx[3]*alpha[15])));
        SetPixelChannel(destination,channel,ClampToQuantum(gamma*(cy[0]*(cx[0]*
          pixels[0]+cx[1]*pixels[1]+cx[2]*pixels[2]+cx[3]*pixels[3])+cy[1]*
          (cx[0]*pixels[4]+cx[1]*pixels[5]+cx[2]*pixels[6]+cx[3]*pixels[7])+
          cy[2]*(cx[0]*pixels[8]+cx[1]*pixels[9]+cx[2]*pixels[10]+cx[3]*
          pixels[11])+cy[3]*(cx[0]*pixels[12]+cx[1]*pixels[13]+cx[2]*
          pixels[14]+cx[3]*pixels[15]))),pixel);
      }
      break;
    }
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n t e r p o l a t e P i x e l I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpolatePixelInfo() applies a pixel interpolation method between a
%  floating point coordinate and the pixels surrounding that coordinate.  No
%  pixel area resampling, or scaling of the result is performed.
%
%  Interpolation is restricted to just RGBKA channels.
%
%  The format of the InterpolatePixelInfo method is:
%
%      MagickBooleanType InterpolatePixelInfo(const Image *image,
%        const CacheView *image_view,const PixelInterpolateMethod method,
%        const double x,const double y,PixelInfo *pixel,
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

static inline void AlphaBlendPixelInfo(const Image *image,
  const Quantum *pixel,PixelInfo *pixel_info,double *alpha)
{
  if ((image->alpha_trait & BlendPixelTrait) == 0)
    {
      *alpha=1.0;
      pixel_info->red=(double) GetPixelRed(image,pixel);
      pixel_info->green=(double) GetPixelGreen(image,pixel);
      pixel_info->blue=(double) GetPixelBlue(image,pixel);
      pixel_info->black=0.0;
      if (image->colorspace == CMYKColorspace)
        pixel_info->black=(double) GetPixelBlack(image,pixel);
      pixel_info->alpha=(double) GetPixelAlpha(image,pixel);
      return;
    }
  *alpha=QuantumScale*(double) GetPixelAlpha(image,pixel);
  pixel_info->red=(*alpha*(double) GetPixelRed(image,pixel));
  pixel_info->green=(*alpha*(double) GetPixelGreen(image,pixel));
  pixel_info->blue=(*alpha*(double) GetPixelBlue(image,pixel));
  pixel_info->black=0.0;
  if (image->colorspace == CMYKColorspace)
    pixel_info->black=(*alpha*(double) GetPixelBlack(image,pixel));
  pixel_info->alpha=(double) GetPixelAlpha(image,pixel);
}

MagickExport MagickBooleanType InterpolatePixelInfo(const Image *image,
  const CacheView_ *image_view,const PixelInterpolateMethod method,
  const double x,const double y,PixelInfo *pixel,ExceptionInfo *exception)
{
  const Quantum
    *p;

  double
    alpha[16],
    gamma;

  MagickBooleanType
    status;

  PixelInfo
    pixels[16];

  PixelInterpolateMethod
    interpolate;

  ssize_t
    i,
    x_offset,
    y_offset;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image_view != (CacheView *) NULL);
  status=MagickTrue;
  x_offset=CastDoubleToSsizeT(floor(x));
  y_offset=CastDoubleToSsizeT(floor(y));
  interpolate=method;
  if (interpolate == UndefinedInterpolatePixel)
    interpolate=image->interpolate;
  GetPixelInfoPixel(image,(const Quantum *) NULL,pixel);
  (void) memset(&pixels,0,sizeof(pixels));
  switch (interpolate)
  {
    case AverageInterpolatePixel:  /* nearest 4 neighbours */
    case Average9InterpolatePixel:  /* nearest 9 neighbours */
    case Average16InterpolatePixel:  /* nearest 16 neighbours */
    {
      ssize_t
        count;

      count=2;  /* size of the area to average - default nearest 4 */
      if (interpolate == Average9InterpolatePixel)
        {
          count=3;
          x_offset=CastDoubleToSsizeT(floor(x+0.5)-1.0);
          y_offset=CastDoubleToSsizeT(floor(y+0.5)-1.0);
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
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      pixel->red=0.0;
      pixel->green=0.0;
      pixel->blue=0.0;
      pixel->black=0.0;
      pixel->alpha=0.0;
      count*=count;  /* number of pixels - square of size */
      for (i=0; i < (ssize_t) count; i++)
      {
        AlphaBlendPixelInfo(image,p,pixels,alpha);
        gamma=MagickSafeReciprocal(alpha[0]);
        pixel->red+=gamma*pixels[0].red;
        pixel->green+=gamma*pixels[0].green;
        pixel->blue+=gamma*pixels[0].blue;
        pixel->black+=gamma*pixels[0].black;
        pixel->alpha+=pixels[0].alpha;
        p+=(ptrdiff_t) GetPixelChannels(image);
      }
      gamma=1.0/count;   /* average weighting of each pixel in area */
      pixel->red*=gamma;
      pixel->green*=gamma;
      pixel->blue*=gamma;
      pixel->black*=gamma;
      pixel->alpha*=gamma;
      break;
    }
    case BackgroundInterpolatePixel:
    {
      *pixel=image->background_color;  /* Copy PixelInfo Structure  */
      break;
    }
    case BilinearInterpolatePixel:
    default:
    {
      PointInfo
        delta,
        epsilon;

      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,2,2,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < 4L; i++)
        AlphaBlendPixelInfo(image,p+i*(ssize_t) GetPixelChannels(image),
          pixels+i,alpha+i);
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      epsilon.x=1.0-delta.x;
      epsilon.y=1.0-delta.y;
      gamma=((epsilon.y*(epsilon.x*alpha[0]+delta.x*alpha[1])+delta.y*
        (epsilon.x*alpha[2]+delta.x*alpha[3])));
      gamma=MagickSafeReciprocal(gamma);
      pixel->red=gamma*(epsilon.y*(epsilon.x*pixels[0].red+delta.x*
        pixels[1].red)+delta.y*(epsilon.x*pixels[2].red+delta.x*pixels[3].red));
      pixel->green=gamma*(epsilon.y*(epsilon.x*pixels[0].green+delta.x*
        pixels[1].green)+delta.y*(epsilon.x*pixels[2].green+delta.x*
        pixels[3].green));
      pixel->blue=gamma*(epsilon.y*(epsilon.x*pixels[0].blue+delta.x*
        pixels[1].blue)+delta.y*(epsilon.x*pixels[2].blue+delta.x*
        pixels[3].blue));
      if (image->colorspace == CMYKColorspace)
        pixel->black=gamma*(epsilon.y*(epsilon.x*pixels[0].black+delta.x*
          pixels[1].black)+delta.y*(epsilon.x*pixels[2].black+delta.x*
          pixels[3].black));
      gamma=((epsilon.y*(epsilon.x+delta.x)+delta.y*(epsilon.x+delta.x)));
      gamma=MagickSafeReciprocal(gamma);
      pixel->alpha=gamma*(epsilon.y*(epsilon.x*pixels[0].alpha+delta.x*
        pixels[1].alpha)+delta.y*(epsilon.x*pixels[2].alpha+delta.x*
        pixels[3].alpha));
      break;
    }
    case BlendInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,2,2,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < 4L; i++)
      {
        GetPixelInfoPixel(image,p+i*(ssize_t) GetPixelChannels(image),pixels+i);
        AlphaBlendPixelInfo(image,p+i*(ssize_t) GetPixelChannels(image),
          pixels+i,alpha+i);
      }
      gamma=1.0;  /* number of pixels blended together (its variable) */
      for (i=0; i <= 1L; i++)
      {
        if ((y-y_offset) >= 0.75)
          {
            alpha[i]=alpha[i+2];  /* take right pixels */
            pixels[i]=pixels[i+2];
          }
        else
          if ((y-y_offset) > 0.25)
            {
              gamma=2.0;  /* blend both pixels in row */
              alpha[i]+=alpha[i+2];  /* add up alpha weights */
              pixels[i].red+=pixels[i+2].red;
              pixels[i].green+=pixels[i+2].green;
              pixels[i].blue+=pixels[i+2].blue;
              pixels[i].black+=pixels[i+2].black;
              pixels[i].alpha+=pixels[i+2].alpha;
            }
      }
      if ((x-x_offset) >= 0.75)
        {
          alpha[0]=alpha[1];
          pixels[0]=pixels[1];
        }
      else
        if ((x-x_offset) > 0.25)
          {
            gamma*=2.0;  /* blend both rows */
            alpha[0]+= alpha[1];  /* add up alpha weights */
            pixels[0].red+=pixels[1].red;
            pixels[0].green+=pixels[1].green;
            pixels[0].blue+=pixels[1].blue;
            pixels[0].black+=pixels[1].black;
            pixels[0].alpha+=pixels[1].alpha;
          }
      gamma=1.0/gamma;
      alpha[0]=MagickSafeReciprocal(alpha[0]);
      pixel->red=alpha[0]*pixels[0].red;
      pixel->green=alpha[0]*pixels[0].green;  /* divide by sum of alpha */
      pixel->blue=alpha[0]*pixels[0].blue;
      pixel->black=alpha[0]*pixels[0].black;
      pixel->alpha=gamma*pixels[0].alpha;   /* divide by number of pixels */
      break;
    }
    case CatromInterpolatePixel:
    {
      double
        cx[4],
        cy[4];

      p=GetCacheViewVirtualPixels(image_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < 16L; i++)
        AlphaBlendPixelInfo(image,p+i*(ssize_t) GetPixelChannels(image),
          pixels+i,alpha+i);
      CatromWeights((double) (x-x_offset),&cx);
      CatromWeights((double) (y-y_offset),&cy);
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
        pixels[10].green+cx[3]*pixels[11].green)+cy[3]*(cx[0]*
        pixels[12].green+cx[1]*pixels[13].green+cx[2]*pixels[14].green+cx[3]*
        pixels[15].green));
      pixel->blue=(cy[0]*(cx[0]*pixels[0].blue+cx[1]*pixels[1].blue+cx[2]*
        pixels[2].blue+cx[3]*pixels[3].blue)+cy[1]*(cx[0]*pixels[4].blue+cx[1]*
        pixels[5].blue+cx[2]*pixels[6].blue+cx[3]*pixels[7].blue)+cy[2]*(cx[0]*
        pixels[8].blue+cx[1]*pixels[9].blue+cx[2]*pixels[10].blue+cx[3]*
        pixels[11].blue)+cy[3]*(cx[0]*pixels[12].blue+cx[1]*pixels[13].blue+
        cx[2]*pixels[14].blue+cx[3]*pixels[15].blue));
      if (image->colorspace == CMYKColorspace)
        pixel->black=(cy[0]*(cx[0]*pixels[0].black+cx[1]*pixels[1].black+cx[2]*
          pixels[2].black+cx[3]*pixels[3].black)+cy[1]*(cx[0]*pixels[4].black+
          cx[1]*pixels[5].black+cx[2]*pixels[6].black+cx[3]*pixels[7].black)+
          cy[2]*(cx[0]*pixels[8].black+cx[1]*pixels[9].black+cx[2]*
          pixels[10].black+cx[3]*pixels[11].black)+cy[3]*(cx[0]*
          pixels[12].black+cx[1]*pixels[13].black+cx[2]*pixels[14].black+cx[3]*
          pixels[15].black));
      pixel->alpha=(cy[0]*(cx[0]*pixels[0].alpha+cx[1]*pixels[1].alpha+cx[2]*
        pixels[2].alpha+cx[3]*pixels[3].alpha)+cy[1]*(cx[0]*pixels[4].alpha+
        cx[1]*pixels[5].alpha+cx[2]*pixels[6].alpha+cx[3]*pixels[7].alpha)+
        cy[2]*(cx[0]*pixels[8].alpha+cx[1]*pixels[9].alpha+cx[2]*
        pixels[10].alpha+cx[3]*pixels[11].alpha)+cy[3]*(cx[0]*pixels[12].alpha+
        cx[1]*pixels[13].alpha+cx[2]*pixels[14].alpha+cx[3]*pixels[15].alpha));
      break;
    }
    case IntegerInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,1,1,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      GetPixelInfoPixel(image,p,pixel);
      break;
    }
    case MeshInterpolatePixel:
    {
      PointInfo
        delta,
        luminance;

      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,2,2,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      luminance.x=GetPixelLuma(image,p)-(double)
        GetPixelLuma(image,p+3*GetPixelChannels(image));
      luminance.y=GetPixelLuma(image,p+GetPixelChannels(image))-(double)
        GetPixelLuma(image,p+2*GetPixelChannels(image));
      AlphaBlendPixelInfo(image,p,pixels+0,alpha+0);
      AlphaBlendPixelInfo(image,p+GetPixelChannels(image),pixels+1,alpha+1);
      AlphaBlendPixelInfo(image,p+2*GetPixelChannels(image),pixels+2,alpha+2);
      AlphaBlendPixelInfo(image,p+3*GetPixelChannels(image),pixels+3,alpha+3);
      if (fabs((double) luminance.x) < fabs((double) luminance.y))
        {
          /*
            Diagonal 0-3 NW-SE.
          */
          if (delta.x <= delta.y)
            {
              /*
                Bottom-left triangle (pixel: 2, diagonal: 0-3).
              */
              delta.y=1.0-delta.y;
              gamma=MeshInterpolate(&delta,alpha[2],alpha[3],alpha[0]);
              gamma=MagickSafeReciprocal(gamma);
              pixel->red=gamma*MeshInterpolate(&delta,pixels[2].red,
                pixels[3].red,pixels[0].red);
              pixel->green=gamma*MeshInterpolate(&delta,pixels[2].green,
                pixels[3].green,pixels[0].green);
              pixel->blue=gamma*MeshInterpolate(&delta,pixels[2].blue,
                pixels[3].blue,pixels[0].blue);
              if (image->colorspace == CMYKColorspace)
                pixel->black=gamma*MeshInterpolate(&delta,pixels[2].black,
                  pixels[3].black,pixels[0].black);
              gamma=MeshInterpolate(&delta,1.0,1.0,1.0);
              pixel->alpha=gamma*MeshInterpolate(&delta,pixels[2].alpha,
                pixels[3].alpha,pixels[0].alpha);
            }
          else
            {
              /*
                Top-right triangle (pixel:1 , diagonal: 0-3).
              */
              delta.x=1.0-delta.x;
              gamma=MeshInterpolate(&delta,alpha[1],alpha[0],alpha[3]);
              gamma=MagickSafeReciprocal(gamma);
              pixel->red=gamma*MeshInterpolate(&delta,pixels[1].red,
                pixels[0].red,pixels[3].red);
              pixel->green=gamma*MeshInterpolate(&delta,pixels[1].green,
                pixels[0].green,pixels[3].green);
              pixel->blue=gamma*MeshInterpolate(&delta,pixels[1].blue,
                pixels[0].blue,pixels[3].blue);
              if (image->colorspace == CMYKColorspace)
                pixel->black=gamma*MeshInterpolate(&delta,pixels[1].black,
                  pixels[0].black,pixels[3].black);
              gamma=MeshInterpolate(&delta,1.0,1.0,1.0);
              pixel->alpha=gamma*MeshInterpolate(&delta,pixels[1].alpha,
                pixels[0].alpha,pixels[3].alpha);
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
              gamma=MagickSafeReciprocal(gamma);
              pixel->red=gamma*MeshInterpolate(&delta,pixels[0].red,
                pixels[1].red,pixels[2].red);
              pixel->green=gamma*MeshInterpolate(&delta,pixels[0].green,
                pixels[1].green,pixels[2].green);
              pixel->blue=gamma*MeshInterpolate(&delta,pixels[0].blue,
                pixels[1].blue,pixels[2].blue);
              if (image->colorspace == CMYKColorspace)
                pixel->black=gamma*MeshInterpolate(&delta,pixels[0].black,
                  pixels[1].black,pixels[2].black);
              gamma=MeshInterpolate(&delta,1.0,1.0,1.0);
              pixel->alpha=gamma*MeshInterpolate(&delta,pixels[0].alpha,
                pixels[1].alpha,pixels[2].alpha);
            }
          else
            {
              /*
                Bottom-right triangle (pixel: 3, diagonal: 1-2).
              */
              delta.x=1.0-delta.x;
              delta.y=1.0-delta.y;
              gamma=MeshInterpolate(&delta,alpha[3],alpha[2],alpha[1]);
              gamma=MagickSafeReciprocal(gamma);
              pixel->red=gamma*MeshInterpolate(&delta,pixels[3].red,
                pixels[2].red,pixels[1].red);
              pixel->green=gamma*MeshInterpolate(&delta,pixels[3].green,
                pixels[2].green,pixels[1].green);
              pixel->blue=gamma*MeshInterpolate(&delta,pixels[3].blue,
                pixels[2].blue,pixels[1].blue);
              if (image->colorspace == CMYKColorspace)
                pixel->black=gamma*MeshInterpolate(&delta,pixels[3].black,
                  pixels[2].black,pixels[1].black);
              gamma=MeshInterpolate(&delta,1.0,1.0,1.0);
              pixel->alpha=gamma*MeshInterpolate(&delta,pixels[3].alpha,
                pixels[2].alpha,pixels[1].alpha);
            }
        }
      break;
    }
    case NearestInterpolatePixel:
    {
      x_offset=CastDoubleToSsizeT(floor(x+0.5));
      y_offset=CastDoubleToSsizeT(floor(y+0.5));
      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,1,1,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      GetPixelInfoPixel(image,p,pixel);
      break;
    }
    case SplineInterpolatePixel:
    {
      double
        cx[4],
        cy[4];

      p=GetCacheViewVirtualPixels(image_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < 16L; i++)
        AlphaBlendPixelInfo(image,p+i*(ssize_t) GetPixelChannels(image),
          pixels+i,alpha+i);
      SplineWeights((double) (x-x_offset),&cx);
      SplineWeights((double) (y-y_offset),&cy);
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
      if (image->colorspace == CMYKColorspace)
        pixel->black=(cy[0]*(cx[0]*pixels[0].black+cx[1]*pixels[1].black+cx[2]*
          pixels[2].black+cx[3]*pixels[3].black)+cy[1]*(cx[0]*pixels[4].black+
          cx[1]*pixels[5].black+cx[2]*pixels[6].black+cx[3]*pixels[7].black)+
          cy[2]*(cx[0]*pixels[8].black+cx[1]*pixels[9].black+cx[2]*
          pixels[10].black+cx[3]*pixels[11].black)+cy[3]*(cx[0]*
          pixels[12].black+cx[1]*pixels[13].black+cx[2]*pixels[14].black+cx[3]*
          pixels[15].black));
      pixel->alpha=(cy[0]*(cx[0]*pixels[0].alpha+cx[1]*pixels[1].alpha+cx[2]*
        pixels[2].alpha+cx[3]*pixels[3].alpha)+cy[1]*(cx[0]*pixels[4].alpha+
        cx[1]*pixels[5].alpha+cx[2]*pixels[6].alpha+cx[3]*pixels[7].alpha)+
        cy[2]*(cx[0]*pixels[8].alpha+cx[1]*pixels[9].alpha+cx[2]*
        pixels[10].alpha+cx[3]*pixels[11].alpha)+cy[3]*(cx[0]*pixels[12].alpha+
        cx[1]*pixels[13].alpha+cx[2]*pixels[14].alpha+cx[3]*pixels[15].alpha));
      break;
    }
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s F u z z y E q u i v a l e n c e P i x e l                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsFuzzyEquivalencePixel() returns MagickTrue if the distance between two
%  pixels is less than the specified distance in a linear three (or four)
%  dimensional color space.
%
%  The format of the IsFuzzyEquivalencePixel method is:
%
%      void IsFuzzyEquivalencePixel(const Image *image,const Quantum *p,
%        const Image *target_image,const Quantum *q)
%
%  A description of each parameter follows:
%
%    o image: the source image.
%
%    o p: Pixel p.
%
%    o target: the target image.
%
%    o q: Pixel q.
%
*/
MagickExport MagickBooleanType IsFuzzyEquivalencePixel(const Image *image,
  const Quantum *p,const Image *target_image,const Quantum *q)
{
  double
    alpha = QuantumScale*(double) GetPixelAlpha(image,p),
    fuzz = GetFuzzyColorDistance(image,target_image),
    target_alpha = QuantumScale*(double) GetPixelAlpha(target_image,q);

  ssize_t
    i;

  /*
    MSE metric for comparing two pixels.
  */
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  { 
    double
      error;

    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    PixelTrait target_traits = GetPixelChannelTraits(target_image,channel);
    if (((traits & UpdatePixelTrait) == 0) ||
        ((target_traits & UpdatePixelTrait) == 0))
      continue;
    if (channel == AlphaPixelChannel)
      error=(double) p[i]-(double) GetPixelChannel(target_image,channel,q);
    else
      error=alpha*(double) p[i]-target_alpha*
        GetPixelChannel(target_image,channel,q);
    if (MagickSafeSignificantError(error*error,fuzz) != MagickFalse)
      return(MagickFalse);
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s F u z z y E q u i v a l e n c e P i x e l I n f o                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsFuzzyEquivalencePixelInfo() returns true if the distance between two
%  colors is less than the specified distance in a linear three (or four)
%  dimensional color space.
%
%  This implements the equivalent of:
%    fuzz < sqrt(color_distance^2 * u.a*v.a  + alpha_distance^2)
%
%  Which produces a multi-dimensional cone for that colorspace along the
%  transparency vector.
%
%  For example for an RGB:
%    color_distance^2  = ( (u.r-v.r)^2 + (u.g-v.g)^2 + (u.b-v.b)^2 ) / 3
%
%  See https://imagemagick.org/Usage/bugs/fuzz_distance/
%
%  Hue colorspace distances need more work.  Hue is not a distance, it is an
%  angle!
%
%  A check that q is in the same color space as p should be made and the
%  appropriate mapping made.  -- Anthony Thyssen  8 December 2010
%
%  The format of the IsFuzzyEquivalencePixelInfo method is:
%
%      MagickBooleanType IsFuzzyEquivalencePixelInfo(const PixelInfo *p,
%        const PixelInfo *q)
%
%  A description of each parameter follows:
%
%    o p: Pixel p.
%
%    o q: Pixel q.
%
*/
MagickExport MagickBooleanType IsFuzzyEquivalencePixelInfo(const PixelInfo *p,
  const PixelInfo *q)
{
  double
    distance,
    fuzz,
    pixel,
    scale;

  fuzz=p->fuzz*p->fuzz+q->fuzz*q->fuzz;
  scale=1.0;
  distance=0.0;
  if ((p->alpha_trait != UndefinedPixelTrait) ||
      (q->alpha_trait != UndefinedPixelTrait))
    {
      /*
        Transparencies are involved - set alpha distance.
      */
      pixel=(p->alpha_trait != UndefinedPixelTrait ? p->alpha :
        (double) OpaqueAlpha)-(q->alpha_trait != UndefinedPixelTrait ?
        q->alpha : (double) OpaqueAlpha);
      distance=pixel*pixel;
      if (MagickSafeSignificantError(distance*distance,fuzz) != MagickFalse)
        return(MagickFalse);
      /*
        Generate a alpha scaling factor to generate a 4D cone on colorspace.
        If one color is transparent, distance has no color component.
      */
      if (p->alpha_trait != UndefinedPixelTrait)
        scale=(QuantumScale*p->alpha);
      if (q->alpha_trait != UndefinedPixelTrait)
        scale*=(QuantumScale*q->alpha);
      if (scale <= MagickEpsilon)
        return(MagickTrue);
    }
  /*
    CMYK create a CMY cube with a multi-dimensional cone toward black.
  */
  if (p->colorspace == CMYKColorspace)
    {
      pixel=p->black-q->black;
      distance+=pixel*pixel*scale;
      if (MagickSafeSignificantError(distance*distance,fuzz) != MagickFalse)
        return(MagickFalse);
      scale*=QuantumScale*((double) QuantumRange-(double) p->black);
      scale*=QuantumScale*((double) QuantumRange-(double) q->black);
    }
  /*
    RGB or CMY color cube.
  */
  distance*=3.0;  /* rescale appropriately */
  fuzz*=3.0;
  pixel=p->red-q->red;
  if (IsHueCompatibleColorspace(p->colorspace) != MagickFalse)
    {
      /*
        This calculates a arc distance for hue-- it should be a vector
        angle of 'S'/'W' length with 'L'/'B' forming appropriate cones.
        In other words this is a hack - Anthony.
      */
      if (fabs((double) pixel) > ((double) QuantumRange/2.0))
        pixel-=(double) QuantumRange;
      pixel*=2.0;
    }
  distance+=pixel*pixel*scale;
  if (MagickSafeSignificantError(distance,fuzz) != MagickFalse)
    return(MagickFalse);
  pixel=p->green-q->green;
  distance+=pixel*pixel*scale;
  if (MagickSafeSignificantError(distance,fuzz) != MagickFalse)
    return(MagickFalse);
  pixel=p->blue-q->blue;
  distance+=pixel*pixel*scale;
  if (MagickSafeSignificantError(distance,fuzz) != MagickFalse)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e s e t P i x e l C h a n n e l M a p                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetPixelChannelMap() defines the standard pixel component map.
%
%  The format of the ResetPixelChannelMap() method is:
%
%      MagickBooleanType ResetPixelChannelMap(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate MagickBooleanType ResetPixelChannelMap(Image *image,
  ExceptionInfo *exception)
{
  PixelTrait
    trait;

  ssize_t
    n;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  (void) memset(image->channel_map,0,MaxPixelChannels*
    sizeof(*image->channel_map));
  trait=UpdatePixelTrait;
  if (image->alpha_trait != UndefinedPixelTrait)
    trait=(PixelTrait) (trait | BlendPixelTrait);
  n=0;
  if ((image->colorspace == LinearGRAYColorspace) ||
      (image->colorspace == GRAYColorspace))
    {
      SetPixelChannelAttributes(image,BluePixelChannel,trait,n);
      SetPixelChannelAttributes(image,GreenPixelChannel,trait,n);
      SetPixelChannelAttributes(image,RedPixelChannel,trait,n++);
    }
  else
    {
      SetPixelChannelAttributes(image,RedPixelChannel,trait,n++);
      SetPixelChannelAttributes(image,GreenPixelChannel,trait,n++);
      SetPixelChannelAttributes(image,BluePixelChannel,trait,n++);
    }
  if (image->colorspace == CMYKColorspace)
    SetPixelChannelAttributes(image,BlackPixelChannel,trait,n++);
  if (image->alpha_trait != UndefinedPixelTrait)
    SetPixelChannelAttributes(image,AlphaPixelChannel,CopyPixelTrait,n++);
  if (image->storage_class == PseudoClass)
    SetPixelChannelAttributes(image,IndexPixelChannel,CopyPixelTrait,n++);
  if ((image->channels & ReadMaskChannel) != 0)
    SetPixelChannelAttributes(image,ReadMaskPixelChannel,CopyPixelTrait,n++);
  if ((image->channels & WriteMaskChannel) != 0)
    SetPixelChannelAttributes(image,WriteMaskPixelChannel,CopyPixelTrait,n++);
  if ((image->channels & CompositeMaskChannel) != 0)
    SetPixelChannelAttributes(image,CompositeMaskPixelChannel,CopyPixelTrait,
      n++);
  if (image->number_meta_channels != 0)
    {
      PixelChannel
        meta_channel;

      ssize_t
        i;

      if (image->number_meta_channels >= (size_t) (MaxPixelChannels-MetaPixelChannels))
        {
          image->number_channels=(size_t) n;
          image->number_meta_channels=0;
          (void) SetPixelChannelMask(image,image->channel_mask);
          ThrowBinaryException(CorruptImageError,"MaximumChannelsExceeded",
            image->filename);
        }
      meta_channel=MetaPixelChannels;
      for (i=0; i < (ssize_t) image->number_meta_channels; i++)
      {
        SetPixelChannelAttributes(image,meta_channel,UpdatePixelTrait,n);
        meta_channel=(PixelChannel) (meta_channel+1);
        n++;
      }
    }
  image->number_channels=(size_t) n;
  (void) SetPixelChannelMask(image,image->channel_mask);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t P i x e l C h a n n e l M a s k                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetPixelChannelMask() sets the pixel channel map from the specified channel
%  mask.
%
%  The format of the SetPixelChannelMask method is:
%
%      ChannelType SetPixelChannelMask(Image *image,
%        const ChannelType channel_mask)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel_mask: the channel mask.
%
*/

static void LogPixelChannels(const Image *image)
{
  ssize_t
    i;

  (void) LogMagickEvent(PixelEvent,GetMagickModule(),"%s[0x%08llx]",
    image->filename,(MagickOffsetType) image->channel_mask);
  for (i=0; i < (ssize_t) image->number_channels; i++)
  {
    char
      channel_name[MagickPathExtent],
      traits[MagickPathExtent];

    const char
      *name;

    PixelChannel
      channel;

    channel=GetPixelChannelChannel(image,i);
    switch (channel)
    {
      case RedPixelChannel:
      {
        name="red";
        if (image->colorspace == CMYKColorspace)
          name="cyan";
        if ((image->colorspace == LinearGRAYColorspace) ||
            (image->colorspace == GRAYColorspace))
          name="gray";
        break;
      }
      case GreenPixelChannel:
      {
        name="green";
        if (image->colorspace == CMYKColorspace)
          name="magenta";
        break;
      }
      case BluePixelChannel:
      {
        name="blue";
        if (image->colorspace == CMYKColorspace)
          name="yellow";
        break;
      }
      case BlackPixelChannel:
      {
        name="black";
        if (image->storage_class == PseudoClass)
          name="index";
        break;
      }
      case IndexPixelChannel:
      {
        name="index";
        break;
      }
      case AlphaPixelChannel:
      {
        name="alpha";
        break;
      }
      case ReadMaskPixelChannel:
      {
        name="read-mask";
        break;
      }
      case WriteMaskPixelChannel:
      {
        name="write-mask";
        break;
      }
      case CompositeMaskPixelChannel:
      {
        name="composite-mask";
        break;
      }
      case MetaPixelChannels:
      {
        name="meta";
        break;
      }
      default:
        name="undefined";
    }
    if (image->colorspace ==  UndefinedColorspace)
      {
        (void) FormatLocaleString(channel_name,MagickPathExtent,"%.20g",
          (double) channel);
        name=(const char *) channel_name;
      }
    *traits='\0';
    if ((GetPixelChannelTraits(image,channel) & UpdatePixelTrait) != 0)
      (void) ConcatenateMagickString(traits,"update,",MagickPathExtent);
    if ((GetPixelChannelTraits(image,channel) & BlendPixelTrait) != 0)
      (void) ConcatenateMagickString(traits,"blend,",MagickPathExtent);
    if ((GetPixelChannelTraits(image,channel) & CopyPixelTrait) != 0)
      (void) ConcatenateMagickString(traits,"copy,",MagickPathExtent);
    if (*traits == '\0')
      (void) ConcatenateMagickString(traits,"undefined,",MagickPathExtent);
    traits[strlen(traits)-1]='\0';
    (void) LogMagickEvent(PixelEvent,GetMagickModule(),"  %.20g: %s (%s)",
      (double) i,name,traits);
  }
}

MagickExport ChannelType SetPixelChannelMask(Image *image,
  const ChannelType channel_mask)
{
#define GetChannelBit(mask,bit)  (((size_t) (mask) >> (size_t) (bit)) & 0x01)

  ChannelType
    mask;

  ssize_t
    i;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(PixelEvent,GetMagickModule(),"%s[0x%08llx]",
      image->filename,(MagickOffsetType) channel_mask);
  mask=image->channel_mask;
  image->channel_mask=channel_mask;
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,i);
    if (GetChannelBit(channel_mask,channel) == 0)
      {
        SetPixelChannelTraits(image,channel,CopyPixelTrait);
        continue;
      }
    if (channel == AlphaPixelChannel)
      {
        if ((image->alpha_trait & CopyPixelTrait) != 0)
          {
            SetPixelChannelTraits(image,channel,CopyPixelTrait);
            continue;
          }
        SetPixelChannelTraits(image,channel,UpdatePixelTrait);
        continue;
      }
    if (image->alpha_trait != UndefinedPixelTrait)
      {
        SetPixelChannelTraits(image,channel,(PixelTrait) (UpdatePixelTrait |
          BlendPixelTrait));
        continue;
      }
    SetPixelChannelTraits(image,channel,UpdatePixelTrait);
  }
  if (image->storage_class == PseudoClass)
    SetPixelChannelTraits(image,IndexPixelChannel,CopyPixelTrait);
  if ((image->channels & ReadMaskChannel) != 0)
    SetPixelChannelTraits(image,ReadMaskPixelChannel,CopyPixelTrait);
  if ((image->channels & WriteMaskChannel) != 0)
    SetPixelChannelTraits(image,WriteMaskPixelChannel,CopyPixelTrait);
  if ((image->channels & CompositeMaskChannel) != 0)
    SetPixelChannelTraits(image,CompositeMaskPixelChannel,CopyPixelTrait);
  if ((GetLogEventMask() & PixelEvent) != 0)
    LogPixelChannels(image);
  return(mask);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t P i x e l M e t a C h a n n e l s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetPixelMetaChannels() sets the image meta channels.
%
%  The format of the SetPixelMetaChannels method is:
%
%      MagickBooleanType SetPixelMetaChannels(Image *image,
%        const size_t number_meta_channels,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o number_meta_channels:  the number of meta channels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetPixelMetaChannels(Image *image,
  const size_t number_meta_channels,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  if (number_meta_channels >= (size_t) (MaxPixelChannels-MetaPixelChannels))
   ThrowBinaryException(CorruptImageError,"MaximumChannelsExceeded",
     image->filename);
  image->number_meta_channels=number_meta_channels;
  status=ResetPixelChannelMap(image,exception);
  if (status == MagickFalse)
    return(MagickFalse);
  return(SyncImagePixelCache(image,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S o r t I m a g e P i x e l s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SortImagePixels() sorts pixels within each scanline in ascending order of
%  intensity.
%
%  The format of the SortImagePixels method is:
%
%      MagickBooleanType SortImagePixels(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SortImagePixels(Image *image,
  ExceptionInfo *exception)
{
#define SolarizeImageTag  "Solarize/Image"

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  /*
    Sort image pixels.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns-1; x++)
    {
      MagickRealType
        current,
        previous;

      ssize_t
        j;

      previous=GetPixelIntensity(image,q);
      for (j=0; j < ((ssize_t) image->columns-x-1); j++)
      {
        current=GetPixelIntensity(image,q+(j+1)*(ssize_t)
          GetPixelChannels(image));
        if (previous > current)
          {
            Quantum
              pixel[MaxPixelChannels];

            /*
              Swap adjacent pixels.
            */
            (void) memcpy(pixel,q+j*(ssize_t) GetPixelChannels(image),
              GetPixelChannels(image)*sizeof(Quantum));
            (void) memcpy(q+j*(ssize_t) GetPixelChannels(image),q+(j+1)*
              (ssize_t) GetPixelChannels(image),GetPixelChannels(image)*
              sizeof(Quantum));
            (void) memcpy(q+(j+1)*(ssize_t) GetPixelChannels(image),pixel,
              GetPixelChannels(image)*sizeof(Quantum));
          }
        else
          previous=current;
      }
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,SolarizeImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

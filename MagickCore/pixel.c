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
%                               John Cristy                                   %
%                               October 1998                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"

#define LogPixelChannels(image) \
{ \
  register ssize_t \
    i; \
 \
  (void) LogMagickEvent(PixelEvent,GetMagickModule(),"%s[%.20g]", \
    image->filename,(double) image->number_channels); \
  for (i=0; i < (ssize_t) image->number_channels; i++) \
  { \
    char \
      traits[MaxTextExtent]; \
 \
    const char \
      *name; \
 \
    PixelChannel \
      channel; \
 \
    switch (GetPixelChannelMapChannel(image,i)) \
    { \
      case RedPixelChannel: \
      { \
        name="red"; \
        if (image->colorspace == CMYKColorspace) \
          name="cyan"; \
        if (image->colorspace == GRAYColorspace) \
          name="gray"; \
        break; \
      } \
      case GreenPixelChannel: \
      { \
        name="green"; \
        if (image->colorspace == CMYKColorspace) \
          name="magenta"; \
        break; \
      } \
      case BluePixelChannel: \
      { \
        name="blue"; \
        if (image->colorspace == CMYKColorspace) \
          name="yellow"; \
        break; \
      } \
      case BlackPixelChannel: \
      { \
        name="black"; \
        if (image->storage_class == PseudoClass) \
          name="index"; \
        break; \
      } \
      case IndexPixelChannel: \
      { \
        name="index"; \
        break; \
      } \
      case AlphaPixelChannel: \
      { \
        name="alpha"; \
        break; \
      } \
      case MaskPixelChannel: \
      { \
        name="mask"; \
        break; \
      } \
      case MetaPixelChannel: \
      { \
        name="meta"; \
        break; \
      } \
      default: \
        name="undefined"; \
    } \
    channel=GetPixelChannelMapChannel(image,i); \
    *traits='\0'; \
    if ((GetPixelChannelMapTraits(image,channel) & UpdatePixelTrait) != 0) \
      (void) ConcatenateMagickString(traits,"update,",MaxTextExtent); \
    if ((GetPixelChannelMapTraits(image,channel) & BlendPixelTrait) != 0) \
      (void) ConcatenateMagickString(traits,"blend,",MaxTextExtent); \
    if ((GetPixelChannelMapTraits(image,channel) & CopyPixelTrait) != 0) \
      (void) ConcatenateMagickString(traits,"copy,",MaxTextExtent); \
    if (*traits == '\0') \
      (void) ConcatenateMagickString(traits,"undefined,",MaxTextExtent); \
    traits[strlen(traits)-1]='\0'; \
    (void) LogMagickEvent(PixelEvent,GetMagickModule(),"  %.20g: %s (%s)", \
      (double) i,name,traits); \
  } \
}

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

  register ssize_t
    i;

  channel_map=(PixelChannelMap *) AcquireQuantumMemory(MaxPixelChannels,
    sizeof(*channel_map));
  if (channel_map == (PixelChannelMap *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(channel_map,0,MaxPixelChannels*sizeof(*channel_map));
  for (i=0; i < MaxPixelChannels; i++)
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
  (void) CopyMagickMemory(clone_map,channel_map,MaxPixelChannels*
    sizeof(*channel_map));
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
%      PixelInfo *ClonePixelInfo(const PixelInfo *pixel_info)
%
%  A description of each parameter follows:
%
%    o pixel_info: the pixel info.
%
*/
MagickExport PixelInfo *ClonePixelInfo(const PixelInfo *pixel)
{
  PixelInfo
    *pixel_info;

  pixel_info=(PixelInfo *) AcquireQuantumMemory(1,sizeof(*pixel_info));
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
%   E x p o r t I m a g e P i x e l s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExportImagePixels() extracts pixel data from an image and returns it to you.
%  The method returns MagickTrue on success otherwise MagickFalse if an error is
%  encountered.  The data is returned as char, short int, int, ssize_t, float,
%  or double in the order specified by map.
%
%  Suppose you want to extract the first scanline of a 640x480 image as
%  character data in red-green-blue order:
%
%      ExportImagePixels(image,0,0,640,1,"RGB",CharPixel,pixels,exception);
%
%  The format of the ExportImagePixels method is:
%
%      MagickBooleanType ExportImagePixels(const Image *image,
%        const ssize_t x_offset,const ssize_t y_offset,const size_t columns,
%        const size_t rows,const char *map,const StorageType type,
%        void *pixels,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x_offset,y_offset,columns,rows:  These values define the perimeter
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
%      types: CharPixel, DoublePixel, FloatPixel, IntegerPixel, LongPixel,
%      QuantumPixel, or ShortPixel.
%
%    o pixels: This array of values contain the pixel components as defined by
%      map and type.  You must preallocate this array where the expected
%      length varies depending on the values of width, height, map, and type.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ExportImagePixels(const Image *image,
  const ssize_t x_offset,const ssize_t y_offset,const size_t columns,
  const size_t rows,const char *map,const StorageType type,void *pixels,
  ExceptionInfo *exception)
{
  QuantumType
    *quantum_map;

  register ssize_t
    i,
    x;

  register const Quantum
    *p;

  size_t
    length;

  ssize_t
    y;

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
  switch (type)
  {
    case CharPixel:
    {
      register unsigned char
        *q;

      q=(unsigned char *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
              *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
              *q++=ScaleQuantumToChar(GetPixelRed(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
              *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
              *q++=ScaleQuantumToChar(GetPixelRed(image,p));
              *q++=ScaleQuantumToChar(GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
              *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
              *q++=ScaleQuantumToChar(GetPixelRed(image,p));
              *q++=ScaleQuantumToChar((Quantum) 0);
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToChar(GetPixelIntensity(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToChar(GetPixelRed(image,p));
              *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
              *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToChar(GetPixelRed(image,p));
              *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
              *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
              *q++=ScaleQuantumToChar(GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToChar(GetPixelRed(image,p));
              *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
              *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
              *q++=ScaleQuantumToChar((Quantum) 0);
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
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
                *q=ScaleQuantumToChar(GetPixelIntensity(image,p));
                break;
              }
              default:
                break;
            }
            q++;
          }
          p+=GetPixelChannels(image);
        }
      }
      break;
    }
    case DoublePixel:
    {
      register double
        *q;

      q=(double *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(double) (QuantumScale*GetPixelBlue(image,p));
              *q++=(double) (QuantumScale*GetPixelGreen(image,p));
              *q++=(double) (QuantumScale*GetPixelRed(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(double) (QuantumScale*GetPixelBlue(image,p));
              *q++=(double) (QuantumScale*GetPixelGreen(image,p));
              *q++=(double) (QuantumScale*GetPixelRed(image,p));
              *q++=(double) (QuantumScale*GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(double) (QuantumScale*GetPixelBlue(image,p));
              *q++=(double) (QuantumScale*GetPixelGreen(image,p));
              *q++=(double) (QuantumScale*GetPixelRed(image,p));
              *q++=0.0;
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(double) (QuantumScale*GetPixelIntensity(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(double) (QuantumScale*GetPixelRed(image,p));
              *q++=(double) (QuantumScale*GetPixelGreen(image,p));
              *q++=(double) (QuantumScale*GetPixelBlue(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(double) (QuantumScale*GetPixelRed(image,p));
              *q++=(double) (QuantumScale*GetPixelGreen(image,p));
              *q++=(double) (QuantumScale*GetPixelBlue(image,p));
              *q++=(double) (QuantumScale*GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(double) (QuantumScale*GetPixelRed(image,p));
              *q++=(double) (QuantumScale*GetPixelGreen(image,p));
              *q++=(double) (QuantumScale*GetPixelBlue(image,p));
              *q++=0.0;
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
          for (i=0; i < (ssize_t) length; i++)
          {
            *q=0;
            switch (quantum_map[i])
            {
              case RedQuantum:
              case CyanQuantum:
              {
                *q=(double) (QuantumScale*GetPixelRed(image,p));
                break;
              }
              case GreenQuantum:
              case MagentaQuantum:
              {
                *q=(double) (QuantumScale*GetPixelGreen(image,p));
                break;
              }
              case BlueQuantum:
              case YellowQuantum:
              {
                *q=(double) (QuantumScale*GetPixelBlue(image,p));
                break;
              }
              case AlphaQuantum:
              {
                *q=(double) (QuantumScale*GetPixelAlpha(image,p));
                break;
              }
              case OpacityQuantum:
              {
                *q=(double) (QuantumScale*GetPixelAlpha(image,p));
                break;
              }
              case BlackQuantum:
              {
                if (image->colorspace == CMYKColorspace)
                  *q=(double) (QuantumScale*
                    GetPixelBlack(image,p));
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
          p+=GetPixelChannels(image);
        }
      }
      break;
    }
    case FloatPixel:
    {
      register float
        *q;

      q=(float *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(float) (QuantumScale*GetPixelBlue(image,p));
              *q++=(float) (QuantumScale*GetPixelGreen(image,p));
              *q++=(float) (QuantumScale*GetPixelRed(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(float) (QuantumScale*GetPixelBlue(image,p));
              *q++=(float) (QuantumScale*GetPixelGreen(image,p));
              *q++=(float) (QuantumScale*GetPixelRed(image,p));
              *q++=(float) (QuantumScale*GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(float) (QuantumScale*GetPixelBlue(image,p));
              *q++=(float) (QuantumScale*GetPixelGreen(image,p));
              *q++=(float) (QuantumScale*GetPixelRed(image,p));
              *q++=0.0;
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(float) (QuantumScale*GetPixelIntensity(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(float) (QuantumScale*GetPixelRed(image,p));
              *q++=(float) (QuantumScale*GetPixelGreen(image,p));
              *q++=(float) (QuantumScale*GetPixelBlue(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(float) (QuantumScale*GetPixelRed(image,p));
              *q++=(float) (QuantumScale*GetPixelGreen(image,p));
              *q++=(float) (QuantumScale*GetPixelBlue(image,p));
              *q++=(float) (QuantumScale*GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(float) (QuantumScale*GetPixelRed(image,p));
              *q++=(float) (QuantumScale*GetPixelGreen(image,p));
              *q++=(float) (QuantumScale*GetPixelBlue(image,p));
              *q++=0.0;
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
          for (i=0; i < (ssize_t) length; i++)
          {
            *q=0;
            switch (quantum_map[i])
            {
              case RedQuantum:
              case CyanQuantum:
              {
                *q=(float) (QuantumScale*GetPixelRed(image,p));
                break;
              }
              case GreenQuantum:
              case MagentaQuantum:
              {
                *q=(float) (QuantumScale*GetPixelGreen(image,p));
                break;
              }
              case BlueQuantum:
              case YellowQuantum:
              {
                *q=(float) (QuantumScale*GetPixelBlue(image,p));
                break;
              }
              case AlphaQuantum:
              {
                *q=(float) (QuantumScale*((Quantum) (GetPixelAlpha(image,p))));
                break;
              }
              case OpacityQuantum:
              {
                *q=(float) (QuantumScale*GetPixelAlpha(image,p));
                break;
              }
              case BlackQuantum:
              {
                if (image->colorspace == CMYKColorspace)
                  *q=(float) (QuantumScale* GetPixelBlack(image,p));
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
          p+=GetPixelChannels(image);
        }
      }
      break;
    }
    case IntegerPixel:
    {
      register unsigned int
        *q;

      q=(unsigned int *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(image,p));
              *q++=0U;
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(unsigned int) ScaleQuantumToLong(
                GetPixelIntensity(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(image,p));
              *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(image,p));
              *q++=0U;
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
          for (i=0; i < (ssize_t) length; i++)
          {
            *q=0;
            switch (quantum_map[i])
            {
              case RedQuantum:
              case CyanQuantum:
              {
                *q=(unsigned int) ScaleQuantumToLong(GetPixelRed(image,p));
                break;
              }
              case GreenQuantum:
              case MagentaQuantum:
              {
                *q=(unsigned int) ScaleQuantumToLong(GetPixelGreen(image,p));
                break;
              }
              case BlueQuantum:
              case YellowQuantum:
              {
                *q=(unsigned int) ScaleQuantumToLong(GetPixelBlue(image,p));
                break;
              }
              case AlphaQuantum:
              {
                *q=(unsigned int) ScaleQuantumToLong(GetPixelAlpha(image,p));
                break;
              }
              case OpacityQuantum:
              {
                *q=(unsigned int) ScaleQuantumToLong(GetPixelAlpha(image,p));
                break;
              }
              case BlackQuantum:
              {
                if (image->colorspace == CMYKColorspace)
                  *q=(unsigned int) ScaleQuantumToLong(GetPixelBlack(image,p));
                break;
              }
              case IndexQuantum:
              {
                *q=(unsigned int) ScaleQuantumToLong(
                  GetPixelIntensity(image,p));
                break;
              }
              default:
                *q=0;
            }
            q++;
          }
          p+=GetPixelChannels(image);
        }
      }
      break;
    }
    case LongPixel:
    {
      register size_t
        *q;

      q=(size_t *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToLong(GetPixelBlue(image,p));
              *q++=ScaleQuantumToLong(GetPixelGreen(image,p));
              *q++=ScaleQuantumToLong(GetPixelRed(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToLong(GetPixelBlue(image,p));
              *q++=ScaleQuantumToLong(GetPixelGreen(image,p));
              *q++=ScaleQuantumToLong(GetPixelRed(image,p));
              *q++=ScaleQuantumToLong(GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToLong(GetPixelBlue(image,p));
              *q++=ScaleQuantumToLong(GetPixelGreen(image,p));
              *q++=ScaleQuantumToLong(GetPixelRed(image,p));
              *q++=0;
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToLong(GetPixelIntensity(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToLong(GetPixelRed(image,p));
              *q++=ScaleQuantumToLong(GetPixelGreen(image,p));
              *q++=ScaleQuantumToLong(GetPixelBlue(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToLong(GetPixelRed(image,p));
              *q++=ScaleQuantumToLong(GetPixelGreen(image,p));
              *q++=ScaleQuantumToLong(GetPixelBlue(image,p));
              *q++=ScaleQuantumToLong(GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToLong(GetPixelRed(image,p));
              *q++=ScaleQuantumToLong(GetPixelGreen(image,p));
              *q++=ScaleQuantumToLong(GetPixelBlue(image,p));
              *q++=0;
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
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
                *q=ScaleQuantumToLong(GetPixelIntensity(image,p));
                break;
              }
              default:
                break;
            }
            q++;
          }
          p+=GetPixelChannels(image);
        }
      }
      break;
    }
    case QuantumPixel:
    {
      register Quantum
        *q;

      q=(Quantum *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=GetPixelBlue(image,p);
              *q++=GetPixelGreen(image,p);
              *q++=GetPixelRed(image,p);
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=GetPixelBlue(image,p);
              *q++=GetPixelGreen(image,p);
              *q++=GetPixelRed(image,p);
              *q++=(Quantum) (GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=GetPixelBlue(image,p);
              *q++=GetPixelGreen(image,p);
              *q++=GetPixelRed(image,p);
              *q++=(Quantum) 0;
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=GetPixelIntensity(image,p);
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=GetPixelRed(image,p);
              *q++=GetPixelGreen(image,p);
              *q++=GetPixelBlue(image,p);
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=GetPixelRed(image,p);
              *q++=GetPixelGreen(image,p);
              *q++=GetPixelBlue(image,p);
              *q++=(Quantum) (GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=GetPixelRed(image,p);
              *q++=GetPixelGreen(image,p);
              *q++=GetPixelBlue(image,p);
              *q++=(Quantum) 0;
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
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
                *q=(Quantum) (GetPixelAlpha(image,p));
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
                *q=(GetPixelIntensity(image,p));
                break;
              }
              default:
                *q=(Quantum) 0;
            }
            q++;
          }
          p+=GetPixelChannels(image);
        }
      }
      break;
    }
    case ShortPixel:
    {
      register unsigned short
        *q;

      q=(unsigned short *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToShort(GetPixelBlue(image,p));
              *q++=ScaleQuantumToShort(GetPixelGreen(image,p));
              *q++=ScaleQuantumToShort(GetPixelRed(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToShort(GetPixelBlue(image,p));
              *q++=ScaleQuantumToShort(GetPixelGreen(image,p));
              *q++=ScaleQuantumToShort(GetPixelRed(image,p));
              *q++=ScaleQuantumToShort(GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToShort(GetPixelBlue(image,p));
              *q++=ScaleQuantumToShort(GetPixelGreen(image,p));
              *q++=ScaleQuantumToShort(GetPixelRed(image,p));
              *q++=0;
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToShort(GetPixelIntensity(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToShort(GetPixelRed(image,p));
              *q++=ScaleQuantumToShort(GetPixelGreen(image,p));
              *q++=ScaleQuantumToShort(GetPixelBlue(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToShort(GetPixelRed(image,p));
              *q++=ScaleQuantumToShort(GetPixelGreen(image,p));
              *q++=ScaleQuantumToShort(GetPixelBlue(image,p));
              *q++=ScaleQuantumToShort(GetPixelAlpha(image,p));
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              *q++=ScaleQuantumToShort(GetPixelRed(image,p));
              *q++=ScaleQuantumToShort(GetPixelGreen(image,p));
              *q++=ScaleQuantumToShort(GetPixelBlue(image,p));
              *q++=0;
              p+=GetPixelChannels(image);
            }
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        p=GetVirtualPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
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
                *q=ScaleQuantumToShort(GetPixelIntensity(image,p));
                break;
              }
              default:
                break;
            }
            q++;
          }
          p+=GetPixelChannels(image);
        }
      }
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
%    o image: the image.
%
%    o pixel: Specifies a pointer to a PixelInfo structure.
%
*/
MagickExport void GetPixelInfo(const Image *image,PixelInfo *pixel)
{
  pixel->storage_class=DirectClass;
  pixel->colorspace=RGBColorspace;
  pixel->matte=MagickFalse;
  pixel->fuzz=0.0;
  pixel->depth=MAGICKCORE_QUANTUM_DEPTH;
  pixel->red=0.0;
  pixel->green=0.0;
  pixel->blue=0.0;
  pixel->black=0.0;
  pixel->alpha=(MagickRealType) OpaqueAlpha;
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
%   I m p o r t I m a g e P i x e l s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImportImagePixels() accepts pixel data and stores in the image at the
%  location you specify.  The method returns MagickTrue on success otherwise
%  MagickFalse if an error is encountered.  The pixel data can be either char,
%  short int, int, ssize_t, float, or double in the order specified by map.
%
%  Suppose your want to upload the first scanline of a 640x480 image from
%  character data in red-green-blue order:
%
%      ImportImagePixels(image,0,0,640,1,"RGB",CharPixel,pixels);
%
%  The format of the ImportImagePixels method is:
%
%      MagickBooleanType ImportImagePixels(Image *image,const ssize_t x_offset,
%        const ssize_t y_offset,const size_t columns,
%        const size_t rows,const char *map,const StorageType type,
%        const void *pixels,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x_offset,y_offset,columns,rows:  These values define the perimeter
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
%      types: CharPixel, ShortPixel, IntegerPixel, LongPixel, FloatPixel, or
%      DoublePixel.
%
%    o pixels: This array of values contain the pixel components as defined by
%      map and type.  You must preallocate this array where the expected
%      length varies depending on the values of width, height, map, and type.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ImportImagePixels(Image *image,
  const ssize_t x_offset,const ssize_t y_offset,const size_t columns,
  const size_t rows,const char *map,const StorageType type,
  const void *pixels,ExceptionInfo *exception)
{
  QuantumType
    *quantum_map;

  register Quantum
    *q;

  register ssize_t
    i,
    x;

  size_t
    length;

  ssize_t
    y;

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
    Transfer the pixels from the pixel datarray to the image.
  */
  switch (type)
  {
    case CharPixel:
    {
      register const unsigned char
        *p;

      p=(const unsigned char *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
              SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
              SetPixelRed(image,ScaleCharToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
              SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
              SetPixelRed(image,ScaleCharToQuantum(*p++),q);
              SetPixelAlpha(image,ScaleCharToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRO") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
              SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
              SetPixelRed(image,ScaleCharToQuantum(*p++),q);
              SetPixelAlpha(image,ScaleCharToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
              SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
              SetPixelRed(image,ScaleCharToQuantum(*p++),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleCharToQuantum(*p++),q);
              SetPixelGreen(image,GetPixelRed(image,q),q);
              SetPixelBlue(image,GetPixelRed(image,q),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleCharToQuantum(*p++),q);
              SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
              SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleCharToQuantum(*p++),q);
              SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
              SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
              SetPixelAlpha(image,ScaleCharToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBO") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleCharToQuantum(*p++),q);
              SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
              SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
              SetPixelAlpha(image,ScaleCharToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleCharToQuantum(*p++),q);
              SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
              SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
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
                SetPixelRed(image,ScaleCharToQuantum(*p),q);
                SetPixelGreen(image,GetPixelRed(image,q),q);
                SetPixelBlue(image,GetPixelRed(image,q),q);
                break;
              }
              default:
                break;
            }
            p++;
          }
          q+=GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      break;
    }
    case DoublePixel:
    {
      register const double
        *p;

      p=(const double *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelGreen(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelRed(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelGreen(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelRed(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelAlpha(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelGreen(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelRed(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              SetPixelGreen(image,GetPixelRed(image,q),q);
              SetPixelBlue(image,GetPixelRed(image,q),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelGreen(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelBlue(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelGreen(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelBlue(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelAlpha(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelGreen(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelBlue(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
          for (i=0; i < (ssize_t) length; i++)
          {
            switch (quantum_map[i])
            {
              case RedQuantum:
              case CyanQuantum:
              {
                SetPixelRed(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                break;
              }
              case GreenQuantum:
              case MagentaQuantum:
              {
                SetPixelGreen(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                break;
              }
              case BlueQuantum:
              case YellowQuantum:
              {
                SetPixelBlue(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                break;
              }
              case AlphaQuantum:
              {
                SetPixelAlpha(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                break;
              }
              case OpacityQuantum:
              {
                SetPixelAlpha(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                break;
              }
              case BlackQuantum:
              {
                SetPixelBlack(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                break;
              }
              case IndexQuantum:
              {
                SetPixelRed(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                SetPixelGreen(image,GetPixelRed(image,q),q);
                SetPixelBlue(image,GetPixelRed(image,q),q);
                break;
              }
              default:
                break;
            }
            p++;
          }
          q+=GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      break;
    }
    case FloatPixel:
    {
      register const float
        *p;

      p=(const float *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelGreen(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelRed(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelGreen(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelRed(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelAlpha(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelGreen(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelRed(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              SetPixelGreen(image,GetPixelRed(image,q),q);
              SetPixelBlue(image,GetPixelRed(image,q),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelGreen(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelBlue(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ClampToQuantum((MagickRealType)
                QuantumRange*(*p)),q);
              p++;
              SetPixelGreen(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelBlue(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelAlpha(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelGreen(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              SetPixelBlue(image,ClampToQuantum((MagickRealType) QuantumRange*
                (*p)),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
          for (i=0; i < (ssize_t) length; i++)
          {
            switch (quantum_map[i])
            {
              case RedQuantum:
              case CyanQuantum:
              {
                SetPixelRed(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                break;
              }
              case GreenQuantum:
              case MagentaQuantum:
              {
                SetPixelGreen(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                break;
              }
              case BlueQuantum:
              case YellowQuantum:
              {
                SetPixelBlue(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                break;
              }
              case AlphaQuantum:
              {
                SetPixelAlpha(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                break;
              }
              case OpacityQuantum:
              {
                SetPixelAlpha(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                break;
              }
              case BlackQuantum:
              {
                SetPixelBlack(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                break;
              }
              case IndexQuantum:
              {
                SetPixelRed(image,ClampToQuantum((MagickRealType)
                  QuantumRange*(*p)),q);
                SetPixelGreen(image,GetPixelRed(image,q),q);
                SetPixelBlue(image,GetPixelRed(image,q),q);
                break;
              }
              default:
                break;
            }
            p++;
          }
          q+=GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      break;
    }
    case IntegerPixel:
    {
      register const unsigned int
        *p;

      p=(const unsigned int *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              SetPixelAlpha(image,ScaleLongToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,GetPixelRed(image,q),q);
              SetPixelBlue(image,GetPixelRed(image,q),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
              SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
              SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
              SetPixelAlpha(image,ScaleLongToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
              SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
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
                SetPixelRed(image,ScaleLongToQuantum(*p),q);
                SetPixelGreen(image,GetPixelRed(image,q),q);
                SetPixelBlue(image,GetPixelRed(image,q),q);
                break;
              }
              default:
                break;
            }
            p++;
          }
          q+=GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      break;
    }
    case LongPixel:
    {
      register const unsigned int
        *p;

      p=(const unsigned int *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              SetPixelAlpha(image,ScaleLongToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,GetPixelRed(image,q),q);
              SetPixelBlue(image,GetPixelRed(image,q),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
              SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
              SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
              SetPixelAlpha(image,ScaleLongToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleLongToQuantum(*p++),q);
              SetPixelGreen(image,ScaleLongToQuantum(*p++),q);
              SetPixelBlue(image,ScaleLongToQuantum(*p++),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
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
                SetPixelRed(image,ScaleLongToQuantum(*p),q);
                SetPixelGreen(image,GetPixelRed(image,q),q);
                SetPixelBlue(image,GetPixelRed(image,q),q);
                break;
              }
              default:
                break;
            }
            p++;
          }
          q+=GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      break;
    }
    case QuantumPixel:
    {
      register const Quantum
        *p;

      p=(const Quantum *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,*p++,q);
              SetPixelGreen(image,*p++,q);
              SetPixelRed(image,*p++,q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,*p++,q);
              SetPixelGreen(image,*p++,q);
              SetPixelRed(image,*p++,q);
              SetPixelAlpha(image,*p++,q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,*p++,q);
              SetPixelGreen(image,*p++,q);
              SetPixelRed(image,*p++,q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,*p++,q);
              SetPixelGreen(image,GetPixelRed(image,q),q);
              SetPixelBlue(image,GetPixelRed(image,q),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,*p++,q);
              SetPixelGreen(image,*p++,q);
              SetPixelBlue(image,*p++,q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,*p++,q);
              SetPixelGreen(image,*p++,q);
              SetPixelBlue(image,*p++,q);
              SetPixelAlpha(image,*p++,q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,*p++,q);
              SetPixelGreen(image,*p++,q);
              SetPixelBlue(image,*p++,q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
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
                SetPixelRed(image,*p,q);
                SetPixelGreen(image,GetPixelRed(image,q),q);
                SetPixelBlue(image,GetPixelRed(image,q),q);
                break;
              }
              default:
                break;
            }
            p++;
          }
          q+=GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      break;
    }
    case ShortPixel:
    {
      register const unsigned short
        *p;

      p=(const unsigned short *) pixels;
      if (LocaleCompare(map,"BGR") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
              SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
              SetPixelRed(image,ScaleShortToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
              SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
              SetPixelRed(image,ScaleShortToQuantum(*p++),q);
              SetPixelAlpha(image,ScaleShortToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"BGRP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
              SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
              SetPixelRed(image,ScaleShortToQuantum(*p++),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"I") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleShortToQuantum(*p++),q);
              SetPixelGreen(image,GetPixelRed(image,q),q);
              SetPixelBlue(image,GetPixelRed(image,q),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGB") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleShortToQuantum(*p++),q);
              SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
              SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBA") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleShortToQuantum(*p++),q);
              SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
              SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
              SetPixelAlpha(image,ScaleShortToQuantum(*p++),q);
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      if (LocaleCompare(map,"RGBP") == 0)
        {
          for (y=0; y < (ssize_t) rows; y++)
          {
            q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) columns; x++)
            {
              SetPixelRed(image,ScaleShortToQuantum(*p++),q);
              SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
              SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
          break;
        }
      for (y=0; y < (ssize_t) rows; y++)
      {
        q=GetAuthenticPixels(image,x_offset,y_offset+y,columns,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) columns; x++)
        {
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
                SetPixelRed(image,ScaleShortToQuantum(*p),q);
                SetPixelGreen(image,GetPixelRed(image,q),q);
                SetPixelBlue(image,GetPixelRed(image,q),q);
                break;
              }
              default:
                break;
            }
            p++;
          }
          q+=GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
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
+   I n i t i a l i z e P i x e l C h a n n e l M a p                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializePixelChannelMap() defines the standard pixel component map.
%
%  The format of the InitializePixelChannelMap() method is:
%
%      void InitializePixelChannelMap(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void InitializePixelChannelMap(Image *image)
{
  PixelTrait
    trait;

  register ssize_t
    i;

  ssize_t
    n;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  (void) ResetMagickMemory(image->channel_map,0,MaxPixelChannels*
    sizeof(*image->channel_map));
  trait=UpdatePixelTrait;
  if (image->matte != MagickFalse)
    trait=(PixelTrait) (trait | BlendPixelTrait);
  n=0;
  if (0 && image->colorspace == GRAYColorspace)
    {
      SetPixelChannelMap(image,BluePixelChannel,trait,n);
      SetPixelChannelMap(image,GreenPixelChannel,trait,n);
      SetPixelChannelMap(image,RedPixelChannel,trait,n++);
    }
  else
    {
      SetPixelChannelMap(image,RedPixelChannel,trait,n++);
      SetPixelChannelMap(image,GreenPixelChannel,trait,n++);
      SetPixelChannelMap(image,BluePixelChannel,trait,n++);
    }
  if (image->colorspace == CMYKColorspace)
    SetPixelChannelMap(image,BlackPixelChannel,trait,n++);
  if (image->matte != MagickFalse)
    SetPixelChannelMap(image,AlphaPixelChannel,CopyPixelTrait,n++);
  if (image->storage_class == PseudoClass)
    SetPixelChannelMap(image,IndexPixelChannel,CopyPixelTrait,n++);
  assert((n+image->number_meta_channels) < MaxPixelChannels);
  for (i=0; i < (ssize_t) image->number_meta_channels; i++)
    SetPixelChannelMap(image,(PixelChannel) (MetaPixelChannel+i),CopyPixelTrait,
      n++);
  image->number_channels=(size_t) n;
  if (image->debug != MagickFalse)
    LogPixelChannels(image);
  (void) SetPixelChannelMask(image,image->channel_mask);
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
%  The format of the InterpolatePixelChannel method is:
%
%      MagickBooleanType InterpolatePixelChannel(const Image *image,
%        const CacheView *image_view,const PixelChannel channel,
%        const PixelInterpolateMethod method,const double x,const double y,
%        double *pixel,ExceptionInfo *exception)
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

static inline double MagickMax(const MagickRealType x,const MagickRealType y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline MagickRealType CubicWeightingFunction(const MagickRealType x)
{
  MagickRealType
    alpha,
    gamma;

  alpha=MagickMax(x+2.0,0.0);
  gamma=1.0*alpha*alpha*alpha;
  alpha=MagickMax(x+1.0,0.0);
  gamma-=4.0*alpha*alpha*alpha;
  alpha=MagickMax(x+0.0,0.0);
  gamma+=6.0*alpha*alpha*alpha;
  alpha=MagickMax(x-1.0,0.0);
  gamma-=4.0*alpha*alpha*alpha;
  return(gamma/6.0);
}

static inline double MeshInterpolate(const PointInfo *delta,const double p,
  const double x,const double y)
{
  return(delta->x*x+delta->y*y+(1.0-delta->x-delta->y)*p);
}

static inline ssize_t NearestNeighbor(const MagickRealType x)
{
  if (x >= 0.0)
    return((ssize_t) (x+0.5));
  return((ssize_t) (x-0.5));
}

MagickExport MagickBooleanType InterpolatePixelChannel(const Image *image,
  const CacheView *image_view,const PixelChannel channel,
  const PixelInterpolateMethod method,const double x,const double y,
  double *pixel,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickRealType
    alpha[16],
    gamma,
    pixels[16];

  PixelTrait
    traits;

  register const Quantum
    *p;

  register ssize_t
    i;

  ssize_t
    x_offset,
    y_offset;

  assert(image != (Image *) NULL);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image_view != (CacheView *) NULL);
  status=MagickTrue;
  *pixel=0.0;
  traits=GetPixelChannelMapTraits(image,channel);
  x_offset=(ssize_t) floor(x);
  y_offset=(ssize_t) floor(y);
  switch (method == UndefinedInterpolatePixel ? image->interpolate : method)
  {
    case AverageInterpolatePixel:
    {
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
          pixels[i]=(MagickRealType) p[i*GetPixelChannels(image)+channel];
        }
      else
        for (i=0; i < 16; i++)
        {
          alpha[i]=QuantumScale*GetPixelAlpha(image,p+i*
            GetPixelChannels(image));
          pixels[i]=alpha[i]*p[i*GetPixelChannels(image)+channel];
        }
      for (i=0; i < 16; i++)
      {
        gamma=1.0/(fabs((double) alpha[i]) <= MagickEpsilon ? 1.0 : alpha[i]);
        *pixel+=gamma*0.0625*pixels[i];
      }
      break;
    }
    case BicubicInterpolatePixel:
    {
      MagickRealType
        u[4],
        v[4];

      PointInfo
        delta;

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
          pixels[i]=(MagickRealType) p[i*GetPixelChannels(image)+channel];
        }
      else
        for (i=0; i < 16; i++)
        {
          alpha[i]=QuantumScale*GetPixelAlpha(image,p+i*
            GetPixelChannels(image));
          pixels[i]=alpha[i]*p[i*GetPixelChannels(image)+channel];
        }
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      for (i=0; i < 4; i++)
      {
        u[0]=(pixels[4*i+3]-pixels[4*i+2])-(pixels[4*i+0]-pixels[4*i+1]);
        u[1]=(pixels[4*i+0]-pixels[4*i+1])-u[0];
        u[2]=pixels[4*i+2]-pixels[4*i+0];
        u[3]=pixels[4*i+1];
        v[i]=(delta.x*delta.x*delta.x*u[0])+(delta.x*delta.x*u[1])+(delta.x*
          u[2])+u[3];
      }
      u[0]=(v[3]-v[2])-(v[0]-v[1]);
      u[1]=(v[0]-v[1])-u[0];
      u[2]=v[2]-v[0];
      u[3]=v[1];
      *pixel=(delta.y*delta.y*delta.y*u[0])+(delta.y*delta.y*u[1])+(delta.y*
        u[2])+u[3];
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
          pixels[i]=(MagickRealType) p[i*GetPixelChannels(image)+channel];
        }
      else
        for (i=0; i < 4; i++)
        {
          alpha[i]=QuantumScale*GetPixelAlpha(image,p+i*
            GetPixelChannels(image));
          pixels[i]=alpha[i]*p[i*GetPixelChannels(image)+channel];
        }
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      epsilon.x=1.0-delta.x;
      epsilon.y=1.0-delta.y;
      gamma=((epsilon.y*(epsilon.x*alpha[0]+delta.x*alpha[1])+delta.y*
        (epsilon.x*alpha[2]+delta.x*alpha[3])));
      gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
      *pixel=gamma*(epsilon.y*(epsilon.x*pixels[0]+delta.x*pixels[1])+delta.y*
        (epsilon.x*pixels[2]+delta.x*pixels[3]));
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
      filter_view=AcquireCacheView(filter_image);
      p=GetCacheViewVirtualPixels(filter_view,0,0,1,1,exception);
      if (p == (const Quantum *) NULL)
        status=MagickFalse;
      else
        *pixel=(double) GetPixelChannel(image,channel,p);
      filter_view=DestroyCacheView(filter_view);
      filter_image=DestroyImage(filter_image);
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
    case NearestNeighborInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(image_view,NearestNeighbor(x),
        NearestNeighbor(y),1,1,exception);
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
          pixels[i]=(MagickRealType) p[i*GetPixelChannels(image)+channel];
        }
      else
        for (i=0; i < 4; i++)
        {
          alpha[i]=QuantumScale*GetPixelAlpha(image,p+i*
            GetPixelChannels(image));
          pixels[i]=alpha[i]*p[i*GetPixelChannels(image)+channel];
        }
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      luminance.x=GetPixelLuminance(image,p)-(double)
        GetPixelLuminance(image,p+3*GetPixelChannels(image));
      luminance.y=GetPixelLuminance(image,p+GetPixelChannels(image))-(double)
        GetPixelLuminance(image,p+2*GetPixelChannels(image));
      if (fabs(luminance.x) < fabs(luminance.y))
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
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
              *pixel=gamma*MeshInterpolate(&delta,pixels[3],pixels[2],
                pixels[1]);
            }
        }
      break;
    }
    case SplineInterpolatePixel:
    {
      MagickRealType
        dx,
        dy;

      PointInfo
        delta;

      ssize_t
        j,
        n;

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
          pixels[i]=(MagickRealType) p[i*GetPixelChannels(image)+channel];
        }
      else
        for (i=0; i < 16; i++)
        {
          alpha[i]=QuantumScale*GetPixelAlpha(image,p+i*
            GetPixelChannels(image));
          pixels[i]=alpha[i]*p[i*GetPixelChannels(image)+channel];
        }
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      n=0;
      for (i=(-1); i < 3L; i++)
      {
        dy=CubicWeightingFunction((MagickRealType) i-delta.y);
        for (j=(-1); j < 3L; j++)
        {
          dx=CubicWeightingFunction(delta.x-(MagickRealType) j);
          gamma=1.0/(fabs((double) alpha[n]) <= MagickEpsilon ? 1.0 : alpha[n]);
          *pixel+=gamma*dx*dy*pixels[n];
          n++;
        }
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
%  The format of the InterpolatePixelChannels method is:
%
%      MagickBooleanType InterpolatePixelChannels(const Image *source,
%        const CacheView *source_view,const Image *destination,
%        const PixelInterpolateMethod method,const double x,const double y,
%        Quantum *pixel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o source: the source.
%
%    o source_view: the source view.
%
%    o destination: the destination image.
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
MagickExport MagickBooleanType InterpolatePixelChannels(const Image *source,
  const CacheView *source_view,const Image *destination,
  const PixelInterpolateMethod method,const double x,const double y,
  Quantum *pixel,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickRealType
    alpha[16],
    gamma,
    pixels[16];

  PixelChannel
    channel;

  PixelTrait
    destination_traits,
    traits;

  register const Quantum
    *p;

  register ssize_t
    i;

  ssize_t
    x_offset,
    y_offset;

  assert(source != (Image *) NULL);
  assert(source != (Image *) NULL);
  assert(source->signature == MagickSignature);
  assert(source_view != (CacheView *) NULL);
  status=MagickTrue;
  x_offset=(ssize_t) floor(x);
  y_offset=(ssize_t) floor(y);
  switch (method == UndefinedInterpolatePixel ? source->interpolate : method)
  {
    case AverageInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(source_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        double
          sum;

        register ssize_t
          j;

        channel=GetPixelChannelMapChannel(source,i);
        traits=GetPixelChannelMapTraits(source,channel);
        destination_traits=GetPixelChannelMapTraits(destination,channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        for (j=0; j < 16; j++)
          pixels[j]=(MagickRealType) p[j*GetPixelChannels(source)+i];
        sum=0.0;
        if ((traits & BlendPixelTrait) == 0)
          {
            for (j=0; j < 16; j++)
              sum+=0.0625*pixels[j];
            SetPixelChannel(destination,channel,ClampToQuantum(sum),pixel);
            continue;
          }
        for (j=0; j < 16; j++)
        {
          alpha[j]=QuantumScale*GetPixelAlpha(source,p+j*
            GetPixelChannels(source));
          pixels[j]*=alpha[j];
          gamma=1.0/(fabs((double) alpha[j]) <= MagickEpsilon ? 1.0 : alpha[j]);
          sum+=gamma*0.0625*pixels[j];
        }
        SetPixelChannel(destination,channel,ClampToQuantum(sum),pixel);
      }
      break;
    }
    case BicubicInterpolatePixel:
    {
      MagickRealType
        u[4],
        v[4];

      PointInfo
        delta;

      p=GetCacheViewVirtualPixels(source_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        register ssize_t
          j;

        channel=GetPixelChannelMapChannel(source,i);
        traits=GetPixelChannelMapTraits(source,channel);
        destination_traits=GetPixelChannelMapTraits(destination,channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        if ((traits & BlendPixelTrait) == 0)
          for (j=0; j < 16; j++)
          {
            alpha[j]=1.0;
            pixels[j]=(MagickRealType) p[j*GetPixelChannels(source)+i];
          }
        else
          for (j=0; j < 16; j++)
          {
            alpha[j]=QuantumScale*GetPixelAlpha(source,p+j*
              GetPixelChannels(source));
            pixels[j]=alpha[j]*p[j*GetPixelChannels(source)+i];
          }
        delta.x=x-x_offset;
        delta.y=y-y_offset;
        for (j=0; j < 4; j++)
        {
          u[0]=(pixels[4*j+3]-pixels[4*j+2])-(pixels[4*j+0]-pixels[4*j+1]);
          u[1]=(pixels[4*j+0]-pixels[4*j+1])-u[0];
          u[2]=pixels[4*j+2]-pixels[4*j+0];
          u[3]=pixels[4*j+1];
          v[j]=(delta.x*delta.x*delta.x*u[0])+(delta.x*delta.x*u[1])+(delta.x*
            u[2])+u[3];
        }
        u[0]=(v[3]-v[2])-(v[0]-v[1]);
        u[1]=(v[0]-v[1])-u[0];
        u[2]=v[2]-v[0];
        u[3]=v[1];
        SetPixelChannel(destination,channel,ClampToQuantum((delta.y*delta.y*
          delta.y*u[0])+(delta.y*delta.y*u[1])+(delta.y*u[2])+u[3]),pixel);
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

        channel=GetPixelChannelMapChannel(source,i);
        traits=GetPixelChannelMapTraits(source,channel);
        destination_traits=GetPixelChannelMapTraits(destination,channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        delta.x=x-x_offset;
        delta.y=y-y_offset;
        epsilon.x=1.0-delta.x;
        epsilon.y=1.0-delta.y;
        pixels[0]=(MagickRealType) p[i];
        pixels[1]=(MagickRealType) p[GetPixelChannels(source)+i];
        pixels[2]=(MagickRealType) p[2*GetPixelChannels(source)+i];
        pixels[3]=(MagickRealType) p[3*GetPixelChannels(source)+i];
        if ((traits & BlendPixelTrait) == 0)
          {
            gamma=((epsilon.y*(epsilon.x+delta.x)+delta.y*(epsilon.x+delta.x)));
            gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
            SetPixelChannel(destination,channel,ClampToQuantum(gamma*(epsilon.y*
              (epsilon.x*pixels[0]+delta.x*pixels[1])+delta.y*(epsilon.x*
              pixels[2]+delta.x*pixels[3]))),pixel);
            continue;
          }
        alpha[0]=QuantumScale*GetPixelAlpha(source,p);
        alpha[1]=QuantumScale*GetPixelAlpha(source,p+GetPixelChannels(source));
        alpha[2]=QuantumScale*GetPixelAlpha(source,p+2*
          GetPixelChannels(source));
        alpha[3]=QuantumScale*GetPixelAlpha(source,p+3*
          GetPixelChannels(source));
        pixels[0]*=alpha[0];
        pixels[1]*=alpha[1];
        pixels[2]*=alpha[2];
        pixels[3]*=alpha[3];
        gamma=((epsilon.y*(epsilon.x*alpha[0]+delta.x*alpha[1])+delta.y*
          (epsilon.x*alpha[2]+delta.x*alpha[3])));
        gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
        SetPixelChannel(destination,channel,ClampToQuantum(gamma*(epsilon.y*
          (epsilon.x*pixels[0]+delta.x*pixels[1])+delta.y*(epsilon.x*pixels[2]+
          delta.x*pixels[3]))),pixel);
      }
      break;
    }
    case FilterInterpolatePixel:
    {
      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        CacheView
          *filter_view;

        Image
          *excerpt_source,
          *filter_source;

        RectangleInfo
          geometry;

        channel=GetPixelChannelMapChannel(source,i);
        traits=GetPixelChannelMapTraits(source,channel);
        destination_traits=GetPixelChannelMapTraits(destination,channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        geometry.width=4L;
        geometry.height=4L;
        geometry.x=x_offset-1;
        geometry.y=y_offset-1;
        excerpt_source=ExcerptImage(source,&geometry,exception);
        if (excerpt_source == (Image *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        filter_source=ResizeImage(excerpt_source,1,1,source->filter,
          source->blur,exception);
        excerpt_source=DestroyImage(excerpt_source);
        if (filter_source == (Image *) NULL)
          continue;
        filter_view=AcquireCacheView(filter_source);
        p=GetCacheViewVirtualPixels(filter_view,0,0,1,1,exception);
        if (p == (const Quantum *) NULL)
          status=MagickFalse;
        else
          {
            SetPixelChannel(destination,channel,p[i],pixel);
          }
        filter_view=DestroyCacheView(filter_view);
        filter_source=DestroyImage(filter_source);
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
        channel=GetPixelChannelMapChannel(source,i);
        traits=GetPixelChannelMapTraits(source,channel);
        destination_traits=GetPixelChannelMapTraits(destination,channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(destination,channel,p[i],pixel);
      }
      break;
    }
    case NearestNeighborInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(source_view,NearestNeighbor(x),
        NearestNeighbor(y),1,1,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        channel=GetPixelChannelMapChannel(source,i);
        traits=GetPixelChannelMapTraits(source,channel);
        destination_traits=GetPixelChannelMapTraits(destination,channel);
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

        channel=GetPixelChannelMapChannel(source,i);
        traits=GetPixelChannelMapTraits(source,channel);
        destination_traits=GetPixelChannelMapTraits(destination,channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        pixels[0]=(MagickRealType) p[i];
        pixels[1]=(MagickRealType) p[GetPixelChannels(source)+i];
        pixels[2]=(MagickRealType) p[2*GetPixelChannels(source)+i];
        pixels[3]=(MagickRealType) p[3*GetPixelChannels(source)+i];
        if ((traits & BlendPixelTrait) == 0)
          {
            alpha[0]=1.0;
            alpha[1]=1.0;
            alpha[2]=1.0;
            alpha[3]=1.0;
          }
        else
          {
            alpha[0]=QuantumScale*GetPixelAlpha(source,p);
            alpha[1]=QuantumScale*GetPixelAlpha(source,p+
              GetPixelChannels(source));
            alpha[2]=QuantumScale*GetPixelAlpha(source,p+2*
              GetPixelChannels(source));
            alpha[3]=QuantumScale*GetPixelAlpha(source,p+3*
              GetPixelChannels(source));
          }
        delta.x=x-x_offset;
        delta.y=y-y_offset;
        luminance.x=GetPixelLuminance(source,p)-(double)
          GetPixelLuminance(source,p+3*GetPixelChannels(source));
        luminance.y=GetPixelLuminance(source,p+GetPixelChannels(source))-
          (double) GetPixelLuminance(source,p+2*GetPixelChannels(source));
        if (fabs(luminance.x) < fabs(luminance.y))
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
                gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
                gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
                gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
                gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
                SetPixelChannel(destination,channel,ClampToQuantum(gamma*
                  MeshInterpolate(&delta,pixels[3],pixels[2],pixels[1])),pixel);
              }
          }
      }
      break;
    }
    case SplineInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(source_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        double
          sum;

        MagickRealType
          dx,
          dy;

        PointInfo
          delta;

        register ssize_t
          j;

        ssize_t
          k,
          n;

        channel=GetPixelChannelMapChannel(source,i);
        traits=GetPixelChannelMapTraits(source,channel);
        destination_traits=GetPixelChannelMapTraits(destination,channel);
        if ((traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        if ((traits & BlendPixelTrait) == 0)
          for (j=0; j < 16; j++)
          {
            alpha[j]=1.0;
            pixels[j]=(MagickRealType) p[j*GetPixelChannels(source)+i];
          }
        else
          for (j=0; j < 16; j++)
          {
            alpha[j]=QuantumScale*GetPixelAlpha(source,p+j*
              GetPixelChannels(source));
            pixels[j]=alpha[j]*p[j*GetPixelChannels(source)+i];
          }
        delta.x=x-x_offset;
        delta.y=y-y_offset;
        sum=0.0;
        n=0;
        for (j=(-1); j < 3L; j++)
        {
          dy=CubicWeightingFunction((MagickRealType) j-delta.y);
          for (k=(-1); k < 3L; k++)
          {
            dx=CubicWeightingFunction(delta.x-(MagickRealType) k);
            gamma=1.0/(fabs((double) alpha[n]) <= MagickEpsilon ? 1.0 :
              alpha[n]);
            sum+=gamma*dx*dy*pixels[n];
            n++;
          }
        }
        SetPixelChannel(destination,channel,p[i],pixel);
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
  const Quantum *pixel,PixelInfo *pixel_info,MagickRealType *alpha)
{
  if (image->matte == MagickFalse)
    {
      *alpha=1.0;
      pixel_info->red=(MagickRealType) GetPixelRed(image,pixel);
      pixel_info->green=(MagickRealType) GetPixelGreen(image,pixel);
      pixel_info->blue=(MagickRealType) GetPixelBlue(image,pixel);
      pixel_info->black=0.0;
      if (image->colorspace == CMYKColorspace)
        pixel_info->black=(MagickRealType) GetPixelBlack(image,pixel);
      pixel_info->alpha=(MagickRealType) GetPixelAlpha(image,pixel);
      return;
    }
  *alpha=QuantumScale*GetPixelAlpha(image,pixel);
  pixel_info->red=(*alpha*GetPixelRed(image,pixel));
  pixel_info->green=(*alpha*GetPixelGreen(image,pixel));
  pixel_info->blue=(*alpha*GetPixelBlue(image,pixel));
  pixel_info->black=0.0;
  if (image->colorspace == CMYKColorspace)
    pixel_info->black=(*alpha*GetPixelBlack(image,pixel));
  pixel_info->alpha=(MagickRealType) GetPixelAlpha(image,pixel);
}

static void BicubicInterpolate(const PixelInfo *pixels,const double dx,
  PixelInfo *pixel)
{
  MagickRealType
    dx2,
    p,
    q,
    r,
    s;

  dx2=dx*dx;
  p=(pixels[3].red-pixels[2].red)-(pixels[0].red-pixels[1].red);
  q=(pixels[0].red-pixels[1].red)-p;
  r=pixels[2].red-pixels[0].red;
  s=pixels[1].red;
  pixel->red=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
  p=(pixels[3].green-pixels[2].green)-(pixels[0].green-pixels[1].green);
  q=(pixels[0].green-pixels[1].green)-p;
  r=pixels[2].green-pixels[0].green;
  s=pixels[1].green;
  pixel->green=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
  p=(pixels[3].blue-pixels[2].blue)-(pixels[0].blue-pixels[1].blue);
  q=(pixels[0].blue-pixels[1].blue)-p;
  r=pixels[2].blue-pixels[0].blue;
  s=pixels[1].blue;
  pixel->blue=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
  p=(pixels[3].alpha-pixels[2].alpha)-(pixels[0].alpha-pixels[1].alpha);
  q=(pixels[0].alpha-pixels[1].alpha)-p;
  r=pixels[2].alpha-pixels[0].alpha;
  s=pixels[1].alpha;
  pixel->alpha=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
  if (pixel->colorspace == CMYKColorspace)
    {
      p=(pixels[3].black-pixels[2].black)-(pixels[0].black-pixels[1].black);
      q=(pixels[0].black-pixels[1].black)-p;
      r=pixels[2].black-pixels[0].black;
      s=pixels[1].black;
      pixel->black=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
    }
}

MagickExport MagickBooleanType InterpolatePixelInfo(const Image *image,
  const CacheView *image_view,const PixelInterpolateMethod method,
  const double x,const double y,PixelInfo *pixel,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickRealType
    alpha[16],
    gamma;

  PixelInfo
    pixels[16];

  register const Quantum
    *p;

  register ssize_t
    i;

  ssize_t
    x_offset,
    y_offset;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image_view != (CacheView *) NULL);
  status=MagickTrue;
  x_offset=(ssize_t) floor(x);
  y_offset=(ssize_t) floor(y);
  switch (method == UndefinedInterpolatePixel ? image->interpolate : method)
  {
    case AverageInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(image_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      AlphaBlendPixelInfo(image,p,pixels+0,alpha+0);
      AlphaBlendPixelInfo(image,p+GetPixelChannels(image),pixels+1,alpha+1);
      AlphaBlendPixelInfo(image,p+2*GetPixelChannels(image),pixels+2,alpha+2);
      AlphaBlendPixelInfo(image,p+3*GetPixelChannels(image),pixels+3,alpha+3);
      AlphaBlendPixelInfo(image,p+4*GetPixelChannels(image),pixels+4,alpha+4);
      AlphaBlendPixelInfo(image,p+5*GetPixelChannels(image),pixels+5,alpha+5);
      AlphaBlendPixelInfo(image,p+6*GetPixelChannels(image),pixels+6,alpha+6);
      AlphaBlendPixelInfo(image,p+7*GetPixelChannels(image),pixels+7,alpha+7);
      AlphaBlendPixelInfo(image,p+8*GetPixelChannels(image),pixels+8,alpha+8);
      AlphaBlendPixelInfo(image,p+9*GetPixelChannels(image),pixels+9,alpha+9);
      AlphaBlendPixelInfo(image,p+10*GetPixelChannels(image),pixels+10,alpha+
        10);
      AlphaBlendPixelInfo(image,p+11*GetPixelChannels(image),pixels+11,alpha+
        11);
      AlphaBlendPixelInfo(image,p+12*GetPixelChannels(image),pixels+12,alpha+
        12);
      AlphaBlendPixelInfo(image,p+13*GetPixelChannels(image),pixels+13,alpha+
        13);
      AlphaBlendPixelInfo(image,p+14*GetPixelChannels(image),pixels+14,alpha+
        14);
      AlphaBlendPixelInfo(image,p+15*GetPixelChannels(image),pixels+15,alpha+
        15);
      pixel->red=0.0;
      pixel->green=0.0;
      pixel->blue=0.0;
      pixel->black=0.0;
      pixel->alpha=0.0;
      for (i=0; i < 16L; i++)
      {
        gamma=1.0/(fabs((double) alpha[i]) <= MagickEpsilon ? 1.0 : alpha[i]);
        pixel->red+=gamma*0.0625*pixels[i].red;
        pixel->green+=gamma*0.0625*pixels[i].green;
        pixel->blue+=gamma*0.0625*pixels[i].blue;
        if (image->colorspace == CMYKColorspace)
          pixel->black+=gamma*0.0625*pixels[i].black;
        pixel->alpha+=0.0625*pixels[i].alpha;
      }
      break;
    }
    case BicubicInterpolatePixel:
    {
      PixelInfo
        u[4];

      PointInfo
        delta;

      p=GetCacheViewVirtualPixels(image_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      AlphaBlendPixelInfo(image,p,pixels+0,alpha+0);
      AlphaBlendPixelInfo(image,p+GetPixelChannels(image),pixels+1,alpha+1);
      AlphaBlendPixelInfo(image,p+2*GetPixelChannels(image),pixels+2,alpha+2);
      AlphaBlendPixelInfo(image,p+3*GetPixelChannels(image),pixels+3,alpha+3);
      AlphaBlendPixelInfo(image,p+4*GetPixelChannels(image),pixels+4,alpha+4);
      AlphaBlendPixelInfo(image,p+5*GetPixelChannels(image),pixels+5,alpha+5);
      AlphaBlendPixelInfo(image,p+6*GetPixelChannels(image),pixels+6,alpha+6);
      AlphaBlendPixelInfo(image,p+7*GetPixelChannels(image),pixels+7,alpha+7);
      AlphaBlendPixelInfo(image,p+8*GetPixelChannels(image),pixels+8,alpha+8);
      AlphaBlendPixelInfo(image,p+9*GetPixelChannels(image),pixels+9,alpha+9);
      AlphaBlendPixelInfo(image,p+10*GetPixelChannels(image),pixels+10,alpha+
        10);
      AlphaBlendPixelInfo(image,p+11*GetPixelChannels(image),pixels+11,alpha+
        11);
      AlphaBlendPixelInfo(image,p+12*GetPixelChannels(image),pixels+12,alpha+
        12);
      AlphaBlendPixelInfo(image,p+13*GetPixelChannels(image),pixels+13,alpha+
        13);
      AlphaBlendPixelInfo(image,p+14*GetPixelChannels(image),pixels+14,alpha+
        14);
      AlphaBlendPixelInfo(image,p+15*GetPixelChannels(image),pixels+15,alpha+
        15);
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      for (i=0; i < 4L; i++)
        BicubicInterpolate(pixels+4*i,delta.x,u+i);
      BicubicInterpolate(u,delta.y,pixel);
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
      AlphaBlendPixelInfo(image,p,pixels+0,alpha+0);
      AlphaBlendPixelInfo(image,p+GetPixelChannels(image),pixels+1,alpha+1);
      AlphaBlendPixelInfo(image,p+2*GetPixelChannels(image),pixels+2,alpha+2);
      AlphaBlendPixelInfo(image,p+3*GetPixelChannels(image),pixels+3,alpha+3);
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      epsilon.x=1.0-delta.x;
      epsilon.y=1.0-delta.y;
      gamma=((epsilon.y*(epsilon.x*alpha[0]+delta.x*alpha[1])+delta.y*
        (epsilon.x*alpha[2]+delta.x*alpha[3])));
      gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
      gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
      pixel->alpha=(epsilon.y*(epsilon.x*pixels[0].alpha+delta.x*
        pixels[1].alpha)+delta.y*(epsilon.x*pixels[2].alpha+delta.x*
        pixels[3].alpha));
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
      filter_view=AcquireCacheView(filter_image);
      p=GetCacheViewVirtualPixels(filter_view,0,0,1,1,exception);
      if (p != (const Quantum *) NULL)
        GetPixelInfoPixel(image,p,pixel);
      filter_view=DestroyCacheView(filter_view);
      filter_image=DestroyImage(filter_image);
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
      luminance.x=GetPixelLuminance(image,p)-(double)
        GetPixelLuminance(image,p+3*GetPixelChannels(image));
      luminance.y=GetPixelLuminance(image,p+GetPixelChannels(image))-(double)
        GetPixelLuminance(image,p+2*GetPixelChannels(image));
      AlphaBlendPixelInfo(image,p,pixels+0,alpha+0);
      AlphaBlendPixelInfo(image,p+GetPixelChannels(image),pixels+1,alpha+1);
      AlphaBlendPixelInfo(image,p+2*GetPixelChannels(image),pixels+2,alpha+2);
      AlphaBlendPixelInfo(image,p+3*GetPixelChannels(image),pixels+3,alpha+3);
      if (fabs(luminance.x) < fabs(luminance.y))
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
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
    case NearestNeighborInterpolatePixel:
    {
      p=GetCacheViewVirtualPixels(image_view,NearestNeighbor(x),
        NearestNeighbor(y),1,1,exception);
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
      MagickRealType
        dx,
        dy;

      PointInfo
        delta;

      ssize_t
        j,
        n;

      p=GetCacheViewVirtualPixels(image_view,x_offset-1,y_offset-1,4,4,
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      AlphaBlendPixelInfo(image,p,pixels+0,alpha+0);
      AlphaBlendPixelInfo(image,p+GetPixelChannels(image),pixels+1,alpha+1);
      AlphaBlendPixelInfo(image,p+2*GetPixelChannels(image),pixels+2,alpha+2);
      AlphaBlendPixelInfo(image,p+3*GetPixelChannels(image),pixels+3,alpha+3);
      AlphaBlendPixelInfo(image,p+4*GetPixelChannels(image),pixels+4,alpha+4);
      AlphaBlendPixelInfo(image,p+5*GetPixelChannels(image),pixels+5,alpha+5);
      AlphaBlendPixelInfo(image,p+6*GetPixelChannels(image),pixels+6,alpha+6);
      AlphaBlendPixelInfo(image,p+7*GetPixelChannels(image),pixels+7,alpha+7);
      AlphaBlendPixelInfo(image,p+8*GetPixelChannels(image),pixels+8,alpha+8);
      AlphaBlendPixelInfo(image,p+9*GetPixelChannels(image),pixels+9,alpha+9);
      AlphaBlendPixelInfo(image,p+10*GetPixelChannels(image),pixels+10,alpha+
        10);
      AlphaBlendPixelInfo(image,p+11*GetPixelChannels(image),pixels+11,alpha+
        11);
      AlphaBlendPixelInfo(image,p+12*GetPixelChannels(image),pixels+12,alpha+
        12);
      AlphaBlendPixelInfo(image,p+13*GetPixelChannels(image),pixels+13,alpha+
        13);
      AlphaBlendPixelInfo(image,p+14*GetPixelChannels(image),pixels+14,alpha+
        14);
      AlphaBlendPixelInfo(image,p+15*GetPixelChannels(image),pixels+15,alpha+
        15);
      pixel->red=0.0;
      pixel->green=0.0;
      pixel->blue=0.0;
      pixel->black=0.0;
      pixel->alpha=0.0;
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      n=0;
      for (i=(-1); i < 3L; i++)
      {
        dy=CubicWeightingFunction((MagickRealType) i-delta.y);
        for (j=(-1); j < 3L; j++)
        {
          dx=CubicWeightingFunction(delta.x-(MagickRealType) j);
          gamma=1.0/(fabs((double) alpha[n]) <= MagickEpsilon ? 1.0 : alpha[n]);
          pixel->red+=gamma*dx*dy*pixels[n].red;
          pixel->green+=gamma*dx*dy*pixels[n].green;
          pixel->blue+=gamma*dx*dy*pixels[n].blue;
          if (image->colorspace == CMYKColorspace)
            pixel->black+=gamma*dx*dy*pixels[n].black;
          pixel->alpha+=dx*dy*pixels[n].alpha;
          n++;
        }
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
+   I s F u z z y E q u i v a l e n c e P i x e l                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsFuzzyEquivalencePixel() returns MagickTrue if the distance between two
%  pixels is less than the specified distance in a linear three (or four)u
%  dimensional color space.
%
%  The format of the IsFuzzyEquivalencePixel method is:
%
%      void IsFuzzyEquivalencePixel(const Image *source,const Quantum *p,
%        const Image *destination,const Quantum *q)
%
%  A description of each parameter follows:
%
%    o source: the source image.
%
%    o p: Pixel p.
%
%    o destination: the destination image.
%
%    o q: Pixel q.
%
*/
MagickExport MagickBooleanType IsFuzzyEquivalencePixel(const Image *source,
  const Quantum *p,const Image *destination,const Quantum *q)
{
  MagickRealType
    fuzz,
    pixel;

  register MagickRealType
    distance,
    scale;

  fuzz=MagickMax(source->fuzz,(MagickRealType) MagickSQ1_2)*MagickMax(
    destination->fuzz,(MagickRealType) MagickSQ1_2);
  scale=1.0;
  distance=0.0;
  if (source->matte != MagickFalse)
    {
      /*
        Transparencies are involved - set alpha distance
      */
      pixel=(MagickRealType) ((source->matte != MagickFalse ?
        GetPixelAlpha(source,p) : OpaqueAlpha)-(source->matte != MagickFalse ?
        GetPixelAlpha(destination,q) : OpaqueAlpha));
      distance=pixel*pixel;
      if (distance > fuzz)
        return(MagickFalse);
      /*
        Generate a alpha scaling factor to generate a 4D cone on colorspace
        Note that if one color is transparent, distance has no color component.
      */
      scale=QuantumScale*GetPixelAlpha(source,p);
      scale*=QuantumScale*GetPixelAlpha(destination,q);
      if (scale <= MagickEpsilon)
        return(MagickTrue);
    }
  /*
    RGB or CMY color cube
  */
  distance*=3.0;  /* rescale appropriately */
  fuzz*=3.0;
  pixel=GetPixelRed(source,p)-(MagickRealType) GetPixelRed(destination,q);
  if ((source->colorspace == HSLColorspace) ||
      (source->colorspace == HSBColorspace) ||
      (source->colorspace == HWBColorspace))
    {
      /*
        Compute an arc distance for hue.  It should be a vector angle of
        'S'/'W' length with 'L'/'B' forming appropriate cones.
      */
      if (fabs((double) pixel) > (QuantumRange/2))
        pixel-=QuantumRange;
      pixel*=2;
    }
  distance+=scale*pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=GetPixelGreen(source,p)-(MagickRealType) GetPixelGreen(destination,q);
  distance+=scale*pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=GetPixelBlue(source,p)-(MagickRealType) GetPixelBlue(destination,q);
  distance+=scale*pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
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
%  See http://www.imagemagick.org/Usage/bugs/fuzz_distance/
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
  MagickRealType
    fuzz,
    pixel;

  register MagickRealType
    scale,
    distance;

  if ((p->fuzz == 0.0) && (q->fuzz == 0.0))
    return(IsPixelInfoEquivalent(p,q));
  if (p->fuzz == 0.0)
    fuzz=MagickMax(q->fuzz,(MagickRealType) MagickSQ1_2)*MagickMax(q->fuzz,
      (MagickRealType) MagickSQ1_2);
  else if (q->fuzz == 0.0)
    fuzz=MagickMax(p->fuzz,(MagickRealType) MagickSQ1_2)*MagickMax(p->fuzz,
      (MagickRealType) MagickSQ1_2);
  else
    fuzz=MagickMax(p->fuzz,(MagickRealType) MagickSQ1_2)*MagickMax(q->fuzz,
      (MagickRealType) MagickSQ1_2);
  scale=1.0;
  distance=0.0;
  if ((p->matte != MagickFalse) || (q->matte != MagickFalse))
    {
      /*
        Transparencies are involved - set alpha distance.
      */
      pixel=(p->matte != MagickFalse ? p->alpha : OpaqueAlpha)-
        (q->matte != MagickFalse ? q->alpha : OpaqueAlpha);
      distance=pixel*pixel;
      if (distance > fuzz)
        return(MagickFalse);
      /*
        Generate a alpha scaling factor to generate a 4D cone on colorspace.
        If one color is transparent, distance has no color component.
      */
      if (p->matte != MagickFalse)
        scale=(QuantumScale*p->alpha);
      if (q->matte != MagickFalse)
        scale*=(QuantumScale*q->alpha);
      if (scale <= MagickEpsilon )
        return(MagickTrue);
    }
  /*
    CMYK create a CMY cube with a multi-dimensional cone toward black.
  */
  if (p->colorspace == CMYKColorspace)
    {
      pixel=p->black-q->black;
      distance+=pixel*pixel*scale;
      if (distance > fuzz)
        return(MagickFalse);
      scale*=(MagickRealType) (QuantumScale*(QuantumRange-p->black));
      scale*=(MagickRealType) (QuantumScale*(QuantumRange-q->black));
    }
  /*
    RGB or CMY color cube.
  */
  distance*=3.0;  /* rescale appropriately */
  fuzz*=3.0;
  pixel=p->red-q->red;
  if ((p->colorspace == HSLColorspace) || (p->colorspace == HSBColorspace) ||
      (p->colorspace == HWBColorspace))
    {
      /*
        This calculates a arc distance for hue-- it should be a vector angle
        of 'S'/'W' length with 'L'/'B' forming appropriate cones.  In other
        words this is a hack - Anthony.
      */
      if (fabs((double) pixel) > (QuantumRange/2))
        pixel-=QuantumRange;
      pixel*=2;
    }
  distance+=pixel*pixel*scale;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=p->green-q->green;
  distance+=pixel*pixel*scale;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=p->blue-q->blue;
  distance+=pixel*pixel*scale;
  if (distance > fuzz)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t P i x e l C h a n n e l M a p M a s k                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetPixelChannelMapMask() sets the pixel channel map from the specified
%  channel mask.
%
%  The format of the SetPixelChannelMapMask method is:
%
%      void SetPixelChannelMapMask(Image *image,const ChannelType channel_mask)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o mask: the channel mask.
%
*/
MagickExport void SetPixelChannelMapMask(Image *image,
  const ChannelType channel_mask)
{
#define GetChannelBit(mask,bit)  (((size_t) (mask) >> (size_t) (bit)) & 0x01)

  register ssize_t
    i;

  image->channel_mask=channel_mask;
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel
      channel;

    channel=GetPixelChannelMapChannel(image,i);
    SetPixelChannelMapTraits(image,channel,
      GetChannelBit(channel_mask,channel) == 0 ? CopyPixelTrait :
      image->matte == MagickFalse || (channel == AlphaPixelChannel) ?
      UpdatePixelTrait : (PixelTrait) (UpdatePixelTrait | BlendPixelTrait));
  }
  if (image->storage_class == PseudoClass)
    SetPixelChannelMapTraits(image,IndexPixelChannel,CopyPixelTrait);
  if (image->debug != MagickFalse)
    LogPixelChannels(image);
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
%  SetPixelChannelMask() sets the pixel channel mask from the specified channel
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
MagickExport ChannelType SetPixelChannelMask(Image *image,
  const ChannelType channel_mask)
{
  ChannelType
    mask;

  mask=image->channel_mask;
  image->channel_mask=channel_mask;
  SetPixelChannelMapMask(image,channel_mask);
  return(mask);
}

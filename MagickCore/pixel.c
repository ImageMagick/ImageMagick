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
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e P i x e l C o m p o n e n t M a p                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquirePixelComponentMap() acquires a pixel component map.
%
%  The format of the AcquirePixelComponentMap() method is:
%
%      PixelComponentMap **AcquirePixelComponentMap(void)
%
*/
MagickExport PixelComponentMap **AcquirePixelComponentMap(void)
{
  PixelComponentMap
    **component_map;

  register ssize_t
    i;

  component_map=(PixelComponentMap **) AcquireAlignedMemory(
    MaxPixelComponentMaps,sizeof(**component_map));
  if (component_map == (PixelComponentMap **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  for (i=0; i < MaxPixelComponentMaps; i++)
  {
    register ssize_t
      j;

    component_map[i]=(PixelComponentMap *) AcquireQuantumMemory(
      MaxPixelComponents,sizeof(*component_map[i]));
    if (component_map[i] == (PixelComponentMap *) NULL)
      ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
    (void) ResetMagickMemory(component_map[i],0,MaxPixelComponents*
      sizeof(*component_map[i]));
    for (j=0; j < MaxPixelComponents; j++)
      component_map[i][j].component=(PixelComponent) j;
  }
  return(component_map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l o n e P i x e l C o m p o n e n t M a p                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClonePixelComponentMap() clones a pixel component map.
%
%  The format of the ClonePixelComponentMap() method is:
%
%      PixelComponentMap **ClonePixelComponentMap(
%        PixelComponentMap **component_map)
%
%  A description of each parameter follows:
%
%    o component_map: the pixel component map.
%
*/
MagickExport PixelComponentMap **ClonePixelComponentMap(
  PixelComponentMap **component_map)
{
  PixelComponentMap
    **clone_map;

  register ssize_t
    i;

  assert(component_map != (PixelComponentMap **) NULL);
  clone_map=AcquirePixelComponentMap();
  if (clone_map == (PixelComponentMap **) NULL)
    return((PixelComponentMap **) NULL);
  for (i=0; i < MaxPixelComponentMaps; i++)
    (void) CopyMagickMemory(clone_map[i],component_map[i],MaxPixelComponents*
      sizeof(*component_map[i]));
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

  pixel_info=(PixelInfo *) AcquireAlignedMemory(1,sizeof(*pixel_info));
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
+   D e s t r o y P i x e l C o m p o n e n t M a p                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyPixelComponentMap() deallocates memory associated with the pixel
%  component map.
%
%  The format of the DestroyPixelComponentMap() method is:
%
%      PixelComponentMap **DestroyPixelComponentMap(
%        PixelComponentMap **component_map)
%
%  A description of each parameter follows:
%
%    o component_map: the pixel component map.
%
*/
MagickExport PixelComponentMap **DestroyPixelComponentMap(
  PixelComponentMap **component_map)
{
  register ssize_t
    i;

  assert(component_map != (PixelComponentMap **) NULL);
  for (i=0; i < MaxPixelComponentMaps; i++)
    component_map[i]=(PixelComponentMap *) RelinquishMagickMemory(
      component_map[i]);
  return((PixelComponentMap **) RelinquishMagickMemory(component_map));
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
          p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
          p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
          p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
          p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
          p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
          p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
              p+=GetPixelComponents(image);
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
          p+=GetPixelComponents(image);
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
%   G e t M a g i c k P i x e l P a c k e t                                   %
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
%    o pixel: Specifies a pointer to a PixelPacket structure.
%
*/
MagickExport void GetPixelInfo(const Image *image,
  PixelInfo *pixel)
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
%        const void *pixels)
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
*/
MagickExport MagickBooleanType ImportImagePixels(Image *image,
  const ssize_t x_offset,const ssize_t y_offset,const size_t columns,
  const size_t rows,const char *map,const StorageType type,
  const void *pixels)
{
  ExceptionInfo
    *exception;

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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
          q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
          q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
          q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
          q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
          q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
          q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
              q+=GetPixelComponents(image);
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
          q+=GetPixelComponents(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      break;
    }
    default:
    {
      quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        OptionError,"UnrecognizedPixelMap","`%s'",map);
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
%  InterpolatePixelInfo() applies bi-linear or tri-linear interpolation
%  between a floating point coordinate and the pixels surrounding that
%  coordinate.  No pixel area resampling, or scaling of the result is
%  performed.
%
%  The format of the InterpolatePixelInfo method is:
%
%      MagickBooleanType InterpolatePixelInfo(const Image *image,
%        const CacheView *image_view,const InterpolatePixelMethod method,
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

MagickExport MagickBooleanType InterpolatePixelInfo(const Image *image,
  const CacheView *image_view,const InterpolatePixelMethod method,
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
      AlphaBlendPixelInfo(image,p+1*GetPixelComponents(image),pixels+1,alpha+1);
      AlphaBlendPixelInfo(image,p+2*GetPixelComponents(image),pixels+2,alpha+2);
      AlphaBlendPixelInfo(image,p+3*GetPixelComponents(image),pixels+3,alpha+3);
      AlphaBlendPixelInfo(image,p+4*GetPixelComponents(image),pixels+4,alpha+4);
      AlphaBlendPixelInfo(image,p+5*GetPixelComponents(image),pixels+5,alpha+5);
      AlphaBlendPixelInfo(image,p+6*GetPixelComponents(image),pixels+6,alpha+6);
      AlphaBlendPixelInfo(image,p+7*GetPixelComponents(image),pixels+7,alpha+7);
      AlphaBlendPixelInfo(image,p+8*GetPixelComponents(image),pixels+8,alpha+8);
      AlphaBlendPixelInfo(image,p+9*GetPixelComponents(image),pixels+9,alpha+9);
      AlphaBlendPixelInfo(image,p+10*GetPixelComponents(image),pixels+10,alpha+
        10);
      AlphaBlendPixelInfo(image,p+11*GetPixelComponents(image),pixels+11,alpha+
        11);
      AlphaBlendPixelInfo(image,p+12*GetPixelComponents(image),pixels+12,alpha+
        12);
      AlphaBlendPixelInfo(image,p+13*GetPixelComponents(image),pixels+13,alpha+
        13);
      AlphaBlendPixelInfo(image,p+14*GetPixelComponents(image),pixels+14,alpha+
        14);
      AlphaBlendPixelInfo(image,p+15*GetPixelComponents(image),pixels+15,alpha+
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
      AlphaBlendPixelInfo(image,p+1*GetPixelComponents(image),pixels+1,alpha+1);
      AlphaBlendPixelInfo(image,p+2*GetPixelComponents(image),pixels+2,alpha+2);
      AlphaBlendPixelInfo(image,p+3*GetPixelComponents(image),pixels+3,alpha+3);
      AlphaBlendPixelInfo(image,p+4*GetPixelComponents(image),pixels+4,alpha+4);
      AlphaBlendPixelInfo(image,p+5*GetPixelComponents(image),pixels+5,alpha+5);
      AlphaBlendPixelInfo(image,p+6*GetPixelComponents(image),pixels+6,alpha+6);
      AlphaBlendPixelInfo(image,p+7*GetPixelComponents(image),pixels+7,alpha+7);
      AlphaBlendPixelInfo(image,p+8*GetPixelComponents(image),pixels+8,alpha+8);
      AlphaBlendPixelInfo(image,p+9*GetPixelComponents(image),pixels+9,alpha+9);
      AlphaBlendPixelInfo(image,p+10*GetPixelComponents(image),pixels+10,alpha+
        10);
      AlphaBlendPixelInfo(image,p+11*GetPixelComponents(image),pixels+11,alpha+
        11);
      AlphaBlendPixelInfo(image,p+12*GetPixelComponents(image),pixels+12,alpha+
        12);
      AlphaBlendPixelInfo(image,p+13*GetPixelComponents(image),pixels+13,alpha+
        13);
      AlphaBlendPixelInfo(image,p+14*GetPixelComponents(image),pixels+14,alpha+
        14);
      AlphaBlendPixelInfo(image,p+15*GetPixelComponents(image),pixels+15,alpha+
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
      AlphaBlendPixelInfo(image,p+1*GetPixelComponents(image),pixels+1,alpha+1);
      AlphaBlendPixelInfo(image,p+2*GetPixelComponents(image),pixels+2,alpha+2);
      AlphaBlendPixelInfo(image,p+3*GetPixelComponents(image),pixels+3,alpha+3);
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
        SetPixelInfo(image,p,pixel);
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
      SetPixelInfo(image,p,pixel);
      break;
    }
    case MeshInterpolatePixel:
    {
      PointInfo
        delta,
        luminance;

      p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset,2,2,
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      AlphaBlendPixelInfo(image,p,pixels+0,alpha+0);
      AlphaBlendPixelInfo(image,p+1*GetPixelComponents(image),pixels+1,alpha+1);
      AlphaBlendPixelInfo(image,p+2*GetPixelComponents(image),pixels+2,alpha+2);
      AlphaBlendPixelInfo(image,p+3*GetPixelComponents(image),pixels+3,alpha+3);
      delta.x=x-x_offset;
      delta.y=y-y_offset;
      luminance.x=GetPixelInfoLuminance(pixels+0)-(double)
        GetPixelInfoLuminance(pixels+3);
      luminance.y=GetPixelInfoLuminance(pixels+1)-(double)
        GetPixelInfoLuminance(pixels+2);
      if (fabs(luminance.x) < fabs(luminance.y))
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
              pixel->alpha=gamma*MeshInterpolate(&delta,pixels[2].alpha,
                pixels[3].alpha,pixels[0].alpha);
            }
          else
            {
              /*
                Top-right triangle (pixel:1, diagonal: 0-3).
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
                Top-left triangle (pixel 0, diagonal: 1-2).
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
      SetPixelInfo(image,p,pixel);
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
      AlphaBlendPixelInfo(image,p+1*GetPixelComponents(image),pixels+1,alpha+1);
      AlphaBlendPixelInfo(image,p+2*GetPixelComponents(image),pixels+2,alpha+2);
      AlphaBlendPixelInfo(image,p+3*GetPixelComponents(image),pixels+3,alpha+3);
      AlphaBlendPixelInfo(image,p+4*GetPixelComponents(image),pixels+4,alpha+4);
      AlphaBlendPixelInfo(image,p+5*GetPixelComponents(image),pixels+5,alpha+5);
      AlphaBlendPixelInfo(image,p+6*GetPixelComponents(image),pixels+6,alpha+6);
      AlphaBlendPixelInfo(image,p+7*GetPixelComponents(image),pixels+7,alpha+7);
      AlphaBlendPixelInfo(image,p+8*GetPixelComponents(image),pixels+8,alpha+8);
      AlphaBlendPixelInfo(image,p+9*GetPixelComponents(image),pixels+9,alpha+9);
      AlphaBlendPixelInfo(image,p+10*GetPixelComponents(image),pixels+10,alpha+
        10);
      AlphaBlendPixelInfo(image,p+11*GetPixelComponents(image),pixels+11,alpha+
        11);
      AlphaBlendPixelInfo(image,p+12*GetPixelComponents(image),pixels+12,alpha+
        12);
      AlphaBlendPixelInfo(image,p+13*GetPixelComponents(image),pixels+13,alpha+
        13);
      AlphaBlendPixelInfo(image,p+14*GetPixelComponents(image),pixels+14,alpha+
        14);
      AlphaBlendPixelInfo(image,p+15*GetPixelComponents(image),pixels+15,alpha+
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
%      void IsFuzzyEquivalencePixel(const Image *image,const Quantum *p,
%        const Quantum *q)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o p: Pixel p.
%
%    o q: Pixel q.
%
*/
MagickExport MagickBooleanType IsFuzzyEquivalencePixel(const Image *image,
  const Quantum *p,const Quantum *q)
{
  MagickRealType
    fuzz,
    pixel;

  register MagickRealType
    distance,
    scale;

  fuzz=MagickMax(image->fuzz,(MagickRealType) MagickSQ1_2)*
    MagickMax(image->fuzz,(MagickRealType) MagickSQ1_2);
  scale=1.0;
  distance=0.0;
  if (image->matte != MagickFalse)
    {
      /*
        Transparencies are involved - set alpha distance
      */
      pixel=(MagickRealType) ((image->matte != MagickFalse ?
        GetPixelAlpha(image,p) : OpaqueAlpha)-(image->matte != MagickFalse ?
        GetPixelAlpha(image,q) : OpaqueAlpha));
      distance=pixel*pixel;
      if (distance > fuzz)
        return(MagickFalse);
      /*
        Generate a alpha scaling factor to generate a 4D cone on colorspace
        Note that if one color is transparent, distance has no color component.
      */
      scale=QuantumScale*GetPixelAlpha(image,p);
      scale*=QuantumScale*GetPixelAlpha(image,q);
      if (scale <= MagickEpsilon)
        return(MagickTrue);
    }
  /*
    RGB or CMY color cube
  */
  distance*=3.0;  /* rescale appropriately */
  fuzz*=3.0;
  pixel=GetPixelRed(image,p)-(MagickRealType) GetPixelRed(image,q);
  if ((image->colorspace == HSLColorspace) ||
      (image->colorspace == HSBColorspace) ||
      (image->colorspace == HWBColorspace))
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
  pixel=GetPixelGreen(image,p)-(MagickRealType) GetPixelGreen(image,q);
  distance+=scale*pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=GetPixelBlue(image,p)-(MagickRealType) GetPixelBlue(image,q);
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
%  This implements the equivalent of...
%    fuzz < sqrt( color_distance^2 * u.a*v.a  + alpha_distance^2 )
%
%  Which produces a multi-dimensional cone for that colorspace along the
%  transparency vector.
%
%  For example for an RGB
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
    fuzz=MagickMax(q->fuzz,(MagickRealType) MagickSQ1_2)*
      MagickMax(q->fuzz,(MagickRealType) MagickSQ1_2);
  else if (q->fuzz == 0.0)
    fuzz=MagickMax(p->fuzz,(MagickRealType) MagickSQ1_2)*
      MagickMax(p->fuzz,(MagickRealType) MagickSQ1_2);
  else
    fuzz=MagickMax(p->fuzz,(MagickRealType) MagickSQ1_2)*
      MagickMax(q->fuzz,(MagickRealType) MagickSQ1_2);
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
        Note that if one color is transparent, distance has no color component.
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
      /* This calculates a arc distance for hue
         Really if should be a vector angle of 'S'/'W' length
         with 'L'/'B' forming appropriate cones.
         In other words this is a hack - Anthony
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
+   I s F u z z y E q u i v a l e n c e P i x e l P a c k e t                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsFuzzyEquivalencePixelPacket() returns MagickTrue if the distance between
%  two pixels is less than the specified distance in a linear three (or four)
%  dimensional color space.
%
%  The format of the IsFuzzyEquivalencePixelPacket method is:
%
%      void IsFuzzyEquivalencePixelPacket(const Image *image,
%        const PixelPacket *p,const PixelPacket *q)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o p: Pixel p.
%
%    o q: Pixel q.
%
*/
MagickExport MagickBooleanType IsFuzzyEquivalencePixelPacket(const Image *image,
  const PixelPacket *p,const PixelPacket *q)
{
  MagickRealType
    fuzz,
    pixel;

  register MagickRealType
    distance,
    scale;

  if ((image->fuzz == 0.0) && (image->matte == MagickFalse))
    return(IsPixelPacketEquivalent(p,q));
  fuzz=MagickMax(image->fuzz,(MagickRealType) MagickSQ1_2)*
    MagickMax(image->fuzz,(MagickRealType) MagickSQ1_2);
  scale=1.0;
  distance=0.0;
  if (image->matte != MagickFalse)
    {
      /*
        Transparencies are involved - set alpha distance
      */
      pixel=(MagickRealType) ((image->matte != MagickFalse ? p->alpha :
        OpaqueAlpha)-(image->matte != MagickFalse ? q->alpha : OpaqueAlpha));
      distance=pixel*pixel;
      if (distance > fuzz)
        return(MagickFalse);
      /*
        Generate a alpha scaling factor to generate a 4D cone on colorspace
        Note that if one color is transparent, distance has no color component.
      */
      scale=QuantumScale*p->alpha;
      scale*=QuantumScale*q->alpha;
      if (scale <= MagickEpsilon)
        return(MagickTrue);
    }
  /*
    RGB or CMY color cube
  */
  distance*=3.0;  /* rescale appropriately */
  fuzz*=3.0;
  pixel=p->red-(MagickRealType) q->red;
  if ((image->colorspace == HSLColorspace) ||
      (image->colorspace == HSBColorspace) ||
      (image->colorspace == HWBColorspace))
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
  pixel=(MagickRealType) p->green-q->green;
  distance+=scale*pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=(MagickRealType) p->blue-q->blue;
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
%   P o p P i x e l C o m p o n e n t M a p                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PopPixelComponentMap() pops the pixel component map.
%
%  The format of the PopPixelComponentMap method is:
%
%      void PopPixelComponentMap(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void PopPixelComponentMap(Image *image)
{
  image->map--;
  if (image->map < 0)
    ThrowFatalException(ResourceLimitFatalError,"PixelComponentMapStack");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P u s h P i x e l C o m p o n e n t M a p                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PushPixelComponentMap() pushes the pixel component map from the specified
%  channel mask.
%
%  The format of the PushPixelComponentMap method is:
%
%      void PushPixelComponentMap(Image *image,const ChannelType channel_mask)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel_mask: the channel mask.
%
*/
MagickExport void PushPixelComponentMap(Image *image,
  const ChannelType channel_mask)
{
  image->map++;
  if (image->map >= MaxPixelComponentMaps)
    ThrowFatalException(ResourceLimitFatalError,"PixelComponentMapStack");
  SetPixelComponentMap(image,channel_mask);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t P i x e l C o m p o n e n t M a p                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetPixelComponentMap() sets the pixel component map from the specified
%  channel mask.
%
%  The format of the SetPixelComponentMap method is:
%
%      void SetPixelComponentMap(Image *image,const ChannelType channel_mask)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel_mask: the channel mask.
%
*/
MagickExport void SetPixelComponentMap(Image *image,
  const ChannelType channel_mask)
{
  register ssize_t
    i;

  for (i=0; i < MaxPixelComponents; i++)
    SetPixelComponentTraits(image,(PixelComponent) i,UndefinedPixelTrait);
  image->sync=(channel_mask & SyncChannels) != 0 ? MagickTrue : MagickFalse;
  if ((channel_mask & RedChannel) != 0)
    SetPixelRedTraits(image,ActivePixelTrait);
  if ((channel_mask & GreenChannel) != 0)
    SetPixelGreenTraits(image,ActivePixelTrait);
  if ((channel_mask & BlueChannel) != 0)
    SetPixelBlueTraits(image,ActivePixelTrait);
  if ((channel_mask & BlackChannel) != 0)
    SetPixelBlackTraits(image,ActivePixelTrait);
  if ((channel_mask & AlphaChannel) != 0)
    SetPixelAlphaTraits(image,ActivePixelTrait);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S t a n d a r d P i x e l C o m p o n e n t M a p                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StandardPixelComponentMap() defines the standard pixel component map.
%
%  The format of the StandardPixelComponentMap() method is:
%
%      void StandardPixelComponentMap(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void StandardPixelComponentMap(Image *image)
{
  image->pixel_components=4;
  SetPixelComponent(image,RedPixelComponent,RedPixelComponent);
  SetPixelComponentTraits(image,RedPixelComponent,ActivePixelTrait);
  SetPixelComponent(image,GreenPixelComponent,GreenPixelComponent);
  SetPixelComponentTraits(image,GreenPixelComponent,ActivePixelTrait);
  SetPixelComponent(image,BluePixelComponent,BluePixelComponent);
  SetPixelComponentTraits(image,BluePixelComponent,ActivePixelTrait);
  SetPixelComponent(image,AlphaPixelComponent,AlphaPixelComponent);
  SetPixelComponentTraits(image,AlphaPixelComponent,ActivePixelTrait);
  if (image->colorspace == CMYKColorspace)
    {
      image->pixel_components++;
      SetPixelComponent(image,BlackPixelComponent,BlackPixelComponent);
      SetPixelComponentTraits(image,BlackPixelComponent,ActivePixelTrait);
    }
  if (image->storage_class == PseudoClass)
    {
      image->pixel_components++;
      SetPixelComponent(image,IndexPixelComponent,IndexPixelComponent);
      SetPixelComponentTraits(image,IndexPixelComponent,ActivePixelTrait);
    }
}

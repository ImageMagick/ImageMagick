/*
  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore quantum inline methods.
*/
#ifndef MAGICKCORE_QUANTUM_H
#define MAGICKCORE_QUANTUM_H

#include "MagickCore/image.h"
#include "MagickCore/semaphore.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedEndian,
  LSBEndian,
  MSBEndian
} EndianType;

typedef enum
{
  UndefinedQuantumAlpha,
  AssociatedQuantumAlpha,
  DisassociatedQuantumAlpha
} QuantumAlphaType;

typedef enum
{
  UndefinedQuantumFormat,
  FloatingPointQuantumFormat,
  SignedQuantumFormat,
  UnsignedQuantumFormat
} QuantumFormatType;

typedef enum
{
  UndefinedQuantum,
  AlphaQuantum,
  BGRAQuantum,
  BGROQuantum,
  BGRQuantum,
  BlackQuantum,
  BlueQuantum,
  CbYCrAQuantum,
  CbYCrQuantum,
  CbYCrYQuantum,
  CMYKAQuantum,
  CMYKOQuantum,
  CMYKQuantum,
  CyanQuantum,
  GrayAlphaQuantum,
  GrayQuantum,
  GreenQuantum,
  IndexAlphaQuantum,
  IndexQuantum,
  MagentaQuantum,
  OpacityQuantum,
  RedQuantum,
  RGBAQuantum,
  RGBOQuantum,
  RGBPadQuantum,
  RGBQuantum,
  YellowQuantum
} QuantumType;

typedef struct _QuantumInfo
  QuantumInfo;

static inline Quantum ClampToQuantum(const MagickRealType value)
{
#if defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) value);
#else
  if (value <= 0.0f)
    return((Quantum) 0);
  if (value >= (MagickRealType) QuantumRange)
    return(QuantumRange);
  return((Quantum) (value+0.5f));
#endif
}

#if (MAGICKCORE_QUANTUM_DEPTH == 8)
static inline unsigned char ScaleQuantumToChar(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((unsigned char) quantum);
#else
  if (quantum <= 0.0)
    return(0);
  if (quantum >= 255.0)
    return(255);
  return((unsigned char) (quantum+0.5));
#endif
}
#elif (MAGICKCORE_QUANTUM_DEPTH == 16)
static inline unsigned char ScaleQuantumToChar(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((unsigned char) (((quantum+128UL)-((quantum+128UL) >> 8)) >> 8));
#else
  if (quantum <= 0.0)
    return(0);
  if ((quantum/257.0) >= 255.0)
    return(255);
  return((unsigned char) (quantum/257.0+0.5));
#endif
}
#elif (MAGICKCORE_QUANTUM_DEPTH == 32)
static inline unsigned char ScaleQuantumToChar(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((unsigned char) ((quantum+MagickULLConstant(8421504))/
    MagickULLConstant(16843009)));
#else
  if (quantum <= 0.0)
    return(0);
  if ((quantum/16843009.0) >= 255.0)
    return(255);
  return((unsigned char) (quantum/16843009.0+0.5));
#endif
}
#elif (MAGICKCORE_QUANTUM_DEPTH == 64)
static inline unsigned char ScaleQuantumToChar(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((unsigned char) (quantum/72340172838076673.0+0.5));
#else
  if (quantum <= 0.0)
    return(0);
  if ((quantum/72340172838076673.0) >= 255.0)
    return(255);
  return((unsigned char) (quantum/72340172838076673.0+0.5));
#endif
}
#endif

extern MagickExport EndianType
  GetQuantumEndian(const QuantumInfo *);

extern MagickExport MagickBooleanType
  SetQuantumDepth(const Image *,QuantumInfo *,const size_t),
  SetQuantumEndian(const Image *,QuantumInfo *,const EndianType),
  SetQuantumFormat(const Image *,QuantumInfo *,const QuantumFormatType),
  SetQuantumPad(const Image *,QuantumInfo *,const size_t);

extern MagickExport QuantumFormatType
  GetQuantumFormat(const QuantumInfo *);

extern MagickExport QuantumInfo
  *AcquireQuantumInfo(const ImageInfo *,Image *),
  *DestroyQuantumInfo(QuantumInfo *);

extern MagickExport QuantumType
  GetQuantumType(Image *,ExceptionInfo *);

extern MagickExport size_t
  ExportQuantumPixels(const Image *,CacheView *,QuantumInfo *,const QuantumType,
    unsigned char *magick_restrict,ExceptionInfo *),
  GetQuantumExtent(const Image *,const QuantumInfo *,const QuantumType),
  ImportQuantumPixels(const Image *,CacheView *,QuantumInfo *,const QuantumType,
    const unsigned char *magick_restrict,ExceptionInfo *);

extern MagickExport unsigned char
  *GetQuantumPixels(const QuantumInfo *);

extern MagickExport void
  GetQuantumInfo(const ImageInfo *,QuantumInfo *),
  SetQuantumAlphaType(QuantumInfo *,const QuantumAlphaType),
  SetQuantumImageType(Image *,const QuantumType),
  SetQuantumMinIsWhite(QuantumInfo *,const MagickBooleanType),
  SetQuantumPack(QuantumInfo *,const MagickBooleanType),
  SetQuantumQuantum(QuantumInfo *,const size_t),
  SetQuantumScale(QuantumInfo *,const double);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

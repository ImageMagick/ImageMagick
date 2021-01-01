/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore quantum inline methods.
*/
#ifndef MAGICKCORE_QUANTUM_PRIVATE_H
#define MAGICKCORE_QUANTUM_PRIVATE_H

#include "MagickCore/memory_.h"
#include "MagickCore/cache.h"
#include "MagickCore/image-private.h"
#include "MagickCore/pixel-accessor.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _QuantumState
{
  double
    inverse_scale;

  unsigned int
    pixel;

  size_t
    bits;

  const unsigned int
    *mask;
} QuantumState;

struct _QuantumInfo
{
  size_t
    depth,
    quantum;

  QuantumFormatType
    format;

  double
    minimum,
    maximum,
    scale;

  size_t
    pad;

  MagickBooleanType
    min_is_white,
    pack;

  QuantumAlphaType
    alpha_type;

  size_t
    number_threads;

  MemoryInfo
    **pixels;

  size_t
    extent;

  EndianType
    endian;

  QuantumState
    state;

  SemaphoreInfo
    *semaphore;

  size_t
    signature;
};

extern MagickPrivate void
  ResetQuantumState(QuantumInfo *);

static inline MagickSizeType GetQuantumRange(const size_t depth)
{
  MagickSizeType
    one;

  size_t
    max_depth;

  if (depth == 0)
    return(0);
  one=1;
  max_depth=8*sizeof(MagickSizeType);
  return((MagickSizeType) ((one << (MagickMin(depth,max_depth)-1))+
    ((one << (MagickMin(depth,max_depth)-1))-1)));
}

static inline float HalfToSinglePrecision(const unsigned short half)
{
#define ExponentBias  (127-15)
#define ExponentMask  0x7c00
#define ExponentShift  23
#define SignBitShift  31
#define SignificandShift  13
#define SignificandMask  0x00000400

  typedef union _SinglePrecision
  {
    unsigned int
      fixed_point;

    float
      single_precision;
  } SinglePrecision;

  unsigned int
    exponent,
    significand,
    sign_bit;

  SinglePrecision
    map;

  unsigned int
    value;

  /*
    The IEEE 754 standard specifies half precision as having:

      Sign bit: 1 bit
      Exponent width: 5 bits
      Significand precision: 11 (10 explicitly stored)
  */
  sign_bit=(unsigned int) ((half >> 15) & 0x00000001);
  exponent=(unsigned int) ((half >> 10) & 0x0000001f);
  significand=(unsigned int) (half & 0x000003ff);
  if (exponent == 0)
    {
      if (significand == 0)
        value=sign_bit << SignBitShift;
      else
        {
          while ((significand & SignificandMask) == 0)
          {
            significand<<=1;
            exponent--;
          }
          exponent++;
          significand&=(~SignificandMask);
          exponent+=ExponentBias;
          value=(sign_bit << SignBitShift) | (exponent << ExponentShift) |
            (significand << SignificandShift);
        }
    }
  else
    if (exponent == SignBitShift)
      {
        value=(sign_bit << SignBitShift) | 0x7f800000;
        if (significand != 0)
          value|=(significand << SignificandShift);
      }
    else
      {
        exponent+=ExponentBias;
        significand<<=SignificandShift;
        value=(sign_bit << SignBitShift) | (exponent << ExponentShift) |
          significand;
      }
  map.fixed_point=value;
  return(map.single_precision);
}

static inline unsigned char *PopCharPixel(const unsigned char pixel,
  unsigned char *magick_restrict pixels)
{
  *pixels++=pixel;
  return(pixels);
}

static inline unsigned char *PopLongPixel(const EndianType endian,
  const unsigned int pixel,unsigned char *magick_restrict pixels)
{
  unsigned int
    quantum;

  quantum=(unsigned int) pixel;
  if (endian == LSBEndian)
    {
      *pixels++=(unsigned char) (quantum);
      *pixels++=(unsigned char) (quantum >> 8);
      *pixels++=(unsigned char) (quantum >> 16);
      *pixels++=(unsigned char) (quantum >> 24);
      return(pixels);
    }
  *pixels++=(unsigned char) (quantum >> 24);
  *pixels++=(unsigned char) (quantum >> 16);
  *pixels++=(unsigned char) (quantum >> 8);
  *pixels++=(unsigned char) (quantum);
  return(pixels);
}

static inline unsigned char *PopShortPixel(const EndianType endian,
  const unsigned short pixel,unsigned char *magick_restrict pixels)
{
  unsigned int
    quantum;

  quantum=pixel;
  if (endian == LSBEndian)
    {
      *pixels++=(unsigned char) (quantum);
      *pixels++=(unsigned char) (quantum >> 8);
      return(pixels);
    }
  *pixels++=(unsigned char) (quantum >> 8);
  *pixels++=(unsigned char) (quantum);
  return(pixels);
}

static inline const unsigned char *PushCharPixel(
  const unsigned char *magick_restrict pixels,
  unsigned char *magick_restrict pixel)
{
  *pixel=(*pixels++);
  return(pixels);
}

static inline const unsigned char *PushLongPixel(const EndianType endian,
  const unsigned char *magick_restrict pixels,
  unsigned int *magick_restrict pixel)
{
  unsigned int
    quantum;

  if (endian == LSBEndian)
    {
      quantum=((unsigned int) *pixels++);
      quantum|=((unsigned int) *pixels++ << 8);
      quantum|=((unsigned int) *pixels++ << 16);
      quantum|=((unsigned int) *pixels++ << 24);
      *pixel=quantum;
      return(pixels);
    }
  quantum=((unsigned int) *pixels++ << 24);
  quantum|=((unsigned int) *pixels++ << 16);
  quantum|=((unsigned int) *pixels++ << 8);
  quantum|=((unsigned int) *pixels++);
  *pixel=quantum;
  return(pixels);
}

static inline const unsigned char *PushShortPixel(const EndianType endian,
  const unsigned char *magick_restrict pixels,
  unsigned short *magick_restrict pixel)
{
  unsigned int
    quantum;

  if (endian == LSBEndian)
    {
      quantum=(unsigned int) *pixels++;
      quantum|=(unsigned int) (*pixels++ << 8);
      *pixel=(unsigned short) (quantum & 0xffff);
      return(pixels);
    }
  quantum=(unsigned int) (*pixels++ << 8);
  quantum|=(unsigned int) *pixels++;
  *pixel=(unsigned short) (quantum & 0xffff);
  return(pixels);
}

static inline const unsigned char *PushFloatPixel(const EndianType endian,
  const unsigned char *magick_restrict pixels,
  MagickFloatType *magick_restrict pixel)
{
  union
  {
    unsigned int
      unsigned_value;

    MagickFloatType
      float_value;
  } quantum;

  if (endian == LSBEndian)
    {
      quantum.unsigned_value=((unsigned int) *pixels++);
      quantum.unsigned_value|=((unsigned int) *pixels++ << 8);
      quantum.unsigned_value|=((unsigned int) *pixels++ << 16);
      quantum.unsigned_value|=((unsigned int) *pixels++ << 24);
      *pixel=quantum.float_value;
      return(pixels);
    }
  quantum.unsigned_value=((unsigned int) *pixels++ << 24);
  quantum.unsigned_value|=((unsigned int) *pixels++ << 16);
  quantum.unsigned_value|=((unsigned int) *pixels++ << 8);
  quantum.unsigned_value|=((unsigned int) *pixels++);
  *pixel=quantum.float_value;
  return(pixels);
}

static inline Quantum ScaleAnyToQuantum(const QuantumAny quantum,
  const QuantumAny range)
{
  if (quantum > range)
    return(QuantumRange);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) ((double) QuantumRange*(quantum*
    PerceptibleReciprocal((double) range))+0.5));
#else
  return((Quantum) ((double) QuantumRange*(quantum*
    PerceptibleReciprocal((double) range))));
#endif
}

static inline QuantumAny ScaleQuantumToAny(const Quantum quantum,
  const QuantumAny range)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((QuantumAny) ((double) range*quantum/QuantumRange));
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return((QuantumAny) 0UL);
  if (((double) range*quantum/QuantumRange) >= 18446744073709551615.0)
    return((QuantumAny) MagickULLConstant(18446744073709551615));
  return((QuantumAny) ((double) range*quantum/QuantumRange+0.5));
#endif
}

#if (MAGICKCORE_QUANTUM_DEPTH == 8)
static inline Quantum ScaleCharToQuantum(const unsigned char value)
{
  return((Quantum) value);
}

static inline Quantum ScaleLongToQuantum(const unsigned int value)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) ((value)/16843009UL));
#else
  return((Quantum) (value/16843009.0));
#endif
}

static inline Quantum ScaleLongLongToQuantum(const MagickSizeType value)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (value/MagickULLConstant(72340172838076673)));
#else
  return((Quantum) (value/72340172838076673.0));
#endif
}

static inline Quantum ScaleMapToQuantum(const MagickRealType value)
{
  if (value <= 0.0)
    return((Quantum) 0);
  if (value >= MaxMap)
    return(QuantumRange);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (value+0.5));
#else
  return((Quantum) value);
#endif
}

static inline unsigned int ScaleQuantumToLong(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((unsigned int) (16843009UL*quantum));
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0U);
  if ((16843009.0*quantum) >= 4294967295.0)
    return(4294967295UL);
  return((unsigned int) (16843009.0*quantum+0.5));
#endif
}

static inline MagickSizeType ScaleQuantumToLongLong(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((MagickSizeType) (MagickULLConstant(72340172838076673)*quantum));
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0UL);
  if ((72340172838076673.0*quantum) >= 18446744073709551615.0)
    return(MagickULLConstant(18446744073709551615));
  return((MagickSizeType) (72340172838076673*quantum+0.5));
#endif
}

static inline unsigned int ScaleQuantumToMap(const Quantum quantum)
{
  if (quantum >= (Quantum) MaxMap)
    return((unsigned int) MaxMap);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((unsigned int) quantum);
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0U);
  return((unsigned int) (quantum+0.5));
#endif
}

static inline unsigned short ScaleQuantumToShort(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((unsigned short) (257UL*quantum));
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0);
  if ((257.0*quantum) >= 65535.0)
    return(65535);
  return((unsigned short) (257.0*quantum+0.5));
#endif
}

static inline Quantum ScaleShortToQuantum(const unsigned short value)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) ((value+128U)/257U));
#else
  return((Quantum) (value/257.0));
#endif
}
#elif (MAGICKCORE_QUANTUM_DEPTH == 16)
static inline Quantum ScaleCharToQuantum(const unsigned char value)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (257U*value));
#else
  return((Quantum) (257.0*value));
#endif
}

static inline Quantum ScaleLongToQuantum(const unsigned int value)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) ((value)/MagickULLConstant(65537)));
#else
  return((Quantum) (value/65537.0));
#endif
}

static inline Quantum ScaleLongLongToQuantum(const MagickSizeType value)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) ((value)/MagickULLConstant(281479271743489)));
#else
  return((Quantum) (value/281479271743489.0));
#endif
}

static inline Quantum ScaleMapToQuantum(const MagickRealType value)
{
  if (value <= 0.0)
    return((Quantum) 0);
  if (value >= MaxMap)
    return(QuantumRange);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (value+0.5));
#else
  return((Quantum) value);
#endif
}

static inline unsigned int ScaleQuantumToLong(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((unsigned int) (65537UL*quantum));
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0U);
  if ((65537.0*quantum) >= 4294967295.0)
    return(4294967295U);
  return((unsigned int) (65537.0*quantum+0.5));
#endif
}

static inline MagickSizeType ScaleQuantumToLongLong(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((MagickSizeType) (MagickULLConstant(281479271743489)*quantum));
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0UL);
  if ((281479271743489.0*quantum) >= 18446744073709551615.0)
    return(MagickULLConstant(18446744073709551615));
  return((MagickSizeType) (281479271743489.0*quantum+0.5));
#endif
}

static inline unsigned int ScaleQuantumToMap(const Quantum quantum)
{
  if (quantum >= (Quantum) MaxMap)
    return((unsigned int) MaxMap);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((unsigned int) quantum);
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0U);
  return((unsigned int) (quantum+0.5));
#endif
}

static inline unsigned short ScaleQuantumToShort(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((unsigned short) quantum);
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0);
  if (quantum >= 65535.0)
    return(65535);
  return((unsigned short) (quantum+0.5));
#endif
}

static inline Quantum ScaleShortToQuantum(const unsigned short value)
{
  return((Quantum) value);
}
#elif (MAGICKCORE_QUANTUM_DEPTH == 32)
static inline Quantum ScaleCharToQuantum(const unsigned char value)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (16843009UL*value));
#else
  return((Quantum) (16843009.0*value));
#endif
}

static inline Quantum ScaleLongToQuantum(const unsigned int value)
{
  return((Quantum) value);
}

static inline Quantum ScaleLongLongToQuantum(const MagickSizeType value)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) ((value)/MagickULLConstant(4294967297)));
#else
  return((Quantum) (value/4294967297.0));
#endif
}

static inline Quantum ScaleMapToQuantum(const MagickRealType value)
{
  if (value <= 0.0)
    return((Quantum) 0);
  if (value >= (Quantum) MaxMap)
    return(QuantumRange);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (65537.0*value+0.5));
#else
  return((Quantum) (65537.0*value));
#endif
}

static inline unsigned int ScaleQuantumToLong(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((unsigned int) quantum);
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0U);
  if ((quantum) >= 4294967295.0)
    return(4294967295);
  return((unsigned int) (quantum+0.5));
#endif
}

static inline MagickSizeType ScaleQuantumToLongLong(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((MagickSizeType) (MagickULLConstant(4294967297)*quantum));
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0UL);
  if ((4294967297.0*quantum) >= 18446744073709551615.0)
    return(MagickULLConstant(18446744073709551615));
  return((MagickSizeType) (4294967297.0*quantum+0.5));
#endif
}

static inline unsigned int ScaleQuantumToMap(const Quantum quantum)
{
  if ((quantum/65537) >= (Quantum) MaxMap)
    return((unsigned int) MaxMap);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((unsigned int) ((quantum+MagickULLConstant(32768))/
    MagickULLConstant(65537)));
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0U);
  return((unsigned int) (quantum/65537.0+0.5));
#endif
}

static inline unsigned short ScaleQuantumToShort(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((unsigned short) ((quantum+MagickULLConstant(32768))/
    MagickULLConstant(65537)));
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0);
  if ((quantum/65537.0) >= 65535.0)
    return(65535);
  return((unsigned short) (quantum/65537.0+0.5));
#endif
}

static inline Quantum ScaleShortToQuantum(const unsigned short value)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (65537UL*value));
#else
  return((Quantum) (65537.0*value));
#endif
}
#elif (MAGICKCORE_QUANTUM_DEPTH == 64)
static inline Quantum ScaleCharToQuantum(const unsigned char value)
{
  return((Quantum) (72340172838076673.0*value));
}

static inline Quantum ScaleLongToQuantum(const unsigned int value)
{
  return((Quantum) (4294967297.0*value));
}

static inline Quantum ScaleLongLongToQuantum(const MagickSizeType value)
{
  return((Quantum) (value));
}

static inline Quantum ScaleMapToQuantum(const MagickRealType value)
{
  if (value <= 0.0)
    return((Quantum) 0);
  if (value >= MaxMap)
    return(QuantumRange);
  return((Quantum) (281479271743489.0*value));
}

static inline unsigned int ScaleQuantumToLong(const Quantum quantum)
{
  return((unsigned int) (quantum/4294967297.0+0.5));
}

static inline MagickSizeType ScaleQuantumToLongLong(const Quantum quantum)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((MagickSizeType) quantum);
#else
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0UL);
  if (quantum >= 18446744073709551615.0)
    return(MagickULLConstant(18446744073709551615));
  return((MagickSizeType) (quantum+0.5));
#endif
}

static inline unsigned int ScaleQuantumToMap(const Quantum quantum)
{
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0U);
  if ((quantum/281479271743489.0) >= MaxMap)
    return((unsigned int) MaxMap);
  return((unsigned int) (quantum/281479271743489.0+0.5));
}

static inline unsigned short ScaleQuantumToShort(const Quantum quantum)
{
  if ((IsNaN(quantum) != 0) || (quantum <= 0.0))
    return(0);
  if ((quantum/281479271743489.0) >= 65535.0)
    return(65535);
  return((unsigned short) (quantum/281479271743489.0+0.5));
}

static inline Quantum ScaleShortToQuantum(const unsigned short value)
{
  return((Quantum) (281479271743489.0*value));
}
#endif

static inline unsigned short SinglePrecisionToHalf(const float value)
{
  typedef union _SinglePrecision
  {
    unsigned int
      fixed_point;

    float
      single_precision;
  } SinglePrecision;

  int
    exponent;

  unsigned int
    significand,
    sign_bit;

  SinglePrecision
    map;

  unsigned short
    half;

  /*
    The IEEE 754 standard specifies half precision as having:

      Sign bit: 1 bit
      Exponent width: 5 bits
      Significand precision: 11 (10 explicitly stored)
  */
  map.single_precision=value;
  sign_bit=(map.fixed_point >> 16) & 0x00008000;
  exponent=(int) ((map.fixed_point >> ExponentShift) & 0x000000ff)-ExponentBias;
  significand=map.fixed_point & 0x007fffff;
  if (exponent <= 0)
    {
      int
        shift;

      if (exponent < -10)
        return((unsigned short) sign_bit);
      significand=significand | 0x00800000;
      shift=(int) (14-exponent);
      significand=(unsigned int) ((significand+((1 << (shift-1))-1)+
        ((significand >> shift) & 0x01)) >> shift);
      return((unsigned short) (sign_bit | significand));
    }
  else
    if (exponent == (0xff-ExponentBias))
      {
        if (significand == 0)
          return((unsigned short) (sign_bit | ExponentMask));
        else
          {
            significand>>=SignificandShift;
            half=(unsigned short) (sign_bit | significand |
              (significand == 0) | ExponentMask);
            return(half);
          }
      }
  significand=significand+((significand >> SignificandShift) & 0x01)+0x00000fff;
  if ((significand & 0x00800000) != 0)
    {
      significand=0;
      exponent++;
    }
  if (exponent > 30)
    {
      float
        alpha;

      int
        i;

      /*
        Float overflow.
      */
      alpha=1.0e10;
      for (i=0; i < 10; i++)
        alpha*=alpha;
      return((unsigned short) (sign_bit | ExponentMask));
    }
  half=(unsigned short) (sign_bit | (exponent << 10) |
    (significand >> SignificandShift));
  return(half);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

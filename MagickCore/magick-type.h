/*
  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore types.
*/
#ifndef MAGICKCORE_MAGICK_TYPE_H
#define MAGICKCORE_MAGICK_TYPE_H

#include "MagickCore/magick-config.h"

#if MAGICKCORE_HAVE_UINTPTR_T
#  include <stdint.h>
#endif

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(MAGICKCORE_QUANTUM_DEPTH)
#define MAGICKCORE_QUANTUM_DEPTH  16
#endif

#if defined(MAGICKCORE_WINDOWS_SUPPORT) && !defined(__MINGW32__)
#  define MagickLLConstant(c)  ((MagickOffsetType) (c ## i64))
#  define MagickULLConstant(c)  ((MagickSizeType) (c ## ui64))
#else
#  define MagickLLConstant(c)  ((MagickOffsetType) (c ## LL))
#  define MagickULLConstant(c)  ((MagickSizeType) (c ## ULL))
#endif

#if MAGICKCORE_SIZEOF_FLOAT_T == 0
typedef float MagickFloatType;
#elif (MAGICKCORE_SIZEOF_FLOAT_T == MAGICKCORE_SIZEOF_FLOAT)
typedef float MagickFloatType;
#elif (MAGICKCORE_SIZEOF_FLOAT_T == MAGICKCORE_SIZEOF_DOUBLE)
typedef double MagickFloatType;
#elif (MAGICKCORE_SIZEOF_FLOAT_T == MAGICKCORE_SIZEOF_LONG_DOUBLE)
typedef double MagickFloatType;
#else
#error Your MagickFloatType type is neither a float, nor a double, nor a long double
#endif
#if MAGICKCORE_SIZEOF_DOUBLE_T == 0
typedef double MagickDoubleType;
#elif (MAGICKCORE_SIZEOF_DOUBLE_T == MAGICKCORE_SIZEOF_DOUBLE)
typedef double MagickDoubleType;
#elif (MAGICKCORE_SIZEOF_DOUBLE_T == MAGICKCORE_SIZEOF_LONG_DOUBLE)
typedef double MagickDoubleType;
#else
#error Your MagickDoubleType type is neither a float, nor a double, nor a long double
#endif

#if (MAGICKCORE_QUANTUM_DEPTH == 8)
#define MaxColormapSize  256UL
#define MaxMap  255UL

#if defined(MAGICKCORE_HDRI_SUPPORT)
typedef MagickFloatType Quantum;
#define QuantumRange  255.0
#define QuantumFormat  "%g"
#else
typedef unsigned char Quantum;
#define QuantumRange  ((Quantum) 255)
#define QuantumFormat  "%u"
#endif
#elif (MAGICKCORE_QUANTUM_DEPTH == 16)
#define MaxColormapSize  65536UL
#define MaxMap  65535UL

#if defined(MAGICKCORE_HDRI_SUPPORT)
typedef MagickFloatType Quantum;
#define QuantumRange  65535.0f
#define QuantumFormat  "%g"
#else
typedef unsigned short Quantum;
#define QuantumRange  ((Quantum) 65535)
#define QuantumFormat  "%u"
#endif
#elif (MAGICKCORE_QUANTUM_DEPTH == 32)
#define MaxColormapSize  65536UL
#define MaxMap  65535UL

#if defined(MAGICKCORE_HDRI_SUPPORT)
typedef MagickDoubleType Quantum;
#define QuantumRange  4294967295.0
#define QuantumFormat  "%g"
#else
typedef unsigned int Quantum;
#define QuantumRange  ((Quantum) 4294967295)
#define QuantumFormat  "%u"
#endif
#elif (MAGICKCORE_QUANTUM_DEPTH == 64)
#define MAGICKCORE_HDRI_SUPPORT  1
#define MaxColormapSize  65536UL
#define MaxMap  65535UL

typedef MagickDoubleType Quantum;
#define QuantumRange  18446744073709551615.0
#define QuantumFormat  "%g"
#else
#error "MAGICKCORE_QUANTUM_DEPTH must be one of 8, 16, 32, or 64"
#endif
#define MagickEpsilon  1.0e-12
#define MagickMaximumValue  1.79769313486231570E+308
#define MagickMinimumValue   2.22507385850720140E-308
#define MagickStringify(macro_or_string)  MagickStringifyArg(macro_or_string)
#define MagickStringifyArg(contents)  #contents
#define QuantumScale  ((double) 1.0/(double) QuantumRange)

/*
  Typedef declarations.
*/
typedef MagickDoubleType MagickRealType;
typedef unsigned int MagickStatusType;
#if !defined(MAGICKCORE_WINDOWS_SUPPORT)
#if (MAGICKCORE_SIZEOF_UNSIGNED_LONG_LONG == 8)
typedef long long MagickOffsetType;
typedef unsigned long long MagickSizeType;
#define MagickOffsetFormat  "lld"
#define MagickSizeFormat  "llu"
#else
typedef ssize_t MagickOffsetType;
typedef size_t MagickSizeType;
#define MagickOffsetFormat  "ld"
#define MagickSizeFormat  "lu"
#endif
#else
typedef __int64 MagickOffsetType;
typedef unsigned __int64 MagickSizeType;
#define MagickOffsetFormat  "I64i"
#define MagickSizeFormat  "I64u"
#endif

#if MAGICKCORE_HAVE_UINTPTR_T || defined(uintptr_t)
typedef uintptr_t MagickAddressType;
#else
/* Hope for the best, I guess. */
typedef size_t MagickAddressType;
#endif

#if defined(_MSC_VER) && (_MSC_VER == 1200)
typedef MagickOffsetType QuantumAny;
#else
typedef MagickSizeType QuantumAny;
#endif

#if defined(macintosh)
#define ExceptionInfo  MagickExceptionInfo
#endif

typedef enum
{
  UndefinedClass,
  DirectClass,
  PseudoClass
} ClassType;

typedef enum
{
  MagickFalse = 0,
  MagickTrue = 1
} MagickBooleanType;

/*
  The IsNaN test is for special floating point numbers of value Nan (not a
  number). NaN's are defined as part of the IEEE standard for floating point
  number representation, and need to be watched out for. Morphology Kernels
  often use these special numbers as neighbourhood masks.

  The special property that two NaN's are never equal, even if they are from
  the same variable allows you to test if a value is special NaN value.

  The macros are thus is only true if the value given is NaN.
*/
#if defined(MAGICKCORE_HAVE_ISNAN)
#  define IsNaN(a) isnan(a)
#elif defined(_MSC_VER) && (_MSC_VER >= 1310)
#  include <float.h>
#  define IsNaN(a) _isnan(a)
#else
#  define IsNaN(a) ((a) != (a))
#endif
#if !defined(INFINITY)
#  define INFINITY ((double) -logf(0f))
#endif

typedef struct _BlobInfo BlobInfo;

typedef struct _ExceptionInfo ExceptionInfo;

typedef struct _Image Image;

typedef struct _ImageInfo ImageInfo;

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

/*
  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore types.
*/
#ifndef _MAGICKCORE_MAGICK_TYPE_H
#define _MAGICKCORE_MAGICK_TYPE_H

#include "MagickCore/magick-config.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(MAGICKCORE_QUANTUM_DEPTH)
#define MAGICKCORE_QUANTUM_DEPTH  16
#endif

#if defined(MAGICKCORE_WINDOWS_SUPPORT) && !defined(__MINGW32__) && !defined(__MINGW64__)
#  define MagickLLConstant(c)  (MagickOffsetType) (c ## i64)
#  define MagickULLConstant(c)  (MagickSizeType) (c ## ui64)
#else
#  define MagickLLConstant(c)  (MagickOffsetType) (c ## LL)
#  define MagickULLConstant(c)  (MagickSizeType) (c ## ULL)
#endif

#if !defined(MAGICKCORE_HAVE_DOUBLE_T)
typedef double double_t;
#endif
#if !defined(MAGICKCORE_HAVE_FLOAT_T)
typedef float float_t;
#endif

#if (MAGICKCORE_QUANTUM_DEPTH == 8)
#define MaxColormapSize  256UL
#define MaxMap  255UL
typedef float_t MagickRealType;

#if defined(MAGICKCORE_HDRI_SUPPORT)
typedef float Quantum;
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
typedef float_t MagickRealType;

#if defined(MAGICKCORE_HDRI_SUPPORT)
typedef float Quantum;
#define QuantumRange  65535.0
#define QuantumFormat  "%g"
#else
typedef unsigned short Quantum;
#define QuantumRange  ((Quantum) 65535)
#define QuantumFormat  "%u"
#endif
#elif (MAGICKCORE_QUANTUM_DEPTH == 32)
#define MaxColormapSize  65536UL
#define MaxMap  65535UL
typedef float_t MagickRealType;

#if defined(MAGICKCORE_HDRI_SUPPORT)
typedef float Quantum;
#define QuantumRange  4294967295.0
#define QuantumFormat  "%g"
#else
typedef unsigned int Quantum;
#define QuantumRange  ((Quantum) 4294967295)
#define QuantumFormat  "%u"
#endif
#elif (MAGICKCORE_QUANTUM_DEPTH == 64)
#define MAGICKCORE_HDRI_SUPPORT
#define MaxColormapSize  65536UL
#define MaxMap  65535UL

typedef long double MagickRealType;
typedef double_t Quantum;
#define QuantumRange  18446744073709551615.0
#define QuantumFormat  "%g"
#else
#error "MAGICKCORE_QUANTUM_DEPTH must be one of 8, 16, 32, or 64"
#endif
#define MagickEpsilon  (1.0e-15)
#define MagickMaximumValue  1.79769313486231570E+308
#define MagickMinimumValue   2.22507385850720140E-308
#define MagickPI  3.14159265358979323846264338327950288419716939937510
#define QuantumScale  ((double) 1.0/(double) QuantumRange)

/*
  Typedef declarations.
*/
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
   Define some short-hand macros for handling MagickBooleanType
   and uses fast C typing with C boolean operations

     Is  -- returns a MagickBooleanType (for storage)
     If  -- returns C integer boolean (for if's and while's)

   IfMagickTrue()     converts MagickBooleanType to C integer Boolean
   IfMagickFalse()    Not the MagickBooleanType to C integer Boolean

   IsMagickTrue()     converts a C integer boolean to a MagickBooleanType
   IsMagickFalse()    converts and is also a MagickBooleanType 'not' operation

   IsMagickNULL()
   IsMagickNotNULL()  converts C pointers tests MagickBooleanType
*/
#if 1
/* Fast C typing method assumes MagickBooleanType match 0,1 values */
#  define IfMagickTrue(v)  ((int)(v))
#  define IfMagickFalse(v) (!(int)(v))
#  define IsMagickTrue(v)  ((MagickBooleanType)((int)(v)!=0))
#  define IsMagickFalse(v) ((MagickBooleanType)(!(int)(v)))
#  define IsMagickNot(v)   ((MagickBooleanType)(!(int)(v)))
#else
/* Do not depend MagickBooleanType's values */
#  define IfMagickTrue(v)  ((v) != MagickFalse)
#  define IfMagickFalse(v) ((v) == MagickFalse)
#  define IsMagickTrue(v)  ((v)?MagickTrue:MagickFalse)
#  define IsMagickFalse(v) ((v)?MagickFalse:MagickTrue)
#  define IsMagickNot(v)   (IfMagickTrue(v)?MagickFalse:MagickTrue)
#endif
#define IfStringTrue(v)       IfMagickTrue(IsStringTrue(v))
#define IfStringNotFalse(v)   IfMagickTrue(IsStringNotFalse(v))
#define IsMagickNULL(v)       (((void *)(v) == NULL)?MagickTrue:MagickFalse)
#define IsMagickNotNULL(v)    (((void *)(v) != NULL)?MagickTrue:MagickFalse)

/*
  The IsNaN and IfNan tests for special floating point numbers of
  value NaN (not a number).  NaN's are defined as part of the IEEE standard
  for floating point number representation, and need to be watched out for.
  Morphology Kernels often use these special numbers as neighbourhood masks.

  The special property that two NaN's are never equal, even if they are from
  the same variable allows you to test if a value is special NaN value.

  The macros are thus is only true if the value given is NaN.
*/
#  define IfNaN(a) ((a) != (a))
#  define IsNaN(a) ((a) != (a)?MagickTrue:MagickFalse)


typedef struct _BlobInfo BlobInfo;

typedef struct _ExceptionInfo ExceptionInfo;

typedef struct _Image Image;

typedef struct _ImageInfo ImageInfo;

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

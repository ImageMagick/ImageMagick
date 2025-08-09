/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP   N   N   GGGG                              %
%                            P   P  NN  N  G                                  %
%                            PPPP   N N N  G  GG                              %
%                            P      N  NN  G   G                              %
%                            P      N   N   GGG                               %
%                                                                             %
%                                                                             %
%              Read/Write Portable Network Graphics Image Format              %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                           Glenn Randers-Pehrson                             %
%                               November 1997                                 %
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
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/channel.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/histogram.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/layer.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/magick-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile-private.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/timer-private.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"
#include "coders/coders-private.h"
#if defined(MAGICKCORE_PNG_DELEGATE)

/* Suppress libpng pedantic warnings that were added in
 * libpng-1.2.41 and libpng-1.4.0.  If you are working on
 * migration to libpng-1.5, remove these defines and then
 * fix any code that generates warnings.
 */
/* #define PNG_DEPRECATED   Use of this function is deprecated */
/* #define PNG_USE_RESULT   The result of this function must be checked */
/* #define PNG_NORETURN     This function does not return */
/* #define PNG_ALLOCATED    The result of the function is new memory */

#include <png.h>
#include <zlib.h>

#if defined(MAGICKCORE_JPEG_DELEGATE)
#  define JNG_SUPPORTED /* Not finished as of 5.5.2.  See "To do" comments. */
#endif
#define IsPNGColorEqual(color,target) \
       (((color).red == (target).red) && \
        ((color).green == (target).green) && \
        ((color).blue == (target).blue))

/* Table of recognized sRGB ICC profiles */
struct sRGB_info_struct
{
    png_uint_32 len;
    png_uint_32 crc;
    png_byte intent;
};

const struct sRGB_info_struct sRGB_info[] =
{
    /* ICC v2 perceptual sRGB_IEC61966-2-1_black_scaled.icc */
    { 3048, 0x3b8772b9UL, 0},

    /* ICC v2 relative sRGB_IEC61966-2-1_no_black_scaling.icc */
    { 3052, 0x427ebb21UL, 1},

    /* ICC v4 perceptual sRGB_v4_ICC_preference_displayclass.icc */
    {60988, 0x306fd8aeUL, 0},

    /* ICC v4 perceptual sRGB_v4_ICC_preference.icc perceptual */
     {60960, 0xbbef7812UL, 0},

    /* HP? sRGB v2 media-relative sRGB_IEC61966-2-1_noBPC.icc */
     { 3024, 0x5d5129ceUL, 1},

     /* HP-Microsoft sRGB v2 perceptual */
     { 3144, 0x182ea552UL, 0},

     /* HP-Microsoft sRGB v2 media-relative */
     { 3144, 0xf29e526dUL, 1},

     /* Facebook's "2012/01/25 03:41:57", 524, "TINYsRGB.icc" */
     {  524, 0xd4938c39UL, 0},

     /* "2012/11/28 22:35:21", 3212, "Argyll_sRGB.icm") */
     { 3212, 0x034af5a1UL, 0},

     /* Not recognized */
     {    0, 0x00000000UL, 0},
};

/* Macros for left-bit-replication to ensure that pixels
 * and PixelInfos all have the same image->depth, and for use
 * in PNG8 quantization.
 */

/* LBR01: Replicate top bit */

#define LBR01PacketRed(pixelpacket) \
     (pixelpacket).red=(ScaleQuantumToChar((Quantum) (pixelpacket).red) < 0x10 ? \
        0 : QuantumRange);

#define LBR01PacketGreen(pixelpacket) \
     (pixelpacket).green=(ScaleQuantumToChar((Quantum) (pixelpacket).green) < 0x10 ? \
        0 : QuantumRange);

#define LBR01PacketBlue(pixelpacket) \
     (pixelpacket).blue=(ScaleQuantumToChar((Quantum) (pixelpacket).blue) < 0x10 ? \
        0 : QuantumRange);

#define LBR01PacketAlpha(pixelpacket) \
     (pixelpacket).alpha=(ScaleQuantumToChar((Quantum) (pixelpacket).alpha) < 0x10 ? \
        0 : QuantumRange);

#define LBR01PacketRGB(pixelpacket) \
        { \
        LBR01PacketRed((pixelpacket)); \
        LBR01PacketGreen((pixelpacket)); \
        LBR01PacketBlue((pixelpacket)); \
        }

#define LBR01PacketRGBA(pixelpacket) \
        { \
        LBR01PacketRGB((pixelpacket)); \
        LBR01PacketAlpha((pixelpacket)); \
        }

#define LBR01PixelRed(pixel) \
        (SetPixelRed(image, \
        ScaleQuantumToChar(GetPixelRed(image,(pixel))) < 0x10 ? \
        0 : QuantumRange,(pixel)));

#define LBR01PixelGreen(pixel) \
        (SetPixelGreen(image, \
        ScaleQuantumToChar(GetPixelGreen(image,(pixel))) < 0x10 ? \
        0 : QuantumRange,(pixel)));

#define LBR01PixelBlue(pixel) \
        (SetPixelBlue(image, \
        ScaleQuantumToChar(GetPixelBlue(image,(pixel))) < 0x10 ? \
        0 : QuantumRange,(pixel)));

#define LBR01PixelAlpha(pixel) \
        (SetPixelAlpha(image, \
        ScaleQuantumToChar(GetPixelAlpha(image,(pixel))) < 0x10 ? \
        0 : QuantumRange,(pixel)));

#define LBR01PixelRGB(pixel) \
        { \
        LBR01PixelRed((pixel)); \
        LBR01PixelGreen((pixel)); \
        LBR01PixelBlue((pixel)); \
        }

#define LBR01PixelRGBA(pixel) \
        { \
        LBR01PixelRGB((pixel)); \
        LBR01PixelAlpha((pixel)); \
        }

/* LBR02: Replicate top 2 bits */

#define LBR02PacketRed(pixelpacket) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar((Quantum) (pixelpacket).red) & 0xc0; \
     (pixelpacket).red=ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 2) | (lbr_bits >> 4) | (lbr_bits >> 6))); \
   }
#define LBR02PacketGreen(pixelpacket) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar((Quantum) (pixelpacket).green) & 0xc0; \
     (pixelpacket).green=ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 2) | (lbr_bits >> 4) | (lbr_bits >> 6))); \
   }
#define LBR02PacketBlue(pixelpacket) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar((Quantum) (pixelpacket).blue) & 0xc0; \
     (pixelpacket).blue=ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 2) | (lbr_bits >> 4) | (lbr_bits >> 6))); \
   }
#define LBR02PacketAlpha(pixelpacket) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar((Quantum) (pixelpacket).alpha) & 0xc0; \
     (pixelpacket).alpha=ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 2) | (lbr_bits >> 4) | (lbr_bits >> 6))); \
   }

#define LBR02PacketRGB(pixelpacket) \
        { \
        LBR02PacketRed((pixelpacket)); \
        LBR02PacketGreen((pixelpacket)); \
        LBR02PacketBlue((pixelpacket)); \
        }

#define LBR02PacketRGBA(pixelpacket) \
        { \
        LBR02PacketRGB((pixelpacket)); \
        LBR02PacketAlpha((pixelpacket)); \
        }

#define LBR02PixelRed(pixel) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar(GetPixelRed(image,(pixel))) \
       & 0xc0; \
     SetPixelRed(image, ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 2) | (lbr_bits >> 4) | (lbr_bits >> 6))), \
       (pixel)); \
   }
#define LBR02PixelGreen(pixel) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar(GetPixelGreen(image,(pixel)))\
       & 0xc0; \
     SetPixelGreen(image, ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 2) | (lbr_bits >> 4) | (lbr_bits >> 6))), \
       (pixel)); \
   }
#define LBR02PixelBlue(pixel) \
   { \
     unsigned char lbr_bits= \
       ScaleQuantumToChar(GetPixelBlue(image,(pixel))) & 0xc0; \
     SetPixelBlue(image, ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 2) | (lbr_bits >> 4) | (lbr_bits >> 6))), \
       (pixel)); \
   }
#define LBR02PixelAlpha(pixel) \
   { \
     unsigned char lbr_bits= \
       ScaleQuantumToChar(GetPixelAlpha(image,(pixel))) & 0xc0; \
     SetPixelAlpha(image, ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 2) | (lbr_bits >> 4) | (lbr_bits >> 6))), \
       (pixel) ); \
   }

#define LBR02PixelRGB(pixel) \
        { \
        LBR02PixelRed((pixel)); \
        LBR02PixelGreen((pixel)); \
        LBR02PixelBlue((pixel)); \
        }

#define LBR02PixelRGBA(pixel) \
        { \
        LBR02PixelRGB((pixel)); \
        LBR02PixelAlpha((pixel)); \
        }

/* LBR03: Replicate top 3 bits (only used with opaque pixels during
   PNG8 quantization) */

#define LBR03PacketRed(pixelpacket) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar((Quantum) (pixelpacket).red) & 0xe0; \
     (pixelpacket).red=ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 3) | (lbr_bits >> 6))); \
   }
#define LBR03PacketGreen(pixelpacket) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar((Quantum) (pixelpacket).green) & 0xe0; \
     (pixelpacket).green=ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 3) | (lbr_bits >> 6))); \
   }
#define LBR03PacketBlue(pixelpacket) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar((Quantum) (pixelpacket).blue) & 0xe0; \
     (pixelpacket).blue=ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 3) | (lbr_bits >> 6))); \
   }

#define LBR03PacketRGB(pixelpacket) \
        { \
        LBR03PacketRed((pixelpacket)); \
        LBR03PacketGreen((pixelpacket)); \
        LBR03PacketBlue((pixelpacket)); \
        }

#define LBR03PixelRed(pixel) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar(GetPixelRed(image,(pixel))) \
       & 0xe0; \
     SetPixelRed(image, ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 3) | (lbr_bits >> 6))), (pixel)); \
   }
#define LBR03Green(pixel) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar(GetPixelGreen(image,(pixel)))\
       & 0xe0; \
     SetPixelGreen(image, ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 3) | (lbr_bits >> 6))), (pixel)); \
   }
#define LBR03Blue(pixel) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar(GetPixelBlue(image,(pixel))) \
       & 0xe0; \
     SetPixelBlue(image, ScaleCharToQuantum( \
       (lbr_bits | (lbr_bits >> 3) | (lbr_bits >> 6))), (pixel)); \
   }

#define LBR03RGB(pixel) \
        { \
        LBR03PixelRed((pixel)); \
        LBR03Green((pixel)); \
        LBR03Blue((pixel)); \
        }

/* LBR04: Replicate top 4 bits */

#define LBR04PacketRed(pixelpacket) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar((Quantum) (pixelpacket).red) & 0xf0; \
     (pixelpacket).red=ScaleCharToQuantum((lbr_bits | (lbr_bits >> 4))); \
   }
#define LBR04PacketGreen(pixelpacket) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar((Quantum) (pixelpacket).green) & 0xf0; \
     (pixelpacket).green=ScaleCharToQuantum((lbr_bits | (lbr_bits >> 4))); \
   }
#define LBR04PacketBlue(pixelpacket) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar((Quantum) (pixelpacket).blue) & 0xf0; \
     (pixelpacket).blue=ScaleCharToQuantum((lbr_bits | (lbr_bits >> 4))); \
   }
#define LBR04PacketAlpha(pixelpacket) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar((Quantum) (pixelpacket).alpha) & 0xf0; \
     (pixelpacket).alpha=ScaleCharToQuantum((lbr_bits | (lbr_bits >> 4))); \
   }

#define LBR04PacketRGB(pixelpacket) \
        { \
        LBR04PacketRed((pixelpacket)); \
        LBR04PacketGreen((pixelpacket)); \
        LBR04PacketBlue((pixelpacket)); \
        }

#define LBR04PacketRGBA(pixelpacket) \
        { \
        LBR04PacketRGB((pixelpacket)); \
        LBR04PacketAlpha((pixelpacket)); \
        }

#define LBR04PixelRed(pixel) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar(GetPixelRed(image,(pixel))) \
       & 0xf0; \
     SetPixelRed(image,\
       ScaleCharToQuantum((lbr_bits | (lbr_bits >> 4))), (pixel)); \
   }
#define LBR04PixelGreen(pixel) \
   { \
     unsigned char lbr_bits=ScaleQuantumToChar(GetPixelGreen(image,(pixel)))\
       & 0xf0; \
     SetPixelGreen(image,\
       ScaleCharToQuantum((lbr_bits | (lbr_bits >> 4))), (pixel)); \
   }
#define LBR04PixelBlue(pixel) \
   { \
     unsigned char lbr_bits= \
       ScaleQuantumToChar(GetPixelBlue(image,(pixel))) & 0xf0; \
     SetPixelBlue(image,\
       ScaleCharToQuantum((lbr_bits | (lbr_bits >> 4))), (pixel)); \
   }
#define LBR04PixelAlpha(pixel) \
   { \
     unsigned char lbr_bits= \
       ScaleQuantumToChar(GetPixelAlpha(image,(pixel))) & 0xf0; \
     SetPixelAlpha(image,\
       ScaleCharToQuantum((lbr_bits | (lbr_bits >> 4))), (pixel)); \
   }

#define LBR04PixelRGB(pixel) \
        { \
        LBR04PixelRed((pixel)); \
        LBR04PixelGreen((pixel)); \
        LBR04PixelBlue((pixel)); \
        }

#define LBR04PixelRGBA(pixel) \
        { \
        LBR04PixelRGB((pixel)); \
        LBR04PixelAlpha((pixel)); \
        }

/*
  Establish thread safety.
  setjmp/longjmp is claimed to be safe on these platforms:
  setjmp/longjmp is alleged to be unsafe on these platforms:
*/
#ifdef PNG_SETJMP_SUPPORTED
# ifndef MAGICKCORE_SETJMP_IS_THREAD_SAFE
#   define IMPNG_SETJMP_NOT_THREAD_SAFE
# endif

# ifdef IMPNG_SETJMP_NOT_THREAD_SAFE
static SemaphoreInfo
  *ping_semaphore = (SemaphoreInfo *) NULL;
# endif
#endif

/*
  This temporary until I set up malloc'ed object attributes array.
  Recompile with MNG_MAX_OBJECTS=65536L to avoid this limit but
  waste more memory.
*/
#define MNG_MAX_OBJECTS 256

/*
  Maximum valid size_t in PNG/MNG chunks is (2^31)-1
  This macro is only defined in libpng-1.0.3 and later.
  Previously it was PNG_MAX_UINT but that was deprecated in libpng-1.2.6
*/
#ifndef PNG_UINT_31_MAX
#define PNG_UINT_31_MAX (png_uint_32) 0x7fffffffL
#endif

/*
  Constant strings for known chunk types.  If you need to add a chunk,
  add a string holding the name here.   To make the code more
  portable, we use ASCII numbers like this, not characters.
*/

static const png_byte mng_MHDR[5]={ 77,  72,  68,  82, (png_byte) '\0'};
static const png_byte mng_BACK[5]={ 66,  65,  67,  75, (png_byte) '\0'};
static const png_byte mng_BASI[5]={ 66,  65,  83,  73, (png_byte) '\0'};
static const png_byte mng_CLIP[5]={ 67,  76,  73,  80, (png_byte) '\0'};
static const png_byte mng_CLON[5]={ 67,  76,  79,  78, (png_byte) '\0'};
static const png_byte mng_DEFI[5]={ 68,  69,  70,  73, (png_byte) '\0'};
static const png_byte mng_DHDR[5]={ 68,  72,  68,  82, (png_byte) '\0'};
static const png_byte mng_DISC[5]={ 68,  73,  83,  67, (png_byte) '\0'};
static const png_byte mng_ENDL[5]={ 69,  78,  68,  76, (png_byte) '\0'};
static const png_byte mng_FRAM[5]={ 70,  82,  65,  77, (png_byte) '\0'};
static const png_byte mng_IEND[5]={ 73,  69,  78,  68, (png_byte) '\0'};
static const png_byte mng_IHDR[5]={ 73,  72,  68,  82, (png_byte) '\0'};
static const png_byte mng_JHDR[5]={ 74,  72,  68,  82, (png_byte) '\0'};
static const png_byte mng_LOOP[5]={ 76,  79,  79,  80, (png_byte) '\0'};
static const png_byte mng_MAGN[5]={ 77,  65,  71,  78, (png_byte) '\0'};
static const png_byte mng_MEND[5]={ 77,  69,  78,  68, (png_byte) '\0'};
static const png_byte mng_MOVE[5]={ 77,  79,  86,  69, (png_byte) '\0'};
static const png_byte mng_PAST[5]={ 80,  65,  83,  84, (png_byte) '\0'};
static const png_byte mng_PLTE[5]={ 80,  76,  84,  69, (png_byte) '\0'};
static const png_byte mng_SAVE[5]={ 83,  65,  86,  69, (png_byte) '\0'};
static const png_byte mng_SEEK[5]={ 83,  69,  69,  75, (png_byte) '\0'};
static const png_byte mng_SHOW[5]={ 83,  72,  79,  87, (png_byte) '\0'};
static const png_byte mng_TERM[5]={ 84,  69,  82,  77, (png_byte) '\0'};
static const png_byte mng_bKGD[5]={ 98,  75,  71,  68, (png_byte) '\0'};
static const png_byte mng_caNv[5]={ 99,  97,  78, 118, (png_byte) '\0'};
static const png_byte mng_cHRM[5]={ 99,  72,  82,  77, (png_byte) '\0'};
static const png_byte mng_eXIf[5]={101,  88,  73, 102, (png_byte) '\0'};
static const png_byte mng_gAMA[5]={103,  65,  77,  65, (png_byte) '\0'};
static const png_byte mng_iCCP[5]={105,  67,  67,  80, (png_byte) '\0'};
static const png_byte mng_nEED[5]={110,  69,  69,  68, (png_byte) '\0'};
static const png_byte mng_orNT[5]={111, 114,  78,  84, (png_byte) '\0'};
static const png_byte mng_pHYg[5]={112,  72,  89, 103, (png_byte) '\0'};
static const png_byte mng_pHYs[5]={112,  72,  89, 115, (png_byte) '\0'};
static const png_byte mng_sBIT[5]={115,  66,  73,  84, (png_byte) '\0'};
static const png_byte mng_sRGB[5]={115,  82,  71,  66, (png_byte) '\0'};
static const png_byte mng_tRNS[5]={116,  82,  78,  83, (png_byte) '\0'};
static const png_byte mng_vpAg[5]={118, 112,  65, 103, (png_byte) '\0'};

#if defined(JNG_SUPPORTED)
static const png_byte mng_IDAT[5]={ 73,  68,  65,  84, (png_byte) '\0'};
static const png_byte mng_JDAT[5]={ 74,  68,  65,  84, (png_byte) '\0'};
static const png_byte mng_JDAA[5]={ 74,  68,  65,  65, (png_byte) '\0'};
static const png_byte mng_JdAA[5]={ 74, 100,  65,  65, (png_byte) '\0'};
static const png_byte mng_JSEP[5]={ 74,  83,  69,  80, (png_byte) '\0'};
static const png_byte mng_oFFs[5]={111,  70,  70, 115, (png_byte) '\0'};
#endif

#if 0
/* Other known chunks that are not yet supported by ImageMagick: */
static const png_byte mng_hIST[5]={104,  73,  83,  84, (png_byte) '\0'};
static const png_byte mng_iTXt[5]={105,  84,  88, 116, (png_byte) '\0'};
static const png_byte mng_sPLT[5]={115,  80,  76,  84, (png_byte) '\0'};
static const png_byte mng_sTER[5]={115,  84,  69,  82, (png_byte) '\0'};
static const png_byte mng_tEXt[5]={116,  69,  88, 116, (png_byte) '\0'};
static const png_byte mng_tIME[5]={116,  73,  77,  69, (png_byte) '\0'};
static const png_byte mng_zTXt[5]={122,  84,  88, 116, (png_byte) '\0'};
#endif

typedef struct _MngBox
{
  long
    left,
    right,
    top,
    bottom;
} MngBox;

typedef struct _MngPair
{
  volatile long
    a,
    b;
} MngPair;

typedef struct _MngReadInfo
{
  ChromaticityInfo
    global_chrm;

  float
    global_gamma;

  Image
    *image;

  int
    framing_mode,
    object_id;

  MagickBooleanType
    have_global_bkgd,
    have_global_chrm,
    have_global_gama,
    have_global_phys,
    have_global_sbit,
    have_global_srgb;

  MagickOffsetType
    loop_jump[256];

  MngBox
    clip,
    frame,
    image_box,
    object_clip[MNG_MAX_OBJECTS];

  PixelInfo
    mng_global_bkgd;

  png_byte
    global_trns[256];

  png_color_8
    global_sbit;

  png_colorp
    global_plte;

  png_uint_16
    magn_mb,
    magn_ml,
    magn_mr,
    magn_mt,
    magn_mx,
    magn_my,
    magn_methx,
    magn_methy;

  RenderingIntent
    global_srgb_intent;

  ssize_t
    image_found,
    loop_count[256],
    loop_iteration[256],
    scenes_found,
    x_off[MNG_MAX_OBJECTS],
    y_off[MNG_MAX_OBJECTS];

  unsigned char
    /* These flags could be combined into one byte */
    exists[MNG_MAX_OBJECTS],
    frozen[MNG_MAX_OBJECTS],
    loop_active[256],
    invisible[MNG_MAX_OBJECTS],
    viewable[MNG_MAX_OBJECTS];

  unsigned long
    global_plte_length,
    global_trns_length,
    global_x_pixels_per_unit,
    global_y_pixels_per_unit,
    mng_width,
    mng_height,
    ticks_per_second;

  unsigned int
    basi_warning,
    clon_warning,
    dhdr_warning,
    global_phys_unit_type,
    jhdr_warning,
    magn_warning,
    mng_type,
    past_warning,
    phyg_warning,
    phys_warning,
    sbit_warning,
    show_warning;
} MngReadInfo;

typedef struct _MngWriteInfo
{
  Image *
    image;

  RectangleInfo
    page;

  int
    framing_mode,
    old_framing_mode;

  unsigned long
    delay,
    ticks_per_second;

  MagickBooleanType
    adjoin,
    need_blob,
    equal_backgrounds,
    equal_chrms,
    equal_gammas,
    equal_palettes,
    equal_physs,
    equal_srgbs,
    exclude_bKGD,
    exclude_cHRM,
    exclude_date,
    exclude_eXIf,
    exclude_EXIF,
    exclude_gAMA,
    exclude_iCCP,
    exclude_oFFs,
    exclude_pHYs,
    exclude_sRGB,
    exclude_tEXt,
    exclude_tIME,
    exclude_tRNS,
    exclude_caNv,
    exclude_zCCP,
    exclude_zTXt,
    have_global_chrm,
    have_global_gama,
    have_global_plte,
    have_global_srgb,
    is_palette,
    need_fram,
    preserve_colormap,
    preserve_iCCP,
    write_mng,
    write_png8,
    write_png24,
    write_png32,
    write_png48,
    write_png64;

  unsigned int
    colortype,
    compression_filter,
    compression_level,
    compression_strategy,
    depth;

} MngWriteInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePNGImage(const ImageInfo *,Image *,ExceptionInfo *);

static MagickBooleanType
  WriteMNGImage(const ImageInfo *,Image *,ExceptionInfo *);

#if defined(JNG_SUPPORTED)
static MagickBooleanType
  WriteJNGImage(const ImageInfo *,Image *,ExceptionInfo *);
#endif

#if (MAGICKCORE_QUANTUM_DEPTH >= 16)
static MagickBooleanType
LosslessReduceDepthOK(Image *image,ExceptionInfo *exception)
{
    /* Reduce bit depth if it can be reduced losslessly from 16+ to 8.
     *
     * This is true if the high byte and the next highest byte of
     * each sample of the image, the colormap, and the background color
     * are equal to each other.  We check this by seeing if the samples
     * are unchanged when we scale them down to 8 and back up to Quantum.
     *
     * We don't use the method GetImageDepth() because it doesn't check
     * background and doesn't handle PseudoClass specially.
     */

#define QuantumToCharToQuantumEqQuantum(quantum) \
 (((double) ScaleCharToQuantum((unsigned char) ScaleQuantumToChar((Quantum) quantum))) == (double) quantum)

    MagickBooleanType
      ok_to_reduce=MagickFalse;

    if (image->depth >= 16)
      {

        const Quantum
          *p;

        ok_to_reduce=
           QuantumToCharToQuantumEqQuantum(image->background_color.red) &&
           QuantumToCharToQuantumEqQuantum(image->background_color.green) &&
           QuantumToCharToQuantumEqQuantum(image->background_color.blue) ?
           MagickTrue : MagickFalse;

        if (ok_to_reduce != MagickFalse && image->storage_class == PseudoClass && image->colormap != NULL)
          {
            int indx;

            for (indx=0; indx < (ssize_t) image->colors; indx++)
              {
                ok_to_reduce=(
                   QuantumToCharToQuantumEqQuantum(
                   image->colormap[indx].red) &&
                   QuantumToCharToQuantumEqQuantum(
                   image->colormap[indx].green) &&
                   QuantumToCharToQuantumEqQuantum(
                   image->colormap[indx].blue)) ?
                   MagickTrue : MagickFalse;

                if (ok_to_reduce == MagickFalse)
                   break;
              }
          }

        if ((ok_to_reduce != MagickFalse) &&
            (image->storage_class != PseudoClass))
          {
            ssize_t
              y;

            ssize_t
              x;

            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,exception);

              if (p == (const Quantum *) NULL)
                {
                  ok_to_reduce = MagickFalse;
                  break;
                }

              for (x=(ssize_t) image->columns-1; x >= 0; x--)
              {
                ok_to_reduce=
                   QuantumToCharToQuantumEqQuantum(GetPixelRed(image,p)) &&
                   QuantumToCharToQuantumEqQuantum(GetPixelGreen(image,p)) &&
                   QuantumToCharToQuantumEqQuantum(GetPixelBlue(image,p)) ?
                   MagickTrue : MagickFalse;

                if (ok_to_reduce == MagickFalse)
                  break;

                p+=(ptrdiff_t) GetPixelChannels(image);
              }
              if (x >= 0)
                break;
            }
          }

        if ((image->debug != MagickFalse) && (ok_to_reduce != MagickFalse))
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    OK to reduce PNG bit depth to 8 without loss of info");
          }
        else
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    Not OK to reduce PNG bit depth to 8 without losing info");
          }
      }

    return ok_to_reduce;
}
#endif /* MAGICKCORE_QUANTUM_DEPTH >= 16 */

static const char *PngColorTypeToString(const unsigned int color_type)
{
  const char
    *result = "Unknown";

  switch (color_type)
    {
    case PNG_COLOR_TYPE_GRAY:
      result = "Gray";
      break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      result = "Gray+Alpha";
      break;
    case PNG_COLOR_TYPE_PALETTE:
      result = "Palette";
      break;
    case PNG_COLOR_TYPE_RGB:
      result = "RGB";
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
      result = "RGB+Alpha";
      break;
    }

  return result;
}

static int
Magick_Orientation_to_Exif_Orientation(const OrientationType orientation)
{
  switch (orientation)
  {
    /* Convert to Exif orientations as defined in "Exchangeable image file
     * format for digital still cameras: Exif Version 2.31",
     * http://www.cipa.jp/std/documents/e/DC-008-Translation-2016-E.pdf
     */

    case TopLeftOrientation:
       return 1;
    case TopRightOrientation:
       return 2;
    case BottomRightOrientation:
       return 3;
    case BottomLeftOrientation:
       return 4;
    case LeftTopOrientation:
       return 5;
    case RightTopOrientation:
       return 6;
    case RightBottomOrientation:
       return 7;
    case LeftBottomOrientation:
       return 8;
    case UndefinedOrientation:
    default:
       return 0;
  }
}
static OrientationType
Magick_Orientation_from_Exif_Orientation(const int orientation)
{
  switch (orientation)
  {
    case 1:
      return TopLeftOrientation;
    case 2:
      return TopRightOrientation;
    case 3:
      return BottomRightOrientation;
    case 4:
      return BottomLeftOrientation;
    case 5:
      return LeftTopOrientation;
    case 6:
      return RightTopOrientation;
    case 7:
      return RightBottomOrientation;
    case 8:
      return LeftBottomOrientation;
    case 0:
    default:
      return UndefinedOrientation;
  }
}

static int
Magick_RenderingIntent_to_PNG_RenderingIntent(const RenderingIntent intent)
{
  switch (intent)
  {
    case PerceptualIntent:
       return 0;

    case RelativeIntent:
       return 1;

    case SaturationIntent:
       return 2;

    case AbsoluteIntent:
       return 3;

    default:
       return -1;
  }
}

static RenderingIntent
Magick_RenderingIntent_from_PNG_RenderingIntent(const int ping_intent)
{
  switch (ping_intent)
  {
    case 0:
      return PerceptualIntent;

    case 1:
      return RelativeIntent;

    case 2:
      return SaturationIntent;

    case 3:
      return AbsoluteIntent;

    default:
      return UndefinedIntent;
    }
}

static const char *
Magick_RenderingIntentString_from_PNG_RenderingIntent(const int ping_intent)
{
  switch (ping_intent)
  {
    case 0:
      return "Perceptual Intent";

    case 1:
      return "Relative Intent";

    case 2:
      return "Saturation Intent";

    case 3:
      return "Absolute Intent";

    default:
      return "Undefined Intent";
    }
}

static const char *
Magick_ColorType_from_PNG_ColorType(const int ping_colortype)
{
  switch (ping_colortype)
  {
    case 0:
      return "Grayscale";

    case 2:
      return "Truecolor";

    case 3:
      return "Indexed";

    case 4:
      return "GrayAlpha";

    case 6:
      return "RGBA";

    default:
      return "UndefinedColorType";
    }
}

#endif /* MAGICKCORE_PNG_DELEGATE */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s M N G                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMNG() returns MagickTrue if the image format type, identified by the
%  magick string, is MNG.
%
%  The format of the IsMNG method is:
%
%      MagickBooleanType IsMNG(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static MagickBooleanType IsMNG(const unsigned char *magick,const size_t length)
{
  if (length < 8)
    return(MagickFalse);

  if (memcmp(magick,"\212MNG\r\n\032\n",8) == 0)
    return(MagickTrue);

  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s J N G                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsJNG() returns MagickTrue if the image format type, identified by the
%  magick string, is JNG.
%
%  The format of the IsJNG method is:
%
%      MagickBooleanType IsJNG(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static MagickBooleanType IsJNG(const unsigned char *magick,const size_t length)
{
  if (length < 8)
    return(MagickFalse);

  if (memcmp(magick,"\213JNG\r\n\032\n",8) == 0)
    return(MagickTrue);

  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P N G                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPNG() returns MagickTrue if the image format type, identified by the
%  magick string, is PNG.
%
%  The format of the IsPNG method is:
%
%      MagickBooleanType IsPNG(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsPNG(const unsigned char *magick,const size_t length)
{
  if (length < 8)
    return(MagickFalse);

  if (memcmp(magick,"\211PNG\r\n\032\n",8) == 0)
    return(MagickTrue);

  return(MagickFalse);
}

#if defined(MAGICKCORE_PNG_DELEGATE)
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static size_t WriteBlobMSBULong(Image *image,const size_t value)
{
  unsigned char
    buffer[4];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  buffer[0]=(unsigned char) (value >> 24);
  buffer[1]=(unsigned char) (value >> 16);
  buffer[2]=(unsigned char) (value >> 8);
  buffer[3]=(unsigned char) value;
  return((size_t) WriteBlob(image,4,buffer));
}

static void PNGLong(png_bytep p,png_uint_32 value)
{
  *p++=(png_byte) ((value >> 24) & 0xff);
  *p++=(png_byte) ((value >> 16) & 0xff);
  *p++=(png_byte) ((value >> 8) & 0xff);
  *p++=(png_byte) (value & 0xff);
}

static void PNGsLong(png_bytep p,png_int_32 value)
{
  *p++=(png_byte) ((value >> 24) & 0xff);
  *p++=(png_byte) ((value >> 16) & 0xff);
  *p++=(png_byte) ((value >> 8) & 0xff);
  *p++=(png_byte) (value & 0xff);
}

static void PNGShort(png_bytep p,png_uint_16 value)
{
  *p++=(png_byte) ((value >> 8) & 0xff);
  *p++=(png_byte) (value & 0xff);
}

static void PNGType(png_bytep p,const png_byte *type)
{
  (void) memcpy(p,type,4*sizeof(png_byte));
}

static void LogPNGChunk(MagickBooleanType logging, const png_byte *type,
   size_t length)
{
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Writing %c%c%c%c chunk, length: %.20g",
      type[0],type[1],type[2],type[3],(double) length);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P N G I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPNGImage() reads a Portable Network Graphics (PNG) or
%  Multiple-image Network Graphics (MNG) image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image or set of images.
%
%  MNG support written by Glenn Randers-Pehrson, glennrp@image...
%
%  The format of the ReadPNGImage method is:
%
%     Image *ReadPNGImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
%  To do, more or less in chronological order (as of version 5.5.2,
%   November 26, 2002 -- glennrp -- see also "To do" under WriteMNGImage):
%
%    Get 16-bit cheap transparency working.
%
%    (At this point, PNG decoding is supposed to be in full MNG-LC compliance)
%
%    Preserve all unknown and not-yet-handled known chunks found in input
%    PNG file and copy them into output PNG files according to the PNG
%    copying rules.
%
%    (At this point, PNG encoding should be in full MNG compliance)
%
%    Provide options for choice of background to use when the MNG BACK
%    chunk is not present or is not mandatory (i.e., leave transparent,
%    user specified, MNG BACK, PNG bKGD)
%
%    Implement LOOP/ENDL [done, but could do discretionary loops more
%    efficiently by linking in the duplicate frames.].
%
%    Decode and act on the MHDR simplicity profile (offer option to reject
%    files or attempt to process them anyway when the profile isn't LC or VLC).
%
%    Upgrade to full MNG without Delta-PNG.
%
%        o  BACK [done a while ago except for background image ID]
%        o  MOVE [done 15 May 1999]
%        o  CLIP [done 15 May 1999]
%        o  DISC [done 19 May 1999]
%        o  SAVE [partially done 19 May 1999 (marks objects frozen)]
%        o  SEEK [partially done 19 May 1999 (discard function only)]
%        o  SHOW
%        o  PAST
%        o  BASI
%        o  MNG-level tEXt/iTXt/zTXt
%        o  pHYg
%        o  pHYs
%        o  sBIT
%        o  bKGD
%        o  iTXt (wait for libpng implementation).
%
%    Use the scene signature to discover when an identical scene is
%    being reused, and just point to the original image->exception instead
%    of storing another set of pixels.  This not specific to MNG
%    but could be applied generally.
%
%    Upgrade to full MNG with Delta-PNG.
%
%    JNG tEXt/iTXt/zTXt
%
%    We will not attempt to read files containing the CgBI chunk.
%    They are really Xcode files meant for display on the iPhone.
%    These are not valid PNG files and it is impossible to recover
%    the original PNG from files that have been converted to Xcode-PNG,
%    since irretrievable loss of color data has occurred due to the
%    use of premultiplied alpha.
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _PNGErrorInfo
{
  Image
    *image;

  ExceptionInfo
    *exception;
} PNGErrorInfo;

static void MagickPNGError(png_struct *ping,const ExceptionType severity,
  png_const_charp message)
{
  ExceptionInfo
    *exception;

  Image
    *image;

  PNGErrorInfo
    *error_info;

  error_info=(PNGErrorInfo *) png_get_error_ptr(ping);
  image=error_info->image;
  exception=error_info->exception;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  libpng-%s error: %s", png_get_libpng_ver(NULL),message);

  (void) ThrowMagickException(exception,GetMagickModule(),severity,message,
    "`%s'",image->filename);

#if (PNG_LIBPNG_VER < 10500)
  /* A warning about deprecated use of jmpbuf here is unavoidable if you
   * are building with libpng-1.4.x and can be ignored.
   */
  longjmp(ping->jmpbuf,1);
#else
  png_longjmp(ping,1);
#endif
}

/*
  This the function that does the actual reading of data.  It is
  the same as the one supplied in libpng, except that it receives the
  datastream from the ReadBlob() function instead of standard input.
*/
static void png_get_data(png_structp png_ptr,png_bytep data,png_size_t length)
{
  Image
    *image;

  image=(Image *) png_get_io_ptr(png_ptr);
  if (length != 0)
    {
      png_size_t
        check;

      check=(png_size_t) ReadBlob(image,(size_t) length,data);
      if (check != length)
        {
          char
            msg[MagickPathExtent];

          if (check < length)
            (void) memset(data+check,0,length-check);
          (void) FormatLocaleString(msg,MagickPathExtent,
            "Expected %.20g bytes; found %.20g bytes",(double) length,
            (double) check);
          png_warning(png_ptr,msg);
          MagickPNGError(png_ptr,CorruptImageError,"UnexpectedEndOfFile");
        }
    }
}

static void png_put_data(png_structp png_ptr,png_bytep data,png_size_t length)
{
  Image
    *image;

  image=(Image *) png_get_io_ptr(png_ptr);
  if (length != 0)
    {
      png_size_t
        check;

      check=(png_size_t) WriteBlob(image,(size_t) length,data);

      if (check != length)
        MagickPNGError(png_ptr,BlobError,"UnableToWriteBlob");
    }
}

static void png_flush_data(png_structp png_ptr)
{
  (void) png_ptr;
}

static MagickBooleanType PalettesAreEqual(Image *a,Image *b)
{
  ssize_t
    i;

  if ((a == (Image *) NULL) || (b == (Image *) NULL))
    return(MagickFalse);

  if (a->storage_class != PseudoClass || b->storage_class != PseudoClass)
    return(MagickFalse);

  if (a->colors != b->colors)
    return(MagickFalse);

  for (i=0; i < (ssize_t) a->colors; i++)
  {
    if ((a->colormap[i].red != b->colormap[i].red) ||
        (a->colormap[i].green != b->colormap[i].green) ||
        (a->colormap[i].blue != b->colormap[i].blue))
      return(MagickFalse);
  }

  return(MagickTrue);
}

static void MngReadInfoDiscardObject(MngReadInfo *mng_info,int i)
{
  if (i && (i < MNG_MAX_OBJECTS) && (mng_info != (MngReadInfo *) NULL) &&
      mng_info->exists[i] && !mng_info->frozen[i])
    {
      mng_info->exists[i]=MagickFalse;
      mng_info->invisible[i]=MagickFalse;
      mng_info->viewable[i]=MagickFalse;
      mng_info->frozen[i]=MagickFalse;
      mng_info->x_off[i]=0;
      mng_info->y_off[i]=0;
      mng_info->object_clip[i].left=0;
      mng_info->object_clip[i].right=(ssize_t) PNG_UINT_31_MAX;
      mng_info->object_clip[i].top=0;
      mng_info->object_clip[i].bottom=(ssize_t) PNG_UINT_31_MAX;
    }
}

static MngReadInfo *MngReadInfoFreeStruct(MngReadInfo *mng_info)
{
  int
    i;

  if (mng_info == (MngReadInfo *) NULL)
    return((MngReadInfo *) NULL);

  for (i=1; i < MNG_MAX_OBJECTS; i++)
    MngReadInfoDiscardObject(mng_info,i);

  mng_info->global_plte=(png_colorp)
    RelinquishMagickMemory(mng_info->global_plte);

  return((MngReadInfo *) RelinquishMagickMemory(mng_info));
}

static MngBox mng_minimum_box(MngBox box1,MngBox box2)
{
  MngBox
    box;

  box=box1;
  if (box.left < box2.left)
    box.left=box2.left;

  if (box.top < box2.top)
    box.top=box2.top;

  if (box.right > box2.right)
    box.right=box2.right;

  if (box.bottom > box2.bottom)
    box.bottom=box2.bottom;

  return box;
}

static long mng_get_long(unsigned char *p)
{
  return ((long) (((png_uint_32) p[0] << 24) | ((png_uint_32) p[1] << 16) |
    ((png_uint_32) p[2] << 8) | (png_uint_32) p[3]));
}

static MngBox mng_read_box(MngBox previous_box,char delta_type,
  unsigned char *p)
{
   MngBox
      box;

  /*
    Read clipping boundaries from DEFI, CLIP, FRAM, or PAST chunk.
  */
  box.left=mng_get_long(p);
  box.right=mng_get_long(&p[4]);
  box.top=mng_get_long(&p[8]);
  box.bottom=mng_get_long(&p[12]);
  if (delta_type != 0)
    {
      box.left+=previous_box.left;
      box.right+=previous_box.right;
      box.top+=previous_box.top;
      box.bottom+=previous_box.bottom;
    }

  return(box);
}

static MngPair mng_read_pair(MngPair previous_pair,int delta_type,
  unsigned char *p)
{
  MngPair
    pair;

  /*
    Read two ssize_t's from CLON, MOVE or PAST chunk
  */
  pair.a=mng_get_long(p);
  pair.b=mng_get_long(&p[4]);
  if (delta_type != 0)
    {
      pair.a+=previous_pair.a;
      pair.b+=previous_pair.b;
    }

  return(pair);
}

static void MagickPNGErrorHandler(png_struct *ping,png_const_charp message)
{
  MagickPNGError(ping,CoderError,message);
}

static void MagickPNGWarningHandler(png_struct *ping,png_const_charp message)
{
  ExceptionInfo
    *exception;

  Image
    *image;

  PNGErrorInfo
    *error_info;

  if (LocaleCompare(message, "Missing PLTE before tRNS") == 0)
    png_error(ping, message);

  error_info=(PNGErrorInfo *) png_get_error_ptr(ping);
  image=error_info->image;
  exception=error_info->exception;
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  libpng-%s warning: %s", png_get_libpng_ver(NULL),message);

  (void) ThrowMagickException(exception,GetMagickModule(),CoderWarning,
    message,"`%s'",image->filename);
}

#ifdef PNG_USER_MEM_SUPPORTED
#if PNG_LIBPNG_VER >= 10400
static png_voidp Magick_png_malloc(png_structp png_ptr,png_alloc_size_t size)
#else
static png_voidp Magick_png_malloc(png_structp png_ptr,png_size_t size)
#endif
{
  (void) png_ptr;
  return((png_voidp) AcquireQuantumMemory(1,(size_t) size));
}

/*
  Free a pointer.  It is removed from the list at the same time.
*/
static void Magick_png_free(png_structp png_ptr,png_voidp ptr)
{
  (void) png_ptr;
  ptr=RelinquishMagickMemory(ptr);
}
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static int
Magick_png_read_raw_profile(png_struct *ping,Image *image,
   const ImageInfo *image_info, png_textp text,int ii,ExceptionInfo *exception)
{
  png_charp
    ep;

  ssize_t
    i;

  unsigned char
    *dp;

  png_charp
    sp;

  size_t
    extent,
    nibbles;

  ssize_t
    length=0;

  StringInfo
    *profile;

  static const unsigned char
    unhex[103]={0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                 0,0,0,0,0,0,0,0,0,1, 2,3,4,5,6,7,8,9,0,0,
                 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,10,11,12,
                 13,14,15};

  sp=text[ii].text+1;
  extent=text[ii].text_length;
  ep=text[ii].text+extent;
  if (ep <= sp)
    {
      png_warning(ping,"invalid profile length");
      return(MagickFalse);
    }
  /* look for newline */
  while ((extent != 0) && (*sp != '\n') && extent--)
    sp++;

  /* look for length */
  while ((extent != 0) && ((*sp == '\0' || *sp == ' ' || *sp == '\n')) && extent--)
     sp++;

  if (extent == 0)
    {
      png_warning(ping,"invalid profile length");
      return(MagickFalse);
    }

  if (extent >= 8)
    length=StringToLong(sp);

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
         "      length: %lu",(unsigned long) length);

  while ((extent != 0) && (*sp != ' ' && *sp != '\n') && extent--)
    sp++;

  if (extent == 0)
    {
      png_warning(ping,"missing profile length");
      return(MagickFalse);
    }

  /* allocate space */
  if (length == 0)
  {
    png_warning(ping,"invalid profile length");
    return(MagickFalse);
  }

  profile=AcquireProfileStringInfo(&text[ii].key[17],(size_t) length,
    exception);

  if (profile == (StringInfo *) NULL)
    return(MagickTrue);

  /* copy profile, skipping white space and column 1 "=" signs */
  dp=GetStringInfoDatum(profile);
  nibbles=(size_t) length*2;

  for (i=0; i < (ssize_t) nibbles; i++)
  {
    while ((extent != 0) && (*sp < '0' || (*sp > '9' && *sp < 'a') || *sp > 'f'))
    {
      if (*sp == '\0')
        {
          png_warning(ping, "ran out of profile data");
          profile=DestroyStringInfo(profile);
          return(MagickFalse);
        }
      sp++;
      extent--;
    }
    if (extent != 0)
      {
        if (i % 2 == 0)
          *dp=(unsigned char) (16*unhex[(int) *sp++]);
        else
          (*dp++)+=unhex[(int) *sp++];
        extent--;
      }
    if (extent == 0)
      break;
  }
  /*
    We have already read "Raw profile type.
  */
  (void) SetImageProfilePrivate(image,profile,exception);

  if (image_info->verbose)
    (void) printf(" Found a generic profile, type %s\n",&text[ii].key[17]);

  return(MagickTrue);
}

static int PNGSetExifProfile(Image *image,png_byte *data,png_size_t size,
  ExceptionInfo *exception)
{
  StringInfo
    *profile;

  if ((data == (png_byte *) NULL) || (size < 6))
    return(1);
  if ((data[0] == 'E') && (data[1] == 'x') && (data[2] == 'i') &&
      (data[3] == 'f') && (data[4] == '\0') && (data[5] == '\0'))
    profile=BlobToProfileStringInfo("exif",(const void *) data,size,exception);
  else
    {
      unsigned char
        *p;

      profile=AcquireProfileStringInfo("exif",size+6,exception);
      if (profile != (StringInfo *) NULL)
        {
          p=GetStringInfoDatum(profile);
          *p++ ='E';
          *p++ ='x';
          *p++ ='i';
          *p++ ='f';
          *p++ ='\0';
          *p++ ='\0';
          (void) CopyMagickMemory(p,data,size);
        }
    }
  if (profile != (StringInfo*)NULL)
    (void) SetImageProfilePrivate(image,profile,exception);
  return(1);
}

#if defined(PNG_READ_eXIf_SUPPORTED)
static void read_eXIf_chunk(Image *image,png_struct *ping,png_info *info,
  ExceptionInfo *exception)
{
  png_uint_32
    size;

  png_bytep
    data;

#if PNG_LIBPNG_VER > 10631
  if (png_get_eXIf_1(ping,info,&size,&data))
    (void) PNGSetExifProfile(image,data,size,exception);
#endif
}
#endif

#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
static int PNGParseiTXt(Image *image,png_byte *data,png_size_t size,
  ExceptionInfo *exception)
{
  char
    compressed,
    key[MagickPathExtent],
    language[MagickPathExtent],
    method;

  ssize_t
    offset;

  StringInfo
    *profile,
    *property;

  if ((size > 19) && (LocaleNCompare((const char *) data,
      "XML:com.adobe.xmp",17) == 0) && (data[18] == 0) && (data[19] == 0))
    {
      /*
        XMP profile.
      */
      offset=20;
      while (offset < (ssize_t) size)
      {
        if (data[offset++] == 0)
          break;
      }
      while (offset < (ssize_t) size)
      {
        if (data[offset++] == 0)
          break;
      }
      if (((MagickOffsetType) size-offset) < 1)
        return(0);
      profile=BlobToProfileStringInfo("xmp",(const void *) (data+offset),
        size-(size_t) offset,exception);
      (void) SetImageProfilePrivate(image,profile,exception);
      return(1);
    }
  /*
    iTXt chunk.
  */
  profile=BlobToStringInfo((const void *) data,size);
  if (profile == (StringInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(-1);
    }
  (void) FormatLocaleString(key,MagickPathExtent,"%s",(const char *)
    GetStringInfoDatum(profile));
  offset=(ssize_t) strlen(key)+1;
  if (((MagickOffsetType) size-offset) < 1)
    {
      profile=DestroyStringInfo(profile);
      return(0);
    }
  compressed=(char) data[offset++];
  if ((((MagickOffsetType) size-offset) < 1) || (compressed != 0))
    {
      profile=DestroyStringInfo(profile);
      return(0);
    }
  method=(char) data[offset++];
  if ((((MagickOffsetType) size-offset) < 1) || (method != 0))
    {
      profile=DestroyStringInfo(profile);
      return(0);
    }
  (void) FormatLocaleString(language,MagickPathExtent,"%s",(const char *)
    GetStringInfoDatum(profile)+offset++);
  offset+=(ssize_t) strlen(language)+1;
  if (((MagickOffsetType) size-offset) < 1)
    {
      profile=DestroyStringInfo(profile);
      return(0);
    }
  property=BlobToStringInfo((const void *) (GetStringInfoDatum(profile)+offset),
    size-(size_t) offset);
  if (property == (StringInfo *) NULL)
    {
      profile=DestroyStringInfo(profile);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(-1);
    }
  (void) SetImageProperty(image,key,(const char *) GetStringInfoDatum(property),
    exception);
  property=DestroyStringInfo(property);
  profile=DestroyStringInfo(profile);
  return(1);
}

static int read_user_chunk_callback(png_struct *ping, png_unknown_chunkp chunk)
{
  Image
    *image;

  PNGErrorInfo
    *error_info;

  /* The unknown chunk structure contains the chunk data:
     png_byte name[5];
     png_byte *data;
     png_size_t size;

     Note that libpng has already taken care of the CRC handling.

     Returns one of the following:
         return(-n);  chunk had an error
         return(0);  did not recognize
         return(n);  success
  */

  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
       "    read_user_chunk: found %c%c%c%c chunk",
         chunk->name[0],chunk->name[1],chunk->name[2],chunk->name[3]);

  if ((chunk->name[0] == 101) &&
      ((chunk->name[1] == 88) || (chunk->name[1] == 120)) &&
      (chunk->name[2] ==  73) && (chunk->name[3] == 102))
    {
      image=(Image *) png_get_user_chunk_ptr(ping);

      error_info=(PNGErrorInfo *) png_get_error_ptr(ping);

      return(PNGSetExifProfile(image,chunk->data,chunk->size,
        error_info->exception));
    }

  /* orNT */
  if ((chunk->name[0] == 111) && (chunk->name[1] == 114) &&
      (chunk->name[2] ==  78) && (chunk->name[3] ==  84))
    {
     if (chunk->size != 1)
       return(-1);

     image=(Image *) png_get_user_chunk_ptr(ping);

     image->orientation=
       Magick_Orientation_from_Exif_Orientation((int) chunk->data[0]);

     return(1);
    }

  /* vpAg (deprecated, replaced by caNv) */
  if ((chunk->name[0] == 118) && (chunk->name[1] == 112) &&
      (chunk->name[2] ==  65) && (chunk->name[3] == 103))
    {
      if (chunk->size != 9)
        return(-1);

       /* ImageMagick requires pixel units */
      if (chunk->data[8] != 0)
        return(0);

      image=(Image *) png_get_user_chunk_ptr(ping);

      image->page.width=(size_t)mng_get_long(chunk->data);
      image->page.height=(size_t)mng_get_long(&chunk->data[4]);

      return(1);
    }

  /* caNv */
  if ((chunk->name[0] == 99) && (chunk->name[1] ==  97) &&
      (chunk->name[2] == 78) && (chunk->name[3] == 118))
    {
      if (chunk->size != 16)
        return(-1); /* Error return */

      image=(Image *) png_get_user_chunk_ptr(ping);

      image->page.width=(size_t) mng_get_long(chunk->data);
      image->page.height=(size_t) mng_get_long(&chunk->data[4]);
      image->page.x=(ssize_t) ((int) mng_get_long(&chunk->data[8]));
      image->page.y=(ssize_t) ((int) mng_get_long(&chunk->data[12]));

      return(1);
    }

  /* acTL */
  if ((chunk->name[0] == 97) && (chunk->name[1] == 99) &&
      (chunk->name[2] == 84) && (chunk->name[3] == 76))
    {
      image=(Image *) png_get_user_chunk_ptr(ping);
      error_info=(PNGErrorInfo *) png_get_error_ptr(ping);

      (void) SetImageProperty(image,"png:acTL","chunk was found",
        error_info->exception);

      return(1);
    }

  /* iTXt */
  if ((chunk->name[0] == 105) && (chunk->name[1] ==  84) &&
      (chunk->name[2] ==  88) && (chunk->name[3] == 116))
    {
      image=(Image *) png_get_user_chunk_ptr(ping);

      error_info=(PNGErrorInfo *) png_get_error_ptr(ping);

      return(PNGParseiTXt(image,chunk->data,chunk->size,
        error_info->exception));
    }

  return(0); /* Did not recognize */
}
#endif /* PNG_UNKNOWN_CHUNKS_SUPPORTED */

#if defined(PNG_tIME_SUPPORTED)
static void read_tIME_chunk(Image *image,png_struct *ping,png_info *info,
  ExceptionInfo *exception)
{
  png_timep
    time;

  if (png_get_tIME(ping,info,&time))
    {
      char
        timestamp[21];

      FormatLocaleString(timestamp,21,"%04d-%02d-%02dT%02d:%02d:%02dZ",
        time->year,time->month,time->day,time->hour,time->minute,time->second);
      SetImageProperty(image,"png:tIME",timestamp,exception);
    }
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d O n e P N G I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadOnePNGImage() reads a Portable Network Graphics (PNG) image file
%  (minus the 8-byte signature)  and returns it.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  The format of the ReadOnePNGImage method is:
%
%      Image *ReadOnePNGImage(MngReadInfo *mng_info,const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o mng_info: Specifies a pointer to a MngReadInfo structure.
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadOnePNGImage(MngReadInfo *mng_info,
  const ImageInfo *image_info,ExceptionInfo *exception)
{
  /* Read one PNG image */

  /* To do: Read the tEXt/Creation Time chunk into the date:create property */

  double
    file_gamma;

  Image
    *image;

  int
    intent, /* "PNG Rendering intent", which is ICC intent + 1 */
    num_raw_profiles,
    num_text,
    num_text_total,
    num_passes,
    number_colors = 0,
    pass,
    ping_bit_depth = 0,
    ping_color_type = 0,
    ping_file_depth = 0,
    ping_interlace_method = 0,
    ping_compression_method = 0,
    ping_filter_method = 0,
    ping_num_trans = 0,
    unit_type = 0;

  MagickBooleanType
    logging = MagickFalse,
    ping_found_cHRM,
    ping_found_gAMA,
    ping_found_iCCP,
    ping_found_sRGB,
    ping_preserve_iCCP,
    status;

  MemoryInfo
    *volatile pixel_info;

  PixelInfo
    transparent_color;

  PNGErrorInfo
    error_info;

  png_bytep
     ping_trans_alpha = NULL;

  png_color_16p
     ping_background = (png_color_16p) NULL,
     ping_trans_color = (png_color_16p) NULL;

  png_info
    *end_info,
    *ping_info;

  png_struct
    *ping;

  png_textp
    text;

  png_uint_32
    ping_height = 0,
    ping_width = 0,
    x_resolution,
    y_resolution;

  QuantumInfo
    *volatile quantum_info;

  Quantum
    *volatile quantum_scanline;

  Quantum
    *q;

  size_t
    length,
    ping_rowbytes = 0,
    row_offset;

  ssize_t
    i,
    j,
    x,
    y;

  unsigned char
    *p,
    *ping_pixels;

#ifdef PNG_UNKNOWN_CHUNKS_SUPPORTED
  static const png_byte unused_chunks[]=
  {
    104,  73,  83,  84, (png_byte) '\0',   /* hIST */
    105,  84,  88, 116, (png_byte) '\0',   /* iTXt */
    112,  67,  65,  76, (png_byte) '\0',   /* pCAL */
    115,  67,  65,  76, (png_byte) '\0',   /* sCAL */
    115,  80,  76,  84, (png_byte) '\0',   /* sPLT */
#if !defined(PNG_tIME_SUPPORTED)
    116,  73,  77,  69, (png_byte) '\0',   /* tIME */
#endif
#ifdef PNG_APNG_SUPPORTED /* libpng was built with APNG patch; */
                          /* ignore the APNG chunks */
     97,  99,  84,  76, (png_byte) '\0',   /* acTL */
    102,  99,  84,  76, (png_byte) '\0',   /* fcTL */
    102, 100,  65,  84, (png_byte) '\0',   /* fdAT */
#endif
  };
#endif

  logging=IsEventLogging();
  if (logging != MagickFalse)
    {
      char
        im_vers[32],
        libpng_runv[32],
        libpng_vers[32],
        zlib_runv[32],
        zlib_vers[32];

      *im_vers='\0';
      (void) ConcatenateMagickString(im_vers,
             MagickLibVersionText,32);
      (void) ConcatenateMagickString(im_vers,
             MagickLibAddendum,32);

      *libpng_vers='\0';
      (void) ConcatenateMagickString(libpng_vers,
             PNG_LIBPNG_VER_STRING,32);
      *libpng_runv='\0';
      (void) ConcatenateMagickString(libpng_runv,
             png_get_libpng_ver(NULL),32);

      *zlib_vers='\0';
      (void) ConcatenateMagickString(zlib_vers,
             ZLIB_VERSION,32);
      *zlib_runv='\0';
      (void) ConcatenateMagickString(zlib_runv,
             zlib_version,32);

      LogMagickEvent(CoderEvent,GetMagickModule(),
         "  Enter ReadOnePNGImage()\n"
         "    IM version     = %s\n"
         "    Libpng version = %s",
         im_vers, libpng_vers);

      if (LocaleCompare(libpng_vers,libpng_runv) != 0)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "      running with   %s", libpng_runv);
      if (LocaleCompare(libpng_vers,zlib_vers) != 0)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Zlib version   = %s", zlib_vers);
      if (LocaleCompare(zlib_vers,zlib_runv) != 0)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "      running with   %s", zlib_runv);
    }

  quantum_info = (QuantumInfo *) NULL;
  image=mng_info->image;

  if (logging != MagickFalse)
  {
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
       "    Before reading:\n"
       "      image->alpha_trait=%d\n"
       "      image->rendering_intent=%d\n"
       "      image->colorspace=%d\n"
       "      image->gamma=%f",
       (int) image->alpha_trait, (int) image->rendering_intent,
       (int) image->colorspace, image->gamma);
  }
  intent=
    Magick_RenderingIntent_to_PNG_RenderingIntent(image->rendering_intent);

  /* Set to an out-of-range color unless tRNS chunk is present */
  transparent_color.red=65537;
  transparent_color.green=65537;
  transparent_color.blue=65537;
  transparent_color.alpha=65537;

  number_colors=0;
  num_text = 0;
  num_text_total = 0;
  num_raw_profiles = 0;

  ping_found_cHRM = MagickFalse;
  ping_found_gAMA = MagickFalse;
  ping_found_iCCP = MagickFalse;
  ping_found_sRGB = MagickFalse;
  ping_preserve_iCCP = MagickFalse;


  /*
    Allocate the PNG structures
  */
#ifdef PNG_USER_MEM_SUPPORTED
 error_info.image=image;
 error_info.exception=exception;
 ping=png_create_read_struct_2(PNG_LIBPNG_VER_STRING,&error_info,
   MagickPNGErrorHandler,MagickPNGWarningHandler, NULL,
   (png_malloc_ptr) Magick_png_malloc,(png_free_ptr) Magick_png_free);
#else
  ping=png_create_read_struct(PNG_LIBPNG_VER_STRING,&error_info,
    MagickPNGErrorHandler,MagickPNGWarningHandler);
#endif
  if (ping == (png_struct *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");

  ping_info=png_create_info_struct(ping);

  if (ping_info == (png_info *) NULL)
    {
      png_destroy_read_struct(&ping,(png_info **) NULL,(png_info **) NULL);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }

  end_info=png_create_info_struct(ping);

  if (end_info == (png_info *) NULL)
    {
      png_destroy_read_struct(&ping,&ping_info,(png_info **) NULL);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }

  pixel_info=(MemoryInfo *) NULL;
  quantum_scanline = (Quantum *) NULL;
  quantum_info = (QuantumInfo *) NULL;

  if (setjmp(png_jmpbuf(ping)))
    {
      /*
        PNG image is corrupt.
      */
      png_destroy_read_struct(&ping,&ping_info,&end_info);

      if (pixel_info != (MemoryInfo *) NULL)
        pixel_info=RelinquishVirtualMemory(pixel_info);

      if (quantum_info != (QuantumInfo *) NULL)
        quantum_info=DestroyQuantumInfo(quantum_info);

      quantum_scanline=(Quantum *) RelinquishMagickMemory(quantum_scanline);

#ifdef IMPNG_SETJMP_NOT_THREAD_SAFE
      UnlockSemaphoreInfo(ping_semaphore);
#endif

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  exit ReadOnePNGImage() with error.");

      if (image != (Image *) NULL)
        image=DestroyImageList(image);
      return(image);
    }

  /* {  For navigation to end of SETJMP-protected block.  Within this
   *    block, use png_error() instead of Throwing an Exception, to ensure
   *    that libpng is able to clean up, and that the semaphore is unlocked.
   */

#ifdef IMPNG_SETJMP_NOT_THREAD_SAFE
  LockSemaphoreInfo(ping_semaphore);
#endif

#ifdef PNG_BENIGN_ERRORS_SUPPORTED
  /* Allow benign errors */
  png_set_benign_errors(ping, 1);
#endif

#ifdef PNG_SET_USER_LIMITS_SUPPORTED
  {
    const char
      *option;

    /* Reject images with too many rows or columns */
    png_set_user_limits(ping,(png_uint_32) MagickMin(PNG_UINT_31_MAX,
      GetMagickResourceLimit(WidthResource)),(png_uint_32)
      MagickMin(PNG_UINT_31_MAX,GetMagickResourceLimit(HeightResource)));

#if (PNG_LIBPNG_VER >= 10400)
    option=GetImageOption(image_info,"png:chunk-cache-max");
    if (option != (const char *) NULL)
      png_set_chunk_cache_max(ping,(png_uint_32) MagickMin(PNG_UINT_32_MAX,
        StringToLong(option)));
    else
      png_set_chunk_cache_max(ping,32767);
#endif

#if (PNG_LIBPNG_VER >= 10401)
    option=GetImageOption(image_info,"png:chunk-malloc-max");
    if (option != (const char *) NULL)
      png_set_chunk_malloc_max(ping,(png_alloc_size_t) MagickMin(
        MAGICK_SSIZE_MAX,(size_t) StringToLong(option)));
#endif
  }
#endif /* PNG_SET_USER_LIMITS_SUPPORTED */

  /*
    Prepare PNG for reading.
  */

  mng_info->image_found++;
  png_set_sig_bytes(ping,8);

  if (LocaleCompare(image_info->magick,"MNG") == 0)
    (void) png_permit_mng_features(ping,PNG_ALL_MNG_FEATURES);
  png_set_read_fn(ping,image,png_get_data);

  {
    const char
      *value;

    value=GetImageOption(image_info,"png:ignore-crc");
    if (IsStringTrue(value) != MagickFalse)
    {
       /* Turn off CRC checking while reading */
       png_set_crc_action(ping, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);
#ifdef PNG_IGNORE_ADLER32
       /* Turn off ADLER32 checking while reading */
       png_set_option(ping, PNG_IGNORE_ADLER32, PNG_OPTION_ON);
#endif
     }

    value=GetImageOption(image_info,"profile:skip");

    if (IsOptionMember("ICC",value) == MagickFalse)
    {

       value=GetImageOption(image_info,"png:preserve-iCCP");

       if (value == NULL)
          value=GetImageArtifact(image,"png:preserve-iCCP");

       if (value != NULL)
          ping_preserve_iCCP=MagickTrue;

#if defined(PNG_SKIP_sRGB_CHECK_PROFILE) && defined(PNG_SET_OPTION_SUPPORTED)
       /* Don't let libpng check for ICC/sRGB profile because we're going
        * to do that anyway.  This feature was added at libpng-1.6.12.
        * If logging, go ahead and check and issue a warning as appropriate.
        */
       if (logging == MagickFalse)
          png_set_option(ping, PNG_SKIP_sRGB_CHECK_PROFILE, PNG_OPTION_ON);
#endif
    }
#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
    else
    {
       png_set_keep_unknown_chunks(ping, 1, (png_bytep) mng_iCCP, 1);
    }
#endif
  }
#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
  /* Ignore unused chunks and all unknown chunks except for caNv and vpAg */
# if PNG_LIBPNG_VER < 10700 /* Avoid libpng16 warning */
  png_set_keep_unknown_chunks(ping, 2, NULL, 0);
# else
  png_set_keep_unknown_chunks(ping, 1, NULL, 0);
# endif
  png_set_keep_unknown_chunks(ping, 2, (png_bytep) mng_caNv, 1);
  png_set_keep_unknown_chunks(ping, 2, (png_bytep) mng_vpAg, 1);
  png_set_keep_unknown_chunks(ping, 1, unused_chunks,
     (int)sizeof(unused_chunks)/5);
  /* Callback for other unknown chunks */
  png_set_read_user_chunk_fn(ping, image, read_user_chunk_callback);
#endif

#ifdef PNG_READ_CHECK_FOR_INVALID_INDEX_SUPPORTED
    /* Disable new libpng-1.5.10 feature */
    png_set_check_for_invalid_index (ping, 0);
#endif

#if (PNG_LIBPNG_VER < 10400)
#  if defined(PNG_USE_PNGGCCRD) && defined(PNG_ASSEMBLER_CODE_SUPPORTED) && \
   (PNG_LIBPNG_VER >= 10200) && (PNG_LIBPNG_VER < 10220) && defined(__i386__)
  /* Disable thread-unsafe features of pnggccrd */
  if (png_access_version_number() >= 10200)
  {
    png_uint_32 mmx_disable_mask=0;
    png_uint_32 asm_flags;

    mmx_disable_mask |= ( PNG_ASM_FLAG_MMX_READ_COMBINE_ROW  \
                        | PNG_ASM_FLAG_MMX_READ_FILTER_SUB   \
                        | PNG_ASM_FLAG_MMX_READ_FILTER_AVG   \
                        | PNG_ASM_FLAG_MMX_READ_FILTER_PAETH );
    asm_flags=png_get_asm_flags(ping);
    png_set_asm_flags(ping, asm_flags & ~mmx_disable_mask);
  }
#  endif
#endif

  png_read_info(ping,ping_info);

  png_get_IHDR(ping,ping_info,&ping_width,&ping_height,
               &ping_bit_depth,&ping_color_type,
               &ping_interlace_method,&ping_compression_method,
               &ping_filter_method);

  ping_file_depth = ping_bit_depth;

  /* Swap bytes if requested */
  if (ping_file_depth == 16)
  {
     const char
       *value;

     value=GetImageOption(image_info,"png:swap-bytes");

     if (value == NULL)
        value=GetImageArtifact(image,"png:swap-bytes");

     if (value != NULL)
        png_set_swap(ping);
  }

  /* Save bit-depth and color-type in case we later want to write a PNG00 */
  {
      char
        msg[MagickPathExtent];

      (void) FormatLocaleString(msg,MagickPathExtent,"%d",
         (int) ping_color_type);
      (void) SetImageProperty(image,"png:IHDR.color-type-orig",msg,exception);

      (void) FormatLocaleString(msg,MagickPathExtent,"%d",
         (int) ping_bit_depth);
      (void) SetImageProperty(image,"png:IHDR.bit-depth-orig",msg,exception);
  }

  (void) png_get_tRNS(ping, ping_info, &ping_trans_alpha, &ping_num_trans,
                      &ping_trans_color);

  (void) png_get_bKGD(ping, ping_info, &ping_background);

  if (ping_bit_depth < 8)
    {
       png_set_packing(ping);
       ping_bit_depth = 8;
    }

  image->depth=(size_t) ping_bit_depth;
  image->depth=GetImageQuantumDepth(image,MagickFalse);
  image->interlace=ping_interlace_method != 0 ? PNGInterlace : NoInterlace;

  if (((int) ping_color_type == PNG_COLOR_TYPE_GRAY) ||
      ((int) ping_color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
    {
      image->rendering_intent=UndefinedIntent;
      intent=Magick_RenderingIntent_to_PNG_RenderingIntent(UndefinedIntent);
      (void) memset(&image->chromaticity,0,sizeof(image->chromaticity));
    }

  if (logging != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    PNG width: %.20g, height: %.20g\n"
        "    PNG color_type: %d, bit_depth: %d\n"
        "    PNG compression_method: %d\n"
        "    PNG interlace_method: %d, filter_method: %d",
        (double) ping_width, (double) ping_height,
        ping_color_type, ping_bit_depth,
        ping_compression_method,
        ping_interlace_method,ping_filter_method);

    }

  if (png_get_valid(ping,ping_info, PNG_INFO_iCCP))
    {
      ping_found_iCCP=MagickTrue;
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Found PNG iCCP chunk.");
    }

  if (png_get_valid(ping,ping_info,PNG_INFO_gAMA))
    {
      ping_found_gAMA=MagickTrue;
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Found PNG gAMA chunk.");
    }

  if (png_get_valid(ping,ping_info,PNG_INFO_cHRM))
    {
      ping_found_cHRM=MagickTrue;
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Found PNG cHRM chunk.");
    }

  if (ping_found_iCCP != MagickTrue && png_get_valid(ping,ping_info,
      PNG_INFO_sRGB))
    {
      ping_found_sRGB=MagickTrue;
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Found PNG sRGB chunk.");
    }

#ifdef PNG_READ_iCCP_SUPPORTED
    if (ping_found_iCCP !=MagickTrue &&
      ping_found_sRGB != MagickTrue &&
      png_get_valid(ping,ping_info, PNG_INFO_iCCP))
    {
      ping_found_iCCP=MagickTrue;
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Found PNG iCCP chunk.");
    }

  if (png_get_valid(ping,ping_info,PNG_INFO_iCCP))
    {
      int
        compression;

#if (PNG_LIBPNG_VER < 10500)
      png_charp
        info;
#else
      png_bytep
        info;
#endif

      png_charp
        name;

      png_uint_32
        profile_length;

      (void) png_get_iCCP(ping,ping_info,&name,(int *) &compression,&info,
        &profile_length);

      if (profile_length != 0)
        {
          StringInfo
            *profile;

          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Reading PNG iCCP chunk.");

          profile=BlobToProfileStringInfo("icc",info,(size_t) profile_length,
            exception);

          if (profile != (StringInfo *) NULL)
          {
            if (ping_preserve_iCCP != MagickFalse)
              (void) SetImageProfilePrivate(image,profile,exception);
            else
            {
              int
                icheck,
                got_crc=0;


              png_uint_32
                profile_crc=0;

              unsigned char
                *data;

              profile_length=(png_uint_32) GetStringInfoLength(profile);

              for (icheck=0; sRGB_info[icheck].len > 0; icheck++)
              {
                if (profile_length == sRGB_info[icheck].len)
                {
                  if (got_crc == 0)
                  {
                    if (logging != MagickFalse)
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "    Got a %lu-byte ICC profile (potentially sRGB)",
                        (unsigned long) profile_length);

                    data=GetStringInfoDatum(profile);
                    profile_crc=crc32(0,data,profile_length);

                    if (logging != MagickFalse)
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "      with crc=%8x",(unsigned int) profile_crc);
                    got_crc++;
                  }

                  if (profile_crc == sRGB_info[icheck].crc)
                  {
                    if (logging != MagickFalse)
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "      It is sRGB with rendering intent = %s",
                    Magick_RenderingIntentString_from_PNG_RenderingIntent(
                          sRGB_info[icheck].intent));
                    if (image->rendering_intent==UndefinedIntent)
                    {
                      image->rendering_intent=
                      Magick_RenderingIntent_from_PNG_RenderingIntent(
                          sRGB_info[icheck].intent);
                    }
                    break;
                  }
                }
              }
              if (sRGB_info[icheck].len != 0)
                profile=DestroyStringInfo(profile);
              else
                {
                  if (logging != MagickFalse)
                    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "    Got %lu-byte ICC profile not recognized as sRGB",
                        (unsigned long) profile_length);
                  (void) SetImageProfilePrivate(image,profile,exception);
                }
            }
          }
      }
    }
#endif

#if defined(PNG_READ_sRGB_SUPPORTED)
  {
    if (ping_found_iCCP==MagickFalse && png_get_valid(ping,ping_info,
        PNG_INFO_sRGB))
    {
      if (png_get_sRGB(ping,ping_info,&intent))
      {
        if (image->rendering_intent == UndefinedIntent)
          image->rendering_intent=
             Magick_RenderingIntent_from_PNG_RenderingIntent (intent);

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Reading PNG sRGB chunk: rendering_intent: %d",intent);
      }
    }

    else if (mng_info->have_global_srgb != MagickFalse)
      {
        if (image->rendering_intent == UndefinedIntent)
          image->rendering_intent=
            Magick_RenderingIntent_from_PNG_RenderingIntent
            ((int) mng_info->global_srgb_intent);
      }
  }
#endif


  {
     if (!png_get_gAMA(ping,ping_info,&file_gamma))
       if (mng_info->have_global_gama != MagickFalse)
         png_set_gAMA(ping,ping_info,mng_info->global_gamma);

     if (png_get_gAMA(ping,ping_info,&file_gamma))
       {
         image->gamma=file_gamma;
         if (logging != MagickFalse)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "    Reading PNG gAMA chunk: gamma: %f",file_gamma);
       }
  }

  if (!png_get_valid(ping,ping_info,PNG_INFO_cHRM))
    {
      if (mng_info->have_global_chrm != MagickFalse)
        {
          (void) png_set_cHRM(ping,ping_info,
            mng_info->global_chrm.white_point.x,
            mng_info->global_chrm.white_point.y,
            mng_info->global_chrm.red_primary.x,
            mng_info->global_chrm.red_primary.y,
            mng_info->global_chrm.green_primary.x,
            mng_info->global_chrm.green_primary.y,
            mng_info->global_chrm.blue_primary.x,
            mng_info->global_chrm.blue_primary.y);
        }
    }

  if (png_get_valid(ping,ping_info,PNG_INFO_cHRM))
    {
      (void) png_get_cHRM(ping,ping_info,
        &image->chromaticity.white_point.x,
        &image->chromaticity.white_point.y,
        &image->chromaticity.red_primary.x,
        &image->chromaticity.red_primary.y,
        &image->chromaticity.green_primary.x,
        &image->chromaticity.green_primary.y,
        &image->chromaticity.blue_primary.x,
        &image->chromaticity.blue_primary.y);

       ping_found_cHRM=MagickTrue;
    }

#if defined(PNG_oFFs_SUPPORTED)
  if (png_get_valid(ping,ping_info,PNG_INFO_oFFs))
    {
      image->page.x=(ssize_t) png_get_x_offset_pixels(ping, ping_info);
      image->page.y=(ssize_t) png_get_y_offset_pixels(ping, ping_info);

      if (logging != MagickFalse)
        if (image->page.x || image->page.y)
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Reading PNG oFFs chunk: x: %.20g, y: %.20g.",(double)
              image->page.x,(double) image->page.y);
    }
#endif
#if defined(PNG_pHYs_SUPPORTED)
  if (!png_get_valid(ping,ping_info,PNG_INFO_pHYs))
    {
      if (mng_info->have_global_phys != MagickFalse)
        {
          png_set_pHYs(ping,ping_info,
                       mng_info->global_x_pixels_per_unit,
                       mng_info->global_y_pixels_per_unit,
                       (int) mng_info->global_phys_unit_type);
        }
    }

  x_resolution=0;
  y_resolution=0;
  unit_type=0;
  if (png_get_valid(ping,ping_info,PNG_INFO_pHYs))
    {
      /*
        Set image resolution.
      */
      (void) png_get_pHYs(ping,ping_info,&x_resolution,&y_resolution,
        &unit_type);
      image->resolution.x=(double) x_resolution;
      image->resolution.y=(double) y_resolution;

      if (unit_type == PNG_RESOLUTION_METER)
        {
          image->units=PixelsPerCentimeterResolution;
          image->resolution.x=(double) x_resolution/100.0;
          image->resolution.y=(double) y_resolution/100.0;
        }

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Reading PNG pHYs chunk: xres: %.20g, yres: %.20g, units: %d.",
          (double) x_resolution,(double) y_resolution,unit_type);
    }
#endif

  if (png_get_valid(ping,ping_info,PNG_INFO_PLTE))
    {
      png_colorp
        palette = (png_colorp) NULL;

      (void) png_get_PLTE(ping,ping_info,&palette,&number_colors);

      if ((number_colors == 0) &&
          ((int) ping_color_type == PNG_COLOR_TYPE_PALETTE))
        {
          if (mng_info->global_plte_length)
            {
              png_set_PLTE(ping,ping_info,mng_info->global_plte,
                (int) mng_info->global_plte_length);

              if (!png_get_valid(ping,ping_info,PNG_INFO_tRNS))
              {
                if (mng_info->global_trns_length)
                  {
                    png_warning(ping,
                      "global tRNS has more entries than global PLTE");
                  }
                else
                  {
                     png_set_tRNS(ping,ping_info,mng_info->global_trns,
                       (int) mng_info->global_trns_length,NULL);
                  }
               }
#ifdef PNG_READ_bKGD_SUPPORTED
              if (
                   png_get_valid(ping,ping_info,PNG_INFO_bKGD))
                    {
                      png_color_16
                         background = { 0, 0, 0, 0, 0 };

                      if (png_get_valid(ping, ping_info, PNG_INFO_bKGD))
                        background.index=ping_background->index;

                      background.red=(png_uint_16)
                        mng_info->global_plte[background.index].red;

                      background.green=(png_uint_16)
                        mng_info->global_plte[background.index].green;

                      background.blue=(png_uint_16)
                        mng_info->global_plte[background.index].blue;

                      background.gray=(png_uint_16)
                        mng_info->global_plte[background.index].green;

                      png_set_bKGD(ping,ping_info,&background);
                    }
#endif
                }
              else
                png_error(ping,"No global PLTE in file");
            }
        }

#ifdef PNG_READ_bKGD_SUPPORTED
  if (mng_info->have_global_bkgd != MagickFalse &&
          (!png_get_valid(ping,ping_info,PNG_INFO_bKGD)))
      image->background_color=mng_info->mng_global_bkgd;

  if (png_get_valid(ping,ping_info,PNG_INFO_bKGD))
    {
      unsigned int
        bkgd_scale;

      /* Set image background color.
       * Scale background components to 16-bit, then scale
       * to quantum depth
       */

        bkgd_scale = 1;

        if (ping_file_depth == 1)
           bkgd_scale = 255;

        else if (ping_file_depth == 2)
           bkgd_scale = 85;

        else if (ping_file_depth == 4)
           bkgd_scale = 17;

        if (ping_file_depth <= 8)
           bkgd_scale *= 257;

        ping_background->red *= (png_uint_16) bkgd_scale;
        ping_background->green *= (png_uint_16) bkgd_scale;
        ping_background->blue *= (png_uint_16) bkgd_scale;

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Reading PNG bKGD chunk, raw ping_background=(%d,%d,%d)\n"
              "    bkgd_scale=%d.  ping_background=(%d,%d,%d)",
              ping_background->red,ping_background->green,
              ping_background->blue,bkgd_scale,ping_background->red,
              ping_background->green,ping_background->blue);

        image->background_color.red=
            ScaleShortToQuantum(ping_background->red);

        image->background_color.green=
            ScaleShortToQuantum(ping_background->green);

        image->background_color.blue=
          ScaleShortToQuantum(ping_background->blue);

        image->background_color.alpha=OpaqueAlpha;

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    image->background_color=(%.20g,%.20g,%.20g).",
            (double) image->background_color.red,
            (double) image->background_color.green,
            (double) image->background_color.blue);
    }
#endif /* PNG_READ_bKGD_SUPPORTED */

  if ((png_get_valid(ping,ping_info,PNG_INFO_tRNS)) &&
      (ping_trans_color != (png_color_16p) NULL))
    {
      /*
        Image has a tRNS chunk.
      */
      int
        max_sample;

      size_t
        one = 1;

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Reading PNG tRNS chunk.");

      max_sample = (int) ((one << ping_file_depth) - 1);

      if ((ping_color_type == PNG_COLOR_TYPE_GRAY &&
          (int)ping_trans_color->gray > max_sample) ||
          (ping_color_type == PNG_COLOR_TYPE_RGB &&
          ((int)ping_trans_color->red > max_sample ||
          (int)ping_trans_color->green > max_sample ||
          (int)ping_trans_color->blue > max_sample)))
        {
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Ignoring PNG tRNS chunk with out-of-range sample.");
          png_free_data(ping, ping_info, PNG_FREE_TRNS, 0);
          png_set_invalid(ping,ping_info,PNG_INFO_tRNS);
          image->alpha_trait=UndefinedPixelTrait;
        }
      else
        {
          int
            scale_to_short;

          scale_to_short = 65535L/((1UL << ping_file_depth)-1);

          /* Scale transparent_color to short */
          transparent_color.red= scale_to_short*ping_trans_color->red;
          transparent_color.green= scale_to_short*ping_trans_color->green;
          transparent_color.blue= scale_to_short*ping_trans_color->blue;
          transparent_color.alpha= scale_to_short*ping_trans_color->gray;

          if (ping_color_type == PNG_COLOR_TYPE_GRAY)
            {
              if (logging != MagickFalse)
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    Raw tRNS graylevel = %d, scaled graylevel = %d.",
                  (int) ping_trans_color->gray,(int) transparent_color.alpha);

              }
              transparent_color.red=transparent_color.alpha;
              transparent_color.green=transparent_color.alpha;
              transparent_color.blue=transparent_color.alpha;
            }
        }
    }
#if defined(PNG_READ_sBIT_SUPPORTED)
  if (mng_info->have_global_sbit != MagickFalse)
    {
      if (!png_get_valid(ping,ping_info,PNG_INFO_sBIT))
        png_set_sBIT(ping,ping_info,&mng_info->global_sbit);
    }
#endif
  num_passes=png_set_interlace_handling(ping);

  png_read_update_info(ping,ping_info);

  ping_rowbytes=png_get_rowbytes(ping,ping_info);

  /*
    Initialize image structure.
  */
  mng_info->image_box.left=0;
  mng_info->image_box.right=(ssize_t) ping_width;
  mng_info->image_box.top=0;
  mng_info->image_box.bottom=(ssize_t) ping_height;
  if (mng_info->mng_type == 0)
    {
      mng_info->mng_width=ping_width;
      mng_info->mng_height=ping_height;
      mng_info->frame=mng_info->image_box;
      mng_info->clip=mng_info->image_box;
    }

  else
    {
      image->page.y=mng_info->y_off[mng_info->object_id];
    }

  image->compression=ZipCompression;
  image->columns=ping_width;
  image->rows=ping_height;

  if (((int) ping_color_type == PNG_COLOR_TYPE_GRAY) ||
      ((int) ping_color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
    {
      double
        image_gamma = image->gamma;

      if (logging != MagickFalse)
        (void)LogMagickEvent(CoderEvent,GetMagickModule(),
           "    image->gamma=%g",image_gamma);

      if (image_gamma > 0.75)
        {
          /* Set image->rendering_intent to Undefined,
           * image->colorspace to GRAY, and reset image->chromaticity.
           */
          SetImageColorspace(image,LinearGRAYColorspace,exception);
        }
      else
        {
          RenderingIntent
            save_rendering_intent = image->rendering_intent;
          ChromaticityInfo
            save_chromaticity = image->chromaticity;

          SetImageColorspace(image,GRAYColorspace,exception);
          image->rendering_intent = save_rendering_intent;
          image->chromaticity = save_chromaticity;
        }

      image->gamma = image_gamma;
    }
  else
    {
      double
        image_gamma = image->gamma;

      if (logging != MagickFalse)
        (void)LogMagickEvent(CoderEvent,GetMagickModule(),
           "    image->gamma=%g",image_gamma);

      if (image_gamma > 0.75)
        {
          /* Set image->rendering_intent to Undefined,
           * image->colorspace to GRAY, and reset image->chromaticity.
           */
          image->intensity = Rec709LuminancePixelIntensityMethod;
          SetImageColorspace(image,RGBColorspace,exception);
        }
      else
        {
          RenderingIntent
            save_rendering_intent = image->rendering_intent;
          ChromaticityInfo
            save_chromaticity = image->chromaticity;

          if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
            (void) SetImageColorspace(image,sRGBColorspace,exception);
          image->rendering_intent = save_rendering_intent;
          image->chromaticity = save_chromaticity;
        }

      image->gamma = image_gamma;
    }

  if (logging != MagickFalse)
    (void)LogMagickEvent(CoderEvent,GetMagickModule(),
      "    image->colorspace=%d",(int) image->colorspace);

  if (((int) ping_color_type == PNG_COLOR_TYPE_PALETTE) ||
      ((int) ping_bit_depth < 16 &&
      (int) ping_color_type == PNG_COLOR_TYPE_GRAY))
    {
      size_t
        one;

      image->storage_class=PseudoClass;
      one=1;
      image->colors=one << ping_file_depth;
#if (MAGICKCORE_QUANTUM_DEPTH == 8)
      if (image->colors > 256)
        image->colors=256;
#else
      if (image->colors > 65536L)
        image->colors=65536L;
#endif
      if ((int) ping_color_type == PNG_COLOR_TYPE_PALETTE)
        {
          png_colorp
            palette = (png_colorp) NULL;

          (void) png_get_PLTE(ping,ping_info,&palette,&number_colors);
          image->colors=(size_t) number_colors;

          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Reading PNG PLTE chunk: number_colors: %d.",number_colors);
        }
    }

  if (image->storage_class == PseudoClass)
    {
      /*
        Initialize image colormap.
      */
      if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
        png_error(ping,"Memory allocation failed");

      if ((int) ping_color_type == PNG_COLOR_TYPE_PALETTE)
        {
          png_colorp
            palette = (png_colorp) NULL;

          (void) png_get_PLTE(ping,ping_info,&palette,&number_colors);

          for (i=0; i < (ssize_t) number_colors; i++)
          {
            image->colormap[i].red=ScaleCharToQuantum(palette[i].red);
            image->colormap[i].green=ScaleCharToQuantum(palette[i].green);
            image->colormap[i].blue=ScaleCharToQuantum(palette[i].blue);
          }

          for ( ; i < (ssize_t) image->colors; i++)
          {
            image->colormap[i].red=0;
            image->colormap[i].green=0;
            image->colormap[i].blue=0;
          }
        }
    }

   /* Set some properties for reporting by "identify" */
    {
      char
        msg[MagickPathExtent];

     /* encode ping_width, ping_height, ping_file_depth, ping_color_type,
        ping_interlace_method in value */

     (void) FormatLocaleString(msg,MagickPathExtent,
         "%d, %d",(int) ping_width, (int) ping_height);
     (void) SetImageProperty(image,"png:IHDR.width,height",msg,exception);

     (void) FormatLocaleString(msg,MagickPathExtent,"%d",(int) ping_file_depth);
     (void) SetImageProperty(image,"png:IHDR.bit_depth",msg,exception);

     (void) FormatLocaleString(msg,MagickPathExtent,"%d (%s)",
         (int) ping_color_type,
         Magick_ColorType_from_PNG_ColorType((int)ping_color_type));
     (void) SetImageProperty(image,"png:IHDR.color_type",msg,exception);

     if (ping_interlace_method == 0)
       {
         (void) FormatLocaleString(msg,MagickPathExtent,"%d (Not interlaced)",
            (int) ping_interlace_method);
       }
     else if (ping_interlace_method == 1)
       {
         (void) FormatLocaleString(msg,MagickPathExtent,"%d (Adam7 method)",
            (int) ping_interlace_method);
       }
     else
       {
         (void) FormatLocaleString(msg,MagickPathExtent,"%d (Unknown method)",
            (int) ping_interlace_method);
       }
       (void) SetImageProperty(image,"png:IHDR.interlace_method",msg,exception);

     if (number_colors != 0)
       {
         (void) FormatLocaleString(msg,MagickPathExtent,"%d",
            (int) number_colors);
         (void) SetImageProperty(image,"png:PLTE.number_colors",msg,exception);
       }
   }
#if defined(PNG_tIME_SUPPORTED)
   read_tIME_chunk(image,ping,ping_info,exception);
#endif
#if defined(PNG_READ_eXIf_SUPPORTED)
  read_eXIf_chunk(image,ping,ping_info,exception);
#endif


  /*
    Read image scanlines.
  */
  if (image->delay != 0)
    mng_info->scenes_found++;

  if ((mng_info->mng_type == 0 && (image->ping != MagickFalse)) || (
      (image_info->number_scenes != 0) && (mng_info->scenes_found > (ssize_t)
      (image_info->scene+image_info->number_scenes))))
    {
      /* This happens later in non-ping decodes */
      if (png_get_valid(ping,ping_info,PNG_INFO_tRNS))
        image->storage_class=DirectClass;
      image->alpha_trait=
        (((int) ping_color_type == PNG_COLOR_TYPE_RGB_ALPHA) ||
         ((int) ping_color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ||
         (png_get_valid(ping,ping_info,PNG_INFO_tRNS))) ?
        BlendPixelTrait : UndefinedPixelTrait;

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Skipping PNG image data for scene %.20g",(double)
          mng_info->scenes_found-1);
      png_destroy_read_struct(&ping,&ping_info,&end_info);

#ifdef IMPNG_SETJMP_NOT_THREAD_SAFE
      UnlockSemaphoreInfo(ping_semaphore);
#endif

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  exit ReadOnePNGImage().");

      return(image);
    }

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    Reading PNG IDAT chunk(s)");

  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status != MagickFalse)
    status=ResetImagePixels(image,exception);
  if (status == MagickFalse)
    {
      png_destroy_read_struct(&ping,&ping_info,&end_info);
#ifdef IMPNG_SETJMP_NOT_THREAD_SAFE
      UnlockSemaphoreInfo(ping_semaphore);
#endif
      return(DestroyImageList(image));
    }

  if (num_passes > 1)
    pixel_info=AcquireVirtualMemory(image->rows,ping_rowbytes*
      sizeof(*ping_pixels));
  else
    pixel_info=AcquireVirtualMemory(ping_rowbytes,sizeof(*ping_pixels));

  if (pixel_info == (MemoryInfo *) NULL)
    png_error(ping,"Memory allocation failed");
  ping_pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    Converting PNG pixels to pixel packets");
  /*
    Convert PNG pixels to pixel packets.
  */
  quantum_info=AcquireQuantumInfo(image_info,image);

  if (quantum_info == (QuantumInfo *) NULL)
     png_error(ping,"Failed to allocate quantum_info");

  (void) SetQuantumEndian(image,quantum_info,MSBEndian);

  {

   MagickBooleanType
     found_transparent_pixel;

  found_transparent_pixel=MagickFalse;

  if ((image->storage_class == DirectClass) ||
      (image_info->stream != (StreamHandler) NULL))
    {
      for (pass=0; pass < num_passes; pass++)
      {
        /*
          Convert image to DirectClass pixel packets.
        */
        image->alpha_trait=
            (((int) ping_color_type == PNG_COLOR_TYPE_RGB_ALPHA) ||
            ((int) ping_color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ||
            (png_get_valid(ping,ping_info,PNG_INFO_tRNS))) ?
            BlendPixelTrait : UndefinedPixelTrait;

        for (y=0; y < (ssize_t) image->rows; y++)
        {
          if (num_passes > 1)
            row_offset=ping_rowbytes*(size_t) y;

          else
            row_offset=0;

          png_read_row(ping,ping_pixels+row_offset,NULL);

          if (pass < num_passes-1)
            continue;

          q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

          if (q == (Quantum *) NULL)
            break;

          if ((int) ping_color_type == PNG_COLOR_TYPE_GRAY)
            (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
              GrayQuantum,ping_pixels+row_offset,exception);

          else if ((int) ping_color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
              GrayAlphaQuantum,ping_pixels+row_offset,exception);

          else if ((int) ping_color_type == PNG_COLOR_TYPE_RGB_ALPHA)
            (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
              RGBAQuantum,ping_pixels+row_offset,exception);

          else if ((int) ping_color_type == PNG_COLOR_TYPE_PALETTE)
            (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
              IndexQuantum,ping_pixels+row_offset,exception);

          else /* ping_color_type == PNG_COLOR_TYPE_RGB */
            (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
              RGBQuantum,ping_pixels+row_offset,exception);

          if (found_transparent_pixel == MagickFalse)
            {
              /* Is there a transparent pixel in the row? */
              if (y== 0 && logging != MagickFalse)
                 (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "    Looking for cheap transparent pixel");

              for (x=(ssize_t) image->columns-1; x >= 0; x--)
              {
                if ((ping_color_type == PNG_COLOR_TYPE_RGBA ||
                    ping_color_type == PNG_COLOR_TYPE_GRAY_ALPHA) &&
                   (GetPixelAlpha(image,q) != OpaqueAlpha))
                  {
                    if (logging != MagickFalse)
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "    ...got one.");

                    found_transparent_pixel = MagickTrue;
                    break;
                  }
                if ((ping_color_type == PNG_COLOR_TYPE_RGB ||
                    ping_color_type == PNG_COLOR_TYPE_GRAY) &&
                    (ScaleQuantumToShort(GetPixelRed(image,q)) ==
                    transparent_color.red &&
                    ScaleQuantumToShort(GetPixelGreen(image,q)) ==
                    transparent_color.green &&
                    ScaleQuantumToShort(GetPixelBlue(image,q)) ==
                    transparent_color.blue))
                  {
                    if (logging != MagickFalse)
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "    ...got one.");
                    found_transparent_pixel = MagickTrue;
                    break;
                  }
                q+=(ptrdiff_t) GetPixelChannels(image);
              }
            }

          if (num_passes == 1)
            {
              status=SetImageProgress(image,LoadImageTag,
                  (MagickOffsetType) y, image->rows);

              if (status == MagickFalse)
                break;
            }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
        }
        if (y < (long) image->rows)
          break;

        if (num_passes != 1)
          {
            status=SetImageProgress(image,LoadImageTag,pass,(MagickSizeType)
              num_passes);
            if (status == MagickFalse)
              break;
          }
      }
    }

  else /* image->storage_class != DirectClass */

    for (pass=0; pass < num_passes; pass++)
    {
      Quantum
        *r;

      /*
        Convert grayscale image to PseudoClass pixel packets.
      */
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Converting grayscale pixels to pixel packets");

      image->alpha_trait=ping_color_type == PNG_COLOR_TYPE_GRAY_ALPHA ?
        BlendPixelTrait : UndefinedPixelTrait;

      quantum_scanline=(Quantum *) AcquireQuantumMemory(image->columns,
        (image->alpha_trait  == BlendPixelTrait?  2 : 1)*
        sizeof(*quantum_scanline));

      if (quantum_scanline == (Quantum *) NULL)
        png_error(ping,"Memory allocation failed");

      for (y=0; y < (ssize_t) image->rows; y++)
      {
        Quantum
           alpha;

        if (num_passes > 1)
          row_offset=ping_rowbytes*(size_t) y;

        else
          row_offset=0;

        png_read_row(ping,ping_pixels+row_offset,NULL);

        if (pass < num_passes-1)
          continue;

        q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

        if (q == (Quantum *) NULL)
          break;

        p=ping_pixels+row_offset;
        r=quantum_scanline;

        switch (ping_bit_depth)
        {
          case 8:
          {

            if (ping_color_type == 4)
              for (x=(ssize_t) image->columns-1; x >= 0; x--)
              {
                *r++=*p++;

                alpha=ScaleCharToQuantum((unsigned char)*p++);

                SetPixelAlpha(image,alpha,q);

                if (alpha != OpaqueAlpha)
                  found_transparent_pixel = MagickTrue;

                q+=(ptrdiff_t) GetPixelChannels(image);
              }

            else
              for (x=(ssize_t) image->columns-1; x >= 0; x--)
                *r++=*p++;

            break;
          }

          case 16:
          {
            for (x=(ssize_t) image->columns-1; x >= 0; x--)
            {
#if (MAGICKCORE_QUANTUM_DEPTH >= 16)
              unsigned long
                quantum;

              if (image->colors > 256)
                quantum=(((unsigned int) *p++) << 8);

              else
                quantum=0;

              quantum|=(*p++);
              *r=ScaleShortToQuantum((unsigned short) quantum);
              r++;

              if (ping_color_type == 4)
                {
                  if (image->colors > 256)
                    quantum=(((unsigned int) *p++) << 8);
                  else
                    quantum=0;

                  quantum|=(*p++);

                  alpha=ScaleShortToQuantum((unsigned short) quantum);
                  SetPixelAlpha(image,alpha,q);

                  if (alpha != OpaqueAlpha)
                    found_transparent_pixel = MagickTrue;

                  q+=(ptrdiff_t) GetPixelChannels(image);
                }

#else /* MAGICKCORE_QUANTUM_DEPTH == 8 */
              *r++=(*p++);
              p++; /* strip low byte */

              if (ping_color_type == 4)
                {
                  SetPixelAlpha(image,*p++,q);

                  if (GetPixelAlpha(image,q) != OpaqueAlpha)
                    found_transparent_pixel = MagickTrue;

                  p++;
                  q+=(ptrdiff_t) GetPixelChannels(image);
                }
#endif
            }

            break;
          }

          default:
            break;
        }

        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;

        /*
          Transfer image scanline.
        */
        r=quantum_scanline;

        q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          ssize_t index=ConstrainColormapIndex(image,(ssize_t) *r,exception);
          SetPixelRed(image,ClampToQuantum(image->colormap[index].red),q);
          SetPixelGreen(image,ClampToQuantum(image->colormap[index].green),q);
          SetPixelBlue(image,ClampToQuantum(image->colormap[index].blue),q);
          SetPixelIndex(image,(Quantum) index,q);
          r++;
          q+=(ptrdiff_t) GetPixelChannels(image);
        }

        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;

        if (num_passes == 1)
          {
            status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
              image->rows);

            if (status == MagickFalse)
              break;
          }
      }
      quantum_scanline=(Quantum *) RelinquishMagickMemory(quantum_scanline);
      if (y < (long) image->rows)
        break;
      if (num_passes != 1)
        {
          status=SetImageProgress(image,LoadImageTag,pass,(MagickSizeType)
            num_passes);

          if (status == MagickFalse)
            break;
        }
    }

    image->alpha_trait=found_transparent_pixel ? BlendPixelTrait :
      UndefinedPixelTrait;

    if (logging != MagickFalse)
      {
        if (found_transparent_pixel != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Found transparent pixel");
        else
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    No transparent pixel was found");

            ping_color_type&=0x03;
          }
      }
  }
  quantum_info=DestroyQuantumInfo(quantum_info);

  png_read_end(ping,end_info);

  if (logging != MagickFalse)
  {
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
       "  image->storage_class=%d\n",(int) image->storage_class);
  }

  if (image_info->number_scenes != 0 && mng_info->scenes_found-1 <
      (ssize_t) image_info->scene && image->delay != 0)
    {
      png_destroy_read_struct(&ping,&ping_info,&end_info);
      pixel_info=RelinquishVirtualMemory(pixel_info);
      if (AcquireImageColormap(image,2,exception) == MagickFalse)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      (void) SetImageBackgroundColor(image,exception);
#ifdef IMPNG_SETJMP_NOT_THREAD_SAFE
      UnlockSemaphoreInfo(ping_semaphore);
#endif
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  exit ReadOnePNGImage() early.");
      return(image);
    }

  if (png_get_valid(ping,ping_info,PNG_INFO_tRNS))
    {
      ClassType
        storage_class;

      /*
        Image has a transparent background.
      */
      storage_class=image->storage_class;
      image->alpha_trait=BlendPixelTrait;

/* Balfour fix from imagemagick discourse server, 5 Feb 2010 */

      if (storage_class == PseudoClass)
        {
          if ((int) ping_color_type == PNG_COLOR_TYPE_PALETTE)
            {
              for (x=0; x < ping_num_trans; x++)
              {
                 image->colormap[x].alpha_trait=BlendPixelTrait;
                 image->colormap[x].alpha=OpaqueAlpha;
                 if (ping_trans_alpha != (png_bytep) NULL)
                   image->colormap[x].alpha=ScaleCharToQuantum(
                     (unsigned char) ping_trans_alpha[x]);
              }
            }

          else if (ping_color_type == PNG_COLOR_TYPE_GRAY)
            {
              for (x=0; x < (int) image->colors; x++)
              {
                 if (ScaleQuantumToShort((Quantum) image->colormap[x].red) ==
                     transparent_color.alpha)
                 {
                    image->colormap[x].alpha_trait=BlendPixelTrait;
                    image->colormap[x].alpha = (Quantum) TransparentAlpha;
                 }
              }
            }
          (void) SyncImage(image,exception);
        }

#if 1 /* Should have already been done above, but glennrp problem P10
       * needs this.
       */
      else
        {
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            image->storage_class=storage_class;
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

            if (q == (Quantum *) NULL)
              break;


            /* Caution: on a Q8 build, this does not distinguish between
             * 16-bit colors that differ only in the low byte
             */
            for (x=(ssize_t) image->columns-1; x >= 0; x--)
            {
              if (ScaleQuantumToShort(GetPixelRed(image,q)) ==
                  transparent_color.red &&
                  ScaleQuantumToShort(GetPixelGreen(image,q)) ==
                  transparent_color.green &&
                  ScaleQuantumToShort(GetPixelBlue(image,q)) ==
                  transparent_color.blue)
                {
                  SetPixelAlpha(image,TransparentAlpha,q);
                }
              else
                {
                  SetPixelAlpha(image,OpaqueAlpha,q);
                }

              q+=(ptrdiff_t) GetPixelChannels(image);
            }

            if (SyncAuthenticPixels(image,exception) == MagickFalse)
               break;
          }
        }
#endif
    }

  for (j = 0; j < 2; j++)
  {
    if (j == 0)
      status=png_get_text(ping,ping_info,&text,&num_text) != 0 ?
        MagickTrue : MagickFalse;
    else
      status=png_get_text(ping,end_info,&text,&num_text) != 0 ?
        MagickTrue : MagickFalse;

    if (status != MagickFalse)
      for (i=0; i < (ssize_t) num_text; i++)
      {
        /* Check for a profile */

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Reading PNG text chunk");

        if (strlen(text[i].key) > 16 &&
            memcmp(text[i].key, "Raw profile type ",17) == 0)
          {
            const char
              *value;

            value=GetImageOption(image_info,"profile:skip");

            if (IsOptionMember(text[i].key+17,value) == MagickFalse)
            {
               (void) Magick_png_read_raw_profile(ping,image,image_info,text,
                  (int) i,exception);
               num_raw_profiles++;
               if (logging != MagickFalse)
                 (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "    Read raw profile %s",text[i].key+17);
            }
            else
            {
               if (logging != MagickFalse)
                 (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "    Skipping raw profile %s",text[i].key+17);
            }
          }

        else
          {
            char
              *value;

            length=text[i].text_length;
            value=(char *) AcquireQuantumMemory(length+MagickPathExtent,
              sizeof(*value));
            if (value == (char *) NULL)
              {
                png_error(ping,"Memory allocation failed");
                break;
              }
            *value='\0';
            (void) ConcatenateMagickString(value,text[i].text,length+2);

            /* Don't save "density" or "units" property if we have a pHYs
             * chunk
             */
            if (!png_get_valid(ping,ping_info,PNG_INFO_pHYs) ||
                (LocaleCompare(text[i].key,"density") != 0 &&
                LocaleCompare(text[i].key,"units") != 0))
              {
                char
                  key[MagickPathExtent];

                (void) FormatLocaleString(key,MagickPathExtent,"%s",
                  text[i].key);
                if ((LocaleCompare(key,"version") == 0) ||
                    (LocaleCompare(key,"width") == 0))
                  (void) FormatLocaleString(key,MagickPathExtent,"png:%s",
                    text[i].key);
                (void) SetImageProperty(image,key,value,exception);
              }

            if (logging != MagickFalse)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "      length: %lu\n"
                "      Keyword: %s",
                (unsigned long) length,
                text[i].key);
            }

            value=DestroyString(value);
          }
      }
    num_text_total += num_text;
  }

  /* Set image->alpha_trait to MagickTrue if the input colortype supports
  * alpha or if a valid tRNS chunk is present, no matter whether there
  * is actual transparency present.
  */
  image->alpha_trait=(((int) ping_color_type == PNG_COLOR_TYPE_RGB_ALPHA) ||
      ((int) ping_color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ||
      (png_get_valid(ping,ping_info,PNG_INFO_tRNS))) ?
      BlendPixelTrait : UndefinedPixelTrait;
  if (((image->alpha_trait & BlendPixelTrait) != 0) ||
      ((image->alpha_trait & UpdatePixelTrait) != 0))
    {
      if (ping_color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        image->type=GrayscaleAlphaType;
      else if (ping_color_type == PNG_COLOR_TYPE_PALETTE)
        image->type=PaletteAlphaType;
      else
        image->type=TrueColorAlphaType;
    }
  else
    {
      if (ping_color_type == PNG_COLOR_TYPE_GRAY)
        image->type=GrayscaleType;
      else if (ping_color_type == PNG_COLOR_TYPE_PALETTE)
        image->type=PaletteType;
      else
        image->type=TrueColorType;
    }

   /* Set more properties for identify to retrieve */
   {
     char
       msg[MagickPathExtent];

     if (num_text_total != 0)
       {
         /* libpng doesn't tell us whether they were tEXt, zTXt, or iTXt */
         (void) FormatLocaleString(msg,MagickPathExtent,
            "%d tEXt/zTXt/iTXt chunks were found", num_text_total);
         (void) SetImageProperty(image,"png:text",msg,exception);
       }

     if (num_raw_profiles != 0)
       {
         (void) FormatLocaleString(msg,MagickPathExtent,
            "%d were found", num_raw_profiles);
         (void) SetImageProperty(image,"png:text-encoded profiles",msg,
                exception);
       }

     /* cHRM chunk: */
     if (ping_found_cHRM != MagickFalse)
       {
         (void) FormatLocaleString(msg,MagickPathExtent,"%s",
            "chunk was found (see Chromaticity, above)");
         (void) SetImageProperty(image,"png:cHRM",msg,exception);
       }

     /* bKGD chunk: */
     if (png_get_valid(ping,ping_info,PNG_INFO_bKGD))
       {
         (void) FormatLocaleString(msg,MagickPathExtent,"%s",
            "chunk was found (see Background color, above)");
         (void) SetImageProperty(image,"png:bKGD",msg,exception);
       }

     (void) FormatLocaleString(msg,MagickPathExtent,"%s",
        "chunk was found");

#if defined(PNG_iCCP_SUPPORTED)
     /* iCCP chunk: */
     if (ping_found_iCCP != MagickFalse)
        (void) SetImageProperty(image,"png:iCCP",msg,exception);
#endif

     if (png_get_valid(ping,ping_info,PNG_INFO_tRNS))
        (void) SetImageProperty(image,"png:tRNS",msg,exception);

#if defined(PNG_sRGB_SUPPORTED)
     /* sRGB chunk: */
     if (ping_found_sRGB != MagickFalse)
       {
         (void) FormatLocaleString(msg,MagickPathExtent,
            "intent=%d (%s)",
            (int) intent,
            Magick_RenderingIntentString_from_PNG_RenderingIntent(intent));
         (void) SetImageProperty(image,"png:sRGB",msg,exception);
       }
#endif

     /* gAMA chunk: */
     if (ping_found_gAMA != MagickFalse)
       {
         (void) FormatLocaleString(msg,MagickPathExtent,
            "gamma=%.8g (See Gamma, above)",
            file_gamma);
         (void) SetImageProperty(image,"png:gAMA",msg,exception);
       }

#if defined(PNG_pHYs_SUPPORTED)
     /* pHYs chunk: */
     if (png_get_valid(ping,ping_info,PNG_INFO_pHYs))
       {
         (void) FormatLocaleString(msg,MagickPathExtent,
            "x_res=%.10g, y_res=%.10g, units=%d",
            (double) x_resolution,(double) y_resolution, unit_type);
         (void) SetImageProperty(image,"png:pHYs",msg,exception);
       }
#endif

#if defined(PNG_oFFs_SUPPORTED)
     /* oFFs chunk: */
     if (png_get_valid(ping,ping_info,PNG_INFO_oFFs))
       {
         (void) FormatLocaleString(msg,MagickPathExtent,
            "x_off=%.20g, y_off=%.20g",
            (double) image->page.x,(double) image->page.y);
         (void) SetImageProperty(image,"png:oFFs",msg,exception);
       }
#endif

#if defined(PNG_tIME_SUPPORTED)
     read_tIME_chunk(image,ping,end_info,exception);
#endif
#if defined(PNG_READ_eXIf_SUPPORTED)
    read_eXIf_chunk(image,ping,end_info,exception);
#endif

     /* caNv chunk: */
     if ((image->page.width != 0 && image->page.width != image->columns) ||
         (image->page.height != 0 && image->page.height != image->rows) ||
         (image->page.x != 0 || image->page.y != 0))
       {
         (void) FormatLocaleString(msg,MagickPathExtent,
            "width=%.20g, height=%.20g, x_offset=%.20g, y_offset=%.20g",
            (double) image->page.width,(double) image->page.height,
            (double) image->page.x,(double) image->page.y);
         (void) SetImageProperty(image,"png:caNv",msg,exception);
       }
   }

  /*
    Relinquish resources.
  */
  png_destroy_read_struct(&ping,&ping_info,&end_info);

  pixel_info=RelinquishVirtualMemory(pixel_info);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  exit ReadOnePNGImage()");

#ifdef IMPNG_SETJMP_NOT_THREAD_SAFE
  UnlockSemaphoreInfo(ping_semaphore);
#endif

  /* }  for navigation to beginning of SETJMP-protected block, revert to
   *    Throwing an Exception when an error occurs.
   */
  image->depth=(size_t) ping_file_depth;
  return(image);

/* end of reading one PNG image */
}

static Image *ReadPNGImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    logging = MagickFalse,
    status;

  MngReadInfo
    *mng_info;

  char
    magic_number[MagickPathExtent];

  ssize_t
    count;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image=AcquireImage(image_info,exception);
  if (image->debug != MagickFalse)
    logging=LogMagickEvent(CoderEvent,GetMagickModule(),"Enter ReadPNGImage()");
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);

  if (status == MagickFalse)
    return(DestroyImageList(image));

  /*
    Verify PNG signature.
  */
  count=ReadBlob(image,8,(unsigned char *) magic_number);

  if ((count < 8) || (memcmp(magic_number,"\211PNG\r\n\032\n",8) != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  /*
     Verify that file size large enough to contain a PNG datastream.
  */
  if (GetBlobSize(image) < 61)
    ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");

  mng_info=(MngReadInfo *) AcquireMagickMemory(sizeof(*mng_info));
  if (mng_info == (MngReadInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  (void) memset(mng_info,0,sizeof(*mng_info));
  mng_info->image=image;

  image=ReadOnePNGImage(mng_info,image_info,exception);
  mng_info=MngReadInfoFreeStruct(mng_info);

  if (image == (Image *) NULL)
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "exit ReadPNGImage() with error");

      return((Image *) NULL);
    }

  (void) CloseBlob(image);

  if ((image->columns == 0) || (image->rows == 0))
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "exit ReadPNGImage() with error.");

      ThrowReaderException(CorruptImageError,"CorruptImage");
    }

  if ((IssRGBColorspace(image->colorspace) != MagickFalse) &&
      (image->gamma > .75) &&
           !(image->chromaticity.red_primary.x>0.6399 &&
           image->chromaticity.red_primary.x<0.6401 &&
           image->chromaticity.red_primary.y>0.3299 &&
           image->chromaticity.red_primary.y<0.3301 &&
           image->chromaticity.green_primary.x>0.2999 &&
           image->chromaticity.green_primary.x<0.3001 &&
           image->chromaticity.green_primary.y>0.5999 &&
           image->chromaticity.green_primary.y<0.6001 &&
           image->chromaticity.blue_primary.x>0.1499 &&
           image->chromaticity.blue_primary.x<0.1501 &&
           image->chromaticity.blue_primary.y>0.0599 &&
           image->chromaticity.blue_primary.y<0.0601 &&
           image->chromaticity.white_point.x>0.3126 &&
           image->chromaticity.white_point.x<0.3128 &&
           image->chromaticity.white_point.y>0.3289 &&
           image->chromaticity.white_point.y<0.3291))
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "SetImageColorspace to RGBColorspace");
      SetImageColorspace(image,RGBColorspace,exception);
    }

  if (logging != MagickFalse)
    {
       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "  page.w: %.20g, page.h: %.20g,page.x: %.20g, page.y: %.20g.",
               (double) image->page.width,(double) image->page.height,
               (double) image->page.x,(double) image->page.y);
       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "  image->colorspace: %d", (int) image->colorspace);
    }

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit ReadPNGImage()");

  return(image);
}



#if defined(JNG_SUPPORTED)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d O n e J N G I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadOneJNGImage() reads a JPEG Network Graphics (JNG) image file
%  (minus the 8-byte signature)  and returns it.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  JNG support written by Glenn Randers-Pehrson, glennrp@image...
%
%  The format of the ReadOneJNGImage method is:
%
%      Image *ReadOneJNGImage(MngReadInfo *mng_info,const ImageInfo *image_info,
%         ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o mng_info: Specifies a pointer to a MngReadInfo structure.
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static void
DestroyJNG(unsigned char *chunk,Image **color_image,
  ImageInfo **color_image_info,Image **alpha_image,
  ImageInfo **alpha_image_info)
{
  (void) RelinquishMagickMemory(chunk);
  if (color_image_info && *color_image_info)
  {
    DestroyImageInfo(*color_image_info);
    *color_image_info = (ImageInfo *)NULL;
  }
  if (alpha_image_info && *alpha_image_info)
  {
    DestroyImageInfo(*alpha_image_info);
    *alpha_image_info = (ImageInfo *)NULL;
  }
  if (color_image && *color_image)
  {
    DestroyImageList(*color_image);
    *color_image = (Image *)NULL;
  }
  if (alpha_image && *alpha_image)
  {
    DestroyImageList(*alpha_image);
    *alpha_image = (Image *)NULL;
  }
}
static Image *ReadOneJNGImage(MngReadInfo *mng_info,
  const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *alpha_image,
    *color_image,
    *image,
    *jng_image;

  ImageInfo
    *alpha_image_info,
    *color_image_info;

  MagickBooleanType
    logging = MagickFalse;

  ssize_t
    y;

  MagickBooleanType
    status;

  png_uint_32
    jng_height,
    jng_width;

  png_byte
    jng_color_type,
    jng_image_sample_depth,
    jng_image_compression_method,
    jng_image_interlace_method,
    jng_alpha_sample_depth,
    jng_alpha_compression_method,
    jng_alpha_filter_method,
    jng_alpha_interlace_method;

  const Quantum
    *s;

  ssize_t
    i,
    x;

  Quantum
    *q;

  unsigned char
    *p;

  unsigned int
    read_JSEP,
    reading_idat;

  size_t
    length;

  jng_alpha_compression_method=0;
  jng_alpha_sample_depth=8;
  jng_color_type=0;
  jng_height=0;
  jng_width=0;
  alpha_image=(Image *) NULL;
  color_image=(Image *) NULL;
  alpha_image_info=(ImageInfo *) NULL;
  color_image_info=(ImageInfo *) NULL;

  if (IsEventLogging() != MagickFalse)
    logging=LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Enter ReadOneJNGImage()");

  image=mng_info->image;

  if (GetAuthenticPixelQueue(image) != (Quantum *) NULL)
    {
      /*
        Allocate next image structure.
      */
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "  AcquireNextImage()");

      AcquireNextImage(image_info,image,exception);

      if (GetNextImageInList(image) == (Image *) NULL)
        return(DestroyImageList(image));

      image=SyncNextImageInList(image);
    }
  mng_info->image=image;

  /*
    Signature bytes have already been read.
  */

  read_JSEP=MagickFalse;
  reading_idat=MagickFalse;
  for (;;)
  {
    char
      type[MagickPathExtent];

    unsigned char
      *chunk;

    unsigned int
      count;

    /*
      Read a new JNG chunk.
    */
    status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
      2*GetBlobSize(image));

    if (status == MagickFalse)
      break;

    type[0]='\0';
    (void) ConcatenateMagickString(type,"errr",MagickPathExtent);
    length=(size_t) ReadBlobMSBLong(image);
    count=(unsigned int) ReadBlob(image,4,(unsigned char *) type);

    if (logging != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Reading JNG chunk type %c%c%c%c, length: %.20g",
        type[0],type[1],type[2],type[3],(double) length);

    if (length > PNG_UINT_31_MAX || count == 0)
      {
        DestroyJNG(NULL,&color_image,&color_image_info,
          &alpha_image,&alpha_image_info);
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }
    if (length > GetBlobSize(image))
      {
        DestroyJNG(NULL,&color_image,&color_image_info,
          &alpha_image,&alpha_image_info);
        ThrowReaderException(CorruptImageError,
          "InsufficientImageDataInFile");
      }

    p=NULL;
    chunk=(unsigned char *) NULL;

    if (length != 0)
      {
        chunk=(unsigned char *) AcquireQuantumMemory(length,sizeof(*chunk));

        if (chunk == (unsigned char *) NULL)
          {
            DestroyJNG(NULL,&color_image,&color_image_info,
              &alpha_image,&alpha_image_info);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }

        for (i=0; i < (ssize_t) length; i++)
        {
          int
            c;

          c=ReadBlobByte(image);
          if (c == EOF)
            break;
          chunk[i]=(unsigned char) c;
        }
        for ( ; i < (ssize_t) length; i++)
          chunk[i]='\0';

        p=chunk;
      }

    (void) ReadBlobMSBLong(image);  /* read crc word */

    if (memcmp(type,mng_JHDR,4) == 0)
      {
        if (length == 16)
          {
            jng_width=(png_uint_32)mng_get_long(p);
            jng_height=(png_uint_32)mng_get_long(&p[4]);
            if ((jng_width == 0) || (jng_height == 0))
              {
                DestroyJNG(chunk,&color_image,&color_image_info,
                  &alpha_image,&alpha_image_info);
                ThrowReaderException(CorruptImageError,
                  "NegativeOrZeroImageSize");
              }
            jng_color_type=p[8];
            jng_image_sample_depth=p[9];
            jng_image_compression_method=p[10];
            jng_image_interlace_method=p[11];

            image->interlace=jng_image_interlace_method != 0 ? PNGInterlace :
              NoInterlace;

            jng_alpha_sample_depth=p[12];
            jng_alpha_compression_method=p[13];
            jng_alpha_filter_method=p[14];
            jng_alpha_interlace_method=p[15];

            if (logging != MagickFalse)
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    jng_width:      %16lu,    jng_height:     %16lu\n"
                  "    jng_color_type: %16d,     jng_image_sample_depth: %3d\n"
                  "    jng_image_compression_method:%3d",
                  (unsigned long) jng_width, (unsigned long) jng_height,
                  jng_color_type, jng_image_sample_depth,
                  jng_image_compression_method);

                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    jng_image_interlace_method:  %3d"
                  "    jng_alpha_sample_depth:      %3d",
                  jng_image_interlace_method,
                  jng_alpha_sample_depth);

                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    jng_alpha_compression_method:%3d\n"
                  "    jng_alpha_filter_method:     %3d\n"
                  "    jng_alpha_interlace_method:  %3d",
                  jng_alpha_compression_method,
                  jng_alpha_filter_method,
                  jng_alpha_interlace_method);
              }
          }

        chunk=(unsigned char *) RelinquishMagickMemory(chunk);

        if ((jng_width > 65535) || (jng_height > 65535) ||
            (MagickSizeType) jng_width > GetMagickResourceLimit(WidthResource) ||
            (MagickSizeType) jng_height > GetMagickResourceLimit(HeightResource))
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "    JNG width or height too large: (%lu x %lu)",
                (long) jng_width, (long) jng_height);
            DestroyJNG(chunk,&color_image,&color_image_info,
              &alpha_image,&alpha_image_info);
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
          }

        continue;
      }


    if ((reading_idat == MagickFalse) && (read_JSEP == MagickFalse) &&
        ((memcmp(type,mng_JDAT,4) == 0) || (memcmp(type,mng_JdAA,4) == 0) ||
         (memcmp(type,mng_IDAT,4) == 0) || (memcmp(type,mng_JDAA,4) == 0)))
      {
        /*
           o create color_image
           o open color_blob, attached to color_image
           o if (color type has alpha)
               open alpha_blob, attached to alpha_image
        */

        color_image_info=(ImageInfo *)AcquireMagickMemory(sizeof(ImageInfo));

        if (color_image_info == (ImageInfo *) NULL)
        {
          DestroyJNG(chunk,&color_image,&color_image_info,
              &alpha_image,&alpha_image_info);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }

        GetImageInfo(color_image_info);
        color_image=AcquireImage(color_image_info,exception);

        if (color_image == (Image *) NULL)
        {
          DestroyJNG(chunk,&color_image,&color_image_info,
              &alpha_image,&alpha_image_info);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Creating color_blob.");

        (void) AcquireUniqueFilename(color_image->filename);
        status=OpenBlob(color_image_info,color_image,WriteBinaryBlobMode,
          exception);

        if (status == MagickFalse)
          {
            DestroyJNG(chunk,&color_image,&color_image_info,
              &alpha_image,&alpha_image_info);
            return(DestroyImageList(image));
          }

        if ((image_info->ping == MagickFalse) && (jng_color_type >= 12))
          {
            if ((jng_alpha_compression_method != 0) &&
                (jng_alpha_compression_method != 8))
              {
                DestroyJNG(chunk,&color_image,&color_image_info,&alpha_image,
                  &alpha_image_info);
                ThrowReaderException(CorruptImageError,"ImproperImageHeader");
              }
            alpha_image_info=(ImageInfo *)
              AcquireMagickMemory(sizeof(ImageInfo));

            if (alpha_image_info == (ImageInfo *) NULL)
              {
                DestroyJNG(chunk,&color_image,&color_image_info,
                  &alpha_image,&alpha_image_info);
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }

            GetImageInfo(alpha_image_info);
            alpha_image=AcquireImage(alpha_image_info,exception);

            if (alpha_image == (Image *) NULL)
              {
                DestroyJNG(chunk,&color_image,&color_image_info,
                  &alpha_image,&alpha_image_info);
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }

            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    Creating alpha_blob.");

            (void) AcquireUniqueFilename(alpha_image->filename);
            status=OpenBlob(alpha_image_info,alpha_image,WriteBinaryBlobMode,
              exception);

            if (status == MagickFalse)
              {
                DestroyJNG(chunk,&color_image,&color_image_info,
                  &alpha_image,&alpha_image_info);
                return(DestroyImageList(image));
              }

            if (jng_alpha_compression_method == 0)
              {
                unsigned char
                  data[18];

                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "    Writing IHDR chunk to alpha_blob.");

                (void) WriteBlob(alpha_image,8,(const unsigned char *)
                  "\211PNG\r\n\032\n");

                (void) WriteBlobMSBULong(alpha_image,13L);
                PNGType(data,mng_IHDR);
                LogPNGChunk(logging,mng_IHDR,13L);
                PNGLong(data+4,jng_width);
                PNGLong(data+8,jng_height);
                data[12]=jng_alpha_sample_depth;
                data[13]=0; /* color_type gray */
                data[14]=0; /* compression method 0 */
                data[15]=0; /* filter_method 0 */
                data[16]=0; /* interlace_method 0 */
                (void) WriteBlob(alpha_image,17,data);
                (void) WriteBlobMSBULong(alpha_image,crc32(0,data,17));
              }
          }
        reading_idat=MagickTrue;
      }

    if (memcmp(type,mng_JDAT,4) == 0)
      {
        /* Copy chunk to color_image->blob */

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Copying JDAT chunk data to color_blob.");

        if ((length != 0) && (color_image != (Image *) NULL))
          (void) WriteBlob(color_image,length,chunk);
        chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_IDAT,4) == 0)
      {
        png_byte
           data[5];

        /* Copy IDAT header and chunk data to alpha_image->blob */

        if (alpha_image != NULL && image_info->ping == MagickFalse)
          {
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    Copying IDAT chunk data to alpha_blob.");

            (void) WriteBlobMSBULong(alpha_image,(size_t) length);
            PNGType(data,mng_IDAT);
            LogPNGChunk(logging,mng_IDAT,length);
            (void) WriteBlob(alpha_image,4,data);
            (void) WriteBlob(alpha_image,length,chunk);
            (void) WriteBlobMSBULong(alpha_image,
              crc32(crc32(0,data,4),chunk,(uInt) length));
          }

        chunk=(unsigned char *) RelinquishMagickMemory(chunk);

        continue;
      }

    if ((memcmp(type,mng_JDAA,4) == 0) || (memcmp(type,mng_JdAA,4) == 0))
      {
        /* Copy chunk data to alpha_image->blob */

        if ((alpha_image != NULL) && (image_info->ping == MagickFalse) &&
            (length != 0))
          {
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    Copying JDAA chunk data to alpha_blob.");

            (void) WriteBlob(alpha_image,length,chunk);
          }

        chunk=(unsigned char *) RelinquishMagickMemory(chunk);

        continue;
      }

    if (memcmp(type,mng_JSEP,4) == 0)
      {
        read_JSEP=MagickTrue;

        chunk=(unsigned char *) RelinquishMagickMemory(chunk);

        continue;
      }

    if (memcmp(type,mng_bKGD,4) == 0)
      {
        if (length == 2)
          {
            image->background_color.red=ScaleCharToQuantum(p[1]);
            image->background_color.green=image->background_color.red;
            image->background_color.blue=image->background_color.red;
          }

        if (length == 6)
          {
            image->background_color.red=ScaleCharToQuantum(p[1]);
            image->background_color.green=ScaleCharToQuantum(p[3]);
            image->background_color.blue=ScaleCharToQuantum(p[5]);
          }

        chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_gAMA,4) == 0)
      {
        if (length == 4)
          image->gamma=((double) mng_get_long(p))*0.00001;

        chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_cHRM,4) == 0)
      {
        if (length == 32)
          {
            image->chromaticity.white_point.x=0.00001*mng_get_long(p);
            image->chromaticity.white_point.y=0.00001*mng_get_long(&p[4]);
            image->chromaticity.red_primary.x=0.00001*mng_get_long(&p[8]);
            image->chromaticity.red_primary.y=0.00001*mng_get_long(&p[12]);
            image->chromaticity.green_primary.x=0.00001*mng_get_long(&p[16]);
            image->chromaticity.green_primary.y=0.00001*mng_get_long(&p[20]);
            image->chromaticity.blue_primary.x=0.00001*mng_get_long(&p[24]);
            image->chromaticity.blue_primary.y=0.00001*mng_get_long(&p[28]);
          }

        chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_sRGB,4) == 0)
      {
        if (length == 1)
          {
            image->rendering_intent=
              Magick_RenderingIntent_from_PNG_RenderingIntent(p[0]);
            image->gamma=0.45455f;
            image->chromaticity.red_primary.x=0.6400f;
            image->chromaticity.red_primary.y=0.3300f;
            image->chromaticity.green_primary.x=0.3000f;
            image->chromaticity.green_primary.y=0.6000f;
            image->chromaticity.blue_primary.x=0.1500f;
            image->chromaticity.blue_primary.y=0.0600f;
            image->chromaticity.white_point.x=0.3127f;
            image->chromaticity.white_point.y=0.3290f;
          }

        chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_oFFs,4) == 0)
      {
        if (length > 8)
          {
            image->page.x=(ssize_t) mng_get_long(p);
            image->page.y=(ssize_t) mng_get_long(&p[4]);

            if ((int) p[8] != 0)
              {
                image->page.x/=10000;
                image->page.y/=10000;
              }
          }

        chunk=(unsigned char *) RelinquishMagickMemory(chunk);

        continue;
      }

    if (memcmp(type,mng_pHYs,4) == 0)
      {
        if (length > 8)
          {
            image->resolution.x=(double) mng_get_long(p);
            image->resolution.y=(double) mng_get_long(&p[4]);
            if ((int) p[8] == PNG_RESOLUTION_METER)
              {
                image->units=PixelsPerCentimeterResolution;
                image->resolution.x=image->resolution.x/100.0;
                image->resolution.y=image->resolution.y/100.0;
              }
          }

        chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

#if 0
    if (memcmp(type,mng_iCCP,4) == 0)
      {
        /* To do: */
        chunk=(unsigned char *) RelinquishMagickMemory(chunk);

        continue;
      }
#endif

    chunk=(unsigned char *) RelinquishMagickMemory(chunk);

    if (memcmp(type,mng_IEND,4))
      continue;

    break;
  }


  /* IEND found */

  /*
    Finish up reading image data:

       o read main image from color_blob.

       o close color_blob.

       o if (color_type has alpha)
            if alpha_encoding is PNG
               read secondary image from alpha_blob via ReadPNG
            if alpha_encoding is JPEG
               read secondary image from alpha_blob via ReadJPEG

       o close alpha_blob.

       o copy intensity of secondary image into
         alpha samples of main image.

       o destroy the secondary image.
  */

  if (color_image_info == (ImageInfo *) NULL)
    {
      assert(color_image == (Image *) NULL);
      assert(alpha_image == (Image *) NULL);
      if (color_image != (Image *) NULL)
        color_image=DestroyImageList(color_image);
      return(DestroyImageList(image));
    }

  if (color_image == (Image *) NULL)
    {
      assert(alpha_image == (Image *) NULL);
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    }

  (void) SeekBlob(color_image,0,SEEK_SET);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    Reading jng_image from color_blob.");

  assert(color_image_info != (ImageInfo *) NULL);
  (void) FormatLocaleString(color_image_info->filename,MagickPathExtent,
    "jpeg:%s",color_image->filename);

  color_image_info->ping=MagickFalse;   /* To do: avoid this */
  jng_image=ReadImage(color_image_info,exception);

  (void) RelinquishUniqueFileResource(color_image->filename);
  color_image=DestroyImageList(color_image);
  color_image_info=DestroyImageInfo(color_image_info);

  if (jng_image == (Image *) NULL)
  {
    DestroyJNG(NULL,NULL,NULL,&alpha_image,&alpha_image_info);
    return(DestroyImageList(image));
  }


  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    Copying jng_image pixels to main image.");

  image->rows=jng_height;
  image->columns=jng_width;

  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      DestroyJNG(NULL,&color_image,&color_image_info,&alpha_image,
        &alpha_image_info);
      jng_image=DestroyImageList(jng_image);
      return(DestroyImageList(image));
    }
  if ((image->columns != jng_image->columns) ||
      (image->rows != jng_image->rows))
    {
      DestroyJNG(NULL,&color_image,&color_image_info,&alpha_image,
        &alpha_image_info);
      jng_image=DestroyImageList(jng_image);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    s=GetVirtualPixels(jng_image,0,y,image->columns,1,exception);
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if ((s == (const Quantum *)  NULL) || (q == (Quantum *) NULL))
      break;
    for (x=(ssize_t) image->columns; x != 0; x--)
    {
      SetPixelRed(image,GetPixelRed(jng_image,s),q);
      SetPixelGreen(image,GetPixelGreen(jng_image,s),q);
      SetPixelBlue(image,GetPixelBlue(jng_image,s),q);
      q+=(ptrdiff_t) GetPixelChannels(image);
      s+=(ptrdiff_t) GetPixelChannels(jng_image);
    }

    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }

  jng_image=DestroyImage(jng_image);

  if ((image_info->ping == MagickFalse) && (alpha_image != (Image *) NULL) &&
      (jng_color_type >= 12))
    {
      switch (jng_alpha_compression_method)
      {
        case 0:
        {
          png_byte
            data[5];

          (void) FormatLocaleString(alpha_image_info->filename,MagickPathExtent,
            "png:%s",alpha_image->filename);
          (void) WriteBlobMSBULong(alpha_image,0x00000000L);
          PNGType(data,mng_IEND);
          LogPNGChunk(logging,mng_IEND,0L);
          (void) WriteBlob(alpha_image,4,data);
          (void) WriteBlobMSBULong(alpha_image,crc32(0,data,4));
          break;
        }
        case 8:
        {
          (void) FormatLocaleString(alpha_image_info->filename,MagickPathExtent,
            "jpeg:%s",alpha_image->filename);
          break;
        }
        default:
        {
          (void) FormatLocaleString(alpha_image_info->filename,MagickPathExtent,
            "alpha:%s",alpha_image->filename);
          break;
        }
      }
      (void) CloseBlob(alpha_image);

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Reading alpha from alpha_blob.");
      jng_image=ReadImage(alpha_image_info,exception);

      if (jng_image != (Image *) NULL)
        {
          image->alpha_trait=BlendPixelTrait;
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            s=GetVirtualPixels(jng_image,0,y,image->columns,1,exception);
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
            if ((s == (const Quantum *)  NULL) || (q == (Quantum *) NULL))
              break;

            for (x=(ssize_t) image->columns; x != 0; x--)
            {
              SetPixelAlpha(image,GetPixelRed(jng_image,s),q);
              q+=(ptrdiff_t) GetPixelChannels(image);
              s+=(ptrdiff_t) GetPixelChannels(jng_image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
        }
      (void) RelinquishUniqueFileResource(alpha_image->filename);
      alpha_image=DestroyImageList(alpha_image);
      alpha_image_info=DestroyImageInfo(alpha_image_info);
      if (jng_image != (Image *) NULL)
        jng_image=DestroyImageList(jng_image);
    }

  if (alpha_image != (Image *) NULL)
    alpha_image=DestroyImageList(alpha_image);
  if (alpha_image_info != (ImageInfo *) NULL)
    alpha_image_info=DestroyImageInfo(alpha_image_info);

  /* Read the JNG image.  */

  if (mng_info->mng_type == 0)
    {
      mng_info->mng_width=jng_width;
      mng_info->mng_height=jng_height;
    }

  if (image->page.width == 0 && image->page.height == 0)
    {
      image->page.width=jng_width;
      image->page.height=jng_height;
    }

  if (image->page.x == 0 && image->page.y == 0)
    {
      image->page.x=mng_info->x_off[mng_info->object_id];
      image->page.y=mng_info->y_off[mng_info->object_id];
    }

  else
    {
      image->page.y=mng_info->y_off[mng_info->object_id];
    }

  mng_info->image_found++;
  status=SetImageProgress(image,LoadImagesTag,2*TellBlob(image),
    2*GetBlobSize(image));

  if (status == MagickFalse)
    return(DestroyImageList(image));

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  exit ReadOneJNGImage()");

  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d J N G I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadJNGImage() reads a JPEG Network Graphics (JNG) image file
%  (including the 8-byte signature)  and returns it.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  JNG support written by Glenn Randers-Pehrson, glennrp@image...
%
%  The format of the ReadJNGImage method is:
%
%      Image *ReadJNGImage(const ImageInfo *image_info, ExceptionInfo
%         *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static Image *ReadJNGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    magic_number[MagickPathExtent];

  Image
    *image;

  MagickBooleanType
    logging = MagickFalse,
    status;

  MngReadInfo
    *mng_info;

  size_t
    count;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
       image_info->filename);
  image=AcquireImage(image_info,exception);
  if (image->debug != MagickFalse)
    logging=LogMagickEvent(CoderEvent,GetMagickModule(),"Enter ReadJNGImage()");
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);

  if (status == MagickFalse)
    return(DestroyImageList(image));

  if (LocaleCompare(image_info->magick,"JNG") != 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  /* Verify JNG signature.  */

  count=(size_t) ReadBlob(image,8,(unsigned char *) magic_number);

  if ((count < 8) || (memcmp(magic_number,"\213JNG\r\n\032\n",8) != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  /*
     Verify that file size large enough to contain a JNG datastream.
  */
  if (GetBlobSize(image) < 147)
    ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");

  mng_info=(MngReadInfo *) AcquireMagickMemory(sizeof(*mng_info));
  if (mng_info == (MngReadInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  (void) memset(mng_info,0,sizeof(*mng_info));

  mng_info->image=image;
  image=ReadOneJNGImage(mng_info,image_info,exception);
  mng_info=MngReadInfoFreeStruct(mng_info);

  if (image == (Image *) NULL)
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "exit ReadJNGImage() with error");

      return((Image *) NULL);
    }
  (void) CloseBlob(image);

  if (image->columns == 0 || image->rows == 0)
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "exit ReadJNGImage() with error");

      ThrowReaderException(CorruptImageError,"CorruptImage");
    }

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit ReadJNGImage()");

  return(image);
}
#endif

static Image *ReadOneMNGImage(MngReadInfo* mng_info,
  const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    page_geometry[MagickPathExtent];

  Image
    *image;

  MagickBooleanType
    logging;

  volatile int
    first_mng_object,
    object_id,
    term_chunk_found,
    skip_to_iend;

  volatile ssize_t
    image_count=0;

  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  MngBox
    default_fb,
    fb,
    previous_fb;

  PixelInfo
    mng_background_color;

  ssize_t
    i;

  size_t
    count;

  ssize_t
    loop_level;

  unsigned char
    *p;

  volatile short
    skipping_loop;

  unsigned int
    mandatory_back=0;

  volatile unsigned int
    mng_type=0;   /* 0: PNG or JNG; 1: MNG; 2: MNG-LC; 3: MNG-VLC */

  size_t
    default_frame_timeout,
    frame_timeout,
    image_height,
    image_width,
    length;

  /* These delays are all measured in image ticks_per_second,
   * not in MNG ticks_per_second
   */
  volatile size_t
    default_frame_delay,
    final_delay,
    final_image_delay,
    frame_delay,
    insert_layers,
    mng_iterations=1,
    simplicity=0,
    subframe_height=0,
    subframe_width=0;

  previous_fb.top=0;
  previous_fb.bottom=0;
  previous_fb.left=0;
  previous_fb.right=0;
  default_fb.top=0;
  default_fb.bottom=0;
  default_fb.left=0;
  default_fb.right=0;

  logging=LogMagickEvent(CoderEvent,GetMagickModule(),
    "  Enter ReadOneMNGImage()");

  image=mng_info->image;

  if (LocaleCompare(image_info->magick,"MNG") == 0)
    {
      char
        magic_number[MagickPathExtent];

      /* Verify MNG signature.  */
      count=(size_t) ReadBlob(image,8,(unsigned char *) magic_number);
      if ((count < 8) || (memcmp(magic_number,"\212MNG\r\n\032\n",8) != 0))
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");

      /* Initialize some nonzero members of the MngReadInfo structure.  */
      for (i=0; i < MNG_MAX_OBJECTS; i++)
      {
        mng_info->object_clip[i].right=(ssize_t) PNG_UINT_31_MAX;
        mng_info->object_clip[i].bottom=(ssize_t) PNG_UINT_31_MAX;
      }
      mng_info->exists[0]=MagickTrue;
    }

  skipping_loop=(-1);
  first_mng_object=MagickTrue;
  mng_type=0;
  insert_layers=MagickFalse; /* should be False during convert or mogrify */
  default_frame_delay=0;
  default_frame_timeout=0;
  frame_delay=0;
  final_delay=1;
  mng_info->ticks_per_second=(unsigned long) image->ticks_per_second;
  object_id=0;
  skip_to_iend=MagickFalse;
  term_chunk_found=MagickFalse;
  mng_info->framing_mode=1;
  mandatory_back=MagickFalse;
  mng_background_color=image->background_color;
  default_fb=mng_info->frame;
  previous_fb=mng_info->frame;
  do
  {
    char
      type[MagickPathExtent] = "\0";

    if (LocaleCompare(image_info->magick,"MNG") == 0)
      {
        unsigned char
          *chunk;

        /*
          Read a new chunk.
        */
        type[0]='\0';
        (void) ConcatenateMagickString(type,"errr",MagickPathExtent);
        length=(size_t) ReadBlobMSBLong(image);
        count=(size_t) ReadBlob(image,4,(unsigned char *) type);

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "  Reading MNG chunk type %c%c%c%c, length: %.20g",
           type[0],type[1],type[2],type[3],(double) length);

        if ((length > PNG_UINT_31_MAX) || (length > GetBlobSize(image)) ||
            (count < 4))
          ThrowReaderException(CorruptImageError,"CorruptImage");

        p=NULL;
        chunk=(unsigned char *) NULL;
        if (length != 0)
          {
            chunk=(unsigned char *) AcquireQuantumMemory(length,sizeof(*chunk));
            if (chunk == (unsigned char *) NULL)
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            if (ReadBlob(image,length,chunk) != (ssize_t) length)
              {
                chunk=(unsigned char *) RelinquishMagickMemory(chunk);
                ThrowReaderException(CorruptImageError,"CorruptImage");
              }
            p=chunk;
          }
        (void) ReadBlobMSBLong(image);  /* read crc word */

#if !defined(JNG_SUPPORTED)
        if (memcmp(type,mng_JHDR,4) == 0)
          {
            skip_to_iend=MagickTrue;

            if (mng_info->jhdr_warning == 0)
              (void) ThrowMagickException(exception,GetMagickModule(),
                CoderError,"JNGCompressNotSupported","`%s'",image->filename);

            mng_info->jhdr_warning++;
          }
#endif
        if (memcmp(type,mng_DHDR,4) == 0)
          {
            skip_to_iend=MagickTrue;

            if (mng_info->dhdr_warning == 0)
              (void) ThrowMagickException(exception,GetMagickModule(),
                CoderError,"DeltaPNGNotSupported","`%s'",image->filename);

            mng_info->dhdr_warning++;
          }
        if (memcmp(type,mng_MEND,4) == 0)
          {
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            break;
          }

        if (skip_to_iend)
          {
            if (memcmp(type,mng_IEND,4) == 0)
              skip_to_iend=MagickFalse;

            chunk=(unsigned char *) RelinquishMagickMemory(chunk);

            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  Skip to IEND.");

            continue;
          }

        if (memcmp(type,mng_MHDR,4) == 0)
          {
            if (length != 28)
              {
                chunk=(unsigned char *) RelinquishMagickMemory(chunk);
                ThrowReaderException(CorruptImageError,"CorruptImage");
              }

            mng_info->mng_width=(unsigned long)mng_get_long(p);
            mng_info->mng_height=(unsigned long)mng_get_long(&p[4]);

            if (logging != MagickFalse)
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  MNG width: %.20g",(double) mng_info->mng_width);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  MNG height: %.20g",(double) mng_info->mng_height);
              }

            p+=(ptrdiff_t) 8;
            mng_info->ticks_per_second=(size_t) mng_get_long(p);

            if (mng_info->ticks_per_second == 0)
              default_frame_delay=0;

            else
              default_frame_delay=(size_t) image->ticks_per_second/
                (size_t) mng_info->ticks_per_second;

            frame_delay=default_frame_delay;
            simplicity=0;

            p+=(ptrdiff_t) 16;
            simplicity=(size_t) mng_get_long(p);

            mng_type=1;    /* Full MNG */

            if ((simplicity != 0) && ((simplicity | 11) == 11))
              mng_type=2; /* LC */

            if ((simplicity != 0) && ((simplicity | 9) == 9))
              mng_type=3; /* VLC */

            if (mng_type != 3)
              insert_layers=MagickTrue;
            if (GetAuthenticPixelQueue(image) != (Quantum *) NULL)
              {
                /* Allocate next image structure.  */
                AcquireNextImage(image_info,image,exception);

                if (GetNextImageInList(image) == (Image *) NULL)
                  return((Image *) NULL);

                image=SyncNextImageInList(image);
                mng_info->image=image;
              }

            if ((mng_info->mng_width > 65535L) ||
                (mng_info->mng_height > 65535L))
              {
                chunk=(unsigned char *) RelinquishMagickMemory(chunk);
                ThrowReaderException(ImageError,"WidthOrHeightExceedsLimit");
              }

            (void) FormatLocaleString(page_geometry,MagickPathExtent,
              "%.20gx%.20g+0+0",(double) mng_info->mng_width,(double)
              mng_info->mng_height);

            mng_info->frame.left=0;
            mng_info->frame.right=(ssize_t) mng_info->mng_width;
            mng_info->frame.top=0;
            mng_info->frame.bottom=(ssize_t) mng_info->mng_height;
            mng_info->clip=default_fb=previous_fb=mng_info->frame;

            for (i=0; i < MNG_MAX_OBJECTS; i++)
              mng_info->object_clip[i]=mng_info->frame;

            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_TERM,4) == 0)
          {
            int
              repeat=0;

            if (length != 0)
              repeat=p[0];

            if (repeat == 3 && length > 9)
              {
                final_delay=(png_uint_32) mng_get_long(&p[2]);
                mng_iterations=(png_uint_32) mng_get_long(&p[6]);

                if (mng_iterations == PNG_UINT_31_MAX)
                  mng_iterations=0;

                image->iterations=mng_iterations;
                term_chunk_found=MagickTrue;
              }

            if (logging != MagickFalse)
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    repeat=%d,  final_delay=%.20g,  iterations=%.20g",
                  repeat,(double) final_delay, (double) image->iterations);
              }

            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_DEFI,4) == 0)
          {
            if (mng_type == 3)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  CoderError,"DEFI chunk found in MNG-VLC datastream","`%s'",
                  image->filename);
                chunk=(unsigned char *) RelinquishMagickMemory(chunk);
                continue;
              }

            if (length < 2)
              {
                chunk=(unsigned char *) RelinquishMagickMemory(chunk);
                ThrowReaderException(CorruptImageError,"CorruptImage");
              }

            object_id=(int) (((unsigned int) p[0] << 8) | (unsigned int) p[1]);

            if (mng_type == 2 && object_id != 0)
              (void) ThrowMagickException(exception,GetMagickModule(),
                CoderError,"Nonzero object_id in MNG-LC datastream","`%s'",
                image->filename);

            if (object_id >= MNG_MAX_OBJECTS)
              {
                /*
                  Instead of using a warning we should allocate a larger
                  MngReadInfo structure and continue.
                */
                (void) ThrowMagickException(exception,GetMagickModule(),
                  CoderError,"object id too large","`%s'",image->filename);
                object_id=MNG_MAX_OBJECTS-1;
              }

            if (mng_info->exists[object_id])
              if (mng_info->frozen[object_id])
                {
                  chunk=(unsigned char *) RelinquishMagickMemory(chunk);
                  (void) ThrowMagickException(exception,
                    GetMagickModule(),CoderError,
                    "DEFI cannot redefine a frozen MNG object","`%s'",
                    image->filename);
                  continue;
                }

            mng_info->exists[object_id]=MagickTrue;

            if (length > 2)
              mng_info->invisible[object_id]=p[2];

            /*
              Extract object offset info.
            */
            if (length > 11)
              {
                mng_info->x_off[object_id]=(ssize_t) mng_get_long(&p[4]);
                mng_info->y_off[object_id]=(ssize_t) mng_get_long(&p[8]);
                if (logging != MagickFalse)
                  {
                    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                      "  x_off[%d]: %.20g,  y_off[%d]: %.20g",
                      object_id,(double) mng_info->x_off[object_id],
                      object_id,(double) mng_info->y_off[object_id]);
                  }
              }

            /*
              Extract object clipping info.
            */
            if (length > 27)
              mng_info->object_clip[object_id]=mng_read_box(mng_info->frame,0,
                &p[12]);

            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_bKGD,4) == 0)
          {
            mng_info->have_global_bkgd=MagickFalse;

            if (length > 5)
              {
                mng_info->mng_global_bkgd.red=ScaleShortToQuantum(
                  (unsigned short) (((unsigned int) p[0] << 8) | p[1]));
                mng_info->mng_global_bkgd.green=ScaleShortToQuantum(
                  (unsigned short) (((unsigned int) p[2] << 8) | p[3]));
                mng_info->mng_global_bkgd.blue=ScaleShortToQuantum(
                  (unsigned short) (((unsigned int) p[4] << 8) | p[5]));
                mng_info->have_global_bkgd=MagickTrue;
              }

            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_BACK,4) == 0)
          {
            if (length > 6)
              mandatory_back=p[6];

            else
              mandatory_back=0;

            if (mandatory_back && length > 5)
              {
                mng_background_color.red=ScaleShortToQuantum((unsigned short)
                  (((unsigned int) p[0] << 8) | p[1]));
                mng_background_color.green=ScaleShortToQuantum((unsigned short)
                  (((unsigned int) p[2] << 8) | p[3]));
                mng_background_color.blue=ScaleShortToQuantum((unsigned short)
                  (((unsigned int) p[4] << 8) | p[5]));
                mng_background_color.alpha=OpaqueAlpha;
              }
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_PLTE,4) == 0)
          {
            /* Read global PLTE.  */

            if (length && (length < 769))
              {
                /* Read global PLTE.  */

                if (mng_info->global_plte == (png_colorp) NULL)
                  mng_info->global_plte=(png_colorp) AcquireQuantumMemory(256,
                    sizeof(*mng_info->global_plte));

                if (mng_info->global_plte == (png_colorp) NULL)
                  {
                    mng_info->global_plte_length=0;
                    chunk=(unsigned char *) RelinquishMagickMemory(chunk);
                    ThrowReaderException(ResourceLimitError,
                      "MemoryAllocationFailed");
                  }

                for (i=0; i < (ssize_t) (length/3); i++)
                {
                  mng_info->global_plte[i].red=p[3*i];
                  mng_info->global_plte[i].green=p[3*i+1];
                  mng_info->global_plte[i].blue=p[3*i+2];
                }

                mng_info->global_plte_length=(unsigned int) (length/3);
              }
            else
              mng_info->global_plte_length=0;

            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_tRNS,4) == 0)
          {
            /* read global tRNS */

            if (length > 0 && length < 257)
              for (i=0; i < (ssize_t) length; i++)
                mng_info->global_trns[i]=p[i];
            mng_info->global_trns_length=(unsigned int) length;
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_gAMA,4) == 0)
          {
            if (length == 4)
              {
                ssize_t
                  igamma;

                igamma=mng_get_long(p);
                mng_info->global_gamma=((float) igamma)*0.00001f;
                mng_info->have_global_gama=MagickTrue;
              }

            else
              mng_info->have_global_gama=MagickFalse;

            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_cHRM,4) == 0)
          {
            /* Read global cHRM */

            if (length == 32)
              {
                mng_info->global_chrm.white_point.x=0.00001*mng_get_long(p);
                mng_info->global_chrm.white_point.y=0.00001*mng_get_long(&p[4]);
                mng_info->global_chrm.red_primary.x=0.00001*mng_get_long(&p[8]);
                mng_info->global_chrm.red_primary.y=0.00001*
                  mng_get_long(&p[12]);
                mng_info->global_chrm.green_primary.x=0.00001*
                  mng_get_long(&p[16]);
                mng_info->global_chrm.green_primary.y=0.00001*
                  mng_get_long(&p[20]);
                mng_info->global_chrm.blue_primary.x=0.00001*
                  mng_get_long(&p[24]);
                mng_info->global_chrm.blue_primary.y=0.00001*
                  mng_get_long(&p[28]);
                mng_info->have_global_chrm=MagickTrue;
              }
            else
              mng_info->have_global_chrm=MagickFalse;

            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_sRGB,4) == 0)
          {
            /*
              Read global sRGB.
            */
            if (length != 0)
              {
                mng_info->global_srgb_intent=
                  Magick_RenderingIntent_from_PNG_RenderingIntent(p[0]);
                mng_info->have_global_srgb=MagickTrue;
              }
            else
              mng_info->have_global_srgb=MagickFalse;

            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_iCCP,4) == 0)
          {
            /* To do: */

            /*
              Read global iCCP.
            */
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);

            continue;
          }

        if (memcmp(type,mng_FRAM,4) == 0)
          {
            if (mng_type == 3)
              (void) ThrowMagickException(exception,GetMagickModule(),
                CoderError,"FRAM chunk found in MNG-VLC datastream","`%s'",
                image->filename);

            if ((mng_info->framing_mode == 2) || (mng_info->framing_mode == 4))
              image->delay=frame_delay;

            frame_delay=default_frame_delay;
            frame_timeout=default_frame_timeout;
            fb=default_fb;

            if (length != 0)
              if (p[0])
                mng_info->framing_mode=p[0];

            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    Framing_mode=%d",mng_info->framing_mode);

            if (length > 6)
              {
                /* Note the delay and frame clipping boundaries.  */

                p++; /* framing mode */

                while (((p-chunk) < (long) length) && *p)
                  p++;  /* frame name */

                p++;  /* frame name terminator */

                if ((p-chunk) < (ssize_t) (length-4))
                  {
                    int
                      change_delay,
                      change_timeout,
                      change_clipping;

                    change_delay=(*p++);
                    change_timeout=(*p++);
                    change_clipping=(*p++);
                    p++; /* change_sync */

                    if (change_delay && ((p-chunk) < (ssize_t) (length-4)))
                      {
                        frame_delay=(size_t) image->ticks_per_second*
                          (size_t) mng_get_long(p);

                        if (mng_info->ticks_per_second != 0)
                          frame_delay/=mng_info->ticks_per_second;

                        else
                          frame_delay=PNG_UINT_31_MAX;

                        if (change_delay == 2)
                          default_frame_delay=frame_delay;

                        p+=(ptrdiff_t) 4;

                        if (logging != MagickFalse)
                          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "    Framing_delay=%.20g",(double) frame_delay);
                      }

                    if (change_timeout && ((p-chunk) < (ssize_t) (length-4)))
                      {
                        frame_timeout=(size_t) image->ticks_per_second*
                          (size_t) mng_get_long(p);

                        if (mng_info->ticks_per_second != 0)
                          frame_timeout/=mng_info->ticks_per_second;

                        else
                          frame_timeout=PNG_UINT_31_MAX;

                        if (change_timeout == 2)
                          default_frame_timeout=frame_timeout;

                        p+=(ptrdiff_t) 4;

                        if (logging != MagickFalse)
                          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "    Framing_timeout=%.20g",(double) frame_timeout);
                      }

                    if (change_clipping && ((p-chunk) < (ssize_t) (length-16)))
                      {
                        fb=mng_read_box(previous_fb,(char) p[0],&p[1]);
                        p+=(ptrdiff_t) 16;
                        previous_fb=fb;

                        if (logging != MagickFalse)
                          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "    Frame_clip: L=%.20g R=%.20g T=%.20g B=%.20g",
                            (double) fb.left,(double) fb.right,(double) fb.top,
                            (double) fb.bottom);

                        if (change_clipping == 2)
                          default_fb=fb;
                      }
                  }
              }
            mng_info->clip=fb;
            mng_info->clip=mng_minimum_box(fb,mng_info->frame);

            subframe_width=(size_t) (mng_info->clip.right
               -mng_info->clip.left);

            subframe_height=(size_t) (mng_info->clip.bottom
               -mng_info->clip.top);
            /*
              Insert a background layer behind the frame if framing_mode is 4.
            */
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "   subframe_width=%.20g, subframe_height=%.20g",(double)
                subframe_width,(double) subframe_height);

            if (insert_layers && (mng_info->framing_mode == 4) &&
                (subframe_width) && (subframe_height))
              {
                /* Allocate next image structure.  */
                if (GetAuthenticPixelQueue(image) != (Quantum *) NULL)
                  {
                    AcquireNextImage(image_info,image,exception);

                    if (GetNextImageInList(image) == (Image *) NULL)
                      return(DestroyImageList(image));

                    image=SyncNextImageInList(image);
                  }

                mng_info->image=image;

                if (term_chunk_found)
                  {
                    image->start_loop=MagickTrue;
                    image->iterations=mng_iterations;
                    term_chunk_found=MagickFalse;
                  }

                else
                    image->start_loop=MagickFalse;

                image->columns=subframe_width;
                image->rows=subframe_height;
                image->page.width=subframe_width;
                image->page.height=subframe_height;
                image->page.x=mng_info->clip.left;
                image->page.y=mng_info->clip.top;
                image->background_color=mng_background_color;
                image->alpha_trait=UndefinedPixelTrait;
                image->delay=0;
                if (SetImageBackgroundColor(image,exception) == MagickFalse)
                  {
                    chunk=(unsigned char *) RelinquishMagickMemory(chunk);
                    return(DestroyImageList(image));
                  }
                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  Insert backgd layer, L=%.20g, R=%.20g T=%.20g, B=%.20g",
                    (double) mng_info->clip.left,
                    (double) mng_info->clip.right,
                    (double) mng_info->clip.top,
                    (double) mng_info->clip.bottom);
              }
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_CLIP,4) == 0)
          {
            unsigned int
              first_object,
              last_object;

            /*
              Read CLIP.
            */
            if (length > 3)
              {
                first_object=(unsigned int) ((p[0] << 8) | p[1]);
                last_object=(unsigned int) ((p[2] << 8) | p[3]);
                p+=(ptrdiff_t) 4;

                for (i=(int) first_object; i <= (int) last_object; i++)
                {
                  if ((i < 0) || (i >= MNG_MAX_OBJECTS))
                    continue;

                  if (mng_info->exists[i] && !mng_info->frozen[i])
                    {
                      MngBox
                        box;

                      box=mng_info->object_clip[i];
                      if ((p-chunk) < (ssize_t) (length-17))
                        mng_info->object_clip[i]=
                           mng_read_box(box,(char) p[0],&p[1]);
                    }
                }

              }
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_SAVE,4) == 0)
          {
            for (i=1; i < MNG_MAX_OBJECTS; i++)
              if (mng_info->exists[i])
                 mng_info->frozen[i]=MagickTrue;

            chunk=(unsigned char *) RelinquishMagickMemory(chunk);

            continue;
          }

        if ((memcmp(type,mng_DISC,4) == 0) || (memcmp(type,mng_SEEK,4) == 0))
          {
            /* Read DISC or SEEK.  */

            if ((length == 0) || (length % 2) || !memcmp(type,mng_SEEK,4))
              {
                for (i=1; i < MNG_MAX_OBJECTS; i++)
                  MngReadInfoDiscardObject(mng_info,(int) i);
              }

            else
              {
                ssize_t
                  j;

                for (j=1; j < (ssize_t) length; j+=2)
                {
                  i=p[j-1] << 8 | p[j];
                  MngReadInfoDiscardObject(mng_info,(int) i);
                }
              }

            chunk=(unsigned char *) RelinquishMagickMemory(chunk);

            continue;
          }

        if (memcmp(type,mng_MOVE,4) == 0)
          {
            size_t
              first_object,
              last_object;

            /* read MOVE */

            if (length > 3)
            {
              first_object=(unsigned int) ((p[0] << 8) | p[1]);
              last_object=(unsigned int) ((p[2] << 8) | p[3]);
              p+=(ptrdiff_t) 4;

              for (i=(ssize_t) first_object; i <= (ssize_t) last_object; i++)
              {
                if ((i < 0) || (i >= MNG_MAX_OBJECTS))
                  continue;

                if (mng_info->exists[i] && !mng_info->frozen[i] &&
                    (p-chunk) < (ssize_t) (length-8))
                  {
                    MngPair
                      new_pair;

                    MngPair
                      old_pair;

                    old_pair.a=(volatile long) mng_info->x_off[i];
                    old_pair.b=(volatile long) mng_info->y_off[i];
                    new_pair=mng_read_pair(old_pair,(int) p[0],&p[1]);
                    mng_info->x_off[i]=new_pair.a;
                    mng_info->y_off[i]=new_pair.b;
                  }
              }
            }

            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_LOOP,4) == 0)
          {
            ssize_t loop_iters=1;
            if (length > 4)
              {
                loop_level=chunk[0];
                mng_info->loop_active[loop_level]=1;  /* mark loop active */

                /* Record starting point.  */
                loop_iters=mng_get_long(&chunk[1]);

                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  LOOP level %.20g has %.20g iterations ",
                    (double) loop_level, (double) loop_iters);

                if (loop_iters <= 0)
                  skipping_loop=(volatile short) loop_level;

                else
                  {
                    if ((MagickSizeType) loop_iters > GetMagickResourceLimit(ListLengthResource))
                      loop_iters=(ssize_t) GetMagickResourceLimit(ListLengthResource);
                    if (loop_iters >= 2147483647L)
                      loop_iters=2147483647L;
                    if (image_info->number_scenes != 0)
                      if (loop_iters > (ssize_t) image_info->number_scenes)
                        loop_iters=(ssize_t) image_info->number_scenes;
                    mng_info->loop_jump[loop_level]=TellBlob(image);
                    mng_info->loop_count[loop_level]=loop_iters;
                  }

                mng_info->loop_iteration[loop_level]=0;
              }
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_ENDL,4) == 0)
          {
            if (length > 0)
              {
                loop_level=chunk[0];

                if (skipping_loop > 0)
                  {
                    if (skipping_loop == loop_level)
                      {
                        /*
                          Found end of zero-iteration loop.
                        */
                        skipping_loop=(-1);
                        mng_info->loop_active[loop_level]=0;
                      }
                  }

                else
                  {
                    if (mng_info->loop_active[loop_level] == 1)
                      {
                        mng_info->loop_count[loop_level]--;
                        mng_info->loop_iteration[loop_level]++;

                        if (logging != MagickFalse)
                          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "  ENDL: LOOP level %.20g has %.20g remaining iters",
                            (double) loop_level,(double)
                            mng_info->loop_count[loop_level]);

                        if (mng_info->loop_count[loop_level] > 0)
                          {
                            offset=
                              SeekBlob(image,mng_info->loop_jump[loop_level],
                              SEEK_SET);

                            if (offset < 0)
                              {
                                chunk=(unsigned char *) RelinquishMagickMemory(
                                  chunk);
                                ThrowReaderException(CorruptImageError,
                                  "ImproperImageHeader");
                              }
                          }

                        else
                          {
                            short
                              last_level;

                            /*
                              Finished loop.
                            */
                            mng_info->loop_active[loop_level]=0;
                            last_level=(-1);
                            for (i=0; i < loop_level; i++)
                              if (mng_info->loop_active[i] == 1)
                                last_level=(short) i;
                            loop_level=last_level;
                          }
                      }
                  }
              }

            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_CLON,4) == 0)
          {
            if (mng_info->clon_warning == 0)
              (void) ThrowMagickException(exception,GetMagickModule(),
                CoderError,"CLON is not implemented yet","`%s'",
                image->filename);

            mng_info->clon_warning++;
          }

        if (memcmp(type,mng_MAGN,4) == 0)
          {
            png_uint_16
              magn_first,
              magn_last,
              magn_mb,
              magn_ml,
              magn_mr,
              magn_mt,
              magn_mx,
              magn_my,
              magn_methx,
              magn_methy;

            if (length > 1)
              magn_first=(p[0] << 8) | p[1];

            else
              magn_first=0;

            if (length > 3)
              magn_last=(p[2] << 8) | p[3];

            else
              magn_last=magn_first;
            if (magn_first || magn_last)
              if (mng_info->magn_warning == 0)
                {
                  (void) ThrowMagickException(exception,
                     GetMagickModule(),CoderError,
                     "MAGN is not implemented yet for nonzero objects",
                     "`%s'",image->filename);

                   mng_info->magn_warning++;
                }
            if (length > 4)
              magn_methx=p[4];

            else
              magn_methx=0;

            if (length > 6)
              magn_mx=(p[5] << 8) | p[6];

            else
              magn_mx=1;

            if (magn_mx == 0)
              magn_mx=1;

            if (length > 8)
              magn_my=(p[7] << 8) | p[8];

            else
              magn_my=magn_mx;

            if (magn_my == 0)
              magn_my=1;

            if (length > 10)
              magn_ml=(p[9] << 8) | p[10];

            else
              magn_ml=magn_mx;

            if (magn_ml == 0)
              magn_ml=1;

            if (length > 12)
              magn_mr=(p[11] << 8) | p[12];

            else
              magn_mr=magn_mx;

            if (magn_mr == 0)
              magn_mr=1;

            if (length > 14)
              magn_mt=(p[13] << 8) | p[14];

            else
              magn_mt=magn_my;

            if (magn_mt == 0)
              magn_mt=1;

            if (length > 16)
              magn_mb=(p[15] << 8) | p[16];

            else
              magn_mb=magn_my;

            if (magn_mb == 0)
              magn_mb=1;

            if (length > 17)
              magn_methy=p[17];

            else
              magn_methy=magn_methx;


            if (magn_methx > 5 || magn_methy > 5)
              if (mng_info->magn_warning == 0)
                {
                  (void) ThrowMagickException(exception,
                     GetMagickModule(),CoderError,
                     "Unknown MAGN method in MNG datastream","`%s'",
                     image->filename);

                   mng_info->magn_warning++;
                }
            if (magn_first == 0 || magn_last == 0)
              {
                /* Save the magnification factors for object 0 */
                mng_info->magn_mb=magn_mb;
                mng_info->magn_ml=magn_ml;
                mng_info->magn_mr=magn_mr;
                mng_info->magn_mt=magn_mt;
                mng_info->magn_mx=magn_mx;
                mng_info->magn_my=magn_my;
                mng_info->magn_methx=magn_methx;
                mng_info->magn_methy=magn_methy;
              }
          }

        if (memcmp(type,mng_PAST,4) == 0)
          {
            if (mng_info->past_warning == 0)
              (void) ThrowMagickException(exception,GetMagickModule(),
                CoderError,"PAST is not implemented yet","`%s'",
                image->filename);

            mng_info->past_warning++;
          }

        if (memcmp(type,mng_SHOW,4) == 0)
          {
            if (mng_info->show_warning == 0)
              (void) ThrowMagickException(exception,GetMagickModule(),
                CoderError,"SHOW is not implemented yet","`%s'",
                image->filename);

            mng_info->show_warning++;
          }

        if (memcmp(type,mng_sBIT,4) == 0)
          {
            if (length < 4)
              mng_info->have_global_sbit=MagickFalse;

            else
              {
                mng_info->global_sbit.gray=p[0];
                mng_info->global_sbit.red=p[0];
                mng_info->global_sbit.green=p[1];
                mng_info->global_sbit.blue=p[2];
                mng_info->global_sbit.alpha=p[3];
                mng_info->have_global_sbit=MagickTrue;
             }
          }
        if (memcmp(type,mng_pHYs,4) == 0)
          {
            if (length > 8)
              {
                mng_info->global_x_pixels_per_unit=
                    (size_t) mng_get_long(p);
                mng_info->global_y_pixels_per_unit=
                    (size_t) mng_get_long(&p[4]);
                mng_info->global_phys_unit_type=p[8];
                mng_info->have_global_phys=MagickTrue;
              }

            else
              mng_info->have_global_phys=MagickFalse;
          }
        if (memcmp(type,mng_pHYg,4) == 0)
          {
            if (mng_info->phyg_warning == 0)
              (void) ThrowMagickException(exception,GetMagickModule(),
                CoderError,"pHYg is not implemented.","`%s'",image->filename);

            mng_info->phyg_warning++;
          }
        if (memcmp(type,mng_BASI,4) == 0)
          {
            skip_to_iend=MagickTrue;

            if (mng_info->basi_warning == 0)
              (void) ThrowMagickException(exception,GetMagickModule(),
                CoderError,"BASI is not implemented yet","`%s'",
                image->filename);

            mng_info->basi_warning++;
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_IHDR,4)
#if defined(JNG_SUPPORTED)
            && memcmp(type,mng_JHDR,4)
#endif
            )
          {
            /* Not an IHDR or JHDR chunk */
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);

            continue;
          }
/* Process IHDR */
        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Processing %c%c%c%c chunk",type[0],type[1],type[2],type[3]);

        mng_info->exists[object_id]=MagickTrue;
        mng_info->viewable[object_id]=MagickTrue;

        if (mng_info->invisible[object_id])
          {
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  Skipping invisible object");

            skip_to_iend=MagickTrue;
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (length < 8)
          {
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
          }

        image_width=(size_t) mng_get_long(p);
        image_height=(size_t) mng_get_long(&p[4]);
        chunk=(unsigned char *) RelinquishMagickMemory(chunk);

        /*
          Insert a transparent background layer behind the entire animation
          if it is not full screen.
        */
        if (insert_layers && mng_type && first_mng_object)
          {
            if ((mng_info->clip.left > 0) || (mng_info->clip.top > 0) ||
                (image_width < mng_info->mng_width) ||
                (mng_info->clip.right < (ssize_t) mng_info->mng_width) ||
                (image_height < mng_info->mng_height) ||
                (mng_info->clip.bottom < (ssize_t) mng_info->mng_height))
              {
                if (GetAuthenticPixelQueue(image) != (Quantum *) NULL)
                  {
                    /*
                      Allocate next image structure.
                    */
                    AcquireNextImage(image_info,image,exception);

                    if (GetNextImageInList(image) == (Image *) NULL)
                      return(DestroyImageList(image));

                    image=SyncNextImageInList(image);
                  }
                mng_info->image=image;

                if (term_chunk_found)
                  {
                    image->start_loop=MagickTrue;
                    image->iterations=mng_iterations;
                    term_chunk_found=MagickFalse;
                  }

                else
                    image->start_loop=MagickFalse;

                /* Make a background rectangle.  */

                image->delay=0;
                image->columns=mng_info->mng_width;
                image->rows=mng_info->mng_height;
                image->page.width=mng_info->mng_width;
                image->page.height=mng_info->mng_height;
                image->page.x=0;
                image->page.y=0;
                image->background_color=mng_background_color;
                (void) SetImageBackgroundColor(image,exception);
                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  Inserted transparent background layer, W=%.20g, H=%.20g",
                    (double) mng_info->mng_width,(double) mng_info->mng_height);
              }
          }
        /*
          Insert a background layer behind the upcoming image if
          framing_mode is 3, and we haven't already inserted one.
        */
        if (insert_layers && (mng_info->framing_mode == 3) &&
                (subframe_width) && (subframe_height) && (simplicity == 0 ||
                (simplicity & 0x08)))
          {
            if (GetAuthenticPixelQueue(image) != (Quantum *) NULL)
            {
              /*
                Allocate next image structure.
              */
              AcquireNextImage(image_info,image,exception);

              if (GetNextImageInList(image) == (Image *) NULL)
                return(DestroyImageList(image));

              image=SyncNextImageInList(image);
            }

            mng_info->image=image;

            if (term_chunk_found)
              {
                image->start_loop=MagickTrue;
                image->iterations=mng_iterations;
                term_chunk_found=MagickFalse;
              }

            else
                image->start_loop=MagickFalse;

            image->delay=0;
            image->columns=subframe_width;
            image->rows=subframe_height;
            image->page.width=subframe_width;
            image->page.height=subframe_height;
            image->page.x=mng_info->clip.left;
            image->page.y=mng_info->clip.top;
            image->background_color=mng_background_color;
            image->alpha_trait=UndefinedPixelTrait;
            (void) SetImageBackgroundColor(image,exception);

            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  Insert background layer, L=%.20g, R=%.20g T=%.20g, B=%.20g",
                (double) mng_info->clip.left,(double) mng_info->clip.right,
                (double) mng_info->clip.top,(double) mng_info->clip.bottom);
          }
        first_mng_object=MagickFalse;

        if (GetAuthenticPixelQueue(image) != (Quantum *) NULL)
          {
            /*
              Allocate next image structure.
            */
            AcquireNextImage(image_info,image,exception);

            if (GetNextImageInList(image) == (Image *) NULL)
              return(DestroyImageList(image));

            image=SyncNextImageInList(image);
          }
        mng_info->image=image;
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));

        if (status == MagickFalse)
          break;

        if (term_chunk_found)
          {
            image->start_loop=MagickTrue;
            term_chunk_found=MagickFalse;
          }

        else
            image->start_loop=MagickFalse;

        if (mng_info->framing_mode == 1 || mng_info->framing_mode == 3)
          {
            image->delay=frame_delay;
            frame_delay=default_frame_delay;
          }

        else
          image->delay=0;

        image->page.width=mng_info->mng_width;
        image->page.height=mng_info->mng_height;
        image->page.x=mng_info->x_off[object_id];
        image->page.y=mng_info->y_off[object_id];
        image->iterations=mng_iterations;

        /*
          Seek back to the beginning of the IHDR or JHDR chunk's length field.
        */

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Seeking back to beginning of %c%c%c%c chunk",type[0],type[1],
            type[2],type[3]);

        offset=SeekBlob(image,-((ssize_t) length+12),SEEK_CUR);

        if (offset < 0)
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }

    mng_info->image=image;
    mng_info->mng_type=mng_type;
    mng_info->object_id=object_id;

    if (memcmp(type,mng_IHDR,4) == 0)
      image=ReadOnePNGImage(mng_info,image_info,exception);

#if defined(JNG_SUPPORTED)
    else
      image=ReadOneJNGImage(mng_info,image_info,exception);
#endif

    if (image == (Image *) NULL)
      {
        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "exit ReadJNGImage() with error");
        return((Image *) NULL);
      }

    if (image->columns == 0 || image->rows == 0)
      {
        (void) CloseBlob(image);
        return(DestroyImageList(image));
      }

    if (exception->severity > ErrorException)
      {
        (void) CloseBlob(image);
        return(DestroyImageList(image));
      }

    mng_info->image=image;

    if (mng_type)
      {
        MngBox
          crop_box;

        if (((mng_info->magn_methx > 0) && (mng_info->magn_methx <= 5)) &&
            ((mng_info->magn_methy > 0) && (mng_info->magn_methy <= 5)))
          {
            size_t
               magnified_height,
               magnified_width;

            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  Processing MNG MAGN chunk");

            if (image->columns == 1)
              mng_info->magn_methx = 1;
            if (image->rows == 1)
              mng_info->magn_methy = 1;
            if (mng_info->magn_methx == 1)
              {
                magnified_width=(size_t) mng_info->magn_ml;

                if (image->columns > 1)
                   magnified_width += mng_info->magn_mr;

                if (image->columns > 2)
                   magnified_width += (size_t)
                      ((image->columns-2)*(mng_info->magn_mx));
              }

            else
              {
                magnified_width=(size_t) image->columns;

                if (image->columns > 1)
                   magnified_width += mng_info->magn_ml-1;

                if (image->columns > 2)
                   magnified_width += mng_info->magn_mr-1;

                if (image->columns > 3)
                   magnified_width += (size_t)
                      ((image->columns-3)*(mng_info->magn_mx-1));
              }

            if (mng_info->magn_methy == 1)
              {
                magnified_height=(size_t) mng_info->magn_mt;

                if (image->rows > 1)
                   magnified_height += mng_info->magn_mb;

                if (image->rows > 2)
                   magnified_height += (size_t)
                      ((image->rows-2)*(mng_info->magn_my));
              }

            else
              {
                magnified_height=(size_t) image->rows;

                if (image->rows > 1)
                   magnified_height += mng_info->magn_mt-1;

                if (image->rows > 2)
                   magnified_height += mng_info->magn_mb-1;

                if (image->rows > 3)
                   magnified_height += (size_t)
                      ((image->rows-3)*(mng_info->magn_my-1));
              }

            if (magnified_height > image->rows ||
                magnified_width > image->columns)
              {
                Image
                  *large_image;

                int
                  yy;

                Quantum
                  *next,
                  *prev;

                png_uint_16
                  magn_methx,
                  magn_methy;

                ssize_t
                  m,
                  y;

                Quantum
                  *n,
                  *q;

                ssize_t
                  x;

                /* Allocate next image structure.  */

                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "    Allocate magnified image");

                AcquireNextImage(image_info,image,exception);

                if (GetNextImageInList(image) == (Image *) NULL)
                  return(DestroyImageList(image));

                large_image=SyncNextImageInList(image);

                large_image->columns=magnified_width;
                large_image->rows=magnified_height;

                magn_methx=mng_info->magn_methx;
                magn_methy=mng_info->magn_methy;

#if (MAGICKCORE_QUANTUM_DEPTH > 16)
#define QM unsigned short
                if (magn_methx != 1 || magn_methy != 1)
                  {
                  /*
                     Scale pixels to unsigned shorts to prevent
                     overflow of intermediate values of interpolations
                  */
                     for (y=0; y < (ssize_t) image->rows; y++)
                     {
                       q=GetAuthenticPixels(image,0,y,image->columns,1,
                          exception);
                       if (q == (Quantum *) NULL)
                         break;
                       for (x=(ssize_t) image->columns-1; x >= 0; x--)
                       {
                          SetPixelRed(image,ScaleQuantumToShort(
                            GetPixelRed(image,q)),q);
                          SetPixelGreen(image,ScaleQuantumToShort(
                            GetPixelGreen(image,q)),q);
                          SetPixelBlue(image,ScaleQuantumToShort(
                            GetPixelBlue(image,q)),q);
                          SetPixelAlpha(image,ScaleQuantumToShort(
                            GetPixelAlpha(image,q)),q);
                          q+=(ptrdiff_t) GetPixelChannels(image);
                       }

                       if (SyncAuthenticPixels(image,exception) == MagickFalse)
                         break;
                     }
                  }
#else
#define QM Quantum
#endif

                if (image->alpha_trait != UndefinedPixelTrait)
                   (void) SetImageBackgroundColor(large_image,exception);

                else
                  {
                    large_image->background_color.alpha=OpaqueAlpha;
                    (void) SetImageBackgroundColor(large_image,exception);

                    if (magn_methx == 4)
                      magn_methx=2;

                    if (magn_methx == 5)
                      magn_methx=3;

                    if (magn_methy == 4)
                      magn_methy=2;

                    if (magn_methy == 5)
                      magn_methy=3;
                  }

                /* magnify the rows into the right side of the large image */

                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "    Magnify the rows to %.20g",
                    (double) large_image->rows);
                m=(ssize_t) mng_info->magn_mt;
                yy=0;
                length=(size_t) GetPixelChannels(image)*image->columns;
                next=(Quantum *) AcquireQuantumMemory(length,sizeof(*next));
                prev=(Quantum *) AcquireQuantumMemory(length,sizeof(*prev));

                if ((prev == (Quantum *) NULL) ||
                    (next == (Quantum *) NULL))
                  {
                    if (prev != (Quantum *) NULL)
                      prev=(Quantum *) RelinquishMagickMemory(prev);
                    if (next != (Quantum *) NULL)
                      next=(Quantum *) RelinquishMagickMemory(next);
                    image=DestroyImageList(image);
                    ThrowReaderException(ResourceLimitError,
                      "MemoryAllocationFailed");
                  }

                n=GetAuthenticPixels(image,0,0,image->columns,1,exception);
                (void) memcpy(next,n,length);

                for (y=0; y < (ssize_t) image->rows; y++)
                {
                  if (y == 0)
                    m=(ssize_t) mng_info->magn_mt;

                  else if (magn_methy > 1 && y == (ssize_t) image->rows-2)
                    m=(ssize_t) mng_info->magn_mb;

                  else if (magn_methy <= 1 && y == (ssize_t) image->rows-1)
                    m=(ssize_t) mng_info->magn_mb;

                  else if (magn_methy > 1 && y == (ssize_t) image->rows-1)
                    m=1;

                  else
                    m=(ssize_t) mng_info->magn_my;

                  n=prev;
                  prev=next;
                  next=n;

                  if (y < (ssize_t) image->rows-1)
                    {
                      n=GetAuthenticPixels(image,0,y+1,image->columns,1,
                          exception);
                      (void) memcpy(next,n,length);
                    }

                  for (i=0; i < m; i++, yy++)
                  {
                    Quantum
                      *pixels;

                    assert(yy < (ssize_t) large_image->rows);
                    pixels=prev;
                    n=next;
                    q=GetAuthenticPixels(large_image,0,yy,large_image->columns,
                      1,exception);
                    if (q == (Quantum *) NULL)
                      break;
                    q+=(ptrdiff_t) (large_image->columns-image->columns)*
                      GetPixelChannels(large_image);

                    for (x=(ssize_t) image->columns-1; x >= 0; x--)
                    {
                      /* To do: get color as function of indexes[x] */
                      /*
                      if (image->storage_class == PseudoClass)
                        {
                        }
                      */

                      if (magn_methy <= 1)
                        {
                          /* replicate previous */
                          SetPixelRed(large_image,GetPixelRed(image,pixels),q);
                          SetPixelGreen(large_image,GetPixelGreen(image,
                             pixels),q);
                          SetPixelBlue(large_image,GetPixelBlue(image,
                             pixels),q);
                          SetPixelAlpha(large_image,GetPixelAlpha(image,
                             pixels),q);
                        }

                      else if (magn_methy == 2 || magn_methy == 4)
                        {
                          if (i == 0)
                            {
                              SetPixelRed(large_image,GetPixelRed(image,
                                 pixels),q);
                              SetPixelGreen(large_image,GetPixelGreen(image,
                                 pixels),q);
                              SetPixelBlue(large_image,GetPixelBlue(image,
                                 pixels),q);
                              SetPixelAlpha(large_image,GetPixelAlpha(image,
                                 pixels),q);
                            }

                          else
                            {
                              /* Interpolate */
                              SetPixelRed(large_image,((QM) (((ssize_t)
                                 (2*i*(GetPixelRed(image,n)
                                 -GetPixelRed(image,pixels)+m))/
                                 ((ssize_t) (m*2))
                                 +GetPixelRed(image,pixels)))),q);
                              SetPixelGreen(large_image,((QM) (((ssize_t)
                                 (2*i*(GetPixelGreen(image,n)
                                 -GetPixelGreen(image,pixels)+m))/
                                 ((ssize_t) (m*2))
                                 +GetPixelGreen(image,pixels)))),q);
                              SetPixelBlue(large_image,((QM) (((ssize_t)
                                 (2*i*(GetPixelBlue(image,n)
                                 -GetPixelBlue(image,pixels)+m))/
                                 ((ssize_t) (m*2))
                                 +GetPixelBlue(image,pixels)))),q);

                              if (image->alpha_trait != UndefinedPixelTrait)
                                 SetPixelAlpha(large_image, ((QM) (((ssize_t)
                                    (2*i*(GetPixelAlpha(image,n)
                                    -GetPixelAlpha(image,pixels)+m))
                                    /((ssize_t) (m*2))+
                                   GetPixelAlpha(image,pixels)))),q);
                            }

                          if (magn_methy == 4)
                            {
                              /* Replicate nearest */
                              if (i <= ((m+1) << 1))
                                 SetPixelAlpha(large_image,GetPixelAlpha(image,
                                    pixels),q);
                              else
                                 SetPixelAlpha(large_image,GetPixelAlpha(image,
                                    n),q);
                            }
                        }

                      else /* if (magn_methy == 3 || magn_methy == 5) */
                        {
                          /* Replicate nearest */
                          if (i <= ((m+1) << 1))
                          {
                             SetPixelRed(large_image,GetPixelRed(image,
                                    pixels),q);
                             SetPixelGreen(large_image,GetPixelGreen(image,
                                    pixels),q);
                             SetPixelBlue(large_image,GetPixelBlue(image,
                                    pixels),q);
                             SetPixelAlpha(large_image,GetPixelAlpha(image,
                                    pixels),q);
                          }

                          else
                          {
                             SetPixelRed(large_image,GetPixelRed(image,n),q);
                             SetPixelGreen(large_image,GetPixelGreen(image,n),
                                    q);
                             SetPixelBlue(large_image,GetPixelBlue(image,n),
                                    q);
                             SetPixelAlpha(large_image,GetPixelAlpha(image,n),
                                    q);
                          }

                          if (magn_methy == 5)
                            {
                              SetPixelAlpha(large_image,(QM) (((ssize_t) (2*i*
                                 (GetPixelAlpha(image,n)
                                 -GetPixelAlpha(image,pixels))
                                 +m))/((ssize_t) (m*2))
                                 +GetPixelAlpha(image,pixels)),q);
                            }
                        }
                      n+=(ptrdiff_t) GetPixelChannels(image);
                      q+=(ptrdiff_t) GetPixelChannels(large_image);
                      pixels+=(ptrdiff_t) GetPixelChannels(image);
                    } /* x */

                    if (SyncAuthenticPixels(large_image,exception) == 0)
                      break;

                  } /* i */
                } /* y */

                prev=(Quantum *) RelinquishMagickMemory(prev);
                next=(Quantum *) RelinquishMagickMemory(next);

                length=image->columns;

                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "    Delete original image");

                DeleteImageFromList(&image);

                image=large_image;

                mng_info->image=image;

                /* magnify the columns */
                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "    Magnify the columns to %.20g",
                    (double) image->columns);

                for (y=0; y < (ssize_t) image->rows; y++)
                {
                  Quantum
                    *pixels;

                  q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
                  if (q == (Quantum *) NULL)
                    break;
                  pixels=q+(image->columns-length)*GetPixelChannels(image);
                  n=pixels+GetPixelChannels(image);

                  for (x=(ssize_t) (image->columns-length);
                    x < (ssize_t) image->columns; x++)
                  {
                    /* To do: Rewrite using Get/Set***PixelChannel() */

                    if (x == (ssize_t) (image->columns-length))
                      m=(ssize_t) mng_info->magn_ml;

                    else if (magn_methx > 1 && x == (ssize_t) image->columns-2)
                      m=(ssize_t) mng_info->magn_mr;

                    else if (magn_methx <= 1 &&
                        x == (ssize_t) image->columns-1)
                      m=(ssize_t) mng_info->magn_mr;

                    else if (magn_methx > 1 && x == (ssize_t) image->columns-1)
                      m=1;

                    else
                      m=(ssize_t) mng_info->magn_mx;

                    for (i=0; i < m; i++)
                    {
                      if (magn_methx <= 1)
                        {
                          /* replicate previous */
                          SetPixelRed(image,GetPixelRed(image,pixels),q);
                          SetPixelGreen(image,GetPixelGreen(image,pixels),q);
                          SetPixelBlue(image,GetPixelBlue(image,pixels),q);
                          SetPixelAlpha(image,GetPixelAlpha(image,pixels),q);
                        }

                      else if (magn_methx == 2 || magn_methx == 4)
                        {
                          if (i == 0)
                          {
                            SetPixelRed(image,GetPixelRed(image,pixels),q);
                            SetPixelGreen(image,GetPixelGreen(image,pixels),q);
                            SetPixelBlue(image,GetPixelBlue(image,pixels),q);
                            SetPixelAlpha(image,GetPixelAlpha(image,pixels),q);
                          }

                          /* To do: Rewrite using Get/Set***PixelChannel() */
                          else
                            {
                              /* Interpolate */
                              SetPixelRed(image,(QM) ((2*i*(
                                 GetPixelRed(image,n)
                                 -GetPixelRed(image,pixels))+m)
                                 /((ssize_t) (m*2))+
                                 GetPixelRed(image,pixels)),q);

                              SetPixelGreen(image,(QM) ((2*i*(
                                 GetPixelGreen(image,n)
                                 -GetPixelGreen(image,pixels))+m)
                                 /((ssize_t) (m*2))+
                                 GetPixelGreen(image,pixels)),q);

                              SetPixelBlue(image,(QM) ((2*i*(
                                 GetPixelBlue(image,n)
                                 -GetPixelBlue(image,pixels))+m)
                                 /((ssize_t) (m*2))+
                                 GetPixelBlue(image,pixels)),q);
                              if (image->alpha_trait != UndefinedPixelTrait)
                                 SetPixelAlpha(image,(QM) ((2*i*(
                                   GetPixelAlpha(image,n)
                                   -GetPixelAlpha(image,pixels))+m)
                                   /((ssize_t) (m*2))+
                                   GetPixelAlpha(image,pixels)),q);
                            }

                          if (magn_methx == 4)
                            {
                              /* Replicate nearest */
                              if (i <= ((m+1) << 1))
                              {
                                 SetPixelAlpha(image,
                                   GetPixelAlpha(image,pixels)+0,q);
                              }
                              else
                              {
                                 SetPixelAlpha(image,
                                   GetPixelAlpha(image,n)+0,q);
                              }
                            }
                        }

                      else /* if (magn_methx == 3 || magn_methx == 5) */
                        {
                          /* Replicate nearest */
                          if (i <= ((m+1) << 1))
                          {
                             SetPixelRed(image,GetPixelRed(image,pixels),q);
                             SetPixelGreen(image,GetPixelGreen(image,
                                 pixels),q);
                             SetPixelBlue(image,GetPixelBlue(image,pixels),q);
                             SetPixelAlpha(image,GetPixelAlpha(image,
                                 pixels),q);
                          }

                          else
                          {
                             SetPixelRed(image,GetPixelRed(image,n),q);
                             SetPixelGreen(image,GetPixelGreen(image,n),q);
                             SetPixelBlue(image,GetPixelBlue(image,n),q);
                             SetPixelAlpha(image,GetPixelAlpha(image,n),q);
                          }

                          if (magn_methx == 5)
                            {
                              /* Interpolate */
                              SetPixelAlpha(image,
                                 (QM) ((2*i*( GetPixelAlpha(image,n)
                                 -GetPixelAlpha(image,pixels))+m)/
                                 ((ssize_t) (m*2))
                                 +GetPixelAlpha(image,pixels)),q);
                            }
                        }
                      q+=(ptrdiff_t) GetPixelChannels(image);
                    }
                    n+=(ptrdiff_t) GetPixelChannels(image);
                  }

                  if (SyncAuthenticPixels(image,exception) == MagickFalse)
                    break;
                }
#if (MAGICKCORE_QUANTUM_DEPTH > 16)
              if (magn_methx != 1 || magn_methy != 1)
                {
                /*
                   Rescale pixels to Quantum
                */
                   for (y=0; y < (ssize_t) image->rows; y++)
                   {
                     q=GetAuthenticPixels(image,0,y,image->columns,1,
                       exception);
                     if (q == (Quantum *) NULL)
                       break;

                     for (x=(ssize_t) image->columns-1; x >= 0; x--)
                     {
                        SetPixelRed(image,ScaleShortToQuantum(
                          (unsigned short) GetPixelRed(image,q)),q);
                        SetPixelGreen(image,ScaleShortToQuantum(
                          (unsigned short) GetPixelGreen(image,q)),q);
                        SetPixelBlue(image,ScaleShortToQuantum(
                          (unsigned short) GetPixelBlue(image,q)),q);
                        SetPixelAlpha(image,ScaleShortToQuantum(
                          (unsigned short) GetPixelAlpha(image,q)),q);
                        q+=(ptrdiff_t) GetPixelChannels(image);
                     }

                     if (SyncAuthenticPixels(image,exception) == MagickFalse)
                       break;
                   }
                }
#endif
                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  Finished MAGN processing");
              }
          }

        /*
          Crop_box is with respect to the upper left corner of the MNG.
        */
        crop_box.left=mng_info->image_box.left+(long) mng_info->x_off[object_id];
        crop_box.right=mng_info->image_box.right+(long) mng_info->x_off[object_id];
        crop_box.top=mng_info->image_box.top+(long) mng_info->y_off[object_id];
        crop_box.bottom=mng_info->image_box.bottom+(long) mng_info->y_off[object_id];
        crop_box=mng_minimum_box(crop_box,mng_info->clip);
        crop_box=mng_minimum_box(crop_box,mng_info->frame);
        crop_box=mng_minimum_box(crop_box,mng_info->object_clip[object_id]);
        if ((crop_box.left != (mng_info->image_box.left
            +mng_info->x_off[object_id])) ||
            (crop_box.right != (mng_info->image_box.right
            +mng_info->x_off[object_id])) ||
            (crop_box.top != (mng_info->image_box.top
            +mng_info->y_off[object_id])) ||
            (crop_box.bottom != (mng_info->image_box.bottom
            +mng_info->y_off[object_id])))
          {
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  Crop the PNG image");

            if ((crop_box.left < crop_box.right) &&
                (crop_box.top < crop_box.bottom))
              {
                Image
                  *im;

                RectangleInfo
                  crop_info;

                /*
                  Crop_info is with respect to the upper left corner of
                  the image.
                */
                crop_info.x=(crop_box.left-mng_info->x_off[object_id]);
                crop_info.y=(crop_box.top-mng_info->y_off[object_id]);
                crop_info.width=(size_t) (crop_box.right-crop_box.left);
                crop_info.height=(size_t) (crop_box.bottom-crop_box.top);
                image->page.width=image->columns;
                image->page.height=image->rows;
                image->page.x=0;
                image->page.y=0;
                im=CropImage(image,&crop_info,exception);

                if (im != (Image *) NULL)
                  {
                    image->columns=im->columns;
                    image->rows=im->rows;
                    im=DestroyImage(im);
                    image->page.width=image->columns;
                    image->page.height=image->rows;
                    image->page.x=crop_box.left;
                    image->page.y=crop_box.top;
                  }
              }

            else
              {
                /*
                  No pixels in crop area.  The MNG spec still requires
                  a layer, though, so make a single transparent pixel in
                  the top left corner.
                */
                image->columns=1;
                image->rows=1;
                image->colors=2;
                (void) SetImageBackgroundColor(image,exception);
                image->page.width=1;
                image->page.height=1;
                image->page.x=0;
                image->page.y=0;
              }
          }
      }

#if (MAGICKCORE_QUANTUM_DEPTH > 16)
      /* PNG does not handle depths greater than 16 so reduce it even
       * if lossy.
       */
      if (image->depth > 16)
         image->depth=16;
#endif

#if (MAGICKCORE_QUANTUM_DEPTH > 8)
      if (image->depth > 8)
        {
          /* To do: fill low byte properly */
          image->depth=16;
        }

#endif

      if (image_info->number_scenes != 0)
        {
          if (mng_info->scenes_found >
             (ssize_t) (image_info->scene+image_info->number_scenes))
            break;
        }

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  Finished reading image datastream.");

  } while (LocaleCompare(image_info->magick,"MNG") == 0);

  (void) CloseBlob(image);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Finished reading all image datastreams.");

  if (insert_layers && !mng_info->image_found && (mng_info->mng_width) &&
       (mng_info->mng_height))
    {
      /*
        Insert a background layer if nothing else was found.
      */
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  No images found.  Inserting a background layer.");

      if (GetAuthenticPixelQueue(image) != (Quantum *) NULL)
        {
          /*
            Allocate next image structure.
          */
          AcquireNextImage(image_info,image,exception);
          if (GetNextImageInList(image) == (Image *) NULL)
            {
              if (logging != MagickFalse)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Allocation failed, returning NULL.");

              return(DestroyImageList(image));;
            }
          image=SyncNextImageInList(image);
        }
      image->columns=mng_info->mng_width;
      image->rows=mng_info->mng_height;
      image->page.width=mng_info->mng_width;
      image->page.height=mng_info->mng_height;
      image->page.x=0;
      image->page.y=0;
      image->background_color=mng_background_color;
      image->alpha_trait=UndefinedPixelTrait;

      if (image_info->ping == MagickFalse)
        (void) SetImageBackgroundColor(image,exception);

      mng_info->image_found++;
    }
  image->iterations=mng_iterations;

  if (mng_iterations == 1)
    image->start_loop=MagickTrue;

  while (GetPreviousImageInList(image) != (Image *) NULL)
  {
    image_count++;
    if (image_count > 10*mng_info->image_found)
      {
        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  No beginning");

        (void) ThrowMagickException(exception,GetMagickModule(),
          CoderError,"Linked list is corrupted, beginning of list not found",
          "`%s'",image_info->filename);

        return(DestroyImageList(image));
      }

    image=GetPreviousImageInList(image);

    if (GetNextImageInList(image) == (Image *) NULL)
      {
        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  Corrupt list");

        (void) ThrowMagickException(exception,GetMagickModule(),
          CoderError,"Linked list is corrupted; next_image is NULL","`%s'",
          image_info->filename);
      }
  }

  if (mng_info->ticks_per_second && mng_info->image_found > 1 &&
             GetNextImageInList(image) ==
     (Image *) NULL)
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  First image null");

      (void) ThrowMagickException(exception,GetMagickModule(),
        CoderError,"image->next for first image is NULL but shouldn't be.",
        "`%s'",image_info->filename);
    }

  if (mng_info->image_found == 0)
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  No visible images found.");

      (void) ThrowMagickException(exception,GetMagickModule(),
        CoderError,"No visible images in file","`%s'",image_info->filename);

      return(DestroyImageList(image));
    }

  if (mng_info->ticks_per_second)
    final_delay=(size_t) MagickMax(image->ticks_per_second,1L)*
            final_delay/mng_info->ticks_per_second;

  else
    image->start_loop=MagickTrue;

  /* Find final nonzero image delay */
  final_image_delay=0;

  while (GetNextImageInList(image) != (Image *) NULL)
    {
      if (image->delay)
        final_image_delay=image->delay;

      image=GetNextImageInList(image);
    }

  if (final_delay < final_image_delay)
    final_delay=final_image_delay;

  image->delay=final_delay;

  if (logging != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  image->delay=%.20g, final_delay=%.20g",(double) image->delay,
        (double) final_delay);

  if (logging != MagickFalse)
    {
      int
        scene;

      scene=0;
      image=GetFirstImageInList(image);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Before coalesce:");

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    scene 0 delay=%.20g",(double) image->delay);

      while (GetNextImageInList(image) != (Image *) NULL)
      {
        image=GetNextImageInList(image);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    scene %.20g delay=%.20g",(double) scene++,
          (double) image->delay);
      }
    }

  image=GetFirstImageInList(image);
  if (insert_layers && image->next)
    {
      Image
        *next_image,
        *next;

      size_t
        scene;

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  Coalesce Images");

      scene=image->scene;
      next_image=CoalesceImages(image,exception);
      image=DestroyImageList(image);
      if (next_image == (Image *) NULL)
        return((Image *) NULL);
      image=next_image;

      for (next=image; next != (Image *) NULL; next=next_image)
      {
         next->page.width=mng_info->mng_width;
         next->page.height=mng_info->mng_height;
         next->page.x=0;
         next->page.y=0;
         next->scene=scene++;
         next_image=GetNextImageInList(next);

         if (next_image == (Image *) NULL)
           break;

         if (next->delay == 0)
           {
             scene--;
             next_image->previous=GetPreviousImageInList(next);
             if (GetPreviousImageInList(next) == (Image *) NULL)
               image=next_image;
             else
               next->previous->next=next_image;
             next=DestroyImage(next);
           }
      }
    }

  while (GetNextImageInList(image) != (Image *) NULL)
      image=GetNextImageInList(image);

  image->dispose=BackgroundDispose;

  if (logging != MagickFalse)
    {
      int
        scene;

      scene=0;
      image=GetFirstImageInList(image);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  After coalesce:");

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    scene 0 delay=%.20g dispose=%.20g",(double) image->delay,
        (double) image->dispose);

      while (GetNextImageInList(image) != (Image *) NULL)
      {
        image=GetNextImageInList(image);

        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    scene %.20g delay=%.20g dispose=%.20g",(double) scene++,
          (double) image->delay,(double) image->dispose);
      }
   }

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  exit ReadOneMNGImage();");

  return(image);
}

static Image *ReadMNGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    logging = MagickFalse,
    status;

  MngReadInfo
    *mng_info;

  /* Open image file.  */

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
       image_info->filename);
  image=AcquireImage(image_info,exception);
  if (image->debug != MagickFalse)
    logging=LogMagickEvent(CoderEvent,GetMagickModule(),"Enter ReadMNGImage()");
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);

  if (status == MagickFalse)
    return(DestroyImageList(image));

  mng_info=(MngReadInfo *) AcquireMagickMemory(sizeof(*mng_info));

  if (mng_info == (MngReadInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");

  (void) memset(mng_info,0,sizeof(*mng_info));
  mng_info->image=image;
  image=ReadOneMNGImage(mng_info,image_info,exception);
  mng_info=MngReadInfoFreeStruct(mng_info);

  if (image == (Image *) NULL)
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "exit ReadMNGImage() with error");

      return((Image *) NULL);
    }
  (void) CloseBlob(image);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit ReadMNGImage()");

  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P N G I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPNGImage() adds properties for the PNG image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPNGImage method is:
%
%      size_t RegisterPNGImage(void)
%
*/
ModuleExport size_t RegisterPNGImage(void)
{
  char
    version[MagickPathExtent];

  MagickInfo
    *entry;

  static const char
    PNGNote[] =
    {
      "See http://www.libpng.org/ for details about the PNG format."
    },

    JNGNote[] =
    {
      "See http://www.libpng.org/pub/mng/ for details about the JNG\n"
      "format."
    },

    MNGNote[] =
    {
      "See http://www.libpng.org/pub/mng/ for details about the MNG\n"
      "format."
    };

  *version='\0';

#if defined(PNG_LIBPNG_VER_STRING)
  (void) ConcatenateMagickString(version,"libpng ",MagickPathExtent);
  (void) ConcatenateMagickString(version,PNG_LIBPNG_VER_STRING,
   MagickPathExtent);

  if (LocaleCompare(PNG_LIBPNG_VER_STRING,png_get_header_ver(NULL)) != 0)
    {
      (void) ConcatenateMagickString(version,",",MagickPathExtent);
      (void) ConcatenateMagickString(version,png_get_libpng_ver(NULL),
            MagickPathExtent);
    }
#endif

  entry=AcquireMagickInfo("PNG","MNG","Multiple-image Network Graphics");
  entry->flags|=CoderDecoderSeekableStreamFlag;

#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadMNGImage;
  entry->encoder=(EncodeImageHandler *) WriteMNGImage;
#endif

  entry->magick=(IsImageFormatHandler *) IsMNG;

  if (*version != '\0')
    entry->version=ConstantString(version);

  entry->mime_type=ConstantString("video/x-mng");
  entry->note=ConstantString(MNGNote);
  (void) RegisterMagickInfo(entry);

  entry=AcquireMagickInfo("PNG","PNG","Portable Network Graphics");

#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPNGImage;
  entry->encoder=(EncodeImageHandler *) WritePNGImage;
#endif

  entry->magick=(IsImageFormatHandler *) IsPNG;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->mime_type=ConstantString("image/png");

  if (*version != '\0')
    entry->version=ConstantString(version);

  entry->note=ConstantString(PNGNote);
  (void) RegisterMagickInfo(entry);

  entry=AcquireMagickInfo("PNG","PNG8",
    "8-bit indexed with optional binary transparency");

#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPNGImage;
  entry->encoder=(EncodeImageHandler *) WritePNGImage;
#endif

  entry->magick=(IsImageFormatHandler *) IsPNG;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->mime_type=ConstantString("image/png");
  (void) RegisterMagickInfo(entry);

  entry=AcquireMagickInfo("PNG","PNG24",
    "opaque or binary transparent 24-bit RGB");
  *version='\0';

  if (*version != '\0')
    entry->version=ConstantString(version);

#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPNGImage;
  entry->encoder=(EncodeImageHandler *) WritePNGImage;
#endif

  entry->magick=(IsImageFormatHandler *) IsPNG;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->mime_type=ConstantString("image/png");
  (void) RegisterMagickInfo(entry);

  entry=AcquireMagickInfo("PNG","PNG32","opaque or transparent 32-bit RGBA");

#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPNGImage;
  entry->encoder=(EncodeImageHandler *) WritePNGImage;
#endif

  entry->magick=(IsImageFormatHandler *) IsPNG;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->mime_type=ConstantString("image/png");
  (void) RegisterMagickInfo(entry);

  entry=AcquireMagickInfo("PNG","PNG48",
    "opaque or binary transparent 48-bit RGB");

#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPNGImage;
  entry->encoder=(EncodeImageHandler *) WritePNGImage;
#endif

  entry->magick=(IsImageFormatHandler *) IsPNG;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->mime_type=ConstantString("image/png");
  (void) RegisterMagickInfo(entry);

  entry=AcquireMagickInfo("PNG","PNG64","opaque or transparent 64-bit RGBA");

#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPNGImage;
  entry->encoder=(EncodeImageHandler *) WritePNGImage;
#endif

  entry->magick=(IsImageFormatHandler *) IsPNG;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->mime_type=ConstantString("image/png");
  (void) RegisterMagickInfo(entry);

  entry=AcquireMagickInfo("PNG","PNG00",
    "PNG inheriting bit-depth, color-type from original, if possible");

#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPNGImage;
  entry->encoder=(EncodeImageHandler *) WritePNGImage;
#endif

  entry->magick=(IsImageFormatHandler *) IsPNG;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->mime_type=ConstantString("image/png");
  (void) RegisterMagickInfo(entry);

  entry=AcquireMagickInfo("PNG","JNG","JPEG Network Graphics");

#if defined(JNG_SUPPORTED)
#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJNGImage;
  entry->encoder=(EncodeImageHandler *) WriteJNGImage;
#endif
#endif

  entry->magick=(IsImageFormatHandler *) IsJNG;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->mime_type=ConstantString("image/x-jng");
  entry->note=ConstantString(JNGNote);
  (void) RegisterMagickInfo(entry);

#ifdef IMPNG_SETJMP_NOT_THREAD_SAFE
  ping_semaphore=AcquireSemaphoreInfo();
#endif

  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P N G I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPNGImage() removes format registrations made by the
%  PNG module from the list of supported formats.
%
%  The format of the UnregisterPNGImage method is:
%
%      UnregisterPNGImage(void)
%
*/
ModuleExport void UnregisterPNGImage(void)
{
  (void) UnregisterMagickInfo("MNG");
  (void) UnregisterMagickInfo("PNG");
  (void) UnregisterMagickInfo("PNG8");
  (void) UnregisterMagickInfo("PNG24");
  (void) UnregisterMagickInfo("PNG32");
  (void) UnregisterMagickInfo("PNG48");
  (void) UnregisterMagickInfo("PNG64");
  (void) UnregisterMagickInfo("PNG00");
  (void) UnregisterMagickInfo("JNG");

#ifdef IMPNG_SETJMP_NOT_THREAD_SAFE
  if (ping_semaphore != (SemaphoreInfo *) NULL)
    RelinquishSemaphoreInfo(&ping_semaphore);
#endif
}

#if defined(MAGICKCORE_PNG_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M N G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteMNGImage() writes an image in the Portable Network Graphics
%  Group's "Multiple-image Network Graphics" encoded image format.
%
%  MNG support written by Glenn Randers-Pehrson, glennrp@image...
%
%  The format of the WriteMNGImage method is:
%
%      MagickBooleanType WriteMNGImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
%  To do (as of version 5.5.2, November 26, 2002 -- glennrp -- see also
%    "To do" under ReadPNGImage):
%
%    Preserve all unknown and not-yet-handled known chunks found in input
%    PNG file and copy them  into output PNG files according to the PNG
%    copying rules.
%
%    Write the iCCP chunk at MNG level when (icc profile length > 0)
%
%    Improve selection of color type (use indexed-color or indexed-color
%    with tRNS when 256 or fewer unique RGBA values are present).
%
%    Figure out what to do with "dispose=<restore-to-previous>" (dispose == 3)
%    This will be complicated if we limit ourselves to generating MNG-LC
%    files.  For now we ignore disposal method 3 and simply overlay the next
%    image on it.
%
%    Check for identical PLTE's or PLTE/tRNS combinations and use a
%    global MNG PLTE or PLTE/tRNS combination when appropriate.
%    [mostly done 15 June 1999 but still need to take care of tRNS]
%
%    Check for identical sRGB and replace with a global sRGB (and remove
%    gAMA/cHRM if sRGB is found; check for identical gAMA/cHRM and
%    replace with global gAMA/cHRM (or with sRGB if appropriate; replace
%    local gAMA/cHRM with local sRGB if appropriate).
%
%    Check for identical sBIT chunks and write global ones.
%
%    Provide option to skip writing the signature tEXt chunks.
%
%    Use signatures to detect identical objects and reuse the first
%    instance of such objects instead of writing duplicate objects.
%
%    Use a smaller-than-32k value of compression window size when
%    appropriate.
%
%    Encode JNG datastreams.  Mostly done as of 5.5.2; need to write
%    ancillary text chunks and save profiles.
%
%    Provide an option to force LC files (to ensure exact framing rate)
%    instead of VLC.
%
%    Provide an option to force VLC files instead of LC, even when offsets
%    are present.  This will involve expanding the embedded images with a
%    transparent region at the top and/or left.
*/

static void
Magick_png_write_raw_profile(const ImageInfo *image_info,png_struct *ping,
  png_info *ping_info, unsigned char *profile_type, unsigned char
  *profile_description, unsigned char *profile_data, png_uint_32 length,
  ExceptionInfo *exception)
{
   png_charp
     dp;

   png_textp
     text;

   png_uint_32
     allocated_length,
     description_length;

   ssize_t
     i;

   unsigned char
     hex[16] =
       { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' },
     *sp;

   if (length > 1)
     {
       if (LocaleNCompare((char *) profile_type+1, "ng-chunk-",9) == 0)
          return;
     }
   if (image_info->verbose != MagickFalse)
     {
       (void) printf("writing raw profile: type=%s, length=%.20g\n",
         (char *) profile_type, (double) length);
     }
   description_length=(png_uint_32) strlen((const char *) profile_description);
   allocated_length=(png_uint_32) (2*length+(length >> 5)+description_length+
     20);
   if (allocated_length < length)
     {
       (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
         "maximum profile length exceeded","`%s'",image_info->filename);
       return;
     }
#if PNG_LIBPNG_VER >= 10400
   text=(png_textp) png_malloc(ping,(png_alloc_size_t) sizeof(png_text));
#else
   text=(png_textp) png_malloc(ping,(png_size_t) sizeof(png_text));
#endif
#if PNG_LIBPNG_VER >= 10400
   text[0].text=(png_charp) png_malloc(ping,(png_alloc_size_t)
     allocated_length);
   text[0].key=(png_charp) png_malloc(ping,(png_alloc_size_t) 80);
#else
   text[0].text=(png_charp) png_malloc(ping,(png_size_t) allocated_length);
   text[0].key=(png_charp) png_malloc(ping,(png_size_t) 80);
#endif
   text[0].key[0]='\0';
   (void) ConcatenateMagickString(text[0].key,"Raw profile type ",
     MagickPathExtent);
   (void) ConcatenateMagickString(text[0].key,(const char *) profile_type,62);
   sp=profile_data;
   dp=text[0].text;
   *dp++='\n';
   (void) CopyMagickString(dp,(const char *) profile_description,
     allocated_length);
   dp+=description_length;
   *dp++='\n';
   (void) FormatLocaleString(dp,allocated_length-
     (png_size_t) (dp-text[0].text),"%8lu",(unsigned long) length);
   dp+=8;

   for (i=0; i < (ssize_t) length; i++)
   {
     if (i%36 == 0)
       *dp++='\n';
     *(dp++)=(char) hex[((*sp >> 4) & 0x0f)];
     *(dp++)=(char) hex[((*sp++ ) & 0x0f)];
   }

   *dp++='\n';
   *dp='\0';
   text[0].text_length=(png_size_t) (dp-text[0].text);
   text[0].compression=image_info->compression == NoCompression ||
     (image_info->compression == UndefinedCompression &&
     text[0].text_length < 128) ? -1 : 0;

   if (text[0].text_length <= allocated_length)
     png_set_text(ping,ping_info,text,1);

   png_free(ping,text[0].text);
   png_free(ping,text[0].key);
   png_free(ping,text);
}

static inline MagickBooleanType IsColorEqual(const Image *image,
  const Quantum *p, const PixelInfo *q)
{
  MagickRealType
    blue,
    green,
    red;

  red=(MagickRealType) GetPixelRed(image,p);
  green=(MagickRealType) GetPixelGreen(image,p);
  blue=(MagickRealType) GetPixelBlue(image,p);
  if ((AbsolutePixelValue(red-q->red) < MagickEpsilon) &&
      (AbsolutePixelValue(green-q->green) < MagickEpsilon) &&
      (AbsolutePixelValue(blue-q->blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

#if defined(PNG_tIME_SUPPORTED)
static void write_tIME_chunk(Image *image,png_struct *ping,png_info *info,
  const char *timestamp,ExceptionInfo *exception)
{
  int
    ret;

  int
    day,
    hour,
    minute,
    month,
    second,
    year;

  int
    addhours=0,
    addminutes=0;

  png_time
    ptime;

  assert(timestamp != (const char *) NULL);
  LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Writing tIME chunk: timestamp property is %30s\n",timestamp);
  ret=MagickSscanf(timestamp,"%d-%d-%dT%d:%d:%d",&year,&month,&day,&hour,
      &minute, &second);
  addhours=0;
  addminutes=0;
  ret=MagickSscanf(timestamp,"%d-%d-%dT%d:%d:%d%d:%d",&year,&month,&day,&hour,
      &minute, &second, &addhours, &addminutes);
    LogMagickEvent(CoderEvent,GetMagickModule(),
      "   Date format specified for png:tIME=%s" ,timestamp);
    LogMagickEvent(CoderEvent,GetMagickModule(),
      "      ret=%d,y=%d, m=%d, d=%d, h=%d, m=%d, s=%d, ah=%d, as=%d",
      ret,year,month,day,hour,minute,second,addhours,addminutes);
  if (ret < 6)
  {
    LogMagickEvent(CoderEvent,GetMagickModule(),
      "      Invalid date, ret=%d",ret);
    (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
      "Invalid date format specified for png:tIME","`%s' (ret=%d)",
      image->filename,ret);
    return;
  }
  if (addhours < 0)
  {
    addhours+=24;
    addminutes=-addminutes;
    day--;
  }
  hour+=addhours;
  minute+=addminutes;
  if (day == 0)
  {
    month--;
    day=31;
    if(month == 2)
      day=28;
    else
    {
      if(month == 4 || month == 6 || month == 9 || month == 11)
        day=30;
      else
        day=31;
    }
  }
  if (month == 0)
  {
    month++;
    year--;
  }
  if (minute > 59)
  {
     hour++;
     minute-=60;
  }
  if (hour > 23)
  {
     day ++;
     hour -=24;
  }
  if (hour < 0)
  {
     day --;
     hour +=24;
  }
  /* To do: fix this for leap years */
  if (day > 31 || (month == 2 && day > 28) || ((month == 4 || month == 6 ||
      month == 9 || month == 11) && day > 30))
  {
     month++;
     day = 1;
  }
  if (month > 12)
  {
     year++;
     month=1;
  }

  ptime.year = (png_uint_16) year;
  ptime.month = (png_byte) month;
  ptime.day = (png_byte) day;
  ptime.hour = (png_byte) hour;
  ptime.minute = (png_byte) minute;
  ptime.second = (png_byte) second;
  png_convert_from_time_t(&ptime,GetMagickTime());
  LogMagickEvent(CoderEvent,GetMagickModule(),
      "      png_set_tIME: y=%d, m=%d, d=%d, h=%d, m=%d, s=%d, ah=%d, am=%d",
      ptime.year, ptime.month, ptime.day, ptime.hour, ptime.minute,
      ptime.second, addhours, addminutes);
  png_set_tIME(ping,info,&ptime);
}
#endif

static void Magick_png_set_text(png_struct *ping,png_info *ping_info,
  MngWriteInfo *mng_info, const ImageInfo *image_info,const char* key,
  const char *value)
{
#if PNG_LIBPNG_VER >= 10600
  const char
    *c;

  MagickBooleanType
    write_itxt=MagickFalse;
#endif

  int
    compresion_none=PNG_TEXT_COMPRESSION_NONE,
    compresion_zTXt=PNG_TEXT_COMPRESSION_zTXt;

  png_textp
    text;

  size_t
    length;

#if PNG_LIBPNG_VER >= 10600
  /*
    Check if the string contains non-Latin1 characters.
  */
  c=value;
  while(*c != '\0')
  {
      if (((const unsigned char) *c) > 127) {
        write_itxt=MagickTrue;
        compresion_none=PNG_ITXT_COMPRESSION_NONE;
        compresion_zTXt=PNG_ITXT_COMPRESSION_zTXt;
        break;
      }
      c++;
  }
#endif
#if PNG_LIBPNG_VER >= 10400
  text=(png_textp) png_malloc(ping,(png_alloc_size_t) sizeof(png_text));
#else
  text=(png_textp) png_malloc(ping,(png_size_t) sizeof(png_text));
#endif
  if (text == (png_textp) NULL)
    return;
  (void) memset(text,0,sizeof(png_text));
  text[0].key=(char *) key;
  text[0].text=(char *) value;
  length=strlen(value);
#if PNG_LIBPNG_VER >= 10600
  if (write_itxt != MagickFalse)
    text[0].itxt_length=length;
  else
#endif
    text[0].text_length=length;
  if (mng_info->exclude_tEXt != MagickFalse)
    text[0].compression=compresion_zTXt;
  else
    if (mng_info->exclude_zTXt != MagickFalse)
      text[0].compression=compresion_none;
    else
      text[0].compression=
        image_info->compression == NoCompression ||
        (image_info->compression == UndefinedCompression &&
        (length < 128)) ?  compresion_none : compresion_zTXt;
  png_set_text(ping,ping_info,text,1);
  png_free(ping,text);
}

/* Write one PNG image */
static MagickBooleanType WriteOnePNGImage(MngWriteInfo *mng_info,
  const ImageInfo *IMimage_info,Image *IMimage,ExceptionInfo *exception)
{
  Image
    *image;

  ImageInfo
    *image_info;

  char
    *name,
    s[2];

  const char
    *property,
    *value;

  const StringInfo
    *profile;

  int
    num_passes,
    pass,
    ping_wrote_caNv;

  png_byte
     ping_trans_alpha[257];

  png_color
     palette[257];

  png_color_16
    ping_background,
    ping_trans_color;

  png_info
    *ping_info;

  png_struct
    *ping;

  png_uint_32
    ping_height,
    ping_width;

  ssize_t
    y;

  MagickBooleanType
    image_matte,
    logging = MagickFalse,
    matte,

    ping_exclude_iCCP,
    ping_exclude_zCCP,

    ping_have_blob,
    ping_have_cheap_transparency,
    ping_have_color,
    ping_have_non_bw,
    ping_have_PLTE,
    ping_have_bKGD,
    ping_have_eXIf,
    ping_have_iCCP,
    ping_have_pHYs,
    ping_have_sRGB,
    ping_have_tRNS,

    ping_need_colortype_warning,

    status,
    tried_332,
    tried_333,
    tried_444;

  MemoryInfo
    *volatile pixel_info;

  QuantumInfo
    *quantum_info;

  PNGErrorInfo
    error_info;

  ssize_t
    i,
    x;

  unsigned char
    *ping_pixels;

  volatile int
    image_colors,
    ping_bit_depth,
    ping_color_type,
    ping_interlace_method,
    ping_compression_method,
    ping_filter_method,
    ping_num_trans;

  volatile size_t
    image_depth,
    old_bit_depth;

  size_t
    quality,
    rowbytes,
    save_image_depth;

  int
    j,
    number_colors,
    number_opaque,
    number_semitransparent,
    number_transparent,
    ping_pHYs_unit_type;

  png_uint_32
    ping_pHYs_x_resolution,
    ping_pHYs_y_resolution;

  logging=LogMagickEvent(CoderEvent,GetMagickModule(),
    "  Enter WriteOnePNGImage()");

  image = CloneImage(IMimage,0,0,MagickFalse,exception);
  if (image == (Image *) NULL)
    return(MagickFalse);
  image_info=(ImageInfo *) CloneImageInfo(IMimage_info);

  /* Initialize some stuff */
  ping_bit_depth=0,
  ping_color_type=0,
  ping_interlace_method=0,
  ping_compression_method=0,
  ping_filter_method=0,
  ping_num_trans = 0;

  ping_background.red = 0;
  ping_background.green = 0;
  ping_background.blue = 0;
  ping_background.gray = 0;
  ping_background.index = 0;

  ping_trans_color.red=0;
  ping_trans_color.green=0;
  ping_trans_color.blue=0;
  ping_trans_color.gray=0;

  ping_pHYs_unit_type = 0;
  ping_pHYs_x_resolution = 0;
  ping_pHYs_y_resolution = 0;

  ping_have_blob=MagickFalse;
  ping_have_cheap_transparency=MagickFalse;
  ping_have_color=MagickTrue;
  ping_have_non_bw=MagickTrue;
  ping_have_PLTE=MagickFalse;
  ping_have_bKGD=MagickFalse;
  ping_have_eXIf=MagickTrue;
  ping_have_iCCP=MagickFalse;
  ping_have_pHYs=MagickFalse;
  ping_have_sRGB=MagickFalse;
  ping_have_tRNS=MagickFalse;

  ping_exclude_iCCP=mng_info->exclude_iCCP;
  ping_exclude_zCCP=mng_info->exclude_zCCP;

  ping_need_colortype_warning = MagickFalse;

  /* Recognize the ICC sRGB profile and convert it to the sRGB chunk,
   * i.e., eliminate the ICC profile and set image->rendering_intent.
   * Note that this will not involve any changes to the actual pixels
   * but merely passes information to applications that read the resulting
   * PNG image.
   *
   * To do: recognize other variants of the sRGB profile, using the CRC to
   * verify all recognized variants including the 7 already known.
   *
   * Work around libpng16+ rejecting some "known invalid sRGB profiles".
   *
   * Use something other than image->rendering_intent to record the fact
   * that the sRGB profile was found.
   *
   * Record the ICC version (currently v2 or v4) of the incoming sRGB ICC
   * profile.  Record the Blackpoint Compensation, if any.
   */
   if (mng_info->exclude_sRGB == MagickFalse && mng_info->preserve_iCCP == MagickFalse)
   {
      ResetImageProfileIterator(image);
      for (name=GetNextImageProfile(image); name != (char *) NULL; )
      {
        profile=GetImageProfile(image,name);

        if (profile != (StringInfo *) NULL)
          {
            if ((LocaleCompare(name,"ICC") == 0) ||
                (LocaleCompare(name,"ICM") == 0))

             {
                 int
                   icheck,
                   got_crc=0;


                 png_uint_32
                   length,
                   profile_crc=0;

                 unsigned char
                   *data;

                 length=(png_uint_32) GetStringInfoLength(profile);

                 for (icheck=0; sRGB_info[icheck].len > 0; icheck++)
                 {
                   if (length == sRGB_info[icheck].len)
                   {
                     if (got_crc == 0)
                     {
                       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                         "    Got a %lu-byte ICC profile (potentially sRGB)",
                         (unsigned long) length);

                       data=GetStringInfoDatum(profile);
                       profile_crc=crc32(0,data,length);

                       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                           "      with crc=%8x",(unsigned int) profile_crc);
                       got_crc++;
                     }

                     if (profile_crc == sRGB_info[icheck].crc)
                     {
                        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "      It is sRGB with rendering intent = %s",
                        Magick_RenderingIntentString_from_PNG_RenderingIntent(
                             sRGB_info[icheck].intent));
                        if (image->rendering_intent==UndefinedIntent)
                        {
                          image->rendering_intent=
                          Magick_RenderingIntent_from_PNG_RenderingIntent(
                             sRGB_info[icheck].intent);
                        }
                        ping_exclude_iCCP = MagickTrue;
                        ping_exclude_zCCP = MagickTrue;
                        ping_have_sRGB = MagickTrue;
                        break;
                     }
                   }
                 }
                 if (sRGB_info[icheck].len == 0)
                    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "    Got %lu-byte ICC profile not recognized as sRGB",
                        (unsigned long) length);
              }
          }
        name=GetNextImageProfile(image);
      }
  }

  number_opaque = 0;
  number_semitransparent = 0;
  number_transparent = 0;

  if (logging != MagickFalse)
    {
      if (image->storage_class == UndefinedClass)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    image->storage_class=UndefinedClass");
      if (image->storage_class == DirectClass)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    image->storage_class=DirectClass");
      if (image->storage_class == PseudoClass)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    image->storage_class=PseudoClass");
      (void) LogMagickEvent(CoderEvent,GetMagickModule(), image->taint ?
          "    image->taint=MagickTrue":
          "    image->taint=MagickFalse");
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    image->gamma=%g", image->gamma);
    }

  if (image->storage_class == PseudoClass &&
     ((mng_info->write_png8 != MagickFalse) ||
      (mng_info->write_png24 != MagickFalse) ||
      (mng_info->write_png32 != MagickFalse) ||
      (mng_info->write_png48 != MagickFalse) ||
      (mng_info->write_png64 != MagickFalse) ||
      (mng_info->colortype == 3) || // PNG_COLOR_TYPE_RGB
      (mng_info->colortype == 7))) // PNG_COLOR_TYPE_RGB_ALPHA
    {
      (void) SyncImage(image,exception);
      image->storage_class = DirectClass;
    }

  if (mng_info->preserve_colormap == MagickFalse)
    {
      if ((image->storage_class != PseudoClass) &&
          (image->colormap != (PixelInfo *) NULL))
        {
          /* Free the bogus colormap; it can cause trouble later */
           if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Freeing bogus colormap");
           image->colormap=(PixelInfo *) RelinquishMagickMemory(
             image->colormap);
        }
    }

  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace,exception);

  /*
    Sometimes we get PseudoClass images whose RGB values don't match
    the colors in the colormap.  This code syncs the RGB values.
  */
  image->depth=GetImageQuantumDepth(image,MagickFalse);
  if (image->depth <= 8 && image->taint && image->storage_class == PseudoClass)
     (void) SyncImage(image,exception);

#if (MAGICKCORE_QUANTUM_DEPTH == 8)
  if (image->depth > 8)
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Reducing PNG bit depth to 8 since this is a Q8 build.");

      image->depth=8;
    }
#endif

  /* Respect the -depth option */
  if (image->depth < 4)
    {
       Quantum
         *r;

       if (image->depth > 2)
         {
           /* Scale to 4-bit */
           LBR04PacketRGBA(image->background_color);

           for (y=0; y < (ssize_t) image->rows; y++)
           {
             r=GetAuthenticPixels(image,0,y,image->columns,1,exception);

             if (r == (Quantum *) NULL)
               break;

             for (x=0; x < (ssize_t) image->columns; x++)
             {
                LBR04PixelRGBA(r);
                r+=(ptrdiff_t) GetPixelChannels(image);
             }

             if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
           }

           if (image->storage_class == PseudoClass && image->colormap != NULL)
           {
             for (i=0; i < (ssize_t) image->colors; i++)
             {
               LBR04PacketRGBA(image->colormap[i]);
             }
           }
         }
       else if (image->depth > 1)
         {
           /* Scale to 2-bit */
           LBR02PacketRGBA(image->background_color);

           for (y=0; y < (ssize_t) image->rows; y++)
           {
             r=GetAuthenticPixels(image,0,y,image->columns,1,exception);

             if (r == (Quantum *) NULL)
               break;

             for (x=0; x < (ssize_t) image->columns; x++)
             {
                LBR02PixelRGBA(r);
                r+=(ptrdiff_t) GetPixelChannels(image);
             }

             if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
           }

           if (image->storage_class == PseudoClass && image->colormap != NULL)
           {
             for (i=0; i < (ssize_t) image->colors; i++)
             {
               LBR02PacketRGBA(image->colormap[i]);
             }
           }
         }
       else
         {
           /* Scale to 1-bit */
           LBR01PacketRGBA(image->background_color);

           for (y=0; y < (ssize_t) image->rows; y++)
           {
             r=GetAuthenticPixels(image,0,y,image->columns,1,exception);

             if (r == (Quantum *) NULL)
               break;

             for (x=0; x < (ssize_t) image->columns; x++)
             {
                LBR01PixelRGBA(r);
                r+=(ptrdiff_t) GetPixelChannels(image);
             }

             if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
           }

           if (image->storage_class == PseudoClass && image->colormap != NULL)
           {
             for (i=0; i < (ssize_t) image->colors; i++)
             {
               LBR01PacketRGBA(image->colormap[i]);
             }
           }
         }
    }

  /* To do: set to next higher multiple of 8 */
  if (image->depth < 8)
     image->depth=8;

#if (MAGICKCORE_QUANTUM_DEPTH > 16)
  /* PNG does not handle depths greater than 16 so reduce it even
   * if lossy
   */
  if (image->depth > 8)
      image->depth=16;
#endif

#if (MAGICKCORE_QUANTUM_DEPTH > 8)
  if (image->depth > 8)
    {
      /* To do: fill low byte properly */
      image->depth=16;
    }

  if (image->depth == 16 && mng_info->depth != 16)
    if (mng_info->write_png8 != MagickFalse ||
        LosslessReduceDepthOK(image,exception) != MagickFalse)
      image->depth = 8;
#endif

  image_colors = (int) image->colors;
  number_opaque = (int) image->colors;
  number_transparent = 0;
  number_semitransparent = 0;
  if (IdentifyImageCoderGray(image,exception) != MagickFalse)
    ping_have_color=MagickFalse;

  if (mng_info->colortype &&
     (mng_info->colortype > 4 || (mng_info->depth >= 8 &&
     mng_info->colortype < 4 &&
     image->alpha_trait == UndefinedPixelTrait)))
  {
     /* Avoid the expensive BUILD_PALETTE operation if we're sure that we
      * are not going to need the result.
      */
     if (mng_info->colortype == 1 ||
        mng_info->colortype == 5)
       ping_have_color=MagickFalse;

     if (image->alpha_trait != UndefinedPixelTrait)
       {
         number_transparent = 2;
         number_semitransparent = 1;
       }
  }

  if (mng_info->colortype < 7)
  {
  /* BUILD_PALETTE
   *
   * Normally we run this just once, but in the case of writing PNG8
   * we reduce the transparency to binary and run again, then if there
   * are still too many colors we reduce to a simple 4-4-4-1, then 3-3-3-1
   * RGBA palette and run again, and then to a simple 3-3-2-1 RGBA
   * palette.  Then (To do) we take care of a final reduction that is only
   * needed if there are still 256 colors present and one of them has both
   * transparent and opaque instances.
   */

  tried_332 = MagickFalse;
  tried_333 = MagickFalse;
  tried_444 = MagickFalse;

  if (image->depth != GetImageDepth(image,exception))
    (void) SetImageDepth(image,image->depth,exception);
  for (j=0; j<6; j++)
  {
    /*
     * Sometimes we get DirectClass images that have 256 colors or fewer.
     * This code will build a colormap.
     *
     * Also, sometimes we get PseudoClass images with an out-of-date
     * colormap.  This code will replace the colormap with a new one.
     * Sometimes we get PseudoClass images that have more than 256 colors.
     * This code will delete the colormap and change the image to
     * DirectClass.
     *
     * If image->alpha_trait is MagickFalse, we ignore the alpha channel
     * even though it sometimes contains left-over non-opaque values.
     *
     * Also we gather some information (number of opaque, transparent,
     * and semitransparent pixels, and whether the image has any non-gray
     * pixels or only black-and-white pixels) that we might need later.
     *
     * Even if the user wants to force GrayAlpha or RGBA (colortype 4 or 6)
     * we need to check for bogus non-opaque values, at least.
     */

   int
     n;

   PixelInfo
     opaque[260],
     semitransparent[260],
     transparent[260];

   const Quantum
     *r;

   Quantum
     *q;

   if (logging != MagickFalse)
     (void) LogMagickEvent(CoderEvent,GetMagickModule(),
         "    Enter BUILD_PALETTE:");

   if (logging != MagickFalse)
     {
       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "      image->columns=%.20g",(double) image->columns);
       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "      image->rows=%.20g",(double) image->rows);
       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "      image->alpha_trait=%.20g",(double) image->alpha_trait);
       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "      image->depth=%.20g",(double) image->depth);

       if (image->storage_class == PseudoClass && image->colormap != NULL)
       {
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "      Original colormap:");
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "        i    (red,green,blue,alpha)");

         for (i=0; i < (ssize_t) MagickMin(image->colors,256); i++)
         {
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "        %d    (%d,%d,%d,%d)",
                    (int) i,
                    (int) image->colormap[i].red,
                    (int) image->colormap[i].green,
                    (int) image->colormap[i].blue,
                    (int) image->colormap[i].alpha);
         }

         for (i=(ssize_t) image->colors - 10; i < (ssize_t) image->colors; i++)
         {
           if (i > 255)
             {
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "        %d    (%d,%d,%d,%d)",
                    (int) i,
                    (int) image->colormap[i].red,
                    (int) image->colormap[i].green,
                    (int) image->colormap[i].blue,
                    (int) image->colormap[i].alpha);
             }
         }
       }

       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "      image->colors=%d",(int) image->colors);

       if (image->colors == 0)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "        (zero means unknown)");

       if (mng_info->preserve_colormap == MagickFalse)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "      Regenerate the colormap");
     }

     image_colors=0;
     number_opaque = 0;
     number_semitransparent = 0;
     number_transparent = 0;

     for (y=0; y < (ssize_t) image->rows; y++)
     {
       r=GetVirtualPixels(image,0,y,image->columns,1,exception);

       if (r == (const Quantum *) NULL)
         break;

       for (x=0; x < (ssize_t) image->columns; x++)
       {
           if ((((image->alpha_trait & BlendPixelTrait) == 0) &&
                ((image->alpha_trait & UpdatePixelTrait) == 0)) ||
               (GetPixelAlpha(image,r) == OpaqueAlpha))
             {
               if (number_opaque < 259)
                 {
                   if (number_opaque == 0)
                     {
                       GetPixelInfoPixel(image,r,opaque);
                       opaque[0].alpha=OpaqueAlpha;
                       number_opaque=1;
                     }

                   for (i=0; i< (ssize_t) number_opaque; i++)
                     {
                       if (IsColorEqual(image,r,opaque+i))
                         break;
                     }

                   if (i ==  (ssize_t) number_opaque && number_opaque < 259)
                     {
                       number_opaque++;
                       GetPixelInfoPixel(image,r,opaque+i);
                       opaque[i].alpha=OpaqueAlpha;
                     }
                 }
             }
           else if (GetPixelAlpha(image,r) == TransparentAlpha)
             {
               if (number_transparent < 259)
                 {
                   if (number_transparent == 0)
                     {
                       GetPixelInfoPixel(image,r,transparent);
                       ping_trans_color.red=(unsigned short)
                         GetPixelRed(image,r);
                       ping_trans_color.green=(unsigned short)
                         GetPixelGreen(image,r);
                       ping_trans_color.blue=(unsigned short)
                         GetPixelBlue(image,r);
                       ping_trans_color.gray=(unsigned short)
                         GetPixelGray(image,r);
                       number_transparent = 1;
                     }

                   for (i=0; i< (ssize_t) number_transparent; i++)
                     {
                       if (IsColorEqual(image,r,transparent+i))
                         break;
                     }

                   if (i ==  (ssize_t) number_transparent &&
                       number_transparent < 259)
                     {
                       number_transparent++;
                       GetPixelInfoPixel(image,r,transparent+i);
                     }
                 }
             }
           else
             {
               if (number_semitransparent < 259)
                 {
                   if (number_semitransparent == 0)
                     {
                       GetPixelInfoPixel(image,r,semitransparent);
                       number_semitransparent = 1;
                     }

                   for (i=0; i< (ssize_t) number_semitransparent; i++)
                     {
                       if (IsColorEqual(image,r,semitransparent+i)
                           && (double) GetPixelAlpha(image,r) ==
                           semitransparent[i].alpha)
                         break;
                     }

                   if (i ==  (ssize_t) number_semitransparent &&
                       number_semitransparent < 259)
                     {
                       number_semitransparent++;
                       GetPixelInfoPixel(image,r,semitransparent+i);
                     }
                 }
             }
           r+=(ptrdiff_t) GetPixelChannels(image);
        }
     }

     image_colors=number_opaque+number_transparent+number_semitransparent;

     if (mng_info->write_png8 == MagickFalse &&
         mng_info->exclude_bKGD == MagickFalse)
       {
         /* Add the background color to the palette, if it
          * isn't already there.
          */
          if (logging != MagickFalse)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "      Check colormap for background (%d,%d,%d)",
                  (int) image->background_color.red,
                  (int) image->background_color.green,
                  (int) image->background_color.blue);
            }
          if (number_opaque < 259)
            {
              for (i=0; i<number_opaque; i++)
              {
                 if (opaque[i].red == image->background_color.red &&
                     opaque[i].green == image->background_color.green &&
                     opaque[i].blue == image->background_color.blue)
                   break;
              }
              if ((i == number_opaque) && (image_colors < 256))
                {
                  ping_background.index=(png_byte) i;
                  if (image->storage_class == PseudoClass)
                    image->colormap[image->colors++]=image->background_color;
                  opaque[i]=image->background_color;
                  number_opaque++;
                  if (logging != MagickFalse)
                    {
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "      background_color index is %d",(int) i);
                    }
                }
            }
          else if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "      No room in the colormap to add background color");
       }

     image_colors=number_opaque+number_transparent+number_semitransparent;

     if (logging != MagickFalse)
       {
         if (image_colors > 256)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "      image has more than 256 colors");

         else
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "      image has %d colors",image_colors);
       }

     if (mng_info->preserve_colormap != MagickFalse)
       break;

     if (mng_info->colortype != 7) /* We won't need this info */
       {
         ping_have_color=MagickFalse;
         ping_have_non_bw=MagickFalse;

         if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
         {
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "incompatible colorspace");
           ping_have_color=MagickTrue;
           ping_have_non_bw=MagickTrue;
         }

         if (image_colors > 256)
           {
             for (y=0; y < (ssize_t) image->rows; y++)
             {
               q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

               if (q == (Quantum *) NULL)
                 break;

               r=q;
               for (x=0; x < (ssize_t) image->columns; x++)
               {
                 if (GetPixelRed(image,r) != GetPixelGreen(image,r) ||
                     GetPixelRed(image,r) != GetPixelBlue(image,r))
                   {
                      ping_have_color=MagickTrue;
                      ping_have_non_bw=MagickTrue;
                      break;
                   }
                 r+=(ptrdiff_t) GetPixelChannels(image);
               }

               if (ping_have_color != MagickFalse)
                 break;

               /* Worst case is black-and-white; we are looking at every
                * pixel twice.
                */

               if (ping_have_non_bw == MagickFalse)
                 {
                   r=q;
                   for (x=0; x < (ssize_t) image->columns; x++)
                   {
                     if (GetPixelRed(image,r) != 0 &&
                         GetPixelRed(image,r) != QuantumRange)
                       {
                         ping_have_non_bw=MagickTrue;
                         break;
                       }
                     r+=(ptrdiff_t) GetPixelChannels(image);
                   }
               }
             }
           }
       }

     if (image_colors < 257)
       {
         PixelInfo
           colormap[260];

         /*
          * Initialize image colormap.
          */

         if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "      Sort the new colormap");

        /* Sort palette, transparent first */;

         n = 0;

         for (i=0; i<number_transparent; i++)
            colormap[n++] = transparent[i];

         for (i=0; i<number_semitransparent; i++)
            colormap[n++] = semitransparent[i];

         for (i=0; i<number_opaque; i++)
            colormap[n++] = opaque[i];

         ping_background.index += (png_byte) (number_transparent +
           number_semitransparent);

         /* image_colors < 257; search the colormap instead of the pixels
          * to get ping_have_color and ping_have_non_bw
          */
         for (i=0; i<n; i++)
         {
           if (ping_have_color == MagickFalse)
             {
                if (colormap[i].red != colormap[i].green ||
                    colormap[i].red != colormap[i].blue)
                  {
                     ping_have_color=MagickTrue;
                     ping_have_non_bw=MagickTrue;
                     break;
                  }
              }

           if (ping_have_non_bw == MagickFalse)
             {
               if (colormap[i].red != 0 && colormap[i].red != (double) QuantumRange)
                   ping_have_non_bw=MagickTrue;
             }
          }

        if ((mng_info->exclude_tRNS == MagickFalse ||
            (number_transparent == 0 && number_semitransparent == 0)) &&
            (((mng_info->colortype-1) == PNG_COLOR_TYPE_PALETTE) ||
            (mng_info->colortype == 0)))
          {
            if (logging != MagickFalse)
              {
                if (n !=  (ssize_t) image_colors)
                   (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "   image_colors (%d) and n (%d)  don't match",
                   image_colors, n);

                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      AcquireImageColormap");
              }

            image->colors = (size_t) image_colors;

            if (AcquireImageColormap(image,(size_t) image_colors,exception) == MagickFalse)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed","`%s'",
                  image->filename);
                break;
              }

            for (i=0; i< (ssize_t) image_colors; i++)
               image->colormap[i] = colormap[i];

            if (logging != MagickFalse)
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                      "      image->colors=%d (%d)",
                      (int) image->colors, image_colors);

                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                      "      Update the pixel indexes");
              }

            /* Sync the pixel indices with the new colormap */

            for (y=0; y < (ssize_t) image->rows; y++)
            {
              q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

              if (q == (Quantum *) NULL)
                break;

              for (x=0; x < (ssize_t) image->columns; x++)
              {
                for (i=0; i< (ssize_t) image_colors; i++)
                {
                  if (((((image->alpha_trait & BlendPixelTrait) == 0) &&
                       (((image->alpha_trait & UpdatePixelTrait) == 0))) ||
                      image->colormap[i].alpha == (double) GetPixelAlpha(image,q)) &&
                      image->colormap[i].red == (double) GetPixelRed(image,q) &&
                      image->colormap[i].green == (double) GetPixelGreen(image,q) &&
                      image->colormap[i].blue == (double) GetPixelBlue(image,q))
                  {
                    SetPixelIndex(image,(Quantum) i,q);
                    break;
                  }
                }
                q+=(ptrdiff_t) GetPixelChannels(image);
              }

              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                 break;
            }
          }
       }

     if (logging != MagickFalse)
       {
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      image->colors=%d", (int) image->colors);

         if (image->colormap != NULL)
           {
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "       i     (red,green,blue,alpha)");

             for (i=0; i < (ssize_t) image->colors; i++)
             {
               if (i < 300 || i >= (ssize_t) image->colors - 10)
                 {
                   (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                       "       %d     (%d,%d,%d,%d)",
                        (int) i,
                        (int) image->colormap[i].red,
                        (int) image->colormap[i].green,
                        (int) image->colormap[i].blue,
                        (int) image->colormap[i].alpha);
                 }
             }
           }

           if (number_transparent < 257)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      number_transparent     = %d",
                   number_transparent);
           else

             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      number_transparent     > 256");

           if (number_opaque < 257)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      number_opaque          = %d",
                   number_opaque);

           else
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      number_opaque          > 256");

           if (number_semitransparent < 257)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      number_semitransparent = %d",
                   number_semitransparent);

           else
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      number_semitransparent > 256");

           if (ping_have_non_bw == MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "      All pixels and the background are black or white");

           else if (ping_have_color == MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "      All pixels and the background are gray");

           else
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "      At least one pixel or the background is non-gray");

           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "    Exit BUILD_PALETTE:");
       }

   if (mng_info->write_png8 == MagickFalse)
      break;

   /* Make any reductions necessary for the PNG8 format */
    if (image_colors <= 256 &&
        image_colors != 0 && image->colormap != NULL &&
        number_semitransparent == 0 &&
        number_transparent <= 1)
      break;

    /* PNG8 can't have semitransparent colors so we threshold the
     * opacity to 0 or OpaqueOpacity, and PNG8 can only have one
     * transparent color so if more than one is transparent we merge
     * them into image->background_color.
     */
    if (number_semitransparent != 0 || number_transparent > 1)
      {
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Thresholding the alpha channel to binary");

        for (y=0; y < (ssize_t) image->rows; y++)
        {
          q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

          if (q == (Quantum *) NULL)
            break;

          for (x=0; x < (ssize_t) image->columns; x++)
          {
              if (GetPixelAlpha(image,q) < OpaqueAlpha/2)
                {
                  SetPixelViaPixelInfo(image,&image->background_color,q);
                  SetPixelAlpha(image,TransparentAlpha,q);
                }
              else
                  SetPixelAlpha(image,OpaqueAlpha,q);
              q+=(ptrdiff_t) GetPixelChannels(image);
          }

          if (SyncAuthenticPixels(image,exception) == MagickFalse)
             break;

          if (image_colors != 0 && image_colors <= 256 &&
             image->colormap != NULL)
            for (i=0; i<image_colors; i++)
                image->colormap[i].alpha =
                    (image->colormap[i].alpha > (double) TransparentAlpha/2 ?
                    TransparentAlpha : OpaqueAlpha);
        }
      continue;
    }

    /* PNG8 can't have more than 256 colors so we quantize the pixels and
     * background color to the 4-4-4-1, 3-3-3-1 or 3-3-2-1 palette.  If the
     * image is mostly gray, the 4-4-4-1 palette is likely to end up with 256
     * colors or less.
     */
    if (tried_444 == MagickFalse && (image_colors == 0 || image_colors > 256))
      {
        if (logging != MagickFalse)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "    Quantizing the background color to 4-4-4");

        tried_444 = MagickTrue;

        LBR04PacketRGB(image->background_color);

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Quantizing the pixel colors to 4-4-4");

        if (image->colormap == NULL)
        {
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

            if (q == (Quantum *) NULL)
              break;

            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (GetPixelAlpha(image,q) == OpaqueAlpha)
                  LBR04PixelRGB(q);
              q+=(ptrdiff_t) GetPixelChannels(image);
            }

            if (SyncAuthenticPixels(image,exception) == MagickFalse)
               break;
          }
        }

        else /* Should not reach this; colormap already exists and
                must be <= 256 */
        {
          if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Quantizing the colormap to 4-4-4");

          for (i=0; i<image_colors; i++)
          {
            LBR04PacketRGB(image->colormap[i]);
          }
        }
        continue;
      }

    if (tried_333 == MagickFalse && (image_colors == 0 || image_colors > 256))
      {
        if (logging != MagickFalse)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "    Quantizing the background color to 3-3-3");

        tried_333 = MagickTrue;

        LBR03PacketRGB(image->background_color);

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Quantizing the pixel colors to 3-3-3-1");

        if (image->colormap == NULL)
        {
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

            if (q == (Quantum *) NULL)
              break;

            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (GetPixelAlpha(image,q) == OpaqueAlpha)
                  LBR03RGB(q);
              q+=(ptrdiff_t) GetPixelChannels(image);
            }

            if (SyncAuthenticPixels(image,exception) == MagickFalse)
               break;
          }
        }

        else /* Should not reach this; colormap already exists and
                must be <= 256 */
        {
          if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Quantizing the colormap to 3-3-3-1");
          for (i=0; i<image_colors; i++)
          {
              LBR03PacketRGB(image->colormap[i]);
          }
        }
        continue;
      }

    if (tried_332 == MagickFalse && (image_colors == 0 || image_colors > 256))
      {
        if (logging != MagickFalse)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "    Quantizing the background color to 3-3-2");

        tried_332 = MagickTrue;

        /* Red and green were already done so we only quantize the blue
         * channel
         */

        LBR02PacketBlue(image->background_color);

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Quantizing the pixel colors to 3-3-2-1");

        if (image->colormap == NULL)
        {
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

            if (q == (Quantum *) NULL)
              break;

            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (GetPixelAlpha(image,q) == OpaqueAlpha)
                  LBR02PixelBlue(q);
              q+=(ptrdiff_t) GetPixelChannels(image);
            }

            if (SyncAuthenticPixels(image,exception) == MagickFalse)
               break;
          }
        }

        else /* Should not reach this; colormap already exists and
                must be <= 256 */
        {
          if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Quantizing the colormap to 3-3-2-1");
          for (i=0; i<image_colors; i++)
          {
              LBR02PacketBlue(image->colormap[i]);
          }
      }
      continue;
    }

    if (image_colors == 0 || image_colors > 256)
    {
      /* Take care of special case with 256 opaque colors + 1 transparent
       * color.  We don't need to quantize to 2-3-2-1; we only need to
       * eliminate one color, so we'll merge the two darkest red
       * colors (0x49, 0, 0) -> (0x24, 0, 0).
       */
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Merging two dark red background colors to 3-3-2-1");

      if (ScaleQuantumToChar((Quantum) image->background_color.red) == 0x49 &&
          ScaleQuantumToChar((Quantum) image->background_color.green) == 0x00 &&
          ScaleQuantumToChar((Quantum) image->background_color.blue) == 0x00)
      {
         image->background_color.red=ScaleCharToQuantum(0x24);
      }

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Merging two dark red pixel colors to 3-3-2-1");

      if (image->colormap == NULL)
      {
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

          if (q == (Quantum *) NULL)
            break;

          for (x=0; x < (ssize_t) image->columns; x++)
          {
            if (ScaleQuantumToChar(GetPixelRed(image,q)) == 0x49 &&
                ScaleQuantumToChar(GetPixelGreen(image,q)) == 0x00 &&
                ScaleQuantumToChar(GetPixelBlue(image,q)) == 0x00 &&
                GetPixelAlpha(image,q) == OpaqueAlpha)
              {
                SetPixelRed(image,ScaleCharToQuantum(0x24),q);
              }
            q+=(ptrdiff_t) GetPixelChannels(image);
          }

          if (SyncAuthenticPixels(image,exception) == MagickFalse)
             break;

        }
      }

      else
      {
         for (i=0; i<image_colors; i++)
         {
            if (ScaleQuantumToChar((Quantum) image->colormap[i].red) == 0x49 &&
                ScaleQuantumToChar((Quantum) image->colormap[i].green) == 0x00 &&
                ScaleQuantumToChar((Quantum) image->colormap[i].blue) == 0x00)
            {
               image->colormap[i].red=ScaleCharToQuantum(0x24);
            }
         }
      }
    }
  }
  }
  /* END OF BUILD_PALETTE */

  /* If we are excluding the tRNS chunk and there is transparency,
   * then we must write a Gray-Alpha (color-type 4) or RGBA (color-type 6)
   * PNG.
   */
  if (mng_info->exclude_tRNS != MagickFalse &&
     (number_transparent != 0 || number_semitransparent != 0))
    {
      volatile unsigned int colortype = mng_info->colortype;

      if (ping_have_color == MagickFalse)
        mng_info->colortype=5;

      else
        mng_info->colortype=7;

      if (colortype != 0 &&
         mng_info->colortype != colortype)
        ping_need_colortype_warning=MagickTrue;

    }

  /* See if cheap transparency is possible.  It is only possible
   * when there is a single transparent color, no semitransparent
   * color, and no opaque color that has the same RGB components
   * as the transparent color.  We only need this information if
   * we are writing a PNG with colortype 0 or 2, and we have not
   * excluded the tRNS chunk.
   */
  if (number_transparent == 1 &&
      mng_info->colortype < 4)
    {
       ping_have_cheap_transparency = MagickTrue;

       if (number_semitransparent != 0)
         ping_have_cheap_transparency = MagickFalse;

       else if (image_colors == 0 || image_colors > 256 ||
           image->colormap == NULL)
         {
           const Quantum
             *q;

           for (y=0; y < (ssize_t) image->rows; y++)
           {
             q=GetVirtualPixels(image,0,y,image->columns,1, exception);

             if (q == (Quantum *) NULL)
               break;

             for (x=0; x < (ssize_t) image->columns; x++)
             {
                 if (GetPixelAlpha(image,q) != TransparentAlpha &&
                     (unsigned short) GetPixelRed(image,q) ==
                                     ping_trans_color.red &&
                     (unsigned short) GetPixelGreen(image,q) ==
                                     ping_trans_color.green &&
                     (unsigned short) GetPixelBlue(image,q) ==
                                     ping_trans_color.blue)
                   {
                     ping_have_cheap_transparency = MagickFalse;
                     break;
                   }

                 q+=(ptrdiff_t) GetPixelChannels(image);
             }

             if (ping_have_cheap_transparency == MagickFalse)
                break;
           }
         }
       else
         {
            /* Assuming that image->colormap[0] is the one transparent color
             * and that all others are opaque.
             */
            if (image_colors > 1)
              for (i=1; i<image_colors; i++)
                if (image->colormap[i].red == image->colormap[0].red &&
                    image->colormap[i].green == image->colormap[0].green &&
                    image->colormap[i].blue == image->colormap[0].blue)
                  {
                     ping_have_cheap_transparency = MagickFalse;
                     break;
                  }
         }

       if (logging != MagickFalse)
         {
           if (ping_have_cheap_transparency == MagickFalse)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "   Cheap transparency is not possible.");

           else
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "   Cheap transparency is possible.");
         }
     }
  else
    ping_have_cheap_transparency = MagickFalse;

  image_depth=image->depth;

  quantum_info = (QuantumInfo *) NULL;
  number_colors=0;
  image_matte=image->alpha_trait !=
        UndefinedPixelTrait ? MagickTrue : MagickFalse;

  if (mng_info->colortype < 5)
    mng_info->is_palette=(image->storage_class == PseudoClass) &&
      (image_colors <= 256) && (image->colormap != NULL) ? MagickTrue :
      MagickFalse;
  else
    mng_info->is_palette=MagickFalse;

  if (((mng_info->colortype == 4) || (mng_info->write_png8 != MagickFalse)) &&
     ((image_colors == 0) || (image->colormap == NULL)))
    {
      image_info=DestroyImageInfo(image_info);
      image=DestroyImage(image);
      (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
          "Cannot write PNG8 or color-type 3; colormap is NULL",
          "`%s'",IMimage->filename);
      return(MagickFalse);
    }

  /*
    Allocate the PNG structures
  */
#ifdef PNG_USER_MEM_SUPPORTED
 error_info.image=image;
 error_info.exception=exception;
  ping=png_create_write_struct_2(PNG_LIBPNG_VER_STRING,&error_info,
    MagickPNGErrorHandler,MagickPNGWarningHandler,(void *) NULL,
    (png_malloc_ptr) Magick_png_malloc,(png_free_ptr) Magick_png_free);

#else
  ping=png_create_write_struct(PNG_LIBPNG_VER_STRING,&error_info,
    MagickPNGErrorHandler,MagickPNGWarningHandler);

#endif
  if (ping == (png_struct *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");

  ping_info=png_create_info_struct(ping);

  if (ping_info == (png_info *) NULL)
    {
      png_destroy_write_struct(&ping,(png_info **) NULL);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }

  png_set_write_fn(ping,image,png_put_data,png_flush_data);
  pixel_info=(MemoryInfo *) NULL;

  if (setjmp(png_jmpbuf(ping)))
    {
      /*
        PNG write failed.
      */
      png_destroy_write_struct(&ping,&ping_info);
#ifdef IMPNG_SETJMP_NOT_THREAD_SAFE
      UnlockSemaphoreInfo(ping_semaphore);
#endif

      if (pixel_info != (MemoryInfo *) NULL)
        pixel_info=RelinquishVirtualMemory(pixel_info);

      if (quantum_info != (QuantumInfo *) NULL)
        quantum_info=DestroyQuantumInfo(quantum_info);

      if (ping_have_blob != MagickFalse)
          (void) CloseBlob(image);
      image_info=DestroyImageInfo(image_info);
      image=DestroyImage(image);
      return(MagickFalse);
    }

  /* {  For navigation to end of SETJMP-protected block.  Within this
   *    block, use png_error() instead of Throwing an Exception, to ensure
   *    that libpng is able to clean up, and that the semaphore is unlocked.
   */

#ifdef IMPNG_SETJMP_NOT_THREAD_SAFE
  LockSemaphoreInfo(ping_semaphore);
#endif

#ifdef PNG_BENIGN_ERRORS_SUPPORTED
  /* Allow benign errors */
  png_set_benign_errors(ping, 1);
#endif

#ifdef PNG_SET_USER_LIMITS_SUPPORTED
  /* Reject images with too many rows or columns */
  png_set_user_limits(ping,
    (png_uint_32) MagickMin(0x7fffffffL,
        GetMagickResourceLimit(WidthResource)),
    (png_uint_32) MagickMin(0x7fffffffL,
        GetMagickResourceLimit(HeightResource)));
#endif /* PNG_SET_USER_LIMITS_SUPPORTED */

  /*
    Prepare PNG for writing.
  */

  if (mng_info->write_mng != MagickFalse)
  {
     (void) png_permit_mng_features(ping,PNG_ALL_MNG_FEATURES);
# ifdef PNG_WRITE_CHECK_FOR_INVALID_INDEX_SUPPORTED
     /* Disable new libpng-1.5.10 feature when writing a MNG because
      * zero-length PLTE is OK
      */
     png_set_check_for_invalid_index (ping, 0);
# endif
  }

  x=0;

  ping_width=(png_uint_32) image->columns;
  ping_height=(png_uint_32) image->rows;

  if ((mng_info->write_png8 != MagickFalse) ||
      (mng_info->write_png24 != MagickFalse) ||
      (mng_info->write_png32 != MagickFalse))
     image_depth=8;

  if ((mng_info->write_png48 != MagickFalse) ||
      (mng_info->write_png64 != MagickFalse))
     image_depth=16;

  if (mng_info->depth != 0)
     image_depth=mng_info->depth;

  /* Adjust requested depth to next higher valid depth if necessary */
  if (image_depth > 8)
     image_depth=16;

  if ((image_depth > 4) && (image_depth < 8))
     image_depth=8;

  if (image_depth == 3)
     image_depth=4;

  if (logging != MagickFalse)
    {
     (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    width=%.20g",(double) ping_width);
     (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    height=%.20g",(double) ping_height);
     (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    image_matte=%.20g",(double) image->alpha_trait);
     (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    image->depth=%.20g",(double) image->depth);
     (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    Tentative ping_bit_depth=%.20g",(double) image_depth);
    }

  save_image_depth=image_depth;
  ping_bit_depth=(png_byte) save_image_depth;


#if defined(PNG_pHYs_SUPPORTED)
  if (mng_info->exclude_pHYs == MagickFalse)
  {
  if ((image->resolution.x != 0) && (image->resolution.y != 0) &&
      (mng_info->write_mng == MagickFalse || mng_info->equal_physs == MagickFalse))
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Setting up pHYs chunk");

      if (image->units == PixelsPerInchResolution)
        {
          ping_pHYs_unit_type=PNG_RESOLUTION_METER;
          ping_pHYs_x_resolution=(png_uint_32) CastDoubleToSizeT((100.0*
            image->resolution.x)/2.54);
          ping_pHYs_y_resolution=(png_uint_32) CastDoubleToSizeT((100.0*
            image->resolution.y+0.5)/2.54);
        }

      else if (image->units == PixelsPerCentimeterResolution)
        {
          ping_pHYs_unit_type=PNG_RESOLUTION_METER;
          ping_pHYs_x_resolution=(png_uint_32) CastDoubleToSizeT(100.0*
            image->resolution.x);
          ping_pHYs_y_resolution=(png_uint_32) CastDoubleToSizeT(100.0*
            image->resolution.y);
        }

      else
        {
          ping_pHYs_unit_type=PNG_RESOLUTION_UNKNOWN;
          ping_pHYs_x_resolution=(png_uint_32) CastDoubleToSizeT(
            image->resolution.x);
          ping_pHYs_y_resolution=(png_uint_32) CastDoubleToSizeT(
            image->resolution.y);
        }

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Set up PNG pHYs chunk: xres: %.20g, yres: %.20g, units: %d.",
          (double) ping_pHYs_x_resolution,(double) ping_pHYs_y_resolution,
          (int) ping_pHYs_unit_type);
       ping_have_pHYs = MagickTrue;
    }
  }
#endif

  if (mng_info->exclude_bKGD == MagickFalse)
  {
  if ((mng_info->adjoin == MagickFalse || mng_info->equal_backgrounds == MagickFalse))
    {
       unsigned int
         mask;

       mask=0xffff;
       if (ping_bit_depth == 8)
          mask=0x00ff;

       if (ping_bit_depth == 4)
          mask=0x000f;

       if (ping_bit_depth == 2)
          mask=0x0003;

       if (ping_bit_depth == 1)
          mask=0x0001;

       ping_background.red=(png_uint_16)
         (ScaleQuantumToShort((Quantum) image->background_color.red) & mask);

       ping_background.green=(png_uint_16)
         (ScaleQuantumToShort((Quantum) image->background_color.green) & mask);

       ping_background.blue=(png_uint_16)
         (ScaleQuantumToShort((Quantum) image->background_color.blue) & mask);

       ping_background.gray=(png_uint_16) ping_background.green;
    }

  if (logging != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Setting up bKGD chunk (1)");
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "      background_color index is %d",
          (int) ping_background.index);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    ping_bit_depth=%d",ping_bit_depth);
    }

  ping_have_bKGD = MagickTrue;
  }

  /*
    Select the color type.
  */
  matte=image_matte;
  old_bit_depth=0;

  if ((mng_info->is_palette != MagickFalse) &&
      (mng_info->write_png8 != MagickFalse))
    {
      /* To do: make this a function cause it's used twice, except
         for reducing the sample depth from 8. */

      number_colors=image_colors;

      ping_have_tRNS=MagickFalse;

      /*
        Set image palette.
      */
      ping_color_type=(png_byte) PNG_COLOR_TYPE_PALETTE;

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Setting up PLTE chunk with %d colors (%d)",
            number_colors, image_colors);

      for (i=0; i < (ssize_t) number_colors; i++)
      {
        palette[i].red=ScaleQuantumToChar((Quantum) image->colormap[i].red);
        palette[i].green=ScaleQuantumToChar((Quantum) image->colormap[i].green);
        palette[i].blue=ScaleQuantumToChar((Quantum) image->colormap[i].blue);
        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
#if MAGICKCORE_QUANTUM_DEPTH == 8
            "    %3ld (%3d,%3d,%3d)",
#else
            "    %5ld (%5d,%5d,%5d)",
#endif
            (long) i,palette[i].red,palette[i].green,palette[i].blue);

      }

      ping_have_PLTE=MagickTrue;
      image_depth=(size_t) ping_bit_depth;
      ping_num_trans=0;

      if (matte != MagickFalse)
      {
          /*
            Identify which colormap entry is transparent.
          */
          assert(number_colors <= 256);
          assert(image->colormap != NULL);

          for (i=0; i < (ssize_t) number_transparent; i++)
             ping_trans_alpha[i]=0;


          ping_num_trans=(unsigned short) (number_transparent +
             number_semitransparent);

          if (ping_num_trans == 0)
             ping_have_tRNS=MagickFalse;

          else
             ping_have_tRNS=MagickTrue;
      }

      if (mng_info->exclude_bKGD == MagickFalse)
      {
       /*
        * Identify which colormap entry is the background color.
        */

        for (i=0; i < (ssize_t) MagickMax(1L*number_colors-1L,1L); i++)
          if (IsPNGColorEqual(ping_background,image->colormap[i]))
            break;

        ping_background.index=(png_byte) i;

        if (logging != MagickFalse)
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "      background_color index is %d",
                 (int) ping_background.index);
          }
      }
    } /* end of write_png8 */

  else if (mng_info->colortype == 1)
    {
      image_matte=MagickFalse;
      ping_color_type=(png_byte) PNG_COLOR_TYPE_GRAY;
    }

  else if ((mng_info->write_png24 != MagickFalse) ||
           (mng_info->write_png48 != MagickFalse) ||
           (mng_info->colortype == 3))
    {
      image_matte=MagickFalse;
      ping_color_type=(png_byte) PNG_COLOR_TYPE_RGB;
    }

  else if ((mng_info->write_png32 != MagickFalse) ||
           (mng_info->write_png64 != MagickFalse) ||
           (mng_info->colortype == 7))
    {
      image_matte=MagickTrue;
      ping_color_type=(png_byte) PNG_COLOR_TYPE_RGB_ALPHA;
    }

  else /* mng_info->write_pngNN not specified */
    {
      image_depth=(size_t) ping_bit_depth;

      if (mng_info->colortype != 0)
        {
          ping_color_type=(png_byte) mng_info->colortype-1;

          if (ping_color_type == PNG_COLOR_TYPE_GRAY_ALPHA ||
              ping_color_type == PNG_COLOR_TYPE_RGB_ALPHA)
            image_matte=MagickTrue;

          else
            image_matte=MagickFalse;

          if (logging != MagickFalse)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "   PNG colortype %d was specified:",(int) ping_color_type);
        }

      else /* write_png_colortype not specified */
        {
          if (logging != MagickFalse)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "  Selecting PNG colortype:");

          if (image_info->type == TrueColorType)
            {
              ping_color_type=(png_byte) PNG_COLOR_TYPE_RGB;
              image_matte=MagickFalse;
            }
          else if (image_info->type == TrueColorAlphaType)
            {
              ping_color_type=(png_byte) PNG_COLOR_TYPE_RGB_ALPHA;
              image_matte=MagickTrue;
            }
          else if (image_info->type == PaletteType ||
                   image_info->type == PaletteAlphaType)
            ping_color_type=(png_byte) PNG_COLOR_TYPE_PALETTE;
          else
            {
              if (ping_have_color == MagickFalse)
                {
                  if (image_matte == MagickFalse)
                    {
                      ping_color_type=(png_byte) PNG_COLOR_TYPE_GRAY;
                      image_matte=MagickFalse;
                    }

                  else
                    {
                      ping_color_type=(png_byte) PNG_COLOR_TYPE_GRAY_ALPHA;
                      image_matte=MagickTrue;
                    }
                }
              else
                {
                  if (image_matte == MagickFalse)
                    {
                      ping_color_type=(png_byte) PNG_COLOR_TYPE_RGB;
                      image_matte=MagickFalse;
                    }

                  else
                    {
                      ping_color_type=(png_byte) PNG_COLOR_TYPE_RGBA;
                      image_matte=MagickTrue;
                    }
                 }
            }

        }

      if (logging != MagickFalse)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
         "    Selected PNG colortype=%d",ping_color_type);

      if (ping_bit_depth < 8)
        {
          if (ping_color_type == PNG_COLOR_TYPE_GRAY_ALPHA ||
              ping_color_type == PNG_COLOR_TYPE_RGB ||
              ping_color_type == PNG_COLOR_TYPE_RGB_ALPHA)
            ping_bit_depth=8;
        }

      old_bit_depth=(size_t) ping_bit_depth;

      if (ping_color_type == PNG_COLOR_TYPE_GRAY)
        {
          if (((image->alpha_trait & BlendPixelTrait) == 0) &&
              ((image->alpha_trait & UpdatePixelTrait) == 0) &&
               (ping_have_non_bw == MagickFalse))
             ping_bit_depth=1;
        }

      if (ping_color_type == PNG_COLOR_TYPE_PALETTE)
        {
           size_t one = 1;
           ping_bit_depth=1;

           if (image_colors == 0)
           {
              /* DO SOMETHING */
                png_error(ping,"image has 0 colors");
           }

           while ((ping_bit_depth < 64) && (int) (one << ping_bit_depth) < (ssize_t) image_colors)
             ping_bit_depth <<= 1;
        }

      if (logging != MagickFalse)
         {
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Number of colors: %.20g",(double) image_colors);

           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Tentative PNG bit depth: %d",ping_bit_depth);
         }

      if (ping_bit_depth < (int) mng_info->depth)
         ping_bit_depth = (int) mng_info->depth;
    }
  (void) old_bit_depth;
  image_depth=(size_t) ping_bit_depth;

  if (logging != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    Tentative PNG color type: %s (%.20g)",
        PngColorTypeToString((unsigned int) ping_color_type),
        (double) ping_color_type);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    image_info->type: %.20g",(double) image_info->type);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    image_depth: %.20g",(double) image_depth);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),

        "    image->depth: %.20g",(double) image->depth);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    ping_bit_depth: %.20g",(double) ping_bit_depth);
    }

  if (matte != MagickFalse)
    {
      if (mng_info->is_palette != MagickFalse)
        {
          if (mng_info->colortype == 0)
            {
              ping_color_type=PNG_COLOR_TYPE_GRAY_ALPHA;

              if (ping_have_color != MagickFalse)
                 ping_color_type=PNG_COLOR_TYPE_RGBA;
            }

          /*
           * Determine if there is any transparent color.
          */
          if (number_transparent + number_semitransparent == 0)
            {
              /*
                No transparent pixels are present.  Change 4 or 6 to 0 or 2.
              */

              image_matte=MagickFalse;

              if (mng_info->colortype == 0)
                ping_color_type&=0x03;
            }

          else
            {
              unsigned int
                mask;

              mask=0xffff;

              if (ping_bit_depth == 8)
                 mask=0x00ff;

              if (ping_bit_depth == 4)
                 mask=0x000f;

              if (ping_bit_depth == 2)
                 mask=0x0003;

              if (ping_bit_depth == 1)
                 mask=0x0001;

              ping_trans_color.red=(png_uint_16)
                (ScaleQuantumToShort((Quantum) image->colormap[0].red) & mask);

              ping_trans_color.green=(png_uint_16)
                (ScaleQuantumToShort((Quantum) image->colormap[0].green) & mask);

              ping_trans_color.blue=(png_uint_16)
                (ScaleQuantumToShort((Quantum) image->colormap[0].blue) & mask);

              ping_trans_color.gray=(png_uint_16)
                (ScaleQuantumToShort((Quantum) GetPixelInfoIntensity(image,
                   image->colormap)) & mask);

              ping_trans_color.index=(png_byte) 0;

              ping_have_tRNS=MagickTrue;
            }

          if (ping_have_tRNS != MagickFalse)
            {
              /*
               * Determine if there is one and only one transparent color
               * and if so if it is fully transparent.
               */
              if (ping_have_cheap_transparency == MagickFalse)
                ping_have_tRNS=MagickFalse;
            }

          if (ping_have_tRNS != MagickFalse)
            {
              if (mng_info->colortype == 0)
                ping_color_type &= 0x03;  /* changes 4 or 6 to 0 or 2 */

              if (image_depth == 8)
                {
                  ping_trans_color.red&=0xff;
                  ping_trans_color.green&=0xff;
                  ping_trans_color.blue&=0xff;
                  ping_trans_color.gray&=0xff;
                }
            }
        }
      else
        {
          if (image_depth == 8)
            {
              ping_trans_color.red&=0xff;
              ping_trans_color.green&=0xff;
              ping_trans_color.blue&=0xff;
              ping_trans_color.gray&=0xff;
            }
        }
    }

    matte=image_matte;

    if (ping_have_tRNS != MagickFalse)
      image_matte=MagickFalse;

    if ((mng_info->is_palette != MagickFalse) &&
        mng_info->colortype-1 != PNG_COLOR_TYPE_PALETTE &&
        ping_have_color == MagickFalse &&
        (image_matte == MagickFalse || image_depth >= 8))
      {
        size_t one=1;

        if (image_matte != MagickFalse)
          ping_color_type=PNG_COLOR_TYPE_GRAY_ALPHA;

        else if (mng_info->colortype-1 != PNG_COLOR_TYPE_GRAY_ALPHA)
          {
            ping_color_type=PNG_COLOR_TYPE_GRAY;

            if (save_image_depth == 16 && image_depth == 8)
              {
                if (logging != MagickFalse)
                  {
                    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "  Scaling ping_trans_color (0)");
                  }
                    ping_trans_color.gray*=0x0101;
              }
          }

        if (image_depth > MAGICKCORE_QUANTUM_DEPTH)
          image_depth=MAGICKCORE_QUANTUM_DEPTH;

        if ((image_colors == 0) ||
             ((ssize_t) (image_colors-1) > (ssize_t) MaxColormapSize))
          image_colors=(int) (one << image_depth);

        if (image_depth > 8)
          ping_bit_depth=16;

        else
          {
            ping_bit_depth=8;
            if ((int) ping_color_type == PNG_COLOR_TYPE_PALETTE)
              {
                if(!mng_info->depth)
                  {
                    ping_bit_depth=1;

                    while ((int) (one << ping_bit_depth)
                        < (ssize_t) image_colors)
                      ping_bit_depth <<= 1;
                  }
              }

            else if (ping_color_type ==
                PNG_COLOR_TYPE_GRAY && image_colors < 17 &&
                mng_info->is_palette != MagickFalse)
              {
              /* Check if grayscale is reducible */

                int
                  depth_4_ok=MagickTrue,
                  depth_2_ok=MagickTrue,
                  depth_1_ok=MagickTrue;

                for (i=0; i < (ssize_t) image_colors; i++)
                {
                   unsigned char
                     intensity;

                   intensity=ScaleQuantumToChar((Quantum) image->colormap[i].red);

                   if ((intensity & 0x0f) != ((intensity & 0xf0) >> 4))
                     depth_4_ok=depth_2_ok=depth_1_ok=MagickFalse;
                   else if ((intensity & 0x03) != ((intensity & 0x0c) >> 2))
                     depth_2_ok=depth_1_ok=MagickFalse;
                   else if ((intensity & 0x01) != ((intensity & 0x02) >> 1))
                     depth_1_ok=MagickFalse;
                }

                if (depth_1_ok && mng_info->depth <= 1)
                  ping_bit_depth=1;

                else if (depth_2_ok && mng_info->depth <= 2)
                  ping_bit_depth=2;

                else if (depth_4_ok && mng_info->depth <= 4)
                  ping_bit_depth=4;
              }
          }

          image_depth=(size_t) ping_bit_depth;
      }

    else

      if (mng_info->is_palette != MagickFalse)
      {
        number_colors=image_colors;

        if (image_depth <= 8)
          {
            /*
              Set image palette.
            */
            ping_color_type=(png_byte) PNG_COLOR_TYPE_PALETTE;

            if (!(mng_info->have_global_plte != MagickFalse && matte == MagickFalse))
              {
                for (i=0; i < (ssize_t) number_colors; i++)
                {
                  palette[i].red=ScaleQuantumToChar((Quantum) image->colormap[i].red);
                  palette[i].green=ScaleQuantumToChar((Quantum) image->colormap[i].green);
                  palette[i].blue=ScaleQuantumToChar((Quantum) image->colormap[i].blue);
                }

                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  Setting up PLTE chunk with %d colors",
                    number_colors);

                ping_have_PLTE=MagickTrue;
              }

            /* color_type is PNG_COLOR_TYPE_PALETTE */
            if (mng_info->depth == 0)
              {
                size_t
                  one;

                ping_bit_depth=1;
                one=1;

                while ((one << ping_bit_depth) < (size_t) number_colors)
                  ping_bit_depth <<= 1;
              }

            ping_num_trans=0;

            if (matte != MagickFalse)
              {
                /*
                 * Set up trans_colors array.
                 */
                assert(number_colors <= 256);

                ping_num_trans=(unsigned short) (number_transparent +
                  number_semitransparent);

                if (ping_num_trans == 0)
                  ping_have_tRNS=MagickFalse;

                else
                  {
                    if (logging != MagickFalse)
                      {
                        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "  Scaling ping_trans_color (1)");
                      }
                    ping_have_tRNS=MagickTrue;

                    for (i=0; i < ping_num_trans; i++)
                    {
                       ping_trans_alpha[i]= (png_byte)
                         ScaleQuantumToChar((Quantum) image->colormap[i].alpha);
                    }
                  }
              }
          }
      }

    else
      {

        if (image_depth < 8)
          image_depth=8;

        if ((save_image_depth == 16) && (image_depth == 8))
          {
            if (logging != MagickFalse)
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    Scaling ping_trans_color from (%d,%d,%d)",
                  (int) ping_trans_color.red,
                  (int) ping_trans_color.green,
                  (int) ping_trans_color.blue);
              }

            ping_trans_color.red*=0x0101;
            ping_trans_color.green*=0x0101;
            ping_trans_color.blue*=0x0101;
            ping_trans_color.gray*=0x0101;

            if (logging != MagickFalse)
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    to (%d,%d,%d)",
                  (int) ping_trans_color.red,
                  (int) ping_trans_color.green,
                  (int) ping_trans_color.blue);
              }
          }
      }

    if (ping_bit_depth <  (int) mng_info->depth)
         ping_bit_depth =  (int) mng_info->depth;

    /*
      Adjust background and transparency samples in sub-8-bit grayscale files.
    */
    if (ping_bit_depth < 8 && ping_color_type ==
        PNG_COLOR_TYPE_GRAY)
      {
         png_uint_16
           maxval;

         size_t
           one=1;

         maxval=(png_uint_16) ((one << ping_bit_depth)-1);

         if (mng_info->exclude_bKGD == MagickFalse)
         {

         ping_background.gray=(png_uint_16) ((maxval/65535.)*
           (ScaleQuantumToShort((Quantum) (((GetPixelInfoIntensity(image,
           &image->background_color))) +.5))));

         if (logging != MagickFalse)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "  Setting up bKGD chunk (2)");
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "      background_color index is %d",
             (int) ping_background.index);

         ping_have_bKGD = MagickTrue;
         }

         if (logging != MagickFalse)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "  Scaling ping_trans_color.gray from %d",
             (int)ping_trans_color.gray);

         ping_trans_color.gray=(png_uint_16) ((maxval/255.)*(
           ping_trans_color.gray)+.5);

         if (logging != MagickFalse)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "      to %d", (int)ping_trans_color.gray);
      }

  if (mng_info->exclude_bKGD == MagickFalse)
  {
    if (mng_info->is_palette != MagickFalse &&
        (int) ping_color_type == PNG_COLOR_TYPE_PALETTE)
      {
        /*
           Identify which colormap entry is the background color.
        */

        number_colors=image_colors;

        for (i=0; i < (ssize_t) MagickMax(1L*number_colors,1L); i++)
          if (IsPNGColorEqual(image->background_color,image->colormap[i]))
            break;

        ping_background.index=(png_byte) i;

        if (logging != MagickFalse)
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Setting up bKGD chunk with index=%d",(int) i);
          }

        if (i < (ssize_t) number_colors)
          {
            ping_have_bKGD = MagickTrue;

            if (logging != MagickFalse)
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "     background   =(%d,%d,%d)",
                        (int) ping_background.red,
                        (int) ping_background.green,
                        (int) ping_background.blue);
              }
          }

        else  /* Can't happen */
          {
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "      No room in PLTE to add bKGD color");
            ping_have_bKGD = MagickFalse;
          }
      }
  }

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    PNG color type: %s (%.20g)", PngColorTypeToString((unsigned int)
      ping_color_type), (double) ping_color_type);
  /*
    Initialize compression level and filtering.
  */
  if (logging != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Setting up deflate compression");

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    Compression buffer size: 32768");
    }

  png_set_compression_buffer_size(ping,32768L);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    Compression mem level: 9");

  png_set_compression_mem_level(ping, 9);

  /* Untangle the "-quality" setting:

     Undefined is 0; the default is used.
     Default is 75

     10's digit:

        0 or omitted: Use Z_HUFFMAN_ONLY strategy with the
           zlib default compression level

        1-9: the zlib compression level

     1's digit:

        0-4: the PNG filter method

        5:   libpng adaptive filtering if compression level > 5
             libpng filter type "none" if compression level <= 5
                or if image is grayscale or palette

        6:   libpng adaptive filtering

        7:   "LOCO" filtering (intrapixel differing) if writing
             a MNG, otherwise "none".  Did not work in IM-6.7.0-9
             and earlier because of a missing "else".

        8:   Z_RLE strategy (or Z_HUFFMAN_ONLY if quality < 10), adaptive
             filtering. Unused prior to IM-6.7.0-10, was same as 6

        9:   Z_RLE strategy (or Z_HUFFMAN_ONLY if quality < 10), no PNG filters
             Unused prior to IM-6.7.0-10, was same as 6

    Note that using the -quality option, not all combinations of
    PNG filter type, zlib compression level, and zlib compression
    strategy are possible.  This will be addressed soon in a
    release that accommodates "-define png:compression-strategy", etc.

   */

  quality=image_info->quality == UndefinedCompressionQuality ? 75UL :
     image_info->quality;

  if (quality <= 9)
    {
      if (mng_info->compression_strategy == 0)
        mng_info->compression_strategy = Z_HUFFMAN_ONLY+1;
    }

  else if (mng_info->compression_level == 0)
    {
      int
        level;

      level=(int) MagickMin((ssize_t) quality/10,9);

      mng_info->compression_level = (size_t) level+1;
    }

  if (mng_info->compression_strategy == 0)
    {
        if ((quality %10) == 8 || (quality %10) == 9)
#ifdef Z_RLE  /* Z_RLE was added to zlib-1.2.0 */
          mng_info->compression_strategy=Z_RLE+1;
#else
          mng_info->write_png_compression_strategy = Z_DEFAULT_STRATEGY+1;
#endif
    }

  if (mng_info->compression_filter == 0)
        mng_info->compression_filter=((size_t) quality % 10) + 1;

  if (logging != MagickFalse)
    {
        if (mng_info->compression_level)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Compression level:    %d",
            (int) mng_info->compression_level-1);

        if (mng_info->compression_strategy)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Compression strategy: %d",
            (int) mng_info->compression_strategy-1);

        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  Setting up filtering");

        if (mng_info->compression_filter == 6)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Base filter method: ADAPTIVE");
        else if (mng_info->compression_filter == 0 ||
                 mng_info->compression_filter == 1)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Base filter method: NONE");
        else
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Base filter method: %d",
            (int) mng_info->compression_filter-1);
    }

  if (mng_info->compression_level != 0)
    png_set_compression_level(ping,(int) mng_info->compression_level-1);

  if (mng_info->compression_filter == 6)
    {
      if (((int) ping_color_type == PNG_COLOR_TYPE_GRAY) ||
         ((int) ping_color_type == PNG_COLOR_TYPE_PALETTE) ||
         (quality < 50))
        png_set_filter(ping,PNG_FILTER_TYPE_BASE,PNG_NO_FILTERS);
      else
        png_set_filter(ping,PNG_FILTER_TYPE_BASE,PNG_ALL_FILTERS);
     }
  else if (mng_info->compression_filter == 7 ||
      mng_info->compression_filter == 10)
    png_set_filter(ping,PNG_FILTER_TYPE_BASE,PNG_ALL_FILTERS);

  else if (mng_info->compression_filter == 8)
    {
#if defined(PNG_INTRAPIXEL_DIFFERENCING)
      if (mng_info->write_mng != MagickFalse)
      {
         if (((int) ping_color_type == PNG_COLOR_TYPE_RGB) ||
             ((int) ping_color_type == PNG_COLOR_TYPE_RGBA))
        ping_filter_method=PNG_INTRAPIXEL_DIFFERENCING;
      }
#endif
      png_set_filter(ping,PNG_FILTER_TYPE_BASE,PNG_NO_FILTERS);
    }

  else if (mng_info->compression_filter == 9)
    png_set_filter(ping,PNG_FILTER_TYPE_BASE,PNG_NO_FILTERS);

  else if (mng_info->compression_filter != 0)
    png_set_filter(ping,PNG_FILTER_TYPE_BASE,
       (int) mng_info->compression_filter-1);

  if (mng_info->compression_strategy != 0)
    png_set_compression_strategy(ping,
       (int) mng_info->compression_strategy-1);

  ping_interlace_method=image_info->interlace != NoInterlace;

  if (mng_info->write_mng != MagickFalse)
    png_set_sig_bytes(ping,8);

  /* Bail out if cannot meet defined png:bit-depth or png:color-type */

  if (mng_info->colortype != 0)
    {
     if (mng_info->colortype-1 == PNG_COLOR_TYPE_GRAY)
       if (ping_have_color != MagickFalse)
         {
           ping_color_type = PNG_COLOR_TYPE_RGB;

           if (ping_bit_depth < 8)
             ping_bit_depth=8;
         }

     if (mng_info->colortype-1 == PNG_COLOR_TYPE_GRAY_ALPHA)
       if (ping_have_color != MagickFalse)
         ping_color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    }

  if (ping_need_colortype_warning != MagickFalse ||
     ((mng_info->depth &&
     (int) mng_info->depth != ping_bit_depth) ||
     (mng_info->colortype &&
     ((int) mng_info->colortype-1 != ping_color_type &&
      mng_info->colortype != 7 &&
      !(mng_info->colortype == 5 && ping_color_type == 0)))))
    {
      if (logging != MagickFalse)
        {
          if (ping_need_colortype_warning != MagickFalse)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "  Image has transparency but tRNS chunk was excluded");
            }

          if (mng_info->depth)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Defined png:bit-depth=%u, Computed depth=%u",
                  mng_info->depth,
                  ping_bit_depth);
            }

          if (mng_info->colortype)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Defined png:color-type=%u, Computed color type=%u",
                  mng_info->colortype-1,
                  ping_color_type);
            }
        }

      png_warning(ping,
        "Cannot write image with defined png:bit-depth or png:color-type.");
    }

  if ((image_matte != MagickFalse) &&
      ((image->alpha_trait & BlendPixelTrait) == 0) &&
      ((image->alpha_trait & UpdatePixelTrait) == 0))
    {
      /* Add an opaque matte channel */
      image->alpha_trait = BlendPixelTrait;
      (void) SetImageAlpha(image,OpaqueAlpha,exception);

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  Added an opaque matte channel");
    }

  if (number_transparent != 0 || number_semitransparent != 0)
    {
      if (ping_color_type < 4)
        {
           ping_have_tRNS=MagickTrue;
           if (logging != MagickFalse)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "  Setting ping_have_tRNS=MagickTrue.");
        }
    }

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Writing PNG header chunks");

  png_set_IHDR(ping,ping_info,ping_width,ping_height,
               ping_bit_depth,ping_color_type,
               ping_interlace_method,ping_compression_method,
               ping_filter_method);

  if (ping_color_type == 3 && ping_have_PLTE != MagickFalse)
    {
      png_set_PLTE(ping,ping_info,palette,number_colors);

      if (logging != MagickFalse)
        {
          for (i=0; i< (ssize_t) number_colors; i++)
          {
            if (i < ping_num_trans)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "     PLTE[%d] = (%d,%d,%d), tRNS[%d] = (%d)",
                      (int) i,
                      (int) palette[i].red,
                      (int) palette[i].green,
                      (int) palette[i].blue,
                      (int) i,
                      (int) ping_trans_alpha[i]);
             else
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "     PLTE[%d] = (%d,%d,%d)",
                      (int) i,
                      (int) palette[i].red,
                      (int) palette[i].green,
                      (int) palette[i].blue);
           }
         }
    }

  /* Only write the iCCP chunk if we are not writing the sRGB chunk. */
  if (mng_info->exclude_sRGB != MagickFalse ||
     (!png_get_valid(ping,ping_info,PNG_INFO_sRGB)))
  {
    if ((mng_info->exclude_tEXt == MagickFalse ||
         mng_info->exclude_zTXt == MagickFalse) &&
       (ping_exclude_iCCP == MagickFalse || ping_exclude_zCCP == MagickFalse))
    {
      ResetImageProfileIterator(image);
      for (name=GetNextImageProfile(image); name != (char *) NULL; )
      {
        profile=GetImageProfile(image,name);

        if (profile != (StringInfo *) NULL)
          {
#ifdef PNG_WRITE_iCCP_SUPPORTED
            if ((LocaleCompare(name,"ICC") == 0) ||
                (LocaleCompare(name,"ICM") == 0))
              {
                ping_have_iCCP = MagickTrue;
                if (ping_exclude_iCCP == MagickFalse)
                  {
                    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "  Setting up iCCP chunk");

                    png_set_iCCP(ping,ping_info,(png_charp) name,0,
#if (PNG_LIBPNG_VER < 10500)
                    (png_charp) GetStringInfoDatum(profile),
#else
                    (const png_byte *) GetStringInfoDatum(profile),
#endif
                    (png_uint_32) GetStringInfoLength(profile));
                  }
                else
                  {
                    /* Do not write hex-encoded ICC chunk */
                       name=GetNextImageProfile(image);
                       continue;
                  }
              }
#endif /* WRITE_iCCP */

            if (LocaleCompare(name,"exif") == 0)
              {
                   /* Do not write hex-encoded ICC chunk; we will
                      write it later as an eXIf chunk */
                   name=GetNextImageProfile(image);
                   continue;
              }

              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "  Setting up zTXt chunk with uuencoded %s profile",
                 name);
              Magick_png_write_raw_profile(image_info,ping,ping_info,
                (unsigned char *) name,(unsigned char *) name,
                GetStringInfoDatum(profile),(png_uint_32)
                GetStringInfoLength(profile),exception);
          }
        name=GetNextImageProfile(image);
      }
    }
  }

#if defined(PNG_WRITE_sRGB_SUPPORTED)
  if ((mng_info->have_global_srgb == MagickFalse) &&
      ping_have_iCCP != MagickTrue &&
      (ping_have_sRGB != MagickFalse ||
      png_get_valid(ping,ping_info,PNG_INFO_sRGB)))
    {
      if (mng_info->exclude_sRGB == MagickFalse)
        {
          /*
            Note image rendering intent.
          */
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  Setting up sRGB chunk");

          (void) png_set_sRGB(ping,ping_info,(
            Magick_RenderingIntent_to_PNG_RenderingIntent(
              image->rendering_intent)));

          ping_have_sRGB = MagickTrue;
        }
    }

  if ((mng_info->write_mng == MagickFalse) || (!png_get_valid(ping,ping_info,PNG_INFO_sRGB)))
#endif
    {
      if (mng_info->exclude_gAMA == MagickFalse &&
          ping_have_iCCP == MagickFalse &&
          ping_have_sRGB != MagickFalse &&
          (mng_info->exclude_sRGB == MagickFalse ||
          (image->gamma < .45 || image->gamma > .46)))
      {
      if ((mng_info->have_global_gama == MagickFalse) && (image->gamma != 0.0))
        {
          /*
            Note image gamma.
          */
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Setting up gAMA chunk (%.20g)",image->gamma);

          png_set_gAMA(ping,ping_info,image->gamma);
        }
      }

      if (mng_info->exclude_cHRM == MagickFalse && ping_have_sRGB == MagickFalse)
        {
          if ((mng_info->have_global_chrm == MagickFalse) &&
              (image->chromaticity.red_primary.x != 0.0))
            {
              /*
                Note image chromaticity.
              */
               PrimaryInfo
                 bp,
                 gp,
                 rp,
                 wp;

               wp=image->chromaticity.white_point;
               rp=image->chromaticity.red_primary;
               gp=image->chromaticity.green_primary;
               bp=image->chromaticity.blue_primary;

               if (logging != MagickFalse)
                 (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "  Setting up cHRM chunk");

               png_set_cHRM(ping,ping_info,wp.x,wp.y,rp.x,rp.y,gp.x,gp.y,
                   bp.x,bp.y);
           }
        }
    }

  if (mng_info->exclude_bKGD == MagickFalse)
    {
      if (ping_have_bKGD != MagickFalse)
        {
          png_set_bKGD(ping,ping_info,&ping_background);
          if (logging != MagickFalse)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "    Setting up bKGD chunk");
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      background color = (%d,%d,%d)",
                        (int) ping_background.red,
                        (int) ping_background.green,
                        (int) ping_background.blue);
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      index = %d, gray=%d",
                        (int) ping_background.index,
                        (int) ping_background.gray);
            }
         }
    }

  if (mng_info->exclude_pHYs == MagickFalse)
    {
      if (ping_have_pHYs != MagickFalse)
        {
          png_set_pHYs(ping,ping_info,
             ping_pHYs_x_resolution,
             ping_pHYs_y_resolution,
             ping_pHYs_unit_type);

          if (logging != MagickFalse)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "    Setting up pHYs chunk");
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      x_resolution=%g",(double) ping_pHYs_x_resolution);
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      y_resolution=%g",(double) ping_pHYs_y_resolution);
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      unit_type=%g",(double) ping_pHYs_unit_type);
            }
        }
    }

#if defined(PNG_tIME_SUPPORTED)
  if (mng_info->exclude_tIME == MagickFalse)
    {
      const char
        *timestamp;

      if (image->taint == MagickFalse)
        {
          timestamp=GetImageOption(image_info,"png:tIME");

          if (timestamp == (const char *) NULL)
            timestamp=GetImageProperty(image,"png:tIME",exception);
        }

      else
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "  Reset tIME in tainted image");

          timestamp=GetImageProperty(image,"date:modify",exception);
        }

      if (timestamp != (const char *) NULL)
          write_tIME_chunk(image,ping,ping_info,timestamp,exception);
    }
#endif

  if (mng_info->need_blob != MagickFalse)
  {
    if (OpenBlob(image_info,image,WriteBinaryBlobMode,exception) ==
       MagickFalse)
       png_error(ping,"WriteBlob Failed");

     ping_have_blob=MagickTrue;
  }

  png_write_info_before_PLTE(ping, ping_info);

  if (ping_have_tRNS != MagickFalse && ping_color_type < 4)
    {
      if (logging != MagickFalse)
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Calling png_set_tRNS with num_trans=%d",ping_num_trans);
        }

      if (ping_color_type == 3)
         (void) png_set_tRNS(ping, ping_info,
                ping_trans_alpha,
                ping_num_trans,
                NULL);

      else
        {
           (void) png_set_tRNS(ping, ping_info,
                  NULL,
                  0,
                  &ping_trans_color);

           if (logging != MagickFalse)
             {
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "     tRNS color   =(%d,%d,%d)",
                       (int) ping_trans_color.red,
                       (int) ping_trans_color.green,
                       (int) ping_trans_color.blue);
             }
         }
    }

  png_write_info(ping,ping_info);

  /* write orNT if image->orientation is defined */
  if (image->orientation != UndefinedOrientation)
    {
      unsigned char
        chunk[6];
      (void) WriteBlobMSBULong(image,1L);  /* data length=1 */
      PNGType(chunk,mng_orNT);
      LogPNGChunk(logging,mng_orNT,1L);
      /* PNG uses Exif orientation values */
      chunk[4]=(unsigned char) Magick_Orientation_to_Exif_Orientation(image->orientation);
      (void) WriteBlob(image,5,chunk);
      (void) WriteBlobMSBULong(image,crc32(0,chunk,5));
    }

  ping_wrote_caNv = MagickFalse;

  /* write caNv chunk */
  if (mng_info->exclude_caNv == MagickFalse)
    {
      if ((image->page.width != 0 && image->page.width != image->columns) ||
          (image->page.height != 0 && image->page.height != image->rows) ||
          image->page.x != 0 || image->page.y != 0)
        {
          unsigned char
            chunk[20];

          (void) WriteBlobMSBULong(image,16L);  /* data length=8 */
          PNGType(chunk,mng_caNv);
          LogPNGChunk(logging,mng_caNv,16L);
          PNGLong(chunk+4,(png_uint_32) image->page.width);
          PNGLong(chunk+8,(png_uint_32) image->page.height);
          PNGsLong(chunk+12,(png_int_32) image->page.x);
          PNGsLong(chunk+16,(png_int_32) image->page.y);
          (void) WriteBlob(image,20,chunk);
          (void) WriteBlobMSBULong(image,crc32(0,chunk,20));
          ping_wrote_caNv = MagickTrue;
        }
    }

#if defined(PNG_oFFs_SUPPORTED)
  if (mng_info->exclude_oFFs == MagickFalse && ping_wrote_caNv == MagickFalse)
    {
      if (image->page.x || image->page.y)
        {
           png_set_oFFs(ping,ping_info,(png_int_32) image->page.x,
              (png_int_32) image->page.y, 0);

           if (logging != MagickFalse)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "    Setting up oFFs chunk with x=%d, y=%d, units=0",
                 (int) image->page.x, (int) image->page.y);
        }
    }
#endif

#if (PNG_LIBPNG_VER == 10206)
    /* avoid libpng-1.2.6 bug by setting PNG_HAVE_IDAT flag */
#define PNG_HAVE_IDAT               0x04
    ping->mode |= PNG_HAVE_IDAT;
#undef PNG_HAVE_IDAT
#endif

  png_set_packing(ping);
  /*
    Allocate memory.
  */
  rowbytes=image->columns;
  if (image_depth > 8)
    rowbytes*=2;
  switch (ping_color_type)
    {
      case PNG_COLOR_TYPE_RGB:
        rowbytes*=3;
        break;

      case PNG_COLOR_TYPE_GRAY_ALPHA:
        rowbytes*=2;
        break;

      case PNG_COLOR_TYPE_RGBA:
        rowbytes*=4;
        break;

      default:
        break;
    }

  if (logging != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Writing PNG image data");

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    Allocating %.20g bytes of memory for pixels",(double) rowbytes);
    }
  pixel_info=AcquireVirtualMemory(rowbytes,GetPixelChannels(image)*
    sizeof(*ping_pixels));
  if (pixel_info == (MemoryInfo *) NULL)
    png_error(ping,"Allocation of memory for pixels failed");
  ping_pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
  (void) memset(ping_pixels,0,rowbytes*GetPixelChannels(image)*
    sizeof(*ping_pixels));
  /*
    Initialize image scanlines.
  */
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    png_error(ping,"Memory allocation for quantum_info failed");
  (void) SetQuantumFormat(image,quantum_info,UndefinedQuantumFormat);
  (void) SetQuantumDepth(image,quantum_info,image_depth);
  (void) SetQuantumEndian(image,quantum_info,MSBEndian);
  num_passes=png_set_interlace_handling(ping);

  if ((mng_info->colortype-1 == PNG_COLOR_TYPE_PALETTE) ||
      ((mng_info->write_png8 == MagickFalse) &&
       (mng_info->write_png24 == MagickFalse) &&
       (mng_info->write_png48 == MagickFalse) &&
       (mng_info->write_png64 == MagickFalse) &&
       (mng_info->write_png32 == MagickFalse) &&
       (image_matte == MagickFalse) &&
       (ping_have_non_bw == MagickFalse) &&
       ((mng_info->is_palette != MagickFalse) ||
        (image_info->type == BilevelType))))
     {
      /* Palette, Bilevel, or Opaque Monochrome */
      QuantumType
        quantum_type;

      const Quantum
        *p;

      quantum_type=RedQuantum;
      if (mng_info->is_palette != MagickFalse)
        {
          quantum_type=GrayQuantum;
          if (mng_info->colortype-1 == PNG_COLOR_TYPE_PALETTE)
            quantum_type=IndexQuantum;
        }
      (void) SetQuantumDepth(image,quantum_info,8);
      for (pass=0; pass < num_passes; pass++)
      {
        /*
          Convert PseudoClass image to a PNG monochrome image.
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          if (logging != MagickFalse && y == 0)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "    Writing row of pixels (0)");

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);

          if (p == (const Quantum *) NULL)
            break;

          (void) ExportQuantumPixels(image,(CacheView *) NULL,
            quantum_info,quantum_type,ping_pixels,exception);
          if (mng_info->colortype-1 != PNG_COLOR_TYPE_PALETTE)
            for (i=0; i < (ssize_t) image->columns; i++)
               *(ping_pixels+i)=(unsigned char) ((*(ping_pixels+i) > 127) ?
                      255 : 0);

          if (logging != MagickFalse && y == 0)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    Writing row of pixels (1)");

          png_write_row(ping,ping_pixels);

          status=SetImageProgress(image,SaveImageTag,
              (MagickOffsetType) (pass * (ssize_t) image->rows + y),
              (MagickSizeType) num_passes * image->rows);

          if (status == MagickFalse)
            break;
        }
      }
    }

  else   /* Not Palette, Bilevel, or Opaque Monochrome */
    {
      if (((mng_info->write_png8 == MagickFalse) &&
           (mng_info->write_png24 == MagickFalse) &&
           (mng_info->write_png48 == MagickFalse) &&
           (mng_info->write_png64 == MagickFalse) &&
           (mng_info->write_png32 == MagickFalse)) &&
          ((image_matte != MagickFalse) || ((ping_bit_depth >= MAGICKCORE_QUANTUM_DEPTH))) &&
          ((mng_info->is_palette != MagickFalse)) && (ping_have_color == MagickFalse))
        {
          const Quantum
            *p;

          for (pass=0; pass < num_passes; pass++)
          {

          for (y=0; y < (ssize_t) image->rows; y++)
          {
            p=GetVirtualPixels(image,0,y,image->columns,1,exception);

            if (p == (const Quantum *) NULL)
              break;

            if (ping_color_type == PNG_COLOR_TYPE_GRAY)
              {
                if (mng_info->is_palette != MagickFalse)
                  (void) ExportQuantumPixels(image,(CacheView *) NULL,
                    quantum_info,GrayQuantum,ping_pixels,exception);

                else
                  (void) ExportQuantumPixels(image,(CacheView *) NULL,
                    quantum_info,RedQuantum,ping_pixels,exception);

                if (logging != MagickFalse && y == 0)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                       "    Writing GRAY PNG pixels (2)");
              }

            else /* PNG_COLOR_TYPE_GRAY_ALPHA */
              {
                if (logging != MagickFalse && y == 0)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                         "    Writing GRAY_ALPHA PNG pixels (2)");

                (void) ExportQuantumPixels(image,(CacheView *) NULL,
                  quantum_info,GrayAlphaQuantum,ping_pixels,exception);
              }

            if (logging != MagickFalse && y == 0)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    Writing row of pixels (2)");

            png_write_row(ping,ping_pixels);

            status=SetImageProgress(image,SaveImageTag,
              (MagickOffsetType) (pass * (ssize_t) image->rows + y),
              (MagickSizeType) num_passes * image->rows);

            if (status == MagickFalse)
              break;
            }
          }
        }

      else
        {
          const Quantum
            *p;

          for (pass=0; pass < num_passes; pass++)
          {
            if ((image_depth > 8) ||
                (mng_info->write_png24 != MagickFalse) ||
                (mng_info->write_png32 != MagickFalse) ||
                (mng_info->write_png48 != MagickFalse) ||
                (mng_info->write_png64 != MagickFalse) ||
                ((mng_info->write_png8 == MagickFalse) &&
                 (mng_info->is_palette == MagickFalse)))
            {
              for (y=0; y < (ssize_t) image->rows; y++)
              {
                p=GetVirtualPixels(image,0,y,image->columns,1, exception);

                if (p == (const Quantum *) NULL)
                  break;

                if (ping_color_type == PNG_COLOR_TYPE_GRAY)
                  {
                    if (image->storage_class == DirectClass)
                      (void) ExportQuantumPixels(image,(CacheView *) NULL,
                        quantum_info,RedQuantum,ping_pixels,exception);

                    else
                      (void) ExportQuantumPixels(image,(CacheView *) NULL,
                        quantum_info,GrayQuantum,ping_pixels,exception);
                  }

                else if (ping_color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
                  {
                    (void) ExportQuantumPixels(image,(CacheView *) NULL,
                      quantum_info,GrayAlphaQuantum,ping_pixels,exception);

                    if (logging != MagickFalse && y == 0)
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                           "    Writing GRAY_ALPHA PNG pixels (3)");
                  }

                else if (image_matte != MagickFalse)
                  (void) ExportQuantumPixels(image,(CacheView *) NULL,
                    quantum_info,RGBAQuantum,ping_pixels,exception);

                else
                  (void) ExportQuantumPixels(image,(CacheView *) NULL,
                    quantum_info,RGBQuantum,ping_pixels,exception);

                if (logging != MagickFalse && y == 0)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                      "    Writing row of pixels (3)");

                png_write_row(ping,ping_pixels);

                status=SetImageProgress(image,SaveImageTag,
                  (MagickOffsetType) (pass * (ssize_t) image->rows + y),
                  (MagickSizeType) num_passes * image->rows);

                if (status == MagickFalse)
                  break;
              }
            }

          else
            /* not ((image_depth > 8) ||
                mng_info->write_png24 || mng_info->write_png32 ||
                mng_info->write_png48 || mng_info->write_png64 ||
                (!mng_info->write_png8 && !mng_info->IsPalette))
             */
            {
              if ((ping_color_type != PNG_COLOR_TYPE_GRAY) &&
                  (ping_color_type != PNG_COLOR_TYPE_GRAY_ALPHA))
                {
                  if (logging != MagickFalse)
                    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                      "  pass %d, Image Is not GRAY or GRAY_ALPHA",pass);

                  (void) SetQuantumDepth(image,quantum_info,8);
                  image_depth=8;
                }

              for (y=0; y < (ssize_t) image->rows; y++)
              {
                if (logging != MagickFalse && y == 0)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  pass %d, Image Is RGB, 16-bit GRAY, or GRAY_ALPHA",
                    pass);

                p=GetVirtualPixels(image,0,y,image->columns,1, exception);

                if (p == (const Quantum *) NULL)
                  break;

                if (ping_color_type == PNG_COLOR_TYPE_GRAY)
                  {
                    (void) SetQuantumDepth(image,quantum_info,image->depth);

                    (void) ExportQuantumPixels(image,(CacheView *) NULL,
                       quantum_info,GrayQuantum,ping_pixels,exception);
                  }

                else if (ping_color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
                  {
                    if (logging != MagickFalse && y == 0)
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                           "  Writing GRAY_ALPHA PNG pixels (4)");

                    (void) ExportQuantumPixels(image,(CacheView *) NULL,
                         quantum_info,GrayAlphaQuantum,ping_pixels,
                         exception);
                  }

                else
                  {
                    (void) ExportQuantumPixels(image,(CacheView *) NULL,
                      quantum_info,IndexQuantum,ping_pixels,exception);

                    if (logging != MagickFalse && y <= 2)
                    {
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "  Writing row of non-gray pixels (4)");

                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "  ping_pixels[0]=%d,ping_pixels[1]=%d",
                          (int)ping_pixels[0],(int)ping_pixels[1]);
                    }
                  }
                png_write_row(ping,ping_pixels);

                status=SetImageProgress(image,SaveImageTag,
                  (MagickOffsetType) pass * (ssize_t) image->rows + y,
                  (MagickSizeType) num_passes * image->rows);

                if (status == MagickFalse)
                  break;
              }
            }
          }
        }
    }

  if (quantum_info != (QuantumInfo *) NULL)
    quantum_info=DestroyQuantumInfo(quantum_info);

  if (logging != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Wrote PNG image data");

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    Width: %.20g",(double) ping_width);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    Height: %.20g",(double) ping_height);

      if (mng_info->depth)
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Defined png:bit-depth: %d",mng_info->depth);
        }

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    PNG bit-depth written: %d",ping_bit_depth);

      if (mng_info->colortype)
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Defined png:color-type: %d",mng_info->colortype-1);
        }

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    PNG color-type written: %d",ping_color_type);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    PNG Interlace method: %d",ping_interlace_method);
    }
  /*
    Generate text chunks after IDAT.
  */
  if (mng_info->exclude_tEXt == MagickFalse || mng_info->exclude_zTXt == MagickFalse)
  {
    ResetImagePropertyIterator(image);
    while ((property=GetNextImageProperty(image)) != (const char *) NULL)
    {
      /* Don't write any "png:" or "jpeg:" properties; those are just for
       * "identify" or for passing through to another JPEG
       */
      if ((LocaleNCompare(property,"png:",4) == 0 ||
           LocaleNCompare(property,"jpeg:",5) == 0))
        continue;
      /* Suppress density and units if we wrote a pHYs chunk */
      if ((mng_info->exclude_pHYs == MagickFalse) && (
          ((LocaleCompare(property,"exif:ResolutionUnit") == 0) ||
           (LocaleCompare(property,"exif:XResolution") == 0) ||
           (LocaleCompare(property,"exif:YResolution") == 0) ||
           (LocaleCompare(property,"tiff:ResolutionUnit") == 0) ||
           (LocaleCompare(property,"tiff:XResolution") == 0) ||
           (LocaleCompare(property,"tiff:YResolution") == 0) ||
           (LocaleCompare(property,"density") == 0) ||
           (LocaleCompare(property,"units") == 0))))
        continue;
      /* Suppress the IM-generated date:create and date:modify */
      if ((mng_info->exclude_date != MagickFalse) &&
          (LocaleNCompare(property, "date:",5) == 0))
        continue;
      value=GetImageProperty(image,property,exception);
      if (value == (const char *) NULL)
        continue;
      Magick_png_set_text(ping,ping_info,mng_info,image_info,property,value);
    }
  }

  /* write eXIf profile */
  if (ping_have_eXIf != MagickFalse && mng_info->exclude_eXIf == MagickFalse)
    {
      ResetImageProfileIterator(image);

      for (name=GetNextImageProfile(image); name != (char *) NULL; )
      {
        if (LocaleCompare(name,"exif") == 0)
          {
            profile=GetImageProfile(image,name);

            if (profile != (StringInfo *) NULL)
              {
                png_uint_32
                  length;

                unsigned char
                  chunk[4],
                  *data;

                StringInfo
                  *ping_profile;

                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  Have eXIf profile");

                ping_profile=CloneStringInfo(profile);
                data=GetStringInfoDatum(ping_profile),
                length=(png_uint_32) GetStringInfoLength(ping_profile);

                PNGType(chunk,mng_eXIf);
                if (length < 7)
                  {
                    ping_profile=DestroyStringInfo(ping_profile);
                    break;  /* otherwise crashes */
                  }

                if (*data == 'E' && *(data+1) == 'x' && *(data+2) == 'i' &&
                    *(data+3) == 'f' && *(data+4) == '\0' && *(data+5) == '\0')
                  {
                    /* skip the "Exif\0\0" JFIF Exif Header ID */
                    length -= 6;
                    data += 6;
                  }

                LogPNGChunk(logging,chunk,length);
                (void) WriteBlobMSBULong(image,length);
                (void) WriteBlob(image,4,chunk);
                (void) WriteBlob(image,length,data);
                (void) WriteBlobMSBULong(image,crc32(crc32(0,chunk,4), data,
                  (uInt) length));
                ping_profile=DestroyStringInfo(ping_profile);
                break;
             }
         }
       name=GetNextImageProfile(image);
     }
  }

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Writing PNG end info");

  png_write_end(ping,ping_info);

  if (mng_info->need_fram != MagickFalse &&
      (int) image->dispose == BackgroundDispose)
    {
      if (mng_info->page.x || mng_info->page.y ||
          (ping_width != mng_info->page.width) ||
          (ping_height != mng_info->page.height))
        {
          unsigned char
            chunk[32];

          /*
            Write FRAM 4 with clipping boundaries followed by FRAM 1.
          */
          (void) WriteBlobMSBULong(image,27L);  /* data length=27 */
          PNGType(chunk,mng_FRAM);
          LogPNGChunk(logging,mng_FRAM,27L);
          chunk[4]=4;
          chunk[5]=0;  /* frame name separator (no name) */
          chunk[6]=1;  /* flag for changing delay, for next frame only */
          chunk[7]=0;  /* flag for changing frame timeout */
          chunk[8]=1;  /* flag for changing frame clipping for next frame */
          chunk[9]=0;  /* flag for changing frame sync_id */
          PNGLong(chunk+10,(png_uint_32) (0L)); /* temporary 0 delay */
          chunk[14]=0; /* clipping boundaries delta type */
          PNGLong(chunk+15,(png_uint_32) (mng_info->page.x)); /* left cb */
          PNGLong(chunk+19,
             (png_uint_32) (mng_info->page.x + ping_width));
          PNGLong(chunk+23,(png_uint_32) (mng_info->page.y)); /* top cb */
          PNGLong(chunk+27,
             (png_uint_32) (mng_info->page.y + ping_height));
          (void) WriteBlob(image,31,chunk);
          (void) WriteBlobMSBULong(image,crc32(0,chunk,31));
          mng_info->old_framing_mode=4;
          mng_info->framing_mode=1;
        }

      else
        mng_info->framing_mode=3;
    }
  if ((mng_info->write_mng != MagickFalse) &&
      (mng_info->need_fram == MagickFalse) && ((int) image->dispose == 3))
     png_error(ping, "Cannot convert GIF with disposal method 3 to MNG-LC");

  /*
    Free PNG resources.
  */

  png_destroy_write_struct(&ping,&ping_info);

  pixel_info=RelinquishVirtualMemory(pixel_info);

  if (ping_have_blob != MagickFalse)
     (void) CloseBlob(image);

  image_info=DestroyImageInfo(image_info);
  image=DestroyImage(image);

  /* Store bit depth actually written */
  s[0]=(char) ping_bit_depth;
  s[1]='\0';

  (void) SetImageProperty(IMimage,"png:bit-depth-written",s,exception);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  exit WriteOnePNGImage()");

#ifdef IMPNG_SETJMP_NOT_THREAD_SAFE
  UnlockSemaphoreInfo(ping_semaphore);
#endif

   /* }  for navigation to beginning of SETJMP-protected block. Revert to
    *    Throwing an Exception when an error occurs.
    */

  return(MagickTrue);
/*  End write one PNG image */

}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P N G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePNGImage() writes a Portable Network Graphics (PNG) or
%  Multiple-image Network Graphics (MNG) image file.
%
%  MNG support written by Glenn Randers-Pehrson, glennrp@image...
%
%  The format of the WritePNGImage method is:
%
%      MagickBooleanType WritePNGImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
%  Returns MagickTrue on success, MagickFalse on failure.
%
%  Communicating with the PNG encoder:
%
%  While the datastream written is always in PNG format and normally would
%  be given the "png" file extension, this method also writes the following
%  pseudo-formats which are subsets of png:
%
%    o PNG8:    An 8-bit indexed PNG datastream is written.  If the image has
%               a depth greater than 8, the depth is reduced. If transparency
%               is present, the tRNS chunk must only have values 0 and 255
%               (i.e., transparency is binary: fully opaque or fully
%               transparent).  If other values are present they will be
%               50%-thresholded to binary transparency.  If more than 256
%               colors are present, they will be quantized to the 4-4-4-1,
%               3-3-3-1, or 3-3-2-1 palette.  The underlying RGB color
%               of any resulting fully-transparent pixels is changed to
%               the image's background color.
%
%               If you want better quantization or dithering of the colors
%               or alpha than that, you need to do it before calling the
%               PNG encoder. The pixels contain 8-bit indices even if
%               they could be represented with 1, 2, or 4 bits.  Grayscale
%               images will be written as indexed PNG files even though the
%               PNG grayscale type might be slightly more efficient.  Please
%               note that writing to the PNG8 format may result in loss
%               of color and alpha data.
%
%    o PNG24:   An 8-bit per sample RGB PNG datastream is written.  The tRNS
%               chunk can be present to convey binary transparency by naming
%               one of the colors as transparent.  The only loss incurred
%               is reduction of sample depth to 8.  If the image has more
%               than one transparent color, has semitransparent pixels, or
%               has an opaque pixel with the same RGB components as the
%               transparent color, an image is not written.
%
%    o PNG32:   An 8-bit per sample RGBA PNG is written.  Partial
%               transparency is permitted, i.e., the alpha sample for
%               each pixel can have any value from 0 to 255. The alpha
%               channel is present even if the image is fully opaque.
%               The only loss in data is the reduction of the sample depth
%               to 8.
%
%    o PNG48:   A 16-bit per sample RGB PNG datastream is written.  The tRNS
%               chunk can be present to convey binary transparency by naming
%               one of the colors as transparent.  If the image has more
%               than one transparent color, has semitransparent pixels, or
%               has an opaque pixel with the same RGB components as the
%               transparent color, an image is not written.
%
%    o PNG64:   A 16-bit per sample RGBA PNG is written.  Partial
%               transparency is permitted, i.e., the alpha sample for
%               each pixel can have any value from 0 to 65535. The alpha
%               channel is present even if the image is fully opaque.
%
%    o PNG00:   A PNG that inherits its colortype and bit-depth from the input
%               image, if the input was a PNG, is written.  If these values
%               cannot be found, or if the pixels have been changed in a way
%               that makes this impossible, then "PNG00" falls back to the
%               regular "PNG" format.
%
%    o -define: For more precise control of the PNG output, you can use the
%               Image options "png:bit-depth" and "png:color-type".  These
%               can be set from the commandline with "-define" and also
%               from the application programming interfaces.  The options
%               are case-independent and are converted to lowercase before
%               being passed to this encoder.
%
%               png:color-type can be 0, 2, 3, 4, or 6.
%
%               When png:color-type is 0 (Grayscale), png:bit-depth can
%               be 1, 2, 4, 8, or 16.
%
%               When png:color-type is 2 (RGB), png:bit-depth can
%               be 8 or 16.
%
%               When png:color-type is 3 (Indexed), png:bit-depth can
%               be 1, 2, 4, or 8.  This refers to the number of bits
%               used to store the index.  The color samples always have
%               bit-depth 8 in indexed PNG files.
%
%               When png:color-type is 4 (Gray-Matte) or 6 (RGB-Matte),
%               png:bit-depth can be 8 or 16.
%
%               If the image cannot be written without loss with the
%               requested bit-depth and color-type, a PNG file will not
%               be written, a warning will be issued, and the encoder will
%               return MagickFalse.
%
%  Since image encoders should not be responsible for the "heavy lifting",
%  the user should make sure that ImageMagick has already reduced the
%  image depth and number of colors and limit transparency to binary
%  transparency prior to attempting to write the image with depth, color,
%  or transparency limitations.
%
%  Note that another definition, "png:bit-depth-written" exists, but it
%  is not intended for external use.  It is only used internally by the
%  PNG encoder to inform the JNG encoder of the depth of the alpha channel.
%
%  As of version 6.6.6 the following optimizations are always done:
%
%   o  32-bit depth is reduced to 16.
%   o  16-bit depth is reduced to 8 if all pixels contain samples whose
%      high byte and low byte are identical.
%   o  Palette is sorted to remove unused entries and to put a
%      transparent color first, if BUILD_PNG_PALETTE is defined.
%   o  Opaque matte channel is removed when writing an indexed PNG.
%   o  Grayscale images are reduced to 1, 2, or 4 bit depth if
%      this can be done without loss and a larger bit depth N was not
%      requested via the "-define png:bit-depth=N" option.
%   o  If matte channel is present but only one transparent color is
%      present, RGB+tRNS is written instead of RGBA
%   o  Opaque matte channel is removed (or added, if color-type 4 or 6
%      was requested when converting an opaque image).
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/
static MagickBooleanType WritePNGImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  MagickBooleanType
    excluding,
    logging = MagickFalse,
    status;

  MngWriteInfo
    *mng_info;

  const char
    *value;

  int
    source;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->debug != MagickFalse)
    logging=LogMagickEvent(CoderEvent,GetMagickModule(),
      "Enter WritePNGImage()");
  mng_info=(MngWriteInfo *) AcquireMagickMemory(sizeof(*mng_info));
  if (mng_info == (MngWriteInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) memset(mng_info,0,sizeof(*mng_info));
  mng_info->image=image;
  mng_info->equal_backgrounds=MagickTrue;

  /* See if user has requested a specific PNG subformat */

  mng_info->write_png8=LocaleCompare(image_info->magick,"PNG8") == 0 ?
    MagickTrue : MagickFalse;
  mng_info->write_png24=LocaleCompare(image_info->magick,"PNG24") == 0 ?
    MagickTrue : MagickFalse;
  mng_info->write_png32=LocaleCompare(image_info->magick,"PNG32") == 0 ?
    MagickTrue : MagickFalse;
  mng_info->write_png48=LocaleCompare(image_info->magick,"PNG48") == 0 ?
    MagickTrue : MagickFalse;
  mng_info->write_png64=LocaleCompare(image_info->magick,"PNG64") == 0 ?
    MagickTrue : MagickFalse;

  value=GetImageOption(image_info,"png:format");

  if (value != (char *) NULL || LocaleCompare(image_info->magick,"PNG00") == 0)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
         "  Format=%s",value);

      mng_info->write_png8 = MagickFalse;
      mng_info->write_png24 = MagickFalse;
      mng_info->write_png32 = MagickFalse;
      mng_info->write_png48 = MagickFalse;
      mng_info->write_png64 = MagickFalse;

      if (LocaleCompare(value,"png8") == 0)
        mng_info->write_png8 = MagickTrue;

      else if (LocaleCompare(value,"png24") == 0)
        mng_info->write_png24 = MagickTrue;

      else if (LocaleCompare(value,"png32") == 0)
        mng_info->write_png32 = MagickTrue;

      else if (LocaleCompare(value,"png48") == 0)
        mng_info->write_png48 = MagickTrue;

      else if (LocaleCompare(value,"png64") == 0)
        mng_info->write_png64 = MagickTrue;

      else if ((LocaleCompare(value,"png00") == 0) ||
         LocaleCompare(image_info->magick,"PNG00") == 0)
        {
          /* Retrieve png:IHDR.bit-depth-orig and png:IHDR.color-type-orig. */
          value=GetImageProperty(image,"png:IHDR.bit-depth-orig",exception);

          if (value != (char *) NULL)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "  png00 inherited bit depth=%s",value);

              if (LocaleCompare(value,"1") == 0)
                mng_info->depth = 1;

              else if (LocaleCompare(value,"2") == 0)
                mng_info->depth = 2;

              else if (LocaleCompare(value,"4") == 0)
                mng_info->depth = 4;

              else if (LocaleCompare(value,"8") == 0)
                mng_info->depth = 8;

              else if (LocaleCompare(value,"16") == 0)
                mng_info->depth = 16;
            }

          value=GetImageProperty(image,"png:IHDR.color-type-orig",exception);

          if (value != (char *) NULL)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "  png00 inherited color type=%s",value);

              if (LocaleCompare(value,"0") == 0)
                mng_info->colortype=1;
              else if (LocaleCompare(value,"2") == 0)
                mng_info->colortype=3;
              else if (LocaleCompare(value,"3") == 0)
                mng_info->colortype=4;
              else if (LocaleCompare(value,"4") == 0)
                mng_info->colortype=5;
              else if (LocaleCompare(value,"6") == 0)
                mng_info->colortype=7;
            }
        }
    }

  if (mng_info->write_png8 != MagickFalse)
    {
      mng_info->colortype=4; /* 3 */
      mng_info->depth = 8;
      image->depth = 8;
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) SetImageType(image,PaletteAlphaType,exception);
      else
        (void) SetImageType(image,PaletteType,exception);
    }

  if (mng_info->write_png24 != MagickFalse)
    {
      mng_info->colortype=3; /* 2 */
      mng_info->depth = 8;
      image->depth = 8;

      if (image->alpha_trait != UndefinedPixelTrait)
        (void) SetImageType(image,TrueColorAlphaType,exception);
      else
        (void) SetImageType(image,TrueColorType,exception);

      (void) SyncImage(image,exception);
    }

  if (mng_info->write_png32 != MagickFalse)
    {
      mng_info->colortype = /* 6 */  7;
      mng_info->depth = 8;
      image->depth = 8;
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) SetImageType(image,TrueColorAlphaType,exception);
      else
        (void) SetImageType(image,TrueColorType,exception);
      (void) SyncImage(image,exception);
    }

  if (mng_info->write_png48 != MagickFalse)
    {
      mng_info->colortype = /* 2 */ 3;
      mng_info->depth = 16;
      image->depth = 16;
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) SetImageType(image,TrueColorAlphaType,exception);
      else
        (void) SetImageType(image,TrueColorType,exception);
      (void) SyncImage(image,exception);
    }

  if (mng_info->write_png64 != MagickFalse)
    {
      mng_info->colortype = /* 6 */  7;
      mng_info->depth = 16;
      image->depth = 16;
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) SetImageType(image,TrueColorAlphaType,exception);
      else
        (void) SetImageType(image,TrueColorType,exception);
      (void) SyncImage(image,exception);
    }

  value=GetImageOption(image_info,"png:bit-depth");

  if (value != (char *) NULL)
    {
      if (LocaleCompare(value,"1") == 0)
        mng_info->depth = 1;

      else if (LocaleCompare(value,"2") == 0)
        mng_info->depth = 2;

      else if (LocaleCompare(value,"4") == 0)
        mng_info->depth = 4;

      else if (LocaleCompare(value,"8") == 0)
        mng_info->depth = 8;

      else if (LocaleCompare(value,"16") == 0)
        mng_info->depth = 16;

      else
        (void) ThrowMagickException(exception,GetMagickModule(),CoderWarning,
          "ignoring invalid defined png:bit-depth","=%s",value);

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  png:bit-depth=%d was defined.\n",mng_info->depth);
    }

  value=GetImageOption(image_info,"png:color-type");

  if (value != (char *) NULL)
    {
      /* We must store colortype+1 because 0 is a valid colortype */
      if (LocaleCompare(value,"0") == 0)
        mng_info->colortype = 1;

      else if (LocaleCompare(value,"1") == 0)
        mng_info->colortype = 2;

      else if (LocaleCompare(value,"2") == 0)
        mng_info->colortype = 3;

      else if (LocaleCompare(value,"3") == 0)
        mng_info->colortype = 4;

      else if (LocaleCompare(value,"4") == 0)
        mng_info->colortype = 5;

      else if (LocaleCompare(value,"6") == 0)
        mng_info->colortype = 7;

      else
        (void) ThrowMagickException(exception,GetMagickModule(),CoderWarning,
          "ignoring invalid defined png:color-type","=%s",value);

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  png:color-type=%d was defined.\n",
          mng_info->colortype-1);
    }

  /* Check for chunks to be excluded:
   *
   * The default is to not exclude any known chunks except for any
   * listed in the "unused_chunks" array, above.
   *
   * Chunks can be listed for exclusion via a "png:exclude-chunk"
   * define (in the image properties or in the image artifacts)
   * or via a mng_info member.  For convenience, in addition
   * to or instead of a comma-separated list of chunks, the
   * "exclude-chunk" string can be simply "all" or "none".
   *
   * Note that the "-strip" option provides a convenient way of
   * doing the equivalent of
   *
   *    -define png:exclude-chunk="bKGD,caNv,cHRM,eXIf,gAMA,iCCP,
   *            iTXt,pHYs,sRGB,tEXt,zCCP,zTXt,date"
   *
   * The exclude-chunk define takes priority over the mng_info.
   *
   * A "png:include-chunk" define takes  priority over both the
   * mng_info and the "png:exclude-chunk" define.  Like the
   * "exclude-chunk" string, it can define "all" or "none" as
   * well as a comma-separated list.  Chunks that are unknown to
   * ImageMagick are always excluded, regardless of their "copy-safe"
   * status according to the PNG specification, and even if they
   * appear in the "include-chunk" list. Such defines appearing among
   * the image options take priority over those found among the image
   * artifacts.
   *
   * Finally, all chunks listed in the "unused_chunks" array are
   * automatically excluded, regardless of the other instructions
   * or lack thereof.
   *
   * if you exclude sRGB but not gAMA (recommended), then sRGB chunk
   * will not be written and the gAMA chunk will only be written if it
   * is not between .45 and .46, or approximately (1.0/2.2).
   *
   * If you exclude tRNS and the image has transparency, the colortype
   * is forced to be 4 or 6 (GRAY_ALPHA or RGB_ALPHA).
   *
   * The -strip option causes StripImage() to set the png:include-chunk
   * artifact to "none,trns,gama".
   */

  mng_info->exclude_bKGD=MagickFalse;
  mng_info->exclude_caNv=MagickFalse;
  mng_info->exclude_cHRM=MagickFalse;
  mng_info->exclude_date=MagickFalse;
  mng_info->exclude_eXIf=MagickFalse;
  mng_info->exclude_EXIF=MagickFalse;
  mng_info->exclude_gAMA=MagickFalse;
  mng_info->exclude_iCCP=MagickFalse;
  mng_info->exclude_oFFs=MagickFalse;
  mng_info->exclude_pHYs=MagickFalse;
  mng_info->exclude_sRGB=MagickFalse;
  mng_info->exclude_tEXt=MagickFalse;
  mng_info->exclude_tIME=MagickFalse;
  mng_info->exclude_tRNS=MagickFalse;
  mng_info->exclude_zCCP=MagickFalse;
  mng_info->exclude_zTXt=MagickFalse;

  mng_info->preserve_colormap=MagickFalse;

  value=GetImageOption(image_info,"png:preserve-colormap");
  if (value == NULL)
     value=GetImageArtifact(image,"png:preserve-colormap");
  if (value != NULL)
     mng_info->preserve_colormap=MagickTrue;

  mng_info->preserve_iCCP=MagickFalse;

  value=GetImageOption(image_info,"png:preserve-iCCP");
  if (value == NULL)
     value=GetImageArtifact(image,"png:preserve-iCCP");
  if (value != NULL)
     mng_info->preserve_iCCP=MagickTrue;

  /* These compression-level, compression-strategy, and compression-filter
   * defines take precedence over values from the -quality option.
   */
  value=GetImageOption(image_info,"png:compression-level");
  if (value == NULL)
     value=GetImageArtifact(image,"png:compression-level");
  if (value != NULL)
  {
      /* We have to add 1 to everything because 0 is a valid input,
       * and we want to use 0 (the default) to mean undefined.
       */
      if (LocaleCompare(value,"0") == 0)
        mng_info->compression_level = 1;

      else if (LocaleCompare(value,"1") == 0)
        mng_info->compression_level = 2;

      else if (LocaleCompare(value,"2") == 0)
        mng_info->compression_level = 3;

      else if (LocaleCompare(value,"3") == 0)
        mng_info->compression_level = 4;

      else if (LocaleCompare(value,"4") == 0)
        mng_info->compression_level = 5;

      else if (LocaleCompare(value,"5") == 0)
        mng_info->compression_level = 6;

      else if (LocaleCompare(value,"6") == 0)
        mng_info->compression_level = 7;

      else if (LocaleCompare(value,"7") == 0)
        mng_info->compression_level = 8;

      else if (LocaleCompare(value,"8") == 0)
        mng_info->compression_level = 9;

      else if (LocaleCompare(value,"9") == 0)
        mng_info->compression_level = 10;

      else
        (void) ThrowMagickException(exception,GetMagickModule(),CoderWarning,
          "ignoring invalid defined png:compression-level","=%s",value);
    }

  value=GetImageOption(image_info,"png:compression-strategy");
  if (value == NULL)
     value=GetImageArtifact(image,"png:compression-strategy");
  if (value != NULL)
  {
      if (LocaleCompare(value,"0") == 0)
        mng_info->compression_strategy = Z_DEFAULT_STRATEGY+1;

      else if (LocaleCompare(value,"1") == 0)
        mng_info->compression_strategy = Z_FILTERED+1;

      else if (LocaleCompare(value,"2") == 0)
        mng_info->compression_strategy = Z_HUFFMAN_ONLY+1;

      else if (LocaleCompare(value,"3") == 0)
#ifdef Z_RLE  /* Z_RLE was added to zlib-1.2.0 */
        mng_info->compression_strategy = Z_RLE+1;
#else
        mng_info->write_png_compression_strategy = Z_DEFAULT_STRATEGY+1;
#endif

      else if (LocaleCompare(value,"4") == 0)
#ifdef Z_FIXED  /* Z_FIXED was added to zlib-1.2.2.2 */
        mng_info->compression_strategy = Z_FIXED+1;
#else
        mng_info->write_png_compression_strategy = Z_DEFAULT_STRATEGY+1;
#endif

      else
        (void) ThrowMagickException(exception,GetMagickModule(),CoderWarning,
          "ignoring invalid defined png:compression-strategy","=%s",value);
    }

  value=GetImageOption(image_info,"png:compression-filter");
  if (value == NULL)
     value=GetImageArtifact(image,"png:compression-filter");
  if (value != NULL)
  {
      /* To do: combinations of filters allowed by libpng
       * masks 0x08 through 0xf8
       *
       * Implement this as a comma-separated list of 0,1,2,3,4,5
       * where 5 is a special case meaning PNG_ALL_FILTERS.
       */

      mng_info->compression_filter = StringToUnsignedLong(value)+1;
      if (mng_info->compression_filter > 6)
        (void) ThrowMagickException(exception,GetMagickModule(),CoderWarning,
          "ignoring invalid defined png:compression-filter","=%s",value);
  }

  for (source=0; source<8; source++)
  {
    value = (const char *) NULL;

    switch(source)
    {
      case 0:
        value=GetImageOption(image_info,"png:exclude-chunks");
        break;
      case 1:
        value=GetImageArtifact(image,"png:exclude-chunks");
        break;
      case 2:
        value=GetImageOption(image_info,"png:exclude-chunk");
        break;
      case 3:
        value=GetImageArtifact(image,"png:exclude-chunk");
        break;
      case 4:
        value=GetImageOption(image_info,"png:include-chunks");
        break;
      case 5:
        value=GetImageArtifact(image,"png:include-chunks");
        break;
      case 6:
        value=GetImageOption(image_info,"png:include-chunk");
        break;
      case 7:
        value=GetImageArtifact(image,"png:include-chunk");
        break;
    }

    if (value == NULL)
       continue;

    if (source < 4)
      excluding = MagickTrue;
    else
      excluding = MagickFalse;

    if (logging != MagickFalse)
      {
        if (source == 0 || source == 2)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  png:exclude-chunk=%s found in image options.\n", value);
        else if (source == 1 || source == 3)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  png:exclude-chunk=%s found in image artifacts.\n", value);
        else if (source == 4 || source == 6)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  png:include-chunk=%s found in image options.\n", value);
        else /* if (source == 5 || source == 7) */
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  png:include-chunk=%s found in image artifacts.\n", value);
      }

    if (IsOptionMember("all",value) != MagickFalse)
      {
        mng_info->exclude_bKGD=excluding;
        mng_info->exclude_caNv=excluding;
        mng_info->exclude_cHRM=excluding;
        mng_info->exclude_date=excluding;
        mng_info->exclude_EXIF=excluding;
        mng_info->exclude_eXIf=excluding;
        mng_info->exclude_gAMA=excluding;
        mng_info->exclude_iCCP=excluding;
        mng_info->exclude_oFFs=excluding;
        mng_info->exclude_pHYs=excluding;
        mng_info->exclude_sRGB=excluding;
        mng_info->exclude_tEXt=excluding;
        mng_info->exclude_tIME=excluding;
        mng_info->exclude_tRNS=excluding;
        mng_info->exclude_zCCP=excluding;
        mng_info->exclude_zTXt=excluding;
      }

    if (IsOptionMember("none",value) != MagickFalse)
      {
        mng_info->exclude_bKGD=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_caNv=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_cHRM=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_date=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_eXIf=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_EXIF=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_gAMA=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_iCCP=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_oFFs=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_pHYs=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_sRGB=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_tEXt=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_tIME=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_tRNS=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_zCCP=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
        mng_info->exclude_zTXt=excluding != MagickFalse ? MagickFalse :
          MagickTrue;
      }

    if (IsOptionMember("bkgd",value) != MagickFalse)
      mng_info->exclude_bKGD=excluding;

    if (IsOptionMember("caNv",value) != MagickFalse)
      mng_info->exclude_caNv=excluding;

    if (IsOptionMember("chrm",value) != MagickFalse)
      mng_info->exclude_cHRM=excluding;

    if (IsOptionMember("date",value) != MagickFalse)
      mng_info->exclude_date=excluding;

    if (IsOptionMember("exif",value) != MagickFalse)
      {
        mng_info->exclude_EXIF=excluding;
        mng_info->exclude_eXIf=excluding;
      }

    if (IsOptionMember("gama",value) != MagickFalse)
      mng_info->exclude_gAMA=excluding;

    if (IsOptionMember("iccp",value) != MagickFalse)
      mng_info->exclude_iCCP=excluding;

    if (IsOptionMember("offs",value) != MagickFalse)
      mng_info->exclude_oFFs=excluding;

    if (IsOptionMember("phys",value) != MagickFalse)
      mng_info->exclude_pHYs=excluding;

    if (IsOptionMember("srgb",value) != MagickFalse)
      mng_info->exclude_sRGB=excluding;

    if (IsOptionMember("text",value) != MagickFalse)
      mng_info->exclude_tEXt=excluding;

    if (IsOptionMember("time",value) != MagickFalse)
      mng_info->exclude_tIME=excluding;

    if (IsOptionMember("trns",value) != MagickFalse)
      mng_info->exclude_tRNS=excluding;

    if (IsOptionMember("zccp",value) != MagickFalse)
      mng_info->exclude_zCCP=excluding;

    if (IsOptionMember("ztxt",value) != MagickFalse)
      mng_info->exclude_zTXt=excluding;
  }

  if (logging != MagickFalse)
  {
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Chunks to be excluded from the output png:");
    if (mng_info->exclude_bKGD != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    bKGD");
    if (mng_info->exclude_caNv != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    caNv");
    if (mng_info->exclude_cHRM != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    cHRM");
    if (mng_info->exclude_date != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    date");
    if (mng_info->exclude_EXIF != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    EXIF");
    if (mng_info->exclude_eXIf != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    eXIf");
    if (mng_info->exclude_gAMA != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    gAMA");
    if (mng_info->exclude_iCCP != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    iCCP");
    if (mng_info->exclude_oFFs != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    oFFs");
    if (mng_info->exclude_pHYs != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    pHYs");
    if (mng_info->exclude_sRGB != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    sRGB");
    if (mng_info->exclude_tEXt != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    tEXt");
    if (mng_info->exclude_tIME != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    tIME");
    if (mng_info->exclude_tRNS != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    tRNS");
    if (mng_info->exclude_zCCP != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    zCCP");
    if (mng_info->exclude_zTXt != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    zTXt");
  }

  mng_info->need_blob = MagickTrue;

  status=WriteOnePNGImage(mng_info,image_info,image,exception);

  mng_info=(MngWriteInfo *) RelinquishMagickMemory(mng_info);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit WritePNGImage()");

  return(status);
}

#if defined(JNG_SUPPORTED)

/* Write one JNG image */
static MagickBooleanType WriteOneJNGImage(MngWriteInfo *mng_info,
   const ImageInfo *image_info,Image *image,ExceptionInfo *exception)
{
  Image
    *jpeg_image;

  ImageInfo
    *jpeg_image_info;

  MagickBooleanType
    logging = MagickFalse,
    status;

  size_t
    length;

  unsigned char
    *blob,
    chunk[80],
    *p;

  unsigned int
    jng_alpha_compression_method,
    jng_alpha_sample_depth,
    jng_color_type,
    transparent;

  size_t
    jng_alpha_quality,
    jng_quality;

  logging=LogMagickEvent(CoderEvent,GetMagickModule(),
    "  Enter WriteOneJNGImage()");

  if ((image->columns > 65500U) || (image->rows > 65500U))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");

  blob=(unsigned char *) NULL;
  jpeg_image=(Image *) NULL;
  jpeg_image_info=(ImageInfo *) NULL;
  length=0;

  status=MagickTrue;
  transparent=image_info->type==GrayscaleAlphaType ||
     image_info->type==TrueColorAlphaType ||
     image->alpha_trait != UndefinedPixelTrait;

  jng_alpha_sample_depth = 0;

  jng_quality=image_info->quality == 0UL ? 75UL : image_info->quality%1000;

  jng_alpha_compression_method=(image->compression==JPEGCompression ||
    image_info->compression==JPEGCompression) ? 8 : 0;

  jng_alpha_quality=image_info->quality == 0UL ? 75UL :
      image_info->quality;

  if (jng_alpha_quality >= 1000)
    jng_alpha_quality /= 1000;

  length=0;

  if (transparent != 0)
    {
      jng_color_type=14;

      /* Create JPEG blob, image, and image_info */
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  Creating jpeg_image_info for alpha.");

      jpeg_image_info=(ImageInfo *) CloneImageInfo(image_info);

      if (jpeg_image_info == (ImageInfo *) NULL)
        {
          jpeg_image_info=DestroyImageInfo(jpeg_image_info);
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        }

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  Creating jpeg_image.");

      jpeg_image=SeparateImage(image,AlphaChannel,exception);
      if (jpeg_image == (Image *) NULL)
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      (void) CopyMagickString(jpeg_image->magick,"JPEG",MagickPathExtent);
      jpeg_image->alpha_trait=UndefinedPixelTrait;
      jpeg_image->quality=jng_alpha_quality;
      jpeg_image_info->type=GrayscaleType;
      (void) SetImageType(jpeg_image,GrayscaleType,exception);
      (void) AcquireUniqueFilename(jpeg_image->filename);
      (void) FormatLocaleString(jpeg_image_info->filename,MagickPathExtent,
        "%s",jpeg_image->filename);
    }
  else
    {
      jng_alpha_compression_method=0;
      jng_color_type=10;
      jng_alpha_sample_depth=0;
    }

  /* To do: check bit depth of PNG alpha channel */

  /* Check if image is grayscale. */
  if ((image_info->type != TrueColorAlphaType) &&
      (image_info->type != TrueColorType))
    {
      if (IdentifyImageCoderGray(image,exception) != MagickFalse)
        jng_color_type-=2;
    }

  if (logging != MagickFalse)
    {
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    JNG Quality           = %d",(int) jng_quality);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    JNG Color Type        = %d",jng_color_type);
        if (transparent != 0)
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    JNG Alpha Compression = %d",jng_alpha_compression_method);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    JNG Alpha Depth       = %d",jng_alpha_sample_depth);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    JNG Alpha Quality     = %d",(int) jng_alpha_quality);
          }
    }

  if (transparent != 0)
    {
      if (jng_alpha_compression_method==0)
        {
          const char
            *value;

          /* Encode alpha as a grayscale PNG blob */
          status=OpenBlob(jpeg_image_info,jpeg_image,WriteBinaryBlobMode,
            exception);
          if (status == MagickFalse)
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");

          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Creating PNG blob.");

          (void) CopyMagickString(jpeg_image_info->magick,"PNG",
             MagickPathExtent);
          (void) CopyMagickString(jpeg_image->magick,"PNG",MagickPathExtent);
          jpeg_image_info->interlace=NoInterlace;

          /* Exclude all ancillary chunks */
          (void) SetImageArtifact(jpeg_image,"png:exclude-chunks","all");

          blob=(unsigned char *) ImageToBlob(jpeg_image_info,jpeg_image,
            &length,exception);

          /* Retrieve sample depth used */
          value=GetImageProperty(jpeg_image,"png:bit-depth-written",exception);
          if (value != (char *) NULL)
            jng_alpha_sample_depth= (unsigned int) value[0];
        }
      else
        {
          /* Encode alpha as a grayscale JPEG blob */

          status=OpenBlob(jpeg_image_info,jpeg_image,WriteBinaryBlobMode,
            exception);
          if (status == MagickFalse)
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");

          (void) CopyMagickString(jpeg_image_info->magick,"JPEG",
            MagickPathExtent);
          (void) CopyMagickString(jpeg_image->magick,"JPEG",MagickPathExtent);
          jpeg_image_info->interlace=NoInterlace;
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Creating blob.");
          blob=(unsigned char *) ImageToBlob(jpeg_image_info,jpeg_image,
            &length,exception);
          if (blob == (unsigned char *) NULL)
            {
              jpeg_image=DestroyImage(jpeg_image);
              jpeg_image_info=DestroyImageInfo(jpeg_image_info);
              return(MagickFalse);
            }
          jng_alpha_sample_depth=8;

          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Successfully read jpeg_image into a blob, length=%.20g.",
              (double) length);

        }
      /* Destroy JPEG image and image_info */
      jpeg_image=DestroyImage(jpeg_image);
      (void) RelinquishUniqueFileResource(jpeg_image_info->filename);
      jpeg_image_info=DestroyImageInfo(jpeg_image_info);
    }

  /* Write JHDR chunk */
  (void) WriteBlobMSBULong(image,16L);  /* chunk data length=16 */
  PNGType(chunk,mng_JHDR);
  LogPNGChunk(logging,mng_JHDR,16L);
  PNGLong(chunk+4,(png_uint_32) image->columns);
  PNGLong(chunk+8,(png_uint_32) image->rows);
  chunk[12]=(unsigned char) jng_color_type;
  chunk[13]=8;  /* sample depth */
  chunk[14]=8; /*jng_image_compression_method */
  chunk[15]=(unsigned char) (image_info->interlace == NoInterlace ? 0 : 8);
  chunk[16]=(unsigned char) jng_alpha_sample_depth;
  chunk[17]=(unsigned char) jng_alpha_compression_method;
  chunk[18]=0; /*jng_alpha_filter_method */
  chunk[19]=0; /*jng_alpha_interlace_method */
  (void) WriteBlob(image,20,chunk);
  (void) WriteBlobMSBULong(image,crc32(0,chunk,20));
  if (logging != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG width:%15lu",(unsigned long) image->columns);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG height:%14lu",(unsigned long) image->rows);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG color type:%10d",jng_color_type);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG sample depth:%8d",8);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG compression:%9d",8);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG interlace:%11d",0);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG alpha depth:%9d",jng_alpha_sample_depth);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG alpha compression:%3d",jng_alpha_compression_method);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG alpha filter:%8d",0);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG alpha interlace:%5d",0);
    }

  /*
     Write leading ancillary chunks
  */

  if (transparent != 0)
  {
    /*
      Write JNG bKGD chunk
    */

    unsigned char
      blue,
      green,
      red;

    ssize_t
      num_bytes;

    if (jng_color_type == 8 || jng_color_type == 12)
      num_bytes=6L;
    else
      num_bytes=10L;
    (void) WriteBlobMSBULong(image,(size_t) (num_bytes-4L));
    PNGType(chunk,mng_bKGD);
    LogPNGChunk(logging,mng_bKGD,(size_t) (num_bytes-4L));
    red=ScaleQuantumToChar((Quantum) image->background_color.red);
    green=ScaleQuantumToChar((Quantum) image->background_color.green);
    blue=ScaleQuantumToChar((Quantum) image->background_color.blue);
    *(chunk+4)=0;
    *(chunk+5)=red;
    *(chunk+6)=0;
    *(chunk+7)=green;
    *(chunk+8)=0;
    *(chunk+9)=blue;
    (void) WriteBlob(image,(size_t) num_bytes,chunk);
    (void) WriteBlobMSBULong(image,crc32(0,chunk,(uInt) num_bytes));
  }

  if ((image->colorspace == sRGBColorspace || image->rendering_intent))
    {
      /*
        Write JNG sRGB chunk
      */
      (void) WriteBlobMSBULong(image,1L);
      PNGType(chunk,mng_sRGB);
      LogPNGChunk(logging,mng_sRGB,1L);

      if (image->rendering_intent != UndefinedIntent)
        chunk[4]=(unsigned char)
          Magick_RenderingIntent_to_PNG_RenderingIntent(
          (image->rendering_intent));

      else
        chunk[4]=(unsigned char)
          Magick_RenderingIntent_to_PNG_RenderingIntent(
          (PerceptualIntent));

      (void) WriteBlob(image,5,chunk);
      (void) WriteBlobMSBULong(image,crc32(0,chunk,5));
    }
  else
    {
      if (image->gamma != 0.0)
        {
          /*
             Write JNG gAMA chunk
          */
          (void) WriteBlobMSBULong(image,4L);
          PNGType(chunk,mng_gAMA);
          LogPNGChunk(logging,mng_gAMA,4L);
          PNGLong(chunk+4,(png_uint_32) (100000*image->gamma+0.5));
          (void) WriteBlob(image,8,chunk);
          (void) WriteBlobMSBULong(image,crc32(0,chunk,8));
        }

      if ((mng_info->equal_chrms == MagickFalse) &&
          (image->chromaticity.red_primary.x != 0.0))
        {
          PrimaryInfo
            primary;

          /*
             Write JNG cHRM chunk
          */
          (void) WriteBlobMSBULong(image,32L);
          PNGType(chunk,mng_cHRM);
          LogPNGChunk(logging,mng_cHRM,32L);
          primary=image->chromaticity.white_point;
          PNGLong(chunk+4,(png_uint_32) (100000*primary.x+0.5));
          PNGLong(chunk+8,(png_uint_32) (100000*primary.y+0.5));
          primary=image->chromaticity.red_primary;
          PNGLong(chunk+12,(png_uint_32) (100000*primary.x+0.5));
          PNGLong(chunk+16,(png_uint_32) (100000*primary.y+0.5));
          primary=image->chromaticity.green_primary;
          PNGLong(chunk+20,(png_uint_32) (100000*primary.x+0.5));
          PNGLong(chunk+24,(png_uint_32) (100000*primary.y+0.5));
          primary=image->chromaticity.blue_primary;
          PNGLong(chunk+28,(png_uint_32) (100000*primary.x+0.5));
          PNGLong(chunk+32,(png_uint_32) (100000*primary.y+0.5));
          (void) WriteBlob(image,36,chunk);
          (void) WriteBlobMSBULong(image,crc32(0,chunk,36));
        }
    }

  if (image->resolution.x && image->resolution.y &&
      mng_info->equal_physs == MagickFalse)
    {
      /*
         Write JNG pHYs chunk
      */
      (void) WriteBlobMSBULong(image,9L);
      PNGType(chunk,mng_pHYs);
      LogPNGChunk(logging,mng_pHYs,9L);
      if (image->units == PixelsPerInchResolution)
        {
          PNGLong(chunk+4,(png_uint_32)
            (image->resolution.x*100.0/2.54+0.5));

          PNGLong(chunk+8,(png_uint_32)
            (image->resolution.y*100.0/2.54+0.5));

          chunk[12]=1;
        }

      else
        {
          if (image->units == PixelsPerCentimeterResolution)
            {
              PNGLong(chunk+4,(png_uint_32)
                (image->resolution.x*100.0+0.5));

              PNGLong(chunk+8,(png_uint_32)
                (image->resolution.y*100.0+0.5));

              chunk[12]=1;
            }

          else
            {
              PNGLong(chunk+4,(png_uint_32) (image->resolution.x+0.5));
              PNGLong(chunk+8,(png_uint_32) (image->resolution.y+0.5));
              chunk[12]=0;
            }
        }
      (void) WriteBlob(image,13,chunk);
      (void) WriteBlobMSBULong(image,crc32(0,chunk,13));
    }

  if ((mng_info->write_mng == MagickFalse) && (image->page.x || image->page.y))
    {
      /*
         Write JNG oFFs chunk
      */
      (void) WriteBlobMSBULong(image,9L);
      PNGType(chunk,mng_oFFs);
      LogPNGChunk(logging,mng_oFFs,9L);
      PNGsLong(chunk+4,(png_int_32) (image->page.x));
      PNGsLong(chunk+8,(png_int_32) (image->page.y));
      chunk[12]=0;
      (void) WriteBlob(image,13,chunk);
      (void) WriteBlobMSBULong(image,crc32(0,chunk,13));
    }

  if (transparent != 0)
    {
      if (jng_alpha_compression_method==0)
        {
          ssize_t
            i;

          size_t
            len;

          /* Write IDAT chunk header */
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Write IDAT chunks from blob, length=%.20g.",(double)
              length);

          /* Copy IDAT chunks */
          len=0;
          p=blob+8;
          for (i=8; i<(ssize_t) length; i+=(ssize_t) len+12)
          {
            len=(((unsigned int) *(p    ) & 0xff) << 24) +
                (((unsigned int) *(p + 1) & 0xff) << 16) +
                (((unsigned int) *(p + 2) & 0xff) <<  8) +
                (((unsigned int) *(p + 3) & 0xff)      ) ;
            p+=(ptrdiff_t) 4;

            if (*(p)==73 && *(p+1)==68 && *(p+2)==65 && *(p+3)==84) /* IDAT */
              {
                /* Found an IDAT chunk. */
                (void) WriteBlobMSBULong(image,len);
                LogPNGChunk(logging,mng_IDAT,len);
                (void) WriteBlob(image,len+4,p);
                (void) WriteBlobMSBULong(image, crc32(0,p,(uInt) len+4));
              }

            else
              {
                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "    Skipping %c%c%c%c chunk, length=%.20g.",
                    *(p),*(p+1),*(p+2),*(p+3),(double) len);
              }
            p+=(ptrdiff_t) (8+len);
          }
        }
      else if (length != 0)
        {
          /* Write JDAA chunk header */
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Write JDAA chunk, length=%.20g.",(double) length);
          (void) WriteBlobMSBULong(image,(size_t) length);
          PNGType(chunk,mng_JDAA);
          LogPNGChunk(logging,mng_JDAA,length);
          /* Write JDAT chunk(s) data */
          (void) WriteBlob(image,4,chunk);
          (void) WriteBlob(image,length,blob);
          (void) WriteBlobMSBULong(image,crc32(crc32(0,chunk,4),blob,
             (uInt) length));
        }
      blob=(unsigned char *) RelinquishMagickMemory(blob);
    }

  /* Encode image as a JPEG blob */
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Creating jpeg_image_info.");
  jpeg_image_info=(ImageInfo *) CloneImageInfo(image_info);
  if (jpeg_image_info == (ImageInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Creating jpeg_image.");

  jpeg_image=CloneImage(image,0,0,MagickTrue,exception);
  if (jpeg_image == (Image *) NULL)
    {
      jpeg_image_info=DestroyImageInfo(jpeg_image_info);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  (void) CopyMagickString(jpeg_image->magick,"JPEG",MagickPathExtent);

  (void) AcquireUniqueFilename(jpeg_image->filename);
  (void) FormatLocaleString(jpeg_image_info->filename,MagickPathExtent,"%s",
    jpeg_image->filename);

  status=OpenBlob(jpeg_image_info,jpeg_image,WriteBinaryBlobMode,
    exception);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Created jpeg_image, %.20g x %.20g.",(double) jpeg_image->columns,
      (double) jpeg_image->rows);

  if (status == MagickFalse)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");

  if (jng_color_type == 8 || jng_color_type == 12)
    jpeg_image_info->type=GrayscaleType;

  jpeg_image_info->quality=jng_quality;
  jpeg_image->quality=jng_quality;
  (void) CopyMagickString(jpeg_image_info->magick,"JPEG",MagickPathExtent);
  (void) CopyMagickString(jpeg_image->magick,"JPEG",MagickPathExtent);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Creating blob.");

  blob=(unsigned char *) ImageToBlob(jpeg_image_info,jpeg_image,&length,
    exception);

  if (logging != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Successfully read jpeg_image into a blob, length=%.20g.",
        (double) length);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Write JDAT chunk, length=%.20g.",(double) length);
    }

  /* Write JDAT chunk(s) */
  (void) WriteBlobMSBULong(image,(size_t) length);
  PNGType(chunk,mng_JDAT);
  LogPNGChunk(logging,mng_JDAT,length);
  (void) WriteBlob(image,4,chunk);
  (void) WriteBlob(image,length,blob);
  (void) WriteBlobMSBULong(image,crc32(crc32(0,chunk,4),blob,(uInt) length));

  jpeg_image=DestroyImage(jpeg_image);
  (void) RelinquishUniqueFileResource(jpeg_image_info->filename);
  jpeg_image_info=DestroyImageInfo(jpeg_image_info);
  blob=(unsigned char *) RelinquishMagickMemory(blob);

  /* Write IEND chunk */
  (void) WriteBlobMSBULong(image,0L);
  PNGType(chunk,mng_IEND);
  LogPNGChunk(logging,mng_IEND,0);
  (void) WriteBlob(image,4,chunk);
  (void) WriteBlobMSBULong(image,crc32(0,chunk,4));

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  exit WriteOneJNGImage()");

  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e J N G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteJNGImage() writes a JPEG Network Graphics (JNG) image file.
%
%  JNG support written by Glenn Randers-Pehrson, glennrp@image...
%
%  The format of the WriteJNGImage method is:
%
%      MagickBooleanType WriteJNGImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/
static MagickBooleanType WriteJNGImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  MagickBooleanType
    logging = MagickFalse,
    status;

  MngWriteInfo
    *mng_info;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->debug != MagickFalse)
    logging=LogMagickEvent(CoderEvent,GetMagickModule(),
      "Enter WriteJNGImage()");
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  if ((image->columns > 65535UL) || (image->rows > 65535UL))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  mng_info=(MngWriteInfo *) AcquireMagickMemory(sizeof(*mng_info));
  if (mng_info == (MngWriteInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) memset(mng_info,0,sizeof(*mng_info));
  mng_info->image=image;

  (void) WriteBlob(image,8,(const unsigned char *) "\213JNG\r\n\032\n");

  status=WriteOneJNGImage(mng_info,image_info,image,exception);

  mng_info=(MngWriteInfo *) RelinquishMagickMemory(mng_info);

  (void) CloseBlob(image);

  (void) CatchImageException(image);
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit WriteJNGImage()");
  return(status);
}
#endif

static MagickBooleanType WriteMNGImage(const ImageInfo *image_info,Image *image,ExceptionInfo *exception)
{
  Image
    *next_image;

  MagickBooleanType
    status,
    write_jng,
    write_mng;

  volatile MagickBooleanType
    logging = MagickFalse;

  MngWriteInfo
    *mng_info;

  int
    image_count,
    need_iterations,
    need_matte;

  volatile int
    need_local_plte,
    all_images_are_gray,
    need_defi,
    use_global_plte;

  unsigned char
    chunk[800];

  volatile size_t
    scene;

  size_t
    final_delay=0,
    number_scenes,
    initial_delay;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->debug != MagickFalse)
    logging=LogMagickEvent(CoderEvent,GetMagickModule(),
      "Enter WriteMNGImage()");
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  mng_info=(MngWriteInfo *) AcquireMagickMemory(sizeof(*mng_info));
  if (mng_info == (MngWriteInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) memset(mng_info,0,sizeof(*mng_info));
  mng_info->image=image;
  write_mng=LocaleCompare(image_info->magick,"MNG") == 0 ?
    MagickTrue : MagickFalse;

  /*
   * See if user has requested a specific PNG subformat to be used
   * for all of the PNGs in the MNG being written, e.g.,
   *
   *    convert *.png png8:animation.mng
   *
   * To do: check -define png:bit_depth and png:color_type as well,
   * or perhaps use mng:bit_depth and mng:color_type instead for
   * global settings.
   */

  mng_info->write_png8=LocaleCompare(image_info->magick,"PNG8") == 0 ?
    MagickTrue : MagickFalse;
  mng_info->write_png24=LocaleCompare(image_info->magick,"PNG24") == 0 ?
    MagickTrue : MagickFalse;
  mng_info->write_png32=LocaleCompare(image_info->magick,"PNG32") == 0 ?
    MagickTrue : MagickFalse;

  write_jng=MagickFalse;
  if (image_info->compression == JPEGCompression)
    write_jng=MagickTrue;

  mng_info->adjoin=(image_info->adjoin != MagickFalse) &&
    (write_mng != MagickFalse) &&
    (GetNextImageInList(image) != (Image *) NULL) ?
    MagickTrue : MagickFalse;

  if (logging != MagickFalse)
    {
      /* Log some info about the input */
      Image
        *p;

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Checking input image(s)\n"
        "    Image_info depth: %.20g,    Type: %d",
        (double) image_info->depth, image_info->type);

      scene=0;
      for (p=image; p != (Image *) NULL; p=GetNextImageInList(p))
      {

        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "    Scene: %.20g\n,   Image depth: %.20g",
           (double) scene++, (double) p->depth);

        if (p->alpha_trait != UndefinedPixelTrait)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      Matte: True");

        else
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      Matte: False");

        if (p->storage_class == PseudoClass)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      Storage class: PseudoClass");

        else
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      Storage class: DirectClass");

        if (p->colors)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      Number of colors: %.20g",(double) p->colors);

        else
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      Number of colors: unspecified");

        if (mng_info->adjoin == MagickFalse)
          break;
      }
    }

  use_global_plte=MagickFalse;
  all_images_are_gray=MagickFalse;
  need_local_plte=MagickTrue;
  need_defi=MagickFalse;
  need_matte=MagickFalse;
  mng_info->framing_mode=1;
  mng_info->old_framing_mode=1;

  if (write_mng != MagickFalse)
    {
      unsigned int
        need_geom;

      unsigned short
        red,
        green,
        blue;

      const char *
        option;

      if (image_info->page != (char *) NULL)
        {
          /*
            Determine image bounding box.
          */
          SetGeometry(image,&mng_info->page);
          (void) ParseMetaGeometry(image_info->page,&mng_info->page.x,
            &mng_info->page.y,&mng_info->page.width,&mng_info->page.height);
        }
      mng_info->page=image->page;
      need_geom=MagickTrue;
      if (mng_info->page.width || mng_info->page.height)
         need_geom=MagickFalse;
      /*
        Check all the scenes.
      */
      initial_delay=image->delay;
      need_iterations=MagickFalse;
      mng_info->equal_chrms=image->chromaticity.red_primary.x != 0.0 ?
        MagickTrue : MagickFalse;
      mng_info->equal_physs=MagickTrue,
      mng_info->equal_gammas=MagickTrue;
      mng_info->equal_srgbs=MagickTrue;
      mng_info->equal_backgrounds=MagickTrue;
      image_count=0;
      all_images_are_gray=MagickTrue;
      mng_info->equal_palettes=MagickFalse;
      need_local_plte=MagickFalse;
      for (next_image=image; next_image != (Image *) NULL; )
      {
        if (need_geom)
          {
            if ((next_image->columns+(size_t) next_image->page.x) > mng_info->page.width)
              mng_info->page.width=next_image->columns+(size_t) next_image->page.x;

            if ((next_image->rows+(size_t) next_image->page.y) > mng_info->page.height)
              mng_info->page.height=next_image->rows+(size_t) next_image->page.y;
          }

        if (next_image->page.x || next_image->page.y)
          need_defi=MagickTrue;

        if (next_image->alpha_trait != UndefinedPixelTrait)
          need_matte=MagickTrue;

        if ((int) next_image->dispose >= BackgroundDispose)
          if ((next_image->alpha_trait != UndefinedPixelTrait) ||
               next_image->page.x || next_image->page.y ||
              ((next_image->columns < mng_info->page.width) &&
               (next_image->rows < mng_info->page.height)))
            mng_info->need_fram=MagickTrue;

        if (next_image->iterations)
          need_iterations=MagickTrue;

        final_delay=next_image->delay;

        if (final_delay != initial_delay || (ssize_t) final_delay > 
           next_image->ticks_per_second)
          mng_info->need_fram=MagickTrue;

        /*
          check for global palette possibility.
        */
        if (image->alpha_trait != UndefinedPixelTrait)
           need_local_plte=MagickTrue;

        if (need_local_plte == 0)
          {
            if (IdentifyImageCoderGray(image,exception) == MagickFalse)
              all_images_are_gray=MagickFalse;
            mng_info->equal_palettes=PalettesAreEqual(image,next_image);
            if (use_global_plte == 0)
              use_global_plte=(int) mng_info->equal_palettes;
            need_local_plte=!mng_info->equal_palettes;
          }
        if (GetNextImageInList(next_image) != (Image *) NULL)
          {
            if (next_image->background_color.red !=
                next_image->next->background_color.red ||
                next_image->background_color.green !=
                next_image->next->background_color.green ||
                next_image->background_color.blue !=
                next_image->next->background_color.blue)
              mng_info->equal_backgrounds=MagickFalse;

            if (next_image->gamma != next_image->next->gamma)
              mng_info->equal_gammas=MagickFalse;

            if (next_image->rendering_intent !=
                next_image->next->rendering_intent)
              mng_info->equal_srgbs=MagickFalse;

            if ((next_image->units != next_image->next->units) ||
                (next_image->resolution.x != next_image->next->resolution.x) ||
                (next_image->resolution.y != next_image->next->resolution.y))
              mng_info->equal_physs=MagickFalse;

            if (mng_info->equal_chrms != MagickFalse)
              {
                if (next_image->chromaticity.red_primary.x !=
                    next_image->next->chromaticity.red_primary.x ||
                    next_image->chromaticity.red_primary.y !=
                    next_image->next->chromaticity.red_primary.y ||
                    next_image->chromaticity.green_primary.x !=
                    next_image->next->chromaticity.green_primary.x ||
                    next_image->chromaticity.green_primary.y !=
                    next_image->next->chromaticity.green_primary.y ||
                    next_image->chromaticity.blue_primary.x !=
                    next_image->next->chromaticity.blue_primary.x ||
                    next_image->chromaticity.blue_primary.y !=
                    next_image->next->chromaticity.blue_primary.y ||
                    next_image->chromaticity.white_point.x !=
                    next_image->next->chromaticity.white_point.x ||
                    next_image->chromaticity.white_point.y !=
                    next_image->next->chromaticity.white_point.y)
                  mng_info->equal_chrms=MagickFalse;
              }
          }
        image_count++;
        next_image=GetNextImageInList(next_image);
      }
      if (image_count < 2)
        {
          mng_info->equal_backgrounds=MagickFalse;
          mng_info->equal_chrms=MagickFalse;
          mng_info->equal_gammas=MagickFalse;
          mng_info->equal_srgbs=MagickFalse;
          mng_info->equal_physs=MagickFalse;
          use_global_plte=MagickFalse;
          need_local_plte=MagickTrue;
          need_iterations=MagickFalse;
        }

     if (mng_info->need_fram == MagickFalse)
       {
         /*
           Only certain framing rates 100/n are exactly representable without
           the FRAM chunk but we'll allow some slop in VLC files
         */
         if (final_delay == 0)
           {
             if (need_iterations != MagickFalse)
               {
                 /*
                   It's probably a GIF with loop; don't run it *too* fast.
                 */
                 if (mng_info->adjoin != MagickFalse)
                   {
                     final_delay=10;
                     (void) ThrowMagickException(exception,GetMagickModule(),
                       CoderWarning,
                       "input has zero delay between all frames; assuming",
                       " 10 cs `%s'","");
                   }
               }
             else
               mng_info->ticks_per_second=0;
           }
         if (final_delay != 0)
           mng_info->ticks_per_second=(png_uint_32)
              image->ticks_per_second/(png_uint_32) final_delay;
         if (final_delay > 50)
           mng_info->ticks_per_second=2;

         if (final_delay > 75)
           mng_info->ticks_per_second=1;

         if (final_delay > 125)
           mng_info->need_fram=MagickTrue;

         if (need_defi && final_delay > 2 && (final_delay != 4) &&
            (final_delay != 5) && (final_delay != 10) && (final_delay != 20) &&
            (final_delay != 25) && (final_delay != 50) &&
            (final_delay != (size_t) image->ticks_per_second))
           mng_info->need_fram=MagickTrue;  /* make it exact; cannot be VLC */
       }

     if (mng_info->need_fram != MagickFalse)
        mng_info->ticks_per_second=(unsigned long) image->ticks_per_second;
     /*
        If pseudocolor, we should also check to see if all the
        palettes are identical and write a global PLTE if they are.
        ../glennrp Feb 99.
     */
     /*
        Write the MNG version 1.0 signature and MHDR chunk.
     */
     (void) WriteBlob(image,8,(const unsigned char *) "\212MNG\r\n\032\n");
     (void) WriteBlobMSBULong(image,28L);  /* chunk data length=28 */
     PNGType(chunk,mng_MHDR);
     LogPNGChunk(logging,mng_MHDR,28L);
     PNGLong(chunk+4,(png_uint_32) mng_info->page.width);
     PNGLong(chunk+8,(png_uint_32) mng_info->page.height);
     PNGLong(chunk+12,mng_info->ticks_per_second);
     PNGLong(chunk+16,0L);  /* layer count=unknown */
     PNGLong(chunk+20,0L);  /* frame count=unknown */
     PNGLong(chunk+24,0L);  /* play time=unknown   */
     if (write_jng != MagickFalse)
       {
         if (need_matte)
           {
             if (need_defi || mng_info->need_fram != MagickFalse || use_global_plte)
               PNGLong(chunk+28,27L);    /* simplicity=LC+JNG */

             else
               PNGLong(chunk+28,25L);    /* simplicity=VLC+JNG */
           }

         else
           {
             if (need_defi || mng_info->need_fram != MagickFalse || use_global_plte)
               PNGLong(chunk+28,19L);  /* simplicity=LC+JNG, no transparency */

             else
               PNGLong(chunk+28,17L);  /* simplicity=VLC+JNG, no transparency */
           }
       }

     else
       {
         if (need_matte)
           {
             if (need_defi || mng_info->need_fram != MagickFalse || use_global_plte)
               PNGLong(chunk+28,11L);    /* simplicity=LC */

             else
               PNGLong(chunk+28,9L);    /* simplicity=VLC */
           }

         else
           {
             if (need_defi || mng_info->need_fram != MagickFalse || use_global_plte)
               PNGLong(chunk+28,3L);    /* simplicity=LC, no transparency */

             else
               PNGLong(chunk+28,1L);    /* simplicity=VLC, no transparency */
           }
       }
     (void) WriteBlob(image,32,chunk);
     (void) WriteBlobMSBULong(image,crc32(0,chunk,32));
     option=GetImageOption(image_info,"mng:need-cacheoff");
     if (option != (const char *) NULL)
       {
         size_t
           length;
         /*
           Write "nEED CACHEOFF" to turn playback caching off for streaming MNG.
         */
         PNGType(chunk,mng_nEED);
         length=CopyMagickString((char *) chunk+4,"CACHEOFF",20);
         (void) WriteBlobMSBULong(image,(size_t) length);
         LogPNGChunk(logging,mng_nEED,(size_t) length);
         length+=4;
         (void) WriteBlob(image,length,chunk);
         (void) WriteBlobMSBULong(image,crc32(0,chunk,(uInt) length));
       }
     if ((GetPreviousImageInList(image) == (Image *) NULL) &&
         (GetNextImageInList(image) != (Image *) NULL) &&
         (image->iterations != 1))
       {
         /*
           Write MNG TERM chunk
         */
         (void) WriteBlobMSBULong(image,10L);  /* data length=10 */
         PNGType(chunk,mng_TERM);
         LogPNGChunk(logging,mng_TERM,10L);
         chunk[4]=3;  /* repeat animation */
         chunk[5]=0;  /* show last frame when done */
         PNGLong(chunk+6,(png_uint_32) (mng_info->ticks_per_second*
            final_delay/(size_t) MagickMax(image->ticks_per_second,1)));

         if (image->iterations == 0)
           PNGLong(chunk+10,PNG_UINT_31_MAX);

         else
           PNGLong(chunk+10,(png_uint_32) image->iterations);

         if (logging != MagickFalse)
           {
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "     TERM delay: %.20g",(double) (mng_info->ticks_per_second*
              final_delay/(size_t) MagickMax(image->ticks_per_second,1)));

             if (image->iterations == 0)
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "     TERM iterations: %.20g",(double) PNG_UINT_31_MAX);

             else
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "     Image iterations: %.20g",(double) image->iterations);
           }
         (void) WriteBlob(image,14,chunk);
         (void) WriteBlobMSBULong(image,crc32(0,chunk,14));
       }
     /*
       To do: check for cHRM+gAMA == sRGB, and write sRGB instead.
     */
     if ((image->colorspace == sRGBColorspace || image->rendering_intent) &&
          mng_info->equal_srgbs != MagickFalse)
       {
         /*
           Write MNG sRGB chunk
         */
         (void) WriteBlobMSBULong(image,1L);
         PNGType(chunk,mng_sRGB);
         LogPNGChunk(logging,mng_sRGB,1L);

         if (image->rendering_intent != UndefinedIntent)
           chunk[4]=(unsigned char)
             Magick_RenderingIntent_to_PNG_RenderingIntent(
             (image->rendering_intent));

         else
           chunk[4]=(unsigned char)
             Magick_RenderingIntent_to_PNG_RenderingIntent(
               (PerceptualIntent));

         (void) WriteBlob(image,5,chunk);
         (void) WriteBlobMSBULong(image,crc32(0,chunk,5));
         mng_info->have_global_srgb=MagickTrue;
       }

     else
       {
         if (image->gamma && mng_info->equal_gammas != MagickFalse)
           {
             /*
                Write MNG gAMA chunk
             */
             (void) WriteBlobMSBULong(image,4L);
             PNGType(chunk,mng_gAMA);
             LogPNGChunk(logging,mng_gAMA,4L);
             PNGLong(chunk+4,(png_uint_32) (100000*image->gamma+0.5));
             (void) WriteBlob(image,8,chunk);
             (void) WriteBlobMSBULong(image,crc32(0,chunk,8));
             mng_info->have_global_gama=MagickTrue;
           }
         if (mng_info->equal_chrms != MagickFalse)
           {
             PrimaryInfo
               primary;

             /*
                Write MNG cHRM chunk
             */
             (void) WriteBlobMSBULong(image,32L);
             PNGType(chunk,mng_cHRM);
             LogPNGChunk(logging,mng_cHRM,32L);
             primary=image->chromaticity.white_point;
             PNGLong(chunk+4,(png_uint_32) (100000*primary.x+0.5));
             PNGLong(chunk+8,(png_uint_32) (100000*primary.y+0.5));
             primary=image->chromaticity.red_primary;
             PNGLong(chunk+12,(png_uint_32) (100000*primary.x+0.5));
             PNGLong(chunk+16,(png_uint_32) (100000*primary.y+0.5));
             primary=image->chromaticity.green_primary;
             PNGLong(chunk+20,(png_uint_32) (100000*primary.x+0.5));
             PNGLong(chunk+24,(png_uint_32) (100000*primary.y+0.5));
             primary=image->chromaticity.blue_primary;
             PNGLong(chunk+28,(png_uint_32) (100000*primary.x+0.5));
             PNGLong(chunk+32,(png_uint_32) (100000*primary.y+0.5));
             (void) WriteBlob(image,36,chunk);
             (void) WriteBlobMSBULong(image,crc32(0,chunk,36));
             mng_info->have_global_chrm=MagickTrue;
           }
       }
     if (image->resolution.x && image->resolution.y &&
         mng_info->equal_physs == MagickFalse)
       {
         /*
            Write MNG pHYs chunk
         */
         (void) WriteBlobMSBULong(image,9L);
         PNGType(chunk,mng_pHYs);
         LogPNGChunk(logging,mng_pHYs,9L);

         if (image->units == PixelsPerInchResolution)
           {
             PNGLong(chunk+4,(png_uint_32)
               (image->resolution.x*100.0/2.54+0.5));

             PNGLong(chunk+8,(png_uint_32)
               (image->resolution.y*100.0/2.54+0.5));

             chunk[12]=1;
           }

         else
           {
             if (image->units == PixelsPerCentimeterResolution)
               {
                 PNGLong(chunk+4,(png_uint_32)
                   (image->resolution.x*100.0+0.5));

                 PNGLong(chunk+8,(png_uint_32)
                   (image->resolution.y*100.0+0.5));

                 chunk[12]=1;
               }

             else
               {
                 PNGLong(chunk+4,(png_uint_32) (image->resolution.x+0.5));
                 PNGLong(chunk+8,(png_uint_32) (image->resolution.y+0.5));
                 chunk[12]=0;
               }
           }
         (void) WriteBlob(image,13,chunk);
         (void) WriteBlobMSBULong(image,crc32(0,chunk,13));
       }
     /*
       Write MNG BACK chunk and global bKGD chunk, if the image is transparent
       or does not cover the entire frame.
     */
     if ((write_mng != MagickFalse) && ((image->alpha_trait != UndefinedPixelTrait) ||
         image->page.x > 0 || image->page.y > 0 || (image->page.width &&
         ((ssize_t) image->page.width+image->page.x < (ssize_t) mng_info->page.width))
         || (image->page.height && ((ssize_t) image->page.height+image->page.y
         < (ssize_t) mng_info->page.height))))
       {
         (void) WriteBlobMSBULong(image,6L);
         PNGType(chunk,mng_BACK);
         LogPNGChunk(logging,mng_BACK,6L);
         red=ScaleQuantumToShort((Quantum) image->background_color.red);
         green=ScaleQuantumToShort((Quantum) image->background_color.green);
         blue=ScaleQuantumToShort((Quantum) image->background_color.blue);
         PNGShort(chunk+4,red);
         PNGShort(chunk+6,green);
         PNGShort(chunk+8,blue);
         (void) WriteBlob(image,10,chunk);
         (void) WriteBlobMSBULong(image,crc32(0,chunk,10));
         if (mng_info->equal_backgrounds != MagickFalse)
           {
             (void) WriteBlobMSBULong(image,6L);
             PNGType(chunk,mng_bKGD);
             LogPNGChunk(logging,mng_bKGD,6L);
             (void) WriteBlob(image,10,chunk);
             (void) WriteBlobMSBULong(image,crc32(0,chunk,10));
           }
       }

     if ((need_local_plte == MagickFalse) &&
         (image->storage_class == PseudoClass) &&
         (all_images_are_gray == MagickFalse))
       {
         size_t
           data_length;

         ssize_t
           i;

         /*
           Write MNG PLTE chunk
         */
         data_length=3*image->colors;
         (void) WriteBlobMSBULong(image,data_length);
         PNGType(chunk,mng_PLTE);
         LogPNGChunk(logging,mng_PLTE,data_length);

         for (i=0; i < (ssize_t) image->colors; i++)
         {
           chunk[4+i*3]=(unsigned char) (ScaleQuantumToChar(
             (Quantum) image->colormap[i].red) & 0xff);
           chunk[5+i*3]=(unsigned char) (ScaleQuantumToChar(
             (Quantum) image->colormap[i].green) & 0xff);
           chunk[6+i*3]=(unsigned char) (ScaleQuantumToChar(
             (Quantum) image->colormap[i].blue) & 0xff);
         }

         (void) WriteBlob(image,data_length+4,chunk);
         (void) WriteBlobMSBULong(image,crc32(0,chunk,(uInt) (data_length+4)));
         mng_info->have_global_plte=MagickTrue;
       }
    }
  scene=0;
  mng_info->delay=0;
  mng_info->equal_palettes=MagickFalse;
  number_scenes=GetImageListLength(image);
  do
  {
    if (mng_info->adjoin != MagickFalse)
    {
    /*
      If we aren't using a global palette for the entire MNG, check to
      see if we can use one for two or more consecutive images.
    */
    if (need_local_plte && use_global_plte && !all_images_are_gray)
      {
        if (mng_info->is_palette != MagickFalse)
          {
            /*
              When equal_palettes is true, this image has the same palette
              as the previous PseudoClass image
            */
            mng_info->have_global_plte=mng_info->equal_palettes;
            mng_info->equal_palettes=PalettesAreEqual(image,image->next);
            if (mng_info->equal_palettes && !mng_info->have_global_plte)
              {
                /*
                  Write MNG PLTE chunk
                */
                size_t
                  data_length;

                ssize_t
                  i;

                data_length=3*image->colors;
                (void) WriteBlobMSBULong(image,data_length);
                PNGType(chunk,mng_PLTE);
                LogPNGChunk(logging,mng_PLTE,data_length);

                for (i=0; i < (ssize_t) image->colors; i++)
                {
                  chunk[4+i*3]=ScaleQuantumToChar((Quantum) image->colormap[i].red);
                  chunk[5+i*3]=ScaleQuantumToChar((Quantum) image->colormap[i].green);
                  chunk[6+i*3]=ScaleQuantumToChar((Quantum) image->colormap[i].blue);
                }

                (void) WriteBlob(image,data_length+4,chunk);
                (void) WriteBlobMSBULong(image,crc32(0,chunk,
                   (uInt) (data_length+4)));
                mng_info->have_global_plte=MagickTrue;
              }
          }
        else
          mng_info->have_global_plte=MagickFalse;
      }
    if (need_defi)
      {
        ssize_t
          previous_x,
          previous_y;

        if (scene != 0)
          {
            previous_x=mng_info->page.x;
            previous_y=mng_info->page.y;
          }
        else
          {
            previous_x=0;
            previous_y=0;
          }
        mng_info->page=image->page;
        if ((mng_info->page.x !=  previous_x) ||
            (mng_info->page.y != previous_y))
          {
             (void) WriteBlobMSBULong(image,12L);  /* data length=12 */
             PNGType(chunk,mng_DEFI);
             LogPNGChunk(logging,mng_DEFI,12L);
             chunk[4]=0; /* object 0 MSB */
             chunk[5]=0; /* object 0 LSB */
             chunk[6]=0; /* visible  */
             chunk[7]=0; /* abstract */
             PNGLong(chunk+8,(png_uint_32) mng_info->page.x);
             PNGLong(chunk+12,(png_uint_32) mng_info->page.y);
             (void) WriteBlob(image,16,chunk);
             (void) WriteBlobMSBULong(image,crc32(0,chunk,16));
          }
      }
    }

   mng_info->write_mng=write_mng;

   if ((int) image->dispose >= 3)
     mng_info->framing_mode=3;

   if (mng_info->need_fram != MagickFalse && mng_info->adjoin != MagickFalse &&
       ((image->delay != mng_info->delay) ||
        (mng_info->framing_mode != mng_info->old_framing_mode)))
     {
       if (image->delay == mng_info->delay)
         {
           /*
             Write a MNG FRAM chunk with the new framing mode.
           */
           (void) WriteBlobMSBULong(image,1L);  /* data length=1 */
           PNGType(chunk,mng_FRAM);
           LogPNGChunk(logging,mng_FRAM,1L);
           chunk[4]=(unsigned char) mng_info->framing_mode;
           (void) WriteBlob(image,5,chunk);
           (void) WriteBlobMSBULong(image,crc32(0,chunk,5));
         }
       else
         {
           /*
             Write a MNG FRAM chunk with the delay.
           */
           (void) WriteBlobMSBULong(image,10L);  /* data length=10 */
           PNGType(chunk,mng_FRAM);
           LogPNGChunk(logging,mng_FRAM,10L);
           chunk[4]=(unsigned char) mng_info->framing_mode;
           chunk[5]=0;  /* frame name separator (no name) */
           chunk[6]=2;  /* flag for changing default delay */
           chunk[7]=0;  /* flag for changing frame timeout */
           chunk[8]=0;  /* flag for changing frame clipping */
           chunk[9]=0;  /* flag for changing frame sync_id */
           PNGLong(chunk+10,(png_uint_32) ((mng_info->ticks_per_second*
             image->delay)/(size_t) MagickMax(image->ticks_per_second,1)));
           (void) WriteBlob(image,14,chunk);
           (void) WriteBlobMSBULong(image,crc32(0,chunk,14));
           mng_info->delay=(png_uint_32) image->delay;
         }
       mng_info->old_framing_mode=mng_info->framing_mode;
     }

#if defined(JNG_SUPPORTED)
   if (image_info->compression == JPEGCompression)
     {
       ImageInfo
         *write_info;

       if (logging != MagickFalse)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "  Writing JNG object.");
       /* To do: specify the desired alpha compression method. */
       write_info=CloneImageInfo(image_info);
       write_info->compression=UndefinedCompression;
       status=WriteOneJNGImage(mng_info,write_info,image,exception);
       write_info=DestroyImageInfo(write_info);
     }
   else
#endif
     {
       if (logging != MagickFalse)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "  Writing PNG object.");

       mng_info->need_blob = MagickFalse;
       mng_info->preserve_colormap = MagickFalse;

       /* We don't want any ancillary chunks written */
       mng_info->exclude_bKGD=MagickTrue;
       mng_info->exclude_caNv=MagickTrue;
       mng_info->exclude_cHRM=MagickTrue;
       mng_info->exclude_date=MagickTrue;
       mng_info->exclude_EXIF=MagickTrue;
       mng_info->exclude_gAMA=MagickTrue;
       mng_info->exclude_iCCP=MagickTrue;
       mng_info->exclude_oFFs=MagickTrue;
       mng_info->exclude_pHYs=MagickTrue;
       mng_info->exclude_sRGB=MagickTrue;
       mng_info->exclude_tEXt=MagickTrue;
       mng_info->exclude_tRNS=MagickTrue;
       mng_info->exclude_zCCP=MagickTrue;
       mng_info->exclude_zTXt=MagickTrue;

       status=WriteOnePNGImage(mng_info,image_info,image,exception);
     }

    if (status == MagickFalse)
      {
        (void) CloseBlob(image);
        return(MagickFalse);
      }
    (void) CatchImageException(image);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,(MagickOffsetType) scene++,
      number_scenes);

    if (status == MagickFalse)
      break;

  } while (mng_info->adjoin != MagickFalse);

  if (write_mng != MagickFalse)
    {
      while (GetPreviousImageInList(image) != (Image *) NULL)
        image=GetPreviousImageInList(image);
      /*
        Write the MEND chunk.
      */
      (void) WriteBlobMSBULong(image,0x00000000L);
      PNGType(chunk,mng_MEND);
      LogPNGChunk(logging,mng_MEND,0L);
      (void) WriteBlob(image,4,chunk);
      (void) WriteBlobMSBULong(image,crc32(0,chunk,4));
    }
  /*
    Relinquish resources.
  */
  mng_info=(MngWriteInfo *) RelinquishMagickMemory(mng_info);
  (void) CloseBlob(image);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit WriteMNGImage()");

  return(MagickTrue);
}
#endif

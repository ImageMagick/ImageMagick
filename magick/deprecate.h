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

  MagickCore deprecated methods.
*/
#ifndef _MAGICKCORE_DEPRECATE_H
#define _MAGICKCORE_DEPRECATE_H

#include <stdarg.h>
#include "magick/blob.h"
#include "magick/cache-view.h"
#include "magick/draw.h"
#include "magick/constitute.h"
#include "magick/magick-config.h"
#include "magick/pixel.h"
#include "magick/quantize.h"
#include "magick/quantum.h"
#include "magick/registry.h"
#include "magick/semaphore.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)

#if !defined(magick_attribute)
#  if !defined(__GNUC__)
#    define magick_attribute(x) /*nothing*/
#  else
#    define magick_attribute __attribute__
#  endif
#endif

#define Downscale(quantum)  ScaleQuantumToChar(quantum)
#define LABColorspace LabColorspace
#define CompressPixelGamma(pixel)  DecodePixelGamma(pixel)
#define DecodesRGBGamma(pixel)  DecodePixelGamma(pixel)
#define EncodesRGBGamma(pixel)  EncodePixelGamma(pixel)
#define ExpandPixelGamma(pixel)  EncodePixelGamma(pixel)
#define Intensity(color)  PixelIntensityToQuantum(color)
#define LiberateUniqueFileResource(resource) \
  RelinquishUniqueFileResource(resource)
#define LiberateMagickResource(resource)  RelinquishMagickResource(resource)
#define LiberateSemaphore(semaphore)  RelinquishSemaphore(semaphore)
#define QuantumDepth  MAGICKCORE_QUANTUM_DEPTH
#define MaxRGB  QuantumRange  /* deprecated */
#define RunlengthEncodedCompression  RLECompression
#define Upscale(value)  ScaleCharToQuantum(value)
#define XDownscale(value)  ScaleShortToQuantum(value)
#define XUpscale(quantum)  ScaleQuantumToShort(quantum)

typedef enum
{
  UndefinedMagickLayerMethod
} MagickLayerMethod;

typedef MagickOffsetType ExtendedSignedIntegralType;
typedef MagickSizeType ExtendedUnsignedIntegralType;
typedef MagickRealType ExtendedRationalType;
typedef struct _ViewInfo ViewInfo;

typedef MagickBooleanType
  (*MonitorHandler)(const char *,const MagickOffsetType,const MagickSizeType,
    ExceptionInfo *);

typedef struct _ImageAttribute
{
  char
    *key,
    *value;
                                                                                
  MagickBooleanType
    compression;
                                                                                
  struct _ImageAttribute
    *previous,
    *next;  /* deprecated */
} ImageAttribute;

extern MagickExport CacheView
  *CloseCacheView(CacheView *) magick_attribute((deprecated)),
  *OpenCacheView(const Image *) magick_attribute((deprecated));

extern MagickExport char
  *AllocateString(const char *) magick_attribute((deprecated)),
  *InterpretImageAttributes(const ImageInfo *,Image *,const char *)
    magick_attribute((deprecated)),
  *PostscriptGeometry(const char *) magick_attribute((deprecated)),
  *TranslateText(const ImageInfo *,Image *,const char *)
     magick_attribute((deprecated));

extern MagickExport const ImageAttribute
  *GetImageAttribute(const Image *,const char *),
  *GetImageClippingPathAttribute(Image *) magick_attribute((deprecated)),
  *GetNextImageAttribute(const Image *) magick_attribute((deprecated));

extern MagickExport const IndexPacket
  *AcquireCacheViewIndexes(const CacheView *) magick_attribute((deprecated)),
  *AcquireIndexes(const Image *) magick_attribute((deprecated));

extern MagickExport const PixelPacket
  *AcquirePixels(const Image *) magick_attribute((deprecated)),
  *AcquireCacheViewPixels(const CacheView *,const ssize_t,const ssize_t,
    const size_t,const size_t,ExceptionInfo *)
    magick_attribute((deprecated)),
  *AcquireImagePixels(const Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,ExceptionInfo *) magick_attribute((deprecated));

extern MagickExport FILE
  *OpenMagickStream(const char *,const char *);

extern MagickExport Image
  *AllocateImage(const ImageInfo *) magick_attribute((deprecated)),
  *AverageImages(const Image *,ExceptionInfo *),
  *ExtractSubimageFromImage(Image *,const Image *,ExceptionInfo *)
    magick_attribute((deprecated)),
  *GetImageFromMagickRegistry(const char *,ssize_t *id,ExceptionInfo *)
    magick_attribute((deprecated)),
  *GetImageList(const Image *,const ssize_t,ExceptionInfo *)
    magick_attribute((deprecated)),
  *GetNextImage(const Image *) magick_attribute((deprecated)),
  *GetPreviousImage(const Image *) magick_attribute((deprecated)),
  *FlattenImages(Image *,ExceptionInfo *) magick_attribute((deprecated)),
  *MaximumImages(const Image *,ExceptionInfo *),
  *MedianFilterImage(const Image *,const double,ExceptionInfo *),
  *ModeImage(const Image *,const double,ExceptionInfo *),
  *MinimumImages(const Image *,ExceptionInfo *),
  *MosaicImages(Image *,ExceptionInfo *) magick_attribute((deprecated)),
  *PopImageList(Image **) magick_attribute((deprecated)),
  *RecolorImage(const Image *,const size_t,const double *,ExceptionInfo *)
    magick_attribute((deprecated)),
  *ReduceNoiseImage(const Image *,const double,ExceptionInfo *),
  *ShiftImageList(Image **) magick_attribute((deprecated)),
  *SpliceImageList(Image *,const ssize_t,const size_t,const Image *,
    ExceptionInfo *) magick_attribute((deprecated)),
  *ZoomImage(const Image *,const size_t,const size_t,ExceptionInfo *)
    magick_attribute((deprecated));

extern MagickExport IndexPacket
  *GetCacheViewIndexes(CacheView *) magick_attribute((deprecated)),
  *GetIndexes(const Image *) magick_attribute((deprecated)),
  ValidateColormapIndex(Image *,const size_t) magick_attribute((deprecated));

extern MagickExport int
  GetImageGeometry(Image *,const char *,const unsigned int,RectangleInfo *)
    magick_attribute((deprecated)),
  ParseImageGeometry(const char *,ssize_t *,ssize_t *,size_t *,size_t *)
    magick_attribute((deprecated));

extern MagickExport MagickBooleanType
  AcquireOneCacheViewPixel(const CacheView *,const ssize_t,const ssize_t,
    PixelPacket *,ExceptionInfo *) magick_attribute((deprecated)),
  AcquireOneCacheViewVirtualPixel(const CacheView *,const VirtualPixelMethod,
    const ssize_t,const ssize_t,PixelPacket *,ExceptionInfo *)
    magick_attribute((deprecated)),
  AffinityImage(const QuantizeInfo *,Image *,const Image *)
    magick_attribute((deprecated)),
  AffinityImages(const QuantizeInfo *,Image *,const Image *)
    magick_attribute((deprecated)),
  AllocateImageColormap(Image *,const size_t)
    magick_attribute((deprecated)),
  ClipPathImage(Image *,const char *,const MagickBooleanType)
    magick_attribute((deprecated)),
  CloneImageAttributes(Image *,const Image *) magick_attribute((deprecated)),
  ColorFloodfillImage(Image *,const DrawInfo *,const PixelPacket,const ssize_t,
    const ssize_t,const PaintMethod) magick_attribute((deprecated)),
  DeleteImageAttribute(Image *,const char *) magick_attribute((deprecated)),
  DeleteMagickRegistry(const ssize_t) magick_attribute((deprecated)),
  DescribeImage(Image *,FILE *,const MagickBooleanType)
    magick_attribute((deprecated)),
  FormatImageAttribute(Image *,const char *,const char *,...)
    magick_attribute((__format__ (__printf__,3,4)))
    magick_attribute((deprecated)),
  FormatImageAttributeList(Image *,const char *,const char *,va_list)
    magick_attribute((__format__ (__printf__,3,0)))
    magick_attribute((deprecated)),
  FormatImagePropertyList(Image *,const char *,const char *,va_list)
    magick_attribute((__format__ (__printf__,3,0))),
  FuzzyColorCompare(const Image *,const PixelPacket *,const PixelPacket *)
    magick_attribute((deprecated)),
  FuzzyOpacityCompare(const Image *,const PixelPacket *,const PixelPacket *)
    magick_attribute((deprecated)),
  IsMagickInstantiated(void) magick_attribute((deprecated)),
  LevelImageColors(Image *,const ChannelType,const MagickPixelPacket *,
    const MagickPixelPacket *, const MagickBooleanType)
    magick_attribute((deprecated)),
  MagickMonitor(const char *,const MagickOffsetType,const MagickSizeType,
    void *) magick_attribute((deprecated)),
  MapImage(Image *,const Image *,const MagickBooleanType)
    magick_attribute((deprecated)),
  MapImages(Image *,const Image *,const MagickBooleanType)
    magick_attribute((deprecated)),
  MatteFloodfillImage(Image *,const PixelPacket,const Quantum,const ssize_t,
    const ssize_t,const PaintMethod) magick_attribute((deprecated)),
  OpaqueImage(Image *,const PixelPacket,const PixelPacket)
    magick_attribute((deprecated)),
  PaintFloodfillImage(Image *,const ChannelType,const MagickPixelPacket *,
    const ssize_t,const ssize_t,const DrawInfo *,const PaintMethod)
    magick_attribute((deprecated)),
  PaintOpaqueImage(Image *,const MagickPixelPacket *,const MagickPixelPacket *)
    magick_attribute((deprecated)),
  PaintOpaqueImageChannel(Image *,const ChannelType,const MagickPixelPacket *,
    const MagickPixelPacket *) magick_attribute((deprecated)),
  PaintTransparentImage(Image *,const MagickPixelPacket *,const Quantum)
    magick_attribute((deprecated)),
  SetExceptionInfo(ExceptionInfo *,ExceptionType)
    magick_attribute((deprecated)),
  SetImageAttribute(Image *,const char *,const char *)
    magick_attribute((deprecated)),
  SyncCacheViewPixels(CacheView *) magick_attribute((deprecated)),
  SyncImagePixels(Image *) magick_attribute((deprecated)),
  TransparentImage(Image *,const PixelPacket,const Quantum)
    magick_attribute((deprecated));

extern MagickExport MagickPixelPacket
  AcquireOneMagickPixel(const Image *,const ssize_t,const ssize_t,
    ExceptionInfo *) magick_attribute((deprecated));

extern MagickExport MonitorHandler
  GetMonitorHandler(void) magick_attribute((deprecated)),
  SetMonitorHandler(MonitorHandler) magick_attribute((deprecated));

extern MagickExport MagickOffsetType
  SizeBlob(Image *image) magick_attribute((deprecated));

extern MagickExport MagickPixelPacket
  InterpolatePixelColor(const Image *,CacheView *,const InterpolatePixelMethod,
    const double,const double,ExceptionInfo *) magick_attribute((deprecated));

extern MagickExport MagickStatusType
  ParseSizeGeometry(const Image *,const char *,RectangleInfo *)
    magick_attribute((deprecated));

extern MagickExport PixelPacket
  AcquireOnePixel(const Image *,const ssize_t,const ssize_t,ExceptionInfo *)
    magick_attribute((deprecated)),
  AcquireOneVirtualPixel(const Image *,const VirtualPixelMethod,const ssize_t,
    const ssize_t,ExceptionInfo *) magick_attribute((deprecated)),
  *GetCacheView(CacheView *,const ssize_t,const ssize_t,const size_t,
    const size_t) magick_attribute((deprecated)),
  *GetCacheViewPixels(CacheView *,const ssize_t,const ssize_t,const size_t,
    const size_t) magick_attribute((deprecated)),
  *GetImagePixels(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t) magick_attribute((deprecated)),
  GetOnePixel(Image *,const ssize_t,const ssize_t)
    magick_attribute((deprecated)),
  *GetPixels(const Image *) magick_attribute((deprecated)),
  *SetCacheViewPixels(CacheView *,const ssize_t,const ssize_t,const size_t,
    const size_t) magick_attribute((deprecated)),
  *SetImagePixels(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t) magick_attribute((deprecated));

extern MagickExport MagickRealType
  InversesRGBCompandor(const MagickRealType) magick_attribute((deprecated)),
  sRGBCompandor(const MagickRealType) magick_attribute((deprecated));

extern MagickExport size_t
  GetImageListSize(const Image *) magick_attribute((deprecated)),
  PopImagePixels(Image *,const QuantumType,unsigned char *)
    magick_attribute((deprecated)),
  PushImagePixels(Image *,const QuantumType,const unsigned char *)
    magick_attribute((deprecated));

extern MagickExport ssize_t
  FormatMagickString(char *,const size_t,const char *,...)
    magick_attribute((__format__ (__printf__,3,4)))
    magick_attribute((deprecated)),
  FormatMagickStringList(char *,const size_t,const char *,va_list)
    magick_attribute((__format__ (__printf__,3,0))),
  GetImageListIndex(const Image *) magick_attribute((deprecated)),
  SetMagickRegistry(const RegistryType,const void *,const size_t,
    ExceptionInfo *) magick_attribute((deprecated));

extern MagickExport unsigned int
  ChannelImage(Image *,const ChannelType) magick_attribute((deprecated)),
  ChannelThresholdImage(Image *,const char *) magick_attribute((deprecated)),
  DispatchImage(const Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,void *,ExceptionInfo *)
    magick_attribute((deprecated)),
  FuzzyColorMatch(const PixelPacket *,const PixelPacket *,const double)
    magick_attribute((deprecated)),
  GetNumberScenes(const Image *) magick_attribute((deprecated)),
  GetMagickGeometry(const char *,ssize_t *,ssize_t *,size_t *,size_t *)
    magick_attribute((deprecated)),
  IsSubimage(const char *,const unsigned int) magick_attribute((deprecated)),
  PushImageList(Image **,const Image *,ExceptionInfo *)
    magick_attribute((deprecated)),
  QuantizationError(Image *) magick_attribute((deprecated)),
  RandomChannelThresholdImage(Image *,const char *,const char *,
    ExceptionInfo *) magick_attribute((deprecated)),
  SetImageList(Image **,const Image *,const ssize_t,ExceptionInfo *)
    magick_attribute((deprecated)),
  TransformColorspace(Image *,const ColorspaceType)
    magick_attribute((deprecated)),
  ThresholdImage(Image *,const double) magick_attribute((deprecated)),
  ThresholdImageChannel(Image *,const char *) magick_attribute((deprecated)),
  UnshiftImageList(Image **,const Image *,ExceptionInfo *)
    magick_attribute((deprecated));

extern MagickExport void
  *AcquireMemory(const size_t) magick_attribute((deprecated)),
  AllocateNextImage(const ImageInfo *,Image *) magick_attribute((deprecated)),
  *CloneMemory(void *,const void *,const size_t) magick_attribute((deprecated)),
  DestroyConstitute(void),
  DestroyImageAttributes(Image *) magick_attribute((deprecated)),
  DestroyImages(Image *) magick_attribute((deprecated)),
  DestroyMagick(void) magick_attribute((deprecated)),
  DestroyMagickRegistry(void) magick_attribute((deprecated)),
  *GetConfigureBlob(const char *,char *,size_t *,ExceptionInfo *)
    magick_attribute((deprecated)),
  *GetMagickRegistry(const ssize_t,RegistryType *,size_t *,ExceptionInfo *)
    magick_attribute((deprecated)),
  IdentityAffine(AffineMatrix *) magick_attribute((deprecated)),
  LiberateMemory(void **) magick_attribute((deprecated)),
  LiberateSemaphoreInfo(SemaphoreInfo **) magick_attribute((deprecated)),
  FormatString(char *,const char *,...)
    magick_attribute((__format__ (__printf__,2,3)))
    magick_attribute((deprecated)),
  FormatStringList(char *,const char *,va_list)
    magick_attribute((__format__ (__printf__,2,0)))
    magick_attribute((deprecated)),
  HSLTransform(const double,const double,const double,Quantum *,Quantum *,
    Quantum *) magick_attribute((deprecated)),
  InitializeMagick(const char *) magick_attribute((deprecated)),
  MagickIncarnate(const char *) magick_attribute((deprecated)),
  ReacquireMemory(void **,const size_t) magick_attribute((deprecated)),
  ResetImageAttributeIterator(const Image *) magick_attribute((deprecated)),
  SetCacheThreshold(const size_t) magick_attribute((deprecated)),
  SetImage(Image *,const Quantum) magick_attribute((deprecated)),
  Strip(char *) magick_attribute((deprecated)),
  TemporaryFilename(char *) magick_attribute((deprecated)),
  TransformHSL(const Quantum,const Quantum,const Quantum,double *,double *,
    double *) magick_attribute((deprecated));

/*
  Inline methods.
*/
static inline double MagickEpsilonReciprocal(const double x)
{
  double
    sign;

  sign=x < 0.0 ? -1.0 : 1.0;
  if ((sign*x) >= MagickEpsilon)
    return(1.0/x);
  return(sign/MagickEpsilon);
}

static inline Quantum PixelIntensityToQuantum(const Image *restrict image,
  const PixelPacket *restrict pixel)
{
  return(ClampToQuantum(GetPixelIntensity(image,pixel)));
}

#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

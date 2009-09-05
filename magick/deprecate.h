/*
  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
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

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)

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

#if !defined(magick_attribute)
#  if !defined(__GNUC__)
#    define magick_attribute(x) /*nothing*/
#  else
#    define magick_attribute __attribute__
#  endif
#endif

#define Downscale(quantum)  ScaleQuantumToChar(quantum)
#define LABColorspace LabColorspace
#define Intensity(color)  PixelIntensityToQuantum(color)
#define LiberateUniqueFileResource(resource) \
  RelinquishUniqueFileResource(resource)
#define LiberateMagickResource(resource)  RelinquishMagickResource(resource)
#define LiberateSemaphore(semaphore)  RelinquishSemaphore(semaphore)
#define QuantumDepth  MAGICKCORE_QUANTUM_DEPTH
#define RunlengthEncodedCompression  RLECompression
#define Upscale(value)  ScaleCharToQuantum(value)
#define XDownscale(value)  ScaleShortToQuantum(value)
#define XUpscale(quantum)  ScaleQuantumToShort(quantum)

typedef struct _DoublePixelPacket
{
  double
    red,
    green,
    blue,
    opacity,
    index;
} DoublePixelPacket;

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

extern MagickExport char
  *AllocateString(const char *),
  *InterpretImageAttributes(const ImageInfo *,Image *,const char *),
  *PostscriptGeometry(const char *),
  *TranslateText(const ImageInfo *,Image *,const char *);

extern MagickExport const ImageAttribute
  *GetImageAttribute(const Image *,const char *),
  *GetImageClippingPathAttribute(Image *),
  *GetNextImageAttribute(const Image *);

extern MagickExport const IndexPacket
  *AcquireCacheViewIndexes(const CacheView *),
  *AcquireIndexes(const Image *);

extern MagickExport const PixelPacket
  *AcquirePixels(const Image *),
  *AcquireCacheViewPixels(const CacheView *,const long,const long,
    const unsigned long,const unsigned long,ExceptionInfo *),
  *AcquireImagePixels(const Image *,const long,const long,const unsigned long,
    const unsigned long,ExceptionInfo *);

extern MagickExport Image
  *AllocateImage(const ImageInfo *),
  *ExtractSubimageFromImage(Image *,const Image *,ExceptionInfo *),
  *GetImageFromMagickRegistry(const char *,long *id,ExceptionInfo *),
  *GetImageList(const Image *,const long,ExceptionInfo *),
  *GetNextImage(const Image *),
  *GetPreviousImage(const Image *),
  *FlattenImages(Image *,ExceptionInfo *),
  *MosaicImages(Image *,ExceptionInfo *),
  *PopImageList(Image **),
  *ShiftImageList(Image **),
  *SpliceImageList(Image *,const long,const unsigned long,const Image *,
    ExceptionInfo *);

extern MagickExport IndexPacket
  *GetCacheViewIndexes(CacheView *),
  *GetIndexes(const Image *),
  ValidateColormapIndex(Image *,const unsigned long);

extern MagickExport int
  GetImageGeometry(Image *,const char *,const unsigned int,RectangleInfo *),
  ParseImageGeometry(const char *,long *,long *,unsigned long *,
    unsigned long *);

extern MagickExport long
  GetImageListIndex(const Image *),
  SetMagickRegistry(const RegistryType,const void *,const size_t,
    ExceptionInfo *);

extern MagickExport MagickBooleanType
  AcquireOneCacheViewPixel(const CacheView *,const long,const long,
    PixelPacket *,ExceptionInfo *),
  AcquireOneCacheViewVirtualPixel(const CacheView *,const VirtualPixelMethod,
    const long,const long,PixelPacket *,ExceptionInfo *),
  AffinityImage(const QuantizeInfo *,Image *,const Image *),
  AffinityImages(const QuantizeInfo *,Image *,const Image *),
  AllocateImageColormap(Image *,const unsigned long),
  ClipPathImage(Image *,const char *,const MagickBooleanType),
  CloneImageAttributes(Image *,const Image *),
  ColorFloodfillImage(Image *,const DrawInfo *,const PixelPacket,const long,
    const long,const PaintMethod),
  DeleteImageAttribute(Image *,const char *),
  DeleteMagickRegistry(const long),
  DescribeImage(Image *,FILE *,const MagickBooleanType),
  FormatImageAttribute(Image *,const char *,const char *,...)
    magick_attribute((format (printf,3,4))),
  FormatImageAttributeList(Image *,const char *,const char *,va_list)
    magick_attribute((format (printf,3,0))),
  FuzzyColorCompare(const Image *,const PixelPacket *,const PixelPacket *),
  FuzzyOpacityCompare(const Image *,const PixelPacket *,const PixelPacket *),
  MagickMonitor(const char *,const MagickOffsetType,const MagickSizeType,
    void *),
  MapImage(Image *,const Image *,const MagickBooleanType),
  MapImages(Image *,const Image *,const MagickBooleanType),
  MatteFloodfillImage(Image *,const PixelPacket,const Quantum,const long,
    const long,const PaintMethod),
  OpaqueImage(Image *,const PixelPacket,const PixelPacket),
  PaintFloodfillImage(Image *,const ChannelType,const MagickPixelPacket *,
    const long,const long,const DrawInfo *,const PaintMethod),
  PaintOpaqueImage(Image *,const MagickPixelPacket *,const MagickPixelPacket *),
  PaintOpaqueImageChannel(Image *,const ChannelType,const MagickPixelPacket *,
    const MagickPixelPacket *),
  PaintTransparentImage(Image *,const MagickPixelPacket *,const Quantum),
  SetExceptionInfo(ExceptionInfo *,ExceptionType),
  SetImageAttribute(Image *,const char *,const char *),
  SyncCacheViewPixels(CacheView *),
  SyncImagePixels(Image *),
  TransparentImage(Image *,const PixelPacket,const Quantum);

extern MagickExport MagickPixelPacket
  AcquireOneMagickPixel(const Image *,const long,const long,ExceptionInfo *);

extern MagickExport MonitorHandler
  GetMonitorHandler(void),
  SetMonitorHandler(MonitorHandler);

extern MagickExport MagickOffsetType
  SizeBlob(Image *image);

extern MagickExport MagickPixelPacket
  InterpolatePixelColor(const Image *,CacheView *,const InterpolatePixelMethod,
    const double,const double,ExceptionInfo *);

extern MagickExport MagickStatusType
  ParseSizeGeometry(const Image *,const char *,RectangleInfo *);

extern MagickExport PixelPacket
  AcquireOnePixel(const Image *,const long,const long,ExceptionInfo *),
  AcquireOneVirtualPixel(const Image *,const VirtualPixelMethod,const long,
    const long,ExceptionInfo *),
  *GetCacheView(CacheView *,const long,const long,const unsigned long,
    const unsigned long),
  *GetCacheViewPixels(CacheView *,const long,const long,const unsigned long,
    const unsigned long),
  *GetImagePixels(Image *,const long,const long,const unsigned long,
    const unsigned long),
  GetOnePixel(Image *,const long,const long),
  *GetPixels(const Image *),
  *SetCacheViewPixels(CacheView *,const long,const long,const unsigned long,
    const unsigned long),
  *SetImagePixels(Image *,const long,const long,const unsigned long,
    const unsigned long);

extern MagickExport size_t
  PopImagePixels(Image *,const QuantumType,unsigned char *),
  PushImagePixels(Image *,const QuantumType,const unsigned char *);

extern MagickExport unsigned int
  ChannelImage(Image *,const ChannelType),
  ChannelThresholdImage(Image *,const char *),
  DispatchImage(const Image *,const long,const long,const unsigned long,
    const unsigned long,const char *,const StorageType,void *,ExceptionInfo *),
  FuzzyColorMatch(const PixelPacket *,const PixelPacket *,const double),
  GetNumberScenes(const Image *),
  GetMagickGeometry(const char *,long *,long *,unsigned long *,unsigned long *),
  IsSubimage(const char *,const unsigned int),
  PushImageList(Image **,const Image *,ExceptionInfo *),
  QuantizationError(Image *),
  RandomChannelThresholdImage(Image *,const char *,const char *,
    ExceptionInfo *),
  SetImageList(Image **,const Image *,const long,ExceptionInfo *),
  TransformColorspace(Image *,const ColorspaceType),
  ThresholdImage(Image *,const double),
  ThresholdImageChannel(Image *,const char *),
  UnshiftImageList(Image **,const Image *,ExceptionInfo *);

extern MagickExport unsigned long
  GetImageListSize(const Image *);

extern MagickExport CacheView
  *CloseCacheView(CacheView *),
  *OpenCacheView(const Image *);

extern MagickExport void
  *AcquireMemory(const size_t),
  AllocateNextImage(const ImageInfo *,Image *),
  *CloneMemory(void *,const void *,const size_t),
  DestroyImageAttributes(Image *),
  DestroyImages(Image *),
  DestroyMagick(void),
  DestroyMagickRegistry(void),
  *GetConfigureBlob(const char *,char *,size_t *,ExceptionInfo *),
  *GetMagickRegistry(const long,RegistryType *,size_t *,ExceptionInfo *),
  IdentityAffine(AffineMatrix *),
  LiberateMemory(void **),
  LiberateSemaphoreInfo(SemaphoreInfo **),
  FormatString(char *,const char *,...) magick_attribute((format (printf,2,3))),
  FormatStringList(char *,const char *,va_list)
    magick_attribute((format (printf,2,0))),
  HSLTransform(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  InitializeMagick(const char *),
  ReacquireMemory(void **,const size_t),
  ResetImageAttributeIterator(const Image *),
  SetCacheThreshold(const unsigned long),
  SetImage(Image *,const Quantum),
  Strip(char *),
  TemporaryFilename(char *),
  TransformHSL(const Quantum,const Quantum,const Quantum,double *,double *,
    double *);
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

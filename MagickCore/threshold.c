/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%       TTTTT  H   H  RRRR   EEEEE  SSSSS  H   H   OOO   L      DDDD          %
%         T    H   H  R   R  E      SS     H   H  O   O  L      D   D         %
%         T    HHHHH  RRRR   EEE     SSS   HHHHH  O   O  L      D   D         %
%         T    H   H  R R    E         SS  H   H  O   O  L      D   D         %
%         T    H   H  R  R   EEEEE  SSSSS  H   H   OOO   LLLLL  DDDD          %
%                                                                             %
%                                                                             %
%                      MagickCore Image Threshold Methods                     %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                 October 1996                                %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/property.h"
#include "MagickCore/blob.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/configure.h"
#include "MagickCore/constitute.h"
#include "MagickCore/decorate.h"
#include "MagickCore/draw.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/effect.h"
#include "MagickCore/fx.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/montage.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum.h"
#include "MagickCore/random_.h"
#include "MagickCore/random-private.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/segment.h"
#include "MagickCore/shear.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/threshold.h"
#include "MagickCore/token.h"
#include "MagickCore/transform.h"
#include "MagickCore/xml-tree.h"
#include "MagickCore/xml-tree-private.h"

/*
  Define declarations.
*/
#define ThresholdsFilename  "thresholds.xml"

/*
  Typedef declarations.
*/
struct _ThresholdMap
{
  char
    *map_id,
    *description;

  size_t
    width,
    height;

  ssize_t
    divisor,
    *levels;
};

/*
  Forward declarations.
*/
static ThresholdMap
  *GetThresholdMapFile(const char *,const char *,const char *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A d a p t i v e T h r e s h o l d I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AdaptiveThresholdImage() selects an individual threshold for each pixel
%  based on the range of intensity values in its local neighborhood.  This
%  allows for thresholding of an image whose global intensity histogram
%  doesn't contain distinctive peaks.
%
%  The format of the AdaptiveThresholdImage method is:
%
%      Image *AdaptiveThresholdImage(const Image *image,const size_t width,
%        const size_t height,const double bias,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o width: the width of the local neighborhood.
%
%    o height: the height of the local neighborhood.
%
%    o bias: the mean bias.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *AdaptiveThresholdImage(const Image *image,
  const size_t width,const size_t height,const double bias,
  ExceptionInfo *exception)
{
#define AdaptiveThresholdImageTag  "AdaptiveThreshold/Image"

  CacheView
    *image_view,
    *threshold_view;

  Image
    *threshold_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickSizeType
    number_pixels;

  ssize_t
    y;

  /*
    Initialize threshold image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if ((width % 2) == 0)
    ThrowImageException(OptionError,"KernelWidthMustBeAnOddNumber");
  threshold_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (threshold_image == (Image *) NULL)
    return((Image *) NULL);
  status=SetImageStorageClass(threshold_image,DirectClass,exception);
  if (status == MagickFalse)
    {
      threshold_image=DestroyImage(threshold_image);
      return((Image *) NULL);
    }
  /*
    Threshold image.
  */
  status=MagickTrue;
  progress=0;
  number_pixels=(MagickSizeType) width*height;
  image_view=AcquireCacheView(image);
  threshold_view=AcquireCacheView(threshold_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    ssize_t
      center;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) width/2L),y-(ssize_t)
      (height/2L),image->columns+width,height,exception);
    q=QueueCacheViewAuthenticPixels(threshold_view,0,y,threshold_image->columns,
      1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    center=(ssize_t) GetPixelChannels(image)*(image->columns+width)*(height/2L)+
      GetPixelChannels(image)*(width/2);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        MagickRealType
          mean,
          pixel;

        PixelChannel
          channel;

        PixelTrait
          threshold_traits,
          traits;

        register const Quantum
          *restrict pixels;

        register ssize_t
          u;

        ssize_t
          v;

        channel=GetPixelChannelMapChannel(image,i);
        traits=GetPixelChannelMapTraits(image,channel);
        threshold_traits=GetPixelChannelMapTraits(threshold_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (threshold_traits == UndefinedPixelTrait))
          continue;
        if (((threshold_traits & CopyPixelTrait) != 0) ||
            (GetPixelMask(image,p) != 0))
          {
            SetPixelChannel(threshold_image,channel,p[center+i],q);
            continue;
          }
        pixels=p;
        pixel=0.0;
        for (v=0; v < (ssize_t) height; v++)
        {
          for (u=0; u < (ssize_t) width; u++)
          {
            pixel+=pixels[i];
            pixels+=GetPixelChannels(image);
          }
          pixels+=image->columns*GetPixelChannels(image);
        }
        mean=(MagickRealType) (pixel/number_pixels+bias);
        SetPixelChannel(threshold_image,channel,(Quantum) ((MagickRealType)
          p[center+i] <= mean ? 0 : QuantumRange),q);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(threshold_image);
    }
    if (SyncCacheViewAuthenticPixels(threshold_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_AdaptiveThresholdImage)
#endif
        proceed=SetImageProgress(image,AdaptiveThresholdImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  threshold_image->type=image->type;
  threshold_view=DestroyCacheView(threshold_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    threshold_image=DestroyImage(threshold_image);
  return(threshold_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     B i l e v e l I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BilevelImage() changes the value of individual pixels based on the
%  intensity of each pixel channel.  The result is a high-contrast image.
%
%  More precisely each channel value of the image is 'thresholded' so that if
%  it is equal to or less than the given value it is set to zero, while any
%  value greater than that give is set to it maximum or QuantumRange.
%
%  This function is what is used to implement the "-threshold" operator for
%  the command line API.
%
%  If the default channel setting is given the image is thresholded using just
%  the gray 'intensity' of the image, rather than the individual channels.
%
%  The format of the BilevelImage method is:
%
%      MagickBooleanType BilevelImage(Image *image,const double threshold,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: define the threshold values.
%
%    o exception: return any errors or warnings in this structure.
%
%  Aside: You can get the same results as operator using LevelImages()
%  with the 'threshold' value for both the black_point and the white_point.
%
*/
MagickExport MagickBooleanType BilevelImage(Image *image,const double threshold,
  ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  /*
    Bilevel threshold image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,8) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register Quantum
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      if (GetPixelMask(image,q) != 0)
        {
          q+=GetPixelChannels(image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel
          channel;

        PixelTrait
          traits;

        channel=GetPixelChannelMapChannel(image,i);
        traits=GetPixelChannelMapTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=(Quantum) ((MagickRealType) q[i] <= threshold ? 0 : QuantumRange);
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_BilevelImage)
#endif
        proceed=SetImageProgress(image,ThresholdImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     B l a c k T h r e s h o l d I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BlackThresholdImage() is like ThresholdImage() but forces all pixels below
%  the threshold into black while leaving all pixels at or above the threshold
%  unchanged.
%
%  The format of the BlackThresholdImage method is:
%
%      MagickBooleanType BlackThresholdImage(Image *image,
%        const char *threshold,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: define the threshold value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType BlackThresholdImage(Image *image,
  const char *thresholds,ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  GeometryInfo
    geometry_info;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickRealType
    threshold[5];

  MagickStatusType
    flags;

  register ssize_t
    i;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (thresholds == (const char *) NULL)
    return(MagickTrue);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  flags=ParseGeometry(thresholds,&geometry_info);
  for (i=0; i < 5; i++)
    threshold[i]=geometry_info.rho;
  if ((flags & SigmaValue) != 0)
    threshold[1]=geometry_info.sigma;
  if ((flags & XiValue) != 0)
    threshold[2]=geometry_info.xi;
  if ((flags & PsiValue) != 0)
    threshold[3]=geometry_info.psi;
  if ((flags & ChiValue) != 0)
    threshold[4]=geometry_info.chi;
  if ((flags & PercentValue) != 0)
    for (i=0; i < 5; i++)
      threshold[i]*=(QuantumRange/100.0);
  /*
    White threshold image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,8) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register Quantum
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      ssize_t
        n;

      if (GetPixelMask(image,q) != 0)
        {
          q+=GetPixelChannels(image);
          continue;
        }
      n=0;
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel
          channel;

        PixelTrait
          traits;

        channel=GetPixelChannelMapChannel(image,i);
        traits=GetPixelChannelMapTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if ((MagickRealType) q[i] < threshold[n++ % 5])
          q[i]=QuantumRange;
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_WhiteThresholdImage)
#endif
        proceed=SetImageProgress(image,ThresholdImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C l a m p I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClampImage() restricts the color range from 0 to the quantum depth.
%
%  The format of the ClampImage method is:
%
%      MagickBooleanType ClampImage(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline Quantum ClampToUnsignedQuantum(const Quantum quantum)
{
#if defined(MAGICKCORE_HDRI_SUPPORT)
  if (quantum <= 0)
    return(0);
  if (quantum >= QuantumRange)
    return(QuantumRange);
  return(quantum);
#else
  return(quantum);
#endif
}

MagickExport MagickBooleanType ClampImage(Image *image,ExceptionInfo *exception)
{
#define ClampImageTag  "Clamp/Image"

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
    {
      register ssize_t
        i;

      register PixelInfo
        *restrict q;

      q=image->colormap;
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        q->red=(double) ClampToUnsignedQuantum(ClampToQuantum(q->red));
        q->green=(double) ClampToUnsignedQuantum(ClampToQuantum(q->green));
        q->blue=(double) ClampToUnsignedQuantum(ClampToQuantum(q->blue));
        q->alpha=(double) ClampToUnsignedQuantum(ClampToQuantum(q->alpha));
        q++;
      }
      return(SyncImage(image,exception));
    }
  /*
    Clamp image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,8) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register Quantum
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      if (GetPixelMask(image,q) != 0)
        {
          q+=GetPixelChannels(image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel
          channel;

        PixelTrait
          traits;

        channel=GetPixelChannelMapChannel(image,i);
        traits=GetPixelChannelMapTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        q[i]=ClampToUnsignedQuantum(q[i]);
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ClampImage)
#endif
        proceed=SetImageProgress(image,ClampImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  D e s t r o y T h r e s h o l d M a p                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyThresholdMap() de-allocate the given ThresholdMap
%
%  The format of the ListThresholdMaps method is:
%
%      ThresholdMap *DestroyThresholdMap(Threshold *map)
%
%  A description of each parameter follows.
%
%    o map:    Pointer to the Threshold map to destroy
%
*/
MagickExport ThresholdMap *DestroyThresholdMap(ThresholdMap *map)
{
  assert(map != (ThresholdMap *) NULL);
  if (map->map_id != (char *) NULL)
    map->map_id=DestroyString(map->map_id);
  if (map->description != (char *) NULL)
    map->description=DestroyString(map->description);
  if (map->levels != (ssize_t *) NULL)
    map->levels=(ssize_t *) RelinquishMagickMemory(map->levels);
  map=(ThresholdMap *) RelinquishMagickMemory(map);
  return(map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t T h r e s h o l d M a p                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetThresholdMap() loads and searches one or more threshold map files for the
%  map matching the given name or alias.
%
%  The format of the GetThresholdMap method is:
%
%      ThresholdMap *GetThresholdMap(const char *map_id,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o map_id:  ID of the map to look for.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ThresholdMap *GetThresholdMap(const char *map_id,
  ExceptionInfo *exception)
{
  const StringInfo
    *option;

  LinkedListInfo
    *options;

  ThresholdMap
    *map;

  map=(ThresholdMap *)NULL;
  options=GetConfigureOptions(ThresholdsFilename,exception);
  while ((option=(const StringInfo *) GetNextValueInLinkedList(options)) !=
         (const StringInfo *) NULL && (map == (ThresholdMap *) NULL))
    map=GetThresholdMapFile((const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),map_id,exception);
  options=DestroyConfigureOptions(options);
  return(map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  G e t T h r e s h o l d M a p F i l e                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetThresholdMapFile() look for a given threshold map name or alias in the
%  given XML file data, and return the allocated the map when found.
%
%  The format of the ListThresholdMaps method is:
%
%      ThresholdMap *GetThresholdMap(const char *xml,const char *filename,
%         const char *map_id,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o xml:  The threshold map list in XML format.
%
%    o filename:  The threshold map XML filename.
%
%    o map_id:  ID of the map to look for in XML list.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static ThresholdMap *GetThresholdMapFile(const char *xml,
  const char *filename,const char *map_id,ExceptionInfo *exception)
{
  char
    *p;

  const char
    *attribute,
    *content;

  double
    value;

  register ssize_t
    i;

  ThresholdMap
    *map;

  XMLTreeInfo
    *description,
    *levels,
    *threshold,
    *thresholds;

  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading threshold map file \"%s\" ...",filename);
  map=(ThresholdMap *) NULL;
  thresholds=NewXMLTree(xml,exception);
  if (thresholds == (XMLTreeInfo *) NULL)
    return(map);
  for (threshold=GetXMLTreeChild(thresholds,"threshold");
       threshold != (XMLTreeInfo *) NULL;
       threshold=GetNextXMLTreeTag(threshold))
  {
    attribute=GetXMLTreeAttribute(threshold,"map");
    if ((attribute != (char *) NULL) && (LocaleCompare(map_id,attribute) == 0))
      break;
    attribute=GetXMLTreeAttribute(threshold,"alias");
    if ((attribute != (char *) NULL) && (LocaleCompare(map_id,attribute) == 0))
      break;
  }
  if (threshold == (XMLTreeInfo *) NULL)
    return(map);
  description=GetXMLTreeChild(threshold,"description");
  if (description == (XMLTreeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingElement", "<description>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      return(map);
    }
  levels=GetXMLTreeChild(threshold,"levels");
  if (levels == (XMLTreeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingElement", "<levels>, map \"%s\"", map_id);
      thresholds=DestroyXMLTree(thresholds);
      return(map);
    }
  map=(ThresholdMap *) AcquireMagickMemory(sizeof(ThresholdMap));
  if (map == (ThresholdMap *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireThresholdMap");
  map->map_id=(char *) NULL;
  map->description=(char *) NULL;
  map->levels=(ssize_t *) NULL;
  attribute=GetXMLTreeAttribute(threshold,"map");
  if (attribute != (char *) NULL)
    map->map_id=ConstantString(attribute);
  content=GetXMLTreeContent(description);
  if (content != (char *) NULL)
    map->description=ConstantString(content);
  attribute=GetXMLTreeAttribute(levels,"width");
  if (attribute == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingAttribute", "<levels width>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  map->width=StringToUnsignedLong(attribute);
  if (map->width == 0)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
       "XmlInvalidAttribute", "<levels width>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  attribute=GetXMLTreeAttribute(levels,"height");
  if (attribute == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingAttribute", "<levels height>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  map->height=StringToUnsignedLong(attribute);
  if (map->height == 0)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlInvalidAttribute", "<levels height>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  attribute=GetXMLTreeAttribute(levels,"divisor");
  if (attribute == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingAttribute", "<levels divisor>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  map->divisor=(ssize_t) StringToLong(attribute);
  if (map->divisor < 2)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlInvalidAttribute", "<levels divisor>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  content=GetXMLTreeContent(levels);
  if (content == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingContent", "<levels>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  map->levels=(ssize_t *) AcquireQuantumMemory((size_t) map->width,map->height*
    sizeof(*map->levels));
  if (map->levels == (ssize_t *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireThresholdMap");
  for (i=0; i < (ssize_t) (map->width*map->height); i++)
  {
    map->levels[i]=(ssize_t) strtol(content,&p,10);
    if (p == content)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlInvalidContent", "<level> too few values, map \"%s\"",map_id);
        thresholds=DestroyXMLTree(thresholds);
        map=DestroyThresholdMap(map);
        return(map);
      }
    if ((map->levels[i] < 0) || (map->levels[i] > map->divisor))
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlInvalidContent", "<level> %.20g out of range, map \"%s\"",
          (double) map->levels[i],map_id);
        thresholds=DestroyXMLTree(thresholds);
        map=DestroyThresholdMap(map);
        return(map);
      }
    content=p;
  }
  value=(double) strtol(content,&p,10);
  (void) value;
  if (p != content)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlInvalidContent", "<level> too many values, map \"%s\"",map_id);
     thresholds=DestroyXMLTree(thresholds);
     map=DestroyThresholdMap(map);
     return(map);
   }
  thresholds=DestroyXMLTree(thresholds);
  return(map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  L i s t T h r e s h o l d M a p F i l e                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListThresholdMapFile() lists the threshold maps and their descriptions
%  in the given XML file data.
%
%  The format of the ListThresholdMaps method is:
%
%      MagickBooleanType ListThresholdMaps(FILE *file,const char*xml,
%         const char *filename,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to the output FILE.
%
%    o xml:  The threshold map list in XML format.
%
%    o filename:  The threshold map XML filename.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickBooleanType ListThresholdMapFile(FILE *file,const char *xml,
  const char *filename,ExceptionInfo *exception)
{
  const char
    *alias,
    *content,
    *map;

  XMLTreeInfo
    *description,
    *threshold,
    *thresholds;

  assert( xml != (char *)NULL );
  assert( file != (FILE *)NULL );
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading threshold map file \"%s\" ...",filename);
  thresholds=NewXMLTree(xml,exception);
  if ( thresholds == (XMLTreeInfo *)NULL )
    return(MagickFalse);
  (void) FormatLocaleFile(file,"%-16s %-12s %s\n","Map","Alias","Description");
  (void) FormatLocaleFile(file,
    "----------------------------------------------------\n");
  threshold=GetXMLTreeChild(thresholds,"threshold");
  for ( ; threshold != (XMLTreeInfo *) NULL;
          threshold=GetNextXMLTreeTag(threshold))
  {
    map=GetXMLTreeAttribute(threshold,"map");
    if (map == (char *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlMissingAttribute", "<map>");
        thresholds=DestroyXMLTree(thresholds);
        return(MagickFalse);
      }
    alias=GetXMLTreeAttribute(threshold,"alias");
    description=GetXMLTreeChild(threshold,"description");
    if (description == (XMLTreeInfo *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlMissingElement", "<description>, map \"%s\"",map);
        thresholds=DestroyXMLTree(thresholds);
        return(MagickFalse);
      }
    content=GetXMLTreeContent(description);
    if (content == (char *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlMissingContent", "<description>, map \"%s\"", map);
        thresholds=DestroyXMLTree(thresholds);
        return(MagickFalse);
      }
    (void) FormatLocaleFile(file,"%-16s %-12s %s\n",map,alias ? alias : "",
      content);
  }
  thresholds=DestroyXMLTree(thresholds);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t T h r e s h o l d M a p s                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListThresholdMaps() lists the threshold maps and their descriptions
%  as defined by "threshold.xml" to a file.
%
%  The format of the ListThresholdMaps method is:
%
%      MagickBooleanType ListThresholdMaps(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to the output FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListThresholdMaps(FILE *file,
  ExceptionInfo *exception)
{
  const StringInfo
    *option;

  LinkedListInfo
    *options;

  MagickStatusType
    status;

  status=MagickFalse;
  if (file == (FILE *) NULL)
    file=stdout;
  options=GetConfigureOptions(ThresholdsFilename,exception);
  (void) FormatLocaleFile(file,
    "\n   Threshold Maps for Ordered Dither Operations\n");
  while ((option=(const StringInfo *) GetNextValueInLinkedList(options)) !=
         (const StringInfo *) NULL)
  {
    (void) FormatLocaleFile(file,"\nPATH: %s\n\n",GetStringInfoPath(option));
    status|=ListThresholdMapFile(file,(const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),exception);
  }
  options=DestroyConfigureOptions(options);
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     O r d e r e d P o s t e r i z e I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OrderedPosterizeImage() will perform a ordered dither based on a number
%  of pre-defined dithering threshold maps, but over multiple intensity
%  levels, which can be different for different channels, according to the
%  input argument.
%
%  The format of the OrderedPosterizeImage method is:
%
%      MagickBooleanType OrderedPosterizeImage(Image *image,
%        const char *threshold_map,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold_map: A string containing the name of the threshold dither
%      map to use, followed by zero or more numbers representing the number
%      of color levels tho dither between.
%
%      Any level number less than 2 will be equivalent to 2, and means only
%      binary dithering will be applied to each color channel.
%
%      No numbers also means a 2 level (bitmap) dither will be applied to all
%      channels, while a single number is the number of levels applied to each
%      channel in sequence.  More numbers will be applied in turn to each of
%      the color channels.
%
%      For example: "o3x3,6" will generate a 6 level posterization of the
%      image with a ordered 3x3 diffused pixel dither being applied between
%      each level. While checker,8,8,4 will produce a 332 colormaped image
%      with only a single checkerboard hash pattern (50% grey) between each
%      color level, to basically double the number of color levels with
%      a bare minimim of dithering.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType OrderedPosterizeImage(Image *image,
  const char *threshold_map,ExceptionInfo *exception)
{
#define DitherImageTag  "Dither/Image"

  CacheView
    *image_view;

  char
    token[MaxTextExtent];

  const char
    *p;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickRealType
    levels[CompositePixelChannel];

  register ssize_t
    i;

  ssize_t
    y;

  ThresholdMap
    *map;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (threshold_map == (const char *) NULL)
    return(MagickTrue);
  p=(char *) threshold_map;
  while (((isspace((int) ((unsigned char) *p)) != 0) || (*p == ',')) &&
         (*p != '\0'))
    p++;
  threshold_map=p;
  while (((isspace((int) ((unsigned char) *p)) == 0) && (*p != ',')) &&
         (*p != '\0'))
  {
    if ((p-threshold_map) >= (MaxTextExtent-1))
      break;
    token[p-threshold_map]=(*p);
    p++;
  }
  token[p-threshold_map]='\0';
  map=GetThresholdMap(token,exception);
  if (map == (ThresholdMap *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "InvalidArgument","%s : '%s'","ordered-dither",threshold_map);
      return(MagickFalse);
    }
  for (i=0; i < MaxPixelChannels; i++)
    levels[i]=2.0;
  p=strchr((char *) threshold_map,',');
  if ((p != (char *) NULL) && (isdigit((int) ((unsigned char) *(++p))) != 0))
    for (i=0; (*p != '\0') && (i < MaxPixelChannels); i++)
    {
      GetMagickToken(p,&p,token);
      if (*token == ',')
        GetMagickToken(p,&p,token);
      levels[i]=StringToDouble(token,(char **) NULL);
    }
  for (i=0; i < MaxPixelChannels; i++)
    if (fabs(levels[i]) >= 1)
      levels[i]-=1.0;
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,8) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register Quantum
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      ssize_t
        n;

      n=0;
      if (GetPixelMask(image,q) != 0)
        {
          q+=GetPixelChannels(image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel
          channel;

        PixelTrait
          traits;

        ssize_t
          level,
          threshold;

        channel=GetPixelChannelMapChannel(image,i);
        traits=GetPixelChannelMapTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if (fabs(levels[n++]) < MagickEpsilon)
          continue;
        threshold=(ssize_t) (QuantumScale*q[i]*(levels[n]*(map->divisor-1)+1));
        level=threshold/(map->divisor-1);
        threshold-=level*(map->divisor-1);
        q[i]=RoundToQuantum((MagickRealType) (level+(threshold >=
          map->levels[(x % map->width)+map->width*(y % map->height)]))*
          QuantumRange/levels[n]);
        n++;
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
#pragma omp critical (MagickCore_OrderedPosterizeImage)
#endif
        proceed=SetImageProgress(image,DitherImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  map=DestroyThresholdMap(map);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     R a n d o m T h r e s h o l d I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RandomThresholdImage() changes the value of individual pixels based on the
%  intensity of each pixel compared to a random threshold.  The result is a
%  low-contrast, two color image.
%
%  The format of the RandomThresholdImage method is:
%
%      MagickBooleanType RandomThresholdImage(Image *image,
%        const char *thresholds,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o thresholds: a geometry string containing low,high thresholds.  If the
%      string contains 2x2, 3x3, or 4x4, an ordered dither of order 2, 3, or 4
%      is performed instead.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType RandomThresholdImage(Image *image,
  const char *thresholds,ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  GeometryInfo
    geometry_info;

  MagickStatusType
    flags;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    threshold;

  MagickRealType
    min_threshold,
    max_threshold;

  RandomInfo
    **restrict random_info;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (thresholds == (const char *) NULL)
    return(MagickTrue);
  GetPixelInfo(image,&threshold);
  min_threshold=0.0;
  max_threshold=(MagickRealType) QuantumRange;
  flags=ParseGeometry(thresholds,&geometry_info);
  min_threshold=geometry_info.rho;
  max_threshold=geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    max_threshold=min_threshold;
  if (strchr(thresholds,'%') != (char *) NULL)
    {
      max_threshold*=(MagickRealType) (0.01*QuantumRange);
      min_threshold*=(MagickRealType) (0.01*QuantumRange);
    }
  /*
    Random threshold image.
  */
  status=MagickTrue;
  progress=0;
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  random_info=AcquireRandomInfoThreadSet();
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,8) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      if (GetPixelMask(image,q) != 0)
        {
          q+=GetPixelChannels(image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        MagickRealType
          threshold;

        PixelChannel
          channel;

        PixelTrait
          traits;

        channel=GetPixelChannelMapChannel(image,i);
        traits=GetPixelChannelMapTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if ((MagickRealType) q[i] < min_threshold)
          threshold=min_threshold;
        else
          if ((MagickRealType) q[i] > max_threshold)
            threshold=max_threshold;
          else
            threshold=(MagickRealType) (QuantumRange*
              GetPseudoRandomValue(random_info[id]));
          q[i]=(Quantum) ((MagickRealType) q[i] <= threshold ? 0 :
            QuantumRange);
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_RandomThresholdImage)
#endif
        proceed=SetImageProgress(image,ThresholdImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  random_info=DestroyRandomInfoThreadSet(random_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     W h i t e T h r e s h o l d I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WhiteThresholdImage() is like ThresholdImage() but forces all pixels above
%  the threshold into white while leaving all pixels at or below the threshold
%  unchanged.
%
%  The format of the WhiteThresholdImage method is:
%
%      MagickBooleanType WhiteThresholdImage(Image *image,
%        const char *threshold,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: Define the threshold value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType WhiteThresholdImage(Image *image,
  const char *thresholds,ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  GeometryInfo
    geometry_info;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickRealType
    threshold[5];

  MagickStatusType
    flags;

  register ssize_t
    i;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (thresholds == (const char *) NULL)
    return(MagickTrue);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  flags=ParseGeometry(thresholds,&geometry_info);
  for (i=0; i < 5; i++)
    threshold[i]=geometry_info.rho;
  if ((flags & SigmaValue) != 0)
    threshold[1]=geometry_info.sigma;
  if ((flags & XiValue) != 0)
    threshold[2]=geometry_info.xi;
  if ((flags & PsiValue) != 0)
    threshold[3]=geometry_info.psi;
  if ((flags & ChiValue) != 0)
    threshold[4]=geometry_info.chi;
  if ((flags & PercentValue) != 0)
    for (i=0; i < 5; i++)
      threshold[i]*=(QuantumRange/100.0);
  /*
    White threshold image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,8) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register Quantum
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      ssize_t
        n;

      if (GetPixelMask(image,q) != 0)
        {
          q+=GetPixelChannels(image);
          continue;
        }
      n=0;
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel
          channel;

        PixelTrait
          traits;

        channel=GetPixelChannelMapChannel(image,i);
        traits=GetPixelChannelMapTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if ((MagickRealType) q[i] > threshold[n++ % 5])
          q[i]=QuantumRange;
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_WhiteThresholdImage)
#endif
        proceed=SetImageProgress(image,ThresholdImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

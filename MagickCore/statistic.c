/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        SSSSS  TTTTT   AAA   TTTTT  IIIII  SSSSS  TTTTT  IIIII   CCCC        %
%        SS       T    A   A    T      I    SS       T      I    C            %
%         SSS     T    AAAAA    T      I     SSS     T      I    C            %
%           SS    T    A   A    T      I       SS    T      I    C            %
%        SSSSS    T    A   A    T    IIIII  SSSSS    T    IIIII   CCCC        %
%                                                                             %
%                                                                             %
%                     MagickCore Image Statistical Methods                    %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/property.h"
#include "MagickCore/animate.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/client.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/compress.h"
#include "MagickCore/constitute.h"
#include "MagickCore/display.h"
#include "MagickCore/draw.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/gem-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/list.h"
#include "MagickCore/image-private.h"
#include "MagickCore/magic.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/random_.h"
#include "MagickCore/random-private.h"
#include "MagickCore/segment.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/timer.h"
#include "MagickCore/utility.h"
#include "MagickCore/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     E v a l u a t e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EvaluateImage() applies a value to the image with an arithmetic, relational,
%  or logical operator to an image. Use these operations to lighten or darken
%  an image, to increase or decrease contrast in an image, or to produce the
%  "negative" of an image.
%
%  The format of the EvaluateImage method is:
%
%      MagickBooleanType EvaluateImage(Image *image,
%        const MagickEvaluateOperator op,const double value,
%        ExceptionInfo *exception)
%      MagickBooleanType EvaluateImages(Image *images,
%        const MagickEvaluateOperator op,const double value,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o op: A channel op.
%
%    o value: A value value.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct _WenusInfo
{
  MagickRealType
    channel[MaxPixelChannels];
} WenusInfo;

static WenusInfo **DestroyPixelThreadSet(WenusInfo **pixels)
{
  register ssize_t
    i;

  assert(pixels != (WenusInfo **) NULL);
  for (i=0; i < (ssize_t) GetOpenMPMaximumThreads(); i++)
    if (pixels[i] != (WenusInfo *) NULL)
      pixels[i]=(WenusInfo *) RelinquishMagickMemory(pixels[i]);
  pixels=(WenusInfo **) RelinquishMagickMemory(pixels);
  return(pixels);
}

static WenusInfo **AcquirePixelThreadSet(const Image *image,
  const size_t number_images)
{
  register ssize_t
    i;

  WenusInfo
    **pixels;

  size_t
    length,
    number_threads;

  number_threads=GetOpenMPMaximumThreads();
  pixels=(WenusInfo **) AcquireQuantumMemory(number_threads,sizeof(*pixels));
  if (pixels == (WenusInfo **) NULL)
    return((WenusInfo **) NULL);
  (void) ResetMagickMemory(pixels,0,number_threads*sizeof(*pixels));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    register ssize_t
      j;

    length=image->columns;
    if (length < number_images)
      length=number_images;
    pixels[i]=(WenusInfo *) AcquireQuantumMemory(length,sizeof(**pixels));
    if (pixels[i] == (WenusInfo *) NULL)
      return(DestroyPixelThreadSet(pixels));
    for (j=0; j < (ssize_t) length; j++)
    {
      register ssize_t
        k;

      for (k=0; k < MaxPixelChannels; k++)
        pixels[i][j].channel[k]=0.0;
    }
  }
  return(pixels);
}

static inline double MagickMax(const double x,const double y)
{
  if (x > y)
    return(x);
  return(y);
}

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int IntensityCompare(const void *x,const void *y)
{
  const WenusInfo
    *color_1,
    *color_2;

  MagickRealType
    distance;

  register ssize_t
    i;

  color_1=(const WenusInfo *) x;
  color_2=(const WenusInfo *) y;
  distance=0.0;
  for (i=0; i < MaxPixelChannels; i++)
    distance+=color_1->channel[i]-(MagickRealType) color_2->channel[i];
  return(distance < 0 ? -1 : distance > 0 ? 1 : 0);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static inline double MagickMin(const double x,const double y)
{
  if (x < y)
    return(x);
  return(y);
}

static MagickRealType ApplyEvaluateOperator(RandomInfo *random_info,
  Quantum pixel,const MagickEvaluateOperator op,const MagickRealType value)
{
  MagickRealType
    result;

  result=0.0;
  switch (op)
  {
    case UndefinedEvaluateOperator:
      break;
    case AbsEvaluateOperator:
    {
      result=(MagickRealType) fabs((double) (pixel+value));
      break;
    }
    case AddEvaluateOperator:
    {
      result=(MagickRealType) (pixel+value);
      break;
    }
    case AddModulusEvaluateOperator:
    {
      /*
        This returns a 'floored modulus' of the addition which is a
        positive result.  It differs from % or fmod() that returns a
        'truncated modulus' result, where floor() is replaced by trunc() and
        could return a negative result (which is clipped).
      */
      result=pixel+value;
      result-=(QuantumRange+1.0)*floor((double) result/(QuantumRange+1.0));
      break;
    }
    case AndEvaluateOperator:
    {
      result=(MagickRealType) ((size_t) pixel & (size_t) (value+0.5));
      break;
    }
    case CosineEvaluateOperator:
    {
      result=(MagickRealType) (QuantumRange*(0.5*cos((double) (2.0*MagickPI*
        QuantumScale*pixel*value))+0.5));
      break;
    }
    case DivideEvaluateOperator:
    {
      result=pixel/(value == 0.0 ? 1.0 : value);
      break;
    }
    case ExponentialEvaluateOperator:
    {
      result=(MagickRealType) (QuantumRange*exp((double) (value*QuantumScale*
        pixel)));
      break;
    }
    case GaussianNoiseEvaluateOperator:
    {
      result=(MagickRealType) GenerateDifferentialNoise(random_info,pixel,
        GaussianNoise,value);
      break;
    }
    case ImpulseNoiseEvaluateOperator:
    {
      result=(MagickRealType) GenerateDifferentialNoise(random_info,pixel,
        ImpulseNoise,value);
      break;
    }
    case LaplacianNoiseEvaluateOperator:
    {
      result=(MagickRealType) GenerateDifferentialNoise(random_info,pixel,
        LaplacianNoise,value);
      break;
    }
    case LeftShiftEvaluateOperator:
    {
      result=(MagickRealType) ((size_t) pixel << (size_t) (value+0.5));
      break;
    }
    case LogEvaluateOperator:
    {
      result=(MagickRealType) (QuantumRange*log((double) (QuantumScale*value*
        pixel+1.0))/log((double) (value+1.0)));
      break;
    }
    case MaxEvaluateOperator:
    {
      result=(MagickRealType) MagickMax((double) pixel,value);
      break;
    }
    case MeanEvaluateOperator:
    {
      result=(MagickRealType) (pixel+value);
      break;
    }
    case MedianEvaluateOperator:
    {
      result=(MagickRealType) (pixel+value);
      break;
    }
    case MinEvaluateOperator:
    {
      result=(MagickRealType) MagickMin((double) pixel,value);
      break;
    }
    case MultiplicativeNoiseEvaluateOperator:
    {
      result=(MagickRealType) GenerateDifferentialNoise(random_info,pixel,
        MultiplicativeGaussianNoise,value);
      break;
    }
    case MultiplyEvaluateOperator:
    {
      result=(MagickRealType) (value*pixel);
      break;
    }
    case OrEvaluateOperator:
    {
      result=(MagickRealType) ((size_t) pixel | (size_t) (value+0.5));
      break;
    }
    case PoissonNoiseEvaluateOperator:
    {
      result=(MagickRealType) GenerateDifferentialNoise(random_info,pixel,
        PoissonNoise,value);
      break;
    }
    case PowEvaluateOperator:
    {
      result=(MagickRealType) (QuantumRange*pow((double) (QuantumScale*pixel),
        (double) value));
      break;
    }
    case RightShiftEvaluateOperator:
    {
      result=(MagickRealType) ((size_t) pixel >> (size_t) (value+0.5));
      break;
    }
    case SetEvaluateOperator:
    {
      result=value;
      break;
    }
    case SineEvaluateOperator:
    {
      result=(MagickRealType) (QuantumRange*(0.5*sin((double) (2.0*MagickPI*
        QuantumScale*pixel*value))+0.5));
      break;
    }
    case SubtractEvaluateOperator:
    {
      result=(MagickRealType) (pixel-value);
      break;
    }
    case ThresholdEvaluateOperator:
    {
      result=(MagickRealType) (((MagickRealType) pixel <= value) ? 0 :
        QuantumRange);
      break;
    }
    case ThresholdBlackEvaluateOperator:
    {
      result=(MagickRealType) (((MagickRealType) pixel <= value) ? 0 : pixel);
      break;
    }
    case ThresholdWhiteEvaluateOperator:
    {
      result=(MagickRealType) (((MagickRealType) pixel > value) ? QuantumRange :
        pixel);
      break;
    }
    case UniformNoiseEvaluateOperator:
    {
      result=(MagickRealType) GenerateDifferentialNoise(random_info,pixel,
        UniformNoise,value);
      break;
    }
    case XorEvaluateOperator:
    {
      result=(MagickRealType) ((size_t) pixel ^ (size_t) (value+0.5));
      break;
    }
  }
  return(result);
}

MagickExport Image *EvaluateImages(const Image *images,
  const MagickEvaluateOperator op,ExceptionInfo *exception)
{
#define EvaluateImageTag  "Evaluate/Image"

  CacheView
    *evaluate_view;

  const Image
    *next;

  Image
    *evaluate_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  WenusInfo
    **restrict evaluate_pixels;

  RandomInfo
    **restrict random_info;

  size_t
    number_images;

  ssize_t
    y;

  /*
    Ensure the image are the same size.
  */
  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  for (next=images; next != (Image *) NULL; next=GetNextImageInList(next))
    if ((next->columns != images->columns) || (next->rows != images->rows))
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "ImageWidthsOrHeightsDiffer","`%s'",images->filename);
        return((Image *) NULL);
      }
  /*
    Initialize evaluate next attributes.
  */
  evaluate_image=CloneImage(images,images->columns,images->rows,MagickTrue,
    exception);
  if (evaluate_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(evaluate_image,DirectClass,exception) == MagickFalse)
    {
      evaluate_image=DestroyImage(evaluate_image);
      return((Image *) NULL);
    }
  number_images=GetImageListLength(images);
  evaluate_pixels=AcquirePixelThreadSet(images,number_images);
  if (evaluate_pixels == (WenusInfo **) NULL)
    {
      evaluate_image=DestroyImage(evaluate_image);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",images->filename);
      return((Image *) NULL);
    }
  /*
    Evaluate image pixels.
  */
  status=MagickTrue;
  progress=0;
  random_info=AcquireRandomInfoThreadSet();
  evaluate_view=AcquireCacheView(evaluate_image);
  if (op == MedianEvaluateOperator)
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic) shared(progress,status)
#endif
    for (y=0; y < (ssize_t) evaluate_image->rows; y++)
    {
      CacheView
        *image_view;

      const Image
        *next;

      const int
        id = GetOpenMPThreadId();

      register WenusInfo
        *evaluate_pixel;

      register Quantum
        *restrict q;

      register ssize_t
        x;

      if (status == MagickFalse)
        continue;
      q=QueueCacheViewAuthenticPixels(evaluate_view,0,y,evaluate_image->columns,
        1,exception);
      if (q == (Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      evaluate_pixel=evaluate_pixels[id];
      for (x=0; x < (ssize_t) evaluate_image->columns; x++)
      {
        register ssize_t
          j,
          k;

        for (j=0; j < (ssize_t) number_images; j++)
          for (k=0; k < MaxPixelChannels; k++)
            evaluate_pixel[j].channel[k]=0.0;
        next=images;
        for (j=0; j < (ssize_t) number_images; j++)
        {
          register const Quantum
            *p;

          register ssize_t
            i;

          image_view=AcquireCacheView(next);
          p=GetCacheViewVirtualPixels(image_view,x,y,1,1,exception);
          if (p == (const Quantum *) NULL)
            {
              image_view=DestroyCacheView(image_view);
              break;
            }
          for (i=0; i < (ssize_t) GetPixelChannels(evaluate_image); i++)
          {
            PixelChannel
              channel;

            PixelTrait
              evaluate_traits,
              traits;

            evaluate_traits=GetPixelChannelMapTraits(evaluate_image,
              (PixelChannel) i);
            channel=GetPixelChannelMapChannel(evaluate_image,(PixelChannel) i);
            traits=GetPixelChannelMapTraits(next,channel);
            if ((traits == UndefinedPixelTrait) ||
                (evaluate_traits == UndefinedPixelTrait))
              continue;
            if ((evaluate_traits & UpdatePixelTrait) == 0)
              continue;
            evaluate_pixel[j].channel[i]=ApplyEvaluateOperator(random_info[id],
              GetPixelChannel(evaluate_image,channel,p),op,
              evaluate_pixel[j].channel[i]);
          }
          image_view=DestroyCacheView(image_view);
          next=GetNextImageInList(next);
        }
        qsort((void *) evaluate_pixel,number_images,sizeof(*evaluate_pixel),
          IntensityCompare);
        for (k=0; k < (ssize_t) GetPixelChannels(evaluate_image); k++)
          q[k]=ClampToQuantum(evaluate_pixel[j/2].channel[k]);
        q+=GetPixelChannels(evaluate_image);
      }
      if (SyncCacheViewAuthenticPixels(evaluate_view,exception) == MagickFalse)
        status=MagickFalse;
      if (images->progress_monitor != (MagickProgressMonitor) NULL)
        {
          MagickBooleanType
            proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
          #pragma omp critical (MagickCore_EvaluateImages)
#endif
          proceed=SetImageProgress(images,EvaluateImageTag,progress++,
            evaluate_image->rows);
          if (proceed == MagickFalse)
            status=MagickFalse;
        }
    }
  else
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp parallel for schedule(dynamic) shared(progress,status)
#endif
    for (y=0; y < (ssize_t) evaluate_image->rows; y++)
    {
      CacheView
        *image_view;

      const Image
        *next;

      const int
        id = GetOpenMPThreadId();

      register ssize_t
        i,
        x;

      register WenusInfo
        *evaluate_pixel;

      register Quantum
        *restrict q;

      ssize_t
        j;

      if (status == MagickFalse)
        continue;
      q=QueueCacheViewAuthenticPixels(evaluate_view,0,y,evaluate_image->columns,
        1,exception);
      if (q == (Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      evaluate_pixel=evaluate_pixels[id];
      for (j=0; j < (ssize_t) evaluate_image->columns; j++)
        for (i=0; i < MaxPixelChannels; i++)
          evaluate_pixel[j].channel[i]=0.0;
      next=images;
      for (j=0; j < (ssize_t) number_images; j++)
      {
        register const Quantum
          *p;

        image_view=AcquireCacheView(next);
        p=GetCacheViewVirtualPixels(image_view,0,y,next->columns,1,exception);
        if (p == (const Quantum *) NULL)
          {
            image_view=DestroyCacheView(image_view);
            break;
          }
        for (x=0; x < (ssize_t) next->columns; x++)
        {
          register ssize_t
            i;

          for (i=0; i < (ssize_t) GetPixelChannels(evaluate_image); i++)
          {
            PixelChannel
              channel;

            PixelTrait
              evaluate_traits,
              traits;

            evaluate_traits=GetPixelChannelMapTraits(evaluate_image,
              (PixelChannel) i);
            channel=GetPixelChannelMapChannel(evaluate_image,(PixelChannel) i);
            traits=GetPixelChannelMapTraits(next,channel);
            if ((traits == UndefinedPixelTrait) ||
                (evaluate_traits == UndefinedPixelTrait))
              continue;
            if ((traits & UpdatePixelTrait) == 0)
              continue;
            evaluate_pixel[x].channel[i]=ApplyEvaluateOperator(random_info[id],
              GetPixelChannel(evaluate_image,channel,p),j == 0 ?
              AddEvaluateOperator : op,evaluate_pixel[x].channel[i]);
          }
          p+=GetPixelChannels(next);
        }
        image_view=DestroyCacheView(image_view);
        next=GetNextImageInList(next);
      }
      for (x=0; x < (ssize_t) evaluate_image->columns; x++)
      {
        register ssize_t
           i;

        switch (op)
        {
          case MeanEvaluateOperator:
          {
            for (i=0; i < (ssize_t) GetPixelChannels(evaluate_image); i++)
              evaluate_pixel[x].channel[i]/=(MagickRealType) number_images;
            break;
          }
          case MultiplyEvaluateOperator:
          {
            for (i=0; i < (ssize_t) GetPixelChannels(evaluate_image); i++)
            {
              register ssize_t
                j;

              for (j=0; j < (ssize_t) (number_images-1); j++)
                evaluate_pixel[x].channel[i]*=QuantumScale;
            }
            break;
          }
          default:
            break;
        }
      }
      for (x=0; x < (ssize_t) evaluate_image->columns; x++)
      {
        register ssize_t
          i;

        for (i=0; i < (ssize_t) GetPixelChannels(evaluate_image); i++)
        {
          PixelTrait
            traits;

          traits=GetPixelChannelMapTraits(evaluate_image,(PixelChannel) i);
          if (traits == UndefinedPixelTrait)
            continue;
          if ((traits & UpdatePixelTrait) == 0)
            continue;
          q[i]=ClampToQuantum(evaluate_pixel[x].channel[i]);
        }
        q+=GetPixelChannels(evaluate_image);
      }
      if (SyncCacheViewAuthenticPixels(evaluate_view,exception) == MagickFalse)
        status=MagickFalse;
      if (images->progress_monitor != (MagickProgressMonitor) NULL)
        {
          MagickBooleanType
            proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
          #pragma omp critical (MagickCore_EvaluateImages)
#endif
          proceed=SetImageProgress(images,EvaluateImageTag,progress++,
            evaluate_image->rows);
          if (proceed == MagickFalse)
            status=MagickFalse;
        }
    }
  evaluate_view=DestroyCacheView(evaluate_view);
  evaluate_pixels=DestroyPixelThreadSet(evaluate_pixels);
  random_info=DestroyRandomInfoThreadSet(random_info);
  if (status == MagickFalse)
    evaluate_image=DestroyImage(evaluate_image);
  return(evaluate_image);
}

MagickExport MagickBooleanType EvaluateImage(Image *image,
  const MagickEvaluateOperator op,const double value,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

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
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  random_info=AcquireRandomInfoThreadSet();
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
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

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelTrait
          traits;

        traits=GetPixelChannelMapTraits(image,(PixelChannel) i);
        if (traits == UndefinedPixelTrait)
          continue;
        q[i]=ClampToQuantum(ApplyEvaluateOperator(random_info[id],q[i],op,
          value));
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
  #pragma omp critical (MagickCore_EvaluateImage)
#endif
        proceed=SetImageProgress(image,EvaluateImageTag,progress++,image->rows);
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
%     F u n c t i o n I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FunctionImage() applies a value to the image with an arithmetic, relational,
%  or logical operator to an image. Use these operations to lighten or darken
%  an image, to increase or decrease contrast in an image, or to produce the
%  "negative" of an image.
%
%  The format of the FunctionImage method is:
%
%      MagickBooleanType FunctionImage(Image *image,
%        const MagickFunction function,const ssize_t number_parameters,
%        const double *parameters,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o function: A channel function.
%
%    o parameters: one or more parameters.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static Quantum ApplyFunction(Quantum pixel,const MagickFunction function,
  const size_t number_parameters,const double *parameters,
  ExceptionInfo *exception)
{
  MagickRealType
    result;

  register ssize_t
    i;

  (void) exception;
  result=0.0;
  switch (function)
  {
    case PolynomialFunction:
    {
      /*
        Polynomial: polynomial constants, highest to lowest order
        (e.g. c0*x^3 + c1*x^2 + c2*x + c3).
      */
      result=0.0;
      for (i=0; i < (ssize_t) number_parameters; i++)
        result=result*QuantumScale*pixel+parameters[i];
      result*=QuantumRange;
      break;
    }
    case SinusoidFunction:
    {
      MagickRealType
        amplitude,
        bias,
        frequency,
        phase;

      /*
        Sinusoid: frequency, phase, amplitude, bias.
      */
      frequency=(number_parameters >= 1) ? parameters[0] : 1.0;
      phase=(number_parameters >= 2) ? parameters[1] : 0.0;
      amplitude=(number_parameters >= 3) ? parameters[2] : 0.5;
      bias=(number_parameters >= 4) ? parameters[3] : 0.5;
      result=(MagickRealType) (QuantumRange*(amplitude*sin((double) (2.0*
        MagickPI*(frequency*QuantumScale*pixel+phase/360.0)))+bias));
      break;
    }
    case ArcsinFunction:
    {
      MagickRealType
        bias,
        center,
        range,
        width;

      /*
        Arcsin (peged at range limits for invalid results):
        width, center, range, and bias.
      */
      width=(number_parameters >= 1) ? parameters[0] : 1.0;
      center=(number_parameters >= 2) ? parameters[1] : 0.5;
      range=(number_parameters >= 3) ? parameters[2] : 1.0;
      bias=(number_parameters >= 4) ? parameters[3] : 0.5;
      result=2.0/width*(QuantumScale*pixel-center);
      if ( result <= -1.0 )
        result=bias-range/2.0;
      else
        if (result >= 1.0)
          result=bias+range/2.0;
        else
          result=(MagickRealType) (range/MagickPI*asin((double) result)+bias);
      result*=QuantumRange;
      break;
    }
    case ArctanFunction:
    {
      MagickRealType
        center,
        bias,
        range,
        slope;

      /*
        Arctan: slope, center, range, and bias.
      */
      slope=(number_parameters >= 1) ? parameters[0] : 1.0;
      center=(number_parameters >= 2) ? parameters[1] : 0.5;
      range=(number_parameters >= 3) ? parameters[2] : 1.0;
      bias=(number_parameters >= 4) ? parameters[3] : 0.5;
      result=(MagickRealType) (MagickPI*slope*(QuantumScale*pixel-center));
      result=(MagickRealType) (QuantumRange*(range/MagickPI*atan((double)
        result)+bias));
      break;
    }
    case UndefinedFunction:
      break;
  }
  return(ClampToQuantum(result));
}

MagickExport MagickBooleanType FunctionImage(Image *image,
  const MagickFunction function,const size_t number_parameters,
  const double *parameters,ExceptionInfo *exception)
{
#define FunctionImageTag  "Function/Image "

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
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
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

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelTrait
          traits;

        traits=GetPixelChannelMapTraits(image,(PixelChannel) i);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ApplyFunction(q[i],function,number_parameters,parameters,
          exception);
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
  #pragma omp critical (MagickCore_FunctionImage)
#endif
        proceed=SetImageProgress(image,FunctionImageTag,progress++,image->rows);
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
%   G e t I m a g e E x t r e m a                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageExtrema() returns the extrema of one or more image channels.
%
%  The format of the GetImageExtrema method is:
%
%      MagickBooleanType GetImageExtrema(const Image *image,size_t *minima,
%        size_t *maxima,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o minima: the minimum value in the channel.
%
%    o maxima: the maximum value in the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetImageExtrema(const Image *image,
  size_t *minima,size_t *maxima,ExceptionInfo *exception)
{
  double
    max,
    min;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=GetImageRange(image,&min,&max,exception);
  *minima=(size_t) ceil(min-0.5);
  *maxima=(size_t) floor(max+0.5);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e M e a n                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageMean() returns the mean and standard deviation of one or more
%  image channels.
%
%  The format of the GetImageMean method is:
%
%      MagickBooleanType GetImageMean(const Image *image,double *mean,
%        double *standard_deviation,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o mean: the average value in the channel.
%
%    o standard_deviation: the standard deviation of the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetImageMean(const Image *image,double *mean,
  double *standard_deviation,ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  register ssize_t
    i;

  size_t
    area;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=GetImageStatistics(image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    return(MagickFalse);
  area=0;
  channel_statistics[MaxPixelChannels].mean=0.0;
  channel_statistics[MaxPixelChannels].standard_deviation=0.0;
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelTrait
      traits;

    traits=GetPixelChannelMapTraits(image,(PixelChannel) i);
    if (traits == UndefinedPixelTrait)
      continue;
    if ((traits & UpdatePixelTrait) == 0)
      continue;
    channel_statistics[MaxPixelChannels].mean+=channel_statistics[i].mean;
    channel_statistics[MaxPixelChannels].standard_deviation+=
      channel_statistics[i].variance-channel_statistics[i].mean*
      channel_statistics[i].mean;
    area++;
  }
  channel_statistics[MaxPixelChannels].mean/=area;
  channel_statistics[MaxPixelChannels].standard_deviation=
    sqrt(channel_statistics[MaxPixelChannels].standard_deviation/area);
  *mean=channel_statistics[MaxPixelChannels].mean;
  *standard_deviation=channel_statistics[MaxPixelChannels].standard_deviation;
  channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
    channel_statistics);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e K u r t o s i s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageKurtosis() returns the kurtosis and skewness of one or more
%  image channels.
%
%  The format of the GetImageKurtosis method is:
%
%      MagickBooleanType GetImageKurtosis(const Image *image,double *kurtosis,
%        double *skewness,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o kurtosis: the kurtosis of the channel.
%
%    o skewness: the skewness of the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetImageKurtosis(const Image *image,
  double *kurtosis,double *skewness,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  double
    area,
    mean,
    standard_deviation,
    sum_squares,
    sum_cubes,
    sum_fourth_power;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=MagickTrue;
  *kurtosis=0.0;
  *skewness=0.0;
  area=0.0;
  mean=0.0;
  standard_deviation=0.0;
  sum_squares=0.0;
  sum_cubes=0.0;
  sum_fourth_power=0.0;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic) shared(status) omp_throttle(1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *restrict p;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelTrait
          traits;

        traits=GetPixelChannelMapTraits(image,(PixelChannel) i);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & UpdatePixelTrait) == 0)
          continue;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_GetImageKurtosis)
#endif
        {
          mean+=p[i];
          sum_squares+=(double) p[i]*p[i];
          sum_cubes+=(double) p[i]*p[i]*p[i];
          sum_fourth_power+=(double) p[i]*p[i]*p[i]*p[i];
          area++;
        }
      }
      p+=GetPixelChannels(image);
    }
  }
  image_view=DestroyCacheView(image_view);
  if (area != 0.0)
    {
      mean/=area;
      sum_squares/=area;
      sum_cubes/=area;
      sum_fourth_power/=area;
    }
  standard_deviation=sqrt(sum_squares-(mean*mean));
  if (standard_deviation != 0.0)
    {
      *kurtosis=sum_fourth_power-4.0*mean*sum_cubes+6.0*mean*mean*sum_squares-
        3.0*mean*mean*mean*mean;
      *kurtosis/=standard_deviation*standard_deviation*standard_deviation*
        standard_deviation;
      *kurtosis-=3.0;
      *skewness=sum_cubes-3.0*mean*sum_squares+2.0*mean*mean*mean;
      *skewness/=standard_deviation*standard_deviation*standard_deviation;
    }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e R a n g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageRange() returns the range of one or more image channels.
%
%  The format of the GetImageRange method is:
%
%      MagickBooleanType GetImageRange(const Image *image,double *minima,
%        double *maxima,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o minima: the minimum value in the channel.
%
%    o maxima: the maximum value in the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetImageRange(const Image *image,double *minima,
  double *maxima,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=MagickTrue;
  *maxima=(-MagickHuge);
  *minima=MagickHuge;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic) shared(status) omp_throttle(1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *restrict p;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelTrait
          traits;

        traits=GetPixelChannelMapTraits(image,(PixelChannel) i);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & UpdatePixelTrait) == 0)
          continue;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_GetImageRange)
#endif
        {
          if (p[i] < *minima)
            *minima=(double) p[i];
          if (p[i] > *maxima)
            *maxima=(double) p[i];
        }
      }
      p+=GetPixelChannels(image);
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
%   G e t I m a g e S t a t i s t i c s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageStatistics() returns statistics for each channel in the
%  image.  The statistics include the channel depth, its minima, maxima, mean,
%  standard deviation, kurtosis and skewness.  You can access the red channel
%  mean, for example, like this:
%
%      channel_statistics=GetImageStatistics(image,exception);
%      red_mean=channel_statistics[RedPixelChannel].mean;
%
%  Use MagickRelinquishMemory() to free the statistics buffer.
%
%  The format of the GetImageStatistics method is:
%
%      ChannelStatistics *GetImageStatistics(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static size_t GetImageChannels(const Image *image)
{
  register ssize_t
    i;

  size_t
    channels;

  channels=0;
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelTrait
      traits;

    traits=GetPixelChannelMapTraits(image,(PixelChannel) i);
    if ((traits & UpdatePixelTrait) != 0)
      channels++;
  }
  return(channels);
}

MagickExport ChannelStatistics *GetImageStatistics(const Image *image,
  ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  double
    area;

  MagickStatusType
    status;

  QuantumAny
    range;

  register ssize_t
    i;

  size_t
    channels,
    depth;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=(ChannelStatistics *) AcquireQuantumMemory(
    MaxPixelChannels+1,sizeof(*channel_statistics));
  if (channel_statistics == (ChannelStatistics *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(channel_statistics,0,(MaxPixelChannels+1)*
    sizeof(*channel_statistics));
  for (i=0; i <= (ssize_t) MaxPixelChannels; i++)
  {
    channel_statistics[i].depth=1;
    channel_statistics[i].maxima=(-MagickHuge);
    channel_statistics[i].minima=MagickHuge;
  }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *restrict p;

    register ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelTrait
          traits;

        traits=GetPixelChannelMapTraits(image,(PixelChannel) i);
        if (traits == UndefinedPixelTrait)
          continue;
        if (channel_statistics[i].depth != MAGICKCORE_QUANTUM_DEPTH)
          {
            depth=channel_statistics[i].depth;
            range=GetQuantumRange(depth);
            status=p[i] != ScaleAnyToQuantum(ScaleQuantumToAny(p[i],range),
              range) ? MagickTrue : MagickFalse;
            if (status != MagickFalse)
              {
                channel_statistics[i].depth++;
                continue;
              }
          }
        if ((double) p[i] < channel_statistics[i].minima)
          channel_statistics[i].minima=(double) p[i];
        if ((double) p[i] > channel_statistics[i].maxima)
          channel_statistics[i].maxima=(double) p[i];
        channel_statistics[i].sum+=p[i];
        channel_statistics[i].sum_squared+=(double) p[i]*p[i];
        channel_statistics[i].sum_cubed+=(double) p[i]*p[i]*p[i];
        channel_statistics[i].sum_fourth_power+=(double) p[i]*p[i]*p[i]*p[i];
      }
      p+=GetPixelChannels(image);
    }
  }
  area=(double) image->columns*image->rows;
  for (i=0; i < (ssize_t) MaxPixelChannels; i++)
  {
    channel_statistics[i].sum/=area;
    channel_statistics[i].sum_squared/=area;
    channel_statistics[i].sum_cubed/=area;
    channel_statistics[i].sum_fourth_power/=area;
    channel_statistics[i].mean=channel_statistics[i].sum;
    channel_statistics[i].variance=channel_statistics[i].sum_squared;
    channel_statistics[i].standard_deviation=sqrt(
      channel_statistics[i].variance-(channel_statistics[i].mean*
      channel_statistics[i].mean));
  }
  for (i=0; i < (ssize_t) MaxPixelChannels; i++)
  {
    channel_statistics[MaxPixelChannels].depth=(size_t) MagickMax((double)
      channel_statistics[MaxPixelChannels].depth,(double)
      channel_statistics[i].depth);
    channel_statistics[MaxPixelChannels].minima=MagickMin(
      channel_statistics[MaxPixelChannels].minima,
      channel_statistics[i].minima);
    channel_statistics[MaxPixelChannels].maxima=MagickMax(
      channel_statistics[MaxPixelChannels].maxima,
      channel_statistics[i].maxima);
    channel_statistics[MaxPixelChannels].sum+=channel_statistics[i].sum;
    channel_statistics[MaxPixelChannels].sum_squared+=
      channel_statistics[i].sum_squared;
    channel_statistics[MaxPixelChannels].sum_cubed+=
      channel_statistics[i].sum_cubed;
    channel_statistics[MaxPixelChannels].sum_fourth_power+=
      channel_statistics[i].sum_fourth_power;
    channel_statistics[MaxPixelChannels].mean+=channel_statistics[i].mean;
    channel_statistics[MaxPixelChannels].variance+=
      channel_statistics[i].variance-channel_statistics[i].mean*
      channel_statistics[i].mean;
    channel_statistics[MaxPixelChannels].standard_deviation+=
      channel_statistics[i].variance-channel_statistics[i].mean*
      channel_statistics[i].mean;
  }
  channels=GetImageChannels(image);
  channel_statistics[MaxPixelChannels].sum/=channels;
  channel_statistics[MaxPixelChannels].sum_squared/=channels;
  channel_statistics[MaxPixelChannels].sum_cubed/=channels;
  channel_statistics[MaxPixelChannels].sum_fourth_power/=channels;
  channel_statistics[MaxPixelChannels].mean/=channels;
  channel_statistics[MaxPixelChannels].variance/=channels;
  channel_statistics[MaxPixelChannels].standard_deviation=
    sqrt(channel_statistics[MaxPixelChannels].standard_deviation/channels);
  channel_statistics[MaxPixelChannels].kurtosis/=channels;
  channel_statistics[MaxPixelChannels].skewness/=channels;
  for (i=0; i <= (ssize_t) MaxPixelChannels; i++)
  {
    if (channel_statistics[i].standard_deviation == 0.0)
      continue;
    channel_statistics[i].skewness=(channel_statistics[i].sum_cubed-
      3.0*channel_statistics[i].mean*channel_statistics[i].sum_squared+
      2.0*channel_statistics[i].mean*channel_statistics[i].mean*
      channel_statistics[i].mean)/(channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation);
    channel_statistics[i].kurtosis=(channel_statistics[i].sum_fourth_power-
      4.0*channel_statistics[i].mean*channel_statistics[i].sum_cubed+
      6.0*channel_statistics[i].mean*channel_statistics[i].mean*
      channel_statistics[i].sum_squared-3.0*channel_statistics[i].mean*
      channel_statistics[i].mean*1.0*channel_statistics[i].mean*
      channel_statistics[i].mean)/(channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation)-3.0;
  }
  return(channel_statistics);
}

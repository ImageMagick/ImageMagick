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

typedef struct _PixelChannels
{
  MagickRealType
    channel[CompositePixelChannel];
} PixelChannels;

static PixelChannels **DestroyPixelThreadSet(PixelChannels **pixels)
{
  register ssize_t
    i;

  assert(pixels != (PixelChannels **) NULL);
  for (i=0; i < (ssize_t) GetOpenMPMaximumThreads(); i++)
    if (pixels[i] != (PixelChannels *) NULL)
      pixels[i]=(PixelChannels *) RelinquishMagickMemory(pixels[i]);
  pixels=(PixelChannels **) RelinquishMagickMemory(pixels);
  return(pixels);
}

static PixelChannels **AcquirePixelThreadSet(const Image *image,
  const size_t number_images)
{
  register ssize_t
    i;

  PixelChannels
    **pixels;

  size_t
    length,
    number_threads;

  number_threads=GetOpenMPMaximumThreads();
  pixels=(PixelChannels **) AcquireQuantumMemory(number_threads,
    sizeof(*pixels));
  if (pixels == (PixelChannels **) NULL)
    return((PixelChannels **) NULL);
  (void) ResetMagickMemory(pixels,0,number_threads*sizeof(*pixels));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    register ssize_t
      j;

    length=image->columns;
    if (length < number_images)
      length=number_images;
    pixels[i]=(PixelChannels *) AcquireQuantumMemory(length,sizeof(**pixels));
    if (pixels[i] == (PixelChannels *) NULL)
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

static inline double EvaluateMax(const double x,const double y)
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
  const PixelChannels
    *color_1,
    *color_2;

  MagickRealType
    distance;

  register ssize_t
    i;

  color_1=(const PixelChannels *) x;
  color_2=(const PixelChannels *) y;
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
        This returns a 'floored modulus' of the addition which is a positive
        result.  It differs from % or fmod() that returns a 'truncated modulus'
        result, where floor() is replaced by trunc() and could return a
        negative result (which is clipped).
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
      result=(MagickRealType) EvaluateMax((double) pixel,value);
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
    case SumEvaluateOperator:
    {
      result=(MagickRealType) (pixel+value);
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

  PixelChannels
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
  if (evaluate_pixels == (PixelChannels **) NULL)
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
    {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(progress,status)
#endif
      for (y=0; y < (ssize_t) evaluate_image->rows; y++)
      {
        CacheView
          *image_view;

        const Image
          *next;

        const int
          id = GetOpenMPThreadId();

        register PixelChannels
          *evaluate_pixel;

        register Quantum
          *restrict q;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        q=QueueCacheViewAuthenticPixels(evaluate_view,0,y,
          evaluate_image->columns,1,exception);
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

              channel=GetPixelChannelMapChannel(evaluate_image,i);
              evaluate_traits=GetPixelChannelMapTraits(evaluate_image,channel);
              traits=GetPixelChannelMapTraits(next,channel);
              if ((traits == UndefinedPixelTrait) ||
                  (evaluate_traits == UndefinedPixelTrait))
                continue;
              if ((evaluate_traits & UpdatePixelTrait) == 0)
                continue;
              evaluate_pixel[j].channel[i]=ApplyEvaluateOperator(
                random_info[id],GetPixelChannel(evaluate_image,channel,p),op,
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

#if   defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp critical (MagickCore_EvaluateImages)
#endif
            proceed=SetImageProgress(images,EvaluateImageTag,progress++,
              evaluate_image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
    }
  else
    {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(progress,status)
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

        register PixelChannels
          *evaluate_pixel;

        register Quantum
          *restrict q;

        ssize_t
          j;

        if (status == MagickFalse)
          continue;
        q=QueueCacheViewAuthenticPixels(evaluate_view,0,y,
          evaluate_image->columns,1,exception);
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

            if (GetPixelMask(next,p) != 0)
              {
                p+=GetPixelChannels(next);
                continue;
              }
            for (i=0; i < (ssize_t) GetPixelChannels(next); i++)
            {
              PixelChannel
                channel;

              PixelTrait
                evaluate_traits,
                traits;

              channel=GetPixelChannelMapChannel(evaluate_image,i);
              traits=GetPixelChannelMapTraits(next,channel);
              evaluate_traits=GetPixelChannelMapTraits(evaluate_image,channel);
              if ((traits == UndefinedPixelTrait) ||
                  (evaluate_traits == UndefinedPixelTrait))
                continue;
              if ((traits & UpdatePixelTrait) == 0)
                continue;
              evaluate_pixel[x].channel[i]=ApplyEvaluateOperator(
                random_info[id],GetPixelChannel(evaluate_image,channel,p),j ==
                0 ? AddEvaluateOperator : op,evaluate_pixel[x].channel[i]);
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

          if (GetPixelMask(evaluate_image,q) != 0)
            {
              q+=GetPixelChannels(evaluate_image);
              continue;
            }
          for (i=0; i < (ssize_t) GetPixelChannels(evaluate_image); i++)
          {
            PixelChannel
              channel;

            PixelTrait
              traits;

            channel=GetPixelChannelMapChannel(evaluate_image,i);
            traits=GetPixelChannelMapTraits(evaluate_image,channel);
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

#if   defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp critical (MagickCore_EvaluateImages)
#endif
            proceed=SetImageProgress(images,EvaluateImageTag,progress++,
              evaluate_image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
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
  #pragma omp parallel for schedule(static,4) shared(progress,status)
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
        PixelChannel
          channel;

        PixelTrait
          traits;

        channel=GetPixelChannelMapChannel(image,i);
        traits=GetPixelChannelMapTraits(image,channel);
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
        Polynomial: polynomial constants, highest to lowest order (e.g. c0*x^3+
        c1*x^2+c2*x+c3).
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
        Arcsin (peged at range limits for invalid results): width, center,
        range, and bias.
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
  #pragma omp parallel for schedule(static,4) shared(progress,status)
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
  channel_statistics[CompositePixelChannel].mean=0.0;
  channel_statistics[CompositePixelChannel].standard_deviation=0.0;
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
    if ((traits & UpdatePixelTrait) == 0)
      continue;
    channel_statistics[CompositePixelChannel].mean+=channel_statistics[i].mean;
    channel_statistics[CompositePixelChannel].standard_deviation+=
      channel_statistics[i].variance-channel_statistics[i].mean*
      channel_statistics[i].mean;
    area++;
  }
  channel_statistics[CompositePixelChannel].mean/=area;
  channel_statistics[CompositePixelChannel].standard_deviation=
    sqrt(channel_statistics[CompositePixelChannel].standard_deviation/area);
  *mean=channel_statistics[CompositePixelChannel].mean;
  *standard_deviation=
    channel_statistics[CompositePixelChannel].standard_deviation;
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
  #pragma omp parallel for schedule(static) shared(status)
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

      if (GetPixelMask(image,p) != 0)
        {
          p+=GetPixelChannels(image);
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
  #pragma omp parallel for schedule(static) shared(status)
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

      if (GetPixelMask(image,p) != 0)
        {
          p+=GetPixelChannels(image);
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
    PixelChannel
      channel;

    PixelTrait
      traits;

    channel=GetPixelChannelMapChannel(image,i);
    traits=GetPixelChannelMapTraits(image,channel);
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

      if (GetPixelMask(image,p) != 0)
        {
          p+=GetPixelChannels(image);
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
        if (channel_statistics[channel].depth != MAGICKCORE_QUANTUM_DEPTH)
          {
            depth=channel_statistics[channel].depth;
            range=GetQuantumRange(depth);
            status=p[i] != ScaleAnyToQuantum(ScaleQuantumToAny(p[i],range),
              range) ? MagickTrue : MagickFalse;
            if (status != MagickFalse)
              {
                channel_statistics[channel].depth++;
                continue;
              }
          }
        if ((double) p[i] < channel_statistics[channel].minima)
          channel_statistics[channel].minima=(double) p[i];
        if ((double) p[i] > channel_statistics[channel].maxima)
          channel_statistics[channel].maxima=(double) p[i];
        channel_statistics[channel].sum+=p[i];
        channel_statistics[channel].sum_squared+=(double) p[i]*p[i];
        channel_statistics[channel].sum_cubed+=(double) p[i]*p[i]*p[i];
        channel_statistics[channel].sum_fourth_power+=(double) p[i]*p[i]*p[i]*
          p[i];
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
    channel_statistics[CompositePixelChannel].depth=(size_t) EvaluateMax(
      (double) channel_statistics[CompositePixelChannel].depth,(double)
      channel_statistics[i].depth);
    channel_statistics[CompositePixelChannel].minima=MagickMin(
      channel_statistics[CompositePixelChannel].minima,
      channel_statistics[i].minima);
    channel_statistics[CompositePixelChannel].maxima=EvaluateMax(
      channel_statistics[CompositePixelChannel].maxima,
      channel_statistics[i].maxima);
    channel_statistics[CompositePixelChannel].sum+=channel_statistics[i].sum;
    channel_statistics[CompositePixelChannel].sum_squared+=
      channel_statistics[i].sum_squared;
    channel_statistics[CompositePixelChannel].sum_cubed+=
      channel_statistics[i].sum_cubed;
    channel_statistics[CompositePixelChannel].sum_fourth_power+=
      channel_statistics[i].sum_fourth_power;
    channel_statistics[CompositePixelChannel].mean+=channel_statistics[i].mean;
    channel_statistics[CompositePixelChannel].variance+=
      channel_statistics[i].variance-channel_statistics[i].mean*
      channel_statistics[i].mean;
    channel_statistics[CompositePixelChannel].standard_deviation+=
      channel_statistics[i].variance-channel_statistics[i].mean*
      channel_statistics[i].mean;
  }
  channels=GetImageChannels(image);
  channel_statistics[CompositePixelChannel].sum/=channels;
  channel_statistics[CompositePixelChannel].sum_squared/=channels;
  channel_statistics[CompositePixelChannel].sum_cubed/=channels;
  channel_statistics[CompositePixelChannel].sum_fourth_power/=channels;
  channel_statistics[CompositePixelChannel].mean/=channels;
  channel_statistics[CompositePixelChannel].variance/=channels;
  channel_statistics[CompositePixelChannel].standard_deviation=
    sqrt(channel_statistics[CompositePixelChannel].standard_deviation/channels);
  channel_statistics[CompositePixelChannel].kurtosis/=channels;
  channel_statistics[CompositePixelChannel].skewness/=channels;
  for (i=0; i <= (ssize_t) MaxPixelChannels; i++)
  {
    if (channel_statistics[i].standard_deviation == 0.0)
      continue;
    channel_statistics[i].skewness=(channel_statistics[i].sum_cubed-3.0*
      channel_statistics[i].mean*channel_statistics[i].sum_squared+2.0*
      channel_statistics[i].mean*channel_statistics[i].mean*
      channel_statistics[i].mean)/(channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation);
    channel_statistics[i].kurtosis=(channel_statistics[i].sum_fourth_power-4.0*
      channel_statistics[i].mean*channel_statistics[i].sum_cubed+6.0*
      channel_statistics[i].mean*channel_statistics[i].mean*
      channel_statistics[i].sum_squared-3.0*channel_statistics[i].mean*
      channel_statistics[i].mean*1.0*channel_statistics[i].mean*
      channel_statistics[i].mean)/(channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation)-3.0;
  }
  return(channel_statistics);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S t a t i s t i c I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StatisticImage() makes each pixel the min / max / median / mode / etc. of
%  the neighborhood of the specified width and height.
%
%  The format of the StatisticImage method is:
%
%      Image *StatisticImage(const Image *image,const StatisticType type,
%        const size_t width,const size_t height,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o type: the statistic type (median, mode, etc.).
%
%    o width: the width of the pixel neighborhood.
%
%    o height: the height of the pixel neighborhood.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct _SkipNode
{
  size_t
    next[9],
    count,
    signature;
} SkipNode;

typedef struct _SkipList
{
  ssize_t
    level;

  SkipNode
    *nodes;
} SkipList;

typedef struct _PixelList
{
  size_t
    length,
    seed;

  SkipList
    skip_list;

  size_t
    signature;
} PixelList;

static PixelList *DestroyPixelList(PixelList *pixel_list)
{
  if (pixel_list == (PixelList *) NULL)
    return((PixelList *) NULL);
  if (pixel_list->skip_list.nodes != (SkipNode *) NULL)
    pixel_list->skip_list.nodes=(SkipNode *) RelinquishMagickMemory(
      pixel_list->skip_list.nodes);
  pixel_list=(PixelList *) RelinquishMagickMemory(pixel_list);
  return(pixel_list);
}

static PixelList **DestroyPixelListThreadSet(PixelList **pixel_list)
{
  register ssize_t
    i;

  assert(pixel_list != (PixelList **) NULL);
  for (i=0; i < (ssize_t) GetOpenMPMaximumThreads(); i++)
    if (pixel_list[i] != (PixelList *) NULL)
      pixel_list[i]=DestroyPixelList(pixel_list[i]);
  pixel_list=(PixelList **) RelinquishMagickMemory(pixel_list);
  return(pixel_list);
}

static PixelList *AcquirePixelList(const size_t width,const size_t height)
{
  PixelList
    *pixel_list;

  pixel_list=(PixelList *) AcquireMagickMemory(sizeof(*pixel_list));
  if (pixel_list == (PixelList *) NULL)
    return(pixel_list);
  (void) ResetMagickMemory((void *) pixel_list,0,sizeof(*pixel_list));
  pixel_list->length=width*height;
  pixel_list->skip_list.nodes=(SkipNode *) AcquireQuantumMemory(65537UL,
    sizeof(*pixel_list->skip_list.nodes));
  if (pixel_list->skip_list.nodes == (SkipNode *) NULL)
    return(DestroyPixelList(pixel_list));
  (void) ResetMagickMemory(pixel_list->skip_list.nodes,0,65537UL*
    sizeof(*pixel_list->skip_list.nodes));
  pixel_list->signature=MagickSignature;
  return(pixel_list);
}

static PixelList **AcquirePixelListThreadSet(const size_t width,
  const size_t height)
{
  PixelList
    **pixel_list;

  register ssize_t
    i;

  size_t
    number_threads;

  number_threads=GetOpenMPMaximumThreads();
  pixel_list=(PixelList **) AcquireQuantumMemory(number_threads,
    sizeof(*pixel_list));
  if (pixel_list == (PixelList **) NULL)
    return((PixelList **) NULL);
  (void) ResetMagickMemory(pixel_list,0,number_threads*sizeof(*pixel_list));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    pixel_list[i]=AcquirePixelList(width,height);
    if (pixel_list[i] == (PixelList *) NULL)
      return(DestroyPixelListThreadSet(pixel_list));
  }
  return(pixel_list);
}

static void AddNodePixelList(PixelList *pixel_list,const size_t color)
{
  register SkipList
    *p;

  register ssize_t
    level;

  size_t
    search,
    update[9];

  /*
    Initialize the node.
  */
  p=(&pixel_list->skip_list);
  p->nodes[color].signature=pixel_list->signature;
  p->nodes[color].count=1;
  /*
    Determine where it belongs in the list.
  */
  search=65536UL;
  for (level=p->level; level >= 0; level--)
  {
    while (p->nodes[search].next[level] < color)
      search=p->nodes[search].next[level];
    update[level]=search;
  }
  /*
    Generate a pseudo-random level for this node.
  */
  for (level=0; ; level++)
  {
    pixel_list->seed=(pixel_list->seed*42893621L)+1L;
    if ((pixel_list->seed & 0x300) != 0x300)
      break;
  }
  if (level > 8)
    level=8;
  if (level > (p->level+2))
    level=p->level+2;
  /*
    If we're raising the list's level, link back to the root node.
  */
  while (level > p->level)
  {
    p->level++;
    update[p->level]=65536UL;
  }
  /*
    Link the node into the skip-list.
  */
  do
  {
    p->nodes[color].next[level]=p->nodes[update[level]].next[level];
    p->nodes[update[level]].next[level]=color;
  } while (level-- > 0);
}

static inline void GetMaximumPixelList(PixelList *pixel_list,Quantum *pixel)
{
  register SkipList
    *p;

  size_t
    color,
    maximum;

  ssize_t
    count;

  /*
    Find the maximum value for each of the color.
  */
  p=(&pixel_list->skip_list);
  color=65536L;
  count=0;
  maximum=p->nodes[color].next[0];
  do
  {
    color=p->nodes[color].next[0];
    if (color > maximum)
      maximum=color;
    count+=p->nodes[color].count;
  } while (count < (ssize_t) pixel_list->length);
  *pixel=ScaleShortToQuantum((unsigned short) maximum);
}

static inline void GetMeanPixelList(PixelList *pixel_list,Quantum *pixel)
{
  MagickRealType
    sum;

  register SkipList
    *p;

  size_t
    color;

  ssize_t
    count;

  /*
    Find the mean value for each of the color.
  */
  p=(&pixel_list->skip_list);
  color=65536L;
  count=0;
  sum=0.0;
  do
  {
    color=p->nodes[color].next[0];
    sum+=(MagickRealType) p->nodes[color].count*color;
    count+=p->nodes[color].count;
  } while (count < (ssize_t) pixel_list->length);
  sum/=pixel_list->length;
  *pixel=ScaleShortToQuantum((unsigned short) sum);
}

static inline void GetMedianPixelList(PixelList *pixel_list,Quantum *pixel)
{
  register SkipList
    *p;

  size_t
    color;

  ssize_t
    count;

  /*
    Find the median value for each of the color.
  */
  p=(&pixel_list->skip_list);
  color=65536L;
  count=0;
  do
  {
    color=p->nodes[color].next[0];
    count+=p->nodes[color].count;
  } while (count <= (ssize_t) (pixel_list->length >> 1));
  *pixel=ScaleShortToQuantum((unsigned short) color);
}

static inline void GetMinimumPixelList(PixelList *pixel_list,Quantum *pixel)
{
  register SkipList
    *p;

  size_t
    color,
    minimum;

  ssize_t
    count;

  /*
    Find the minimum value for each of the color.
  */
  p=(&pixel_list->skip_list);
  count=0;
  color=65536UL;
  minimum=p->nodes[color].next[0];
  do
  {
    color=p->nodes[color].next[0];
    if (color < minimum)
      minimum=color;
    count+=p->nodes[color].count;
  } while (count < (ssize_t) pixel_list->length);
  *pixel=ScaleShortToQuantum((unsigned short) minimum);
}

static inline void GetModePixelList(PixelList *pixel_list,Quantum *pixel)
{
  register SkipList
    *p;

  size_t
    color,
    max_count,
    mode;

  ssize_t
    count;

  /*
    Make each pixel the 'predominant color' of the specified neighborhood.
  */
  p=(&pixel_list->skip_list);
  color=65536L;
  mode=color;
  max_count=p->nodes[mode].count;
  count=0;
  do
  {
    color=p->nodes[color].next[0];
    if (p->nodes[color].count > max_count)
      {
        mode=color;
        max_count=p->nodes[mode].count;
      }
    count+=p->nodes[color].count;
  } while (count < (ssize_t) pixel_list->length);
  *pixel=ScaleShortToQuantum((unsigned short) mode);
}

static inline void GetNonpeakPixelList(PixelList *pixel_list,Quantum *pixel)
{
  register SkipList
    *p;

  size_t
    color,
    next,
    previous;

  ssize_t
    count;

  /*
    Finds the non peak value for each of the colors.
  */
  p=(&pixel_list->skip_list);
  color=65536L;
  next=p->nodes[color].next[0];
  count=0;
  do
  {
    previous=color;
    color=next;
    next=p->nodes[color].next[0];
    count+=p->nodes[color].count;
  } while (count <= (ssize_t) (pixel_list->length >> 1));
  if ((previous == 65536UL) && (next != 65536UL))
    color=next;
  else
    if ((previous != 65536UL) && (next == 65536UL))
      color=previous;
  *pixel=ScaleShortToQuantum((unsigned short) color);
}

static inline void GetStandardDeviationPixelList(PixelList *pixel_list,
  Quantum *pixel)
{
  MagickRealType
    sum,
    sum_squared;

  register SkipList
    *p;

  size_t
    color;

  ssize_t
    count;

  /*
    Find the standard-deviation value for each of the color.
  */
  p=(&pixel_list->skip_list);
  color=65536L;
  count=0;
  sum=0.0;
  sum_squared=0.0;
  do
  {
    register ssize_t
      i;

    color=p->nodes[color].next[0];
    sum+=(MagickRealType) p->nodes[color].count*color;
    for (i=0; i < (ssize_t) p->nodes[color].count; i++)
      sum_squared+=((MagickRealType) color)*((MagickRealType) color);
    count+=p->nodes[color].count;
  } while (count < (ssize_t) pixel_list->length);
  sum/=pixel_list->length;
  sum_squared/=pixel_list->length;
  *pixel=ScaleShortToQuantum((unsigned short) sqrt(sum_squared-(sum*sum)));
}

static inline void InsertPixelList(const Image *image,const Quantum pixel,
  PixelList *pixel_list)
{
  size_t
    signature;

  unsigned short
    index;

  index=ScaleQuantumToShort(pixel);
  signature=pixel_list->skip_list.nodes[index].signature;
  if (signature == pixel_list->signature)
    {
      pixel_list->skip_list.nodes[index].count++;
      return;
    }
  AddNodePixelList(pixel_list,index);
}

static inline MagickRealType MagickAbsoluteValue(const MagickRealType x)
{
  if (x < 0)
    return(-x);
  return(x);
}

static inline size_t MagickMax(const size_t x,const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static void ResetPixelList(PixelList *pixel_list)
{
  int
    level;

  register SkipNode
    *root;

  register SkipList
    *p;

  /*
    Reset the skip-list.
  */
  p=(&pixel_list->skip_list);
  root=p->nodes+65536UL;
  p->level=0;
  for (level=0; level < 9; level++)
    root->next[level]=65536UL;
  pixel_list->seed=pixel_list->signature++;
}

MagickExport Image *StatisticImage(const Image *image,const StatisticType type,
  const size_t width,const size_t height,ExceptionInfo *exception)
{
#define StatisticImageTag  "Statistic/Image"

  CacheView
    *image_view,
    *statistic_view;

  Image
    *statistic_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelList
    **restrict pixel_list;

  ssize_t
    center,
    y;

  /*
    Initialize statistics image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  statistic_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (statistic_image == (Image *) NULL)
    return((Image *) NULL);
  status=SetImageStorageClass(statistic_image,DirectClass,exception);
  if (status == MagickFalse)
    {
      statistic_image=DestroyImage(statistic_image);
      return((Image *) NULL);
    }
  pixel_list=AcquirePixelListThreadSet(MagickMax(width,1),MagickMax(height,1));
  if (pixel_list == (PixelList **) NULL)
    {
      statistic_image=DestroyImage(statistic_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Make each pixel the min / max / median / mode / etc. of the neighborhood.
  */
  center=(ssize_t) GetPixelChannels(image)*(image->columns+MagickMax(width,1))*
    (MagickMax(height,1)/2L)+GetPixelChannels(image)*(MagickMax(width,1)/2L);
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  statistic_view=AcquireCacheView(statistic_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) statistic_image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) MagickMax(width,1)/2L),y-
      (ssize_t) (MagickMax(height,1)/2L),image->columns+MagickMax(width,1),
      MagickMax(height,1),exception);
    q=QueueCacheViewAuthenticPixels(statistic_view,0,y,statistic_image->columns,      1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) statistic_image->columns; x++)
    {
      register ssize_t
        i;

      if (GetPixelMask(image,p) != 0)
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(statistic_image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel
          channel;

        PixelTrait
          statistic_traits,
          traits;

        Quantum
          pixel;

        register const Quantum
          *restrict pixels;

        register ssize_t
          u;

        ssize_t
          v;

        channel=GetPixelChannelMapChannel(image,i);
        traits=GetPixelChannelMapTraits(image,channel);
        statistic_traits=GetPixelChannelMapTraits(statistic_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (statistic_traits == UndefinedPixelTrait))
          continue;
        if ((statistic_traits & CopyPixelTrait) != 0)
          {
            SetPixelChannel(statistic_image,channel,p[center+i],q);
            continue;
          }
        pixels=p;
        ResetPixelList(pixel_list[id]);
        for (v=0; v < (ssize_t) MagickMax(height,1); v++)
        {
          for (u=0; u < (ssize_t) MagickMax(width,1); u++)
          {
            InsertPixelList(image,pixels[i],pixel_list[id]);
            pixels+=GetPixelChannels(image);
          }
          pixels+=image->columns*GetPixelChannels(image);
        }
        switch (type)
        {
          case GradientStatistic:
          {
            MagickRealType
              maximum,
              minimum;

            GetMinimumPixelList(pixel_list[id],&pixel);
            minimum=(MagickRealType) pixel;
            GetMaximumPixelList(pixel_list[id],&pixel);
            maximum=(MagickRealType) pixel;
            pixel=ClampToQuantum(MagickAbsoluteValue(maximum-minimum));
            break;
          }
          case MaximumStatistic:
          {
            GetMaximumPixelList(pixel_list[id],&pixel);
            break;
          }
          case MeanStatistic:
          {
            GetMeanPixelList(pixel_list[id],&pixel);
            break;
          }
          case MedianStatistic:
          default:
          {
            GetMedianPixelList(pixel_list[id],&pixel);
            break;
          }
          case MinimumStatistic:
          {
            GetMinimumPixelList(pixel_list[id],&pixel);
            break;
          }
          case ModeStatistic:
          {
            GetModePixelList(pixel_list[id],&pixel);
            break;
          }
          case NonpeakStatistic:
          {
            GetNonpeakPixelList(pixel_list[id],&pixel);
            break;
          }
          case StandardDeviationStatistic:
          {
            GetStandardDeviationPixelList(pixel_list[id],&pixel);
            break;
          }
        }
        SetPixelChannel(statistic_image,channel,pixel,q);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(statistic_image);
    }
    if (SyncCacheViewAuthenticPixels(statistic_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_StatisticImage)
#endif
        proceed=SetImageProgress(image,StatisticImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  statistic_view=DestroyCacheView(statistic_view);
  image_view=DestroyCacheView(image_view);
  pixel_list=DestroyPixelListThreadSet(pixel_list);
  return(statistic_image);
}

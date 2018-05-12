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
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
#include "MagickCore/accelerate-private.h"
#include "MagickCore/animate.h"
#include "MagickCore/artifact.h"
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
#include "MagickCore/property.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/random_.h"
#include "MagickCore/random-private.h"
#include "MagickCore/resource_.h"
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
  double
    channel[CompositePixelChannel];
} PixelChannels;

static PixelChannels **DestroyPixelThreadSet(PixelChannels **pixels)
{
  register ssize_t
    i;

  assert(pixels != (PixelChannels **) NULL);
  for (i=0; i < (ssize_t) GetMagickResourceLimit(ThreadResource); i++)
    if (pixels[i] != (PixelChannels *) NULL)
      pixels[i]=(PixelChannels *) RelinquishMagickMemory(pixels[i]);
  pixels=(PixelChannels **) RelinquishMagickMemory(pixels);
  return(pixels);
}

static PixelChannels **AcquirePixelThreadSet(const Image *image)
{
  PixelChannels
    **pixels;

  register ssize_t
    i;

  size_t
    number_threads;

  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  pixels=(PixelChannels **) AcquireQuantumMemory(number_threads,
    sizeof(*pixels));
  if (pixels == (PixelChannels **) NULL)
    return((PixelChannels **) NULL);
  (void) memset(pixels,0,number_threads*sizeof(*pixels));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    register ssize_t
      j;

    pixels[i]=(PixelChannels *) AcquireQuantumMemory(image->columns,
      sizeof(**pixels));
    if (pixels[i] == (PixelChannels *) NULL)
      return(DestroyPixelThreadSet(pixels));
    for (j=0; j < (ssize_t) image->columns; j++)
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

  double
    distance;

  register ssize_t
    i;

  color_1=(const PixelChannels *) x;
  color_2=(const PixelChannels *) y;
  distance=0.0;
  for (i=0; i < MaxPixelChannels; i++)
    distance+=color_1->channel[i]-(double) color_2->channel[i];
  return(distance < 0 ? -1 : distance > 0 ? 1 : 0);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static double ApplyEvaluateOperator(RandomInfo *random_info,const Quantum pixel,
  const MagickEvaluateOperator op,const double value)
{
  double
    result;

  result=0.0;
  switch (op)
  {
    case UndefinedEvaluateOperator:
      break;
    case AbsEvaluateOperator:
    {
      result=(double) fabs((double) (pixel+value));
      break;
    }
    case AddEvaluateOperator:
    {
      result=(double) (pixel+value);
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
      result=(double) ((size_t) pixel & (size_t) (value+0.5));
      break;
    }
    case CosineEvaluateOperator:
    {
      result=(double) (QuantumRange*(0.5*cos((double) (2.0*MagickPI*
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
      result=(double) (QuantumRange*exp((double) (value*QuantumScale*pixel)));
      break;
    }
    case GaussianNoiseEvaluateOperator:
    {
      result=(double) GenerateDifferentialNoise(random_info,pixel,
        GaussianNoise,value);
      break;
    }
    case ImpulseNoiseEvaluateOperator:
    {
      result=(double) GenerateDifferentialNoise(random_info,pixel,ImpulseNoise,
        value);
      break;
    }
    case LaplacianNoiseEvaluateOperator:
    {
      result=(double) GenerateDifferentialNoise(random_info,pixel,
        LaplacianNoise,value);
      break;
    }
    case LeftShiftEvaluateOperator:
    {
      result=(double) ((size_t) pixel << (size_t) (value+0.5));
      break;
    }
    case LogEvaluateOperator:
    {
      if ((QuantumScale*pixel) >= MagickEpsilon)
        result=(double) (QuantumRange*log((double) (QuantumScale*value*pixel+
          1.0))/log((double) (value+1.0)));
      break;
    }
    case MaxEvaluateOperator:
    {
      result=(double) EvaluateMax((double) pixel,value);
      break;
    }
    case MeanEvaluateOperator:
    {
      result=(double) (pixel+value);
      break;
    }
    case MedianEvaluateOperator:
    {
      result=(double) (pixel+value);
      break;
    }
    case MinEvaluateOperator:
    {
      result=(double) MagickMin((double) pixel,value);
      break;
    }
    case MultiplicativeNoiseEvaluateOperator:
    {
      result=(double) GenerateDifferentialNoise(random_info,pixel,
        MultiplicativeGaussianNoise,value);
      break;
    }
    case MultiplyEvaluateOperator:
    {
      result=(double) (value*pixel);
      break;
    }
    case OrEvaluateOperator:
    {
      result=(double) ((size_t) pixel | (size_t) (value+0.5));
      break;
    }
    case PoissonNoiseEvaluateOperator:
    {
      result=(double) GenerateDifferentialNoise(random_info,pixel,PoissonNoise,
        value);
      break;
    }
    case PowEvaluateOperator:
    {
      result=(double) (QuantumRange*pow((double) (QuantumScale*pixel),(double)
        value));
      break;
    }
    case RightShiftEvaluateOperator:
    {
      result=(double) ((size_t) pixel >> (size_t) (value+0.5));
      break;
    }
    case RootMeanSquareEvaluateOperator:
    {
      result=(double) (pixel*pixel+value);
      break;
    }
    case SetEvaluateOperator:
    {
      result=value;
      break;
    }
    case SineEvaluateOperator:
    {
      result=(double) (QuantumRange*(0.5*sin((double) (2.0*MagickPI*
        QuantumScale*pixel*value))+0.5));
      break;
    }
    case SubtractEvaluateOperator:
    {
      result=(double) (pixel-value);
      break;
    }
    case SumEvaluateOperator:
    {
      result=(double) (pixel+value);
      break;
    }
    case ThresholdEvaluateOperator:
    {
      result=(double) (((double) pixel <= value) ? 0 : QuantumRange);
      break;
    }
    case ThresholdBlackEvaluateOperator:
    {
      result=(double) (((double) pixel <= value) ? 0 : pixel);
      break;
    }
    case ThresholdWhiteEvaluateOperator:
    {
      result=(double) (((double) pixel > value) ? QuantumRange : pixel);
      break;
    }
    case UniformNoiseEvaluateOperator:
    {
      result=(double) GenerateDifferentialNoise(random_info,pixel,UniformNoise,
        value);
      break;
    }
    case XorEvaluateOperator:
    {
      result=(double) ((size_t) pixel ^ (size_t) (value+0.5));
      break;
    }
  }
  return(result);
}

static Image *AcquireImageCanvas(const Image *images,ExceptionInfo *exception)
{
  const Image
    *p,
    *q;

  size_t
    columns,
    rows;

  q=images;
  columns=images->columns;
  rows=images->rows;
  for (p=images; p != (Image *) NULL; p=p->next)
  {
    if (p->number_channels > q->number_channels)
      q=p;
    if (p->columns > columns)
      columns=p->columns;
    if (p->rows > rows)
      rows=p->rows;
  }
  return(CloneImage(q,columns,rows,MagickTrue,exception));
}

MagickExport Image *EvaluateImages(const Image *images,
  const MagickEvaluateOperator op,ExceptionInfo *exception)
{
#define EvaluateImageTag  "Evaluate/Image"

  CacheView
    *evaluate_view;

  Image
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelChannels
    **magick_restrict evaluate_pixels;

  RandomInfo
    **magick_restrict random_info;

  size_t
    number_images;

  ssize_t
    y;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  unsigned long
    key;
#endif

  assert(images != (Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImageCanvas(images,exception);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  number_images=GetImageListLength(images);
  evaluate_pixels=AcquirePixelThreadSet(images);
  if (evaluate_pixels == (PixelChannels **) NULL)
    {
      image=DestroyImage(image);
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
  evaluate_view=AcquireAuthenticCacheView(image,exception);
  if (op == MedianEvaluateOperator)
    {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      key=GetRandomSecretKey(random_info[0]);
      #pragma omp parallel for schedule(static) shared(progress,status) \
        magick_number_threads(image,images,image->rows,key == ~0UL)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
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
          *magick_restrict q;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        q=QueueCacheViewAuthenticPixels(evaluate_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        evaluate_pixel=evaluate_pixels[id];
        for (x=0; x < (ssize_t) image->columns; x++)
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

            image_view=AcquireVirtualCacheView(next,exception);
            p=GetCacheViewVirtualPixels(image_view,x,y,1,1,exception);
            if (p == (const Quantum *) NULL)
              {
                image_view=DestroyCacheView(image_view);
                break;
              }
            for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
            {
              PixelChannel channel = GetPixelChannelChannel(image,i);
              PixelTrait evaluate_traits=GetPixelChannelTraits(image,channel);
              PixelTrait traits = GetPixelChannelTraits(next,channel);
              if ((traits == UndefinedPixelTrait) ||
                  (evaluate_traits == UndefinedPixelTrait))
                continue;
              if ((traits & UpdatePixelTrait) == 0)
                continue;
              evaluate_pixel[j].channel[i]=ApplyEvaluateOperator(
                random_info[id],GetPixelChannel(image,channel,p),op,
                evaluate_pixel[j].channel[i]);
            }
            image_view=DestroyCacheView(image_view);
            next=GetNextImageInList(next);
          }
          qsort((void *) evaluate_pixel,number_images,sizeof(*evaluate_pixel),
            IntensityCompare);
          for (k=0; k < (ssize_t) GetPixelChannels(image); k++)
            q[k]=ClampToQuantum(evaluate_pixel[j/2].channel[k]);
          q+=GetPixelChannels(image);
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
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
    }
  else
    {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      key=GetRandomSecretKey(random_info[0]);
      #pragma omp parallel for schedule(static) shared(progress,status) \
        magick_number_threads(image,images,image->rows,key == ~0UL)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
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
          *magick_restrict q;

        ssize_t
          j;

        if (status == MagickFalse)
          continue;
        q=QueueCacheViewAuthenticPixels(evaluate_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        evaluate_pixel=evaluate_pixels[id];
        for (j=0; j < (ssize_t) image->columns; j++)
          for (i=0; i < MaxPixelChannels; i++)
            evaluate_pixel[j].channel[i]=0.0;
        next=images;
        for (j=0; j < (ssize_t) number_images; j++)
        {
          register const Quantum
            *p;

          image_view=AcquireVirtualCacheView(next,exception);
          p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,
            exception);
          if (p == (const Quantum *) NULL)
            {
              image_view=DestroyCacheView(image_view);
              break;
            }
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            register ssize_t
              i;

            for (i=0; i < (ssize_t) GetPixelChannels(next); i++)
            {
              PixelChannel channel = GetPixelChannelChannel(image,i);
              PixelTrait traits = GetPixelChannelTraits(next,channel);
              PixelTrait evaluate_traits=GetPixelChannelTraits(image,channel);
              if ((traits == UndefinedPixelTrait) ||
                  (evaluate_traits == UndefinedPixelTrait))
                continue;
              if ((traits & UpdatePixelTrait) == 0)
                continue;
              evaluate_pixel[x].channel[i]=ApplyEvaluateOperator(
                random_info[id],GetPixelChannel(image,channel,p),j == 0 ?
                AddEvaluateOperator : op,evaluate_pixel[x].channel[i]);
            }
            p+=GetPixelChannels(next);
          }
          image_view=DestroyCacheView(image_view);
          next=GetNextImageInList(next);
        }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          register ssize_t
             i;

          switch (op)
          {
            case MeanEvaluateOperator:
            {
              for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
                evaluate_pixel[x].channel[i]/=(double) number_images;
              break;
            }
            case MultiplyEvaluateOperator:
            {
              for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
              {
                register ssize_t
                  j;

                for (j=0; j < (ssize_t) (number_images-1); j++)
                  evaluate_pixel[x].channel[i]*=QuantumScale;
              }
              break;
            }
            case RootMeanSquareEvaluateOperator:
            {
              for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
                evaluate_pixel[x].channel[i]=sqrt(evaluate_pixel[x].channel[i]/
                  number_images);
              break;
            }
            default:
              break;
          }
        }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          register ssize_t
            i;

          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            if (traits == UndefinedPixelTrait)
              continue;
            if ((traits & UpdatePixelTrait) == 0)
              continue;
            q[i]=ClampToQuantum(evaluate_pixel[x].channel[i]);
          }
          q+=GetPixelChannels(image);
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
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
    }
  evaluate_view=DestroyCacheView(evaluate_view);
  evaluate_pixels=DestroyPixelThreadSet(evaluate_pixels);
  random_info=DestroyRandomInfoThreadSet(random_info);
  if (status == MagickFalse)
    image=DestroyImage(image);
  return(image);
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
    **magick_restrict random_info;

  ssize_t
    y;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  unsigned long
    key;
#endif

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  random_info=AcquireRandomInfoThreadSet();
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  key=GetRandomSecretKey(random_info[0]);
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,key == ~0UL)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register Quantum
      *magick_restrict q;

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
      double
        result;

      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & CopyPixelTrait) != 0)
          continue;
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        result=ApplyEvaluateOperator(random_info[id],q[i],op,value);
        if (op == MeanEvaluateOperator)
          result/=2.0;
        q[i]=ClampToQuantum(result);
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
  double
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
      double
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
      result=(double) (QuantumRange*(amplitude*sin((double) (2.0*
        MagickPI*(frequency*QuantumScale*pixel+phase/360.0)))+bias));
      break;
    }
    case ArcsinFunction:
    {
      double
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
          result=(double) (range/MagickPI*asin((double) result)+bias);
      result*=QuantumRange;
      break;
    }
    case ArctanFunction:
    {
      double
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
      result=(double) (MagickPI*slope*(QuantumScale*pixel-center));
      result=(double) (QuantumRange*(range/MagickPI*atan((double)
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
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  if (AccelerateFunctionImage(image,function,number_parameters,parameters,
        exception) != MagickFalse)
    return(MagickTrue);
#endif
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *magick_restrict q;

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
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
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
%   G e t I m a g e E n t r o p y                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageEntropy() returns the entropy of one or more image channels.
%
%  The format of the GetImageEntropy method is:
%
%      MagickBooleanType GetImageEntropy(const Image *image,double *entropy,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o entropy: the average entropy of the selected channels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetImageEntropy(const Image *image,
  double *entropy,ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=GetImageStatistics(image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    return(MagickFalse);
  *entropy=channel_statistics[CompositePixelChannel].entropy;
  channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
    channel_statistics);
  return(MagickTrue);
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
  assert(image->signature == MagickCoreSignature);
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
%   G e t I m a g e K u r t o s i s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageKurtosis() returns the kurtosis and skewness of one or more image
%  channels.
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
  ChannelStatistics
    *channel_statistics;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=GetImageStatistics(image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    return(MagickFalse);
  *kurtosis=channel_statistics[CompositePixelChannel].kurtosis;
  *skewness=channel_statistics[CompositePixelChannel].skewness;
  channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
    channel_statistics);
  return(MagickTrue);
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
%  GetImageMean() returns the mean and standard deviation of one or more image
%  channels.
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

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=GetImageStatistics(image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    return(MagickFalse);
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
%   G e t I m a g e M o m e n t s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageMoments() returns the normalized moments of one or more image
%  channels.
%
%  The format of the GetImageMoments method is:
%
%      ChannelMoments *GetImageMoments(const Image *image,
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
    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    if (traits == UndefinedPixelTrait)
      continue;
    if ((traits & UpdatePixelTrait) == 0)
      continue;
    channels++;
  }
  return((size_t) (channels == 0 ? 1 : channels));
}

MagickExport ChannelMoments *GetImageMoments(const Image *image,
  ExceptionInfo *exception)
{
#define MaxNumberImageMoments  8

  CacheView
    *image_view;

  ChannelMoments
    *channel_moments;

  double
    M00[MaxPixelChannels+1],
    M01[MaxPixelChannels+1],
    M02[MaxPixelChannels+1],
    M03[MaxPixelChannels+1],
    M10[MaxPixelChannels+1],
    M11[MaxPixelChannels+1],
    M12[MaxPixelChannels+1],
    M20[MaxPixelChannels+1],
    M21[MaxPixelChannels+1],
    M22[MaxPixelChannels+1],
    M30[MaxPixelChannels+1];

  PointInfo
    centroid[MaxPixelChannels+1];

  ssize_t
    channel,
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_moments=(ChannelMoments *) AcquireQuantumMemory(MaxPixelChannels+1,
    sizeof(*channel_moments));
  if (channel_moments == (ChannelMoments *) NULL)
    return(channel_moments);
  (void) memset(channel_moments,0,(MaxPixelChannels+1)*
    sizeof(*channel_moments));
  (void) memset(centroid,0,sizeof(centroid));
  (void) memset(M00,0,sizeof(M00));
  (void) memset(M01,0,sizeof(M01));
  (void) memset(M02,0,sizeof(M02));
  (void) memset(M03,0,sizeof(M03));
  (void) memset(M10,0,sizeof(M10));
  (void) memset(M11,0,sizeof(M11));
  (void) memset(M12,0,sizeof(M12));
  (void) memset(M20,0,sizeof(M20));
  (void) memset(M21,0,sizeof(M21));
  (void) memset(M22,0,sizeof(M22));
  (void) memset(M30,0,sizeof(M30));
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    /*
      Compute center of mass (centroid).
    */
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        M00[channel]+=QuantumScale*p[i];
        M00[MaxPixelChannels]+=QuantumScale*p[i];
        M10[channel]+=x*QuantumScale*p[i];
        M10[MaxPixelChannels]+=x*QuantumScale*p[i];
        M01[channel]+=y*QuantumScale*p[i];
        M01[MaxPixelChannels]+=y*QuantumScale*p[i];
      }
      p+=GetPixelChannels(image);
    }
  }
  for (channel=0; channel <= MaxPixelChannels; channel++)
  {
    /*
       Compute center of mass (centroid).
    */
    if (M00[channel] < MagickEpsilon)
      {
        M00[channel]+=MagickEpsilon;
        centroid[channel].x=(double) image->columns/2.0;
        centroid[channel].y=(double) image->rows/2.0;
        continue;
      }
    M00[channel]+=MagickEpsilon;
    centroid[channel].x=M10[channel]/M00[channel];
    centroid[channel].y=M01[channel]/M00[channel];
  }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    /*
      Compute the image moments.
    */
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        M11[channel]+=(x-centroid[channel].x)*(y-centroid[channel].y)*
          QuantumScale*p[i];
        M11[MaxPixelChannels]+=(x-centroid[channel].x)*(y-centroid[channel].y)*
          QuantumScale*p[i];
        M20[channel]+=(x-centroid[channel].x)*(x-centroid[channel].x)*
          QuantumScale*p[i];
        M20[MaxPixelChannels]+=(x-centroid[channel].x)*(x-centroid[channel].x)*
          QuantumScale*p[i];
        M02[channel]+=(y-centroid[channel].y)*(y-centroid[channel].y)*
          QuantumScale*p[i];
        M02[MaxPixelChannels]+=(y-centroid[channel].y)*(y-centroid[channel].y)*
          QuantumScale*p[i];
        M21[channel]+=(x-centroid[channel].x)*(x-centroid[channel].x)*
          (y-centroid[channel].y)*QuantumScale*p[i];
        M21[MaxPixelChannels]+=(x-centroid[channel].x)*(x-centroid[channel].x)*
          (y-centroid[channel].y)*QuantumScale*p[i];
        M12[channel]+=(x-centroid[channel].x)*(y-centroid[channel].y)*
          (y-centroid[channel].y)*QuantumScale*p[i];
        M12[MaxPixelChannels]+=(x-centroid[channel].x)*(y-centroid[channel].y)*
          (y-centroid[channel].y)*QuantumScale*p[i];
        M22[channel]+=(x-centroid[channel].x)*(x-centroid[channel].x)*
          (y-centroid[channel].y)*(y-centroid[channel].y)*QuantumScale*p[i];
        M22[MaxPixelChannels]+=(x-centroid[channel].x)*(x-centroid[channel].x)*
          (y-centroid[channel].y)*(y-centroid[channel].y)*QuantumScale*p[i];
        M30[channel]+=(x-centroid[channel].x)*(x-centroid[channel].x)*
          (x-centroid[channel].x)*QuantumScale*p[i];
        M30[MaxPixelChannels]+=(x-centroid[channel].x)*(x-centroid[channel].x)*
          (x-centroid[channel].x)*QuantumScale*p[i];
        M03[channel]+=(y-centroid[channel].y)*(y-centroid[channel].y)*
          (y-centroid[channel].y)*QuantumScale*p[i];
        M03[MaxPixelChannels]+=(y-centroid[channel].y)*(y-centroid[channel].y)*
          (y-centroid[channel].y)*QuantumScale*p[i];
      }
      p+=GetPixelChannels(image);
    }
  }
  M00[MaxPixelChannels]/=GetImageChannels(image);
  M01[MaxPixelChannels]/=GetImageChannels(image);
  M02[MaxPixelChannels]/=GetImageChannels(image);
  M03[MaxPixelChannels]/=GetImageChannels(image);
  M10[MaxPixelChannels]/=GetImageChannels(image);
  M11[MaxPixelChannels]/=GetImageChannels(image);
  M12[MaxPixelChannels]/=GetImageChannels(image);
  M20[MaxPixelChannels]/=GetImageChannels(image);
  M21[MaxPixelChannels]/=GetImageChannels(image);
  M22[MaxPixelChannels]/=GetImageChannels(image);
  M30[MaxPixelChannels]/=GetImageChannels(image);
  for (channel=0; channel <= MaxPixelChannels; channel++)
  {
    /*
      Compute elliptical angle, major and minor axes, eccentricity, & intensity.
    */
    channel_moments[channel].centroid=centroid[channel];
    channel_moments[channel].ellipse_axis.x=sqrt((2.0/M00[channel])*
      ((M20[channel]+M02[channel])+sqrt(4.0*M11[channel]*M11[channel]+
      (M20[channel]-M02[channel])*(M20[channel]-M02[channel]))));
    channel_moments[channel].ellipse_axis.y=sqrt((2.0/M00[channel])*
      ((M20[channel]+M02[channel])-sqrt(4.0*M11[channel]*M11[channel]+
      (M20[channel]-M02[channel])*(M20[channel]-M02[channel]))));
    channel_moments[channel].ellipse_angle=RadiansToDegrees(0.5*atan(2.0*
      M11[channel]/(M20[channel]-M02[channel]+MagickEpsilon)));
    if (fabs(M11[channel]) < MagickEpsilon)
      {
        if (fabs(M20[channel]-M02[channel]) < MagickEpsilon)
          channel_moments[channel].ellipse_angle+=0.0;
        else
          if ((M20[channel]-M02[channel]) < 0.0)
            channel_moments[channel].ellipse_angle+=90.0;
          else
            channel_moments[channel].ellipse_angle+=0.0;
      }
    else
      if (M11[channel] < 0.0)
        {
          if (fabs(M20[channel]-M02[channel]) < MagickEpsilon)
            channel_moments[channel].ellipse_angle+=0.0;
          else
            if ((M20[channel]-M02[channel]) < 0.0)
              channel_moments[channel].ellipse_angle+=90.0;
            else
              channel_moments[channel].ellipse_angle+=180.0;
        }
      else
        {
          if (fabs(M20[channel]-M02[channel]) < MagickEpsilon)
            channel_moments[channel].ellipse_angle+=0.0;
          else
            if ((M20[channel]-M02[channel]) < 0.0)
              channel_moments[channel].ellipse_angle+=90.0;
            else
              channel_moments[channel].ellipse_angle+=0.0;
       }
    channel_moments[channel].ellipse_eccentricity=sqrt(1.0-(
      channel_moments[channel].ellipse_axis.y/
      (channel_moments[channel].ellipse_axis.x+MagickEpsilon)));
    channel_moments[channel].ellipse_intensity=M00[channel]/
      (MagickPI*channel_moments[channel].ellipse_axis.x*
      channel_moments[channel].ellipse_axis.y+MagickEpsilon);
  }
  for (channel=0; channel <= MaxPixelChannels; channel++)
  {
    /*
      Normalize image moments.
    */
    M10[channel]=0.0;
    M01[channel]=0.0;
    M11[channel]/=pow(M00[channel],1.0+(1.0+1.0)/2.0);
    M20[channel]/=pow(M00[channel],1.0+(2.0+0.0)/2.0);
    M02[channel]/=pow(M00[channel],1.0+(0.0+2.0)/2.0);
    M21[channel]/=pow(M00[channel],1.0+(2.0+1.0)/2.0);
    M12[channel]/=pow(M00[channel],1.0+(1.0+2.0)/2.0);
    M22[channel]/=pow(M00[channel],1.0+(2.0+2.0)/2.0);
    M30[channel]/=pow(M00[channel],1.0+(3.0+0.0)/2.0);
    M03[channel]/=pow(M00[channel],1.0+(0.0+3.0)/2.0);
    M00[channel]=1.0;
  }
  image_view=DestroyCacheView(image_view);
  for (channel=0; channel <= MaxPixelChannels; channel++)
  {
    /*
      Compute Hu invariant moments.
    */
    channel_moments[channel].invariant[0]=M20[channel]+M02[channel];
    channel_moments[channel].invariant[1]=(M20[channel]-M02[channel])*
      (M20[channel]-M02[channel])+4.0*M11[channel]*M11[channel];
    channel_moments[channel].invariant[2]=(M30[channel]-3.0*M12[channel])*
      (M30[channel]-3.0*M12[channel])+(3.0*M21[channel]-M03[channel])*
      (3.0*M21[channel]-M03[channel]);
    channel_moments[channel].invariant[3]=(M30[channel]+M12[channel])*
      (M30[channel]+M12[channel])+(M21[channel]+M03[channel])*
      (M21[channel]+M03[channel]);
    channel_moments[channel].invariant[4]=(M30[channel]-3.0*M12[channel])*
      (M30[channel]+M12[channel])*((M30[channel]+M12[channel])*
      (M30[channel]+M12[channel])-3.0*(M21[channel]+M03[channel])*
      (M21[channel]+M03[channel]))+(3.0*M21[channel]-M03[channel])*
      (M21[channel]+M03[channel])*(3.0*(M30[channel]+M12[channel])*
      (M30[channel]+M12[channel])-(M21[channel]+M03[channel])*
      (M21[channel]+M03[channel]));
    channel_moments[channel].invariant[5]=(M20[channel]-M02[channel])*
      ((M30[channel]+M12[channel])*(M30[channel]+M12[channel])-
      (M21[channel]+M03[channel])*(M21[channel]+M03[channel]))+
      4.0*M11[channel]*(M30[channel]+M12[channel])*(M21[channel]+M03[channel]);
    channel_moments[channel].invariant[6]=(3.0*M21[channel]-M03[channel])*
      (M30[channel]+M12[channel])*((M30[channel]+M12[channel])*
      (M30[channel]+M12[channel])-3.0*(M21[channel]+M03[channel])*
      (M21[channel]+M03[channel]))-(M30[channel]-3*M12[channel])*
      (M21[channel]+M03[channel])*(3.0*(M30[channel]+M12[channel])*
      (M30[channel]+M12[channel])-(M21[channel]+M03[channel])*
      (M21[channel]+M03[channel]));
    channel_moments[channel].invariant[7]=M11[channel]*((M30[channel]+
      M12[channel])*(M30[channel]+M12[channel])-(M03[channel]+M21[channel])*
      (M03[channel]+M21[channel]))-(M20[channel]-M02[channel])*
      (M30[channel]+M12[channel])*(M03[channel]+M21[channel]);
  }
  if (y < (ssize_t) image->rows)
    channel_moments=(ChannelMoments *) RelinquishMagickMemory(channel_moments);
  return(channel_moments);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l P e r c e p t u a l H a s h                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImagePerceptualHash() returns the perceptual hash of one or more
%  image channels.
%
%  The format of the GetImagePerceptualHash method is:
%
%      ChannelPerceptualHash *GetImagePerceptualHash(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline double MagickLog10(const double x)
{
#define Log10Epsilon  (1.0e-11)

  if (fabs(x) < Log10Epsilon)
    return(log10(Log10Epsilon));
  return(log10(fabs(x)));
}

MagickExport ChannelPerceptualHash *GetImagePerceptualHash(const Image *image,
  ExceptionInfo *exception)
{
  ChannelPerceptualHash
    *perceptual_hash;

  char
    *colorspaces,
    *q;

  const char
    *artifact;

  MagickBooleanType
    status;

  register char
    *p;

  register ssize_t
    i;

  perceptual_hash=(ChannelPerceptualHash *) AcquireQuantumMemory(
    MaxPixelChannels+1UL,sizeof(*perceptual_hash));
  if (perceptual_hash == (ChannelPerceptualHash *) NULL)
    return((ChannelPerceptualHash *) NULL);
  artifact=GetImageArtifact(image,"phash:colorspaces");
  if (artifact != NULL)
    colorspaces=AcquireString(artifact);
  else
    colorspaces=AcquireString("sRGB,HCLp");
  perceptual_hash[0].number_colorspaces=0;
  perceptual_hash[0].number_channels=0;
  q=colorspaces;
  for (i=0; (p=StringToken(",",&q)) != (char *) NULL; i++)
  {
    ChannelMoments
      *moments;

    Image
      *hash_image;

    size_t
      j;

    ssize_t
      channel,
      colorspace;

    if (i >= MaximumNumberOfPerceptualColorspaces)
      break;
    colorspace=ParseCommandOption(MagickColorspaceOptions,MagickFalse,p);
    if (colorspace < 0)
      break;
    perceptual_hash[0].colorspace[i]=(ColorspaceType) colorspace;
    hash_image=BlurImage(image,0.0,1.0,exception);
    if (hash_image == (Image *) NULL)
      break;
    hash_image->depth=8;
    status=TransformImageColorspace(hash_image,(ColorspaceType) colorspace,
      exception);
    if (status == MagickFalse)
      break;
    moments=GetImageMoments(hash_image,exception);
    perceptual_hash[0].number_colorspaces++;
    perceptual_hash[0].number_channels+=GetImageChannels(hash_image);
    hash_image=DestroyImage(hash_image);
    if (moments == (ChannelMoments *) NULL)
      break;
    for (channel=0; channel <= MaxPixelChannels; channel++)
      for (j=0; j < MaximumNumberOfImageMoments; j++)
        perceptual_hash[channel].phash[i][j]=
          (-MagickLog10(moments[channel].invariant[j]));
    moments=(ChannelMoments *) RelinquishMagickMemory(moments);
  }
  colorspaces=DestroyString(colorspaces);
  return(perceptual_hash);
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
    initialize,
    status;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=MagickTrue;
  initialize=MagickTrue;
  *maxima=0.0;
  *minima=0.0;
  image_view=AcquireVirtualCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status,initialize) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      row_maxima = 0.0,
      row_minima = 0.0;

    MagickBooleanType
      row_initialize;

    register const Quantum
      *magick_restrict p;

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
    row_initialize=MagickTrue;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & UpdatePixelTrait) == 0)
          continue;
				if (row_initialize != MagickFalse)
          {
            row_minima=(double) p[i];
            row_maxima=(double) p[i];
            row_initialize=MagickFalse;
          }
        else
          {
            if ((double) p[i] < row_minima)
              row_minima=(double) p[i];
            if ((double) p[i] > row_maxima)
              row_maxima=(double) p[i];
         }
      }
      p+=GetPixelChannels(image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
#pragma omp critical (MagickCore_GetImageRange)
#endif
    {
      if (initialize != MagickFalse)
        {
          *minima=row_minima;
          *maxima=row_maxima;
          initialize=MagickFalse;
        }
      else
        {
          if (row_minima < *minima)
            *minima=row_minima;
          if (row_maxima > *maxima)
            *maxima=row_maxima;
        }
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
%  GetImageStatistics() returns statistics for each channel in the image.  The
%  statistics include the channel depth, its minima, maxima, mean, standard
%  deviation, kurtosis and skewness.  You can access the red channel mean, for
%  example, like this:
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
MagickExport ChannelStatistics *GetImageStatistics(const Image *image,
  ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  double
    area,
    *histogram,
    standard_deviation;

  MagickStatusType
    status;

  QuantumAny
    range;

  register ssize_t
    i;

  size_t
    depth;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  histogram=(double *) AcquireQuantumMemory(MaxMap+1UL,GetPixelChannels(image)*
    sizeof(*histogram));
  channel_statistics=(ChannelStatistics *) AcquireQuantumMemory(
    MaxPixelChannels+1,sizeof(*channel_statistics));
  if ((channel_statistics == (ChannelStatistics *) NULL) ||
      (histogram == (double *) NULL))
    {
      if (histogram != (double *) NULL)
        histogram=(double *) RelinquishMagickMemory(histogram);
      if (channel_statistics != (ChannelStatistics *) NULL)
        channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
          channel_statistics);
      return(channel_statistics);
    }
  (void) memset(channel_statistics,0,(MaxPixelChannels+1)*
    sizeof(*channel_statistics));
  for (i=0; i <= (ssize_t) MaxPixelChannels; i++)
  {
    channel_statistics[i].depth=1;
    channel_statistics[i].maxima=(-MagickMaximumValue);
    channel_statistics[i].minima=MagickMaximumValue;
  }
  (void) memset(histogram,0,(MaxMap+1)*GetPixelChannels(image)*
    sizeof(*histogram));
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    /*
      Compute pixel statistics.
    */
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & UpdatePixelTrait) == 0)
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
                i--;
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
        channel_statistics[channel].area++;
        if ((double) p[i] < channel_statistics[CompositePixelChannel].minima)
          channel_statistics[CompositePixelChannel].minima=(double) p[i];
        if ((double) p[i] > channel_statistics[CompositePixelChannel].maxima)
          channel_statistics[CompositePixelChannel].maxima=(double) p[i];
        histogram[GetPixelChannels(image)*ScaleQuantumToMap(
          ClampToQuantum((double) p[i]))+i]++;
        channel_statistics[CompositePixelChannel].sum+=(double) p[i];
        channel_statistics[CompositePixelChannel].sum_squared+=(double)
          p[i]*p[i];
        channel_statistics[CompositePixelChannel].sum_cubed+=(double)
          p[i]*p[i]*p[i];
        channel_statistics[CompositePixelChannel].sum_fourth_power+=(double)
          p[i]*p[i]*p[i]*p[i];
        channel_statistics[CompositePixelChannel].area++;
      }
      p+=GetPixelChannels(image);
    }
  }
  for (i=0; i <= (ssize_t) MaxPixelChannels; i++)
  {
    /*
      Normalize pixel statistics.
    */
    area=PerceptibleReciprocal(channel_statistics[i].area);
    channel_statistics[i].sum*=area;
    channel_statistics[i].sum_squared*=area;
    channel_statistics[i].sum_cubed*=area;
    channel_statistics[i].sum_fourth_power*=area;
    channel_statistics[i].mean=channel_statistics[i].sum;
    channel_statistics[i].variance=channel_statistics[i].sum_squared;
    standard_deviation=sqrt(channel_statistics[i].variance-
      (channel_statistics[i].mean*channel_statistics[i].mean));
    standard_deviation=sqrt(PerceptibleReciprocal(channel_statistics[i].area-
      1.0)*channel_statistics[i].area*standard_deviation*standard_deviation);
    channel_statistics[i].standard_deviation=standard_deviation;
  }
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    double
      number_bins;

    register ssize_t
      j;

    /*
      Compute pixel entropy.
    */
    PixelChannel channel = GetPixelChannelChannel(image,i);
    number_bins=0.0;
    for (j=0; j <= (ssize_t) MaxMap; j++)
      if (histogram[GetPixelChannels(image)*j+i] > 0.0)
        number_bins++;
    area=PerceptibleReciprocal(channel_statistics[channel].area);
    for (j=0; j <= (ssize_t) MaxMap; j++)
    {
      double
        count;

      count=area*histogram[GetPixelChannels(image)*j+i];
      channel_statistics[channel].entropy+=-count*MagickLog10(count)*
        PerceptibleReciprocal(MagickLog10(number_bins));
      channel_statistics[CompositePixelChannel].entropy+=-count*
        MagickLog10(count)*PerceptibleReciprocal(MagickLog10(number_bins))/
        GetPixelChannels(image);
    }
  }
  histogram=(double *) RelinquishMagickMemory(histogram);
  for (i=0; i <= (ssize_t) MaxPixelChannels; i++)
  {
    /*
      Compute kurtosis & skewness statistics.
    */
    standard_deviation=PerceptibleReciprocal(
      channel_statistics[i].standard_deviation);
    channel_statistics[i].skewness=(channel_statistics[i].sum_cubed-3.0*
      channel_statistics[i].mean*channel_statistics[i].sum_squared+2.0*
      channel_statistics[i].mean*channel_statistics[i].mean*
      channel_statistics[i].mean)*(standard_deviation*standard_deviation*
      standard_deviation);
    channel_statistics[i].kurtosis=(channel_statistics[i].sum_fourth_power-4.0*
      channel_statistics[i].mean*channel_statistics[i].sum_cubed+6.0*
      channel_statistics[i].mean*channel_statistics[i].mean*
      channel_statistics[i].sum_squared-3.0*channel_statistics[i].mean*
      channel_statistics[i].mean*1.0*channel_statistics[i].mean*
      channel_statistics[i].mean)*(standard_deviation*standard_deviation*
      standard_deviation*standard_deviation)-3.0;
  }
  channel_statistics[CompositePixelChannel].mean=0.0;
  channel_statistics[CompositePixelChannel].standard_deviation=0.0;
  channel_statistics[CompositePixelChannel].entropy=0.0;
  for (i=0; i < (ssize_t) MaxPixelChannels; i++)
  {
    channel_statistics[CompositePixelChannel].mean+=
      channel_statistics[i].mean;
    channel_statistics[CompositePixelChannel].standard_deviation+=
      channel_statistics[i].standard_deviation;
    channel_statistics[CompositePixelChannel].entropy+=
      channel_statistics[i].entropy;
  }
  channel_statistics[CompositePixelChannel].mean/=(double)
    GetImageChannels(image);
  channel_statistics[CompositePixelChannel].standard_deviation/=(double)
    GetImageChannels(image);
  channel_statistics[CompositePixelChannel].entropy/=(double)
    GetImageChannels(image);
  if (y < (ssize_t) image->rows)
    channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
      channel_statistics);
  return(channel_statistics);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     P o l y n o m i a l I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PolynomialImage() returns a new image where each pixel is the sum of the
%  pixels in the image sequence after applying its corresponding terms
%  (coefficient and degree pairs).
%
%  The format of the PolynomialImage method is:
%
%      Image *PolynomialImage(const Image *images,const size_t number_terms,
%        const double *terms,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image sequence.
%
%    o number_terms: the number of terms in the list.  The actual list length
%      is 2 x number_terms + 1 (the constant).
%
%    o terms: the list of polynomial coefficients and degree pairs and a
%      constant.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image *PolynomialImage(const Image *images,
  const size_t number_terms,const double *terms,ExceptionInfo *exception)
{
#define PolynomialImageTag  "Polynomial/Image"

  CacheView
    *polynomial_view;

  Image
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelChannels
    **magick_restrict polynomial_pixels;

  size_t
    number_images;

  ssize_t
    y;

  assert(images != (Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImageCanvas(images,exception);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  number_images=GetImageListLength(images);
  polynomial_pixels=AcquirePixelThreadSet(images);
  if (polynomial_pixels == (PixelChannels **) NULL)
    {
      image=DestroyImage(image);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",images->filename);
      return((Image *) NULL);
    }
  /*
    Polynomial image pixels.
  */
  status=MagickTrue;
  progress=0;
  polynomial_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
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
      *polynomial_pixel;

    register Quantum
      *magick_restrict q;

    ssize_t
      j;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(polynomial_view,0,y,image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    polynomial_pixel=polynomial_pixels[id];
    for (j=0; j < (ssize_t) image->columns; j++)
      for (i=0; i < MaxPixelChannels; i++)
        polynomial_pixel[j].channel[i]=0.0;
    next=images;
    for (j=0; j < (ssize_t) number_images; j++)
    {
      register const Quantum
        *p;

      if (j >= (ssize_t) number_terms)
        continue;
      image_view=AcquireVirtualCacheView(next,exception);
      p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        {
          image_view=DestroyCacheView(image_view);
          break;
        }
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        register ssize_t
          i;

        for (i=0; i < (ssize_t) GetPixelChannels(next); i++)
        {
          MagickRealType
            coefficient,
            degree;

          PixelChannel channel = GetPixelChannelChannel(image,i);
          PixelTrait traits = GetPixelChannelTraits(next,channel);
          PixelTrait polynomial_traits=GetPixelChannelTraits(image,channel);
          if ((traits == UndefinedPixelTrait) ||
              (polynomial_traits == UndefinedPixelTrait))
            continue;
          if ((traits & UpdatePixelTrait) == 0)
            continue;
          coefficient=(MagickRealType) terms[2*j];
          degree=(MagickRealType) terms[(j << 1)+1];
          polynomial_pixel[x].channel[i]+=coefficient*
            pow(QuantumScale*GetPixelChannel(image,channel,p),degree);
        }
        p+=GetPixelChannels(next);
      }
      image_view=DestroyCacheView(image_view);
      next=GetNextImageInList(next);
    }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ClampToQuantum(QuantumRange*polynomial_pixel[x].channel[i]);
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(polynomial_view,exception) == MagickFalse)
      status=MagickFalse;
    if (images->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_PolynomialImages)
#endif
        proceed=SetImageProgress(images,PolynomialImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  polynomial_view=DestroyCacheView(polynomial_view);
  polynomial_pixels=DestroyPixelThreadSet(polynomial_pixels);
  if (status == MagickFalse)
    image=DestroyImage(image);
  return(image);
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
    pixel_list->skip_list.nodes=(SkipNode *) RelinquishAlignedMemory(
      pixel_list->skip_list.nodes);
  pixel_list=(PixelList *) RelinquishMagickMemory(pixel_list);
  return(pixel_list);
}

static PixelList **DestroyPixelListThreadSet(PixelList **pixel_list)
{
  register ssize_t
    i;

  assert(pixel_list != (PixelList **) NULL);
  for (i=0; i < (ssize_t) GetMagickResourceLimit(ThreadResource); i++)
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
  (void) memset((void *) pixel_list,0,sizeof(*pixel_list));
  pixel_list->length=width*height;
  pixel_list->skip_list.nodes=(SkipNode *) AcquireAlignedMemory(65537UL,
    sizeof(*pixel_list->skip_list.nodes));
  if (pixel_list->skip_list.nodes == (SkipNode *) NULL)
    return(DestroyPixelList(pixel_list));
  (void) memset(pixel_list->skip_list.nodes,0,65537UL*
    sizeof(*pixel_list->skip_list.nodes));
  pixel_list->signature=MagickCoreSignature;
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

  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  pixel_list=(PixelList **) AcquireQuantumMemory(number_threads,
    sizeof(*pixel_list));
  if (pixel_list == (PixelList **) NULL)
    return((PixelList **) NULL);
  (void) memset(pixel_list,0,number_threads*sizeof(*pixel_list));
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
  double
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
    sum+=(double) p->nodes[color].count*color;
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

static inline void GetRootMeanSquarePixelList(PixelList *pixel_list,
  Quantum *pixel)
{
  double
    sum;

  register SkipList
    *p;

  size_t
    color;

  ssize_t
    count;

  /*
    Find the root mean square value for each of the color.
  */
  p=(&pixel_list->skip_list);
  color=65536L;
  count=0;
  sum=0.0;
  do
  {
    color=p->nodes[color].next[0];
    sum+=(double) (p->nodes[color].count*color*color);
    count+=p->nodes[color].count;
  } while (count < (ssize_t) pixel_list->length);
  sum/=pixel_list->length;
  *pixel=ScaleShortToQuantum((unsigned short) sqrt(sum));
}

static inline void GetStandardDeviationPixelList(PixelList *pixel_list,
  Quantum *pixel)
{
  double
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
    sum+=(double) p->nodes[color].count*color;
    for (i=0; i < (ssize_t) p->nodes[color].count; i++)
      sum_squared+=((double) color)*((double) color);
    count+=p->nodes[color].count;
  } while (count < (ssize_t) pixel_list->length);
  sum/=pixel_list->length;
  sum_squared/=pixel_list->length;
  *pixel=ScaleShortToQuantum((unsigned short) sqrt(sum_squared-(sum*sum)));
}

static inline void InsertPixelList(const Quantum pixel,PixelList *pixel_list)
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
    **magick_restrict pixel_list;

  ssize_t
    center,
    y;

  /*
    Initialize statistics image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
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
  image_view=AcquireVirtualCacheView(image,exception);
  statistic_view=AcquireAuthenticCacheView(statistic_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,statistic_image,statistic_image->rows,1)
#endif
  for (y=0; y < (ssize_t) statistic_image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

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

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        Quantum
          pixel;

        register const Quantum
          *magick_restrict pixels;

        register ssize_t
          u;

        ssize_t
          v;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait statistic_traits=GetPixelChannelTraits(statistic_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (statistic_traits == UndefinedPixelTrait))
          continue;
        if (((statistic_traits & CopyPixelTrait) != 0) ||
            (GetPixelWriteMask(image,p) <= (QuantumRange/2)))
          {
            SetPixelChannel(statistic_image,channel,p[center+i],q);
            continue;
          }
        if ((statistic_traits & UpdatePixelTrait) == 0)
          continue;
        pixels=p;
        ResetPixelList(pixel_list[id]);
        for (v=0; v < (ssize_t) MagickMax(height,1); v++)
        {
          for (u=0; u < (ssize_t) MagickMax(width,1); u++)
          {
            InsertPixelList(pixels[i],pixel_list[id]);
            pixels+=GetPixelChannels(image);
          }
          pixels+=GetPixelChannels(image)*image->columns;
        }
        switch (type)
        {
          case GradientStatistic:
          {
            double
              maximum,
              minimum;

            GetMinimumPixelList(pixel_list[id],&pixel);
            minimum=(double) pixel;
            GetMaximumPixelList(pixel_list[id],&pixel);
            maximum=(double) pixel;
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
          case RootMeanSquareStatistic:
          {
            GetRootMeanSquarePixelList(pixel_list[id],&pixel);
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
  if (status == MagickFalse)
    statistic_image=DestroyImage(statistic_image);
  return(statistic_image);
}

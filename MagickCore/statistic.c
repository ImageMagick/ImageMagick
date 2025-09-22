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
#include "MagickCore/statistic-private.h"
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
    channel[MaxPixelChannels];
} PixelChannels;

static PixelChannels **DestroyPixelTLS(const Image *images,
  PixelChannels **pixels)
{
  ssize_t
    i;

  size_t
    rows;

  assert(pixels != (PixelChannels **) NULL);
  rows=MagickMax(GetImageListLength(images),(size_t)
    GetMagickResourceLimit(ThreadResource));
  for (i=0; i < (ssize_t) rows; i++)
    if (pixels[i] != (PixelChannels *) NULL)
      pixels[i]=(PixelChannels *) RelinquishMagickMemory(pixels[i]);
  pixels=(PixelChannels **) RelinquishMagickMemory(pixels);
  return(pixels);
}

static PixelChannels **AcquirePixelTLS(const Image *images)
{
  const Image
    *next;

  PixelChannels
    **pixels;

  ssize_t
    i;

  size_t
    columns,
    number_images,
    rows;

  number_images=GetImageListLength(images);
  rows=MagickMax(number_images,(size_t) GetMagickResourceLimit(ThreadResource));
  pixels=(PixelChannels **) AcquireQuantumMemory(rows,sizeof(*pixels));
  if (pixels == (PixelChannels **) NULL)
    return((PixelChannels **) NULL);
  (void) memset(pixels,0,rows*sizeof(*pixels));
  columns=MagickMax(number_images,MaxPixelChannels);
  for (next=images; next != (Image *) NULL; next=next->next)
    columns=MagickMax(next->columns,columns);
  for (i=0; i < (ssize_t) rows; i++)
  {
    ssize_t
      j;

    pixels[i]=(PixelChannels *) AcquireQuantumMemory(columns,sizeof(**pixels));
    if (pixels[i] == (PixelChannels *) NULL)
      return(DestroyPixelTLS(images,pixels));
    for (j=0; j < (ssize_t) columns; j++)
    {
      ssize_t
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

  ssize_t
    i;

  color_1=(const PixelChannels *) x;
  color_2=(const PixelChannels *) y;
  distance=0.0;
  for (i=0; i < MaxPixelChannels; i++)
    distance+=color_1->channel[i]-(double) color_2->channel[i];
  return(distance < 0.0 ? -1 : distance > 0.0 ? 1 : 0);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static double ApplyEvaluateOperator(RandomInfo *random_info,const Quantum pixel,
  const MagickEvaluateOperator op,const double value)
{
  double
    result;

  ssize_t
    i;

  result=0.0;
  switch (op)
  {
    case UndefinedEvaluateOperator:
      break;
    case AbsEvaluateOperator:
    {
      result=(double) fabs((double) pixel+value);
      break;
    }
    case AddEvaluateOperator:
    {
      result=(double) pixel+value;
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
      result=(double) pixel+value;
      result-=((double) QuantumRange+1.0)*floor(result/((double)
        QuantumRange+1.0));
      break;
    }
    case AndEvaluateOperator:
    {
      result=(double) ((ssize_t) pixel & (ssize_t) (value+0.5));
      break;
    }
    case CosineEvaluateOperator:
    {
      result=(double) QuantumRange*(0.5*cos((double) (2.0*MagickPI*
        QuantumScale*(double) pixel*value))+0.5);
      break;
    }
    case DivideEvaluateOperator:
    {
      result=(double) pixel/(value == 0.0 ? 1.0 : value);
      break;
    }
    case ExponentialEvaluateOperator:
    {
      result=(double) QuantumRange*exp(value*QuantumScale*(double) pixel);
      break;
    }
    case GaussianNoiseEvaluateOperator:
    {
      result=(double) GenerateDifferentialNoise(random_info,pixel,GaussianNoise,
        value);
      break;
    }
    case ImpulseNoiseEvaluateOperator:
    {
      result=(double) GenerateDifferentialNoise(random_info,pixel,ImpulseNoise,
        value);
      break;
    }
    case InverseLogEvaluateOperator:
    {
      result=(double) QuantumRange*pow((value+1.0),QuantumScale*(double)
        pixel-1.0)*MagickSafeReciprocal(value);
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
      result=(double) pixel;
      for (i=0; i < (ssize_t) value; i++)
        result*=2.0;
      break;
    }
    case LogEvaluateOperator:
    {
      if ((QuantumScale*(double) pixel) >= MagickEpsilon)
        result=(double) QuantumRange*log(QuantumScale*value*
          (double) pixel+1.0)/log((double) (value+1.0));
      break;
    }
    case MaxEvaluateOperator:
    {
      result=(double) EvaluateMax((double) pixel,value);
      break;
    }
    case MeanEvaluateOperator:
    {
      result=(double) pixel+value;
      break;
    }
    case MedianEvaluateOperator:
    {
      result=(double) pixel+value;
      break;
    }
    case MinEvaluateOperator:
    {
      result=MagickMin((double) pixel,value);
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
      result=(double) pixel*value;
      break;
    }
    case OrEvaluateOperator:
    {
      result=(double) ((ssize_t) pixel | (ssize_t) (value+0.5));
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
      if (fabs(value) <= MagickEpsilon)
        break;
      if (((double) pixel < 0.0) && ((value-floor(value)) > MagickEpsilon))
        result=(double) -((double) QuantumRange*pow(-(QuantumScale*(double)
          pixel),value));
      else
        result=(double) QuantumRange*pow(QuantumScale*(double) pixel,value);
      break;
    }
    case RightShiftEvaluateOperator:
    {
      result=(double) pixel;
      for (i=0; i < (ssize_t) value; i++)
        result/=2.0;
      break;
    }
    case RootMeanSquareEvaluateOperator:
    {
      result=((double) pixel*(double) pixel+value);
      break;
    }
    case SetEvaluateOperator:
    {
      result=value;
      break;
    }
    case SineEvaluateOperator:
    {
      result=(double) QuantumRange*(0.5*sin((double) (2.0*MagickPI*
        QuantumScale*(double) pixel*value))+0.5);
      break;
    }
    case SubtractEvaluateOperator:
    {
      result=(double) pixel-value;
      break;
    }
    case SumEvaluateOperator:
    {
      result=(double) pixel+value;
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
      result=(double) ((ssize_t) pixel ^ (ssize_t) (value+0.5));
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
    *evaluate_view,
    **image_view;

  const Image
    *view;

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
    n,
    y;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  unsigned long
    key;
#endif

  assert(images != (Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  image=AcquireImageCanvas(images,exception);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  number_images=GetImageListLength(images);
  evaluate_pixels=AcquirePixelTLS(images);
  if (evaluate_pixels == (PixelChannels **) NULL)
    {
      image=DestroyImage(image);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",images->filename);
      return((Image *) NULL);
    }
  image_view=(CacheView **) AcquireQuantumMemory(number_images,
    sizeof(*image_view));
  if (image_view == (CacheView **) NULL)
    {
      image=DestroyImage(image);
      evaluate_pixels=DestroyPixelTLS(images,evaluate_pixels);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",images->filename);
      return(image);
    }
  view=images;
  for (n=0; n < (ssize_t) number_images; n++)
  {
    image_view[n]=AcquireVirtualCacheView(view,exception);
    view=GetNextImageInList(view);
  }
  /*
    Evaluate image pixels.
  */
  status=MagickTrue;
  progress=0;
  random_info=AcquireRandomInfoTLS();
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
        const int
          id = GetOpenMPThreadId();

        const Quantum
          **p;

        PixelChannels
          *evaluate_pixel;

        Quantum
          *magick_restrict q;

        ssize_t
          x;

        ssize_t
          j;

        if (status == MagickFalse)
          continue;
        p=(const Quantum **) AcquireQuantumMemory(number_images,sizeof(*p));
        if (p == (const Quantum **) NULL)
          {
            status=MagickFalse;
            (void) ThrowMagickException(exception,GetMagickModule(),
              ResourceLimitError,"MemoryAllocationFailed","`%s'",
              images->filename);
            continue;
          }
        for (j=0; j < (ssize_t) number_images; j++)
        {
          p[j]=GetCacheViewVirtualPixels(image_view[j],0,y,image->columns,1,
            exception);
          if (p[j] == (const Quantum *) NULL)
            break;
        }
        q=QueueCacheViewAuthenticPixels(evaluate_view,0,y,image->columns,1,
          exception);
        if ((j < (ssize_t) number_images) || (q == (Quantum *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        evaluate_pixel=evaluate_pixels[id];
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          const Image
            *next;

          ssize_t
            i;

          next=images;
          for (j=0; j < (ssize_t) number_images; j++)
          {
            for (i=0; i < MaxPixelChannels; i++)
              evaluate_pixel[j].channel[i]=0.0;
            for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
            {
              PixelChannel channel = GetPixelChannelChannel(image,i);
              PixelTrait traits = GetPixelChannelTraits(next,channel);
              PixelTrait evaluate_traits = GetPixelChannelTraits(image,channel);
              if (((traits & UpdatePixelTrait) == 0) ||
                  ((evaluate_traits & UpdatePixelTrait) == 0))
                continue;
              evaluate_pixel[j].channel[i]=ApplyEvaluateOperator(
                random_info[id],GetPixelChannel(next,channel,p[j]),op,
                evaluate_pixel[j].channel[i]);
            }
            p[j]+=(ptrdiff_t) GetPixelChannels(next);
            next=GetNextImageInList(next);
          }
          qsort((void *) evaluate_pixel,number_images,sizeof(*evaluate_pixel),
            IntensityCompare);
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            if ((traits & UpdatePixelTrait) == 0)
              continue;
            q[i]=ClampToQuantum(evaluate_pixel[number_images/2].channel[i]);
          }
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        p=(const Quantum **) RelinquishMagickMemory((void *) p);
        if (SyncCacheViewAuthenticPixels(evaluate_view,exception) == MagickFalse)
          status=MagickFalse;
        if (images->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp atomic
#endif
            progress++;
            proceed=SetImageProgress(images,EvaluateImageTag,progress,
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
        const Image
          *next;

        const int
          id = GetOpenMPThreadId();

        const Quantum
          **p;

        PixelChannels
          *evaluate_pixel;

        Quantum
          *magick_restrict q;

        ssize_t
          i,
          x;

        ssize_t
          j;

        if (status == MagickFalse)
          continue;
        p=(const Quantum **) AcquireQuantumMemory(number_images,sizeof(*p));
        if (p == (const Quantum **) NULL)
          {
            status=MagickFalse;
            (void) ThrowMagickException(exception,GetMagickModule(),
              ResourceLimitError,"MemoryAllocationFailed","`%s'",
              images->filename);
            continue;
          }
        for (j=0; j < (ssize_t) number_images; j++)
        {
          p[j]=GetCacheViewVirtualPixels(image_view[j],0,y,image->columns,1,
            exception);
          if (p[j] == (const Quantum *) NULL)
            break;
        }
        q=QueueCacheViewAuthenticPixels(evaluate_view,0,y,image->columns,1,
          exception);
        if ((j < (ssize_t) number_images) || (q == (Quantum *) NULL))
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
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            for (i=0; i < (ssize_t) GetPixelChannels(next); i++)
            {
              PixelChannel channel = GetPixelChannelChannel(image,i);
              PixelTrait traits = GetPixelChannelTraits(next,channel);
              PixelTrait evaluate_traits = GetPixelChannelTraits(image,channel);
              if (((traits & UpdatePixelTrait) == 0) ||
                  ((evaluate_traits & UpdatePixelTrait) == 0))
                continue;
              evaluate_pixel[x].channel[i]=ApplyEvaluateOperator(
                random_info[id],GetPixelChannel(next,channel,p[j]),j == 0 ?
                AddEvaluateOperator : op,evaluate_pixel[x].channel[i]);
            }
            p[j]+=(ptrdiff_t) GetPixelChannels(next);
          }
          next=GetNextImageInList(next);
        }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
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
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            if ((traits & UpdatePixelTrait) == 0)
              continue;
            q[i]=ClampToQuantum(evaluate_pixel[x].channel[i]);
          }
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        p=(const Quantum **) RelinquishMagickMemory((void *) p);
        if (SyncCacheViewAuthenticPixels(evaluate_view,exception) == MagickFalse)
          status=MagickFalse;
        if (images->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp atomic
#endif
            progress++;
            proceed=SetImageProgress(images,EvaluateImageTag,progress,
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
    }
  for (n=0; n < (ssize_t) number_images; n++)
    image_view[n]=DestroyCacheView(image_view[n]);
  image_view=(CacheView **) RelinquishMagickMemory(image_view);
  evaluate_view=DestroyCacheView(evaluate_view);
  evaluate_pixels=DestroyPixelTLS(images,evaluate_pixels);
  random_info=DestroyRandomInfoTLS(random_info);
  if (status == MagickFalse)
    image=DestroyImage(image);
  return(image);
}

MagickExport MagickBooleanType EvaluateImage(Image *image,
  const MagickEvaluateOperator op,const double value,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  const char
    *artifact;

  MagickBooleanType
    clamp,
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
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  clamp=MagickFalse;
  artifact=GetImageArtifact(image,"evaluate:clamp");
  if (artifact != (const char *) NULL)
    clamp=IsStringTrue(artifact);
  random_info=AcquireRandomInfoTLS();
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

    Quantum
      *magick_restrict q;

    ssize_t
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

      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        result=ApplyEvaluateOperator(random_info[id],q[i],op,value);
        if (op == MeanEvaluateOperator)
          result/=2.0;
        q[i]=clamp != MagickFalse ? ClampPixel(result) : ClampToQuantum(result);
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,EvaluateImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  random_info=DestroyRandomInfoTLS(random_info);
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

  ssize_t
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
        result=result*QuantumScale*(double) pixel+parameters[i];
      result*=(double) QuantumRange;
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
      result=(double) QuantumRange*(amplitude*sin((double) (2.0*
        MagickPI*(frequency*QuantumScale*(double) pixel+phase/360.0)))+bias);
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
        Arcsin (pegged at range limits for invalid results): width, center,
        range, and bias.
      */
      width=(number_parameters >= 1) ? parameters[0] : 1.0;
      center=(number_parameters >= 2) ? parameters[1] : 0.5;
      range=(number_parameters >= 3) ? parameters[2] : 1.0;
      bias=(number_parameters >= 4) ? parameters[3] : 0.5;
      result=2.0*MagickSafeReciprocal(width)*(QuantumScale*(double) pixel-
        center);
      if (result <= -1.0)
        result=bias-range/2.0;
      else
        if (result >= 1.0)
          result=bias+range/2.0;
        else
          result=(double) (range/MagickPI*asin((double) result)+bias);
      result*=(double) QuantumRange;
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
      result=MagickPI*slope*(QuantumScale*(double) pixel-center);
      result=(double) QuantumRange*(range/MagickPI*atan((double) result)+bias);
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
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
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
    Quantum
      *magick_restrict q;

    ssize_t
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
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ApplyFunction(q[i],function,number_parameters,parameters,
          exception);
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,FunctionImageTag,progress,image->rows);
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
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=GetImageStatistics(image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    {
      *entropy=NAN;
      return(MagickFalse);
    }
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
  if (IsEventLogging() != MagickFalse)
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
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=GetImageStatistics(image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    {
      *kurtosis=NAN;
      *skewness=NAN;
      return(MagickFalse);
    }
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
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=GetImageStatistics(image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    {
      *mean=NAN;
      *standard_deviation=NAN;
      return(MagickFalse);
    }
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
%   G e t I m a g e M e d i a n                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageMedian() returns the median pixel of one or more image channels.
%
%  The format of the GetImageMedian method is:
%
%      MagickBooleanType GetImageMedian(const Image *image,double *median,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o median: the average value in the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetImageMedian(const Image *image,double *median,
  ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=GetImageStatistics(image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    {
      *median=NAN;
      return(MagickFalse);
    }
  *median=channel_statistics[CompositePixelChannel].median;
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
MagickExport ChannelMoments *GetImageMoments(const Image *image,
  ExceptionInfo *exception)
{
#define MaxNumberImageMoments  8

  CacheView
    *image_view;

  ChannelMoments
    *channel_moments;

  double
    channels,
    M00[2*MaxPixelChannels+1] = { 0.0 },
    M01[2*MaxPixelChannels+1] = { 0.0 },
    M02[2*MaxPixelChannels+1] = { 0.0 },
    M03[2*MaxPixelChannels+1] = { 0.0 },
    M10[2*MaxPixelChannels+1] = { 0.0 },
    M11[2*MaxPixelChannels+1] = { 0.0 },
    M12[2*MaxPixelChannels+1] = { 0.0 },
    M20[2*MaxPixelChannels+1] = { 0.0 },
    M21[2*MaxPixelChannels+1] = { 0.0 },
    M22[2*MaxPixelChannels+1] = { 0.0 },
    M30[2*MaxPixelChannels+1] = { 0.0 };

  PointInfo
    centroid[2*MaxPixelChannels+1] = { 0 };

  ssize_t
    c,
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_moments=(ChannelMoments *) AcquireQuantumMemory(MaxPixelChannels+1,
    sizeof(*channel_moments));
  if (channel_moments == (ChannelMoments *) NULL)
    return(channel_moments);
  (void) memset(channel_moments,0,(MaxPixelChannels+1)*
    sizeof(*channel_moments));
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    /*
      Compute center of mass (centroid).
    */
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          pixel;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        pixel=QuantumScale*p[i];
        M00[channel]+=pixel;
        M00[CompositePixelChannel]+=pixel;
        M10[channel]+=x*pixel;
        M10[CompositePixelChannel]+=x*pixel;
        M01[channel]+=y*pixel;
        M01[CompositePixelChannel]+=y*pixel;
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  for (c=0; c <= MaxPixelChannels; c++)
  {
    /*
       Compute center of mass (centroid).
    */
    centroid[c].x=M10[c]*MagickSafeReciprocal(M00[c]);
    centroid[c].y=M01[c]*MagickSafeReciprocal(M00[c]);
  }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    /*
      Compute the image moments.
    */
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        M11[channel]+=(x-centroid[channel].x)*(y-centroid[channel].y)*
          QuantumScale*(double) p[i];
        M11[CompositePixelChannel]+=(x-centroid[channel].x)*(y-
          centroid[channel].y)*QuantumScale*(double) p[i];
        M20[channel]+=(x-centroid[channel].x)*(x-centroid[channel].x)*
          QuantumScale*(double) p[i];
        M20[CompositePixelChannel]+=(x-centroid[channel].x)*(x-
          centroid[channel].x)*QuantumScale*(double) p[i];
        M02[channel]+=(y-centroid[channel].y)*(y-centroid[channel].y)*
          QuantumScale*(double) p[i];
        M02[CompositePixelChannel]+=(y-centroid[channel].y)*(y-
          centroid[channel].y)*QuantumScale*(double) p[i];
        M21[channel]+=(x-centroid[channel].x)*(x-centroid[channel].x)*
          (y-centroid[channel].y)*QuantumScale*(double) p[i];
        M21[CompositePixelChannel]+=(x-centroid[channel].x)*(x-
          centroid[channel].x)*(y-centroid[channel].y)*QuantumScale*(double)
          p[i];
        M12[channel]+=(x-centroid[channel].x)*(y-centroid[channel].y)*
          (y-centroid[channel].y)*QuantumScale*(double) p[i];
        M12[CompositePixelChannel]+=(x-centroid[channel].x)*(y-
          centroid[channel].y)*(y-centroid[channel].y)*QuantumScale*(double)
          p[i];
        M22[channel]+=(x-centroid[channel].x)*(x-centroid[channel].x)*
          (y-centroid[channel].y)*(y-centroid[channel].y)*QuantumScale*(double)
          p[i];
        M22[CompositePixelChannel]+=(x-centroid[channel].x)*(x-
          centroid[channel].x)*(y-centroid[channel].y)*(y-centroid[channel].y)*
          QuantumScale*(double) p[i];
        M30[channel]+=(x-centroid[channel].x)*(x-centroid[channel].x)*
          (x-centroid[channel].x)*QuantumScale*(double) p[i];
        M30[CompositePixelChannel]+=(x-centroid[channel].x)*(x-
          centroid[channel].x)*(x-centroid[channel].x)*QuantumScale*(double)
          p[i];
        M03[channel]+=(y-centroid[channel].y)*(y-centroid[channel].y)*
          (y-centroid[channel].y)*QuantumScale*(double) p[i];
        M03[CompositePixelChannel]+=(y-centroid[channel].y)*(y-
          centroid[channel].y)*(y-centroid[channel].y)*QuantumScale*(double)
          p[i];
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  channels=(double) GetImageChannels(image);
  M00[CompositePixelChannel]/=channels;
  M01[CompositePixelChannel]/=channels;
  M02[CompositePixelChannel]/=channels;
  M03[CompositePixelChannel]/=channels;
  M10[CompositePixelChannel]/=channels;
  M11[CompositePixelChannel]/=channels;
  M12[CompositePixelChannel]/=channels;
  M20[CompositePixelChannel]/=channels;
  M21[CompositePixelChannel]/=channels;
  M22[CompositePixelChannel]/=channels;
  M30[CompositePixelChannel]/=channels;
  for (c=0; c <= MaxPixelChannels; c++)
  {
    /*
      Compute elliptical angle, major and minor axes, eccentricity, & intensity.
    */
    channel_moments[c].centroid=centroid[c];
    channel_moments[c].ellipse_axis.x=sqrt((2.0*MagickSafeReciprocal(M00[c]))*
      ((M20[c]+M02[c])+sqrt(4.0*M11[c]*M11[c]+(M20[c]-M02[c])*
       (M20[c]-M02[c]))));
    channel_moments[c].ellipse_axis.y=sqrt((2.0*MagickSafeReciprocal(M00[c]))*
      ((M20[c]+M02[c])-sqrt(4.0*M11[c]*M11[c]+(M20[c]-M02[c])*
       (M20[c]-M02[c]))));
    channel_moments[c].ellipse_angle=RadiansToDegrees(1.0/2.0*atan(2.0*
      M11[c]*MagickSafeReciprocal(M20[c]-M02[c])));
    if (fabs(M11[c]) < 0.0)
      {
        if ((fabs(M20[c]-M02[c]) >= 0.0) && ((M20[c]-M02[c]) < 0.0))
          channel_moments[c].ellipse_angle+=90.0;
      }
    else
      if (M11[c] < 0.0)
        {
          if (fabs(M20[c]-M02[c]) >= 0.0)
            {
              if ((M20[c]-M02[c]) < 0.0)
                channel_moments[c].ellipse_angle+=90.0;
              else
                channel_moments[c].ellipse_angle+=180.0;
            }
        }
      else
        if ((fabs(M20[c]-M02[c]) >= 0.0) && ((M20[c]-M02[c]) < 0.0))
          channel_moments[c].ellipse_angle+=90.0;
    channel_moments[c].ellipse_eccentricity=sqrt(1.0-(
      channel_moments[c].ellipse_axis.y*
      channel_moments[c].ellipse_axis.y*MagickSafeReciprocal(
      channel_moments[c].ellipse_axis.x*
      channel_moments[c].ellipse_axis.x)));
    channel_moments[c].ellipse_intensity=M00[c]*
      MagickSafeReciprocal(MagickPI*channel_moments[c].ellipse_axis.x*
      channel_moments[c].ellipse_axis.y+MagickEpsilon);
  }
  for (c=0; c <= MaxPixelChannels; c++)
  {
    /*
      Normalize image moments.
    */
    M10[c]=0.0;
    M01[c]=0.0;
    M11[c]*=MagickSafeReciprocal(pow(M00[c],1.0+(1.0+1.0)/2.0));
    M20[c]*=MagickSafeReciprocal(pow(M00[c],1.0+(2.0+0.0)/2.0));
    M02[c]*=MagickSafeReciprocal(pow(M00[c],1.0+(0.0+2.0)/2.0));
    M21[c]*=MagickSafeReciprocal(pow(M00[c],1.0+(2.0+1.0)/2.0));
    M12[c]*=MagickSafeReciprocal(pow(M00[c],1.0+(1.0+2.0)/2.0));
    M22[c]*=MagickSafeReciprocal(pow(M00[c],1.0+(2.0+2.0)/2.0));
    M30[c]*=MagickSafeReciprocal(pow(M00[c],1.0+(3.0+0.0)/2.0));
    M03[c]*=MagickSafeReciprocal(pow(M00[c],1.0+(0.0+3.0)/2.0));
    M00[c]=1.0;
  }
  image_view=DestroyCacheView(image_view);
  for (c=0; c <= MaxPixelChannels; c++)
  {
    /*
      Compute Hu invariant moments.
    */
    channel_moments[c].invariant[0]=M20[c]+M02[c];
    channel_moments[c].invariant[1]=(M20[c]-M02[c])*(M20[c]-M02[c])+4.0*M11[c]*
      M11[c];
    channel_moments[c].invariant[2]=(M30[c]-3.0*M12[c])*(M30[c]-3.0*M12[c])+
      (3.0*M21[c]-M03[c])*(3.0*M21[c]-M03[c]);
    channel_moments[c].invariant[3]=(M30[c]+M12[c])*(M30[c]+M12[c])+(M21[c]+
      M03[c])*(M21[c]+M03[c]);
    channel_moments[c].invariant[4]=(M30[c]-3.0*M12[c])*(M30[c]+M12[c])*
      ((M30[c]+M12[c])*(M30[c]+M12[c])-3.0*(M21[c]+M03[c])*(M21[c]+M03[c]))+
      (3.0*M21[c]-M03[c])*(M21[c]+M03[c])*(3.0*(M30[c]+M12[c])*(M30[c]+M12[c])-
      (M21[c]+M03[c])*(M21[c]+M03[c]));
    channel_moments[c].invariant[5]=(M20[c]-M02[c])*((M30[c]+M12[c])*(M30[c]+
      M12[c])-(M21[c]+M03[c])*(M21[c]+M03[c]))+4.0*M11[c]*(M30[c]+M12[c])*
      (M21[c]+M03[c]);
    channel_moments[c].invariant[6]=(3.0*M21[c]-M03[c])*(M30[c]+M12[c])*
      ((M30[c]+M12[c])*(M30[c]+M12[c])-3.0*(M21[c]+M03[c])*(M21[c]+M03[c]))-
      (M30[c]-3*M12[c])*(M21[c]+M03[c])*(3.0*(M30[c]+M12[c])*(M30[c]+M12[c])-
      (M21[c]+M03[c])*(M21[c]+M03[c]));
    channel_moments[c].invariant[7]=M11[c]*((M30[c]+M12[c])*(M30[c]+M12[c])-
      (M03[c]+M21[c])*(M03[c]+M21[c]))-(M20[c]-M02[c])*(M30[c]+M12[c])*
      (M03[c]+M21[c]);
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
MagickExport ChannelPerceptualHash *GetImagePerceptualHash(const Image *image,
  ExceptionInfo *exception)
{
  ChannelPerceptualHash
    *perceptual_hash;

  char
    *colorspaces,
    *p,
    *q;

  const char
    *artifact;

  MagickBooleanType
    status;

  ssize_t
    i;

  perceptual_hash=(ChannelPerceptualHash *) AcquireQuantumMemory(
    MaxPixelChannels+1UL,sizeof(*perceptual_hash));
  if (perceptual_hash == (ChannelPerceptualHash *) NULL)
    return((ChannelPerceptualHash *) NULL);
  artifact=GetImageArtifact(image,"phash:colorspaces");
  if (artifact != (const char *) NULL)
    colorspaces=AcquireString(artifact);
  else
    colorspaces=AcquireString("xyY,HSB");
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
          (-MagickSafeLog10(moments[channel].invariant[j]));
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
  typedef struct
  {
    double
      maxima,
      minima;
  } RangeInfo;

  CacheView
    *image_view;

  MagickBooleanType
    status;

  RangeInfo
    range_info = { -MagickMaximumValue, MagickMaximumValue };

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(range_info,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    RangeInfo
      channel_range;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    channel_range=range_info;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if ((double) p[i] > channel_range.maxima)
          channel_range.maxima=(double) p[i];
        if ((double) p[i] < channel_range.minima)
          channel_range.minima=(double) p[i];
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetImageRange)
#endif
    {
      if (channel_range.maxima > range_info.maxima)
        range_info.maxima=channel_range.maxima;
      if (channel_range.minima < range_info.minima)
        range_info.minima=channel_range.minima;
    }
  }
  image_view=DestroyCacheView(image_view);
  *maxima=range_info.maxima;
  *minima=range_info.minima;
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

static ssize_t GetMedianPixel(Quantum *pixels,const size_t n)
{
#define SwapPixels(alpha,beta) \
{ \
  Quantum gamma=(alpha); \
  (alpha)=(beta);(beta)=gamma; \
}

  ssize_t
    low = 0,
    high = (ssize_t) n-1,
    median = (low+high)/2;

  for ( ; ; )
  {
    ssize_t
      l = low+1,
      h = high,
      mid = (low+high)/2;

    if (high <= low)
      return(median);
    if (high == (low+1))
      {
        if (pixels[low] > pixels[high])
          SwapPixels(pixels[low],pixels[high]);
        return(median);
      }
    if (pixels[mid] > pixels[high])
      SwapPixels(pixels[mid],pixels[high]);
    if (pixels[low] > pixels[high])
      SwapPixels(pixels[low], pixels[high]);
    if (pixels[mid] > pixels[low])
      SwapPixels(pixels[mid],pixels[low]);
    SwapPixels(pixels[mid],pixels[low+1]);
    for ( ; ; )
    {
      do l++; while (pixels[low] > pixels[l]);
      do h--; while (pixels[h] > pixels[low]);
      if (h < l)
        break;
      SwapPixels(pixels[l],pixels[h]);
    }
    SwapPixels(pixels[low],pixels[h]);
    if (h <= median)
      low=l;
    if (h >= median)
      high=h-1;
  }
}

static inline long double MagickSafeReciprocalLD(const long double x)
{
  long double
    sign;

  /*
    Return 1/x where x is perceptible (not unlimited or infinitesimal).
  */
  sign=x < 0.0 ? -1.0 : 1.0;
  if ((sign*x) >= MagickEpsilon)
    return(1.0/x);
  return(sign/MagickEpsilon);
}

MagickExport ChannelStatistics *GetImageStatistics(const Image *image,
  ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  double
    channels,
    *histogram;

  long double
    area;

  MagickStatusType
    status;

  MemoryInfo
    *median_info;

  Quantum
    *median;

  QuantumAny
    range;

  size_t
    depth;

  ssize_t
    i,
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  histogram=(double *) AcquireQuantumMemory(MaxMap+1UL,
    MagickMax(GetPixelChannels(image),1)*sizeof(*histogram));
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
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(channel_statistics);
    }
  (void) memset(channel_statistics,0,(MaxPixelChannels+1)*
    sizeof(*channel_statistics));
  for (i=0; i <= (ssize_t) MaxPixelChannels; i++)
  {
    ChannelStatistics *cs = channel_statistics+i;
    cs->area=0.0;
    cs->depth=1;
    cs->maxima=(-MagickMaximumValue);
    cs->minima=MagickMaximumValue;
    cs->sum=0.0;
    cs->sumLD=0.0;
    cs->mean=0.0;
    cs->standard_deviation=0.0;
    cs->variance=0.0;
    cs->skewness=0.0;
    cs->kurtosis=0.0;
    cs->entropy=0.0;
  }
  (void) memset(histogram,0,(MaxMap+1)*GetPixelChannels(image)*
    sizeof(*histogram));
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    /*
      Compute pixel statistics.
    */
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (GetPixelReadMask(image,p) <= (QuantumRange/2))
        {
          p+=(ptrdiff_t) GetPixelChannels(image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        ChannelStatistics
          *cs;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        cs=channel_statistics+channel;
        if (cs->depth != MAGICKCORE_QUANTUM_DEPTH)
          {
            depth=cs->depth;
            range=GetQuantumRange(depth);
            status=p[i] != ScaleAnyToQuantum(ScaleQuantumToAny(p[i],range),
              range) ? MagickTrue : MagickFalse;
            if (status != MagickFalse)
              {
                cs->depth++;
                if (cs->depth > channel_statistics[CompositePixelChannel].depth)
                  channel_statistics[CompositePixelChannel].depth=cs->depth;
                i--;
                continue;
              }
          }
        cs->area++;
        if ((double) p[i] < cs->minima)
          cs->minima=(double) p[i];
        if ((double) p[i] > cs->maxima)
          cs->maxima=(double) p[i];
        histogram[(ssize_t) GetPixelChannels(image)*ScaleQuantumToMap(
          ClampToQuantum((double) p[i]))+i]++;
        cs->sumLD+=(long double) p[i];
        /*
          sum_squared, sum_cubed and sum_fourth_power are not used in
          MagickCore or MagickWand, but are made available in
          Magick++/lib/Statistic.cpp, so we need to calculate these.
        */
        cs->sum_squared+=(double) p[i]*(double) p[i];
        cs->sum_cubed+=(double) p[i]*(double) p[i]*(double) p[i];
        cs->sum_fourth_power+=(double) p[i]*(double) p[i]*(double) p[i]*
          (double) p[i];
        {
          /* Calculate running totals for Welford's method.
          */
          double
            n = cs->area,
            n1 = cs->area-1;

          long double
            delta,
            delta_n,
            delta_n2,
            term1;

          delta=(double) p[i]-cs->M1;
          delta_n=delta/n;
          delta_n2=delta_n*delta_n;
          term1=delta*delta_n*n1;
          cs->M4+=term1*delta_n2*(n*n-3.0*n+3.0)+6.0*delta_n2*cs->M2-4.0*
            delta_n*cs->M3;
          cs->M3+=term1*delta_n*(n-2.0)-3.0*delta_n*cs->M2;
          cs->M2+=term1;
          cs->M1+=delta_n;
        }
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    ChannelStatistics
      *cs;

    double
      adj_area = 1.0;

    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    if ((traits & UpdatePixelTrait) == 0)
      continue;
    cs=channel_statistics+(ssize_t) channel;
    cs->mean=0.0;
    if (cs->area > 0)
      {
        cs->mean=(double) (cs->sumLD/(long double) cs->area);
        if (cs->area > 1.0)
          adj_area=cs->area/(cs->area-1.0);
      }
    cs->sum=(double) cs->sum;
    if (cs->M2 == 0.0)
      {
        cs->standard_deviation=0.0;
        cs->variance=0.0;
        cs->skewness=0.0;
        cs->kurtosis=0.0;
      }
    else
      {
        if (cs->area > 1.0)
          cs->standard_deviation=(double) sqrtl(cs->M2/((long double)
            cs->area-1.0));
        else
          cs->standard_deviation=(double) sqrtl(cs->M2/((long double)
            cs->area));
        cs->variance=cs->standard_deviation*cs->standard_deviation;
        cs->skewness=(double) (sqrtl(cs->area)*cs->M3/powl(cs->M2*adj_area,
          1.5));
        cs->kurtosis=(double) (cs->area*cs->M4/(cs->M2*cs->M2*adj_area*
          adj_area)-3.0);
      }
  }
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    ChannelStatistics
      *cs;

    double
      number_bins;

    ssize_t
      j;

    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    if ((traits & UpdatePixelTrait) == 0)
      continue;
    cs=channel_statistics+(ssize_t) channel;
    if (cs->area > 0.0)
      {
        cs->sum/=cs->area;
        cs->sum_squared/=cs->area;
        cs->sum_cubed/=cs->area;
        cs->sum_fourth_power/=cs->area;
      }
    /*
      Compute pixel entropy.
    */
    number_bins=0.0;
    for (j=0; j <= (ssize_t) MaxMap; j++)
      if (histogram[(ssize_t) GetPixelChannels(image)*j+i] > 0.0)
        number_bins++;
    area=MagickSafeReciprocalLD(channel_statistics[channel].area);
    number_bins=(double) MagickSafeReciprocalLD((long double) log2(number_bins));
    for (j=0; j <= (ssize_t) MaxMap; j++)
    {
      double
        entropy,
        count;

      count=(double) (area*histogram[(ssize_t) GetPixelChannels(image)*j+i]);
      entropy=-count*log2(count)*number_bins;
      if (IsNaN(entropy) != 0)
        continue;
      channel_statistics[channel].entropy+=(double) entropy;
      channel_statistics[CompositePixelChannel].entropy+=((double) entropy/
        GetPixelChannels(image));
    }
  }
  histogram=(double *) RelinquishMagickMemory(histogram);
  median_info=AcquireVirtualMemory(image->columns,image->rows*sizeof(*median));
  if (median_info == (MemoryInfo *) NULL)
    (void) ThrowMagickException(exception,GetMagickModule(),
      ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
  else
    {
      median=(Quantum *) GetVirtualMemoryBlob(median_info);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        size_t
          n = 0;

        /*
          Compute median statistics for each channel.
        */
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          const Quantum
            *magick_restrict p;

          ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            if (GetPixelReadMask(image,p) <= (QuantumRange/2))
              {
                p+=(ptrdiff_t) GetPixelChannels(image);
                continue;
              }
            median[n++]=p[i];
            p+=(ptrdiff_t) GetPixelChannels(image);
          }
        }
        channel_statistics[channel].median=(double) median[
          GetMedianPixel(median,n)];
      }
      median_info=RelinquishVirtualMemory(median_info);
    }
  {
    ChannelStatistics *cs_comp = channel_statistics+CompositePixelChannel;
    cs_comp->sum=0.0;
    cs_comp->sum_squared=0.0;
    cs_comp->sum_cubed=0.0;
    cs_comp->sum_fourth_power=0.0;
    cs_comp->maxima=(-MagickMaximumValue);
    cs_comp->minima=MagickMaximumValue;
    cs_comp->area=0.0;
    cs_comp->mean=0.0;
    cs_comp->median=0.0;
    cs_comp->variance=0.0;
    cs_comp->standard_deviation=0.0;
    cs_comp->entropy=0.0;
    cs_comp->skewness=0.0;
    cs_comp->kurtosis=0.0;
    for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
    {
      ChannelStatistics
        *cs;

      PixelChannel channel = GetPixelChannelChannel(image,i);
      PixelTrait traits = GetPixelChannelTraits(image,channel);
      if ((traits & UpdatePixelTrait) == 0)
        continue;
      cs=channel_statistics+channel;
      if (cs_comp->maxima < cs->maxima)
        cs_comp->maxima=cs->maxima;
      if (cs_comp->minima > cs->minima)
        cs_comp->minima=cs->minima;
      cs_comp->sum+=cs->sum;
      cs_comp->sum_squared+=cs->sum_squared;
      cs_comp->sum_cubed+=cs->sum_cubed;
      cs_comp->sum_fourth_power+=cs->sum_fourth_power;
      cs_comp->median+=cs->median;
      cs_comp->area+=cs->area;
      cs_comp->mean+=cs->mean;
      cs_comp->variance+=cs->variance;
      cs_comp->standard_deviation+=cs->standard_deviation;
      cs_comp->skewness+=cs->skewness;
      cs_comp->kurtosis+=cs->kurtosis;
      cs_comp->entropy+=cs->entropy;
    }
    channels=(double) GetImageChannels(image);
    cs_comp->sum/=channels;
    cs_comp->sum_squared/=channels;
    cs_comp->sum_cubed/=channels;
    cs_comp->sum_fourth_power/=channels;
    cs_comp->median/=channels;
    cs_comp->area/=channels;
    cs_comp->mean/=channels;
    cs_comp->variance/=channels;
    cs_comp->standard_deviation/=channels;
    cs_comp->skewness/=channels;
    cs_comp->kurtosis/=channels;
    cs_comp->entropy/=channels;
  }
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
  if (IsEventLogging() != MagickFalse)
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
  polynomial_pixels=AcquirePixelTLS(images);
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

    PixelChannels
      *polynomial_pixel;

    Quantum
      *magick_restrict q;

    ssize_t
      i,
      j,
      x;

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
      const Quantum
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
        for (i=0; i < (ssize_t) GetPixelChannels(next); i++)
        {
          MagickRealType
            coefficient,
            degree;

          PixelChannel channel = GetPixelChannelChannel(image,i);
          PixelTrait traits = GetPixelChannelTraits(next,channel);
          PixelTrait polynomial_traits = GetPixelChannelTraits(image,channel);
          if (((traits & UpdatePixelTrait) == 0) ||
              ((polynomial_traits & UpdatePixelTrait) == 0))
            continue;
          coefficient=(MagickRealType) terms[2*j];
          degree=(MagickRealType) terms[(j << 1)+1];
          polynomial_pixel[x].channel[i]+=coefficient*
            pow(QuantumScale*(double) GetPixelChannel(image,channel,p),degree);
        }
        p+=(ptrdiff_t) GetPixelChannels(next);
      }
      image_view=DestroyCacheView(image_view);
      next=GetNextImageInList(next);
    }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ClampToQuantum((double) QuantumRange*
          polynomial_pixel[x].channel[i]);
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(polynomial_view,exception) == MagickFalse)
      status=MagickFalse;
    if (images->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(images,PolynomialImageTag,progress,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  polynomial_view=DestroyCacheView(polynomial_view);
  polynomial_pixels=DestroyPixelTLS(images,polynomial_pixels);
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

static PixelList **DestroyPixelListTLS(PixelList **pixel_list)
{
  ssize_t
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

static PixelList **AcquirePixelListTLS(const size_t width,
  const size_t height)
{
  PixelList
    **pixel_list;

  ssize_t
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
      return(DestroyPixelListTLS(pixel_list));
  }
  return(pixel_list);
}

static void AddNodePixelList(PixelList *pixel_list,const size_t color)
{
  SkipList
    *p;

  ssize_t
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
  (void) memset(update,0,sizeof(update));
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

static inline void GetMedianPixelList(PixelList *pixel_list,Quantum *pixel)
{
  SkipList
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
    count+=(ssize_t) p->nodes[color].count;
  } while (count <= (ssize_t) (pixel_list->length >> 1));
  *pixel=ScaleShortToQuantum((unsigned short) color);
}

static inline void GetModePixelList(PixelList *pixel_list,Quantum *pixel)
{
  SkipList
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
    count+=(ssize_t) p->nodes[color].count;
  } while (count < (ssize_t) pixel_list->length);
  *pixel=ScaleShortToQuantum((unsigned short) mode);
}

static inline void GetNonpeakPixelList(PixelList *pixel_list,Quantum *pixel)
{
  SkipList
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
    count+=(ssize_t) p->nodes[color].count;
  } while (count <= (ssize_t) (pixel_list->length >> 1));
  if ((previous == 65536UL) && (next != 65536UL))
    color=next;
  else
    if ((previous != 65536UL) && (next == 65536UL))
      color=previous;
  *pixel=ScaleShortToQuantum((unsigned short) color);
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

  SkipNode
    *root;

  SkipList
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
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  statistic_image=CloneImage(image,0,0,MagickTrue,
    exception);
  if (statistic_image == (Image *) NULL)
    return((Image *) NULL);
  status=SetImageStorageClass(statistic_image,DirectClass,exception);
  if (status == MagickFalse)
    {
      statistic_image=DestroyImage(statistic_image);
      return((Image *) NULL);
    }
  pixel_list=AcquirePixelListTLS(MagickMax(width,1),MagickMax(height,1));
  if (pixel_list == (PixelList **) NULL)
    {
      statistic_image=DestroyImage(statistic_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Make each pixel the min / max / median / mode / etc. of the neighborhood.
  */
  center=(ssize_t) GetPixelChannels(image)*((ssize_t) image->columns+
    MagickMax((ssize_t) width,1L))*(MagickMax((ssize_t) height,1)/2L)+(ssize_t)
    GetPixelChannels(image)*(MagickMax((ssize_t) width,1L)/2L);
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

    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
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
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          area,
          maximum,
          minimum,
          sum,
          sum_squared;

        Quantum
          pixel;

        const Quantum
          *magick_restrict pixels;

        ssize_t
          u;

        ssize_t
          v;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait statistic_traits=GetPixelChannelTraits(statistic_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((statistic_traits & UpdatePixelTrait) == 0))
          continue;
        if (((statistic_traits & CopyPixelTrait) != 0) ||
            (GetPixelWriteMask(image,p) <= (QuantumRange/2)))
          {
            SetPixelChannel(statistic_image,channel,p[center+i],q);
            continue;
          }
        pixels=p;
        area=0.0;
        minimum=pixels[i];
        maximum=pixels[i];
        sum=0.0;
        sum_squared=0.0;
        ResetPixelList(pixel_list[id]);
        for (v=0; v < (ssize_t) MagickMax(height,1); v++)
        {
          for (u=0; u < (ssize_t) MagickMax(width,1); u++)
          {
            if ((type == MedianStatistic) || (type == ModeStatistic) ||
                (type == NonpeakStatistic))
              {
                InsertPixelList(pixels[i],pixel_list[id]);
                pixels+=(ptrdiff_t) GetPixelChannels(image);
                continue;
              }
            area++;
            if ((double) pixels[i] < minimum)
              minimum=(double) pixels[i];
            if ((double) pixels[i] > maximum)
              maximum=(double) pixels[i];
            sum+=(double) pixels[i];
            sum_squared+=(double) pixels[i]*(double) pixels[i];
            pixels+=(ptrdiff_t) GetPixelChannels(image);
          }
          pixels+=(ptrdiff_t) GetPixelChannels(image)*image->columns;
        }
        switch (type)
        {
          case ContrastStatistic:
          {
            pixel=ClampToQuantum(MagickAbsoluteValue((maximum-minimum)*
              MagickSafeReciprocal(maximum+minimum)));
            break;
          }
          case GradientStatistic:
          {
            pixel=ClampToQuantum(MagickAbsoluteValue(maximum-minimum));
            break;
          }
          case MaximumStatistic:
          {
            pixel=ClampToQuantum(maximum);
            break;
          }
          case MeanStatistic:
          default:
          {
            pixel=ClampToQuantum(sum/area);
            break;
          }
          case MedianStatistic:
          {
            GetMedianPixelList(pixel_list[id],&pixel);
            break;
          }
          case MinimumStatistic:
          {
            pixel=ClampToQuantum(minimum);
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
            pixel=ClampToQuantum(sqrt(sum_squared/area));
            break;
          }
          case StandardDeviationStatistic:
          {
            pixel=ClampToQuantum(sqrt(sum_squared/area-(sum/area*sum/area)));
            break;
          }
        }
        SetPixelChannel(statistic_image,channel,pixel,q);
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(statistic_image);
    }
    if (SyncCacheViewAuthenticPixels(statistic_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,StatisticImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  statistic_view=DestroyCacheView(statistic_view);
  image_view=DestroyCacheView(image_view);
  pixel_list=DestroyPixelListTLS(pixel_list);
  if (status == MagickFalse)
    statistic_image=DestroyImage(statistic_image);
  return(statistic_image);
}

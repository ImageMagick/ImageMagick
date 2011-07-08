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

static PixelInfo **DestroyPixelThreadSet(PixelInfo **pixels)
{
  register ssize_t
    i;

  assert(pixels != (PixelInfo **) NULL);
  for (i=0; i < (ssize_t) GetOpenMPMaximumThreads(); i++)
    if (pixels[i] != (PixelInfo *) NULL)
      pixels[i]=(PixelInfo *) RelinquishMagickMemory(pixels[i]);
  pixels=(PixelInfo **) RelinquishMagickMemory(pixels);
  return(pixels);
}

static PixelInfo **AcquirePixelThreadSet(const Image *image,
  const size_t number_images)
{
  register ssize_t
    i,
    j;

  PixelInfo
    **pixels;

  size_t
    length,
    number_threads;

  number_threads=GetOpenMPMaximumThreads();
  pixels=(PixelInfo **) AcquireQuantumMemory(number_threads,
    sizeof(*pixels));
  if (pixels == (PixelInfo **) NULL)
    return((PixelInfo **) NULL);
  (void) ResetMagickMemory(pixels,0,number_threads*sizeof(*pixels));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    length=image->columns;
    if (length < number_images)
      length=number_images;
    pixels[i]=(PixelInfo *) AcquireQuantumMemory(length,
      sizeof(**pixels));
    if (pixels[i] == (PixelInfo *) NULL)
      return(DestroyPixelThreadSet(pixels));
    for (j=0; j < (ssize_t) length; j++)
      GetPixelInfo(image,&pixels[i][j]);
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
  const PixelInfo
    *color_1,
    *color_2;

  int
    intensity;

  color_1=(const PixelInfo *) x;
  color_2=(const PixelInfo *) y;
  intensity=(int) GetPixelInfoIntensity(color_2)-(int)
    GetPixelInfoIntensity(color_1);
  return(intensity);
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
        positive result.  It differs from  % or fmod() which returns a
        'truncated modulus' result, where floor() is replaced by trunc()
        and could return a negative result (which is clipped).
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

  PixelInfo
    **restrict evaluate_pixels,
    zero;

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
  if (SetImageStorageClass(evaluate_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&evaluate_image->exception);
      evaluate_image=DestroyImage(evaluate_image);
      return((Image *) NULL);
    }
  number_images=GetImageListLength(images);
  evaluate_pixels=AcquirePixelThreadSet(images,number_images);
  if (evaluate_pixels == (PixelInfo **) NULL)
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
  GetPixelInfo(images,&zero);
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

      register PixelInfo
        *evaluate_pixel;

      register Quantum
        *restrict q;

      register ssize_t
        x;

      if (status == MagickFalse)
        continue;
      q=QueueCacheViewAuthenticPixels(evaluate_view,0,y,evaluate_image->columns,
        1,exception);
      if (q == (const Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      evaluate_pixel=evaluate_pixels[id];
      for (x=0; x < (ssize_t) evaluate_image->columns; x++)
      {
        register ssize_t
          i;

        for (i=0; i < (ssize_t) number_images; i++)
          evaluate_pixel[i]=zero;
        next=images;
        for (i=0; i < (ssize_t) number_images; i++)
        {
          register const Quantum
            *p;

          image_view=AcquireCacheView(next);
          p=GetCacheViewVirtualPixels(image_view,x,y,1,1,exception);
          if (p == (const Quantum *) NULL)
            {
              image_view=DestroyCacheView(image_view);
              break;
            }
          evaluate_pixel[i].red=ApplyEvaluateOperator(random_info[id],
            GetPixelRed(next,p),op,evaluate_pixel[i].red);
          evaluate_pixel[i].green=ApplyEvaluateOperator(random_info[id],
            GetPixelGreen(next,p),op,evaluate_pixel[i].green);
          evaluate_pixel[i].blue=ApplyEvaluateOperator(random_info[id],
            GetPixelBlue(next,p),op,evaluate_pixel[i].blue);
          if (evaluate_image->colorspace == CMYKColorspace)
            evaluate_pixel[i].black=ApplyEvaluateOperator(random_info[id],
              GetPixelBlack(next,p),op,evaluate_pixel[i].black);
          evaluate_pixel[i].alpha=ApplyEvaluateOperator(random_info[id],
            GetPixelAlpha(next,p),op,evaluate_pixel[i].alpha);
          image_view=DestroyCacheView(image_view);
          next=GetNextImageInList(next);
        }
        qsort((void *) evaluate_pixel,number_images,sizeof(*evaluate_pixel),
          IntensityCompare);
        SetPixelRed(evaluate_image,
          ClampToQuantum(evaluate_pixel[i/2].red),q);
        SetPixelGreen(evaluate_image,
          ClampToQuantum(evaluate_pixel[i/2].green),q);
        SetPixelBlue(evaluate_image,
          ClampToQuantum(evaluate_pixel[i/2].blue),q);
        if (evaluate_image->colorspace == CMYKColorspace)
          SetPixelBlack(evaluate_image,
          ClampToQuantum(evaluate_pixel[i/2].black),q);
        if (evaluate_image->matte == MagickFalse)
          SetPixelAlpha(evaluate_image,
            ClampToQuantum(evaluate_pixel[i/2].alpha),q);
        else
          SetPixelAlpha(evaluate_image,
            ClampToQuantum(evaluate_pixel[i/2].alpha),q);
        q+=GetPixelComponents(evaluate_image);
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

      register PixelInfo
        *evaluate_pixel;

      register Quantum
        *restrict q;

      if (status == MagickFalse)
        continue;
      q=QueueCacheViewAuthenticPixels(evaluate_view,0,y,evaluate_image->columns,
        1,exception);
      if (q == (const Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      evaluate_pixel=evaluate_pixels[id];
      for (x=0; x < (ssize_t) evaluate_image->columns; x++)
        evaluate_pixel[x]=zero;
      next=images;
      for (i=0; i < (ssize_t) number_images; i++)
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
          evaluate_pixel[x].red=ApplyEvaluateOperator(random_info[id],
            GetPixelRed(next,p),i == 0 ? AddEvaluateOperator : op,
            evaluate_pixel[x].red);
          evaluate_pixel[x].green=ApplyEvaluateOperator(random_info[id],
            GetPixelGreen(next,p),i == 0 ? AddEvaluateOperator : op,
              evaluate_pixel[x].green);
          evaluate_pixel[x].blue=ApplyEvaluateOperator(random_info[id],
            GetPixelBlue(next,p),i == 0 ? AddEvaluateOperator : op,
              evaluate_pixel[x].blue);
          if (evaluate_image->colorspace == CMYKColorspace)
            evaluate_pixel[x].black=ApplyEvaluateOperator(random_info[id],
              GetPixelBlack(next,p),i == 0 ? AddEvaluateOperator : op,
              evaluate_pixel[x].black);
          evaluate_pixel[x].alpha=ApplyEvaluateOperator(random_info[id],
            GetPixelAlpha(next,p),i == 0 ? AddEvaluateOperator : op,
            evaluate_pixel[x].alpha);
          p+=GetPixelComponents(next);
        }
        image_view=DestroyCacheView(image_view);
        next=GetNextImageInList(next);
      }
      if (op == MeanEvaluateOperator)
        for (x=0; x < (ssize_t) evaluate_image->columns; x++)
        {
          evaluate_pixel[x].red/=number_images;
          evaluate_pixel[x].green/=number_images;
          evaluate_pixel[x].blue/=number_images;
          evaluate_pixel[x].black/=number_images;
          evaluate_pixel[x].alpha/=number_images;
        }
      for (x=0; x < (ssize_t) evaluate_image->columns; x++)
      {
        SetPixelRed(evaluate_image,ClampToQuantum(evaluate_pixel[x].red),q);
        SetPixelGreen(evaluate_image,ClampToQuantum(evaluate_pixel[x].green),q);
        SetPixelBlue(evaluate_image,ClampToQuantum(evaluate_pixel[x].blue),q);
        if (evaluate_image->colorspace == CMYKColorspace)
          SetPixelBlack(evaluate_image,ClampToQuantum(evaluate_pixel[x].black),
            q);
        if (evaluate_image->matte == MagickFalse)
          SetPixelAlpha(evaluate_image,ClampToQuantum(evaluate_pixel[x].alpha),
            q);
        else
          SetPixelAlpha(evaluate_image,ClampToQuantum(evaluate_pixel[x].alpha),
            q);
        q+=GetPixelComponents(evaluate_image);
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
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&image->exception);
      return(MagickFalse);
    }
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
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        SetPixelRed(image,ClampToQuantum(ApplyEvaluateOperator(
          random_info[id],GetPixelRed(image,q),op,value)),q);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        SetPixelGreen(image,ClampToQuantum(ApplyEvaluateOperator(
          random_info[id],GetPixelGreen(image,q),op,value)),q);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        SetPixelBlue(image,ClampToQuantum(ApplyEvaluateOperator(
          random_info[id],GetPixelBlue(image,q),op,value)),q);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelBlack(image,ClampToQuantum(ApplyEvaluateOperator(
          random_info[id],GetPixelBlack(image,q),op,value)),q);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        {
          if (image->matte == MagickFalse)
            SetPixelAlpha(image,ClampToQuantum(ApplyEvaluateOperator(
              random_info[id],GetPixelAlpha(image,q),op,value)),q);
          else
            SetPixelAlpha(image,ClampToQuantum(ApplyEvaluateOperator(
              random_info[id],GetPixelAlpha(image,q),op,value)),q);
        }
      q+=GetPixelComponents(image);
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
       * Polynomial
       * Parameters:   polynomial constants,  highest to lowest order
       *   For example:      c0*x^3 + c1*x^2 + c2*x  + c3
       */
      result=0.0;
      for (i=0; i < (ssize_t) number_parameters; i++)
        result = result*QuantumScale*pixel + parameters[i];
      result *= QuantumRange;
      break;
    }
    case SinusoidFunction:
    {
      /* Sinusoid Function
       * Parameters:   Freq, Phase, Ampl, bias
       */
      double  freq,phase,ampl,bias;
      freq  = ( number_parameters >= 1 ) ? parameters[0] : 1.0;
      phase = ( number_parameters >= 2 ) ? parameters[1] : 0.0;
      ampl  = ( number_parameters >= 3 ) ? parameters[2] : 0.5;
      bias  = ( number_parameters >= 4 ) ? parameters[3] : 0.5;
      result=(MagickRealType) (QuantumRange*(ampl*sin((double) (2.0*MagickPI*
        (freq*QuantumScale*pixel + phase/360.0) )) + bias ) );
      break;
    }
    case ArcsinFunction:
    {
      /* Arcsin Function  (peged at range limits for invalid results)
       * Parameters:   Width, Center, Range, Bias
       */
      double  width,range,center,bias;
      width  = ( number_parameters >= 1 ) ? parameters[0] : 1.0;
      center = ( number_parameters >= 2 ) ? parameters[1] : 0.5;
      range  = ( number_parameters >= 3 ) ? parameters[2] : 1.0;
      bias   = ( number_parameters >= 4 ) ? parameters[3] : 0.5;
      result = 2.0/width*(QuantumScale*pixel - center);
      if ( result <= -1.0 )
        result = bias - range/2.0;
      else if ( result >= 1.0 )
        result = bias + range/2.0;
      else
        result=(MagickRealType) (range/MagickPI*asin((double) result)+bias);
      result *= QuantumRange;
      break;
    }
    case ArctanFunction:
    {
      /* Arctan Function
       * Parameters:   Slope, Center, Range, Bias
       */
      double  slope,range,center,bias;
      slope  = ( number_parameters >= 1 ) ? parameters[0] : 1.0;
      center = ( number_parameters >= 2 ) ? parameters[1] : 0.5;
      range  = ( number_parameters >= 3 ) ? parameters[2] : 1.0;
      bias   = ( number_parameters >= 4 ) ? parameters[3] : 0.5;
      result=(MagickRealType) (MagickPI*slope*(QuantumScale*pixel-center));
      result=(MagickRealType) (QuantumRange*(range/MagickPI*atan((double)
                  result) + bias ) );
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
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&image->exception);
      return(MagickFalse);
    }
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
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
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        SetPixelRed(image,ApplyFunction(GetPixelRed(image,q),function,
          number_parameters,parameters,exception),q);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        SetPixelGreen(image,ApplyFunction(GetPixelGreen(image,q),function,
          number_parameters,parameters,exception),q);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        SetPixelBlue(image,ApplyFunction(GetPixelBlue(image,q),function,
          number_parameters,parameters,exception),q);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelBlack(image,ApplyFunction(GetPixelBlack(image,q),function,
          number_parameters,parameters,exception),q);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        {
          if (image->matte == MagickFalse)
            SetPixelAlpha(image,ApplyFunction(GetPixelAlpha(image,q),function,
              number_parameters,parameters,exception),q);
          else
            SetPixelAlpha(image,ApplyFunction(GetPixelAlpha(image,q),function,
              number_parameters,parameters,exception),q);
        }
      q+=GetPixelComponents(image);
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

  size_t
    channels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=GetImageStatistics(image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    return(MagickFalse);
  channels=0;
  channel_statistics[CompositeChannels].mean=0.0;
  channel_statistics[CompositeChannels].standard_deviation=0.0;
  if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
    {
      channel_statistics[CompositeChannels].mean+=
        channel_statistics[RedChannel].mean;
      channel_statistics[CompositeChannels].standard_deviation+=
        channel_statistics[RedChannel].variance-
        channel_statistics[RedChannel].mean*
        channel_statistics[RedChannel].mean;
      channels++;
    }
  if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
    {
      channel_statistics[CompositeChannels].mean+=
        channel_statistics[GreenChannel].mean;
      channel_statistics[CompositeChannels].standard_deviation+=
        channel_statistics[GreenChannel].variance-
        channel_statistics[GreenChannel].mean*
        channel_statistics[GreenChannel].mean;
      channels++;
    }
  if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
    {
      channel_statistics[CompositeChannels].mean+=
        channel_statistics[BlueChannel].mean;
      channel_statistics[CompositeChannels].standard_deviation+=
        channel_statistics[BlueChannel].variance-
        channel_statistics[BlueChannel].mean*
        channel_statistics[BlueChannel].mean;
      channels++;
    }
  if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
      (image->colorspace == CMYKColorspace))
    {
      channel_statistics[CompositeChannels].mean+=
        channel_statistics[BlackChannel].mean;
      channel_statistics[CompositeChannels].standard_deviation+=
        channel_statistics[BlackChannel].variance-
        channel_statistics[BlackChannel].mean*
        channel_statistics[BlackChannel].mean;
      channels++;
    }
  if (((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0) &&
      (image->matte != MagickFalse))
    {
      channel_statistics[CompositeChannels].mean+=
        channel_statistics[AlphaChannel].mean;
      channel_statistics[CompositeChannels].standard_deviation+=
        channel_statistics[AlphaChannel].variance-
        channel_statistics[AlphaChannel].mean*
        channel_statistics[AlphaChannel].mean;
      channels++;
    }
  channel_statistics[CompositeChannels].mean/=channels;
  channel_statistics[CompositeChannels].standard_deviation=
    sqrt(channel_statistics[CompositeChannels].standard_deviation/channels);
  *mean=channel_statistics[CompositeChannels].mean;
  *standard_deviation=channel_statistics[CompositeChannels].standard_deviation;
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
  double
    area,
    mean,
    standard_deviation,
    sum_squares,
    sum_cubes,
    sum_fourth_power;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *kurtosis=0.0;
  *skewness=0.0;
  area=0.0;
  mean=0.0;
  standard_deviation=0.0;
  sum_squares=0.0;
  sum_cubes=0.0;
  sum_fourth_power=0.0;
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
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        {
          mean+=GetPixelRed(image,p);
          sum_squares+=(double) GetPixelRed(image,p)*GetPixelRed(image,p);
          sum_cubes+=(double) GetPixelRed(image,p)*GetPixelRed(image,p)*
            GetPixelRed(image,p);
          sum_fourth_power+=(double) GetPixelRed(image,p)*
            GetPixelRed(image,p)*GetPixelRed(image,p)*GetPixelRed(image,p);
          area++;
        }
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        {
          mean+=GetPixelGreen(image,p);
          sum_squares+=(double) GetPixelGreen(image,p)*GetPixelGreen(image,p);
          sum_cubes+=(double) GetPixelGreen(image,p)*GetPixelGreen(image,p)*
            GetPixelGreen(image,p);
          sum_fourth_power+=(double) GetPixelGreen(image,p)*
            GetPixelGreen(image,p)*GetPixelGreen(image,p)*
            GetPixelGreen(image,p);
          area++;
        }
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        {
          mean+=GetPixelBlue(image,p);
          sum_squares+=(double) GetPixelBlue(image,p)*GetPixelBlue(image,p);
          sum_cubes+=(double) GetPixelBlue(image,p)*GetPixelBlue(image,p)*
            GetPixelBlue(image,p);
          sum_fourth_power+=(double) GetPixelBlue(image,p)*
            GetPixelBlue(image,p)*GetPixelBlue(image,p)*
            GetPixelBlue(image,p);
          area++;
        }
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          mean+=GetPixelBlack(image,p);
          sum_squares+=(double) GetPixelBlack(image,p)*GetPixelBlack(image,p);
          sum_cubes+=(double) GetPixelBlack(image,p)*GetPixelBlack(image,p)*
            GetPixelBlack(image,p);
          sum_fourth_power+=(double) GetPixelBlack(image,p)*
            GetPixelBlack(image,p)*GetPixelBlack(image,p)*
            GetPixelBlack(image,p);
          area++;
        }
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        {
          mean+=GetPixelAlpha(image,p);
          sum_squares+=(double) GetPixelAlpha(image,p)*GetPixelAlpha(image,p);
          sum_cubes+=(double) GetPixelAlpha(image,p)*GetPixelAlpha(image,p)*
            GetPixelAlpha(image,p);
          sum_fourth_power+=(double) GetPixelAlpha(image,p)*
            GetPixelAlpha(image,p)*GetPixelAlpha(image,p)*
            GetPixelAlpha(image,p);
          area++;
        }
      p+=GetPixelComponents(image);
    }
  }
  if (y < (ssize_t) image->rows)
    return(MagickFalse);
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
  return(y == (ssize_t) image->rows ? MagickTrue : MagickFalse);
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
  PixelInfo
    pixel;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *maxima=(-1.0E-37);
  *minima=1.0E+37;
  GetPixelInfo(image,&pixel);
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
      SetPixelInfo(image,p,&pixel);
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        {
          if (pixel.red < *minima)
            *minima=(double) pixel.red;
          if (pixel.red > *maxima)
            *maxima=(double) pixel.red;
        }
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        {
          if (pixel.green < *minima)
            *minima=(double) pixel.green;
          if (pixel.green > *maxima)
            *maxima=(double) pixel.green;
        }
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        {
          if (pixel.blue < *minima)
            *minima=(double) pixel.blue;
          if (pixel.blue > *maxima)
            *maxima=(double) pixel.blue;
        }
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          if (pixel.black < *minima)
            *minima=(double) pixel.black;
          if (pixel.black > *maxima)
            *maxima=(double) pixel.black;
        }
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        {
          if (pixel.alpha < *minima)
            *minima=(double) pixel.alpha;
          if (pixel.alpha > *maxima)
            *maxima=(double) pixel.alpha;
        }
      p+=GetPixelComponents(image);
    }
  }
  return(y == (ssize_t) image->rows ? MagickTrue : MagickFalse);
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
%      red_mean=channel_statistics[RedChannel].mean;
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
    area;

  MagickStatusType
    status;

  QuantumAny
    range;

  register ssize_t
    i;

  size_t
    channels,
    depth,
    length;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  length=CompositeChannels+1UL;
  channel_statistics=(ChannelStatistics *) AcquireQuantumMemory(length,
    sizeof(*channel_statistics));
  if (channel_statistics == (ChannelStatistics *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(channel_statistics,0,length*
    sizeof(*channel_statistics));
  for (i=0; i <= (ssize_t) CompositeChannels; i++)
  {
    channel_statistics[i].depth=1;
    channel_statistics[i].maxima=(-1.0E-37);
    channel_statistics[i].minima=1.0E+37;
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
    for (x=0; x < (ssize_t) image->columns; )
    {
      if (channel_statistics[RedChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[RedChannel].depth;
          range=GetQuantumRange(depth);
          status=GetPixelRed(image,p) != ScaleAnyToQuantum(ScaleQuantumToAny(
            GetPixelRed(image,p),range),range) ? MagickTrue : MagickFalse;
          if (status != MagickFalse)
            {
              channel_statistics[RedChannel].depth++;
              continue;
            }
        }
      if (channel_statistics[GreenChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[GreenChannel].depth;
          range=GetQuantumRange(depth);
          status=GetPixelGreen(image,p) != ScaleAnyToQuantum(ScaleQuantumToAny(
            GetPixelGreen(image,p),range),range) ? MagickTrue : MagickFalse;
          if (status != MagickFalse)
            {
              channel_statistics[GreenChannel].depth++;
              continue;
            }
        }
      if (channel_statistics[BlueChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[BlueChannel].depth;
          range=GetQuantumRange(depth);
          status=GetPixelBlue(image,p) != ScaleAnyToQuantum(ScaleQuantumToAny(
            GetPixelBlue(image,p),range),range) ? MagickTrue : MagickFalse;
          if (status != MagickFalse)
            {
              channel_statistics[BlueChannel].depth++;
              continue;
            }
        }
      if (image->matte != MagickFalse)
        {
          if (channel_statistics[AlphaChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
            {
              depth=channel_statistics[AlphaChannel].depth;
              range=GetQuantumRange(depth);
              status=GetPixelAlpha(image,p) != ScaleAnyToQuantum(
                ScaleQuantumToAny(GetPixelAlpha(image,p),range),range) ?
                MagickTrue : MagickFalse;
              if (status != MagickFalse)
                {
                  channel_statistics[AlphaChannel].depth++;
                  continue;
                }
            }
          }
      if (image->colorspace == CMYKColorspace)
        {
          if (channel_statistics[BlackChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
            {
              depth=channel_statistics[BlackChannel].depth;
              range=GetQuantumRange(depth);
              status=GetPixelBlack(image,p) != ScaleAnyToQuantum(
                ScaleQuantumToAny(GetPixelBlack(image,p),range),range) ?
                MagickTrue : MagickFalse;
              if (status != MagickFalse)
                {
                  channel_statistics[BlackChannel].depth++;
                  continue;
                }
            }
        }
      if ((double) GetPixelRed(image,p) < channel_statistics[RedChannel].minima)
        channel_statistics[RedChannel].minima=(double) GetPixelRed(image,p);
      if ((double) GetPixelRed(image,p) > channel_statistics[RedChannel].maxima)
        channel_statistics[RedChannel].maxima=(double) GetPixelRed(image,p);
      channel_statistics[RedChannel].sum+=GetPixelRed(image,p);
      channel_statistics[RedChannel].sum_squared+=(double)
        GetPixelRed(image,p)*GetPixelRed(image,p);
      channel_statistics[RedChannel].sum_cubed+=(double)
        GetPixelRed(image,p)*GetPixelRed(image,p)*GetPixelRed(image,p);
      channel_statistics[RedChannel].sum_fourth_power+=(double)
        GetPixelRed(image,p)*GetPixelRed(image,p)*GetPixelRed(image,p)*
        GetPixelRed(image,p);
      if ((double) GetPixelGreen(image,p) < channel_statistics[GreenChannel].minima)
        channel_statistics[GreenChannel].minima=(double)
          GetPixelGreen(image,p);
      if ((double) GetPixelGreen(image,p) > channel_statistics[GreenChannel].maxima)
        channel_statistics[GreenChannel].maxima=(double) GetPixelGreen(image,p);
      channel_statistics[GreenChannel].sum+=GetPixelGreen(image,p);
      channel_statistics[GreenChannel].sum_squared+=(double)
        GetPixelGreen(image,p)*GetPixelGreen(image,p);
      channel_statistics[GreenChannel].sum_cubed+=(double)
        GetPixelGreen(image,p)*GetPixelGreen(image,p)*GetPixelGreen(image,p);
      channel_statistics[GreenChannel].sum_fourth_power+=(double)
        GetPixelGreen(image,p)*GetPixelGreen(image,p)*GetPixelGreen(image,p)*
        GetPixelGreen(image,p);
      if ((double) GetPixelBlue(image,p) < channel_statistics[BlueChannel].minima)
        channel_statistics[BlueChannel].minima=(double) GetPixelBlue(image,p);
      if ((double) GetPixelBlue(image,p) > channel_statistics[BlueChannel].maxima)
        channel_statistics[BlueChannel].maxima=(double) GetPixelBlue(image,p);
      channel_statistics[BlueChannel].sum+=GetPixelBlue(image,p);
      channel_statistics[BlueChannel].sum_squared+=(double)
        GetPixelBlue(image,p)*GetPixelBlue(image,p);
      channel_statistics[BlueChannel].sum_cubed+=(double)
        GetPixelBlue(image,p)*GetPixelBlue(image,p)*GetPixelBlue(image,p);
      channel_statistics[BlueChannel].sum_fourth_power+=(double)
        GetPixelBlue(image,p)*GetPixelBlue(image,p)*GetPixelBlue(image,p)*
        GetPixelBlue(image,p);
      if (image->colorspace == CMYKColorspace)
        {
          if ((double) GetPixelBlack(image,p) < channel_statistics[BlackChannel].minima)
            channel_statistics[BlackChannel].minima=(double)
              GetPixelBlack(image,p);
          if ((double) GetPixelBlack(image,p) > channel_statistics[BlackChannel].maxima)
            channel_statistics[BlackChannel].maxima=(double)
              GetPixelBlack(image,p);
          channel_statistics[BlackChannel].sum+=GetPixelBlack(image,p);
          channel_statistics[BlackChannel].sum_squared+=(double)
            GetPixelBlack(image,p)*GetPixelBlack(image,p);
          channel_statistics[BlackChannel].sum_cubed+=(double)
            GetPixelBlack(image,p)*GetPixelBlack(image,p)*
            GetPixelBlack(image,p);
          channel_statistics[BlackChannel].sum_fourth_power+=(double)
            GetPixelBlack(image,p)*GetPixelBlack(image,p)*
            GetPixelBlack(image,p)*GetPixelBlack(image,p);
        }
      if (image->matte != MagickFalse)
        {
          if ((double) GetPixelAlpha(image,p) < channel_statistics[AlphaChannel].minima)
            channel_statistics[AlphaChannel].minima=(double)
              GetPixelAlpha(image,p);
          if ((double) GetPixelAlpha(image,p) > channel_statistics[AlphaChannel].maxima)
            channel_statistics[AlphaChannel].maxima=(double)
              GetPixelAlpha(image,p);
          channel_statistics[AlphaChannel].sum+=GetPixelAlpha(image,p);
          channel_statistics[AlphaChannel].sum_squared+=(double)
            GetPixelAlpha(image,p)*GetPixelAlpha(image,p);
          channel_statistics[AlphaChannel].sum_cubed+=(double)
            GetPixelAlpha(image,p)*GetPixelAlpha(image,p)*
            GetPixelAlpha(image,p);
          channel_statistics[AlphaChannel].sum_fourth_power+=(double)
            GetPixelAlpha(image,p)*GetPixelAlpha(image,p)*
            GetPixelAlpha(image,p)*GetPixelAlpha(image,p);
        }
      x++;
      p+=GetPixelComponents(image);
    }
  }
  area=(double) image->columns*image->rows;
  for (i=0; i < (ssize_t) CompositeChannels; i++)
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
  for (i=0; i < (ssize_t) CompositeChannels; i++)
  {
    channel_statistics[CompositeChannels].depth=(size_t) MagickMax((double)
      channel_statistics[CompositeChannels].depth,(double)
      channel_statistics[i].depth);
    channel_statistics[CompositeChannels].minima=MagickMin(
      channel_statistics[CompositeChannels].minima,
      channel_statistics[i].minima);
    channel_statistics[CompositeChannels].maxima=MagickMax(
      channel_statistics[CompositeChannels].maxima,
      channel_statistics[i].maxima);
    channel_statistics[CompositeChannels].sum+=channel_statistics[i].sum;
    channel_statistics[CompositeChannels].sum_squared+=
      channel_statistics[i].sum_squared;
    channel_statistics[CompositeChannels].sum_cubed+=
      channel_statistics[i].sum_cubed;
    channel_statistics[CompositeChannels].sum_fourth_power+=
      channel_statistics[i].sum_fourth_power;
    channel_statistics[CompositeChannels].mean+=channel_statistics[i].mean;
    channel_statistics[CompositeChannels].variance+=
      channel_statistics[i].variance-channel_statistics[i].mean*
      channel_statistics[i].mean;
    channel_statistics[CompositeChannels].standard_deviation+=
      channel_statistics[i].variance-channel_statistics[i].mean*
      channel_statistics[i].mean;
  }
  channels=3;
  if (image->matte != MagickFalse)
    channels++;
  if (image->colorspace == CMYKColorspace)
    channels++;
  channel_statistics[CompositeChannels].sum/=channels;
  channel_statistics[CompositeChannels].sum_squared/=channels;
  channel_statistics[CompositeChannels].sum_cubed/=channels;
  channel_statistics[CompositeChannels].sum_fourth_power/=channels;
  channel_statistics[CompositeChannels].mean/=channels;
  channel_statistics[CompositeChannels].variance/=channels;
  channel_statistics[CompositeChannels].standard_deviation=
    sqrt(channel_statistics[CompositeChannels].standard_deviation/channels);
  channel_statistics[CompositeChannels].kurtosis/=channels;
  channel_statistics[CompositeChannels].skewness/=channels;
  for (i=0; i <= (ssize_t) CompositeChannels; i++)
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

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
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/animate.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/cache-private.h"
#include "magick/cache-view.h"
#include "magick/client.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/composite-private.h"
#include "magick/compress.h"
#include "magick/constitute.h"
#include "magick/deprecate.h"
#include "magick/display.h"
#include "magick/draw.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/list.h"
#include "magick/image-private.h"
#include "magick/magic.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/paint.h"
#include "magick/pixel-private.h"
#include "magick/profile.h"
#include "magick/quantize.h"
#include "magick/random_.h"
#include "magick/random-private.h"
#include "magick/segment.h"
#include "magick/semaphore.h"
#include "magick/signature-private.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/thread-private.h"
#include "magick/timer.h"
#include "magick/utility.h"
#include "magick/version.h"

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
%  The format of the EvaluateImageChannel method is:
%
%      MagickBooleanType EvaluateImage(Image *image,
%        const MagickEvaluateOperator op,const double value,
%        ExceptionInfo *exception)
%      MagickBooleanType EvaluateImages(Image *images,
%        const MagickEvaluateOperator op,const double value,
%        ExceptionInfo *exception)
%      MagickBooleanType EvaluateImageChannel(Image *image,
%        const ChannelType channel,const MagickEvaluateOperator op,
%        const double value,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o op: A channel op.
%
%    o value: A value value.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickPixelPacket **DestroyPixelThreadSet(MagickPixelPacket **pixels)
{
  register long
    i;

  assert(pixels != (MagickPixelPacket **) NULL);
  for (i=0; i < (long) GetOpenMPMaximumThreads(); i++)
    if (pixels[i] != (MagickPixelPacket *) NULL)
      pixels[i]=(MagickPixelPacket *) RelinquishMagickMemory(pixels[i]);
  pixels=(MagickPixelPacket **) RelinquishAlignedMemory(pixels);
  return(pixels);
}

static MagickPixelPacket **AcquirePixelThreadSet(const Image *image)
{
  register long
    i,
    j;

  MagickPixelPacket
    **pixels;

  unsigned long
    number_threads;

  number_threads=GetOpenMPMaximumThreads();
  pixels=(MagickPixelPacket **) AcquireAlignedMemory(number_threads,
    sizeof(*pixels));
  if (pixels == (MagickPixelPacket **) NULL)
    return((MagickPixelPacket **) NULL);
  (void) ResetMagickMemory(pixels,0,number_threads*sizeof(*pixels));
  for (i=0; i < (long) number_threads; i++)
  {
    pixels[i]=(MagickPixelPacket *) AcquireQuantumMemory(image->columns,
      sizeof(**pixels));
    if (pixels[i] == (MagickPixelPacket *) NULL)
      return(DestroyPixelThreadSet(pixels));
    for (j=0; j < (long) image->columns; j++)
      GetMagickPixelPacket(image,&pixels[i][j]);
  }
  return(pixels);
}

static inline double MagickMax(const double x,const double y)
{
  if (x > y)
    return(x);
  return(y);
}

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
      result-=(QuantumRange+1)*floor(result/(QuantumRange+1));
      break;
    }
    case AndEvaluateOperator:
    {
      result=(MagickRealType) ((unsigned long) pixel & (unsigned long)
        (value+0.5));
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
      result=(MagickRealType) ((unsigned long) pixel << (unsigned long)
        (value+0.5));
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
      result=(MagickRealType) ((pixel+value)/2.0);
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
      result=(MagickRealType) ((unsigned long) pixel | (unsigned long)
        (value+0.5));
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
      result=(MagickRealType) ((unsigned long) pixel >> (unsigned long)
        (value+0.5));
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
      result=(MagickRealType) ((unsigned long) pixel ^ (unsigned long)
        (value+0.5));
      break;
    }
  }
  return(result);
}

MagickExport MagickBooleanType EvaluateImage(Image *image,
  const MagickEvaluateOperator op,const double value,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=EvaluateImageChannel(image,AllChannels,op,value,exception);
  return(status);
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

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    **restrict evaluate_pixels,
    zero;

  RandomInfo
    **restrict random_info;

  unsigned long
    number_images;

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
  evaluate_pixels=AcquirePixelThreadSet(images);
  if (evaluate_pixels == (MagickPixelPacket **) NULL)
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
  GetMagickPixelPacket(images,&zero);
  random_info=AcquireRandomInfoThreadSet();
  number_images=GetImageListLength(images);
  evaluate_view=AcquireCacheView(evaluate_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic) shared(progress,status)
#endif
  for (y=0; y < (long) evaluate_image->rows; y++)
  {
    CacheView
      *image_view;

    const Image
      *next;

    MagickPixelPacket
      pixel;

    register IndexPacket
      *restrict evaluate_indexes;

    register long
      i,
      id,
      x;

    register MagickPixelPacket
      *evaluate_pixel;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(evaluate_view,0,y,evaluate_image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    evaluate_indexes=GetCacheViewAuthenticIndexQueue(evaluate_view);
    pixel=zero;
    id=GetOpenMPThreadId();
    evaluate_pixel=evaluate_pixels[id];
    for (x=0; x < (long) evaluate_image->columns; x++)
      evaluate_pixel[x]=zero;
    next=images;
    for (i=0; i < (long) number_images; i++)
    {
      register const IndexPacket
        *indexes;

      register const PixelPacket
        *p;

      image_view=AcquireCacheView(next);
      p=GetCacheViewVirtualPixels(image_view,0,y,next->columns,1,exception);
      if (p == (const PixelPacket *) NULL)
        {
          image_view=DestroyCacheView(image_view);
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      for (x=0; x < (long) next->columns; x++)
      {
        evaluate_pixel[x].red=ApplyEvaluateOperator(random_info[id],p->red,
          i == 0 ? AddEvaluateOperator : op,evaluate_pixel[x].red);
        evaluate_pixel[x].green=ApplyEvaluateOperator(random_info[id],p->green,
          i == 0 ? AddEvaluateOperator : op,evaluate_pixel[x].green);
        evaluate_pixel[x].blue=ApplyEvaluateOperator(random_info[id],p->blue,
          i == 0 ? AddEvaluateOperator : op,evaluate_pixel[x].blue);
        evaluate_pixel[x].opacity=ApplyEvaluateOperator(random_info[id],
          p->opacity,i == 0 ? AddEvaluateOperator : op,
          evaluate_pixel[x].opacity);
        if (evaluate_image->colorspace == CMYKColorspace)
          evaluate_pixel[x].index=ApplyEvaluateOperator(random_info[id],
            indexes[x],i == 0 ? AddEvaluateOperator : op,
            evaluate_pixel[x].index);
        p++;
      }
      image_view=DestroyCacheView(image_view);
      next=GetNextImageInList(next);
    }
    for (x=0; x < (long) evaluate_image->columns; x++)
    {
      q->red=ClampToQuantum(evaluate_pixel[x].red);
      q->green=ClampToQuantum(evaluate_pixel[x].green);
      q->blue=ClampToQuantum(evaluate_pixel[x].blue);
      if (evaluate_image->matte == MagickFalse)
        q->opacity=ClampToQuantum(evaluate_pixel[x].opacity);
      else
        q->opacity=ClampToQuantum(QuantumRange-evaluate_pixel[x].opacity);
      if (evaluate_image->colorspace == CMYKColorspace)
        evaluate_indexes[x]=ClampToQuantum(evaluate_pixel[x].index);
      q++;
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

MagickExport MagickBooleanType EvaluateImageChannel(Image *image,
  const ChannelType channel,const MagickEvaluateOperator op,const double value,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  long
    progress,
    y;

  MagickBooleanType
    status;

  RandomInfo
    **restrict random_info;

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
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register long
      id,
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    id=GetOpenMPThreadId();
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        q->red=ClampToQuantum(ApplyEvaluateOperator(random_info[id],q->red,op,
          value));
      if ((channel & GreenChannel) != 0)
        q->green=ClampToQuantum(ApplyEvaluateOperator(random_info[id],q->green,
          op,value));
      if ((channel & BlueChannel) != 0)
        q->blue=ClampToQuantum(ApplyEvaluateOperator(random_info[id],q->blue,op,
          value));
      if ((channel & OpacityChannel) != 0)
        {
          if (image->matte == MagickFalse)
            q->opacity=ClampToQuantum(ApplyEvaluateOperator(random_info[id],
              q->opacity,op,value));
          else
            q->opacity=ClampToQuantum(QuantumRange-ApplyEvaluateOperator(
              random_info[id],(Quantum) GetAlphaPixelComponent(q),op,value));
        }
      if (((channel & IndexChannel) != 0) && (indexes != (IndexPacket *) NULL))
        indexes[x]=(IndexPacket) ClampToQuantum(ApplyEvaluateOperator(
          random_info[id],indexes[x],op,value));
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_EvaluateImageChannel)
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
%  The format of the FunctionImageChannel method is:
%
%      MagickBooleanType FunctionImage(Image *image,
%        const MagickFunction function,const long number_parameters,
%        const double *parameters,ExceptionInfo *exception)
%      MagickBooleanType FunctionImageChannel(Image *image,
%        const ChannelType channel,const MagickFunction function,
%        const long number_parameters,const double *argument,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o function: A channel function.
%
%    o parameters: one or more parameters.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static Quantum ApplyFunction(Quantum pixel,const MagickFunction function,
  const unsigned long number_parameters,const double *parameters,
  ExceptionInfo *exception)
{
  MagickRealType
    result;

  register long
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
      for (i=0; i < (long) number_parameters; i++)
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
        result=range/MagickPI*asin((double)result) + bias;
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
      result = MagickPI*slope*(QuantumScale*pixel - center);
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
  const MagickFunction function,const unsigned long number_parameters,
  const double *parameters,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=FunctionImageChannel(image,AllChannels,function,number_parameters,
    parameters,exception);
  return(status);
}

MagickExport MagickBooleanType FunctionImageChannel(Image *image,
  const ChannelType channel,const MagickFunction function,
  const unsigned long number_parameters,const double *parameters,
  ExceptionInfo *exception)
{
#define FunctionImageTag  "Function/Image "

  CacheView
    *image_view;

  long
    progress,
    y;

  MagickBooleanType
    status;

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
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        q->red=ApplyFunction(q->red,function,number_parameters,parameters,
          exception);
      if ((channel & GreenChannel) != 0)
        q->green=ApplyFunction(q->green,function,number_parameters,parameters,
          exception);
      if ((channel & BlueChannel) != 0)
        q->blue=ApplyFunction(q->blue,function,number_parameters,parameters,
          exception);
      if ((channel & OpacityChannel) != 0)
        {
          if (image->matte == MagickFalse)
            q->opacity=ApplyFunction(q->opacity,function,number_parameters,
              parameters,exception);
          else
            q->opacity=(Quantum) QuantumRange-ApplyFunction((Quantum)
              GetAlphaPixelComponent(q),function,number_parameters,parameters,
              exception);
        }
      if (((channel & IndexChannel) != 0) && (indexes != (IndexPacket *) NULL))
        indexes[x]=(IndexPacket) ApplyFunction(indexes[x],function,
          number_parameters,parameters,exception);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_FunctionImageChannel)
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
+   G e t I m a g e C h a n n e l E x t r e m a                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelExtrema() returns the extrema of one or more image channels.
%
%  The format of the GetImageChannelExtrema method is:
%
%      MagickBooleanType GetImageChannelExtrema(const Image *image,
%        const ChannelType channel,unsigned long *minima,unsigned long *maxima,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o minima: the minimum value in the channel.
%
%    o maxima: the maximum value in the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageExtrema(const Image *image,
  unsigned long *minima,unsigned long *maxima,ExceptionInfo *exception)
{
  return(GetImageChannelExtrema(image,AllChannels,minima,maxima,exception));
}

MagickExport MagickBooleanType GetImageChannelExtrema(const Image *image,
  const ChannelType channel,unsigned long *minima,unsigned long *maxima,
  ExceptionInfo *exception)
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
  status=GetImageChannelRange(image,channel,&min,&max,exception);
  *minima=(unsigned long) (min+0.5);
  *maxima=(unsigned long) (max+0.5);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l M e a n                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelMean() returns the mean and standard deviation of one or more
%  image channels.
%
%  The format of the GetImageChannelMean method is:
%
%      MagickBooleanType GetImageChannelMean(const Image *image,
%        const ChannelType channel,double *mean,double *standard_deviation,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
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
  MagickBooleanType
    status;

  status=GetImageChannelMean(image,AllChannels,mean,standard_deviation,
    exception);
  return(status);
}

MagickExport MagickBooleanType GetImageChannelMean(const Image *image,
  const ChannelType channel,double *mean,double *standard_deviation,
  ExceptionInfo *exception)
{
  double
    area;

  long
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *mean=0.0;
  *standard_deviation=0.0;
  area=0.0;
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register long
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        {
          *mean+=GetRedPixelComponent(p);
          *standard_deviation+=(double) p->red*GetRedPixelComponent(p);
          area++;
        }
      if ((channel & GreenChannel) != 0)
        {
          *mean+=GetGreenPixelComponent(p);
          *standard_deviation+=(double) p->green*GetGreenPixelComponent(p);
          area++;
        }
      if ((channel & BlueChannel) != 0)
        {
          *mean+=GetBluePixelComponent(p);
          *standard_deviation+=(double) p->blue*GetBluePixelComponent(p);
          area++;
        }
      if ((channel & OpacityChannel) != 0)
        {
          *mean+=GetOpacityPixelComponent(p);
          *standard_deviation+=(double) p->opacity*GetOpacityPixelComponent(p);
          area++;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          *mean+=indexes[x];
          *standard_deviation+=(double) indexes[x]*indexes[x];
          area++;
        }
      p++;
    }
  }
  if (y < (long) image->rows)
    return(MagickFalse);
  if (area != 0)
    {
      *mean/=area;
      *standard_deviation/=area;
    }
  *standard_deviation=sqrt(*standard_deviation-(*mean*(*mean)));
  return(y == (long) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l K u r t o s i s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelKurtosis() returns the kurtosis and skewness of one or more
%  image channels.
%
%  The format of the GetImageChannelKurtosis method is:
%
%      MagickBooleanType GetImageChannelKurtosis(const Image *image,
%        const ChannelType channel,double *kurtosis,double *skewness,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
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
  MagickBooleanType
    status;

  status=GetImageChannelKurtosis(image,AllChannels,kurtosis,skewness,
    exception);
  return(status);
}

MagickExport MagickBooleanType GetImageChannelKurtosis(const Image *image,
  const ChannelType channel,double *kurtosis,double *skewness,
  ExceptionInfo *exception)
{
  double
    area,
    mean,
    standard_deviation,
    sum_squares,
    sum_cubes,
    sum_fourth_power;

  long
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
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register long
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        {
          mean+=GetRedPixelComponent(p);
          sum_squares+=(double) p->red*GetRedPixelComponent(p);
          sum_cubes+=(double) p->red*p->red*GetRedPixelComponent(p);
          sum_fourth_power+=(double) p->red*p->red*p->red*
            GetRedPixelComponent(p);
          area++;
        }
      if ((channel & GreenChannel) != 0)
        {
          mean+=GetGreenPixelComponent(p);
          sum_squares+=(double) p->green*GetGreenPixelComponent(p);
          sum_cubes+=(double) p->green*p->green*GetGreenPixelComponent(p);
          sum_fourth_power+=(double) p->green*p->green*p->green*
            GetGreenPixelComponent(p);
          area++;
        }
      if ((channel & BlueChannel) != 0)
        {
          mean+=GetBluePixelComponent(p);
          sum_squares+=(double) p->blue*GetBluePixelComponent(p);
          sum_cubes+=(double) p->blue*p->blue*GetBluePixelComponent(p);
          sum_fourth_power+=(double) p->blue*p->blue*p->blue*
            GetBluePixelComponent(p);
          area++;
        }
      if ((channel & OpacityChannel) != 0)
        {
          mean+=GetOpacityPixelComponent(p);
          sum_squares+=(double) p->opacity*GetOpacityPixelComponent(p);
          sum_cubes+=(double) p->opacity*p->opacity*GetOpacityPixelComponent(p);
          sum_fourth_power+=(double) p->opacity*p->opacity*p->opacity*
            GetOpacityPixelComponent(p);
          area++;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          mean+=indexes[x];
          sum_squares+=(double) indexes[x]*indexes[x];
          sum_cubes+=(double) indexes[x]*indexes[x]*indexes[x];
          sum_fourth_power+=(double) indexes[x]*indexes[x]*indexes[x]*
            indexes[x];
          area++;
        }
      p++;
    }
  }
  if (y < (long) image->rows)
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
  return(y == (long) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l R a n g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelRange() returns the range of one or more image channels.
%
%  The format of the GetImageChannelRange method is:
%
%      MagickBooleanType GetImageChannelRange(const Image *image,
%        const ChannelType channel,double *minima,double *maxima,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o minima: the minimum value in the channel.
%
%    o maxima: the maximum value in the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageRange(const Image *image,
  double *minima,double *maxima,ExceptionInfo *exception)
{
  return(GetImageChannelRange(image,AllChannels,minima,maxima,exception));
}

MagickExport MagickBooleanType GetImageChannelRange(const Image *image,
  const ChannelType channel,double *minima,double *maxima,
  ExceptionInfo *exception)
{
  long
    y;

  MagickPixelPacket
    pixel;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *maxima=(-1.0E-37);
  *minima=1.0E+37;
  GetMagickPixelPacket(image,&pixel);
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register long
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (long) image->columns; x++)
    {
      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      if ((channel & RedChannel) != 0)
        {
          if (pixel.red < *minima)
            *minima=(double) pixel.red;
          if (pixel.red > *maxima)
            *maxima=(double) pixel.red;
        }
      if ((channel & GreenChannel) != 0)
        {
          if (pixel.green < *minima)
            *minima=(double) pixel.green;
          if (pixel.green > *maxima)
            *maxima=(double) pixel.green;
        }
      if ((channel & BlueChannel) != 0)
        {
          if (pixel.blue < *minima)
            *minima=(double) pixel.blue;
          if (pixel.blue > *maxima)
            *maxima=(double) pixel.blue;
        }
      if ((channel & OpacityChannel) != 0)
        {
          if (pixel.opacity < *minima)
            *minima=(double) pixel.opacity;
          if (pixel.opacity > *maxima)
            *maxima=(double) pixel.opacity;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          if ((double) indexes[x] < *minima)
            *minima=(double) indexes[x];
          if ((double) indexes[x] > *maxima)
            *maxima=(double) indexes[x];
        }
      p++;
    }
  }
  return(y == (long) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l S t a t i s t i c s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelStatistics() returns statistics for each channel in the
%  image.  The statistics include the channel depth, its minima, maxima, mean,
%  standard deviation, kurtosis and skewness.  You can access the red channel
%  mean, for example, like this:
%
%      channel_statistics=GetImageChannelStatistics(image,excepton);
%      red_mean=channel_statistics[RedChannel].mean;
%
%  Use MagickRelinquishMemory() to free the statistics buffer.
%
%  The format of the GetImageChannelStatistics method is:
%
%      ChannelStatistics *GetImageChannelStatistics(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ChannelStatistics *GetImageChannelStatistics(const Image *image,
  ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  double
    area,
    sum_squares,
    sum_cubes;

  long
    y;

  MagickStatusType
    status;

  QuantumAny
    range;

  register long
    i;

  size_t
    length;

  unsigned long
    channels,
    depth;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  length=AllChannels+1UL;
  channel_statistics=(ChannelStatistics *) AcquireQuantumMemory(length,
    sizeof(*channel_statistics));
  if (channel_statistics == (ChannelStatistics *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(channel_statistics,0,length*
    sizeof(*channel_statistics));
  for (i=0; i <= AllChannels; i++)
  {
    channel_statistics[i].depth=1;
    channel_statistics[i].maxima=(-1.0E-37);
    channel_statistics[i].minima=1.0E+37;
    channel_statistics[i].mean=0.0;
    channel_statistics[i].standard_deviation=0.0;
    channel_statistics[i].kurtosis=0.0;
    channel_statistics[i].skewness=0.0;
  }
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register long
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (long) image->columns; )
    {
      if (channel_statistics[RedChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[RedChannel].depth;
          range=GetQuantumRange(depth);
          status=p->red != ScaleAnyToQuantum(ScaleQuantumToAny(p->red,range),
            range) ? MagickTrue : MagickFalse;
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
          status=p->green != ScaleAnyToQuantum(ScaleQuantumToAny(p->green,
            range),range) ? MagickTrue : MagickFalse;
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
          status=p->blue != ScaleAnyToQuantum(ScaleQuantumToAny(p->blue,
            range),range) ? MagickTrue : MagickFalse;
          if (status != MagickFalse)
            {
              channel_statistics[BlueChannel].depth++;
              continue;
            }
        }
      if (image->matte != MagickFalse)
        {
          if (channel_statistics[OpacityChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
            {
              depth=channel_statistics[OpacityChannel].depth;
              range=GetQuantumRange(depth);
              status=p->opacity != ScaleAnyToQuantum(ScaleQuantumToAny(
                p->opacity,range),range) ? MagickTrue : MagickFalse;
              if (status != MagickFalse)
                {
                  channel_statistics[OpacityChannel].depth++;
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
              status=indexes[x] != ScaleAnyToQuantum(ScaleQuantumToAny(
                indexes[x],range),range) ? MagickTrue : MagickFalse;
              if (status != MagickFalse)
                {
                  channel_statistics[BlackChannel].depth++;
                  continue;
                }
            }
        }
      if ((double) p->red < channel_statistics[RedChannel].minima)
        channel_statistics[RedChannel].minima=(double) GetRedPixelComponent(p);
      if ((double) p->red > channel_statistics[RedChannel].maxima)
        channel_statistics[RedChannel].maxima=(double) GetRedPixelComponent(p);
      channel_statistics[RedChannel].mean+=GetRedPixelComponent(p);
      channel_statistics[RedChannel].standard_deviation+=(double) p->red*
        GetRedPixelComponent(p);
      channel_statistics[RedChannel].kurtosis+=(double) p->red*p->red*
        p->red*GetRedPixelComponent(p);
      channel_statistics[RedChannel].skewness+=(double) p->red*p->red*
        GetRedPixelComponent(p);
      if ((double) p->green < channel_statistics[GreenChannel].minima)
        channel_statistics[GreenChannel].minima=(double)
          GetGreenPixelComponent(p);
      if ((double) p->green > channel_statistics[GreenChannel].maxima)
        channel_statistics[GreenChannel].maxima=(double)
          GetGreenPixelComponent(p);
      channel_statistics[GreenChannel].mean+=GetGreenPixelComponent(p);
      channel_statistics[GreenChannel].standard_deviation+=(double) p->green*
        GetGreenPixelComponent(p);
      channel_statistics[GreenChannel].kurtosis+=(double) p->green*p->green*
        p->green*GetGreenPixelComponent(p);
      channel_statistics[GreenChannel].skewness+=(double) p->green*p->green*
        GetGreenPixelComponent(p);
      if ((double) p->blue < channel_statistics[BlueChannel].minima)
        channel_statistics[BlueChannel].minima=(double)
          GetBluePixelComponent(p);
      if ((double) p->blue > channel_statistics[BlueChannel].maxima)
        channel_statistics[BlueChannel].maxima=(double)
          GetBluePixelComponent(p);
      channel_statistics[BlueChannel].mean+=GetBluePixelComponent(p);
      channel_statistics[BlueChannel].standard_deviation+=(double) p->blue*
        GetBluePixelComponent(p);
      channel_statistics[BlueChannel].kurtosis+=(double) p->blue*p->blue*
        p->blue*GetBluePixelComponent(p);
      channel_statistics[BlueChannel].skewness+=(double) p->blue*p->blue*
        GetBluePixelComponent(p);
      if (image->matte != MagickFalse)
        {
          if ((double) p->opacity < channel_statistics[OpacityChannel].minima)
            channel_statistics[OpacityChannel].minima=(double)
              GetOpacityPixelComponent(p);
          if ((double) p->opacity > channel_statistics[OpacityChannel].maxima)
            channel_statistics[OpacityChannel].maxima=(double)
              GetOpacityPixelComponent(p);
          channel_statistics[OpacityChannel].mean+=GetOpacityPixelComponent(p);
          channel_statistics[OpacityChannel].standard_deviation+=(double)
            p->opacity*GetOpacityPixelComponent(p);
          channel_statistics[OpacityChannel].kurtosis+=(double) p->opacity*
            p->opacity*p->opacity*GetOpacityPixelComponent(p);
          channel_statistics[OpacityChannel].skewness+=(double) p->opacity*
            p->opacity*GetOpacityPixelComponent(p);
        }
      if (image->colorspace == CMYKColorspace)
        {
          if ((double) indexes[x] < channel_statistics[BlackChannel].minima)
            channel_statistics[BlackChannel].minima=(double) indexes[x];
          if ((double) indexes[x] > channel_statistics[BlackChannel].maxima)
            channel_statistics[BlackChannel].maxima=(double) indexes[x];
          channel_statistics[BlackChannel].mean+=indexes[x];
          channel_statistics[BlackChannel].standard_deviation+=(double)
            indexes[x]*indexes[x];
          channel_statistics[BlackChannel].kurtosis+=(double) indexes[x]*
            indexes[x]*indexes[x]*indexes[x];
          channel_statistics[BlackChannel].skewness+=(double) indexes[x]*
            indexes[x]*indexes[x];
        }
      x++;
      p++;
    }
  }
  area=(double) image->columns*image->rows;
  for (i=0; i < AllChannels; i++)
  {
    channel_statistics[i].mean/=area;
    channel_statistics[i].standard_deviation/=area;
    channel_statistics[i].kurtosis/=area;
    channel_statistics[i].skewness/=area;
  }
  for (i=0; i < AllChannels; i++)
  {
    channel_statistics[AllChannels].depth=(unsigned long) MagickMax((double)
      channel_statistics[AllChannels].depth,(double)
      channel_statistics[i].depth);
    channel_statistics[AllChannels].minima=MagickMin(
      channel_statistics[AllChannels].minima,channel_statistics[i].minima);
    channel_statistics[AllChannels].maxima=MagickMax(
      channel_statistics[AllChannels].maxima,channel_statistics[i].maxima);
    channel_statistics[AllChannels].mean+=channel_statistics[i].mean;
    channel_statistics[AllChannels].standard_deviation+=
      channel_statistics[i].standard_deviation;
    channel_statistics[AllChannels].kurtosis+=channel_statistics[i].kurtosis;
    channel_statistics[AllChannels].skewness+=channel_statistics[i].skewness;
  }
  channels=4;
  if (image->colorspace == CMYKColorspace)
    channels++;
  channel_statistics[AllChannels].mean/=channels;
  channel_statistics[AllChannels].standard_deviation/=channels;
  channel_statistics[AllChannels].kurtosis/=channels;
  channel_statistics[AllChannels].skewness/=channels;
  for (i=0; i <= AllChannels; i++)
  {
    sum_squares=0.0;
    sum_squares=channel_statistics[i].standard_deviation;
    sum_cubes=0.0;
    sum_cubes=channel_statistics[i].skewness;
    channel_statistics[i].standard_deviation=sqrt(
      channel_statistics[i].standard_deviation-
       (channel_statistics[i].mean*channel_statistics[i].mean));
    if (channel_statistics[i].standard_deviation == 0.0)
      {
        channel_statistics[i].kurtosis=0.0;
        channel_statistics[i].skewness=0.0;
      }
    else
      {
        channel_statistics[i].skewness=(channel_statistics[i].skewness-
          3.0*channel_statistics[i].mean*sum_squares+
          2.0*channel_statistics[i].mean*channel_statistics[i].mean*
          channel_statistics[i].mean)/
          (channel_statistics[i].standard_deviation*
           channel_statistics[i].standard_deviation*
           channel_statistics[i].standard_deviation);
        channel_statistics[i].kurtosis=(channel_statistics[i].kurtosis-
          4.0*channel_statistics[i].mean*sum_cubes+
          6.0*channel_statistics[i].mean*channel_statistics[i].mean*sum_squares-
          3.0*channel_statistics[i].mean*channel_statistics[i].mean*
          1.0*channel_statistics[i].mean*channel_statistics[i].mean)/
          (channel_statistics[i].standard_deviation*
           channel_statistics[i].standard_deviation*
           channel_statistics[i].standard_deviation*
           channel_statistics[i].standard_deviation)-3.0;
      }
  }
  return(channel_statistics);
}

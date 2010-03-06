/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                   EEEEE  FFFFF  FFFFF  EEEEE  CCCC  TTTTT                   %
%                   E      F      F      E     C        T                     %
%                   EEE    FFF    FFF    EEE   C        T                     %
%                   E      F      F      E     C        T                     %
%                   EEEEE  F      F      EEEEE  CCCC    T                     %
%                                                                             %
%                                                                             %
%                       MagickCore Image Effects Methods                      %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                 October 1996                                %
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
#include "magick/accelerate.h"
#include "magick/blob.h"
#include "magick/cache-view.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/constitute.h"
#include "magick/decorate.h"
#include "magick/draw.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/effect.h"
#include "magick/fx.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/montage.h"
#include "magick/morphology.h"
#include "magick/paint.h"
#include "magick/pixel-private.h"
#include "magick/property.h"
#include "magick/quantize.h"
#include "magick/quantum.h"
#include "magick/random_.h"
#include "magick/random-private.h"
#include "magick/resample.h"
#include "magick/resample-private.h"
#include "magick/resize.h"
#include "magick/resource_.h"
#include "magick/segment.h"
#include "magick/shear.h"
#include "magick/signature-private.h"
#include "magick/string_.h"
#include "magick/thread-private.h"
#include "magick/transform.h"
#include "magick/threshold.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A d a p t i v e B l u r I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AdaptiveBlurImage() adaptively blurs the image by blurring less
%  intensely near image edges and more intensely far from edges.  We blur the
%  image with a Gaussian operator of the given radius and standard deviation
%  (sigma).  For reasonable results, radius should be larger than sigma.  Use a
%  radius of 0 and AdaptiveBlurImage() selects a suitable radius for you.
%
%  The format of the AdaptiveBlurImage method is:
%
%      Image *AdaptiveBlurImage(const Image *image,const double radius,
%        const double sigma,ExceptionInfo *exception)
%      Image *AdaptiveBlurImageChannel(const Image *image,
%        const ChannelType channel,double radius,const double sigma,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Laplacian, in pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image *AdaptiveBlurImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
{
  Image
    *blur_image;

  blur_image=AdaptiveBlurImageChannel(image,DefaultChannels,radius,sigma,
    exception);
  return(blur_image);
}

MagickExport Image *AdaptiveBlurImageChannel(const Image *image,
  const ChannelType channel,const double radius,const double sigma,
  ExceptionInfo *exception)
{
#define AdaptiveBlurImageTag  "Convolve/Image"
#define MagickSigma  (fabs(sigma) <= MagickEpsilon ? 1.0 : sigma)

  CacheView
    *blur_view,
    *edge_view,
    *image_view;

  double
    **kernel,
    normalize;

  Image
    *blur_image,
    *edge_image,
    *gaussian_image;

  long
    j,
    k,
    progress,
    u,
    v,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    bias;

  register long
    i;

  unsigned long
    width;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  blur_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (blur_image == (Image *) NULL)
    return((Image *) NULL);
  if (fabs(sigma) <= MagickEpsilon)
    return(blur_image);
  if (SetImageStorageClass(blur_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&blur_image->exception);
      blur_image=DestroyImage(blur_image);
      return((Image *) NULL);
    }
  /*
    Edge detect the image brighness channel, level, blur, and level again.
  */
  edge_image=EdgeImage(image,radius,exception);
  if (edge_image == (Image *) NULL)
    {
      blur_image=DestroyImage(blur_image);
      return((Image *) NULL);
    }
  (void) LevelImage(edge_image,"20%,95%");
  gaussian_image=GaussianBlurImage(edge_image,radius,sigma,exception);
  if (gaussian_image != (Image *) NULL)
    {
      edge_image=DestroyImage(edge_image);
      edge_image=gaussian_image;
    }
  (void) LevelImage(edge_image,"10%,95%");
  /*
    Create a set of kernels from maximum (radius,sigma) to minimum.
  */
  width=GetOptimalKernelWidth2D(radius,sigma);
  kernel=(double **) AcquireQuantumMemory((size_t) width,sizeof(*kernel));
  if (kernel == (double **) NULL)
    {
      edge_image=DestroyImage(edge_image);
      blur_image=DestroyImage(blur_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  (void) ResetMagickMemory(kernel,0,(size_t) width*sizeof(*kernel));
  for (i=0; i < (long) width; i+=2)
  {
    kernel[i]=(double *) AcquireQuantumMemory((size_t) (width-i),(width-i)*
      sizeof(**kernel));
    if (kernel[i] == (double *) NULL)
      break;
    normalize=0.0;
    j=(long) (width-i)/2;
    k=0;
    for (v=(-j); v <= j; v++)
    {
      for (u=(-j); u <= j; u++)
      {
        kernel[i][k]=exp(-((double) u*u+v*v)/(2.0*MagickSigma*MagickSigma))/
          (2.0*MagickPI*MagickSigma*MagickSigma);
        normalize+=kernel[i][k];
        k++;
      }
    }
    if (fabs(normalize) <= MagickEpsilon)
      normalize=1.0;
    normalize=1.0/normalize;
    for (k=0; k < (j*j); k++)
      kernel[i][k]=normalize*kernel[i][k];
  }
  if (i < (long) width)
    {
      for (i-=2; i >= 0; i-=2)
        kernel[i]=(double *) RelinquishMagickMemory(kernel[i]);
      kernel=(double **) RelinquishMagickMemory(kernel);
      edge_image=DestroyImage(edge_image);
      blur_image=DestroyImage(blur_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Adaptively blur image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&bias);
  SetMagickPixelPacketBias(image,&bias);
  image_view=AcquireCacheView(image);
  edge_view=AcquireCacheView(edge_image);
  blur_view=AcquireCacheView(blur_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) blur_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p,
      *restrict r;

    register IndexPacket
      *restrict blur_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    r=GetCacheViewVirtualPixels(edge_view,0,y,edge_image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if ((r == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    blur_indexes=GetCacheViewAuthenticIndexQueue(blur_view);
    for (x=0; x < (long) blur_image->columns; x++)
    {
      MagickPixelPacket
        pixel;

      MagickRealType
        alpha,
        gamma;

      register const double
        *restrict k;

      register long
        i,
        u,
        v;

      gamma=0.0;
      i=(long) (width*QuantumScale*PixelIntensity(r)+0.5);
      if (i < 0)
        i=0;
      else
        if (i > (long) width)
          i=(long) width;
      if ((i & 0x01) != 0)
        i--;
      p=GetCacheViewVirtualPixels(image_view,x-((long) (width-i)/2L),y-(long)
        ((width-i)/2L),width-i,width-i,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      pixel=bias;
      k=kernel[i];
      for (v=0; v < (long) (width-i); v++)
      {
        for (u=0; u < (long) (width-i); u++)
        {
          alpha=1.0;
          if (((channel & OpacityChannel) != 0) &&
              (image->matte != MagickFalse))
            alpha=(MagickRealType) (QuantumScale*GetAlphaPixelComponent(p));
          if ((channel & RedChannel) != 0)
            pixel.red+=(*k)*alpha*GetRedPixelComponent(p);
          if ((channel & GreenChannel) != 0)
            pixel.green+=(*k)*alpha*GetGreenPixelComponent(p);
          if ((channel & BlueChannel) != 0)
            pixel.blue+=(*k)*alpha*GetBluePixelComponent(p);
          if ((channel & OpacityChannel) != 0)
            pixel.opacity+=(*k)*GetOpacityPixelComponent(p);
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            pixel.index+=(*k)*alpha*indexes[x+(width-i)*v+u];
          gamma+=(*k)*alpha;
          k++;
          p++;
        }
      }
      gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
      if ((channel & RedChannel) != 0)
        q->red=ClampToQuantum(gamma*GetRedPixelComponent(&pixel));
      if ((channel & GreenChannel) != 0)
        q->green=ClampToQuantum(gamma*GetGreenPixelComponent(&pixel));
      if ((channel & BlueChannel) != 0)
        q->blue=ClampToQuantum(gamma*GetBluePixelComponent(&pixel));
      if ((channel & OpacityChannel) != 0)
        SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        blur_indexes[x]=ClampToQuantum(gamma*GetIndexPixelComponent(&pixel));
      q++;
      r++;
    }
    if (SyncCacheViewAuthenticPixels(blur_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_AdaptiveBlurImageChannel)
#endif
        proceed=SetImageProgress(image,AdaptiveBlurImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  blur_image->type=image->type;
  blur_view=DestroyCacheView(blur_view);
  edge_view=DestroyCacheView(edge_view);
  image_view=DestroyCacheView(image_view);
  edge_image=DestroyImage(edge_image);
  for (i=0; i < (long) width;  i+=2)
    kernel[i]=(double *) RelinquishMagickMemory(kernel[i]);
  kernel=(double **) RelinquishMagickMemory(kernel);
  if (status == MagickFalse)
    blur_image=DestroyImage(blur_image);
  return(blur_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A d a p t i v e S h a r p e n I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AdaptiveSharpenImage() adaptively sharpens the image by sharpening more
%  intensely near image edges and less intensely far from edges. We sharpen the
%  image with a Gaussian operator of the given radius and standard deviation
%  (sigma).  For reasonable results, radius should be larger than sigma.  Use a
%  radius of 0 and AdaptiveSharpenImage() selects a suitable radius for you.
%
%  The format of the AdaptiveSharpenImage method is:
%
%      Image *AdaptiveSharpenImage(const Image *image,const double radius,
%        const double sigma,ExceptionInfo *exception)
%      Image *AdaptiveSharpenImageChannel(const Image *image,
%        const ChannelType channel,double radius,const double sigma,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Laplacian, in pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image *AdaptiveSharpenImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
{
  Image
    *sharp_image;

  sharp_image=AdaptiveSharpenImageChannel(image,DefaultChannels,radius,sigma,
    exception);
  return(sharp_image);
}

MagickExport Image *AdaptiveSharpenImageChannel(const Image *image,
  const ChannelType channel,const double radius,const double sigma,
  ExceptionInfo *exception)
{
#define AdaptiveSharpenImageTag  "Convolve/Image"
#define MagickSigma  (fabs(sigma) <= MagickEpsilon ? 1.0 : sigma)

  CacheView
    *sharp_view,
    *edge_view,
    *image_view;

  double
    **kernel,
    normalize;

  Image
    *sharp_image,
    *edge_image,
    *gaussian_image;

  long
    j,
    k,
    progress,
    u,
    v,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    bias;

  register long
    i;

  unsigned long
    width;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  sharp_image=CloneImage(image,0,0,MagickTrue,exception);
  if (sharp_image == (Image *) NULL)
    return((Image *) NULL);
  if (fabs(sigma) <= MagickEpsilon)
    return(sharp_image);
  if (SetImageStorageClass(sharp_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&sharp_image->exception);
      sharp_image=DestroyImage(sharp_image);
      return((Image *) NULL);
    }
  /*
    Edge detect the image brighness channel, level, sharp, and level again.
  */
  edge_image=EdgeImage(image,radius,exception);
  if (edge_image == (Image *) NULL)
    {
      sharp_image=DestroyImage(sharp_image);
      return((Image *) NULL);
    }
  (void) LevelImage(edge_image,"20%,95%");
  gaussian_image=GaussianBlurImage(edge_image,radius,sigma,exception);
  if (gaussian_image != (Image *) NULL)
    {
      edge_image=DestroyImage(edge_image);
      edge_image=gaussian_image;
    }
  (void) LevelImage(edge_image,"10%,95%");
  /*
    Create a set of kernels from maximum (radius,sigma) to minimum.
  */
  width=GetOptimalKernelWidth2D(radius,sigma);
  kernel=(double **) AcquireQuantumMemory((size_t) width,sizeof(*kernel));
  if (kernel == (double **) NULL)
    {
      edge_image=DestroyImage(edge_image);
      sharp_image=DestroyImage(sharp_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  (void) ResetMagickMemory(kernel,0,(size_t) width*sizeof(*kernel));
  for (i=0; i < (long) width; i+=2)
  {
    kernel[i]=(double *) AcquireQuantumMemory((size_t) (width-i),(width-i)*
      sizeof(**kernel));
    if (kernel[i] == (double *) NULL)
      break;
    normalize=0.0;
    j=(long) (width-i)/2;
    k=0;
    for (v=(-j); v <= j; v++)
    {
      for (u=(-j); u <= j; u++)
      {
        kernel[i][k]=(-exp(-((double) u*u+v*v)/(2.0*MagickSigma*MagickSigma))/
          (2.0*MagickPI*MagickSigma*MagickSigma));
        normalize+=kernel[i][k];
        k++;
      }
    }
    if (fabs(normalize) <= MagickEpsilon)
      normalize=1.0;
    normalize=1.0/normalize;
    for (k=0; k < (j*j); k++)
      kernel[i][k]=normalize*kernel[i][k];
  }
  if (i < (long) width)
    {
      for (i-=2; i >= 0; i-=2)
        kernel[i]=(double *) RelinquishMagickMemory(kernel[i]);
      kernel=(double **) RelinquishMagickMemory(kernel);
      edge_image=DestroyImage(edge_image);
      sharp_image=DestroyImage(sharp_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Adaptively sharpen image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&bias);
  SetMagickPixelPacketBias(image,&bias);
  image_view=AcquireCacheView(image);
  edge_view=AcquireCacheView(edge_image);
  sharp_view=AcquireCacheView(sharp_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) sharp_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p,
      *restrict r;

    register IndexPacket
      *restrict sharp_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    r=GetCacheViewVirtualPixels(edge_view,0,y,edge_image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(sharp_view,0,y,sharp_image->columns,1,
      exception);
    if ((r == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    sharp_indexes=GetCacheViewAuthenticIndexQueue(sharp_view);
    for (x=0; x < (long) sharp_image->columns; x++)
    {
      MagickPixelPacket
        pixel;

      MagickRealType
        alpha,
        gamma;

      register const double
        *restrict k;

      register long
        i,
        u,
        v;

      gamma=0.0;
      i=(long) (width*(QuantumRange-QuantumScale*PixelIntensity(r))+0.5);
      if (i < 0)
        i=0;
      else
        if (i > (long) width)
          i=(long) width;
      if ((i & 0x01) != 0)
        i--;
      p=GetCacheViewVirtualPixels(image_view,x-((long) (width-i)/2L),y-(long)
        ((width-i)/2L),width-i,width-i,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      k=kernel[i];
      pixel=bias;
      for (v=0; v < (long) (width-i); v++)
      {
        for (u=0; u < (long) (width-i); u++)
        {
          alpha=1.0;
          if (((channel & OpacityChannel) != 0) &&
              (image->matte != MagickFalse))
            alpha=(MagickRealType) (QuantumScale*GetAlphaPixelComponent(p));
          if ((channel & RedChannel) != 0)
            pixel.red+=(*k)*alpha*GetRedPixelComponent(p);
          if ((channel & GreenChannel) != 0)
            pixel.green+=(*k)*alpha*GetGreenPixelComponent(p);
          if ((channel & BlueChannel) != 0)
            pixel.blue+=(*k)*alpha*GetBluePixelComponent(p);
          if ((channel & OpacityChannel) != 0)
            pixel.opacity+=(*k)*GetOpacityPixelComponent(p);
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            pixel.index+=(*k)*alpha*indexes[x+(width-i)*v+u];
          gamma+=(*k)*alpha;
          k++;
          p++;
        }
      }
      gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
      if ((channel & RedChannel) != 0)
        q->red=ClampToQuantum(gamma*GetRedPixelComponent(&pixel));
      if ((channel & GreenChannel) != 0)
        q->green=ClampToQuantum(gamma*GetGreenPixelComponent(&pixel));
      if ((channel & BlueChannel) != 0)
        q->blue=ClampToQuantum(gamma*GetBluePixelComponent(&pixel));
      if ((channel & OpacityChannel) != 0)
        SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        sharp_indexes[x]=ClampToQuantum(gamma*GetIndexPixelComponent(&pixel));
      q++;
      r++;
    }
    if (SyncCacheViewAuthenticPixels(sharp_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_AdaptiveSharpenImageChannel)
#endif
        proceed=SetImageProgress(image,AdaptiveSharpenImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  sharp_image->type=image->type;
  sharp_view=DestroyCacheView(sharp_view);
  edge_view=DestroyCacheView(edge_view);
  image_view=DestroyCacheView(image_view);
  edge_image=DestroyImage(edge_image);
  for (i=0; i < (long) width;  i+=2)
    kernel[i]=(double *) RelinquishMagickMemory(kernel[i]);
  kernel=(double **) RelinquishMagickMemory(kernel);
  if (status == MagickFalse)
    sharp_image=DestroyImage(sharp_image);
  return(sharp_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     B l u r I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BlurImage() blurs an image.  We convolve the image with a Gaussian operator
%  of the given radius and standard deviation (sigma).  For reasonable results,
%  the radius should be larger than sigma.  Use a radius of 0 and BlurImage()
%  selects a suitable radius for you.
%
%  BlurImage() differs from GaussianBlurImage() in that it uses a separable
%  kernel which is faster but mathematically equivalent to the non-separable
%  kernel.
%
%  The format of the BlurImage method is:
%
%      Image *BlurImage(const Image *image,const double radius,
%        const double sigma,ExceptionInfo *exception)
%      Image *BlurImageChannel(const Image *image,const ChannelType channel,
%        const double radius,const double sigma,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image *BlurImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
{
  Image
    *blur_image;

  blur_image=BlurImageChannel(image,DefaultChannels,radius,sigma,exception);
  return(blur_image);
}

static double *GetBlurKernel(const unsigned long width,const double sigma)
{
  double
    *kernel,
    normalize;

  long
    j,
    k;

  register long
    i;

  /*
    Generate a 1-D convolution kernel.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  kernel=(double *) AcquireQuantumMemory((size_t) width,sizeof(*kernel));
  if (kernel == (double *) NULL)
    return(0);
  normalize=0.0;
  j=(long) width/2;
  i=0;
  for (k=(-j); k <= j; k++)
  {
    kernel[i]=exp(-((double) k*k)/(2.0*MagickSigma*MagickSigma))/
      (MagickSQ2PI*MagickSigma);
    normalize+=kernel[i];
    i++;
  }
  for (i=0; i < (long) width; i++)
    kernel[i]/=normalize;
  return(kernel);
}

MagickExport Image *BlurImageChannel(const Image *image,
  const ChannelType channel,const double radius,const double sigma,
  ExceptionInfo *exception)
{
#define BlurImageTag  "Blur/Image"

  CacheView
    *blur_view,
    *image_view;

  double
    *kernel;

  Image
    *blur_image;

  long
    progress,
    x,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    bias;

  register long
    i;

  unsigned long
    width;

  /*
    Initialize blur image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  blur_image=CloneImage(image,0,0,MagickTrue,exception);
  if (blur_image == (Image *) NULL)
    return((Image *) NULL);
  if (fabs(sigma) <= MagickEpsilon)
    return(blur_image);
  if (SetImageStorageClass(blur_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&blur_image->exception);
      blur_image=DestroyImage(blur_image);
      return((Image *) NULL);
    }
  width=GetOptimalKernelWidth1D(radius,sigma);
  kernel=GetBlurKernel(width,sigma);
  if (kernel == (double *) NULL)
    {
      blur_image=DestroyImage(blur_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  if (image->debug != MagickFalse)
    {
      char
        format[MaxTextExtent],
        *message;

      register const double
        *k;

      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  BlurImage with %ld kernel:",width);
      message=AcquireString("");
      k=kernel;
      for (i=0; i < (long) width; i++)
      {
        *message='\0';
        (void) FormatMagickString(format,MaxTextExtent,"%ld: ",i);
        (void) ConcatenateString(&message,format);
        (void) FormatMagickString(format,MaxTextExtent,"%g ",*k++);
        (void) ConcatenateString(&message,format);
        (void) LogMagickEvent(TransformEvent,GetMagickModule(),"%s",message);
      }
      message=DestroyString(message);
    }
  /*
    Blur rows.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&bias);
  SetMagickPixelPacketBias(image,&bias);
  image_view=AcquireCacheView(image);
  blur_view=AcquireCacheView(blur_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) blur_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict blur_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((long) width/2L),y,image->columns+
      width,1,exception);
    q=GetCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    blur_indexes=GetCacheViewAuthenticIndexQueue(blur_view);
    for (x=0; x < (long) blur_image->columns; x++)
    {
      MagickPixelPacket
        pixel;

      register const double
        *restrict k;

      register const PixelPacket
        *restrict kernel_pixels;

      register long
        i;

      pixel=bias;
      k=kernel;
      kernel_pixels=p;
      if (((channel & OpacityChannel) == 0) || (image->matte == MagickFalse))
        {
          for (i=0; i < (long) width; i++)
          {
            pixel.red+=(*k)*kernel_pixels->red;
            pixel.green+=(*k)*kernel_pixels->green;
            pixel.blue+=(*k)*kernel_pixels->blue;
            k++;
            kernel_pixels++;
          }
          if ((channel & RedChannel) != 0)
            SetRedPixelComponent(q,ClampRedPixelComponent(&pixel));
          if ((channel & GreenChannel) != 0)
            SetGreenPixelComponent(q,ClampGreenPixelComponent(&pixel));
          if ((channel & BlueChannel) != 0)
            SetBluePixelComponent(q,ClampBluePixelComponent(&pixel));
          if ((channel & OpacityChannel) != 0)
            {
              k=kernel;
              kernel_pixels=p;
              for (i=0; i < (long) width; i++)
              {
                pixel.opacity+=(*k)*kernel_pixels->opacity;
                k++;
                kernel_pixels++;
              }
              SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=kernel;
              kernel_indexes=indexes;
              for (i=0; i < (long) width; i++)
              {
                pixel.index+=(*k)*(*kernel_indexes);
                k++;
                kernel_indexes++;
              }
              blur_indexes[x]=ClampToQuantum(pixel.index);
            }
        }
      else
        {
          MagickRealType
            alpha,
            gamma;

          gamma=0.0;
          for (i=0; i < (long) width; i++)
          {
            alpha=(MagickRealType) (QuantumScale*
              GetAlphaPixelComponent(kernel_pixels));
            pixel.red+=(*k)*alpha*kernel_pixels->red;
            pixel.green+=(*k)*alpha*kernel_pixels->green;
            pixel.blue+=(*k)*alpha*kernel_pixels->blue;
            gamma+=(*k)*alpha;
            k++;
            kernel_pixels++;
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          if ((channel & RedChannel) != 0)
            q->red=ClampToQuantum(gamma*GetRedPixelComponent(&pixel));
          if ((channel & GreenChannel) != 0)
            q->green=ClampToQuantum(gamma*GetGreenPixelComponent(&pixel));
          if ((channel & BlueChannel) != 0)
            q->blue=ClampToQuantum(gamma*GetBluePixelComponent(&pixel));
          if ((channel & OpacityChannel) != 0)
            {
              k=kernel;
              kernel_pixels=p;
              for (i=0; i < (long) width; i++)
              {
                pixel.opacity+=(*k)*kernel_pixels->opacity;
                k++;
                kernel_pixels++;
              }
              SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=kernel;
              kernel_pixels=p;
              kernel_indexes=indexes;
              for (i=0; i < (long) width; i++)
              {
                alpha=(MagickRealType) (QuantumScale*
                  GetAlphaPixelComponent(kernel_pixels));
                pixel.index+=(*k)*alpha*(*kernel_indexes);
                k++;
                kernel_pixels++;
                kernel_indexes++;
              }
              blur_indexes[x]=ClampToQuantum(gamma*
                GetIndexPixelComponent(&pixel));
            }
        }
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(blur_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_BlurImageChannel)
#endif
        proceed=SetImageProgress(image,BlurImageTag,progress++,blur_image->rows+
          blur_image->columns);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  blur_view=DestroyCacheView(blur_view);
  image_view=DestroyCacheView(image_view);
  /*
    Blur columns.
  */
  image_view=AcquireCacheView(blur_image);
  blur_view=AcquireCacheView(blur_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (x=0; x < (long) blur_image->columns; x++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict blur_indexes;

    register long
      y;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,x,-((long) width/2L),1,image->rows+
      width,exception);
    q=GetCacheViewAuthenticPixels(blur_view,x,0,1,blur_image->rows,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    blur_indexes=GetCacheViewAuthenticIndexQueue(blur_view);
    for (y=0; y < (long) blur_image->rows; y++)
    {
      MagickPixelPacket
        pixel;

      register const double
        *restrict k;

      register const PixelPacket
        *restrict kernel_pixels;

      register long
        i;

      pixel=bias;
      k=kernel;
      kernel_pixels=p;
      if (((channel & OpacityChannel) == 0) || (image->matte == MagickFalse))
        {
          for (i=0; i < (long) width; i++)
          {
            pixel.red+=(*k)*kernel_pixels->red;
            pixel.green+=(*k)*kernel_pixels->green;
            pixel.blue+=(*k)*kernel_pixels->blue;
            k++;
            kernel_pixels++;
          }
          if ((channel & RedChannel) != 0)
            SetRedPixelComponent(q,ClampRedPixelComponent(&pixel));
          if ((channel & GreenChannel) != 0)
            SetGreenPixelComponent(q,ClampGreenPixelComponent(&pixel));
          if ((channel & BlueChannel) != 0)
            SetBluePixelComponent(q,ClampBluePixelComponent(&pixel));
          if ((channel & OpacityChannel) != 0)
            {
              k=kernel;
              kernel_pixels=p;
              for (i=0; i < (long) width; i++)
              {
                pixel.opacity+=(*k)*kernel_pixels->opacity;
                k++;
                kernel_pixels++;
              }
              SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=kernel;
              kernel_indexes=indexes;
              for (i=0; i < (long) width; i++)
              {
                pixel.index+=(*k)*(*kernel_indexes);
                k++;
                kernel_indexes++;
              }
              blur_indexes[y]=ClampToQuantum(pixel.index);
            }
        }
      else
        {
          MagickRealType
            alpha,
            gamma;

          gamma=0.0;
          for (i=0; i < (long) width; i++)
          {
            alpha=(MagickRealType) (QuantumScale*
              GetAlphaPixelComponent(kernel_pixels));
            pixel.red+=(*k)*alpha*kernel_pixels->red;
            pixel.green+=(*k)*alpha*kernel_pixels->green;
            pixel.blue+=(*k)*alpha*kernel_pixels->blue;
            gamma+=(*k)*alpha;
            k++;
            kernel_pixels++;
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          if ((channel & RedChannel) != 0)
            q->red=ClampToQuantum(gamma*GetRedPixelComponent(&pixel));
          if ((channel & GreenChannel) != 0)
            q->green=ClampToQuantum(gamma*GetGreenPixelComponent(&pixel));
          if ((channel & BlueChannel) != 0)
            q->blue=ClampToQuantum(gamma*GetBluePixelComponent(&pixel));
          if ((channel & OpacityChannel) != 0)
            {
              k=kernel;
              kernel_pixels=p;
              for (i=0; i < (long) width; i++)
              {
                pixel.opacity+=(*k)*kernel_pixels->opacity;
                k++;
                kernel_pixels++;
              }
              SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=kernel;
              kernel_pixels=p;
              kernel_indexes=indexes;
              for (i=0; i < (long) width; i++)
              {
                alpha=(MagickRealType) (QuantumScale*
                  GetAlphaPixelComponent(kernel_pixels));
                pixel.index+=(*k)*alpha*(*kernel_indexes);
                k++;
                kernel_pixels++;
                kernel_indexes++;
              }
              blur_indexes[y]=ClampToQuantum(gamma*
                GetIndexPixelComponent(&pixel));
            }
        }
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(blur_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_BlurImageChannel)
#endif
        proceed=SetImageProgress(image,BlurImageTag,progress++,blur_image->rows+
          blur_image->columns);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  blur_view=DestroyCacheView(blur_view);
  image_view=DestroyCacheView(image_view);
  kernel=(double *) RelinquishMagickMemory(kernel);
  if (status == MagickFalse)
    blur_image=DestroyImage(blur_image);
  blur_image->type=image->type;
  return(blur_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o n v o l v e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConvolveImage() applies a custom convolution kernel to the image.
%
%  The format of the ConvolveImage method is:
%
%      Image *ConvolveImage(const Image *image,const unsigned long order,
%        const double *kernel,ExceptionInfo *exception)
%      Image *ConvolveImageChannel(const Image *image,const ChannelType channel,
%        const unsigned long order,const double *kernel,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o order: the number of columns and rows in the filter kernel.
%
%    o kernel: An array of double representing the convolution kernel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image *ConvolveImage(const Image *image,const unsigned long order,
  const double *kernel,ExceptionInfo *exception)
{
  Image
    *convolve_image;

  convolve_image=ConvolveImageChannel(image,DefaultChannels,order,kernel,
    exception);
  return(convolve_image);
}

MagickExport Image *ConvolveImageChannel(const Image *image,
  const ChannelType channel,const unsigned long order,const double *kernel,
  ExceptionInfo *exception)
{
#define ConvolveImageTag  "Convolve/Image"

  CacheView
    *convolve_view,
    *image_view;

  double
    *normal_kernel;

  Image
    *convolve_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    bias;

  MagickRealType
    gamma;

  register long
    i;

  unsigned long
    width;

  /*
    Initialize convolve image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=order;
  if ((width % 2) == 0)
    ThrowImageException(OptionError,"KernelWidthMustBeAnOddNumber");
  convolve_image=CloneImage(image,0,0,MagickTrue,exception);
  if (convolve_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(convolve_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&convolve_image->exception);
      convolve_image=DestroyImage(convolve_image);
      return((Image *) NULL);
    }
  if (image->debug != MagickFalse)
    {
      char
        format[MaxTextExtent],
        *message;

      long
        u,
        v;

      register const double
        *k;

      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  ConvolveImage with %ldx%ld kernel:",width,width);
      message=AcquireString("");
      k=kernel;
      for (v=0; v < (long) width; v++)
      {
        *message='\0';
        (void) FormatMagickString(format,MaxTextExtent,"%ld: ",v);
        (void) ConcatenateString(&message,format);
        for (u=0; u < (long) width; u++)
        {
          (void) FormatMagickString(format,MaxTextExtent,"%g ",*k++);
          (void) ConcatenateString(&message,format);
        }
        (void) LogMagickEvent(TransformEvent,GetMagickModule(),"%s",message);
      }
      message=DestroyString(message);
    }
  /*
    Normalize kernel.
  */
  normal_kernel=(double *) AcquireQuantumMemory(width*width,
    sizeof(*normal_kernel));
  if (normal_kernel == (double *) NULL)
    {
      convolve_image=DestroyImage(convolve_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  gamma=0.0;
  for (i=0; i < (long) (width*width); i++)
    gamma+=kernel[i];
  gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
  for (i=0; i < (long) (width*width); i++)
    normal_kernel[i]=gamma*kernel[i];
  /*
    Convolve image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&bias);
  SetMagickPixelPacketBias(image,&bias);
  image_view=AcquireCacheView(image);
  convolve_view=AcquireCacheView(convolve_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict convolve_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((long) width/2L),y-(long) (width/
      2L),image->columns+width,width,exception);
    q=GetCacheViewAuthenticPixels(convolve_view,0,y,convolve_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    convolve_indexes=GetCacheViewAuthenticIndexQueue(convolve_view);
    for (x=0; x < (long) image->columns; x++)
    {
      long
        v;

      MagickPixelPacket
        pixel;

      register const double
        *restrict k;

      register const PixelPacket
        *restrict kernel_pixels;

      register long
        u;

      pixel=bias;
      k=normal_kernel;
      kernel_pixels=p;
      if (((channel & OpacityChannel) == 0) || (image->matte == MagickFalse))
        {
          for (v=0; v < (long) width; v++)
          {
            for (u=0; u < (long) width; u++)
            {
              pixel.red+=(*k)*kernel_pixels[u].red;
              pixel.green+=(*k)*kernel_pixels[u].green;
              pixel.blue+=(*k)*kernel_pixels[u].blue;
              k++;
            }
            kernel_pixels+=image->columns+width;
          }
          if ((channel & RedChannel) != 0)
            SetRedPixelComponent(q,ClampRedPixelComponent(&pixel));
          if ((channel & GreenChannel) != 0)
            SetGreenPixelComponent(q,ClampGreenPixelComponent(&pixel));
          if ((channel & BlueChannel) != 0)
            SetBluePixelComponent(q,ClampBluePixelComponent(&pixel));
          if ((channel & OpacityChannel) != 0)
            {
              k=normal_kernel;
              kernel_pixels=p;
              for (v=0; v < (long) width; v++)
              {
                for (u=0; u < (long) width; u++)
                {
                  pixel.opacity+=(*k)*kernel_pixels[u].opacity;
                  k++;
                }
                kernel_pixels+=image->columns+width;
              }
              SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=normal_kernel;
              kernel_indexes=indexes;
              for (v=0; v < (long) width; v++)
              {
                for (u=0; u < (long) width; u++)
                {
                  pixel.index+=(*k)*kernel_indexes[u];
                  k++;
                }
                kernel_indexes+=image->columns+width;
              }
              convolve_indexes[x]=ClampToQuantum(pixel.index);
            }
        }
      else
        {
          MagickRealType
            alpha,
            gamma;

          gamma=0.0;
          for (v=0; v < (long) width; v++)
          {
            for (u=0; u < (long) width; u++)
            {
              alpha=(MagickRealType) (QuantumScale*(QuantumRange-
                kernel_pixels[u].opacity));
              pixel.red+=(*k)*alpha*kernel_pixels[u].red;
              pixel.green+=(*k)*alpha*kernel_pixels[u].green;
              pixel.blue+=(*k)*alpha*kernel_pixels[u].blue;
              gamma+=(*k)*alpha;
              k++;
            }
            kernel_pixels+=image->columns+width;
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          if ((channel & RedChannel) != 0)
            q->red=ClampToQuantum(gamma*GetRedPixelComponent(&pixel));
          if ((channel & GreenChannel) != 0)
            q->green=ClampToQuantum(gamma*GetGreenPixelComponent(&pixel));
          if ((channel & BlueChannel) != 0)
            q->blue=ClampToQuantum(gamma*GetBluePixelComponent(&pixel));
          if ((channel & OpacityChannel) != 0)
            {
              k=normal_kernel;
              kernel_pixels=p;
              for (v=0; v < (long) width; v++)
              {
                for (u=0; u < (long) width; u++)
                {
                  pixel.opacity+=(*k)*kernel_pixels[u].opacity;
                  k++;
                }
                kernel_pixels+=image->columns+width;
              }
              SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=normal_kernel;
              kernel_pixels=p;
              kernel_indexes=indexes;
              for (v=0; v < (long) width; v++)
              {
                for (u=0; u < (long) width; u++)
                {
                  alpha=(MagickRealType) (QuantumScale*(QuantumRange-
                    kernel_pixels[u].opacity));
                  pixel.index+=(*k)*alpha*kernel_indexes[u];
                  k++;
                }
                kernel_pixels+=image->columns+width;
                kernel_indexes+=image->columns+width;
              }
              convolve_indexes[x]=ClampToQuantum(gamma*
                GetIndexPixelComponent(&pixel));
            }
        }
      p++;
      q++;
    }
    sync=SyncCacheViewAuthenticPixels(convolve_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ConvolveImageChannel)
#endif
        proceed=SetImageProgress(image,ConvolveImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  convolve_image->type=image->type;
  convolve_view=DestroyCacheView(convolve_view);
  image_view=DestroyCacheView(image_view);
  normal_kernel=(double *) RelinquishMagickMemory(normal_kernel);
  if (status == MagickFalse)
    convolve_image=DestroyImage(convolve_image);
  return(convolve_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     D e s p e c k l e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DespeckleImage() reduces the speckle noise in an image while perserving the
%  edges of the original image.
%
%  The format of the DespeckleImage method is:
%
%      Image *DespeckleImage(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static Quantum **DestroyPixelThreadSet(Quantum **pixels)
{
  register long
    i;

  assert(pixels != (Quantum **) NULL);
  for (i=0; i < (long) GetOpenMPMaximumThreads(); i++)
    if (pixels[i] != (Quantum *) NULL)
      pixels[i]=(Quantum *) RelinquishMagickMemory(pixels[i]);
  pixels=(Quantum **) RelinquishAlignedMemory(pixels);
  return(pixels);
}

static Quantum **AcquirePixelThreadSet(const size_t count)
{
  register long
    i;

  Quantum
    **pixels;

  unsigned long
    number_threads;

  number_threads=GetOpenMPMaximumThreads();
  pixels=(Quantum **) AcquireAlignedMemory(number_threads,sizeof(*pixels));
  if (pixels == (Quantum **) NULL)
    return((Quantum **) NULL);
  (void) ResetMagickMemory(pixels,0,number_threads*sizeof(*pixels));
  for (i=0; i < (long) number_threads; i++)
  {
    pixels[i]=(Quantum *) AcquireQuantumMemory(count,sizeof(**pixels));
    if (pixels[i] == (Quantum *) NULL)
      return(DestroyPixelThreadSet(pixels));
  }
  return(pixels);
}

static void Hull(const long x_offset,const long y_offset,
  const unsigned long columns,const unsigned long rows,Quantum *f,Quantum *g,
  const int polarity)
{
  long
    y;

  MagickRealType
    v;

  register long
    x;

  register Quantum
    *p,
    *q,
    *r,
    *s;

  assert(f != (Quantum *) NULL);
  assert(g != (Quantum *) NULL);
  p=f+(columns+2);
  q=g+(columns+2);
  r=p+(y_offset*((long) columns+2)+x_offset);
  for (y=0; y < (long) rows; y++)
  {
    p++;
    q++;
    r++;
    if (polarity > 0)
      for (x=(long) columns; x != 0; x--)
      {
        v=(MagickRealType) (*p);
        if ((MagickRealType) *r >= (v+(MagickRealType) ScaleCharToQuantum(2)))
          v+=ScaleCharToQuantum(1);
        *q=(Quantum) v;
        p++;
        q++;
        r++;
      }
    else
      for (x=(long) columns; x != 0; x--)
      {
        v=(MagickRealType) (*p);
        if ((MagickRealType) *r <= (v-(MagickRealType) ScaleCharToQuantum(2)))
          v-=(long) ScaleCharToQuantum(1);
        *q=(Quantum) v;
        p++;
        q++;
        r++;
      }
    p++;
    q++;
    r++;
  }
  p=f+(columns+2);
  q=g+(columns+2);
  r=q+(y_offset*((long) columns+2)+x_offset);
  s=q-(y_offset*((long) columns+2)+x_offset);
  for (y=0; y < (long) rows; y++)
  {
    p++;
    q++;
    r++;
    s++;
    if (polarity > 0)
      for (x=(long) columns; x != 0; x--)
      {
        v=(MagickRealType) (*q);
        if (((MagickRealType) *s >=
             (v+(MagickRealType) ScaleCharToQuantum(2))) &&
            ((MagickRealType) *r > v))
          v+=ScaleCharToQuantum(1);
        *p=(Quantum) v;
        p++;
        q++;
        r++;
        s++;
      }
    else
      for (x=(long) columns; x != 0; x--)
      {
        v=(MagickRealType) (*q);
        if (((MagickRealType) *s <=
             (v-(MagickRealType) ScaleCharToQuantum(2))) &&
            ((MagickRealType) *r < v))
          v-=(MagickRealType) ScaleCharToQuantum(1);
        *p=(Quantum) v;
        p++;
        q++;
        r++;
        s++;
      }
    p++;
    q++;
    r++;
    s++;
  }
}

MagickExport Image *DespeckleImage(const Image *image,ExceptionInfo *exception)
{
#define DespeckleImageTag  "Despeckle/Image"

  CacheView
    *despeckle_view,
    *image_view;

  Image
    *despeckle_image;

  long
    channel;

  MagickBooleanType
    status;

  Quantum
    **restrict buffers,
    **restrict pixels;

  size_t
    length;

  static const int
    X[4] = {0, 1, 1,-1},
    Y[4] = {1, 0, 1, 1};

  /*
    Allocate despeckled image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  despeckle_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (despeckle_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(despeckle_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&despeckle_image->exception);
      despeckle_image=DestroyImage(despeckle_image);
      return((Image *) NULL);
    }
  /*
    Allocate image buffers.
  */
  length=(size_t) ((image->columns+2)*(image->rows+2));
  pixels=AcquirePixelThreadSet(length);
  buffers=AcquirePixelThreadSet(length);
  if ((pixels == (Quantum **) NULL) || (buffers == (Quantum **) NULL))
    {
      if (buffers != (Quantum **) NULL)
        buffers=DestroyPixelThreadSet(buffers);
      if (pixels != (Quantum **) NULL)
        pixels=DestroyPixelThreadSet(pixels);
      despeckle_image=DestroyImage(despeckle_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Reduce speckle in the image.
  */
  status=MagickTrue;
  image_view=AcquireCacheView(image);
  despeckle_view=AcquireCacheView(despeckle_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
  for (channel=0; channel <= 3; channel++)
  {
    long
      j,
      y;

    register long
      i,
      id,
      x;

    register Quantum
      *buffer,
      *pixel;

    if (status == MagickFalse)
      continue;
    id=GetOpenMPThreadId();
    pixel=pixels[id];
    (void) ResetMagickMemory(pixel,0,length*sizeof(*pixel));
    buffer=buffers[id];
    j=(long) image->columns+2;
    for (y=0; y < (long) image->rows; y++)
    {
      register const PixelPacket
        *restrict p;

      p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      j++;
      for (x=0; x < (long) image->columns; x++)
      {
        switch (channel)
        {
          case 0: pixel[j]=GetRedPixelComponent(p); break;
          case 1: pixel[j]=GetGreenPixelComponent(p); break;
          case 2: pixel[j]=GetBluePixelComponent(p); break;
          case 3: pixel[j]=GetOpacityPixelComponent(p); break;
          default: break;
        }
        p++;
        j++;
      }
      j++;
    }
    (void) ResetMagickMemory(buffer,0,length*sizeof(*buffer));
    for (i=0; i < 4; i++)
    {
      Hull(X[i],Y[i],image->columns,image->rows,pixel,buffer,1);
      Hull(-X[i],-Y[i],image->columns,image->rows,pixel,buffer,1);
      Hull(-X[i],-Y[i],image->columns,image->rows,pixel,buffer,-1);
      Hull(X[i],Y[i],image->columns,image->rows,pixel,buffer,-1);
    }
    j=(long) image->columns+2;
    for (y=0; y < (long) image->rows; y++)
    {
      MagickBooleanType
        sync;

      register PixelPacket
        *restrict q;

      q=GetCacheViewAuthenticPixels(despeckle_view,0,y,despeckle_image->columns,
        1,exception);
      if (q == (PixelPacket *) NULL)
        break;
      j++;
      for (x=0; x < (long) image->columns; x++)
      {
        switch (channel)
        {
          case 0: q->red=pixel[j]; break;
          case 1: q->green=pixel[j]; break;
          case 2: q->blue=pixel[j]; break;
          case 3: q->opacity=pixel[j]; break;
          default: break;
        }
        q++;
        j++;
      }
      sync=SyncCacheViewAuthenticPixels(despeckle_view,exception);
      if (sync == MagickFalse)
        {
          status=MagickFalse;
          break;
        }
      j++;
    }
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_DespeckleImage)
#endif
        proceed=SetImageProgress(image,DespeckleImageTag,channel,3);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  despeckle_view=DestroyCacheView(despeckle_view);
  image_view=DestroyCacheView(image_view);
  buffers=DestroyPixelThreadSet(buffers);
  pixels=DestroyPixelThreadSet(pixels);
  despeckle_image->type=image->type;
  if (status == MagickFalse)
    despeckle_image=DestroyImage(despeckle_image);
  return(despeckle_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     E d g e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EdgeImage() finds edges in an image.  Radius defines the radius of the
%  convolution filter.  Use a radius of 0 and EdgeImage() selects a suitable
%  radius for you.
%
%  The format of the EdgeImage method is:
%
%      Image *EdgeImage(const Image *image,const double radius,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the pixel neighborhood.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *EdgeImage(const Image *image,const double radius,
  ExceptionInfo *exception)
{
  Image
    *edge_image;

  double
    *kernel;

  register long
    i;

  unsigned long
    width;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth1D(radius,0.5);
  kernel=(double *) AcquireQuantumMemory((size_t) width,width*sizeof(*kernel));
  if (kernel == (double *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  for (i=0; i < (long) (width*width); i++)
    kernel[i]=(-1.0);
  kernel[i/2]=(double) (width*width-1.0);
  edge_image=ConvolveImage(image,width,kernel,exception);
  kernel=(double *) RelinquishMagickMemory(kernel);
  return(edge_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     E m b o s s I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EmbossImage() returns a grayscale image with a three-dimensional effect.
%  We convolve the image with a Gaussian operator of the given radius and
%  standard deviation (sigma).  For reasonable results, radius should be
%  larger than sigma.  Use a radius of 0 and Emboss() selects a suitable
%  radius for you.
%
%  The format of the EmbossImage method is:
%
%      Image *EmbossImage(const Image *image,const double radius,
%        const double sigma,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the pixel neighborhood.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *EmbossImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
{
  double
    *kernel;

  Image
    *emboss_image;

  long
    j,
    k,
    u,
    v;

  register long
    i;

  unsigned long
    width;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth2D(radius,sigma);
  kernel=(double *) AcquireQuantumMemory((size_t) width,width*sizeof(*kernel));
  if (kernel == (double *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  j=(long) width/2;
  k=j;
  i=0;
  for (v=(-j); v <= j; v++)
  {
    for (u=(-j); u <= j; u++)
    {
      kernel[i]=((u < 0) || (v < 0) ? -8.0 : 8.0)*
        exp(-((double) u*u+v*v)/(2.0*MagickSigma*MagickSigma))/
        (2.0*MagickPI*MagickSigma*MagickSigma);
      if (u != k)
        kernel[i]=0.0;
      i++;
    }
    k--;
  }
  emboss_image=ConvolveImage(image,width,kernel,exception);
  if (emboss_image != (Image *) NULL)
    (void) EqualizeImage(emboss_image);
  kernel=(double *) RelinquishMagickMemory(kernel);
  return(emboss_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     F i l t e r I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FilterImage() applies a custom convolution kernel to the image.
%
%  The format of the FilterImage method is:
%
%      Image *FilterImage(const Image *image,const KernelInfo *kernel,
%        ExceptionInfo *exception)
%      Image *FilterImageChannel(const Image *image,const ChannelType channel,
%        const KernelInfo *kernel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o kernel: the filtering kernel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image *FilterImage(const Image *image,const KernelInfo *kernel,
  ExceptionInfo *exception)
{
  Image
    *filter_image;

  filter_image=FilterImageChannel(image,DefaultChannels,kernel,exception);
  return(filter_image);
}

MagickExport Image *FilterImageChannel(const Image *image,
  const ChannelType channel,const KernelInfo *kernel,ExceptionInfo *exception)
{
#define FilterImageTag  "Filter/Image"

  CacheView
    *filter_view,
    *image_view;

  Image
    *filter_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    bias;

  /*
    Initialize filter image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if ((kernel->width % 2) == 0)
    ThrowImageException(OptionError,"KernelWidthMustBeAnOddNumber");
  filter_image=CloneImage(image,0,0,MagickTrue,exception);
  if (filter_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(filter_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&filter_image->exception);
      filter_image=DestroyImage(filter_image);
      return((Image *) NULL);
    }
  if (image->debug != MagickFalse)
    {
      char
        format[MaxTextExtent],
        *message;

      long
        u,
        v;

      register const double
        *k;

      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  FilterImage with %ldx%ld kernel:",kernel->width,kernel->height);
      message=AcquireString("");
      k=kernel->values;
      for (v=0; v < (long) kernel->height; v++)
      {
        *message='\0';
        (void) FormatMagickString(format,MaxTextExtent,"%ld: ",v);
        (void) ConcatenateString(&message,format);
        for (u=0; u < (long) kernel->width; u++)
        {
          (void) FormatMagickString(format,MaxTextExtent,"%g ",*k++);
          (void) ConcatenateString(&message,format);
        }
        (void) LogMagickEvent(TransformEvent,GetMagickModule(),"%s",message);
      }
      message=DestroyString(message);
    }
  status=AccelerateConvolveImage(image,kernel,filter_image,exception);
  if (status == MagickTrue)
    return(filter_image);
  /*
    Filter image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&bias);
  SetMagickPixelPacketBias(image,&bias);
  image_view=AcquireCacheView(image);
  filter_view=AcquireCacheView(filter_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict filter_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((long) kernel->width/2L),
      y-(long) (kernel->height/2L),image->columns+kernel->width,kernel->height,
      exception);
    q=GetCacheViewAuthenticPixels(filter_view,0,y,filter_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    filter_indexes=GetCacheViewAuthenticIndexQueue(filter_view);
    for (x=0; x < (long) image->columns; x++)
    {
      long
        v;

      MagickPixelPacket
        pixel;

      register const double
        *restrict k;

      register const PixelPacket
        *restrict kernel_pixels;

      register long
        u;

      pixel=bias;
      k=kernel->values;
      kernel_pixels=p;
      if (((channel & OpacityChannel) == 0) || (image->matte == MagickFalse))
        {
          for (v=0; v < (long) kernel->width; v++)
          {
            for (u=0; u < (long) kernel->height; u++)
            {
              pixel.red+=(*k)*kernel_pixels[u].red;
              pixel.green+=(*k)*kernel_pixels[u].green;
              pixel.blue+=(*k)*kernel_pixels[u].blue;
              k++;
            }
            kernel_pixels+=image->columns+kernel->width;
          }
          if ((channel & RedChannel) != 0)
            SetRedPixelComponent(q,ClampRedPixelComponent(&pixel));
          if ((channel & GreenChannel) != 0)
            SetGreenPixelComponent(q,ClampGreenPixelComponent(&pixel));
          if ((channel & BlueChannel) != 0)
            SetBluePixelComponent(q,ClampBluePixelComponent(&pixel));
          if ((channel & OpacityChannel) != 0)
            {
              k=kernel->values;
              kernel_pixels=p;
              for (v=0; v < (long) kernel->width; v++)
              {
                for (u=0; u < (long) kernel->height; u++)
                {
                  pixel.opacity+=(*k)*kernel_pixels[u].opacity;
                  k++;
                }
                kernel_pixels+=image->columns+kernel->width;
              }
              SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=kernel->values;
              kernel_indexes=indexes;
              for (v=0; v < (long) kernel->width; v++)
              {
                for (u=0; u < (long) kernel->height; u++)
                {
                  pixel.index+=(*k)*kernel_indexes[u];
                  k++;
                }
                kernel_indexes+=image->columns+kernel->width;
              }
              filter_indexes[x]=ClampToQuantum(pixel.index);
            }
        }
      else
        {
          MagickRealType
            alpha,
            gamma;

          gamma=0.0;
          for (v=0; v < (long) kernel->width; v++)
          {
            for (u=0; u < (long) kernel->height; u++)
            {
              alpha=(MagickRealType) (QuantumScale*(QuantumRange-
                kernel_pixels[u].opacity));
              pixel.red+=(*k)*alpha*kernel_pixels[u].red;
              pixel.green+=(*k)*alpha*kernel_pixels[u].green;
              pixel.blue+=(*k)*alpha*kernel_pixels[u].blue;
              gamma+=(*k)*alpha;
              k++;
            }
            kernel_pixels+=image->columns+kernel->width;
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          if ((channel & RedChannel) != 0)
            q->red=ClampToQuantum(gamma*GetRedPixelComponent(&pixel));
          if ((channel & GreenChannel) != 0)
            q->green=ClampToQuantum(gamma*GetGreenPixelComponent(&pixel));
          if ((channel & BlueChannel) != 0)
            q->blue=ClampToQuantum(gamma*GetBluePixelComponent(&pixel));
          if ((channel & OpacityChannel) != 0)
            {
              k=kernel->values;
              kernel_pixels=p;
              for (v=0; v < (long) kernel->width; v++)
              {
                for (u=0; u < (long) kernel->height; u++)
                {
                  pixel.opacity+=(*k)*kernel_pixels[u].opacity;
                  k++;
                }
                kernel_pixels+=image->columns+kernel->width;
              }
              SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=kernel->values;
              kernel_pixels=p;
              kernel_indexes=indexes;
              for (v=0; v < (long) kernel->width; v++)
              {
                for (u=0; u < (long) kernel->height; u++)
                {
                  alpha=(MagickRealType) (QuantumScale*(QuantumRange-
                    kernel_pixels[u].opacity));
                  pixel.index+=(*k)*alpha*kernel_indexes[u];
                  k++;
                }
                kernel_pixels+=image->columns+kernel->width;
                kernel_indexes+=image->columns+kernel->width;
              }
              filter_indexes[x]=ClampToQuantum(gamma*
                GetIndexPixelComponent(&pixel));
            }
        }
      p++;
      q++;
    }
    sync=SyncCacheViewAuthenticPixels(filter_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_FilterImageChannel)
#endif
        proceed=SetImageProgress(image,FilterImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  filter_image->type=image->type;
  filter_view=DestroyCacheView(filter_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    filter_image=DestroyImage(filter_image);
  return(filter_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     G a u s s i a n B l u r I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GaussianBlurImage() blurs an image.  We convolve the image with a
%  Gaussian operator of the given radius and standard deviation (sigma).
%  For reasonable results, the radius should be larger than sigma.  Use a
%  radius of 0 and GaussianBlurImage() selects a suitable radius for you
%
%  The format of the GaussianBlurImage method is:
%
%      Image *GaussianBlurImage(const Image *image,onst double radius,
%        const double sigma,ExceptionInfo *exception)
%      Image *GaussianBlurImageChannel(const Image *image,
%        const ChannelType channel,const double radius,const double sigma,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image *GaussianBlurImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
{
  Image
    *blur_image;

  blur_image=GaussianBlurImageChannel(image,DefaultChannels,radius,sigma,
    exception);
  return(blur_image);
}

MagickExport Image *GaussianBlurImageChannel(const Image *image,
  const ChannelType channel,const double radius,const double sigma,
  ExceptionInfo *exception)
{
  double
    *kernel;

  Image
    *blur_image;

  long
    j,
    u,
    v;

  register long
    i;

  unsigned long
    width;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth2D(radius,sigma);
  kernel=(double *) AcquireQuantumMemory((size_t) width,width*sizeof(*kernel));
  if (kernel == (double *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  j=(long) width/2;
  i=0;
  for (v=(-j); v <= j; v++)
  {
    for (u=(-j); u <= j; u++)
      kernel[i++]=exp(-((double) u*u+v*v)/(2.0*MagickSigma*MagickSigma))/
        (2.0*MagickPI*MagickSigma*MagickSigma);
  }
  blur_image=ConvolveImageChannel(image,channel,width,kernel,exception);
  kernel=(double *) RelinquishMagickMemory(kernel);
  return(blur_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M e d i a n F i l t e r I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MedianFilterImage() applies a digital filter that improves the quality
%  of a noisy image.  Each pixel is replaced by the median in a set of
%  neighboring pixels as defined by radius.
%
%  The algorithm was contributed by Mike Edmonds and implements an insertion
%  sort for selecting median color-channel values.  For more on this algorithm
%  see "Skip Lists: A probabilistic Alternative to Balanced Trees" by William
%  Pugh in the June 1990 of Communications of the ACM.
%
%  The format of the MedianFilterImage method is:
%
%      Image *MedianFilterImage(const Image *image,const double radius,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the pixel neighborhood.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#define MedianListChannels  5

typedef struct _MedianListNode
{
  unsigned long
    next[9],
    count,
    signature;
} MedianListNode;

typedef struct _MedianSkipList
{
  long
    level;

  MedianListNode
    *nodes;
} MedianSkipList;

typedef struct _MedianPixelList
{
  unsigned long
    center,
    seed,
    signature;

  MedianSkipList
    lists[MedianListChannels];
} MedianPixelList;

static MedianPixelList *DestroyMedianPixelList(MedianPixelList *pixel_list)
{
  register long
    i;

  if (pixel_list == (MedianPixelList *) NULL)
    return((MedianPixelList *) NULL);
  for (i=0; i < MedianListChannels; i++)
    if (pixel_list->lists[i].nodes != (MedianListNode *) NULL)
      pixel_list->lists[i].nodes=(MedianListNode *) RelinquishMagickMemory(
        pixel_list->lists[i].nodes);
  pixel_list=(MedianPixelList *) RelinquishAlignedMemory(pixel_list);
  return(pixel_list);
}

static MedianPixelList **DestroyMedianPixelListThreadSet(
  MedianPixelList **pixel_list)
{
  register long
    i;

  assert(pixel_list != (MedianPixelList **) NULL);
  for (i=0; i < (long) GetOpenMPMaximumThreads(); i++)
    if (pixel_list[i] != (MedianPixelList *) NULL)
      pixel_list[i]=DestroyMedianPixelList(pixel_list[i]);
  pixel_list=(MedianPixelList **) RelinquishAlignedMemory(pixel_list);
  return(pixel_list);
}

static MedianPixelList *AcquireMedianPixelList(const unsigned long width)
{
  MedianPixelList
    *pixel_list;

  register long
    i;

  pixel_list=(MedianPixelList *) AcquireAlignedMemory(1,sizeof(*pixel_list));
  if (pixel_list == (MedianPixelList *) NULL)
    return(pixel_list);
  (void) ResetMagickMemory((void *) pixel_list,0,sizeof(*pixel_list));
  pixel_list->center=width*width/2;
  for (i=0; i < MedianListChannels; i++)
  {
    pixel_list->lists[i].nodes=(MedianListNode *) AcquireQuantumMemory(65537UL,
      sizeof(*pixel_list->lists[i].nodes));
    if (pixel_list->lists[i].nodes == (MedianListNode *) NULL)
      return(DestroyMedianPixelList(pixel_list));
    (void) ResetMagickMemory(pixel_list->lists[i].nodes,0,65537UL*
      sizeof(*pixel_list->lists[i].nodes));
  }
  pixel_list->signature=MagickSignature;
  return(pixel_list);
}

static MedianPixelList **AcquireMedianPixelListThreadSet(
  const unsigned long width)
{
  register long
    i;

  MedianPixelList
    **pixel_list;

  unsigned long
    number_threads;

  number_threads=GetOpenMPMaximumThreads();
  pixel_list=(MedianPixelList **) AcquireAlignedMemory(number_threads,
    sizeof(*pixel_list));
  if (pixel_list == (MedianPixelList **) NULL)
    return((MedianPixelList **) NULL);
  (void) ResetMagickMemory(pixel_list,0,number_threads*sizeof(*pixel_list));
  for (i=0; i < (long) number_threads; i++)
  {
    pixel_list[i]=AcquireMedianPixelList(width);
    if (pixel_list[i] == (MedianPixelList *) NULL)
      return(DestroyMedianPixelListThreadSet(pixel_list));
  }
  return(pixel_list);
}

static void AddNodeMedianPixelList(MedianPixelList *pixel_list,
  const long channel,const unsigned long color)
{
  register long
    level;

  register MedianSkipList
    *list;

  unsigned long
    search,
    update[9];

  /*
    Initialize the node.
  */
  list=pixel_list->lists+channel;
  list->nodes[color].signature=pixel_list->signature;
  list->nodes[color].count=1;
  /*
    Determine where it belongs in the list.
  */
  search=65536UL;
  for (level=list->level; level >= 0; level--)
  {
    while (list->nodes[search].next[level] < color)
      search=list->nodes[search].next[level];
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
  if (level > (list->level+2))
    level=list->level+2;
  /*
    If we're raising the list's level, link back to the root node.
  */
  while (level > list->level)
  {
    list->level++;
    update[list->level]=65536UL;
  }
  /*
    Link the node into the skip-list.
  */
  do
  {
    list->nodes[color].next[level]=list->nodes[update[level]].next[level];
    list->nodes[update[level]].next[level]=color;
  }
  while (level-- > 0);
}

static MagickPixelPacket GetMedianPixelList(MedianPixelList *pixel_list)
{
  MagickPixelPacket
    pixel;

  register long
    channel;

  register MedianSkipList
    *list;

  unsigned long
    center,
    color,
    count;

  unsigned short
    channels[MedianListChannels];

  /*
    Find the median value for each of the color.
  */
  center=pixel_list->center;
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536UL;
    count=0;
    do
    {
      color=list->nodes[color].next[0];
      count+=list->nodes[color].count;
    }
    while (count <= center);
    channels[channel]=(unsigned short) color;
  }
  GetMagickPixelPacket((const Image *) NULL,&pixel);
  pixel.red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel.green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel.blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel.opacity=(MagickRealType) ScaleShortToQuantum(channels[3]);
  pixel.index=(MagickRealType) ScaleShortToQuantum(channels[4]);
  return(pixel);
}

static inline void InsertMedianPixelList(const Image *image,
  const PixelPacket *pixel,const IndexPacket *indexes,
  MedianPixelList *pixel_list)
{
  unsigned long
    signature;

  unsigned short
    index;

  index=ScaleQuantumToShort(pixel->red);
  signature=pixel_list->lists[0].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[0].nodes[index].count++;
  else
    AddNodeMedianPixelList(pixel_list,0,index);
  index=ScaleQuantumToShort(pixel->green);
  signature=pixel_list->lists[1].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[1].nodes[index].count++;
  else
    AddNodeMedianPixelList(pixel_list,1,index);
  index=ScaleQuantumToShort(pixel->blue);
  signature=pixel_list->lists[2].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[2].nodes[index].count++;
  else
    AddNodeMedianPixelList(pixel_list,2,index);
  index=ScaleQuantumToShort(pixel->opacity);
  signature=pixel_list->lists[3].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[3].nodes[index].count++;
  else
    AddNodeMedianPixelList(pixel_list,3,index);
  if (image->colorspace == CMYKColorspace)
    index=ScaleQuantumToShort(*indexes);
  signature=pixel_list->lists[4].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[4].nodes[index].count++;
  else
    AddNodeMedianPixelList(pixel_list,4,index);
}

static void ResetMedianPixelList(MedianPixelList *pixel_list)
{
  int
    level;

  register long
    channel;

  register MedianListNode
    *root;

  register MedianSkipList
    *list;

  /*
    Reset the skip-list.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    root=list->nodes+65536UL;
    list->level=0;
    for (level=0; level < 9; level++)
      root->next[level]=65536UL;
  }
  pixel_list->seed=pixel_list->signature++;
}

MagickExport Image *MedianFilterImage(const Image *image,const double radius,
  ExceptionInfo *exception)
{
#define MedianFilterImageTag  "MedianFilter/Image"

  CacheView
    *image_view,
    *median_view;

  Image
    *median_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MedianPixelList
    **restrict pixel_list;

  unsigned long
    width;

  /*
    Initialize median image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth2D(radius,0.5);
  if ((image->columns < width) || (image->rows < width))
    ThrowImageException(OptionError,"ImageSmallerThanKernelRadius");
  median_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (median_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(median_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&median_image->exception);
      median_image=DestroyImage(median_image);
      return((Image *) NULL);
    }
  pixel_list=AcquireMedianPixelListThreadSet(width);
  if (pixel_list == (MedianPixelList **) NULL)
    {
      median_image=DestroyImage(median_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Median filter each image row.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  median_view=AcquireCacheView(median_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) median_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict median_indexes;

    register long
      id,
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((long) width/2L),y-(long) (width/
      2L),image->columns+width,width,exception);
    q=QueueCacheViewAuthenticPixels(median_view,0,y,median_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    median_indexes=GetCacheViewAuthenticIndexQueue(median_view);
    id=GetOpenMPThreadId();
    for (x=0; x < (long) median_image->columns; x++)
    {
      MagickPixelPacket
        pixel;

      register const PixelPacket
        *restrict r;

      register const IndexPacket
        *restrict s;

      register long
        u,
        v;

      r=p;
      s=indexes+x;
      ResetMedianPixelList(pixel_list[id]);
      for (v=0; v < (long) width; v++)
      {
        for (u=0; u < (long) width; u++)
          InsertMedianPixelList(image,r+u,s+u,pixel_list[id]);
        r+=image->columns+width;
        s+=image->columns+width;
      }
      pixel=GetMedianPixelList(pixel_list[id]);
      SetPixelPacket(median_image,&pixel,q,median_indexes+x);
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(median_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_MedianFilterImage)
#endif
        proceed=SetImageProgress(image,MedianFilterImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  median_view=DestroyCacheView(median_view);
  image_view=DestroyCacheView(image_view);
  pixel_list=DestroyMedianPixelListThreadSet(pixel_list);
  return(median_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M o t i o n B l u r I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MotionBlurImage() simulates motion blur.  We convolve the image with a
%  Gaussian operator of the given radius and standard deviation (sigma).
%  For reasonable results, radius should be larger than sigma.  Use a
%  radius of 0 and MotionBlurImage() selects a suitable radius for you.
%  Angle gives the angle of the blurring motion.
%
%  Andrew Protano contributed this effect.
%
%  The format of the MotionBlurImage method is:
%
%    Image *MotionBlurImage(const Image *image,const double radius,
%      const double sigma,const double angle,ExceptionInfo *exception)
%    Image *MotionBlurImageChannel(const Image *image,const ChannelType channel,
%      const double radius,const double sigma,const double angle,
%      ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%    o radius: the radius of the Gaussian, in pixels, not counting
%      the center pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o angle: Apply the effect along this angle.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static double *GetMotionBlurKernel(const unsigned long width,const double sigma)
{
  double
    *kernel,
    normalize;

  register long
    i;

  /*
   Generate a 1-D convolution kernel.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  kernel=(double *) AcquireQuantumMemory((size_t) width,sizeof(*kernel));
  if (kernel == (double *) NULL)
    return(kernel);
  normalize=0.0;
  for (i=0; i < (long) width; i++)
  {
    kernel[i]=exp((-((double) i*i)/(double) (2.0*MagickSigma*MagickSigma)))/
      (MagickSQ2PI*MagickSigma);
    normalize+=kernel[i];
  }
  for (i=0; i < (long) width; i++)
    kernel[i]/=normalize;
  return(kernel);
}

MagickExport Image *MotionBlurImage(const Image *image,const double radius,
  const double sigma,const double angle,ExceptionInfo *exception)
{
  Image
    *motion_blur;

  motion_blur=MotionBlurImageChannel(image,DefaultChannels,radius,sigma,angle,
    exception);
  return(motion_blur);
}

MagickExport Image *MotionBlurImageChannel(const Image *image,
  const ChannelType channel,const double radius,const double sigma,
  const double angle,ExceptionInfo *exception)
{
  CacheView
    *blur_view,
    *image_view;

  double
    *kernel;

  Image
    *blur_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    bias;

  OffsetInfo
    *offset;

  PointInfo
    point;

  register long
    i;

  unsigned long
    width;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  width=GetOptimalKernelWidth1D(radius,sigma);
  kernel=GetMotionBlurKernel(width,sigma);
  if (kernel == (double *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  offset=(OffsetInfo *) AcquireQuantumMemory(width,sizeof(*offset));
  if (offset == (OffsetInfo *) NULL)
    {
      kernel=(double *) RelinquishMagickMemory(kernel);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  blur_image=CloneImage(image,0,0,MagickTrue,exception);
  if (blur_image == (Image *) NULL)
    {
      kernel=(double *) RelinquishMagickMemory(kernel);
      offset=(OffsetInfo *) RelinquishMagickMemory(offset);
      return((Image *) NULL);
    }
  if (SetImageStorageClass(blur_image,DirectClass) == MagickFalse)
    {
      kernel=(double *) RelinquishMagickMemory(kernel);
      offset=(OffsetInfo *) RelinquishMagickMemory(offset);
      InheritException(exception,&blur_image->exception);
      blur_image=DestroyImage(blur_image);
      return((Image *) NULL);
    }
  point.x=(double) width*sin(DegreesToRadians(angle));
  point.y=(double) width*cos(DegreesToRadians(angle));
  for (i=0; i < (long) width; i++)
  {
    offset[i].x=(long) ((i*point.y)/hypot(point.x,point.y)+0.5);
    offset[i].y=(long) ((i*point.x)/hypot(point.x,point.y)+0.5);
  }
  /*
    Motion blur image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&bias);
  image_view=AcquireCacheView(image);
  blur_view=AcquireCacheView(blur_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *restrict blur_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    blur_indexes=GetCacheViewAuthenticIndexQueue(blur_view);
    for (x=0; x < (long) image->columns; x++)
    {
      MagickPixelPacket
        qixel;

      PixelPacket
        pixel;

      register double
        *restrict k;

      register long
        i;

      register const IndexPacket
        *restrict indexes;

      k=kernel;
      qixel=bias;
      if (((channel & OpacityChannel) == 0) || (image->matte == MagickFalse))
        {
          for (i=0; i < (long) width; i++)
          {
            (void) GetOneCacheViewVirtualPixel(image_view,x+offset[i].x,y+
              offset[i].y,&pixel,exception);
            qixel.red+=(*k)*pixel.red;
            qixel.green+=(*k)*pixel.green;
            qixel.blue+=(*k)*pixel.blue;
            qixel.opacity+=(*k)*pixel.opacity;
            if (image->colorspace == CMYKColorspace)
              {
                indexes=GetCacheViewVirtualIndexQueue(image_view);
                qixel.index+=(*k)*(*indexes);
              }
            k++;
          }
          if ((channel & RedChannel) != 0)
            q->red=ClampToQuantum(qixel.red);
          if ((channel & GreenChannel) != 0)
            q->green=ClampToQuantum(qixel.green);
          if ((channel & BlueChannel) != 0)
            q->blue=ClampToQuantum(qixel.blue);
          if ((channel & OpacityChannel) != 0)
            q->opacity=ClampToQuantum(qixel.opacity);
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            blur_indexes[x]=(IndexPacket) ClampToQuantum(qixel.index);
        }
      else
        {
          MagickRealType
            alpha,
            gamma;

          alpha=0.0;
          gamma=0.0;
          for (i=0; i < (long) width; i++)
          {
            (void) GetOneCacheViewVirtualPixel(image_view,x+offset[i].x,y+
              offset[i].y,&pixel,exception);
            alpha=(MagickRealType) (QuantumScale*GetAlphaPixelComponent(&pixel));
            qixel.red+=(*k)*alpha*pixel.red;
            qixel.green+=(*k)*alpha*pixel.green;
            qixel.blue+=(*k)*alpha*pixel.blue;
            qixel.opacity+=(*k)*pixel.opacity;
            if (image->colorspace == CMYKColorspace)
              {
                indexes=GetCacheViewVirtualIndexQueue(image_view);
                qixel.index+=(*k)*alpha*(*indexes);
              }
            gamma+=(*k)*alpha;
            k++;
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          if ((channel & RedChannel) != 0)
            q->red=ClampToQuantum(gamma*qixel.red);
          if ((channel & GreenChannel) != 0)
            q->green=ClampToQuantum(gamma*qixel.green);
          if ((channel & BlueChannel) != 0)
            q->blue=ClampToQuantum(gamma*qixel.blue);
          if ((channel & OpacityChannel) != 0)
            q->opacity=ClampToQuantum(qixel.opacity);
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            blur_indexes[x]=(IndexPacket) ClampToQuantum(gamma*qixel.index);
        }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(blur_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_MotionBlurImageChannel)
#endif
        proceed=SetImageProgress(image,BlurImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  blur_view=DestroyCacheView(blur_view);
  image_view=DestroyCacheView(image_view);
  kernel=(double *) RelinquishMagickMemory(kernel);
  offset=(OffsetInfo *) RelinquishMagickMemory(offset);
  if (status == MagickFalse)
    blur_image=DestroyImage(blur_image);
  return(blur_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     P r e v i e w I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PreviewImage() tiles 9 thumbnails of the specified image with an image
%  processing operation applied with varying parameters.  This may be helpful
%  pin-pointing an appropriate parameter for a particular image processing
%  operation.
%
%  The format of the PreviewImages method is:
%
%      Image *PreviewImages(const Image *image,const PreviewType preview,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o preview: the image processing operation.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *PreviewImage(const Image *image,const PreviewType preview,
  ExceptionInfo *exception)
{
#define NumberTiles  9
#define PreviewImageTag  "Preview/Image"
#define DefaultPreviewGeometry  "204x204+10+10"

  char
    factor[MaxTextExtent],
    label[MaxTextExtent];

  double
    degrees,
    gamma,
    percentage,
    radius,
    sigma,
    threshold;

  Image
    *images,
    *montage_image,
    *preview_image,
    *thumbnail;

  ImageInfo
    *preview_info;

  long
    y;

  MagickBooleanType
    proceed;

  MontageInfo
    *montage_info;

  QuantizeInfo
    quantize_info;

  RectangleInfo
    geometry;

  register long
    i,
    x;

  unsigned long
    colors;

  /*
    Open output image file.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  colors=2;
  degrees=0.0;
  gamma=(-0.2f);
  preview_info=AcquireImageInfo();
  SetGeometry(image,&geometry);
  (void) ParseMetaGeometry(DefaultPreviewGeometry,&geometry.x,&geometry.y,
    &geometry.width,&geometry.height);
  images=NewImageList();
  percentage=12.5;
  GetQuantizeInfo(&quantize_info);
  radius=0.0;
  sigma=1.0;
  threshold=0.0;
  x=0;
  y=0;
  for (i=0; i < NumberTiles; i++)
  {
    thumbnail=ThumbnailImage(image,geometry.width,geometry.height,exception);
    if (thumbnail == (Image *) NULL)
      break;
    (void) SetImageProgressMonitor(thumbnail,(MagickProgressMonitor) NULL,
      (void *) NULL);
    (void) SetImageProperty(thumbnail,"label",DefaultTileLabel);
    if (i == (NumberTiles/2))
      {
        (void) QueryColorDatabase("#dfdfdf",&thumbnail->matte_color,exception);
        AppendImageToList(&images,thumbnail);
        continue;
      }
    switch (preview)
    {
      case RotatePreview:
      {
        degrees+=45.0;
        preview_image=RotateImage(thumbnail,degrees,exception);
        (void) FormatMagickString(label,MaxTextExtent,"rotate %g",degrees);
        break;
      }
      case ShearPreview:
      {
        degrees+=5.0;
        preview_image=ShearImage(thumbnail,degrees,degrees,exception);
        (void) FormatMagickString(label,MaxTextExtent,"shear %gx%g",
          degrees,2.0*degrees);
        break;
      }
      case RollPreview:
      {
        x=(long) ((i+1)*thumbnail->columns)/NumberTiles;
        y=(long) ((i+1)*thumbnail->rows)/NumberTiles;
        preview_image=RollImage(thumbnail,x,y,exception);
        (void) FormatMagickString(label,MaxTextExtent,"roll %ldx%ld",x,y);
        break;
      }
      case HuePreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) FormatMagickString(factor,MaxTextExtent,"100,100,%g",
          2.0*percentage);
        (void) ModulateImage(preview_image,factor);
        (void) FormatMagickString(label,MaxTextExtent,"modulate %s",factor);
        break;
      }
      case SaturationPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) FormatMagickString(factor,MaxTextExtent,"100,%g",
          2.0*percentage);
        (void) ModulateImage(preview_image,factor);
        (void) FormatMagickString(label,MaxTextExtent,"modulate %s",factor);
        break;
      }
      case BrightnessPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) FormatMagickString(factor,MaxTextExtent,"%g",2.0*percentage);
        (void) ModulateImage(preview_image,factor);
        (void) FormatMagickString(label,MaxTextExtent,"modulate %s",factor);
        break;
      }
      case GammaPreview:
      default:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        gamma+=0.4f;
        (void) GammaImageChannel(preview_image,DefaultChannels,gamma);
        (void) FormatMagickString(label,MaxTextExtent,"gamma %g",gamma);
        break;
      }
      case SpiffPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image != (Image *) NULL)
          for (x=0; x < i; x++)
            (void) ContrastImage(preview_image,MagickTrue);
        (void) FormatMagickString(label,MaxTextExtent,"contrast (%ld)",i+1);
        break;
      }
      case DullPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        for (x=0; x < i; x++)
          (void) ContrastImage(preview_image,MagickFalse);
        (void) FormatMagickString(label,MaxTextExtent,"+contrast (%ld)",i+1);
        break;
      }
      case GrayscalePreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        colors<<=1;
        quantize_info.number_colors=colors;
        quantize_info.colorspace=GRAYColorspace;
        (void) QuantizeImage(&quantize_info,preview_image);
        (void) FormatMagickString(label,MaxTextExtent,
          "-colorspace gray -colors %ld",colors);
        break;
      }
      case QuantizePreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        colors<<=1;
        quantize_info.number_colors=colors;
        (void) QuantizeImage(&quantize_info,preview_image);
        (void) FormatMagickString(label,MaxTextExtent,"colors %ld",colors);
        break;
      }
      case DespecklePreview:
      {
        for (x=0; x < (i-1); x++)
        {
          preview_image=DespeckleImage(thumbnail,exception);
          if (preview_image == (Image *) NULL)
            break;
          thumbnail=DestroyImage(thumbnail);
          thumbnail=preview_image;
        }
        preview_image=DespeckleImage(thumbnail,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) FormatMagickString(label,MaxTextExtent,"despeckle (%ld)",i+1);
        break;
      }
      case ReduceNoisePreview:
      {
        preview_image=ReduceNoiseImage(thumbnail,radius,exception);
        (void) FormatMagickString(label,MaxTextExtent,"noise %g",radius);
        break;
      }
      case AddNoisePreview:
      {
        switch ((int) i)
        {
          case 0:
          {
            (void) CopyMagickString(factor,"uniform",MaxTextExtent);
            break;
          }
          case 1:
          {
            (void) CopyMagickString(factor,"gaussian",MaxTextExtent);
            break;
          }
          case 2:
          {
            (void) CopyMagickString(factor,"multiplicative",MaxTextExtent);
            break;
          }
          case 3:
          {
            (void) CopyMagickString(factor,"impulse",MaxTextExtent);
            break;
          }
          case 4:
          {
            (void) CopyMagickString(factor,"laplacian",MaxTextExtent);
            break;
          }
          case 5:
          {
            (void) CopyMagickString(factor,"Poisson",MaxTextExtent);
            break;
          }
          default:
          {
            (void) CopyMagickString(thumbnail->magick,"NULL",MaxTextExtent);
            break;
          }
        }
        preview_image=ReduceNoiseImage(thumbnail,(double) i,exception);
        (void) FormatMagickString(label,MaxTextExtent,"+noise %s",factor);
        break;
      }
      case SharpenPreview:
      {
        preview_image=SharpenImage(thumbnail,radius,sigma,exception);
        (void) FormatMagickString(label,MaxTextExtent,"sharpen %gx%g",
          radius,sigma);
        break;
      }
      case BlurPreview:
      {
        preview_image=BlurImage(thumbnail,radius,sigma,exception);
        (void) FormatMagickString(label,MaxTextExtent,"blur %gx%g",radius,
          sigma);
        break;
      }
      case ThresholdPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) BilevelImage(thumbnail,
          (double) (percentage*((MagickRealType) QuantumRange+1.0))/100.0);
        (void) FormatMagickString(label,MaxTextExtent,"threshold %g",
          (double) (percentage*((MagickRealType) QuantumRange+1.0))/100.0);
        break;
      }
      case EdgeDetectPreview:
      {
        preview_image=EdgeImage(thumbnail,radius,exception);
        (void) FormatMagickString(label,MaxTextExtent,"edge %g",radius);
        break;
      }
      case SpreadPreview:
      {
        preview_image=SpreadImage(thumbnail,radius,exception);
        (void) FormatMagickString(label,MaxTextExtent,"spread %g",
          radius+0.5);
        break;
      }
      case SolarizePreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) SolarizeImage(preview_image,(double) QuantumRange*
          percentage/100.0);
        (void) FormatMagickString(label,MaxTextExtent,"solarize %g",
          (QuantumRange*percentage)/100.0);
        break;
      }
      case ShadePreview:
      {
        degrees+=10.0;
        preview_image=ShadeImage(thumbnail,MagickTrue,degrees,degrees,
          exception);
        (void) FormatMagickString(label,MaxTextExtent,"shade %gx%g",
          degrees,degrees);
        break;
      }
      case RaisePreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        geometry.width=(unsigned long) (2*i+2);
        geometry.height=(unsigned long) (2*i+2);
        geometry.x=i/2;
        geometry.y=i/2;
        (void) RaiseImage(preview_image,&geometry,MagickTrue);
        (void) FormatMagickString(label,MaxTextExtent,"raise %lux%lu%+ld%+ld",
          geometry.width,geometry.height,geometry.x,geometry.y);
        break;
      }
      case SegmentPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        threshold+=0.4f;
        (void) SegmentImage(preview_image,RGBColorspace,MagickFalse,threshold,
          threshold);
        (void) FormatMagickString(label,MaxTextExtent,"segment %gx%g",
          threshold,threshold);
        break;
      }
      case SwirlPreview:
      {
        preview_image=SwirlImage(thumbnail,degrees,exception);
        (void) FormatMagickString(label,MaxTextExtent,"swirl %g",degrees);
        degrees+=45.0;
        break;
      }
      case ImplodePreview:
      {
        degrees+=0.1f;
        preview_image=ImplodeImage(thumbnail,degrees,exception);
        (void) FormatMagickString(label,MaxTextExtent,"implode %g",degrees);
        break;
      }
      case WavePreview:
      {
        degrees+=5.0f;
        preview_image=WaveImage(thumbnail,0.5*degrees,2.0*degrees,exception);
        (void) FormatMagickString(label,MaxTextExtent,"wave %gx%g",
          0.5*degrees,2.0*degrees);
        break;
      }
      case OilPaintPreview:
      {
        preview_image=OilPaintImage(thumbnail,(double) radius,exception);
        (void) FormatMagickString(label,MaxTextExtent,"paint %g",radius);
        break;
      }
      case CharcoalDrawingPreview:
      {
        preview_image=CharcoalImage(thumbnail,(double) radius,(double) sigma,
          exception);
        (void) FormatMagickString(label,MaxTextExtent,"charcoal %gx%g",
          radius,sigma);
        break;
      }
      case JPEGPreview:
      {
        char
          filename[MaxTextExtent];

        int
          file;

        MagickBooleanType
          status;

        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        preview_info->quality=(unsigned long) percentage;
        (void) FormatMagickString(factor,MaxTextExtent,"%lu",
          preview_info->quality);
        file=AcquireUniqueFileResource(filename);
        if (file != -1)
          file=close(file)-1;
        (void) FormatMagickString(preview_image->filename,MaxTextExtent,
          "jpeg:%s",filename);
        status=WriteImage(preview_info,preview_image);
        if (status != MagickFalse)
          {
            Image
              *quality_image;

            (void) CopyMagickString(preview_info->filename,
              preview_image->filename,MaxTextExtent);
            quality_image=ReadImage(preview_info,exception);
            if (quality_image != (Image *) NULL)
              {
                preview_image=DestroyImage(preview_image);
                preview_image=quality_image;
              }
          }
        (void) RelinquishUniqueFileResource(preview_image->filename);
        if ((GetBlobSize(preview_image)/1024) >= 1024)
          (void) FormatMagickString(label,MaxTextExtent,"quality %s\n%gmb ",
            factor,(double) ((MagickOffsetType) GetBlobSize(preview_image))/
            1024.0/1024.0);
        else
          if (GetBlobSize(preview_image) >= 1024)
            (void) FormatMagickString(label,MaxTextExtent,
              "quality %s\n%gkb ",factor,(double) ((MagickOffsetType)
              GetBlobSize(preview_image))/1024.0);
          else
            (void) FormatMagickString(label,MaxTextExtent,"quality %s\n%lub ",
              factor,(unsigned long) GetBlobSize(thumbnail));
        break;
      }
    }
    thumbnail=DestroyImage(thumbnail);
    percentage+=12.5;
    radius+=0.5;
    sigma+=0.25;
    if (preview_image == (Image *) NULL)
      break;
    (void) DeleteImageProperty(preview_image,"label");
    (void) SetImageProperty(preview_image,"label",label);
    AppendImageToList(&images,preview_image);
    proceed=SetImageProgress(image,PreviewImageTag,i,NumberTiles);
    if (proceed == MagickFalse)
      break;
  }
  if (images == (Image *) NULL)
    {
      preview_info=DestroyImageInfo(preview_info);
      return((Image *) NULL);
    }
  /*
    Create the montage.
  */
  montage_info=CloneMontageInfo(preview_info,(MontageInfo *) NULL);
  (void) CopyMagickString(montage_info->filename,image->filename,MaxTextExtent);
  montage_info->shadow=MagickTrue;
  (void) CloneString(&montage_info->tile,"3x3");
  (void) CloneString(&montage_info->geometry,DefaultPreviewGeometry);
  (void) CloneString(&montage_info->frame,DefaultTileFrame);
  montage_image=MontageImages(images,montage_info,exception);
  montage_info=DestroyMontageInfo(montage_info);
  images=DestroyImageList(images);
  if (montage_image == (Image *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  if (montage_image->montage != (char *) NULL)
    {
      /*
        Free image directory.
      */
      montage_image->montage=(char *) RelinquishMagickMemory(
        montage_image->montage);
      if (image->directory != (char *) NULL)
        montage_image->directory=(char *) RelinquishMagickMemory(
          montage_image->directory);
    }
  preview_info=DestroyImageInfo(preview_info);
  return(montage_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     R a d i a l B l u r I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RadialBlurImage() applies a radial blur to the image.
%
%  Andrew Protano contributed this effect.
%
%  The format of the RadialBlurImage method is:
%
%    Image *RadialBlurImage(const Image *image,const double angle,
%      ExceptionInfo *exception)
%    Image *RadialBlurImageChannel(const Image *image,const ChannelType channel,
%      const double angle,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o angle: the angle of the radial blur.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image *RadialBlurImage(const Image *image,const double angle,
  ExceptionInfo *exception)
{
  Image
    *blur_image;

  blur_image=RadialBlurImageChannel(image,DefaultChannels,angle,exception);
  return(blur_image);
}

MagickExport Image *RadialBlurImageChannel(const Image *image,
  const ChannelType channel,const double angle,ExceptionInfo *exception)
{
  CacheView
    *blur_view,
    *image_view;

  Image
    *blur_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    bias;

  MagickRealType
    blur_radius,
    *cos_theta,
    offset,
    *sin_theta,
    theta;

  PointInfo
    blur_center;

  register long
    i;

  unsigned long
    n;

  /*
    Allocate blur image.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  blur_image=CloneImage(image,0,0,MagickTrue,exception);
  if (blur_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(blur_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&blur_image->exception);
      blur_image=DestroyImage(blur_image);
      return((Image *) NULL);
    }
  blur_center.x=(double) image->columns/2.0;
  blur_center.y=(double) image->rows/2.0;
  blur_radius=hypot(blur_center.x,blur_center.y);
  n=(unsigned long) fabs(4.0*DegreesToRadians(angle)*sqrt((double) blur_radius)+
    2UL);
  theta=DegreesToRadians(angle)/(MagickRealType) (n-1);
  cos_theta=(MagickRealType *) AcquireQuantumMemory((size_t) n,
    sizeof(*cos_theta));
  sin_theta=(MagickRealType *) AcquireQuantumMemory((size_t) n,
    sizeof(*sin_theta));
  if ((cos_theta == (MagickRealType *) NULL) ||
      (sin_theta == (MagickRealType *) NULL))
    {
      blur_image=DestroyImage(blur_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  offset=theta*(MagickRealType) (n-1)/2.0;
  for (i=0; i < (long) n; i++)
  {
    cos_theta[i]=cos((double) (theta*i-offset));
    sin_theta[i]=sin((double) (theta*i-offset));
  }
  /*
    Radial blur image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&bias);
  image_view=AcquireCacheView(image);
  blur_view=AcquireCacheView(blur_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) blur_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register IndexPacket
      *restrict blur_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    blur_indexes=GetCacheViewAuthenticIndexQueue(blur_view);
    for (x=0; x < (long) blur_image->columns; x++)
    {
      MagickPixelPacket
        qixel;

      MagickRealType
        normalize,
        radius;

      PixelPacket
        pixel;

      PointInfo
        center;

      register long
        i;

      unsigned long
        step;

      center.x=(double) x-blur_center.x;
      center.y=(double) y-blur_center.y;
      radius=hypot((double) center.x,center.y);
      if (radius == 0)
        step=1;
      else
        {
          step=(unsigned long) (blur_radius/radius);
          if (step == 0)
            step=1;
          else
            if (step >= n)
              step=n-1;
        }
      normalize=0.0;
      qixel=bias;
      if (((channel & OpacityChannel) == 0) || (image->matte == MagickFalse))
        {
          for (i=0; i < (long) n; i+=step)
          {
            (void) GetOneCacheViewVirtualPixel(image_view,(long) (blur_center.x+
              center.x*cos_theta[i]-center.y*sin_theta[i]+0.5),(long) (
              blur_center.y+center.x*sin_theta[i]+center.y*cos_theta[i]+0.5),
              &pixel,exception);
            qixel.red+=pixel.red;
            qixel.green+=pixel.green;
            qixel.blue+=pixel.blue;
            qixel.opacity+=pixel.opacity;
            if (image->colorspace == CMYKColorspace)
              {
                indexes=GetCacheViewVirtualIndexQueue(image_view);
                qixel.index+=(*indexes);
              }
            normalize+=1.0;
          }
          normalize=1.0/(fabs((double) normalize) <= MagickEpsilon ? 1.0 :
            normalize);
          if ((channel & RedChannel) != 0)
            q->red=ClampToQuantum(normalize*qixel.red);
          if ((channel & GreenChannel) != 0)
            q->green=ClampToQuantum(normalize*qixel.green);
          if ((channel & BlueChannel) != 0)
            q->blue=ClampToQuantum(normalize*qixel.blue);
          if ((channel & OpacityChannel) != 0)
            q->opacity=ClampToQuantum(normalize*qixel.opacity);
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            blur_indexes[x]=(IndexPacket) ClampToQuantum(normalize*qixel.index);
        }
      else
        {
          MagickRealType
            alpha,
            gamma;

          alpha=1.0;
          gamma=0.0;
          for (i=0; i < (long) n; i+=step)
          {
            (void) GetOneCacheViewVirtualPixel(image_view,(long) (blur_center.x+
              center.x*cos_theta[i]-center.y*sin_theta[i]+0.5),(long) (
              blur_center.y+center.x*sin_theta[i]+center.y*cos_theta[i]+0.5),
              &pixel,exception);
            alpha=(MagickRealType) (QuantumScale*
              GetAlphaPixelComponent(&pixel));
            qixel.red+=alpha*pixel.red;
            qixel.green+=alpha*pixel.green;
            qixel.blue+=alpha*pixel.blue;
            qixel.opacity+=pixel.opacity;
            if (image->colorspace == CMYKColorspace)
              {
                indexes=GetCacheViewVirtualIndexQueue(image_view);
                qixel.index+=alpha*(*indexes);
              }
            gamma+=alpha;
            normalize+=1.0;
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          normalize=1.0/(fabs((double) normalize) <= MagickEpsilon ? 1.0 :
            normalize);
          if ((channel & RedChannel) != 0)
            q->red=ClampToQuantum(gamma*qixel.red);
          if ((channel & GreenChannel) != 0)
            q->green=ClampToQuantum(gamma*qixel.green);
          if ((channel & BlueChannel) != 0)
            q->blue=ClampToQuantum(gamma*qixel.blue);
          if ((channel & OpacityChannel) != 0)
            q->opacity=ClampToQuantum(normalize*qixel.opacity);
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            blur_indexes[x]=(IndexPacket) ClampToQuantum(gamma*qixel.index);
        }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(blur_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_RadialBlurImageChannel)
#endif
        proceed=SetImageProgress(image,BlurImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  blur_view=DestroyCacheView(blur_view);
  image_view=DestroyCacheView(image_view);
  cos_theta=(MagickRealType *) RelinquishMagickMemory(cos_theta);
  sin_theta=(MagickRealType *) RelinquishMagickMemory(sin_theta);
  if (status == MagickFalse)
    blur_image=DestroyImage(blur_image);
  return(blur_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     R e d u c e N o i s e I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReduceNoiseImage() smooths the contours of an image while still preserving
%  edge information.  The algorithm works by replacing each pixel with its
%  neighbor closest in value.  A neighbor is defined by radius.  Use a radius
%  of 0 and ReduceNoise() selects a suitable radius for you.
%
%  The format of the ReduceNoiseImage method is:
%
%      Image *ReduceNoiseImage(const Image *image,const double radius,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the pixel neighborhood.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickPixelPacket GetNonpeakMedianPixelList(MedianPixelList *pixel_list)
{
  MagickPixelPacket
    pixel;

  register long
    channel;

  register MedianSkipList
    *list;

  unsigned long
    center,
    color,
    count,
    previous,
    next;

  unsigned short
    channels[5];

  /*
    Finds the median value for each of the color.
  */
  center=pixel_list->center;
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536UL;
    next=list->nodes[color].next[0];
    count=0;
    do
    {
      previous=color;
      color=next;
      next=list->nodes[color].next[0];
      count+=list->nodes[color].count;
    }
    while (count <= center);
    if ((previous == 65536UL) && (next != 65536UL))
      color=next;
    else
      if ((previous != 65536UL) && (next == 65536UL))
        color=previous;
    channels[channel]=(unsigned short) color;
  }
  GetMagickPixelPacket((const Image *) NULL,&pixel);
  pixel.red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel.green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel.blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel.opacity=(MagickRealType) ScaleShortToQuantum(channels[3]);
  pixel.index=(MagickRealType) ScaleShortToQuantum(channels[4]);
  return(pixel);
}

MagickExport Image *ReduceNoiseImage(const Image *image,const double radius,
  ExceptionInfo *exception)
{
#define ReduceNoiseImageTag  "ReduceNoise/Image"

  CacheView
    *image_view,
    *noise_view;

  Image
    *noise_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MedianPixelList
    **restrict pixel_list;

  unsigned long
    width;

  /*
    Initialize noise image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth2D(radius,0.5);
  if ((image->columns < width) || (image->rows < width))
    ThrowImageException(OptionError,"ImageSmallerThanKernelRadius");
  noise_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (noise_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(noise_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&noise_image->exception);
      noise_image=DestroyImage(noise_image);
      return((Image *) NULL);
    }
  pixel_list=AcquireMedianPixelListThreadSet(width);
  if (pixel_list == (MedianPixelList **) NULL)
    {
      noise_image=DestroyImage(noise_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Reduce noise image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  noise_view=AcquireCacheView(noise_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) noise_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict noise_indexes;

    register long
      id,
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((long) width/2L),y-(long) (width/
      2L),image->columns+width,width,exception);
    q=QueueCacheViewAuthenticPixels(noise_view,0,y,noise_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    noise_indexes=GetCacheViewAuthenticIndexQueue(noise_view);
    id=GetOpenMPThreadId();
    for (x=0; x < (long) noise_image->columns; x++)
    {
      MagickPixelPacket
        pixel;

      register const PixelPacket
        *restrict r;

      register const IndexPacket
        *restrict s;

      register long
        u,
        v;

      r=p;
      s=indexes+x;
      ResetMedianPixelList(pixel_list[id]);
      for (v=0; v < (long) width; v++)
      {
        for (u=0; u < (long) width; u++)
          InsertMedianPixelList(image,r+u,s+u,pixel_list[id]);
        r+=image->columns+width;
        s+=image->columns+width;
      }
      pixel=GetNonpeakMedianPixelList(pixel_list[id]);
      SetPixelPacket(noise_image,&pixel,q,noise_indexes+x);
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(noise_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ReduceNoiseImage)
#endif
        proceed=SetImageProgress(image,ReduceNoiseImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  noise_view=DestroyCacheView(noise_view);
  image_view=DestroyCacheView(image_view);
  pixel_list=DestroyMedianPixelListThreadSet(pixel_list);
  return(noise_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S e l e c t i v e B l u r I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SelectiveBlurImage() selectively blur pixels within a contrast threshold.
%  It is similar to the unsharpen mask that sharpens everything with contrast
%  above a certain threshold.
%
%  The format of the SelectiveBlurImage method is:
%
%      Image *SelectiveBlurImage(const Image *image,const double radius,
%        const double sigma,const double threshold,ExceptionInfo *exception)
%      Image *SelectiveBlurImageChannel(const Image *image,
%        const ChannelType channel,const double radius,const double sigma,
%        const double threshold,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o threshold: only pixels within this contrast threshold are included
%      in the blur operation.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline MagickBooleanType SelectiveContrast(const PixelPacket *p,
  const PixelPacket *q,const double threshold)
{
  if (fabs(PixelIntensity(p)-PixelIntensity(q)) < threshold)
    return(MagickTrue);
  return(MagickFalse);
}

MagickExport Image *SelectiveBlurImage(const Image *image,const double radius,
  const double sigma,const double threshold,ExceptionInfo *exception)
{
  Image
    *blur_image;

  blur_image=SelectiveBlurImageChannel(image,DefaultChannels,radius,sigma,
    threshold,exception);
  return(blur_image);
}

MagickExport Image *SelectiveBlurImageChannel(const Image *image,
  const ChannelType channel,const double radius,const double sigma,
  const double threshold,ExceptionInfo *exception)
{
#define SelectiveBlurImageTag  "SelectiveBlur/Image"

  CacheView
    *blur_view,
    *image_view;

  double
    *kernel;

  Image
    *blur_image;

  long
    j,
    progress,
    u,
    v,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    bias;

  register long
    i;

  unsigned long
    width;

  /*
    Initialize blur image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth1D(radius,sigma);
  kernel=(double *) AcquireQuantumMemory((size_t) width,width*sizeof(*kernel));
  if (kernel == (double *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  j=(long) width/2;
  i=0;
  for (v=(-j); v <= j; v++)
  {
    for (u=(-j); u <= j; u++)
      kernel[i++]=exp(-((double) u*u+v*v)/(2.0*MagickSigma*MagickSigma))/
        (2.0*MagickPI*MagickSigma*MagickSigma);
  }
  if (image->debug != MagickFalse)
    {
      char
        format[MaxTextExtent],
        *message;

      long
        u,
        v;

      register const double
        *k;

      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  SelectiveBlurImage with %ldx%ld kernel:",width,width);
      message=AcquireString("");
      k=kernel;
      for (v=0; v < (long) width; v++)
      {
        *message='\0';
        (void) FormatMagickString(format,MaxTextExtent,"%ld: ",v);
        (void) ConcatenateString(&message,format);
        for (u=0; u < (long) width; u++)
        {
          (void) FormatMagickString(format,MaxTextExtent,"%+f ",*k++);
          (void) ConcatenateString(&message,format);
        }
        (void) LogMagickEvent(TransformEvent,GetMagickModule(),"%s",message);
      }
      message=DestroyString(message);
    }
  blur_image=CloneImage(image,0,0,MagickTrue,exception);
  if (blur_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(blur_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&blur_image->exception);
      blur_image=DestroyImage(blur_image);
      return((Image *) NULL);
    }
  /*
    Threshold blur image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&bias);
  SetMagickPixelPacketBias(image,&bias);
  image_view=AcquireCacheView(image);
  blur_view=AcquireCacheView(blur_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickBooleanType
      sync;

    MagickRealType
      gamma;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict blur_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((long) width/2L),y-(long) (width/
      2L),image->columns+width,width,exception);
    q=GetCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    blur_indexes=GetCacheViewAuthenticIndexQueue(blur_view);
    for (x=0; x < (long) image->columns; x++)
    {
      long
        j,
        v;

      MagickPixelPacket
        pixel;

      register const double
        *restrict k;

      register long
        u;

      pixel=bias;
      k=kernel;
      gamma=0.0;
      j=0;
      if (((channel & OpacityChannel) == 0) || (image->matte == MagickFalse))
        {
          for (v=0; v < (long) width; v++)
          {
            for (u=0; u < (long) width; u++)
            {
              if (SelectiveContrast(p+u+j,q,threshold) != MagickFalse)
                {
                  pixel.red+=(*k)*(p+u+j)->red;
                  pixel.green+=(*k)*(p+u+j)->green;
                  pixel.blue+=(*k)*(p+u+j)->blue;
                  gamma+=(*k);
                  k++;
                }
            }
            j+=image->columns+width;
          }
          if (gamma != 0.0)
            {
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
              if ((channel & RedChannel) != 0)
                q->red=ClampToQuantum(gamma*GetRedPixelComponent(&pixel));
              if ((channel & GreenChannel) != 0)
                q->green=ClampToQuantum(gamma*GetGreenPixelComponent(&pixel));
              if ((channel & BlueChannel) != 0)
                q->blue=ClampToQuantum(gamma*GetBluePixelComponent(&pixel));
            }
          if ((channel & OpacityChannel) != 0)
            {
              gamma=0.0;
              j=0;
              for (v=0; v < (long) width; v++)
              {
                for (u=0; u < (long) width; u++)
                {
                  if (SelectiveContrast(p+u+j,q,threshold) != MagickFalse)
                    {
                      pixel.opacity+=(*k)*(p+u+j)->opacity;
                      gamma+=(*k);
                      k++;
                    }
                }
                j+=image->columns+width;
              }
              if (gamma != 0.0)
                {
                  gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 :
                    gamma);
                  SetOpacityPixelComponent(q,ClampToQuantum(gamma*
                    GetOpacityPixelComponent(&pixel)));
                }
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              gamma=0.0;
              j=0;
              for (v=0; v < (long) width; v++)
              {
                for (u=0; u < (long) width; u++)
                {
                  if (SelectiveContrast(p+u+j,q,threshold) != MagickFalse)
                    {
                      pixel.index+=(*k)*indexes[x+u+j];
                      gamma+=(*k);
                      k++;
                    }
                }
                j+=image->columns+width;
              }
              if (gamma != 0.0)
                {
                  gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 :
                    gamma);
                  blur_indexes[x]=ClampToQuantum(gamma*
                    GetIndexPixelComponent(&pixel));
                }
            }
        }
      else
        {
          MagickRealType
            alpha;

          for (v=0; v < (long) width; v++)
          {
            for (u=0; u < (long) width; u++)
            {
              if (SelectiveContrast(p+u+j,q,threshold) != MagickFalse)
                {
                  alpha=(MagickRealType) (QuantumScale*
                    GetAlphaPixelComponent(p+u+j));
                  pixel.red+=(*k)*alpha*(p+u+j)->red;
                  pixel.green+=(*k)*alpha*(p+u+j)->green;
                  pixel.blue+=(*k)*alpha*(p+u+j)->blue;
                  pixel.opacity+=(*k)*(p+u+j)->opacity;
                  gamma+=(*k)*alpha;
                  k++;
                }
            }
            j+=image->columns+width;
          }
          if (gamma != 0.0)
            {
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
              if ((channel & RedChannel) != 0)
                q->red=ClampToQuantum(gamma*GetRedPixelComponent(&pixel));
              if ((channel & GreenChannel) != 0)
                q->green=ClampToQuantum(gamma*GetGreenPixelComponent(&pixel));
              if ((channel & BlueChannel) != 0)
                q->blue=ClampToQuantum(gamma*GetBluePixelComponent(&pixel));
            }
          if ((channel & OpacityChannel) != 0)
            {
              gamma=0.0;
              j=0;
              for (v=0; v < (long) width; v++)
              {
                for (u=0; u < (long) width; u++)
                {
                  if (SelectiveContrast(p+u+j,q,threshold) != MagickFalse)
                    {
                      pixel.opacity+=(*k)*(p+u+j)->opacity;
                      gamma+=(*k);
                      k++;
                    }
                }
                j+=image->columns+width;
              }
              if (gamma != 0.0)
                {
                  gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 :
                    gamma);
                  SetOpacityPixelComponent(q,
                    ClampOpacityPixelComponent(&pixel));
                }
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              gamma=0.0;
              j=0;
              for (v=0; v < (long) width; v++)
              {
                for (u=0; u < (long) width; u++)
                {
                  if (SelectiveContrast(p+u+j,q,threshold) != MagickFalse)
                    {
                      alpha=(MagickRealType) (QuantumScale*
                        GetAlphaPixelComponent(p+u+j));
                      pixel.index+=(*k)*alpha*indexes[x+u+j];
                      gamma+=(*k);
                      k++;
                    }
                }
                j+=image->columns+width;
              }
              if (gamma != 0.0)
                {
                  gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 :
                    gamma);
                  blur_indexes[x]=ClampToQuantum(gamma*
                    GetIndexPixelComponent(&pixel));
                }
            }
        }
      p++;
      q++;
    }
    sync=SyncCacheViewAuthenticPixels(blur_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_SelectiveBlurImageChannel)
#endif
        proceed=SetImageProgress(image,SelectiveBlurImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  blur_image->type=image->type;
  blur_view=DestroyCacheView(blur_view);
  image_view=DestroyCacheView(image_view);
  kernel=(double *) RelinquishMagickMemory(kernel);
  if (status == MagickFalse)
    blur_image=DestroyImage(blur_image);
  return(blur_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S h a d e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ShadeImage() shines a distant light on an image to create a
%  three-dimensional effect. You control the positioning of the light with
%  azimuth and elevation; azimuth is measured in degrees off the x axis
%  and elevation is measured in pixels above the Z axis.
%
%  The format of the ShadeImage method is:
%
%      Image *ShadeImage(const Image *image,const MagickBooleanType gray,
%        const double azimuth,const double elevation,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o gray: A value other than zero shades the intensity of each pixel.
%
%    o azimuth, elevation:  Define the light source direction.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ShadeImage(const Image *image,const MagickBooleanType gray,
  const double azimuth,const double elevation,ExceptionInfo *exception)
{
#define ShadeImageTag  "Shade/Image"

  CacheView
    *image_view,
    *shade_view;

  Image
    *shade_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  PrimaryInfo
    light;

  /*
    Initialize shaded image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  shade_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (shade_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(shade_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&shade_image->exception);
      shade_image=DestroyImage(shade_image);
      return((Image *) NULL);
    }
  /*
    Compute the light vector.
  */
  light.x=(double) QuantumRange*cos(DegreesToRadians(azimuth))*
    cos(DegreesToRadians(elevation));
  light.y=(double) QuantumRange*sin(DegreesToRadians(azimuth))*
    cos(DegreesToRadians(elevation));
  light.z=(double) QuantumRange*sin(DegreesToRadians(elevation));
  /*
    Shade image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  shade_view=AcquireCacheView(shade_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickRealType
      distance,
      normal_distance,
      shade;

    PrimaryInfo
      normal;

    register const PixelPacket
      *restrict p,
      *restrict s0,
      *restrict s1,
      *restrict s2;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-1,y-1,image->columns+2,3,exception);
    q=QueueCacheViewAuthenticPixels(shade_view,0,y,shade_image->columns,1,
      exception);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    /*
      Shade this row of pixels.
    */
    normal.z=2.0*(double) QuantumRange;  /* constant Z of surface normal */
    s0=p+1;
    s1=s0+image->columns+2;
    s2=s1+image->columns+2;
    for (x=0; x < (long) image->columns; x++)
    {
      /*
        Determine the surface normal and compute shading.
      */
      normal.x=(double) (PixelIntensity(s0-1)+PixelIntensity(s1-1)+
        PixelIntensity(s2-1)-PixelIntensity(s0+1)-PixelIntensity(s1+1)-
        PixelIntensity(s2+1));
      normal.y=(double) (PixelIntensity(s2-1)+PixelIntensity(s2)+
        PixelIntensity(s2+1)-PixelIntensity(s0-1)-PixelIntensity(s0)-
        PixelIntensity(s0+1));
      if ((normal.x == 0.0) && (normal.y == 0.0))
        shade=light.z;
      else
        {
          shade=0.0;
          distance=normal.x*light.x+normal.y*light.y+normal.z*light.z;
          if (distance > MagickEpsilon)
            {
              normal_distance=
                normal.x*normal.x+normal.y*normal.y+normal.z*normal.z;
              if (normal_distance > (MagickEpsilon*MagickEpsilon))
                shade=distance/sqrt((double) normal_distance);
            }
        }
      if (gray != MagickFalse)
        {
          q->red=(Quantum) shade;
          q->green=(Quantum) shade;
          q->blue=(Quantum) shade;
        }
      else
        {
          q->red=ClampToQuantum(QuantumScale*shade*s1->red);
          q->green=ClampToQuantum(QuantumScale*shade*s1->green);
          q->blue=ClampToQuantum(QuantumScale*shade*s1->blue);
        }
      q->opacity=s1->opacity;
      s0++;
      s1++;
      s2++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(shade_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ShadeImage)
#endif
        proceed=SetImageProgress(image,ShadeImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  shade_view=DestroyCacheView(shade_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    shade_image=DestroyImage(shade_image);
  return(shade_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S h a r p e n I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SharpenImage() sharpens the image.  We convolve the image with a Gaussian
%  operator of the given radius and standard deviation (sigma).  For
%  reasonable results, radius should be larger than sigma.  Use a radius of 0
%  and SharpenImage() selects a suitable radius for you.
%
%  Using a separable kernel would be faster, but the negative weights cancel
%  out on the corners of the kernel producing often undesirable ringing in the
%  filtered result; this can be avoided by using a 2D gaussian shaped image
%  sharpening kernel instead.
%
%  The format of the SharpenImage method is:
%
%    Image *SharpenImage(const Image *image,const double radius,
%      const double sigma,ExceptionInfo *exception)
%    Image *SharpenImageChannel(const Image *image,const ChannelType channel,
%      const double radius,const double sigma,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Laplacian, in pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image *SharpenImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
{
  Image
    *sharp_image;

  sharp_image=SharpenImageChannel(image,DefaultChannels,radius,sigma,exception);
  return(sharp_image);
}

MagickExport Image *SharpenImageChannel(const Image *image,
  const ChannelType channel,const double radius,const double sigma,
  ExceptionInfo *exception)
{
  double
    *kernel,
    normalize;

  Image
    *sharp_image;

  long
    j,
    u,
    v;

  register long
    i;

  unsigned long
    width;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth2D(radius,sigma);
  kernel=(double *) AcquireQuantumMemory((size_t) width*width,sizeof(*kernel));
  if (kernel == (double *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  normalize=0.0;
  j=(long) width/2;
  i=0;
  for (v=(-j); v <= j; v++)
  {
    for (u=(-j); u <= j; u++)
    {
      kernel[i]=(-exp(-((double) u*u+v*v)/(2.0*MagickSigma*MagickSigma))/
        (2.0*MagickPI*MagickSigma*MagickSigma));
      normalize+=kernel[i];
      i++;
    }
  }
  kernel[i/2]=(double) ((-2.0)*normalize);
  sharp_image=ConvolveImageChannel(image,channel,width,kernel,exception);
  kernel=(double *) RelinquishMagickMemory(kernel);
  return(sharp_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S p r e a d I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SpreadImage() is a special effects method that randomly displaces each
%  pixel in a block defined by the radius parameter.
%
%  The format of the SpreadImage method is:
%
%      Image *SpreadImage(const Image *image,const double radius,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius:  Choose a random pixel in a neighborhood of this extent.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *SpreadImage(const Image *image,const double radius,
  ExceptionInfo *exception)
{
#define SpreadImageTag  "Spread/Image"

  CacheView
    *image_view;

  Image
    *spread_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    bias;

  RandomInfo
    **restrict random_info;

  ResampleFilter
    **restrict resample_filter;

  unsigned long
    width;

  /*
    Initialize spread image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  spread_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (spread_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(spread_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&spread_image->exception);
      spread_image=DestroyImage(spread_image);
      return((Image *) NULL);
    }
  /*
    Spread image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(spread_image,&bias);
  width=GetOptimalKernelWidth1D(radius,0.5);
  resample_filter=AcquireResampleFilterThreadSet(image,
    UndefinedVirtualPixelMethod,MagickTrue,exception);
  random_info=AcquireRandomInfoThreadSet();
  image_view=AcquireCacheView(spread_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status)
#endif
  for (y=0; y < (long) spread_image->rows; y++)
  {
    MagickPixelPacket
      pixel;

    register IndexPacket
      *restrict indexes;

    register long
      id,
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(image_view,0,y,spread_image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    pixel=bias;
    id=GetOpenMPThreadId();
    for (x=0; x < (long) spread_image->columns; x++)
    {
      (void) ResamplePixelColor(resample_filter[id],(double) x+width*
        (GetPseudoRandomValue(random_info[id])-0.5),(double) y+width*
        (GetPseudoRandomValue(random_info[id])-0.5),&pixel);
      SetPixelPacket(spread_image,&pixel,q,indexes+x);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_SpreadImage)
#endif
        proceed=SetImageProgress(image,SpreadImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  random_info=DestroyRandomInfoThreadSet(random_info);
  resample_filter=DestroyResampleFilterThreadSet(resample_filter);
  return(spread_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     U n s h a r p M a s k I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnsharpMaskImage() sharpens one or more image channels.  We convolve the
%  image with a Gaussian operator of the given radius and standard deviation
%  (sigma).  For reasonable results, radius should be larger than sigma.  Use a
%  radius of 0 and UnsharpMaskImage() selects a suitable radius for you.
%
%  The format of the UnsharpMaskImage method is:
%
%    Image *UnsharpMaskImage(const Image *image,const double radius,
%      const double sigma,const double amount,const double threshold,
%      ExceptionInfo *exception)
%    Image *UnsharpMaskImageChannel(const Image *image,
%      const ChannelType channel,const double radius,const double sigma,
%      const double amount,const double threshold,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o amount: the percentage of the difference between the original and the
%      blur image that is added back into the original.
%
%    o threshold: the threshold in pixels needed to apply the diffence amount.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image *UnsharpMaskImage(const Image *image,const double radius,
  const double sigma,const double amount,const double threshold,
  ExceptionInfo *exception)
{
  Image
    *sharp_image;

  sharp_image=UnsharpMaskImageChannel(image,DefaultChannels,radius,sigma,amount,
    threshold,exception);
  return(sharp_image);
}

MagickExport Image *UnsharpMaskImageChannel(const Image *image,
  const ChannelType channel,const double radius,const double sigma,
  const double amount,const double threshold,ExceptionInfo *exception)
{
#define SharpenImageTag  "Sharpen/Image"

  CacheView
    *image_view,
    *unsharp_view;

  Image
    *unsharp_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    bias;

  MagickRealType
    quantum_threshold;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  unsharp_image=BlurImageChannel(image,channel,radius,sigma,exception);
  if (unsharp_image == (Image *) NULL)
    return((Image *) NULL);
  quantum_threshold=(MagickRealType) QuantumRange*threshold;
  /*
    Unsharp-mask image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&bias);
  image_view=AcquireCacheView(image);
  unsharp_view=AcquireCacheView(unsharp_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickPixelPacket
      pixel;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict unsharp_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewAuthenticPixels(unsharp_view,0,y,unsharp_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    unsharp_indexes=GetCacheViewAuthenticIndexQueue(unsharp_view);
    pixel=bias;
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        {
          pixel.red=p->red-(MagickRealType) q->red;
          if (fabs(2.0*pixel.red) < quantum_threshold)
            pixel.red=(MagickRealType) GetRedPixelComponent(p);
          else
            pixel.red=(MagickRealType) p->red+(pixel.red*amount);
          SetRedPixelComponent(q,ClampRedPixelComponent(&pixel));
        }
      if ((channel & GreenChannel) != 0)
        {
          pixel.green=p->green-(MagickRealType) q->green;
          if (fabs(2.0*pixel.green) < quantum_threshold)
            pixel.green=(MagickRealType) GetGreenPixelComponent(p);
          else
            pixel.green=(MagickRealType) p->green+(pixel.green*amount);
          SetGreenPixelComponent(q,ClampGreenPixelComponent(&pixel));
        }
      if ((channel & BlueChannel) != 0)
        {
          pixel.blue=p->blue-(MagickRealType) q->blue;
          if (fabs(2.0*pixel.blue) < quantum_threshold)
            pixel.blue=(MagickRealType) GetBluePixelComponent(p);
          else
            pixel.blue=(MagickRealType) p->blue+(pixel.blue*amount);
          SetBluePixelComponent(q,ClampBluePixelComponent(&pixel));
        }
      if ((channel & OpacityChannel) != 0)
        {
          pixel.opacity=p->opacity-(MagickRealType) q->opacity;
          if (fabs(2.0*pixel.opacity) < quantum_threshold)
            pixel.opacity=(MagickRealType) GetOpacityPixelComponent(p);
          else
            pixel.opacity=p->opacity+(pixel.opacity*amount);
          SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          pixel.index=unsharp_indexes[x]-(MagickRealType) indexes[x];
          if (fabs(2.0*pixel.index) < quantum_threshold)
            pixel.index=(MagickRealType) unsharp_indexes[x];
          else
            pixel.index=(MagickRealType) unsharp_indexes[x]+(pixel.index*
              amount);
          unsharp_indexes[x]=ClampToQuantum(pixel.index);
        }
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(unsharp_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_UnsharpMaskImageChannel)
#endif
        proceed=SetImageProgress(image,SharpenImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  unsharp_image->type=image->type;
  unsharp_view=DestroyCacheView(unsharp_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    unsharp_image=DestroyImage(unsharp_image);
  return(unsharp_image);
}

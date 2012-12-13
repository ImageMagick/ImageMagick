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
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/distort.h"
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
#include "magick/memory-private.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/montage.h"
#include "magick/morphology.h"
#include "magick/paint.h"
#include "magick/pixel-accessor.h"
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
#include "magick/statistic.h"
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
#define MagickSigma  (fabs(sigma) < MagickEpsilon ? MagickEpsilon : sigma)

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

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    bias;

  register ssize_t
    i;

  size_t
    width;

  ssize_t
    j,
    k,
    u,
    v,
    y;

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
  kernel=(double **) MagickAssumeAligned(AcquireAlignedMemory((size_t) width,
    sizeof(*kernel)));
  if (kernel == (double **) NULL)
    {
      edge_image=DestroyImage(edge_image);
      blur_image=DestroyImage(blur_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  (void) ResetMagickMemory(kernel,0,(size_t) width*sizeof(*kernel));
  for (i=0; i < (ssize_t) width; i+=2)
  {
    kernel[i]=(double *) MagickAssumeAligned(AcquireAlignedMemory((size_t)
      (width-i),(width-i)*sizeof(**kernel)));
    if (kernel[i] == (double *) NULL)
      break;
    normalize=0.0;
    j=(ssize_t) (width-i)/2;
    k=0;
    for (v=(-j); v <= j; v++)
    {
      for (u=(-j); u <= j; u++)
      {
        kernel[i][k]=(double) (exp(-((double) u*u+v*v)/(2.0*MagickSigma*
          MagickSigma))/(2.0*MagickPI*MagickSigma*MagickSigma));
        normalize+=kernel[i][k];
        k++;
      }
    }
    if (fabs(normalize) < MagickEpsilon)
      normalize=MagickEpsilon;
    normalize=PerceptibleReciprocal(normalize);
    for (k=0; k < (j*j); k++)
      kernel[i][k]=normalize*kernel[i][k];
  }
  if (i < (ssize_t) width)
    {
      for (i-=2; i >= 0; i-=2)
        kernel[i]=(double *) RelinquishAlignedMemory(kernel[i]);
      kernel=(double **) RelinquishAlignedMemory(kernel);
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
  image_view=AcquireVirtualCacheView(image,exception);
  edge_view=AcquireVirtualCacheView(edge_image,exception);
  blur_view=AcquireAuthenticCacheView(blur_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) blur_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p,
      *restrict r;

    register IndexPacket
      *restrict blur_indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

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
    for (x=0; x < (ssize_t) blur_image->columns; x++)
    {
      MagickPixelPacket
        pixel;

      MagickRealType
        alpha,
        gamma;

      register const double
        *restrict k;

      register ssize_t
        i,
        u,
        v;

      gamma=0.0;
      i=(ssize_t) ceil((double) width*QuantumScale*
        GetPixelIntensity(edge_image,r)-0.5);
      if (i < 0)
        i=0;
      else
        if (i > (ssize_t) width)
          i=(ssize_t) width;
      if ((i & 0x01) != 0)
        i--;
      p=GetCacheViewVirtualPixels(image_view,x-((ssize_t) (width-i)/2L),y-
        (ssize_t) ((width-i)/2L),width-i,width-i,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      pixel=bias;
      k=kernel[i];
      for (v=0; v < (ssize_t) (width-i); v++)
      {
        for (u=0; u < (ssize_t) (width-i); u++)
        {
          alpha=1.0;
          if (((channel & OpacityChannel) != 0) &&
              (image->matte != MagickFalse))
            alpha=(MagickRealType) (QuantumScale*GetPixelAlpha(p));
          if ((channel & RedChannel) != 0)
            pixel.red+=(*k)*alpha*GetPixelRed(p);
          if ((channel & GreenChannel) != 0)
            pixel.green+=(*k)*alpha*GetPixelGreen(p);
          if ((channel & BlueChannel) != 0)
            pixel.blue+=(*k)*alpha*GetPixelBlue(p);
          if ((channel & OpacityChannel) != 0)
            pixel.opacity+=(*k)*GetPixelOpacity(p);
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            pixel.index+=(*k)*alpha*GetPixelIndex(indexes+x+(width-i)*v+u);
          gamma+=(*k)*alpha;
          k++;
          p++;
        }
      }
      gamma=PerceptibleReciprocal(gamma);
      if ((channel & RedChannel) != 0)
        SetPixelRed(q,ClampToQuantum(gamma*pixel.red));
      if ((channel & GreenChannel) != 0)
        SetPixelGreen(q,ClampToQuantum(gamma*pixel.green));
      if ((channel & BlueChannel) != 0)
        SetPixelBlue(q,ClampToQuantum(gamma*pixel.blue));
      if ((channel & OpacityChannel) != 0)
        SetPixelOpacity(q,ClampToQuantum(pixel.opacity));
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelIndex(blur_indexes+x,ClampToQuantum(gamma*pixel.index));
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
  for (i=0; i < (ssize_t) width;  i+=2)
    kernel[i]=(double *) RelinquishAlignedMemory(kernel[i]);
  kernel=(double **) RelinquishAlignedMemory(kernel);
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
#define MagickSigma  (fabs(sigma) < MagickEpsilon ? MagickEpsilon : sigma)

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

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    bias;

  register ssize_t
    i;

  size_t
    width;

  ssize_t
    j,
    k,
    u,
    v,
    y;

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
  kernel=(double **) MagickAssumeAligned(AcquireAlignedMemory((size_t) width,
    sizeof(*kernel)));
  if (kernel == (double **) NULL)
    {
      edge_image=DestroyImage(edge_image);
      sharp_image=DestroyImage(sharp_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  (void) ResetMagickMemory(kernel,0,(size_t) width*sizeof(*kernel));
  for (i=0; i < (ssize_t) width; i+=2)
  {
    kernel[i]=(double *) MagickAssumeAligned(AcquireAlignedMemory((size_t)
      (width-i),(width-i)*sizeof(**kernel)));
    if (kernel[i] == (double *) NULL)
      break;
    normalize=0.0;
    j=(ssize_t) (width-i)/2;
    k=0;
    for (v=(-j); v <= j; v++)
    {
      for (u=(-j); u <= j; u++)
      {
        kernel[i][k]=(double) (-exp(-((double) u*u+v*v)/(2.0*MagickSigma*
          MagickSigma))/(2.0*MagickPI*MagickSigma*MagickSigma));
        normalize+=kernel[i][k];
        k++;
      }
    }
    if (fabs(normalize) < MagickEpsilon)
      normalize=MagickEpsilon;
    normalize=PerceptibleReciprocal(normalize);
    for (k=0; k < (j*j); k++)
      kernel[i][k]=normalize*kernel[i][k];
  }
  if (i < (ssize_t) width)
    {
      for (i-=2; i >= 0; i-=2)
        kernel[i]=(double *) RelinquishAlignedMemory(kernel[i]);
      kernel=(double **) RelinquishAlignedMemory(kernel);
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
  image_view=AcquireVirtualCacheView(image,exception);
  edge_view=AcquireVirtualCacheView(edge_image,exception);
  sharp_view=AcquireAuthenticCacheView(sharp_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) sharp_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p,
      *restrict r;

    register IndexPacket
      *restrict sharp_indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

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
    for (x=0; x < (ssize_t) sharp_image->columns; x++)
    {
      MagickPixelPacket
        pixel;

      MagickRealType
        alpha,
        gamma;

      register const double
        *restrict k;

      register ssize_t
        i,
        u,
        v;

      gamma=0.0;
      i=(ssize_t) ceil((double) width*(QuantumRange-QuantumScale*
        GetPixelIntensity(edge_image,r))-0.5);
      if (i < 0)
        i=0;
      else
        if (i > (ssize_t) width)
          i=(ssize_t) width;
      if ((i & 0x01) != 0)
        i--;
      p=GetCacheViewVirtualPixels(image_view,x-((ssize_t) (width-i)/2L),y-
        (ssize_t) ((width-i)/2L),width-i,width-i,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      k=kernel[i];
      pixel=bias;
      for (v=0; v < (ssize_t) (width-i); v++)
      {
        for (u=0; u < (ssize_t) (width-i); u++)
        {
          alpha=1.0;
          if (((channel & OpacityChannel) != 0) &&
              (image->matte != MagickFalse))
            alpha=(MagickRealType) (QuantumScale*GetPixelAlpha(p));
          if ((channel & RedChannel) != 0)
            pixel.red+=(*k)*alpha*GetPixelRed(p);
          if ((channel & GreenChannel) != 0)
            pixel.green+=(*k)*alpha*GetPixelGreen(p);
          if ((channel & BlueChannel) != 0)
            pixel.blue+=(*k)*alpha*GetPixelBlue(p);
          if ((channel & OpacityChannel) != 0)
            pixel.opacity+=(*k)*GetPixelOpacity(p);
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            pixel.index+=(*k)*alpha*GetPixelIndex(indexes+x+(width-i)*v+u);
          gamma+=(*k)*alpha;
          k++;
          p++;
        }
      }
      gamma=PerceptibleReciprocal(gamma);
      if ((channel & RedChannel) != 0)
        SetPixelRed(q,ClampToQuantum(gamma*pixel.red));
      if ((channel & GreenChannel) != 0)
        SetPixelGreen(q,ClampToQuantum(gamma*pixel.green));
      if ((channel & BlueChannel) != 0)
        SetPixelBlue(q,ClampToQuantum(gamma*pixel.blue));
      if ((channel & OpacityChannel) != 0)
        SetPixelOpacity(q,ClampToQuantum(pixel.opacity));
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelIndex(sharp_indexes+x,ClampToQuantum(gamma*pixel.index));
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
  for (i=0; i < (ssize_t) width;  i+=2)
    kernel[i]=(double *) RelinquishAlignedMemory(kernel[i]);
  kernel=(double **) RelinquishAlignedMemory(kernel);
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

static MagickRealType *GetBlurKernel(const size_t width,const double sigma)
{
  double
    normalize;

  MagickRealType
    *kernel;

  register ssize_t
    i;

  ssize_t
    j,
    k;

  /*
    Generate a 1-D convolution kernel.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  kernel=(MagickRealType *) MagickAssumeAligned(AcquireAlignedMemory((size_t)
    width,sizeof(*kernel)));
  if (kernel == (MagickRealType *) NULL)
    return(0);
  normalize=0.0;
  j=(ssize_t) width/2;
  i=0;
  for (k=(-j); k <= j; k++)
  {
    kernel[i]=(MagickRealType) (exp(-((double) k*k)/(2.0*MagickSigma*
      MagickSigma))/(MagickSQ2PI*MagickSigma));
    normalize+=kernel[i];
    i++;
  }
  for (i=0; i < (ssize_t) width; i++)
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

  Image
    *blur_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickRealType
    *kernel;

  MagickPixelPacket
    bias;

  register ssize_t
    i;

  size_t
    width;

  ssize_t
    x,
    y;

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
  if (fabs(sigma) < MagickEpsilon)
    return(blur_image);
  if (SetImageStorageClass(blur_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&blur_image->exception);
      blur_image=DestroyImage(blur_image);
      return((Image *) NULL);
    }
  width=GetOptimalKernelWidth1D(radius,sigma);
  kernel=GetBlurKernel(width,sigma);
  if (kernel == (MagickRealType *) NULL)
    {
      blur_image=DestroyImage(blur_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  if (image->debug != MagickFalse)
    {
      char
        format[MaxTextExtent],
        *message;

      register const MagickRealType
        *k;

      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  BlurImage with %.20g kernel:",(double) width);
      message=AcquireString("");
      k=kernel;
      for (i=0; i < (ssize_t) width; i++)
      {
        *message='\0';
        (void) FormatLocaleString(format,MaxTextExtent,"%.20g: ",(double) i);
        (void) ConcatenateString(&message,format);
        (void) FormatLocaleString(format,MaxTextExtent,"%g ",(double) *k++);
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
  image_view=AcquireVirtualCacheView(image,exception);
  blur_view=AcquireAuthenticCacheView(blur_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) blur_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict blur_indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) width/2L),y,
      image->columns+width,1,exception);
    q=GetCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    blur_indexes=GetCacheViewAuthenticIndexQueue(blur_view);
    for (x=0; x < (ssize_t) blur_image->columns; x++)
    {
      MagickPixelPacket
        pixel;

      register const MagickRealType
        *restrict k;

      register const PixelPacket
        *restrict kernel_pixels;

      register ssize_t
        i;

      pixel=bias;
      k=kernel;
      kernel_pixels=p;
      if (((channel & OpacityChannel) == 0) || (image->matte == MagickFalse))
        {
          for (i=0; i < (ssize_t) width; i++)
          {
            pixel.red+=(*k)*GetPixelRed(kernel_pixels);
            pixel.green+=(*k)*GetPixelGreen(kernel_pixels);
            pixel.blue+=(*k)*GetPixelBlue(kernel_pixels);
            k++;
            kernel_pixels++;
          }
          if ((channel & RedChannel) != 0)
            SetPixelRed(q,ClampToQuantum(pixel.red));
          if ((channel & GreenChannel) != 0)
            SetPixelGreen(q,ClampToQuantum(pixel.green));
          if ((channel & BlueChannel) != 0)
            SetPixelBlue(q,ClampToQuantum(pixel.blue));
          if ((channel & OpacityChannel) != 0)
            {
              k=kernel;
              kernel_pixels=p;
              for (i=0; i < (ssize_t) width; i++)
              {
                pixel.opacity+=(*k)*GetPixelOpacity(kernel_pixels);
                k++;
                kernel_pixels++;
              }
              SetPixelOpacity(q,ClampToQuantum(pixel.opacity));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=kernel;
              kernel_indexes=indexes;
              for (i=0; i < (ssize_t) width; i++)
              {
                pixel.index+=(*k)*GetPixelIndex(kernel_indexes);
                k++;
                kernel_indexes++;
              }
              SetPixelIndex(blur_indexes+x,ClampToQuantum(pixel.index));
            }
        }
      else
        {
          MagickRealType
            alpha,
            gamma;

          gamma=0.0;
          for (i=0; i < (ssize_t) width; i++)
          {
            alpha=(MagickRealType) (QuantumScale*GetPixelAlpha(kernel_pixels));
            pixel.red+=(*k)*alpha*GetPixelRed(kernel_pixels);
            pixel.green+=(*k)*alpha*GetPixelGreen(kernel_pixels);
            pixel.blue+=(*k)*alpha*GetPixelBlue(kernel_pixels);
            gamma+=(*k)*alpha;
            k++;
            kernel_pixels++;
          }
          gamma=PerceptibleReciprocal(gamma);
          if ((channel & RedChannel) != 0)
            SetPixelRed(q,ClampToQuantum(gamma*pixel.red));
          if ((channel & GreenChannel) != 0)
            SetPixelGreen(q,ClampToQuantum(gamma*pixel.green));
          if ((channel & BlueChannel) != 0)
            SetPixelBlue(q,ClampToQuantum(gamma*pixel.blue));
          if ((channel & OpacityChannel) != 0)
            {
              k=kernel;
              kernel_pixels=p;
              for (i=0; i < (ssize_t) width; i++)
              {
                pixel.opacity+=(*k)*GetPixelOpacity(kernel_pixels);
                k++;
                kernel_pixels++;
              }
              SetPixelOpacity(q,ClampToQuantum(pixel.opacity));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=kernel;
              kernel_pixels=p;
              kernel_indexes=indexes;
              for (i=0; i < (ssize_t) width; i++)
              {
                alpha=(MagickRealType) (QuantumScale*
                  GetPixelAlpha(kernel_pixels));
                pixel.index+=(*k)*alpha*(*kernel_indexes);
                k++;
                kernel_pixels++;
                kernel_indexes++;
              }
              SetPixelIndex(blur_indexes+x,ClampToQuantum(gamma*pixel.index));
            }
        }
      indexes++;
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
  image_view=AcquireVirtualCacheView(blur_image,exception);
  blur_view=AcquireAuthenticCacheView(blur_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (x=0; x < (ssize_t) blur_image->columns; x++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict blur_indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      y;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,x,-((ssize_t) width/2L),1,
      image->rows+width,exception);
    q=GetCacheViewAuthenticPixels(blur_view,x,0,1,blur_image->rows,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    blur_indexes=GetCacheViewAuthenticIndexQueue(blur_view);
    for (y=0; y < (ssize_t) blur_image->rows; y++)
    {
      MagickPixelPacket
        pixel;

      register const MagickRealType
        *restrict k;

      register const PixelPacket
        *restrict kernel_pixels;

      register ssize_t
        i;

      pixel=bias;
      k=kernel;
      kernel_pixels=p;
      if (((channel & OpacityChannel) == 0) || (image->matte == MagickFalse))
        {
          for (i=0; i < (ssize_t) width; i++)
          {
            pixel.red+=(*k)*GetPixelRed(kernel_pixels);
            pixel.green+=(*k)*GetPixelGreen(kernel_pixels);
            pixel.blue+=(*k)*GetPixelBlue(kernel_pixels);
            k++;
            kernel_pixels++;
          }
          if ((channel & RedChannel) != 0)
            SetPixelRed(q,ClampToQuantum(pixel.red));
          if ((channel & GreenChannel) != 0)
            SetPixelGreen(q,ClampToQuantum(pixel.green));
          if ((channel & BlueChannel) != 0)
            SetPixelBlue(q,ClampToQuantum(pixel.blue));
          if ((channel & OpacityChannel) != 0)
            {
              k=kernel;
              kernel_pixels=p;
              for (i=0; i < (ssize_t) width; i++)
              {
                pixel.opacity+=(*k)*GetPixelOpacity(kernel_pixels);
                k++;
                kernel_pixels++;
              }
              SetPixelOpacity(q,ClampToQuantum(pixel.opacity));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=kernel;
              kernel_indexes=indexes;
              for (i=0; i < (ssize_t) width; i++)
              {
                pixel.index+=(*k)*GetPixelIndex(kernel_indexes);
                k++;
                kernel_indexes++;
              }
              SetPixelIndex(blur_indexes+y,ClampToQuantum(pixel.index));
            }
        }
      else
        {
          MagickRealType
            alpha,
            gamma;

          gamma=0.0;
          for (i=0; i < (ssize_t) width; i++)
          {
            alpha=(MagickRealType) (QuantumScale*GetPixelAlpha(kernel_pixels));
            pixel.red+=(*k)*alpha*GetPixelRed(kernel_pixels);
            pixel.green+=(*k)*alpha*GetPixelGreen(kernel_pixels);
            pixel.blue+=(*k)*alpha*GetPixelBlue(kernel_pixels);
            gamma+=(*k)*alpha;
            k++;
            kernel_pixels++;
          }
          gamma=PerceptibleReciprocal(gamma);
          if ((channel & RedChannel) != 0)
            SetPixelRed(q,ClampToQuantum(gamma*pixel.red));
          if ((channel & GreenChannel) != 0)
            SetPixelGreen(q,ClampToQuantum(gamma*pixel.green));
          if ((channel & BlueChannel) != 0)
            SetPixelBlue(q,ClampToQuantum(gamma*pixel.blue));
          if ((channel & OpacityChannel) != 0)
            {
              k=kernel;
              kernel_pixels=p;
              for (i=0; i < (ssize_t) width; i++)
              {
                pixel.opacity+=(*k)*GetPixelOpacity(kernel_pixels);
                k++;
                kernel_pixels++;
              }
              SetPixelOpacity(q,ClampToQuantum(pixel.opacity));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=kernel;
              kernel_pixels=p;
              kernel_indexes=indexes;
              for (i=0; i < (ssize_t) width; i++)
              {
                alpha=(MagickRealType) (QuantumScale*
                  GetPixelAlpha(kernel_pixels));
                pixel.index+=(*k)*alpha*(*kernel_indexes);
                k++;
                kernel_pixels++;
                kernel_indexes++;
              }
              SetPixelIndex(blur_indexes+y,ClampToQuantum(gamma*pixel.index));
            }
        }
      indexes++;
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
  kernel=(MagickRealType *) RelinquishAlignedMemory(kernel);
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
%      Image *ConvolveImage(const Image *image,const size_t order,
%        const double *kernel,ExceptionInfo *exception)
%      Image *ConvolveImageChannel(const Image *image,const ChannelType channel,
%        const size_t order,const double *kernel,ExceptionInfo *exception)
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

MagickExport Image *ConvolveImage(const Image *image,const size_t order,
  const double *kernel,ExceptionInfo *exception)
{
  Image
    *convolve_image;

  convolve_image=ConvolveImageChannel(image,DefaultChannels,order,kernel,
    exception);
  return(convolve_image);
}

MagickExport Image *ConvolveImageChannel(const Image *image,
  const ChannelType channel,const size_t order,const double *kernel,
  ExceptionInfo *exception)
{
#define ConvolveImageTag  "Convolve/Image"

  CacheView
    *convolve_view,
    *image_view;

  Image
    *convolve_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    bias;

  MagickRealType
    gamma,
    *normal_kernel;

  register ssize_t
    i;

  size_t
    width;

  ssize_t
    y;

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

      register const double
        *k;

      ssize_t
        u,
        v;

      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  ConvolveImage with %.20gx%.20g kernel:",(double) width,(double)
        width);
      message=AcquireString("");
      k=kernel;
      for (v=0; v < (ssize_t) width; v++)
      {
        *message='\0';
        (void) FormatLocaleString(format,MaxTextExtent,"%.20g: ",(double) v);
        (void) ConcatenateString(&message,format);
        for (u=0; u < (ssize_t) width; u++)
        {
          (void) FormatLocaleString(format,MaxTextExtent,"%g ",*k++);
          (void) ConcatenateString(&message,format);
        }
        (void) LogMagickEvent(TransformEvent,GetMagickModule(),"%s",message);
      }
      message=DestroyString(message);
    }
  /*
    Normalize kernel.
  */
  normal_kernel=(MagickRealType *) MagickAssumeAligned(AcquireAlignedMemory(
    width,width*sizeof(*normal_kernel)));
  if (normal_kernel == (MagickRealType *) NULL)
    {
      convolve_image=DestroyImage(convolve_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  gamma=0.0;
  for (i=0; i < (ssize_t) (width*width); i++)
    gamma+=kernel[i];
  gamma=PerceptibleReciprocal(gamma);
  for (i=0; i < (ssize_t) (width*width); i++)
    normal_kernel[i]=gamma*kernel[i];
  /*
    Convolve image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&bias);
  SetMagickPixelPacketBias(image,&bias);
  image_view=AcquireVirtualCacheView(image,exception);
  convolve_view=AcquireAuthenticCacheView(convolve_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict convolve_indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) width/2L),y-(ssize_t)
      (width/2L),image->columns+width,width,exception);
    q=GetCacheViewAuthenticPixels(convolve_view,0,y,convolve_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    convolve_indexes=GetCacheViewAuthenticIndexQueue(convolve_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickPixelPacket
        pixel;

      register const MagickRealType
        *restrict k;

      register const PixelPacket
        *restrict kernel_pixels;

      register ssize_t
        u;

      ssize_t
        v;

      pixel=bias;
      k=normal_kernel;
      kernel_pixels=p;
      if (((channel & OpacityChannel) == 0) || (image->matte == MagickFalse))
        {
          for (v=0; v < (ssize_t) width; v++)
          {
            for (u=0; u < (ssize_t) width; u++)
            {
              pixel.red+=(*k)*kernel_pixels[u].red;
              pixel.green+=(*k)*kernel_pixels[u].green;
              pixel.blue+=(*k)*kernel_pixels[u].blue;
              k++;
            }
            kernel_pixels+=image->columns+width;
          }
          if ((channel & RedChannel) != 0)
            SetPixelRed(q,ClampToQuantum(pixel.red));
          if ((channel & GreenChannel) != 0)
            SetPixelGreen(q,ClampToQuantum(pixel.green));
          if ((channel & BlueChannel) != 0)
            SetPixelBlue(q,ClampToQuantum(pixel.blue));
          if ((channel & OpacityChannel) != 0)
            {
              k=normal_kernel;
              kernel_pixels=p;
              for (v=0; v < (ssize_t) width; v++)
              {
                for (u=0; u < (ssize_t) width; u++)
                {
                  pixel.opacity+=(*k)*kernel_pixels[u].opacity;
                  k++;
                }
                kernel_pixels+=image->columns+width;
              }
              SetPixelOpacity(q,ClampToQuantum(pixel.opacity));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=normal_kernel;
              kernel_indexes=indexes;
              for (v=0; v < (ssize_t) width; v++)
              {
                for (u=0; u < (ssize_t) width; u++)
                {
                  pixel.index+=(*k)*GetPixelIndex(kernel_indexes+u);
                  k++;
                }
                kernel_indexes+=image->columns+width;
              }
              SetPixelIndex(convolve_indexes+x,ClampToQuantum(pixel.index));
            }
        }
      else
        {
          MagickRealType
            alpha,
            gamma;

          gamma=0.0;
          for (v=0; v < (ssize_t) width; v++)
          {
            for (u=0; u < (ssize_t) width; u++)
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
          gamma=PerceptibleReciprocal(gamma);
          if ((channel & RedChannel) != 0)
            SetPixelRed(q,ClampToQuantum(gamma*pixel.red));
          if ((channel & GreenChannel) != 0)
            SetPixelGreen(q,ClampToQuantum(gamma*pixel.green));
          if ((channel & BlueChannel) != 0)
            SetPixelBlue(q,ClampToQuantum(gamma*pixel.blue));
          if ((channel & OpacityChannel) != 0)
            {
              k=normal_kernel;
              kernel_pixels=p;
              for (v=0; v < (ssize_t) width; v++)
              {
                for (u=0; u < (ssize_t) width; u++)
                {
                  pixel.opacity+=(*k)*GetPixelOpacity(kernel_pixels+u);
                  k++;
                }
                kernel_pixels+=image->columns+width;
              }
              SetPixelOpacity(q,ClampToQuantum(pixel.opacity));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=normal_kernel;
              kernel_pixels=p;
              kernel_indexes=indexes;
              for (v=0; v < (ssize_t) width; v++)
              {
                for (u=0; u < (ssize_t) width; u++)
                {
                  alpha=(MagickRealType) (QuantumScale*(QuantumRange-
                    kernel_pixels[u].opacity));
                  pixel.index+=(*k)*alpha*GetPixelIndex(
                    kernel_indexes+u);
                  k++;
                }
                kernel_pixels+=image->columns+width;
                kernel_indexes+=image->columns+width;
              }
              SetPixelIndex(convolve_indexes+x,ClampToQuantum(gamma*
                pixel.index));
            }
        }
      indexes++;
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
  normal_kernel=(MagickRealType *) RelinquishAlignedMemory(normal_kernel);
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
%  edges of the original image.  A speckle removing filter uses a complementary %  hulling technique (raising pixels that are darker than their surrounding
%  neighbors, then complementarily lowering pixels that are brighter than their
%  surrounding neighbors) to reduce the speckle index of that image (reference
%  Crimmins speckle removal).
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

static void Hull(const Image *image,const ssize_t x_offset,
  const ssize_t y_offset,const size_t columns,const size_t rows,
  const int polarity,Quantum *restrict f,Quantum *restrict g)
{
  register Quantum
    *p,
    *q,
    *r,
    *s;

  ssize_t
    y;

  assert(f != (Quantum *) NULL);
  assert(g != (Quantum *) NULL);
  p=f+(columns+2);
  q=g+(columns+2);
  r=p+(y_offset*(columns+2)+x_offset);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) \
    dynamic_number_threads(image,columns,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    register ssize_t
      i,
      x;

    SignedQuantum
      v;

    i=(2*y+1)+y*columns;
    if (polarity > 0)
      for (x=0; x < (ssize_t) columns; x++)
      {
        v=(SignedQuantum) p[i];
        if ((SignedQuantum) r[i] >= (v+ScaleCharToQuantum(2)))
          v+=ScaleCharToQuantum(1);
        q[i]=(Quantum) v;
        i++;
      }
    else
      for (x=0; x < (ssize_t) columns; x++)
      {
        v=(SignedQuantum) p[i];
        if ((SignedQuantum) r[i] <= (v-ScaleCharToQuantum(2)))
          v-=ScaleCharToQuantum(1);
        q[i]=(Quantum) v;
        i++;
      }
  }
  p=f+(columns+2);
  q=g+(columns+2);
  r=q+(y_offset*(columns+2)+x_offset);
  s=q-(y_offset*(columns+2)+x_offset);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) \
    dynamic_number_threads(image,columns,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    register ssize_t
      i,
      x;

    SignedQuantum
      v;

    i=(2*y+1)+y*columns;
    if (polarity > 0)
      for (x=0; x < (ssize_t) columns; x++)
      {
        v=(SignedQuantum) q[i];
        if (((SignedQuantum) s[i] >= (v+ScaleCharToQuantum(2))) &&
            ((SignedQuantum) r[i] > v))
          v+=ScaleCharToQuantum(1);
        p[i]=(Quantum) v;
        i++;
      }
    else
      for (x=0; x < (ssize_t) columns; x++)
      {
        v=(SignedQuantum) q[i];
        if (((SignedQuantum) s[i] <= (v-ScaleCharToQuantum(2))) &&
            ((SignedQuantum) r[i] < v))
          v-=ScaleCharToQuantum(1);
        p[i]=(Quantum) v;
        i++;
      }
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

  MagickBooleanType
    status;

  register ssize_t
    i;

  Quantum
    *restrict buffer,
    *restrict pixels;

  size_t
    length,
    number_channels;

  static const ssize_t
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
    Allocate image buffer.
  */
  length=(size_t) ((image->columns+2)*(image->rows+2));
  pixels=(Quantum *) AcquireQuantumMemory(length,sizeof(*pixels));
  buffer=(Quantum *) AcquireQuantumMemory(length,sizeof(*pixels));
  if ((pixels == (Quantum *) NULL) || (buffer == (Quantum *) NULL))
    {
      if (buffer != (Quantum *) NULL)
        buffer=(Quantum *) RelinquishMagickMemory(buffer);
      if (pixels != (Quantum *) NULL)
        pixels=(Quantum *) RelinquishMagickMemory(pixels);
      despeckle_image=DestroyImage(despeckle_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Reduce speckle in the image.
  */
  status=MagickTrue;
  number_channels=(size_t) (image->colorspace == CMYKColorspace ? 5 : 4);
  image_view=AcquireVirtualCacheView(image,exception);
  despeckle_view=AcquireAuthenticCacheView(despeckle_image,exception);
  for (i=0; i < (ssize_t) number_channels; i++)
  {
    register ssize_t
      k,
      x;

    ssize_t
      j,
      y;

    if (status == MagickFalse)
      continue;
    if ((image->matte == MagickFalse) && (i == 3))
      continue;
    (void) ResetMagickMemory(pixels,0,length*sizeof(*pixels));
    j=(ssize_t) image->columns+2;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      register const IndexPacket
        *restrict indexes;

      register const PixelPacket
        *restrict p;

      p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      j++;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        switch (i)
        {
          case 0: pixels[j]=GetPixelRed(p); break;
          case 1: pixels[j]=GetPixelGreen(p); break;
          case 2: pixels[j]=GetPixelBlue(p); break;
          case 3: pixels[j]=GetPixelOpacity(p); break;
          case 4: pixels[j]=GetPixelBlack(indexes+x); break;
          default: break;
        }
        p++;
        j++;
      }
      j++;
    }
    (void) ResetMagickMemory(buffer,0,length*sizeof(*buffer));
    for (k=0; k < 4; k++)
    {
      Hull(image,X[k],Y[k],image->columns,image->rows,1,pixels,buffer);
      Hull(image,-X[k],-Y[k],image->columns,image->rows,1,pixels,buffer);
      Hull(image,-X[k],-Y[k],image->columns,image->rows,-1,pixels,buffer);
      Hull(image,X[k],Y[k],image->columns,image->rows,-1,pixels,buffer);
    }
    j=(ssize_t) image->columns+2;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      MagickBooleanType
        sync;

      register IndexPacket
        *restrict indexes;

      register PixelPacket
        *restrict q;

      q=QueueCacheViewAuthenticPixels(despeckle_view,0,y,
        despeckle_image->columns,1,exception);
      if (q == (PixelPacket *) NULL)
        break;
      indexes=GetCacheViewAuthenticIndexQueue(despeckle_view);
      j++;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        switch (i)
        {
          case 0: SetPixelRed(q,pixels[j]); break;
          case 1: SetPixelGreen(q,pixels[j]); break;
          case 2: SetPixelBlue(q,pixels[j]); break;
          case 3: SetPixelOpacity(q,pixels[j]); break;
          case 4: SetPixelIndex(indexes+x,pixels[j]); break;
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

        proceed=SetImageProgress(image,DespeckleImageTag,(MagickOffsetType) i,
          number_channels);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  despeckle_view=DestroyCacheView(despeckle_view);
  image_view=DestroyCacheView(image_view);
  buffer=(Quantum *) RelinquishMagickMemory(buffer);
  pixels=(Quantum *) RelinquishMagickMemory(pixels);
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

  register ssize_t
    i;

  size_t
    width;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth1D(radius,0.5);
  kernel=(double *) MagickAssumeAligned(AcquireAlignedMemory((size_t) width,
    width*sizeof(*kernel)));
  if (kernel == (double *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  for (i=0; i < (ssize_t) (width*width); i++)
    kernel[i]=(-1.0);
  kernel[i/2]=(double) (width*width-1.0);
  edge_image=ConvolveImage(image,width,kernel,exception);
  kernel=(double *) RelinquishAlignedMemory(kernel);
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

  register ssize_t
    i;

  size_t
    width;

  ssize_t
    j,
    k,
    u,
    v;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth2D(radius,sigma);
  kernel=(double *) MagickAssumeAligned(AcquireAlignedMemory((size_t) width,
    width*sizeof(*kernel)));
  if (kernel == (double *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  j=(ssize_t) width/2;
  k=j;
  i=0;
  for (v=(-j); v <= j; v++)
  {
    for (u=(-j); u <= j; u++)
    {
      kernel[i]=(double) (((u < 0) || (v < 0) ? -8.0 : 8.0)*
        exp(-((double) u*u+v*v)/(2.0*MagickSigma*MagickSigma))/
        (2.0*MagickPI*MagickSigma*MagickSigma));
      if (u != k)
        kernel[i]=0.0;
      i++;
    }
    k--;
  }
  emboss_image=ConvolveImage(image,width,kernel,exception);
  if (emboss_image != (Image *) NULL)
    (void) EqualizeImage(emboss_image);
  kernel=(double *) RelinquishAlignedMemory(kernel);
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

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    bias;

  MagickRealType
    *filter_kernel;

  register ssize_t
    i;

  ssize_t
    y;

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

      register const double
        *k;

      ssize_t
        u,
        v;

      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  FilterImage with %.20gx%.20g kernel:",(double) kernel->width,(double)
        kernel->height);
      message=AcquireString("");
      k=kernel->values;
      for (v=0; v < (ssize_t) kernel->height; v++)
      {
        *message='\0';
        (void) FormatLocaleString(format,MaxTextExtent,"%.20g: ",(double) v);
        (void) ConcatenateString(&message,format);
        for (u=0; u < (ssize_t) kernel->width; u++)
        {
          (void) FormatLocaleString(format,MaxTextExtent,"%g ",*k++);
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
    Normalize kernel.
  */
  filter_kernel=(MagickRealType *) MagickAssumeAligned(AcquireAlignedMemory(
    kernel->width,kernel->width*sizeof(*filter_kernel)));
  if (filter_kernel == (MagickRealType *) NULL)
    {
      filter_image=DestroyImage(filter_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  for (i=0; i < (ssize_t) (kernel->width*kernel->width); i++)
    filter_kernel[i]=(MagickRealType) kernel->values[i];
  /*
    Filter image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&bias);
  SetMagickPixelPacketBias(image,&bias);
  image_view=AcquireVirtualCacheView(image,exception);
  filter_view=AcquireAuthenticCacheView(filter_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict filter_indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) kernel->width/2L),y-
      (ssize_t) (kernel->height/2L),image->columns+kernel->width,kernel->height,
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
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickPixelPacket
        pixel;

      register const MagickRealType
        *restrict k;

      register const PixelPacket
        *restrict kernel_pixels;

      register ssize_t
        u;

      ssize_t
        v;

      pixel=bias;
      k=filter_kernel;
      kernel_pixels=p;
      if (((channel & OpacityChannel) == 0) || (image->matte == MagickFalse))
        {
          for (v=0; v < (ssize_t) kernel->width; v++)
          {
            for (u=0; u < (ssize_t) kernel->height; u++)
            {
              pixel.red+=(*k)*kernel_pixels[u].red;
              pixel.green+=(*k)*kernel_pixels[u].green;
              pixel.blue+=(*k)*kernel_pixels[u].blue;
              k++;
            }
            kernel_pixels+=image->columns+kernel->width;
          }
          if ((channel & RedChannel) != 0)
            SetPixelRed(q,ClampToQuantum(pixel.red));
          if ((channel & GreenChannel) != 0)
            SetPixelGreen(q,ClampToQuantum(pixel.green));
          if ((channel & BlueChannel) != 0)
            SetPixelBlue(q,ClampToQuantum(pixel.blue));
          if ((channel & OpacityChannel) != 0)
            {
              k=filter_kernel;
              kernel_pixels=p;
              for (v=0; v < (ssize_t) kernel->width; v++)
              {
                for (u=0; u < (ssize_t) kernel->height; u++)
                {
                  pixel.opacity+=(*k)*kernel_pixels[u].opacity;
                  k++;
                }
                kernel_pixels+=image->columns+kernel->width;
              }
              SetPixelOpacity(q,ClampToQuantum(pixel.opacity));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=filter_kernel;
              kernel_indexes=indexes;
              for (v=0; v < (ssize_t) kernel->width; v++)
              {
                for (u=0; u < (ssize_t) kernel->height; u++)
                {
                  pixel.index+=(*k)*GetPixelIndex(kernel_indexes+u);
                  k++;
                }
                kernel_indexes+=image->columns+kernel->width;
              }
              SetPixelIndex(filter_indexes+x,ClampToQuantum(pixel.index));
            }
        }
      else
        {
          MagickRealType
            alpha,
            gamma;

          gamma=0.0;
          for (v=0; v < (ssize_t) kernel->width; v++)
          {
            for (u=0; u < (ssize_t) kernel->height; u++)
            {
              alpha=(MagickRealType) (QuantumScale*(QuantumRange-
                GetPixelOpacity(kernel_pixels+u)));
              pixel.red+=(*k)*alpha*GetPixelRed(kernel_pixels+u);
              pixel.green+=(*k)*alpha*GetPixelGreen(kernel_pixels+u);
              pixel.blue+=(*k)*alpha*GetPixelBlue(kernel_pixels+u);
              gamma+=(*k)*alpha;
              k++;
            }
            kernel_pixels+=image->columns+kernel->width;
          }
          gamma=PerceptibleReciprocal(gamma);
          if ((channel & RedChannel) != 0)
            SetPixelRed(q,ClampToQuantum(gamma*pixel.red));
          if ((channel & GreenChannel) != 0)
            SetPixelGreen(q,ClampToQuantum(gamma*pixel.green));
          if ((channel & BlueChannel) != 0)
            SetPixelBlue(q,ClampToQuantum(gamma*pixel.blue));
          if ((channel & OpacityChannel) != 0)
            {
              k=filter_kernel;
              kernel_pixels=p;
              for (v=0; v < (ssize_t) kernel->width; v++)
              {
                for (u=0; u < (ssize_t) kernel->height; u++)
                {
                  pixel.opacity+=(*k)*GetPixelOpacity(kernel_pixels+u);
                  k++;
                }
                kernel_pixels+=image->columns+kernel->width;
              }
              SetPixelOpacity(q,ClampToQuantum(pixel.opacity));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              register const IndexPacket
                *restrict kernel_indexes;

              k=filter_kernel;
              kernel_pixels=p;
              kernel_indexes=indexes;
              for (v=0; v < (ssize_t) kernel->width; v++)
              {
                for (u=0; u < (ssize_t) kernel->height; u++)
                {
                  alpha=(MagickRealType) (QuantumScale*(QuantumRange-
                    kernel_pixels[u].opacity));
                  pixel.index+=(*k)*alpha*GetPixelIndex(kernel_indexes+u);
                  k++;
                }
                kernel_pixels+=image->columns+kernel->width;
                kernel_indexes+=image->columns+kernel->width;
              }
              SetPixelIndex(filter_indexes+x,ClampToQuantum(gamma*pixel.index));
            }
        }
      indexes++;
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
  filter_kernel=(MagickRealType *) RelinquishAlignedMemory(filter_kernel);
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

  register ssize_t
    i;

  size_t
    width;

  ssize_t
    j,
    u,
    v;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth2D(radius,sigma);
  kernel=(double *) MagickAssumeAligned(AcquireAlignedMemory((size_t) width,
    width*sizeof(*kernel)));
  if (kernel == (double *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  j=(ssize_t) width/2;
  i=0;
  for (v=(-j); v <= j; v++)
  {
    for (u=(-j); u <= j; u++)
      kernel[i++]=(double) (exp(-((double) u*u+v*v)/(2.0*MagickSigma*
        MagickSigma))/(2.0*MagickPI*MagickSigma*MagickSigma));
  }
  blur_image=ConvolveImageChannel(image,channel,width,kernel,exception);
  kernel=(double *) RelinquishAlignedMemory(kernel);
  return(blur_image);
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
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o angle: Apply the effect along this angle.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static double *GetMotionBlurKernel(const size_t width,const double sigma)
{
  double
    *kernel,
    normalize;

  register ssize_t
    i;

  /*
    Generate a 1-D convolution kernel.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  kernel=(double *) MagickAssumeAligned(AcquireAlignedMemory((size_t) width,
    sizeof(*kernel)));
  if (kernel == (double *) NULL)
    return(kernel);
  normalize=0.0;
  for (i=0; i < (ssize_t) width; i++)
  {
    kernel[i]=(double) (exp((-((double) i*i)/(double) (2.0*MagickSigma*
      MagickSigma)))/(MagickSQ2PI*MagickSigma));
    normalize+=kernel[i];
  }
  for (i=0; i < (ssize_t) width; i++)
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

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    bias;

  OffsetInfo
    *offset;

  PointInfo
    point;

  register ssize_t
    i;

  size_t
    width;

  ssize_t
    y;

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
      kernel=(double *) RelinquishAlignedMemory(kernel);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  blur_image=CloneImage(image,0,0,MagickTrue,exception);
  if (blur_image == (Image *) NULL)
    {
      kernel=(double *) RelinquishAlignedMemory(kernel);
      offset=(OffsetInfo *) RelinquishMagickMemory(offset);
      return((Image *) NULL);
    }
  if (SetImageStorageClass(blur_image,DirectClass) == MagickFalse)
    {
      kernel=(double *) RelinquishAlignedMemory(kernel);
      offset=(OffsetInfo *) RelinquishMagickMemory(offset);
      InheritException(exception,&blur_image->exception);
      blur_image=DestroyImage(blur_image);
      return((Image *) NULL);
    }
  point.x=(double) width*sin(DegreesToRadians(angle));
  point.y=(double) width*cos(DegreesToRadians(angle));
  for (i=0; i < (ssize_t) width; i++)
  {
    offset[i].x=(ssize_t) ceil((double) (i*point.y)/hypot(point.x,point.y)-0.5);
    offset[i].y=(ssize_t) ceil((double) (i*point.x)/hypot(point.x,point.y)-0.5);
  }
  /*
    Motion blur image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&bias);
  image_view=AcquireVirtualCacheView(image,exception);
  blur_view=AcquireAuthenticCacheView(blur_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *restrict blur_indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

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
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickPixelPacket
        qixel;

      PixelPacket
        pixel;

      register const IndexPacket
        *restrict indexes;

      register double
        *restrict k;

      register ssize_t
        i;

      k=kernel;
      qixel=bias;
      if (((channel & OpacityChannel) == 0) || (image->matte == MagickFalse))
        {
          for (i=0; i < (ssize_t) width; i++)
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
            SetPixelRed(q,ClampToQuantum(qixel.red));
          if ((channel & GreenChannel) != 0)
            SetPixelGreen(q,ClampToQuantum(qixel.green));
          if ((channel & BlueChannel) != 0)
            SetPixelBlue(q,ClampToQuantum(qixel.blue));
          if ((channel & OpacityChannel) != 0)
            SetPixelOpacity(q,ClampToQuantum(qixel.opacity));
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            SetPixelIndex(blur_indexes+x,ClampToQuantum(qixel.index));
        }
      else
        {
          MagickRealType
            alpha,
            gamma;

          alpha=0.0;
          gamma=0.0;
          for (i=0; i < (ssize_t) width; i++)
          {
            (void) GetOneCacheViewVirtualPixel(image_view,x+offset[i].x,y+
              offset[i].y,&pixel,exception);
            alpha=(MagickRealType) (QuantumScale*GetPixelAlpha(&pixel));
            qixel.red+=(*k)*alpha*pixel.red;
            qixel.green+=(*k)*alpha*pixel.green;
            qixel.blue+=(*k)*alpha*pixel.blue;
            qixel.opacity+=(*k)*pixel.opacity;
            if (image->colorspace == CMYKColorspace)
              {
                indexes=GetCacheViewVirtualIndexQueue(image_view);
                qixel.index+=(*k)*alpha*GetPixelIndex(indexes);
              }
            gamma+=(*k)*alpha;
            k++;
          }
          gamma=PerceptibleReciprocal(gamma);
          if ((channel & RedChannel) != 0)
            SetPixelRed(q,ClampToQuantum(gamma*qixel.red));
          if ((channel & GreenChannel) != 0)
            SetPixelGreen(q,ClampToQuantum(gamma*qixel.green));
          if ((channel & BlueChannel) != 0)
            SetPixelBlue(q,ClampToQuantum(gamma*qixel.blue));
          if ((channel & OpacityChannel) != 0)
            SetPixelOpacity(q,ClampToQuantum(qixel.opacity));
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            SetPixelIndex(blur_indexes+x,ClampToQuantum(gamma*qixel.index));
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
  kernel=(double *) RelinquishAlignedMemory(kernel);
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

  MagickBooleanType
    proceed;

  MontageInfo
    *montage_info;

  QuantizeInfo
    quantize_info;

  RectangleInfo
    geometry;

  register ssize_t
    i,
    x;

  size_t
    colors;

  ssize_t
    y;

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
        (void) FormatLocaleString(label,MaxTextExtent,"rotate %g",degrees);
        break;
      }
      case ShearPreview:
      {
        degrees+=5.0;
        preview_image=ShearImage(thumbnail,degrees,degrees,exception);
        (void) FormatLocaleString(label,MaxTextExtent,"shear %gx%g",
          degrees,2.0*degrees);
        break;
      }
      case RollPreview:
      {
        x=(ssize_t) ((i+1)*thumbnail->columns)/NumberTiles;
        y=(ssize_t) ((i+1)*thumbnail->rows)/NumberTiles;
        preview_image=RollImage(thumbnail,x,y,exception);
        (void) FormatLocaleString(label,MaxTextExtent,"roll %+.20gx%+.20g",
          (double) x,(double) y);
        break;
      }
      case HuePreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) FormatLocaleString(factor,MaxTextExtent,"100,100,%g",
          2.0*percentage);
        (void) ModulateImage(preview_image,factor);
        (void) FormatLocaleString(label,MaxTextExtent,"modulate %s",factor);
        break;
      }
      case SaturationPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) FormatLocaleString(factor,MaxTextExtent,"100,%g",
          2.0*percentage);
        (void) ModulateImage(preview_image,factor);
        (void) FormatLocaleString(label,MaxTextExtent,"modulate %s",factor);
        break;
      }
      case BrightnessPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) FormatLocaleString(factor,MaxTextExtent,"%g",2.0*percentage);
        (void) ModulateImage(preview_image,factor);
        (void) FormatLocaleString(label,MaxTextExtent,"modulate %s",factor);
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
        (void) FormatLocaleString(label,MaxTextExtent,"gamma %g",gamma);
        break;
      }
      case SpiffPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image != (Image *) NULL)
          for (x=0; x < i; x++)
            (void) ContrastImage(preview_image,MagickTrue);
        (void) FormatLocaleString(label,MaxTextExtent,"contrast (%.20g)",
          (double) i+1);
        break;
      }
      case DullPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        for (x=0; x < i; x++)
          (void) ContrastImage(preview_image,MagickFalse);
        (void) FormatLocaleString(label,MaxTextExtent,"+contrast (%.20g)",
          (double) i+1);
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
        (void) FormatLocaleString(label,MaxTextExtent,
          "-colorspace gray -colors %.20g",(double) colors);
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
        (void) FormatLocaleString(label,MaxTextExtent,"colors %.20g",(double)
          colors);
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
        (void) FormatLocaleString(label,MaxTextExtent,"despeckle (%.20g)",
          (double) i+1);
        break;
      }
      case ReduceNoisePreview:
      {
        preview_image=StatisticImage(thumbnail,NonpeakStatistic,(size_t) radius,
          (size_t) radius,exception);
        (void) FormatLocaleString(label,MaxTextExtent,"noise %g",radius);
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
        preview_image=StatisticImage(thumbnail,NonpeakStatistic,(size_t) i,
          (size_t) i,exception);
        (void) FormatLocaleString(label,MaxTextExtent,"+noise %s",factor);
        break;
      }
      case SharpenPreview:
      {
        preview_image=SharpenImage(thumbnail,radius,sigma,exception);
        (void) FormatLocaleString(label,MaxTextExtent,"sharpen %gx%g",
          radius,sigma);
        break;
      }
      case BlurPreview:
      {
        preview_image=BlurImage(thumbnail,radius,sigma,exception);
        (void) FormatLocaleString(label,MaxTextExtent,"blur %gx%g",radius,
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
        (void) FormatLocaleString(label,MaxTextExtent,"threshold %g",
          (double) (percentage*((MagickRealType) QuantumRange+1.0))/100.0);
        break;
      }
      case EdgeDetectPreview:
      {
        preview_image=EdgeImage(thumbnail,radius,exception);
        (void) FormatLocaleString(label,MaxTextExtent,"edge %g",radius);
        break;
      }
      case SpreadPreview:
      {
        preview_image=SpreadImage(thumbnail,radius,exception);
        (void) FormatLocaleString(label,MaxTextExtent,"spread %g",
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
        (void) FormatLocaleString(label,MaxTextExtent,"solarize %g",
          (QuantumRange*percentage)/100.0);
        break;
      }
      case ShadePreview:
      {
        degrees+=10.0;
        preview_image=ShadeImage(thumbnail,MagickTrue,degrees,degrees,
          exception);
        (void) FormatLocaleString(label,MaxTextExtent,"shade %gx%g",
          degrees,degrees);
        break;
      }
      case RaisePreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        geometry.width=(size_t) (2*i+2);
        geometry.height=(size_t) (2*i+2);
        geometry.x=i/2;
        geometry.y=i/2;
        (void) RaiseImage(preview_image,&geometry,MagickTrue);
        (void) FormatLocaleString(label,MaxTextExtent,
          "raise %.20gx%.20g%+.20g%+.20g",(double) geometry.width,(double)
          geometry.height,(double) geometry.x,(double) geometry.y);
        break;
      }
      case SegmentPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        threshold+=0.4f;
        (void) SegmentImage(preview_image,sRGBColorspace,MagickFalse,threshold,
          threshold);
        (void) FormatLocaleString(label,MaxTextExtent,"segment %gx%g",
          threshold,threshold);
        break;
      }
      case SwirlPreview:
      {
        preview_image=SwirlImage(thumbnail,degrees,exception);
        (void) FormatLocaleString(label,MaxTextExtent,"swirl %g",degrees);
        degrees+=45.0;
        break;
      }
      case ImplodePreview:
      {
        degrees+=0.1f;
        preview_image=ImplodeImage(thumbnail,degrees,exception);
        (void) FormatLocaleString(label,MaxTextExtent,"implode %g",degrees);
        break;
      }
      case WavePreview:
      {
        degrees+=5.0f;
        preview_image=WaveImage(thumbnail,0.5*degrees,2.0*degrees,exception);
        (void) FormatLocaleString(label,MaxTextExtent,"wave %gx%g",
          0.5*degrees,2.0*degrees);
        break;
      }
      case OilPaintPreview:
      {
        preview_image=OilPaintImage(thumbnail,(double) radius,exception);
        (void) FormatLocaleString(label,MaxTextExtent,"paint %g",radius);
        break;
      }
      case CharcoalDrawingPreview:
      {
        preview_image=CharcoalImage(thumbnail,(double) radius,(double) sigma,
          exception);
        (void) FormatLocaleString(label,MaxTextExtent,"charcoal %gx%g",
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
        preview_info->quality=(size_t) percentage;
        (void) FormatLocaleString(factor,MaxTextExtent,"%.20g",(double)
          preview_info->quality);
        file=AcquireUniqueFileResource(filename);
        if (file != -1)
          file=close(file)-1;
        (void) FormatLocaleString(preview_image->filename,MaxTextExtent,
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
          (void) FormatLocaleString(label,MaxTextExtent,"quality %s\n%gmb ",
            factor,(double) ((MagickOffsetType) GetBlobSize(preview_image))/
            1024.0/1024.0);
        else
          if (GetBlobSize(preview_image) >= 1024)
            (void) FormatLocaleString(label,MaxTextExtent,
              "quality %s\n%gkb ",factor,(double) ((MagickOffsetType)
              GetBlobSize(preview_image))/1024.0);
          else
            (void) FormatLocaleString(label,MaxTextExtent,"quality %s\n%.20gb ",
              factor,(double) ((MagickOffsetType) GetBlobSize(thumbnail)));
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
    proceed=SetImageProgress(image,PreviewImageTag,(MagickOffsetType) i,
      NumberTiles);
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

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

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

  register ssize_t
    i;

  size_t
    n;

  ssize_t
    y;

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
  n=(size_t) fabs(4.0*DegreesToRadians(angle)*sqrt((double) blur_radius)+2UL);
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
  for (i=0; i < (ssize_t) n; i++)
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
  image_view=AcquireVirtualCacheView(image,exception);
  blur_view=AcquireAuthenticCacheView(blur_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) blur_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register IndexPacket
      *restrict blur_indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

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
    for (x=0; x < (ssize_t) blur_image->columns; x++)
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

      register ssize_t
        i;

      size_t
        step;

      center.x=(double) x-blur_center.x;
      center.y=(double) y-blur_center.y;
      radius=hypot((double) center.x,center.y);
      if (radius == 0)
        step=1;
      else
        {
          step=(size_t) (blur_radius/radius);
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
          for (i=0; i < (ssize_t) n; i+=(ssize_t) step)
          {
            (void) GetOneCacheViewVirtualPixel(image_view,(ssize_t)
              (blur_center.x+center.x*cos_theta[i]-center.y*sin_theta[i]+0.5),
              (ssize_t) (blur_center.y+center.x*sin_theta[i]+center.y*
              cos_theta[i]+0.5),&pixel,exception);
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
          normalize=PerceptibleReciprocal(normalize);
          if ((channel & RedChannel) != 0)
            SetPixelRed(q,ClampToQuantum(normalize*qixel.red));
          if ((channel & GreenChannel) != 0)
            SetPixelGreen(q,ClampToQuantum(normalize*qixel.green));
          if ((channel & BlueChannel) != 0)
            SetPixelBlue(q,ClampToQuantum(normalize*qixel.blue));
          if ((channel & OpacityChannel) != 0)
            SetPixelOpacity(q,ClampToQuantum(normalize*qixel.opacity));
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            SetPixelIndex(blur_indexes+x,ClampToQuantum(normalize*qixel.index));
        }
      else
        {
          MagickRealType
            alpha,
            gamma;

          alpha=1.0;
          gamma=0.0;
          for (i=0; i < (ssize_t) n; i+=(ssize_t) step)
          {
            (void) GetOneCacheViewVirtualPixel(image_view,(ssize_t)
              (blur_center.x+center.x*cos_theta[i]-center.y*sin_theta[i]+0.5),
              (ssize_t) (blur_center.y+center.x*sin_theta[i]+center.y*
              cos_theta[i]+0.5),&pixel,exception);
            alpha=(MagickRealType) (QuantumScale*
              GetPixelAlpha(&pixel));
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
          gamma=PerceptibleReciprocal(gamma);
          normalize=PerceptibleReciprocal(normalize);
          if ((channel & RedChannel) != 0)
            SetPixelRed(q,ClampToQuantum(gamma*qixel.red));
          if ((channel & GreenChannel) != 0)
            SetPixelGreen(q,ClampToQuantum(gamma*qixel.green));
          if ((channel & BlueChannel) != 0)
            SetPixelBlue(q,ClampToQuantum(gamma*qixel.blue));
          if ((channel & OpacityChannel) != 0)
            SetPixelOpacity(q,ClampToQuantum(normalize*qixel.opacity));
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            SetPixelIndex(blur_indexes+x,ClampToQuantum(gamma*qixel.index));
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
    *image_view,
    *luminance_view;

  double
    *kernel;

  Image
    *blur_image,
    *luminance_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    bias;

  register ssize_t
    i;

  size_t
    width;

  ssize_t
    center,
    j,
    u,
    v,
    y;

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
  kernel=(double *) MagickAssumeAligned(AcquireAlignedMemory((size_t) width,
    width*sizeof(*kernel)));
  if (kernel == (double *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  j=(ssize_t) width/2;
  i=0;
  for (v=(-j); v <= j; v++)
  {
    for (u=(-j); u <= j; u++)
      kernel[i++]=(double) (exp(-((double) u*u+v*v)/(2.0*MagickSigma*
        MagickSigma))/(2.0*MagickPI*MagickSigma*MagickSigma));
  }
  if (image->debug != MagickFalse)
    {
      char
        format[MaxTextExtent],
        *message;

      register const double
        *k;

      ssize_t
        u,
        v;

      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  SelectiveBlurImage with %.20gx%.20g kernel:",(double) width,(double)
        width);
      message=AcquireString("");
      k=kernel;
      for (v=0; v < (ssize_t) width; v++)
      {
        *message='\0';
        (void) FormatLocaleString(format,MaxTextExtent,"%.20g: ",(double) v);
        (void) ConcatenateString(&message,format);
        for (u=0; u < (ssize_t) width; u++)
        {
          (void) FormatLocaleString(format,MaxTextExtent,"%+f ",*k++);
          (void) ConcatenateString(&message,format);
        }
        (void) LogMagickEvent(TransformEvent,GetMagickModule(),"%s",message);
      }
      message=DestroyString(message);
    }
  blur_image=CloneImage(image,0,0,MagickTrue,exception);
  if (blur_image == (Image *) NULL)
    {
      kernel=(double *) RelinquishAlignedMemory(kernel);
      return((Image *) NULL);
    }
  if (SetImageStorageClass(blur_image,DirectClass) == MagickFalse)
    {
      kernel=(double *) RelinquishAlignedMemory(kernel);
      InheritException(exception,&blur_image->exception);
      blur_image=DestroyImage(blur_image);
      return((Image *) NULL);
    }
  luminance_image=CloneImage(image,0,0,MagickTrue,exception);
  if (luminance_image == (Image *) NULL)
    {
      kernel=(double *) RelinquishAlignedMemory(kernel);
      blur_image=DestroyImage(blur_image);
      return((Image *) NULL);
    }
  status=TransformImageColorspace(luminance_image,GRAYColorspace);
  if (status == MagickFalse)
    {
      InheritException(exception,&luminance_image->exception);
      kernel=(double *) RelinquishAlignedMemory(kernel);
      blur_image=DestroyImage(blur_image);
      luminance_image=DestroyImage(luminance_image);
      return((Image *) NULL);
    }
  /*
    Threshold blur image.
  */
  status=MagickTrue;
  progress=0;
  center=(ssize_t) ((image->columns+width)*(width/2L)+(width/2L));
  GetMagickPixelPacket(image,&bias);
  SetMagickPixelPacketBias(image,&bias);
  image_view=AcquireVirtualCacheView(image,exception);
  luminance_view=AcquireVirtualCacheView(luminance_image,exception);
  blur_view=AcquireAuthenticCacheView(blur_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      sync;

    MagickRealType
      gamma;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict l,
      *restrict p;

    register IndexPacket
      *restrict blur_indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) width/2L),y-(ssize_t)
      (width/2L),image->columns+width,width,exception);
    l=GetCacheViewVirtualPixels(luminance_view,-((ssize_t) width/2L),y-(ssize_t)
      (width/2L),luminance_image->columns+width,width,exception);
    q=GetCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) ||
        (l == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    blur_indexes=GetCacheViewAuthenticIndexQueue(blur_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        contrast;

      MagickPixelPacket
        pixel;

      MagickRealType
        intensity;

      register const double
        *restrict k;

      register ssize_t
        u;

      ssize_t
        j,
        v;

      pixel=bias;
      k=kernel;
      intensity=GetPixelIntensity(image,p+center);
      gamma=0.0;
      j=0;
      if (((channel & OpacityChannel) == 0) || (image->matte == MagickFalse))
        {
          for (v=0; v < (ssize_t) width; v++)
          {
            for (u=0; u < (ssize_t) width; u++)
            {
              contrast=GetPixelIntensity(luminance_image,l+u+j)-intensity;
              if (fabs(contrast) < threshold)
                {
                  pixel.red+=(*k)*GetPixelRed(p+u+j);
                  pixel.green+=(*k)*GetPixelGreen(p+u+j);
                  pixel.blue+=(*k)*GetPixelBlue(p+u+j);
                  gamma+=(*k);
                }
              k++;
            }
            j+=(ssize_t) (image->columns+width);
          }
          if (gamma != 0.0)
            {
              gamma=PerceptibleReciprocal(gamma);
              if ((channel & RedChannel) != 0)
                SetPixelRed(q,ClampToQuantum(gamma*pixel.red));
              if ((channel & GreenChannel) != 0)
                SetPixelGreen(q,ClampToQuantum(gamma*pixel.green));
              if ((channel & BlueChannel) != 0)
                SetPixelBlue(q,ClampToQuantum(gamma*pixel.blue));
            }
          if ((channel & OpacityChannel) != 0)
            {
              gamma=0.0;
              j=0;
              for (v=0; v < (ssize_t) width; v++)
              {
                for (u=0; u < (ssize_t) width; u++)
                {
                  contrast=GetPixelIntensity(luminance_image,l+u+j)-intensity;
                  if (fabs(contrast) < threshold)
                    {
                      pixel.opacity+=(*k)*(p+u+j)->opacity;
                      gamma+=(*k);
                    }
                  k++;
                }
                j+=(ssize_t) (image->columns+width);
              }
              gamma=PerceptibleReciprocal(gamma);
              SetPixelOpacity(q,ClampToQuantum(gamma*pixel.opacity));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              gamma=0.0;
              j=0;
              for (v=0; v < (ssize_t) width; v++)
              {
                for (u=0; u < (ssize_t) width; u++)
                {
                  contrast=GetPixelIntensity(luminance_image,l+u+j)-intensity;
                  if (fabs(contrast) < threshold)
                    {
                      pixel.index+=(*k)*GetPixelIndex(indexes+x+u+j);
                      gamma+=(*k);
                    }
                  k++;
                }
                j+=(ssize_t) (image->columns+width);
              }
              gamma=PerceptibleReciprocal(gamma);
              SetPixelIndex(blur_indexes+x,ClampToQuantum(gamma*pixel.index));
            }
        }
      else
        {
          MagickRealType
            alpha;

          for (v=0; v < (ssize_t) width; v++)
          {
            for (u=0; u < (ssize_t) width; u++)
            {
              contrast=GetPixelIntensity(luminance_image,l+u+j)-intensity;
              if (fabs(contrast) < threshold)
                {
                  alpha=(MagickRealType) (QuantumScale*GetPixelAlpha(p+u+j));
                  pixel.red+=(*k)*alpha*GetPixelRed(p+u+j);
                  pixel.green+=(*k)*alpha*GetPixelGreen(p+u+j);
                  pixel.blue+=(*k)*alpha*GetPixelBlue(p+u+j);
                  pixel.opacity+=(*k)*GetPixelOpacity(p+u+j);
                  gamma+=(*k)*alpha;
                }
              k++;
            }
            j+=(ssize_t) (image->columns+width);
          }
          if (gamma != 0.0)
            {
              gamma=PerceptibleReciprocal(gamma);
              if ((channel & RedChannel) != 0)
                SetPixelRed(q,ClampToQuantum(gamma*pixel.red));
              if ((channel & GreenChannel) != 0)
                SetPixelGreen(q,ClampToQuantum(gamma*pixel.green));
              if ((channel & BlueChannel) != 0)
                SetPixelBlue(q,ClampToQuantum(gamma*pixel.blue));
            }
          if ((channel & OpacityChannel) != 0)
            {
              gamma=0.0;
              j=0;
              for (v=0; v < (ssize_t) width; v++)
              {
                for (u=0; u < (ssize_t) width; u++)
                {
                  contrast=GetPixelIntensity(luminance_image,l+u+j)-intensity;
                  if (fabs(contrast) < threshold)
                    {
                      pixel.opacity+=(*k)*GetPixelOpacity(p+u+j);
                      gamma+=(*k);
                    }
                  k++;
                }
                j+=(ssize_t) (image->columns+width);
              }
              gamma=PerceptibleReciprocal(gamma);
              SetPixelOpacity(q,ClampToQuantum(pixel.opacity));
            }
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            {
              gamma=0.0;
              j=0;
              for (v=0; v < (ssize_t) width; v++)
              {
                for (u=0; u < (ssize_t) width; u++)
                {
                  contrast=GetPixelIntensity(luminance_image,l+u+j)-intensity;
                  if (fabs(contrast) < threshold)
                    {
                      alpha=(MagickRealType) (QuantumScale*
                        GetPixelAlpha(p+u+j));
                      pixel.index+=(*k)*alpha*GetPixelIndex(indexes+x+u+j);
                      gamma+=(*k);
                    }
                  k++;
                }
                j+=(ssize_t) (image->columns+width);
              }
              gamma=PerceptibleReciprocal(gamma);
              SetPixelIndex(blur_indexes+x,ClampToQuantum(gamma*pixel.index));
            }
        }
      p++;
      l++;
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
  luminance_view=DestroyCacheView(luminance_view);
  image_view=DestroyCacheView(image_view);
  luminance_image=DestroyImage(luminance_image);
  kernel=(double *) RelinquishAlignedMemory(kernel);
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

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PrimaryInfo
    light;

  ssize_t
    y;

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
  image_view=AcquireVirtualCacheView(image,exception);
  shade_view=AcquireAuthenticCacheView(shade_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
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

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

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
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      /*
        Determine the surface normal and compute shading.
      */
      normal.x=(double) (GetPixelIntensity(image,s0-1)+
        GetPixelIntensity(image,s1-1)+GetPixelIntensity(image,s2-1)-
        GetPixelIntensity(image,s0+1)-GetPixelIntensity(image,s1+1)-
        GetPixelIntensity(image,s2+1));
      normal.y=(double) (GetPixelIntensity(image,s2-1)+
        GetPixelIntensity(image,s2)+GetPixelIntensity(image,s2+1)-
        GetPixelIntensity(image,s0-1)-GetPixelIntensity(image,s0)-
        GetPixelIntensity(image,s0+1));
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
          SetPixelRed(q,shade);
          SetPixelGreen(q,shade);
          SetPixelBlue(q,shade);
        }
      else
        {
          SetPixelRed(q,ClampToQuantum(QuantumScale*shade*GetPixelRed(s1)));
          SetPixelGreen(q,ClampToQuantum(QuantumScale*shade*GetPixelGreen(s1)));
          SetPixelBlue(q,ClampToQuantum(QuantumScale*shade*GetPixelBlue(s1)));
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

  register ssize_t
    i;

  size_t
    width;

  ssize_t
    j,
    u,
    v;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth2D(radius,sigma);
  kernel=(double *) MagickAssumeAligned(AcquireAlignedMemory((size_t) width*
    width,sizeof(*kernel)));
  if (kernel == (double *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  normalize=0.0;
  j=(ssize_t) width/2;
  i=0;
  for (v=(-j); v <= j; v++)
  {
    for (u=(-j); u <= j; u++)
    {
      kernel[i]=(double) (-exp(-((double) u*u+v*v)/(2.0*MagickSigma*
        MagickSigma))/(2.0*MagickPI*MagickSigma*MagickSigma));
      normalize+=kernel[i];
      i++;
    }
  }
  kernel[i/2]=(double) ((-2.0)*normalize);
  sharp_image=ConvolveImageChannel(image,channel,width,kernel,exception);
  kernel=(double *) RelinquishAlignedMemory(kernel);
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
    *image_view,
    *spread_view;

  Image
    *spread_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    bias;

  RandomInfo
    **restrict random_info;

  size_t
    width;

  ssize_t
    y;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  unsigned long
    key;
#endif

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
  random_info=AcquireRandomInfoThreadSet();
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  key=GetRandomSecretKey(random_info[0]);
#endif
  image_view=AcquireVirtualCacheView(image,exception);
  spread_view=AcquireAuthenticCacheView(spread_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,8) shared(progress,status) \
    dynamic_number_threads(image,image->columns,image->rows,key == ~0UL)
#endif
  for (y=0; y < (ssize_t) spread_image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    MagickPixelPacket
      pixel;

    register IndexPacket
      *restrict indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(spread_view,0,y,spread_image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(spread_view);
    pixel=bias;
    for (x=0; x < (ssize_t) spread_image->columns; x++)
    {
      (void) InterpolateMagickPixelPacket(image,image_view,
        UndefinedInterpolatePixel,(double) x+width*(GetPseudoRandomValue(
        random_info[id])-0.5),(double) y+width*(GetPseudoRandomValue(
        random_info[id])-0.5),&pixel,exception);
      SetPixelPacket(spread_image,&pixel,q,indexes+x);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(spread_view,exception) == MagickFalse)
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
  spread_view=DestroyCacheView(spread_view);
  image_view=DestroyCacheView(image_view);
  random_info=DestroyRandomInfoThreadSet(random_info);
  if (status == MagickFalse)
    spread_image=DestroyImage(spread_image);
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

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    bias;

  MagickRealType
    quantum_threshold;

  ssize_t
    y;

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
  image_view=AcquireVirtualCacheView(image,exception);
  unsharp_view=AcquireAuthenticCacheView(unsharp_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickPixelPacket
      pixel;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict unsharp_indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

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
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        {
          pixel.red=GetPixelRed(p)-(MagickRealType) GetPixelRed(q);
          if (fabs(2.0*pixel.red) < quantum_threshold)
            pixel.red=(MagickRealType) GetPixelRed(p);
          else
            pixel.red=(MagickRealType) GetPixelRed(p)+(pixel.red*amount);
          SetPixelRed(q,ClampToQuantum(pixel.red));
        }
      if ((channel & GreenChannel) != 0)
        {
          pixel.green=GetPixelGreen(p)-(MagickRealType) q->green;
          if (fabs(2.0*pixel.green) < quantum_threshold)
            pixel.green=(MagickRealType) GetPixelGreen(p);
          else
            pixel.green=(MagickRealType) GetPixelGreen(p)+(pixel.green*amount);
          SetPixelGreen(q,ClampToQuantum(pixel.green));
        }
      if ((channel & BlueChannel) != 0)
        {
          pixel.blue=GetPixelBlue(p)-(MagickRealType) q->blue;
          if (fabs(2.0*pixel.blue) < quantum_threshold)
            pixel.blue=(MagickRealType) GetPixelBlue(p);
          else
            pixel.blue=(MagickRealType) GetPixelBlue(p)+(pixel.blue*amount);
          SetPixelBlue(q,ClampToQuantum(pixel.blue));
        }
      if ((channel & OpacityChannel) != 0)
        {
          pixel.opacity=GetPixelOpacity(p)-(MagickRealType) q->opacity;
          if (fabs(2.0*pixel.opacity) < quantum_threshold)
            pixel.opacity=(MagickRealType) GetPixelOpacity(p);
          else
            pixel.opacity=GetPixelOpacity(p)+(pixel.opacity*amount);
          SetPixelOpacity(q,ClampToQuantum(pixel.opacity));
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          pixel.index=GetPixelIndex(indexes+x)-(MagickRealType)
            GetPixelIndex(unsharp_indexes+x);
          if (fabs(2.0*pixel.index) < quantum_threshold)
            pixel.index=(MagickRealType) GetPixelIndex(indexes+x);
          else
            pixel.index=(MagickRealType) GetPixelIndex(indexes+x)+
              (pixel.index*amount);
          SetPixelIndex(unsharp_indexes+x,ClampToQuantum(pixel.index));
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

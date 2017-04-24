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
%                                    Cristy                                   %
%                                 October 1996                                %
%                                                                             %
%                                                                             %
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/blob.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/constitute.h"
#include "MagickCore/decorate.h"
#include "MagickCore/distort.h"
#include "MagickCore/draw.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/effect.h"
#include "MagickCore/fx.h"
#include "MagickCore/gem.h"
#include "MagickCore/gem-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/matrix.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/montage.h"
#include "MagickCore/morphology.h"
#include "MagickCore/morphology-private.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/pixel-private.h"
#include "MagickCore/property.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/random_.h"
#include "MagickCore/random-private.h"
#include "MagickCore/resample.h"
#include "MagickCore/resample-private.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/segment.h"
#include "MagickCore/shear.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/transform.h"
#include "MagickCore/threshold.h"

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
%
%  A description of each parameter follows:
%
%    o image: the image.
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
#define AdaptiveBlurImageTag  "Convolve/Image"
#define MagickSigma  (fabs(sigma) < MagickEpsilon ? MagickEpsilon : sigma)

  CacheView
    *blur_view,
    *edge_view,
    *image_view;

  double
    normalize,
    **kernel;

  Image
    *blur_image,
    *edge_image,
    *gaussian_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

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
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  blur_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (blur_image == (Image *) NULL)
    return((Image *) NULL);
  if (fabs(sigma) < MagickEpsilon)
    return(blur_image);
  if (SetImageStorageClass(blur_image,DirectClass,exception) == MagickFalse)
    {
      blur_image=DestroyImage(blur_image);
      return((Image *) NULL);
    }
  /*
    Edge detect the image brightness channel, level, blur, and level again.
  */
  edge_image=EdgeImage(image,radius,exception);
  if (edge_image == (Image *) NULL)
    {
      blur_image=DestroyImage(blur_image);
      return((Image *) NULL);
    }
  (void) AutoLevelImage(edge_image,exception);
  gaussian_image=BlurImage(edge_image,radius,sigma,exception);
  if (gaussian_image != (Image *) NULL)
    {
      edge_image=DestroyImage(edge_image);
      edge_image=gaussian_image;
    }
  (void) AutoLevelImage(edge_image,exception);
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
    kernel[i]=(double *) MagickAssumeAligned(AcquireAlignedMemory(
      (size_t) (width-i),(width-i)*sizeof(**kernel)));
    if (kernel[i] == (double *) NULL)
      break;
    normalize=0.0;
    j=(ssize_t) (width-i-1)/2;
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
    kernel[i][(k-1)/2]+=(double) (1.0-normalize);
    if (sigma < MagickEpsilon)
      kernel[i][(k-1)/2]=1.0;
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
  image_view=AcquireVirtualCacheView(image,exception);
  edge_view=AcquireVirtualCacheView(edge_image,exception);
  blur_view=AcquireAuthenticCacheView(blur_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,blur_image,blur_image->rows,1)
#endif
  for (y=0; y < (ssize_t) blur_image->rows; y++)
  {
    register const Quantum
      *magick_restrict r;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    r=GetCacheViewVirtualPixels(edge_view,0,y,edge_image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if ((r == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) blur_image->columns; x++)
    {
      register const Quantum
        *magick_restrict p;

      register ssize_t
        i;

      ssize_t
        center,
        j;

      j=(ssize_t) ceil((double) width*(1.0-QuantumScale*
        GetPixelIntensity(edge_image,r))-0.5);
      if (j < 0)
        j=0;
      else
        if (j > (ssize_t) width)
          j=(ssize_t) width;
      if ((j & 0x01) != 0)
        j--;
      p=GetCacheViewVirtualPixels(image_view,x-((ssize_t) (width-j)/2L),y-
        (ssize_t) ((width-j)/2L),width-j,width-j,exception);
      if (p == (const Quantum *) NULL)
        break;
      center=(ssize_t) GetPixelChannels(image)*(width-j)*((width-j)/2L)+
        GetPixelChannels(image)*((width-j)/2);
      for (i=0; i < (ssize_t) GetPixelChannels(blur_image); i++)
      {
        double
          alpha,
          gamma,
          pixel;

        PixelChannel
          channel;

        PixelTrait
          blur_traits,
          traits;

        register const double
          *magick_restrict k;

        register const Quantum
          *magick_restrict pixels;

        register ssize_t
          u;

        ssize_t
          v;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        blur_traits=GetPixelChannelTraits(blur_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (blur_traits == UndefinedPixelTrait))
          continue;
        if (((blur_traits & CopyPixelTrait) != 0) ||
            (GetPixelWriteMask(image,p+center) == 0))
          {
            SetPixelChannel(blur_image,channel,p[center+i],q);
            continue;
          }
        k=kernel[j];
        pixels=p;
        pixel=0.0;
        gamma=0.0;
        if ((blur_traits & BlendPixelTrait) == 0)
          {
            /*
              No alpha blending.
            */
            for (v=0; v < (ssize_t) (width-j); v++)
            {
              for (u=0; u < (ssize_t) (width-j); u++)
              {
                pixel+=(*k)*pixels[i];
                gamma+=(*k);
                k++;
                pixels+=GetPixelChannels(image);
              }
            }
            gamma=PerceptibleReciprocal(gamma);
            SetPixelChannel(blur_image,channel,ClampToQuantum(gamma*pixel),q);
            continue;
          }
        /*
          Alpha blending.
        */
        for (v=0; v < (ssize_t) (width-j); v++)
        {
          for (u=0; u < (ssize_t) (width-j); u++)
          {
            alpha=(double) (QuantumScale*GetPixelAlpha(image,pixels));
            pixel+=(*k)*alpha*pixels[i];
            gamma+=(*k)*alpha;
            k++;
            pixels+=GetPixelChannels(image);
          }
        }
        gamma=PerceptibleReciprocal(gamma);
        SetPixelChannel(blur_image,channel,ClampToQuantum(gamma*pixel),q);
      }
      q+=GetPixelChannels(blur_image);
      r+=GetPixelChannels(edge_image);
    }
    if (SyncCacheViewAuthenticPixels(blur_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_AdaptiveBlurImage)
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
  for (i=0; i < (ssize_t) width; i+=2)
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
%
%  A description of each parameter follows:
%
%    o image: the image.
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
#define AdaptiveSharpenImageTag  "Convolve/Image"
#define MagickSigma  (fabs(sigma) < MagickEpsilon ? MagickEpsilon : sigma)

  CacheView
    *sharp_view,
    *edge_view,
    *image_view;

  double
    normalize,
    **kernel;

  Image
    *sharp_image,
    *edge_image,
    *gaussian_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

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
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  sharp_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (sharp_image == (Image *) NULL)
    return((Image *) NULL);
  if (fabs(sigma) < MagickEpsilon)
    return(sharp_image);
  if (SetImageStorageClass(sharp_image,DirectClass,exception) == MagickFalse)
    {
      sharp_image=DestroyImage(sharp_image);
      return((Image *) NULL);
    }
  /*
    Edge detect the image brightness channel, level, sharp, and level again.
  */
  edge_image=EdgeImage(image,radius,exception);
  if (edge_image == (Image *) NULL)
    {
      sharp_image=DestroyImage(sharp_image);
      return((Image *) NULL);
    }
  (void) AutoLevelImage(edge_image,exception);
  gaussian_image=BlurImage(edge_image,radius,sigma,exception);
  if (gaussian_image != (Image *) NULL)
    {
      edge_image=DestroyImage(edge_image);
      edge_image=gaussian_image;
    }
  (void) AutoLevelImage(edge_image,exception);
  /*
    Create a set of kernels from maximum (radius,sigma) to minimum.
  */
  width=GetOptimalKernelWidth2D(radius,sigma);
  kernel=(double **) MagickAssumeAligned(AcquireAlignedMemory((size_t)
    width,sizeof(*kernel)));
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
    j=(ssize_t) (width-i-1)/2;
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
    kernel[i][(k-1)/2]=(double) ((-2.0)*normalize);
    if (sigma < MagickEpsilon)
      kernel[i][(k-1)/2]=1.0;
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
  image_view=AcquireVirtualCacheView(image,exception);
  edge_view=AcquireVirtualCacheView(edge_image,exception);
  sharp_view=AcquireAuthenticCacheView(sharp_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,sharp_image,sharp_image->rows,1)
#endif
  for (y=0; y < (ssize_t) sharp_image->rows; y++)
  {
    register const Quantum
      *magick_restrict r;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    r=GetCacheViewVirtualPixels(edge_view,0,y,edge_image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(sharp_view,0,y,sharp_image->columns,1,
      exception);
    if ((r == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) sharp_image->columns; x++)
    {
      register const Quantum
        *magick_restrict p;

      register ssize_t
        i;

      ssize_t
        center,
        j;

      j=(ssize_t) ceil((double) width*(1.0-QuantumScale*
        GetPixelIntensity(edge_image,r))-0.5);
      if (j < 0)
        j=0;
      else
        if (j > (ssize_t) width)
          j=(ssize_t) width;
      if ((j & 0x01) != 0)
        j--;
      p=GetCacheViewVirtualPixels(image_view,x-((ssize_t) (width-j)/2L),y-
        (ssize_t) ((width-j)/2L),width-j,width-j,exception);
      if (p == (const Quantum *) NULL)
        break;
      center=(ssize_t) GetPixelChannels(image)*(width-j)*((width-j)/2L)+
        GetPixelChannels(image)*((width-j)/2);
      for (i=0; i < (ssize_t) GetPixelChannels(sharp_image); i++)
      {
        double
          alpha,
          gamma,
          pixel;

        PixelChannel
          channel;

        PixelTrait
          sharp_traits,
          traits;

        register const double
          *magick_restrict k;

        register const Quantum
          *magick_restrict pixels;

        register ssize_t
          u;

        ssize_t
          v;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        sharp_traits=GetPixelChannelTraits(sharp_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (sharp_traits == UndefinedPixelTrait))
          continue;
        if (((sharp_traits & CopyPixelTrait) != 0) ||
            (GetPixelWriteMask(image,p+center) == 0))
          {
            SetPixelChannel(sharp_image,channel,p[center+i],q);
            continue;
          }
        k=kernel[j];
        pixels=p;
        pixel=0.0;
        gamma=0.0;
        if ((sharp_traits & BlendPixelTrait) == 0)
          {
            /*
              No alpha blending.
            */
            for (v=0; v < (ssize_t) (width-j); v++)
            {
              for (u=0; u < (ssize_t) (width-j); u++)
              {
                pixel+=(*k)*pixels[i];
                gamma+=(*k);
                k++;
                pixels+=GetPixelChannels(image);
              }
            }
            gamma=PerceptibleReciprocal(gamma);
            SetPixelChannel(sharp_image,channel,ClampToQuantum(gamma*pixel),q);
            continue;
          }
        /*
          Alpha blending.
        */
        for (v=0; v < (ssize_t) (width-j); v++)
        {
          for (u=0; u < (ssize_t) (width-j); u++)
          {
            alpha=(double) (QuantumScale*GetPixelAlpha(image,pixels));
            pixel+=(*k)*alpha*pixels[i];
            gamma+=(*k)*alpha;
            k++;
            pixels+=GetPixelChannels(image);
          }
        }
        gamma=PerceptibleReciprocal(gamma);
        SetPixelChannel(sharp_image,channel,ClampToQuantum(gamma*pixel),q);
      }
      q+=GetPixelChannels(sharp_image);
      r+=GetPixelChannels(edge_image);
    }
    if (SyncCacheViewAuthenticPixels(sharp_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_AdaptiveSharpenImage)
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
  for (i=0; i < (ssize_t) width; i+=2)
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
%  The format of the BlurImage method is:
%
%      Image *BlurImage(const Image *image,const double radius,
%        const double sigma,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
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
  char
    geometry[MagickPathExtent];

  KernelInfo
    *kernel_info;

  Image
    *blur_image;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  blur_image=AccelerateBlurImage(image,radius,sigma,exception);
  if (blur_image != (Image *) NULL)
    return(blur_image);
#endif
  (void) FormatLocaleString(geometry,MagickPathExtent,
    "blur:%.20gx%.20g;blur:%.20gx%.20g+90",radius,sigma,radius,sigma);
  kernel_info=AcquireKernelInfo(geometry,exception);
  if (kernel_info == (KernelInfo *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  blur_image=ConvolveImage(image,kernel_info,exception);
  kernel_info=DestroyKernelInfo(kernel_info);
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
%      Image *ConvolveImage(const Image *image,const KernelInfo *kernel,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o kernel: the filtering kernel.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ConvolveImage(const Image *image,
  const KernelInfo *kernel_info,ExceptionInfo *exception)
{
  Image
    *convolve_image;

#if defined(MAGICKCORE_OPENCL_SUPPORT)
  convolve_image=AccelerateConvolveImage(image,kernel_info,exception);
  if (convolve_image != (Image *) NULL)
    return(convolve_image);
#endif

  convolve_image=MorphologyImage(image,ConvolveMorphology,1,kernel_info,
    exception);
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
%  edges of the original image.  A speckle removing filter uses a complementary
%  hulling technique (raising pixels that are darker than their surrounding
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
  const int polarity,Quantum *magick_restrict f,Quantum *magick_restrict g)
{
  register Quantum
    *p,
    *q,
    *r,
    *s;

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(f != (Quantum *) NULL);
  assert(g != (Quantum *) NULL);
  p=f+(columns+2);
  q=g+(columns+2);
  r=p+(y_offset*(columns+2)+x_offset);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) \
    magick_threads(image,image,1,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    MagickRealType
      v;

    register ssize_t
      i,
      x;

    i=(2*y+1)+y*columns;
    if (polarity > 0)
      for (x=0; x < (ssize_t) columns; x++)
      {
        v=(MagickRealType) p[i];
        if ((MagickRealType) r[i] >= (v+ScaleCharToQuantum(2)))
          v+=ScaleCharToQuantum(1);
        q[i]=(Quantum) v;
        i++;
      }
    else
      for (x=0; x < (ssize_t) columns; x++)
      {
        v=(MagickRealType) p[i];
        if ((MagickRealType) r[i] <= (v-ScaleCharToQuantum(2)))
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
  #pragma omp parallel for schedule(static,4) \
    magick_threads(image,image,1,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    register ssize_t
      i,
      x;

    MagickRealType
      v;

    i=(2*y+1)+y*columns;
    if (polarity > 0)
      for (x=0; x < (ssize_t) columns; x++)
      {
        v=(MagickRealType) q[i];
        if (((MagickRealType) s[i] >= (v+ScaleCharToQuantum(2))) &&
            ((MagickRealType) r[i] > v))
          v+=ScaleCharToQuantum(1);
        p[i]=(Quantum) v;
        i++;
      }
    else
      for (x=0; x < (ssize_t) columns; x++)
      {
        v=(MagickRealType) q[i];
        if (((MagickRealType) s[i] <= (v-ScaleCharToQuantum(2))) &&
            ((MagickRealType) r[i] < v))
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

  MemoryInfo
    *buffer_info,
    *pixel_info;

  Quantum
    *magick_restrict buffer,
    *magick_restrict pixels;

  register ssize_t
    i;

  size_t
    length;

  static const ssize_t
    X[4] = {0, 1, 1,-1},
    Y[4] = {1, 0, 1, 1};

  /*
    Allocate despeckled image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  despeckle_image=AccelerateDespeckleImage(image,exception);
  if (despeckle_image != (Image *) NULL)
    return(despeckle_image);
#endif
  despeckle_image=CloneImage(image,0,0,MagickTrue,exception);
  if (despeckle_image == (Image *) NULL)
    return((Image *) NULL);
  status=SetImageStorageClass(despeckle_image,DirectClass,exception);
  if (status == MagickFalse)
    {
      despeckle_image=DestroyImage(despeckle_image);
      return((Image *) NULL);
    }
  /*
    Allocate image buffer.
  */
  length=(size_t) ((image->columns+2)*(image->rows+2));
  pixel_info=AcquireVirtualMemory(length,sizeof(*pixels));
  buffer_info=AcquireVirtualMemory(length,sizeof(*buffer));
  if ((pixel_info == (MemoryInfo *) NULL) ||
      (buffer_info == (MemoryInfo *) NULL))
    {
      if (buffer_info != (MemoryInfo *) NULL)
        buffer_info=RelinquishVirtualMemory(buffer_info);
      if (pixel_info != (MemoryInfo *) NULL)
        pixel_info=RelinquishVirtualMemory(pixel_info);
      despeckle_image=DestroyImage(despeckle_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  pixels=(Quantum *) GetVirtualMemoryBlob(pixel_info);
  buffer=(Quantum *) GetVirtualMemoryBlob(buffer_info);
  /*
    Reduce speckle in the image.
  */
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
  despeckle_view=AcquireAuthenticCacheView(despeckle_image,exception);
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel
       channel;

    PixelTrait
      despeckle_traits,
      traits;

    register ssize_t
      k,
      x;

    ssize_t
      j,
      y;

    if (status == MagickFalse)
      continue;
    channel=GetPixelChannelChannel(image,i);
    traits=GetPixelChannelTraits(image,channel);
    despeckle_traits=GetPixelChannelTraits(despeckle_image,channel);
    if ((traits == UndefinedPixelTrait) ||
        (despeckle_traits == UndefinedPixelTrait))
      continue;
    if ((despeckle_traits & CopyPixelTrait) != 0)
      continue;
    (void) ResetMagickMemory(pixels,0,length*sizeof(*pixels));
    j=(ssize_t) image->columns+2;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      register const Quantum
        *magick_restrict p;

      p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      j++;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        pixels[j++]=p[i];
        p+=GetPixelChannels(image);
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

      register Quantum
        *magick_restrict q;

      q=GetCacheViewAuthenticPixels(despeckle_view,0,y,despeckle_image->columns,
        1,exception);
      if (q == (Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      j++;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        SetPixelChannel(despeckle_image,channel,pixels[j++],q);
        q+=GetPixelChannels(despeckle_image);
      }
      sync=SyncCacheViewAuthenticPixels(despeckle_view,exception);
      if (sync == MagickFalse)
        status=MagickFalse;
      j++;
    }
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,DespeckleImageTag,(MagickOffsetType) i,
          GetPixelChannels(image));
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  despeckle_view=DestroyCacheView(despeckle_view);
  image_view=DestroyCacheView(image_view);
  buffer_info=RelinquishVirtualMemory(buffer_info);
  pixel_info=RelinquishVirtualMemory(pixel_info);
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

  KernelInfo
    *kernel_info;

  register ssize_t
    i;

  size_t
    width;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  width=GetOptimalKernelWidth1D(radius,0.5);
  kernel_info=AcquireKernelInfo((const char *) NULL,exception);
  if (kernel_info == (KernelInfo *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(kernel_info,0,sizeof(*kernel_info));
  kernel_info->width=width;
  kernel_info->height=width;
  kernel_info->x=(ssize_t) (kernel_info->width-1)/2;
  kernel_info->y=(ssize_t) (kernel_info->height-1)/2;
  kernel_info->signature=MagickCoreSignature;
  kernel_info->values=(MagickRealType *) MagickAssumeAligned(
    AcquireAlignedMemory(kernel_info->width,kernel_info->height*
    sizeof(*kernel_info->values)));
  if (kernel_info->values == (MagickRealType *) NULL)
    {
      kernel_info=DestroyKernelInfo(kernel_info);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  for (i=0; i < (ssize_t) (kernel_info->width*kernel_info->height); i++)
    kernel_info->values[i]=(-1.0);
  kernel_info->values[i/2]=(double) kernel_info->width*kernel_info->height-1.0;
  edge_image=ConvolveImage(image,kernel_info,exception);
  kernel_info=DestroyKernelInfo(kernel_info);
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
    gamma,
    normalize;

  Image
    *emboss_image;

  KernelInfo
    *kernel_info;

  register ssize_t
    i;

  size_t
    width;

  ssize_t
    j,
    k,
    u,
    v;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  width=GetOptimalKernelWidth1D(radius,sigma);
  kernel_info=AcquireKernelInfo((const char *) NULL,exception);
  if (kernel_info == (KernelInfo *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  kernel_info->width=width;
  kernel_info->height=width;
  kernel_info->x=(ssize_t) (width-1)/2;
  kernel_info->y=(ssize_t) (width-1)/2;
  kernel_info->values=(MagickRealType *) MagickAssumeAligned(
    AcquireAlignedMemory(kernel_info->width,kernel_info->width*
    sizeof(*kernel_info->values)));
  if (kernel_info->values == (MagickRealType *) NULL)
    {
      kernel_info=DestroyKernelInfo(kernel_info);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  j=(ssize_t) (kernel_info->width-1)/2;
  k=j;
  i=0;
  for (v=(-j); v <= j; v++)
  {
    for (u=(-j); u <= j; u++)
    {
      kernel_info->values[i]=(MagickRealType) (((u < 0) || (v < 0) ? -8.0 :
        8.0)*exp(-((double) u*u+v*v)/(2.0*MagickSigma*MagickSigma))/
        (2.0*MagickPI*MagickSigma*MagickSigma));
      if (u != k)
        kernel_info->values[i]=0.0;
      i++;
    }
    k--;
  }
  normalize=0.0;
  for (i=0; i < (ssize_t) (kernel_info->width*kernel_info->height); i++)
    normalize+=kernel_info->values[i];
  gamma=PerceptibleReciprocal(normalize);
  for (i=0; i < (ssize_t) (kernel_info->width*kernel_info->height); i++)
    kernel_info->values[i]*=gamma;
  emboss_image=ConvolveImage(image,kernel_info,exception);
  kernel_info=DestroyKernelInfo(kernel_info);
  if (emboss_image != (Image *) NULL)
    (void) EqualizeImage(emboss_image,exception);
  return(emboss_image);
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
%
%  A description of each parameter follows:
%
%    o image: the image.
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
  char
    geometry[MagickPathExtent];

  KernelInfo
    *kernel_info;

  Image
    *blur_image;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  (void) FormatLocaleString(geometry,MagickPathExtent,"gaussian:%.20gx%.20g",
    radius,sigma);
  kernel_info=AcquireKernelInfo(geometry,exception);
  if (kernel_info == (KernelInfo *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  blur_image=ConvolveImage(image,kernel_info,exception);
  kernel_info=DestroyKernelInfo(kernel_info);
  return(blur_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     K u w a h a r a I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  KuwaharaImage() is an edge preserving noise reduction filter.
%
%  The format of the KuwaharaImage method is:
%
%      Image *KuwaharaImage(const Image *image,const double radius,
%        const double sigma,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the square window radius.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline MagickRealType GetMeanLuma(const Image *magick_restrict image,
  const double *magick_restrict pixel)
{
  return(0.212656f*pixel[image->channel_map[RedPixelChannel].offset]+
    0.715158f*pixel[image->channel_map[GreenPixelChannel].offset]+
    0.072186f*pixel[image->channel_map[BluePixelChannel].offset]);  /* Rec709 */
}

MagickExport Image *KuwaharaImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
{
#define KuwaharaImageTag  "Kuwahara/Image"

  CacheView
    *image_view,
    *kuwahara_view;

  Image
    *gaussian_image,
    *kuwahara_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  size_t
    width;

  ssize_t
    y;

  /*
    Initialize Kuwahara image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  width=(size_t) radius+1;
  gaussian_image=BlurImage(image,radius,sigma,exception);
  if (gaussian_image == (Image *) NULL)
    return((Image *) NULL);
  kuwahara_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (kuwahara_image == (Image *) NULL)
    {
      gaussian_image=DestroyImage(gaussian_image);
      return((Image *) NULL);
    }
  if (SetImageStorageClass(kuwahara_image,DirectClass,exception) == MagickFalse)
    {
      gaussian_image=DestroyImage(gaussian_image);
      kuwahara_image=DestroyImage(kuwahara_image);
      return((Image *) NULL);
    }
  /*
    Edge preserving noise reduction filter.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(gaussian_image,exception);
  kuwahara_view=AcquireAuthenticCacheView(kuwahara_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,kuwahara_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) gaussian_image->rows; y++)
  {
    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(kuwahara_view,0,y,kuwahara_image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) gaussian_image->columns; x++)
    {
      const Quantum
        *magick_restrict p;

      double
        min_variance;

      RectangleInfo
        quadrant,
        target;

      register size_t
        i;

      min_variance=MagickMaximumValue;
      SetGeometry(gaussian_image,&target);
      quadrant.width=width;
      quadrant.height=width;
      for (i=0; i < 4; i++)
      {
        const Quantum
          *magick_restrict k;

        double
          mean[MaxPixelChannels],
          variance;

        register ssize_t
          n;

        ssize_t
          j;

        quadrant.x=x;
        quadrant.y=y;
        switch (i)
        {
          case 0:
          {
            quadrant.x=x-(ssize_t) (width-1);
            quadrant.y=y-(ssize_t) (width-1);
            break;
          }
          case 1:
          {
            quadrant.y=y-(ssize_t) (width-1);
            break;
          }
          case 2:
          {
            quadrant.x=x-(ssize_t) (width-1);
            break;
          }
          case 3:
          default:
            break;
        }
        p=GetCacheViewVirtualPixels(image_view,quadrant.x,quadrant.y,
          quadrant.width,quadrant.height,exception);
        if (p == (const Quantum *) NULL)
          break;
        for (j=0; j < (ssize_t) GetPixelChannels(gaussian_image); j++)
          mean[j]=0.0;
        k=p;
        for (n=0; n < (ssize_t) (width*width); n++)
        {
          for (j=0; j < (ssize_t) GetPixelChannels(gaussian_image); j++)
            mean[j]+=(double) k[j];
          k+=GetPixelChannels(gaussian_image);
        }
        for (j=0; j < (ssize_t) GetPixelChannels(gaussian_image); j++)
          mean[j]/=(double) (width*width);
        k=p;
        variance=0.0;
        for (n=0; n < (ssize_t) (width*width); n++)
        {
          double
            luma;

          luma=GetPixelLuma(gaussian_image,k);
          variance+=(luma-GetMeanLuma(gaussian_image,mean))*
            (luma-GetMeanLuma(gaussian_image,mean));
          k+=GetPixelChannels(gaussian_image);
        }
        if (variance < min_variance)
          {
            min_variance=variance;
            target=quadrant;
          }
      }
      if (i < 4)
        {
          status=MagickFalse;
          break;
        }
      status=InterpolatePixelChannels(gaussian_image,image_view,kuwahara_image,
        UndefinedInterpolatePixel,(double) target.x+target.width/2.0,(double)
        target.y+target.height/2.0,q,exception);
      q+=GetPixelChannels(kuwahara_image);
    }
    if (SyncCacheViewAuthenticPixels(kuwahara_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_KuwaharaImage)
#endif
        proceed=SetImageProgress(image,KuwaharaImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  kuwahara_view=DestroyCacheView(kuwahara_view);
  image_view=DestroyCacheView(image_view);
  gaussian_image=DestroyImage(gaussian_image);
  if (status == MagickFalse)
    kuwahara_image=DestroyImage(kuwahara_image);
  return(kuwahara_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     L o c a l C o n t r a s t I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocalContrastImage() attempts to increase the appearance of large-scale
%  light-dark transitions. Local contrast enhancement works similarly to
%  sharpening with an unsharp mask, however the mask is instead created using
%  an image with a greater blur distance.
%
%  The format of the LocalContrastImage method is:
%
%      Image *LocalContrastImage(const Image *image, const double radius,
%        const double strength,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the Gaussian blur, in percentage with 100%
%      resulting in a blur radius of 20% of largest dimension.
%
%    o strength: the strength of the blur mask in percentage.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *LocalContrastImage(const Image *image,const double radius,
  const double strength,ExceptionInfo *exception)
{
#define LocalContrastImageTag  "LocalContrast/Image"

  CacheView
    *image_view,
    *contrast_view;

  float
    *interImage,
    *scanLinePixels,
    totalWeight;

  Image
    *contrast_image;

  MagickBooleanType
    status;

  MemoryInfo
    *scanLinePixels_info,
    *interImage_info;

  ssize_t
    scanLineSize,
    width;

  /*
    Initialize contrast image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  contrast_image=AccelerateLocalContrastImage(image,radius,strength,exception);
  if (contrast_image != (Image *) NULL)
    return(contrast_image);
#endif
  contrast_image=CloneImage(image,0,0,MagickTrue,exception);
  if (contrast_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(contrast_image,DirectClass,exception) == MagickFalse)
    {
      contrast_image=DestroyImage(contrast_image);
      return((Image *) NULL);
    }
  image_view=AcquireVirtualCacheView(image,exception);
  contrast_view=AcquireAuthenticCacheView(contrast_image,exception);
  scanLineSize=(ssize_t) MagickMax(image->columns,image->rows);
  width=(ssize_t) scanLineSize*0.002f*fabs(radius);
  scanLineSize+=(2*width);
  scanLinePixels_info=AcquireVirtualMemory((size_t) GetOpenMPMaximumThreads()*
    scanLineSize,sizeof(*scanLinePixels));
  if (scanLinePixels_info == (MemoryInfo *) NULL)
    {
      contrast_view=DestroyCacheView(contrast_view);
      image_view=DestroyCacheView(image_view);
      contrast_image=DestroyImage(contrast_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  scanLinePixels=(float *) GetVirtualMemoryBlob(scanLinePixels_info);
  /*
    Create intermediate buffer.
  */
  interImage_info=AcquireVirtualMemory(image->rows*(image->columns+(2*width)),
    sizeof(*interImage));
  if (interImage_info == (MemoryInfo *) NULL)
    {
      scanLinePixels_info=RelinquishVirtualMemory(scanLinePixels_info);
      contrast_view=DestroyCacheView(contrast_view);
      image_view=DestroyCacheView(image_view);
      contrast_image=DestroyImage(contrast_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  interImage=(float *) GetVirtualMemoryBlob(interImage_info);
  totalWeight=(float) ((width+1)*(width+1));
  /*
    Vertical pass.
  */
  status=MagickTrue;
  {
    ssize_t
      x;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
#pragma omp parallel for schedule(static,4) \
    magick_threads(image,image,image->columns,1)
#endif
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      const int
        id = GetOpenMPThreadId();

      const Quantum
        *magick_restrict p;

      float
        *out,
        *pix,
        *pixels;

      register ssize_t
        y;

      ssize_t
        i;

      if (status == MagickFalse)
        continue;
      pixels=scanLinePixels;
      pixels+=id*scanLineSize;
      pix=pixels;
      p=GetCacheViewVirtualPixels(image_view,x,-width,1,image->rows+(2*width),
        exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      for (y=0; y < (ssize_t) image->rows+(2*width); y++)
      {
        *pix++=(float)GetPixelLuma(image,p);
        p+=image->number_channels;
      }
      out=interImage+x+width;
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        float
          sum,
          weight;

        weight=1.0f;
        sum=0;
        pix=pixels+y;
        for (i=0; i < width; i++)
        {
          sum+=weight*(*pix++);
          weight+=1.0f;
        }
        for (i=width+1; i < (2*width); i++)
        {
          sum+=weight*(*pix++);
          weight-=1.0f;
        }
        /* write to output */
        *out=sum/totalWeight;
        /* mirror into padding */
        if (x <= width && x != 0)
          *(out-(x*2))=*out;
        if ((x > (ssize_t) image->columns-width-2) &&
            (x != (ssize_t) image->columns-1))
          *(out+((image->columns-x-1)*2))=*out;
        out+=image->columns+(width*2);
      }
    }
  }
  /*
    Horizontal pass.
  */
  {
    ssize_t
      y;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
#pragma omp parallel for schedule(static,4) \
    magick_threads(image,image,image->rows,1)
#endif
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      const int
        id = GetOpenMPThreadId();

      const Quantum
        *magick_restrict p;

      float
        *pix,
        *pixels;

      register Quantum
        *magick_restrict q;

      register ssize_t
        x;

      ssize_t
        i;

      if (status == MagickFalse)
        continue;
      pixels=scanLinePixels;
      pixels+=id*scanLineSize;
      p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
      q=GetCacheViewAuthenticPixels(contrast_view,0,y,image->columns,1,
        exception);
      if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
        {
          status=MagickFalse;
          continue;
        }
      memcpy(pixels,interImage+(y*(image->columns+(2*width))),(image->columns+
        (2*width))*sizeof(float));
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        float
          mult,
          srcVal,
          sum,
          weight;

        weight=1.0f;
        sum=0;
        pix=pixels+x;
        for (i=0; i < width; i++)
        {
          sum+=weight*(*pix++);
          weight+=1.0f;
        }
        for (i=width+1; i < (2*width); i++)
        {
          sum+=weight*(*pix++);
          weight-=1.0f;
        }
        /* Apply and write */
        srcVal=(float) GetPixelLuma(image,p);
        mult=(srcVal-(sum/totalWeight))*(strength/100.0f);
        mult=(srcVal+mult)/srcVal;
        SetPixelRed(contrast_image,ClampToQuantum(GetPixelRed(image,p)*mult),
          q);
        SetPixelGreen(contrast_image,ClampToQuantum(GetPixelGreen(image,p)*
          mult),q);
        SetPixelBlue(contrast_image,ClampToQuantum(GetPixelBlue(image,p)*mult),
          q);
        p+=image->number_channels;
        q+=contrast_image->number_channels;
      }
      if (SyncCacheViewAuthenticPixels(contrast_view,exception) == MagickFalse)
        status=MagickFalse;
    }
  }
  scanLinePixels_info=RelinquishVirtualMemory(scanLinePixels_info);
  interImage_info=RelinquishVirtualMemory(interImage_info);
  contrast_view=DestroyCacheView(contrast_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    contrast_image=DestroyImage(contrast_image);
  return(contrast_image);
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
%
%  A description of each parameter follows:
%
%    o image: the image.
%
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

static MagickRealType *GetMotionBlurKernel(const size_t width,
  const double sigma)
{
  MagickRealType
    *kernel,
    normalize;

  register ssize_t
    i;

  /*
   Generate a 1-D convolution kernel.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  kernel=(MagickRealType *) MagickAssumeAligned(AcquireAlignedMemory((size_t)
    width,sizeof(*kernel)));
  if (kernel == (MagickRealType *) NULL)
    return(kernel);
  normalize=0.0;
  for (i=0; i < (ssize_t) width; i++)
  {
    kernel[i]=(MagickRealType) (exp((-((double) i*i)/(double) (2.0*MagickSigma*
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
#define BlurImageTag  "Blur/Image"

  CacheView
    *blur_view,
    *image_view,
    *motion_view;

  Image
    *blur_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickRealType
    *kernel;

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
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  width=GetOptimalKernelWidth1D(radius,sigma);
  kernel=GetMotionBlurKernel(width,sigma);
  if (kernel == (MagickRealType *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  offset=(OffsetInfo *) AcquireQuantumMemory(width,sizeof(*offset));
  if (offset == (OffsetInfo *) NULL)
    {
      kernel=(MagickRealType *) RelinquishAlignedMemory(kernel);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  blur_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (blur_image == (Image *) NULL)
    {
      kernel=(MagickRealType *) RelinquishAlignedMemory(kernel);
      offset=(OffsetInfo *) RelinquishMagickMemory(offset);
      return((Image *) NULL);
    }
  if (SetImageStorageClass(blur_image,DirectClass,exception) == MagickFalse)
    {
      kernel=(MagickRealType *) RelinquishAlignedMemory(kernel);
      offset=(OffsetInfo *) RelinquishMagickMemory(offset);
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
  image_view=AcquireVirtualCacheView(image,exception);
  motion_view=AcquireVirtualCacheView(image,exception);
  blur_view=AcquireAuthenticCacheView(blur_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,blur_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
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
        double
          alpha,
          gamma,
          pixel;

        PixelChannel
          channel;

        PixelTrait
          blur_traits,
          traits;

        register const Quantum
          *magick_restrict r;

        register MagickRealType
          *magick_restrict k;

        register ssize_t
          j;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        blur_traits=GetPixelChannelTraits(blur_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (blur_traits == UndefinedPixelTrait))
          continue;
        if (((blur_traits & CopyPixelTrait) != 0) ||
            (GetPixelWriteMask(image,p) == 0))
          {
            SetPixelChannel(blur_image,channel,p[i],q);
            continue;
          }
        k=kernel;
        pixel=0.0;
        if ((blur_traits & BlendPixelTrait) == 0)
          {
            for (j=0; j < (ssize_t) width; j++)
            {
              r=GetCacheViewVirtualPixels(motion_view,x+offset[j].x,y+
                offset[j].y,1,1,exception);
              if (r == (const Quantum *) NULL)
                {
                  status=MagickFalse;
                  continue;
                }
              pixel+=(*k)*r[i];
              k++;
            }
            SetPixelChannel(blur_image,channel,ClampToQuantum(pixel),q);
            continue;
          }
        alpha=0.0;
        gamma=0.0;
        for (j=0; j < (ssize_t) width; j++)
        {
          r=GetCacheViewVirtualPixels(motion_view,x+offset[j].x,y+offset[j].y,1,
            1,exception);
          if (r == (const Quantum *) NULL)
            {
              status=MagickFalse;
              continue;
            }
          alpha=(double) (QuantumScale*GetPixelAlpha(image,r));
          pixel+=(*k)*alpha*r[i];
          gamma+=(*k)*alpha;
          k++;
        }
        gamma=PerceptibleReciprocal(gamma);
        SetPixelChannel(blur_image,channel,ClampToQuantum(gamma*pixel),q);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(blur_image);
    }
    if (SyncCacheViewAuthenticPixels(blur_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_MotionBlurImage)
#endif
        proceed=SetImageProgress(image,BlurImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  blur_view=DestroyCacheView(blur_view);
  motion_view=DestroyCacheView(motion_view);
  image_view=DestroyCacheView(image_view);
  kernel=(MagickRealType *) RelinquishAlignedMemory(kernel);
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
    factor[MagickPathExtent],
    label[MagickPathExtent];

  double
    degrees,
    gamma,
    percentage,
    radius,
    sigma,
    threshold;

  extern const char
    DefaultTileFrame[];

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
  assert(image->signature == MagickCoreSignature);
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
    (void) SetImageProperty(thumbnail,"label",DefaultTileLabel,exception);
    if (i == (NumberTiles/2))
      {
        (void) QueryColorCompliance("#dfdfdf",AllCompliance,
          &thumbnail->matte_color,exception);
        AppendImageToList(&images,thumbnail);
        continue;
      }
    switch (preview)
    {
      case RotatePreview:
      {
        degrees+=45.0;
        preview_image=RotateImage(thumbnail,degrees,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"rotate %g",degrees);
        break;
      }
      case ShearPreview:
      {
        degrees+=5.0;
        preview_image=ShearImage(thumbnail,degrees,degrees,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"shear %gx%g",degrees,
          2.0*degrees);
        break;
      }
      case RollPreview:
      {
        x=(ssize_t) ((i+1)*thumbnail->columns)/NumberTiles;
        y=(ssize_t) ((i+1)*thumbnail->rows)/NumberTiles;
        preview_image=RollImage(thumbnail,x,y,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"roll %+.20gx%+.20g",
          (double) x,(double) y);
        break;
      }
      case HuePreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) FormatLocaleString(factor,MagickPathExtent,"100,100,%g",2.0*
          percentage);
        (void) ModulateImage(preview_image,factor,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"modulate %s",factor);
        break;
      }
      case SaturationPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) FormatLocaleString(factor,MagickPathExtent,"100,%g",2.0*percentage);
        (void) ModulateImage(preview_image,factor,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"modulate %s",factor);
        break;
      }
      case BrightnessPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) FormatLocaleString(factor,MagickPathExtent,"%g",2.0*percentage);
        (void) ModulateImage(preview_image,factor,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"modulate %s",factor);
        break;
      }
      case GammaPreview:
      default:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        gamma+=0.4f;
        (void) GammaImage(preview_image,gamma,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"gamma %g",gamma);
        break;
      }
      case SpiffPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image != (Image *) NULL)
          for (x=0; x < i; x++)
            (void) ContrastImage(preview_image,MagickTrue,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"contrast (%.20g)",
          (double) i+1);
        break;
      }
      case DullPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        for (x=0; x < i; x++)
          (void) ContrastImage(preview_image,MagickFalse,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"+contrast (%.20g)",
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
        (void) QuantizeImage(&quantize_info,preview_image,exception);
        (void) FormatLocaleString(label,MagickPathExtent,
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
        (void) QuantizeImage(&quantize_info,preview_image,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"colors %.20g",(double)
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
        (void) FormatLocaleString(label,MagickPathExtent,"despeckle (%.20g)",
          (double) i+1);
        break;
      }
      case ReduceNoisePreview:
      {
        preview_image=StatisticImage(thumbnail,NonpeakStatistic,(size_t) radius,
          (size_t) radius,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"noise %g",radius);
        break;
      }
      case AddNoisePreview:
      {
        switch ((int) i)
        {
          case 0:
          {
            (void) CopyMagickString(factor,"uniform",MagickPathExtent);
            break;
          }
          case 1:
          {
            (void) CopyMagickString(factor,"gaussian",MagickPathExtent);
            break;
          }
          case 2:
          {
            (void) CopyMagickString(factor,"multiplicative",MagickPathExtent);
            break;
          }
          case 3:
          {
            (void) CopyMagickString(factor,"impulse",MagickPathExtent);
            break;
          }
          case 5:
          {
            (void) CopyMagickString(factor,"laplacian",MagickPathExtent);
            break;
          }
          case 6:
          {
            (void) CopyMagickString(factor,"Poisson",MagickPathExtent);
            break;
          }
          default:
          {
            (void) CopyMagickString(thumbnail->magick,"NULL",MagickPathExtent);
            break;
          }
        }
        preview_image=StatisticImage(thumbnail,NonpeakStatistic,(size_t) i,
          (size_t) i,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"+noise %s",factor);
        break;
      }
      case SharpenPreview:
      {
        preview_image=SharpenImage(thumbnail,radius,sigma,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"sharpen %gx%g",radius,
          sigma);
        break;
      }
      case BlurPreview:
      {
        preview_image=BlurImage(thumbnail,radius,sigma,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"blur %gx%g",radius,
          sigma);
        break;
      }
      case ThresholdPreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) BilevelImage(thumbnail,(double) (percentage*((double)
          QuantumRange+1.0))/100.0,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"threshold %g",(double)
          (percentage*((double) QuantumRange+1.0))/100.0);
        break;
      }
      case EdgeDetectPreview:
      {
        preview_image=EdgeImage(thumbnail,radius,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"edge %g",radius);
        break;
      }
      case SpreadPreview:
      {
        preview_image=SpreadImage(thumbnail,image->interpolate,radius,
          exception);
        (void) FormatLocaleString(label,MagickPathExtent,"spread %g",
          radius+0.5);
        break;
      }
      case SolarizePreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        (void) SolarizeImage(preview_image,(double) QuantumRange*percentage/
          100.0,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"solarize %g",
          (QuantumRange*percentage)/100.0);
        break;
      }
      case ShadePreview:
      {
        degrees+=10.0;
        preview_image=ShadeImage(thumbnail,MagickTrue,degrees,degrees,
          exception);
        (void) FormatLocaleString(label,MagickPathExtent,"shade %gx%g",degrees,
          degrees);
        break;
      }
      case RaisePreview:
      {
        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        geometry.width=(size_t) (2*i+2);
        geometry.height=(size_t) (2*i+2);
        geometry.x=(i-1)/2;
        geometry.y=(i-1)/2;
        (void) RaiseImage(preview_image,&geometry,MagickTrue,exception);
        (void) FormatLocaleString(label,MagickPathExtent,
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
          threshold,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"segment %gx%g",
          threshold,threshold);
        break;
      }
      case SwirlPreview:
      {
        preview_image=SwirlImage(thumbnail,degrees,image->interpolate,
          exception);
        (void) FormatLocaleString(label,MagickPathExtent,"swirl %g",degrees);
        degrees+=45.0;
        break;
      }
      case ImplodePreview:
      {
        degrees+=0.1f;
        preview_image=ImplodeImage(thumbnail,degrees,image->interpolate,
          exception);
        (void) FormatLocaleString(label,MagickPathExtent,"implode %g",degrees);
        break;
      }
      case WavePreview:
      {
        degrees+=5.0f;
        preview_image=WaveImage(thumbnail,0.5*degrees,2.0*degrees,
          image->interpolate,exception);
        (void) FormatLocaleString(label,MagickPathExtent,"wave %gx%g",0.5*degrees,
          2.0*degrees);
        break;
      }
      case OilPaintPreview:
      {
        preview_image=OilPaintImage(thumbnail,(double) radius,(double) sigma,
          exception);
        (void) FormatLocaleString(label,MagickPathExtent,"charcoal %gx%g",radius,
          sigma);
        break;
      }
      case CharcoalDrawingPreview:
      {
        preview_image=CharcoalImage(thumbnail,(double) radius,(double) sigma,
          exception);
        (void) FormatLocaleString(label,MagickPathExtent,"charcoal %gx%g",radius,
          sigma);
        break;
      }
      case JPEGPreview:
      {
        char
          filename[MagickPathExtent];

        int
          file;

        MagickBooleanType
          status;

        preview_image=CloneImage(thumbnail,0,0,MagickTrue,exception);
        if (preview_image == (Image *) NULL)
          break;
        preview_info->quality=(size_t) percentage;
        (void) FormatLocaleString(factor,MagickPathExtent,"%.20g",(double)
          preview_info->quality);
        file=AcquireUniqueFileResource(filename);
        if (file != -1)
          file=close(file)-1;
        (void) FormatLocaleString(preview_image->filename,MagickPathExtent,
          "jpeg:%s",filename);
        status=WriteImage(preview_info,preview_image,exception);
        if (status != MagickFalse)
          {
            Image
              *quality_image;

            (void) CopyMagickString(preview_info->filename,
              preview_image->filename,MagickPathExtent);
            quality_image=ReadImage(preview_info,exception);
            if (quality_image != (Image *) NULL)
              {
                preview_image=DestroyImage(preview_image);
                preview_image=quality_image;
              }
          }
        (void) RelinquishUniqueFileResource(preview_image->filename);
        if ((GetBlobSize(preview_image)/1024) >= 1024)
          (void) FormatLocaleString(label,MagickPathExtent,"quality %s\n%gmb ",
            factor,(double) ((MagickOffsetType) GetBlobSize(preview_image))/
            1024.0/1024.0);
        else
          if (GetBlobSize(preview_image) >= 1024)
            (void) FormatLocaleString(label,MagickPathExtent,
              "quality %s\n%gkb ",factor,(double) ((MagickOffsetType)
              GetBlobSize(preview_image))/1024.0);
          else
            (void) FormatLocaleString(label,MagickPathExtent,"quality %s\n%.20gb ",
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
    (void) SetImageProperty(preview_image,"label",label,exception);
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
  (void) CopyMagickString(montage_info->filename,image->filename,MagickPathExtent);
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
%     R o t a t i o n a l B l u r I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RotationalBlurImage() applies a radial blur to the image.
%
%  Andrew Protano contributed this effect.
%
%  The format of the RotationalBlurImage method is:
%
%    Image *RotationalBlurImage(const Image *image,const double angle,
%      ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o angle: the angle of the radial blur.
%
%    o blur: the blur.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *RotationalBlurImage(const Image *image,const double angle,
  ExceptionInfo *exception)
{
  CacheView
    *blur_view,
    *image_view,
    *radial_view;

  double
    blur_radius,
    *cos_theta,
    offset,
    *sin_theta,
    theta;

  Image
    *blur_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

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
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  blur_image=AccelerateRotationalBlurImage(image,angle,exception);
  if (blur_image != (Image *) NULL)
    return(blur_image);
#endif
  blur_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (blur_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(blur_image,DirectClass,exception) == MagickFalse)
    {
      blur_image=DestroyImage(blur_image);
      return((Image *) NULL);
    }
  blur_center.x=(double) (image->columns-1)/2.0;
  blur_center.y=(double) (image->rows-1)/2.0;
  blur_radius=hypot(blur_center.x,blur_center.y);
  n=(size_t) fabs(4.0*DegreesToRadians(angle)*sqrt((double) blur_radius)+2UL);
  theta=DegreesToRadians(angle)/(double) (n-1);
  cos_theta=(double *) AcquireQuantumMemory((size_t) n,
    sizeof(*cos_theta));
  sin_theta=(double *) AcquireQuantumMemory((size_t) n,
    sizeof(*sin_theta));
  if ((cos_theta == (double *) NULL) ||
      (sin_theta == (double *) NULL))
    {
      blur_image=DestroyImage(blur_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  offset=theta*(double) (n-1)/2.0;
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
  image_view=AcquireVirtualCacheView(image,exception);
  radial_view=AcquireVirtualCacheView(image,exception);
  blur_view=AcquireAuthenticCacheView(blur_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,blur_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        radius;

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
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          gamma,
          pixel;

        PixelChannel
          channel;

        PixelTrait
          blur_traits,
          traits;

        register const Quantum
          *magick_restrict r;

        register ssize_t
          j;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        blur_traits=GetPixelChannelTraits(blur_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (blur_traits == UndefinedPixelTrait))
          continue;
        if (((blur_traits & CopyPixelTrait) != 0) ||
            (GetPixelWriteMask(image,p) == 0))
          {
            SetPixelChannel(blur_image,channel,p[i],q);
            continue;
          }
        gamma=0.0;
        pixel=0.0;
        if ((GetPixelChannelTraits(image,AlphaPixelChannel) == UndefinedPixelTrait) ||
            (channel == AlphaPixelChannel))
          {
            for (j=0; j < (ssize_t) n; j+=(ssize_t) step)
            {
              r=GetCacheViewVirtualPixels(radial_view, (ssize_t) (blur_center.x+
                center.x*cos_theta[j]-center.y*sin_theta[j]+0.5),(ssize_t)
                (blur_center.y+center.x*sin_theta[j]+center.y*cos_theta[j]+0.5),
                1,1,exception);
              if (r == (const Quantum *) NULL)
                {
                  status=MagickFalse;
                  continue;
                }
              pixel+=r[i];
              gamma++;
            }
            gamma=PerceptibleReciprocal(gamma);
            SetPixelChannel(blur_image,channel,ClampToQuantum(gamma*pixel),q);
            continue;
          }
        for (j=0; j < (ssize_t) n; j+=(ssize_t) step)
        {
          double
            alpha;

          r=GetCacheViewVirtualPixels(radial_view, (ssize_t) (blur_center.x+
            center.x*cos_theta[j]-center.y*sin_theta[j]+0.5),(ssize_t)
            (blur_center.y+center.x*sin_theta[j]+center.y*cos_theta[j]+0.5),
            1,1,exception);
          if (r == (const Quantum *) NULL)
            {
              status=MagickFalse;
              continue;
            }
          alpha=(double) QuantumScale*GetPixelAlpha(image,r);
          pixel+=alpha*r[i];
          gamma+=alpha;
        }
        gamma=PerceptibleReciprocal(gamma);
        SetPixelChannel(blur_image,channel,ClampToQuantum(gamma*pixel),q);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(blur_image);
    }
    if (SyncCacheViewAuthenticPixels(blur_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_RotationalBlurImage)
#endif
        proceed=SetImageProgress(image,BlurImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  blur_view=DestroyCacheView(blur_view);
  radial_view=DestroyCacheView(radial_view);
  image_view=DestroyCacheView(image_view);
  cos_theta=(double *) RelinquishMagickMemory(cos_theta);
  sin_theta=(double *) RelinquishMagickMemory(sin_theta);
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
%
%  A description of each parameter follows:
%
%    o image: the image.
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
#define SelectiveBlurImageTag  "SelectiveBlur/Image"

  CacheView
    *blur_view,
    *image_view,
    *luminance_view;

  Image
    *blur_image,
    *luminance_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickRealType
    *kernel;

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
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  width=GetOptimalKernelWidth1D(radius,sigma);
  kernel=(MagickRealType *) MagickAssumeAligned(AcquireAlignedMemory((size_t)
    width,width*sizeof(*kernel)));
  if (kernel == (MagickRealType *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  j=(ssize_t) (width-1)/2;
  i=0;
  for (v=(-j); v <= j; v++)
  {
    for (u=(-j); u <= j; u++)
      kernel[i++]=(MagickRealType) (exp(-((double) u*u+v*v)/(2.0*MagickSigma*
        MagickSigma))/(2.0*MagickPI*MagickSigma*MagickSigma));
  }
  if (image->debug != MagickFalse)
    {
      char
        format[MagickPathExtent],
        *message;

      register const MagickRealType
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
        (void) FormatLocaleString(format,MagickPathExtent,"%.20g: ",(double) v);
        (void) ConcatenateString(&message,format);
        for (u=0; u < (ssize_t) width; u++)
        {
          (void) FormatLocaleString(format,MagickPathExtent,"%+f ",(double) *k++);
          (void) ConcatenateString(&message,format);
        }
        (void) LogMagickEvent(TransformEvent,GetMagickModule(),"%s",message);
      }
      message=DestroyString(message);
    }
  blur_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (blur_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(blur_image,DirectClass,exception) == MagickFalse)
    {
      blur_image=DestroyImage(blur_image);
      kernel=(MagickRealType *) RelinquishAlignedMemory(kernel);
      return((Image *) NULL);
    }
  luminance_image=CloneImage(image,0,0,MagickTrue,exception);
  if (luminance_image == (Image *) NULL)
    {
      blur_image=DestroyImage(blur_image);
      kernel=(MagickRealType *) RelinquishAlignedMemory(kernel);
      return((Image *) NULL);
    }
  status=TransformImageColorspace(luminance_image,GRAYColorspace,exception);
  if (status == MagickFalse)
    {
      luminance_image=DestroyImage(luminance_image);
      blur_image=DestroyImage(blur_image);
      kernel=(MagickRealType *) RelinquishAlignedMemory(kernel);
      return((Image *) NULL);
    }
  /*
    Threshold blur image.
  */
  status=MagickTrue;
  progress=0;
  center=(ssize_t) (GetPixelChannels(image)*(image->columns+width)*
    ((width-1)/2L)+GetPixelChannels(image)*((width-1)/2L));
  image_view=AcquireVirtualCacheView(image,exception);
  luminance_view=AcquireVirtualCacheView(luminance_image,exception);
  blur_view=AcquireAuthenticCacheView(blur_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,blur_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      contrast;

    MagickBooleanType
      sync;

    register const Quantum
      *magick_restrict l,
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) (width-1)/2L),y-(ssize_t)
      ((width-1)/2L),image->columns+width,width,exception);
    l=GetCacheViewVirtualPixels(luminance_view,-((ssize_t) (width-1)/2L),y-
      (ssize_t) ((width-1)/2L),luminance_image->columns+width,width,exception);
    q=QueueCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        intensity;

      register ssize_t
        i;

      intensity=GetPixelIntensity(image,p+center);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          alpha,
          gamma,
          pixel;

        PixelChannel
          channel;

        PixelTrait
          blur_traits,
          traits;

        register const MagickRealType
          *magick_restrict k;

        register const Quantum
          *magick_restrict luminance_pixels,
          *magick_restrict pixels;

        register ssize_t
          u;

        ssize_t
          v;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        blur_traits=GetPixelChannelTraits(blur_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (blur_traits == UndefinedPixelTrait))
          continue;
        if (((blur_traits & CopyPixelTrait) != 0) ||
            (GetPixelWriteMask(image,p+center) == 0))
          {
            SetPixelChannel(blur_image,channel,p[center+i],q);
            continue;
          }
        k=kernel;
        pixel=0.0;
        pixels=p;
        luminance_pixels=l;
        gamma=0.0;
        if ((blur_traits & BlendPixelTrait) == 0)
          {
            for (v=0; v < (ssize_t) width; v++)
            {
              for (u=0; u < (ssize_t) width; u++)
              {
                contrast=GetPixelIntensity(luminance_image,luminance_pixels)-
                  intensity;
                if (fabs(contrast) < threshold)
                  {
                    pixel+=(*k)*pixels[i];
                    gamma+=(*k);
                  }
                k++;
                pixels+=GetPixelChannels(image);
                luminance_pixels+=GetPixelChannels(luminance_image);
              }
              pixels+=GetPixelChannels(image)*image->columns;
              luminance_pixels+=GetPixelChannels(luminance_image)*
                luminance_image->columns;
            }
            if (fabs((double) gamma) < MagickEpsilon)
              {
                SetPixelChannel(blur_image,channel,p[center+i],q);
                continue;
              }
            gamma=PerceptibleReciprocal(gamma);
            SetPixelChannel(blur_image,channel,ClampToQuantum(gamma*pixel),q);
            continue;
          }
        for (v=0; v < (ssize_t) width; v++)
        {
          for (u=0; u < (ssize_t) width; u++)
          {
            contrast=GetPixelIntensity(image,pixels)-intensity;
            if (fabs(contrast) < threshold)
              {
                alpha=(double) (QuantumScale*GetPixelAlpha(image,pixels));
                pixel+=(*k)*alpha*pixels[i];
                gamma+=(*k)*alpha;
              }
            k++;
            pixels+=GetPixelChannels(image);
            luminance_pixels+=GetPixelChannels(luminance_image);
          }
          pixels+=GetPixelChannels(image)*image->columns;
          luminance_pixels+=GetPixelChannels(luminance_image)*
            luminance_image->columns;
        }
        if (fabs((double) gamma) < MagickEpsilon)
          {
            SetPixelChannel(blur_image,channel,p[center+i],q);
            continue;
          }
        gamma=PerceptibleReciprocal(gamma);
        SetPixelChannel(blur_image,channel,ClampToQuantum(gamma*pixel),q);
      }
      p+=GetPixelChannels(image);
      l+=GetPixelChannels(luminance_image);
      q+=GetPixelChannels(blur_image);
    }
    sync=SyncCacheViewAuthenticPixels(blur_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_SelectiveBlurImage)
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
  luminance_image=DestroyImage(luminance_image);
  kernel=(MagickRealType *) RelinquishAlignedMemory(kernel);
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
    *linear_image,
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
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  linear_image=CloneImage(image,0,0,MagickTrue,exception);
  shade_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if ((linear_image == (Image *) NULL) || (shade_image == (Image *) NULL))
    {
      if (linear_image != (Image *) NULL)
        linear_image=DestroyImage(linear_image);
      if (shade_image != (Image *) NULL)
        shade_image=DestroyImage(shade_image);
      return((Image *) NULL);
    }
  if (SetImageStorageClass(shade_image,DirectClass,exception) == MagickFalse)
    {
      linear_image=DestroyImage(linear_image);
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
  image_view=AcquireVirtualCacheView(linear_image,exception);
  shade_view=AcquireAuthenticCacheView(shade_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(linear_image,shade_image,linear_image->rows,1)
#endif
  for (y=0; y < (ssize_t) linear_image->rows; y++)
  {
    double
      distance,
      normal_distance,
      shade;

    PrimaryInfo
      normal;

    register const Quantum
      *magick_restrict center,
      *magick_restrict p,
      *magick_restrict post,
      *magick_restrict pre;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-1,y-1,linear_image->columns+2,3,
      exception);
    q=QueueCacheViewAuthenticPixels(shade_view,0,y,shade_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    /*
      Shade this row of pixels.
    */
    normal.z=2.0*(double) QuantumRange;  /* constant Z of surface normal */
    for (x=0; x < (ssize_t) linear_image->columns; x++)
    {
      register ssize_t
        i;

      /*
        Determine the surface normal and compute shading.
      */
      pre=p+GetPixelChannels(linear_image);
      center=pre+(linear_image->columns+2)*GetPixelChannels(linear_image);
      post=center+(linear_image->columns+2)*GetPixelChannels(linear_image);
      normal.x=(double) (
        GetPixelIntensity(linear_image,pre-GetPixelChannels(linear_image))+
        GetPixelIntensity(linear_image,center-GetPixelChannels(linear_image))+
        GetPixelIntensity(linear_image,post-GetPixelChannels(linear_image))-
        GetPixelIntensity(linear_image,pre+GetPixelChannels(linear_image))-
        GetPixelIntensity(linear_image,center+GetPixelChannels(linear_image))-
        GetPixelIntensity(linear_image,post+GetPixelChannels(linear_image)));
      normal.y=(double) (
        GetPixelIntensity(linear_image,post-GetPixelChannels(linear_image))+
        GetPixelIntensity(linear_image,post)+
        GetPixelIntensity(linear_image,post+GetPixelChannels(linear_image))-
        GetPixelIntensity(linear_image,pre-GetPixelChannels(linear_image))-
        GetPixelIntensity(linear_image,pre)-
        GetPixelIntensity(linear_image,pre+GetPixelChannels(linear_image)));
      if ((fabs(normal.x) <= MagickEpsilon) &&
          (fabs(normal.y) <= MagickEpsilon))
        shade=light.z;
      else
        {
          shade=0.0;
          distance=normal.x*light.x+normal.y*light.y+normal.z*light.z;
          if (distance > MagickEpsilon)
            {
              normal_distance=normal.x*normal.x+normal.y*normal.y+
                normal.z*normal.z;
              if (normal_distance > (MagickEpsilon*MagickEpsilon))
                shade=distance/sqrt((double) normal_distance);
            }
        }
      for (i=0; i < (ssize_t) GetPixelChannels(linear_image); i++)
      {
        PixelChannel
          channel;

        PixelTrait
          shade_traits,
          traits;

        channel=GetPixelChannelChannel(linear_image,i);
        traits=GetPixelChannelTraits(linear_image,channel);
        shade_traits=GetPixelChannelTraits(shade_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (shade_traits == UndefinedPixelTrait))
          continue;
        if (((shade_traits & CopyPixelTrait) != 0) ||
            (GetPixelWriteMask(linear_image,center) == 0))
          {
            SetPixelChannel(shade_image,channel,center[i],q);
            continue;
          }
        if ((traits & UpdatePixelTrait) == 0)
          {
            SetPixelChannel(shade_image,channel,center[i],q);
            continue;
          }
        if (gray != MagickFalse)
          {
            SetPixelChannel(shade_image,channel,ClampToQuantum(shade),q);
            continue;
          }
        SetPixelChannel(shade_image,channel,ClampToQuantum(QuantumScale*shade*
          center[i]),q);
      }
      p+=GetPixelChannels(linear_image);
      q+=GetPixelChannels(shade_image);
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
  linear_image=DestroyImage(linear_image);
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
%
%  A description of each parameter follows:
%
%    o image: the image.
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
  double
    gamma,
    normalize;

  Image
    *sharp_image;

  KernelInfo
    *kernel_info;

  register ssize_t
    i;

  size_t
    width;

  ssize_t
    j,
    u,
    v;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  width=GetOptimalKernelWidth2D(radius,sigma);
  kernel_info=AcquireKernelInfo((const char *) NULL,exception);
  if (kernel_info == (KernelInfo *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(kernel_info,0,sizeof(*kernel_info));
  kernel_info->width=width;
  kernel_info->height=width;
  kernel_info->x=(ssize_t) (width-1)/2;
  kernel_info->y=(ssize_t) (width-1)/2;
  kernel_info->signature=MagickCoreSignature;
  kernel_info->values=(MagickRealType *) MagickAssumeAligned(
    AcquireAlignedMemory(kernel_info->width,kernel_info->height*
    sizeof(*kernel_info->values)));
  if (kernel_info->values == (MagickRealType *) NULL)
    {
      kernel_info=DestroyKernelInfo(kernel_info);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  normalize=0.0;
  j=(ssize_t) (kernel_info->width-1)/2;
  i=0;
  for (v=(-j); v <= j; v++)
  {
    for (u=(-j); u <= j; u++)
    {
      kernel_info->values[i]=(MagickRealType) (-exp(-((double) u*u+v*v)/(2.0*
        MagickSigma*MagickSigma))/(2.0*MagickPI*MagickSigma*MagickSigma));
      normalize+=kernel_info->values[i];
      i++;
    }
  }
  kernel_info->values[i/2]=(double) ((-2.0)*normalize);
  normalize=0.0;
  for (i=0; i < (ssize_t) (kernel_info->width*kernel_info->height); i++)
    normalize+=kernel_info->values[i];
  gamma=PerceptibleReciprocal(normalize);
  for (i=0; i < (ssize_t) (kernel_info->width*kernel_info->height); i++)
    kernel_info->values[i]*=gamma;
  sharp_image=ConvolveImage(image,kernel_info,exception);
  kernel_info=DestroyKernelInfo(kernel_info);
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
%  pixel in a square area defined by the radius parameter.
%
%  The format of the SpreadImage method is:
%
%      Image *SpreadImage(const Image *image,
%        const PixelInterpolateMethod method,const double radius,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o method:  intepolation method.
%
%    o radius:  choose a random pixel in a neighborhood of this extent.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *SpreadImage(const Image *image,
  const PixelInterpolateMethod method,const double radius,
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

  RandomInfo
    **magick_restrict random_info;

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
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  spread_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (spread_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(spread_image,DirectClass,exception) == MagickFalse)
    {
      spread_image=DestroyImage(spread_image);
      return((Image *) NULL);
    }
  /*
    Spread image.
  */
  status=MagickTrue;
  progress=0;
  width=GetOptimalKernelWidth1D(radius,0.5);
  random_info=AcquireRandomInfoThreadSet();
  image_view=AcquireVirtualCacheView(image,exception);
  spread_view=AcquireAuthenticCacheView(spread_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  key=GetRandomSecretKey(random_info[0]);
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,spread_image,image->rows,key == ~0UL)
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
    q=QueueCacheViewAuthenticPixels(spread_view,0,y,spread_image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      PointInfo
        point;

      point.x=GetPseudoRandomValue(random_info[id]);
      point.y=GetPseudoRandomValue(random_info[id]);
      status=InterpolatePixelChannels(image,image_view,spread_image,method,
        (double) x+width*(point.x-0.5),(double) y+width*(point.y-0.5),q,
        exception);
      q+=GetPixelChannels(spread_image);
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
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o gain: the percentage of the difference between the original and the
%      blur image that is added back into the original.
%
%    o threshold: the threshold in pixels needed to apply the diffence gain.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *UnsharpMaskImage(const Image *image,const double radius,
  const double sigma,const double gain,const double threshold,
  ExceptionInfo *exception)
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

  double
    quantum_threshold;

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  unsharp_image=AccelerateUnsharpMaskImage(image,radius,sigma,gain,threshold,
    exception);
  if (unsharp_image != (Image *) NULL)
    return(unsharp_image);
#endif
  unsharp_image=BlurImage(image,radius,sigma,exception);
  if (unsharp_image == (Image *) NULL)
    return((Image *) NULL);
  quantum_threshold=(double) QuantumRange*threshold;
  /*
    Unsharp-mask image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  unsharp_view=AcquireAuthenticCacheView(unsharp_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,unsharp_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(unsharp_view,0,y,unsharp_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
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
        double
          pixel;

        PixelChannel
          channel;

        PixelTrait
          traits,
          unsharp_traits;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        unsharp_traits=GetPixelChannelTraits(unsharp_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (unsharp_traits == UndefinedPixelTrait))
          continue;
        if (((unsharp_traits & CopyPixelTrait) != 0) ||
            (GetPixelWriteMask(image,p) == 0))
          {
            SetPixelChannel(unsharp_image,channel,p[i],q);
            continue;
          }
        pixel=p[i]-(double) GetPixelChannel(unsharp_image,channel,q);
        if (fabs(2.0*pixel) < quantum_threshold)
          pixel=(double) p[i];
        else
          pixel=(double) p[i]+gain*pixel;
        SetPixelChannel(unsharp_image,channel,ClampToQuantum(pixel),q);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(unsharp_image);
    }
    if (SyncCacheViewAuthenticPixels(unsharp_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_UnsharpMaskImage)
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

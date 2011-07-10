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
#include "MagickCore/accelerate.h"
#include "MagickCore/blob.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
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
#include "MagickCore/morphology.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel-accessor.h"
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

MagickExport MagickBooleanType AdaptiveLevelImage(Image *image,
  const char *levels)
{
  double
    black_point,
    gamma,
    white_point;

  GeometryInfo
    geometry_info;

  MagickBooleanType
    status;

  MagickStatusType
    flags;

  /*
    Parse levels.
  */
  if (levels == (char *) NULL)
    return(MagickFalse);
  flags=ParseGeometry(levels,&geometry_info);
  black_point=geometry_info.rho;
  white_point=(double) QuantumRange;
  if ((flags & SigmaValue) != 0)
    white_point=geometry_info.sigma;
  gamma=1.0;
  if ((flags & XiValue) != 0)
    gamma=geometry_info.xi;
  if ((flags & PercentValue) != 0)
    {
      black_point*=(double) image->columns*image->rows/100.0;
      white_point*=(double) image->columns*image->rows/100.0;
    }
  if ((flags & SigmaValue) == 0)
    white_point=(double) QuantumRange-black_point;
  if ((flags & AspectValue ) == 0)
    status=LevelImage(image,black_point,white_point,gamma);
  else
    status=LevelizeImage(image,black_point,white_point,gamma);
  return(status);
}

MagickExport Image *AdaptiveBlurImage(const Image *image,
  const double radius,const double sigma,ExceptionInfo *exception)
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

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
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
  (void) AdaptiveLevelImage(edge_image,"20%,95%");
  gaussian_image=GaussianBlurImage(edge_image,radius,sigma,exception);
  if (gaussian_image != (Image *) NULL)
    {
      edge_image=DestroyImage(edge_image);
      edge_image=gaussian_image;
    }
  (void) AdaptiveLevelImage(edge_image,"10%,95%");
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
  for (i=0; i < (ssize_t) width; i+=2)
  {
    kernel[i]=(double *) AcquireQuantumMemory((size_t) (width-i),(width-i)*
      sizeof(**kernel));
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
    if (fabs(normalize) <= MagickEpsilon)
      normalize=1.0;
    normalize=1.0/normalize;
    for (k=0; k < (j*j); k++)
      kernel[i][k]=normalize*kernel[i][k];
  }
  if (i < (ssize_t) width)
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
  GetPixelInfo(image,&bias);
  SetPixelInfoBias(image,&bias);
  image_view=AcquireCacheView(image);
  edge_view=AcquireCacheView(edge_image);
  blur_view=AcquireCacheView(blur_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) blur_image->rows; y++)
  {
    register const Quantum
      *restrict p,
      *restrict r;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    r=GetCacheViewVirtualPixels(edge_view,0,y,edge_image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if ((r == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) blur_image->columns; x++)
    {
      PixelInfo
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
      if (p == (const Quantum *) NULL)
        break;
      pixel=bias;
      k=kernel[i];
      for (v=0; v < (ssize_t) (width-i); v++)
      {
        for (u=0; u < (ssize_t) (width-i); u++)
        {
          alpha=1.0;
          if (((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0) &&
              (image->matte != MagickFalse))
            alpha=(MagickRealType) (QuantumScale*GetPixelAlpha(image,p));
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            pixel.red+=(*k)*alpha*GetPixelRed(image,p);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            pixel.green+=(*k)*alpha*GetPixelGreen(image,p);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            pixel.blue+=(*k)*alpha*GetPixelBlue(image,p);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (image->colorspace == CMYKColorspace))
            pixel.black+=(*k)*alpha*GetPixelBlack(image,p);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            pixel.alpha+=(*k)*GetPixelAlpha(image,p);
          gamma+=(*k)*alpha;
          k++;
          p+=GetPixelComponents(image);
        }
      }
      gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        SetPixelRed(blur_image,ClampToQuantum(gamma*pixel.red),q);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        SetPixelGreen(blur_image,ClampToQuantum(gamma*pixel.green),q);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        SetPixelBlue(blur_image,ClampToQuantum(gamma*pixel.blue),q);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelBlack(blur_image,ClampToQuantum(gamma*pixel.black),q);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        SetPixelAlpha(blur_image,ClampToQuantum(pixel.alpha),q);
      q+=GetPixelComponents(blur_image);
      r+=GetPixelComponents(edge_image);
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
  for (i=0; i < (ssize_t) width;  i+=2)
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

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
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
  (void) AdaptiveLevelImage(edge_image,"20%,95%");
  gaussian_image=GaussianBlurImage(edge_image,radius,sigma,exception);
  if (gaussian_image != (Image *) NULL)
    {
      edge_image=DestroyImage(edge_image);
      edge_image=gaussian_image;
    }
  (void) AdaptiveLevelImage(edge_image,"10%,95%");
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
  for (i=0; i < (ssize_t) width; i+=2)
  {
    kernel[i]=(double *) AcquireQuantumMemory((size_t) (width-i),(width-i)*
      sizeof(**kernel));
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
    if (fabs(normalize) <= MagickEpsilon)
      normalize=1.0;
    normalize=1.0/normalize;
    for (k=0; k < (j*j); k++)
      kernel[i][k]=normalize*kernel[i][k];
  }
  if (i < (ssize_t) width)
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
  GetPixelInfo(image,&bias);
  SetPixelInfoBias(image,&bias);
  image_view=AcquireCacheView(image);
  edge_view=AcquireCacheView(edge_image);
  sharp_view=AcquireCacheView(sharp_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) sharp_image->rows; y++)
  {
    register const Quantum
      *restrict p,
      *restrict r;

    register Quantum
      *restrict q;

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
      PixelInfo
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
      if (p == (const Quantum *) NULL)
        break;
      k=kernel[i];
      pixel=bias;
      for (v=0; v < (ssize_t) (width-i); v++)
      {
        for (u=0; u < (ssize_t) (width-i); u++)
        {
          alpha=1.0;
          if (((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0) &&
              (image->matte != MagickFalse))
            alpha=(MagickRealType) (QuantumScale*GetPixelAlpha(image,p));
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            pixel.red+=(*k)*alpha*GetPixelRed(image,p);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            pixel.green+=(*k)*alpha*GetPixelGreen(image,p);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            pixel.blue+=(*k)*alpha*GetPixelBlue(image,p);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (image->colorspace == CMYKColorspace))
            pixel.black+=(*k)*alpha*GetPixelBlack(image,p);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            pixel.alpha+=(*k)*GetPixelAlpha(image,p);
          gamma+=(*k)*alpha;
          k++;
          p+=GetPixelComponents(image);
        }
      }
      gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        SetPixelRed(sharp_image,ClampToQuantum(gamma*pixel.red),q);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        SetPixelGreen(sharp_image,ClampToQuantum(gamma*pixel.green),q);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        SetPixelBlue(sharp_image,ClampToQuantum(gamma*pixel.blue),q);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelBlack(sharp_image,ClampToQuantum(gamma*pixel.black),q);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        SetPixelAlpha(sharp_image,ClampToQuantum(pixel.alpha),q);
      q+=GetPixelComponents(sharp_image);
      r+=GetPixelComponents(edge_image);
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
  for (i=0; i < (ssize_t) width;  i+=2)
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

static double *GetBlurKernel(const size_t width,const double sigma)
{
  double
    *kernel,
    normalize;

  register ssize_t
    i;

  ssize_t
    j,
    k;

  /*
    Generate a 1-D convolution kernel.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  kernel=(double *) AcquireQuantumMemory((size_t) width,sizeof(*kernel));
  if (kernel == (double *) NULL)
    return(0);
  normalize=0.0;
  j=(ssize_t) width/2;
  i=0;
  for (k=(-j); k <= j; k++)
  {
    kernel[i]=(double) (exp(-((double) k*k)/(2.0*MagickSigma*MagickSigma))/
      (MagickSQ2PI*MagickSigma));
    normalize+=kernel[i];
    i++;
  }
  for (i=0; i < (ssize_t) width; i++)
    kernel[i]/=normalize;
  return(kernel);
}

MagickExport Image *BlurImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
{
#define BlurImageTag  "Blur/Image"

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

  PixelInfo
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
        "  BlurImage with %.20g kernel:",(double) width);
      message=AcquireString("");
      k=kernel;
      for (i=0; i < (ssize_t) width; i++)
      {
        *message='\0';
        (void) FormatLocaleString(format,MaxTextExtent,"%.20g: ",(double) i);
        (void) ConcatenateString(&message,format);
        (void) FormatLocaleString(format,MaxTextExtent,"%g ",*k++);
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
  GetPixelInfo(image,&bias);
  SetPixelInfoBias(image,&bias);
  image_view=AcquireCacheView(image);
  blur_view=AcquireCacheView(blur_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) blur_image->rows; y++)
  {
    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) width/2L),y,
      image->columns+width,1,exception);
    q=GetCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) blur_image->columns; x++)
    {
      PixelInfo
        pixel;

      register const double
        *restrict k;

      register const Quantum
        *restrict kernel_pixels;

      register ssize_t
        i;

      pixel=bias;
      k=kernel;
      kernel_pixels=p;
      if (((GetPixelAlphaTraits(image) & ActivePixelTrait) == 0) ||
          (image->matte == MagickFalse))
        {
          for (i=0; i < (ssize_t) width; i++)
          {
            pixel.red+=(*k)*GetPixelRed(image,kernel_pixels);
            pixel.green+=(*k)*GetPixelGreen(image,kernel_pixels);
            pixel.blue+=(*k)*GetPixelBlue(image,kernel_pixels);
            if (image->colorspace == CMYKColorspace)
              pixel.black+=(*k)*GetPixelBlack(image,kernel_pixels);
            k++;
            kernel_pixels+=GetPixelComponents(image);
          }
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            SetPixelRed(blur_image,ClampToQuantum(pixel.red),q);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            SetPixelGreen(blur_image,ClampToQuantum(pixel.green),q);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            SetPixelBlue(blur_image,ClampToQuantum(pixel.blue),q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (blur_image->colorspace == CMYKColorspace))
            SetPixelBlack(blur_image,ClampToQuantum(pixel.black),q);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            {
              k=kernel;
              kernel_pixels=p;
              for (i=0; i < (ssize_t) width; i++)
              {
                pixel.alpha+=(*k)*GetPixelAlpha(image,kernel_pixels);
                k++;
                kernel_pixels+=GetPixelComponents(image);
              }
              SetPixelAlpha(blur_image,ClampToQuantum(pixel.alpha),q);
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
            alpha=(MagickRealType) (QuantumScale*
              GetPixelAlpha(image,kernel_pixels));
            pixel.red+=(*k)*alpha*GetPixelRed(image,kernel_pixels);
            pixel.green+=(*k)*alpha*GetPixelGreen(image,kernel_pixels);
            pixel.blue+=(*k)*alpha*GetPixelBlue(image,kernel_pixels);
            if (image->colorspace == CMYKColorspace)
              pixel.black+=(*k)*alpha*GetPixelBlack(image,kernel_pixels);
            gamma+=(*k)*alpha;
            k++;
            kernel_pixels+=GetPixelComponents(image);
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            SetPixelRed(blur_image,ClampToQuantum(gamma*pixel.red),q);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            SetPixelGreen(blur_image,ClampToQuantum(gamma*pixel.green),q);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            SetPixelBlue(blur_image,ClampToQuantum(gamma*pixel.blue),q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (blur_image->colorspace == CMYKColorspace))
            SetPixelBlack(blur_image,ClampToQuantum(gamma*pixel.black),q);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            {
              k=kernel;
              kernel_pixels=p;
              for (i=0; i < (ssize_t) width; i++)
              {
                pixel.alpha+=(*k)*GetPixelAlpha(image,kernel_pixels);
                k++;
                kernel_pixels+=GetPixelComponents(image);
              }
              SetPixelAlpha(blur_image,ClampToQuantum(pixel.alpha),q);
            }
        }
      p+=GetPixelComponents(image);
      q+=GetPixelComponents(blur_image);
    }
    if (SyncCacheViewAuthenticPixels(blur_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_BlurImage)
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
  for (x=0; x < (ssize_t) blur_image->columns; x++)
  {
    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      y;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,x,-((ssize_t) width/2L),1,
      image->rows+width,exception);
    q=GetCacheViewAuthenticPixels(blur_view,x,0,1,blur_image->rows,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (y=0; y < (ssize_t) blur_image->rows; y++)
    {
      PixelInfo
        pixel;

      register const double
        *restrict k;

      register const Quantum
        *restrict kernel_pixels;

      register ssize_t
        i;

      pixel=bias;
      k=kernel;
      kernel_pixels=p;
      if (((GetPixelAlphaTraits(image) & ActivePixelTrait) == 0) ||
          (blur_image->matte == MagickFalse))
        {
          for (i=0; i < (ssize_t) width; i++)
          {
            pixel.red+=(*k)*GetPixelRed(blur_image,kernel_pixels);
            pixel.green+=(*k)*GetPixelGreen(blur_image,kernel_pixels);
            pixel.blue+=(*k)*GetPixelBlue(blur_image,kernel_pixels);
            if (blur_image->colorspace == CMYKColorspace)
              pixel.black+=(*k)*GetPixelBlack(blur_image,kernel_pixels);
            k++;
            kernel_pixels+=GetPixelComponents(blur_image);
          }
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            SetPixelRed(blur_image,ClampToQuantum(pixel.red),q);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            SetPixelGreen(blur_image,ClampToQuantum(pixel.green),q);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            SetPixelBlue(blur_image,ClampToQuantum(pixel.blue),q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (blur_image->colorspace == CMYKColorspace))
            SetPixelBlack(blur_image,ClampToQuantum(pixel.black),q);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            {
              k=kernel;
              kernel_pixels=p;
              for (i=0; i < (ssize_t) width; i++)
              {
                pixel.alpha+=(*k)*GetPixelAlpha(blur_image,kernel_pixels);
                k++;
                kernel_pixels+=GetPixelComponents(blur_image);
              }
              SetPixelAlpha(blur_image,ClampToQuantum(pixel.alpha),q);
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
            alpha=(MagickRealType) (QuantumScale*
              GetPixelAlpha(blur_image,kernel_pixels));
            pixel.red+=(*k)*alpha*GetPixelRed(blur_image,kernel_pixels);
            pixel.green+=(*k)*alpha*GetPixelGreen(blur_image,kernel_pixels);
            pixel.blue+=(*k)*alpha*GetPixelBlue(blur_image,kernel_pixels);
            if (blur_image->colorspace == CMYKColorspace)
              pixel.black+=(*k)*alpha*GetPixelBlack(blur_image,kernel_pixels);
            gamma+=(*k)*alpha;
            k++;
            kernel_pixels+=GetPixelComponents(blur_image);
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            SetPixelRed(blur_image,ClampToQuantum(gamma*pixel.red),q);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            SetPixelGreen(blur_image,ClampToQuantum(gamma*pixel.green),q);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            SetPixelBlue(blur_image,ClampToQuantum(gamma*pixel.blue),q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (blur_image->colorspace == CMYKColorspace))
            SetPixelBlack(blur_image,ClampToQuantum(gamma*pixel.black),q);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            {
              k=kernel;
              kernel_pixels=p;
              for (i=0; i < (ssize_t) width; i++)
              {
                pixel.alpha+=(*k)*GetPixelAlpha(blur_image,kernel_pixels);
                k++;
                kernel_pixels+=GetPixelComponents(blur_image);
              }
              SetPixelAlpha(blur_image,ClampToQuantum(pixel.alpha),q);
            }
        }
      p+=GetPixelComponents(blur_image);
      q+=GetPixelComponents(blur_image);
    }
    if (SyncCacheViewAuthenticPixels(blur_view,exception) == MagickFalse)
      status=MagickFalse;
    if (blur_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_BlurImage)
#endif
        proceed=SetImageProgress(blur_image,BlurImageTag,progress++,
          blur_image->rows+blur_image->columns);
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
%      Image *ConvolveImage(const Image *image,const size_t order,
%        const double *kernel,ExceptionInfo *exception)
%  A description of each parameter follows:
%
%    o image: the image.
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
#define ConvolveImageTag  "Convolve/Image"

  CacheView
    *convolve_view,
    *image_view;

  double
    *normal_kernel;

  Image
    *convolve_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    bias;

  MagickRealType
    gamma;

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
  normal_kernel=(double *) AcquireQuantumMemory(width*width,
    sizeof(*normal_kernel));
  if (normal_kernel == (double *) NULL)
    {
      convolve_image=DestroyImage(convolve_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  gamma=0.0;
  for (i=0; i < (ssize_t) (width*width); i++)
    gamma+=kernel[i];
  gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
  for (i=0; i < (ssize_t) (width*width); i++)
    normal_kernel[i]=gamma*kernel[i];
  /*
    Convolve image.
  */
  status=MagickTrue;
  progress=0;
  GetPixelInfo(image,&bias);
  SetPixelInfoBias(image,&bias);
  image_view=AcquireCacheView(image);
  convolve_view=AcquireCacheView(convolve_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) width/2L),y-(ssize_t)
      (width/2L),image->columns+width,width,exception);
    q=GetCacheViewAuthenticPixels(convolve_view,0,y,convolve_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      PixelInfo
        pixel;

      register const double
        *restrict k;

      register const Quantum
        *restrict kernel_pixels;

      register ssize_t
        u;

      ssize_t
        v;

      pixel=bias;
      k=normal_kernel;
      kernel_pixels=p;
      if (((GetPixelAlphaTraits(image) & ActivePixelTrait) == 0) ||
          (image->matte == MagickFalse))
        {
          for (v=0; v < (ssize_t) width; v++)
          {
            for (u=0; u < (ssize_t) width; u++)
            {
              pixel.red+=(*k)*GetPixelRed(image,kernel_pixels+u*
                GetPixelComponents(image));
              pixel.green+=(*k)*GetPixelGreen(image,kernel_pixels+u*
                GetPixelComponents(image));
              pixel.blue+=(*k)*GetPixelBlue(image,kernel_pixels+u*
                GetPixelComponents(image));
              if (image->colorspace == CMYKColorspace)
                pixel.black+=(*k)*GetPixelBlack(image,kernel_pixels+u*
                  GetPixelComponents(image));
              k++;
            }
            kernel_pixels+=(image->columns+width)*GetPixelComponents(image);
          }
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            SetPixelRed(convolve_image,ClampToQuantum(pixel.red),q);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            SetPixelGreen(convolve_image,ClampToQuantum(pixel.green),q);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            SetPixelBlue(convolve_image,ClampToQuantum(pixel.blue),q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (image->colorspace == CMYKColorspace))
            SetPixelBlack(convolve_image,ClampToQuantum(pixel.black),q);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            {
              k=normal_kernel;
              kernel_pixels=p;
              for (v=0; v < (ssize_t) width; v++)
              {
                for (u=0; u < (ssize_t) width; u++)
                {
                  pixel.alpha+=(*k)*GetPixelAlpha(image,kernel_pixels+u*
                    GetPixelComponents(image));
                  k++;
                }
                kernel_pixels+=(image->columns+width)*GetPixelComponents(image);
              }
              SetPixelAlpha(convolve_image,ClampToQuantum(pixel.alpha),q);
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
              alpha=(MagickRealType) (QuantumScale*GetPixelAlpha(image,
                kernel_pixels+u*GetPixelComponents(image)));
              pixel.red+=(*k)*alpha*GetPixelRed(image,kernel_pixels+u*
                GetPixelComponents(image));
              pixel.green+=(*k)*alpha*GetPixelGreen(image,kernel_pixels+u*
                GetPixelComponents(image));
              pixel.blue+=(*k)*alpha*GetPixelBlue(image,kernel_pixels+u*
                GetPixelComponents(image));
              if (image->colorspace == CMYKColorspace)
                pixel.black+=(*k)*alpha*GetPixelBlack(image,kernel_pixels+u*
                  GetPixelComponents(image));
              gamma+=(*k)*alpha;
              k++;
            }
            kernel_pixels+=(image->columns+width)*GetPixelComponents(image);
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            SetPixelRed(convolve_image,ClampToQuantum(gamma*pixel.red),q);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            SetPixelGreen(convolve_image,ClampToQuantum(gamma*pixel.green),q);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            SetPixelBlue(convolve_image,ClampToQuantum(gamma*pixel.blue),q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (image->colorspace == CMYKColorspace))
            SetPixelBlack(convolve_image,ClampToQuantum(gamma*pixel.black),q);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            {
              k=normal_kernel;
              kernel_pixels=p;
              for (v=0; v < (ssize_t) width; v++)
              {
                for (u=0; u < (ssize_t) width; u++)
                {
                  pixel.alpha+=(*k)*GetPixelAlpha(image,kernel_pixels+u);
                  k++;
                }
                kernel_pixels+=(image->columns+width)*GetPixelComponents(image);
              }
              SetPixelAlpha(convolve_image,ClampToQuantum(pixel.alpha),q);
            }
        }
      p+=GetPixelComponents(image);
      q+=GetPixelComponents(convolve_image);
    }
    sync=SyncCacheViewAuthenticPixels(convolve_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ConvolveImage)
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

static void Hull(const ssize_t x_offset,const ssize_t y_offset,
  const size_t columns,const size_t rows,Quantum *f,Quantum *g,
  const int polarity)
{
  MagickRealType
    v;

  register Quantum
    *p,
    *q,
    *r,
    *s;

  register ssize_t
    x;

  ssize_t
    y;

  assert(f != (Quantum *) NULL);
  assert(g != (Quantum *) NULL);
  p=f+(columns+2);
  q=g+(columns+2);
  r=p+(y_offset*((ssize_t) columns+2)+x_offset);
  for (y=0; y < (ssize_t) rows; y++)
  {
    p++;
    q++;
    r++;
    if (polarity > 0)
      for (x=(ssize_t) columns; x != 0; x--)
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
      for (x=(ssize_t) columns; x != 0; x--)
      {
        v=(MagickRealType) (*p);
        if ((MagickRealType) *r <= (v-(MagickRealType) ScaleCharToQuantum(2)))
          v-=(ssize_t) ScaleCharToQuantum(1);
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
  r=q+(y_offset*((ssize_t) columns+2)+x_offset);
  s=q-(y_offset*((ssize_t) columns+2)+x_offset);
  for (y=0; y < (ssize_t) rows; y++)
  {
    p++;
    q++;
    r++;
    s++;
    if (polarity > 0)
      for (x=(ssize_t) columns; x != 0; x--)
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
      for (x=(ssize_t) columns; x != 0; x--)
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

  MagickBooleanType
    status;

  register ssize_t
    i;

  Quantum
    *restrict buffers,
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
    Allocate image buffers.
  */
  length=(size_t) ((image->columns+2)*(image->rows+2));
  pixels=(Quantum *) AcquireQuantumMemory(length,2*sizeof(*pixels));
  buffers=(Quantum *) AcquireQuantumMemory(length,2*sizeof(*pixels));
  if ((pixels == (Quantum *) NULL) || (buffers == (Quantum *) NULL))
    {
      if (buffers != (Quantum *) NULL)
        buffers=(Quantum *) RelinquishMagickMemory(buffers);
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
  image_view=AcquireCacheView(image);
  despeckle_view=AcquireCacheView(despeckle_image);
  for (i=0; i < (ssize_t) number_channels; i++)
  {
    register Quantum
      *buffer,
      *pixel;

    register ssize_t
      k,
      x;

    ssize_t
      j,
      y;

    if (status == MagickFalse)
      continue;
    pixel=pixels;
    (void) ResetMagickMemory(pixel,0,length*sizeof(*pixel));
    buffer=buffers;
    j=(ssize_t) image->columns+2;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      register const Quantum
        *restrict p;

      p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        break;
      j++;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        switch (i)
        {
          case 0: pixel[j]=GetPixelRed(image,p); break;
          case 1: pixel[j]=GetPixelGreen(image,p); break;
          case 2: pixel[j]=GetPixelBlue(image,p); break;
          case 3: pixel[j]=GetPixelAlpha(image,p); break;
          case 4: pixel[j]=GetPixelBlack(image,p); break;
          default: break;
        }
        p+=GetPixelComponents(image);
        j++;
      }
      j++;
    }
    (void) ResetMagickMemory(buffer,0,length*sizeof(*buffer));
    for (k=0; k < 4; k++)
    {
      Hull(X[k],Y[k],image->columns,image->rows,pixel,buffer,1);
      Hull(-X[k],-Y[k],image->columns,image->rows,pixel,buffer,1);
      Hull(-X[k],-Y[k],image->columns,image->rows,pixel,buffer,-1);
      Hull(X[k],Y[k],image->columns,image->rows,pixel,buffer,-1);
    }
    j=(ssize_t) image->columns+2;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      MagickBooleanType
        sync;

      register Quantum
        *restrict q;

      q=GetCacheViewAuthenticPixels(despeckle_view,0,y,despeckle_image->columns,
        1,exception);
      if (q == (const Quantum *) NULL)
        break;
      j++;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        switch (i)
        {
          case 0: SetPixelRed(despeckle_image,pixel[j],q); break;
          case 1: SetPixelGreen(despeckle_image,pixel[j],q); break;
          case 2: SetPixelBlue(despeckle_image,pixel[j],q); break;
          case 3: SetPixelAlpha(despeckle_image,pixel[j],q); break;
          case 4: SetPixelBlack(despeckle_image,pixel[j],q); break;
          default: break;
        }
        q+=GetPixelComponents(despeckle_image);
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
  buffers=(Quantum *) RelinquishMagickMemory(buffers);
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
  kernel=(double *) AcquireQuantumMemory((size_t) width,width*sizeof(*kernel));
  if (kernel == (double *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  for (i=0; i < (ssize_t) (width*width); i++)
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
  kernel=(double *) AcquireQuantumMemory((size_t) width,width*sizeof(*kernel));
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
MagickExport Image *FilterImage(const Image *image,
  const KernelInfo *kernel,ExceptionInfo *exception)
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

  PixelInfo
    bias;

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
    Filter image.
  */
  status=MagickTrue;
  progress=0;
  GetPixelInfo(image,&bias);
  SetPixelInfoBias(image,&bias);
  image_view=AcquireCacheView(image);
  filter_view=AcquireCacheView(filter_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) kernel->width/2L),
      y-(ssize_t) (kernel->height/2L),image->columns+kernel->width,
      kernel->height,exception);
    q=GetCacheViewAuthenticPixels(filter_view,0,y,filter_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      PixelInfo
        pixel;

      register const double
        *restrict k;

      register const Quantum
        *restrict kernel_pixels;

      register ssize_t
        u;

      ssize_t
        v;

      pixel=bias;
      k=kernel->values;
      kernel_pixels=p;
      if (((GetPixelAlphaTraits(image) & ActivePixelTrait) == 0) ||
          (image->matte == MagickFalse))
        {
          for (v=0; v < (ssize_t) kernel->width; v++)
          {
            for (u=0; u < (ssize_t) kernel->height; u++)
            {
              pixel.red+=(*k)*GetPixelRed(image,kernel_pixels+u*
                GetPixelComponents(image));
              pixel.green+=(*k)*GetPixelGreen(image,kernel_pixels+u*
                GetPixelComponents(image));
              pixel.blue+=(*k)*GetPixelBlue(image,kernel_pixels+u*
                GetPixelComponents(image));
              if (image->colorspace == CMYKColorspace)
                pixel.black+=(*k)*GetPixelBlack(image,kernel_pixels+u*
                  GetPixelComponents(image));
              k++;
            }
            kernel_pixels+=(image->columns+kernel->width)*
              GetPixelComponents(image);
          }
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            SetPixelRed(filter_image,ClampToQuantum(pixel.red),q);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            SetPixelGreen(filter_image,ClampToQuantum(pixel.green),q);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            SetPixelBlue(filter_image,ClampToQuantum(pixel.blue),q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (image->colorspace == CMYKColorspace))
            SetPixelBlack(filter_image,ClampToQuantum(pixel.black),q);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            {
              k=kernel->values;
              kernel_pixels=p;
              for (v=0; v < (ssize_t) kernel->width; v++)
              {
                for (u=0; u < (ssize_t) kernel->height; u++)
                {
                  pixel.alpha+=(*k)*GetPixelRed(image,kernel_pixels+u*
                    GetPixelComponents(image));
                  k++;
                }
                kernel_pixels+=(image->columns+kernel->width)*
                  GetPixelComponents(image);
              }
              SetPixelAlpha(filter_image,ClampToQuantum(pixel.alpha),q);
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
              alpha=(MagickRealType) (QuantumScale*GetPixelAlpha(image,
                kernel_pixels+u*GetPixelComponents(image)));
              pixel.red+=(*k)*alpha*GetPixelRed(image,kernel_pixels+u*
                GetPixelComponents(image));
              pixel.green+=(*k)*alpha*GetPixelGreen(image,kernel_pixels+u*
                GetPixelComponents(image));
              pixel.blue+=(*k)*alpha*GetPixelBlue(image,kernel_pixels+u*
                GetPixelComponents(image));
              if (image->colorspace == CMYKColorspace)
                pixel.black+=(*k)*alpha*GetPixelBlack(image,kernel_pixels+u*
                  GetPixelComponents(image));
              gamma+=(*k)*alpha;
              k++;
            }
            kernel_pixels+=(image->columns+kernel->width)*
              GetPixelComponents(image);
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            SetPixelRed(filter_image,ClampToQuantum(gamma*pixel.red),q);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            SetPixelGreen(filter_image,ClampToQuantum(gamma*pixel.green),q);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            SetPixelBlue(filter_image,ClampToQuantum(gamma*pixel.blue),q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (image->colorspace == CMYKColorspace))
            SetPixelBlack(filter_image,ClampToQuantum(gamma*pixel.black),q);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            {
              k=kernel->values;
              kernel_pixels=p;
              for (v=0; v < (ssize_t) kernel->width; v++)
              {
                for (u=0; u < (ssize_t) kernel->height; u++)
                {
                  pixel.alpha+=(*k)*GetPixelAlpha(image,kernel_pixels+u*
                    GetPixelComponents(image));
                  k++;
                }
                kernel_pixels+=(image->columns+kernel->width)*
                  GetPixelComponents(image);
              }
              SetPixelAlpha(filter_image,ClampToQuantum(pixel.alpha),q);
            }
        }
      p+=GetPixelComponents(image);
      q+=GetPixelComponents(filter_image);
    }
    sync=SyncCacheViewAuthenticPixels(filter_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_FilterImage)
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
MagickExport Image *GaussianBlurImage(const Image *image,
  const double radius,const double sigma,ExceptionInfo *exception)
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
  kernel=(double *) AcquireQuantumMemory((size_t) width,width*sizeof(*kernel));
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
  blur_image=ConvolveImage(image,width,kernel,exception);
  kernel=(double *) RelinquishMagickMemory(kernel);
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
  kernel=(double *) AcquireQuantumMemory((size_t) width,sizeof(*kernel));
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

MagickExport Image *MotionBlurImage(const Image *image,
  const double radius,const double sigma,const double angle,
  ExceptionInfo *exception)
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

  PixelInfo
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
  GetPixelInfo(image,&bias);
  image_view=AcquireCacheView(image);
  blur_view=AcquireCacheView(blur_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status) omp_throttle(1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      PixelInfo
        qixel;

      PixelPacket
        pixel;

      register double
        *restrict k;

      register ssize_t
        i;

      k=kernel;
      qixel=bias;
      if (((GetPixelAlphaTraits(image) & ActivePixelTrait) == 0) || (image->matte == MagickFalse))
        {
          for (i=0; i < (ssize_t) width; i++)
          {
            (void) GetOneCacheViewVirtualPixel(image_view,x+offset[i].x,y+
              offset[i].y,&pixel,exception);
            qixel.red+=(*k)*pixel.red;
            qixel.green+=(*k)*pixel.green;
            qixel.blue+=(*k)*pixel.blue;
            qixel.alpha+=(*k)*pixel.alpha;
            if (image->colorspace == CMYKColorspace)
              qixel.black+=(*k)*pixel.black;
            k++;
          }
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            SetPixelRed(blur_image,
              ClampToQuantum(qixel.red),q);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            SetPixelGreen(blur_image,
              ClampToQuantum(qixel.green),q);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            SetPixelBlue(blur_image,
              ClampToQuantum(qixel.blue),q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (image->colorspace == CMYKColorspace))
            SetPixelBlack(blur_image,
              ClampToQuantum(qixel.black),q);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            SetPixelAlpha(blur_image,
              ClampToQuantum(qixel.alpha),q);
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
            alpha=(MagickRealType) (QuantumScale*pixel.alpha);
            qixel.red+=(*k)*alpha*pixel.red;
            qixel.green+=(*k)*alpha*pixel.green;
            qixel.blue+=(*k)*alpha*pixel.blue;
            qixel.alpha+=(*k)*pixel.alpha;
            if (image->colorspace == CMYKColorspace)
              qixel.black+=(*k)*alpha*pixel.black;
            gamma+=(*k)*alpha;
            k++;
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            SetPixelRed(blur_image,
              ClampToQuantum(gamma*qixel.red),q);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            SetPixelGreen(blur_image,
              ClampToQuantum(gamma*qixel.green),q);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            SetPixelBlue(blur_image,
              ClampToQuantum(gamma*qixel.blue),q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (image->colorspace == CMYKColorspace))
            SetPixelBlack(blur_image,
              ClampToQuantum(gamma*qixel.black),q);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            SetPixelAlpha(blur_image,
              ClampToQuantum(qixel.alpha),q);
        }
      q+=GetPixelComponents(blur_image);
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
        (void) GammaImage(preview_image,gamma);
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
        (void) SegmentImage(preview_image,RGBColorspace,MagickFalse,threshold,
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
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o angle: the angle of the radial blur.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *RadialBlurImage(const Image *image,
  const double angle,ExceptionInfo *exception)
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

  PixelInfo
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
  GetPixelInfo(image,&bias);
  image_view=AcquireCacheView(image);
  blur_view=AcquireCacheView(blur_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) blur_image->rows; y++)
  {
    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) blur_image->columns; x++)
    {
      PixelInfo
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
      if (((GetPixelAlphaTraits(image) & ActivePixelTrait) == 0) || (image->matte == MagickFalse))
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
            if (image->colorspace == CMYKColorspace)
              qixel.black+=pixel.black;
            qixel.alpha+=pixel.alpha;
            normalize+=1.0;
          }
          normalize=1.0/(fabs((double) normalize) <= MagickEpsilon ? 1.0 :
            normalize);
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            SetPixelRed(blur_image,
              ClampToQuantum(normalize*qixel.red),q);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            SetPixelGreen(blur_image,
              ClampToQuantum(normalize*qixel.green),q);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            SetPixelBlue(blur_image,
              ClampToQuantum(normalize*qixel.blue),q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (image->colorspace == CMYKColorspace))
            SetPixelBlack(blur_image,
              ClampToQuantum(normalize*qixel.black),q);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            SetPixelAlpha(blur_image,
              ClampToQuantum(normalize*qixel.alpha),q);
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
            alpha=(MagickRealType) (QuantumScale*pixel.alpha);
            qixel.red+=alpha*pixel.red;
            qixel.green+=alpha*pixel.green;
            qixel.blue+=alpha*pixel.blue;
            qixel.alpha+=pixel.alpha;
            if (image->colorspace == CMYKColorspace)
              qixel.black+=alpha*pixel.black;
            gamma+=alpha;
            normalize+=1.0;
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          normalize=1.0/(fabs((double) normalize) <= MagickEpsilon ? 1.0 :
            normalize);
          if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
            SetPixelRed(blur_image,
              ClampToQuantum(gamma*qixel.red),q);
          if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
            SetPixelGreen(blur_image,
              ClampToQuantum(gamma*qixel.green),q);
          if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
            SetPixelBlue(blur_image,
              ClampToQuantum(gamma*qixel.blue),q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (image->colorspace == CMYKColorspace))
            SetPixelBlack(blur_image,
              ClampToQuantum(gamma*qixel.black),q);
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            SetPixelAlpha(blur_image,
              ClampToQuantum(normalize*qixel.alpha),q);
        }
      q+=GetPixelComponents(blur_image);
    }
    if (SyncCacheViewAuthenticPixels(blur_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_RadialBlurImage)
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
MagickExport Image *SelectiveBlurImage(const Image *image,
  const double radius,const double sigma,const double threshold,
  ExceptionInfo *exception)
{
#define SelectiveBlurImageTag  "SelectiveBlur/Image"

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

  PixelInfo
    bias;

  register ssize_t
    i;

  size_t
    width;

  ssize_t
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
  kernel=(double *) AcquireQuantumMemory((size_t) width,width*sizeof(*kernel));
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
  GetPixelInfo(image,&bias);
  SetPixelInfoBias(image,&bias);
  image_view=AcquireCacheView(image);
  blur_view=AcquireCacheView(blur_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      contrast;

    MagickBooleanType
      sync;

    MagickRealType
      gamma;

    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) width/2L),y-(ssize_t)
      (width/2L),image->columns+width,width,exception);
    q=GetCacheViewAuthenticPixels(blur_view,0,y,blur_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      PixelInfo
        pixel;

      register const double
        *restrict k;

      register ssize_t
        u;

      ssize_t
        j,
        v;

      pixel=bias;
      k=kernel;
      gamma=0.0;
      j=0;
      if (((GetPixelAlphaTraits(image) & ActivePixelTrait) == 0) || (image->matte == MagickFalse))
        {
          for (v=0; v < (ssize_t) width; v++)
          {
            for (u=0; u < (ssize_t) width; u++)
            {
              contrast=GetPixelIntensity(image,p+(u+j)*GetPixelComponents(image))-
                (double) GetPixelIntensity(blur_image,q);
              if (fabs(contrast) < threshold)
                {
                  pixel.red+=(*k)*
                    GetPixelRed(image,p+(u+j)*GetPixelComponents(image));
                  pixel.green+=(*k)*
                    GetPixelGreen(image,p+(u+j)*GetPixelComponents(image));
                  pixel.blue+=(*k)*
                    GetPixelBlue(image,p+(u+j)*GetPixelComponents(image));
                  if (image->colorspace == CMYKColorspace)
                    pixel.black+=(*k)*
                      GetPixelBlack(image,p+(u+j)*GetPixelComponents(image));
                  gamma+=(*k);
                  k++;
                }
            }
            j+=(ssize_t) (image->columns+width);
          }
          if (gamma != 0.0)
            {
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
              if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
                SetPixelRed(blur_image,ClampToQuantum(gamma*pixel.red),q);
              if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
                SetPixelGreen(blur_image,ClampToQuantum(gamma*pixel.green),q);
              if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
                SetPixelBlue(blur_image,ClampToQuantum(gamma*pixel.blue),q);
              if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
                  (image->colorspace == CMYKColorspace))
                SetPixelBlack(blur_image,ClampToQuantum(gamma*pixel.black),q);
            }
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            {
              gamma=0.0;
              j=0;
              for (v=0; v < (ssize_t) width; v++)
              {
                for (u=0; u < (ssize_t) width; u++)
                {
                  contrast=GetPixelIntensity(image,p+(u+j)*
                    GetPixelComponents(image))-(double)
                    GetPixelIntensity(blur_image,q);
                  if (fabs(contrast) < threshold)
                    {
                      pixel.alpha+=(*k)*
                        GetPixelAlpha(image,p+(u+j)*GetPixelComponents(image));
                      gamma+=(*k);
                      k++;
                    }
                }
                j+=(ssize_t) (image->columns+width);
              }
              if (gamma != 0.0)
                {
                  gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 :
                    gamma);
                  SetPixelAlpha(blur_image,ClampToQuantum(gamma*pixel.alpha),q);
                }
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
              contrast=GetPixelIntensity(image,p+(u+j)*
                GetPixelComponents(image))-(double)
                GetPixelIntensity(blur_image,q);
              if (fabs(contrast) < threshold)
                {
                  alpha=(MagickRealType) (QuantumScale*
                    GetPixelAlpha(image,p+(u+j)*GetPixelComponents(image)));
                  pixel.red+=(*k)*alpha*
                    GetPixelRed(image,p+(u+j)*GetPixelComponents(image));
                  pixel.green+=(*k)*alpha*GetPixelGreen(image,p+(u+j)*
                    GetPixelComponents(image));
                  pixel.blue+=(*k)*alpha*GetPixelBlue(image,p+(u+j)*
                    GetPixelComponents(image));
                  pixel.alpha+=(*k)*GetPixelAlpha(image,p+(u+j)*
                    GetPixelComponents(image));
                  if (image->colorspace == CMYKColorspace)
                    pixel.black+=(*k)*GetPixelBlack(image,p+(u+j)*
                      GetPixelComponents(image));
                  gamma+=(*k)*alpha;
                  k++;
                }
            }
            j+=(ssize_t) (image->columns+width);
          }
          if (gamma != 0.0)
            {
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
              if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
                SetPixelRed(blur_image,ClampToQuantum(gamma*pixel.red),q);
              if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
                SetPixelGreen(blur_image,ClampToQuantum(gamma*pixel.green),q);
              if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
                SetPixelBlue(blur_image,ClampToQuantum(gamma*pixel.blue),q);
              if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
                  (image->colorspace == CMYKColorspace))
                SetPixelBlack(blur_image,ClampToQuantum(gamma*pixel.black),q);
            }
          if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
            {
              gamma=0.0;
              j=0;
              for (v=0; v < (ssize_t) width; v++)
              {
                for (u=0; u < (ssize_t) width; u++)
                {
                  contrast=GetPixelIntensity(image,p+(u+j)*
                    GetPixelComponents(image))-(double)
                    GetPixelIntensity(blur_image,q);
                  if (fabs(contrast) < threshold)
                    {
                      pixel.alpha+=(*k)*
                        GetPixelAlpha(image,p+(u+j)*GetPixelComponents(image));
                      gamma+=(*k);
                      k++;
                    }
                }
                j+=(ssize_t) (image->columns+width);
              }
              if (gamma != 0.0)
                {
                  gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 :
                    gamma);
                  SetPixelAlpha(blur_image,ClampToQuantum(pixel.alpha),q);
                }
            }
        }
      p+=GetPixelComponents(image);
      q+=GetPixelComponents(blur_image);
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
  image_view=AcquireCacheView(image);
  shade_view=AcquireCacheView(shade_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickRealType
      distance,
      normal_distance,
      shade;

    PrimaryInfo
      normal;

    register const Quantum
      *restrict p,
      *restrict s0,
      *restrict s1,
      *restrict s2;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-1,y-1,image->columns+2,3,exception);
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
    s0=p+GetPixelComponents(image);
    s1=s0+(image->columns+2)*GetPixelComponents(image);
    s2=s1+(image->columns+2)*GetPixelComponents(image);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      /*
        Determine the surface normal and compute shading.
      */
      normal.x=(double) (GetPixelIntensity(image,s0-GetPixelComponents(image))+
        GetPixelIntensity(image,s1-GetPixelComponents(image))+
        GetPixelIntensity(image,s2-GetPixelComponents(image))-
        GetPixelIntensity(image,s0+GetPixelComponents(image))-
        GetPixelIntensity(image,s1+GetPixelComponents(image))-
        GetPixelIntensity(image,s2+GetPixelComponents(image)));
      normal.y=(double) (GetPixelIntensity(image,s2-GetPixelComponents(image))+
        GetPixelIntensity(image,s2)+
        GetPixelIntensity(image,s2+GetPixelComponents(image))-
        GetPixelIntensity(image,s0-GetPixelComponents(image))-
        GetPixelIntensity(image,s0)-
        GetPixelIntensity(image,s0+GetPixelComponents(image)));
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
          SetPixelRed(shade_image,ClampToQuantum(shade),q);
          SetPixelGreen(shade_image,ClampToQuantum(shade),q);
          SetPixelBlue(shade_image,ClampToQuantum(shade),q);
        }
      else
        {
          SetPixelRed(shade_image,ClampToQuantum(QuantumScale*shade*
            GetPixelRed(image,s1)),q);
          SetPixelGreen(shade_image,ClampToQuantum(QuantumScale*shade*
            GetPixelGreen(image,s1)),q);
          SetPixelBlue(shade_image,ClampToQuantum(QuantumScale*shade*
            GetPixelBlue(image,s1)),q);
        }
      SetPixelAlpha(shade_image,GetPixelAlpha(image,s1),q);
      s0+=GetPixelComponents(image);
      s1+=GetPixelComponents(image);
      s2+=GetPixelComponents(image);
      q+=GetPixelComponents(shade_image);
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
  kernel=(double *) AcquireQuantumMemory((size_t) width*width,sizeof(*kernel));
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
  sharp_image=ConvolveImage(image,width,kernel,exception);
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
    *image_view,
    *spread_view;

  Image
    *spread_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    bias;

  RandomInfo
    **restrict random_info;

  size_t
    width;

  ssize_t
    y;

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
  GetPixelInfo(spread_image,&bias);
  width=GetOptimalKernelWidth1D(radius,0.5);
  random_info=AcquireRandomInfoThreadSet();
  image_view=AcquireCacheView(image);
  spread_view=AcquireCacheView(spread_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status) omp_throttle(1)
#endif
  for (y=0; y < (ssize_t) spread_image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    PixelInfo
      pixel;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(spread_view,0,y,spread_image->columns,1,
      exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    pixel=bias;
    for (x=0; x < (ssize_t) spread_image->columns; x++)
    {
      (void) InterpolatePixelInfo(image,image_view,
        UndefinedInterpolatePixel,(double) x+width*(GetPseudoRandomValue(
        random_info[id])-0.5),(double) y+width*(GetPseudoRandomValue(
        random_info[id])-0.5),&pixel,exception);
      SetPixelPixelInfo(spread_image,&pixel,q);
      q+=GetPixelComponents(spread_image);
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
  return(spread_image);
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

#define ListChannels  5

typedef struct _ListNode
{
  size_t
    next[9],
    count,
    signature;
} ListNode;

typedef struct _SkipList
{
  ssize_t
    level;

  ListNode
    *nodes;
} SkipList;

typedef struct _PixelList
{
  size_t
    length,
    seed,
    signature;

  SkipList
    lists[ListChannels];
} PixelList;

static PixelList *DestroyPixelList(PixelList *pixel_list)
{
  register ssize_t
    i;

  if (pixel_list == (PixelList *) NULL)
    return((PixelList *) NULL);
  for (i=0; i < ListChannels; i++)
    if (pixel_list->lists[i].nodes != (ListNode *) NULL)
      pixel_list->lists[i].nodes=(ListNode *) RelinquishMagickMemory(
        pixel_list->lists[i].nodes);
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

  register ssize_t
    i;

  pixel_list=(PixelList *) AcquireMagickMemory(sizeof(*pixel_list));
  if (pixel_list == (PixelList *) NULL)
    return(pixel_list);
  (void) ResetMagickMemory((void *) pixel_list,0,sizeof(*pixel_list));
  pixel_list->length=width*height;
  for (i=0; i < ListChannels; i++)
  {
    pixel_list->lists[i].nodes=(ListNode *) AcquireQuantumMemory(65537UL,
      sizeof(*pixel_list->lists[i].nodes));
    if (pixel_list->lists[i].nodes == (ListNode *) NULL)
      return(DestroyPixelList(pixel_list));
    (void) ResetMagickMemory(pixel_list->lists[i].nodes,0,65537UL*
      sizeof(*pixel_list->lists[i].nodes));
  }
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

static void AddNodePixelList(PixelList *pixel_list,const ssize_t channel,
  const size_t color)
{
  register SkipList
    *list;

  register ssize_t
    level;

  size_t
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
  } while (level-- > 0);
}

static PixelInfo GetMaximumPixelList(PixelList *pixel_list)
{
  PixelInfo
    pixel;

  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color,
    maximum;

  ssize_t
    count;

  unsigned short
    channels[ListChannels];

  /*
    Find the maximum value for each of the color.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536L;
    count=0;
    maximum=list->nodes[color].next[0];
    do
    {
      color=list->nodes[color].next[0];
      if (color > maximum)
        maximum=color;
      count+=list->nodes[color].count;
    } while (count < (ssize_t) pixel_list->length);
    channels[channel]=(unsigned short) maximum;
  }
  GetPixelInfo((const Image *) NULL,&pixel);
  pixel.red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel.green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel.blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel.alpha=(MagickRealType) ScaleShortToQuantum(channels[3]);
  pixel.black=(MagickRealType) ScaleShortToQuantum(channels[4]);
  return(pixel);
}

static PixelInfo GetMeanPixelList(PixelList *pixel_list)
{
  PixelInfo
    pixel;

  MagickRealType
    sum;

  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color;

  ssize_t
    count;

  unsigned short
    channels[ListChannels];

  /*
    Find the mean value for each of the color.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536L;
    count=0;
    sum=0.0;
    do
    {
      color=list->nodes[color].next[0];
      sum+=(MagickRealType) list->nodes[color].count*color;
      count+=list->nodes[color].count;
    } while (count < (ssize_t) pixel_list->length);
    sum/=pixel_list->length;
    channels[channel]=(unsigned short) sum;
  }
  GetPixelInfo((const Image *) NULL,&pixel);
  pixel.red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel.green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel.blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel.black=(MagickRealType) ScaleShortToQuantum(channels[4]);
  pixel.alpha=(MagickRealType) ScaleShortToQuantum(channels[3]);
  return(pixel);
}

static PixelInfo GetMedianPixelList(PixelList *pixel_list)
{
  PixelInfo
    pixel;

  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color;

  ssize_t
    count;

  unsigned short
    channels[ListChannels];

  /*
    Find the median value for each of the color.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536L;
    count=0;
    do
    {
      color=list->nodes[color].next[0];
      count+=list->nodes[color].count;
    } while (count <= (ssize_t) (pixel_list->length >> 1));
    channels[channel]=(unsigned short) color;
  }
  GetPixelInfo((const Image *) NULL,&pixel);
  pixel.red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel.green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel.blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel.black=(MagickRealType) ScaleShortToQuantum(channels[4]);
  pixel.alpha=(MagickRealType) ScaleShortToQuantum(channels[3]);
  return(pixel);
}

static PixelInfo GetMinimumPixelList(PixelList *pixel_list)
{
  PixelInfo
    pixel;

  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color,
    minimum;

  ssize_t
    count;

  unsigned short
    channels[ListChannels];

  /*
    Find the minimum value for each of the color.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    count=0;
    color=65536UL;
    minimum=list->nodes[color].next[0];
    do
    {
      color=list->nodes[color].next[0];
      if (color < minimum)
        minimum=color;
      count+=list->nodes[color].count;
    } while (count < (ssize_t) pixel_list->length);
    channels[channel]=(unsigned short) minimum;
  }
  GetPixelInfo((const Image *) NULL,&pixel);
  pixel.red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel.green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel.blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel.black=(MagickRealType) ScaleShortToQuantum(channels[4]);
  pixel.alpha=(MagickRealType) ScaleShortToQuantum(channels[3]);
  return(pixel);
}

static PixelInfo GetModePixelList(PixelList *pixel_list)
{
  PixelInfo
    pixel;

  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color,
    max_count,
    mode;

  ssize_t
    count;

  unsigned short
    channels[5];

  /*
    Make each pixel the 'predominant color' of the specified neighborhood.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536L;
    mode=color;
    max_count=list->nodes[mode].count;
    count=0;
    do
    {
      color=list->nodes[color].next[0];
      if (list->nodes[color].count > max_count)
        {
          mode=color;
          max_count=list->nodes[mode].count;
        }
      count+=list->nodes[color].count;
    } while (count < (ssize_t) pixel_list->length);
    channels[channel]=(unsigned short) mode;
  }
  GetPixelInfo((const Image *) NULL,&pixel);
  pixel.red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel.green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel.blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel.black=(MagickRealType) ScaleShortToQuantum(channels[4]);
  pixel.alpha=(MagickRealType) ScaleShortToQuantum(channels[3]);
  return(pixel);
}

static PixelInfo GetNonpeakPixelList(PixelList *pixel_list)
{
  PixelInfo
    pixel;

  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color,
    next,
    previous;

  ssize_t
    count;

  unsigned short
    channels[5];

  /*
    Finds the non peak value for each of the colors.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536L;
    next=list->nodes[color].next[0];
    count=0;
    do
    {
      previous=color;
      color=next;
      next=list->nodes[color].next[0];
      count+=list->nodes[color].count;
    } while (count <= (ssize_t) (pixel_list->length >> 1));
    if ((previous == 65536UL) && (next != 65536UL))
      color=next;
    else
      if ((previous != 65536UL) && (next == 65536UL))
        color=previous;
    channels[channel]=(unsigned short) color;
  }
  GetPixelInfo((const Image *) NULL,&pixel);
  pixel.red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel.green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel.blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel.alpha=(MagickRealType) ScaleShortToQuantum(channels[3]);
  pixel.black=(MagickRealType) ScaleShortToQuantum(channels[4]);
  return(pixel);
}

static PixelInfo GetStandardDeviationPixelList(PixelList *pixel_list)
{
  PixelInfo
    pixel;

  MagickRealType
    sum,
    sum_squared;

  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color;

  ssize_t
    count;

  unsigned short
    channels[ListChannels];

  /*
    Find the standard-deviation value for each of the color.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536L;
    count=0;
    sum=0.0;
    sum_squared=0.0;
    do
    {
      register ssize_t
        i;

      color=list->nodes[color].next[0];
      sum+=(MagickRealType) list->nodes[color].count*color;
      for (i=0; i < (ssize_t) list->nodes[color].count; i++)
        sum_squared+=((MagickRealType) color)*((MagickRealType) color);
      count+=list->nodes[color].count;
    } while (count < (ssize_t) pixel_list->length);
    sum/=pixel_list->length;
    sum_squared/=pixel_list->length;
    channels[channel]=(unsigned short) sqrt(sum_squared-(sum*sum));
  }
  GetPixelInfo((const Image *) NULL,&pixel);
  pixel.red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel.green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel.blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel.alpha=(MagickRealType) ScaleShortToQuantum(channels[3]);
  pixel.black=(MagickRealType) ScaleShortToQuantum(channels[4]);
  return(pixel);
}

static inline void InsertPixelList(const Image *image,const Quantum *pixel,
  PixelList *pixel_list)
{
  size_t
    signature;

  unsigned short
    index;

  index=ScaleQuantumToShort(GetPixelRed(image,pixel));
  signature=pixel_list->lists[0].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[0].nodes[index].count++;
  else
    AddNodePixelList(pixel_list,0,index);
  index=ScaleQuantumToShort(GetPixelGreen(image,pixel));
  signature=pixel_list->lists[1].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[1].nodes[index].count++;
  else
    AddNodePixelList(pixel_list,1,index);
  index=ScaleQuantumToShort(GetPixelBlue(image,pixel));
  signature=pixel_list->lists[2].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[2].nodes[index].count++;
  else
    AddNodePixelList(pixel_list,2,index);
  index=ScaleQuantumToShort(GetPixelAlpha(image,pixel));
  signature=pixel_list->lists[3].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[3].nodes[index].count++;
  else
    AddNodePixelList(pixel_list,3,index);
  if (image->colorspace == CMYKColorspace)
    index=ScaleQuantumToShort(GetPixelBlack(image,pixel));
  signature=pixel_list->lists[4].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[4].nodes[index].count++;
  else
    AddNodePixelList(pixel_list,4,index);
}

static inline MagickRealType MagickAbsoluteValue(const MagickRealType x)
{
  if (x < 0)
    return(-x);
  return(x);
}

static void ResetPixelList(PixelList *pixel_list)
{
  int
    level;

  register ListNode
    *root;

  register SkipList
    *list;

  register ssize_t
    channel;

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

MagickExport Image *StatisticImage(const Image *image,const StatisticType type,
  const size_t width,const size_t height,ExceptionInfo *exception)
{
#define StatisticWidth \
  (width == 0 ? GetOptimalKernelWidth2D((double) width,0.5) : width)
#define StatisticHeight \
  (height == 0 ? GetOptimalKernelWidth2D((double) height,0.5) : height)
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
  if (SetImageStorageClass(statistic_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&statistic_image->exception);
      statistic_image=DestroyImage(statistic_image);
      return((Image *) NULL);
    }
  pixel_list=AcquirePixelListThreadSet(StatisticWidth,StatisticHeight);
  if (pixel_list == (PixelList **) NULL)
    {
      statistic_image=DestroyImage(statistic_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Make each pixel the min / max / median / mode / etc. of the neighborhood.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  statistic_view=AcquireCacheView(statistic_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
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
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) StatisticWidth/2L),y-
      (ssize_t) (StatisticHeight/2L),image->columns+StatisticWidth,
      StatisticHeight,exception);
    q=QueueCacheViewAuthenticPixels(statistic_view,0,y,statistic_image->columns,      1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) statistic_image->columns; x++)
    {
      PixelInfo
        pixel;

      register const Quantum
        *restrict r;

      register ssize_t
        u,
        v;

      r=p;
      ResetPixelList(pixel_list[id]);
      for (v=0; v < (ssize_t) StatisticHeight; v++)
      {
        for (u=0; u < (ssize_t) StatisticWidth; u++)
          InsertPixelList(image,r+u*GetPixelComponents(image),pixel_list[id]);
        r+=(image->columns+StatisticWidth)*GetPixelComponents(image);
      }
      GetPixelInfo(image,&pixel);
      SetPixelInfo(image,p+(StatisticWidth*StatisticHeight/2)*
        GetPixelComponents(image),&pixel);
      switch (type)
      {
        case GradientStatistic:
        {
          PixelInfo
            maximum,
            minimum;

          minimum=GetMinimumPixelList(pixel_list[id]);
          maximum=GetMaximumPixelList(pixel_list[id]);
          pixel.red=MagickAbsoluteValue(maximum.red-minimum.red);
          pixel.green=MagickAbsoluteValue(maximum.green-minimum.green);
          pixel.blue=MagickAbsoluteValue(maximum.blue-minimum.blue);
          pixel.alpha=MagickAbsoluteValue(maximum.alpha-minimum.alpha);
          if (image->colorspace == CMYKColorspace)
            pixel.black=MagickAbsoluteValue(maximum.black-minimum.black);
          break;
        }
        case MaximumStatistic:
        {
          pixel=GetMaximumPixelList(pixel_list[id]);
          break;
        }
        case MeanStatistic:
        {
          pixel=GetMeanPixelList(pixel_list[id]);
          break;
        }
        case MedianStatistic:
        default:
        {
          pixel=GetMedianPixelList(pixel_list[id]);
          break;
        }
        case MinimumStatistic:
        {
          pixel=GetMinimumPixelList(pixel_list[id]);
          break;
        }
        case ModeStatistic:
        {
          pixel=GetModePixelList(pixel_list[id]);
          break;
        }
        case NonpeakStatistic:
        {
          pixel=GetNonpeakPixelList(pixel_list[id]);
          break;
        }
        case StandardDeviationStatistic:
        {
          pixel=GetStandardDeviationPixelList(pixel_list[id]);
          break;
        }
      }
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        SetPixelRed(statistic_image,ClampToQuantum(pixel.red),q);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        SetPixelGreen(statistic_image,ClampToQuantum(pixel.green),q);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        SetPixelBlue(statistic_image,ClampToQuantum(pixel.blue),q);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelBlack(statistic_image,ClampToQuantum(pixel.black),q);
      if (((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0) &&
          (image->matte != MagickFalse))
        SetPixelAlpha(statistic_image,ClampToQuantum(pixel.alpha),q);
      p+=GetPixelComponents(image);
      q+=GetPixelComponents(statistic_image);
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
%    o amount: the percentage of the difference between the original and the
%      blur image that is added back into the original.
%
%    o threshold: the threshold in pixels needed to apply the diffence amount.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *UnsharpMaskImage(const Image *image,
  const double radius,const double sigma,const double amount,
  const double threshold,ExceptionInfo *exception)
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

  PixelInfo
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
  unsharp_image=BlurImage(image,radius,sigma,exception);
  if (unsharp_image == (Image *) NULL)
    return((Image *) NULL);
  quantum_threshold=(MagickRealType) QuantumRange*threshold;
  /*
    Unsharp-mask image.
  */
  status=MagickTrue;
  progress=0;
  GetPixelInfo(image,&bias);
  image_view=AcquireCacheView(image);
  unsharp_view=AcquireCacheView(unsharp_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    PixelInfo
      pixel;

    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewAuthenticPixels(unsharp_view,0,y,unsharp_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    pixel=bias;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        {
          pixel.red=GetPixelRed(image,p)-(MagickRealType) GetPixelRed(image,q);
          if (fabs(2.0*pixel.red) < quantum_threshold)
            pixel.red=(MagickRealType) GetPixelRed(image,p);
          else
            pixel.red=(MagickRealType) GetPixelRed(image,p)+(pixel.red*amount);
          SetPixelRed(unsharp_image,ClampToQuantum(pixel.red),q);
        }
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        {
          pixel.green=GetPixelGreen(image,p)-
            (MagickRealType) GetPixelGreen(image,q);
          if (fabs(2.0*pixel.green) < quantum_threshold)
            pixel.green=(MagickRealType)
              GetPixelGreen(image,p);
          else
            pixel.green=(MagickRealType)
              GetPixelGreen(image,p)+
              (pixel.green*amount);
          SetPixelGreen(unsharp_image,
            ClampToQuantum(pixel.green),q);
        }
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        {
          pixel.blue=GetPixelBlue(image,p)-
            (MagickRealType) GetPixelBlue(image,q);
          if (fabs(2.0*pixel.blue) < quantum_threshold)
            pixel.blue=(MagickRealType)
              GetPixelBlue(image,p);
          else
            pixel.blue=(MagickRealType)
              GetPixelBlue(image,p)+(pixel.blue*amount);
          SetPixelBlue(unsharp_image,
            ClampToQuantum(pixel.blue),q);
        }
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          pixel.black=GetPixelBlack(image,p)-
            (MagickRealType) GetPixelBlack(image,q);
          if (fabs(2.0*pixel.black) < quantum_threshold)
            pixel.black=(MagickRealType)
              GetPixelBlack(image,p);
          else
            pixel.black=(MagickRealType)
              GetPixelBlack(image,p)+(pixel.black*
              amount);
          SetPixelBlack(unsharp_image,
            ClampToQuantum(pixel.black),q);
        }
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        {
          pixel.alpha=GetPixelAlpha(image,p)-
            (MagickRealType) GetPixelAlpha(image,q);
          if (fabs(2.0*pixel.alpha) < quantum_threshold)
            pixel.alpha=(MagickRealType)
              GetPixelAlpha(image,p);
          else
            pixel.alpha=GetPixelAlpha(image,p)+
              (pixel.alpha*amount);
          SetPixelAlpha(unsharp_image,
            ClampToQuantum(pixel.alpha),q);
        }
      p+=GetPixelComponents(image);
      q+=GetPixelComponents(unsharp_image);
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                   V   V  IIIII  SSSSS  U   U   AAA   L                      %
%                   V   V    I    SS     U   U  A   A  L                      %
%                   V   V    I     SSS   U   U  AAAAA  L                      %
%                    V V     I       SS  U   U  A   A  L                      %
%                     V    IIIII  SSSSS   UUU   A   A  LLLLL                  %
%                                                                             %
%                EEEEE  FFFFF  FFFFF  EEEEE  CCCC  TTTTT  SSSSS               %
%                E      F      F      E     C        T    SS                  %
%                EEE    FFF    FFF    EEE   C        T     SSS                %
%                E      F      F      E     C        T       SS               %
%                EEEEE  F      F      EEEEE  CCCC    T    SSSSS               %
%                                                                             %
%                                                                             %
%                   MagickCore Image Special Effects Methods                  %
%                                                                             %
%                               Software Design                               %
%                                    Cristy                                   %
%                                 October 1996                                %
%                                                                             %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/annotate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/channel.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/decorate.h"
#include "MagickCore/distort.h"
#include "MagickCore/draw.h"
#include "MagickCore/effect.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/gem-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/layer.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/random_.h"
#include "MagickCore/random-private.h"
#include "MagickCore/resample.h"
#include "MagickCore/resample-private.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/threshold.h"
#include "MagickCore/transform.h"
#include "MagickCore/transform-private.h"
#include "MagickCore/utility.h"
#include "MagickCore/visual-effects.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A d d N o i s e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AddNoiseImage() adds random noise to the image.
%
%  The format of the AddNoiseImage method is:
%
%      Image *AddNoiseImage(const Image *image,const NoiseType noise_type,
%        const double attenuate,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o noise_type:  The type of noise: Uniform, Gaussian, Multiplicative,
%      Impulse, Laplacian, or Poisson.
%
%    o attenuate:  attenuate the random distribution.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *AddNoiseImage(const Image *image,const NoiseType noise_type,
  const double attenuate,ExceptionInfo *exception)
{
#define AddNoiseImageTag  "AddNoise/Image"

  CacheView
    *image_view,
    *noise_view;

  Image
    *noise_image;

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

  /*
    Initialize noise image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  noise_image=AccelerateAddNoiseImage(image,noise_type,attenuate,exception);
  if (noise_image != (Image *) NULL)
    return(noise_image);
#endif
  noise_image=CloneImage(image,0,0,MagickTrue,exception);
  if (noise_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(noise_image,DirectClass,exception) == MagickFalse)
    {
      noise_image=DestroyImage(noise_image);
      return((Image *) NULL);
    }
  /*
    Add noise in each row.
  */
  status=MagickTrue;
  progress=0;
  random_info=AcquireRandomInfoThreadSet();
  image_view=AcquireVirtualCacheView(image,exception);
  noise_view=AcquireAuthenticCacheView(noise_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  key=GetRandomSecretKey(random_info[0]);
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,noise_image,image->rows,key == ~0UL)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    MagickBooleanType
      sync;

    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    register Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(noise_view,0,y,noise_image->columns,1,
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
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait noise_traits=GetPixelChannelTraits(noise_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (noise_traits == UndefinedPixelTrait))
          continue;
        if ((noise_traits & CopyPixelTrait) != 0)
          {
            SetPixelChannel(noise_image,channel,p[i],q);
            continue;
          }
        SetPixelChannel(noise_image,channel,ClampToQuantum(
          GenerateDifferentialNoise(random_info[id],p[i],noise_type,attenuate)),
          q);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(noise_image);
    }
    sync=SyncCacheViewAuthenticPixels(noise_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,AddNoiseImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  noise_view=DestroyCacheView(noise_view);
  image_view=DestroyCacheView(image_view);
  random_info=DestroyRandomInfoThreadSet(random_info);
  if (status == MagickFalse)
    noise_image=DestroyImage(noise_image);
  return(noise_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     B l u e S h i f t I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BlueShiftImage() mutes the colors of the image to simulate a scene at
%  nighttime in the moonlight.
%
%  The format of the BlueShiftImage method is:
%
%      Image *BlueShiftImage(const Image *image,const double factor,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o factor: the shift factor.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *BlueShiftImage(const Image *image,const double factor,
  ExceptionInfo *exception)
{
#define BlueShiftImageTag  "BlueShift/Image"

  CacheView
    *image_view,
    *shift_view;

  Image
    *shift_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  /*
    Allocate blue shift image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  shift_image=CloneImage(image,0,0,MagickTrue,exception);
  if (shift_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(shift_image,DirectClass,exception) == MagickFalse)
    {
      shift_image=DestroyImage(shift_image);
      return((Image *) NULL);
    }
  /*
    Blue-shift DirectClass image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  shift_view=AcquireAuthenticCacheView(shift_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,shift_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      sync;

    PixelInfo
      pixel;

    Quantum
      quantum;

    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    register Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(shift_view,0,y,shift_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      quantum=GetPixelRed(image,p);
      if (GetPixelGreen(image,p) < quantum)
        quantum=GetPixelGreen(image,p);
      if (GetPixelBlue(image,p) < quantum)
        quantum=GetPixelBlue(image,p);
      pixel.red=0.5*(GetPixelRed(image,p)+factor*quantum);
      pixel.green=0.5*(GetPixelGreen(image,p)+factor*quantum);
      pixel.blue=0.5*(GetPixelBlue(image,p)+factor*quantum);
      quantum=GetPixelRed(image,p);
      if (GetPixelGreen(image,p) > quantum)
        quantum=GetPixelGreen(image,p);
      if (GetPixelBlue(image,p) > quantum)
        quantum=GetPixelBlue(image,p);
      pixel.red=0.5*(pixel.red+factor*quantum);
      pixel.green=0.5*(pixel.green+factor*quantum);
      pixel.blue=0.5*(pixel.blue+factor*quantum);
      SetPixelRed(shift_image,ClampToQuantum(pixel.red),q);
      SetPixelGreen(shift_image,ClampToQuantum(pixel.green),q);
      SetPixelBlue(shift_image,ClampToQuantum(pixel.blue),q);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(shift_image);
    }
    sync=SyncCacheViewAuthenticPixels(shift_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,BlueShiftImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  shift_view=DestroyCacheView(shift_view);
  if (status == MagickFalse)
    shift_image=DestroyImage(shift_image);
  return(shift_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C h a r c o a l I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CharcoalImage() creates a new image that is a copy of an existing one with
%  the edge highlighted.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the CharcoalImage method is:
%
%      Image *CharcoalImage(const Image *image,const double radius,
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
MagickExport Image *CharcoalImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
{
  Image
    *charcoal_image,
    *edge_image;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  edge_image=EdgeImage(image,radius,exception);
  if (edge_image == (Image *) NULL)
    return((Image *) NULL);
  edge_image->alpha_trait=UndefinedPixelTrait;
  charcoal_image=(Image *) NULL;
  status=ClampImage(edge_image,exception);
  if (status != MagickFalse)
    charcoal_image=BlurImage(edge_image,radius,sigma,exception);
  edge_image=DestroyImage(edge_image);
  if (charcoal_image == (Image *) NULL)
    return((Image *) NULL);
  status=NormalizeImage(charcoal_image,exception);
  if (status != MagickFalse)
    status=NegateImage(charcoal_image,MagickFalse,exception);
  if (status != MagickFalse)
    status=GrayscaleImage(charcoal_image,image->intensity,exception);
  if (status == MagickFalse)
    charcoal_image=DestroyImage(charcoal_image);
  return(charcoal_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o l o r i z e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ColorizeImage() blends the fill color with each pixel in the image.
%  A percentage blend is specified with opacity.  Control the application
%  of different color components by specifying a different percentage for
%  each component (e.g. 90/100/10 is 90% red, 100% green, and 10% blue).
%
%  The format of the ColorizeImage method is:
%
%      Image *ColorizeImage(const Image *image,const char *blend,
%        const PixelInfo *colorize,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o blend:  A character string indicating the level of blending as a
%      percentage.
%
%    o colorize: A color value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ColorizeImage(const Image *image,const char *blend,
  const PixelInfo *colorize,ExceptionInfo *exception)
{
#define ColorizeImageTag  "Colorize/Image"
#define Colorize(pixel,blend_percentage,colorize)  \
  (((pixel)*(100.0-(blend_percentage))+(colorize)*(blend_percentage))/100.0)

  CacheView
    *image_view;

  GeometryInfo
    geometry_info;

  Image
    *colorize_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickStatusType
    flags;

  PixelInfo
    blend_percentage;

  ssize_t
    y;

  /*
    Allocate colorized image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  colorize_image=CloneImage(image,0,0,MagickTrue,exception);
  if (colorize_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(colorize_image,DirectClass,exception) == MagickFalse)
    {
      colorize_image=DestroyImage(colorize_image);
      return((Image *) NULL);
    }
  if ((IsGrayColorspace(colorize_image->colorspace) != MagickFalse) ||
      (IsPixelInfoGray(colorize) != MagickFalse))
    (void) SetImageColorspace(colorize_image,sRGBColorspace,exception);
  if ((colorize_image->alpha_trait == UndefinedPixelTrait) &&
      (colorize->alpha_trait != UndefinedPixelTrait))
    (void) SetImageAlpha(colorize_image,OpaqueAlpha,exception);
  if (blend == (const char *) NULL)
    return(colorize_image);
  GetPixelInfo(colorize_image,&blend_percentage);
  flags=ParseGeometry(blend,&geometry_info);
  blend_percentage.red=geometry_info.rho;
  blend_percentage.green=geometry_info.rho;
  blend_percentage.blue=geometry_info.rho;
  blend_percentage.black=geometry_info.rho;
  blend_percentage.alpha=(MagickRealType) TransparentAlpha;
  if ((flags & SigmaValue) != 0)
    blend_percentage.green=geometry_info.sigma;
  if ((flags & XiValue) != 0)
    blend_percentage.blue=geometry_info.xi;
  if ((flags & PsiValue) != 0)
    blend_percentage.alpha=geometry_info.psi;
  if (blend_percentage.colorspace == CMYKColorspace)
    {
      if ((flags & PsiValue) != 0)
        blend_percentage.black=geometry_info.psi;
      if ((flags & ChiValue) != 0)
        blend_percentage.alpha=geometry_info.chi;
    }
  /*
    Colorize DirectClass image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(colorize_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(colorize_image,colorize_image,colorize_image->rows,1)
#endif
  for (y=0; y < (ssize_t) colorize_image->rows; y++)
  {
    MagickBooleanType
      sync;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,colorize_image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) colorize_image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(colorize_image); i++)
      {
        PixelTrait traits = GetPixelChannelTraits(colorize_image,
          (PixelChannel) i);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & CopyPixelTrait) != 0)
          continue;
        SetPixelChannel(colorize_image,(PixelChannel) i,ClampToQuantum(
          Colorize(q[i],GetPixelInfoChannel(&blend_percentage,(PixelChannel) i),
          GetPixelInfoChannel(colorize,(PixelChannel) i))),q);
      }
      q+=GetPixelChannels(colorize_image);
    }
    sync=SyncCacheViewAuthenticPixels(image_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,ColorizeImageTag,progress,
          colorize_image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    colorize_image=DestroyImage(colorize_image);
  return(colorize_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o l o r M a t r i x I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ColorMatrixImage() applies color transformation to an image. This method
%  permits saturation changes, hue rotation, luminance to alpha, and various
%  other effects.  Although variable-sized transformation matrices can be used,
%  typically one uses a 5x5 matrix for an RGBA image and a 6x6 for CMYKA
%  (or RGBA with offsets).  The matrix is similar to those used by Adobe Flash
%  except offsets are in column 6 rather than 5 (in support of CMYKA images)
%  and offsets are normalized (divide Flash offset by 255).
%
%  The format of the ColorMatrixImage method is:
%
%      Image *ColorMatrixImage(const Image *image,
%        const KernelInfo *color_matrix,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o color_matrix:  the color matrix.
%
%    o exception: return any errors or warnings in this structure.
%
*/
/* FUTURE: modify to make use of a MagickMatrix Mutliply function
   That should be provided in "matrix.c"
   (ASIDE: actually distorts should do this too but currently doesn't)
*/

MagickExport Image *ColorMatrixImage(const Image *image,
  const KernelInfo *color_matrix,ExceptionInfo *exception)
{
#define ColorMatrixImageTag  "ColorMatrix/Image"

  CacheView
    *color_view,
    *image_view;

  double
    ColorMatrix[6][6] =
    {
      { 1.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
      { 0.0, 1.0, 0.0, 0.0, 0.0, 0.0 },
      { 0.0, 0.0, 1.0, 0.0, 0.0, 0.0 },
      { 0.0, 0.0, 0.0, 1.0, 0.0, 0.0 },
      { 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 },
      { 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 }
    };

  Image
    *color_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  register ssize_t
    i;

  ssize_t
    u,
    v,
    y;

  /*
    Map given color_matrix, into a 6x6 matrix   RGBKA and a constant
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  i=0;
  for (v=0; v < (ssize_t) color_matrix->height; v++)
    for (u=0; u < (ssize_t) color_matrix->width; u++)
    {
      if ((v < 6) && (u < 6))
        ColorMatrix[v][u]=color_matrix->values[i];
      i++;
    }
  /*
    Initialize color image.
  */
  color_image=CloneImage(image,0,0,MagickTrue,exception);
  if (color_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(color_image,DirectClass,exception) == MagickFalse)
    {
      color_image=DestroyImage(color_image);
      return((Image *) NULL);
    }
  if (image->debug != MagickFalse)
    {
      char
        format[MagickPathExtent],
        *message;

      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  ColorMatrix image with color matrix:");
      message=AcquireString("");
      for (v=0; v < 6; v++)
      {
        *message='\0';
        (void) FormatLocaleString(format,MagickPathExtent,"%.20g: ",(double) v);
        (void) ConcatenateString(&message,format);
        for (u=0; u < 6; u++)
        {
          (void) FormatLocaleString(format,MagickPathExtent,"%+f ",
            ColorMatrix[v][u]);
          (void) ConcatenateString(&message,format);
        }
        (void) LogMagickEvent(TransformEvent,GetMagickModule(),"%s",message);
      }
      message=DestroyString(message);
    }
  /*
    Apply the ColorMatrix to image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  color_view=AcquireAuthenticCacheView(color_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,color_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    PixelInfo
      pixel;

    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewAuthenticPixels(color_view,0,y,color_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    GetPixelInfo(image,&pixel);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        v;

      size_t
        height;

      GetPixelInfoPixel(image,p,&pixel);
      height=color_matrix->height > 6 ? 6UL : color_matrix->height;
      for (v=0; v < (ssize_t) height; v++)
      {
        double
          sum;

        sum=ColorMatrix[v][0]*GetPixelRed(image,p)+ColorMatrix[v][1]*
          GetPixelGreen(image,p)+ColorMatrix[v][2]*GetPixelBlue(image,p);
        if (image->colorspace == CMYKColorspace)
          sum+=ColorMatrix[v][3]*GetPixelBlack(image,p);
        if (image->alpha_trait != UndefinedPixelTrait)
          sum+=ColorMatrix[v][4]*GetPixelAlpha(image,p);
        sum+=QuantumRange*ColorMatrix[v][5];
        switch (v)
        {
          case 0: pixel.red=sum; break;
          case 1: pixel.green=sum; break;
          case 2: pixel.blue=sum; break;
          case 3: pixel.black=sum; break;
          case 4: pixel.alpha=sum; break;
          default: break;
        }
      }
      SetPixelViaPixelInfo(color_image,&pixel,q);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(color_image);
    }
    if (SyncCacheViewAuthenticPixels(color_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,ColorMatrixImageTag,progress,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  color_view=DestroyCacheView(color_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    color_image=DestroyImage(color_image);
  return(color_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I m p l o d e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImplodeImage() creates a new image that is a copy of an existing
%  one with the image pixels "implode" by the specified percentage.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ImplodeImage method is:
%
%      Image *ImplodeImage(const Image *image,const double amount,
%        const PixelInterpolateMethod method,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o implode_image: Method ImplodeImage returns a pointer to the image
%      after it is implode.  A null image is returned if there is a memory
%      shortage.
%
%    o image: the image.
%
%    o amount:  Define the extent of the implosion.
%
%    o method: the pixel interpolation method.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ImplodeImage(const Image *image,const double amount,
  const PixelInterpolateMethod method,ExceptionInfo *exception)
{
#define ImplodeImageTag  "Implode/Image"

  CacheView
    *canvas_view,
    *implode_view,
    *interpolate_view;

  double
    radius;

  Image
    *canvas_image,
    *implode_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PointInfo
    center,
    scale;

  ssize_t
    y;

  /*
    Initialize implode image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  canvas_image=CloneImage(image,0,0,MagickTrue,exception);
  if (canvas_image == (Image *) NULL)
    return((Image *) NULL);
  if ((canvas_image->alpha_trait == UndefinedPixelTrait) &&
      (canvas_image->background_color.alpha != OpaqueAlpha))
    (void) SetImageAlphaChannel(canvas_image,OpaqueAlphaChannel,exception);
  implode_image=CloneImage(canvas_image,0,0,MagickTrue,exception);
  if (implode_image == (Image *) NULL)
    {
      canvas_image=DestroyImage(canvas_image);
      return((Image *) NULL);
    }
  if (SetImageStorageClass(implode_image,DirectClass,exception) == MagickFalse)
    {
      canvas_image=DestroyImage(canvas_image);
      implode_image=DestroyImage(implode_image);
      return((Image *) NULL);
    }
  /*
    Compute scaling factor.
  */
  scale.x=1.0;
  scale.y=1.0;
  center.x=0.5*canvas_image->columns;
  center.y=0.5*canvas_image->rows;
  radius=center.x;
  if (canvas_image->columns > canvas_image->rows)
    scale.y=(double) canvas_image->columns/(double) canvas_image->rows;
  else
    if (canvas_image->columns < canvas_image->rows)
      {
        scale.x=(double) canvas_image->rows/(double) canvas_image->columns;
        radius=center.y;
      }
  /*
    Implode image.
  */
  status=MagickTrue;
  progress=0;
  canvas_view=AcquireVirtualCacheView(canvas_image,exception);
  interpolate_view=AcquireVirtualCacheView(canvas_image,exception);
  implode_view=AcquireAuthenticCacheView(implode_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(canvas_image,implode_image,canvas_image->rows,1)
#endif
  for (y=0; y < (ssize_t) canvas_image->rows; y++)
  {
    double
      distance;

    PointInfo
      delta;

    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    register Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(canvas_view,0,y,canvas_image->columns,1,
      exception);
    q=QueueCacheViewAuthenticPixels(implode_view,0,y,implode_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    delta.y=scale.y*(double) (y-center.y);
    for (x=0; x < (ssize_t) canvas_image->columns; x++)
    {
      register ssize_t
        i;

      /*
        Determine if the pixel is within an ellipse.
      */
      delta.x=scale.x*(double) (x-center.x);
      distance=delta.x*delta.x+delta.y*delta.y;
      if (distance >= (radius*radius))
        for (i=0; i < (ssize_t) GetPixelChannels(canvas_image); i++)
        {
          PixelChannel channel = GetPixelChannelChannel(canvas_image,i);
          PixelTrait traits = GetPixelChannelTraits(canvas_image,channel);
          PixelTrait implode_traits = GetPixelChannelTraits(implode_image,
            channel);
          if ((traits == UndefinedPixelTrait) ||
              (implode_traits == UndefinedPixelTrait))
            continue;
          SetPixelChannel(implode_image,channel,p[i],q);
        }
      else
        {
          double
            factor;

          /*
            Implode the pixel.
          */
          factor=1.0;
          if (distance > 0.0)
            factor=pow(sin(MagickPI*sqrt((double) distance)/radius/2),-amount);
          status=InterpolatePixelChannels(canvas_image,interpolate_view,
            implode_image,method,(double) (factor*delta.x/scale.x+center.x),
            (double) (factor*delta.y/scale.y+center.y),q,exception);
          if (status == MagickFalse)
            break;
        }
      p+=GetPixelChannels(canvas_image);
      q+=GetPixelChannels(implode_image);
    }
    if (SyncCacheViewAuthenticPixels(implode_view,exception) == MagickFalse)
      status=MagickFalse;
    if (canvas_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(canvas_image,ImplodeImageTag,progress,
          canvas_image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  implode_view=DestroyCacheView(implode_view);
  interpolate_view=DestroyCacheView(interpolate_view);
  canvas_view=DestroyCacheView(canvas_view);
  canvas_image=DestroyImage(canvas_image);
  if (status == MagickFalse)
    implode_image=DestroyImage(implode_image);
  return(implode_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M o r p h I m a g e s                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  The MorphImages() method requires a minimum of two images.  The first
%  image is transformed into the second by a number of intervening images
%  as specified by frames.
%
%  The format of the MorphImage method is:
%
%      Image *MorphImages(const Image *image,const size_t number_frames,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o number_frames:  Define the number of in-between image to generate.
%      The more in-between frames, the smoother the morph.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *MorphImages(const Image *image,const size_t number_frames,
  ExceptionInfo *exception)
{
#define MorphImageTag  "Morph/Image"

  double
    alpha,
    beta;

  Image
    *morph_image,
    *morph_images;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  register const Image
    *next;

  register ssize_t
    n;

  ssize_t
    y;

  /*
    Clone first frame in sequence.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  morph_images=CloneImage(image,0,0,MagickTrue,exception);
  if (morph_images == (Image *) NULL)
    return((Image *) NULL);
  if (GetNextImageInList(image) == (Image *) NULL)
    {
      /*
        Morph single image.
      */
      for (n=1; n < (ssize_t) number_frames; n++)
      {
        morph_image=CloneImage(image,0,0,MagickTrue,exception);
        if (morph_image == (Image *) NULL)
          {
            morph_images=DestroyImageList(morph_images);
            return((Image *) NULL);
          }
        AppendImageToList(&morph_images,morph_image);
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

            proceed=SetImageProgress(image,MorphImageTag,(MagickOffsetType) n,
              number_frames);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      return(GetFirstImageInList(morph_images));
    }
  /*
    Morph image sequence.
  */
  status=MagickTrue;
  scene=0;
  next=image;
  for ( ; GetNextImageInList(next) != (Image *) NULL; next=GetNextImageInList(next))
  {
    for (n=0; n < (ssize_t) number_frames; n++)
    {
      CacheView
        *image_view,
        *morph_view;

      beta=(double) (n+1.0)/(double) (number_frames+1.0);
      alpha=1.0-beta;
      morph_image=ResizeImage(next,(size_t) (alpha*next->columns+beta*
        GetNextImageInList(next)->columns+0.5),(size_t) (alpha*next->rows+beta*
        GetNextImageInList(next)->rows+0.5),next->filter,exception);
      if (morph_image == (Image *) NULL)
        {
          morph_images=DestroyImageList(morph_images);
          return((Image *) NULL);
        }
      status=SetImageStorageClass(morph_image,DirectClass,exception);
      if (status == MagickFalse)
        {
          morph_image=DestroyImage(morph_image);
          return((Image *) NULL);
        }
      AppendImageToList(&morph_images,morph_image);
      morph_images=GetLastImageInList(morph_images);
      morph_image=ResizeImage(GetNextImageInList(next),morph_images->columns,
        morph_images->rows,GetNextImageInList(next)->filter,exception);
      if (morph_image == (Image *) NULL)
        {
          morph_images=DestroyImageList(morph_images);
          return((Image *) NULL);
        }
      image_view=AcquireVirtualCacheView(morph_image,exception);
      morph_view=AcquireAuthenticCacheView(morph_images,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(morph_image,morph_image,morph_image->rows,1)
#endif
      for (y=0; y < (ssize_t) morph_images->rows; y++)
      {
        MagickBooleanType
          sync;

        register const Quantum
          *magick_restrict p;

        register ssize_t
          x;

        register Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        p=GetCacheViewVirtualPixels(image_view,0,y,morph_image->columns,1,
          exception);
        q=GetCacheViewAuthenticPixels(morph_view,0,y,morph_images->columns,1,
          exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) morph_images->columns; x++)
        {
          register ssize_t
            i;

          for (i=0; i < (ssize_t) GetPixelChannels(morph_image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(morph_image,i);
            PixelTrait traits = GetPixelChannelTraits(morph_image,channel);
            PixelTrait morph_traits=GetPixelChannelTraits(morph_images,channel);
            if ((traits == UndefinedPixelTrait) ||
                (morph_traits == UndefinedPixelTrait))
              continue;
            if ((morph_traits & CopyPixelTrait) != 0)
              {
                SetPixelChannel(morph_image,channel,p[i],q);
                continue;
              }
            SetPixelChannel(morph_image,channel,ClampToQuantum(alpha*
              GetPixelChannel(morph_images,channel,q)+beta*p[i]),q);
          }
          p+=GetPixelChannels(morph_image);
          q+=GetPixelChannels(morph_images);
        }
        sync=SyncCacheViewAuthenticPixels(morph_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      morph_view=DestroyCacheView(morph_view);
      image_view=DestroyCacheView(image_view);
      morph_image=DestroyImage(morph_image);
    }
    if (n < (ssize_t) number_frames)
      break;
    /*
      Clone last frame in sequence.
    */
    morph_image=CloneImage(GetNextImageInList(next),0,0,MagickTrue,exception);
    if (morph_image == (Image *) NULL)
      {
        morph_images=DestroyImageList(morph_images);
        return((Image *) NULL);
      }
    AppendImageToList(&morph_images,morph_image);
    morph_images=GetLastImageInList(morph_images);
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,MorphImageTag,scene,
          GetImageListLength(image));
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
    scene++;
  }
  if (GetNextImageInList(next) != (Image *) NULL)
    {
      morph_images=DestroyImageList(morph_images);
      return((Image *) NULL);
    }
  return(GetFirstImageInList(morph_images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     P l a s m a I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PlasmaImage() initializes an image with plasma fractal values.  The image
%  must be initialized with a base color and the random number generator
%  seeded before this method is called.
%
%  The format of the PlasmaImage method is:
%
%      MagickBooleanType PlasmaImage(Image *image,const SegmentInfo *segment,
%        size_t attenuate,size_t depth,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o segment:   Define the region to apply plasma fractals values.
%
%    o attenuate: Define the plasma attenuation factor.
%
%    o depth: Limit the plasma recursion depth.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline Quantum PlasmaPixel(RandomInfo *magick_restrict random_info,
  const double pixel,const double noise)
{
  MagickRealType
    plasma;

  plasma=pixel+noise*GetPseudoRandomValue(random_info)-noise/2.0;
  return(ClampToQuantum(plasma));
}

static MagickBooleanType PlasmaImageProxy(Image *image,CacheView *image_view,
  CacheView *u_view,CacheView *v_view,RandomInfo *magick_restrict random_info,
  const SegmentInfo *magick_restrict segment,size_t attenuate,size_t depth,
  ExceptionInfo *exception)
{
  double
    plasma;

  MagickStatusType
    status;

  register const Quantum
    *magick_restrict u,
    *magick_restrict v;

  register Quantum
    *magick_restrict q;

  register ssize_t
    i;

  ssize_t
    x,
    x_mid,
    y,
    y_mid;

  if ((fabs(segment->x2-segment->x1) < MagickEpsilon) &&
      (fabs(segment->y2-segment->y1) < MagickEpsilon))
    return(MagickTrue);
  if (depth != 0)
    {
      SegmentInfo
        local_info;

      /*
        Divide the area into quadrants and recurse.
      */
      depth--;
      attenuate++;
      x_mid=(ssize_t) ceil((segment->x1+segment->x2)/2-0.5);
      y_mid=(ssize_t) ceil((segment->y1+segment->y2)/2-0.5);
      local_info=(*segment);
      local_info.x2=(double) x_mid;
      local_info.y2=(double) y_mid;
      status=PlasmaImageProxy(image,image_view,u_view,v_view,random_info,
        &local_info,attenuate,depth,exception);
      local_info=(*segment);
      local_info.y1=(double) y_mid;
      local_info.x2=(double) x_mid;
      status&=PlasmaImageProxy(image,image_view,u_view,v_view,random_info,
        &local_info,attenuate,depth,exception);
      local_info=(*segment);
      local_info.x1=(double) x_mid;
      local_info.y2=(double) y_mid;
      status&=PlasmaImageProxy(image,image_view,u_view,v_view,random_info,
        &local_info,attenuate,depth,exception);
      local_info=(*segment);
      local_info.x1=(double) x_mid;
      local_info.y1=(double) y_mid;
      status&=PlasmaImageProxy(image,image_view,u_view,v_view,random_info,
        &local_info,attenuate,depth,exception);
      return(status == 0 ? MagickFalse : MagickTrue);
    }
  x_mid=(ssize_t) ceil((segment->x1+segment->x2)/2-0.5);
  y_mid=(ssize_t) ceil((segment->y1+segment->y2)/2-0.5);
  if ((fabs(segment->x1-x_mid) < MagickEpsilon) &&
      (fabs(segment->x2-x_mid) < MagickEpsilon) &&
      (fabs(segment->y1-y_mid) < MagickEpsilon) &&
      (fabs(segment->y2-y_mid) < MagickEpsilon))
    return(MagickFalse);
  /*
    Average pixels and apply plasma.
  */
  status=MagickTrue;
  plasma=(double) QuantumRange/(2.0*attenuate);
  if ((fabs(segment->x1-x_mid) >= MagickEpsilon) ||
      (fabs(segment->x2-x_mid) >= MagickEpsilon))
    {
      /*
        Left pixel.
      */
      x=(ssize_t) ceil(segment->x1-0.5);
      u=GetCacheViewVirtualPixels(u_view,x,(ssize_t) ceil(segment->y1-0.5),1,1,
        exception);
      v=GetCacheViewVirtualPixels(v_view,x,(ssize_t) ceil(segment->y2-0.5),1,1,
        exception);
      q=QueueCacheViewAuthenticPixels(image_view,x,y_mid,1,1,exception);
      if ((u == (const Quantum *) NULL) || (v == (const Quantum *) NULL) ||
          (q == (Quantum *) NULL))
        return(MagickTrue);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        q[i]=PlasmaPixel(random_info,((double) u[i]+v[i])/2.0,plasma);
      }
      status=SyncCacheViewAuthenticPixels(image_view,exception);
      if (fabs(segment->x1-segment->x2) >= MagickEpsilon)
        {
          /*
            Right pixel.
          */
          x=(ssize_t) ceil(segment->x2-0.5);
          u=GetCacheViewVirtualPixels(u_view,x,(ssize_t) ceil(segment->y1-0.5),
            1,1,exception);
          v=GetCacheViewVirtualPixels(v_view,x,(ssize_t) ceil(segment->y2-0.5),
            1,1,exception);
          q=QueueCacheViewAuthenticPixels(image_view,x,y_mid,1,1,exception);
          if ((u == (const Quantum *) NULL) || (v == (const Quantum *) NULL) ||
              (q == (Quantum *) NULL))
            return(MagickFalse);
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            if (traits == UndefinedPixelTrait)
              continue;
            q[i]=PlasmaPixel(random_info,((double) u[i]+v[i])/2.0,plasma);
          }
          status=SyncCacheViewAuthenticPixels(image_view,exception);
        }
    }
  if ((fabs(segment->y1-y_mid) >= MagickEpsilon) ||
      (fabs(segment->y2-y_mid) >= MagickEpsilon))
    {
      if ((fabs(segment->x1-x_mid) >= MagickEpsilon) ||
          (fabs(segment->y2-y_mid) >= MagickEpsilon))
        {
          /*
            Bottom pixel.
          */
          y=(ssize_t) ceil(segment->y2-0.5);
          u=GetCacheViewVirtualPixels(u_view,(ssize_t) ceil(segment->x1-0.5),y,
            1,1,exception);
          v=GetCacheViewVirtualPixels(v_view,(ssize_t) ceil(segment->x2-0.5),y,
            1,1,exception);
          q=QueueCacheViewAuthenticPixels(image_view,x_mid,y,1,1,exception);
          if ((u == (const Quantum *) NULL) || (v == (const Quantum *) NULL) ||
              (q == (Quantum *) NULL))
            return(MagickTrue);
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            if (traits == UndefinedPixelTrait)
              continue;
            q[i]=PlasmaPixel(random_info,((double) u[i]+v[i])/2.0,plasma);
          }
          status=SyncCacheViewAuthenticPixels(image_view,exception);
        }
      if (fabs(segment->y1-segment->y2) >= MagickEpsilon)
        {
          /*
            Top pixel.
          */
          y=(ssize_t) ceil(segment->y1-0.5);
          u=GetCacheViewVirtualPixels(u_view,(ssize_t) ceil(segment->x1-0.5),y,
            1,1,exception);
          v=GetCacheViewVirtualPixels(v_view,(ssize_t) ceil(segment->x2-0.5),y,
            1,1,exception);
          q=QueueCacheViewAuthenticPixels(image_view,x_mid,y,1,1,exception);
          if ((u == (const Quantum *) NULL) || (v == (const Quantum *) NULL) ||
              (q == (Quantum *) NULL))
            return(MagickTrue);
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            if (traits == UndefinedPixelTrait)
              continue;
            q[i]=PlasmaPixel(random_info,((double) u[i]+v[i])/2.0,plasma);
          }
          status=SyncCacheViewAuthenticPixels(image_view,exception);
        }
    }
  if ((fabs(segment->x1-segment->x2) >= MagickEpsilon) ||
      (fabs(segment->y1-segment->y2) >= MagickEpsilon))
    {
      /*
        Middle pixel.
      */
      x=(ssize_t) ceil(segment->x1-0.5);
      y=(ssize_t) ceil(segment->y1-0.5);
      u=GetCacheViewVirtualPixels(u_view,x,y,1,1,exception);
      x=(ssize_t) ceil(segment->x2-0.5);
      y=(ssize_t) ceil(segment->y2-0.5);
      v=GetCacheViewVirtualPixels(v_view,x,y,1,1,exception);
      q=QueueCacheViewAuthenticPixels(image_view,x_mid,y_mid,1,1,exception);
      if ((u == (const Quantum *) NULL) || (v == (const Quantum *) NULL) ||
          (q == (Quantum *) NULL))
        return(MagickTrue);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        q[i]=PlasmaPixel(random_info,((double) u[i]+v[i])/2.0,plasma);
      }
      status=SyncCacheViewAuthenticPixels(image_view,exception);
    }
  if ((fabs(segment->x2-segment->x1) < 3.0) &&
      (fabs(segment->y2-segment->y1) < 3.0))
    return(status == 0 ? MagickFalse : MagickTrue);
  return(MagickFalse);
}

MagickExport MagickBooleanType PlasmaImage(Image *image,
  const SegmentInfo *segment,size_t attenuate,size_t depth,
  ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *u_view,
    *v_view;

  MagickBooleanType
    status;

  RandomInfo
    *random_info;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  image_view=AcquireAuthenticCacheView(image,exception);
  u_view=AcquireVirtualCacheView(image,exception);
  v_view=AcquireVirtualCacheView(image,exception);
  random_info=AcquireRandomInfo();
  status=PlasmaImageProxy(image,image_view,u_view,v_view,random_info,segment,
    attenuate,depth,exception);
  random_info=DestroyRandomInfo(random_info);
  v_view=DestroyCacheView(v_view);
  u_view=DestroyCacheView(u_view);
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P o l a r o i d I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PolaroidImage() simulates a Polaroid picture.
%
%  The format of the PolaroidImage method is:
%
%      Image *PolaroidImage(const Image *image,const DrawInfo *draw_info,
%        const char *caption,const double angle,
%        const PixelInterpolateMethod method,ExceptionInfo exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o draw_info: the draw info.
%
%    o caption: the Polaroid caption.
%
%    o angle: Apply the effect along this angle.
%
%    o method: the pixel interpolation method.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *PolaroidImage(const Image *image,const DrawInfo *draw_info,
  const char *caption,const double angle,const PixelInterpolateMethod method,
  ExceptionInfo *exception)
{
  Image
    *bend_image,
    *caption_image,
    *flop_image,
    *picture_image,
    *polaroid_image,
    *rotate_image,
    *trim_image;

  size_t
    height;

  ssize_t
    quantum;

  /*
    Simulate a Polaroid picture.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  quantum=(ssize_t) MagickMax(MagickMax((double) image->columns,(double)
    image->rows)/25.0,10.0);
  height=image->rows+2*quantum;
  caption_image=(Image *) NULL;
  if (caption != (const char *) NULL)
    {
      char
        *text;

      /*
        Generate caption image.
      */
      caption_image=CloneImage(image,image->columns,1,MagickTrue,exception);
      if (caption_image == (Image *) NULL)
        return((Image *) NULL);
      text=InterpretImageProperties((ImageInfo *) NULL,(Image *) image,caption,
        exception);
      if (text != (char *) NULL)
        {
          char
            geometry[MagickPathExtent];

          DrawInfo
            *annotate_info;

          MagickBooleanType
            status;

          ssize_t
            count;

          TypeMetric
            metrics;

          annotate_info=CloneDrawInfo((const ImageInfo *) NULL,draw_info);
          (void) CloneString(&annotate_info->text,text);
          count=FormatMagickCaption(caption_image,annotate_info,MagickTrue,
            &metrics,&text,exception);
          status=SetImageExtent(caption_image,image->columns,(size_t)
            ((count+1)*(metrics.ascent-metrics.descent)+0.5),exception);
          if (status == MagickFalse)
            caption_image=DestroyImage(caption_image);
          else
            {
              caption_image->background_color=image->border_color;
              (void) SetImageBackgroundColor(caption_image,exception);
              (void) CloneString(&annotate_info->text,text);
              (void) FormatLocaleString(geometry,MagickPathExtent,"+0+%.20g",
                metrics.ascent);
              if (annotate_info->gravity == UndefinedGravity)
                (void) CloneString(&annotate_info->geometry,AcquireString(
                  geometry));
              (void) AnnotateImage(caption_image,annotate_info,exception);
              height+=caption_image->rows;
            }
          annotate_info=DestroyDrawInfo(annotate_info);
          text=DestroyString(text);
        }
    }
  picture_image=CloneImage(image,image->columns+2*quantum,height,MagickTrue,
    exception);
  if (picture_image == (Image *) NULL)
    {
      if (caption_image != (Image *) NULL)
        caption_image=DestroyImage(caption_image);
      return((Image *) NULL);
    }
  picture_image->background_color=image->border_color;
  (void) SetImageBackgroundColor(picture_image,exception);
  (void) CompositeImage(picture_image,image,OverCompositeOp,MagickTrue,quantum,
    quantum,exception);
  if (caption_image != (Image *) NULL)
    {
      (void) CompositeImage(picture_image,caption_image,OverCompositeOp,
        MagickTrue,quantum,(ssize_t) (image->rows+3*quantum/2),exception);
      caption_image=DestroyImage(caption_image);
    }
  (void) QueryColorCompliance("none",AllCompliance,
    &picture_image->background_color,exception);
  (void) SetImageAlphaChannel(picture_image,OpaqueAlphaChannel,exception);
  rotate_image=RotateImage(picture_image,90.0,exception);
  picture_image=DestroyImage(picture_image);
  if (rotate_image == (Image *) NULL)
    return((Image *) NULL);
  picture_image=rotate_image;
  bend_image=WaveImage(picture_image,0.01*picture_image->rows,2.0*
    picture_image->columns,method,exception);
  picture_image=DestroyImage(picture_image);
  if (bend_image == (Image *) NULL)
    return((Image *) NULL);
  picture_image=bend_image;
  rotate_image=RotateImage(picture_image,-90.0,exception);
  picture_image=DestroyImage(picture_image);
  if (rotate_image == (Image *) NULL)
    return((Image *) NULL);
  picture_image=rotate_image;
  picture_image->background_color=image->background_color;
  polaroid_image=ShadowImage(picture_image,80.0,2.0,quantum/3,quantum/3,
    exception);
  if (polaroid_image == (Image *) NULL)
    {
      picture_image=DestroyImage(picture_image);
      return(picture_image);
    }
  flop_image=FlopImage(polaroid_image,exception);
  polaroid_image=DestroyImage(polaroid_image);
  if (flop_image == (Image *) NULL)
    {
      picture_image=DestroyImage(picture_image);
      return(picture_image);
    }
  polaroid_image=flop_image;
  (void) CompositeImage(polaroid_image,picture_image,OverCompositeOp,
    MagickTrue,(ssize_t) (-0.01*picture_image->columns/2.0),0L,exception);
  picture_image=DestroyImage(picture_image);
  (void) QueryColorCompliance("none",AllCompliance,
    &polaroid_image->background_color,exception);
  rotate_image=RotateImage(polaroid_image,angle,exception);
  polaroid_image=DestroyImage(polaroid_image);
  if (rotate_image == (Image *) NULL)
    return((Image *) NULL);
  polaroid_image=rotate_image;
  trim_image=TrimImage(polaroid_image,exception);
  polaroid_image=DestroyImage(polaroid_image);
  if (trim_image == (Image *) NULL)
    return((Image *) NULL);
  polaroid_image=trim_image;
  return(polaroid_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S e p i a T o n e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSepiaToneImage() applies a special effect to the image, similar to the
%  effect achieved in a photo darkroom by sepia toning.  Threshold ranges from
%  0 to QuantumRange and is a measure of the extent of the sepia toning.  A
%  threshold of 80% is a good starting point for a reasonable tone.
%
%  The format of the SepiaToneImage method is:
%
%      Image *SepiaToneImage(const Image *image,const double threshold,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: the tone threshold.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *SepiaToneImage(const Image *image,const double threshold,
  ExceptionInfo *exception)
{
#define SepiaToneImageTag  "SepiaTone/Image"

  CacheView
    *image_view,
    *sepia_view;

  Image
    *sepia_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  /*
    Initialize sepia-toned image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  sepia_image=CloneImage(image,0,0,MagickTrue,exception);
  if (sepia_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(sepia_image,DirectClass,exception) == MagickFalse)
    {
      sepia_image=DestroyImage(sepia_image);
      return((Image *) NULL);
    }
  /*
    Tone each row of the image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  sepia_view=AcquireAuthenticCacheView(sepia_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,sepia_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    register Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewAuthenticPixels(sepia_view,0,y,sepia_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        intensity,
        tone;

      intensity=GetPixelIntensity(image,p);
      tone=intensity > threshold ? (double) QuantumRange : intensity+
        (double) QuantumRange-threshold;
      SetPixelRed(sepia_image,ClampToQuantum(tone),q);
      tone=intensity > (7.0*threshold/6.0) ? (double) QuantumRange :
        intensity+(double) QuantumRange-7.0*threshold/6.0;
      SetPixelGreen(sepia_image,ClampToQuantum(tone),q);
      tone=intensity < (threshold/6.0) ? 0 : intensity-threshold/6.0;
      SetPixelBlue(sepia_image,ClampToQuantum(tone),q);
      tone=threshold/7.0;
      if ((double) GetPixelGreen(image,q) < tone)
        SetPixelGreen(sepia_image,ClampToQuantum(tone),q);
      if ((double) GetPixelBlue(image,q) < tone)
        SetPixelBlue(sepia_image,ClampToQuantum(tone),q);
      SetPixelAlpha(sepia_image,GetPixelAlpha(image,p),q);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(sepia_image);
    }
    if (SyncCacheViewAuthenticPixels(sepia_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,SepiaToneImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  sepia_view=DestroyCacheView(sepia_view);
  image_view=DestroyCacheView(image_view);
  (void) NormalizeImage(sepia_image,exception);
  (void) ContrastImage(sepia_image,MagickTrue,exception);
  if (status == MagickFalse)
    sepia_image=DestroyImage(sepia_image);
  return(sepia_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S h a d o w I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ShadowImage() simulates a shadow from the specified image and returns it.
%
%  The format of the ShadowImage method is:
%
%      Image *ShadowImage(const Image *image,const double alpha,
%        const double sigma,const ssize_t x_offset,const ssize_t y_offset,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o alpha: percentage transparency.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o x_offset: the shadow x-offset.
%
%    o y_offset: the shadow y-offset.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ShadowImage(const Image *image,const double alpha,
  const double sigma,const ssize_t x_offset,const ssize_t y_offset,
  ExceptionInfo *exception)
{
#define ShadowImageTag  "Shadow/Image"

  CacheView
    *image_view;

  ChannelType
    channel_mask;

  Image
    *border_image,
    *clone_image,
    *shadow_image;

  MagickBooleanType
    status;

  PixelInfo
    background_color;

  RectangleInfo
    border_info;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  clone_image=CloneImage(image,0,0,MagickTrue,exception);
  if (clone_image == (Image *) NULL)
    return((Image *) NULL);
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    (void) SetImageColorspace(clone_image,sRGBColorspace,exception);
  (void) SetImageVirtualPixelMethod(clone_image,EdgeVirtualPixelMethod,
    exception);
  border_info.width=(size_t) floor(2.0*sigma+0.5);
  border_info.height=(size_t) floor(2.0*sigma+0.5);
  border_info.x=0;
  border_info.y=0;
  (void) QueryColorCompliance("none",AllCompliance,&clone_image->border_color,
    exception);
  clone_image->alpha_trait=BlendPixelTrait;
  border_image=BorderImage(clone_image,&border_info,OverCompositeOp,exception);
  clone_image=DestroyImage(clone_image);
  if (border_image == (Image *) NULL)
    return((Image *) NULL);
  if (border_image->alpha_trait == UndefinedPixelTrait)
    (void) SetImageAlphaChannel(border_image,OpaqueAlphaChannel,exception);
  /*
    Shadow image.
  */
  status=MagickTrue;
  background_color=border_image->background_color;
  background_color.alpha_trait=BlendPixelTrait;
  image_view=AcquireAuthenticCacheView(border_image,exception);
  for (y=0; y < (ssize_t) border_image->rows; y++)
  {
    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(image_view,0,y,border_image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) border_image->columns; x++)
    {
      if (border_image->alpha_trait != UndefinedPixelTrait)
        background_color.alpha=GetPixelAlpha(border_image,q)*alpha/100.0;
      SetPixelViaPixelInfo(border_image,&background_color,q);
      q+=GetPixelChannels(border_image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    {
      border_image=DestroyImage(border_image);
      return((Image *) NULL);
    }
  channel_mask=SetImageChannelMask(border_image,AlphaChannel);
  shadow_image=BlurImage(border_image,0.0,sigma,exception);
  border_image=DestroyImage(border_image);
  if (shadow_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetPixelChannelMask(shadow_image,channel_mask);
  if (shadow_image->page.width == 0)
    shadow_image->page.width=shadow_image->columns;
  if (shadow_image->page.height == 0)
    shadow_image->page.height=shadow_image->rows;
  shadow_image->page.width+=x_offset-(ssize_t) border_info.width;
  shadow_image->page.height+=y_offset-(ssize_t) border_info.height;
  shadow_image->page.x+=x_offset-(ssize_t) border_info.width;
  shadow_image->page.y+=y_offset-(ssize_t) border_info.height;
  return(shadow_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S k e t c h I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SketchImage() simulates a pencil sketch.  We convolve the image with a
%  Gaussian operator of the given radius and standard deviation (sigma).  For
%  reasonable results, radius should be larger than sigma.  Use a radius of 0
%  and SketchImage() selects a suitable radius for you.  Angle gives the angle
%  of the sketch.
%
%  The format of the SketchImage method is:
%
%    Image *SketchImage(const Image *image,const double radius,
%      const double sigma,const double angle,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the
%      center pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o angle: apply the effect along this angle.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *SketchImage(const Image *image,const double radius,
  const double sigma,const double angle,ExceptionInfo *exception)
{
  CacheView
    *random_view;

  Image
    *blend_image,
    *blur_image,
    *dodge_image,
    *random_image,
    *sketch_image;

  MagickBooleanType
    status;

  RandomInfo
    **magick_restrict random_info;

  ssize_t
    y;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  unsigned long
    key;
#endif

  /*
    Sketch image.
  */
  random_image=CloneImage(image,image->columns << 1,image->rows << 1,
    MagickTrue,exception);
  if (random_image == (Image *) NULL)
    return((Image *) NULL);
  status=MagickTrue;
  random_info=AcquireRandomInfoThreadSet();
  random_view=AcquireAuthenticCacheView(random_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  key=GetRandomSecretKey(random_info[0]);
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(random_image,random_image,random_image->rows,key == ~0UL)
#endif
  for (y=0; y < (ssize_t) random_image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(random_view,0,y,random_image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) random_image->columns; x++)
    {
      double
        value;

      register ssize_t
        i;

      value=GetPseudoRandomValue(random_info[id]);
      for (i=0; i < (ssize_t) GetPixelChannels(random_image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        q[i]=ClampToQuantum(QuantumRange*value);
      }
      q+=GetPixelChannels(random_image);
    }
    if (SyncCacheViewAuthenticPixels(random_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  random_view=DestroyCacheView(random_view);
  random_info=DestroyRandomInfoThreadSet(random_info);
  if (status == MagickFalse)
    {
      random_image=DestroyImage(random_image);
      return(random_image);
    }
  blur_image=MotionBlurImage(random_image,radius,sigma,angle,exception);
  random_image=DestroyImage(random_image);
  if (blur_image == (Image *) NULL)
    return((Image *) NULL);
  dodge_image=EdgeImage(blur_image,radius,exception);
  blur_image=DestroyImage(blur_image);
  if (dodge_image == (Image *) NULL)
    return((Image *) NULL);
  status=ClampImage(dodge_image,exception);
  if (status != MagickFalse)
    status=NormalizeImage(dodge_image,exception);
  if (status != MagickFalse)
    status=NegateImage(dodge_image,MagickFalse,exception);
  if (status != MagickFalse)
    status=TransformImage(&dodge_image,(char *) NULL,"50%",exception);
  sketch_image=CloneImage(image,0,0,MagickTrue,exception);
  if (sketch_image == (Image *) NULL)
    {
      dodge_image=DestroyImage(dodge_image);
      return((Image *) NULL);
    }
  (void) CompositeImage(sketch_image,dodge_image,ColorDodgeCompositeOp,
    MagickTrue,0,0,exception);
  dodge_image=DestroyImage(dodge_image);
  blend_image=CloneImage(image,0,0,MagickTrue,exception);
  if (blend_image == (Image *) NULL)
    {
      sketch_image=DestroyImage(sketch_image);
      return((Image *) NULL);
    }
  if (blend_image->alpha_trait != BlendPixelTrait)
    (void) SetImageAlpha(blend_image,TransparentAlpha,exception);
  (void) SetImageArtifact(blend_image,"compose:args","20x80");
  (void) CompositeImage(sketch_image,blend_image,BlendCompositeOp,MagickTrue,
    0,0,exception);
  blend_image=DestroyImage(blend_image);
  return(sketch_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S o l a r i z e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SolarizeImage() applies a special effect to the image, similar to the effect
%  achieved in a photo darkroom by selectively exposing areas of photo
%  sensitive paper to light.  Threshold ranges from 0 to QuantumRange and is a
%  measure of the extent of the solarization.
%
%  The format of the SolarizeImage method is:
%
%      MagickBooleanType SolarizeImage(Image *image,const double threshold,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold:  Define the extent of the solarization.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SolarizeImage(Image *image,
  const double threshold,ExceptionInfo *exception)
{
#define SolarizeImageTag  "Solarize/Image"

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
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    (void) SetImageColorspace(image,sRGBColorspace,exception);
  if (image->storage_class == PseudoClass)
    {
      register ssize_t
        i;

      /*
        Solarize colormap.
      */
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        if ((double) image->colormap[i].red > threshold)
          image->colormap[i].red=QuantumRange-image->colormap[i].red;
        if ((double) image->colormap[i].green > threshold)
          image->colormap[i].green=QuantumRange-image->colormap[i].green;
        if ((double) image->colormap[i].blue > threshold)
          image->colormap[i].blue=QuantumRange-image->colormap[i].blue;
      }
      return(SyncImage(image,exception));
    }
  /*
    Solarize image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register Quantum
      *magick_restrict q;

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
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if ((double) q[i] > threshold)
          q[i]=QuantumRange-q[i];
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
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,SolarizeImageTag,progress,image->rows);
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
%   S t e g a n o I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SteganoImage() hides a digital watermark within the image.  Recover
%  the hidden watermark later to prove that the authenticity of an image.
%  Offset defines the start position within the image to hide the watermark.
%
%  The format of the SteganoImage method is:
%
%      Image *SteganoImage(const Image *image,Image *watermark,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o watermark: the watermark image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *SteganoImage(const Image *image,const Image *watermark,
  ExceptionInfo *exception)
{
#define GetBit(alpha,i) ((((size_t) (alpha) >> (size_t) (i)) & 0x01) != 0)
#define SetBit(alpha,i,set) (Quantum) ((set) != 0 ? (size_t) (alpha) \
  | (one << (size_t) (i)) : (size_t) (alpha) & ~(one << (size_t) (i)))
#define SteganoImageTag  "Stegano/Image"

  CacheView
    *stegano_view,
    *watermark_view;

  Image
    *stegano_image;

  int
    c;

  MagickBooleanType
    status;

  PixelInfo
    pixel;

  register Quantum
    *q;

  register ssize_t
    x;

  size_t
    depth,
    one;

  ssize_t
    i,
    j,
    k,
    y;

  /*
    Initialize steganographic image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(watermark != (const Image *) NULL);
  assert(watermark->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  one=1UL;
  stegano_image=CloneImage(image,0,0,MagickTrue,exception);
  if (stegano_image == (Image *) NULL)
    return((Image *) NULL);
  stegano_image->depth=MAGICKCORE_QUANTUM_DEPTH;
  if (SetImageStorageClass(stegano_image,DirectClass,exception) == MagickFalse)
    {
      stegano_image=DestroyImage(stegano_image);
      return((Image *) NULL);
    }
  /*
    Hide watermark in low-order bits of image.
  */
  c=0;
  i=0;
  j=0;
  depth=stegano_image->depth;
  k=stegano_image->offset;
  status=MagickTrue;
  watermark_view=AcquireVirtualCacheView(watermark,exception);
  stegano_view=AcquireAuthenticCacheView(stegano_image,exception);
  for (i=(ssize_t) depth-1; (i >= 0) && (j < (ssize_t) depth); i--)
  {
    for (y=0; (y < (ssize_t) watermark->rows) && (j < (ssize_t) depth); y++)
    {
      for (x=0; (x < (ssize_t) watermark->columns) && (j < (ssize_t) depth); x++)
      {
        ssize_t
          offset;

        (void) GetOneCacheViewVirtualPixelInfo(watermark_view,x,y,&pixel,
          exception);
        offset=k/(ssize_t) stegano_image->columns;
        if (offset >= (ssize_t) stegano_image->rows)
          break;
        q=GetCacheViewAuthenticPixels(stegano_view,k % (ssize_t)
          stegano_image->columns,k/(ssize_t) stegano_image->columns,1,1,
          exception);
        if (q == (Quantum *) NULL)
          break;
        switch (c)
        {
          case 0:
          {
            SetPixelRed(stegano_image,SetBit(GetPixelRed(stegano_image,q),j,
              GetBit(GetPixelInfoIntensity(stegano_image,&pixel),i)),q);
            break;
          }
          case 1:
          {
            SetPixelGreen(stegano_image,SetBit(GetPixelGreen(stegano_image,q),j,
              GetBit(GetPixelInfoIntensity(stegano_image,&pixel),i)),q);
            break;
          }
          case 2:
          {
            SetPixelBlue(stegano_image,SetBit(GetPixelBlue(stegano_image,q),j,
              GetBit(GetPixelInfoIntensity(stegano_image,&pixel),i)),q);
            break;
          }
        }
        if (SyncCacheViewAuthenticPixels(stegano_view,exception) == MagickFalse)
          break;
        c++;
        if (c == 3)
          c=0;
        k++;
        if (k == (ssize_t) (stegano_image->columns*stegano_image->columns))
          k=0;
        if (k == stegano_image->offset)
          j++;
      }
    }
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,SteganoImageTag,(MagickOffsetType)
          (depth-i),depth);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  stegano_view=DestroyCacheView(stegano_view);
  watermark_view=DestroyCacheView(watermark_view);
  if (status == MagickFalse)
    stegano_image=DestroyImage(stegano_image);
  return(stegano_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t e r e o A n a g l y p h I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StereoAnaglyphImage() combines two images and produces a single image that
%  is the composite of a left and right image of a stereo pair.  Special
%  red-green stereo glasses are required to view this effect.
%
%  The format of the StereoAnaglyphImage method is:
%
%      Image *StereoImage(const Image *left_image,const Image *right_image,
%        ExceptionInfo *exception)
%      Image *StereoAnaglyphImage(const Image *left_image,
%        const Image *right_image,const ssize_t x_offset,const ssize_t y_offset,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o left_image: the left image.
%
%    o right_image: the right image.
%
%    o exception: return any errors or warnings in this structure.
%
%    o x_offset: amount, in pixels, by which the left image is offset to the
%      right of the right image.
%
%    o y_offset: amount, in pixels, by which the left image is offset to the
%      bottom of the right image.
%
%
*/
MagickExport Image *StereoImage(const Image *left_image,
  const Image *right_image,ExceptionInfo *exception)
{
  return(StereoAnaglyphImage(left_image,right_image,0,0,exception));
}

MagickExport Image *StereoAnaglyphImage(const Image *left_image,
  const Image *right_image,const ssize_t x_offset,const ssize_t y_offset,
  ExceptionInfo *exception)
{
#define StereoImageTag  "Stereo/Image"

  const Image
    *image;

  Image
    *stereo_image;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(left_image != (const Image *) NULL);
  assert(left_image->signature == MagickCoreSignature);
  if (left_image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      left_image->filename);
  assert(right_image != (const Image *) NULL);
  assert(right_image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=left_image;
  if ((left_image->columns != right_image->columns) ||
      (left_image->rows != right_image->rows))
    ThrowImageException(ImageError,"LeftAndRightImageSizesDiffer");
  /*
    Initialize stereo image attributes.
  */
  stereo_image=CloneImage(left_image,left_image->columns,left_image->rows,
    MagickTrue,exception);
  if (stereo_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(stereo_image,DirectClass,exception) == MagickFalse)
    {
      stereo_image=DestroyImage(stereo_image);
      return((Image *) NULL);
    }
  (void) SetImageColorspace(stereo_image,sRGBColorspace,exception);
  /*
    Copy left image to red channel and right image to blue channel.
  */
  status=MagickTrue;
  for (y=0; y < (ssize_t) stereo_image->rows; y++)
  {
    register const Quantum
      *magick_restrict p,
      *magick_restrict q;

    register ssize_t
      x;

    register Quantum
      *magick_restrict r;

    p=GetVirtualPixels(left_image,-x_offset,y-y_offset,image->columns,1,
      exception);
    q=GetVirtualPixels(right_image,0,y,right_image->columns,1,exception);
    r=QueueAuthenticPixels(stereo_image,0,y,stereo_image->columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL) ||
        (r == (Quantum *) NULL))
      break;
    for (x=0; x < (ssize_t) stereo_image->columns; x++)
    {
      SetPixelRed(stereo_image,GetPixelRed(left_image,p),r);
      SetPixelGreen(stereo_image,GetPixelGreen(right_image,q),r);
      SetPixelBlue(stereo_image,GetPixelBlue(right_image,q),r);
      if ((GetPixelAlphaTraits(stereo_image) & CopyPixelTrait) != 0)
        SetPixelAlpha(stereo_image,(GetPixelAlpha(left_image,p)+
          GetPixelAlpha(right_image,q))/2,r);
      p+=GetPixelChannels(left_image);
      q+=GetPixelChannels(right_image);
      r+=GetPixelChannels(stereo_image);
    }
    if (SyncAuthenticPixels(stereo_image,exception) == MagickFalse)
      break;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,StereoImageTag,(MagickOffsetType) y,
          stereo_image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  if (status == MagickFalse)
    stereo_image=DestroyImage(stereo_image);
  return(stereo_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S w i r l I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SwirlImage() swirls the pixels about the center of the image, where
%  degrees indicates the sweep of the arc through which each pixel is moved.
%  You get a more dramatic effect as the degrees move from 1 to 360.
%
%  The format of the SwirlImage method is:
%
%      Image *SwirlImage(const Image *image,double degrees,
%        const PixelInterpolateMethod method,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o degrees: Define the tightness of the swirling effect.
%
%    o method: the pixel interpolation method.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *SwirlImage(const Image *image,double degrees,
  const PixelInterpolateMethod method,ExceptionInfo *exception)
{
#define SwirlImageTag  "Swirl/Image"

  CacheView
    *canvas_view,
    *interpolate_view,
    *swirl_view;

  double
    radius;

  Image
    *canvas_image,
    *swirl_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PointInfo
    center,
    scale;

  ssize_t
    y;

  /*
    Initialize swirl image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  canvas_image=CloneImage(image,0,0,MagickTrue,exception);
  if (canvas_image == (Image *) NULL)
    return((Image *) NULL);
  swirl_image=CloneImage(canvas_image,0,0,MagickTrue,exception);
  if (swirl_image == (Image *) NULL)
    {
      canvas_image=DestroyImage(canvas_image);
      return((Image *) NULL);
    }
  if (SetImageStorageClass(swirl_image,DirectClass,exception) == MagickFalse)
    {
      canvas_image=DestroyImage(canvas_image);
      swirl_image=DestroyImage(swirl_image);
      return((Image *) NULL);
    }
  if (swirl_image->background_color.alpha_trait != UndefinedPixelTrait)
    (void) SetImageAlphaChannel(swirl_image,OnAlphaChannel,exception);
  /*
    Compute scaling factor.
  */
  center.x=(double) canvas_image->columns/2.0;
  center.y=(double) canvas_image->rows/2.0;
  radius=MagickMax(center.x,center.y);
  scale.x=1.0;
  scale.y=1.0;
  if (canvas_image->columns > canvas_image->rows)
    scale.y=(double) canvas_image->columns/(double) canvas_image->rows;
  else
    if (canvas_image->columns < canvas_image->rows)
      scale.x=(double) canvas_image->rows/(double) canvas_image->columns;
  degrees=(double) DegreesToRadians(degrees);
  /*
    Swirl image.
  */
  status=MagickTrue;
  progress=0;
  canvas_view=AcquireVirtualCacheView(canvas_image,exception);
  interpolate_view=AcquireVirtualCacheView(image,exception);
  swirl_view=AcquireAuthenticCacheView(swirl_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(canvas_image,swirl_image,canvas_image->rows,1)
#endif
  for (y=0; y < (ssize_t) canvas_image->rows; y++)
  {
    double
      distance;

    PointInfo
      delta;

    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    register Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(canvas_view,0,y,canvas_image->columns,1,
      exception);
    q=QueueCacheViewAuthenticPixels(swirl_view,0,y,swirl_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    delta.y=scale.y*(double) (y-center.y);
    for (x=0; x < (ssize_t) canvas_image->columns; x++)
    {
      /*
        Determine if the pixel is within an ellipse.
      */
      delta.x=scale.x*(double) (x-center.x);
      distance=delta.x*delta.x+delta.y*delta.y;
      if (distance >= (radius*radius))
        {
          register ssize_t
            i;

          for (i=0; i < (ssize_t) GetPixelChannels(canvas_image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(canvas_image,i);
            PixelTrait traits = GetPixelChannelTraits(canvas_image,channel);
            PixelTrait swirl_traits = GetPixelChannelTraits(swirl_image,
              channel);
            if ((traits == UndefinedPixelTrait) ||
                (swirl_traits == UndefinedPixelTrait))
              continue;
            SetPixelChannel(swirl_image,channel,p[i],q);
          }
        }
      else
        {
          double
            cosine,
            factor,
            sine;

          /*
            Swirl the pixel.
          */
          factor=1.0-sqrt((double) distance)/radius;
          sine=sin((double) (degrees*factor*factor));
          cosine=cos((double) (degrees*factor*factor));
          status=InterpolatePixelChannels(canvas_image,interpolate_view,
            swirl_image,method,((cosine*delta.x-sine*delta.y)/scale.x+center.x),
            (double) ((sine*delta.x+cosine*delta.y)/scale.y+center.y),q,
            exception);
          if (status == MagickFalse)
            break;
        }
      p+=GetPixelChannels(canvas_image);
      q+=GetPixelChannels(swirl_image);
    }
    if (SyncCacheViewAuthenticPixels(swirl_view,exception) == MagickFalse)
      status=MagickFalse;
    if (canvas_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(canvas_image,SwirlImageTag,progress,
          canvas_image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  swirl_view=DestroyCacheView(swirl_view);
  interpolate_view=DestroyCacheView(interpolate_view);
  canvas_view=DestroyCacheView(canvas_view);
  canvas_image=DestroyImage(canvas_image);
  if (status == MagickFalse)
    swirl_image=DestroyImage(swirl_image);
  return(swirl_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     T i n t I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TintImage() applies a color vector to each pixel in the image.  The length
%  of the vector is 0 for black and white and at its maximum for the midtones.
%  The vector weighting function is f(x)=(1-(4.0*((x-0.5)*(x-0.5))))
%
%  The format of the TintImage method is:
%
%      Image *TintImage(const Image *image,const char *blend,
%        const PixelInfo *tint,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o blend: A color value used for tinting.
%
%    o tint: A color value used for tinting.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *TintImage(const Image *image,const char *blend,
  const PixelInfo *tint,ExceptionInfo *exception)
{
#define TintImageTag  "Tint/Image"

  CacheView
    *image_view,
    *tint_view;

  double
    intensity;

  GeometryInfo
    geometry_info;

  Image
    *tint_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    color_vector;

  MagickStatusType
    flags;

  ssize_t
    y;

  /*
    Allocate tint image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  tint_image=CloneImage(image,0,0,MagickTrue,exception);
  if (tint_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(tint_image,DirectClass,exception) == MagickFalse)
    {
      tint_image=DestroyImage(tint_image);
      return((Image *) NULL);
    }
  if ((IsGrayColorspace(image->colorspace) != MagickFalse) &&
      (IsPixelInfoGray(tint) == MagickFalse))
    (void) SetImageColorspace(tint_image,sRGBColorspace,exception);
  if (blend == (const char *) NULL)
    return(tint_image);
  /*
    Determine RGB values of the color.
  */
  GetPixelInfo(image,&color_vector);
  flags=ParseGeometry(blend,&geometry_info);
  color_vector.red=geometry_info.rho;
  color_vector.green=geometry_info.rho;
  color_vector.blue=geometry_info.rho;
  color_vector.alpha=(MagickRealType) OpaqueAlpha;
  if ((flags & SigmaValue) != 0)
    color_vector.green=geometry_info.sigma;
  if ((flags & XiValue) != 0)
    color_vector.blue=geometry_info.xi;
  if ((flags & PsiValue) != 0)
    color_vector.alpha=geometry_info.psi;
  if (image->colorspace == CMYKColorspace)
    {
      color_vector.black=geometry_info.rho;
      if ((flags & PsiValue) != 0)
        color_vector.black=geometry_info.psi;
      if ((flags & ChiValue) != 0)
        color_vector.alpha=geometry_info.chi;
    }
  intensity=(double) GetPixelInfoIntensity((const Image *) NULL,tint);
  color_vector.red=(double) (color_vector.red*tint->red/100.0-intensity);
  color_vector.green=(double) (color_vector.green*tint->green/100.0-intensity);
  color_vector.blue=(double) (color_vector.blue*tint->blue/100.0-intensity);
  color_vector.black=(double) (color_vector.black*tint->black/100.0-intensity);
  color_vector.alpha=(double) (color_vector.alpha*tint->alpha/100.0-intensity);
  /*
    Tint image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  tint_view=AcquireAuthenticCacheView(tint_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,tint_image,image->rows,1)
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
    q=QueueCacheViewAuthenticPixels(tint_view,0,y,tint_image->columns,1,
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

      double
        weight;

      GetPixelInfo(image,&pixel);
      weight=QuantumScale*GetPixelRed(image,p)-0.5;
      pixel.red=(MagickRealType) GetPixelRed(image,p)+color_vector.red*
        (1.0-(4.0*(weight*weight)));
      weight=QuantumScale*GetPixelGreen(image,p)-0.5;
      pixel.green=(MagickRealType) GetPixelGreen(image,p)+color_vector.green*
        (1.0-(4.0*(weight*weight)));
      weight=QuantumScale*GetPixelBlue(image,p)-0.5;
      pixel.blue=(MagickRealType) GetPixelBlue(image,p)+color_vector.blue*
        (1.0-(4.0*(weight*weight)));
      weight=QuantumScale*GetPixelBlack(image,p)-0.5;
      pixel.black=(MagickRealType) GetPixelBlack(image,p)+color_vector.black*
        (1.0-(4.0*(weight*weight)));
      pixel.alpha=(MagickRealType) GetPixelAlpha(image,p);
      SetPixelViaPixelInfo(tint_image,&pixel,q);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(tint_image);
    }
    if (SyncCacheViewAuthenticPixels(tint_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,TintImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  tint_view=DestroyCacheView(tint_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    tint_image=DestroyImage(tint_image);
  return(tint_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     V i g n e t t e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  VignetteImage() softens the edges of the image in vignette style.
%
%  The format of the VignetteImage method is:
%
%      Image *VignetteImage(const Image *image,const double radius,
%        const double sigma,const ssize_t x,const ssize_t y,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the pixel neighborhood.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o x, y:  Define the x and y ellipse offset.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *VignetteImage(const Image *image,const double radius,
  const double sigma,const ssize_t x,const ssize_t y,ExceptionInfo *exception)
{
  char
    ellipse[MagickPathExtent];

  DrawInfo
    *draw_info;

  Image
    *canvas,
    *blur_image,
    *oval_image,
    *vignette_image;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  canvas=CloneImage(image,0,0,MagickTrue,exception);
  if (canvas == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(canvas,DirectClass,exception) == MagickFalse)
    {
      canvas=DestroyImage(canvas);
      return((Image *) NULL);
    }
  canvas->alpha_trait=BlendPixelTrait;
  oval_image=CloneImage(canvas,canvas->columns,canvas->rows,MagickTrue,
    exception);
  if (oval_image == (Image *) NULL)
    {
      canvas=DestroyImage(canvas);
      return((Image *) NULL);
    }
  (void) QueryColorCompliance("#000000",AllCompliance,
    &oval_image->background_color,exception);
  (void) SetImageBackgroundColor(oval_image,exception);
  draw_info=CloneDrawInfo((const ImageInfo *) NULL,(const DrawInfo *) NULL);
  (void) QueryColorCompliance("#ffffff",AllCompliance,&draw_info->fill,
    exception);
  (void) QueryColorCompliance("#ffffff",AllCompliance,&draw_info->stroke,
    exception);
  (void) FormatLocaleString(ellipse,MagickPathExtent,"ellipse %g,%g,%g,%g,"
    "0.0,360.0",image->columns/2.0,image->rows/2.0,image->columns/2.0-x,
    image->rows/2.0-y);
  draw_info->primitive=AcquireString(ellipse);
  (void) DrawImage(oval_image,draw_info,exception);
  draw_info=DestroyDrawInfo(draw_info);
  blur_image=BlurImage(oval_image,radius,sigma,exception);
  oval_image=DestroyImage(oval_image);
  if (blur_image == (Image *) NULL)
    {
      canvas=DestroyImage(canvas);
      return((Image *) NULL);
    }
  blur_image->alpha_trait=UndefinedPixelTrait;
  (void) CompositeImage(canvas,blur_image,IntensityCompositeOp,MagickTrue,
    0,0,exception);
  blur_image=DestroyImage(blur_image);
  vignette_image=MergeImageLayers(canvas,FlattenLayer,exception);
  canvas=DestroyImage(canvas);
  if (vignette_image != (Image *) NULL)
    (void) TransformImageColorspace(vignette_image,image->colorspace,exception);
  return(vignette_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     W a v e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WaveImage() creates a "ripple" effect in the image by shifting the pixels
%  vertically along a sine wave whose amplitude and wavelength is specified
%  by the given parameters.
%
%  The format of the WaveImage method is:
%
%      Image *WaveImage(const Image *image,const double amplitude,
%        const double wave_length,const PixelInterpolateMethod method,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o amplitude, wave_length:  Define the amplitude and wave length of the
%      sine wave.
%
%    o interpolate: the pixel interpolation method.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *WaveImage(const Image *image,const double amplitude,
  const double wave_length,const PixelInterpolateMethod method,
  ExceptionInfo *exception)
{
#define WaveImageTag  "Wave/Image"

  CacheView
    *canvas_image_view,
    *wave_view;

  float
    *sine_map;

  Image
    *canvas_image,
    *wave_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  register ssize_t
    i;

  ssize_t
    y;

  /*
    Initialize wave image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  canvas_image=CloneImage(image,0,0,MagickTrue,exception);
  if (canvas_image == (Image *) NULL)
    return((Image *) NULL);
  if ((canvas_image->alpha_trait == UndefinedPixelTrait) &&
      (canvas_image->background_color.alpha != OpaqueAlpha))
    (void) SetImageAlpha(canvas_image,OpaqueAlpha,exception);
  wave_image=CloneImage(canvas_image,canvas_image->columns,(size_t)
    (canvas_image->rows+2.0*fabs(amplitude)),MagickTrue,exception);
  if (wave_image == (Image *) NULL)
    {
      canvas_image=DestroyImage(canvas_image);
      return((Image *) NULL);
    }
  if (SetImageStorageClass(wave_image,DirectClass,exception) == MagickFalse)
    {
      canvas_image=DestroyImage(canvas_image);
      wave_image=DestroyImage(wave_image);
      return((Image *) NULL);
    }
  /*
    Allocate sine map.
  */
  sine_map=(float *) AcquireQuantumMemory((size_t) wave_image->columns,
    sizeof(*sine_map));
  if (sine_map == (float *) NULL)
    {
      canvas_image=DestroyImage(canvas_image);
      wave_image=DestroyImage(wave_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  for (i=0; i < (ssize_t) wave_image->columns; i++)
    sine_map[i]=(float) fabs(amplitude)+amplitude*sin((double)
      ((2.0*MagickPI*i)/wave_length));
  /*
    Wave image.
  */
  status=MagickTrue;
  progress=0;
  canvas_image_view=AcquireVirtualCacheView(canvas_image,exception);
  wave_view=AcquireAuthenticCacheView(wave_image,exception);
  (void) SetCacheViewVirtualPixelMethod(canvas_image_view,
    BackgroundVirtualPixelMethod);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(canvas_image,wave_image,wave_image->rows,1)
#endif
  for (y=0; y < (ssize_t) wave_image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(canvas_image_view,0,y,canvas_image->columns,1,
      exception);
    q=QueueCacheViewAuthenticPixels(wave_view,0,y,wave_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) wave_image->columns; x++)
    {
      status=InterpolatePixelChannels(canvas_image,canvas_image_view,
        wave_image,method,(double) x,(double) (y-sine_map[x]),q,exception);
      if (status == MagickFalse)
        break;
      p+=GetPixelChannels(canvas_image);
      q+=GetPixelChannels(wave_image);
    }
    if (SyncCacheViewAuthenticPixels(wave_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(canvas_image,WaveImageTag,progress,
          canvas_image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  wave_view=DestroyCacheView(wave_view);
  canvas_image_view=DestroyCacheView(canvas_image_view);
  canvas_image=DestroyImage(canvas_image);
  sine_map=(float *) RelinquishMagickMemory(sine_map);
  if (status == MagickFalse)
    wave_image=DestroyImage(wave_image);
  return(wave_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     W a v e l e t D e n o i s e I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WaveletDenoiseImage() removes noise from the image using a wavelet
%  transform.  The wavelet transform is a fast hierarchical scheme for
%  processing an image using a set of consecutive lowpass and high_pass filters,
%  followed by a decimation.  This results in a decomposition into different
%  scales which can be regarded as different “frequency bands”, determined by
%  the mother wavelet.  Adapted from dcraw.c by David Coffin.
%
%  The format of the WaveletDenoiseImage method is:
%
%      Image *WaveletDenoiseImage(const Image *image,const double threshold,
%        const double softness,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: set the threshold for smoothing.
%
%    o softness: attenuate the smoothing threshold.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline void HatTransform(const float *magick_restrict pixels,
  const size_t stride,const size_t extent,const size_t scale,float *kernel)
{
  const float
    *magick_restrict p,
    *magick_restrict q,
    *magick_restrict r;

  register ssize_t
    i;

  p=pixels;
  q=pixels+scale*stride;
  r=pixels+scale*stride;
  for (i=0; i < (ssize_t) scale; i++)
  {
    kernel[i]=0.25f*(*p+(*p)+(*q)+(*r));
    p+=stride;
    q-=stride;
    r+=stride;
  }
  for ( ; i < (ssize_t) (extent-scale); i++)
  {
    kernel[i]=0.25f*(2.0f*(*p)+*(p-scale*stride)+*(p+scale*stride));
    p+=stride;
  }
  q=p-scale*stride;
  r=pixels+stride*(extent-2);
  for ( ; i < (ssize_t) extent; i++)
  {
    kernel[i]=0.25f*(*p+(*p)+(*q)+(*r));
    p+=stride;
    q+=stride;
    r-=stride;
  }
}

MagickExport Image *WaveletDenoiseImage(const Image *image,
  const double threshold,const double softness,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *noise_view;

  float
    *kernel,
    *pixels;

  Image
    *noise_image;

  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  MemoryInfo
    *pixels_info;

  ssize_t
    channel;

  static const float
    noise_levels[] = { 0.8002f, 0.2735f, 0.1202f, 0.0585f, 0.0291f, 0.0152f,
      0.0080f, 0.0044f };

  /*
    Initialize noise image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  noise_image=AccelerateWaveletDenoiseImage(image,threshold,exception);
  if (noise_image != (Image *) NULL)
    return(noise_image);
#endif
  noise_image=CloneImage(image,0,0,MagickTrue,exception);
  if (noise_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(noise_image,DirectClass,exception) == MagickFalse)
    {
      noise_image=DestroyImage(noise_image);
      return((Image *) NULL);
    }
  if (AcquireMagickResource(WidthResource,4*image->columns) == MagickFalse)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  pixels_info=AcquireVirtualMemory(3*image->columns,image->rows*
    sizeof(*pixels));
  kernel=(float *) AcquireQuantumMemory(MagickMax(image->rows,image->columns)+1,
    GetOpenMPMaximumThreads()*sizeof(*kernel));
  if ((pixels_info == (MemoryInfo *) NULL) || (kernel == (float *) NULL))
    {
      if (kernel != (float *) NULL)
        kernel=(float *) RelinquishMagickMemory(kernel);
      if (pixels_info != (MemoryInfo *) NULL)
        pixels_info=RelinquishVirtualMemory(pixels_info);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  pixels=(float *) GetVirtualMemoryBlob(pixels_info);
  status=MagickTrue;
  number_pixels=(MagickSizeType) image->columns*image->rows;
  image_view=AcquireAuthenticCacheView(image,exception);
  noise_view=AcquireAuthenticCacheView(noise_image,exception);
  for (channel=0; channel < (ssize_t) GetPixelChannels(image); channel++)
  {
    register ssize_t
      i;

    size_t
      high_pass,
      low_pass;

    ssize_t
      level,
      y;

    PixelChannel
      pixel_channel;

    PixelTrait
      traits;

    if (status == MagickFalse)
      continue;
    traits=GetPixelChannelTraits(image,(PixelChannel) channel);
    if (traits == UndefinedPixelTrait)
      continue;
    pixel_channel=GetPixelChannelChannel(image,channel);
    if ((pixel_channel != RedPixelChannel) &&
        (pixel_channel != GreenPixelChannel) &&
        (pixel_channel != BluePixelChannel))
      continue;
    /*
      Copy channel from image to wavelet pixel array.
    */
    i=0;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      register const Quantum
        *magick_restrict p;

      ssize_t
        x;

      p=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        pixels[i++]=(float) p[channel];
        p+=GetPixelChannels(image);
      }
    }
    /*
      Low pass filter outputs are called approximation kernel & high pass
      filters are referred to as detail kernel. The detail kernel
      have high values in the noisy parts of the signal.
    */
    high_pass=0;
    for (level=0; level < 5; level++)
    {
      double
        magnitude;

      ssize_t
        x,
        y;

      low_pass=(size_t) (number_pixels*((level & 0x01)+1));
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,1) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        const int
          id = GetOpenMPThreadId();

        register float
          *magick_restrict p,
          *magick_restrict q;

        register ssize_t
          x;

        p=kernel+id*image->columns;
        q=pixels+y*image->columns;
        HatTransform(q+high_pass,1,image->columns,(size_t) (1UL << level),p);
        q+=low_pass;
        for (x=0; x < (ssize_t) image->columns; x++)
          *q++=(*p++);
      }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,1) \
        magick_number_threads(image,image,image->columns,1)
#endif
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        const int
          id = GetOpenMPThreadId();

        register float
          *magick_restrict p,
          *magick_restrict q;

        register ssize_t
          y;

        p=kernel+id*image->rows;
        q=pixels+x+low_pass;
        HatTransform(q,image->columns,image->rows,(size_t) (1UL << level),p);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          *q=(*p++);
          q+=image->columns;
        }
      }
      /*
        To threshold, each coefficient is compared to a threshold value and
        attenuated / shrunk by some factor.
      */
      magnitude=threshold*noise_levels[level];
      for (i=0; i < (ssize_t) number_pixels; ++i)
      {
        pixels[high_pass+i]-=pixels[low_pass+i];
        if (pixels[high_pass+i] < -magnitude)
          pixels[high_pass+i]+=magnitude-softness*magnitude;
        else
          if (pixels[high_pass+i] > magnitude)
            pixels[high_pass+i]-=magnitude-softness*magnitude;
          else
            pixels[high_pass+i]*=softness;
        if (high_pass != 0)
          pixels[i]+=pixels[high_pass+i];
      }
      high_pass=low_pass;
    }
    /*
      Reconstruct image from the thresholded wavelet kernel.
    */
    i=0;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      MagickBooleanType
        sync;

      register Quantum
        *magick_restrict q;

      register ssize_t
        x;

      ssize_t
        offset;

      q=GetCacheViewAuthenticPixels(noise_view,0,y,noise_image->columns,1,
        exception);
      if (q == (Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      offset=GetPixelChannelOffset(noise_image,pixel_channel);
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        MagickRealType
          pixel;

        pixel=(MagickRealType) pixels[i]+pixels[low_pass+i];
        q[offset]=ClampToQuantum(pixel);
        i++;
        q+=GetPixelChannels(noise_image);
      }
      sync=SyncCacheViewAuthenticPixels(noise_view,exception);
      if (sync == MagickFalse)
        status=MagickFalse;
    }
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,AddNoiseImageTag,(MagickOffsetType)
          channel,GetPixelChannels(image));
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  noise_view=DestroyCacheView(noise_view);
  image_view=DestroyCacheView(image_view);
  kernel=(float *) RelinquishMagickMemory(kernel);
  pixels_info=RelinquishVirtualMemory(pixels_info);
  if (status == MagickFalse)
    noise_image=DestroyImage(noise_image);
  return(noise_image);
}

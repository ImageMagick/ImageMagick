/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                 FFFFF  X   X                                %
%                                 F       X X                                 %
%                                 FFF      X                                  %
%                                 F       X X                                 %
%                                 F      X   X                                %
%                                                                             %
%                                                                             %
%                   MagickCore Image Special Effects Methods                  %
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
#include "magick/annotate.h"
#include "magick/artifact.h"
#include "magick/attribute.h"
#include "magick/cache.h"
#include "magick/cache-view.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/composite.h"
#include "magick/decorate.h"
#include "magick/draw.h"
#include "magick/effect.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/fx.h"
#include "magick/fx-private.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/layer.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel-private.h"
#include "magick/property.h"
#include "magick/quantum.h"
#include "magick/quantum-private.h"
#include "magick/random_.h"
#include "magick/random-private.h"
#include "magick/resample.h"
#include "magick/resample-private.h"
#include "magick/resize.h"
#include "magick/shear.h"
#include "magick/splay-tree.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/thread-private.h"
#include "magick/transform.h"
#include "magick/utility.h"

/*
  Define declarations.
*/
#define LeftShiftOperator 0xf5
#define RightShiftOperator 0xf6
#define LessThanEqualOperator 0xf7
#define GreaterThanEqualOperator 0xf8
#define EqualOperator 0xf9
#define NotEqualOperator 0xfa
#define LogicalAndOperator 0xfb
#define LogicalOrOperator 0xfc

struct _FxInfo
{
  const Image
    *images;

  MagickBooleanType
    matte;

  char
    *expression;

  FILE
    *file;

  SplayTreeInfo
    *colors,
    *symbols;

  ResampleFilter
    **resample_filter;

  RandomInfo
    *random_info;

  ExceptionInfo
    *exception;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e F x I n f o                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireFxInfo() allocates the FxInfo structure.
%
%  The format of the AcquireFxInfo method is:
%
%      FxInfo *AcquireFxInfo(Image *image,const char *expression)
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o expression: the expression.
%
*/
MagickExport FxInfo *AcquireFxInfo(const Image *image,const char *expression)
{
  char
    fx_op[2];

  FxInfo
    *fx_info;

  register long
    i;

  fx_info=(FxInfo *) AcquireAlignedMemory(1,sizeof(*fx_info));
  if (fx_info == (FxInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(fx_info,0,sizeof(*fx_info));
  fx_info->exception=AcquireExceptionInfo();
  fx_info->images=image;
  fx_info->matte=image->matte;
  fx_info->colors=NewSplayTree(CompareSplayTreeString,RelinquishMagickMemory,
    RelinquishMagickMemory);
  fx_info->symbols=NewSplayTree(CompareSplayTreeString,RelinquishMagickMemory,
    RelinquishMagickMemory);
  fx_info->resample_filter=(ResampleFilter **) AcquireQuantumMemory(
    GetImageListLength(fx_info->images),sizeof(*fx_info->resample_filter));
  if (fx_info->resample_filter == (ResampleFilter **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  for (i=0; i < (long) GetImageListLength(fx_info->images); i++)
  {
    fx_info->resample_filter[i]=AcquireResampleFilter(GetImageFromList(
      fx_info->images,i),fx_info->exception);
    SetResampleFilter(fx_info->resample_filter[i],PointFilter,1.0);
  }
  fx_info->random_info=AcquireRandomInfo();
  fx_info->expression=ConstantString(expression);
  fx_info->file=stderr;
  (void) SubstituteString(&fx_info->expression," ","");  /* compact string */
  if ((strstr(fx_info->expression,"e+") != (char *) NULL) ||
      (strstr(fx_info->expression,"e-") != (char *) NULL))
    {
      /*
        Convert scientific notation.
      */
      (void) SubstituteString(&fx_info->expression,"0e+","0*10^");
      (void) SubstituteString(&fx_info->expression,"1e+","1*10^");
      (void) SubstituteString(&fx_info->expression,"2e+","2*10^");
      (void) SubstituteString(&fx_info->expression,"3e+","3*10^");
      (void) SubstituteString(&fx_info->expression,"4e+","4*10^");
      (void) SubstituteString(&fx_info->expression,"5e+","5*10^");
      (void) SubstituteString(&fx_info->expression,"6e+","6*10^");
      (void) SubstituteString(&fx_info->expression,"7e+","7*10^");
      (void) SubstituteString(&fx_info->expression,"8e+","8*10^");
      (void) SubstituteString(&fx_info->expression,"9e+","9*10^");
      (void) SubstituteString(&fx_info->expression,"0e-","0*10^-");
      (void) SubstituteString(&fx_info->expression,"1e-","1*10^-");
      (void) SubstituteString(&fx_info->expression,"2e-","2*10^-");
      (void) SubstituteString(&fx_info->expression,"3e-","3*10^-");
      (void) SubstituteString(&fx_info->expression,"4e-","4*10^-");
      (void) SubstituteString(&fx_info->expression,"5e-","5*10^-");
      (void) SubstituteString(&fx_info->expression,"6e-","6*10^-");
      (void) SubstituteString(&fx_info->expression,"7e-","7*10^-");
      (void) SubstituteString(&fx_info->expression,"8e-","8*10^-");
      (void) SubstituteString(&fx_info->expression,"9e-","9*10^-");
    }
  /*
    Convert complex to simple operators.
  */
  fx_op[1]='\0';
  *fx_op=(char) LeftShiftOperator;
  (void) SubstituteString(&fx_info->expression,"<<",fx_op);
  *fx_op=(char) RightShiftOperator;
  (void) SubstituteString(&fx_info->expression,">>",fx_op);
  *fx_op=(char) LessThanEqualOperator;
  (void) SubstituteString(&fx_info->expression,"<=",fx_op);
  *fx_op=(char) GreaterThanEqualOperator;
  (void) SubstituteString(&fx_info->expression,">=",fx_op);
  *fx_op=(char) EqualOperator;
  (void) SubstituteString(&fx_info->expression,"==",fx_op);
  *fx_op=(char) NotEqualOperator;
  (void) SubstituteString(&fx_info->expression,"!=",fx_op);
  *fx_op=(char) LogicalAndOperator;
  (void) SubstituteString(&fx_info->expression,"&&",fx_op);
  *fx_op=(char) LogicalOrOperator;
  (void) SubstituteString(&fx_info->expression,"||",fx_op);
  return(fx_info);
}

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
%        ExceptionInfo *exception)
%      Image *AddNoiseImageChannel(const Image *image,const ChannelType channel,
%        const NoiseType noise_type,ExceptionInfo *exception)
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
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *AddNoiseImage(const Image *image,const NoiseType noise_type,
  ExceptionInfo *exception)
{
  Image
    *noise_image;

  noise_image=AddNoiseImageChannel(image,DefaultChannels,noise_type,exception);
  return(noise_image);
}

MagickExport Image *AddNoiseImageChannel(const Image *image,
  const ChannelType channel,const NoiseType noise_type,ExceptionInfo *exception)
{
#define AddNoiseImageTag  "AddNoise/Image"

  CacheView
    *image_view,
    *noise_view;

  const char
    *option;

  Image
    *noise_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickRealType
    attenuate;

  RandomInfo
    **restrict random_info;

  /*
    Initialize noise image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  noise_image=CloneImage(image,0,0,MagickTrue,exception);
  if (noise_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(noise_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&noise_image->exception);
      noise_image=DestroyImage(noise_image);
      return((Image *) NULL);
    }
  /*
    Add noise in each row.
  */
  attenuate=1.0;
  option=GetImageArtifact(image,"attenuate");
  if (option != (char *) NULL)
    attenuate=StringToDouble(option);
  status=MagickTrue;
  progress=0;
  random_info=AcquireRandomInfoThreadSet();
  image_view=AcquireCacheView(image);
  noise_view=AcquireCacheView(noise_image);
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
      *restrict noise_indexes;

    register long
      id,
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewAuthenticPixels(noise_view,0,y,noise_image->columns,1,
      exception);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    noise_indexes=GetCacheViewAuthenticIndexQueue(noise_view);
    id=GetOpenMPThreadId();
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        q->red=ClampToQuantum(GenerateDifferentialNoise(random_info[id],
          p->red,noise_type,attenuate));
      if ((channel & GreenChannel) != 0)
        q->green=ClampToQuantum(GenerateDifferentialNoise(random_info[id],
          p->green,noise_type,attenuate));
      if ((channel & BlueChannel) != 0)
        q->blue=ClampToQuantum(GenerateDifferentialNoise(random_info[id],
          p->blue,noise_type,attenuate));
      if ((channel & OpacityChannel) != 0)
        q->opacity=ClampToQuantum(GenerateDifferentialNoise(random_info[id],
          p->opacity,noise_type,attenuate));
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        noise_indexes[x]=(IndexPacket) ClampToQuantum(GenerateDifferentialNoise(
          random_info[id],indexes[x],noise_type,attenuate));
      p++;
      q++;
    }
    sync=SyncCacheViewAuthenticPixels(noise_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_AddNoiseImage)
#endif
        proceed=SetImageProgress(image,AddNoiseImageTag,progress++,
          image->rows);
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

  long
    progress,
    y;

  MagickBooleanType
    status;

  /*
    Allocate blue shift image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  shift_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (shift_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(shift_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&shift_image->exception);
      shift_image=DestroyImage(shift_image);
      return((Image *) NULL);
    }
  /*
    Blue-shift DirectClass image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  shift_view=AcquireCacheView(shift_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickBooleanType
      sync;

    MagickPixelPacket
      pixel;

    Quantum
      quantum;

    register const PixelPacket
      *restrict p;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(shift_view,0,y,shift_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (long) image->columns; x++)
    {
      quantum=GetRedPixelComponent(p);
      if (p->green < quantum)
        quantum=GetGreenPixelComponent(p);
      if (p->blue < quantum)
        quantum=GetBluePixelComponent(p);
      pixel.red=0.5*(p->red+factor*quantum);
      pixel.green=0.5*(p->green+factor*quantum);
      pixel.blue=0.5*(p->blue+factor*quantum);
      quantum=GetRedPixelComponent(p);
      if (p->green > quantum)
        quantum=GetGreenPixelComponent(p);
      if (p->blue > quantum)
        quantum=GetBluePixelComponent(p);
      pixel.red=0.5*(pixel.red+factor*quantum);
      pixel.green=0.5*(pixel.green+factor*quantum);
      pixel.blue=0.5*(pixel.blue+factor*quantum);
      SetRedPixelComponent(q,ClampRedPixelComponent(&pixel));
      SetGreenPixelComponent(q,ClampGreenPixelComponent(&pixel));
      SetBluePixelComponent(q,ClampBluePixelComponent(&pixel));
      p++;
      q++;
    }
    sync=SyncCacheViewAuthenticPixels(shift_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_BlueShiftImage)
#endif
        proceed=SetImageProgress(image,BlueShiftImageTag,progress++,
          image->rows);
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
    *clone_image,
    *edge_image;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  clone_image=CloneImage(image,0,0,MagickTrue,exception);
  if (clone_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageType(clone_image,GrayscaleType);
  edge_image=EdgeImage(clone_image,radius,exception);
  clone_image=DestroyImage(clone_image);
  if (edge_image == (Image *) NULL)
    return((Image *) NULL);
  charcoal_image=BlurImage(edge_image,radius,sigma,exception);
  edge_image=DestroyImage(edge_image);
  if (charcoal_image == (Image *) NULL)
    return((Image *) NULL);
  (void) NormalizeImage(charcoal_image);
  (void) NegateImage(charcoal_image,MagickFalse);
  (void) SetImageType(charcoal_image,GrayscaleType);
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
%      Image *ColorizeImage(const Image *image,const char *opacity,
%        const PixelPacket colorize,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o opacity:  A character string indicating the level of opacity as a
%      percentage.
%
%    o colorize: A color value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ColorizeImage(const Image *image,const char *opacity,
  const PixelPacket colorize,ExceptionInfo *exception)
{
#define ColorizeImageTag  "Colorize/Image"

  CacheView
    *colorize_view,
    *image_view;

  GeometryInfo
    geometry_info;

  Image
    *colorize_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    pixel;

  MagickStatusType
    flags;

  /*
    Allocate colorized image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  colorize_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (colorize_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(colorize_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&colorize_image->exception);
      colorize_image=DestroyImage(colorize_image);
      return((Image *) NULL);
    }
  if (opacity == (const char *) NULL)
    return(colorize_image);
  /*
    Determine RGB values of the pen color.
  */
  flags=ParseGeometry(opacity,&geometry_info);
  pixel.red=geometry_info.rho;
  pixel.green=geometry_info.rho;
  pixel.blue=geometry_info.rho;
  pixel.opacity=(MagickRealType) OpaqueOpacity;
  if ((flags & SigmaValue) != 0)
    pixel.green=geometry_info.sigma;
  if ((flags & XiValue) != 0)
    pixel.blue=geometry_info.xi;
  if ((flags & PsiValue) != 0)
    pixel.opacity=geometry_info.psi;
  /*
    Colorize DirectClass image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  colorize_view=AcquireCacheView(colorize_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const PixelPacket
      *restrict p;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(colorize_view,0,y,colorize_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (long) image->columns; x++)
    {
      q->red=(Quantum) ((p->red*(100.0-pixel.red)+
        colorize.red*pixel.red)/100.0);
      q->green=(Quantum) ((p->green*(100.0-pixel.green)+
        colorize.green*pixel.green)/100.0);
      q->blue=(Quantum) ((p->blue*(100.0-pixel.blue)+
        colorize.blue*pixel.blue)/100.0);
      q->opacity=(Quantum) ((p->opacity*(100.0-pixel.opacity)+
        colorize.opacity*pixel.opacity)/100.0);
      p++;
      q++;
    }
    sync=SyncCacheViewAuthenticPixels(colorize_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ColorizeImage)
#endif
        proceed=SetImageProgress(image,ColorizeImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  colorize_view=DestroyCacheView(colorize_view);
  if (status == MagickFalse)
    colorize_image=DestroyImage(colorize_image);
  return(colorize_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y F x I n f o                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyFxInfo() deallocates memory associated with an FxInfo structure.
%
%  The format of the DestroyFxInfo method is:
%
%      ImageInfo *DestroyFxInfo(ImageInfo *fx_info)
%
%  A description of each parameter follows:
%
%    o fx_info: the fx info.
%
*/
MagickExport FxInfo *DestroyFxInfo(FxInfo *fx_info)
{
  register long
    i;

  fx_info->exception=DestroyExceptionInfo(fx_info->exception);
  fx_info->expression=DestroyString(fx_info->expression);
  fx_info->symbols=DestroySplayTree(fx_info->symbols);
  fx_info->colors=DestroySplayTree(fx_info->colors);
  for (i=0; i < (long) GetImageListLength(fx_info->images); i++)
    fx_info->resample_filter[i]=DestroyResampleFilter(
      fx_info->resample_filter[i]);
  fx_info->resample_filter=(ResampleFilter **) RelinquishMagickMemory(
    fx_info->resample_filter);
  fx_info->random_info=DestroyRandomInfo(fx_info->random_info);
  fx_info=(FxInfo *) RelinquishMagickMemory(fx_info);
  return(fx_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     F x E v a l u a t e C h a n n e l E x p r e s s i o n                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FxEvaluateChannelExpression() evaluates an expression and returns the
%  results.
%
%  The format of the FxEvaluateExpression method is:
%
%      MagickRealType FxEvaluateChannelExpression(FxInfo *fx_info,
%        const ChannelType channel,const long x,const long y,
%        MagickRealType *alpha,Exceptioninfo *exception)
%      MagickRealType FxEvaluateExpression(FxInfo *fx_info,
%        MagickRealType *alpha,Exceptioninfo *exception)
%
%  A description of each parameter follows:
%
%    o fx_info: the fx info.
%
%    o channel: the channel.
%
%    o x,y: the pixel position.
%
%    o alpha: the result.
%
%    o exception: return any errors or warnings in this structure.
%
*/

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

static MagickRealType FxChannelStatistics(FxInfo *fx_info,const Image *image,
  ChannelType channel,const char *symbol,ExceptionInfo *exception)
{
  char
    key[MaxTextExtent],
    statistic[MaxTextExtent];

  const char
    *value;

  register const char
    *p;

  for (p=symbol; (*p != '.') && (*p != '\0'); p++) ;
  if (*p == '.')
    switch (*++p)  /* e.g. depth.r */
    {
      case 'r': channel=RedChannel; break;
      case 'g': channel=GreenChannel; break;
      case 'b': channel=BlueChannel; break;
      case 'c': channel=CyanChannel; break;
      case 'm': channel=MagentaChannel; break;
      case 'y': channel=YellowChannel; break;
      case 'k': channel=BlackChannel; break;
      default: break;
    }
  (void) FormatMagickString(key,MaxTextExtent,"%p.%ld.%s",(void *) image,
    (long) channel,symbol);
  value=(const char *) GetValueFromSplayTree(fx_info->symbols,key);
  if (value != (const char *) NULL)
    return(QuantumScale*StringToDouble(value));
  (void) DeleteNodeFromSplayTree(fx_info->symbols,key);
  if (LocaleNCompare(symbol,"depth",5) == 0)
    {
      unsigned long
        depth;

      depth=GetImageChannelDepth(image,channel,exception);
      (void) FormatMagickString(statistic,MaxTextExtent,"%lu",depth);
    }
  if (LocaleNCompare(symbol,"kurtosis",8) == 0)
    {
      double
        kurtosis,
        skewness;

      (void) GetImageChannelKurtosis(image,channel,&kurtosis,&skewness,
        exception);
      (void) FormatMagickString(statistic,MaxTextExtent,"%g",kurtosis);
    }
  if (LocaleNCompare(symbol,"maxima",6) == 0)
    {
      double
        maxima,
        minima;

      (void) GetImageChannelRange(image,channel,&minima,&maxima,exception);
      (void) FormatMagickString(statistic,MaxTextExtent,"%g",maxima);
    }
  if (LocaleNCompare(symbol,"mean",4) == 0)
    {
      double
        mean,
        standard_deviation;

      (void) GetImageChannelMean(image,channel,&mean,&standard_deviation,
        exception);
      (void) FormatMagickString(statistic,MaxTextExtent,"%g",mean);
    }
  if (LocaleNCompare(symbol,"minima",6) == 0)
    {
      double
        maxima,
        minima;

      (void) GetImageChannelRange(image,channel,&minima,&maxima,exception);
      (void) FormatMagickString(statistic,MaxTextExtent,"%g",minima);
    }
  if (LocaleNCompare(symbol,"skewness",8) == 0)
    {
      double
        kurtosis,
        skewness;

      (void) GetImageChannelKurtosis(image,channel,&kurtosis,&skewness,
        exception);
      (void) FormatMagickString(statistic,MaxTextExtent,"%g",skewness);
    }
  if (LocaleNCompare(symbol,"standard_deviation",18) == 0)
    {
      double
        mean,
        standard_deviation;

      (void) GetImageChannelMean(image,channel,&mean,&standard_deviation,
        exception);
      (void) FormatMagickString(statistic,MaxTextExtent,"%g",
        standard_deviation);
    }
  (void) AddValueToSplayTree(fx_info->symbols,ConstantString(key),
    ConstantString(statistic));
  return(QuantumScale*StringToDouble(statistic));
}

static MagickRealType
  FxEvaluateSubexpression(FxInfo *,const ChannelType,const long,const long,
    const char *,MagickRealType *,ExceptionInfo *);

static inline MagickRealType FxMax(FxInfo *fx_info,const ChannelType channel,
  const long x,const long y,const char *expression,ExceptionInfo *exception)
{
  MagickRealType
    alpha,
    beta;

  alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression,&beta,exception);
  return((MagickRealType) MagickMax((double) alpha,(double) beta));
}

static inline MagickRealType FxMin(FxInfo *fx_info,ChannelType channel,
  const long x,const long y,const char *expression,ExceptionInfo *exception)
{
  MagickRealType
    alpha,
    beta;

  alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression,&beta,exception);
  return((MagickRealType) MagickMin((double) alpha,(double) beta));
}

static inline const char *FxSubexpression(const char *expression,
  ExceptionInfo *exception)
{
  const char
    *subexpression;

  register long
    level;

  level=0;
  subexpression=expression;
  while ((*subexpression != '\0') &&
         ((level != 1) || (strchr(")",(int) *subexpression) == (char *) NULL)))
  {
    if (strchr("(",(int) *subexpression) != (char *) NULL)
      level++;
    else
      if (strchr(")",(int) *subexpression) != (char *) NULL)
        level--;
    subexpression++;
  }
  if (*subexpression == '\0')
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "UnbalancedParenthesis","`%s'",expression);
  return(subexpression);
}

static MagickRealType FxGetSymbol(FxInfo *fx_info,const ChannelType channel,
  const long x,const long y,const char *expression,ExceptionInfo *exception)
{
  char
    *q,
    subexpression[MaxTextExtent],
    symbol[MaxTextExtent];

  const char
    *p,
    *value;

  Image
    *image;

  MagickPixelPacket
    pixel;

  MagickRealType
    alpha,
    beta;

  PointInfo
    point;

  register long
    i;

  size_t
    length;

  unsigned long
    level;

  p=expression;
  i=GetImageIndexInList(fx_info->images);
  level=0;
  point.x=(double) x;
  point.y=(double) y;
  if (isalpha((int) *(p+1)) == 0)
    {
      if (strchr("suv",(int) *p) != (char *) NULL)
        {
          switch (*p)
          {
            case 's':
            default:
            {
              i=GetImageIndexInList(fx_info->images);
              break;
            }
            case 'u': i=0; break;
            case 'v': i=1; break;
          }
          p++;
          if (*p == '[')
            {
              level++;
              q=subexpression;
              for (p++; *p != '\0'; )
              {
                if (*p == '[')
                  level++;
                else
                  if (*p == ']')
                    {
                      level--;
                      if (level == 0)
                        break;
                    }
                *q++=(*p++);
              }
              *q='\0';
              alpha=FxEvaluateSubexpression(fx_info,channel,x,y,subexpression,
                &beta,exception);
              i=(long) (alpha+0.5);
              p++;
            }
          if (*p == '.')
            p++;
        }
      if ((isalpha((int) *(p+1)) == 0) && (*p == 'p'))
        {
          p++;
          if (*p == '{')
            {
              level++;
              q=subexpression;
              for (p++; *p != '\0'; )
              {
                if (*p == '{')
                  level++;
                else
                  if (*p == '}')
                    {
                      level--;
                      if (level == 0)
                        break;
                    }
                *q++=(*p++);
              }
              *q='\0';
              alpha=FxEvaluateSubexpression(fx_info,channel,x,y,subexpression,
                &beta,exception);
              point.x=alpha;
              point.y=beta;
              p++;
            }
          else
            if (*p == '[')
              {
                level++;
                q=subexpression;
                for (p++; *p != '\0'; )
                {
                  if (*p == '[')
                    level++;
                  else
                    if (*p == ']')
                      {
                        level--;
                        if (level == 0)
                          break;
                      }
                  *q++=(*p++);
                }
                *q='\0';
                alpha=FxEvaluateSubexpression(fx_info,channel,x,y,subexpression,
                  &beta,exception);
                point.x+=alpha;
                point.y+=beta;
                p++;
              }
          if (*p == '.')
            p++;
        }
    }
  length=GetImageListLength(fx_info->images);
  while (i < 0)
    i+=(long) length;
  i%=length;
  image=GetImageFromList(fx_info->images,i);
  if (image == (Image *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "NoSuchImage","`%s'",expression);
      return(0.0);
    }
  (void) ResamplePixelColor(fx_info->resample_filter[i],point.x,point.y,&pixel);
  if ((strlen(p) > 2) &&
      (LocaleCompare(p,"intensity") != 0) &&
      (LocaleCompare(p,"luminance") != 0) &&
      (LocaleCompare(p,"hue") != 0) &&
      (LocaleCompare(p,"saturation") != 0) &&
      (LocaleCompare(p,"lightness") != 0))
    {
      char
        name[MaxTextExtent];

      (void) CopyMagickString(name,p,MaxTextExtent);
      for (q=name+(strlen(name)-1); q > name; q--)
      {
        if (*q == ')')
          break;
        if (*q == '.')
          {
            *q='\0';
            break;
          }
      }
      if ((strlen(name) > 2) &&
          (GetValueFromSplayTree(fx_info->symbols,name) == (const char *) NULL))
        {
          MagickPixelPacket
            *color;

          color=(MagickPixelPacket *) GetValueFromSplayTree(fx_info->colors,
            name);
          if (color != (MagickPixelPacket *) NULL)
            {
              pixel=(*color);
              p+=strlen(name);
            }
          else
            if (QueryMagickColor(name,&pixel,fx_info->exception) != MagickFalse)
              {
                (void) AddValueToSplayTree(fx_info->colors,ConstantString(name),
                  CloneMagickPixelPacket(&pixel));
                p+=strlen(name);
              }
        }
    }
  (void) CopyMagickString(symbol,p,MaxTextExtent);
  StripString(symbol);
  if (*symbol == '\0')
    {
      switch (channel)
      {
        case RedChannel: return(QuantumScale*pixel.red);
        case GreenChannel: return(QuantumScale*pixel.green);
        case BlueChannel: return(QuantumScale*pixel.blue);
        case OpacityChannel:
        {
          if (pixel.matte == MagickFalse)
            {
              fx_info->matte=MagickFalse;
              return(1.0);
            }
          return((MagickRealType) (QuantumScale*GetAlphaPixelComponent(&pixel)));
        }
        case IndexChannel:
        {
          if (image->colorspace != CMYKColorspace)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"ColorSeparatedImageRequired","`%s'",
                image->filename);
              return(0.0);
            }
          return(QuantumScale*pixel.index);
        }
        default:
          break;
      }
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "UnableToParseExpression","`%s'",p);
      return(0.0);
    }
  switch (*symbol)
  {
    case 'A':
    case 'a':
    {
      if (LocaleCompare(symbol,"a") == 0)
        return((MagickRealType) (QuantumScale*GetAlphaPixelComponent(&pixel)));
      break;
    }
    case 'B':
    case 'b':
    {
      if (LocaleCompare(symbol,"b") == 0)
        return(QuantumScale*pixel.blue);
      break;
    }
    case 'C':
    case 'c':
    {
      if (LocaleNCompare(symbol,"channel",7) == 0)
        {
          GeometryInfo
            channel_info;

          MagickStatusType
            flags;

          flags=ParseGeometry(symbol+7,&channel_info);
          if (image->colorspace == CMYKColorspace)
            switch (channel)
            {
              case CyanChannel:
              {
                if ((flags & RhoValue) == 0)
                  return(0.0);
                return(channel_info.rho);
              }
              case MagentaChannel:
              {
                if ((flags & SigmaValue) == 0)
                  return(0.0);
                return(channel_info.sigma);
              }
              case YellowChannel:
              {
                if ((flags & XiValue) == 0)
                  return(0.0);
                return(channel_info.xi);
              }
              case BlackChannel:
              {
                if ((flags & PsiValue) == 0)
                  return(0.0);
                return(channel_info.psi);
              }
              case OpacityChannel:
              {
                if ((flags & ChiValue) == 0)
                  return(0.0);
                return(channel_info.chi);
              }
              default:
                return(0.0);
            }
          switch (channel)
          {
            case RedChannel:
            {
              if ((flags & RhoValue) == 0)
                return(0.0);
              return(channel_info.rho);
            }
            case GreenChannel:
            {
              if ((flags & SigmaValue) == 0)
                return(0.0);
              return(channel_info.sigma);
            }
            case BlueChannel:
            {
              if ((flags & XiValue) == 0)
                return(0.0);
              return(channel_info.xi);
            }
            case OpacityChannel:
            {
              if ((flags & PsiValue) == 0)
                return(0.0);
              return(channel_info.psi);
            }
            case IndexChannel:
            {
              if ((flags & ChiValue) == 0)
                return(0.0);
              return(channel_info.chi);
            }
            default:
              return(0.0);
          }
          return(0.0);
        }
      if (LocaleCompare(symbol,"c") == 0)
        return(QuantumScale*pixel.red);
      break;
    }
    case 'D':
    case 'd':
    {
      if (LocaleNCompare(symbol,"depth",5) == 0)
        return(FxChannelStatistics(fx_info,image,channel,symbol,exception));
      break;
    }
    case 'G':
    case 'g':
    {
      if (LocaleCompare(symbol,"g") == 0)
        return(QuantumScale*pixel.green);
      break;
    }
    case 'K':
    case 'k':
    {
      if (LocaleNCompare(symbol,"kurtosis",8) == 0)
        return(FxChannelStatistics(fx_info,image,channel,symbol,exception));
      if (LocaleCompare(symbol,"k") == 0)
        {
          if (image->colorspace != CMYKColorspace)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"ColorSeparatedImageRequired","`%s'",
                image->filename);
              return(0.0);
            }
          return(QuantumScale*pixel.index);
        }
      break;
    }
    case 'H':
    case 'h':
    {
      if (LocaleCompare(symbol,"h") == 0)
        return((MagickRealType) image->rows);
      if (LocaleCompare(symbol,"hue") == 0)
        {
          double
            hue,
            lightness,
            saturation;

          ConvertRGBToHSL(ClampToQuantum(pixel.red),ClampToQuantum(pixel.green),
            ClampToQuantum(pixel.blue),&hue,&saturation,&lightness);
          return(hue);
        }
      break;
    }
    case 'I':
    case 'i':
    {
      if ((LocaleCompare(symbol,"image.depth") == 0) ||
          (LocaleCompare(symbol,"image.minima") == 0) ||
          (LocaleCompare(symbol,"image.maxima") == 0) ||
          (LocaleCompare(symbol,"image.mean") == 0) ||
          (LocaleCompare(symbol,"image.kurtosis") == 0) ||
          (LocaleCompare(symbol,"image.skewness") == 0) ||
          (LocaleCompare(symbol,"image.standard_deviation") == 0))
        return(FxChannelStatistics(fx_info,image,channel,symbol+6,exception));
      if (LocaleCompare(symbol,"image.resolution.x") == 0)
        return(image->x_resolution);
      if (LocaleCompare(symbol,"image.resolution.y") == 0)
        return(image->y_resolution);
      if (LocaleCompare(symbol,"intensity") == 0)
        return(QuantumScale*MagickPixelIntensityToQuantum(&pixel));
      if (LocaleCompare(symbol,"i") == 0)
        return((MagickRealType) x);
      break;
    }
    case 'J':
    case 'j':
    {
      if (LocaleCompare(symbol,"j") == 0)
        return((MagickRealType) y);
      break;
    }
    case 'L':
    case 'l':
    {
      if (LocaleCompare(symbol,"lightness") == 0)
        {
          double
            hue,
            lightness,
            saturation;

          ConvertRGBToHSL(ClampToQuantum(pixel.red),ClampToQuantum(pixel.green),
            ClampToQuantum(pixel.blue),&hue,&saturation,&lightness);
          return(lightness);
        }
      if (LocaleCompare(symbol,"luminance") == 0)
        {
          double
            luminence;

          luminence=0.2126*pixel.red+0.7152*pixel.green+0.0722*pixel.blue;
          return(QuantumScale*luminence);
        }
      break;
    }
    case 'M':
    case 'm':
    {
      if (LocaleNCompare(symbol,"maxima",6) == 0)
        return(FxChannelStatistics(fx_info,image,channel,symbol,exception));
      if (LocaleNCompare(symbol,"mean",4) == 0)
        return(FxChannelStatistics(fx_info,image,channel,symbol,exception));
      if (LocaleNCompare(symbol,"minima",6) == 0)
        return(FxChannelStatistics(fx_info,image,channel,symbol,exception));
      if (LocaleCompare(symbol,"m") == 0)
        return(QuantumScale*pixel.blue);
      break;
    }
    case 'N':
    case 'n':
    {
      if (LocaleCompare(symbol,"n") == 0)
        return((MagickRealType) GetImageListLength(fx_info->images));
      break;
    }
    case 'O':
    case 'o':
    {
      if (LocaleCompare(symbol,"o") == 0)
        return(QuantumScale*pixel.opacity);
      break;
    }
    case 'P':
    case 'p':
    {
      if (LocaleCompare(symbol,"page.height") == 0)
        return((MagickRealType) image->page.height);
      if (LocaleCompare(symbol,"page.width") == 0)
        return((MagickRealType) image->page.width);
      if (LocaleCompare(symbol,"page.x") == 0)
        return((MagickRealType) image->page.x);
      if (LocaleCompare(symbol,"page.y") == 0)
        return((MagickRealType) image->page.y);
      break;
    }
    case 'R':
    case 'r':
    {
      if (LocaleCompare(symbol,"resolution.x") == 0)
        return(image->x_resolution);
      if (LocaleCompare(symbol,"resolution.y") == 0)
        return(image->y_resolution);
      if (LocaleCompare(symbol,"r") == 0)
        return(QuantumScale*pixel.red);
      break;
    }
    case 'S':
    case 's':
    {
      if (LocaleCompare(symbol,"saturation") == 0)
        {
          double
            hue,
            lightness,
            saturation;

          ConvertRGBToHSL(ClampToQuantum(pixel.red),ClampToQuantum(pixel.green),
            ClampToQuantum(pixel.blue),&hue,&saturation,&lightness);
          return(saturation);
        }
      if (LocaleNCompare(symbol,"skewness",8) == 0)
        return(FxChannelStatistics(fx_info,image,channel,symbol,exception));
      if (LocaleNCompare(symbol,"standard_deviation",18) == 0)
        return(FxChannelStatistics(fx_info,image,channel,symbol,exception));
      break;
    }
    case 'T':
    case 't':
    {
      if (LocaleCompare(symbol,"t") == 0)
        return((MagickRealType) fx_info->images->scene);
      break;
    }
    case 'W':
    case 'w':
    {
      if (LocaleCompare(symbol,"w") == 0)
        return((MagickRealType) image->columns);
      break;
    }
    case 'Y':
    case 'y':
    {
      if (LocaleCompare(symbol,"y") == 0)
        return(QuantumScale*pixel.green);
      break;
    }
    case 'Z':
    case 'z':
    {
      if (LocaleCompare(symbol,"z") == 0)
        {
          MagickRealType
            depth;

          depth=(MagickRealType) GetImageChannelDepth(image,channel,
            fx_info->exception);
          return(depth);
        }
      break;
    }
    default:
      break;
  }
  value=(const char *) GetValueFromSplayTree(fx_info->symbols,symbol);
  if (value != (const char *) NULL)
    return((MagickRealType) StringToDouble(value));
  (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
    "UnableToParseExpression","`%s'",symbol);
  return(0.0);
}

static const char *FxOperatorPrecedence(const char *expression,
  ExceptionInfo *exception)
{
  typedef enum
  {
    UndefinedPrecedence,
    NullPrecedence,
    BitwiseComplementPrecedence,
    ExponentPrecedence,
    MultiplyPrecedence,
    AdditionPrecedence,
    ShiftPrecedence,
    RelationalPrecedence,
    EquivalencyPrecedence,
    BitwiseAndPrecedence,
    BitwiseOrPrecedence,
    LogicalAndPrecedence,
    LogicalOrPrecedence,
    TernaryPrecedence,
    AssignmentPrecedence,
    CommaPrecedence,
    SeparatorPrecedence
  } FxPrecedence;

  FxPrecedence
    precedence,
    target;

  register const char
    *subexpression;

  register int
    c;

  unsigned long
    level;

  c=0;
  level=0;
  subexpression=(const char *) NULL;
  target=NullPrecedence;
  while (*expression != '\0')
  {
    precedence=UndefinedPrecedence;
    if ((isspace((int) ((char) *expression)) != 0) || (c == (int) '@'))
      {
        expression++;
        continue;
      }
    switch (*expression)
    {
      case 'A':
      case 'a':
      {
        if (LocaleNCompare(expression,"atan2",5) == 0)
          {
            expression+=5;
            break;
          }
        break;
      }
      case 'J':
      case 'j':
      {
        if ((LocaleNCompare(expression,"j0",2) == 0) ||
            (LocaleNCompare(expression,"j1",2) == 0))
          {
            expression+=2;
            break;
          }
        break;
      }
      default:
        break;
    }
    if ((c == (int) '{') || (c == (int) '['))
      level++;
    else
      if ((c == (int) '}') || (c == (int) ']'))
        level--;
    if (level == 0)
      switch ((unsigned char) *expression)
      {
        case '~':
        case '!':
        {
          precedence=BitwiseComplementPrecedence;
          break;
        }
        case '^':
        {
          precedence=ExponentPrecedence;
          break;
        }
        default:
        {
          if (((c != 0) && ((isdigit((int) ((char) c)) != 0) ||
               (strchr(")",c) != (char *) NULL))) &&
              (((islower((int) ((char) *expression)) != 0) ||
               (strchr("(",(int) *expression) != (char *) NULL)) ||
               ((isdigit((int) ((char) c)) == 0) &&
                (isdigit((int) ((char) *expression)) != 0))) &&
              (strchr("xy",(int) *expression) == (char *) NULL))
            precedence=MultiplyPrecedence;
          break;
        }
        case '*':
        case '/':
        case '%':
        {
          precedence=MultiplyPrecedence;
          break;
        }
        case '+':
        case '-':
        {
          if ((strchr("(+-/*%:&^|<>~,",c) == (char *) NULL) ||
              (isalpha(c) != 0))
            precedence=AdditionPrecedence;
          break;
        }
        case LeftShiftOperator:
        case RightShiftOperator:
        {
          precedence=ShiftPrecedence;
          break;
        }
        case '<':
        case LessThanEqualOperator:
        case GreaterThanEqualOperator:
        case '>':
        {
          precedence=RelationalPrecedence;
          break;
        }
        case EqualOperator:
        case NotEqualOperator:
        {
          precedence=EquivalencyPrecedence;
          break;
        }
        case '&':
        {
          precedence=BitwiseAndPrecedence;
          break;
        }
        case '|':
        {
          precedence=BitwiseOrPrecedence;
          break;
        }
        case LogicalAndOperator:
        {
          precedence=LogicalAndPrecedence;
          break;
        }
        case LogicalOrOperator:
        {
          precedence=LogicalOrPrecedence;
          break;
        }
        case ':':
        case '?':
        {
          precedence=TernaryPrecedence;
          break;
        }
        case '=':
        {
          precedence=AssignmentPrecedence;
          break;
        }
        case ',':
        {
          precedence=CommaPrecedence;
          break;
        }
        case ';':
        {
          precedence=SeparatorPrecedence;
          break;
        }
      }
    if ((precedence == BitwiseComplementPrecedence) ||
        (precedence == TernaryPrecedence) ||
        (precedence == AssignmentPrecedence))
      {
        if (precedence > target)
          {
            /*
              Right-to-left associativity.
            */
            target=precedence;
            subexpression=expression;
          }
      }
    else
      if (precedence >= target)
        {
          /*
            Left-to-right associativity.
          */
          target=precedence;
          subexpression=expression;
        }
    if (strchr("(",(int) *expression) != (char *) NULL)
      expression=FxSubexpression(expression,exception);
    c=(int) (*expression++);
  }
  return(subexpression);
}

static MagickRealType FxEvaluateSubexpression(FxInfo *fx_info,
  const ChannelType channel,const long x,const long y,const char *expression,
  MagickRealType *beta,ExceptionInfo *exception)
{
  char
    *q,
    subexpression[MaxTextExtent];

  MagickRealType
    alpha,
    gamma;

  register const char
    *p;

  *beta=0.0;
  if (exception->severity != UndefinedException)
    return(0.0);
  while (isspace((int) *expression) != 0)
    expression++;
  if (*expression == '\0')
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "MissingExpression","`%s'",expression);
      return(0.0);
    }
  p=FxOperatorPrecedence(expression,exception);
  if (p != (const char *) NULL)
    {
      (void) CopyMagickString(subexpression,expression,(size_t)
        (p-expression+1));
      alpha=FxEvaluateSubexpression(fx_info,channel,x,y,subexpression,beta,
        exception);
      switch ((unsigned char) *p)
      {
        case '~':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          *beta=(MagickRealType) (~(unsigned long) *beta);
          return(*beta);
        }
        case '!':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          return(*beta == 0.0 ? 1.0 : 0.0);
        }
        case '^':
        {
          *beta=pow((double) alpha,(double) FxEvaluateSubexpression(fx_info,
            channel,x,y,++p,beta,exception));
          return(*beta);
        }
        case '*':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          return(alpha*(*beta));
        }
        case '/':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          if (*beta == 0.0)
            {
              if (exception->severity == UndefinedException)
                (void) ThrowMagickException(exception,GetMagickModule(),
                  OptionError,"DivideByZero","`%s'",expression);
              return(0.0);
            }
          return(alpha/(*beta));
        }
        case '%':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          *beta=fabs(floor(((double) *beta)+0.5));
          if (*beta == 0.0)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"DivideByZero","`%s'",expression);
              return(0.0);
            }
          return(fmod((double) alpha,(double) *beta));
        }
        case '+':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          return(alpha+(*beta));
        }
        case '-':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          return(alpha-(*beta));
        }
        case LeftShiftOperator:
        {
          gamma=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          *beta=(MagickRealType) ((unsigned long) (alpha+0.5) << (unsigned long)
            (gamma+0.5));
          return(*beta);
        }
        case RightShiftOperator:
        {
          gamma=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          *beta=(MagickRealType) ((unsigned long) (alpha+0.5) >> (unsigned long)
            (gamma+0.5));
          return(*beta);
        }
        case '<':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          return(alpha < *beta ? 1.0 : 0.0);
        }
        case LessThanEqualOperator:
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          return(alpha <= *beta ? 1.0 : 0.0);
        }
        case '>':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          return(alpha > *beta ? 1.0 : 0.0);
        }
        case GreaterThanEqualOperator:
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          return(alpha >= *beta ? 1.0 : 0.0);
        }
        case EqualOperator:
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          return(fabs(alpha-(*beta)) <= MagickEpsilon ? 1.0 : 0.0);
        }
        case NotEqualOperator:
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          return(fabs(alpha-(*beta)) > MagickEpsilon ? 1.0 : 0.0);
        }
        case '&':
        {
          gamma=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          *beta=(MagickRealType) ((unsigned long) (alpha+0.5) & (unsigned long)
            (gamma+0.5));
          return(*beta);
        }
        case '|':
        {
          gamma=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          *beta=(MagickRealType) ((unsigned long) (alpha+0.5) | (unsigned long)
            (gamma+0.5));
          return(*beta);
        }
        case LogicalAndOperator:
        {
          gamma=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          *beta=(alpha > 0.0) && (gamma > 0.0) ? 1.0 : 0.0;
          return(*beta);
        }
        case LogicalOrOperator:
        {
          gamma=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          *beta=(alpha > 0.0) || (gamma > 0.0) ? 1.0 : 0.0;
          return(*beta);
        }
        case '?':
        {
          MagickRealType
            gamma;

          (void) CopyMagickString(subexpression,++p,MaxTextExtent);
          q=subexpression;
          p=StringToken(":",&q);
          if (q == (char *) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              return(0.0);
            }
          if (fabs((double) alpha) > MagickEpsilon)
            gamma=FxEvaluateSubexpression(fx_info,channel,x,y,p,beta,exception);
          else
            gamma=FxEvaluateSubexpression(fx_info,channel,x,y,q,beta,exception);
          return(gamma);
        }
        case '=':
        {
          char
            numeric[MaxTextExtent];

          q=subexpression;
          while (isalpha((int) ((unsigned char) *q)) != 0)
            q++;
          if (*q != '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              return(0.0);
            }
          ClearMagickException(exception);
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          (void) FormatMagickString(numeric,MaxTextExtent,"%g",(double)
            *beta);
          (void) DeleteNodeFromSplayTree(fx_info->symbols,subexpression);
          (void) AddValueToSplayTree(fx_info->symbols,ConstantString(
            subexpression),ConstantString(numeric));
          return(*beta);
        }
        case ',':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          return(alpha);
        }
        case ';':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,beta,exception);
          return(*beta);
        }
        default:
        {
          gamma=alpha*FxEvaluateSubexpression(fx_info,channel,x,y,p,beta,
            exception);
          return(gamma);
        }
      }
    }
  if (strchr("(",(int) *expression) != (char *) NULL)
    {
      (void) CopyMagickString(subexpression,expression+1,MaxTextExtent);
      subexpression[strlen(subexpression)-1]='\0';
      gamma=FxEvaluateSubexpression(fx_info,channel,x,y,subexpression,beta,
        exception);
      return(gamma);
    }
  switch (*expression)
  {
    case '+':
    {
      gamma=FxEvaluateSubexpression(fx_info,channel,x,y,expression+1,beta,
        exception);
      return(1.0*gamma);
    }
    case '-':
    {
      gamma=FxEvaluateSubexpression(fx_info,channel,x,y,expression+1,beta,
        exception);
      return(-1.0*gamma);
    }
    case '~':
    {
      gamma=FxEvaluateSubexpression(fx_info,channel,x,y,expression+1,beta,
        exception);
      return((MagickRealType) (~(unsigned long) (gamma+0.5)));
    }
    case 'A':
    case 'a':
    {
      if (LocaleNCompare(expression,"abs",3) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+3,beta,
            exception);
          return((MagickRealType) fabs((double) alpha));
        }
      if (LocaleNCompare(expression,"acos",4) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+4,beta,
            exception);
          return((MagickRealType) acos((double) alpha));
        }
      if (LocaleNCompare(expression,"airy",4) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+4,beta,
            exception);
          if (alpha == 0.0)
            return(0.5);
          return((MagickRealType) (j0((double) alpha)/alpha));
        }
      if (LocaleNCompare(expression,"asin",4) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+4,beta,
            exception);
          return((MagickRealType) asin((double) alpha));
        }
      if (LocaleNCompare(expression,"alt",3) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+3,beta,
            exception);
          return(((long) alpha) & 0x01 ? -1.0 : 1.0);
        }
      if (LocaleNCompare(expression,"atan2",5) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+5,beta,
            exception);
          return((MagickRealType) atan2((double) alpha,(double) *beta));
        }
      if (LocaleNCompare(expression,"atan",4) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+4,beta,
            exception);
          return((MagickRealType) atan((double) alpha));
        }
      if (LocaleCompare(expression,"a") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'B':
    case 'b':
    {
      if (LocaleCompare(expression,"b") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'C':
    case 'c':
    {
      if (LocaleNCompare(expression,"ceil",4) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+4,beta,
            exception);
          return((MagickRealType) ceil((double) alpha));
        }
      if (LocaleNCompare(expression,"cosh",4) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+4,beta,
            exception);
          return((MagickRealType) cosh((double) alpha));
        }
      if (LocaleNCompare(expression,"cos",3) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+3,beta,
            exception);
          return((MagickRealType) cos((double) alpha));
        }
      if (LocaleCompare(expression,"c") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'D':
    case 'd':
    {
      if (LocaleNCompare(expression,"debug",5) == 0)
        {
          const char
            *type;

          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+5,beta,
            exception);
          if (fx_info->images->colorspace == CMYKColorspace)
            switch (channel)
            {
              case CyanChannel: type="cyan"; break;
              case MagentaChannel: type="magenta"; break;
              case YellowChannel: type="yellow"; break;
              case OpacityChannel: type="opacity"; break;
              case BlackChannel: type="black"; break;
              default: type="unknown"; break;
            }
          else
            switch (channel)
            {
              case RedChannel: type="red"; break;
              case GreenChannel: type="green"; break;
              case BlueChannel: type="blue"; break;
              case OpacityChannel: type="opacity"; break;
              default: type="unknown"; break;
            }
          (void) CopyMagickString(subexpression,expression+6,MaxTextExtent);
          if (strlen(subexpression) > 1)
            subexpression[strlen(subexpression)-1]='\0';
          if (fx_info->file != (FILE *) NULL)
            (void) fprintf(fx_info->file,"%s[%ld,%ld].%s: %s=%.*g\n",
              fx_info->images->filename,x,y,type,subexpression,
              GetMagickPrecision(),(double) alpha);
          return(0.0);
        }
      break;
    }
    case 'E':
    case 'e':
    {
      if (LocaleCompare(expression,"epsilon") == 0)
        return((MagickRealType) MagickEpsilon);
      if (LocaleNCompare(expression,"exp",3) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+3,beta,
            exception);
          return((MagickRealType) exp((double) alpha));
        }
      if (LocaleCompare(expression,"e") == 0)
        return((MagickRealType) 2.7182818284590452354);
      break;
    }
    case 'F':
    case 'f':
    {
      if (LocaleNCompare(expression,"floor",5) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+5,beta,
            exception);
          return((MagickRealType) floor((double) alpha));
        }
      break;
    }
    case 'G':
    case 'g':
    {
      if (LocaleCompare(expression,"g") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'H':
    case 'h':
    {
      if (LocaleCompare(expression,"h") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      if (LocaleCompare(expression,"hue") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      if (LocaleNCompare(expression,"hypot",5) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+5,beta,
            exception);
          return((MagickRealType) hypot((double) alpha,(double) *beta));
        }
      break;
    }
    case 'K':
    case 'k':
    {
      if (LocaleCompare(expression,"k") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'I':
    case 'i':
    {
      if (LocaleCompare(expression,"intensity") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      if (LocaleNCompare(expression,"int",3) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+3,beta,
            exception);
          return((MagickRealType) floor(alpha+0.5));
        }
      if (LocaleCompare(expression,"i") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'J':
    case 'j':
    {
      if (LocaleCompare(expression,"j") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      if (LocaleNCompare(expression,"j0",2) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+2,beta,
            exception);
          return((MagickRealType) j0((double) alpha));
        }
      if (LocaleNCompare(expression,"j1",2) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+2,beta,
            exception);
          return((MagickRealType) j1((double) alpha));
        }
      if (LocaleNCompare(expression,"jinc",4) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+4,beta,
            exception);
          if (alpha == 0.0)
            return((MagickRealType) (MagickPI/4.0));
          return((MagickRealType) j1((double) (MagickPI*alpha))/(2.0*alpha));
        }
      break;
    }
    case 'L':
    case 'l':
    {
      if (LocaleNCompare(expression,"ln",2) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+2,beta,
            exception);
          return((MagickRealType) log((double) alpha));
        }
      if (LocaleNCompare(expression,"logtwo",4) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+4,beta,
            exception);
          return((MagickRealType) log10((double) alpha))/log10(2.0);
        }
      if (LocaleNCompare(expression,"log",3) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+3,beta,
            exception);
          return((MagickRealType) log10((double) alpha));
        }
      if (LocaleCompare(expression,"lightness") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'M':
    case 'm':
    {
      if (LocaleCompare(expression,"MaxRGB") == 0)
        return((MagickRealType) QuantumRange);
      if (LocaleNCompare(expression,"maxima",6) == 0)
        break;
      if (LocaleNCompare(expression,"max",3) == 0)
        return(FxMax(fx_info,channel,x,y,expression+3,exception));
      if (LocaleNCompare(expression,"minima",6) == 0)
        break;
      if (LocaleNCompare(expression,"min",3) == 0)
        return(FxMin(fx_info,channel,x,y,expression+3,exception));
      if (LocaleNCompare(expression,"mod",3) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+3,beta,
            exception);
          return((MagickRealType) fmod((double) alpha,(double) *beta));
        }
      if (LocaleCompare(expression,"m") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'N':
    case 'n':
    {
      if (LocaleCompare(expression,"n") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'O':
    case 'o':
    {
      if (LocaleCompare(expression,"Opaque") == 0)
        return(1.0);
      if (LocaleCompare(expression,"o") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'P':
    case 'p':
    {
      if (LocaleCompare(expression,"pi") == 0)
        return((MagickRealType) MagickPI);
      if (LocaleNCompare(expression,"pow",3) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+3,beta,
            exception);
          return((MagickRealType) pow((double) alpha,(double) *beta));
        }
      if (LocaleCompare(expression,"p") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'Q':
    case 'q':
    {
      if (LocaleCompare(expression,"QuantumRange") == 0)
        return((MagickRealType) QuantumRange);
      if (LocaleCompare(expression,"QuantumScale") == 0)
        return((MagickRealType) QuantumScale);
      break;
    }
    case 'R':
    case 'r':
    {
      if (LocaleNCompare(expression,"rand",4) == 0)
        return((MagickRealType) GetPseudoRandomValue(fx_info->random_info));
      if (LocaleNCompare(expression,"round",5) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+5,beta,
            exception);
          if (alpha >= 0.0)
            return((MagickRealType) floor((double) alpha+0.5));
          return((MagickRealType) ceil((double) alpha-0.5));
        }
      if (LocaleCompare(expression,"r") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'S':
    case 's':
    {
      if (LocaleCompare(expression,"saturation") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      if (LocaleNCompare(expression,"sign",4) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+4,beta,
            exception);
          return(alpha < 0.0 ? -1.0 : 1.0);
        }
      if (LocaleNCompare(expression,"sinc",4) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+4,beta,
            exception);
          if (alpha == 0)
            return(1.0);
          gamma=(MagickRealType) (sin((double) (MagickPI*alpha))/
            (MagickPI*alpha));
          return(gamma);
        }
      if (LocaleNCompare(expression,"sinh",4) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+4,beta,
            exception);
          return((MagickRealType) sinh((double) alpha));
        }
      if (LocaleNCompare(expression,"sin",3) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+3,beta,
            exception);
          return((MagickRealType) sin((double) alpha));
        }
      if (LocaleNCompare(expression,"sqrt",4) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+4,beta,
            exception);
          return((MagickRealType) sqrt((double) alpha));
        }
      if (LocaleCompare(expression,"s") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'T':
    case 't':
    {
      if (LocaleNCompare(expression,"tanh",4) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+4,beta,
            exception);
          return((MagickRealType) tanh((double) alpha));
        }
      if (LocaleNCompare(expression,"tan",3) == 0)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,expression+3,beta,
            exception);
          return((MagickRealType) tan((double) alpha));
        }
      if (LocaleCompare(expression,"Transparent") == 0)
        return(0.0);
      if (LocaleCompare(expression,"t") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'U':
    case 'u':
    {
      if (LocaleCompare(expression,"u") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'V':
    case 'v':
    {
      if (LocaleCompare(expression,"v") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'W':
    case 'w':
    {
      if (LocaleCompare(expression,"w") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'Y':
    case 'y':
    {
      if (LocaleCompare(expression,"y") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    case 'Z':
    case 'z':
    {
      if (LocaleCompare(expression,"z") == 0)
        return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
      break;
    }
    default:
      break;
  }
  q=(char *) expression;
  alpha=strtod(expression,&q);
  if (q == expression)
    return(FxGetSymbol(fx_info,channel,x,y,expression,exception));
  return(alpha);
}

MagickExport MagickBooleanType FxEvaluateExpression(FxInfo *fx_info,
  MagickRealType *alpha,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=FxEvaluateChannelExpression(fx_info,GrayChannel,0,0,alpha,exception);
  return(status);
}

MagickExport MagickBooleanType FxPreprocessExpression(FxInfo *fx_info,
  MagickRealType *alpha,ExceptionInfo *exception)
{
  FILE
    *file;

  MagickBooleanType
    status;

  file=fx_info->file;
  fx_info->file=(FILE *) NULL;
  status=FxEvaluateChannelExpression(fx_info,GrayChannel,0,0,alpha,exception);
  fx_info->file=file;
  return(status);
}

MagickExport MagickBooleanType FxEvaluateChannelExpression(FxInfo *fx_info,
  const ChannelType channel,const long x,const long y,MagickRealType *alpha,
  ExceptionInfo *exception)
{
  MagickRealType
    beta;

  beta=0.0;
  *alpha=FxEvaluateSubexpression(fx_info,channel,x,y,fx_info->expression,&beta,
    exception);
  return(exception->severity == OptionError ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     F x I m a g e                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FxImage() applies a mathematical expression to the specified image.
%
%  The format of the FxImage method is:
%
%      Image *FxImage(const Image *image,const char *expression,
%        ExceptionInfo *exception)
%      Image *FxImageChannel(const Image *image,const ChannelType channel,
%        const char *expression,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o expression: A mathematical expression.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static FxInfo **DestroyFxThreadSet(FxInfo **fx_info)
{
  register long
    i;

  assert(fx_info != (FxInfo **) NULL);
  for (i=0; i < (long) GetOpenMPMaximumThreads(); i++)
    if (fx_info[i] != (FxInfo *) NULL)
      fx_info[i]=DestroyFxInfo(fx_info[i]);
  fx_info=(FxInfo **) RelinquishAlignedMemory(fx_info);
  return(fx_info);
}

static FxInfo **AcquireFxThreadSet(const Image *image,const char *expression,
  ExceptionInfo *exception)
{
  char
    *fx_expression;

  FxInfo
    **fx_info;

  MagickRealType
    alpha;

  register long
    i;

  unsigned long
    number_threads;

  number_threads=GetOpenMPMaximumThreads();
  fx_info=(FxInfo **) AcquireAlignedMemory(number_threads,sizeof(*fx_info));
  if (fx_info == (FxInfo **) NULL)
    return((FxInfo **) NULL);
  (void) ResetMagickMemory(fx_info,0,number_threads*sizeof(*fx_info));
  if (*expression != '@')
    fx_expression=ConstantString(expression);
  else
    fx_expression=FileToString(expression+1,~0,exception);
  for (i=0; i < (long) number_threads; i++)
  {
    fx_info[i]=AcquireFxInfo(image,fx_expression);
    if (fx_info[i] == (FxInfo *) NULL)
      return(DestroyFxThreadSet(fx_info));
    (void) FxPreprocessExpression(fx_info[i],&alpha,fx_info[i]->exception);
  }
  fx_expression=DestroyString(fx_expression);
  return(fx_info);
}

MagickExport Image *FxImage(const Image *image,const char *expression,
  ExceptionInfo *exception)
{
  Image
    *fx_image;

  fx_image=FxImageChannel(image,GrayChannel,expression,exception);
  return(fx_image);
}

MagickExport Image *FxImageChannel(const Image *image,const ChannelType channel,
  const char *expression,ExceptionInfo *exception)
{
#define FxImageTag  "Fx/Image"

  CacheView
    *fx_view;

  FxInfo
    **restrict fx_info;

  Image
    *fx_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickRealType
    alpha;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  fx_image=CloneImage(image,0,0,MagickTrue,exception);
  if (fx_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(fx_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&fx_image->exception);
      fx_image=DestroyImage(fx_image);
      return((Image *) NULL);
    }
  fx_info=AcquireFxThreadSet(image,expression,exception);
  if (fx_info == (FxInfo **) NULL)
    {
      fx_image=DestroyImage(fx_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  status=FxPreprocessExpression(fx_info[0],&alpha,exception);
  if (status == MagickFalse)
    {
      fx_image=DestroyImage(fx_image);
      fx_info=DestroyFxThreadSet(fx_info);
      return((Image *) NULL);
    }
  /*
    Fx image.
  */
  status=MagickTrue;
  progress=0;
  fx_view=AcquireCacheView(fx_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) fx_image->rows; y++)
  {
    MagickRealType
      alpha;

    register IndexPacket
      *restrict fx_indexes;

    register long
      id,
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(fx_view,0,y,fx_image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    fx_indexes=GetCacheViewAuthenticIndexQueue(fx_view);
    id=GetOpenMPThreadId();
    alpha=0.0;
    for (x=0; x < (long) fx_image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        {
          (void) FxEvaluateChannelExpression(fx_info[id],RedChannel,x,y,
            &alpha,exception);
          q->red=ClampToQuantum((MagickRealType) QuantumRange*alpha);
        }
      if ((channel & GreenChannel) != 0)
        {
          (void) FxEvaluateChannelExpression(fx_info[id],GreenChannel,x,y,
            &alpha,exception);
          q->green=ClampToQuantum((MagickRealType) QuantumRange*alpha);
        }
      if ((channel & BlueChannel) != 0)
        {
          (void) FxEvaluateChannelExpression(fx_info[id],BlueChannel,x,y,
            &alpha,exception);
          q->blue=ClampToQuantum((MagickRealType) QuantumRange*alpha);
        }
      if ((channel & OpacityChannel) != 0)
        {
          (void) FxEvaluateChannelExpression(fx_info[id],OpacityChannel,x,y,
            &alpha,exception);
          if (image->matte == MagickFalse)
            q->opacity=ClampToQuantum((MagickRealType) QuantumRange*alpha);
          else
            q->opacity=ClampToQuantum((MagickRealType) (QuantumRange-
              QuantumRange*alpha));
        }
      if (((channel & IndexChannel) != 0) &&
          (fx_image->colorspace == CMYKColorspace))
        {
          (void) FxEvaluateChannelExpression(fx_info[id],IndexChannel,x,y,
            &alpha,exception);
          fx_indexes[x]=(IndexPacket) ClampToQuantum((MagickRealType)
            QuantumRange*alpha);
        }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(fx_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_FxImageChannel)
#endif
        proceed=SetImageProgress(image,FxImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  fx_image->matte=fx_info[0]->matte;
  fx_view=DestroyCacheView(fx_view);
  fx_info=DestroyFxThreadSet(fx_info);
  if (status == MagickFalse)
    fx_image=DestroyImage(fx_image);
  return(fx_image);
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
%        ExceptionInfo *exception)
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
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ImplodeImage(const Image *image,const double amount,
  ExceptionInfo *exception)
{
#define ImplodeImageTag  "Implode/Image"

  CacheView
    *image_view,
    *implode_view;

  Image
    *implode_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  MagickRealType
    radius;

  PointInfo
    center,
    scale;

  ResampleFilter
    **restrict resample_filter;

  /*
    Initialize implode image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  implode_image=CloneImage(image,0,0,MagickTrue,exception);
  if (implode_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(implode_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&implode_image->exception);
      implode_image=DestroyImage(implode_image);
      return((Image *) NULL);
    }
  if (implode_image->background_color.opacity != OpaqueOpacity)
    implode_image->matte=MagickTrue;
  /*
    Compute scaling factor.
  */
  scale.x=1.0;
  scale.y=1.0;
  center.x=0.5*image->columns;
  center.y=0.5*image->rows;
  radius=center.x;
  if (image->columns > image->rows)
    scale.y=(double) image->columns/(double) image->rows;
  else
    if (image->columns < image->rows)
      {
        scale.x=(double) image->rows/(double) image->columns;
        radius=center.y;
      }
  /*
    Implode image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(implode_image,&zero);
  resample_filter=AcquireResampleFilterThreadSet(image,
    UndefinedVirtualPixelMethod,MagickTrue,exception);
  image_view=AcquireCacheView(image);
  implode_view=AcquireCacheView(implode_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickPixelPacket
      pixel;

    MagickRealType
      distance;

    PointInfo
      delta;

    register IndexPacket
      *restrict implode_indexes;

    register long
      id,
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(implode_view,0,y,implode_image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    implode_indexes=GetCacheViewAuthenticIndexQueue(implode_view);
    delta.y=scale.y*(double) (y-center.y);
    pixel=zero;
    id=GetOpenMPThreadId();
    for (x=0; x < (long) image->columns; x++)
    {
      /*
        Determine if the pixel is within an ellipse.
      */
      delta.x=scale.x*(double) (x-center.x);
      distance=delta.x*delta.x+delta.y*delta.y;
      if (distance < (radius*radius))
        {
          double
            factor;

          /*
            Implode the pixel.
          */
          factor=1.0;
          if (distance > 0.0)
            factor=pow(sin((double) (MagickPI*sqrt((double) distance)/
              radius/2)),-amount);
          (void) ResamplePixelColor(resample_filter[id],(double)
            (factor*delta.x/scale.x+center.x),(double) (factor*delta.y/
            scale.y+center.y),&pixel);
          SetPixelPacket(implode_image,&pixel,q,implode_indexes+x);
        }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(implode_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ImplodeImage)
#endif
        proceed=SetImageProgress(image,ImplodeImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  implode_view=DestroyCacheView(implode_view);
  image_view=DestroyCacheView(image_view);
  resample_filter=DestroyResampleFilterThreadSet(resample_filter);
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
%      Image *MorphImages(const Image *image,const unsigned long number_frames,
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
MagickExport Image *MorphImages(const Image *image,
  const unsigned long number_frames,ExceptionInfo *exception)
{
#define MorphImageTag  "Morph/Image"

  Image
    *morph_image,
    *morph_images;

  long
    y;

  MagickOffsetType
    scene;

  MagickRealType
    alpha,
    beta;

  register const Image
    *next;

  register long
    i;

  MagickBooleanType
    status;

  /*
    Clone first frame in sequence.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  morph_images=CloneImage(image,0,0,MagickTrue,exception);
  if (morph_images == (Image *) NULL)
    return((Image *) NULL);
  if (GetNextImageInList(image) == (Image *) NULL)
    {
      /*
        Morph single image.
      */
      for (i=1; i < (long) number_frames; i++)
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

            proceed=SetImageProgress(image,MorphImageTag,i,number_frames);
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
    for (i=0; i < (long) number_frames; i++)
    {
      CacheView
        *image_view,
        *morph_view;

      beta=(MagickRealType) (i+1.0)/(MagickRealType) (number_frames+1.0);
      alpha=1.0-beta;
      morph_image=ZoomImage(next,(unsigned long) (alpha*next->columns+beta*
        GetNextImageInList(next)->columns+0.5),(unsigned long) (alpha*
        next->rows+beta*GetNextImageInList(next)->rows+0.5),exception);
      if (morph_image == (Image *) NULL)
        {
          morph_images=DestroyImageList(morph_images);
          return((Image *) NULL);
        }
      if (SetImageStorageClass(morph_image,DirectClass) == MagickFalse)
        {
          InheritException(exception,&morph_image->exception);
          morph_image=DestroyImage(morph_image);
          return((Image *) NULL);
        }
      AppendImageToList(&morph_images,morph_image);
      morph_images=GetLastImageInList(morph_images);
      morph_image=ZoomImage(GetNextImageInList(next),morph_images->columns,
        morph_images->rows,exception);
      if (morph_image == (Image *) NULL)
        {
          morph_images=DestroyImageList(morph_images);
          return((Image *) NULL);
        }
      image_view=AcquireCacheView(morph_image);
      morph_view=AcquireCacheView(morph_images);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) morph_images->rows; y++)
      {
        MagickBooleanType
          sync;

        register const PixelPacket
          *restrict p;

        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        p=GetCacheViewVirtualPixels(image_view,0,y,morph_image->columns,1,
          exception);
        q=GetCacheViewAuthenticPixels(morph_view,0,y,morph_images->columns,1,
          exception);
        if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (long) morph_images->columns; x++)
        {
          q->red=ClampToQuantum(alpha*q->red+beta*GetRedPixelComponent(p));
          q->green=ClampToQuantum(alpha*q->green+beta*
            GetGreenPixelComponent(p));
          q->blue=ClampToQuantum(alpha*q->blue+beta*GetBluePixelComponent(p));
          q->opacity=ClampToQuantum(alpha*q->opacity+beta*
            GetOpacityPixelComponent(p));
          p++;
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(morph_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      morph_view=DestroyCacheView(morph_view);
      image_view=DestroyCacheView(image_view);
      morph_image=DestroyImage(morph_image);
    }
    if (i < (long) number_frames)
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

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_MorphImages)
#endif
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
%        unsigned long attenuate,unsigned long depth)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o segment:   Define the region to apply plasma fractals values.
%
%    o attenuate: Define the plasmattenuation factor.
%
%    o depth: Limit the plasma recursion depth.
%
*/

static inline Quantum PlasmaPixel(RandomInfo *random_info,
  const MagickRealType pixel,const MagickRealType noise)
{
  Quantum
    plasma;

  plasma=ClampToQuantum(pixel+noise*GetPseudoRandomValue(random_info)-
    noise/2.0);
  return(plasma);
}

MagickExport MagickBooleanType PlasmaImageProxy(Image *image,
  RandomInfo *random_info,const SegmentInfo *segment,unsigned long attenuate,
  unsigned long depth)
{
  ExceptionInfo
    *exception;

  long
    x,
    x_mid,
    y,
    y_mid;

  MagickRealType
    plasma;

  PixelPacket
    u,
    v;

  if (((segment->x2-segment->x1) == 0.0) && ((segment->y2-segment->y1) == 0.0))
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
      x_mid=(long) (segment->x1+segment->x2+0.5)/2;
      y_mid=(long) (segment->y1+segment->y2+0.5)/2;
      local_info=(*segment);
      local_info.x2=(double) x_mid;
      local_info.y2=(double) y_mid;
      (void) PlasmaImageProxy(image,random_info,&local_info,attenuate,depth);
      local_info=(*segment);
      local_info.y1=(double) y_mid;
      local_info.x2=(double) x_mid;
      (void) PlasmaImageProxy(image,random_info,&local_info,attenuate,depth);
      local_info=(*segment);
      local_info.x1=(double) x_mid;
      local_info.y2=(double) y_mid;
      (void) PlasmaImageProxy(image,random_info,&local_info,attenuate,depth);
      local_info=(*segment);
      local_info.x1=(double) x_mid;
      local_info.y1=(double) y_mid;
      return(PlasmaImageProxy(image,random_info,&local_info,attenuate,depth));
    }
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  x_mid=(long) (segment->x1+segment->x2+0.5)/2;
  y_mid=(long) (segment->y1+segment->y2+0.5)/2;
  if ((segment->x1 == (double) x_mid) && (segment->x2 == (double) x_mid) &&
      (segment->y1 == (double) y_mid) && (segment->y2 == (double) y_mid))
    return(MagickFalse);
  /*
    Average pixels and apply plasma.
  */
  exception=(&image->exception);
  plasma=(MagickRealType) QuantumRange/(2.0*attenuate);
  if ((segment->x1 != (double) x_mid) || (segment->x2 != (double) x_mid))
    {
      register PixelPacket
        *restrict q;

      /*
        Left pixel.
      */
      x=(long) (segment->x1+0.5);
      (void) GetOneVirtualPixel(image,x,(long) (segment->y1+0.5),&u,exception);
      (void) GetOneVirtualPixel(image,x,(long) (segment->y2+0.5),&v,exception);
      q=QueueAuthenticPixels(image,x,y_mid,1,1,exception);
      if (q == (PixelPacket *) NULL)
        return(MagickTrue);
      q->red=PlasmaPixel(random_info,(MagickRealType) (u.red+v.red)/2.0,
        plasma);
      q->green=PlasmaPixel(random_info,(MagickRealType) (u.green+v.green)/2.0,
        plasma);
      q->blue=PlasmaPixel(random_info,(MagickRealType) (u.blue+v.blue)/2.0,
        plasma);
      (void) SyncAuthenticPixels(image,exception);
      if (segment->x1 != segment->x2)
        {
          /*
            Right pixel.
          */
          x=(long) (segment->x2+0.5);
          (void) GetOneVirtualPixel(image,x,(long) (segment->y1+0.5),&u,
            exception);
          (void) GetOneVirtualPixel(image,x,(long) (segment->y2+0.5),&v,
            exception);
          q=QueueAuthenticPixels(image,x,y_mid,1,1,exception);
          if (q == (PixelPacket *) NULL)
            return(MagickTrue);
          q->red=PlasmaPixel(random_info,(MagickRealType) (u.red+v.red)/2.0,
            plasma);
          q->green=PlasmaPixel(random_info,(MagickRealType) (u.green+v.green)/
            2.0,plasma);
          q->blue=PlasmaPixel(random_info,(MagickRealType) (u.blue+v.blue)/2.0,
             plasma);
          (void) SyncAuthenticPixels(image,exception);
        }
    }
  if ((segment->y1 != (double) y_mid) || (segment->y2 != (double) y_mid))
    {
      if ((segment->x1 != (double) x_mid) || (segment->y2 != (double) y_mid))
        {
          register PixelPacket
            *restrict q;

          /*
            Bottom pixel.
          */
          y=(long) (segment->y2+0.5);
          (void) GetOneVirtualPixel(image,(long) (segment->x1+0.5),y,&u,
            exception);
          (void) GetOneVirtualPixel(image,(long) (segment->x2+0.5),y,&v,
            exception);
          q=QueueAuthenticPixels(image,x_mid,y,1,1,exception);
          if (q == (PixelPacket *) NULL)
            return(MagickTrue);
          q->red=PlasmaPixel(random_info,(MagickRealType) (u.red+v.red)/2.0,
            plasma);
          q->green=PlasmaPixel(random_info,(MagickRealType) (u.green+v.green)/
            2.0,plasma);
          q->blue=PlasmaPixel(random_info,(MagickRealType) (u.blue+v.blue)/2.0,
            plasma);
          (void) SyncAuthenticPixels(image,exception);
        }
      if (segment->y1 != segment->y2)
        {
          register PixelPacket
            *restrict q;

          /*
            Top pixel.
          */
          y=(long) (segment->y1+0.5);
          (void) GetOneVirtualPixel(image,(long) (segment->x1+0.5),y,&u,
            exception);
          (void) GetOneVirtualPixel(image,(long) (segment->x2+0.5),y,&v,
            exception);
          q=QueueAuthenticPixels(image,x_mid,y,1,1,exception);
          if (q == (PixelPacket *) NULL)
            return(MagickTrue);
          q->red=PlasmaPixel(random_info,(MagickRealType) (u.red+v.red)/2.0,
            plasma);
          q->green=PlasmaPixel(random_info,(MagickRealType) (u.green+v.green)/
            2.0,plasma);
          q->blue=PlasmaPixel(random_info,(MagickRealType) (u.blue+v.blue)/2.0,
            plasma);
          (void) SyncAuthenticPixels(image,exception);
        }
    }
  if ((segment->x1 != segment->x2) || (segment->y1 != segment->y2))
    {
      register PixelPacket
        *restrict q;

      /*
        Middle pixel.
      */
      x=(long) (segment->x1+0.5);
      y=(long) (segment->y1+0.5);
      (void) GetOneVirtualPixel(image,x,y,&u,exception);
      x=(long) (segment->x2+0.5);
      y=(long) (segment->y2+0.5);
      (void) GetOneVirtualPixel(image,x,y,&v,exception);
      q=QueueAuthenticPixels(image,x_mid,y_mid,1,1,exception);
      if (q == (PixelPacket *) NULL)
        return(MagickTrue);
      q->red=PlasmaPixel(random_info,(MagickRealType) (u.red+v.red)/2.0,
        plasma);
      q->green=PlasmaPixel(random_info,(MagickRealType) (u.green+v.green)/2.0,
        plasma);
      q->blue=PlasmaPixel(random_info,(MagickRealType) (u.blue+v.blue)/2.0,
        plasma);
      (void) SyncAuthenticPixels(image,exception);
    }
  if (((segment->x2-segment->x1) < 3.0) && ((segment->y2-segment->y1) < 3.0))
    return(MagickTrue);
  return(MagickFalse);
}

MagickExport MagickBooleanType PlasmaImage(Image *image,
  const SegmentInfo *segment,unsigned long attenuate,unsigned long depth)
{
  MagickBooleanType
    status;

  RandomInfo
    *random_info;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  random_info=AcquireRandomInfo();
  status=PlasmaImageProxy(image,random_info,segment,attenuate,depth);
  random_info=DestroyRandomInfo(random_info);
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
%  The format of the AnnotateImage method is:
%
%      Image *PolaroidImage(const Image *image,const DrawInfo *draw_info,
%        const double angle,ExceptionInfo exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o draw_info: the draw info.
%
%    o angle: Apply the effect along this angle.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *PolaroidImage(const Image *image,const DrawInfo *draw_info,
  const double angle,ExceptionInfo *exception)
{
  const char
    *value;

  long
    quantum;

  Image
    *bend_image,
    *caption_image,
    *flop_image,
    *picture_image,
    *polaroid_image,
    *rotate_image,
    *trim_image;

  unsigned long
    height;

  /*
    Simulate a Polaroid picture.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  quantum=(long) MagickMax(MagickMax((double) image->columns,(double)
    image->rows)/25.0,10.0);
  height=image->rows+2*quantum;
  caption_image=(Image *) NULL;
  value=GetImageProperty(image,"Caption");
  if (value != (const char *) NULL)
    {
      char
        *caption,
        geometry[MaxTextExtent];

      DrawInfo
        *annotate_info;

      long
        count;

      MagickBooleanType
        status;

      TypeMetric
        metrics;

      /*
        Generate caption image.
      */
      caption_image=CloneImage(image,image->columns,1,MagickTrue,exception);
      if (caption_image == (Image *) NULL)
        return((Image *) NULL);
      annotate_info=CloneDrawInfo((const ImageInfo *) NULL,draw_info);
      caption=InterpretImageProperties((ImageInfo *) NULL,(Image *) image,
        value);
      (void) CloneString(&annotate_info->text,caption);
      count=FormatMagickCaption(caption_image,annotate_info,&metrics,&caption);
      status=SetImageExtent(caption_image,image->columns,(unsigned long)
        ((count+1)*(metrics.ascent-metrics.descent)+0.5));
      if (status == MagickFalse)
        caption_image=DestroyImage(caption_image);
      else
        {
          caption_image->background_color=image->border_color;
          (void) SetImageBackgroundColor(caption_image);
          (void) CloneString(&annotate_info->text,caption);
          (void) FormatMagickString(geometry,MaxTextExtent,"+0+%g",
            metrics.ascent);
          if (annotate_info->gravity == UndefinedGravity)
            (void) CloneString(&annotate_info->geometry,AcquireString(
              geometry));
          (void) AnnotateImage(caption_image,annotate_info);
          height+=caption_image->rows;
        }
      annotate_info=DestroyDrawInfo(annotate_info);
      caption=DestroyString(caption);
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
  (void) SetImageBackgroundColor(picture_image);
  (void) CompositeImage(picture_image,OverCompositeOp,image,quantum,quantum);
  if (caption_image != (Image *) NULL)
    {
      (void) CompositeImage(picture_image,OverCompositeOp,caption_image,
        quantum,(long) (image->rows+3*quantum/2));
      caption_image=DestroyImage(caption_image);
    }
  (void) QueryColorDatabase("none",&picture_image->background_color,exception);
  (void) SetImageAlphaChannel(picture_image,OpaqueAlphaChannel);
  rotate_image=RotateImage(picture_image,90.0,exception);
  picture_image=DestroyImage(picture_image);
  if (rotate_image == (Image *) NULL)
    return((Image *) NULL);
  picture_image=rotate_image;
  bend_image=WaveImage(picture_image,0.01*picture_image->rows,2.0*
    picture_image->columns,exception);
  picture_image=DestroyImage(picture_image);
  if (bend_image == (Image *) NULL)
    return((Image *) NULL);
  InheritException(&bend_image->exception,exception);
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
  (void) CompositeImage(polaroid_image,OverCompositeOp,picture_image,
    (long) (-0.01*picture_image->columns/2.0),0L);
  picture_image=DestroyImage(picture_image);
  (void) QueryColorDatabase("none",&polaroid_image->background_color,exception);
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
%     R e c o l o r I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RecolorImage() translate, scale, shear, or rotate image colors.  Although
%  you can use variable sized matrices, typically you use a 5 x 5 for an RGBA
%  image and a 6x6 for CMYKA.  Populate the last row with normalized values to
%  translate.
%
%  The format of the RecolorImage method is:
%
%      Image *RecolorImage(const Image *image,const unsigned long order,
%        const double *color_matrix,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o order: the number of columns and rows in the recolor matrix.
%
%    o color_matrix: An array of double representing the recolor matrix.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *RecolorImage(const Image *image,const unsigned long order,
  const double *color_matrix,ExceptionInfo *exception)
{
#define RecolorImageTag  "Recolor/Image"

  CacheView
    *image_view,
    *recolor_view;

  Image
    *recolor_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  /*
    Initialize image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  recolor_image=CloneImage(image,0,0,MagickTrue,exception);
  if (recolor_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(recolor_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&recolor_image->exception);
      recolor_image=DestroyImage(recolor_image);
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
        "  Recolor image with %ldx%ld color matrix:",order,order);
      message=AcquireString("");
      k=color_matrix;
      for (v=0; v < (long) order; v++)
      {
        *message='\0';
        (void) FormatMagickString(format,MaxTextExtent,"%ld: ",v);
        (void) ConcatenateString(&message,format);
        for (u=0; u < (long) order; u++)
        {
          (void) FormatMagickString(format,MaxTextExtent,"%+f ",*k++);
          (void) ConcatenateString(&message,format);
        }
        (void) LogMagickEvent(TransformEvent,GetMagickModule(),"%s",message);
      }
      message=DestroyString(message);
    }
  /*
    Recolor image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  recolor_view=AcquireCacheView(recolor_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register const double
      *restrict k;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register long
      x;

    register IndexPacket
      *restrict recolor_indexes;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewAuthenticPixels(recolor_view,0,y,recolor_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    recolor_indexes=GetCacheViewAuthenticIndexQueue(recolor_view);
    k=color_matrix;
    switch (order)
    {
      case 0:
        break;
      case 1:
      {
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=ClampToQuantum(k[0]*GetRedPixelComponent(p));
          p++;
          q++;
        }
        break;
      }
      case 2:
      {
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=ClampToQuantum(k[0]*p->red+k[1]*GetGreenPixelComponent(p));
          q->green=ClampToQuantum(k[2]*p->red+k[3]*GetGreenPixelComponent(p));
          p++;
          q++;
        }
        break;
      }
      case 3:
      {
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=ClampToQuantum(k[0]*p->red+k[1]*p->green+k[2]*GetBluePixelComponent(p));
          q->green=ClampToQuantum(k[3]*p->red+k[4]*p->green+k[5]*GetBluePixelComponent(p));
          q->blue=ClampToQuantum(k[6]*p->red+k[7]*p->green+k[8]*GetBluePixelComponent(p));
          p++;
          q++;
        }
        break;
      }
      case 4:
      {
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=ClampToQuantum(k[0]*p->red+k[1]*p->green+k[2]*p->blue+
            k[12]*QuantumRange);
          q->green=ClampToQuantum(k[4]*p->red+k[5]*p->green+k[6]*p->blue+
            k[13]*QuantumRange);
          q->blue=ClampToQuantum(k[8]*p->red+k[9]*p->green+k[10]*p->blue+
            k[14]*QuantumRange);
          p++;
          q++;
        }
        break;
      }
      case 5:
      {
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=ClampToQuantum(k[0]*p->red+k[1]*p->green+k[2]*p->blue+
            k[3]*(GetAlphaPixelComponent(p))+k[20]*QuantumRange);
          q->green=ClampToQuantum(k[5]*p->red+k[6]*p->green+k[7]*p->blue+
            k[8]*(GetAlphaPixelComponent(p))+k[21]*QuantumRange);
          q->blue=ClampToQuantum(k[10]*p->red+k[11]*p->green+k[12]*p->blue+
            k[13]*(GetAlphaPixelComponent(p))+k[22]*QuantumRange);
          if (image->matte != MagickFalse)
            q->opacity=ClampToQuantum((MagickRealType) QuantumRange-(k[15]*
              p->red+k[16]*p->green+k[17]*p->blue+k[18]*
              GetAlphaPixelComponent(p)+k[23]*QuantumRange));
          p++;
          q++;
        }
        break;
      }
      default:
      {
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=ClampToQuantum(k[0]*p->red+k[1]*p->green+k[2]*p->blue+
            k[3]*indexes[x]+k[4]*((Quantum) GetAlphaPixelComponent(p))+
            k[30]*QuantumRange);
          q->green=ClampToQuantum(k[6]*p->red+k[7]*p->green+k[8]*p->blue+
            k[9]*indexes[x]+k[10]*((Quantum) GetAlphaPixelComponent(p))+
            k[31]*QuantumRange);
          q->blue=ClampToQuantum(k[12]*p->red+k[13]*p->green+k[14]*p->blue+
            k[15]*indexes[x]+k[16]*((Quantum) GetAlphaPixelComponent(p))+
            k[32]*QuantumRange);
          if (image->matte != MagickFalse)
            q->opacity=ClampToQuantum((MagickRealType) QuantumRange-(k[24]*
              p->red+k[25]*p->green+k[26]*p->blue+k[27]*indexes[x]+
              k[28]*(GetAlphaPixelComponent(p))+k[34]*QuantumRange));
          if (image->colorspace == CMYKColorspace)
            recolor_indexes[x]=ClampToQuantum(k[18]*p->red+k[19]*p->green+k[20]*
              p->blue+k[21]*indexes[x]+k[22]*((Quantum) QuantumRange-
              GetOpacityPixelComponent(p))+k[33]*QuantumRange);
          p++;
          q++;
        }
        break;
      }
    }
    if (SyncCacheViewAuthenticPixels(recolor_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_RecolorImage)
#endif
        proceed=SetImageProgress(image,RecolorImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  recolor_view=DestroyCacheView(recolor_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    recolor_image=DestroyImage(recolor_image);
  return(recolor_image);
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

  long
    progress,
    y;

  MagickBooleanType
    status;

  /*
    Initialize sepia-toned image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  sepia_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (sepia_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(sepia_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&sepia_image->exception);
      sepia_image=DestroyImage(sepia_image);
      return((Image *) NULL);
    }
  /*
    Tone each row of the image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  sepia_view=AcquireCacheView(sepia_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register const PixelPacket
      *restrict p;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(sepia_view,0,y,sepia_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (long) image->columns; x++)
    {
      MagickRealType
        intensity,
        tone;

      intensity=(MagickRealType) PixelIntensityToQuantum(p);
      tone=intensity > threshold ? (MagickRealType) QuantumRange : intensity+
        (MagickRealType) QuantumRange-threshold;
      q->red=ClampToQuantum(tone);
      tone=intensity > (7.0*threshold/6.0) ? (MagickRealType) QuantumRange :
        intensity+(MagickRealType) QuantumRange-7.0*threshold/6.0;
      q->green=ClampToQuantum(tone);
      tone=intensity < (threshold/6.0) ? 0 : intensity-threshold/6.0;
      q->blue=ClampToQuantum(tone);
      tone=threshold/7.0;
      if ((MagickRealType) q->green < tone)
        q->green=ClampToQuantum(tone);
      if ((MagickRealType) q->blue < tone)
        q->blue=ClampToQuantum(tone);
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(sepia_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_SepiaToneImage)
#endif
        proceed=SetImageProgress(image,SepiaToneImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  sepia_view=DestroyCacheView(sepia_view);
  image_view=DestroyCacheView(image_view);
  (void) NormalizeImage(sepia_image);
  (void) ContrastImage(sepia_image,MagickTrue);
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
%      Image *ShadowImage(const Image *image,const double opacity,
%        const double sigma,const long x_offset,const long y_offset,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o opacity: percentage transparency.
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
MagickExport Image *ShadowImage(const Image *image,const double opacity,
  const double sigma,const long x_offset,const long y_offset,
  ExceptionInfo *exception)
{
#define ShadowImageTag  "Shadow/Image"

  CacheView
    *image_view;

  Image
    *border_image,
    *clone_image,
    *shadow_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  RectangleInfo
    border_info;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  clone_image=CloneImage(image,0,0,MagickTrue,exception);
  if (clone_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageVirtualPixelMethod(clone_image,EdgeVirtualPixelMethod);
  clone_image->compose=OverCompositeOp;
  border_info.width=(unsigned long) (2.0*sigma+0.5);
  border_info.height=(unsigned long) (2.0*sigma+0.5);
  border_info.x=0;
  border_info.y=0;
  (void) QueryColorDatabase("none",&clone_image->border_color,exception);
  border_image=BorderImage(clone_image,&border_info,exception);
  clone_image=DestroyImage(clone_image);
  if (border_image == (Image *) NULL)
    return((Image *) NULL);
  if (border_image->matte == MagickFalse)
    (void) SetImageAlphaChannel(border_image,OpaqueAlphaChannel);
  /*
    Shadow image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(border_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) border_image->rows; y++)
  {
    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,border_image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (long) border_image->columns; x++)
    {
      q->red=border_image->background_color.red;
      q->green=border_image->background_color.green;
      q->blue=border_image->background_color.blue;
      if (border_image->matte == MagickFalse)
        q->opacity=border_image->background_color.opacity;
      else
        q->opacity=ClampToQuantum((MagickRealType) (QuantumRange-
          GetAlphaPixelComponent(q)*opacity/100.0));
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ShadowImage)
#endif
        proceed=SetImageProgress(image,ShadowImageTag,progress++,
          border_image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  shadow_image=BlurImageChannel(border_image,AlphaChannel,0.0,sigma,exception);
  border_image=DestroyImage(border_image);
  if (shadow_image == (Image *) NULL)
    return((Image *) NULL);
  if (shadow_image->page.width == 0)
    shadow_image->page.width=shadow_image->columns;
  if (shadow_image->page.height == 0)
    shadow_image->page.height=shadow_image->rows;
  shadow_image->page.width+=x_offset-(long) border_info.width;
  shadow_image->page.height+=y_offset-(long) border_info.height;
  shadow_image->page.x+=x_offset-(long) border_info.width;
  shadow_image->page.y+=y_offset-(long) border_info.height;
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

  long
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  RandomInfo
    **restrict random_info;

  /*
    Sketch image.
  */
  random_image=CloneImage(image,image->columns << 1,image->rows << 1,
    MagickTrue,exception);
  if (random_image == (Image *) NULL)
    return((Image *) NULL);
  status=MagickTrue;
  GetMagickPixelPacket(random_image,&zero);
  random_info=AcquireRandomInfoThreadSet();
  random_view=AcquireCacheView(random_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
  for (y=0; y < (long) random_image->rows; y++)
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
    q=QueueCacheViewAuthenticPixels(random_view,0,y,random_image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(random_view);
    pixel=zero;
    id=GetOpenMPThreadId();
    for (x=0; x < (long) random_image->columns; x++)
    {
      pixel.red=(MagickRealType) (QuantumRange*
        GetPseudoRandomValue(random_info[id]));
      pixel.green=pixel.red;
      pixel.blue=pixel.red;
      if (image->colorspace == CMYKColorspace)
        pixel.index=pixel.red;
      SetPixelPacket(random_image,&pixel,q,indexes+x);
      q++;
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
  (void) NormalizeImage(dodge_image);
  (void) NegateImage(dodge_image,MagickFalse);
  (void) TransformImage(&dodge_image,(char *) NULL,"50%");
  sketch_image=CloneImage(image,0,0,MagickTrue,exception);
  if (sketch_image == (Image *) NULL)
    {
      dodge_image=DestroyImage(dodge_image);
      return((Image *) NULL);
    }
  (void) CompositeImage(sketch_image,ColorDodgeCompositeOp,dodge_image,0,0);
  dodge_image=DestroyImage(dodge_image);
  blend_image=CloneImage(image,0,0,MagickTrue,exception);
  if (blend_image == (Image *) NULL)
    {
      sketch_image=DestroyImage(sketch_image);
      return((Image *) NULL);
    }
  (void) SetImageArtifact(blend_image,"compose:args","20x80");
  (void) CompositeImage(sketch_image,BlendCompositeOp,blend_image,0,0);
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
%      MagickBooleanType SolarizeImage(Image *image,const double threshold)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold:  Define the extent of the solarization.
%
*/
MagickExport MagickBooleanType SolarizeImage(Image *image,
  const double threshold)
{
#define SolarizeImageTag  "Solarize/Image"

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
    {
      register long
        i;

      /*
        Solarize colormap.
      */
      for (i=0; i < (long) image->colors; i++)
      {
        if ((MagickRealType) image->colormap[i].red > threshold)
          image->colormap[i].red=(Quantum) QuantumRange-image->colormap[i].red;
        if ((MagickRealType) image->colormap[i].green > threshold)
          image->colormap[i].green=(Quantum) QuantumRange-
            image->colormap[i].green;
        if ((MagickRealType) image->colormap[i].blue > threshold)
          image->colormap[i].blue=(Quantum) QuantumRange-
            image->colormap[i].blue;
      }
    }
  /*
    Solarize image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (long) image->columns; x++)
    {
      if ((MagickRealType) q->red > threshold)
        q->red=(Quantum) QuantumRange-q->red;
      if ((MagickRealType) q->green > threshold)
        q->green=(Quantum) QuantumRange-q->green;
      if ((MagickRealType) q->blue > threshold)
        q->blue=(Quantum) QuantumRange-q->blue;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_SolarizeImage)
#endif
        proceed=SetImageProgress(image,SolarizeImageTag,progress++,image->rows);
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
#define GetBit(alpha,i) ((((unsigned long) (alpha) >> (unsigned long) \
  (i)) & 0x01) != 0)
#define SetBit(alpha,i,set) (alpha)=(Quantum) ((set) ? (unsigned long) (alpha) \
  | (1UL << (unsigned long) (i)) : (unsigned long) (alpha) & \
  ~(1UL << (unsigned long) (i)))
#define SteganoImageTag  "Stegano/Image"

  Image
    *stegano_image;

  int
    c;

  long
    i,
    j,
    k,
    y;

  MagickBooleanType
    status;

  PixelPacket
    pixel;

  register long
    x;

  register PixelPacket
    *q;

  unsigned long
    depth;

  /*
    Initialize steganographic image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(watermark != (const Image *) NULL);
  assert(watermark->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  stegano_image=CloneImage(image,0,0,MagickTrue,exception);
  if (stegano_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(stegano_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&stegano_image->exception);
      stegano_image=DestroyImage(stegano_image);
      return((Image *) NULL);
    }
  stegano_image->depth=MAGICKCORE_QUANTUM_DEPTH;
  /*
    Hide watermark in low-order bits of image.
  */
  c=0;
  i=0;
  j=0;
  depth=stegano_image->depth;
  k=image->offset;
  for (i=(long) depth-1; (i >= 0) && (j < (long) depth); i--)
  {
    for (y=0; (y < (long) watermark->rows) && (j < (long) depth); y++)
    {
      for (x=0; (x < (long) watermark->columns) && (j < (long) depth); x++)
      {
        (void) GetOneVirtualPixel(watermark,x,y,&pixel,exception);
        if ((k/(long) stegano_image->columns) >= (long) stegano_image->rows)
          break;
        q=GetAuthenticPixels(stegano_image,k % (long) stegano_image->columns,
          k/(long) stegano_image->columns,1,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        switch (c)
        {
          case 0:
          {
            SetBit(q->red,j,GetBit(PixelIntensityToQuantum(&pixel),i));
            break;
          }
          case 1:
          {
            SetBit(q->green,j,GetBit(PixelIntensityToQuantum(&pixel),i));
            break;
          }
          case 2:
          {
            SetBit(q->blue,j,GetBit(PixelIntensityToQuantum(&pixel),i));
            break;
          }
        }
        if (SyncAuthenticPixels(stegano_image,exception) == MagickFalse)
          break;
        c++;
        if (c == 3)
          c=0;
        k++;
        if (k == (long) (stegano_image->columns*stegano_image->columns))
          k=0;
        if (k == image->offset)
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
  if (stegano_image->storage_class == PseudoClass)
    (void) SyncImage(stegano_image);
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
%        const Image *right_image,const long x_offset,const long y_offset,
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
  const Image *right_image,const long x_offset,const long y_offset,
  ExceptionInfo *exception)
{
#define StereoImageTag  "Stereo/Image"

  const Image
    *image;

  Image
    *stereo_image;

  long
    y;

  MagickBooleanType
    status;

  assert(left_image != (const Image *) NULL);
  assert(left_image->signature == MagickSignature);
  if (left_image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      left_image->filename);
  assert(right_image != (const Image *) NULL);
  assert(right_image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  assert(right_image != (const Image *) NULL);
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
  if (SetImageStorageClass(stereo_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&stereo_image->exception);
      stereo_image=DestroyImage(stereo_image);
      return((Image *) NULL);
    }
  /*
    Copy left image to red channel and right image to blue channel.
  */
  for (y=0; y < (long) stereo_image->rows; y++)
  {
    register const PixelPacket
      *restrict p,
      *restrict q;

    register long
      x;

    register PixelPacket
      *restrict r;

    p=GetVirtualPixels(left_image,-x_offset,y-y_offset,image->columns,1,
      exception);
    q=GetVirtualPixels(right_image,0,y,right_image->columns,1,exception);
    r=QueueAuthenticPixels(stereo_image,0,y,stereo_image->columns,1,exception);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL) ||
        (r == (PixelPacket *) NULL))
      break;
    for (x=0; x < (long) stereo_image->columns; x++)
    {
      r->red=GetRedPixelComponent(p);
      r->green=q->green;
      r->blue=q->blue;
      r->opacity=(Quantum) ((p->opacity+q->opacity)/2);
      p++;
      q++;
      r++;
    }
    if (SyncAuthenticPixels(stereo_image,exception) == MagickFalse)
      break;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,StereoImageTag,y,stereo_image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
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
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o degrees: Define the tightness of the swirling effect.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *SwirlImage(const Image *image,double degrees,
  ExceptionInfo *exception)
{
#define SwirlImageTag  "Swirl/Image"

  CacheView
    *image_view,
    *swirl_view;

  Image
    *swirl_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  MagickRealType
    radius;

  PointInfo
    center,
    scale;

  ResampleFilter
    **restrict resample_filter;

  /*
    Initialize swirl image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  swirl_image=CloneImage(image,0,0,MagickTrue,exception);
  if (swirl_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(swirl_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&swirl_image->exception);
      swirl_image=DestroyImage(swirl_image);
      return((Image *) NULL);
    }
  if (swirl_image->background_color.opacity != OpaqueOpacity)
    swirl_image->matte=MagickTrue;
  /*
    Compute scaling factor.
  */
  center.x=(double) image->columns/2.0;
  center.y=(double) image->rows/2.0;
  radius=MagickMax(center.x,center.y);
  scale.x=1.0;
  scale.y=1.0;
  if (image->columns > image->rows)
    scale.y=(double) image->columns/(double) image->rows;
  else
    if (image->columns < image->rows)
      scale.x=(double) image->rows/(double) image->columns;
  degrees=(double) DegreesToRadians(degrees);
  /*
    Swirl image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(swirl_image,&zero);
  resample_filter=AcquireResampleFilterThreadSet(image,
    UndefinedVirtualPixelMethod,MagickTrue,exception);
  image_view=AcquireCacheView(image);
  swirl_view=AcquireCacheView(swirl_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickPixelPacket
      pixel;

    MagickRealType
      distance;

    PointInfo
      delta;

    register IndexPacket
      *restrict swirl_indexes;

    register long
      id,
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(swirl_view,0,y,swirl_image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    swirl_indexes=GetCacheViewAuthenticIndexQueue(swirl_view);
    delta.y=scale.y*(double) (y-center.y);
    pixel=zero;
    id=GetOpenMPThreadId();
    for (x=0; x < (long) image->columns; x++)
    {
      /*
        Determine if the pixel is within an ellipse.
      */
      delta.x=scale.x*(double) (x-center.x);
      distance=delta.x*delta.x+delta.y*delta.y;
      if (distance < (radius*radius))
        {
          MagickRealType
            cosine,
            factor,
            sine;

          /*
            Swirl the pixel.
          */
          factor=1.0-sqrt((double) distance)/radius;
          sine=sin((double) (degrees*factor*factor));
          cosine=cos((double) (degrees*factor*factor));
          (void) ResamplePixelColor(resample_filter[id],(double) ((cosine*
            delta.x-sine*delta.y)/scale.x+center.x),(double) ((sine*delta.x+
            cosine*delta.y)/scale.y+center.y),&pixel);
          SetPixelPacket(swirl_image,&pixel,q,swirl_indexes+x);
        }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(swirl_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_SwirlImage)
#endif
        proceed=SetImageProgress(image,SwirlImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  swirl_view=DestroyCacheView(swirl_view);
  image_view=DestroyCacheView(image_view);
  resample_filter=DestroyResampleFilterThreadSet(resample_filter);
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
%      Image *TintImage(const Image *image,const char *opacity,
%        const PixelPacket tint,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o opacity: A color value used for tinting.
%
%    o tint: A color value used for tinting.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *TintImage(const Image *image,const char *opacity,
  const PixelPacket tint,ExceptionInfo *exception)
{
#define TintImageTag  "Tint/Image"

  CacheView
    *image_view,
    *tint_view;

  GeometryInfo
    geometry_info;

  Image
    *tint_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickStatusType
    flags;

  MagickPixelPacket
    color_vector,
    pixel;

  /*
    Allocate tint image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  tint_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (tint_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(tint_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&tint_image->exception);
      tint_image=DestroyImage(tint_image);
      return((Image *) NULL);
    }
  if (opacity == (const char *) NULL)
    return(tint_image);
  /*
    Determine RGB values of the color.
  */
  flags=ParseGeometry(opacity,&geometry_info);
  pixel.red=geometry_info.rho;
  if ((flags & SigmaValue) != 0)
    pixel.green=geometry_info.sigma;
  else
    pixel.green=pixel.red;
  if ((flags & XiValue) != 0)
    pixel.blue=geometry_info.xi;
  else
    pixel.blue=pixel.red;
  if ((flags & PsiValue) != 0)
    pixel.opacity=geometry_info.psi;
  else
    pixel.opacity=(MagickRealType) OpaqueOpacity;
  color_vector.red=(MagickRealType) (pixel.red*tint.red/100.0-
    PixelIntensity(&tint));
  color_vector.green=(MagickRealType) (pixel.green*tint.green/100.0-
    PixelIntensity(&tint));
  color_vector.blue=(MagickRealType) (pixel.blue*tint.blue/100.0-
    PixelIntensity(&tint));
  /*
    Tint image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  tint_view=AcquireCacheView(tint_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register const PixelPacket
      *restrict p;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(tint_view,0,y,tint_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (long) image->columns; x++)
    {
      MagickPixelPacket
        pixel;

      MagickRealType
        weight;

      weight=QuantumScale*p->red-0.5;
      pixel.red=(MagickRealType) p->red+color_vector.red*(1.0-(4.0*
        (weight*weight)));
      SetRedPixelComponent(q,ClampRedPixelComponent(&pixel));
      weight=QuantumScale*p->green-0.5;
      pixel.green=(MagickRealType) p->green+color_vector.green*(1.0-(4.0*
        (weight*weight)));
      SetGreenPixelComponent(q,ClampGreenPixelComponent(&pixel));
      weight=QuantumScale*p->blue-0.5;
      pixel.blue=(MagickRealType) p->blue+color_vector.blue*(1.0-(4.0*
        (weight*weight)));
      SetBluePixelComponent(q,ClampBluePixelComponent(&pixel));
      SetOpacityPixelComponent(q,GetOpacityPixelComponent(p));
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(tint_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_TintImage)
#endif
        proceed=SetImageProgress(image,TintImageTag,progress++,image->rows);
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
%        const double sigma,const long x,const long y,ExceptionInfo *exception)
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
  const double sigma,const long x,const long y,ExceptionInfo *exception)
{
  char
    ellipse[MaxTextExtent];

  DrawInfo
    *draw_info;

  Image
    *canvas_image,
    *blur_image,
    *oval_image,
    *vignette_image;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  canvas_image=CloneImage(image,0,0,MagickTrue,exception);
  if (canvas_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(canvas_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&canvas_image->exception);
      canvas_image=DestroyImage(canvas_image);
      return((Image *) NULL);
    }
  canvas_image->matte=MagickTrue;
  oval_image=CloneImage(canvas_image,canvas_image->columns,
    canvas_image->rows,MagickTrue,exception);
  if (oval_image == (Image *) NULL)
    {
      canvas_image=DestroyImage(canvas_image);
      return((Image *) NULL);
    }
  (void) QueryColorDatabase("#000000",&oval_image->background_color,exception);
  (void) SetImageBackgroundColor(oval_image);
  draw_info=CloneDrawInfo((const ImageInfo *) NULL,(const DrawInfo *) NULL);
  (void) QueryColorDatabase("#ffffff",&draw_info->fill,exception);
  (void) QueryColorDatabase("#ffffff",&draw_info->stroke,exception);
  (void) FormatMagickString(ellipse,MaxTextExtent,
    "ellipse %g,%g,%g,%g,0.0,360.0",image->columns/2.0,
    image->rows/2.0,image->columns/2.0-x,image->rows/2.0-y);
  draw_info->primitive=AcquireString(ellipse);
  (void) DrawImage(oval_image,draw_info);
  draw_info=DestroyDrawInfo(draw_info);
  blur_image=BlurImage(oval_image,radius,sigma,exception);
  oval_image=DestroyImage(oval_image);
  if (blur_image == (Image *) NULL)
    {
      canvas_image=DestroyImage(canvas_image);
      return((Image *) NULL);
    }
  blur_image->matte=MagickFalse;
  (void) CompositeImage(canvas_image,CopyOpacityCompositeOp,blur_image,0,0);
  blur_image=DestroyImage(blur_image);
  vignette_image=MergeImageLayers(canvas_image,FlattenLayer,exception);
  canvas_image=DestroyImage(canvas_image);
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
%        const double wave_length,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o amplitude, wave_length:  Define the amplitude and wave length of the
%      sine wave.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *WaveImage(const Image *image,const double amplitude,
  const double wave_length,ExceptionInfo *exception)
{
#define WaveImageTag  "Wave/Image"

  CacheView
    *wave_view;

  Image
    *wave_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  MagickRealType
    *sine_map;

  register long
    i;

  ResampleFilter
    **restrict resample_filter;

  /*
    Initialize wave image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  wave_image=CloneImage(image,image->columns,(unsigned long) (image->rows+2.0*
    fabs(amplitude)),MagickTrue,exception);
  if (wave_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(wave_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&wave_image->exception);
      wave_image=DestroyImage(wave_image);
      return((Image *) NULL);
    }
  if (wave_image->background_color.opacity != OpaqueOpacity)
    wave_image->matte=MagickTrue;
  /*
    Allocate sine map.
  */
  sine_map=(MagickRealType *) AcquireQuantumMemory((size_t) wave_image->columns,
    sizeof(*sine_map));
  if (sine_map == (MagickRealType *) NULL)
    {
      wave_image=DestroyImage(wave_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  for (i=0; i < (long) wave_image->columns; i++)
    sine_map[i]=fabs(amplitude)+amplitude*sin((2*MagickPI*i)/wave_length);
  /*
    Wave image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(wave_image,&zero);
  resample_filter=AcquireResampleFilterThreadSet(image,
    BackgroundVirtualPixelMethod,MagickTrue,exception);
  wave_view=AcquireCacheView(wave_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) wave_image->rows; y++)
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
    q=QueueCacheViewAuthenticPixels(wave_view,0,y,wave_image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(wave_view);
    pixel=zero;
    id=GetOpenMPThreadId();
    for (x=0; x < (long) wave_image->columns; x++)
    {
      (void) ResamplePixelColor(resample_filter[id],(double) x,(double) (y-
        sine_map[x]),&pixel);
      SetPixelPacket(wave_image,&pixel,q,indexes+x);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(wave_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_WaveImage)
#endif
        proceed=SetImageProgress(image,WaveImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  wave_view=DestroyCacheView(wave_view);
  resample_filter=DestroyResampleFilterThreadSet(resample_filter);
  sine_map=(MagickRealType *) RelinquishMagickMemory(sine_map);
  if (status == MagickFalse)
    wave_image=DestroyImage(wave_image);
  return(wave_image);
}

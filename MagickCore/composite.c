/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        CCCC   OOO   M   M  PPPP    OOO   SSSSS  IIIII  TTTTT  EEEEE         %
%       C      O   O  MM MM  P   P  O   O  SS       I      T    E             %
%       C      O   O  M M M  PPPP   O   O   SSS     I      T    EEE           %
%       C      O   O  M   M  P      O   O     SS    I      T    E             %
%        CCCC   OOO   M   M  P       OOO   SSSSS  IIIII    T    EEEEE         %
%                                                                             %
%                                                                             %
%                     MagickCore Image Composite Methods                      %
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
#include "MagickCore/artifact.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/channel.h"
#include "MagickCore/client.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/draw.h"
#include "MagickCore/fx.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum.h"
#include "MagickCore/resample.h"
#include "MagickCore/resource_.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/threshold.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p o s i t e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompositeImage() returns the second image composited onto the first
%  at the specified offset, using the specified composite method.
%
%  The format of the CompositeImage method is:
%
%      MagickBooleanType CompositeImage(Image *image,
%        const Image *source_image,const CompositeOperator compose,
%        const MagickBooleanType clip_to_self,const ssize_t x_offset,
%        const ssize_t y_offset,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the canvas image, modified by he composition
%
%    o source_image: the source image.
%
%    o compose: This operator affects how the composite is applied to
%      the image.  The operators and how they are utilized are listed here
%      http://www.w3.org/TR/SVG12/#compositing.
%
%    o clip_to_self: set to MagickTrue to limit composition to area composed.
%
%    o x_offset: the column offset of the composited image.
%
%    o y_offset: the row offset of the composited image.
%
%  Extra Controls from Image meta-data in 'image' (artifacts)
%
%    o "compose:args"
%        A string containing extra numerical arguments for specific compose
%        methods, generally expressed as a 'geometry' or a comma separated list
%        of numbers.
%
%        Compose methods needing such arguments include "BlendCompositeOp" and
%        "DisplaceCompositeOp".
%
%    o exception: return any errors or warnings in this structure.
%
*/

/*
   Composition based on the SVG specification:

   A Composition is defined by...
      Color Function :  f(Sc,Dc)  where Sc and Dc are the normizalized colors
      Blending areas :  X = 1     for area of overlap, ie: f(Sc,Dc)
                        Y = 1     for source preserved
                        Z = 1     for canvas preserved

   Conversion to transparency (then optimized)
      Dca' = f(Sc, Dc)*Sa*Da + Y*Sca*(1-Da) + Z*Dca*(1-Sa)
      Da'  = X*Sa*Da + Y*Sa*(1-Da) + Z*Da*(1-Sa)

   Where...
      Sca = Sc*Sa     normalized Source color divided by Source alpha
      Dca = Dc*Da     normalized Dest color divided by Dest alpha
      Dc' = Dca'/Da'  the desired color value for this channel.

   Da' in in the follow formula as 'gamma'  The resulting alpla value.

   Most functions use a blending mode of over (X=1,Y=1,Z=1) this results in
   the following optimizations...
      gamma = Sa+Da-Sa*Da;
      gamma = 1 - QuantumScale*alpha * QuantumScale*beta;
      opacity = QuantumScale*alpha*beta;  // over blend, optimized 1-Gamma

   The above SVG definitions also define that Mathematical Composition
   methods should use a 'Over' blending mode for Alpha Channel.
   It however was not applied for composition modes of 'Plus', 'Minus',
   the modulus versions of 'Add' and 'Subtract'.

   Mathematical operator changes to be applied from IM v6.7...

    1) Modulus modes 'Add' and 'Subtract' are obsoleted and renamed
       'ModulusAdd' and 'ModulusSubtract' for clarity.

    2) All mathematical compositions work as per the SVG specification
       with regard to blending.  This now includes 'ModulusAdd' and
       'ModulusSubtract'.

    3) When the special channel flag 'sync' (syncronize channel updates)
       is turned off (enabled by default) then mathematical compositions are
       only performed on the channels specified, and are applied
       independantally of each other.  In other words the mathematics is
       performed as 'pure' mathematical operations, rather than as image
       operations.
*/

static void HCLComposite(const MagickRealType hue,const MagickRealType chroma,
  const MagickRealType luma,MagickRealType *red,MagickRealType *green,
  MagickRealType *blue)
{
  MagickRealType
    b,
    c,
    g,
    h,
    m,
    r,
    x;

  /*
    Convert HCL to RGB colorspace.
  */
  assert(red != (MagickRealType *) NULL);
  assert(green != (MagickRealType *) NULL);
  assert(blue != (MagickRealType *) NULL);
  h=6.0*hue;
  c=chroma;
  x=c*(1.0-fabs(fmod(h,2.0)-1.0));
  r=0.0;
  g=0.0;
  b=0.0;
  if ((0.0 <= h) && (h < 1.0))
    {
      r=c;
      g=x;
    }
  else
    if ((1.0 <= h) && (h < 2.0))
      {
        r=x;
        g=c;
      }
    else
      if ((2.0 <= h) && (h < 3.0))
        {
          g=c;
          b=x;
        }
      else
        if ((3.0 <= h) && (h < 4.0))
          {
            g=x;
            b=c;
          }
        else
          if ((4.0 <= h) && (h < 5.0))
            {
              r=x;
              b=c;
            }
          else
            if ((5.0 <= h) && (h < 6.0))
              {
                r=c;
                b=x;
              }
  m=luma-(0.298839*r+0.586811*g+0.114350*b);
  *red=QuantumRange*(r+m);
  *green=QuantumRange*(g+m);
  *blue=QuantumRange*(b+m);
}

static void CompositeHCL(const MagickRealType red,const MagickRealType green,
  const MagickRealType blue,MagickRealType *hue,MagickRealType *chroma,
  MagickRealType *luma)
{
  MagickRealType
    b,
    c,
    g,
    h,
    max,
    r;

  /*
    Convert RGB to HCL colorspace.
  */
  assert(hue != (MagickRealType *) NULL);
  assert(chroma != (MagickRealType *) NULL);
  assert(luma != (MagickRealType *) NULL);
  r=red;
  g=green;
  b=blue;
  max=MagickMax(r,MagickMax(g,b));
  c=max-(MagickRealType) MagickMin(r,MagickMin(g,b));
  h=0.0;
  if (c == 0)
    h=0.0;
  else
    if (red == max)
      h=fmod((g-b)/c+6.0,6.0);
    else
      if (green == max)
        h=((b-r)/c)+2.0;
      else
        if (blue == max)
          h=((r-g)/c)+4.0;
  *hue=(h/6.0);
  *chroma=QuantumScale*c;
  *luma=QuantumScale*(0.298839*r+0.586811*g+0.114350*b);
}

static MagickBooleanType CompositeOverImage(Image *image,
  const Image *source_image,const MagickBooleanType clip_to_self,
  const ssize_t x_offset,const ssize_t y_offset,ExceptionInfo *exception)
{
#define CompositeImageTag  "Composite/Image"

  CacheView
    *image_view,
    *source_view;

  const char
    *value;

  MagickBooleanType
    clamp,
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  /*
    Composite image.
  */
  status=MagickTrue;
  progress=0;
  clamp=MagickTrue;
  value=GetImageArtifact(image,"compose:clamp");
  if (value != (const char *) NULL)
    clamp=IsStringTrue(value);
  status=MagickTrue;
  progress=0;
  source_view=AcquireVirtualCacheView(source_image,exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_number_threads(source_image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *pixels;

    PixelInfo
      canvas_pixel,
      source_pixel;

    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    if (clip_to_self != MagickFalse)
      {
        if (y < y_offset)
          continue;
        if ((y-y_offset) >= (ssize_t) source_image->rows)
          continue;
      }
    /*
      If pixels is NULL, y is outside overlay region.
    */
    pixels=(Quantum *) NULL;
    p=(Quantum *) NULL;
    if ((y >= y_offset) && ((y-y_offset) < (ssize_t) source_image->rows))
      {
        p=GetCacheViewVirtualPixels(source_view,0,y-y_offset,
          source_image->columns,1,exception);
        if (p == (const Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        pixels=p;
        if (x_offset < 0)
          p-=x_offset*GetPixelChannels(source_image);
      }
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    GetPixelInfo(image,&canvas_pixel);
    GetPixelInfo(source_image,&source_pixel);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        gamma;

      MagickRealType
        alpha,
        Da,
        Dc,
        Dca,
        Sa,
        Sc,
        Sca;

      register ssize_t
        i;

      size_t
        channels;

      if (clip_to_self != MagickFalse)
        {
          if (x < x_offset)
            {
              q+=GetPixelChannels(image);
              continue;
            }
          if ((x-x_offset) >= (ssize_t) source_image->columns)
            break;
        }
      if ((pixels == (Quantum *) NULL) || (x < x_offset) ||
          ((x-x_offset) >= (ssize_t) source_image->columns))
        {
          Quantum
            source[MaxPixelChannels];

          /*
            Virtual composite:
              Sc: source color.
              Dc: canvas color.
          */
          (void) GetOneVirtualPixel(source_image,x-x_offset,y-y_offset,source,
            exception);
          if (GetPixelWriteMask(image,q) <= (QuantumRange/2))
            {
              q+=GetPixelChannels(image);
              continue;
            }
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            MagickRealType
              pixel;

            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            PixelTrait source_traits=GetPixelChannelTraits(source_image,
              channel);
            if ((traits == UndefinedPixelTrait) ||
                (source_traits == UndefinedPixelTrait))
              continue;
            if (channel == AlphaPixelChannel)
              pixel=(MagickRealType) TransparentAlpha;
            else
              pixel=(MagickRealType) q[i];
            q[i]=clamp != MagickFalse ? ClampPixel(pixel) :
              ClampToQuantum(pixel);
          }
          q+=GetPixelChannels(image);
          continue;
        }
      /*
        Authentic composite:
          Sa:  normalized source alpha.
          Da:  normalized canvas alpha.
      */
      Sa=QuantumScale*GetPixelAlpha(source_image,p);
      Da=QuantumScale*GetPixelAlpha(image,q);
      alpha=Sa+Da-Sa*Da;
      if (GetPixelWriteMask(image,q) <= (QuantumRange/2))
        {
          p+=GetPixelChannels(source_image);
          q+=GetPixelChannels(image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        MagickRealType
          pixel;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait source_traits=GetPixelChannelTraits(source_image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((source_traits == UndefinedPixelTrait) &&
            (channel != AlphaPixelChannel))
            continue;
        if (channel == AlphaPixelChannel)
          {
            /*
              Set alpha channel.
            */
            pixel=QuantumRange*alpha;
            q[i]=clamp != MagickFalse ? ClampPixel(pixel) :
              ClampToQuantum(pixel);
            continue;
          }
        /*
          Sc: source color.
          Dc: canvas color.
        */
        Sc=(MagickRealType) GetPixelChannel(source_image,channel,p);
        Dc=(MagickRealType) q[i];
        if ((traits & CopyPixelTrait) != 0)
          {
            /*
              Copy channel.
            */
            q[i]=Sc;
            continue;
          }
        /*
          Porter-Duff compositions:
            Sca: source normalized color multiplied by alpha.
            Dca: normalized canvas color multiplied by alpha.
        */
        Sca=QuantumScale*Sa*Sc;
        Dca=QuantumScale*Da*Dc;
        gamma=PerceptibleReciprocal(alpha);
        pixel=QuantumRange*gamma*(Sca+Dca*(1.0-Sa));
        q[i]=clamp != MagickFalse ? ClampPixel(pixel) : ClampToQuantum(pixel);
      }
      p+=GetPixelChannels(source_image);
      channels=GetPixelChannels(source_image);
      if (p >= (pixels+channels*source_image->columns))
        p=pixels;
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_CompositeImage)
#endif
        proceed=SetImageProgress(image,CompositeImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  source_view=DestroyCacheView(source_view);
  image_view=DestroyCacheView(image_view);
  return(status);
}

MagickExport MagickBooleanType CompositeImage(Image *image,
  const Image *composite,const CompositeOperator compose,
  const MagickBooleanType clip_to_self,const ssize_t x_offset,
  const ssize_t y_offset,ExceptionInfo *exception)
{
#define CompositeImageTag  "Composite/Image"

  CacheView
    *source_view,
    *image_view;

  const char
    *value;

  GeometryInfo
    geometry_info;

  Image
    *canvas_image,
    *source_image;

  MagickBooleanType
    clamp,
    status;

  MagickOffsetType
    progress;

  MagickRealType
    amount,
    canvas_dissolve,
    midpoint,
    percent_luma,
    percent_chroma,
    source_dissolve,
    threshold;

  MagickStatusType
    flags;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(composite != (Image *) NULL);
  assert(composite->signature == MagickCoreSignature);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  source_image=CloneImage(composite,0,0,MagickTrue,exception);
  if (source_image == (const Image *) NULL)
    return(MagickFalse);
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    (void) SetImageColorspace(image,sRGBColorspace,exception);
  (void) SetImageColorspace(source_image,image->colorspace,exception);
  if ((compose == OverCompositeOp) || (compose == SrcOverCompositeOp))
    {
      status=CompositeOverImage(image,source_image,clip_to_self,x_offset,
        y_offset,exception);
      source_image=DestroyImage(source_image);
      return(status);
    }
  amount=0.5;
  canvas_image=(Image *) NULL;
  canvas_dissolve=1.0;
  clamp=MagickTrue;
  value=GetImageArtifact(image,"compose:clamp");
  if (value != (const char *) NULL)
    clamp=IsStringTrue(value);
  SetGeometryInfo(&geometry_info);
  percent_luma=100.0;
  percent_chroma=100.0;
  source_dissolve=1.0;
  threshold=0.05f;
  switch (compose)
  {
    case CopyCompositeOp:
    {
      if ((x_offset < 0) || (y_offset < 0))
        break;
      if ((x_offset+(ssize_t) source_image->columns) > (ssize_t) image->columns)
        break;
      if ((y_offset+(ssize_t) source_image->rows) > (ssize_t) image->rows)
        break;
      status=MagickTrue;
      source_view=AcquireVirtualCacheView(source_image,exception);
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(source_image,image,source_image->rows,1)
#endif
      for (y=0; y < (ssize_t) source_image->rows; y++)
      {
        MagickBooleanType
          sync;

        register const Quantum
          *p;

        register Quantum
          *q;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        p=GetCacheViewVirtualPixels(source_view,0,y,source_image->columns,1,
          exception);
        q=GetCacheViewAuthenticPixels(image_view,x_offset,y+y_offset,
          source_image->columns,1,exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) source_image->columns; x++)
        {
          register ssize_t
            i;

          if (GetPixelReadMask(source_image,p) <= (QuantumRange/2))
            {
              p+=GetPixelChannels(source_image);
              q+=GetPixelChannels(image);
              continue;
            }
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            PixelTrait source_traits=GetPixelChannelTraits(source_image,
              channel);
            if (traits == UndefinedPixelTrait)
              continue;
            if (source_traits != UndefinedPixelTrait)
              SetPixelChannel(image,channel,p[i],q);
            else if (channel == AlphaPixelChannel)
              SetPixelChannel(image,channel,OpaqueAlpha,q);
          }
          p+=GetPixelChannels(source_image);
          q+=GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp critical (MagickCore_CompositeImage)
#endif
            proceed=SetImageProgress(image,CompositeImageTag,
              (MagickOffsetType) y,image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      source_view=DestroyCacheView(source_view);
      image_view=DestroyCacheView(image_view);
      source_image=DestroyImage(source_image);
      return(status);
    }
    case IntensityCompositeOp:
    {
      if ((x_offset < 0) || (y_offset < 0))
        break;
      if ((x_offset+(ssize_t) source_image->columns) > (ssize_t) image->columns)
        break;
      if ((y_offset+(ssize_t) source_image->rows) > (ssize_t) image->rows)
        break;
      status=MagickTrue;
      source_view=AcquireVirtualCacheView(source_image,exception);
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(source_image,image,source_image->rows,1)
#endif
      for (y=0; y < (ssize_t) source_image->rows; y++)
      {
        MagickBooleanType
          sync;

        register const Quantum
          *p;

        register Quantum
          *q;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        p=GetCacheViewVirtualPixels(source_view,0,y,source_image->columns,1,
          exception);
        q=GetCacheViewAuthenticPixels(image_view,x_offset,y+y_offset,
          source_image->columns,1,exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) source_image->columns; x++)
        {
          if (GetPixelReadMask(source_image,p) <= (QuantumRange/2))
            {
              p+=GetPixelChannels(source_image);
              q+=GetPixelChannels(image);
              continue;
            }
          SetPixelAlpha(image,clamp != MagickFalse ?
            ClampPixel(GetPixelIntensity(source_image,p)) :
            ClampToQuantum(GetPixelIntensity(source_image,p)),q);
          p+=GetPixelChannels(source_image);
          q+=GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp critical (MagickCore_CompositeImage)
#endif
            proceed=SetImageProgress(image,CompositeImageTag,
              (MagickOffsetType) y,image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      source_view=DestroyCacheView(source_view);
      image_view=DestroyCacheView(image_view);
      source_image=DestroyImage(source_image);
      return(status);
    }
    case CopyAlphaCompositeOp:
    case ChangeMaskCompositeOp:
    {
      /*
        Modify canvas outside the overlaid region and require an alpha
        channel to exist, to add transparency.
      */
      if (image->alpha_trait == UndefinedPixelTrait)
        (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
      break;
    }
    case BlurCompositeOp:
    {
      CacheView
        *canvas_view;

      MagickRealType
        angle_range,
        angle_start,
        height,
        width;

      PixelInfo
        pixel;

      ResampleFilter
        *resample_filter;

      SegmentInfo
        blur;

      /*
        Blur Image by resampling.

        Blur Image dictated by an overlay gradient map: X = red_channel;
          Y = green_channel; compose:args =  x_scale[,y_scale[,angle]].
      */
      canvas_image=CloneImage(image,image->columns,image->rows,MagickTrue,
        exception);
      if (canvas_image == (Image *) NULL)
        {
          source_image=DestroyImage(source_image);
          return(MagickFalse);
        }
      /*
        Gather the maximum blur sigma values from user.
      */
      flags=NoValue;
      value=GetImageArtifact(image,"compose:args");
      if (value != (const char *) NULL)
        flags=ParseGeometry(value,&geometry_info);
      if ((flags & WidthValue) == 0)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
            "InvalidSetting","'%s' '%s'","compose:args",value);
          source_image=DestroyImage(source_image);
          canvas_image=DestroyImage(canvas_image);
          return(MagickFalse);
        }
      /*
        Users input sigma now needs to be converted to the EWA ellipse size.
        The filter defaults to a sigma of 0.5 so to make this match the
        users input the ellipse size needs to be doubled.
      */
      width=height=geometry_info.rho*2.0;
      if ((flags & HeightValue) != 0 )
        height=geometry_info.sigma*2.0;
      /*
        Default the unrotated ellipse width and height axis vectors.
      */
      blur.x1=width;
      blur.x2=0.0;
      blur.y1=0.0;
      blur.y2=height;
      /* rotate vectors if a rotation angle is given */
      if ((flags & XValue) != 0 )
        {
          MagickRealType
            angle;

          angle=DegreesToRadians(geometry_info.xi);
          blur.x1=width*cos(angle);
          blur.x2=width*sin(angle);
          blur.y1=(-height*sin(angle));
          blur.y2=height*cos(angle);
        }
      /* Otherwise lets set a angle range and calculate in the loop */
      angle_start=0.0;
      angle_range=0.0;
      if ((flags & YValue) != 0 )
        {
          angle_start=DegreesToRadians(geometry_info.xi);
          angle_range=DegreesToRadians(geometry_info.psi)-angle_start;
        }
      /*
        Set up a gaussian cylindrical filter for EWA Bluring.

        As the minimum ellipse radius of support*1.0 the EWA algorithm
        can only produce a minimum blur of 0.5 for Gaussian (support=2.0)
        This means that even 'No Blur' will be still a little blurry!

        The solution (as well as the problem of preventing any user
        expert filter settings, is to set our own user settings, then
        restore them afterwards.
      */
      resample_filter=AcquireResampleFilter(image,exception);
      SetResampleFilter(resample_filter,GaussianFilter);

      /* do the variable blurring of each pixel in image */
      GetPixelInfo(image,&pixel);
      source_view=AcquireVirtualCacheView(source_image,exception);
      canvas_view=AcquireAuthenticCacheView(canvas_image,exception);
      for (y=0; y < (ssize_t) source_image->rows; y++)
      {
        MagickBooleanType
          sync;

        register const Quantum
          *magick_restrict p;

        register Quantum
          *magick_restrict q;

        register ssize_t
          x;

        if (((y+y_offset) < 0) || ((y+y_offset) >= (ssize_t) image->rows))
          continue;
        p=GetCacheViewVirtualPixels(source_view,0,y,source_image->columns,1,
          exception);
        q=QueueCacheViewAuthenticPixels(canvas_view,0,y,canvas_image->columns,1,
          exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          break;
        for (x=0; x < (ssize_t) source_image->columns; x++)
        {
          if (((x_offset+x) < 0) || ((x_offset+x) >= (ssize_t) image->columns))
            {
              p+=GetPixelChannels(source_image);
              continue;
            }
          if (fabs((double) angle_range) > MagickEpsilon)
            {
              MagickRealType
                angle;

              angle=angle_start+angle_range*QuantumScale*
                GetPixelBlue(source_image,p);
              blur.x1=width*cos(angle);
              blur.x2=width*sin(angle);
              blur.y1=(-height*sin(angle));
              blur.y2=height*cos(angle);
            }
#if 0
          if ( x == 10 && y == 60 ) {
            (void) fprintf(stderr, "blur.x=%lf,%lf, blur.y=%lf,%lf\n",blur.x1,
              blur.x2,blur.y1, blur.y2);
            (void) fprintf(stderr, "scaled by=%lf,%lf\n",QuantumScale*
              GetPixelRed(p),QuantumScale*GetPixelGreen(p));
#endif
          ScaleResampleFilter(resample_filter,
            blur.x1*QuantumScale*GetPixelRed(source_image,p),
            blur.y1*QuantumScale*GetPixelGreen(source_image,p),
            blur.x2*QuantumScale*GetPixelRed(source_image,p),
            blur.y2*QuantumScale*GetPixelGreen(source_image,p) );
          (void) ResamplePixelColor(resample_filter,(double) x_offset+x,
            (double) y_offset+y,&pixel,exception);
          SetPixelViaPixelInfo(canvas_image,&pixel,q);
          p+=GetPixelChannels(source_image);
          q+=GetPixelChannels(canvas_image);
        }
        sync=SyncCacheViewAuthenticPixels(canvas_view,exception);
        if (sync == MagickFalse)
          break;
      }
      resample_filter=DestroyResampleFilter(resample_filter);
      source_view=DestroyCacheView(source_view);
      canvas_view=DestroyCacheView(canvas_view);
      source_image=DestroyImage(source_image);
      source_image=canvas_image;
      break;
    }
    case DisplaceCompositeOp:
    case DistortCompositeOp:
    {
      CacheView
        *canvas_view;

      MagickRealType
        horizontal_scale,
        vertical_scale;

      PixelInfo
        pixel;

      PointInfo
        center,
        offset;

      /*
        Displace/Distort based on overlay gradient map:
          X = red_channel;  Y = green_channel;
          compose:args = x_scale[,y_scale[,center.x,center.y]]
      */
      canvas_image=CloneImage(image,image->columns,image->rows,MagickTrue,
        exception);
      if (canvas_image == (Image *) NULL)
        {
          source_image=DestroyImage(source_image);
          return(MagickFalse);
        }
      SetGeometryInfo(&geometry_info);
      flags=NoValue;
      value=GetImageArtifact(image,"compose:args");
      if (value != (char *) NULL)
        flags=ParseGeometry(value,&geometry_info);
      if ((flags & (WidthValue | HeightValue)) == 0 )
        {
          if ((flags & AspectValue) == 0)
            {
              horizontal_scale=(MagickRealType) (source_image->columns-1)/2.0;
              vertical_scale=(MagickRealType) (source_image->rows-1)/2.0;
            }
          else
            {
              horizontal_scale=(MagickRealType) (image->columns-1)/2.0;
              vertical_scale=(MagickRealType) (image->rows-1)/2.0;
            }
        }
      else
        {
          horizontal_scale=geometry_info.rho;
          vertical_scale=geometry_info.sigma;
          if ((flags & PercentValue) != 0)
            {
              if ((flags & AspectValue) == 0)
                {
                  horizontal_scale*=(source_image->columns-1)/200.0;
                  vertical_scale*=(source_image->rows-1)/200.0;
                }
              else
                {
                  horizontal_scale*=(image->columns-1)/200.0;
                  vertical_scale*=(image->rows-1)/200.0;
                }
            }
          if ((flags & HeightValue) == 0)
            vertical_scale=horizontal_scale;
        }
      /*
        Determine fixed center point for absolute distortion map
         Absolute distort ==
           Displace offset relative to a fixed absolute point
           Select that point according to +X+Y user inputs.
           default = center of overlay image
           arg flag '!' = locations/percentage relative to background image
      */
      center.x=(MagickRealType) x_offset;
      center.y=(MagickRealType) y_offset;
      if (compose == DistortCompositeOp)
        {
          if ((flags & XValue) == 0)
            if ((flags & AspectValue) != 0)
              center.x=(MagickRealType) ((image->columns-1)/2.0);
            else
              center.x=(MagickRealType) (x_offset+(source_image->columns-1)/
                2.0);
          else
            if ((flags & AspectValue) != 0)
              center.x=geometry_info.xi;
            else
              center.x=(MagickRealType) (x_offset+geometry_info.xi);
          if ((flags & YValue) == 0)
            if ((flags & AspectValue) != 0)
              center.y=(MagickRealType) ((image->rows-1)/2.0);
            else
              center.y=(MagickRealType) (y_offset+(source_image->rows-1)/2.0);
          else
            if ((flags & AspectValue) != 0)
              center.y=geometry_info.psi;
            else
              center.y=(MagickRealType) (y_offset+geometry_info.psi);
        }
      /*
        Shift the pixel offset point as defined by the provided,
        displacement/distortion map.  -- Like a lens...
      */
      GetPixelInfo(image,&pixel);
      image_view=AcquireVirtualCacheView(image,exception);
      source_view=AcquireVirtualCacheView(source_image,exception);
      canvas_view=AcquireAuthenticCacheView(canvas_image,exception);
      for (y=0; y < (ssize_t) source_image->rows; y++)
      {
        MagickBooleanType
          sync;

        register const Quantum
          *magick_restrict p;

        register Quantum
          *magick_restrict q;

        register ssize_t
          x;

        if (((y+y_offset) < 0) || ((y+y_offset) >= (ssize_t) image->rows))
          continue;
        p=GetCacheViewVirtualPixels(source_view,0,y,source_image->columns,1,
          exception);
        q=QueueCacheViewAuthenticPixels(canvas_view,0,y,canvas_image->columns,1,
          exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          break;
        for (x=0; x < (ssize_t) source_image->columns; x++)
        {
          if (((x_offset+x) < 0) || ((x_offset+x) >= (ssize_t) image->columns))
            {
              p+=GetPixelChannels(source_image);
              continue;
            }
          /*
            Displace the offset.
          */
          offset.x=(double) (horizontal_scale*(GetPixelRed(source_image,p)-
            (((MagickRealType) QuantumRange+1.0)/2.0)))/(((MagickRealType)
            QuantumRange+1.0)/2.0)+center.x+((compose == DisplaceCompositeOp) ?
            x : 0);
          offset.y=(double) (vertical_scale*(GetPixelGreen(source_image,p)-
            (((MagickRealType) QuantumRange+1.0)/2.0)))/(((MagickRealType)
            QuantumRange+1.0)/2.0)+center.y+((compose == DisplaceCompositeOp) ?
            y : 0);
          status=InterpolatePixelInfo(image,image_view,
            UndefinedInterpolatePixel,(double) offset.x,(double) offset.y,
            &pixel,exception);
          if (status == MagickFalse)
            break;
          /*
            Mask with the 'invalid pixel mask' in alpha channel.
          */
          pixel.alpha=(MagickRealType) QuantumRange*(QuantumScale*pixel.alpha)*
            (QuantumScale*GetPixelAlpha(source_image,p));
          SetPixelViaPixelInfo(canvas_image,&pixel,q);
          p+=GetPixelChannels(source_image);
          q+=GetPixelChannels(canvas_image);
        }
        if (x < (ssize_t) source_image->columns)
          break;
        sync=SyncCacheViewAuthenticPixels(canvas_view,exception);
        if (sync == MagickFalse)
          break;
      }
      canvas_view=DestroyCacheView(canvas_view);
      source_view=DestroyCacheView(source_view);
      image_view=DestroyCacheView(image_view);
      source_image=DestroyImage(source_image);
      source_image=canvas_image;
      break;
    }
    case DissolveCompositeOp:
    {
      /*
        Geometry arguments to dissolve factors.
      */
      value=GetImageArtifact(image,"compose:args");
      if (value != (char *) NULL)
        {
          flags=ParseGeometry(value,&geometry_info);
          source_dissolve=geometry_info.rho/100.0;
          canvas_dissolve=1.0;
          if ((source_dissolve-MagickEpsilon) < 0.0)
            source_dissolve=0.0;
          if ((source_dissolve+MagickEpsilon) > 1.0)
            {
              canvas_dissolve=2.0-source_dissolve;
              source_dissolve=1.0;
            }
          if ((flags & SigmaValue) != 0)
            canvas_dissolve=geometry_info.sigma/100.0;
          if ((canvas_dissolve-MagickEpsilon) < 0.0)
            canvas_dissolve=0.0;
        }
      break;
    }
    case BlendCompositeOp:
    {
      value=GetImageArtifact(image,"compose:args");
      if (value != (char *) NULL)
        {
          flags=ParseGeometry(value,&geometry_info);
          source_dissolve=geometry_info.rho/100.0;
          canvas_dissolve=1.0-source_dissolve;
          if ((flags & SigmaValue) != 0)
            canvas_dissolve=geometry_info.sigma/100.0;
        }
      break;
    }
    case MathematicsCompositeOp:
    {
      /*
        Just collect the values from "compose:args", setting.
        Unused values are set to zero automagically.

        Arguments are normally a comma separated list, so this probably should
        be changed to some 'general comma list' parser, (with a minimum
        number of values)
      */
      SetGeometryInfo(&geometry_info);
      value=GetImageArtifact(image,"compose:args");
      if (value != (char *) NULL)
        (void) ParseGeometry(value,&geometry_info);
      break;
    }
    case ModulateCompositeOp:
    {
      /*
        Determine the luma and chroma scale.
      */
      value=GetImageArtifact(image,"compose:args");
      if (value != (char *) NULL)
        {
          flags=ParseGeometry(value,&geometry_info);
          percent_luma=geometry_info.rho;
          if ((flags & SigmaValue) != 0)
            percent_chroma=geometry_info.sigma;
        }
      break;
    }
    case ThresholdCompositeOp:
    {
      /*
        Determine the amount and threshold.
      */
      value=GetImageArtifact(image,"compose:args");
      if (value != (char *) NULL)
        {
          flags=ParseGeometry(value,&geometry_info);
          amount=geometry_info.rho;
          threshold=geometry_info.sigma;
          if ((flags & SigmaValue) == 0)
            threshold=0.05f;
        }
      threshold*=QuantumRange;
      break;
    }
    default:
      break;
  }
  /*
    Composite image.
  */
  status=MagickTrue;
  progress=0;
  midpoint=((MagickRealType) QuantumRange+1.0)/2;
  source_view=AcquireVirtualCacheView(source_image,exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_number_threads(source_image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *pixels;

    MagickRealType
      blue,
      chroma,
      green,
      hue,
      luma,
      red;

    PixelInfo
      canvas_pixel,
      source_pixel;

    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    if (clip_to_self != MagickFalse)
      {
        if (y < y_offset)
          continue;
        if ((y-y_offset) >= (ssize_t) source_image->rows)
          continue;
      }
    /*
      If pixels is NULL, y is outside overlay region.
    */
    pixels=(Quantum *) NULL;
    p=(Quantum *) NULL;
    if ((y >= y_offset) && ((y-y_offset) < (ssize_t) source_image->rows))
      {
        p=GetCacheViewVirtualPixels(source_view,0,y-y_offset,
          source_image->columns,1,exception);
        if (p == (const Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        pixels=p;
        if (x_offset < 0)
          p-=x_offset*GetPixelChannels(source_image);
      }
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    hue=0.0;
    chroma=0.0;
    luma=0.0;
    GetPixelInfo(image,&canvas_pixel);
    GetPixelInfo(source_image,&source_pixel);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        gamma;

      MagickRealType
        alpha,
        Da,
        Dc,
        Dca,
        DcaDa,
        Sa,
        SaSca,
        Sc,
        Sca;

      register ssize_t
        i;

      size_t
        channels;

      if (clip_to_self != MagickFalse)
        {
          if (x < x_offset)
            {
              q+=GetPixelChannels(image);
              continue;
            }
          if ((x-x_offset) >= (ssize_t) source_image->columns)
            break;
        }
      if ((pixels == (Quantum *) NULL) || (x < x_offset) ||
          ((x-x_offset) >= (ssize_t) source_image->columns))
        {
          Quantum
            source[MaxPixelChannels];

          /*
            Virtual composite:
              Sc: source color.
              Dc: canvas color.
          */
          (void) GetOneVirtualPixel(source_image,x-x_offset,y-y_offset,source,
            exception);
          if (GetPixelWriteMask(image,q) <= (QuantumRange/2))
            {
              q+=GetPixelChannels(image);
              continue;
            }
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            MagickRealType
              pixel;

            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            PixelTrait source_traits=GetPixelChannelTraits(source_image,
              channel);
            if ((traits == UndefinedPixelTrait) ||
                (source_traits == UndefinedPixelTrait))
              continue;
            switch (compose)
            {
              case AlphaCompositeOp:
              case ChangeMaskCompositeOp:
              case CopyAlphaCompositeOp:
              case DstAtopCompositeOp:
              case DstInCompositeOp:
              case InCompositeOp:
              case OutCompositeOp:
              case SrcInCompositeOp:
              case SrcOutCompositeOp:
              {
                if (channel == AlphaPixelChannel)
                  pixel=(MagickRealType) TransparentAlpha;
                else
                  pixel=(MagickRealType) q[i];
                break;
              }
              case ClearCompositeOp:
              case CopyCompositeOp:
              case ReplaceCompositeOp:
              case SrcCompositeOp:
              {
                if (channel == AlphaPixelChannel)
                  pixel=(MagickRealType) TransparentAlpha;
                else
                  pixel=0.0;
                break;
              }
              case BlendCompositeOp:
              case DissolveCompositeOp:
              {
                if (channel == AlphaPixelChannel)
                  pixel=canvas_dissolve*GetPixelAlpha(source_image,source);
                else
                  pixel=(MagickRealType) source[channel];
                break;
              }
              default:
              {
                pixel=(MagickRealType) source[channel];
                break;
              }
            }
            q[i]=clamp != MagickFalse ? ClampPixel(pixel) :
              ClampToQuantum(pixel);
          }
          q+=GetPixelChannels(image);
          continue;
        }
      /*
        Authentic composite:
          Sa:  normalized source alpha.
          Da:  normalized canvas alpha.
      */
      Sa=QuantumScale*GetPixelAlpha(source_image,p);
      Da=QuantumScale*GetPixelAlpha(image,q);
      switch (compose)
      {
        case BumpmapCompositeOp:
        {
          alpha=GetPixelIntensity(source_image,p)*Sa;
          break;
        }
        case ColorBurnCompositeOp:
        case ColorDodgeCompositeOp:
        case DarkenCompositeOp:
        case DifferenceCompositeOp:
        case DivideDstCompositeOp:
        case DivideSrcCompositeOp:
        case ExclusionCompositeOp:
        case HardLightCompositeOp:
        case HardMixCompositeOp:
        case LinearBurnCompositeOp:
        case LinearDodgeCompositeOp:
        case LinearLightCompositeOp:
        case LightenCompositeOp:
        case MathematicsCompositeOp:
        case MinusDstCompositeOp:
        case MinusSrcCompositeOp:
        case ModulusAddCompositeOp:
        case ModulusSubtractCompositeOp:
        case MultiplyCompositeOp:
        case OverlayCompositeOp:
        case PegtopLightCompositeOp:
        case PinLightCompositeOp:
        case ScreenCompositeOp:
        case SoftLightCompositeOp:
        case VividLightCompositeOp:
        {
          alpha=RoundToUnity(Sa+Da-Sa*Da);
          break;
        }
        case DstAtopCompositeOp:
        case DstInCompositeOp:
        case InCompositeOp:
        case SrcInCompositeOp:
        {
          alpha=Sa*Da;
          break;
        }
        case DissolveCompositeOp:
        {
          alpha=source_dissolve*Sa*(-canvas_dissolve*Da)+source_dissolve*Sa+
            canvas_dissolve*Da;
          break;
        }
        case DstOverCompositeOp:
        case OverCompositeOp:
        case SrcOverCompositeOp:
        {
          alpha=Sa+Da-Sa*Da;
          break;
        }
        case DstOutCompositeOp:
        {
          alpha=Da*(1.0-Sa);
          break;
        }
        case OutCompositeOp:
        case SrcOutCompositeOp:
        {
          alpha=Sa*(1.0-Da);
          break;
        }
        case BlendCompositeOp:
        case PlusCompositeOp:
        {
          alpha=RoundToUnity(source_dissolve*Sa+canvas_dissolve*Da);
          break;
        }
        case XorCompositeOp:
        {
          alpha=Sa+Da-2.0*Sa*Da;
          break;
        }
        default:
        {
          alpha=1.0;
          break;
        }
      }
      if (GetPixelWriteMask(image,q) <= (QuantumRange/2))
        {
          p+=GetPixelChannels(source_image);
          q+=GetPixelChannels(image);
          continue;
        }
      switch (compose)
      {
        case ColorizeCompositeOp:
        case HueCompositeOp:
        case LuminizeCompositeOp:
        case ModulateCompositeOp:
        case SaturateCompositeOp:
        {
          GetPixelInfoPixel(source_image,p,&source_pixel);
          GetPixelInfoPixel(image,q,&canvas_pixel);
          break;
        }
        default:
          break;
      }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        MagickRealType
          pixel,
          sans;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait source_traits=GetPixelChannelTraits(source_image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if (channel == AlphaPixelChannel)
          {
            /*
              Set alpha channel.
            */
            switch (compose)
            {
              case AlphaCompositeOp:
              {
                pixel=QuantumRange*Sa;
                break;
              }
              case AtopCompositeOp:
              case CopyBlackCompositeOp:
              case CopyBlueCompositeOp:
              case CopyCyanCompositeOp:
              case CopyGreenCompositeOp:
              case CopyMagentaCompositeOp:
              case CopyRedCompositeOp:
              case CopyYellowCompositeOp:
              case SrcAtopCompositeOp:
              case DstCompositeOp:
              case NoCompositeOp:
              {
                pixel=QuantumRange*Da;
                break;
              }
              case ChangeMaskCompositeOp:
              {
                MagickBooleanType
                  equivalent;

                if (Da < 0.5)
                  {
                    pixel=(MagickRealType) TransparentAlpha;
                    break;
                  }
                equivalent=IsFuzzyEquivalencePixel(source_image,p,image,q);
                if (equivalent != MagickFalse)
                  pixel=(MagickRealType) TransparentAlpha;
                else
                  pixel=(MagickRealType) OpaqueAlpha;
                break;
              }
              case ClearCompositeOp:
              {
                pixel=(MagickRealType) TransparentAlpha;
                break;
              }
              case ColorizeCompositeOp:
              case HueCompositeOp:
              case LuminizeCompositeOp:
              case SaturateCompositeOp:
              {
                if (fabs((double) (QuantumRange*Sa-TransparentAlpha)) < MagickEpsilon)
                  {
                    pixel=QuantumRange*Da;
                    break;
                  }
                if (fabs((double) (QuantumRange*Da-TransparentAlpha)) < MagickEpsilon)
                  {
                    pixel=QuantumRange*Sa;
                    break;
                  }
                if (Sa < Da)
                  {
                    pixel=QuantumRange*Da;
                    break;
                  }
                pixel=QuantumRange*Sa;
                break;
              }
              case CopyAlphaCompositeOp:
              {
                if (source_image->alpha_trait == UndefinedPixelTrait)
                  pixel=GetPixelIntensity(source_image,p);
                else
                  pixel=QuantumRange*Sa;
                break;
              }
              case CopyCompositeOp:
              case DisplaceCompositeOp:
              case DistortCompositeOp:
              case DstAtopCompositeOp:
              case ReplaceCompositeOp:
              case SrcCompositeOp:
              {
                pixel=QuantumRange*Sa;
                break;
              }
              case DarkenIntensityCompositeOp:
              {
                pixel=Sa*GetPixelIntensity(source_image,p) <
                  Da*GetPixelIntensity(image,q) ? Sa : Da;
                break;
              }
              case LightenIntensityCompositeOp:
              {
                pixel=Sa*GetPixelIntensity(source_image,p) >
                  Da*GetPixelIntensity(image,q) ? Sa : Da;
                break;
              }
              case ModulateCompositeOp:
              {
                pixel=QuantumRange*Da;
                break;
              }
              case MultiplyCompositeOp:
              {
                pixel=QuantumRange*Sa*Da;
                break;
              }
              default:
              {
                pixel=QuantumRange*alpha;
                break;
              }
            }
            q[i]=clamp != MagickFalse ? ClampPixel(pixel) :
              ClampToQuantum(pixel);
            continue;
          }
        if (source_traits == UndefinedPixelTrait)
          continue;
        /*
          Sc: source color.
          Dc: canvas color.
        */
        Sc=(MagickRealType) GetPixelChannel(source_image,channel,p);
        Dc=(MagickRealType) q[i];
        if ((traits & CopyPixelTrait) != 0)
          {
            /*
              Copy channel.
            */
            q[i]=Dc;
            continue;
          }
        /*
          Porter-Duff compositions:
            Sca: source normalized color multiplied by alpha.
            Dca: normalized canvas color multiplied by alpha.
        */
        Sca=QuantumScale*Sa*Sc;
        Dca=QuantumScale*Da*Dc;
        SaSca=Sa*PerceptibleReciprocal(Sca);
        DcaDa=Dca*PerceptibleReciprocal(Da);
        switch (compose)
        {
          case DarkenCompositeOp:
          case LightenCompositeOp:
          case ModulusSubtractCompositeOp:
          {
            gamma=PerceptibleReciprocal(1.0-alpha);
            break;
          }
          default:
          {
            gamma=PerceptibleReciprocal(alpha);
            break;
          }
        }
        pixel=Dc;
        switch (compose)
        {
          case AlphaCompositeOp:
          {
            pixel=QuantumRange*Sa;
            break;
          }
          case AtopCompositeOp:
          case SrcAtopCompositeOp:
          {
            pixel=QuantumRange*(Sca*Da+Dca*(1.0-Sa));
            break;
          }
          case BlendCompositeOp:
          {
            pixel=gamma*(source_dissolve*Sa*Sc+canvas_dissolve*Da*Dc);
            break;
          }
          case BlurCompositeOp:
          case CopyCompositeOp:
          case ReplaceCompositeOp:
          case SrcCompositeOp:
          {
            pixel=QuantumRange*Sca;
            break;
          }
          case DisplaceCompositeOp:
          case DistortCompositeOp:
          {
            pixel=Sc;
            break;
          }
          case BumpmapCompositeOp:
          {
            if (fabs((double) (QuantumRange*Sa-TransparentAlpha)) < MagickEpsilon)
              {
                pixel=Dc;
                break;
              }
            pixel=QuantumScale*GetPixelIntensity(source_image,p)*Dc;
            break;
          }
          case ChangeMaskCompositeOp:
          {
            pixel=Dc;
            break;
          }
          case ClearCompositeOp:
          {
            pixel=0.0;
            break;
          }
          case ColorBurnCompositeOp:
          {
            if ((Sca == 0.0) && (Dca == Da))
              {
                pixel=QuantumRange*gamma*(Sa*Da+Dca*(1.0-Sa));
                break;
              }
            if (Sca == 0.0)
              {
                pixel=QuantumRange*gamma*(Dca*(1.0-Sa));
                break;
              }
            pixel=QuantumRange*gamma*(Sa*Da-Sa*Da*MagickMin(1.0,(1.0-DcaDa)*
              SaSca)+Sca*(1.0-Da)+Dca*(1.0-Sa));
            break;
          }
          case ColorDodgeCompositeOp:
          {
            if ((Sca*Da+Dca*Sa) >= Sa*Da)
              pixel=QuantumRange*gamma*(Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
            else
              pixel=QuantumRange*gamma*(Dca*Sa*Sa*PerceptibleReciprocal(Sa-Sca)+
                Sca*(1.0-Da)+Dca*(1.0-Sa));
            break;
          }
          case ColorizeCompositeOp:
          {
            if (fabs((double) (QuantumRange*Sa-TransparentAlpha)) < MagickEpsilon)
              {
                pixel=Dc;
                break;
              }
            if (fabs((double) (QuantumRange*Da-TransparentAlpha)) < MagickEpsilon)
              {
                pixel=Sc;
                break;
              }
            CompositeHCL(canvas_pixel.red,canvas_pixel.green,canvas_pixel.blue,
              &sans,&sans,&luma);
            CompositeHCL(source_pixel.red,source_pixel.green,source_pixel.blue,
              &hue,&chroma,&sans);
            HCLComposite(hue,chroma,luma,&red,&green,&blue);
            switch (channel)
            {
              case RedPixelChannel: pixel=red; break;
              case GreenPixelChannel: pixel=green; break;
              case BluePixelChannel: pixel=blue; break;
              default: pixel=Dc; break;
            }
            break;
          }
          case CopyAlphaCompositeOp:
          {
            pixel=Dc;
            break;
          }
          case CopyBlackCompositeOp:
          {
            if (channel == BlackPixelChannel)
              pixel=(MagickRealType) (QuantumRange-
                GetPixelBlack(source_image,p));
            break;
          }
          case CopyBlueCompositeOp:
          case CopyYellowCompositeOp:
          {
            if (channel == BluePixelChannel)
              pixel=(MagickRealType) GetPixelBlue(source_image,p);
            break;
          }
          case CopyGreenCompositeOp:
          case CopyMagentaCompositeOp:
          {
            if (channel == GreenPixelChannel)
              pixel=(MagickRealType) GetPixelGreen(source_image,p);
            break;
          }
          case CopyRedCompositeOp:
          case CopyCyanCompositeOp:
          {
            if (channel == RedPixelChannel)
              pixel=(MagickRealType) GetPixelRed(source_image,p);
            break;
          }
          case DarkenCompositeOp:
          {
            /*
              Darken is equivalent to a 'Minimum' method
                OR a greyscale version of a binary 'Or'
                OR the 'Intersection' of pixel sets.
            */
            if ((Sca*Da) < (Dca*Sa))
              {
                pixel=QuantumRange*(Sca+Dca*(1.0-Sa));
                break;
              }
            pixel=QuantumRange*(Dca+Sca*(1.0-Da));
            break;
          }
          case DarkenIntensityCompositeOp:
          {
            pixel=Sa*GetPixelIntensity(source_image,p) <
              Da*GetPixelIntensity(image,q) ? Sc : Dc;
            break;
          }
          case DifferenceCompositeOp:
          {
            pixel=QuantumRange*gamma*(Sca+Dca-2.0*MagickMin(Sca*Da,Dca*Sa));
            break;
          }
          case DissolveCompositeOp:
          {
            pixel=gamma*(source_dissolve*Sa*Sc-source_dissolve*Sa*
              canvas_dissolve*Da*Dc+canvas_dissolve*Da*Dc);
            break;
          }
          case DivideDstCompositeOp:
          {
            if ((fabs((double) Sca) < MagickEpsilon) &&
                (fabs((double) Dca) < MagickEpsilon))
              {
                pixel=QuantumRange*gamma*(Sca*(1.0-Da)+Dca*(1.0-Sa));
                break;
              }
            if (fabs((double) Dca) < MagickEpsilon)
              {
                pixel=QuantumRange*gamma*(Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
                break;
              }
            pixel=QuantumRange*gamma*(Sca*Da*Da/Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
            break;
          }
          case DivideSrcCompositeOp:
          {
            if ((fabs((double) Dca) < MagickEpsilon) &&
                (fabs((double) Sca) < MagickEpsilon))
              {
                pixel=QuantumRange*gamma*(Dca*(1.0-Sa)+Sca*(1.0-Da));
                break;
              }
            if (fabs((double) Sca) < MagickEpsilon)
              {
                pixel=QuantumRange*gamma*(Da*Sa+Dca*(1.0-Sa)+Sca*(1.0-Da));
                break;
              }
            pixel=QuantumRange*gamma*(Dca*Sa*SaSca+Dca*(1.0-Sa)+Sca*(1.0-Da));
            break;
          }
          case DstAtopCompositeOp:
          {
            pixel=QuantumRange*(Dca*Sa+Sca*(1.0-Da));
            break;
          }
          case DstCompositeOp:
          case NoCompositeOp:
          {
            pixel=QuantumRange*Dca;
            break;
          }
          case DstInCompositeOp:
          {
            pixel=QuantumRange*(Dca*Sa);
            break;
          }
          case DstOutCompositeOp:
          {
            pixel=QuantumRange*(Dca*(1.0-Sa));
            break;
          }
          case DstOverCompositeOp:
          {
            pixel=QuantumRange*gamma*(Dca+Sca*(1.0-Da));
            break;
          }
          case ExclusionCompositeOp:
          {
            pixel=QuantumRange*gamma*(Sca*Da+Dca*Sa-2.0*Sca*Dca+Sca*(1.0-Da)+
              Dca*(1.0-Sa));
            break;
          }
          case HardLightCompositeOp:
          {
            if ((2.0*Sca) < Sa)
              {
                pixel=QuantumRange*gamma*(2.0*Sca*Dca+Sca*(1.0-Da)+Dca*(1.0-
                  Sa));
                break;
              }
            pixel=QuantumRange*gamma*(Sa*Da-2.0*(Da-Dca)*(Sa-Sca)+Sca*(1.0-Da)+
              Dca*(1.0-Sa));
            break;
          }
          case HardMixCompositeOp:
          {
            pixel=gamma*(((Sca+Dca) < 1.0) ? 0.0 : QuantumRange);
            break;
          }
          case HueCompositeOp:
          {
            if (fabs((double) (QuantumRange*Sa-TransparentAlpha)) < MagickEpsilon)
              {
                pixel=Dc;
                break;
              }
            if (fabs((double) (QuantumRange*Da-TransparentAlpha)) < MagickEpsilon)
              {
                pixel=Sc;
                break;
              }
            CompositeHCL(canvas_pixel.red,canvas_pixel.green,canvas_pixel.blue,
              &hue,&chroma,&luma);
            CompositeHCL(source_pixel.red,source_pixel.green,source_pixel.blue,
              &hue,&sans,&sans);
            HCLComposite(hue,chroma,luma,&red,&green,&blue);
            switch (channel)
            {
              case RedPixelChannel: pixel=red; break;
              case GreenPixelChannel: pixel=green; break;
              case BluePixelChannel: pixel=blue; break;
              default: pixel=Dc; break;
            }
            break;
          }
          case InCompositeOp:
          case SrcInCompositeOp:
          {
            pixel=QuantumRange*(Sca*Da);
            break;
          }
          case LinearBurnCompositeOp:
          {
            /*
              LinearBurn: as defined by Abode Photoshop, according to
              http://www.simplefilter.de/en/basics/mixmods.html is:

                f(Sc,Dc) = Sc + Dc - 1
            */
            pixel=QuantumRange*gamma*(Sca+Dca-Sa*Da);
            break;
          }
          case LinearDodgeCompositeOp:
          {
            pixel=gamma*(Sa*Sc+Da*Dc);
            break;
          }
          case LinearLightCompositeOp:
          {
            /*
              LinearLight: as defined by Abode Photoshop, according to
              http://www.simplefilter.de/en/basics/mixmods.html is:

                f(Sc,Dc) = Dc + 2*Sc - 1
            */
            pixel=QuantumRange*gamma*((Sca-Sa)*Da+Sca+Dca);
            break;
          }
          case LightenCompositeOp:
          {
            if ((Sca*Da) > (Dca*Sa))
              {
                pixel=QuantumRange*(Sca+Dca*(1.0-Sa));
                break;
              }
            pixel=QuantumRange*(Dca+Sca*(1.0-Da));
            break;
          }
          case LightenIntensityCompositeOp:
          {
            /*
              Lighten is equivalent to a 'Maximum' method
                OR a greyscale version of a binary 'And'
                OR the 'Union' of pixel sets.
            */
            pixel=Sa*GetPixelIntensity(source_image,p) >
              Da*GetPixelIntensity(image,q) ? Sc : Dc;
            break;
          }
          case LuminizeCompositeOp:
          {
            if (fabs((double) (QuantumRange*Sa-TransparentAlpha)) < MagickEpsilon)
              {
                pixel=Dc;
                break;
              }
            if (fabs((double) (QuantumRange*Da-TransparentAlpha)) < MagickEpsilon)
              {
                pixel=Sc;
                break;
              }
            CompositeHCL(canvas_pixel.red,canvas_pixel.green,canvas_pixel.blue,
              &hue,&chroma,&luma);
            CompositeHCL(source_pixel.red,source_pixel.green,source_pixel.blue,
              &sans,&sans,&luma);
            HCLComposite(hue,chroma,luma,&red,&green,&blue);
            switch (channel)
            {
              case RedPixelChannel: pixel=red; break;
              case GreenPixelChannel: pixel=green; break;
              case BluePixelChannel: pixel=blue; break;
              default: pixel=Dc; break;
            }
            break;
          }
          case MathematicsCompositeOp:
          {
            /*
              'Mathematics' a free form user control mathematical composition
              is defined as...

                f(Sc,Dc) = A*Sc*Dc + B*Sc + C*Dc + D

              Where the arguments A,B,C,D are (currently) passed to composite
              as a command separated 'geometry' string in "compose:args" image
              artifact.

                 A = a->rho,   B = a->sigma,  C = a->xi,  D = a->psi

              Applying the SVG transparency formula (see above), we get...

               Dca' = Sa*Da*f(Sc,Dc) + Sca*(1.0-Da) + Dca*(1.0-Sa)

               Dca' = A*Sca*Dca + B*Sca*Da + C*Dca*Sa + D*Sa*Da + Sca*(1.0-Da) +
                 Dca*(1.0-Sa)
            */
            pixel=QuantumRange*gamma*(geometry_info.rho*Sca*Dca+
              geometry_info.sigma*Sca*Da+geometry_info.xi*Dca*Sa+
              geometry_info.psi*Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
            break;
          }
          case MinusDstCompositeOp:
          {
            pixel=gamma*(Sa*Sc+Da*Dc-2.0*Da*Dc*Sa);
            break;
          }
          case MinusSrcCompositeOp:
          {
            /*
              Minus source from canvas.

                f(Sc,Dc) = Sc - Dc
            */
            pixel=gamma*(Da*Dc+Sa*Sc-2.0*Sa*Sc*Da);
            break;
          }
          case ModulateCompositeOp:
          {
            ssize_t
              offset;

            if (fabs((double) (QuantumRange*Sa-TransparentAlpha)) < MagickEpsilon)
              {
                pixel=Dc;
                break;
              }
            offset=(ssize_t) (GetPixelIntensity(source_image,p)-midpoint);
            if (offset == 0)
              {
                pixel=Dc;
                break;
              }
            CompositeHCL(canvas_pixel.red,canvas_pixel.green,canvas_pixel.blue,
              &hue,&chroma,&luma);
            luma+=(0.01*percent_luma*offset)/midpoint;
            chroma*=0.01*percent_chroma;
            HCLComposite(hue,chroma,luma,&red,&green,&blue);
            switch (channel)
            {
              case RedPixelChannel: pixel=red; break;
              case GreenPixelChannel: pixel=green; break;
              case BluePixelChannel: pixel=blue; break;
              default: pixel=Dc; break;
            }
            break;
          }
          case ModulusAddCompositeOp:
          {
            pixel=Sc+Dc;
            while (pixel > QuantumRange)
              pixel-=QuantumRange;
            while (pixel < 0.0)
              pixel+=QuantumRange;
            pixel=(Sa*Da*pixel+Sa*Sc*(1.0-Da)+Da*Dc*(1.0-Sa));
            break;
          }
          case ModulusSubtractCompositeOp:
          {
            pixel=Sc-Dc;
            while (pixel > QuantumRange)
              pixel-=QuantumRange;
            while (pixel < 0.0)
              pixel+=QuantumRange;
            pixel=(Sa*Da*pixel+Sa*Sc*(1.0-Da)+Da*Dc*(1.0-Sa));
            break;
          }
          case MultiplyCompositeOp:
          {
            pixel=QuantumRange*gamma*(Sca*Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
            break;
          }
          case OutCompositeOp:
          case SrcOutCompositeOp:
          {
            pixel=QuantumRange*(Sca*(1.0-Da));
            break;
          }
          case OverCompositeOp:
          case SrcOverCompositeOp:
          {
            pixel=QuantumRange*gamma*(Sca+Dca*(1.0-Sa));
            break;
          }
          case OverlayCompositeOp:
          {
            if ((2.0*Dca) < Da)
              {
                pixel=QuantumRange*gamma*(2.0*Dca*Sca+Dca*(1.0-Sa)+Sca*(1.0-
                  Da));
                break;
              }
            pixel=QuantumRange*gamma*(Da*Sa-2.0*(Sa-Sca)*(Da-Dca)+Dca*(1.0-Sa)+
              Sca*(1.0-Da));
            break;
          }
          case PegtopLightCompositeOp:
          {
            /*
              PegTop: A Soft-Light alternative: A continuous version of the
              Softlight function, producing very similar results.

                f(Sc,Dc) = Dc^2*(1-2*Sc) + 2*Sc*Dc

              http://www.pegtop.net/delphi/articles/blendmodes/softlight.htm.
            */
            if (fabs((double) Da) < MagickEpsilon)
              {
                pixel=QuantumRange*gamma*(Sca);
                break;
              }
            pixel=QuantumRange*gamma*(Dca*Dca*(Sa-2.0*Sca)/Da+Sca*(2.0*Dca+1.0-
              Da)+Dca*(1.0-Sa));
            break;
          }
          case PinLightCompositeOp:
          {
            /*
              PinLight: A Photoshop 7 composition method
              http://www.simplefilter.de/en/basics/mixmods.html

                f(Sc,Dc) = Dc<2*Sc-1 ? 2*Sc-1 : Dc>2*Sc   ? 2*Sc : Dc
            */
            if ((Dca*Sa) < (Da*(2.0*Sca-Sa)))
              {
                pixel=QuantumRange*gamma*(Sca*(Da+1.0)-Sa*Da+Dca*(1.0-Sa));
                break;
              }
            if ((Dca*Sa) > (2.0*Sca*Da))
              {
                pixel=QuantumRange*gamma*(Sca*Da+Sca+Dca*(1.0-Sa));
                break;
              }
            pixel=QuantumRange*gamma*(Sca*(1.0-Da)+Dca);
            break;
          }
          case PlusCompositeOp:
          {
            pixel=QuantumRange*(Sca+Dca);
            break;
          }
          case SaturateCompositeOp:
          {
            if (fabs((double) (QuantumRange*Sa-TransparentAlpha)) < MagickEpsilon)
              {
                pixel=Dc;
                break;
              }
            if (fabs((double) (QuantumRange*Da-TransparentAlpha)) < MagickEpsilon)
              {
                pixel=Sc;
                break;
              }
            CompositeHCL(canvas_pixel.red,canvas_pixel.green,canvas_pixel.blue,
              &hue,&chroma,&luma);
            CompositeHCL(source_pixel.red,source_pixel.green,source_pixel.blue,
              &sans,&chroma,&sans);
            HCLComposite(hue,chroma,luma,&red,&green,&blue);
            switch (channel)
            {
              case RedPixelChannel: pixel=red; break;
              case GreenPixelChannel: pixel=green; break;
              case BluePixelChannel: pixel=blue; break;
              default: pixel=Dc; break;
            }
            break;
          }
          case ScreenCompositeOp:
          {
            /*
              Screen:  a negated multiply:

                f(Sc,Dc) = 1.0-(1.0-Sc)*(1.0-Dc)
            */
            pixel=QuantumRange*gamma*(Sca+Dca-Sca*Dca);
            break;
          }
          case SoftLightCompositeOp:
          {
            if ((2.0*Sca) < Sa)
              {
                pixel=QuantumRange*gamma*(Dca*(Sa+(2.0*Sca-Sa)*(1.0-DcaDa))+
                  Sca*(1.0-Da)+Dca*(1.0-Sa));
                break;
              }
            if (((2.0*Sca) > Sa) && ((4.0*Dca) <= Da))
              {
                pixel=QuantumRange*gamma*(Dca*Sa+Da*(2.0*Sca-Sa)*(4.0*DcaDa*
                  (4.0*DcaDa+1.0)*(DcaDa-1.0)+7.0*DcaDa)+Sca*(1.0-Da)+
                  Dca*(1.0-Sa));
                break;
              }
            pixel=QuantumRange*gamma*(Dca*Sa+Da*(2.0*Sca-Sa)*(pow(DcaDa,0.5)-
              DcaDa)+Sca*(1.0-Da)+Dca*(1.0-Sa));
            break;
          }
          case StereoCompositeOp:
          {
            if (channel == RedPixelChannel)
              pixel=(MagickRealType) GetPixelRed(source_image,p);
            break;
          }
          case ThresholdCompositeOp:
          {
            MagickRealType
              delta;

            delta=Sc-Dc;
            if ((MagickRealType) fabs((double) (2.0*delta)) < threshold)
              {
                pixel=gamma*Dc;
                break;
              }
            pixel=gamma*(Dc+delta*amount);
            break;
          }
          case VividLightCompositeOp:
          {
            /*
              VividLight: A Photoshop 7 composition method.  See
              http://www.simplefilter.de/en/basics/mixmods.html.

                f(Sc,Dc) = (2*Sc < 1) ? 1-(1-Dc)/(2*Sc) : Dc/(2*(1-Sc))
            */
            if ((fabs((double) Sa) < MagickEpsilon) ||
                (fabs((double) (Sca-Sa)) < MagickEpsilon))
              {
                pixel=QuantumRange*gamma*(Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
                break;
              }
            if ((2.0*Sca) <= Sa)
              {
                pixel=QuantumRange*gamma*(Sa*(Da+Sa*(Dca-Da)*
                  PerceptibleReciprocal(2.0*Sca))+Sca*(1.0-Da)+Dca*(1.0-Sa));
                break;
              }
            pixel=QuantumRange*gamma*(Dca*Sa*Sa*PerceptibleReciprocal(2.0*
              (Sa-Sca))+Sca*(1.0-Da)+Dca*(1.0-Sa));
            break;
          }
          case XorCompositeOp:
          {
            pixel=QuantumRange*(Sca*(1.0-Da)+Dca*(1.0-Sa));
            break;
          }
          default:
          {
            pixel=Sc;
            break;
          }
        }
        q[i]=clamp != MagickFalse ? ClampPixel(pixel) : ClampToQuantum(pixel);
      }
      p+=GetPixelChannels(source_image);
      channels=GetPixelChannels(source_image);
      if (p >= (pixels+channels*source_image->columns))
        p=pixels;
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_CompositeImage)
#endif
        proceed=SetImageProgress(image,CompositeImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  source_view=DestroyCacheView(source_view);
  image_view=DestroyCacheView(image_view);
  if (canvas_image != (Image * ) NULL)
    canvas_image=DestroyImage(canvas_image);
  else
    source_image=DestroyImage(source_image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     T e x t u r e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TextureImage() repeatedly tiles the texture image across and down the image
%  canvas.
%
%  The format of the TextureImage method is:
%
%      MagickBooleanType TextureImage(Image *image,const Image *texture,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o texture_image: This image is the texture to layer on the background.
%
*/
MagickExport MagickBooleanType TextureImage(Image *image,const Image *texture,
  ExceptionInfo *exception)
{
#define TextureImageTag  "Texture/Image"

  CacheView
    *image_view,
    *texture_view;

  Image
    *texture_image;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickCoreSignature);
  if (texture == (const Image *) NULL)
    return(MagickFalse);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  texture_image=CloneImage(texture,0,0,MagickTrue,exception);
  if (texture_image == (const Image *) NULL)
    return(MagickFalse);
  (void) TransformImageColorspace(texture_image,image->colorspace,exception);
  (void) SetImageVirtualPixelMethod(texture_image,TileVirtualPixelMethod,
    exception);
  status=MagickTrue;
  if ((image->compose != CopyCompositeOp) &&
      ((image->compose != OverCompositeOp) ||
       (image->alpha_trait != UndefinedPixelTrait) ||
       (texture_image->alpha_trait != UndefinedPixelTrait)))
    {
      /*
        Tile texture onto the image background.
      */
      for (y=0; y < (ssize_t) image->rows; y+=(ssize_t) texture_image->rows)
      {
        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        for (x=0; x < (ssize_t) image->columns; x+=(ssize_t) texture_image->columns)
        {
          MagickBooleanType
            thread_status;

          thread_status=CompositeImage(image,texture_image,image->compose,
            MagickTrue,x+texture_image->tile_offset.x,y+
            texture_image->tile_offset.y,exception);
          if (thread_status == MagickFalse)
            {
              status=thread_status;
              break;
            }
        }
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

            proceed=SetImageProgress(image,TextureImageTag,(MagickOffsetType) y,
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      (void) SetImageProgress(image,TextureImageTag,(MagickOffsetType)
        image->rows,image->rows);
      texture_image=DestroyImage(texture_image);
      return(status);
    }
  /*
    Tile texture onto the image background (optimized).
  */
  status=MagickTrue;
  texture_view=AcquireVirtualCacheView(texture_image,exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_number_threads(texture_image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const Quantum
      *p,
      *pixels;

    register ssize_t
      x;

    register Quantum
      *q;

    size_t
      width;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewVirtualPixels(texture_view,texture_image->tile_offset.x,
      (y+texture_image->tile_offset.y) % texture_image->rows,
      texture_image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if ((pixels == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x+=(ssize_t) texture_image->columns)
    {
      register ssize_t
        j;

      p=pixels;
      width=texture_image->columns;
      if ((x+(ssize_t) width) > (ssize_t) image->columns)
        width=image->columns-x;
      for (j=0; j < (ssize_t) width; j++)
      {
        register ssize_t
          i;

        if (GetPixelWriteMask(image,q) <= (QuantumRange/2))
          {
            p+=GetPixelChannels(texture_image);
            q+=GetPixelChannels(image);
            continue;
          }
        for (i=0; i < (ssize_t) GetPixelChannels(texture_image); i++)
        {
          PixelChannel channel = GetPixelChannelChannel(texture_image,i);
          PixelTrait traits = GetPixelChannelTraits(image,channel);
          PixelTrait texture_traits=GetPixelChannelTraits(texture_image,
            channel);
          if ((traits == UndefinedPixelTrait) ||
              (texture_traits == UndefinedPixelTrait))
            continue;
          SetPixelChannel(image,channel,p[i],q);
        }
        p+=GetPixelChannels(texture_image);
        q+=GetPixelChannels(image);
      }
    }
    sync=SyncCacheViewAuthenticPixels(image_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,TextureImageTag,(MagickOffsetType) y,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  texture_view=DestroyCacheView(texture_view);
  image_view=DestroyCacheView(image_view);
  texture_image=DestroyImage(texture_image);
  return(status);
}

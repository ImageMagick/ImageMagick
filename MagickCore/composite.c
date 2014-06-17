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
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
%        const Image *composite_image,const CompositeOperator compose,
%        const MagickBooleanType clip_to_self,const ssize_t x_offset,
%        const ssize_t y_offset,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the destination image, modified by he composition
%
%    o composite_image: the composite (source) image.
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
%  Extra Controls from Image meta-data in 'composite_image' (artifacts)
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
                        Z = 1     for destination preserved

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
      gamma = 1 - QuantiumScale*alpha * QuantiumScale*beta;
      opacity = QuantiumScale*alpha*beta;  // over blend, optimized 1-Gamma

   The above SVG definitions also definate that Mathematical Composition
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

static inline MagickRealType MagickMin(const MagickRealType x,
  const MagickRealType y)
{
  if (x < y)
    return(x);
  return(y);
}

static inline MagickRealType MagickMax(const MagickRealType x,
  const MagickRealType y)
{
  if (x > y)
    return(x);
  return(y);
}

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
  const Image *composite_image,const MagickBooleanType clip_to_self,
  const ssize_t x_offset,const ssize_t y_offset,ExceptionInfo *exception)
{
#define CompositeImageTag  "Composite/Image"

  CacheView
    *composite_view,
    *image_view;

  MagickBooleanType
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
  composite_view=AcquireVirtualCacheView(composite_image,exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(composite_image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *pixels;

    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    size_t
      channels;

    if (status == MagickFalse)
      continue;
    if (clip_to_self != MagickFalse)
      {
        if (y < y_offset)
          continue;
        if ((y-y_offset) >= (ssize_t) composite_image->rows)
          continue;
      }
    /*
      If pixels is NULL, y is outside overlay region.
    */
    pixels=(Quantum *) NULL;
    p=(Quantum *) NULL;
    if ((y >= y_offset) && ((y-y_offset) < (ssize_t) composite_image->rows))
      {
        p=GetCacheViewVirtualPixels(composite_view,0,y-y_offset,
          composite_image->columns,1,exception);
        if (p == (const Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        pixels=p;
        if (x_offset < 0)
          p-=x_offset*GetPixelChannels(composite_image);
      }
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        gamma;

      MagickRealType
        alpha,
        Da,
        Dc,
        Sa,
        Sc;

      register ssize_t
        i;

      if (clip_to_self != MagickFalse)
        {
          if (x < x_offset)
            {
              q+=GetPixelChannels(image);
              continue;
            }
          if ((x-x_offset) >= (ssize_t) composite_image->columns)
            break;
        }
      if ((pixels == (Quantum *) NULL) || (x < x_offset) ||
          ((x-x_offset) >= (ssize_t) composite_image->columns))
        {
          Quantum
            source[MaxPixelChannels];

          /*
            Virtual composite:
              Sc: source color.
              Dc: destination color.
          */
          if (GetPixelReadMask(image,q) == 0)
            {
              q+=GetPixelChannels(image);
              continue;
            }
          (void) GetOneVirtualPixel(composite_image,x-x_offset,y-y_offset,
            source,exception);
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel=GetPixelChannelChannel(image,i);
            PixelTrait traits=GetPixelChannelTraits(image,channel);
            PixelTrait composite_traits=GetPixelChannelTraits(composite_image,
              channel);
            if ((traits == UndefinedPixelTrait) ||
                (composite_traits == UndefinedPixelTrait))
              continue;
            q[i]=source[channel];
          }
          q+=GetPixelChannels(image);
          continue;
        }
      /*
        Authentic composite:
          Sa:  normalized source alpha.
          Da:  normalized destination alpha.
      */
      if (GetPixelReadMask(composite_image,p) == 0)
        {
          p+=GetPixelChannels(composite_image);
          channels=GetPixelChannels(composite_image);
          if (p >= (pixels+channels*composite_image->columns))
            p=pixels;
          q+=GetPixelChannels(image);
          continue;
        }
      Sa=QuantumScale*GetPixelAlpha(composite_image,p);
      Da=QuantumScale*GetPixelAlpha(image,q);
      alpha=Sa*(-Da)+Sa+Da;
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        PixelTrait composite_traits=GetPixelChannelTraits(composite_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (composite_traits == UndefinedPixelTrait))
          continue;
        if ((traits & CopyPixelTrait) != 0)
          {
            if (channel != AlphaPixelChannel)
              {
                /*
                  Copy channel.
                */
                q[i]=GetPixelChannel(composite_image,channel,p);
                continue;
              }
            /*
              Set alpha channel.
            */
            q[i]=ClampToQuantum(QuantumRange*alpha);
            continue;
          }
        /*
          Sc: source color.
          Dc: destination color.
        */
        Sc=(MagickRealType) GetPixelChannel(composite_image,channel,p);
        Dc=(MagickRealType) q[i];
        gamma=PerceptibleReciprocal(alpha);
        q[i]=ClampToQuantum(gamma*(Sa*Sc-Sa*Da*Dc+Da*Dc));
      }
      p+=GetPixelChannels(composite_image);
      channels=GetPixelChannels(composite_image);
      if (p >= (pixels+channels*composite_image->columns))
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
  composite_view=DestroyCacheView(composite_view);
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
    *composite_view,
    *image_view;

  GeometryInfo
    geometry_info;

  Image
    *composite_image,
    *destination_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickRealType
    amount,
    destination_dissolve,
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(composite!= (Image *) NULL);
  assert(composite->signature == MagickSignature);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  composite_image=CloneImage(composite,0,0,MagickTrue,exception);
  if (composite_image == (const Image *) NULL)
    return(MagickFalse);
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    (void) SetImageColorspace(image,sRGBColorspace,exception);
  (void) SetImageColorspace(composite_image,image->colorspace,exception);
  if ((image->alpha_trait == BlendPixelTrait) &&
      (composite_image->alpha_trait != BlendPixelTrait))
    (void) SetImageAlphaChannel(composite_image,SetAlphaChannel,exception);
  if ((compose == OverCompositeOp) || (compose == SrcOverCompositeOp))
    {
      status=CompositeOverImage(image,composite_image,clip_to_self,x_offset,
        y_offset,exception);
      composite_image=DestroyImage(composite_image);
      return(status);
    }
  destination_image=(Image *) NULL;
  amount=0.5;
  destination_dissolve=1.0;
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
      if ((x_offset+(ssize_t) composite_image->columns) >= (ssize_t) image->columns)
        break;
      if ((y_offset+(ssize_t) composite_image->rows) >= (ssize_t) image->rows)
        break;
      status=MagickTrue;
      composite_view=AcquireVirtualCacheView(composite_image,exception);
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_threads(composite_image,image,composite_image->rows,1)
#endif
      for (y=0; y < (ssize_t) composite_image->rows; y++)
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
        p=GetCacheViewVirtualPixels(composite_view,0,y,composite_image->columns,
          1,exception);
        q=GetCacheViewAuthenticPixels(image_view,x_offset,y+y_offset,
          composite_image->columns,1,exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) composite_image->columns; x++)
        {
          register ssize_t
            i;

          if (GetPixelReadMask(composite_image,p) == 0)
            {
              p+=GetPixelChannels(composite_image);
              q+=GetPixelChannels(image);
              continue;
            }
          for (i=0; i < (ssize_t) GetPixelChannels(composite_image); i++)
          {
            PixelChannel channel=GetPixelChannelChannel(composite_image,i);
            PixelTrait composite_traits=GetPixelChannelTraits(composite_image,
              channel);
            PixelTrait traits=GetPixelChannelTraits(image,channel);
            if ((traits == UndefinedPixelTrait) ||
                (composite_traits == UndefinedPixelTrait))
              continue;
            SetPixelChannel(image,channel,p[i],q);
          }
          p+=GetPixelChannels(composite_image);
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
      composite_view=DestroyCacheView(composite_view);
      image_view=DestroyCacheView(image_view);
      composite_image=DestroyImage(composite_image);
      return(status);
    }
    case CopyAlphaCompositeOp:
    case ChangeMaskCompositeOp:
    case IntensityCompositeOp:
    {
      /*
        Modify destination outside the overlaid region and require an alpha
        channel to exist, to add transparency.
      */
      if (image->alpha_trait != BlendPixelTrait)
        (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
      break;
    }
    case BlurCompositeOp:
    {
      CacheView
        *composite_view,
        *destination_view;

      const char
        *value;

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
      destination_image=CloneImage(image,image->columns,image->rows,MagickTrue,
        exception);
      if (destination_image == (Image *) NULL)
        {
          composite_image=DestroyImage(composite_image);
          return(MagickFalse);
        }
      /*
        Gather the maximum blur sigma values from user.
      */
      SetGeometryInfo(&geometry_info);
      flags=NoValue;
      value=GetImageArtifact(image,"compose:args");
      if (value != (const char *) NULL)
        flags=ParseGeometry(value,&geometry_info);
      if ((flags & WidthValue) == 0)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
            "InvalidSetting","'%s' '%s'","compose:args",value);
          composite_image=DestroyImage(composite_image);
          destination_image=DestroyImage(destination_image);
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
      composite_view=AcquireVirtualCacheView(composite_image,exception);
      destination_view=AcquireAuthenticCacheView(destination_image,exception);
      for (y=0; y < (ssize_t) composite_image->rows; y++)
      {
        MagickBooleanType
          sync;

        register const Quantum
          *restrict p;

        register Quantum
          *restrict q;

        register ssize_t
          x;

        if (((y+y_offset) < 0) || ((y+y_offset) >= (ssize_t) image->rows))
          continue;
        p=GetCacheViewVirtualPixels(composite_view,0,y,composite_image->columns,
          1,exception);
        q=QueueCacheViewAuthenticPixels(destination_view,0,y,
          destination_image->columns,1,exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          break;
        for (x=0; x < (ssize_t) composite_image->columns; x++)
        {
          if (((x_offset+x) < 0) || ((x_offset+x) >= (ssize_t) image->columns))
            {
              p+=GetPixelChannels(composite_image);
              continue;
            }
          if (fabs(angle_range) > MagickEpsilon)
            {
              MagickRealType
                angle;

              angle=angle_start+angle_range*QuantumScale*
                GetPixelBlue(composite_image,p);
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
            blur.x1*QuantumScale*GetPixelRed(composite_image,p),
            blur.y1*QuantumScale*GetPixelGreen(composite_image,p),
            blur.x2*QuantumScale*GetPixelRed(composite_image,p),
            blur.y2*QuantumScale*GetPixelGreen(composite_image,p) );
          (void) ResamplePixelColor(resample_filter,(double) x_offset+x,
            (double) y_offset+y,&pixel,exception);
          SetPixelInfoPixel(destination_image,&pixel,q);
          p+=GetPixelChannels(composite_image);
          q+=GetPixelChannels(destination_image);
        }
        sync=SyncCacheViewAuthenticPixels(destination_view,exception);
        if (sync == MagickFalse)
          break;
      }
      resample_filter=DestroyResampleFilter(resample_filter);
      composite_view=DestroyCacheView(composite_view);
      destination_view=DestroyCacheView(destination_view);
      composite_image=DestroyImage(composite_image);
      composite_image=destination_image;
      break;
    }
    case DisplaceCompositeOp:
    case DistortCompositeOp:
    {
      CacheView
        *composite_view,
        *destination_view,
        *image_view;

      const char
        *value;

      PixelInfo
        pixel;

      MagickRealType
        horizontal_scale,
        vertical_scale;

      PointInfo
        center,
        offset;

      /*
        Displace/Distort based on overlay gradient map:
          X = red_channel;  Y = green_channel;
          compose:args = x_scale[,y_scale[,center.x,center.y]]
      */
      destination_image=CloneImage(image,image->columns,image->rows,MagickTrue,
        exception);
      if (destination_image == (Image *) NULL)
        {
          composite_image=DestroyImage(composite_image);
          return(MagickFalse);
        }
      SetGeometryInfo(&geometry_info);
      flags=NoValue;
      value=GetImageArtifact(composite_image,"compose:args");
      if (value != (char *) NULL)
        flags=ParseGeometry(value,&geometry_info);
      if ((flags & (WidthValue|HeightValue)) == 0 )
        {
          if ((flags & AspectValue) == 0)
            {
              horizontal_scale=(MagickRealType) (composite_image->columns-1.0)/
                2.0;
              vertical_scale=(MagickRealType) (composite_image->rows-1.0)/2.0;
            }
          else
            {
              horizontal_scale=(MagickRealType) (image->columns-1.0)/2.0;
              vertical_scale=(MagickRealType) (image->rows-1.0)/2.0;
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
                  horizontal_scale*=(composite_image->columns-1.0)/200.0;
                  vertical_scale*=(composite_image->rows-1.0)/200.0;
                }
              else
                {
                  horizontal_scale*=(image->columns-1.0)/200.0;
                  vertical_scale*=(image->rows-1.0)/200.0;
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
            if ((flags & AspectValue) == 0)
              center.x=(MagickRealType) (x_offset+(composite_image->columns-1)/
                2.0);
            else
              center.x=(MagickRealType) ((image->columns-1)/2);
          else
            if ((flags & AspectValue) == 0)
              center.x=(MagickRealType) x_offset+geometry_info.xi;
            else
              center.x=geometry_info.xi;
          if ((flags & YValue) == 0)
            if ((flags & AspectValue) == 0)
              center.y=(MagickRealType) (y_offset+(composite_image->rows-1)/
                2.0);
            else
              center.y=(MagickRealType) ((image->rows-1)/2);
          else
            if ((flags & AspectValue) == 0)
              center.y=(MagickRealType) y_offset+geometry_info.psi;
            else
              center.y=geometry_info.psi;
        }
      /*
        Shift the pixel offset point as defined by the provided,
        displacement/distortion map.  -- Like a lens...
      */
      GetPixelInfo(image,&pixel);
      image_view=AcquireVirtualCacheView(image,exception);
      composite_view=AcquireVirtualCacheView(composite_image,exception);
      destination_view=AcquireAuthenticCacheView(destination_image,exception);
      for (y=0; y < (ssize_t) composite_image->rows; y++)
      {
        MagickBooleanType
          sync;

        register const Quantum
          *restrict p;

        register Quantum
          *restrict q;

        register ssize_t
          x;

        if (((y+y_offset) < 0) || ((y+y_offset) >= (ssize_t) image->rows))
          continue;
        p=GetCacheViewVirtualPixels(composite_view,0,y,composite_image->columns,
          1,exception);
        q=QueueCacheViewAuthenticPixels(destination_view,0,y,
          destination_image->columns,1,exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          break;
        for (x=0; x < (ssize_t) composite_image->columns; x++)
        {
          if (((x_offset+x) < 0) || ((x_offset+x) >= (ssize_t) image->columns))
            {
              p+=GetPixelChannels(composite_image);
              continue;
            }
          /*
            Displace the offset.
          */
          offset.x=(double) (horizontal_scale*(GetPixelRed(composite_image,p)-
            (((MagickRealType) QuantumRange+1.0)/2.0)))/(((MagickRealType)
            QuantumRange+1.0)/2.0)+center.x+((compose == DisplaceCompositeOp) ?
            x : 0);
          offset.y=(double) (vertical_scale*(GetPixelGreen(composite_image,p)-
            (((MagickRealType) QuantumRange+1.0)/2.0)))/(((MagickRealType)
            QuantumRange+1.0)/2.0)+center.y+((compose == DisplaceCompositeOp) ?
            y : 0);
          (void) InterpolatePixelInfo(image,image_view,
            UndefinedInterpolatePixel,(double) offset.x,(double) offset.y,
            &pixel,exception);
          /*
            Mask with the 'invalid pixel mask' in alpha channel.
          */
          pixel.alpha=(MagickRealType) QuantumRange*(1.0-(1.0-QuantumScale*
            pixel.alpha)*(1.0-QuantumScale*GetPixelAlpha(composite_image,p)));
          SetPixelInfoPixel(destination_image,&pixel,q);
          p+=GetPixelChannels(composite_image);
          q+=GetPixelChannels(destination_image);
        }
        sync=SyncCacheViewAuthenticPixels(destination_view,exception);
        if (sync == MagickFalse)
          break;
      }
      destination_view=DestroyCacheView(destination_view);
      composite_view=DestroyCacheView(composite_view);
      image_view=DestroyCacheView(image_view);
      composite_image=DestroyImage(composite_image);
      composite_image=destination_image;
      break;
    }
    case DissolveCompositeOp:
    {
      const char
        *value;

      /*
        Geometry arguments to dissolve factors.
      */
      value=GetImageArtifact(composite_image,"compose:args");
      if (value != (char *) NULL)
        {
          flags=ParseGeometry(value,&geometry_info);
          source_dissolve=geometry_info.rho/100.0;
          destination_dissolve=1.0;
          if ((source_dissolve-MagickEpsilon) < 0.0)
            source_dissolve=0.0;
          if ((source_dissolve+MagickEpsilon) > 1.0)
            {
              destination_dissolve=2.0-source_dissolve;
              source_dissolve=1.0;
            }
          if ((flags & SigmaValue) != 0)
            destination_dissolve=geometry_info.sigma/100.0;
          if ((destination_dissolve-MagickEpsilon) < 0.0)
            destination_dissolve=0.0;
       /* posible speed up?  -- from IMv6 update
          clip_to_self=MagickFalse;
          if ((destination_dissolve+MagickEpsilon) > 1.0 )
            {
              destination_dissolve=1.0;
              clip_to_self=MagickTrue;
            }
        */
        }
      break;
    }
    case BlendCompositeOp:
    {
      const char
        *value;

      value=GetImageArtifact(composite_image,"compose:args");
      if (value != (char *) NULL)
        {
          flags=ParseGeometry(value,&geometry_info);
          source_dissolve=geometry_info.rho/100.0;
          destination_dissolve=1.0-source_dissolve;
          if ((flags & SigmaValue) != 0)
            destination_dissolve=geometry_info.sigma/100.0;
        }
      break;
    }
    case MathematicsCompositeOp:
    {
      const char
        *value;

      /*
        Just collect the values from "compose:args", setting.
        Unused values are set to zero automagically.

        Arguments are normally a comma separated list, so this probably should
        be changed to some 'general comma list' parser, (with a minimum
        number of values)
      */
      SetGeometryInfo(&geometry_info);
      value=GetImageArtifact(composite_image,"compose:args");
      if (value != (char *) NULL)
        (void) ParseGeometry(value,&geometry_info);
      break;
    }
    case ModulateCompositeOp:
    {
      const char
        *value;

      /*
        Determine the luma and chroma scale.
      */
      value=GetImageArtifact(composite_image,"compose:args");
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
      const char
        *value;

      /*
        Determine the amount and threshold.
      */
      value=GetImageArtifact(composite_image,"compose:args");
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
  composite_view=AcquireVirtualCacheView(composite_image,exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(composite_image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *pixels;

    MagickRealType
      blue,
      luma,
      green,
      hue,
      red,
      chroma;

    PixelInfo
      destination_pixel,
      source_pixel;

    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    if (clip_to_self != MagickFalse)
      {
        if (y < y_offset)
          continue;
        if ((y-y_offset) >= (ssize_t) composite_image->rows)
          continue;
      }
    /*
      If pixels is NULL, y is outside overlay region.
    */
    pixels=(Quantum *) NULL;
    p=(Quantum *) NULL;
    if ((y >= y_offset) && ((y-y_offset) < (ssize_t) composite_image->rows))
      {
        p=GetCacheViewVirtualPixels(composite_view,0,y-y_offset,
          composite_image->columns,1,exception);
        if (p == (const Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        pixels=p;
        if (x_offset < 0)
          p-=x_offset*GetPixelChannels(composite_image);
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
    GetPixelInfo(image,&destination_pixel);
    GetPixelInfo(composite_image,&source_pixel);
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
          if ((x-x_offset) >= (ssize_t) composite_image->columns)
            break;
        }
      if ((pixels == (Quantum *) NULL) || (x < x_offset) ||
          ((x-x_offset) >= (ssize_t) composite_image->columns))
        {
          Quantum
            source[MaxPixelChannels];

          /*
            Virtual composite:
              Sc: source color.
              Dc: destination color.
          */
          (void) GetOneVirtualPixel(composite_image,x-x_offset,y-y_offset,
            source,exception);
          if (GetPixelReadMask(image,q) == 0)
            {
              q+=GetPixelChannels(image);
              continue;
            }
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            MagickRealType
              pixel;

            PixelChannel channel=GetPixelChannelChannel(image,i);
            PixelTrait traits=GetPixelChannelTraits(image,channel);
            PixelTrait composite_traits=GetPixelChannelTraits(composite_image,
              channel);
            if ((traits == UndefinedPixelTrait) ||
                (composite_traits == UndefinedPixelTrait))
              continue;
            switch (compose)
            {
              case AlphaCompositeOp:
              case ChangeMaskCompositeOp:
              case CopyAlphaCompositeOp:
              case DstAtopCompositeOp:
              case DstInCompositeOp:
              case InCompositeOp:
              case IntensityCompositeOp:
              case OutCompositeOp:
              case SrcInCompositeOp:
              case SrcOutCompositeOp:
              {
                pixel=(MagickRealType) q[i];
                if (channel == AlphaPixelChannel)
                  pixel=(MagickRealType) TransparentAlpha;
                break;
              }
              case ClearCompositeOp:
              case CopyCompositeOp:
              case ReplaceCompositeOp:
              case SrcCompositeOp:
              {
                if (channel == AlphaPixelChannel)
                  {
                    pixel=(MagickRealType) TransparentAlpha;
                    break;
                  }
                pixel=0.0;
                break;
              }
              case BlendCompositeOp:
              case DissolveCompositeOp:
              {
                if (channel == AlphaPixelChannel)
                  {
                    pixel=destination_dissolve*GetPixelAlpha(composite_image,
                      source);
                    break;
                  }
                pixel=(MagickRealType) source[channel];
                break;
              }
              default:
              {
                pixel=(MagickRealType) source[channel];
                break;
              }
            }
            q[i]=ClampToQuantum(pixel);
          }
          q+=GetPixelChannels(image);
          continue;
        }
      /*
        Authentic composite:
          Sa:  normalized source alpha.
          Da:  normalized destination alpha.
      */
      Sa=QuantumScale*GetPixelAlpha(composite_image,p);
      Da=QuantumScale*GetPixelAlpha(image,q);
      switch (compose)
      {
        case BumpmapCompositeOp:
        {
          alpha=GetPixelIntensity(composite_image,p)*Sa;
          break;
        }
        case ColorBurnCompositeOp:
        case ColorDodgeCompositeOp:
        case DifferenceCompositeOp:
        case DivideDstCompositeOp:
        case DivideSrcCompositeOp:
        case ExclusionCompositeOp:
        case HardLightCompositeOp:
        case LinearBurnCompositeOp:
        case LinearDodgeCompositeOp:
        case LinearLightCompositeOp:
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
        case DarkenCompositeOp:
        case DstAtopCompositeOp:
        case DstInCompositeOp:
        case InCompositeOp:
        case LightenCompositeOp:
        case SrcInCompositeOp:
        {
          alpha=Sa*Da;
          break;
        }
        case DissolveCompositeOp:
        {
          alpha=source_dissolve*Sa*(-destination_dissolve*Da)+source_dissolve*
            Sa+destination_dissolve*Da;
          break;
        }
        case DstOverCompositeOp:
        {
          alpha=Da*(-Sa)+Da+Sa;
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
        case OverCompositeOp:
        case SrcOverCompositeOp:
        {
          alpha=Sa*(-Da)+Sa+Da;
          break;
        }
        case BlendCompositeOp:
        case PlusCompositeOp:
        {
          alpha=RoundToUnity(Sa+Da);
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
      if (GetPixelReadMask(image,q) == 0)
        {
          p+=GetPixelChannels(composite_image);
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
          GetPixelInfoPixel(composite_image,p,&source_pixel);
          GetPixelInfoPixel(image,q,&destination_pixel);
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

        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        PixelTrait composite_traits=GetPixelChannelTraits(composite_image,
          channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((compose != IntensityCompositeOp) &&
            (composite_traits == UndefinedPixelTrait))
          continue;
        /*
          Sc: source color.
          Dc: destination color.
        */
        Sc=(MagickRealType) GetPixelChannel(composite_image,channel,p);
        Dc=(MagickRealType) q[i];
        if ((traits & CopyPixelTrait) != 0)
          {
            if (channel != AlphaPixelChannel)
              {
                /*
                  Copy channel.
                */
                q[i]=ClampToQuantum(Sc);
                continue;
              }
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

                if (Da > ((MagickRealType) QuantumRange/2.0))
                  {
                    pixel=(MagickRealType) TransparentAlpha;
                    break;
                  }
                equivalent=IsFuzzyEquivalencePixel(composite_image,p,image,q);
                if (equivalent != MagickFalse)
                  {
                    pixel=(MagickRealType) TransparentAlpha;
                    break;
                  }
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
                if (fabs(QuantumRange*Sa-TransparentAlpha) < MagickEpsilon)
                  {
                    pixel=QuantumRange*Da;
                    break;
                  }
                if (fabs(QuantumRange*Da-TransparentAlpha) < MagickEpsilon)
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
                pixel=QuantumRange*Sa;
                if (composite_image->alpha_trait == BlendPixelTrait)
                  pixel=GetPixelIntensity(composite_image,p);
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
                pixel=(1.0-Sa)*GetPixelIntensity(composite_image,p) <
                  (1.0-Da)*GetPixelIntensity(image,q) ? Sa : Da;
                break;
              }
              case IntensityCompositeOp:
              {
                pixel=GetPixelIntensity(composite_image,p);
                break;
              }
              case LightenIntensityCompositeOp:
              {
                pixel=Sa*GetPixelIntensity(composite_image,p) >
                  Da*GetPixelIntensity(image,q) ? Sa : Da;
                break;
              }
              case ModulateCompositeOp:
              {
                if (fabs(QuantumRange*Sa-TransparentAlpha) < MagickEpsilon)
                  {
                    pixel=QuantumRange*Da;
                    break;
                  }
                pixel=QuantumRange*Da;
                break;
              }
              default:
              {
                pixel=QuantumRange*alpha;
                break;
              }
            }
            q[i]=ClampToQuantum(pixel);
            continue;
          }
        /*
          Porter-Duff compositions:
            Sca: source normalized color multiplied by alpha.
            Dca: normalized destination color multiplied by alpha.
        */
        Sca=QuantumScale*Sa*Sc;
        Dca=QuantumScale*Da*Dc;
        switch (compose)
        {
          case DarkenCompositeOp:
          case LightenCompositeOp:
          case ModulusSubtractCompositeOp:
          {
            gamma=1.0-alpha;
            break;
          }
          default:
            break;
        }
        gamma=PerceptibleReciprocal(alpha);
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
            pixel=Sc*Sa+Dc*(1.0-Sa);
            break;
          }
          case BlendCompositeOp:
          {
            pixel=gamma*(source_dissolve*Sa*Sc+destination_dissolve*Da*Dc);
            break;
          }
          case BlurCompositeOp:
          case DisplaceCompositeOp:
          case DistortCompositeOp:
          case CopyCompositeOp:
          case ReplaceCompositeOp:
          case SrcCompositeOp:
          {
            pixel=Sc;
            break;
          }
          case BumpmapCompositeOp:
          {
            if (fabs(QuantumRange*Sa-TransparentAlpha) < MagickEpsilon)
              {
                pixel=Dc;
                break;
              }
            pixel=QuantumScale*GetPixelIntensity(composite_image,p)*Dc;
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
            /*
              Refer to the March 2009 SVG specification.
            */
            if ((fabs(Sca) < MagickEpsilon) && (fabs(Dca-Da) < MagickEpsilon))
              {
                pixel=QuantumRange*gamma*(Sa*Da+Dca*(1.0-Sa));
                break;
              }
            if (Sca < MagickEpsilon)
              {
                pixel=QuantumRange*gamma*(Dca*(1.0-Sa));
                break;
              }
            pixel=QuantumRange*gamma*(Sa*Da-Sa*MagickMin(Da,(Da-Dca)*Sa/Sca)+
              Sca*(1.0-Da)+Dca*(1.0-Sa));
            break;
          }
          case ColorDodgeCompositeOp:
          {
            if ((Sca*Da+Dca*Sa) >= Sa*Da)
              {
                pixel=QuantumRange*gamma*(Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
                break;
              }
            pixel=QuantumRange*gamma*(Dca*Sa*Sa/(Sa-Sca)+Sca*(1.0-Da)+Dca*
              (1.0-Sa));
            break;
          }
          case ColorizeCompositeOp:
          {
            if (fabs(QuantumRange*Sa-TransparentAlpha) < MagickEpsilon)
              {
                pixel=Dc;
                break;
              }
            if (fabs(QuantumRange*Da-TransparentAlpha) < MagickEpsilon)
              {
                pixel=Sc;
                break;
              }
            CompositeHCL(destination_pixel.red,destination_pixel.green,
              destination_pixel.blue,&sans,&sans,&luma);
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
          case IntensityCompositeOp:
          {
            pixel=Dc;
            break;
          }
          case CopyBlackCompositeOp:
          {
            if (channel == BlackPixelChannel)
              pixel=(MagickRealType) (QuantumRange-
                GetPixelBlack(composite_image,p));
            break;
          }
          case CopyBlueCompositeOp:
          case CopyYellowCompositeOp:
          {
            if (channel == BluePixelChannel)
              pixel=(MagickRealType) GetPixelBlue(composite_image,p);
            break;
          }
          case CopyGreenCompositeOp:
          case CopyMagentaCompositeOp:
          {
            if (channel == GreenPixelChannel)
              pixel=(MagickRealType) GetPixelGreen(composite_image,p);
            break;
          }
          case CopyRedCompositeOp:
          case CopyCyanCompositeOp:
          {
            if (channel == RedPixelChannel)
              pixel=(MagickRealType) GetPixelRed(composite_image,p);
            break;
          }
          case DarkenCompositeOp:
          {
            /*
              Darken is equivalent to a 'Minimum' method
                OR a greyscale version of a binary 'Or'
                OR the 'Intersection' of pixel sets.
            */
            if (Sc < Dc)
              {
                pixel=gamma*(Sa*Sc-Sa*Da*Dc+Da*Dc);
                break;
              }
            pixel=gamma*(Da*Dc-Da*Sa*Sc+Sa*Sc);
            break;
          }
          case DarkenIntensityCompositeOp:
          {
            pixel=(1.0-Sa)*GetPixelIntensity(composite_image,p) <
              (1.0-Da)*GetPixelIntensity(image,q) ? Sc : Dc;
            break;
          }
          case DifferenceCompositeOp:
          {
            pixel=gamma*(Sa*Sc+Da*Dc-Sa*Da*2.0*MagickMin(Sc,Dc));
            break;
          }
          case DissolveCompositeOp:
          {
            pixel=gamma*(source_dissolve*Sa*Sc-source_dissolve*Sa*
              destination_dissolve*Da*Dc+destination_dissolve*Da*Dc);
            break;
          }
          case DivideDstCompositeOp:
          {
            if ((fabs(Sca) < MagickEpsilon) && (fabs(Dca) < MagickEpsilon))
              {
                pixel=QuantumRange*gamma*(Sca*(1.0-Da)+Dca*(1.0-Sa));
                break;
              }
            if (fabs(Dca) < MagickEpsilon)
              {
                pixel=QuantumRange*gamma*(Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
                break;
              }
            pixel=QuantumRange*gamma*(Sca*Da*Da/Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
            break;
          }
          case DivideSrcCompositeOp:
          {
            if ((fabs(Dca) < MagickEpsilon) && (fabs(Sca) < MagickEpsilon))
              {
                pixel=QuantumRange*gamma*(Dca*(1.0-Sa)+Sca*(1.0-Da));
                break;
              }
            if (fabs(Sca) < MagickEpsilon)
              {
                pixel=QuantumRange*gamma*(Da*Sa+Dca*(1.0-Sa)+Sca*(1.0-Da));
                break;
              }
            pixel=QuantumRange*gamma*(Dca*Sa*Sa/Sca+Dca*(1.0-Sa)+Sca*(1.0-Da));
            break;
          }
          case DstAtopCompositeOp:
          {
            pixel=Dc*Da+Sc*(1.0-Da);
            break;
          }
          case DstCompositeOp:
          case NoCompositeOp:
          {
            pixel=Dc;
            break;
          }
          case DstInCompositeOp:
          {
            pixel=gamma*(Sa*Dc*Sa);
            break;
          }
          case DstOutCompositeOp:
          {
            pixel=gamma*(Da*Dc*(1.0-Sa));
            break;
          }
          case DstOverCompositeOp:
          {
            pixel=gamma*(Da*Dc-Da*Sa*Sc+Sa*Sc);
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
                pixel=QuantumRange*gamma*(2.0*Sca*Dca+Sca*(1.0-Da)+Dca*
                  (1.0-Sa));
                break;
              }
            pixel=QuantumRange*gamma*(Sa*Da-2.0*(Da-Dca)*(Sa-Sca)+Sca*(1.0-Da)+
              Dca*(1.0-Sa));
            break;
          }
          case HueCompositeOp:
          {
            if (fabs(QuantumRange*Sa-TransparentAlpha) < MagickEpsilon)
              {
                pixel=Dc;
                break;
              }
            if (fabs(QuantumRange*Da-TransparentAlpha) < MagickEpsilon)
              {
                pixel=Sc;
                break;
              }
            CompositeHCL(destination_pixel.red,destination_pixel.green,
              destination_pixel.blue,&hue,&chroma,&luma);
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
            pixel=gamma*(Da*Sc*Da);
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
            pixel=QuantumRange*gamma*(Sa*Sc+Da*Dc);
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
            if (Sc > Dc)
              {
                pixel=gamma*(Sa*Sc-Sa*Da*Dc+Da*Dc);
                break;
              }
            pixel=gamma*(Da*Dc-Da*Sa*Sc+Sa*Sc);
            break;
          }
          case LightenIntensityCompositeOp:
          {
            /*
              Lighten is equivalent to a 'Maximum' method
                OR a greyscale version of a binary 'And'
                OR the 'Union' of pixel sets.
            */
            pixel=Sa*GetPixelIntensity(composite_image,p) >
              Da*GetPixelIntensity(image,q) ? Sc : Dc;
            break;
          }
          case LuminizeCompositeOp:
          {
            if (fabs(QuantumRange*Sa-TransparentAlpha) < MagickEpsilon)
              {
                pixel=Dc;
                break;
              }
            if (fabs(QuantumRange*Da-TransparentAlpha) < MagickEpsilon)
              {
                pixel=Sc;
                break;
              }
            CompositeHCL(destination_pixel.red,destination_pixel.green,
              destination_pixel.blue,&hue,&chroma,&luma);
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
            pixel=gamma*geometry_info.rho*Sa*Sc*Da*Dc+geometry_info.sigma*
              Sa*Sc*Da+geometry_info.xi*Da*Dc*Sa+geometry_info.psi*Sa*Da+
              Sa*Sc*(1.0-Da)+Da*Dc*(1.0-Sa);
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
              Minus source from destination.

                f(Sc,Dc) = Sc - Dc
            */
            pixel=QuantumRange*gamma*(Da*Dc+Sa*Sc-2.0*Sa*Sc*Da);
            break;
          }
          case ModulateCompositeOp:
          {
            ssize_t
              offset;

            if (fabs(QuantumRange*Sa-TransparentAlpha) < MagickEpsilon)
              {
                pixel=Dc;
                break;
              }
            offset=(ssize_t) (GetPixelIntensity(composite_image,p)-midpoint);
            if (offset == 0)
              {
                pixel=Dc;
                break;
              }
            CompositeHCL(destination_pixel.red,destination_pixel.green,
              destination_pixel.blue,&hue,&chroma,&luma);
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
            if (pixel > QuantumRange)
              pixel-=QuantumRange;
            pixel=gamma*(Sa*Da*pixel+Sa*Sc*(1.0-Da)+Da*Dc*(1.0-Sa));
            break;
          }
          case ModulusSubtractCompositeOp:
          {
            pixel=Sc-Dc;
            if (pixel < 0.0)
              pixel+=QuantumRange;
            pixel=gamma*(Sa*Da*pixel+Sa*Sc*(1.0-Da)+Da*Dc*(1.0-Sa));
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
            pixel=gamma*(Sa*Sc*(1.0-Da));
            break;
          }
          case OverCompositeOp:
          case SrcOverCompositeOp:
          {
            pixel=QuantumRange*gamma*(Sa*Sc-Sa*Da*Dc+Da*Dc);
            break;
          }
          case OverlayCompositeOp:
          {
            if ((2.0*Dca) < Da)
              {
                pixel=QuantumRange*gamma*(2.0*Dca*Sca+Dca*(1.0-Sa)+Sca*
                  (1.0-Da));
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
            if (fabs(Da) < MagickEpsilon)
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
            pixel=gamma*(Sa*Sc+Da*Dc);
            break;
          }
          case SaturateCompositeOp:
          {
            if (fabs(QuantumRange*Sa-TransparentAlpha) < MagickEpsilon)
              {
                pixel=Dc;
                break;
              }
            if (fabs(QuantumRange*Da-TransparentAlpha) < MagickEpsilon)
              {
                pixel=Sc;
                break;
              }
            CompositeHCL(destination_pixel.red,destination_pixel.green,
              destination_pixel.blue,&hue,&chroma,&luma);
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
            /*
              Refer to the March 2009 SVG specification.
            */
            if ((2.0*Sca) < Sa)
              {
                pixel=QuantumRange*gamma*(Dca*(Sa+(2.0*Sca-Sa)*(1.0-(Dca/Da)))+
                  Sca*(1.0-Da)+Dca*(1.0-Sa));
                break;
              }
            if (((2.0*Sca) > Sa) && ((4.0*Dca) <= Da))
              {
                pixel=QuantumRange*gamma*(Dca*Sa+Da*(2.0*Sca-Sa)*(4.0*(Dca/Da)*
                  (4.0*(Dca/Da)+1.0)*((Dca/Da)-1.0)+7.0*(Dca/Da))+Sca*(1.0-Da)+
                  Dca*(1.0-Sa));
                break;
              }
            pixel=QuantumRange*gamma*(Dca*Sa+Da*(2.0*Sca-Sa)*(pow((Dca/Da),0.5)-
              (Dca/Da))+Sca*(1.0-Da)+Dca*(1.0-Sa));
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
            if ((fabs(Sa) < MagickEpsilon) || (fabs(Sca-Sa) < MagickEpsilon))
              {
                pixel=QuantumRange*gamma*(Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
                break;
              }
            if ((2.0*Sca) <= Sa)
              {
                pixel=QuantumRange*gamma*(Sa*(Da+Sa*(Dca-Da)/(2.0*Sca))+Sca*
                  (1.0-Da)+Dca*(1.0-Sa));
                break;
              }
            pixel=QuantumRange*gamma*(Dca*Sa*Sa/(2.0*(Sa-Sca))+Sca*(1.0-Da)+
              Dca*(1.0-Sa));
            break;
          }
          case XorCompositeOp:
          {
            pixel=QuantumRange*gamma*(Sc*Sa*(1.0-Da)+Dc*Da*(1.0-Sa));
            break;
          }
          default:
          {
            pixel=Sc;
            break;
          }
        }
        q[i]=ClampToQuantum(pixel);
      }
      p+=GetPixelChannels(composite_image);
      channels=GetPixelChannels(composite_image);
      if (p >= (pixels+channels*composite_image->columns))
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
  composite_view=DestroyCacheView(composite_view);
  image_view=DestroyCacheView(image_view);
  if (destination_image != (Image * ) NULL)
    destination_image=DestroyImage(destination_image);
  else
    composite_image=DestroyImage(composite_image);
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
  assert(image->signature == MagickSignature);
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
       (image->alpha_trait == BlendPixelTrait) ||
       (texture_image->alpha_trait == BlendPixelTrait)))
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
            MagickFalse,x+texture_image->tile_offset.x,y+
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

            proceed=SetImageProgress(image,TextureImageTag,(MagickOffsetType)
              y,image->rows);
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
    magick_threads(texture_image,image,1,1)
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

        if (GetPixelReadMask(image,q) == 0)
          {
            p+=GetPixelChannels(texture_image);
            q+=GetPixelChannels(image);
            continue;
          }
        for (i=0; i < (ssize_t) GetPixelChannels(texture_image); i++)
        {
          PixelChannel channel=GetPixelChannelChannel(texture_image,i);
          PixelTrait traits=GetPixelChannelTraits(image,channel);
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%            DDDD   EEEEE   CCCC   OOO   RRRR    AAA   TTTTT  EEEEE           %
%            D   D  E      C      O   O  R   R  A   A    T    E               %
%            D   D  EEE    C      O   O  RRRR   AAAAA    T    EEE             %
%            D   D  E      C      O   O  R R    A   A    T    E               %
%            DDDD   EEEEE   CCCC   OOO   R  R   A   A    T    EEEEE           %
%                                                                             %
%                                                                             %
%                     MagickCore Image Decoration Methods                     %
%                                                                             %
%                                Software Design                              %
%                                  John Cristy                                %
%                                   July 1992                                 %
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
#include "MagickCore/cache-view.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/decorate.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/transform.h"

/*
  Define declarations.
*/
#define AccentuateModulate  ScaleCharToQuantum(80)
#define HighlightModulate  ScaleCharToQuantum(125)
#define ShadowModulate  ScaleCharToQuantum(135)
#define DepthModulate  ScaleCharToQuantum(185)
#define TroughModulate  ScaleCharToQuantum(110)

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   B o r d e r I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BorderImage() surrounds the image with a border of the color defined by
%  the bordercolor member of the image structure.  The width and height
%  of the border are defined by the corresponding members of the border_info
%  structure.
%
%  The format of the BorderImage method is:
%
%      Image *BorderImage(const Image *image,const RectangleInfo *border_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o border_info:  Define the width and height of the border.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *BorderImage(const Image *image,
  const RectangleInfo *border_info,ExceptionInfo *exception)
{
  Image
    *border_image,
    *clone_image;

  FrameInfo
    frame_info;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(border_info != (RectangleInfo *) NULL);
  frame_info.width=image->columns+(border_info->width << 1);
  frame_info.height=image->rows+(border_info->height << 1);
  frame_info.x=(ssize_t) border_info->width;
  frame_info.y=(ssize_t) border_info->height;
  frame_info.inner_bevel=0;
  frame_info.outer_bevel=0;
  clone_image=CloneImage(image,0,0,MagickTrue,exception);
  if (clone_image == (Image *) NULL)
    return((Image *) NULL);
  clone_image->matte_color=image->border_color;
  border_image=FrameImage(clone_image,&frame_info,exception);
  clone_image=DestroyImage(clone_image);
  if (border_image != (Image *) NULL)
    border_image->matte_color=image->matte_color;
  return(border_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F r a m e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FrameImage() adds a simulated three-dimensional border around the image.
%  The color of the border is defined by the matte_color member of image.
%  Members width and height of frame_info specify the border width of the
%  vertical and horizontal sides of the frame.  Members inner and outer
%  indicate the width of the inner and outer shadows of the frame.
%
%  The format of the FrameImage method is:
%
%      Image *FrameImage(const Image *image,const FrameInfo *frame_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o frame_info: Define the width and height of the frame and its bevels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *FrameImage(const Image *image,const FrameInfo *frame_info,
  ExceptionInfo *exception)
{
#define FrameImageTag  "Frame/Image"

  CacheView
    *image_view,
    *frame_view;

  Image
    *frame_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    accentuate,
    border,
    highlight,
    interior,
    matte,
    shadow,
    trough;

  register ssize_t
    x;

  size_t
    bevel_width,
    height,
    width;

  ssize_t
    y;

  /*
    Check frame geometry.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(frame_info != (FrameInfo *) NULL);
  if ((frame_info->outer_bevel < 0) || (frame_info->inner_bevel < 0))
    ThrowImageException(OptionError,"FrameIsLessThanImageSize");
  bevel_width=(size_t) (frame_info->outer_bevel+frame_info->inner_bevel);
  width=frame_info->width-frame_info->x-bevel_width;
  height=frame_info->height-frame_info->y-bevel_width;
  if ((width < image->columns) || (height < image->rows))
    ThrowImageException(OptionError,"FrameIsLessThanImageSize");
  /*
    Initialize framed image attributes.
  */
  frame_image=CloneImage(image,frame_info->width,frame_info->height,MagickTrue,
    exception);
  if (frame_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(frame_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&frame_image->exception);
      frame_image=DestroyImage(frame_image);
      return((Image *) NULL);
    }
  if (frame_image->matte_color.alpha != OpaqueAlpha)
    frame_image->matte=MagickTrue;
  frame_image->page=image->page;
  if ((image->page.width != 0) && (image->page.height != 0))
    {
      frame_image->page.width+=frame_image->columns-image->columns;
      frame_image->page.height+=frame_image->rows-image->rows;
    }
  /*
    Initialize 3D effects color.
  */
  GetPixelInfo(frame_image,&interior);
  SetPixelInfoPacket(frame_image,&image->border_color,&interior);
  GetPixelInfo(frame_image,&matte);
  matte.colorspace=RGBColorspace;
  SetPixelInfoPacket(frame_image,&image->matte_color,&matte);
  GetPixelInfo(frame_image,&border);
  border.colorspace=RGBColorspace;
  SetPixelInfoPacket(frame_image,&image->border_color,&border);
  GetPixelInfo(frame_image,&accentuate);
  accentuate.red=(MagickRealType) (QuantumScale*((QuantumRange-
    AccentuateModulate)*matte.red+(QuantumRange*AccentuateModulate)));
  accentuate.green=(MagickRealType) (QuantumScale*((QuantumRange-
    AccentuateModulate)*matte.green+(QuantumRange*AccentuateModulate)));
  accentuate.blue=(MagickRealType) (QuantumScale*((QuantumRange-
    AccentuateModulate)*matte.blue+(QuantumRange*AccentuateModulate)));
  accentuate.alpha=matte.alpha;
  GetPixelInfo(frame_image,&highlight);
  highlight.red=(MagickRealType) (QuantumScale*((QuantumRange-
    HighlightModulate)*matte.red+(QuantumRange*HighlightModulate)));
  highlight.green=(MagickRealType) (QuantumScale*((QuantumRange-
    HighlightModulate)*matte.green+(QuantumRange*HighlightModulate)));
  highlight.blue=(MagickRealType) (QuantumScale*((QuantumRange-
    HighlightModulate)*matte.blue+(QuantumRange*HighlightModulate)));
  highlight.alpha=matte.alpha;
  GetPixelInfo(frame_image,&shadow);
  shadow.red=QuantumScale*matte.red*ShadowModulate;
  shadow.green=QuantumScale*matte.green*ShadowModulate;
  shadow.blue=QuantumScale*matte.blue*ShadowModulate;
  shadow.alpha=matte.alpha;
  GetPixelInfo(frame_image,&trough);
  trough.red=QuantumScale*matte.red*TroughModulate;
  trough.green=QuantumScale*matte.green*TroughModulate;
  trough.blue=QuantumScale*matte.blue*TroughModulate;
  trough.alpha=matte.alpha;
  if (image->colorspace == CMYKColorspace)
    {
      ConvertRGBToCMYK(&interior);
      ConvertRGBToCMYK(&matte);
      ConvertRGBToCMYK(&border);
      ConvertRGBToCMYK(&accentuate);
      ConvertRGBToCMYK(&highlight);
      ConvertRGBToCMYK(&shadow);
      ConvertRGBToCMYK(&trough);
    }
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  frame_view=AcquireCacheView(frame_image);
  height=(size_t) (frame_info->outer_bevel+(frame_info->y-bevel_width)+
    frame_info->inner_bevel);
  if (height != 0)
    {
      register ssize_t
        x;

      register Quantum
        *restrict q;

      /*
        Draw top of ornamental border.
      */
      q=QueueCacheViewAuthenticPixels(frame_view,0,0,frame_image->columns,
        height,exception);
      if (q != (Quantum *) NULL)
        {
          /*
            Draw top of ornamental border.
          */
          for (y=0; y < (ssize_t) frame_info->outer_bevel; y++)
          {
            for (x=0; x < (ssize_t) (frame_image->columns-y); x++)
            {
              if (x < y)
                SetPixelPixelInfo(frame_image,&highlight,q);
              else
                SetPixelPixelInfo(frame_image,&accentuate,q);
              q+=GetPixelComponents(frame_image);
            }
            for ( ; x < (ssize_t) frame_image->columns; x++)
            {
              SetPixelPixelInfo(frame_image,&shadow,q);
              q+=GetPixelComponents(frame_image);
            }
          }
          for (y=0; y < (ssize_t) (frame_info->y-bevel_width); y++)
          {
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelPixelInfo(frame_image,&highlight,q);
              q+=GetPixelComponents(frame_image);
            }
            width=frame_image->columns-2*frame_info->outer_bevel;
            for (x=0; x < (ssize_t) width; x++)
            {
              SetPixelPixelInfo(frame_image,&matte,q);
              q+=GetPixelComponents(frame_image);
            }
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelPixelInfo(frame_image,&shadow,q);
              q+=GetPixelComponents(frame_image);
            }
          }
          for (y=0; y < (ssize_t) frame_info->inner_bevel; y++)
          {
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelPixelInfo(frame_image,&highlight,q);
              q+=GetPixelComponents(frame_image);
            }
            for (x=0; x < (ssize_t) (frame_info->x-bevel_width); x++)
            {
              SetPixelPixelInfo(frame_image,&matte,q);
              q+=GetPixelComponents(frame_image);
            }
            width=image->columns+((size_t) frame_info->inner_bevel << 1)-
              y;
            for (x=0; x < (ssize_t) width; x++)
            {
              if (x < y)
                SetPixelPixelInfo(frame_image,&shadow,q);
              else
                SetPixelPixelInfo(frame_image,&trough,q);
              q+=GetPixelComponents(frame_image);
            }
            for ( ; x < (ssize_t) (image->columns+2*frame_info->inner_bevel); x++)
            {
              SetPixelPixelInfo(frame_image,&highlight,q);
              q+=GetPixelComponents(frame_image);
            }
            width=frame_info->width-frame_info->x-image->columns-bevel_width;
            for (x=0; x < (ssize_t) width; x++)
            {
              SetPixelPixelInfo(frame_image,&matte,q);
              q+=GetPixelComponents(frame_image);
            }
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelPixelInfo(frame_image,&shadow,q);
              q+=GetPixelComponents(frame_image);
            }
          }
          (void) SyncCacheViewAuthenticPixels(frame_view,exception);
        }
    }
  /*
    Draw sides of ornamental border.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT) 
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status) omp_throttle(1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register Quantum
      *restrict q;

    /*
      Initialize scanline with matte color.
    */
    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(frame_view,0,frame_info->y+y,
      frame_image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
    {
      SetPixelPixelInfo(frame_image,&highlight,q);
      q+=GetPixelComponents(frame_image);
    }
    for (x=0; x < (ssize_t) (frame_info->x-bevel_width); x++)
    {
      SetPixelPixelInfo(frame_image,&matte,q);
      q+=GetPixelComponents(frame_image);
    }
    for (x=0; x < (ssize_t) frame_info->inner_bevel; x++)
    {
      SetPixelPixelInfo(frame_image,&shadow,q);
      q+=GetPixelComponents(frame_image);
    }
    /*
      Set frame interior to interior color.
    */
    if ((image->compose != CopyCompositeOp) &&
        ((image->compose != OverCompositeOp) || (image->matte != MagickFalse)))
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        SetPixelPixelInfo(frame_image,&interior,q);
        q+=GetPixelComponents(frame_image);
      }
    else
      {
        register const Quantum
          *p;

        p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
        if (p == (const Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelRed(frame_image,GetPixelRed(image,p),q);
          SetPixelGreen(frame_image,GetPixelGreen(image,p),q);
          SetPixelBlue(frame_image,GetPixelBlue(image,p),q);
          if (image->colorspace == CMYKColorspace)
            SetPixelBlack(frame_image,GetPixelBlack(image,p),q);
          SetPixelAlpha(frame_image,GetPixelAlpha(image,p),q);
          p+=GetPixelComponents(image);
          q+=GetPixelComponents(frame_image);
        }
      }
    for (x=0; x < (ssize_t) frame_info->inner_bevel; x++)
    {
      SetPixelPixelInfo(frame_image,&highlight,q);
      q+=GetPixelComponents(frame_image);
    }
    width=frame_info->width-frame_info->x-image->columns-bevel_width;
    for (x=0; x < (ssize_t) width; x++)
    {
      SetPixelPixelInfo(frame_image,&matte,q);
      q+=GetPixelComponents(frame_image);
    }
    for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
    {
      SetPixelPixelInfo(frame_image,&shadow,q);
      q+=GetPixelComponents(frame_image);
    }
    if (SyncCacheViewAuthenticPixels(frame_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT) 
  #pragma omp critical (MagickCore_FrameImage)
#endif
        proceed=SetImageProgress(image,FrameImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  height=(size_t) (frame_info->inner_bevel+frame_info->height-
    frame_info->y-image->rows-bevel_width+frame_info->outer_bevel);
  if (height != 0)
    {
      register ssize_t
        x;

      register Quantum
        *restrict q;

      /*
        Draw bottom of ornamental border.
      */
      q=QueueCacheViewAuthenticPixels(frame_view,0,(ssize_t) (frame_image->rows-
        height),frame_image->columns,height,exception);
      if (q != (Quantum *) NULL)
        {
          /*
            Draw bottom of ornamental border.
          */
          for (y=frame_info->inner_bevel-1; y >= 0; y--)
          {
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelPixelInfo(frame_image,&highlight,q);
              q+=GetPixelComponents(frame_image);
            }
            for (x=0; x < (ssize_t) (frame_info->x-bevel_width); x++)
            {
              SetPixelPixelInfo(frame_image,&matte,q);
              q+=GetPixelComponents(frame_image);
            }
            for (x=0; x < y; x++)
            {
              SetPixelPixelInfo(frame_image,&shadow,q);
              q+=GetPixelComponents(frame_image);
            }
            for ( ; x < (ssize_t) (image->columns+2*frame_info->inner_bevel); x++)
            {
              if (x >= (ssize_t) (image->columns+2*frame_info->inner_bevel-y))
                SetPixelPixelInfo(frame_image,&highlight,q);
              else
                SetPixelPixelInfo(frame_image,&accentuate,q);
              q+=GetPixelComponents(frame_image);
            }
            width=frame_info->width-frame_info->x-image->columns-bevel_width;
            for (x=0; x < (ssize_t) width; x++)
            {
              SetPixelPixelInfo(frame_image,&matte,q);
              q+=GetPixelComponents(frame_image);
            }
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelPixelInfo(frame_image,&shadow,q);
              q+=GetPixelComponents(frame_image);
            }
          }
          height=frame_info->height-frame_info->y-image->rows-bevel_width;
          for (y=0; y < (ssize_t) height; y++)
          {
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelPixelInfo(frame_image,&highlight,q);
              q+=GetPixelComponents(frame_image);
            }
            width=frame_image->columns-2*frame_info->outer_bevel;
            for (x=0; x < (ssize_t) width; x++)
            {
              SetPixelPixelInfo(frame_image,&matte,q);
              q+=GetPixelComponents(frame_image);
            }
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelPixelInfo(frame_image,&shadow,q);
              q+=GetPixelComponents(frame_image);
            }
          }
          for (y=frame_info->outer_bevel-1; y >= 0; y--)
          {
            for (x=0; x < y; x++)
            {
              SetPixelPixelInfo(frame_image,&highlight,q);
              q+=GetPixelComponents(frame_image);
            }
            for ( ; x < (ssize_t) frame_image->columns; x++)
            {
              if (x >= (ssize_t) (frame_image->columns-y))
                SetPixelPixelInfo(frame_image,&shadow,q);
              else
                SetPixelPixelInfo(frame_image,&trough,q);
              q+=GetPixelComponents(frame_image);
            }
          }
          (void) SyncCacheViewAuthenticPixels(frame_view,exception);
        }
    }
  frame_view=DestroyCacheView(frame_view);
  image_view=DestroyCacheView(image_view);
  if ((image->compose != CopyCompositeOp) &&
      ((image->compose != OverCompositeOp) || (image->matte != MagickFalse)))
    {
      x=(ssize_t) (frame_info->outer_bevel+(frame_info->x-bevel_width)+
        frame_info->inner_bevel);
      y=(ssize_t) (frame_info->outer_bevel+(frame_info->y-bevel_width)+
        frame_info->inner_bevel);
      (void) CompositeImage(frame_image,image->compose,image,x,y);
    }
  return(frame_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R a i s e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RaiseImage() creates a simulated three-dimensional button-like effect
%  by lightening and darkening the edges of the image.  Members width and
%  height of raise_info define the width of the vertical and horizontal
%  edge of the effect.
%
%  The format of the RaiseImage method is:
%
%      MagickBooleanType RaiseImage(const Image *image,
%        const RectangleInfo *raise_info,const MagickBooleanType raise)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o raise_info: Define the width and height of the raise area.
%
%    o raise: A value other than zero creates a 3-D raise effect,
%      otherwise it has a lowered effect.
%
*/
MagickExport MagickBooleanType RaiseImage(Image *image,
  const RectangleInfo *raise_info,const MagickBooleanType raise)
{
#define AccentuateFactor  ScaleCharToQuantum(135)
#define HighlightFactor  ScaleCharToQuantum(190)
#define ShadowFactor  ScaleCharToQuantum(190)
#define RaiseImageTag  "Raise/Image"
#define TroughFactor  ScaleCharToQuantum(135)

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  Quantum
    foreground,
    background;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(raise_info != (RectangleInfo *) NULL);
  if ((image->columns <= (raise_info->width << 1)) ||
      (image->rows <= (raise_info->height << 1)))
    ThrowBinaryException(OptionError,"ImageSizeMustExceedBevelWidth",
      image->filename);
  foreground=(Quantum) QuantumRange;
  background=(Quantum) 0;
  if (raise == MagickFalse)
    {
      foreground=(Quantum) 0;
      background=(Quantum) QuantumRange;
    }
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  /*
    Raise image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT) 
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status) omp_throttle(1)
#endif
  for (y=0; y < (ssize_t) raise_info->height; y++)
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
    for (x=0; x < y; x++)
    {
      SetPixelRed(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelRed(image,q)*HighlightFactor+(MagickRealType) foreground*
        (QuantumRange-HighlightFactor))),q);
      SetPixelGreen(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelGreen(image,q)*HighlightFactor+(MagickRealType) foreground*
        (QuantumRange-HighlightFactor))),q);
      SetPixelBlue(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelBlue(image,q)*HighlightFactor+(MagickRealType) foreground*
        (QuantumRange-HighlightFactor))),q);
      q+=GetPixelComponents(image);
    }
    for ( ; x < (ssize_t) (image->columns-y); x++)
    {
      SetPixelRed(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelRed(image,q)*AccentuateFactor+(MagickRealType) foreground*
        (QuantumRange-AccentuateFactor))),q);
      SetPixelGreen(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelGreen(image,q)*AccentuateFactor+(MagickRealType) foreground*
        (QuantumRange-AccentuateFactor))),q);
      SetPixelBlue(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelBlue(image,q)*AccentuateFactor+(MagickRealType) foreground*
        (QuantumRange-AccentuateFactor))),q);
      q+=GetPixelComponents(image);
    }
    for ( ; x < (ssize_t) image->columns; x++)
    {
      SetPixelRed(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelRed(image,q)*ShadowFactor+(MagickRealType) background*
        (QuantumRange-ShadowFactor))),q);
      SetPixelGreen(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelGreen(image,q)*ShadowFactor+(MagickRealType) background*
        (QuantumRange-ShadowFactor))),q);
      SetPixelBlue(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelBlue(image,q)*ShadowFactor+(MagickRealType) background*
        (QuantumRange-ShadowFactor))),q);
      q+=GetPixelComponents(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,RaiseImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status) omp_throttle(1)
#endif
  for (y=(ssize_t) raise_info->height; y < (ssize_t) (image->rows-raise_info->height); y++)
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
    for (x=0; x < (ssize_t) raise_info->width; x++)
    {
      SetPixelRed(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelRed(image,q)*HighlightFactor+(MagickRealType) foreground*
        (QuantumRange-HighlightFactor))),q);
      SetPixelGreen(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelGreen(image,q)*HighlightFactor+(MagickRealType) foreground*
        (QuantumRange-HighlightFactor))),q);
      SetPixelBlue(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelBlue(image,q)*HighlightFactor+(MagickRealType) foreground*
        (QuantumRange-HighlightFactor))),q);
      q+=GetPixelComponents(image);
    }
    for ( ; x < (ssize_t) (image->columns-raise_info->width); x++)
      q+=GetPixelComponents(image);
    for ( ; x < (ssize_t) image->columns; x++)
    {
      SetPixelRed(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelRed(image,q)*ShadowFactor+(MagickRealType) background*
        (QuantumRange-ShadowFactor))),q);
      SetPixelGreen(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelGreen(image,q)*ShadowFactor+(MagickRealType) background*
        (QuantumRange-ShadowFactor))),q);
      SetPixelBlue(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelBlue(image,q)*ShadowFactor+(MagickRealType) background*
        (QuantumRange-ShadowFactor))),q);
      q+=GetPixelComponents(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,RaiseImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
#if defined(MAGICKCORE_OPENMP_SUPPORT) 
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status) omp_throttle(1)
#endif
  for (y=(ssize_t) (image->rows-raise_info->height); y < (ssize_t) image->rows; y++)
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
    for (x=0; x < (ssize_t) (image->rows-y); x++)
    {
      SetPixelRed(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelRed(image,q)*HighlightFactor+(MagickRealType) foreground*
        (QuantumRange-HighlightFactor))),q);
      SetPixelGreen(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelGreen(image,q)*HighlightFactor+(MagickRealType) foreground*
        (QuantumRange-HighlightFactor))),q);
      SetPixelBlue(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelBlue(image,q)*HighlightFactor+(MagickRealType) foreground*
        (QuantumRange-HighlightFactor))),q);
      q+=GetPixelComponents(image);
    }
    for ( ; x < (ssize_t) (image->columns-(image->rows-y)); x++)
    {
      SetPixelRed(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelRed(image,q)*TroughFactor+(MagickRealType) background*
        (QuantumRange-TroughFactor))),q);
      SetPixelGreen(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelGreen(image,q)*TroughFactor+(MagickRealType) background*
        (QuantumRange-TroughFactor))),q);
      SetPixelBlue(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelBlue(image,q)*TroughFactor+(MagickRealType) background*
        (QuantumRange-TroughFactor))),q);
      q+=GetPixelComponents(image);
    }
    for ( ; x < (ssize_t) image->columns; x++)
    {
      SetPixelRed(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelRed(image,q)*ShadowFactor+(MagickRealType) background*
        (QuantumRange-ShadowFactor))),q);
      SetPixelGreen(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelGreen(image,q)*ShadowFactor+(MagickRealType) background*
        (QuantumRange-ShadowFactor))),q);
      SetPixelBlue(image,ClampToQuantum(QuantumScale*((MagickRealType)
        GetPixelBlue(image,q)*ShadowFactor+(MagickRealType) background*
        (QuantumRange-ShadowFactor))),q);
      q+=GetPixelComponents(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_RaiseImage)
#endif
        proceed=SetImageProgress(image,RaiseImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

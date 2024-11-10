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
%                                     Cristy                                  %
%                                   July 1992                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
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
#include "MagickCore/resource_.h"
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
%        const CompositeOperator compose,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o border_info:  define the width and height of the border.
%
%    o compose:  the composite operator.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *BorderImage(const Image *image,
  const RectangleInfo *border_info,const CompositeOperator compose,
  ExceptionInfo *exception)
{
  Image
    *border_image,
    *clone_image;

  FrameInfo
    frame_info;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(border_info != (RectangleInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
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
  border_image=FrameImage(clone_image,&frame_info,compose,exception);
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
%        const CompositeOperator compose,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o frame_info: Define the width and height of the frame and its bevels.
%
%    o compose: the composite operator.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *FrameImage(const Image *image,const FrameInfo *frame_info,
  const CompositeOperator compose,ExceptionInfo *exception)
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
    highlight,
    matte,
    shadow,
    trough;

  size_t
    bevel_width,
    height;

  ssize_t
    x_offset,
    y,
    y_offset;

  /*
    Check frame geometry.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(frame_info != (FrameInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((frame_info->outer_bevel < 0) || (frame_info->inner_bevel < 0))
    ThrowImageException(OptionError,"FrameIsLessThanImageSize");
  bevel_width=(size_t) (frame_info->outer_bevel+frame_info->inner_bevel);
  x_offset=(ssize_t) frame_info->width-frame_info->x-(ssize_t) bevel_width;
  y_offset=(ssize_t) frame_info->height-frame_info->y-(ssize_t) bevel_width;
  if ((x_offset < (ssize_t) image->columns) ||
      (y_offset < (ssize_t) image->rows))
    ThrowImageException(OptionError,"FrameIsLessThanImageSize");
  /*
    Initialize framed image attributes.
  */
  frame_image=CloneImage(image,frame_info->width,frame_info->height,MagickTrue,
    exception);
  if (frame_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(frame_image,DirectClass,exception) == MagickFalse)
    {
      frame_image=DestroyImage(frame_image);
      return((Image *) NULL);
    }
  if ((IsPixelInfoGray(&frame_image->border_color) == MagickFalse) &&
      (IsGrayColorspace(frame_image->colorspace) != MagickFalse))
    (void) SetImageColorspace(frame_image,sRGBColorspace,exception);
  if ((frame_image->matte_color.alpha_trait != UndefinedPixelTrait) &&
      (frame_image->alpha_trait == UndefinedPixelTrait))
    (void) SetImageAlpha(frame_image,OpaqueAlpha,exception);
  frame_image->page=image->page;
  if ((image->page.width != 0) && (image->page.height != 0))
    {
      frame_image->page.width+=frame_image->columns-image->columns;
      frame_image->page.height+=frame_image->rows-image->rows;
    }
  /*
    Initialize 3D effects color.
  */
  matte=image->matte_color;
  accentuate=matte;
  accentuate.red=(QuantumScale*(((double) QuantumRange-(double)
    AccentuateModulate)*matte.red+((double) QuantumRange*(double)
    AccentuateModulate)));
  accentuate.green=(QuantumScale*(((double) QuantumRange-(double)
    AccentuateModulate)*matte.green+((double) QuantumRange*(double)
    AccentuateModulate)));
  accentuate.blue=(QuantumScale*(((double) QuantumRange-(double)
    AccentuateModulate)*matte.blue+((double) QuantumRange*(double)
    AccentuateModulate)));
  accentuate.black=(QuantumScale*(((double) QuantumRange-(double)
    AccentuateModulate)*matte.black+((double) QuantumRange*(double)
    AccentuateModulate)));
  accentuate.alpha=matte.alpha;
  highlight=matte;
  highlight.red=(QuantumScale*(((double) QuantumRange-(double)
    HighlightModulate)*matte.red+((double) QuantumRange*(double)
    HighlightModulate)));
  highlight.green=(QuantumScale*(((double) QuantumRange-(double)
    HighlightModulate)*matte.green+((double) QuantumRange*(double)
    HighlightModulate)));
  highlight.blue=(QuantumScale*(((double) QuantumRange-(double)
    HighlightModulate)*matte.blue+((double) QuantumRange*(double)
    HighlightModulate)));
  highlight.black=(QuantumScale*(((double) QuantumRange-(double)
    HighlightModulate)*matte.black+((double) QuantumRange*(double)
    HighlightModulate)));
  highlight.alpha=matte.alpha;
  shadow=matte;
  shadow.red=QuantumScale*matte.red*(double) ShadowModulate;
  shadow.green=QuantumScale*matte.green*(double) ShadowModulate;
  shadow.blue=QuantumScale*matte.blue*(double) ShadowModulate;
  shadow.black=QuantumScale*matte.black*(double) ShadowModulate;
  shadow.alpha=matte.alpha;
  trough=matte;
  trough.red=QuantumScale*matte.red*(double) TroughModulate;
  trough.green=QuantumScale*matte.green*(double) TroughModulate;
  trough.blue=QuantumScale*matte.blue*(double) TroughModulate;
  trough.black=QuantumScale*matte.black*(double) TroughModulate;
  trough.alpha=matte.alpha;
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  frame_view=AcquireAuthenticCacheView(frame_image,exception);
  height=(size_t) (frame_info->outer_bevel+(frame_info->y-(ssize_t)
    bevel_width)+frame_info->inner_bevel);
  if (height != 0)
    {
      Quantum
        *magick_restrict q;

      ssize_t
        x;

      size_t
        width;

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
            for (x=0; x < ((ssize_t) frame_image->columns-y); x++)
            {
              if (x < y)
                SetPixelViaPixelInfo(frame_image,&highlight,q);
              else
                SetPixelViaPixelInfo(frame_image,&accentuate,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            for ( ; x < (ssize_t) frame_image->columns; x++)
            {
              SetPixelViaPixelInfo(frame_image,&shadow,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
          }
          for (y=0; y < (frame_info->y-(ssize_t) bevel_width); y++)
          {
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelViaPixelInfo(frame_image,&highlight,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            width=frame_image->columns-2*(size_t) frame_info->outer_bevel;
            for (x=0; x < (ssize_t) width; x++)
            {
              SetPixelViaPixelInfo(frame_image,&matte,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelViaPixelInfo(frame_image,&shadow,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
          }
          for (y=0; y < (ssize_t) frame_info->inner_bevel; y++)
          {
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelViaPixelInfo(frame_image,&highlight,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            for (x=0; x < (frame_info->x-(ssize_t) bevel_width); x++)
            {
              SetPixelViaPixelInfo(frame_image,&matte,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            width=image->columns+((size_t) frame_info->inner_bevel << 1)-
              (size_t) y;
            for (x=0; x < (ssize_t) width; x++)
            {
              if (x < y)
                SetPixelViaPixelInfo(frame_image,&shadow,q);
              else
                SetPixelViaPixelInfo(frame_image,&trough,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            for ( ; x < ((ssize_t) image->columns+2*frame_info->inner_bevel); x++)
            {
              SetPixelViaPixelInfo(frame_image,&highlight,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            width=frame_info->width-(size_t) frame_info->x-
              image->columns-bevel_width;
            for (x=0; x < (ssize_t) width; x++)
            {
              SetPixelViaPixelInfo(frame_image,&matte,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelViaPixelInfo(frame_image,&shadow,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
          }
          (void) SyncCacheViewAuthenticPixels(frame_view,exception);
        }
    }
  /*
    Draw sides of ornamental border.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,frame_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    ssize_t
      x;

    Quantum
      *magick_restrict q;

    size_t
      width;

    /*
      Initialize scanline with matte color.
    */
    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(frame_view,0,frame_info->y+y,
      frame_image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
    {
      SetPixelViaPixelInfo(frame_image,&highlight,q);
      q+=(ptrdiff_t) GetPixelChannels(frame_image);
    }
    for (x=0; x < (frame_info->x-(ssize_t) bevel_width); x++)
    {
      SetPixelViaPixelInfo(frame_image,&matte,q);
      q+=(ptrdiff_t) GetPixelChannels(frame_image);
    }
    for (x=0; x < (ssize_t) frame_info->inner_bevel; x++)
    {
      SetPixelViaPixelInfo(frame_image,&shadow,q);
      q+=(ptrdiff_t) GetPixelChannels(frame_image);
    }
    /*
      Set frame interior pixels.
    */
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelViaPixelInfo(frame_image,&frame_image->border_color,q);
      q+=(ptrdiff_t) GetPixelChannels(frame_image);
    }
    for (x=0; x < (ssize_t) frame_info->inner_bevel; x++)
    {
      SetPixelViaPixelInfo(frame_image,&highlight,q);
      q+=(ptrdiff_t) GetPixelChannels(frame_image);
    }
    width=frame_info->width-(size_t) frame_info->x-image->columns-bevel_width;
    for (x=0; x < (ssize_t) width; x++)
    {
      SetPixelViaPixelInfo(frame_image,&matte,q);
      q+=(ptrdiff_t) GetPixelChannels(frame_image);
    }
    for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
    {
      SetPixelViaPixelInfo(frame_image,&shadow,q);
      q+=(ptrdiff_t) GetPixelChannels(frame_image);
    }
    if (SyncCacheViewAuthenticPixels(frame_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,FrameImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  height=(size_t) (frame_info->inner_bevel+(ssize_t) frame_info->height-
    frame_info->y-(ssize_t) image->rows-(ssize_t) bevel_width+
    frame_info->outer_bevel);
  if (height != 0)
    {
      size_t
        width;

      ssize_t
        x;

      Quantum
        *magick_restrict q;

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
              SetPixelViaPixelInfo(frame_image,&highlight,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            for (x=0; x < (frame_info->x-(ssize_t) bevel_width); x++)
            {
              SetPixelViaPixelInfo(frame_image,&matte,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            for (x=0; x < y; x++)
            {
              SetPixelViaPixelInfo(frame_image,&shadow,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            for ( ; x < ((ssize_t) image->columns+2*frame_info->inner_bevel); x++)
            {
              if (x >= ((ssize_t) image->columns+2*frame_info->inner_bevel-y))
                SetPixelViaPixelInfo(frame_image,&highlight,q);
              else
                SetPixelViaPixelInfo(frame_image,&accentuate,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            width=(size_t) ((ssize_t) frame_info->width-frame_info->x-
              (ssize_t) image->columns-(ssize_t) bevel_width);
            for (x=0; x < (ssize_t) width; x++)
            {
              SetPixelViaPixelInfo(frame_image,&matte,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelViaPixelInfo(frame_image,&shadow,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
          }
          height=(size_t) ((ssize_t) frame_info->height-frame_info->y-(ssize_t)
            image->rows-(ssize_t) bevel_width);
          for (y=0; y < (ssize_t) height; y++)
          {
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelViaPixelInfo(frame_image,&highlight,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            width=(size_t) ((ssize_t) frame_image->columns-2*(ssize_t)
              frame_info->outer_bevel);
            for (x=0; x < (ssize_t) width; x++)
            {
              SetPixelViaPixelInfo(frame_image,&matte,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            for (x=0; x < (ssize_t) frame_info->outer_bevel; x++)
            {
              SetPixelViaPixelInfo(frame_image,&shadow,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
          }
          for (y=frame_info->outer_bevel-1; y >= 0; y--)
          {
            for (x=0; x < y; x++)
            {
              SetPixelViaPixelInfo(frame_image,&highlight,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
            for ( ; x < (ssize_t) frame_image->columns; x++)
            {
              if (x >= ((ssize_t) frame_image->columns-y))
                SetPixelViaPixelInfo(frame_image,&shadow,q);
              else
                SetPixelViaPixelInfo(frame_image,&trough,q);
              q+=(ptrdiff_t) GetPixelChannels(frame_image);
            }
          }
          (void) SyncCacheViewAuthenticPixels(frame_view,exception);
        }
    }
  frame_view=DestroyCacheView(frame_view);
  image_view=DestroyCacheView(image_view);
  x_offset=frame_info->outer_bevel+(frame_info->x-(ssize_t) bevel_width)+
    frame_info->inner_bevel;
  y_offset=frame_info->outer_bevel+(frame_info->y-(ssize_t) bevel_width)+
    frame_info->inner_bevel;
  if (status != MagickFalse)
    status=CompositeImage(frame_image,image,compose,MagickTrue,x_offset,
      y_offset,exception);
  if (status == MagickFalse)
    frame_image=DestroyImage(frame_image);
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
%        const RectangleInfo *raise_info,const MagickBooleanType raise,
%        ExceptionInfo *exception)
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
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType RaiseImage(Image *image,
  const RectangleInfo *raise_info,const MagickBooleanType raise,
  ExceptionInfo *exception)
{
#define AccentuateFactor  ScaleCharToQuantum(135)
#define HighlightFactor  ScaleCharToQuantum(190)
#define ShadowFactor  ScaleCharToQuantum(190)
#define RaiseImageTag  "Raise/Image"
#define TroughFactor  ScaleCharToQuantum(135)

  CacheView
    *image_view;

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
  assert(image->signature == MagickCoreSignature);
  assert(raise_info != (RectangleInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->columns <= (raise_info->width << 1)) ||
      (image->rows <= (raise_info->height << 1)))
    ThrowBinaryException(OptionError,"ImageSizeMustExceedBevelWidth",
      image->filename);
  foreground=QuantumRange;
  background=(Quantum) 0;
  if (raise == MagickFalse)
    {
      foreground=(Quantum) 0;
      background=QuantumRange;
    }
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  /*
    Raise image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,raise_info->height,1)
#endif
  for (y=0; y < (ssize_t) raise_info->height; y++)
  {
    ssize_t
      i,
      x;

    Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < y; x++)
    {
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ClampToQuantum(QuantumScale*((double) q[i]*(double)
          HighlightFactor+(double) foreground*((double) QuantumRange-(double)
          HighlightFactor)));
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    for ( ; x < ((ssize_t) image->columns-y); x++)
    {
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ClampToQuantum(QuantumScale*((double) q[i]*(double)
          AccentuateFactor+(double) foreground*((double) QuantumRange-(double)
          AccentuateFactor)));
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    for ( ; x < (ssize_t) image->columns; x++)
    {
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ClampToQuantum(QuantumScale*((double) q[i]*(double) ShadowFactor+
          (double) background*((double) QuantumRange-(double) ShadowFactor)));
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
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
        proceed=SetImageProgress(image,RaiseImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows-2*raise_info->height,1)
#endif
  for (y=(ssize_t) raise_info->height; y < (ssize_t) (image->rows-raise_info->height); y++)
  {
    ssize_t
      i,
      x;

    Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) raise_info->width; x++)
    {
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ClampToQuantum(QuantumScale*((double) q[i]*(double)
          HighlightFactor+(double) foreground*((double) QuantumRange-(double)
          HighlightFactor)));
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    for ( ; x < (ssize_t) (image->columns-raise_info->width); x++)
      q+=(ptrdiff_t) GetPixelChannels(image);
    for ( ; x < (ssize_t) image->columns; x++)
    {
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ClampToQuantum(QuantumScale*((double) q[i]*(double) ShadowFactor+
          (double) background*((double) QuantumRange-(double) ShadowFactor)));
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
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
        proceed=SetImageProgress(image,RaiseImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows-raise_info->height,1)
#endif
  for (y=(ssize_t) (image->rows-raise_info->height); y < (ssize_t) image->rows; y++)
  {
    ssize_t
      i,
      x;

    Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < ((ssize_t) image->rows-y); x++)
    {
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ClampToQuantum(QuantumScale*((double) q[i]*(double)
          HighlightFactor+(double) foreground*((double) QuantumRange-
          (double) HighlightFactor)));
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    for ( ; x < ((ssize_t) image->columns-((ssize_t) image->rows-y)); x++)
    {
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ClampToQuantum(QuantumScale*((double) q[i]*(double) TroughFactor+
          (double) background*((double) QuantumRange-(double) TroughFactor)));
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    for ( ; x < (ssize_t) image->columns; x++)
    {
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ClampToQuantum(QuantumScale*((double) q[i]*(double) ShadowFactor+
          (double) background*((double) QuantumRange-(double) ShadowFactor)));
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
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
        proceed=SetImageProgress(image,RaiseImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

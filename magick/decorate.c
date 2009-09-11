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
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/cache-view.h"
#include "magick/color-private.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/decorate.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-private.h"
#include "magick/quantum.h"
#include "magick/transform.h"

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
    *border_image;

  /*
    Decorate the image with a border.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(border_info != (const RectangleInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  border_image=CloneImage(image,image->columns+2*border_info->width,
    image->rows+2*border_info->height,MagickTrue,exception);
  if (border_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(border_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&border_image->exception);
      border_image=DestroyImage(border_image);
      return((Image *) NULL);
    }
  border_image->background_color=border_image->border_color;
  if (border_image->background_color.opacity != OpaqueOpacity)
    border_image->matte=MagickTrue;
  (void) SetImageBackgroundColor(border_image);
  (void) CompositeImage(border_image,image->compose,image,(long)
    border_info->width,(long) border_info->height);
  border_image->background_color=image->background_color;
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

  Image
    *frame_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    accentuate,
    border,
    highlight,
    interior,
    matte,
    shadow,
    trough;

  register long
    x;

  unsigned long
    bevel_width,
    height,
    width;

  CacheView
    *frame_view;

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
  bevel_width=(unsigned long) (frame_info->outer_bevel+frame_info->inner_bevel);
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
  if (frame_image->matte_color.opacity != OpaqueOpacity)
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
  GetMagickPixelPacket(frame_image,&interior);
  SetMagickPixelPacket(frame_image,&image->border_color,(IndexPacket *) NULL,
    &interior);
  GetMagickPixelPacket(frame_image,&matte);
  matte.colorspace=RGBColorspace;
  SetMagickPixelPacket(frame_image,&image->matte_color,(IndexPacket *) NULL,
    &matte);
  GetMagickPixelPacket(frame_image,&border);
  border.colorspace=RGBColorspace;
  SetMagickPixelPacket(frame_image,&image->border_color,(IndexPacket *) NULL,
    &border);
  GetMagickPixelPacket(frame_image,&accentuate);
  accentuate.red=(MagickRealType) (QuantumScale*((QuantumRange-
    AccentuateModulate)*matte.red+(QuantumRange*AccentuateModulate)));
  accentuate.green=(MagickRealType) (QuantumScale*((QuantumRange-
    AccentuateModulate)*matte.green+(QuantumRange*AccentuateModulate)));
  accentuate.blue=(MagickRealType) (QuantumScale*((QuantumRange-
    AccentuateModulate)*matte.blue+(QuantumRange*AccentuateModulate)));
  accentuate.opacity=matte.opacity;
  GetMagickPixelPacket(frame_image,&highlight);
  highlight.red=(MagickRealType) (QuantumScale*((QuantumRange-
    HighlightModulate)*matte.red+(QuantumRange*HighlightModulate)));
  highlight.green=(MagickRealType) (QuantumScale*((QuantumRange-
    HighlightModulate)*matte.green+(QuantumRange*HighlightModulate)));
  highlight.blue=(MagickRealType) (QuantumScale*((QuantumRange-
    HighlightModulate)*matte.blue+(QuantumRange*HighlightModulate)));
  highlight.opacity=matte.opacity;
  GetMagickPixelPacket(frame_image,&shadow);
  shadow.red=QuantumScale*matte.red*ShadowModulate;
  shadow.green=QuantumScale*matte.green*ShadowModulate;
  shadow.blue=QuantumScale*matte.blue*ShadowModulate;
  shadow.opacity=matte.opacity;
  GetMagickPixelPacket(frame_image,&trough);
  trough.red=QuantumScale*matte.red*TroughModulate;
  trough.green=QuantumScale*matte.green*TroughModulate;
  trough.blue=QuantumScale*matte.blue*TroughModulate;
  trough.opacity=matte.opacity;
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
  frame_view=AcquireCacheView(frame_image);
  height=(unsigned long) (frame_info->outer_bevel+(frame_info->y-bevel_width)+
    frame_info->inner_bevel);
  if (height != 0)
    {
      register IndexPacket
        *__restrict frame_indexes;

      register long
        x;

      register PixelPacket
        *__restrict q;

      /*
        Draw top of ornamental border.
      */
      q=QueueCacheViewAuthenticPixels(frame_view,0,0,frame_image->columns,
        height,exception);
      frame_indexes=GetCacheViewAuthenticIndexQueue(frame_view);
      if (q != (PixelPacket *) NULL)
        {
          /*
            Draw top of ornamental border.
          */
          for (y=0; y < (long) frame_info->outer_bevel; y++)
          {
            for (x=0; x < (long) (frame_image->columns-y); x++)
            {
              if (x < y)
                SetPixelPacket(frame_image,&highlight,q,frame_indexes);
              else
                SetPixelPacket(frame_image,&accentuate,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            for ( ; x < (long) frame_image->columns; x++)
            {
              SetPixelPacket(frame_image,&shadow,q,frame_indexes);
              q++;
              frame_indexes++;
            }
          }
          for (y=0; y < (long) (frame_info->y-bevel_width); y++)
          {
            for (x=0; x < (long) frame_info->outer_bevel; x++)
            {
              SetPixelPacket(frame_image,&highlight,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            width=frame_image->columns-2*frame_info->outer_bevel;
            for (x=0; x < (long) width; x++)
            {
              SetPixelPacket(frame_image,&matte,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            for (x=0; x < (long) frame_info->outer_bevel; x++)
            {
              SetPixelPacket(frame_image,&shadow,q,frame_indexes);
              q++;
              frame_indexes++;
            }
          }
          for (y=0; y < (long) frame_info->inner_bevel; y++)
          {
            for (x=0; x < (long) frame_info->outer_bevel; x++)
            {
              SetPixelPacket(frame_image,&highlight,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            for (x=0; x < (long) (frame_info->x-bevel_width); x++)
            {
              SetPixelPacket(frame_image,&matte,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            width=image->columns+((unsigned long) frame_info->inner_bevel << 1)-
              y;
            for (x=0; x < (long) width; x++)
            {
              if (x < y)
                SetPixelPacket(frame_image,&shadow,q,frame_indexes);
              else
                SetPixelPacket(frame_image,&trough,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            for ( ; x < (long) (image->columns+2*frame_info->inner_bevel); x++)
            {
              SetPixelPacket(frame_image,&highlight,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            width=frame_info->width-frame_info->x-image->columns-bevel_width;
            for (x=0; x < (long) width; x++)
            {
              SetPixelPacket(frame_image,&matte,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            for (x=0; x < (long) frame_info->outer_bevel; x++)
            {
              SetPixelPacket(frame_image,&shadow,q,frame_indexes);
              q++;
              frame_indexes++;
            }
          }
          (void) SyncCacheViewAuthenticPixels(frame_view,exception);
        }
    }
  /*
    Draw sides of ornamental border.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *__restrict frame_indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    /*
      Initialize scanline with matte color.
    */
    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(frame_view,0,frame_info->y+y,
      frame_image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    frame_indexes=GetCacheViewAuthenticIndexQueue(frame_view);
    for (x=0; x < (long) frame_info->outer_bevel; x++)
    {
      SetPixelPacket(frame_image,&highlight,q,frame_indexes);
      q++;
      frame_indexes++;
    }
    for (x=0; x < (long) (frame_info->x-bevel_width); x++)
    {
      SetPixelPacket(frame_image,&matte,q,frame_indexes);
      q++;
      frame_indexes++;
    }
    for (x=0; x < (long) frame_info->inner_bevel; x++)
    {
      SetPixelPacket(frame_image,&shadow,q,frame_indexes);
      q++;
      frame_indexes++;
    }
    /*
      Set frame interior to interior color.
    */
    for (x=0; x < (long) image->columns; x++)
    {
      SetPixelPacket(frame_image,&interior,q,frame_indexes);
      q++;
      frame_indexes++;
    }
    for (x=0; x < (long) frame_info->inner_bevel; x++)
    {
      SetPixelPacket(frame_image,&highlight,q,frame_indexes);
      q++;
      frame_indexes++;
    }
    width=frame_info->width-frame_info->x-image->columns-bevel_width;
    for (x=0; x < (long) width; x++)
    {
      SetPixelPacket(frame_image,&matte,q,frame_indexes);
      q++;
      frame_indexes++;
    }
    for (x=0; x < (long) frame_info->outer_bevel; x++)
    {
      SetPixelPacket(frame_image,&shadow,q,frame_indexes);
      q++;
      frame_indexes++;
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
  height=(unsigned long) (frame_info->inner_bevel+frame_info->height-
    frame_info->y-image->rows-bevel_width+frame_info->outer_bevel);
  if (height != 0)
    {
      register IndexPacket
        *__restrict frame_indexes;

      register long
        x;

      register PixelPacket
        *__restrict q;

      /*
        Draw bottom of ornamental border.
      */
      q=QueueCacheViewAuthenticPixels(frame_view,0,(long) (frame_image->rows-
        height),frame_image->columns,height,exception);
      if (q != (PixelPacket *) NULL)
        {
          /*
            Draw bottom of ornamental border.
          */
          frame_indexes=GetCacheViewAuthenticIndexQueue(frame_view);
          for (y=frame_info->inner_bevel-1; y >= 0; y--)
          {
            for (x=0; x < (long) frame_info->outer_bevel; x++)
            {
              SetPixelPacket(frame_image,&highlight,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            for (x=0; x < (long) (frame_info->x-bevel_width); x++)
            {
              SetPixelPacket(frame_image,&matte,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            for (x=0; x < y; x++)
            {
              SetPixelPacket(frame_image,&shadow,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            for ( ; x < (long) (image->columns+2*frame_info->inner_bevel); x++)
            {
              if (x >= (long) (image->columns+2*frame_info->inner_bevel-y))
                SetPixelPacket(frame_image,&highlight,q,frame_indexes);
              else
                SetPixelPacket(frame_image,&accentuate,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            width=frame_info->width-frame_info->x-image->columns-bevel_width;
            for (x=0; x < (long) width; x++)
            {
              SetPixelPacket(frame_image,&matte,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            for (x=0; x < (long) frame_info->outer_bevel; x++)
            {
              SetPixelPacket(frame_image,&shadow,q,frame_indexes);
              q++;
              frame_indexes++;
            }
          }
          height=frame_info->height-frame_info->y-image->rows-bevel_width;
          for (y=0; y < (long) height; y++)
          {
            for (x=0; x < (long) frame_info->outer_bevel; x++)
            {
              SetPixelPacket(frame_image,&highlight,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            width=frame_image->columns-2*frame_info->outer_bevel;
            for (x=0; x < (long) width; x++)
            {
              SetPixelPacket(frame_image,&matte,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            for (x=0; x < (long) frame_info->outer_bevel; x++)
            {
              SetPixelPacket(frame_image,&shadow,q,frame_indexes);
              q++;
              frame_indexes++;
            }
          }
          for (y=frame_info->outer_bevel-1; y >= 0; y--)
          {
            for (x=0; x < y; x++)
            {
              SetPixelPacket(frame_image,&highlight,q,frame_indexes);
              q++;
              frame_indexes++;
            }
            for ( ; x < (long) frame_image->columns; x++)
            {
              if (x >= (long) (frame_image->columns-y))
                SetPixelPacket(frame_image,&shadow,q,frame_indexes);
              else
                SetPixelPacket(frame_image,&trough,q,frame_indexes);
              q++;
              frame_indexes++;
            }
          }
          (void) SyncCacheViewAuthenticPixels(frame_view,exception);
        }
    }
  frame_view=DestroyCacheView(frame_view);
  x=(long) (frame_info->outer_bevel+(frame_info->x-bevel_width)+
    frame_info->inner_bevel);
  y=(long) (frame_info->outer_bevel+(frame_info->y-bevel_width)+
    frame_info->inner_bevel);
  (void) CompositeImage(frame_image,image->compose,image,x,y);
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

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  Quantum
    foreground,
    background;

  CacheView
    *image_view;

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
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) raise_info->height; y++)
  {
    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < y; x++)
    {
      q->red=RoundToQuantum(QuantumScale*((MagickRealType) q->red*
        HighlightFactor+(MagickRealType) foreground*(QuantumRange-
        HighlightFactor)));
      q->green=RoundToQuantum(QuantumScale*((MagickRealType) q->green*
        HighlightFactor+(MagickRealType) foreground*(QuantumRange-
        HighlightFactor)));
      q->blue=RoundToQuantum(QuantumScale*((MagickRealType) q->blue*
        HighlightFactor+(MagickRealType) foreground*(QuantumRange-
        HighlightFactor)));
      q++;
    }
    for ( ; x < (long) (image->columns-y); x++)
    {
      q->red=RoundToQuantum(QuantumScale*((MagickRealType) q->red*
        AccentuateFactor+(MagickRealType) foreground*(QuantumRange-
        AccentuateFactor)));
      q->green=RoundToQuantum(QuantumScale*((MagickRealType) q->green*
        AccentuateFactor+(MagickRealType) foreground*(QuantumRange-
        AccentuateFactor)));
      q->blue=RoundToQuantum(QuantumScale*((MagickRealType) q->blue*
        AccentuateFactor+(MagickRealType) foreground*(QuantumRange-
        AccentuateFactor)));
      q++;
    }
    for ( ; x < (long) image->columns; x++)
    {
      q->red=RoundToQuantum(QuantumScale*((MagickRealType) q->red*ShadowFactor+
        (MagickRealType) background*(QuantumRange-ShadowFactor)));
      q->green=RoundToQuantum(QuantumScale*((MagickRealType) q->green*
        ShadowFactor+(MagickRealType) background*(QuantumRange-ShadowFactor)));
      q->blue=RoundToQuantum(QuantumScale*((MagickRealType) q->blue*
        ShadowFactor+(MagickRealType) background*(QuantumRange-ShadowFactor)));
      q++;
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
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=(long) raise_info->height; y < (long) (image->rows-raise_info->height); y++)
  {
    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (long) raise_info->width; x++)
    {
      q->red=RoundToQuantum(QuantumScale*((MagickRealType) q->red*
        HighlightFactor+(MagickRealType) foreground*(QuantumRange-
        HighlightFactor)));
      q->green=RoundToQuantum(QuantumScale*((MagickRealType) q->green*
        HighlightFactor+(MagickRealType) foreground*(QuantumRange-
        HighlightFactor)));
      q->blue=RoundToQuantum(QuantumScale*((MagickRealType) q->blue*
        HighlightFactor+(MagickRealType) foreground*(QuantumRange-
        HighlightFactor)));
      q++;
    }
    for ( ; x < (long) (image->columns-raise_info->width); x++)
      q++;
    for ( ; x < (long) image->columns; x++)
    {
      q->red=RoundToQuantum(QuantumScale*((MagickRealType) q->red*ShadowFactor+
        (MagickRealType) background*(QuantumRange-ShadowFactor)));
      q->green=RoundToQuantum(QuantumScale*((MagickRealType) q->green*
        ShadowFactor+(MagickRealType) background*(QuantumRange-ShadowFactor)));
      q->blue=RoundToQuantum(QuantumScale*((MagickRealType) q->blue*
        ShadowFactor+(MagickRealType) background*(QuantumRange-ShadowFactor)));
      q++;
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
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=(long) (image->rows-raise_info->height); y < (long) image->rows; y++)
  {
    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (long) (image->rows-y); x++)
    {
      q->red=RoundToQuantum(QuantumScale*((MagickRealType) q->red*
        HighlightFactor+(MagickRealType) foreground*(QuantumRange-
        HighlightFactor)));
      q->green=RoundToQuantum(QuantumScale*((MagickRealType) q->green*
        HighlightFactor+(MagickRealType) foreground*(QuantumRange-
        HighlightFactor)));
      q->blue=RoundToQuantum(QuantumScale*((MagickRealType) q->blue*
        HighlightFactor+(MagickRealType) foreground*(QuantumRange-
        HighlightFactor)));
      q++;
    }
    for ( ; x < (long) (image->columns-(image->rows-y)); x++)
    {
      q->red=RoundToQuantum(QuantumScale*((MagickRealType) q->red*TroughFactor+
        (MagickRealType) background*(QuantumRange-TroughFactor)));
      q->green=RoundToQuantum(QuantumScale*((MagickRealType) q->green*
        TroughFactor+(MagickRealType) background*(QuantumRange-TroughFactor)));
      q->blue=RoundToQuantum(QuantumScale*((MagickRealType) q->blue*
        TroughFactor+(MagickRealType) background*(QuantumRange-TroughFactor)));
      q++;
    }
    for ( ; x < (long) image->columns; x++)
    {
      q->red=RoundToQuantum(QuantumScale*((MagickRealType) q->red*ShadowFactor+
        (MagickRealType) background*(QuantumRange-ShadowFactor)));
      q->green=RoundToQuantum(QuantumScale*((MagickRealType) q->green*
        ShadowFactor+(MagickRealType) background*(QuantumRange-ShadowFactor)));
      q->blue=RoundToQuantum(QuantumScale*((MagickRealType) q->blue*
        ShadowFactor+(MagickRealType) background*(QuantumRange-ShadowFactor)));
      q++;
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

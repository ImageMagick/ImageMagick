/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               CCCC   AAA   PPPP   TTTTT  IIIII   OOO   N   N                %
%              C      A   A  P   P    T      I    O   O  NN  N                %
%              C      AAAAA  PPPP     T      I    O   O  N N N                %
%              C      A   A  P        T      I    O   O  N  NN                %
%               CCCC  A   A  P        T    IIIII   OOO   N   N                %
%                                                                             %
%                                                                             %
%                             Read Text Caption.                              %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                               February 2002                                 %
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
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/annotate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/draw.h"
#include "MagickCore/draw-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/option.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/utility.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d C A P T I O N I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadCAPTIONImage() reads a CAPTION image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadCAPTIONImage method is:
%
%      Image *ReadCAPTIONImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline void AdjustTypeMetricBounds(TypeMetric *metrics)
{
  if (metrics->bounds.x1 >= 0.0)
    metrics->bounds.x1=0.0;
  else
    {
      double x1 = ceil(-metrics->bounds.x1+0.5);
      metrics->width+=x1+x1;
      metrics->bounds.x1=x1;
    }
}

static Image *ReadCAPTIONImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    *caption,
    geometry[MagickPathExtent],
    *text;

  const char
    *gravity,
    *option;

  DrawInfo
    *draw_info;

  Image
    *image;

  MagickBooleanType
    left_bearing,
    split,
    status;

  size_t
    height,
    width;

  ssize_t
    i;

  TypeMetric
    metrics;

  /*
    Initialize Image structure.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image=AcquireImage(image_info,exception);
  (void) ResetImagePage(image,"0x0+0+0");
  if ((image->columns != 0) && (image->rows != 0))
    (void) SetImageBackgroundColor(image,exception);
  /*
    Format caption.
  */
  option=GetImageOption(image_info,"filename");
  if (option == (const char *) NULL)
    caption=InterpretImageProperties((ImageInfo *) image_info,image,
      image_info->filename,exception);
  else
    if (LocaleNCompare(option,"caption:",8) == 0)
      caption=InterpretImageProperties((ImageInfo *) image_info,image,option+8,
        exception);
    else
      caption=InterpretImageProperties((ImageInfo *) image_info,image,option,
        exception);
  if (caption == (char *) NULL)
    return(DestroyImageList(image));
  (void) SetImageProperty(image,"caption",caption,exception);
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  width=CastDoubleToUnsigned(0.5*draw_info->pointsize*strlen(caption)+0.5);
  if (AcquireMagickResource(WidthResource,width) == MagickFalse)
    {
      caption=DestroyString(caption);
      draw_info=DestroyDrawInfo(draw_info);
      ThrowReaderException(ImageError,"WidthOrHeightExceedsLimit");
    }
  (void) CloneString(&draw_info->text,caption);
  gravity=GetImageOption(image_info,"gravity");
  if (gravity != (char *) NULL)
    draw_info->gravity=(GravityType) ParseCommandOption(MagickGravityOptions,
      MagickFalse,gravity);
  split=IsStringTrue(GetImageOption(image_info,"caption:split"));
  status=MagickTrue;
  (void) memset(&metrics,0,sizeof(metrics));
  if (image->columns == 0)
    {
      text=AcquireString(caption);
      i=FormatMagickCaption(image,draw_info,split,&metrics,&text,exception);
      AdjustTypeMetricBounds(&metrics);
      (void) CloneString(&draw_info->text,text);
      text=DestroyString(text);
      (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
        metrics.bounds.x1,metrics.ascent);
      if (draw_info->gravity == UndefinedGravity)
        (void) CloneString(&draw_info->geometry,geometry);
      status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
      AdjustTypeMetricBounds(&metrics);
      image->columns=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
    }
  if (image->rows == 0)
    {
      split=MagickTrue;
      text=AcquireString(caption);
      i=FormatMagickCaption(image,draw_info,split,&metrics,&text,exception);
      AdjustTypeMetricBounds(&metrics);
      (void) CloneString(&draw_info->text,text);
      text=DestroyString(text);
      (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
        metrics.bounds.x1,metrics.ascent);
      if (draw_info->gravity == UndefinedGravity)
        (void) CloneString(&draw_info->geometry,geometry);
      status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
      AdjustTypeMetricBounds(&metrics);
      image->rows=(size_t) ((i+1)*(metrics.ascent-metrics.descent+
        draw_info->interline_spacing+draw_info->stroke_width)+0.5);
    }
  if (status != MagickFalse)
    status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    { 
      caption=DestroyString(caption);
      draw_info=DestroyDrawInfo(draw_info);
      return(DestroyImageList(image));
    }
  if (SetImageBackgroundColor(image,exception) == MagickFalse)
    {
      caption=DestroyString(caption);
      draw_info=DestroyDrawInfo(draw_info);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if ((fabs(image_info->pointsize) < MagickEpsilon) && (strlen(caption) > 0))
    {
      double
        high,
        low;

      ssize_t
        n;

      /*
        Auto fit text into bounding box.
      */
      low=1.0;
      option=GetImageOption(image_info,"caption:max-pointsize");
      if (option != (const char*) NULL)
        {
          high=StringToDouble(option,(char**) NULL);
          if (high < 1.0)
            high=1.0;
          high+=1.0;
        }
      else
        {
          option=GetImageOption(image_info,"caption:start-pointsize");
          if (option != (const char *) NULL)
            {
              draw_info->pointsize=StringToDouble(option,(char**) NULL);
              if (draw_info->pointsize < 1.0)
                draw_info->pointsize=1.0;
            }
          for (n=0; n < 32; n++, draw_info->pointsize*=2.0)
          {
            text=AcquireString(caption);
            i=FormatMagickCaption(image,draw_info,split,&metrics,&text,
              exception);
            AdjustTypeMetricBounds(&metrics);
            (void) CloneString(&draw_info->text,text);
            text=DestroyString(text);
            (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
              metrics.bounds.x1,metrics.ascent);
            if (draw_info->gravity == UndefinedGravity)
              (void) CloneString(&draw_info->geometry,geometry);
            status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
            if (status == MagickFalse)
              break;
            AdjustTypeMetricBounds(&metrics);
            width=CastDoubleToUnsigned(metrics.width+draw_info->stroke_width+0.5);
            height=CastDoubleToUnsigned(metrics.height-metrics.underline_position+
              draw_info->interline_spacing+draw_info->stroke_width+0.5);
            if ((image->columns != 0) && (image->rows != 0))
              {
                if ((width >= image->columns) || (height >= image->rows))
                  break;
                if ((width < image->columns) && (height < image->rows))
                  low=draw_info->pointsize;
              }
            else
              if (((image->columns != 0) && (width >= image->columns)) ||
                  ((image->rows != 0) && (height >= image->rows)))
                break;
          }
          high=draw_info->pointsize;
        }
      while ((high-low) > 0.5)
      {
        draw_info->pointsize=(low+high)/2.0;
        text=AcquireString(caption);
        i=FormatMagickCaption(image,draw_info,split,&metrics,&text,exception);
        AdjustTypeMetricBounds(&metrics);
        (void) CloneString(&draw_info->text,text);
        text=DestroyString(text);
        (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
          metrics.bounds.x1,metrics.ascent);
        if (draw_info->gravity == UndefinedGravity)
          (void) CloneString(&draw_info->geometry,geometry);
        status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
        if (status == MagickFalse)
          break;
        AdjustTypeMetricBounds(&metrics);
        width=CastDoubleToUnsigned(metrics.width+draw_info->stroke_width+0.5);
        height=CastDoubleToUnsigned(metrics.height-metrics.underline_position+
          draw_info->interline_spacing+draw_info->stroke_width+0.5);
        if ((image->columns != 0) && (image->rows != 0))
          {
            if ((width < image->columns) && (height < image->rows))
              low=draw_info->pointsize+0.5;
            else
              high=draw_info->pointsize-0.5;
          }
        else
          if (((image->columns != 0) && (width < image->columns)) ||
              ((image->rows != 0) && (height < image->rows)))
            low=draw_info->pointsize+0.5;
          else
            high=draw_info->pointsize-0.5;
      }
      draw_info->pointsize=floor((low+high)/2.0-0.5);
    }
  /*
    Draw caption.
  */
  i=FormatMagickCaption(image,draw_info,split,&metrics,&caption,exception);
  AdjustTypeMetricBounds(&metrics);
  (void) CloneString(&draw_info->text,caption);
  caption=DestroyString(caption);
  left_bearing=((draw_info->gravity == UndefinedGravity) ||
     (draw_info->gravity == NorthWestGravity) || 
     (draw_info->gravity == WestGravity) ||
     (draw_info->gravity == SouthWestGravity)) ? MagickTrue : MagickFalse;
  (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
    (draw_info->direction == RightToLeftDirection ? (double) image->columns-
    (draw_info->gravity == UndefinedGravity ? metrics.bounds.x2 : 0.0) : 
    (left_bearing != MagickFalse ? metrics.bounds.x1 : 0.0)),
    (draw_info->gravity == UndefinedGravity ? 
    MagickMax(metrics.ascent,metrics.bounds.y2) : 0.0));
  (void) CloneString(&draw_info->geometry,geometry);
  status=AnnotateImage(image,draw_info,exception);
  if (image_info->pointsize == 0.0)
    (void) FormatImageProperty(image,"caption:pointsize","%.*g",
      GetMagickPrecision(),draw_info->pointsize);
  (void) FormatImageProperty(image,"caption:lines","%.*g",GetMagickPrecision(),
    (double) (i+1));
  draw_info=DestroyDrawInfo(draw_info);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r C A P T I O N I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterCAPTIONImage() adds attributes for the CAPTION image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterCAPTIONImage method is:
%
%      size_t RegisterCAPTIONImage(void)
%
*/
ModuleExport size_t RegisterCAPTIONImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("CAPTION","CAPTION","Caption");
  entry->decoder=(DecodeImageHandler *) ReadCAPTIONImage;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r C A P T I O N I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterCAPTIONImage() removes format registrations made by the
%  CAPTION module from the list of supported formats.
%
%  The format of the UnregisterCAPTIONImage method is:
%
%      UnregisterCAPTIONImage(void)
%
*/
ModuleExport void UnregisterCAPTIONImage(void)
{
  (void) UnregisterMagickInfo("CAPTION");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     L       AAA   BBBB   EEEEE  L                           %
%                     L      A   A  B   B  E      L                           %
%                     L      AAAAA  BBBB   EEE    L                           %
%                     L      A   A  B   B  E      L                           %
%                     LLLLL  A   A  BBBB   EEEEE  LLLLL                       %
%                                                                             %
%                                                                             %
%                      Read ASCII String As An Image.                         %
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
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/annotate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d L A B E L I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadLABELImage() reads a LABEL image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadLABELImage method is:
%
%      Image *ReadLABELImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadLABELImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    geometry[MagickPathExtent],
    *label;

  DrawInfo
    *draw_info;

  Image
    *image;

  MagickBooleanType
    status;

  TypeMetric
    metrics;

  size_t
    height,
    width;

  /*
    Initialize Image structure.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  (void) ResetImagePage(image,"0x0+0+0");
  if ((image->columns != 0) && (image->rows != 0))
    (void) SetImageBackgroundColor(image,exception);
  label=InterpretImageProperties((ImageInfo *) image_info,image,
    image_info->filename,exception);
  if (label == (char *) NULL)
    return(DestroyImageList(image));
  (void) SetImageProperty(image,"label",label,exception);
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  width=(size_t) floor(draw_info->pointsize*strlen(label)+0.5);
  if (AcquireMagickResource(WidthResource,width) == MagickFalse)
    {
      label=DestroyString(label);
      draw_info=DestroyDrawInfo(draw_info);
      ThrowReaderException(ImageError,"WidthOrHeightExceedsLimit");
    }
  draw_info->text=ConstantString(label);
  metrics.width=0.0;
  metrics.height=0.0;
  metrics.ascent=0.0;
  status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
  if ((image->columns == 0) && (image->rows == 0))
    {
      image->columns=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
      image->rows=(size_t) floor(metrics.height+draw_info->stroke_width+0.5);
    }
  else
    if ((status != MagickFalse) && (strlen(label) > 0) &&
        (((image->columns == 0) || (image->rows == 0)) ||
         (fabs(image_info->pointsize) < MagickEpsilon)))
      {
        double
          high,
          low;

        ssize_t
          n;

        /*
          Auto fit text into bounding box.
        */
        for (n=0; n < 32; n++, draw_info->pointsize*=2.0)
        {
          (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
            -metrics.bounds.x1,metrics.ascent);
          if (draw_info->gravity == UndefinedGravity)
            (void) CloneString(&draw_info->geometry,geometry);
          status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
          if (status == MagickFalse)
            break;
          width=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
          height=(size_t) floor(metrics.height+draw_info->stroke_width+0.5);
          if ((image->columns != 0) && (image->rows != 0))
            {
              if ((width >= image->columns) && (height >= image->rows))
                break;
            }
          else
            if (((image->columns != 0) && (width >= image->columns)) ||
                ((image->rows != 0) && (height >= image->rows)))
              break;
        }
        if (status == MagickFalse)
          {
            label=DestroyString(label);
            draw_info=DestroyDrawInfo(draw_info);
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        high=draw_info->pointsize;
        for (low=1.0; (high-low) > 0.5; )
        {
          draw_info->pointsize=(low+high)/2.0;
          (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
            -metrics.bounds.x1,metrics.ascent);
          if (draw_info->gravity == UndefinedGravity)
            (void) CloneString(&draw_info->geometry,geometry);
          status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
          if (status == MagickFalse)
            break;
          width=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
          height=(size_t) floor(metrics.height+draw_info->stroke_width+0.5);
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
        if (status != MagickFalse)
          {
            draw_info->pointsize=(low+high)/2.0-0.5;
            status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
          }
      }
   label=DestroyString(label);
   if (status == MagickFalse)
     {
       draw_info=DestroyDrawInfo(draw_info);
       image=DestroyImageList(image);
       return((Image *) NULL);
     }
  if (image->columns == 0)
    image->columns=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
  if (image->columns == 0)
    image->columns=(size_t) floor(draw_info->pointsize+draw_info->stroke_width+
      0.5);
  if (image->rows == 0)
    image->rows=(size_t) floor(metrics.ascent-metrics.descent+
      draw_info->stroke_width+0.5);
  if (image->rows == 0)
    image->rows=(size_t) floor(draw_info->pointsize+draw_info->stroke_width+
      0.5);
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      draw_info=DestroyDrawInfo(draw_info);
      return(DestroyImageList(image));
    }
  if (SetImageBackgroundColor(image,exception) == MagickFalse)
    {
      draw_info=DestroyDrawInfo(draw_info);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Draw label.
  */
  (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
    draw_info->direction == RightToLeftDirection ? (double) image->columns-
    metrics.bounds.x2 : 0.0,draw_info->gravity == UndefinedGravity ?
    metrics.ascent : 0.0);
  (void) CloneString(&draw_info->geometry,geometry);
  status=AnnotateImage(image,draw_info,exception);
  if (image_info->pointsize == 0.0)
    {
      char
        pointsize[MagickPathExtent];

      (void) FormatLocaleString(pointsize,MagickPathExtent,"%.20g",
        draw_info->pointsize);
      (void) SetImageProperty(image,"label:pointsize",pointsize,exception);
    }
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
%   R e g i s t e r L A B E L I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterLABELImage() adds properties for the LABEL image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterLABELImage method is:
%
%      size_t RegisterLABELImage(void)
%
*/
ModuleExport size_t RegisterLABELImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("LABEL","LABEL","Image label");
  entry->decoder=(DecodeImageHandler *) ReadLABELImage;
  entry->flags^=CoderAdjoinFlag;
  entry->format_type=ImplicitFormatType;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r L A B E L I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterLABELImage() removes format registrations made by the
%  LABEL module from the list of supported formats.
%
%  The format of the UnregisterLABELImage method is:
%
%      UnregisterLABELImage(void)
%
*/
ModuleExport void UnregisterLABELImage(void)
{
  (void) UnregisterMagickInfo("LABEL");
}

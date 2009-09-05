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
%                                John Cristy                                  %
%                                 July 1992                                   %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/annotate.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/utility.h"

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
    geometry[MaxTextExtent],
    *property;

  const char
    *label;

  DrawInfo
    *draw_info;

  Image
    *image;

  MagickBooleanType
    status;

  TypeMetric
    metrics;

  unsigned long
    height,
    width;

  /*
    Initialize Image structure.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  (void) ResetImagePage(image,"0x0+0+0");
  property=InterpretImageProperties(image_info,image,image_info->filename);
  (void) SetImageProperty(image,"label",property);
  property=DestroyString(property);
  label=GetImageProperty(image,"label");
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  draw_info->text=ConstantString(label);
  if (((image->columns != 0) || (image->rows != 0)) &&
      (image_info->pointsize == 0.0))
    {
      /*
        Fit label to canvas size.
      */
      status=GetMultilineTypeMetrics(image,draw_info,&metrics);
      for ( ; status != MagickFalse; draw_info->pointsize*=2.0)
      {
        width=(unsigned long) (metrics.width+draw_info->stroke_width+0.5);
        height=(unsigned long) (metrics.height+draw_info->stroke_width+0.5);
        if (((image->columns != 0) && (width > (image->columns+1))) ||
            ((image->rows != 0) && (height > (image->rows+1))))
          break;
        status=GetMultilineTypeMetrics(image,draw_info,&metrics);
      }
      for ( ; status != MagickFalse; draw_info->pointsize--)
      {
        width=(unsigned long) (metrics.width+draw_info->stroke_width+0.5);
        height=(unsigned long) (metrics.height+draw_info->stroke_width+0.5);
        if ((image->columns != 0) && (width <= (image->columns+1)) &&
           ((image->rows == 0) || (height <= (image->rows+1))))
          break;
        if ((image->rows != 0) && (height <= (image->rows+1)) &&
           ((image->columns == 0) || (width <= (image->columns+1))))
          break;
        if (draw_info->pointsize < 2.0)
          break;
        status=GetMultilineTypeMetrics(image,draw_info,&metrics);
      }
    }
  status=GetMultilineTypeMetrics(image,draw_info,&metrics);
  if (status == MagickFalse)
    {
      InheritException(exception,&image->exception);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if (image->columns == 0)
    image->columns=(unsigned long) (metrics.width+draw_info->stroke_width+1.5);
  if (image->columns == 0)
    image->columns=(unsigned long) (draw_info->pointsize+
      draw_info->stroke_width+1.5);
  if (draw_info->gravity == UndefinedGravity)
    {
      (void) FormatMagickString(geometry,MaxTextExtent,"%+g%+g",
        -metrics.bounds.x1+draw_info->stroke_width/2.0,metrics.ascent+
        draw_info->stroke_width/2.0);
      draw_info->geometry=AcquireString(geometry);
    }
  if (image->rows == 0)
    image->rows=(unsigned long) (metrics.height+draw_info->stroke_width+0.5);
  if (image->rows == 0)
    image->rows=(unsigned long) (draw_info->pointsize+draw_info->stroke_width+
      0.5);
  (void) SetImageBackgroundColor(image);
  (void) AnnotateImage(image,draw_info);
  draw_info=DestroyDrawInfo(draw_info);
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
%      unsigned long RegisterLABELImage(void)
%
*/
ModuleExport unsigned long RegisterLABELImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("LABEL");
  entry->decoder=(DecodeImageHandler *) ReadLABELImage;
  entry->adjoin=MagickFalse;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Image label");
  entry->module=ConstantString("LABEL");
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

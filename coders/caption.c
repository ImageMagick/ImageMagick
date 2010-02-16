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
%                                John Cristy                                  %
%                               February 2002                                 %
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
#include "magick/option.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

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
static Image *ReadCAPTIONImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    *caption,
    geometry[MaxTextExtent],
    *property;

  const char
    *gravity;

  DrawInfo
    *draw_info;

  Image
    *image;

  MagickBooleanType
    status;

  register long
    i;

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
  if (image->columns == 0)
    ThrowReaderException(OptionError,"MustSpecifyImageSize");
  (void) ResetImagePage(image,"0x0+0+0");
  /*
    Format caption.
  */
  property=InterpretImageProperties(image_info,image,image_info->filename);
  (void) SetImageProperty(image,"caption",property);
  property=DestroyString(property);
  caption=ConstantString(GetImageProperty(image,"caption"));
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  draw_info->text=ConstantString(caption);
  gravity=GetImageOption(image_info,"gravity");
  if (gravity != (char *) NULL)
    draw_info->gravity=(GravityType) ParseMagickOption(MagickGravityOptions,
      MagickFalse,gravity);
  if ((*caption != '\0') && (image->rows != 0) &&
      (image_info->pointsize == 0.0))
    {
      char
        *text;

      /*
        Scale text to fit bounding box.
      */
      for ( ; ; )
      {
        text=AcquireString(caption);
        i=FormatMagickCaption(image,draw_info,&metrics,&text);
        (void) CloneString(&draw_info->text,text);
        text=DestroyString(text);
        (void) FormatMagickString(geometry,MaxTextExtent,"%+g%+g",
          -metrics.bounds.x1,metrics.ascent);
        if (draw_info->gravity == UndefinedGravity)
          (void) CloneString(&draw_info->geometry,geometry);
        status=GetMultilineTypeMetrics(image,draw_info,&metrics);
        width=(unsigned long) (metrics.width+draw_info->stroke_width+0.5);
        height=(unsigned long) (metrics.height+draw_info->stroke_width+0.5);
        if ((width > (image->columns+1)) || (height > (image->rows+1)))
          break;
        draw_info->pointsize*=2.0;
      }
      draw_info->pointsize/=2.0;
      for ( ; ; )
      {
        text=AcquireString(caption);
        i=FormatMagickCaption(image,draw_info,&metrics,&text);
        (void) CloneString(&draw_info->text,text);
        text=DestroyString(text);
        (void) FormatMagickString(geometry,MaxTextExtent,"%+g%+g",
          -metrics.bounds.x1,metrics.ascent);
        if (draw_info->gravity == UndefinedGravity)
          (void) CloneString(&draw_info->geometry,geometry);
        status=GetMultilineTypeMetrics(image,draw_info,&metrics);
        width=(unsigned long) (metrics.width+draw_info->stroke_width+0.5);
        height=(unsigned long) (metrics.height+draw_info->stroke_width+0.5);
        if ((width > (image->columns+1)) || (height > (image->rows+1)))
          break;
        draw_info->pointsize++;
      }
      draw_info->pointsize--;
    }
  i=FormatMagickCaption(image,draw_info,&metrics,&caption);
  if (image->rows == 0)
    image->rows=(unsigned long) ((i+1)*(metrics.ascent-metrics.descent+
      draw_info->stroke_width)+0.5);
  if (image->rows == 0)
    image->rows=(unsigned long) ((i+1)*draw_info->pointsize+
      draw_info->stroke_width+0.5);
  if (SetImageBackgroundColor(image) == MagickFalse)
    {
      InheritException(exception,&image->exception);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Draw caption.
  */
  (void) CloneString(&draw_info->text,caption);
  status=GetMultilineTypeMetrics(image,draw_info,&metrics);
  if (draw_info->gravity != UndefinedGravity)
    image->page.x=(long) (metrics.bounds.x1-draw_info->stroke_width/2.0);
  else
    {
      (void) FormatMagickString(geometry,MaxTextExtent,"%+g%+g",
        -metrics.bounds.x1+draw_info->stroke_width/2.0,metrics.ascent+
        draw_info->stroke_width/2.0);
      (void) CloneString(&draw_info->geometry,geometry);
    }
  (void) AnnotateImage(image,draw_info);
  draw_info=DestroyDrawInfo(draw_info);
  caption=DestroyString(caption);
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
%      unsigned long RegisterCAPTIONImage(void)
%
*/
ModuleExport unsigned long RegisterCAPTIONImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("CAPTION");
  entry->decoder=(DecodeImageHandler *) ReadCAPTIONImage;
  entry->adjoin=MagickFalse;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Image caption");
  entry->module=ConstantString("CAPTION");
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     PPPP    AAA   N   N   GGGG   OOO                        %
%                     P   P  A   A  NN  N  G      O   O                       %
%                     PPPP   AAAAA  N N N  G GGG  O   O                       %
%                     P   M  A   A  N  NN  G   G  O   O                       %
%                     P      A   A  N   N   GGGG   OOO                        %
%                                                                             %
%                                                                             %
%                     Read Pango Markup Language Format                       %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 March 2012                                  %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/annotate.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/composite-private.h"
#include "magick/draw.h"
#include "magick/draw-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/module.h"
#include "magick/memory_.h"
#include "magick/option.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#if defined(MAGICKCORE_PANGOFT2_DELEGATE)
#include <pango/pango.h>
#include <pango/pangoft2.h>
#endif

#if defined(MAGICKCORE_PANGOFT2_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P A N G O I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPANGOImage() reads an image in the Pango Markup Language Format.
%
%  The format of the ReadPANGOImage method is:
%
%      Image *ReadPANGOImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadPANGOImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    *caption,
    *property;

  DrawInfo
    *draw_info;

  FT_Bitmap
    *canvas;

  Image
    *image;

  PangoContext
    *context;

  PangoFontDescription
    *description;

  PangoFontMap
    *fontmap;

  PangoLayout
    *layout;

  PangoRectangle
    extent;

  PixelPacket
    fill_color;

  register PixelPacket
    *q;

  register unsigned char
    *p;

  ssize_t
    y;

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
  /*
    Get context.
  */
  fontmap=(PangoFontMap *) pango_ft2_font_map_new();
  pango_ft2_font_map_set_resolution((PangoFT2FontMap *) fontmap,
    image->x_resolution,image->y_resolution);
  pango_ft2_font_map_set_default_substitute((PangoFT2FontMap *) fontmap,NULL,
    NULL,NULL);
  context=pango_font_map_create_context(fontmap);
  /*
    Render caption.
  */
  layout=pango_layout_new(context);
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  description=pango_font_description_from_string(draw_info->font ==
    (char *) NULL ? "helvetica" : draw_info->font);
  pango_font_description_set_size(description,PANGO_SCALE*
    draw_info->pointsize);
  pango_layout_set_font_description(layout,description);
  pango_font_description_free(description);
  property=InterpretImageProperties(image_info,image,image_info->filename);
  (void) SetImageProperty(image,"caption",property);
  property=DestroyString(property);
  caption=ConstantString(GetImageProperty(image,"caption"));
  pango_layout_set_text(layout,caption,-1);
  pango_layout_context_changed(layout);
  if (image->columns != 0)
    pango_layout_set_width(layout,(PANGO_SCALE*image->columns*
      image->x_resolution+36.0)/72.0);
  else
    {
      pango_layout_get_pixel_extents(layout,NULL,&extent);
      image->columns=extent.x+extent.width;
    }
  if (image->rows != 0)
    pango_layout_set_height(layout,(PANGO_SCALE*image->columns*
      image->x_resolution+36.0)/72.0);
  else
    {
      pango_layout_get_pixel_extents(layout,NULL,&extent);
      image->rows=extent.y+extent.height;
    }
  /*
    Create canvas.
  */
  canvas=(FT_Bitmap *) AcquireMagickMemory(sizeof(*canvas));
  if (canvas == (FT_Bitmap *) NULL)
    {
      draw_info=DestroyDrawInfo(draw_info);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  canvas->width=image->columns;
  canvas->pitch=(canvas->width+3) & ~3;
  canvas->rows=image->rows;
  canvas->buffer=(unsigned char *) AcquireQuantumMemory(canvas->pitch,
    canvas->rows*sizeof(*canvas->buffer));
  if (canvas->buffer == (unsigned char *) NULL)
    {
      draw_info=DestroyDrawInfo(draw_info);
      canvas=(FT_Bitmap *) RelinquishMagickMemory(canvas);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  canvas->num_grays=256;
  canvas->pixel_mode=ft_pixel_mode_grays;
  ResetMagickMemory(canvas->buffer,0x00,canvas->pitch*canvas->rows);
  pango_ft2_render_layout(canvas,layout,0,0);
  /*
    Convert caption to image.
  */
  if (SetImageBackgroundColor(image) == MagickFalse)
    {
      draw_info=DestroyDrawInfo(draw_info);
      canvas->buffer=(unsigned char *) RelinquishMagickMemory(canvas->buffer);
      canvas=(FT_Bitmap *) RelinquishMagickMemory(canvas);
      caption=DestroyString(caption);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  p=canvas->buffer;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickRealType
        fill_opacity;

      (void) GetFillColor(draw_info,x,y,&fill_color);
      fill_opacity=QuantumRange-(*p)/canvas->num_grays*(QuantumRange-
        fill_color.opacity);
      if (draw_info->text_antialias == MagickFalse)
        fill_opacity=fill_opacity >= 0.5 ? 1.0 : 0.0;
      MagickCompositeOver(&fill_color,fill_opacity,q,q->opacity,q);
      p++;
      q++;
    }
  }
  /*
    Relinquish resources.
  */
  draw_info=DestroyDrawInfo(draw_info);
  canvas->buffer=(unsigned char *) RelinquishMagickMemory(canvas->buffer);
  canvas=(FT_Bitmap *) RelinquishMagickMemory(canvas);
  caption=DestroyString(caption);
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P A N G O I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPANGOImage() adds attributes for the Pango Markup Language format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPANGOImage method is:
%
%      size_t RegisterPANGOImage(void)
%
*/
ModuleExport size_t RegisterPANGOImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("PANGO");
#if defined(MAGICKCORE_PANGOFT2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPANGOImage;
#endif
  entry->description=ConstantString("Pango Markup Language");
  entry->adjoin=MagickFalse;
  entry->module=ConstantString("PANGO");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P A N G O I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPANGOImage() removes format registrations made by the Pango module
%  from the list of supported formats.
%
%  The format of the UnregisterPANGOImage method is:
%
%      UnregisterPANGOImage(void)
%
*/
ModuleExport void UnregisterPANGOImage(void)
{
  (void) UnregisterMagickInfo("PANGO");
}

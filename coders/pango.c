/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     PPPP    AAA   N   N   GGGG   OOO                        %
%                     P   P  A   A  NN  N  G      O   O                       %
%                     PPPP   AAAAA  N N N  G GGG  O   O                       %
%                     P      A   A  N  NN  G   G  O   O                       %
%                     P      A   A  N   N   GGGG   OOO                        %
%                                                                             %
%                                                                             %
%                     Read Pango Markup Language Format                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 March 2012                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#if defined(MAGICKCORE_PANGOCAIRO_DELEGATE)
#include <pango/pango.h>
#include <pango/pangocairo.h>
#include <pango/pango-features.h>
#endif

/*
  Define declarations.
*/
#define DefaultSVGDensity  96.0

#if defined(MAGICKCORE_PANGOCAIRO_DELEGATE)
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
  cairo_font_options_t
    *font_options;

  cairo_surface_t
    *surface;

  char
    *caption,
    *property;

  cairo_t
    *cairo_image;

  const char
    *option;

  DrawInfo
    *draw_info;

  Image
    *image;

  MagickBooleanType
    status;

  MemoryInfo
    *pixel_info;

  PangoAlignment
    align;

  PangoContext
    *context;

  PangoFontDescription
    *description;

  PangoFontMap
    *fontmap;

  PangoGravity
    gravity;

  PangoLayout
    *layout;

  PangoRectangle
    extent;

  PixelInfo
    fill_color;

  RectangleInfo
    page;

  register unsigned char
    *p;

  size_t
    stride;

  ssize_t
    y;

  unsigned char
    *pixels;

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
  /*
    Format caption.
  */
  option=GetImageOption(image_info,"filename");
  if (option == (const char *) NULL)
    property=InterpretImageProperties((ImageInfo *) image_info,image,
      image_info->filename,exception);
  else
    if (LocaleNCompare(option,"pango:",6) == 0)
      property=InterpretImageProperties((ImageInfo *) image_info,image,option+6,
        exception);
    else
      property=InterpretImageProperties((ImageInfo *) image_info,image,option,
        exception);
  if (property != (char *) NULL)
    {
      (void) SetImageProperty(image,"caption",property,exception);
      property=DestroyString(property);
    }
  caption=ConstantString(GetImageProperty(image,"caption",exception));
  /*
    Get context.
  */
  fontmap=pango_cairo_font_map_new();
  pango_cairo_font_map_set_resolution(PANGO_CAIRO_FONT_MAP(fontmap),
    image->resolution.x == 0.0 ? DefaultSVGDensity : image->resolution.x);
  font_options=cairo_font_options_create();
  option=GetImageOption(image_info,"pango:hinting");
  if (option != (const char *) NULL)
    {
      if (LocaleCompare(option,"none") != 0)
        cairo_font_options_set_hint_style(font_options,CAIRO_HINT_STYLE_NONE);
      if (LocaleCompare(option,"full") != 0)
        cairo_font_options_set_hint_style(font_options,CAIRO_HINT_STYLE_FULL);
    }
  context=pango_font_map_create_context(fontmap);
  pango_cairo_context_set_font_options(context,font_options);
  cairo_font_options_destroy(font_options);
  option=GetImageOption(image_info,"pango:language");
  if (option != (const char *) NULL)
    pango_context_set_language(context,pango_language_from_string(option));
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  pango_context_set_base_dir(context,draw_info->direction ==
    RightToLeftDirection ? PANGO_DIRECTION_RTL : PANGO_DIRECTION_LTR);
  switch (draw_info->gravity)
  {
    case NorthGravity:
    {
      gravity=PANGO_GRAVITY_NORTH;
      break;
    }
    case NorthWestGravity:
    case WestGravity:
    case SouthWestGravity:
    {
      gravity=PANGO_GRAVITY_WEST;
      break;
    }
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
    {
      gravity=PANGO_GRAVITY_EAST;
      break;
    }
    case SouthGravity:
    {
      gravity=PANGO_GRAVITY_SOUTH;
      break;
    }
    default:
    {
      gravity=PANGO_GRAVITY_AUTO;
      break;
    }
  }
  pango_context_set_base_gravity(context,gravity);
  option=GetImageOption(image_info,"pango:gravity-hint");
  if (option != (const char *) NULL)
    {
      if (LocaleCompare(option,"line") == 0)
        pango_context_set_gravity_hint(context,PANGO_GRAVITY_HINT_LINE);
      if (LocaleCompare(option,"natural") == 0)
        pango_context_set_gravity_hint(context,PANGO_GRAVITY_HINT_NATURAL);
      if (LocaleCompare(option,"strong") == 0)
        pango_context_set_gravity_hint(context,PANGO_GRAVITY_HINT_STRONG);
    }
  /*
    Configure layout.
  */
  layout=pango_layout_new(context);
  option=GetImageOption(image_info,"pango:auto-dir");
  if (option != (const char *) NULL)
    pango_layout_set_auto_dir(layout,1);
  option=GetImageOption(image_info,"pango:ellipsize");
  if (option != (const char *) NULL)
    {
      if (LocaleCompare(option,"end") == 0)
        pango_layout_set_ellipsize(layout,PANGO_ELLIPSIZE_END);
      if (LocaleCompare(option,"middle") == 0)
        pango_layout_set_ellipsize(layout,PANGO_ELLIPSIZE_MIDDLE);
      if (LocaleCompare(option,"none") == 0)
        pango_layout_set_ellipsize(layout,PANGO_ELLIPSIZE_NONE);
      if (LocaleCompare(option,"start") == 0)
        pango_layout_set_ellipsize(layout,PANGO_ELLIPSIZE_START);
    }
  option=GetImageOption(image_info,"pango:justify");
  if (IsStringTrue(option) != MagickFalse)
    pango_layout_set_justify(layout,1);
  option=GetImageOption(image_info,"pango:single-paragraph");
  if (IsStringTrue(option) != MagickFalse)
    pango_layout_set_single_paragraph_mode(layout,1);
  option=GetImageOption(image_info,"pango:wrap");
  if (option != (const char *) NULL)
    {
      if (LocaleCompare(option,"char") == 0)
        pango_layout_set_wrap(layout,PANGO_WRAP_CHAR);
      if (LocaleCompare(option,"word") == 0)
        pango_layout_set_wrap(layout,PANGO_WRAP_WORD);
      if (LocaleCompare(option,"word-char") == 0)
        pango_layout_set_wrap(layout,PANGO_WRAP_WORD_CHAR);
    }
  option=GetImageOption(image_info,"pango:indent");
  if (option != (const char *) NULL)
    pango_layout_set_indent(layout,(int) ((StringToLong(option)*
      (image->resolution.x == 0.0 ? DefaultSVGDensity : image->resolution.x)*
      PANGO_SCALE+DefaultSVGDensity/2)/DefaultSVGDensity+0.5));
  switch (draw_info->align)
  {
    case CenterAlign: align=PANGO_ALIGN_CENTER; break;
    case RightAlign: align=PANGO_ALIGN_RIGHT; break;
    case LeftAlign: align=PANGO_ALIGN_LEFT; break;
    default:
    {
      if (draw_info->gravity == CenterGravity)
        {
          align=PANGO_ALIGN_CENTER;
          break;
        }
      align=PANGO_ALIGN_LEFT;
      break;
    }
  }
  if ((align != PANGO_ALIGN_CENTER) &&
      (draw_info->direction == RightToLeftDirection))
    align=(PangoAlignment) (PANGO_ALIGN_LEFT+PANGO_ALIGN_RIGHT-align);
  option=GetImageOption(image_info,"pango:align");
  if (option != (const char *) NULL)
    {
      if (LocaleCompare(option,"center") == 0)
        align=PANGO_ALIGN_CENTER;
      if (LocaleCompare(option,"left") == 0)
        align=PANGO_ALIGN_LEFT;
      if (LocaleCompare(option,"right") == 0)
        align=PANGO_ALIGN_RIGHT;
    }
  pango_layout_set_alignment(layout,align);
  if (draw_info->font == (char *) NULL)
    description=pango_font_description_new();
  else
    description=pango_font_description_from_string(draw_info->font);
  pango_font_description_set_size(description,(int) (PANGO_SCALE*
    draw_info->pointsize+0.5));
  pango_layout_set_font_description(layout,description);
  pango_font_description_free(description);
  option=GetImageOption(image_info,"pango:markup");
  if ((option != (const char *) NULL) && (IsStringTrue(option) == MagickFalse))
    pango_layout_set_text(layout,caption,-1);
  else
    {
      GError
        *error;

      error=(GError *) NULL;
      if (pango_parse_markup(caption,-1,0,NULL,NULL,NULL,&error) == 0)
        (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
          error->message,"`%s'",image_info->filename);
      pango_layout_set_markup(layout,caption,-1);
    }
  pango_layout_context_changed(layout);
  page.x=0;
  page.y=0;
  if (image_info->page != (char *) NULL)
    (void) ParseAbsoluteGeometry(image_info->page,&page);
  if (image->columns == 0)
    {
      pango_layout_get_extents(layout,NULL,&extent);
      image->columns=(extent.x+extent.width+PANGO_SCALE/2)/PANGO_SCALE+2*page.x;
    }
  else
    {
      image->columns-=2*page.x;
      pango_layout_set_width(layout,(int) ((PANGO_SCALE*image->columns*
        (image->resolution.x == 0.0 ? DefaultSVGDensity : image->resolution.x)+
        DefaultSVGDensity/2)/DefaultSVGDensity+0.5));
    }
  if (image->rows == 0)
    {
      pango_layout_get_extents(layout,NULL,&extent);
      image->rows=(extent.y+extent.height+PANGO_SCALE/2)/PANGO_SCALE+2*page.y;
    }
  else
    {
      image->rows-=2*page.y;
      pango_layout_set_height(layout,(int) ((PANGO_SCALE*image->rows*
        (image->resolution.y == 0.0 ? DefaultSVGDensity : image->resolution.y)+
        DefaultSVGDensity/2)/DefaultSVGDensity+0.5));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  /*
    Render markup.
  */
  stride=(size_t) cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,
    (int) image->columns);
  pixel_info=AcquireVirtualMemory(image->rows,stride*sizeof(*pixels));
  if (pixel_info == (MemoryInfo *) NULL)
    {
      draw_info=DestroyDrawInfo(draw_info);
      caption=DestroyString(caption);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
  surface=cairo_image_surface_create_for_data(pixels,CAIRO_FORMAT_ARGB32,
    (int) image->columns,(int) image->rows,(int) stride);
  cairo_image=cairo_create(surface);
  cairo_set_operator(cairo_image,CAIRO_OPERATOR_CLEAR);
  cairo_paint(cairo_image);
  cairo_set_operator(cairo_image,CAIRO_OPERATOR_OVER);
  cairo_translate(cairo_image,page.x,page.y);
  cairo_set_source_rgba(cairo_image,QuantumScale*draw_info->fill.red,
    QuantumScale*draw_info->fill.green,QuantumScale*draw_info->fill.blue,
    QuantumScale*draw_info->fill.alpha);
  pango_cairo_show_layout(cairo_image,layout);
  cairo_destroy(cairo_image);
  cairo_surface_destroy(surface);
  g_object_unref(layout);
  g_object_unref(fontmap);
  /*
    Convert surface to image.
  */
  (void) SetImageBackgroundColor(image,exception);
  p=pixels;
  GetPixelInfo(image,&fill_color);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *q;

    register ssize_t
      x;

    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        gamma;

      fill_color.blue=(double) ScaleCharToQuantum(*p++);
      fill_color.green=(double) ScaleCharToQuantum(*p++);
      fill_color.red=(double) ScaleCharToQuantum(*p++);
      fill_color.alpha=(double) ScaleCharToQuantum(*p++);
      /*
        Disassociate alpha.
      */
      gamma=QuantumScale*fill_color.alpha;
      gamma=PerceptibleReciprocal(gamma);
      fill_color.blue*=gamma;
      fill_color.green*=gamma;
      fill_color.red*=gamma;
      CompositePixelOver(image,&fill_color,fill_color.alpha,q,(double)
        GetPixelAlpha(image,q),q);
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    if (image->previous == (Image *) NULL)
      {
        status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
        image->rows);
        if (status == MagickFalse)
          break;
      }
  }
  /*
    Relinquish resources.
  */
  pixel_info=RelinquishVirtualMemory(pixel_info);
  draw_info=DestroyDrawInfo(draw_info);
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
  char
    version[MagickPathExtent];

  MagickInfo
    *entry;

  *version='\0';
#if defined(PANGO_VERSION_STRING)
  (void) FormatLocaleString(version,MagickPathExtent,"Pangocairo %s",
    PANGO_VERSION_STRING);
#endif
  entry=AcquireMagickInfo("PANGO","PANGO","Pango Markup Language");
#if defined(MAGICKCORE_PANGOCAIRO_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPANGOImage;
#endif
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->flags^=CoderAdjoinFlag;
  entry->flags^=CoderDecoderThreadSupportFlag;
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

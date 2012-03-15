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
%  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/studio.h"
#include "MagickCore/annotate.h"
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
#include "MagickCore/module.h"
#include "MagickCore/memory_.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/utility.h"
#if defined(MAGICKCORE_PANGOFT2_DELEGATE)
#include <pango/pango.h>
#include <pango/pangoft2.h>
#include <pango/pango-features.h>
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

static void PangoSubstitute(FcPattern *pattern,void *context)
{
  const char
    *option;

  option=(const char *) context;
  if (option == (const char *) NULL)
    return;
  FcPatternDel(pattern,FC_HINTING);
  FcPatternAddBool(pattern, FC_HINTING,LocaleCompare(option,"none") != 0);
  FcPatternDel(pattern,FC_AUTOHINT);
  FcPatternAddBool(pattern,FC_AUTOHINT,LocaleCompare(option,"auto") == 0);
}

static Image *ReadPANGOImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    *caption,
    *property;

  const char
    *option;

  DrawInfo
    *draw_info;

  FT_Bitmap
    *canvas;

  Image
    *image;

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

  register Quantum
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
  image=AcquireImage(image_info,exception);
  (void) ResetImagePage(image,"0x0+0+0");
  /*
    Get context.
  */
  fontmap=(PangoFontMap *) pango_ft2_font_map_new();
  pango_ft2_font_map_set_resolution((PangoFT2FontMap *) fontmap,
    image->resolution.x,image->resolution.y);
  option=GetImageOption(image_info,"pango:hinting");
  pango_ft2_font_map_set_default_substitute((PangoFT2FontMap *) fontmap,
    PangoSubstitute,(char *) option,NULL);
  context=pango_font_map_create_context(fontmap);
  option=GetImageOption(image_info,"pango:language");
  if (option != (const char *) NULL)
    pango_context_set_language(context,pango_language_from_string(option));
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  pango_context_set_base_dir(context,draw_info->direction ==
    RightToLeftDirection ? PANGO_DIRECTION_RTL : PANGO_DIRECTION_LTR);
  switch (draw_info->gravity)
  {
    case NorthGravity: gravity=PANGO_GRAVITY_NORTH; break;
    case WestGravity: gravity=PANGO_GRAVITY_WEST; break;
    case EastGravity: gravity=PANGO_GRAVITY_EAST; break;
    case SouthGravity: gravity=PANGO_GRAVITY_SOUTH; break;
    default: gravity=PANGO_GRAVITY_AUTO; break;
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
  if ((option != (const char *) NULL) && (IsMagickTrue(option) != MagickFalse))
    pango_layout_set_justify(layout,1);
  option=GetImageOption(image_info,"pango:single-paragraph");
  if ((option != (const char *) NULL) && (IsMagickTrue(option) != MagickFalse))
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
    pango_layout_set_indent(layout,(StringToLong(option)*image->resolution.x*
      PANGO_SCALE+36)/72);
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
  pango_layout_set_alignment(layout,align);
  description=pango_font_description_from_string(draw_info->font ==
    (char *) NULL ? "helvetica" : draw_info->font);
  pango_font_description_set_size(description,PANGO_SCALE*draw_info->pointsize);
  pango_layout_set_font_description(layout,description);
  pango_font_description_free(description);
  property=InterpretImageProperties(image_info,image,image_info->filename,
    exception);
  (void) SetImageProperty(image,"caption",property,exception);
  property=DestroyString(property);
  caption=ConstantString(GetImageProperty(image,"caption",exception));
  /*
    Render caption.
  */
  option=GetImageOption(image_info,"pango:markup");
  if ((option != (const char *) NULL) && (IsMagickTrue(option) != MagickFalse))
    pango_layout_set_markup(layout,caption,-1);
  else
    pango_layout_set_text(layout,caption,-1);
  pango_layout_context_changed(layout);
  page.x=0;
  page.y=0;
  if (image_info->page != (char *) NULL)
    (void) ParseAbsoluteGeometry(image_info->page,&page);
  if (image->columns == 0)
    {
      pango_layout_get_pixel_extents(layout,NULL,&extent);
      image->columns=extent.x+extent.width;
    }
  else
    {
      image->columns-=2*page.x;
      pango_layout_set_width(layout,(PANGO_SCALE*image->columns*
        image->resolution.x+36.0)/72.0);
    }
  if (image->rows == 0)
    {
      pango_layout_get_pixel_extents(layout,NULL,&extent);
      image->rows=extent.y+extent.height;
    }
  else
    {
      image->rows-=2*page.y;
      pango_layout_set_height(layout,(PANGO_SCALE*image->rows*
        image->resolution.y+36.0)/72.0);
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
  image->columns+=2*page.x;
  image->rows+=2*page.y;
  if (SetImageBackgroundColor(image,exception) == MagickFalse)
    {
      draw_info=DestroyDrawInfo(draw_info);
      canvas->buffer=(unsigned char *) RelinquishMagickMemory(canvas->buffer);
      canvas=(FT_Bitmap *) RelinquishMagickMemory(canvas);
      caption=DestroyString(caption);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  GetPixelInfo(image,&fill_color);
  p=canvas->buffer;
  for (y=page.y; y < (ssize_t) (image->rows-page.y); y++)
  {
    register ssize_t
      x;

    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    q+=page.x*GetPixelChannels(image);
    for (x=page.x; x < (ssize_t) (image->columns-page.x); x++)
    {
      MagickRealType
        fill_alpha;

      (void) GetFillColor(draw_info,x,y,&fill_color,exception);
      fill_alpha=(MagickRealType) (*p)/(canvas->num_grays-1);
      if (draw_info->text_antialias == MagickFalse)
        fill_alpha=fill_alpha >= 0.5 ? 1.0 : 0.0;
      fill_alpha=fill_alpha*fill_color.alpha;
      CompositePixelOver(image,&fill_color,fill_alpha,q,GetPixelAlpha(image,q),
        q);
      p++;
      q+=GetPixelChannels(image);
    }
    for ( ; x < (ssize_t) ((canvas->width+3) & ~3); x++)
      p++;
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
  char
    version[MaxTextExtent];

  MagickInfo
    *entry;

  *version='\0';
#if defined(PANGO_VERSION_STRING)
  (void) FormatLocaleString(version,MaxTextExtent,"(Pangoft2 %s)",
    PANGO_VERSION_STRING);
#endif
  entry=SetMagickInfo("PANGO");
#if defined(MAGICKCORE_PANGOFT2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPANGOImage;
#endif
  entry->description=ConstantString("Pango Markup Language");
  if (*version != '\0')
    entry->version=ConstantString(version);
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

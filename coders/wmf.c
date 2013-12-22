/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             W   W   M   M  FFFFF                            %
%                             W   W   MM MM  F                                %
%                             W W W   M M M  FFF                              %
%                             WW WW   M   M  F                                %
%                             W   W   M   M  F                                %
%                                                                             %
%                                                                             %
%                        Read Windows Metafile Format                         %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                               December 2000                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/paint.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/type.h"
#include "magick/module.h"
#include "wand/MagickWand.h"

#if defined(__CYGWIN__)
#undef MAGICKCORE_WMF_DELEGATE
#endif

#if defined(MAGICKCORE_WMF_DELEGATE)
#include "libwmf/api.h"
#include "libwmf/eps.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d W M F I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadWMFImage() reads an Windows Metafile image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadWMFImage method is:
%
%      Image *ReadWMFImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static int WMFReadBlob(void *image)
{
  return(ReadBlobByte((Image *) image));
}

static int WMFSeekBlob(void *image,long offset)
{
  return((int) SeekBlob((Image *) image,(MagickOffsetType) offset,SEEK_SET));
}

static long WMFTellBlob(void *image)
{
  return((long) TellBlob((Image*) image));
}

static Image *ReadWMFImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    filename[MaxTextExtent];

  int
    unique_file;

  FILE
    *file;

  Image
    *image;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

  size_t
    flags;

  wmfAPI
    *wmf_info;

  wmfAPI_Options
    options;

  wmfD_Rect
    bounding_box;

  wmf_eps_t
    *eps_info;

  wmf_error_t
    wmf_status;

  /*
    Read WMF image.
  */
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  wmf_info=(wmfAPI *) NULL;
  flags=0;
  flags|=WMF_OPT_IGNORE_NONFATAL;
  flags|=WMF_OPT_FUNCTION;
  options.function=wmf_eps_function;
  wmf_status=wmf_api_create(&wmf_info,(unsigned long) flags,&options);
  if (wmf_status != wmf_E_None)
    {
      if (wmf_info != (wmfAPI *) NULL)
        wmf_api_destroy(wmf_info);
      ThrowReaderException(DelegateError,"UnableToInitializeWMFLibrary");
    }
  wmf_status=wmf_bbuf_input(wmf_info,WMFReadBlob,WMFSeekBlob,WMFTellBlob,
    (void *) image);
  if (wmf_status != wmf_E_None)
    {
      wmf_api_destroy(wmf_info);
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        image->filename);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  wmf_status=wmf_scan(wmf_info,0,&bounding_box);
  if (wmf_status != wmf_E_None)
    {
      wmf_api_destroy(wmf_info);
      ThrowReaderException(DelegateError,"FailedToScanFile");
    }
  eps_info=WMF_EPS_GetData(wmf_info);
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    {
      wmf_api_destroy(wmf_info);
      ThrowReaderException(FileOpenError,"UnableToCreateTemporaryFile");
    }
  eps_info->out=wmf_stream_create(wmf_info,file);
  eps_info->bbox=bounding_box;
  wmf_status=wmf_play(wmf_info,0,&bounding_box);
  if (wmf_status != wmf_E_None)
    {
      wmf_api_destroy(wmf_info);
      ThrowReaderException(DelegateError,"FailedToRenderFile");
    }
  (void) fclose(file);
  wmf_api_destroy(wmf_info);
  (void) CloseBlob(image);
  image=DestroyImage(image);
  /*
    Read EPS image.
  */
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) FormatLocaleString(read_info->filename,MaxTextExtent,"eps:%s",
    filename);
  image=ReadImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  if (image != (Image *) NULL)
    {
      (void) CopyMagickString(image->filename,image_info->filename,
        MaxTextExtent);
      (void) CopyMagickString(image->magick_filename,image_info->filename,
        MaxTextExtent);
      (void) CopyMagickString(image->magick,"WMF",MaxTextExtent);
    }
  (void) RelinquishUniqueFileResource(filename);
  return(GetFirstImageInList(image));
}
#elif defined(MAGICKCORE_WMFLITE_DELEGATE)

#define ERR(API)  ((API)->err != wmf_E_None)
#define XC(x) ((double) x)
#define YC(y) ((double) y)

#if !defined(M_PI)
#  define M_PI  MagickPI
#endif

#if defined(MAGICKCORE_HAVE_FT2BUILD_H)
#  include <ft2build.h>
#endif

#include "libwmf/fund.h"
#include "libwmf/types.h"
#include "libwmf/api.h"
#undef SRCCOPY
#undef SRCPAINT
#undef SRCAND
#undef SRCINVERT
#undef SRCERASE
#undef NOTSRCCOPY
#undef NOTSRCERASE
#undef MERGECOPY
#undef MERGEPAINT
#undef PATCOPY
#undef PATPAINT
#undef PATINVERT
#undef DSTINVERT
#undef BLACKNESS
#undef WHITENESS

/* The following additinal undefs were required for MinGW */
#undef BS_HOLLOW
#undef PS_STYLE_MASK
#undef PS_ENDCAP_ROUND
#undef PS_ENDCAP_SQUARE
#undef PS_ENDCAP_FLAT
#undef PS_ENDCAP_MASK
#undef PS_JOIN_ROUND
#undef PS_JOIN_BEVEL
#undef PS_JOIN_MITER
#undef PS_COSMETIC
#undef PS_GEOMETRIC
#undef PS_TYPE_MASK
#undef STRETCH_ANDSCANS
#undef STRETCH_ORSCANS
#undef STRETCH_DELETESCANS
#undef STRETCH_HALFTONE
#undef ETO_OPAQUE
#undef ETO_CLIPPED
#undef ETO_GLYPH_INDEX
#undef ETO_RTLREADING

#include "libwmf/defs.h"
#include "libwmf/ipa.h"
#include "libwmf/color.h"
#include "libwmf/macro.h"

/* Unit conversions */
#define TWIPS_PER_INCH        1440
#define CENTIMETERS_PER_INCH  2.54
#define POINTS_PER_INCH       72

#if defined(MAGICKCORE_WMFLITE_DELEGATE)
# define wmf_api_create(api,flags,options) wmf_lite_create(api,flags,options)
# define wmf_api_destroy(api) wmf_lite_destroy(api)
# undef WMF_FONT_PSNAME
# define WMF_FONT_PSNAME(F) ((F)->user_data ? ((wmf_magick_font_t*) (F)->user_data)->ps_name : 0)

typedef struct _wmf_magick_font_t wmf_magick_font_t;

struct _wmf_magick_font_t
{
  char*  ps_name;
  double pointsize;
};

#endif

typedef struct _wmf_magick_t wmf_magick_t;

struct _wmf_magick_t
{
  /* Bounding box */
  wmfD_Rect
    bbox;

  /* Scale and translation factors */
  double
    scale_x,
    scale_y,
    translate_x,
    translate_y,
    rotate;

  /* Vector output */
  DrawingWand
    *draw_wand;

  /* ImageMagick image */
  Image
    *image;

  /* ImageInfo */
  const ImageInfo
    *image_info;

  /* DrawInfo */
  DrawInfo
    *draw_info;

  /* Pattern ID */
  unsigned long
    pattern_id;

  /* Clip path flag */
  MagickBooleanType
    clipping;

  /* Clip path ID */
  unsigned long
    clip_mask_id;

  /* Push depth */
  long
    push_depth;
};


#define WMF_MAGICK_GetData(Z) ((wmf_magick_t*)((Z)->device_data))
#define WMF_MAGICK_GetFontData(Z) \
  ((wmf_magick_font_t*)((wmfFontData *)Z->font_data)->user_data)

#define WmfDrawingWand (((wmf_magick_t*)((API)->device_data))->draw_wand)

/* Enum to control whether util_set_brush applies brush to fill or
   stroke. */
typedef enum
{
  BrushApplyFill,
  BrushApplyStroke
} BrushApply;


/* Enum to specify arc type */
typedef enum
{
  magick_arc_ellipse = 0,
  magick_arc_open,
  magick_arc_pie,
  magick_arc_chord
}
magick_arc_t;

#if defined(MAGICKCORE_WMFLITE_DELEGATE)
static void  lite_font_init (wmfAPI* API, wmfAPI_Options* options);
static void  lite_font_map(wmfAPI* API,wmfFont* font);
static float lite_font_stringwidth(wmfAPI* API, wmfFont* font, char* str);
#endif

static void         draw_fill_color_rgb(wmfAPI* API, const wmfRGB* rgb);
static void         draw_stroke_color_rgb(wmfAPI* API, const wmfRGB* rgb);
static void         draw_pattern_push(wmfAPI* API, unsigned long id, unsigned long columns, unsigned long rows);
static int          ipa_blob_read(void* wand);
static int          ipa_blob_seek(void* wand,long position);
static long         ipa_blob_tell(void* wand);
static void         ipa_bmp_draw(wmfAPI * API, wmfBMP_Draw_t * bmp_draw);
static void         ipa_bmp_free(wmfAPI * API, wmfBMP * bmp);
static void         ipa_bmp_read(wmfAPI * API, wmfBMP_Read_t * bmp_read);
static void         ipa_device_begin(wmfAPI * API);
static void         ipa_device_close(wmfAPI * API);
static void         ipa_device_end(wmfAPI * API);
static void         ipa_device_open(wmfAPI * API);
static void         ipa_draw_arc(wmfAPI * API, wmfDrawArc_t * draw_arc);
static void         ipa_draw_chord(wmfAPI * API, wmfDrawArc_t * draw_arc);
static void         ipa_draw_ellipse(wmfAPI * API, wmfDrawArc_t * draw_arc);
static void         ipa_draw_line(wmfAPI * API, wmfDrawLine_t * draw_line);
static void         ipa_draw_pie(wmfAPI * API, wmfDrawArc_t * draw_arc);
static void         ipa_draw_pixel(wmfAPI * API, wmfDrawPixel_t * draw_pixel);
static void         ipa_draw_polygon(wmfAPI * API, wmfPolyLine_t * poly_line);
#if defined(MAGICKCORE_WMFLITE_DELEGATE)
static void         ipa_draw_polypolygon(wmfAPI * API, wmfPolyPoly_t* polypolygon);
#endif
static void         ipa_draw_rectangle(wmfAPI * API, wmfDrawRectangle_t * draw_rect);
static void         ipa_draw_text(wmfAPI * API, wmfDrawText_t * draw_text);
static void         ipa_flood_exterior(wmfAPI * API, wmfFlood_t * flood);
static void         ipa_flood_interior(wmfAPI * API, wmfFlood_t * flood);
static void         ipa_functions(wmfAPI * API);
static void         ipa_poly_line(wmfAPI * API, wmfPolyLine_t * poly_line);
static void         ipa_region_clip(wmfAPI * API, wmfPolyRectangle_t * poly_rect);
static void         ipa_region_frame(wmfAPI * API, wmfPolyRectangle_t * poly_rect);
static void         ipa_region_paint(wmfAPI * API, wmfPolyRectangle_t * poly_rect);
static void         ipa_rop_draw(wmfAPI * API, wmfROP_Draw_t * rop_draw);
static void         ipa_udata_copy(wmfAPI * API, wmfUserData_t * userdata);
static void         ipa_udata_free(wmfAPI * API, wmfUserData_t * userdata);
static void         ipa_udata_init(wmfAPI * API, wmfUserData_t * userdata);
static void         ipa_udata_set(wmfAPI * API, wmfUserData_t * userdata);
static int          magick_progress_callback(void* wand,float quantum);
static void         util_draw_arc(wmfAPI * API, wmfDrawArc_t * draw_arc,magick_arc_t finish);
#if defined(MAGICKCORE_WMFLITE_DELEGATE)
/*static int          util_font_weight( const char* font );*/
#endif
static double       util_pointsize( wmfAPI* API, wmfFont* font, char* str, double font_height);
static void         util_set_brush(wmfAPI * API, wmfDC * dc, const BrushApply brush_apply);
static void         util_set_pen(wmfAPI * API, wmfDC * dc);

/* Progress callback */
int magick_progress_callback(void *context,float quantum)
{
  Image
    *image;

  MagickBooleanType
    status;

  image=(Image *) context;
  assert(image->signature == MagickSignature);
  status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
    GetBlobSize(image));
  return(status != MagickFalse ? 0 : 1);
}

/* Set fill color */
static void draw_fill_color_string(DrawingWand *drawing_wand,const char *color)
{
  PixelWand
    *fill_color;

  fill_color=NewPixelWand();
  PixelSetColor(fill_color,color);
  DrawSetFillColor(drawing_wand,fill_color);
  fill_color=DestroyPixelWand(fill_color);
}
static void draw_fill_color_rgb( wmfAPI* API, const wmfRGB* rgb )
{
  PixelWand
    *fill_color;

  fill_color=NewPixelWand();
  PixelSetRedQuantum(fill_color,ScaleCharToQuantum(rgb->r));
  PixelSetGreenQuantum(fill_color,ScaleCharToQuantum(rgb->g));
  PixelSetBlueQuantum(fill_color,ScaleCharToQuantum(rgb->b));
  PixelSetOpacityQuantum(fill_color,OpaqueOpacity);
  DrawSetFillColor(WmfDrawingWand,fill_color);
  fill_color=DestroyPixelWand(fill_color);
}

/* Set stroke color */
static void draw_stroke_color_string(DrawingWand *drawing_wand,const char *color)
{
  PixelWand
    *stroke_color;

  stroke_color=NewPixelWand();
  PixelSetColor(stroke_color,color);
  DrawSetStrokeColor(drawing_wand,stroke_color);
  stroke_color=DestroyPixelWand(stroke_color);
}

static void draw_stroke_color_rgb( wmfAPI* API, const wmfRGB* rgb )
{
  PixelWand
    *stroke_color;

  stroke_color=NewPixelWand();
  PixelSetRedQuantum(stroke_color,ScaleCharToQuantum(rgb->r));
  PixelSetGreenQuantum(stroke_color,ScaleCharToQuantum(rgb->g));
  PixelSetBlueQuantum(stroke_color,ScaleCharToQuantum(rgb->b));
  PixelSetOpacityQuantum(stroke_color,OpaqueOpacity);
  DrawSetStrokeColor(WmfDrawingWand,stroke_color);
  stroke_color=DestroyPixelWand(stroke_color);
}

/* Set under color */
static void draw_under_color_string(DrawingWand *drawing_wand,const char *color)
{
  PixelWand
    *under_color;

  under_color=NewPixelWand();
  PixelSetColor(under_color,color);
  DrawSetTextUnderColor(drawing_wand,under_color);
  under_color=DestroyPixelWand(under_color);
}

static void draw_pattern_push( wmfAPI* API,
                               unsigned long id,
                               unsigned long columns,
                               unsigned long rows )
{
  char
    pattern_id[30];

  (void) FormatLocaleString(pattern_id,MaxTextExtent,"brush_%lu",id);
  (void) DrawPushPattern(WmfDrawingWand,pattern_id,0,0,columns,rows);
}

/* Pattern/Bit BLT with raster operation (ROP) support.  Invoked by
   META_PATBLT, which is equivalent to Windows PatBlt() call, or by
   META_DIBBITBLT which is equivalent to Windows BitBlt() call. */

/* The BitBlt function transfers pixels from a rectangular area in one
   device wand called the 'source', to a rectangular area of the
   same size in another device wand, called the 'destination'. */

static void ipa_rop_draw(wmfAPI * API, wmfROP_Draw_t * rop_draw)
{
/*   wmfBrush */
/*     *brush = WMF_DC_BRUSH(rop_draw->dc); */

/*   wmfBMP */
/*     *brush_bmp = WMF_BRUSH_BITMAP(brush); */

  if (TO_FILL(rop_draw) == 0)
    return;

  /* Save graphic wand */
  (void) PushDrawingWand(WmfDrawingWand);

  /* FIXME: finish implementing (once we know what it is supposed to do!) */

  /*
  struct _wmfROP_Draw_t
  {       wmfDC* dc;

    wmfD_Coord TL;
    wmfD_Coord BR;

    U32 ROP;

    double pixel_width;
    double pixel_height;
  };
  */

/*   if (brush_bmp && brush_bmp->data != 0) */
/*     printf("Have an image!\n"); */

  switch (rop_draw->ROP) /* Ternary raster operations */
    {
    case SRCCOPY: /* dest = source */
      printf("ipa_rop_draw SRCCOPY ROP mode not implemented\n");
      break;
    case SRCPAINT: /* dest = source OR dest */
      printf("ipa_rop_draw SRCPAINT ROP mode not implemented\n");
      break;
    case SRCAND: /* dest = source AND dest */
      printf("ipa_rop_draw SRCAND ROP mode not implemented\n");
      break;
    case SRCINVERT: /* dest = source XOR dest */
      printf("ipa_rop_draw SRCINVERT ROP mode not implemented\n");
      break;
    case SRCERASE: /* dest = source AND (NOT dest) */
      printf("ipa_rop_draw SRCERASE ROP mode not implemented\n");
      break;
    case NOTSRCCOPY: /* dest = (NOT source) */
      printf("ipa_rop_draw NOTSRCCOPY ROP mode not implemented\n");
      break;
    case NOTSRCERASE: /* dest = (NOT src) AND (NOT dest) */
      printf("ipa_rop_draw NOTSRCERASE ROP mode not implemented\n");
      break;
    case MERGECOPY: /* dest = (source AND pattern) */
      printf("ipa_rop_draw MERGECOPY ROP mode not implemented\n");
      break;
    case MERGEPAINT: /* dest = (NOT source) OR dest */
      printf("ipa_rop_draw MERGEPAINT ROP mode not implemented\n");
      break;
    case PATCOPY: /* dest = pattern */
      util_set_brush(API, rop_draw->dc, BrushApplyFill);
      break;
    case PATPAINT: /* dest = DPSnoo */
      printf("ipa_rop_draw PATPAINT ROP mode not implemented\n");
      break;
    case PATINVERT: /* dest = pattern XOR dest */
      printf("ipa_rop_draw PATINVERT ROP mode not implemented\n");
      break;
    case DSTINVERT: /* dest = (NOT dest) */
      printf("ipa_rop_draw DSTINVERT ROP mode not implemented\n");
      break;
    case BLACKNESS: /* dest = BLACK */
      draw_fill_color_string(WmfDrawingWand,"black");
      break;
    case WHITENESS: /* dest = WHITE */
      draw_fill_color_string(WmfDrawingWand,"white");
      break;
    default:
      printf("ipa_rop_draw 0x%x ROP mode not implemented\n", rop_draw->ROP);
      break;
    }

  DrawRectangle(WmfDrawingWand,
                 XC(rop_draw->TL.x), YC(rop_draw->TL.y),
                 XC(rop_draw->BR.x), YC(rop_draw->BR.y));

  /* Restore graphic wand */
  (void) PopDrawingWand(WmfDrawingWand);
}

static void ipa_bmp_draw(wmfAPI *API, wmfBMP_Draw_t *bmp_draw)
{
  wmf_magick_t
    *ddata = WMF_MAGICK_GetData(API);

  ExceptionInfo
    exception;

  Image
    *image;

  MagickWand
    *magick_wand;

  MagickRealType
    height,
    width;

  PixelPacket
    white;

  if (bmp_draw->bmp.data == 0)
    return;

  GetExceptionInfo(&exception);
  image = (Image*)bmp_draw->bmp.data;
  if (!image)
    {
       InheritException(&ddata->image->exception,&exception);
       return;
    }

  if (bmp_draw->crop.x || bmp_draw->crop.y ||
     (bmp_draw->crop.w != bmp_draw->bmp.width) ||
     (bmp_draw->crop.h != bmp_draw->bmp.height))
    {
      /* Image needs to be cropped */
      Image
        *crop_image;

      RectangleInfo
        crop_info;

      crop_info.x = bmp_draw->crop.x;
      crop_info.y = bmp_draw->crop.y;
      crop_info.width = bmp_draw->crop.w;
      crop_info.height = bmp_draw->crop.h;

      crop_image = CropImage( image, &crop_info, &exception );
      if (crop_image)
        {
          image=DestroyImageList(image);
          image = crop_image;
          bmp_draw->bmp.data = (void*)image;
        }
      else
        InheritException(&ddata->image->exception,&exception);
    }

  QueryColorDatabase( "white", &white, &exception );

  if ( ddata->image_info->texture ||
       !(IsColorEqual(&ddata->image_info->background_color,&white)) ||
       ddata->image_info->background_color.opacity != OpaqueOpacity )
  {
    MagickPixelPacket
      white;

    /*
      Set image white background to transparent so that it may be
      overlaid over non-white backgrounds.
    */
    QueryMagickColor( "white", &white, &exception );
    TransparentPaintImage( image, &white, QuantumRange, MagickFalse );
  }

  width = fabs(bmp_draw->pixel_width * (double) bmp_draw->crop.w);
  height = fabs(bmp_draw->pixel_height * (double) bmp_draw->crop.h);
  magick_wand=NewMagickWandFromImage(image);
  (void) DrawComposite(WmfDrawingWand, CopyCompositeOp,
    XC(bmp_draw->pt.x) * ddata->scale_x, YC(bmp_draw->pt.y) * ddata->scale_y,
    width * ddata->scale_x, height * ddata->scale_y, magick_wand);
  magick_wand=DestroyMagickWand(magick_wand);

#if 0
  printf("bmp_draw->bmp.data   = 0x%lx\n", (long)bmp_draw->bmp.data);
  printf("registry id          = %li\n", id);
  /* printf("pixel_width          = %g\n", bmp_draw->pixel_width); */
  /* printf("pixel_height         = %g\n", bmp_draw->pixel_height); */
  printf("bmp_draw->bmp WxH    = %ix%i\n", bmp_draw->bmp.width, bmp_draw->bmp.height);
  printf("bmp_draw->crop WxH   = %ix%i\n", bmp_draw->crop.w, bmp_draw->crop.h);
  printf("bmp_draw->crop x,y   = %i,%i\n", bmp_draw->crop.x, bmp_draw->crop.y);
  printf("image size WxH       = %lux%lu\n", image->columns, image->rows);
#endif
}

static void ipa_bmp_read(wmfAPI * API, wmfBMP_Read_t * bmp_read) {
  wmf_magick_t
    *ddata = WMF_MAGICK_GetData(API);

  ExceptionInfo
    exception;

  Image
    *image;

  ImageInfo
    *image_info;

  bmp_read->bmp.data = 0;

  GetExceptionInfo(&exception);

  image_info=CloneImageInfo((ImageInfo *) 0);
  (void) CopyMagickString(image_info->magick,"DIB",MaxTextExtent);
  if (bmp_read->width || bmp_read->height)
    {
      char
        size[MaxTextExtent];

      (void) FormatLocaleString(size,MaxTextExtent,"%ux%u",bmp_read->width,
        bmp_read->height);
      CloneString(&image_info->size,size);
    }
#if 0
  printf("ipa_bmp_read: buffer=0x%lx length=%ld, width=%i, height=%i\n",
   (long) bmp_read->buffer, bmp_read->length,
   bmp_read->width, bmp_read->height);
#endif
  image=BlobToImage(image_info, (const void *) bmp_read->buffer,
    bmp_read->length, &exception);
  image_info=DestroyImageInfo(image_info);
  if (image == (Image *) NULL)
    {
      char
        description[MaxTextExtent];

      (void) FormatLocaleString(description,MaxTextExtent,
        "packed DIB at offset %ld",bmp_read->offset);
      (void) ThrowMagickException(&ddata->image->exception,GetMagickModule(),
        CorruptImageError,exception.reason,"`%s'",exception.description);
    }
  else
    {
#if 0
      printf("ipa_bmp_read: rows=%ld,columns=%ld\n\n", image->rows, image->columns);
#endif

      bmp_read->bmp.data   = (void*)image;
      bmp_read->bmp.width  = (U16)image->columns;
      bmp_read->bmp.height = (U16)image->rows;
    }
}

static void ipa_bmp_free(wmfAPI * API, wmfBMP * bmp)
{
  (void) API;
  DestroyImageList((Image*)bmp->data);
  bmp->data = (void*) 0;
  bmp->width = (U16) 0;
  bmp->height = (U16) 0;
}

/*
  This called by wmf_play() the *first* time the meta file is played
 */
static void ipa_device_open(wmfAPI * API)
{
  wmf_magick_t
    *ddata = WMF_MAGICK_GetData (API);

  ddata->pattern_id = 0;
  ddata->clipping = MagickFalse;
  ddata->clip_mask_id = 0;

  ddata->push_depth = 0;

  ddata->draw_wand = DrawAllocateWand(ddata->draw_info,ddata->image);
}

/*
  This called by wmf_api_destroy()
 */
static void ipa_device_close(wmfAPI * API)
{
  wmf_magick_t
    *ddata = WMF_MAGICK_GetData(API);

  DestroyDrawingWand(ddata->draw_wand);
  DestroyDrawInfo(ddata->draw_info);
  RelinquishMagickMemory(WMF_MAGICK_GetFontData(API)->ps_name);
}

/*
  This called from the beginning of each play for initial page setup
 */
static void ipa_device_begin(wmfAPI * API)
{
  char
    comment[MaxTextExtent];

  wmf_magick_t
    *ddata = WMF_MAGICK_GetData(API);

  /* Make SVG output happy */
  (void) PushDrawingWand(WmfDrawingWand);

  DrawSetViewbox(WmfDrawingWand, 0, 0, ddata->image->columns, ddata->image->rows );

  (void) FormatLocaleString(comment,MaxTextExtent,"Created by ImageMagick %s",
    GetMagickVersion((size_t *) NULL));
  DrawComment(WmfDrawingWand,comment);

  /* Scale width and height to image */
  DrawScale(WmfDrawingWand, ddata->scale_x, ddata->scale_y);

  /* Translate to TL corner of bounding box */
  DrawTranslate(WmfDrawingWand, ddata->translate_x, ddata->translate_y);

  /* Apply rotation */
  DrawRotate(WmfDrawingWand, ddata->rotate);

  if (ddata->image_info->texture == NULL)
    {
      PixelWand
        *background_color;

      /* Draw rectangle in background color */
      background_color=NewPixelWand();
      PixelSetQuantumColor(background_color,&ddata->image->background_color);
      DrawSetFillColor(WmfDrawingWand,background_color);
      background_color=DestroyPixelWand(background_color);
      DrawRectangle(WmfDrawingWand,
                     XC(ddata->bbox.TL.x),YC(ddata->bbox.TL.y),
                     XC(ddata->bbox.BR.x),YC(ddata->bbox.BR.y));
    }
  else
    {
      /* Draw rectangle with texture image the SVG way */
      Image
        *image;

      ImageInfo
        *image_info;

      ExceptionInfo
        exception;

      GetExceptionInfo(&exception);

      image_info = CloneImageInfo((ImageInfo *) 0);
      (void) CopyMagickString(image_info->filename,ddata->image_info->texture,
        MaxTextExtent);
      if ( ddata->image_info->size )
        CloneString(&image_info->size,ddata->image_info->size);

      image = ReadImage(image_info,&exception);
      image_info=DestroyImageInfo(image_info);
      if (image)
        {
          char
            pattern_id[30];

          MagickWand
            *magick_wand;

          (void) CopyMagickString(image->magick,"MIFF",MaxTextExtent);
          DrawPushDefs(WmfDrawingWand);
          draw_pattern_push(API,ddata->pattern_id,image->columns,image->rows);
          magick_wand=NewMagickWandFromImage(image);
          (void) DrawComposite(WmfDrawingWand,CopyCompositeOp,0,0,
            image->columns,image->rows,magick_wand);
          magick_wand=DestroyMagickWand(magick_wand);
          (void) DrawPopPattern(WmfDrawingWand);
          DrawPopDefs(WmfDrawingWand);
          (void) FormatLocaleString(pattern_id,MaxTextExtent,"#brush_%lu",
            ddata->pattern_id);
          (void) DrawSetFillPatternURL(WmfDrawingWand,pattern_id);
          ++ddata->pattern_id;

          DrawRectangle(WmfDrawingWand,
                         XC(ddata->bbox.TL.x),YC(ddata->bbox.TL.y),
                         XC(ddata->bbox.BR.x),YC(ddata->bbox.BR.y));
          image=DestroyImageList(image);
        }
      else
        {
          LogMagickEvent(CoderEvent,GetMagickModule(),
            "reading texture image failed!");
          InheritException(&ddata->image->exception,&exception);
        }
    }

  DrawSetClipRule(WmfDrawingWand,EvenOddRule); /* Default for WMF is ALTERNATE polygon fill mode */
  draw_fill_color_string(WmfDrawingWand,"none"); /* Default brush is WHITE_BRUSH */
  draw_stroke_color_string(WmfDrawingWand,"none"); /* Default pen is BLACK_PEN */
  DrawSetStrokeLineCap(WmfDrawingWand,ButtCap); /* Default linecap is PS_ENDCAP_FLAT */
  DrawSetStrokeLineJoin(WmfDrawingWand,MiterJoin); /* Default linejoin is PS_JOIN_MITER */
  draw_under_color_string(WmfDrawingWand,"white"); /* Default text box is white */
}

/*
  This called from the end of each play for page termination
 */
static void ipa_device_end(wmfAPI * API)
{
  wmf_magick_t
    *ddata = WMF_MAGICK_GetData(API);

  /* Reset any existing clip paths by popping wand */
  if (ddata->clipping)
    (void) PopDrawingWand(WmfDrawingWand);
  ddata->clipping = MagickFalse;

  /* Make SVG output happy */
  (void) PopDrawingWand(WmfDrawingWand);
}

static void ipa_flood_interior(wmfAPI * API, wmfFlood_t * flood)
{
  /* Save graphic wand */
  (void) PushDrawingWand(WmfDrawingWand);

  draw_fill_color_rgb(API,&(flood->color));

  DrawColor(WmfDrawingWand,XC(flood->pt.x), YC(flood->pt.y),
            FillToBorderMethod);

  /* Restore graphic wand */
  (void) PopDrawingWand(WmfDrawingWand);
}

static void ipa_flood_exterior(wmfAPI * API, wmfFlood_t * flood)
{
  /* Save graphic wand */
  (void) PushDrawingWand(WmfDrawingWand);

  draw_fill_color_rgb(API,&(flood->color));

  if (flood->type == FLOODFILLSURFACE)
    DrawColor(WmfDrawingWand, XC(flood->pt.x), YC(flood->pt.y),
              FloodfillMethod);
  else
    DrawColor(WmfDrawingWand, XC(flood->pt.x), YC(flood->pt.y),
              FillToBorderMethod);

  /* Restore graphic wand */
  (void) PopDrawingWand(WmfDrawingWand);
}

static void ipa_draw_pixel(wmfAPI * API, wmfDrawPixel_t * draw_pixel)
{
  /* Save graphic wand */
  (void) PushDrawingWand(WmfDrawingWand);

  draw_stroke_color_string(WmfDrawingWand,"none");

  draw_fill_color_rgb(API,&(draw_pixel->color));

  DrawRectangle(WmfDrawingWand,
                 XC(draw_pixel->pt.x),
                 YC(draw_pixel->pt.y),
                 XC(draw_pixel->pt.x + draw_pixel->pixel_width),
                 YC(draw_pixel->pt.y + draw_pixel->pixel_height));

  /* Restore graphic wand */
  (void) PopDrawingWand(WmfDrawingWand);
}

static void ipa_draw_pie(wmfAPI * API, wmfDrawArc_t * draw_arc)
{
  util_draw_arc(API, draw_arc, magick_arc_pie);
}

static void ipa_draw_chord(wmfAPI * API, wmfDrawArc_t * draw_arc)
{
  util_draw_arc(API, draw_arc, magick_arc_chord);
}

static void ipa_draw_arc(wmfAPI * API, wmfDrawArc_t * draw_arc)
{
  util_draw_arc(API, draw_arc, magick_arc_open);
}

static void ipa_draw_ellipse(wmfAPI * API, wmfDrawArc_t * draw_arc)
{
  util_draw_arc(API, draw_arc, magick_arc_ellipse);
}

static void util_draw_arc(wmfAPI * API,
          wmfDrawArc_t * draw_arc, magick_arc_t finish)
{
  wmfD_Coord
    BR,
    O,
    TL,
    center,
    end,
    start;

  double
    phi_e = 360,
    phi_s = 0;

  double
    Rx,
    Ry;

  /* Save graphic wand */
  (void) PushDrawingWand(WmfDrawingWand);

  if (TO_FILL(draw_arc) || TO_DRAW(draw_arc))
    {
      center.x = (draw_arc->TL.x + draw_arc->BR.x) / 2;
      center.y = (draw_arc->TL.y + draw_arc->BR.y) / 2;
      start = center;
      end = center;

      if (finish != magick_arc_ellipse)
        {
          draw_arc->start.x += center.x;
          draw_arc->start.y += center.y;

          draw_arc->end.x += center.x;
          draw_arc->end.y += center.y;
        }

      TL = draw_arc->TL;
      BR = draw_arc->BR;

      O = center;

      if (finish != magick_arc_ellipse)
        {
          start = draw_arc->start;
          end = draw_arc->end;
        }

      Rx = (BR.x - TL.x) / 2;
      Ry = (BR.y - TL.y) / 2;

      if (finish != magick_arc_ellipse)
        {
          start.x -= O.x;
          start.y -= O.y;

          end.x -= O.x;
          end.y -= O.y;

          phi_s = atan2((double) start.y, (double) start.x) * 180 / MagickPI;
          phi_e = atan2((double) end.y, (double) end.x) * 180 / MagickPI;

          if (phi_e <= phi_s)
            phi_e += 360;
        }

      util_set_pen(API, draw_arc->dc);
      if (finish == magick_arc_open)
        draw_fill_color_string(WmfDrawingWand,"none");
      else
        util_set_brush(API, draw_arc->dc, BrushApplyFill);

      if (finish == magick_arc_ellipse)
        DrawEllipse(WmfDrawingWand, XC(O.x), YC(O.y), Rx, Ry, 0, 360);
      else if (finish == magick_arc_pie)
        {
          DrawPathStart(WmfDrawingWand);
          DrawPathMoveToAbsolute(WmfDrawingWand, XC(O.x+start.x),
            YC(O.y+start.y));
          DrawPathEllipticArcAbsolute(WmfDrawingWand, Rx, Ry, 0, MagickFalse,
            MagickTrue, XC(O.x+end.x), YC(O.y+end.y));
          DrawPathLineToAbsolute(WmfDrawingWand, XC(O.x), YC(O.y));
          DrawPathClose(WmfDrawingWand);
          DrawPathFinish(WmfDrawingWand);
        }
        else if (finish == magick_arc_chord)
        {
          DrawArc(WmfDrawingWand, XC(draw_arc->TL.x), YC(draw_arc->TL.y),
            XC(draw_arc->BR.x), XC(draw_arc->BR.y), phi_s, phi_e);
          DrawLine(WmfDrawingWand, XC(draw_arc->BR.x-start.x),
            YC(draw_arc->BR.y-start.y), XC(draw_arc->BR.x-end.x),
            YC(draw_arc->BR.y-end.y));
        }
        else      /* if (finish == magick_arc_open) */
          DrawArc(WmfDrawingWand, XC(draw_arc->TL.x), YC(draw_arc->TL.y),
            XC(draw_arc->BR.x), XC(draw_arc->BR.y), phi_s, phi_e);
    }

  /* Restore graphic wand */
  (void) PopDrawingWand(WmfDrawingWand);
}

static void ipa_draw_line(wmfAPI * API, wmfDrawLine_t * draw_line)
{
  /* Save graphic wand */
  (void) PushDrawingWand(WmfDrawingWand);

  if (TO_DRAW(draw_line))
    {
      util_set_pen(API, draw_line->dc);
      DrawLine(WmfDrawingWand,
               XC(draw_line->from.x), YC(draw_line->from.y),
               XC(draw_line->to.x), YC(draw_line->to.y));
    }

  /* Restore graphic wand */
  (void) PopDrawingWand(WmfDrawingWand);
}

static void ipa_poly_line(wmfAPI * API, wmfPolyLine_t * polyline)
{
  if (polyline->count <= 2)
    return;

  if (TO_DRAW(polyline))
    {
      int
        point;

      /* Save graphic wand */
      (void) PushDrawingWand(WmfDrawingWand);

      util_set_pen(API, polyline->dc);

      DrawPathStart(WmfDrawingWand);
      DrawPathMoveToAbsolute(WmfDrawingWand,
                             XC(polyline->pt[0].x),
                             YC(polyline->pt[0].y));
      for (point = 1; point < polyline->count; point++)
        {
          DrawPathLineToAbsolute(WmfDrawingWand,
                                 XC(polyline->pt[point].x),
                                 YC(polyline->pt[point].y));
        }
      DrawPathFinish(WmfDrawingWand);

      /* Restore graphic wand */
      (void) PopDrawingWand(WmfDrawingWand);
    }
}

static void ipa_draw_polygon(wmfAPI * API, wmfPolyLine_t * polyline)
{
  if (polyline->count <= 2)
    return;

  if (TO_FILL(polyline) || TO_DRAW(polyline))
    {
      int
        point;

      /* Save graphic wand */
      (void) PushDrawingWand(WmfDrawingWand);

      util_set_pen(API, polyline->dc);
      util_set_brush(API, polyline->dc, BrushApplyFill);

      DrawPathStart(WmfDrawingWand);
      DrawPathMoveToAbsolute(WmfDrawingWand,
                             XC(polyline->pt[0].x),
                             YC(polyline->pt[0].y));
      for (point = 1; point < polyline->count; point++)
        {
          DrawPathLineToAbsolute(WmfDrawingWand,
                                 XC(polyline->pt[point].x),
                                 YC(polyline->pt[point].y));
        }
      DrawPathClose(WmfDrawingWand);
      DrawPathFinish(WmfDrawingWand);

      /* Restore graphic wand */
      (void) PopDrawingWand(WmfDrawingWand);
    }
}

/* Draw a polypolygon.  A polypolygon is a list of polygons */
#if defined(MAGICKCORE_WMFLITE_DELEGATE)
static void ipa_draw_polypolygon(wmfAPI * API, wmfPolyPoly_t* polypolygon)
{
  if (TO_FILL(polypolygon) || TO_DRAW(polypolygon))
    {
      int
        polygon,
        point;

      wmfPolyLine_t
        polyline;

      /* Save graphic wand */
      (void) PushDrawingWand(WmfDrawingWand);

      util_set_pen(API, polypolygon->dc);
      util_set_brush(API, polypolygon->dc, BrushApplyFill);

      DrawPathStart(WmfDrawingWand);
      for (polygon = 0; polygon < polypolygon->npoly; polygon++)
        {
          polyline.dc = polypolygon->dc;
          polyline.pt = polypolygon->pt[polygon];
          polyline.count = polypolygon->count[polygon];
          if ((polyline.count > 2) && polyline.pt)
            {
              DrawPathMoveToAbsolute(WmfDrawingWand,
                                     XC(polyline.pt[0].x),
                                     YC(polyline.pt[0].y));
              for (point = 1; point < polyline.count; point++)
                {
                  DrawPathLineToAbsolute(WmfDrawingWand,
                                         XC(polyline.pt[point].x),
                                         YC(polyline.pt[point].y));
                }
              DrawPathClose(WmfDrawingWand);
            }
        }
      DrawPathFinish(WmfDrawingWand);

      /* Restore graphic wand */
      (void) PopDrawingWand(WmfDrawingWand);
    }
}
#endif

static void ipa_draw_rectangle(wmfAPI * API, wmfDrawRectangle_t * draw_rect)
{
  /* Save graphic wand */
  (void) PushDrawingWand(WmfDrawingWand);

  if (TO_FILL(draw_rect) || TO_DRAW(draw_rect))
    {
      util_set_pen(API, draw_rect->dc);
      util_set_brush(API, draw_rect->dc, BrushApplyFill);

      if ((draw_rect->width > 0) || (draw_rect->height > 0))
        DrawRoundRectangle(WmfDrawingWand,
                           XC(draw_rect->TL.x), YC(draw_rect->TL.y),
                           XC(draw_rect->BR.x), YC(draw_rect->BR.y),
                           draw_rect->width / 2, draw_rect->height / 2);
      else
        DrawRectangle(WmfDrawingWand,
                      XC(draw_rect->TL.x), YC(draw_rect->TL.y),
                      XC(draw_rect->BR.x), YC(draw_rect->BR.y));
    }

  /* Restore graphic wand */
  (void) PopDrawingWand(WmfDrawingWand);
}

/* Draw an un-filled rectangle using the current brush */
static void ipa_region_frame(wmfAPI * API, wmfPolyRectangle_t * poly_rect)
{
  /* Save graphic wand */
  (void) PushDrawingWand(WmfDrawingWand);

  if (TO_FILL(poly_rect) || TO_DRAW(poly_rect))
    {
      long
        i;

      draw_fill_color_string(WmfDrawingWand,"none");
      util_set_brush(API, poly_rect->dc, BrushApplyStroke);

      for (i = 0; i < (long) poly_rect->count; i++)
        {
          DrawRectangle(WmfDrawingWand,
                         XC(poly_rect->TL[i].x), YC(poly_rect->TL[i].y),
                         XC(poly_rect->BR[i].x), YC(poly_rect->BR[i].y));
        }
    }

  /* Restore graphic wand */
  (void) PopDrawingWand(WmfDrawingWand);
}

static void ipa_region_paint(wmfAPI * API, wmfPolyRectangle_t * poly_rect)
{

  if (poly_rect->count == 0)
    return;

  /* Save graphic wand */
  (void) PushDrawingWand(WmfDrawingWand);

  if (TO_FILL (poly_rect))
    {
      long
        i;

      draw_stroke_color_string(WmfDrawingWand,"none");
      util_set_brush(API, poly_rect->dc, BrushApplyFill);

      for (i = 0; i < (long) poly_rect->count; i++)
        {
          DrawRectangle(WmfDrawingWand,
                         XC(poly_rect->TL[i].x), YC(poly_rect->TL[i].y),
                         XC(poly_rect->BR[i].x), YC(poly_rect->BR[i].y));
        }
    }

  /* Restore graphic wand */
  (void) PopDrawingWand(WmfDrawingWand);
}

static void ipa_region_clip(wmfAPI *API, wmfPolyRectangle_t *poly_rect)
{
  long
    i;

  wmf_magick_t
    *ddata = WMF_MAGICK_GetData (API);

  /* Reset any existing clip paths by popping wand */
  if (ddata->clipping)
    (void) PopDrawingWand(WmfDrawingWand);
  ddata->clipping = MagickFalse;

  if (poly_rect->count > 0)
    {
      char
        clip_mask_id[30];

      /* Define clip path */
      ddata->clip_mask_id++;
      DrawPushDefs(WmfDrawingWand);
      (void) FormatLocaleString(clip_mask_id,MaxTextExtent,"clip_%lu",
        ddata->clip_mask_id);
      DrawPushClipPath(WmfDrawingWand,clip_mask_id);
      (void) PushDrawingWand(WmfDrawingWand);
      for (i = 0; i < (long) poly_rect->count; i++)
        {
          DrawRectangle(WmfDrawingWand,
                         XC(poly_rect->TL[i].x), YC(poly_rect->TL[i].y),
                         XC(poly_rect->BR[i].x), YC(poly_rect->BR[i].y));
        }
      (void) PopDrawingWand(WmfDrawingWand);
      DrawPopClipPath(WmfDrawingWand);
      DrawPopDefs(WmfDrawingWand);

      /* Push wand for new clip paths */
      (void) PushDrawingWand(WmfDrawingWand);
      (void) DrawSetClipPath(WmfDrawingWand,clip_mask_id);
      ddata->clipping = MagickTrue;
    }
}

static void ipa_functions(wmfAPI *API)
{
  wmf_magick_t
    *ddata = 0;

  wmfFunctionReference
    *FR = (wmfFunctionReference *) API->function_reference;

  /*
     IPA function reference links
   */
  FR->device_open = ipa_device_open;
  FR->device_close = ipa_device_close;
  FR->device_begin = ipa_device_begin;
  FR->device_end = ipa_device_end;
  FR->flood_interior = ipa_flood_interior;
  FR->flood_exterior = ipa_flood_exterior;
  FR->draw_pixel = ipa_draw_pixel;
  FR->draw_pie = ipa_draw_pie;
  FR->draw_chord = ipa_draw_chord;
  FR->draw_arc = ipa_draw_arc;
  FR->draw_ellipse = ipa_draw_ellipse;
  FR->draw_line = ipa_draw_line;
  FR->poly_line = ipa_poly_line;
  FR->draw_polygon = ipa_draw_polygon;
#if defined(MAGICKCORE_WMFLITE_DELEGATE)
  FR->draw_polypolygon = ipa_draw_polypolygon;
#endif
  FR->draw_rectangle = ipa_draw_rectangle;
  FR->rop_draw = ipa_rop_draw;
  FR->bmp_draw = ipa_bmp_draw;
  FR->bmp_read = ipa_bmp_read;
  FR->bmp_free = ipa_bmp_free;
  FR->draw_text = ipa_draw_text;
  FR->udata_init = ipa_udata_init;
  FR->udata_copy = ipa_udata_copy;
  FR->udata_set = ipa_udata_set;
  FR->udata_free = ipa_udata_free;
  FR->region_frame = ipa_region_frame;
  FR->region_paint = ipa_region_paint;
  FR->region_clip = ipa_region_clip;

  /*
     Allocate device data structure
   */
  ddata = (wmf_magick_t *) wmf_malloc(API, sizeof(wmf_magick_t));
  if (ERR(API))
    return;

  (void) ResetMagickMemory((void *) ddata, 0, sizeof(wmf_magick_t));
  API->device_data = (void *) ddata;

  /*
     Device data defaults
   */
  ddata->image = 0;
}

static void ipa_draw_text(wmfAPI * API, wmfDrawText_t * draw_text)
{
  double
    angle = 0,      /* text rotation angle */
    bbox_height,    /* bounding box height */
    bbox_width,      /* bounding box width */
    pointsize = 0;    /* pointsize to output font with desired height */

  TypeMetric
    metrics;

  wmfD_Coord
    BL,        /* bottom left of bounding box */
    BR,        /* bottom right of bounding box */
    TL,        /* top left of bounding box */
    TR;        /* top right of bounding box */

  wmfD_Coord
    point;      /* text placement point */

  wmfFont
    *font;

  wmf_magick_t
    * ddata = WMF_MAGICK_GetData(API);

  point = draw_text->pt;

  /* Choose bounding box and calculate its width and height */
  {
    double dx,
      dy;

    if ( draw_text->flags)
      {
        TL = draw_text->TL;
        BR = draw_text->BR;
        TR.x = draw_text->BR.x;
        TR.y = draw_text->TL.y;
        BL.x = draw_text->TL.x;
        BL.y = draw_text->BR.y;
      }
    else
      {
        TL = draw_text->bbox.TL;
        BR = draw_text->bbox.BR;
        TR = draw_text->bbox.TR;
        BL = draw_text->bbox.BL;
      }
    dx = ((TR.x - TL.x) + (BR.x - BL.x)) / 2;
    dy = ((TR.y - TL.y) + (BR.y - BL.y)) / 2;
    bbox_width = hypot(dx,dy);
    dx = ((BL.x - TL.x) + (BR.x - TR.x)) / 2;
    dy = ((BL.y - TL.y) + (BR.y - TR.y)) / 2;
    bbox_height = hypot(dx,dy);
  }

  font = WMF_DC_FONT(draw_text->dc);

  /* Convert font_height to equivalent pointsize */
  pointsize = util_pointsize( API, font, draw_text->str, draw_text->font_height);

  /* Save graphic wand */
  (void) PushDrawingWand(WmfDrawingWand);

  (void) bbox_width;
  (void) bbox_height;
#if 0
  printf("\nipa_draw_text\n");
  printf("Text                    = \"%s\"\n", draw_text->str);
  /* printf("WMF_FONT_NAME:          = \"%s\"\n", WMF_FONT_NAME(font)); */
  printf("WMF_FONT_PSNAME:        = \"%s\"\n", WMF_FONT_PSNAME(font));
  printf("Bounding box            TL=%g,%g BR=%g,%g\n",
         TL.x, TL.y, BR.x, BR.y );
  /* printf("Text box                = %gx%g\n", bbox_width, bbox_height); */
  /* printf("WMF_FONT_HEIGHT         = %i\n", (int)WMF_FONT_HEIGHT(font)); */
  printf("Pointsize               = %g\n", pointsize);
  fflush(stdout);
#endif

  /*
   * Obtain font metrics if required
   *
   */
  if ((WMF_DC_TEXTALIGN(draw_text->dc) & TA_CENTER) ||
      (WMF_TEXT_UNDERLINE(font)) || (WMF_TEXT_STRIKEOUT(font)))
    {
      Image
        *image = ddata->image;

      DrawInfo
        *draw_info;

      draw_info=ddata->draw_info;
      draw_info->font=WMF_FONT_PSNAME(font);
      draw_info->pointsize = pointsize;
      draw_info->text=draw_text->str;

      if (GetTypeMetrics(image, draw_info, &metrics) != MagickFalse)
        {
          /* Center the text if it is not yet centered and should be */
          if ((WMF_DC_TEXTALIGN(draw_text->dc) & TA_CENTER))
            {
              double
                text_width = metrics.width * (ddata->scale_y / ddata->scale_x);

#if defined(MAGICKCORE_WMFLITE_DELEGATE)
              point.x -= text_width / 2;
#else
              point.x += bbox_width / 2 - text_width / 2;
#endif
            }
        }
      draw_info->font=NULL;
      draw_info->text=NULL;
    }

  /* Set text background color */
  if (draw_text->flags & ETO_OPAQUE)
    {
      /* Draw bounding-box background color (META_EXTTEXTOUT mode) */
      draw_stroke_color_string(WmfDrawingWand,"none");
      draw_fill_color_rgb(API,WMF_DC_BACKGROUND(draw_text->dc));
      DrawRectangle(WmfDrawingWand,
                    XC(draw_text->TL.x),YC(draw_text->TL.y),
                    XC(draw_text->BR.x),YC(draw_text->BR.y));
      draw_fill_color_string(WmfDrawingWand,"none");
    }
  else
    {
      /* Set text undercolor */
      if (WMF_DC_OPAQUE(draw_text->dc))
        {
          wmfRGB
            *box = WMF_DC_BACKGROUND(draw_text->dc);

          PixelWand
            *under_color;

          under_color=NewPixelWand();
          PixelSetRedQuantum(under_color,ScaleCharToQuantum(box->r));
          PixelSetGreenQuantum(under_color,ScaleCharToQuantum(box->g));
          PixelSetBlueQuantum(under_color,ScaleCharToQuantum(box->b));
          PixelSetOpacityQuantum(under_color,OpaqueOpacity);
          DrawSetTextUnderColor(WmfDrawingWand,under_color);
          under_color=DestroyPixelWand(under_color);
        }
      else
        draw_under_color_string(WmfDrawingWand,"none");
    }

  /* Set text clipping (META_EXTTEXTOUT mode) */
  if ( draw_text->flags & ETO_CLIPPED)
    {
    }

  /* Set stroke color */
  draw_stroke_color_string(WmfDrawingWand,"none");

  /* Set fill color */
  draw_fill_color_rgb(API,WMF_DC_TEXTCOLOR(draw_text->dc));

  /* Output font size */
  (void) DrawSetFontSize(WmfDrawingWand,pointsize);

  /* Output Postscript font name */
  (void) DrawSetFont(WmfDrawingWand, WMF_FONT_PSNAME(font));

  /* Translate coordinates so target is 0,0 */
  DrawTranslate(WmfDrawingWand, XC(point.x), YC(point.y));

  /* Transform horizontal scale to draw text at 1:1 ratio */
  DrawScale(WmfDrawingWand, ddata->scale_y / ddata->scale_x, 1.0);

  /* Apply rotation */
  /* ImageMagick's drawing rotation is clockwise from horizontal
     while WMF drawing rotation is counterclockwise from horizontal */
  angle = fabs(RadiansToDegrees(2 * MagickPI - WMF_TEXT_ANGLE(font)));
  if (angle == 360)
    angle = 0;
  if (angle != 0)
    DrawRotate(WmfDrawingWand, angle);

  /*
   * Render text
   *
   */

  /* Output string */
  DrawAnnotation(WmfDrawingWand, 0, 0, (unsigned char*)draw_text->str);

  /* Underline text the Windows way (at the bottom) */
  if (WMF_TEXT_UNDERLINE(font))
    {
      double
        line_height;

      wmfD_Coord
        ulBR,      /* bottom right of underline rectangle */
        ulTL;      /* top left of underline rectangle */

      line_height = ((double)1/(ddata->scale_x))*metrics.underline_thickness;
      if (metrics.underline_thickness < 1.5)
        line_height *= 0.55;
      ulTL.x = 0;
      ulTL.y = fabs(metrics.descent) - line_height;
      ulBR.x = metrics.width;
      ulBR.y = fabs(metrics.descent);

      DrawRectangle(WmfDrawingWand,
                    XC(ulTL.x), YC(ulTL.y), XC(ulBR.x), YC(ulBR.y));
    }

  /* Strikeout text the Windows way */
  if (WMF_TEXT_STRIKEOUT(font))
    {
      double line_height;

      wmfD_Coord
        ulBR,      /* bottom right of strikeout rectangle */
        ulTL;      /* top left of strikeout rectangle */

      line_height = ((double)1/(ddata->scale_x))*metrics.underline_thickness;

      if (metrics.underline_thickness < 2.0)
        line_height *= 0.55;
      ulTL.x = 0;
      ulTL.y = -(((double) metrics.ascent) / 2 + line_height / 2);
      ulBR.x = metrics.width;
      ulBR.y = -(((double) metrics.ascent) / 2 - line_height / 2);

      DrawRectangle(WmfDrawingWand,
                    XC(ulTL.x), YC(ulTL.y), XC(ulBR.x), YC(ulBR.y));

    }

  /* Restore graphic wand */
  (void) PopDrawingWand(WmfDrawingWand);

#if 0
  (void) PushDrawingWand(WmfDrawingWand);
  draw_stroke_color_string(WmfDrawingWand,"red");
  draw_fill_color_string(WmfDrawingWand,"none");
  DrawRectangle(WmfDrawingWand,
                XC(TL.x), YC(TL.y),
                XC(BR.x), YC(BR.y));
  draw_stroke_color_string(WmfDrawingWand,"none");
  (void) PopDrawingWand(WmfDrawingWand);
#endif

}

static void ipa_udata_init(wmfAPI * API, wmfUserData_t * userdata)
{
  (void) API;
  (void) userdata;
  /* wmf_magick_t* ddata = WMF_MAGICK_GetData (API); */

}

static void ipa_udata_copy(wmfAPI * API, wmfUserData_t * userdata)
{
  (void) API;
  (void) userdata;
  /* wmf_magick_t* ddata = WMF_MAGICK_GetData (API); */

}

static void ipa_udata_set(wmfAPI * API, wmfUserData_t * userdata)
{
  (void) API;
  (void) userdata;
  /* wmf_magick_t* ddata = WMF_MAGICK_GetData (API); */

}

static void ipa_udata_free(wmfAPI * API, wmfUserData_t * userdata)
{
  (void) API;
  (void) userdata;
  /* wmf_magick_t* ddata = WMF_MAGICK_GetData (API); */

}

static inline double MagickMin(const double x,const double y)
{
  if (x < y)
    return(x);
  return(y);
}

static void util_set_brush(wmfAPI * API, wmfDC * dc, const BrushApply brush_apply)
{
  wmf_magick_t
    *ddata = WMF_MAGICK_GetData(API);

  wmfBrush
    *brush = WMF_DC_BRUSH(dc);

  /* Set polygon fill rule */
  switch (WMF_DC_POLYFILL(dc))  /* Is this correct ?? */
    {
    case WINDING:
      DrawSetClipRule(WmfDrawingWand,NonZeroRule);
      break;

    case ALTERNATE:
    default:
      DrawSetClipRule(WmfDrawingWand,EvenOddRule);
      break;
    }

  switch (WMF_BRUSH_STYLE(brush))
    {
    case BS_SOLID /* 0 */:
      /* WMF_BRUSH_COLOR specifies brush color, WMF_BRUSH_HATCH
         ignored */
      {
        if ( brush_apply == BrushApplyStroke )
          draw_stroke_color_rgb(API,WMF_BRUSH_COLOR(brush));
        else
          draw_fill_color_rgb(API,WMF_BRUSH_COLOR(brush));
        break;
      }
    case BS_HOLLOW /* 1 */:    /* BS_HOLLOW & BS_NULL share enum */
      /* WMF_BRUSH_COLOR and WMF_BRUSH_HATCH ignored */
      {
        if ( brush_apply == BrushApplyStroke )
          draw_stroke_color_string(WmfDrawingWand,"none");
        else
          draw_fill_color_string(WmfDrawingWand,"none");
        break;
      }
    case BS_HATCHED /* 2 */:
      /* WMF_BRUSH_COLOR specifies the hatch color, WMF_BRUSH_HATCH
         specifies the hatch brush style. If WMF_DC_OPAQUE, then
         WMF_DC_BACKGROUND specifies hatch background color.  */
      {
        DrawPushDefs(WmfDrawingWand);
        draw_pattern_push(API, ddata->pattern_id, 8, 8);
        (void) PushDrawingWand(WmfDrawingWand);

        if (WMF_DC_OPAQUE(dc))
          {
            if ( brush_apply == BrushApplyStroke )
              draw_stroke_color_rgb(API,WMF_DC_BACKGROUND(dc));
            else
              draw_fill_color_rgb(API,WMF_DC_BACKGROUND(dc));

            DrawRectangle(WmfDrawingWand, 0, 0, 7, 7 );
          }

        DrawSetStrokeAntialias(WmfDrawingWand, MagickFalse);
        DrawSetStrokeWidth(WmfDrawingWand, 1);

        draw_stroke_color_rgb(API,WMF_BRUSH_COLOR(brush));

        switch ((unsigned int) WMF_BRUSH_HATCH(brush))
          {

          case HS_HORIZONTAL:  /* ----- */
            {
              DrawLine(WmfDrawingWand, 0, 3, 7,3);
              break;
            }
          case HS_VERTICAL:  /* ||||| */
            {
              DrawLine(WmfDrawingWand, 3, 0, 3, 7);
              break;
            }
          case HS_FDIAGONAL:  /* \\\\\ */
            {
              DrawLine(WmfDrawingWand, 0, 0, 7, 7);
              break;
            }
          case HS_BDIAGONAL:  /* / */
            {
              DrawLine(WmfDrawingWand, 0, 7, 7, 0 );
              break;
            }
          case HS_CROSS:  /* +++++ */
            {
              DrawLine(WmfDrawingWand, 0, 3, 7, 3 );
              DrawLine(WmfDrawingWand, 3, 0, 3, 7 );
              break;
            }
          case HS_DIAGCROSS:  /* xxxxx */
            {
              DrawLine(WmfDrawingWand, 0, 0, 7, 7 );
              DrawLine(WmfDrawingWand, 0, 7, 7, 0 );
              break;
            }
          default:
            {
              printf("util_set_brush: unexpected brush hatch enumeration %u\n",
                     (unsigned int)WMF_BRUSH_HATCH(brush));
            }
          }
        (void) PopDrawingWand(WmfDrawingWand);
        (void) DrawPopPattern(WmfDrawingWand);
        DrawPopDefs(WmfDrawingWand);
        {
          char
            pattern_id[30];

          (void) FormatLocaleString(pattern_id,MaxTextExtent,"#brush_%lu",
            ddata->pattern_id);
          if (brush_apply == BrushApplyStroke )
            (void) DrawSetStrokePatternURL(WmfDrawingWand,pattern_id);
          else
            (void) DrawSetFillPatternURL(WmfDrawingWand,pattern_id);
          ++ddata->pattern_id;
        }
        break;
      }
    case BS_PATTERN /* 3 */:
      /* WMF_BRUSH_COLOR ignored, WMF_BRUSH_HATCH provides handle to
         bitmap */
      {
        printf("util_set_brush: BS_PATTERN not supported\n");
        break;
      }
    case BS_INDEXED /* 4 */:
      {
        printf("util_set_brush: BS_INDEXED not supported\n");
        break;
      }
    case BS_DIBPATTERN /* 5 */:
      {
        wmfBMP
          *brush_bmp = WMF_BRUSH_BITMAP(brush);

        if (brush_bmp && brush_bmp->data != 0)
          {
            CompositeOperator
              mode;

            const Image
              *image;

            ExceptionInfo
              exception;

            MagickWand
              *magick_wand;

            GetExceptionInfo(&exception);

            image = (Image*)brush_bmp->data;

            mode = CopyCompositeOp;  /* Default is copy */
            switch (WMF_DC_ROP(dc))
              {
                /* Binary raster ops */
              case R2_BLACK:
                printf("util_set_brush: R2_BLACK ROP2 mode not supported!\n");
                break;
              case R2_NOTMERGEPEN:
                printf("util_set_brush: R2_NOTMERGEPEN ROP2 mode not supported!\n");
                break;
              case R2_MASKNOTPEN:
                printf("util_set_brush R2_MASKNOTPEN ROP2 mode not supported!\n");
                break;
              case R2_NOTCOPYPEN:
                printf("util_set_brush: R2_NOTCOPYPEN ROP2 mode not supported!\n");
                break;
              case R2_MASKPENNOT:
                printf("util_set_brush: R2_MASKPENNOT ROP2 mode not supported!\n");
                break;
              case R2_NOT:
                printf("util_set_brush: R2_NOT ROP2 mode not supported!\n");
                break;
              case R2_XORPEN:
                printf("util_set_brush: R2_XORPEN ROP2 mode not supported!\n");
                break;
              case R2_NOTMASKPEN:
                printf("util_set_brush: R2_NOTMASKPEN ROP2 mode not supported!\n");
                break;
              case R2_MASKPEN:
                printf("util_set_brush: R2_MASKPEN ROP2 mode not supported!\n");
                break;
              case R2_NOTXORPEN:
                printf("util_set_brush: R2_NOTXORPEN ROP2 mode not supported!\n");
                break;
              case R2_NOP:
                printf("util_set_brush: R2_NOP ROP2 mode not supported!\n");
                break;
              case R2_MERGENOTPEN:
                printf("util_set_brush: R2_MERGENOTPEN ROP2 mode not supported!\n");
                break;
              case R2_COPYPEN:
                mode = CopyCompositeOp;
                break;
              case R2_MERGEPENNOT:
                printf("util_set_brush: R2_MERGEPENNOT ROP2 mode not supported!\n");
                break;
              case R2_MERGEPEN:
                printf("util_set_brush: R2_MERGEPEN ROP2 mode not supported!\n");
                break;
              case R2_WHITE:
                printf("util_set_brush: R2_WHITE ROP2 mode not supported!\n");
                break;
              default:
                {
                  printf("util_set_brush: unexpected ROP2 enumeration %u!\n",
                         (unsigned int)WMF_DC_ROP(dc));
                }
              }

            DrawPushDefs(WmfDrawingWand);
            draw_pattern_push(API, ddata->pattern_id, brush_bmp->width,
              brush_bmp->height);
            magick_wand=NewMagickWandFromImage(image);
            (void) DrawComposite(WmfDrawingWand,mode, 0, 0, brush_bmp->width,
              brush_bmp->height, magick_wand);
            magick_wand=DestroyMagickWand(magick_wand);
            (void) DrawPopPattern(WmfDrawingWand);
            DrawPopDefs(WmfDrawingWand);

            {
              char
                pattern_id[30];

              (void) FormatLocaleString(pattern_id,MaxTextExtent,"#brush_%lu",
                ddata->pattern_id);

              if ( brush_apply == BrushApplyStroke )
                (void) DrawSetStrokePatternURL(WmfDrawingWand,pattern_id);
              else
                (void) DrawSetFillPatternURL(WmfDrawingWand,pattern_id);
              ++ddata->pattern_id;
            }
          }
        else
          printf("util_set_brush: no BMP image data!\n");

        break;
      }
    case BS_DIBPATTERNPT /* 6 */:
      /* WMF_BRUSH_COLOR ignored, WMF_BRUSH_HATCH provides pointer to
         DIB */
      {
        printf("util_set_brush: BS_DIBPATTERNPT not supported\n");
        break;
      }
    case BS_PATTERN8X8 /* 7 */:
      {
        printf("util_set_brush: BS_PATTERN8X8 not supported\n");
        break;
      }
    case BS_DIBPATTERN8X8 /* 8 */:
      {
        printf("util_set_brush: BS_DIBPATTERN8X8 not supported\n");
        break;
      }
    default:
      {
      }
    }
}

static inline double MagickMax(const double x,const double y)
{
  if (x > y)
    return(x);
  return(y);
}

static void util_set_pen(wmfAPI * API, wmfDC * dc)
{
  wmf_magick_t
    *ddata = WMF_MAGICK_GetData(API);

  wmfPen
    *pen = 0;

  double
    pen_width,
    pixel_width;

  unsigned int
    pen_style,
    pen_type;

  pen = WMF_DC_PEN(dc);

  pen_width = (WMF_PEN_WIDTH(pen) + WMF_PEN_HEIGHT(pen)) / 2;

  /* Pixel width is inverse of pixel scale */
  pixel_width = (((double) 1 / (ddata->scale_x)) +
                 ((double) 1 / (ddata->scale_y))) / 2;

  /* Don't allow pen_width to be much less than pixel_width in order
     to avoid dissapearing or spider-web lines */
  pen_width = MagickMax(pen_width, pixel_width*0.8);

  pen_style = (unsigned int) WMF_PEN_STYLE(pen);
  pen_type = (unsigned int) WMF_PEN_TYPE(pen);
  (void) pen_type;

  /* Pen style specified? */
  if (pen_style == PS_NULL)
    {
      draw_stroke_color_string(WmfDrawingWand,"none");
      return;
    }

  DrawSetStrokeAntialias(WmfDrawingWand, MagickTrue );
  DrawSetStrokeWidth(WmfDrawingWand, (unsigned long) MagickMax(0.0, pen_width));

  {
    LineCap
      linecap;

    switch ((unsigned int) WMF_PEN_ENDCAP(pen))
      {
      case PS_ENDCAP_SQUARE:
        linecap = SquareCap;
        break;
      case PS_ENDCAP_ROUND:
        linecap = RoundCap;
        break;
      case PS_ENDCAP_FLAT:
      default:
        linecap = ButtCap;
        break;
      }
    DrawSetStrokeLineCap(WmfDrawingWand, linecap);
  }

  {
    LineJoin
      linejoin;

    switch ((unsigned int) WMF_PEN_JOIN(pen))
      {
      case PS_JOIN_BEVEL:
        linejoin = BevelJoin;
        break;
      case PS_JOIN_ROUND:
        linejoin = RoundJoin;
        break;
      case PS_JOIN_MITER:
      default:
        linejoin = MiterJoin;
        break;
      }
    DrawSetStrokeLineJoin(WmfDrawingWand,linejoin);
  }

  {
    double
      dasharray[7];

    switch (pen_style)
      {
      case PS_DASH:    /* -------  */
        {
          /* Pattern 18,7 */
          dasharray[0] = pixel_width * 18;
          dasharray[1] = pixel_width * 7;
          dasharray[2] = 0;

          DrawSetStrokeAntialias(WmfDrawingWand,MagickFalse);
          (void) DrawSetStrokeDashArray(WmfDrawingWand,2,dasharray);
          break;
        }
      case PS_ALTERNATE:
      case PS_DOT:    /* .......  */
        {
          /* Pattern 3,3 */
          dasharray[0] = pixel_width * 3;
          dasharray[1] = pixel_width * 3;
          dasharray[2] = 0;

          DrawSetStrokeAntialias(WmfDrawingWand,MagickFalse);
          (void) DrawSetStrokeDashArray(WmfDrawingWand,2,dasharray);
          break;
        }
      case PS_DASHDOT:    /* _._._._  */
        {
          /* Pattern 9,6,3,6 */
          dasharray[0] = pixel_width * 9;
          dasharray[1] = pixel_width * 6;
          dasharray[2] = pixel_width * 3;
          dasharray[3] = pixel_width * 6;
          dasharray[4] = 0;

          DrawSetStrokeAntialias(WmfDrawingWand,MagickFalse);
          (void) DrawSetStrokeDashArray(WmfDrawingWand,4,dasharray);
          break;
        }
      case PS_DASHDOTDOT:  /* _.._.._  */
        {
          /* Pattern 9,3,3,3,3,3 */
          dasharray[0] = pixel_width * 9;
          dasharray[1] = pixel_width * 3;
          dasharray[2] = pixel_width * 3;
          dasharray[3] = pixel_width * 3;
          dasharray[4] = pixel_width * 3;
          dasharray[5] = pixel_width * 3;
          dasharray[6] = 0;

          DrawSetStrokeAntialias(WmfDrawingWand,MagickFalse);
          (void) DrawSetStrokeDashArray(WmfDrawingWand,6,dasharray);
          break;
        }
      case PS_INSIDEFRAME:  /* There is nothing to do in this case... */
      case PS_SOLID:
      default:
        {
          (void) DrawSetStrokeDashArray(WmfDrawingWand,0,(double *)NULL);
          break;
        }
      }
  }

  draw_stroke_color_rgb(API,WMF_PEN_COLOR(pen));
}

/* Estimate font pointsize based on Windows font parameters */
static double util_pointsize( wmfAPI* API, wmfFont* font, char* str, double font_height)
{
  wmf_magick_t
    *ddata = WMF_MAGICK_GetData(API);

  Image
    *image = ddata->image;

  TypeMetric
    metrics;

  DrawInfo
    *draw_info;

  double
    pointsize = 0;

  draw_info=ddata->draw_info;
  if (draw_info == (const DrawInfo *) NULL)
    return 0;

  draw_info->font=WMF_FONT_PSNAME(font);
  draw_info->pointsize=font_height;
  draw_info->text=str;

  if (GetTypeMetrics(image, draw_info, &metrics) != MagickFalse)
    {

      if (strlen(str) == 1)
        {
          pointsize = (font_height *
                       ( font_height / (metrics.ascent + fabs(metrics.descent))));
          draw_info->pointsize = pointsize;
          if (GetTypeMetrics(image, draw_info, &metrics) != MagickFalse)
            pointsize *= (font_height / ( metrics.ascent + fabs(metrics.descent)));
        }
      else
        {
          pointsize = (font_height * (font_height / (metrics.height)));
          draw_info->pointsize = pointsize;
          if (GetTypeMetrics(image, draw_info, &metrics) != MagickFalse)
            pointsize *= (font_height / metrics.height);

        }
#if 0
      draw_info.pointsize = pointsize;
      if (GetTypeMetrics(image, &draw_info, &metrics) != MagickFalse)
        pointsize *= (font_height / (metrics.ascent + fabs(metrics.descent)));
      pointsize *= 1.114286; /* Magic number computed through trial and error */
#endif
    }

  draw_info->font=NULL;
  draw_info->text=NULL;
#if 0
  printf("String    = %s\n", str);
  printf("Font      = %s\n", WMF_FONT_PSNAME(font));
  printf("lfHeight  = %g\n", font_height);
  printf("bounds    = %g,%g %g,%g\n", metrics.bounds.x1, metrics.bounds.y1,
         metrics.bounds.x2,metrics.bounds.y2);
  printf("ascent    = %g\n", metrics.ascent);
  printf("descent   = %g\n", metrics.descent);
  printf("height    = %g\n", metrics.height);
  printf("Pointsize = %g\n", pointsize);
#endif

  return floor(pointsize);
}

#if defined(MAGICKCORE_WMFLITE_DELEGATE)
/* Estimate weight based on font name */
/*
static int util_font_weight( const char* font )
{
  int
    weight;

  weight = 400;
  if ((strstr(font,"Normal") || strstr(font,"Regular")))
    weight = 400;
  else if ( strstr(font,"Bold") )
    {
      weight = 700;
      if ((strstr(font,"Semi") || strstr(font,"Demi")))
        weight = 600;
      if ( (strstr(font,"Extra") || strstr(font,"Ultra")))
        weight = 800;
    }
  else if ( strstr(font,"Light") )
    {
      weight = 300;
      if ( (strstr(font,"Extra") || strstr(font,"Ultra")))
        weight = 200;
    }
  else if ((strstr(font,"Heavy") || strstr(font,"Black")))
    weight = 900;
  else if ( strstr(font,"Thin") )
    weight = 100;
  return weight;
}
*/

/*
 * Returns width of string in points, assuming (unstretched) font size of 1pt
 * (similar to wmf_ipa_font_stringwidth)
 *
 * This extremely odd at best, particularly since player/meta.h has access
 * to the corrected font_height (as drawtext.font_height) when it invokes the
 * stringwidth callback.  It should be possible to compute the real stringwidth!
 */
static float lite_font_stringwidth( wmfAPI* API, wmfFont* font, char* str)
{
#if 0
  wmf_magick_t
    *ddata = WMF_MAGICK_GetData(API);

  Image
    *image = ddata->image;

  DrawInfo
    *draw_info;

  TypeMetric
    metrics;

  float
    stringwidth = 0;

  double
    orig_x_resolution,
    orig_y_resolution;

  ResolutionType
    orig_resolution_units;

  orig_x_resolution = image->x_resolution;
  orig_y_resolution = image->y_resolution;
  orig_resolution_units = image->units;

  draw_info=ddata->draw_info;
  if (draw_info == (const DrawInfo *) NULL)
    return 0;

  draw_info->font=WMF_FONT_PSNAME(font);
  draw_info->pointsize=12;
  draw_info->text=str;

  image->x_resolution = 72;
  image->y_resolution = 72;
  image->units = PixelsPerInchResolution;

  if (GetTypeMetrics(image, draw_info, &metrics) != MagickFalse)
    stringwidth = ((metrics.width * 72)/(image->x_resolution * draw_info->pointsize)); /* *0.916348; */

  draw_info->font=NULL;
  draw_info->text=NULL;

#if 0
  printf("\nlite_font_stringwidth\n");
  printf("string                  = \"%s\"\n", str);
  printf("WMF_FONT_NAME           = \"%s\"\n", WMF_FONT_NAME(font));
  printf("WMF_FONT_PSNAME         = \"%s\"\n", WMF_FONT_PSNAME(font));
  printf("stringwidth             = %g\n", stringwidth);
  /* printf("WMF_FONT_HEIGHT         = %i\n", (int)WMF_FONT_HEIGHT(font)); */
  /* printf("WMF_FONT_WIDTH          = %i\n", (int)WMF_FONT_WIDTH(font)); */
  fflush(stdout);
#endif

  image->x_resolution = orig_x_resolution;
  image->y_resolution = orig_y_resolution;
  image->units = orig_resolution_units;

  return stringwidth;
#else
  (void) API;
  (void) font;
  (void) str;

  return 0;
#endif
}

/* Map font (similar to wmf_ipa_font_map) */

/* Mappings to Postscript fonts: family, normal, italic, bold, bolditalic */
static wmfFontMap WMFFontMap[] = {
  { (char *) "Courier",            (char *) "Courier",
    (char *) "Courier-Oblique",    (char *) "Courier-Bold",
    (char *) "Courier-BoldOblique"   },
  { (char *) "Helvetica",          (char *) "Helvetica",
    (char *) "Helvetica-Oblique",  (char *) "Helvetica-Bold",
    (char *) "Helvetica-BoldOblique" },
  { (char *) "Modern",             (char *) "Courier",
    (char *) "Courier-Oblique",    (char *) "Courier-Bold",
    (char *) "Courier-BoldOblique"   },
  { (char *) "Monotype Corsiva",   (char *) "Courier",
    (char *) "Courier-Oblique",    (char *) "Courier-Bold",
    (char *) "Courier-BoldOblique"   },
  { (char *) "News Gothic",        (char *) "Helvetica",
    (char *) "Helvetica-Oblique",  (char *) "Helvetica-Bold",
    (char *) "Helvetica-BoldOblique" },
  { (char *) "Symbol",             (char *) "Symbol",
    (char *) "Symbol",             (char *) "Symbol",
    (char *) "Symbol"                },
  { (char *) "System",             (char *) "Courier",
    (char *) "Courier-Oblique",    (char *) "Courier-Bold",
    (char *) "Courier-BoldOblique"   },
  { (char *) "Times",              (char *) "Times-Roman",
    (char *) "Times-Italic",       (char *) "Times-Bold",
    (char *) "Times-BoldItalic"      },
  { (char *) NULL,                 (char *) NULL,
    (char *) NULL,                 (char *) NULL,
    (char *) NULL                   }
};


/* Mapping between base name and Ghostscript family name */
static wmfMapping SubFontMap[] =
{
  { (char *) "Arial", (char *) "Helvetica", FT_ENCODING_NONE },
  { (char *) "Courier", (char *) "Courier", FT_ENCODING_NONE },
  { (char *) "Fixed", (char *) "Courier", FT_ENCODING_NONE },
  { (char *) "Helvetica", (char *) "Helvetica", FT_ENCODING_NONE },
  { (char *) "Sans", (char *) "Helvetica", FT_ENCODING_NONE },
  { (char *) "Sym", (char *) "Symbol", FT_ENCODING_NONE },
  { (char *) "Terminal", (char *) "Courier", FT_ENCODING_NONE },
  { (char *) "Times", (char *) "Times", FT_ENCODING_NONE },
  { (char *) "Wingdings", (char *) "Symbol", FT_ENCODING_NONE },
  { (char *)  NULL, (char *) NULL, FT_ENCODING_NONE }
};

static void lite_font_map( wmfAPI* API, wmfFont* font)
{
  wmfFontData
    *font_data;

  wmf_magick_font_t
    *magick_font;

  wmf_magick_t
    *ddata = WMF_MAGICK_GetData(API);

  ExceptionInfo
    exception;

  const TypeInfo
    *type_info,
    *type_info_base;

  const char
    *wmf_font_name;

  if (font == 0)
    return;

  font_data = (wmfFontData*)API->font_data;
  font->user_data = font_data->user_data;
  magick_font = (wmf_magick_font_t*)font->user_data;
  wmf_font_name = WMF_FONT_NAME(font);

  if (magick_font->ps_name != (char *) NULL)
    magick_font->ps_name=DestroyString(magick_font->ps_name);

  GetExceptionInfo(&exception);
  type_info_base=GetTypeInfo("*",&exception);
  if (type_info_base == 0)
    {
      InheritException(&ddata->image->exception,&exception);
      return;
    }

  /* Certain short-hand font names are not the proper Windows names
     and should be promoted to the proper names */
  if (LocaleCompare(wmf_font_name,"Times") == 0)
    wmf_font_name = "Times New Roman";
  else if (LocaleCompare(wmf_font_name,"Courier") == 0)
    wmf_font_name = "Courier New";

  /* Look for a family-based best-match */
  if (!magick_font->ps_name)
    {
      int
        target_weight;

      if (WMF_FONT_WEIGHT(font) == 0)
        target_weight = 400;
      else
        target_weight = WMF_FONT_WEIGHT(font);
      type_info=GetTypeInfoByFamily(wmf_font_name,AnyStyle,AnyStretch,
        target_weight,&exception);
      if (type_info == (const TypeInfo *) NULL)
        type_info=GetTypeInfoByFamily(wmf_font_name,AnyStyle,AnyStretch,0,
          &exception);
      if (type_info != (const TypeInfo *) NULL)
        CloneString(&magick_font->ps_name,type_info->name);
    }

  /* Now let's try simple substitution mappings from WMFFontMap */
  if (!magick_font->ps_name)
    {
      char
        target[MaxTextExtent];

      int
        target_weight = 400,
        want_italic = MagickFalse,
        want_bold = MagickFalse,
        i;

      if ( WMF_FONT_WEIGHT(font) != 0 )
        target_weight = WMF_FONT_WEIGHT(font);

      if ( (target_weight > 550) || ((strstr(wmf_font_name,"Bold") ||
                                     strstr(wmf_font_name,"Heavy") ||
                                     strstr(wmf_font_name,"Black"))) )
        want_bold = MagickTrue;

      if ( (WMF_FONT_ITALIC(font)) || ((strstr(wmf_font_name,"Italic") ||
                                       strstr(wmf_font_name,"Oblique"))) )
        want_italic = MagickTrue;

      (void) CopyMagickString(target,"Times",MaxTextExtent);
      for( i=0; SubFontMap[i].name != NULL; i++ )
        {
          if (LocaleCompare(wmf_font_name, SubFontMap[i].name) == 0)
            {
              (void) CopyMagickString(target,SubFontMap[i].mapping,
                MaxTextExtent);
              break;
            }
        }

      for( i=0; WMFFontMap[i].name != NULL; i++ )
        {
          if (LocaleNCompare(WMFFontMap[i].name,target,strlen(WMFFontMap[i].name)) == 0)
            {
              if (want_bold && want_italic)
                CloneString(&magick_font->ps_name,WMFFontMap[i].bolditalic);
              else if (want_italic)
                CloneString(&magick_font->ps_name,WMFFontMap[i].italic);
              else if (want_bold)
                CloneString(&magick_font->ps_name,WMFFontMap[i].bold);
              else
                CloneString(&magick_font->ps_name,WMFFontMap[i].normal);
            }
        }
    }

#if 0
  printf("\nlite_font_map\n");
  printf("WMF_FONT_NAME           = \"%s\"\n", WMF_FONT_NAME(font));
  printf("WMF_FONT_WEIGHT         = %i\n",  WMF_FONT_WEIGHT(font));
  printf("WMF_FONT_PSNAME         = \"%s\"\n", WMF_FONT_PSNAME(font));
  fflush(stdout);
#endif

}

/* Initialize API font structures */
static void lite_font_init( wmfAPI* API, wmfAPI_Options* options)
{
  wmfFontData
    *font_data;

  (void) options;
  API->fonts = 0;

  /* Allocate wmfFontData data structure */
  API->font_data = wmf_malloc(API,sizeof(wmfFontData));
  if (ERR (API))
    return;

  font_data = (wmfFontData*)API->font_data;

  /* Assign function to map font (type wmfMap) */
  font_data->map = lite_font_map;

  /* Assign function to return string width in points (type wmfStringWidth) */
  font_data->stringwidth = lite_font_stringwidth;

  /* Assign user data, not used by libwmflite (type void*) */
  font_data->user_data = wmf_malloc(API,sizeof(wmf_magick_font_t));
  if (ERR(API))
    return;
  ((wmf_magick_font_t*)font_data->user_data)->ps_name = 0;
  ((wmf_magick_font_t*)font_data->user_data)->pointsize = 0;
}

#endif /* MAGICKCORE_WMFLITE_DELEGATE */

/* BLOB read byte */
static int ipa_blob_read(void* wand)
{
  return ReadBlobByte((Image*)wand);
}

/* BLOB seek */
static int ipa_blob_seek(void* wand,long position)
{
  return (int)SeekBlob((Image*)wand,(MagickOffsetType) position,SEEK_SET);
}

/* BLOB tell */
static long ipa_blob_tell(void* wand)
{
  return (long)TellBlob((Image*)wand);
}

static Image *ReadWMFImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  double
    bounding_height,
    bounding_width,
    image_height,
    image_height_inch,
    image_width,
    image_width_inch,
    resolution_y,
    resolution_x,
    units_per_inch;

  float
    wmf_width,
    wmf_height;

  Image
    *image;

  unsigned long
    wmf_options_flags = 0;

  wmf_error_t
    wmf_error;

  wmf_magick_t
    *ddata = 0;

  wmfAPI
    *API = 0;

  wmfAPI_Options
    wmf_api_options;

  wmfD_Rect
    bbox;

  image=AcquireImage(image_info);
  if (OpenBlob(image_info,image,ReadBinaryBlobMode,exception) == MagickFalse)
    {
      if (image->debug != MagickFalse)
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  OpenBlob failed");
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "leave ReadWMFImage()");
        }
      image=DestroyImageList(image);
      return((Image *) NULL);
    }

  /*
   * Create WMF API
   *
   */

  /* Register callbacks */
  wmf_options_flags |= WMF_OPT_FUNCTION;
  (void) ResetMagickMemory(&wmf_api_options, 0, sizeof(wmf_api_options));
  wmf_api_options.function = ipa_functions;

  /* Ignore non-fatal errors */
  wmf_options_flags |= WMF_OPT_IGNORE_NONFATAL;

  wmf_error = wmf_api_create(&API, wmf_options_flags, &wmf_api_options);
  if (wmf_error != wmf_E_None)
    {
      if (API)
        wmf_api_destroy(API);
      if (image->debug != MagickFalse)
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  wmf_api_create failed");
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "leave ReadWMFImage()");
        }
      ThrowReaderException(DelegateError,"UnableToInitializeWMFLibrary");
    }

  /* Register progress monitor */
  wmf_status_function(API,image,magick_progress_callback);

  ddata=WMF_MAGICK_GetData(API);
  ddata->image=image;
  ddata->image_info=image_info;
  ddata->draw_info=CloneDrawInfo(image_info,(const DrawInfo *) NULL);
  ddata->draw_info->font=(char *)
    RelinquishMagickMemory(ddata->draw_info->font);
  ddata->draw_info->text=(char *)
    RelinquishMagickMemory(ddata->draw_info->text);

#if defined(MAGICKCORE_WMFLITE_DELEGATE)
  /* Must initialize font subystem for WMFlite interface */
  lite_font_init (API,&wmf_api_options); /* similar to wmf_ipa_font_init in src/font.c */
  /* wmf_arg_fontdirs (API,options); */ /* similar to wmf_arg_fontdirs in src/wmf.c */

#endif

  /*
   * Open BLOB input via libwmf API
   *
   */
  wmf_error = wmf_bbuf_input(API,ipa_blob_read,ipa_blob_seek,
    ipa_blob_tell,(void*)image);
  if (wmf_error != wmf_E_None)
    {
      wmf_api_destroy(API);
      if (image->debug != MagickFalse)
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  wmf_bbuf_input failed");
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "leave ReadWMFImage()");
        }
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        image->filename);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }

  /*
   * Scan WMF file
   *
   */
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Scanning WMF to obtain bounding box");
  wmf_error=wmf_scan(API, 0, &bbox);
  if (wmf_error != wmf_E_None)
    {
      wmf_api_destroy(API);
      if (image->debug != MagickFalse)
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  wmf_scan failed with wmf_error %d", wmf_error);
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "leave ReadWMFImage()");
        }
      ThrowReaderException(DelegateError,"FailedToScanFile");
    }

  /*
   * Compute dimensions and scale factors
   *
   */

  ddata->bbox=bbox;

  /* User specified resolution */
  resolution_y=DefaultResolution;
  if (image->y_resolution != 0.0)
    {
      resolution_y = image->y_resolution;
      if (image->units == PixelsPerCentimeterResolution)
        resolution_y *= CENTIMETERS_PER_INCH;
    }
  resolution_x=DefaultResolution;
  if (image->x_resolution != 0.0)
    {
      resolution_x = image->x_resolution;
      if (image->units == PixelsPerCentimeterResolution)
        resolution_x *= CENTIMETERS_PER_INCH;
    }

  /* Obtain output size expressed in metafile units */
  wmf_error=wmf_size(API,&wmf_width,&wmf_height);
  if (wmf_error != wmf_E_None)
    {
      wmf_api_destroy(API);
      if (image->debug != MagickFalse)
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  wmf_size failed with wmf_error %d", wmf_error);
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "leave ReadWMFImage()");
        }
      ThrowReaderException(DelegateError,"FailedToComputeOutputSize");
    }

  /* Obtain (or guess) metafile units */
  if ((API)->File->placeable)
    units_per_inch=(API)->File->pmh->Inch;
  else if ( (wmf_width*wmf_height) < 1024*1024)
    units_per_inch=POINTS_PER_INCH;  /* MM_TEXT */
  else
    units_per_inch=TWIPS_PER_INCH;  /* MM_TWIPS */

  /* Calculate image width and height based on specified DPI
     resolution */
  image_width_inch  = (double) wmf_width / units_per_inch;
  image_height_inch = (double) wmf_height / units_per_inch;
  image_width       = image_width_inch * resolution_x;
  image_height      = image_height_inch * resolution_y;

  /* Compute bounding box scale factors and origin translations
   *
   * This all just a hack since libwmf does not currently seem to
   * provide the mapping between LOGICAL coordinates and DEVICE
   * coordinates. This mapping is necessary in order to know
   * where to place the logical bounding box within the image.
   *
   */

  bounding_width  = bbox.BR.x - bbox.TL.x;
  bounding_height = bbox.BR.y - bbox.TL.y;

  ddata->scale_x = image_width/bounding_width;
  ddata->translate_x = 0-bbox.TL.x;
  ddata->rotate = 0;

  /* Heuristic: guess that if the vertical coordinates mostly span
     negative values, then the image must be inverted. */
  if ( fabs(bbox.BR.y) > fabs(bbox.TL.y) )
    {
      /* Normal (Origin at top left of image) */
      ddata->scale_y = (image_height/bounding_height);
      ddata->translate_y = 0-bbox.TL.y;
    }
  else
    {
      /* Inverted (Origin at bottom left of image) */
      ddata->scale_y = (-image_height/bounding_height);
      ddata->translate_y = 0-bbox.BR.y;
    }

  if (image->debug != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
         "  Placeable metafile:          %s",
         (API)->File->placeable ? "Yes" : "No");

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Size in metafile units:      %gx%g",wmf_width,wmf_height);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Metafile units/inch:         %g",units_per_inch);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Size in inches:              %gx%g",
        image_width_inch,image_height_inch);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Bounding Box:                %g,%g %g,%g",
        bbox.TL.x, bbox.TL.y, bbox.BR.x, bbox.BR.y);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Bounding width x height:     %gx%g",bounding_width,
        bounding_height);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Output resolution:           %gx%g",resolution_x,resolution_y);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Image size:                  %gx%g",image_width,image_height);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Bounding box scale factor:   %g,%g",ddata->scale_x,
        ddata->scale_y);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Translation:                 %g,%g",
        ddata->translate_x, ddata->translate_y);
    }

#if 0
#if 0
  {
    typedef struct _wmfPlayer_t wmfPlayer_t;
    struct _wmfPlayer_t
    {
      wmfPen   default_pen;
      wmfBrush default_brush;
      wmfFont  default_font;

      wmfDC* dc; /* current dc */
    };

    wmfDC
      *dc;

#define WMF_ELICIT_DC(API) (((wmfPlayer_t*)((API)->player_data))->dc)

    dc = WMF_ELICIT_DC(API);

    printf("dc->Window.Ox     = %d\n", dc->Window.Ox);
    printf("dc->Window.Oy     = %d\n", dc->Window.Oy);
    printf("dc->Window.width  = %d\n", dc->Window.width);
    printf("dc->Window.height = %d\n", dc->Window.height);
    printf("dc->pixel_width   = %g\n", dc->pixel_width);
    printf("dc->pixel_height  = %g\n", dc->pixel_height);
#if defined(MAGICKCORE_WMFLITE_DELEGATE)  /* Only in libwmf 0.3 */
    printf("dc->Ox            = %.d\n", dc->Ox);
    printf("dc->Oy            = %.d\n", dc->Oy);
    printf("dc->width         = %.d\n", dc->width);
    printf("dc->height        = %.d\n", dc->height);
#endif

  }
#endif

#endif

  /*
   * Create canvas image
   *
   */
  image->rows=(unsigned long) ceil(image_height);
  image->columns=(unsigned long) ceil(image_width);

  if (image_info->ping != MagickFalse)
    {
      wmf_api_destroy(API);
      (void) CloseBlob(image);
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "leave ReadWMFImage()");
      return(GetFirstImageInList(image));
    }
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
       "  Creating canvas image with size %lux%lu",(unsigned long) image->rows,
       (unsigned long) image->columns);

  /*
   * Set solid background color
   */
  {
    image->background_color = image_info->background_color;
    if (image->background_color.opacity != OpaqueOpacity)
      image->matte = MagickTrue;
    (void) SetImageBackgroundColor(image);
  }
  /*
   * Play file to generate Vector drawing commands
   *
   */

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Playing WMF to prepare vectors");

  wmf_error = wmf_play(API, 0, &bbox);
  if (wmf_error != wmf_E_None)
    {
      wmf_api_destroy(API);
      if (image->debug != MagickFalse)
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Playing WMF failed with wmf_error %d", wmf_error);
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "leave ReadWMFImage()");
        }
      ThrowReaderException(DelegateError,"FailedToRenderFile");
    }

  /*
   * Scribble on canvas image
   *
   */

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Rendering WMF vectors");
  DrawRender(ddata->draw_wand);

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"leave ReadWMFImage()");

  /* Cleanup allocated data */
  wmf_api_destroy(API);
  (void) CloseBlob(image);

  /* Return image */
  return image;
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r W M F I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterWMFImage() adds attributes for the WMF image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterWMFImage method is:
%
%      size_t RegisterWMFImage(void)
%
*/
ModuleExport size_t RegisterWMFImage(void)
{
  MagickInfo
    *entry;

  entry = SetMagickInfo("WMZ");
#if defined(MAGICKCORE_WMF_DELEGATE) || defined(MAGICKCORE_WMFLITE_DELEGATE)
  entry->decoder=ReadWMFImage;
#endif
  entry->description=ConstantString("Compressed Windows Meta File");
  entry->module=ConstantString("WMZ");
  entry->seekable_stream=MagickTrue;
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("WMF");
#if defined(MAGICKCORE_WMF_DELEGATE) || defined(MAGICKCORE_WMFLITE_DELEGATE)
  entry->decoder=ReadWMFImage;
#endif
  entry->description=ConstantString("Windows Meta File");
  entry->module=ConstantString("WMF");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r W M F I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterWMFImage() removes format registrations made by the
%  WMF module from the list of supported formats.
%
%  The format of the UnregisterWMFImage method is:
%
%      UnregisterWMFImage(void)
%
*/
ModuleExport void UnregisterWMFImage(void)
{
  (void) UnregisterMagickInfo("WMZ");
  (void) UnregisterMagickInfo("WMF");
}

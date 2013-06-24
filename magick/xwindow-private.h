/*
  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore X11 window methods.
*/
#ifndef _MAGICKCORE_XWINDOW_PRIVATE_H
#define _MAGICKCORE_XWINDOW_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/geometry.h"
#include "magick/quantize.h"

#if defined(MAGICKCORE_X11_DELEGATE)

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>

#if defined(__cplusplus) || defined(c_plusplus)
# define klass  c_class
#else
# define klass  class
#endif

/*
  Invoke pre-X11R6 ICCCM routines if XlibSpecificationRelease is not 6.
*/
#if XlibSpecificationRelease < 6
#if !defined(PRE_R6_ICCCM)
#define PRE_R6_ICCCM
#endif
#endif
/*
  Invoke pre-X11R5 ICCCM routines if XlibSpecificationRelease is not defined.
*/
#if !defined(XlibSpecificationRelease)
#define PRE_R5_ICCCM
#endif
/*
  Invoke pre-X11R4 ICCCM routines if PWinGravity is not defined.
*/
#if !defined(PWinGravity)
#define PRE_R4_ICCCM
#endif

#define MaxIconSize  96
#define MaxNumberPens  11
#define MaxNumberFonts  11
#define MaxXWindows  12
#undef index

#define ThrowXWindowException(severity,tag,context) \
{ \
  ExceptionInfo \
    exception; \
 \
  GetExceptionInfo(&exception); \
  (void) ThrowMagickException(&exception,GetMagickModule(),severity, \
    tag == (const char *) NULL ? "unknown" : tag,"`%s': %s",context, \
    strerror(errno)); \
  CatchException(&exception); \
  (void) DestroyExceptionInfo(&exception); \
}
#define ThrowXWindowFatalException(severity,tag,context) \
{ \
   ThrowXWindowException(severity,tag,context); \
  _exit(1); \
}

typedef enum
{
  ForegroundStencil,
  BackgroundStencil,
  OpaqueStencil,
  TransparentStencil
} AnnotationStencil;

typedef enum
{
  UndefinedElement,
  PointElement,
  LineElement,
  RectangleElement,
  FillRectangleElement,
  CircleElement,
  FillCircleElement,
  EllipseElement,
  FillEllipseElement,
  PolygonElement,
  FillPolygonElement,
  ColorElement,
  MatteElement,
  TextElement,
  ImageElement
} ElementType;

typedef enum
{
  UndefinedColormap,
  PrivateColormap,
  SharedColormap
} XColormapType;

typedef struct _XDrawInfo
{
  int
    x,
    y;

  unsigned int
    width,
    height;

  double
    degrees;

  AnnotationStencil
    stencil;

  ElementType
    element;

  Pixmap
    stipple;

  unsigned int
    line_width;

  XSegment
    line_info;

  unsigned int
    number_coordinates;

  RectangleInfo
    rectangle_info;

  XPoint
    *coordinate_info;

  char
    geometry[MaxTextExtent];
} XDrawInfo;

typedef enum
{
  DefaultState = 0x0000,
  EscapeState = 0x0001,
  ExitState = 0x0002,
  FormerImageState = 0x0004,
  ModifierState = 0x0008,
  MontageImageState = 0x0010,
  NextImageState = 0x0020,
  RetainColorsState = 0x0040,
  SuspendTime = 50,
  UpdateConfigurationState = 0x0080,
  UpdateRegionState = 0x0100
} XState;

typedef struct _XAnnotateInfo
{
  int
    x,
    y;

  unsigned int
    width,
    height;

  double
    degrees;

  XFontStruct
    *font_info;

  char
    *text;

  AnnotationStencil
    stencil;

  char
    geometry[MaxTextExtent];

  struct _XAnnotateInfo
    *next,
    *previous;
} XAnnotateInfo;

typedef struct _XPixelInfo
{
  ssize_t
    colors;

  unsigned long
    *pixels;

  XColor
    foreground_color,
    background_color,
    border_color,
    matte_color,
    highlight_color,
    shadow_color,
    depth_color,
    trough_color,
    box_color,
    pen_color,
    pen_colors[MaxNumberPens];

  GC
    annotate_context,
    highlight_context,
    widget_context;

  unsigned short
    box_index,
    pen_index;
} XPixelInfo;

typedef struct _XResourceInfo
{
  XrmDatabase
    resource_database;

  ImageInfo
    *image_info;

  QuantizeInfo
    *quantize_info;

  size_t
    colors;

  MagickBooleanType
    close_server,
    backdrop;

  char
    *background_color,
    *border_color;

  char
    *client_name;

  XColormapType
    colormap;

  unsigned int
    border_width;

  size_t
    delay;

  MagickBooleanType
    color_recovery,
    confirm_exit,
    confirm_edit;

  char
    *display_gamma;

  char
    *font,
    *font_name[MaxNumberFonts],
    *foreground_color;

  MagickBooleanType
    display_warnings,
    gamma_correct;

  char
    *icon_geometry;

  MagickBooleanType
    iconic,
    immutable;

  char
    *image_geometry;

  char
    *map_type,
    *matte_color,
    *name;

  unsigned int
    magnify,
    pause;

  char
    *pen_colors[MaxNumberPens];

  char
    *text_font,
    *title;

  int
    quantum;

  unsigned int
    update;

  MagickBooleanType
    use_pixmap,
    use_shared_memory;

  size_t
    undo_cache;

  char
    *visual_type,
    *window_group,
    *window_id,
    *write_filename;

  Image
    *copy_image;

  int
    gravity;

  char
    home_directory[MaxTextExtent];
} XResourceInfo;

typedef struct _XWindowInfo
{
  Window
    id;

  Window
    root;

  Visual
    *visual;

  unsigned int
    storage_class,
    depth;

  XVisualInfo
    *visual_info;

  XStandardColormap
    *map_info;

  XPixelInfo
    *pixel_info;

  XFontStruct
    *font_info;

  GC
    annotate_context,
    highlight_context,
    widget_context;

  Cursor
    cursor,
    busy_cursor;

  char
    *name,
    *geometry,
    *icon_name,
    *icon_geometry,
    *crop_geometry;

  size_t
    data,
    flags;

  int
    x,
    y;

  unsigned int
    width,
    height,
    min_width,
    min_height,
    width_inc,
    height_inc,
    border_width;

  MagickBooleanType
    use_pixmap,
    immutable,
    shape,
    shared_memory;

  int
    screen;

  XImage
    *ximage,
    *matte_image;

  Pixmap
    highlight_stipple,
    shadow_stipple,
    pixmap,
    *pixmaps,
    matte_pixmap,
    *matte_pixmaps;

  XSetWindowAttributes
    attributes;

  XWindowChanges
    window_changes;

  void
    *segment_info;

  long
    mask;

  MagickBooleanType
    orphan,
    mapped,
    stasis;

  Image
    *image;

  MagickBooleanType
    destroy;
} XWindowInfo;

typedef struct _XWindows
{
  Display
    *display;

  XStandardColormap
    *map_info,
    *icon_map;

  XVisualInfo
    *visual_info,
    *icon_visual;

  XPixelInfo
    *pixel_info,
    *icon_pixel;

  XFontStruct
    *font_info;

  XResourceInfo
    *icon_resources;

  XClassHint
    *class_hints;

  XWMHints
    *manager_hints;

  XWindowInfo
    context,
    group_leader,
    backdrop,
    icon,
    image,
    info,
    magnify,
    pan,
    command,
    widget,
    popup;

  Atom
    wm_protocols,
    wm_delete_window,
    wm_take_focus,
    im_protocols,
    im_remote_command,
    im_update_widget,
    im_update_colormap,
    im_former_image,
    im_retain_colors,
    im_next_image,
    im_exit,
    dnd_protocols;
} XWindows;

extern MagickExport char
  *XGetResourceClass(XrmDatabase,const char *,const char *,char *),
  *XGetResourceInstance(XrmDatabase,const char *,const char *,const char *),
  *XGetScreenDensity(Display *);

extern MagickExport Cursor
  XMakeCursor(Display *,Window,Colormap,char *,char *);

extern MagickExport int
  XCheckDefineCursor(Display *,Window,Cursor),
  XError(Display *,XErrorEvent *);

extern MagickExport MagickBooleanType
  XAnnotateImage(Display *,const XPixelInfo *,XAnnotateInfo *,Image *),
  XComponentGenesis(void),
  XDrawImage(Display *,const XPixelInfo *,XDrawInfo *,Image *),
  XGetWindowColor(Display *,XWindows *,char *),
  XMagickProgressMonitor(const char *,const MagickOffsetType,
    const MagickSizeType,void *),
  XMakeImage(Display *,const XResourceInfo *,XWindowInfo *,Image *,unsigned int,
    unsigned int),
  XQueryColorDatabase(const char *,XColor *),
  XRemoteCommand(Display *,const char *,const char *);

extern MagickExport void
  DestroyXResources(void),
  XBestIconSize(Display *,XWindowInfo *,Image *),
  XBestPixel(Display *,const Colormap,XColor *,unsigned int,XColor *),
  XCheckRefreshWindows(Display *,XWindows *),
  XClientMessage(Display *,const Window,const Atom,const Atom,const Time),
  XComponentTerminus(void),
  XConfigureImageColormap(Display *,XResourceInfo *,XWindows *,Image *),
  XConstrainWindowPosition(Display *,XWindowInfo *),
  XDelay(Display *,const size_t),
  XDisplayImageInfo(Display *,const XResourceInfo *,XWindows *,Image *,Image *),
  XDestroyResourceInfo(XResourceInfo *),
  XDestroyWindowColors(Display *,Window),
  XFreeResources(Display *,XVisualInfo *,XStandardColormap *,XPixelInfo *,
    XFontStruct *,XResourceInfo *,XWindowInfo *),
  XFreeStandardColormap(Display *,const XVisualInfo *,XStandardColormap *,
    XPixelInfo *),
  XHighlightEllipse(Display *,Window,GC,const RectangleInfo *),
  XHighlightLine(Display *,Window,GC,const XSegment *),
  XHighlightRectangle(Display *,Window,GC,const RectangleInfo *),
  XGetAnnotateInfo(XAnnotateInfo *),
  XGetPixelPacket(Display *,const XVisualInfo *,const XStandardColormap *,
    const XResourceInfo *,Image *,XPixelInfo *),
  XGetMapInfo(const XVisualInfo *,const Colormap,XStandardColormap *),
  XGetResourceInfo(const ImageInfo *,XrmDatabase,const char *,XResourceInfo *),
  XGetWindowInfo(Display *,XVisualInfo *,XStandardColormap *,XPixelInfo *,
    XFontStruct *,XResourceInfo *,XWindowInfo *),
  XMakeMagnifyImage(Display *,XWindows *),
  XMakeStandardColormap(Display *,XVisualInfo *,XResourceInfo *,Image *,
    XStandardColormap *,XPixelInfo *),
  XMakeWindow(Display *,Window,char **,int,XClassHint *,XWMHints *,
    XWindowInfo *),
  XQueryPosition(Display *,const Window,int *,int *),
  XRefreshWindow(Display *,const XWindowInfo *,const XEvent *),
  XRetainWindowColors(Display *,const Window),
  XSetCursorState(Display *,XWindows *,const MagickStatusType),
  XUserPreferences(XResourceInfo *),
  XWarning(const ExceptionType,const char *,const char *);

extern MagickExport Window
  XWindowByID(Display *,const Window,const size_t),
  XWindowByName(Display *,const Window,const char *),
  XWindowByProperty(Display *,const Window,const Atom);

extern MagickExport XFontStruct
  *XBestFont(Display *,const XResourceInfo *,const MagickBooleanType);

extern MagickExport XrmDatabase
  XGetResourceDatabase(Display *,const char *);

extern MagickExport XVisualInfo
  *XBestVisualInfo(Display *,XStandardColormap *,XResourceInfo *);

extern MagickExport XWindows
  *XInitializeWindows(Display *,XResourceInfo *),
  *XSetWindows(XWindows *);

static inline MagickRealType XPixelIntensity(const XColor *pixel)
{
  MagickRealType
    intensity;

  if ((pixel->red  == pixel->green) && (pixel->green == pixel->blue))
    return((MagickRealType) pixel->red);
  intensity=0.21265*pixel->red+0.715158*pixel->green+0.072186*pixel->blue;
  return(intensity);
}
#endif

extern MagickPrivate MagickBooleanType
  XRenderImage(Image *,const DrawInfo *,const PointInfo *,TypeMetric *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

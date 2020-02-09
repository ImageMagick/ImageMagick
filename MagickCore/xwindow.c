/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%              X   X  W   W  IIIII  N   N  DDDD    OOO   W   W                %
%               X X   W   W    I    NN  N  D   D  O   O  W   W                %
%                X    W   W    I    N N N  D   D  O   O  W   W                %
%               X X   W W W    I    N  NN  D   D  O   O  W W W                %
%              X   X   W W   IIIII  N   N  DDDD    OOO    W W                 %
%                                                                             %
%                                                                             %
%                       MagickCore X11 Utility Methods                        %
%                                                                             %
%                               Software Design                               %
%                                    Cristy                                   %
%                                  July 1992                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/animate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/cache.h"
#include "MagickCore/client.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/composite.h"
#include "MagickCore/constitute.h"
#include "MagickCore/display.h"
#include "MagickCore/distort.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/identify.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/locale_.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/resize.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/transform.h"
#include "MagickCore/transform-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/widget.h"
#include "MagickCore/widget-private.h"
#include "MagickCore/xwindow.h"
#include "MagickCore/xwindow-private.h"
#include "MagickCore/version.h"
#if defined(__BEOS__)
#include <OS.h>
#endif
#if defined(MAGICKCORE_X11_DELEGATE)
#include <X11/Xproto.h>
#include <X11/Xlocale.h>
#if defined(MAGICK_HAVE_POLL)
# include <sys/poll.h>
#endif
#if defined(MAGICKCORE_HAVE_SHARED_MEMORY)
#if defined(MAGICKCORE_HAVE_MACHINE_PARAM_H)
# include <machine/param.h>
#endif
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif
#if defined(MAGICKCORE_HAVE_SHAPE)
#include <X11/extensions/shape.h>
#endif

/*
  X defines.
*/
#define XBlueGamma(color) ClampToQuantum(blue_gamma == 1.0 ? (double) \
  (color) : ((pow(((double) QuantumScale*(color)),1.0/(double) blue_gamma)* \
  QuantumRange)))
#define XGammaPacket(map,color)  (size_t) (map->base_pixel+ \
  ((ScaleQuantumToShort(XRedGamma((color)->red))*map->red_max/65535L)* \
    map->red_mult)+ \
  ((ScaleQuantumToShort(XGreenGamma((color)->green))*map->green_max/65535L)* \
    map->green_mult)+ \
  ((ScaleQuantumToShort(XBlueGamma((color)->blue))*map->blue_max/65535L)* \
    map->blue_mult))
#define XGammaPixel(image,map,color)  (size_t) (map->base_pixel+ \
  ((ScaleQuantumToShort(XRedGamma(GetPixelRed(image,color)))*map->red_max/65535L)* \
    map->red_mult)+ \
  ((ScaleQuantumToShort(XGreenGamma(GetPixelGreen(image,color)))*map->green_max/65535L)* \
    map->green_mult)+ \
  ((ScaleQuantumToShort(XBlueGamma(GetPixelBlue(image,color)))*map->blue_max/65535L)* \
    map->blue_mult))
#define XGreenGamma(color) ClampToQuantum(green_gamma == 1.0 ? (double) \
  (color) : ((pow(((double) QuantumScale*(color)),1.0/(double) green_gamma)* \
  QuantumRange)))
#define XRedGamma(color) ClampToQuantum(red_gamma == 1.0 ? (double) \
  (color) : ((pow(((double) QuantumScale*(color)),1.0/(double) red_gamma)* \
  QuantumRange)))
#define XStandardPixel(map,color)  (size_t) (map->base_pixel+ \
  (((color)->red*map->red_max/65535L)*map->red_mult)+ \
  (((color)->green*map->green_max/65535L)*map->green_mult)+ \
  (((color)->blue*map->blue_max/65535L)*map->blue_mult))

#define AccentuateModulate  ScaleCharToQuantum(80)
#define HighlightModulate  ScaleCharToQuantum(125)
#define ShadowModulate  ScaleCharToQuantum(135)
#define DepthModulate  ScaleCharToQuantum(185)
#define TroughModulate  ScaleCharToQuantum(110)

#define XLIB_ILLEGAL_ACCESS  1
#undef ForgetGravity
#undef NorthWestGravity
#undef NorthGravity
#undef NorthEastGravity
#undef WestGravity
#undef CenterGravity
#undef EastGravity
#undef SouthWestGravity
#undef SouthGravity
#undef SouthEastGravity
#undef StaticGravity

#undef index
#if defined(hpux9)
#define XFD_SET  int
#else
#define XFD_SET  fd_set
#endif

/*
  Enumeration declarations.
*/
typedef enum
{
#undef DoRed
  DoRed = 0x0001,
#undef DoGreen
  DoGreen = 0x0002,
#undef DoBlue
  DoBlue = 0x0004,
  DoMatte = 0x0008
} XColorFlags;

/*
  Typedef declarations.
*/
typedef struct _DiversityPacket
{
  Quantum
    red,
    green,
    blue;

  unsigned short
    index;

  size_t
    count;
} DiversityPacket;

/*
  Constant declaractions.
*/
static MagickBooleanType
  xerror_alert = MagickFalse;

/*
  Method prototypes.
*/
static const char
  *XVisualClassName(const int);

static double
  blue_gamma = 1.0,
  green_gamma = 1.0,
  red_gamma = 1.0;

static MagickBooleanType
  XMakePixmap(Display *,const XResourceInfo *,XWindowInfo *);

static void
  XMakeImageLSBFirst(const XResourceInfo *,const XWindowInfo *,Image *,
    XImage *,XImage *,ExceptionInfo *),
  XMakeImageMSBFirst(const XResourceInfo *,const XWindowInfo *,Image *,
    XImage *,XImage *,ExceptionInfo *);

static Window
  XSelectWindow(Display *,RectangleInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y X R e s o u r c e s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyXResources() destroys any X resources.
%
%  The format of the DestroyXResources method is:
%
%      void DestroyXResources()
%
%  A description of each parameter follows:
%
*/
MagickExport void DestroyXResources(void)
{
  register int
    i;

  unsigned int
    number_windows;

  XWindowInfo
    *magick_windows[MaxXWindows];

  XWindows
    *windows;

  DestroyXWidget();
  windows=XSetWindows((XWindows *) ~0);
  if ((windows == (XWindows *) NULL) || (windows->display == (Display *) NULL))
    return;
  number_windows=0;
  magick_windows[number_windows++]=(&windows->context);
  magick_windows[number_windows++]=(&windows->group_leader);
  magick_windows[number_windows++]=(&windows->backdrop);
  magick_windows[number_windows++]=(&windows->icon);
  magick_windows[number_windows++]=(&windows->image);
  magick_windows[number_windows++]=(&windows->info);
  magick_windows[number_windows++]=(&windows->magnify);
  magick_windows[number_windows++]=(&windows->pan);
  magick_windows[number_windows++]=(&windows->command);
  magick_windows[number_windows++]=(&windows->widget);
  magick_windows[number_windows++]=(&windows->popup);
  for (i=0; i < (int) number_windows; i++)
  {
    if (magick_windows[i]->mapped != MagickFalse)
      {
        (void) XWithdrawWindow(windows->display,magick_windows[i]->id,
          magick_windows[i]->screen);
        magick_windows[i]->mapped=MagickFalse;
      }
    if (magick_windows[i]->name != (char *) NULL)
      magick_windows[i]->name=(char *)
        RelinquishMagickMemory(magick_windows[i]->name);
    if (magick_windows[i]->icon_name != (char *) NULL)
      magick_windows[i]->icon_name=(char *)
        RelinquishMagickMemory(magick_windows[i]->icon_name);
    if (magick_windows[i]->cursor != (Cursor) NULL)
      {
        (void) XFreeCursor(windows->display,magick_windows[i]->cursor);
        magick_windows[i]->cursor=(Cursor) NULL;
      }
    if (magick_windows[i]->busy_cursor != (Cursor) NULL)
      {
        (void) XFreeCursor(windows->display,magick_windows[i]->busy_cursor);
        magick_windows[i]->busy_cursor=(Cursor) NULL;
      }
    if (magick_windows[i]->highlight_stipple != (Pixmap) NULL)
      {
        (void) XFreePixmap(windows->display,
          magick_windows[i]->highlight_stipple);
        magick_windows[i]->highlight_stipple=(Pixmap) NULL;
      }
    if (magick_windows[i]->shadow_stipple != (Pixmap) NULL)
      {
        (void) XFreePixmap(windows->display,magick_windows[i]->shadow_stipple);
        magick_windows[i]->shadow_stipple=(Pixmap) NULL;
      }
    if (magick_windows[i]->matte_image != (XImage *) NULL)
      {
        XDestroyImage(magick_windows[i]->matte_image);
        magick_windows[i]->matte_image=(XImage *) NULL;
      }
    if (magick_windows[i]->ximage != (XImage *) NULL)
      {
        XDestroyImage(magick_windows[i]->ximage);
        magick_windows[i]->ximage=(XImage *) NULL;
      }
    if (magick_windows[i]->pixmap != (Pixmap) NULL)
      {
        (void) XFreePixmap(windows->display,magick_windows[i]->pixmap);
        magick_windows[i]->pixmap=(Pixmap) NULL;
      }
    if (magick_windows[i]->id != (Window) NULL)
      {
        (void) XDestroyWindow(windows->display,magick_windows[i]->id);
        magick_windows[i]->id=(Window) NULL;
      }
    if (magick_windows[i]->destroy != MagickFalse)
      {
        if (magick_windows[i]->image != (Image *) NULL)
          {
            magick_windows[i]->image=DestroyImage(magick_windows[i]->image);
            magick_windows[i]->image=NewImageList();
          }
        if (magick_windows[i]->matte_pixmap != (Pixmap) NULL)
          {
            (void) XFreePixmap(windows->display,
              magick_windows[i]->matte_pixmap);
            magick_windows[i]->matte_pixmap=(Pixmap) NULL;
          }
      }
    if (magick_windows[i]->segment_info != (void *) NULL)
      {
#if defined(MAGICKCORE_HAVE_SHARED_MEMORY)
        XShmSegmentInfo
          *segment_info;

        segment_info=(XShmSegmentInfo *) magick_windows[i]->segment_info;
        if (segment_info != (XShmSegmentInfo *) NULL)
          if (segment_info[0].shmid >= 0)
            {
              if (segment_info[0].shmaddr != NULL)
                (void) shmdt(segment_info[0].shmaddr);
              (void) shmctl(segment_info[0].shmid,IPC_RMID,0);
              segment_info[0].shmaddr=NULL;
              segment_info[0].shmid=(-1);
            }
#endif
        magick_windows[i]->segment_info=(void *)
          RelinquishMagickMemory(magick_windows[i]->segment_info);
      }
  }
  windows->icon_resources=(XResourceInfo *)
    RelinquishMagickMemory(windows->icon_resources);
  if (windows->icon_pixel != (XPixelInfo *) NULL)
    {
      if (windows->icon_pixel->pixels != (unsigned long *) NULL)
        windows->icon_pixel->pixels=(unsigned long *)
          RelinquishMagickMemory(windows->icon_pixel->pixels);
      if (windows->icon_pixel->annotate_context != (GC) NULL)
        XFreeGC(windows->display,windows->icon_pixel->annotate_context);
      windows->icon_pixel=(XPixelInfo *)
        RelinquishMagickMemory(windows->icon_pixel);
    }
  if (windows->pixel_info != (XPixelInfo *) NULL)
    {
      if (windows->pixel_info->pixels != (unsigned long *) NULL)
        windows->pixel_info->pixels=(unsigned long *)
          RelinquishMagickMemory(windows->pixel_info->pixels);
      if (windows->pixel_info->annotate_context != (GC) NULL)
        XFreeGC(windows->display,windows->pixel_info->annotate_context);
      if (windows->pixel_info->widget_context != (GC) NULL)
        XFreeGC(windows->display,windows->pixel_info->widget_context);
      if (windows->pixel_info->highlight_context != (GC) NULL)
        XFreeGC(windows->display,windows->pixel_info->highlight_context);
      windows->pixel_info=(XPixelInfo *)
        RelinquishMagickMemory(windows->pixel_info);
    }
  if (windows->font_info != (XFontStruct *) NULL)
    {
      XFreeFont(windows->display,windows->font_info);
      windows->font_info=(XFontStruct *) NULL;
    }
  if (windows->class_hints != (XClassHint *) NULL)
    {
      if (windows->class_hints->res_name != (char *) NULL)
        windows->class_hints->res_name=DestroyString(
          windows->class_hints->res_name);
      if (windows->class_hints->res_class != (char *) NULL)
        windows->class_hints->res_class=DestroyString(
          windows->class_hints->res_class);
      XFree(windows->class_hints);
      windows->class_hints=(XClassHint *) NULL;
    }
  if (windows->manager_hints != (XWMHints *) NULL)
    {
      XFree(windows->manager_hints);
      windows->manager_hints=(XWMHints *) NULL;
    }
  if (windows->map_info != (XStandardColormap *) NULL)
    {
      XFree(windows->map_info);
      windows->map_info=(XStandardColormap *) NULL;
    }
  if (windows->icon_map != (XStandardColormap *) NULL)
    {
      XFree(windows->icon_map);
      windows->icon_map=(XStandardColormap *) NULL;
    }
  if (windows->visual_info != (XVisualInfo *) NULL)
    {
      XFree(windows->visual_info);
      windows->visual_info=(XVisualInfo *) NULL;
    }
  if (windows->icon_visual != (XVisualInfo *) NULL)
    {
      XFree(windows->icon_visual);
      windows->icon_visual=(XVisualInfo *) NULL;
    }
  (void) XSetWindows((XWindows *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X A n n o t a t e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XAnnotateImage() annotates the image with text.
%
%  The format of the XAnnotateImage method is:
%
%      MagickBooleanType XAnnotateImage(Display *display,
%        const XPixelInfo *pixel,XAnnotateInfo *annotate_info,Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o pixel: Specifies a pointer to a XPixelInfo structure.
%
%    o annotate_info: Specifies a pointer to a XAnnotateInfo structure.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate MagickBooleanType XAnnotateImage(Display *display,
  const XPixelInfo *pixel,XAnnotateInfo *annotate_info,Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *annotate_view;

  GC
    annotate_context;

  Image
    *annotate_image;

  int
    x,
    y;

  PixelTrait
    alpha_trait;

  Pixmap
    annotate_pixmap;

  unsigned int
    depth,
    height,
    width;

  Window
    root_window;

  XGCValues
    context_values;

  XImage
    *annotate_ximage;

  /*
    Initialize annotated image.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(pixel != (XPixelInfo *) NULL);
  assert(annotate_info != (XAnnotateInfo *) NULL);
  assert(image != (Image *) NULL);
  /*
    Initialize annotated pixmap.
  */
  root_window=XRootWindow(display,XDefaultScreen(display));
  depth=(unsigned int) XDefaultDepth(display,XDefaultScreen(display));
  annotate_pixmap=XCreatePixmap(display,root_window,annotate_info->width,
    annotate_info->height,depth);
  if (annotate_pixmap == (Pixmap) NULL)
    return(MagickFalse);
  /*
    Initialize graphics info.
  */
  context_values.background=0;
  context_values.foreground=(size_t) (~0);
  context_values.font=annotate_info->font_info->fid;
  annotate_context=XCreateGC(display,root_window,(unsigned long)
    (GCBackground | GCFont | GCForeground),&context_values);
  if (annotate_context == (GC) NULL)
    return(MagickFalse);
  /*
    Draw text to pixmap.
  */
  (void) XDrawImageString(display,annotate_pixmap,annotate_context,0,
    (int) annotate_info->font_info->ascent,annotate_info->text,
    (int) strlen(annotate_info->text));
  (void) XFreeGC(display,annotate_context);
  /*
    Initialize annotated X image.
  */
  annotate_ximage=XGetImage(display,annotate_pixmap,0,0,annotate_info->width,
    annotate_info->height,AllPlanes,ZPixmap);
  if (annotate_ximage == (XImage *) NULL)
    return(MagickFalse);
  (void) XFreePixmap(display,annotate_pixmap);
  /*
    Initialize annotated image.
  */
  annotate_image=AcquireImage((ImageInfo *) NULL,exception);
  if (annotate_image == (Image *) NULL)
    return(MagickFalse);
  annotate_image->columns=annotate_info->width;
  annotate_image->rows=annotate_info->height;
  /*
    Transfer annotated X image to image.
  */
  width=(unsigned int) image->columns;
  height=(unsigned int) image->rows;
  x=0;
  y=0;
  (void) XParseGeometry(annotate_info->geometry,&x,&y,&width,&height);
  (void) GetOneVirtualPixelInfo(image,UndefinedVirtualPixelMethod,(ssize_t) x,
    (ssize_t) y,&annotate_image->background_color,exception);
  if (annotate_info->stencil == ForegroundStencil)
    annotate_image->alpha_trait=BlendPixelTrait;
  annotate_view=AcquireAuthenticCacheView(annotate_image,exception);
  for (y=0; y < (int) annotate_image->rows; y++)
  {
    register int
      x;

    register Quantum
      *magick_restrict q;

    q=GetCacheViewAuthenticPixels(annotate_view,0,(ssize_t) y,
      annotate_image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (int) annotate_image->columns; x++)
    {
      SetPixelAlpha(annotate_image,OpaqueAlpha,q);
      if (XGetPixel(annotate_ximage,x,y) == 0)
        {
          /*
            Set this pixel to the background color.
          */
          SetPixelRed(annotate_image,ScaleShortToQuantum(
            pixel->box_color.red),q);
          SetPixelGreen(annotate_image,ScaleShortToQuantum(
            pixel->box_color.green),q);
          SetPixelBlue(annotate_image,ScaleShortToQuantum(
            pixel->box_color.blue),q);
          if ((annotate_info->stencil == ForegroundStencil) ||
              (annotate_info->stencil == OpaqueStencil))
            SetPixelAlpha(annotate_image,TransparentAlpha,q);
        }
      else
        {
          /*
            Set this pixel to the pen color.
          */
          SetPixelRed(annotate_image,ScaleShortToQuantum(
            pixel->pen_color.red),q);
          SetPixelGreen(annotate_image,ScaleShortToQuantum(
            pixel->pen_color.green),q);
          SetPixelBlue(annotate_image,ScaleShortToQuantum(
            pixel->pen_color.blue),q);
          if (annotate_info->stencil == BackgroundStencil)
            SetPixelAlpha(annotate_image,TransparentAlpha,q);
        }
      q+=GetPixelChannels(annotate_image);
    }
    if (SyncCacheViewAuthenticPixels(annotate_view,exception) == MagickFalse)
      break;
  }
  annotate_view=DestroyCacheView(annotate_view);
  XDestroyImage(annotate_ximage);
  /*
    Determine annotate geometry.
  */
  (void) XParseGeometry(annotate_info->geometry,&x,&y,&width,&height);
  if ((width != (unsigned int) annotate_image->columns) ||
      (height != (unsigned int) annotate_image->rows))
    {
      char
        image_geometry[MagickPathExtent];

      /*
        Scale image.
      */
      (void) FormatLocaleString(image_geometry,MagickPathExtent,"%ux%u",
        width,height);
      (void) TransformImage(&annotate_image,(char *) NULL,image_geometry,
        exception);
    }
  if (annotate_info->degrees != 0.0)
    {
      Image
        *rotate_image;

      int
        rotations;

      double
        normalized_degrees;

      /*
        Rotate image.
      */
      rotate_image=RotateImage(annotate_image,annotate_info->degrees,exception);
      if (rotate_image == (Image *) NULL)
        return(MagickFalse);
      annotate_image=DestroyImage(annotate_image);
      annotate_image=rotate_image;
      /*
        Annotation is relative to the degree of rotation.
      */
      normalized_degrees=annotate_info->degrees;
      while (normalized_degrees < -45.0)
        normalized_degrees+=360.0;
      for (rotations=0; normalized_degrees > 45.0; rotations++)
        normalized_degrees-=90.0;
      switch (rotations % 4)
      {
        default:
        case 0:
          break;
        case 1:
        {
          /*
            Rotate 90 degrees.
          */
          x-=(int) annotate_image->columns/2;
          y+=(int) annotate_image->columns/2;
          break;
        }
        case 2:
        {
          /*
            Rotate 180 degrees.
          */
          x=x-(int) annotate_image->columns;
          break;
        }
        case 3:
        {
          /*
            Rotate 270 degrees.
          */
          x=x-(int) annotate_image->columns/2;
          y=y-(int) (annotate_image->rows-(annotate_image->columns/2));
          break;
        }
      }
    }
  /*
    Composite text onto the image.
  */
  (void) XParseGeometry(annotate_info->geometry,&x,&y,&width,&height);
  alpha_trait=image->alpha_trait;
  (void) CompositeImage(image,annotate_image,
    annotate_image->alpha_trait != UndefinedPixelTrait ? OverCompositeOp :
    CopyCompositeOp,MagickTrue,(ssize_t) x,(ssize_t) y,exception);
  image->alpha_trait=alpha_trait;
  annotate_image=DestroyImage(annotate_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X B e s t F o n t                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XBestFont() returns the "best" font.  "Best" is defined as a font specified
%  in the X resource database or a font such that the text width displayed
%  with the font does not exceed the specified maximum width.
%
%  The format of the XBestFont method is:
%
%      XFontStruct *XBestFont(Display *display,
%        const XResourceInfo *resource_info,const MagickBooleanType text_font)
%
%  A description of each parameter follows:
%
%    o font: XBestFont returns a pointer to a XFontStruct structure.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o text_font:  True is font should be mono-spaced (typewriter style).
%
*/

static char **FontToList(char *font)
{
  char
    **fontlist;

  register char
    *p,
    *q;

  register int
    i;

  unsigned int
    fonts;

  if (font == (char *) NULL)
    return((char **) NULL);
  /*
    Convert string to an ASCII list.
  */
  fonts=1U;
  for (p=font; *p != '\0'; p++)
    if ((*p == ':') || (*p == ';') || (*p == ','))
      fonts++;
  fontlist=(char **) AcquireQuantumMemory((size_t) fonts+1UL,sizeof(*fontlist));
  if (fontlist == (char **) NULL)
    {
      ThrowXWindowException(ResourceLimitError,"MemoryAllocationFailed",font);
      return((char **) NULL);
    }
  p=font;
  for (i=0; i < (int) fonts; i++)
  {
    for (q=p; *q != '\0'; q++)
      if ((*q == ':') || (*q == ';') || (*q == ','))
        break;
    fontlist[i]=(char *) AcquireQuantumMemory((size_t) (q-p)+1UL,
      sizeof(*fontlist[i]));
    if (fontlist[i] == (char *) NULL)
      {
        ThrowXWindowException(ResourceLimitError,"MemoryAllocationFailed",font);
        return((char **) NULL);
      }
    (void) CopyMagickString(fontlist[i],p,(size_t) (q-p+1));
    p=q+1;
  }
  fontlist[i]=(char *) NULL;
  return(fontlist);
}

MagickPrivate XFontStruct *XBestFont(Display *display,
  const XResourceInfo *resource_info,const MagickBooleanType text_font)
{
  static const char
    *Fonts[]=
    {
      "-*-helvetica-medium-r-normal--12-*-*-*-*-*-iso8859-1",
      "-*-arial-medium-r-normal--12-*-*-*-*-*-iso8859-1",
      "-*-helvetica-medium-r-normal--12-*-*-*-*-*-iso8859-15",
      "-*-arial-medium-r-normal--12-*-*-*-*-*-iso8859-15",
      "-*-helvetica-medium-r-normal--12-*-*-*-*-*-*-*",
      "-*-arial-medium-r-normal--12-*-*-*-*-*-*-*",
      "variable",
      "fixed",
      (char *) NULL
    },
    *TextFonts[]=
    {
      "-*-courier-medium-r-normal-*-12-*-*-*-*-*-iso8859-1",
      "-*-courier-medium-r-normal-*-12-*-*-*-*-*-iso8859-15",
      "-*-fixed-medium-r-normal-*-12-*-*-*-*-*-*-*",
      "fixed",
      (char *) NULL
    };

  char
    *font_name;

  register const char
    **p;

  XFontStruct
    *font_info;

  font_info=(XFontStruct *) NULL;
  font_name=resource_info->font;
  if (text_font != MagickFalse)
    font_name=resource_info->text_font;
  if ((font_name != (char *) NULL) && (*font_name != '\0'))
    {
      char
        **fontlist;

      register int
        i;

      /*
        Load preferred font specified in the X resource database.
      */
      fontlist=FontToList(font_name);
      if (fontlist != (char **) NULL)
        {
          for (i=0; fontlist[i] != (char *) NULL; i++)
          {
            if (font_info == (XFontStruct *) NULL)
              font_info=XLoadQueryFont(display,fontlist[i]);
            fontlist[i]=DestroyString(fontlist[i]);
          }
          fontlist=(char **) RelinquishMagickMemory(fontlist);
        }
      if (font_info == (XFontStruct *) NULL)
        ThrowXWindowException(XServerError,"UnableToLoadFont",font_name);
    }
  /*
    Load fonts from list of fonts until one is found.
  */
  p=Fonts;
  if (text_font != MagickFalse)
    p=TextFonts;
  if (XDisplayHeight(display,XDefaultScreen(display)) >= 748)
    p++;
  while (*p != (char *) NULL)
  {
    if (font_info != (XFontStruct *) NULL)
      break;
    font_info=XLoadQueryFont(display,(char *) *p);
    p++;
  }
  return(font_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X B e s t I c o n S i z e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XBestIconSize() returns the "best" icon size.  "Best" is defined as an icon
%  size that maintains the aspect ratio of the image.  If the window manager
%  has preferred icon sizes, one of the preferred sizes is used.
%
%  The format of the XBestIconSize method is:
%
%      void XBestIconSize(Display *display,XWindowInfo *window,Image *image)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o image: the image.
%
*/
MagickPrivate void XBestIconSize(Display *display,XWindowInfo *window,
  Image *image)
{
  int
    i,
    number_sizes;

  double
    scale_factor;

  unsigned int
    height,
    icon_height,
    icon_width,
    width;

  Window
    root_window;

  XIconSize
    *icon_size,
    *size_list;

  /*
    Determine if the window manager has specified preferred icon sizes.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(window != (XWindowInfo *) NULL);
  assert(image != (Image *) NULL);
  window->width=MaxIconSize;
  window->height=MaxIconSize;
  icon_size=(XIconSize *) NULL;
  number_sizes=0;
  root_window=XRootWindow(display,window->screen);
  if (XGetIconSizes(display,root_window,&size_list,&number_sizes) != 0)
    if ((number_sizes > 0) && (size_list != (XIconSize *) NULL))
      icon_size=size_list;
  if (icon_size == (XIconSize *) NULL)
    {
      /*
        Window manager does not restrict icon size.
      */
      icon_size=XAllocIconSize();
      if (icon_size == (XIconSize *) NULL)
        {
          ThrowXWindowException(ResourceLimitError,"MemoryAllocationFailed",
            image->filename);
          return;
        }
      icon_size->min_width=1;
      icon_size->max_width=MaxIconSize;
      icon_size->min_height=1;
      icon_size->max_height=MaxIconSize;
      icon_size->width_inc=1;
      icon_size->height_inc=1;
    }
  /*
    Determine aspect ratio of image.
  */
  width=(unsigned int) image->columns;
  height=(unsigned int) image->rows;
  i=0;
  if (window->crop_geometry)
    (void) XParseGeometry(window->crop_geometry,&i,&i,&width,&height);
  /*
    Look for an icon size that maintains the aspect ratio of image.
  */
  scale_factor=(double) icon_size->max_width/width;
  if (scale_factor > ((double) icon_size->max_height/height))
    scale_factor=(double) icon_size->max_height/height;
  icon_width=(unsigned int) icon_size->min_width;
  while ((int) icon_width < icon_size->max_width)
  {
    if (icon_width >= (unsigned int) (scale_factor*width+0.5))
      break;
    icon_width+=icon_size->width_inc;
  }
  icon_height=(unsigned int) icon_size->min_height;
  while ((int) icon_height < icon_size->max_height)
  {
    if (icon_height >= (unsigned int) (scale_factor*height+0.5))
      break;
    icon_height+=icon_size->height_inc;
  }
  (void) XFree((void *) icon_size);
  window->width=icon_width;
  window->height=icon_height;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X B e s t P i x e l                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XBestPixel() returns a pixel from an array of pixels that is closest to the
%  requested color.  If the color array is NULL, the colors are obtained from
%  the X server.
%
%  The format of the XBestPixel method is:
%
%      void XBestPixel(Display *display,const Colormap colormap,XColor *colors,
%        unsigned int number_colors,XColor *color)
%
%  A description of each parameter follows:
%
%    o pixel: XBestPixel returns the pixel value closest to the requested
%      color.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o colormap: Specifies the ID of the X server colormap.
%
%    o colors: Specifies an array of XColor structures.
%
%    o number_colors: Specifies the number of XColor structures in the
%      color definition array.
%
%    o color: Specifies the desired RGB value to find in the colors array.
%
*/
MagickPrivate void XBestPixel(Display *display,const Colormap colormap,
  XColor *colors,unsigned int number_colors,XColor *color)
{
  MagickBooleanType
    query_server;

  PixelInfo
    pixel;

  double
    min_distance;

  register double
    distance;

  register int
    i,
    j;

  Status
    status;

  /*
    Find closest representation for the requested RGB color.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(color != (XColor *) NULL);
  status=XAllocColor(display,colormap,color);
  if (status != False)
    return;
  query_server=colors == (XColor *) NULL ? MagickTrue : MagickFalse;
  if (query_server != MagickFalse)
    {
      /*
        Read X server colormap.
      */
      colors=(XColor *) AcquireQuantumMemory(number_colors,sizeof(*colors));
      if (colors == (XColor *) NULL)
        {
          ThrowXWindowException(ResourceLimitError,"MemoryAllocationFailed",
            "...");
          return;
        }
      for (i=0; i < (int) number_colors; i++)
        colors[i].pixel=(size_t) i;
      if (number_colors > 256)
        number_colors=256;
      (void) XQueryColors(display,colormap,colors,(int) number_colors);
    }
  min_distance=3.0*((double) QuantumRange+1.0)*((double)
    QuantumRange+1.0);
  j=0;
  for (i=0; i < (int) number_colors; i++)
  {
    pixel.red=colors[i].red-(double) color->red;
    distance=pixel.red*pixel.red;
    if (distance > min_distance)
      continue;
    pixel.green=colors[i].green-(double) color->green;
    distance+=pixel.green*pixel.green;
    if (distance > min_distance)
      continue;
    pixel.blue=colors[i].blue-(double) color->blue;
    distance+=pixel.blue*pixel.blue;
    if (distance > min_distance)
      continue;
    min_distance=distance;
    color->pixel=colors[i].pixel;
    j=i;
  }
  (void) XAllocColor(display,colormap,&colors[j]);
  if (query_server != MagickFalse)
    colors=(XColor *) RelinquishMagickMemory(colors);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X B e s t V i s u a l I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XBestVisualInfo() returns visual information for a visual that is the "best"
%  the server supports.  "Best" is defined as:
%
%    1. Restrict the visual list to those supported by the default screen.
%
%    2. If a visual type is specified, restrict the visual list to those of
%       that type.
%
%    3. If a map type is specified, choose the visual that matches the id
%       specified by the Standard Colormap.
%
%    4  From the list of visuals, choose one that can display the most
%       simultaneous colors.  If more than one visual can display the same
%       number of simultaneous colors, one is chosen based on a rank.
%
%  The format of the XBestVisualInfo method is:
%
%      XVisualInfo *XBestVisualInfo(Display *display,
%        XStandardColormap *map_info,XResourceInfo *resource_info)
%
%  A description of each parameter follows:
%
%    o visual_info: XBestVisualInfo returns a pointer to a X11 XVisualInfo
%      structure.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o map_info: If map_type is specified, this structure is initialized
%      with info from the Standard Colormap.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
*/
MagickPrivate XVisualInfo *XBestVisualInfo(Display *display,
  XStandardColormap *map_info,XResourceInfo *resource_info)
{
#define MaxStandardColormaps  7
#define XVisualColormapSize(visual_info) MagickMin((unsigned int) (\
  (visual_info->klass == TrueColor) || (visual_info->klass == DirectColor) ? \
   visual_info->red_mask | visual_info->green_mask | visual_info->blue_mask : \
   (unsigned long) visual_info->colormap_size),1UL << visual_info->depth)

  char
    *map_type,
    *visual_type;

  int
    visual_mask;

  register int
    i;

  size_t
    one;

  static int
    number_visuals;

  static XVisualInfo
    visual_template;

  XVisualInfo
    *visual_info,
    *visual_list;

  /*
    Restrict visual search by screen number.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(map_info != (XStandardColormap *) NULL);
  assert(resource_info != (XResourceInfo *) NULL);
  map_type=resource_info->map_type;
  visual_type=resource_info->visual_type;
  visual_mask=VisualScreenMask;
  visual_template.screen=XDefaultScreen(display);
  visual_template.depth=XDefaultDepth(display,XDefaultScreen(display));
  one=1;
  if ((resource_info->immutable != MagickFalse) && (resource_info->colors != 0))
    if (resource_info->colors <= (one << (size_t) visual_template.depth))
      visual_mask|=VisualDepthMask;
  if (visual_type != (char *) NULL)
    {
      /*
        Restrict visual search by class or visual id.
      */
      if (LocaleCompare("staticgray",visual_type) == 0)
        {
          visual_mask|=VisualClassMask;
          visual_template.klass=StaticGray;
        }
      else
        if (LocaleCompare("grayscale",visual_type) == 0)
          {
            visual_mask|=VisualClassMask;
            visual_template.klass=GrayScale;
          }
        else
          if (LocaleCompare("staticcolor",visual_type) == 0)
            {
              visual_mask|=VisualClassMask;
              visual_template.klass=StaticColor;
            }
          else
            if (LocaleCompare("pseudocolor",visual_type) == 0)
              {
                visual_mask|=VisualClassMask;
                visual_template.klass=PseudoColor;
              }
            else
              if (LocaleCompare("truecolor",visual_type) == 0)
                {
                  visual_mask|=VisualClassMask;
                  visual_template.klass=TrueColor;
                }
              else
                if (LocaleCompare("directcolor",visual_type) == 0)
                  {
                    visual_mask|=VisualClassMask;
                    visual_template.klass=DirectColor;
                  }
                else
                  if (LocaleCompare("default",visual_type) == 0)
                    {
                      visual_mask|=VisualIDMask;
                      visual_template.visualid=XVisualIDFromVisual(
                        XDefaultVisual(display,XDefaultScreen(display)));
                    }
                  else
                    if (isdigit((int) ((unsigned char) *visual_type)) != 0)
                      {
                        visual_mask|=VisualIDMask;
                        visual_template.visualid=
                          strtol(visual_type,(char **) NULL,0);
                      }
                    else
                      ThrowXWindowException(XServerError,
                        "UnrecognizedVisualSpecifier",visual_type);
    }
  /*
    Get all visuals that meet our criteria so far.
  */
  number_visuals=0;
  visual_list=XGetVisualInfo(display,visual_mask,&visual_template,
    &number_visuals);
  visual_mask=VisualScreenMask | VisualIDMask;
  if ((number_visuals == 0) || (visual_list == (XVisualInfo *) NULL))
    {
      /*
        Failed to get visual;  try using the default visual.
      */
      ThrowXWindowException(XServerWarning,"UnableToGetVisual",visual_type);
      visual_template.visualid=XVisualIDFromVisual(XDefaultVisual(display,
        XDefaultScreen(display)));
      visual_list=XGetVisualInfo(display,visual_mask,&visual_template,
        &number_visuals);
      if ((number_visuals == 0) || (visual_list == (XVisualInfo *) NULL))
        return((XVisualInfo *) NULL);
      ThrowXWindowException(XServerWarning,"UsingDefaultVisual",
        XVisualClassName(visual_list->klass));
    }
  resource_info->color_recovery=MagickFalse;
  if ((map_info != (XStandardColormap *) NULL) && (map_type != (char *) NULL))
    {
      Atom
        map_property;

      char
        map_name[MagickPathExtent];

      int
        j,
        number_maps;

      Status
        status;

      Window
        root_window;

      XStandardColormap
        *map_list;

      /*
        Choose a visual associated with a standard colormap.
      */
      root_window=XRootWindow(display,XDefaultScreen(display));
      status=False;
      number_maps=0;
      if (LocaleCompare(map_type,"list") != 0)
        {
          /*
            User specified Standard Colormap.
          */
          (void) FormatLocaleString((char *) map_name,MagickPathExtent,
            "RGB_%s_MAP",map_type);
          LocaleUpper(map_name);
          map_property=XInternAtom(display,(char *) map_name,MagickTrue);
          if (map_property != (Atom) NULL)
            status=XGetRGBColormaps(display,root_window,&map_list,&number_maps,
              map_property);
        }
      else
        {
          static const char
            *colormap[MaxStandardColormaps]=
            {
              "_HP_RGB_SMOOTH_MAP_LIST",
              "RGB_BEST_MAP",
              "RGB_DEFAULT_MAP",
              "RGB_GRAY_MAP",
              "RGB_RED_MAP",
              "RGB_GREEN_MAP",
              "RGB_BLUE_MAP",
            };

          /*
            Choose a standard colormap from a list.
          */
          for (i=0; i < MaxStandardColormaps; i++)
          {
            map_property=XInternAtom(display,(char *) colormap[i],MagickTrue);
            if (map_property == (Atom) NULL)
              continue;
            status=XGetRGBColormaps(display,root_window,&map_list,&number_maps,
              map_property);
            if (status != False)
              break;
          }
          resource_info->color_recovery=i == 0 ? MagickTrue : MagickFalse;
        }
      if (status == False)
        {
          ThrowXWindowException(XServerError,"UnableToGetStandardColormap",
            map_type);
          return((XVisualInfo *) NULL);
        }
      /*
        Search all Standard Colormaps and visuals for ids that match.
      */
      *map_info=map_list[0];
#if !defined(PRE_R4_ICCCM)
      visual_template.visualid=XVisualIDFromVisual(visual_list[0].visual);
      for (i=0; i < number_maps; i++)
        for (j=0; j < number_visuals; j++)
          if (map_list[i].visualid ==
              XVisualIDFromVisual(visual_list[j].visual))
            {
              *map_info=map_list[i];
              visual_template.visualid=XVisualIDFromVisual(
                visual_list[j].visual);
              break;
            }
      if (map_info->visualid != visual_template.visualid)
        {
          ThrowXWindowException(XServerError,
            "UnableToMatchVisualToStandardColormap",map_type);
          return((XVisualInfo *) NULL);
        }
#endif
      if (map_info->colormap == (Colormap) NULL)
        {
          ThrowXWindowException(XServerError,"StandardColormapIsNotInitialized",
            map_type);
          return((XVisualInfo *) NULL);
        }
      (void) XFree((void *) map_list);
    }
  else
    {
      static const unsigned int
        rank[]=
          {
            StaticGray,
            GrayScale,
            StaticColor,
            DirectColor,
            TrueColor,
            PseudoColor
          };

      XVisualInfo
        *p;

      /*
        Pick one visual that displays the most simultaneous colors.
      */
      visual_info=visual_list;
      p=visual_list;
      for (i=1; i < number_visuals; i++)
      {
        p++;
        if (XVisualColormapSize(p) > XVisualColormapSize(visual_info))
          visual_info=p;
        else
          if (XVisualColormapSize(p) == XVisualColormapSize(visual_info))
            if (rank[p->klass] > rank[visual_info->klass])
              visual_info=p;
      }
      visual_template.visualid=XVisualIDFromVisual(visual_info->visual);
    }
  (void) XFree((void *) visual_list);
  /*
    Retrieve only one visual by its screen & id number.
  */
  visual_info=XGetVisualInfo(display,visual_mask,&visual_template,
    &number_visuals);
  if ((number_visuals == 0) || (visual_info == (XVisualInfo *) NULL))
    return((XVisualInfo *) NULL);
  return(visual_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C h e c k D e f i n e C u r s o r                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XCheckDefineCursor() prevents cursor changes on the root window.
%
%  The format of the XXCheckDefineCursor method is:
%
%      XCheckDefineCursor(display,window,cursor)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o window: the window.
%
%    o cursor: the cursor.
%
*/
MagickPrivate int XCheckDefineCursor(Display *display,Window window,
  Cursor cursor)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  if (window == XRootWindow(display,XDefaultScreen(display)))
    return(0);
  return(XDefineCursor(display,window,cursor));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C h e c k R e f r e s h W i n d o w s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XCheckRefreshWindows() checks the X server for exposure events for a
%  particular window and updates the areassociated with the exposure event.
%
%  The format of the XCheckRefreshWindows method is:
%
%      void XCheckRefreshWindows(Display *display,XWindows *windows)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
*/
MagickPrivate void XCheckRefreshWindows(Display *display,XWindows *windows)
{
  Window
    id;

  XEvent
    event;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(windows != (XWindows *) NULL);
  XDelay(display,SuspendTime);
  id=windows->command.id;
  while (XCheckTypedWindowEvent(display,id,Expose,&event) != MagickFalse)
    (void) XCommandWidget(display,windows,(char const **) NULL,&event);
  id=windows->image.id;
  while (XCheckTypedWindowEvent(display,id,Expose,&event) != MagickFalse)
    XRefreshWindow(display,&windows->image,&event);
  XDelay(display,SuspendTime << 1);
  id=windows->command.id;
  while (XCheckTypedWindowEvent(display,id,Expose,&event) != MagickFalse)
    (void) XCommandWidget(display,windows,(char const **) NULL,&event);
  id=windows->image.id;
  while (XCheckTypedWindowEvent(display,id,Expose,&event) != MagickFalse)
    XRefreshWindow(display,&windows->image,&event);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C l i e n t M e s s a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XClientMessage() sends a reason to a window with XSendEvent.  The reason is
%  initialized with a particular protocol type and atom.
%
%  The format of the XClientMessage function is:
%
%      XClientMessage(display,window,protocol,reason,timestamp)
%
%  A description of each parameter follows:
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window structure.
%
%    o protocol: Specifies an atom value.
%
%    o reason: Specifies an atom value which is the reason to send.
%
%    o timestamp: Specifies a value of type Time.
%
*/
MagickPrivate void XClientMessage(Display *display,const Window window,
  const Atom protocol,const Atom reason,const Time timestamp)
{
  XClientMessageEvent
    client_event;

  assert(display != (Display *) NULL);
  (void) memset(&client_event,0,sizeof(client_event));
  client_event.type=ClientMessage;
  client_event.window=window;
  client_event.message_type=protocol;
  client_event.format=32;
  client_event.data.l[0]=(long) reason;
  client_event.data.l[1]=(long) timestamp;
  (void) XSendEvent(display,window,MagickFalse,NoEventMask,(XEvent *)
    &client_event);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X C l i e n t W i n d o w                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XClientWindow() finds a window, at or below the specified window, which has
%  a WM_STATE property.  If such a window is found, it is returned, otherwise
%  the argument window is returned.
%
%  The format of the XClientWindow function is:
%
%      client_window=XClientWindow(display,target_window)
%
%  A description of each parameter follows:
%
%    o client_window: XClientWindow returns a window, at or below the specified
%      window, which has a WM_STATE property otherwise the argument
%      target_window is returned.
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o target_window: Specifies the window to find a WM_STATE property.
%
*/
static Window XClientWindow(Display *display,Window target_window)
{
  Atom
    state,
    type;

  int
    format;

  Status
    status;

  unsigned char
    *data;

  unsigned long
    after,
    number_items;

  Window
    client_window;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  state=XInternAtom(display,"WM_STATE",MagickTrue);
  if (state == (Atom) NULL)
    return(target_window);
  type=(Atom) NULL;
  status=XGetWindowProperty(display,target_window,state,0L,0L,MagickFalse,
    (Atom) AnyPropertyType,&type,&format,&number_items,&after,&data);
  if ((status == Success) && (type != (Atom) NULL))
    return(target_window);
  client_window=XWindowByProperty(display,target_window,state);
  if (client_window == (Window) NULL)
    return(target_window);
  return(client_window);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X C o m p o n e n t T e r m i n u s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XComponentTerminus() destroys the module component.
%
%  The format of the XComponentTerminus method is:
%
%      XComponentTerminus(void)
%
*/
MagickPrivate void XComponentTerminus(void)
{
  DestroyXResources();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C o n f i g u r e I m a g e C o l o r m a p                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XConfigureImageColormap() creates a new X colormap.
%
%  The format of the XConfigureImageColormap method is:
%
%      void XConfigureImageColormap(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate void XConfigureImageColormap(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image *image,
  ExceptionInfo *exception)
{
  Colormap
    colormap;

  /*
    Make standard colormap.
  */
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  XMakeStandardColormap(display,windows->visual_info,resource_info,image,
    windows->map_info,windows->pixel_info,exception);
  colormap=windows->map_info->colormap;
  (void) XSetWindowColormap(display,windows->image.id,colormap);
  (void) XSetWindowColormap(display,windows->command.id,colormap);
  (void) XSetWindowColormap(display,windows->widget.id,colormap);
  if (windows->magnify.mapped != MagickFalse)
    (void) XSetWindowColormap(display,windows->magnify.id,colormap);
  if (windows->pan.mapped != MagickFalse)
    (void) XSetWindowColormap(display,windows->pan.id,colormap);
  XSetCursorState(display,windows,MagickFalse);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_colormap,CurrentTime);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C o n s t r a i n W i n d o w P o s i t i o n                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XConstrainWindowPosition() assures a window is positioned within the X
%  server boundaries.
%
%  The format of the XConstrainWindowPosition method is:
%
%      void XConstrainWindowPosition(Display *display,XWindowInfo *window_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o window_info: Specifies a pointer to a XWindowInfo structure.
%
*/
MagickPrivate void XConstrainWindowPosition(Display *display,
  XWindowInfo *window_info)
{
  int
    limit;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(window_info != (XWindowInfo *) NULL);
  limit=XDisplayWidth(display,window_info->screen)-window_info->width;
  if (window_info->x < 0)
    window_info->x=0;
  else
    if (window_info->x > (int) limit)
      window_info->x=(int) limit;
  limit=XDisplayHeight(display,window_info->screen)-window_info->height;
  if (window_info->y < 0)
    window_info->y=0;
  else
    if (window_info->y > limit)
      window_info->y=limit;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X D e l a y                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XDelay() suspends program execution for the number of milliseconds
%  specified.
%
%  The format of the Delay method is:
%
%      void XDelay(Display *display,const size_t milliseconds)
%
%  A description of each parameter follows:
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o milliseconds: Specifies the number of milliseconds to delay before
%      returning.
%
*/
MagickPrivate void XDelay(Display *display,const size_t milliseconds)
{
  assert(display != (Display *) NULL);
  (void) XFlush(display);
  MagickDelay(milliseconds);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X D e s t r o y R e s o u r c e I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XDestroyResourceInfo() frees memory associated with the XResourceInfo
%  structure.
%
%  The format of the XDestroyResourceInfo method is:
%
%      void XDestroyResourceInfo(XResourceInfo *resource_info)
%
%  A description of each parameter follows:
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
*/
MagickExport void XDestroyResourceInfo(XResourceInfo *resource_info)
{
  if (resource_info->image_geometry != (char *) NULL)
    resource_info->image_geometry=(char *)
      RelinquishMagickMemory(resource_info->image_geometry);
  if (resource_info->quantize_info != (QuantizeInfo *) NULL)
    resource_info->quantize_info=DestroyQuantizeInfo(
      resource_info->quantize_info);
  if (resource_info->client_name != (char *) NULL)
    resource_info->client_name=(char *)
      RelinquishMagickMemory(resource_info->client_name);
  if (resource_info->name != (char *) NULL)
    resource_info->name=DestroyString(resource_info->name);
  (void) memset(resource_info,0,sizeof(*resource_info));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X D e s t r o y W i n d o w C o l o r s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XDestroyWindowColors() frees X11 color resources previously saved on a
%  window by XRetainWindowColors or programs like xsetroot.
%
%  The format of the XDestroyWindowColors method is:
%
%      void XDestroyWindowColors(Display *display,Window window)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window structure.
%
*/
MagickPrivate void XDestroyWindowColors(Display *display,Window window)
{
  Atom
    property,
    type;

  int
    format;

  Status
    status;

  unsigned char
    *data;

  unsigned long
    after,
    length;

  /*
    If there are previous resources on the root window, destroy them.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  property=XInternAtom(display,"_XSETROOT_ID",MagickFalse);
  if (property == (Atom) NULL)
    {
      ThrowXWindowException(XServerError,"UnableToCreateProperty",
        "_XSETROOT_ID");
      return;
    }
  status=XGetWindowProperty(display,window,property,0L,1L,MagickTrue,
    (Atom) AnyPropertyType,&type,&format,&length,&after,&data);
  if (status != Success)
    return;
  if ((type == XA_PIXMAP) && (format == 32) && (length == 1) && (after == 0))
    {
      (void) XKillClient(display,(XID) (*((Pixmap *) data)));
      (void) XDeleteProperty(display,window,property);
    }
  if (type != None)
    (void) XFree((void *) data);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X D i s p l a y I m a g e I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XDisplayImageInfo() displays information about an X image.
%
%  The format of the XDisplayImageInfo method is:
%
%      void XDisplayImageInfo(Display *display,
%        const XResourceInfo *resource_info,XWindows *windows,Image *undo_image,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o undo_image: the undo image.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate void XDisplayImageInfo(Display *display,
  const XResourceInfo *resource_info,XWindows *windows,Image *undo_image,
  Image *image,ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent],
    *text,
    **textlist;

  FILE
    *file;

  int
    unique_file;

  register ssize_t
    i;

  size_t
    number_pixels;

  ssize_t
    bytes;

  unsigned int
    levels;

  /*
    Write info about the X server to a file.
  */
  assert(display != (Display *) NULL);
  assert(resource_info != (XResourceInfo *) NULL);
  assert(windows != (XWindows *) NULL);
  assert(image != (Image *) NULL);
  if (image->debug)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"w");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    {
      XNoticeWidget(display,windows,"Unable to display image info",filename);
      return;
    }
  if (resource_info->gamma_correct != MagickFalse)
    if (resource_info->display_gamma != (char *) NULL)
      (void) FormatLocaleFile(file,"Display\n  gamma: %s\n\n",
        resource_info->display_gamma);
  /*
    Write info about the X image to a file.
  */
  (void) FormatLocaleFile(file,"X\n  visual: %s\n",
    XVisualClassName((int) windows->image.storage_class));
  (void) FormatLocaleFile(file,"  depth: %d\n",windows->image.ximage->depth);
  if (windows->visual_info->colormap_size != 0)
    (void) FormatLocaleFile(file,"  colormap size: %d\n",
      windows->visual_info->colormap_size);
  if (resource_info->colormap== SharedColormap)
    (void) FormatLocaleFile(file,"  colormap type: Shared\n");
  else
    (void) FormatLocaleFile(file,"  colormap type: Private\n");
  (void) FormatLocaleFile(file,"  geometry: %dx%d\n",
    windows->image.ximage->width,windows->image.ximage->height);
  if (windows->image.crop_geometry != (char *) NULL)
    (void) FormatLocaleFile(file,"  crop geometry: %s\n",
      windows->image.crop_geometry);
  if (windows->image.pixmap == (Pixmap) NULL)
    (void) FormatLocaleFile(file,"  type: X Image\n");
  else
    (void) FormatLocaleFile(file,"  type: Pixmap\n");
  if (windows->image.shape != MagickFalse)
    (void) FormatLocaleFile(file,"  non-rectangular shape: True\n");
  else
    (void) FormatLocaleFile(file,"  non-rectangular shape: False\n");
  if (windows->image.shared_memory != MagickFalse)
    (void) FormatLocaleFile(file,"  shared memory: True\n");
  else
    (void) FormatLocaleFile(file,"  shared memory: False\n");
  (void) FormatLocaleFile(file,"\n");
  if (resource_info->font != (char *) NULL)
    (void) FormatLocaleFile(file,"Font: %s\n\n",resource_info->font);
  if (resource_info->text_font != (char *) NULL)
    (void) FormatLocaleFile(file,"Text font: %s\n\n",resource_info->text_font);
  /*
    Write info about the undo cache to a file.
  */
  bytes=0;
  for (levels=0; undo_image != (Image *) NULL; levels++)
  {
    number_pixels=undo_image->list->columns*undo_image->list->rows;
    bytes+=number_pixels*sizeof(PixelInfo);
    undo_image=GetPreviousImageInList(undo_image);
  }
  (void) FormatLocaleFile(file,"Undo Edit Cache\n  levels: %u\n",levels);
  (void) FormatLocaleFile(file,"  bytes: %.20gmb\n",(double)
    ((bytes+(1 << 19)) >> 20));
  (void) FormatLocaleFile(file,"  limit: %.20gmb\n\n",(double)
    resource_info->undo_cache);
  /*
    Write info about the image to a file.
  */
  (void) IdentifyImage(image,file,MagickTrue,exception);
  (void) fclose(file);
  text=FileToString(filename,~0UL,exception);
  (void) RelinquishUniqueFileResource(filename);
  if (text == (char *) NULL)
    {
      XNoticeWidget(display,windows,"MemoryAllocationFailed",
        "UnableToDisplayImageInfo");
      return;
    }
  textlist=StringToList(text);
  if (textlist != (char **) NULL)
    {
      char
        title[MagickPathExtent];

      /*
        Display information about the image in the Text View widget.
      */
      (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
      (void) FormatLocaleString(title,MagickPathExtent,"Image Info: %s",
        image->filename);
      XTextViewWidget(display,resource_info,windows,MagickTrue,title,
        (char const **) textlist);
      for (i=0; textlist[i] != (char *) NULL; i++)
        textlist[i]=DestroyString(textlist[i]);
      textlist=(char **) RelinquishMagickMemory(textlist);
    }
  text=DestroyString(text);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     X D i t h e r I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XDitherImage() dithers the reference image as required by the HP Color
%  Recovery algorithm.  The color values are quantized to 3 bits of red and
%  green, and 2 bits of blue (3/3/2) and can be used as indices into a 8-bit X
%  standard colormap.
%
%  The format of the XDitherImage method is:
%
%      void XDitherImage(Image *image,XImage *ximage,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o ximage: Specifies a pointer to a XImage structure;  returned from
%      XCreateImage.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static void XDitherImage(Image *image,XImage *ximage,ExceptionInfo *exception)
{
  static const short int
    dither_red[2][16]=
    {
      {-16,  4, -1, 11,-14,  6, -3,  9,-15,  5, -2, 10,-13,  7, -4,  8},
      { 15, -5,  0,-12, 13, -7,  2,-10, 14, -6,  1,-11, 12, -8,  3, -9}
    },
    dither_green[2][16]=
    {
      { 11,-15,  7, -3,  8,-14,  4, -2, 10,-16,  6, -4,  9,-13,  5, -1},
      {-12, 14, -8,  2, -9, 13, -5,  1,-11, 15, -7,  3,-10, 12, -6,  0}
    },
    dither_blue[2][16]=
    {
      { -3,  9,-13,  7, -1, 11,-15,  5, -4,  8,-14,  6, -2, 10,-16,  4},
      {  2,-10, 12, -8,  0,-12, 14, -6,  3, -9, 13, -7,  1,-11, 15, -5}
    };

  CacheView
    *image_view;

  int
    value,
    y;

  PixelInfo
    color;

  register char
    *q;

  register const Quantum
    *p;

  register int
    i,
    j,
    x;

  unsigned int
    scanline_pad;

  register size_t
    pixel;

  unsigned char
    *blue_map[2][16],
    *green_map[2][16],
    *red_map[2][16];

  /*
    Allocate and initialize dither maps.
  */
  for (i=0; i < 2; i++)
    for (j=0; j < 16; j++)
    {
      red_map[i][j]=(unsigned char *) AcquireCriticalMemory(256UL*
        sizeof(*red_map));
      green_map[i][j]=(unsigned char *) AcquireCriticalMemory(256UL*
        sizeof(*green_map));
      blue_map[i][j]=(unsigned char *) AcquireCriticalMemory(256UL*
        sizeof(*blue_map));
    }
  /*
    Initialize dither tables.
  */
  for (i=0; i < 2; i++)
    for (j=0; j < 16; j++)
      for (x=0; x < 256; x++)
      {
        value=x-16;
        if (x < 48)
          value=x/2+8;
        value+=dither_red[i][j];
        red_map[i][j][x]=(unsigned char)
          ((value < 0) ? 0 : (value > 255) ? 255 : value);
        value=x-16;
        if (x < 48)
          value=x/2+8;
        value+=dither_green[i][j];
        green_map[i][j][x]=(unsigned char)
          ((value < 0) ? 0 : (value > 255) ? 255 : value);
        value=x-32;
        if (x < 112)
          value=x/2+24;
        value+=((size_t) dither_blue[i][j] << 1);
        blue_map[i][j][x]=(unsigned char)
          ((value < 0) ? 0 : (value > 255) ? 255 : value);
      }
  /*
    Dither image.
  */
  scanline_pad=(unsigned int) (ximage->bytes_per_line-
    ((size_t) (ximage->width*ximage->bits_per_pixel) >> 3));
  i=0;
  j=0;
  q=ximage->data;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,(ssize_t) y,image->columns,1,
      exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      color.red=(double) ClampToQuantum((double) (red_map[i][j][
        (int) ScaleQuantumToChar(GetPixelRed(image,p))] << 8));
      color.green=(double) ClampToQuantum((double) (green_map[i][j][
        (int) ScaleQuantumToChar(GetPixelGreen(image,p))] << 8));
      color.blue=(double) ClampToQuantum((double) (blue_map[i][j][
        (int) ScaleQuantumToChar(GetPixelBlue(image,p))] << 8));
      pixel=(size_t) (((size_t) color.red & 0xe0) |
        (((size_t) color.green & 0xe0) >> 3) |
        (((size_t) color.blue & 0xc0) >> 6));
      *q++=(char) pixel;
      p+=GetPixelChannels(image);
      j++;
      if (j == 16)
        j=0;
    }
    q+=scanline_pad;
    i++;
    if (i == 2)
      i=0;
  }
  image_view=DestroyCacheView(image_view);
  /*
    Free allocated memory.
  */
  for (i=0; i < 2; i++)
    for (j=0; j < 16; j++)
    {
      green_map[i][j]=(unsigned char *) RelinquishMagickMemory(green_map[i][j]);
      blue_map[i][j]=(unsigned char *) RelinquishMagickMemory(blue_map[i][j]);
      red_map[i][j]=(unsigned char *) RelinquishMagickMemory(red_map[i][j]);
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X D r a w I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XDrawImage() draws a line on the image.
%
%  The format of the XDrawImage method is:
%
%    MagickBooleanType XDrawImage(Display *display,const XPixelInfo *pixel,
%      XDrawInfo *draw_info,Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o pixel: Specifies a pointer to a XPixelInfo structure.
%
%    o draw_info: Specifies a pointer to a XDrawInfo structure.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate MagickBooleanType XDrawImage(Display *display,
  const XPixelInfo *pixel,XDrawInfo *draw_info,Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *draw_view;

  GC
    draw_context;

  Image
    *draw_image;

  int
    x,
    y;

  PixelTrait
    alpha_trait;

  Pixmap
    draw_pixmap;

  unsigned int
    depth,
    height,
    width;

  Window
    root_window;

  XGCValues
    context_values;

  XImage
    *draw_ximage;

  /*
    Initialize drawd image.
  */
  assert(display != (Display *) NULL);
  assert(pixel != (XPixelInfo *) NULL);
  assert(draw_info != (XDrawInfo *) NULL);
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  /*
    Initialize drawd pixmap.
  */
  root_window=XRootWindow(display,XDefaultScreen(display));
  depth=(unsigned int) XDefaultDepth(display,XDefaultScreen(display));
  draw_pixmap=XCreatePixmap(display,root_window,draw_info->width,
    draw_info->height,depth);
  if (draw_pixmap == (Pixmap) NULL)
    return(MagickFalse);
  /*
    Initialize graphics info.
  */
  context_values.background=(size_t) (~0);
  context_values.foreground=0;
  context_values.line_width=(int) draw_info->line_width;
  draw_context=XCreateGC(display,root_window,(size_t)
    (GCBackground | GCForeground | GCLineWidth),&context_values);
  if (draw_context == (GC) NULL)
    return(MagickFalse);
  /*
    Clear pixmap.
  */
  (void) XFillRectangle(display,draw_pixmap,draw_context,0,0,draw_info->width,
    draw_info->height);
  /*
    Draw line to pixmap.
  */
  (void) XSetBackground(display,draw_context,0);
  (void) XSetForeground(display,draw_context,(size_t) (~0));
  if (draw_info->stipple !=  (Pixmap) NULL)
    {
      (void) XSetFillStyle(display,draw_context,FillOpaqueStippled);
      (void) XSetStipple(display,draw_context,draw_info->stipple);
    }
  switch (draw_info->element)
  {
    case PointElement:
    default:
    {
      (void) XDrawLines(display,draw_pixmap,draw_context,
        draw_info->coordinate_info,(int) draw_info->number_coordinates,
        CoordModeOrigin);
      break;
    }
    case LineElement:
    {
      (void) XDrawLine(display,draw_pixmap,draw_context,draw_info->line_info.x1,
        draw_info->line_info.y1,draw_info->line_info.x2,
        draw_info->line_info.y2);
      break;
    }
    case RectangleElement:
    {
      (void) XDrawRectangle(display,draw_pixmap,draw_context,
        (int) draw_info->rectangle_info.x,(int) draw_info->rectangle_info.y,
        (unsigned int) draw_info->rectangle_info.width,
        (unsigned int) draw_info->rectangle_info.height);
      break;
    }
    case FillRectangleElement:
    {
      (void) XFillRectangle(display,draw_pixmap,draw_context,
        (int) draw_info->rectangle_info.x,(int) draw_info->rectangle_info.y,
        (unsigned int) draw_info->rectangle_info.width,
        (unsigned int) draw_info->rectangle_info.height);
      break;
    }
    case CircleElement:
    case EllipseElement:
    {
      (void) XDrawArc(display,draw_pixmap,draw_context,
        (int) draw_info->rectangle_info.x,(int) draw_info->rectangle_info.y,
        (unsigned int) draw_info->rectangle_info.width,
        (unsigned int) draw_info->rectangle_info.height,0,360*64);
      break;
    }
    case FillCircleElement:
    case FillEllipseElement:
    {
      (void) XFillArc(display,draw_pixmap,draw_context,
        (int) draw_info->rectangle_info.x,(int) draw_info->rectangle_info.y,
        (unsigned int) draw_info->rectangle_info.width,
        (unsigned int) draw_info->rectangle_info.height,0,360*64);
      break;
    }
    case PolygonElement:
    {
      XPoint
        *coordinate_info;

      coordinate_info=draw_info->coordinate_info;
      (void) XDrawLines(display,draw_pixmap,draw_context,coordinate_info,
        (int) draw_info->number_coordinates,CoordModeOrigin);
      (void) XDrawLine(display,draw_pixmap,draw_context,
        coordinate_info[draw_info->number_coordinates-1].x,
        coordinate_info[draw_info->number_coordinates-1].y,
        coordinate_info[0].x,coordinate_info[0].y);
      break;
    }
    case FillPolygonElement:
    {
      (void) XFillPolygon(display,draw_pixmap,draw_context,
        draw_info->coordinate_info,(int) draw_info->number_coordinates,Complex,
        CoordModeOrigin);
      break;
    }
  }
  (void) XFreeGC(display,draw_context);
  /*
    Initialize X image.
  */
  draw_ximage=XGetImage(display,draw_pixmap,0,0,draw_info->width,
    draw_info->height,AllPlanes,ZPixmap);
  if (draw_ximage == (XImage *) NULL)
    return(MagickFalse);
  (void) XFreePixmap(display,draw_pixmap);
  /*
    Initialize draw image.
  */
  draw_image=AcquireImage((ImageInfo *) NULL,exception);
  if (draw_image == (Image *) NULL)
    return(MagickFalse);
  draw_image->columns=draw_info->width;
  draw_image->rows=draw_info->height;
  /*
    Transfer drawn X image to image.
  */
  width=(unsigned int) image->columns;
  height=(unsigned int) image->rows;
  x=0;
  y=0;
  (void) XParseGeometry(draw_info->geometry,&x,&y,&width,&height);
  (void) GetOneVirtualPixelInfo(image,UndefinedVirtualPixelMethod,(ssize_t) x,
    (ssize_t) y,&draw_image->background_color,exception);
  if (SetImageStorageClass(draw_image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  draw_image->alpha_trait=BlendPixelTrait;
  draw_view=AcquireAuthenticCacheView(draw_image,exception);
  for (y=0; y < (int) draw_image->rows; y++)
  {
    register int
      x;

    register Quantum
      *magick_restrict q;

    q=QueueCacheViewAuthenticPixels(draw_view,0,(ssize_t) y,draw_image->columns,
      1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (int) draw_image->columns; x++)
    {
      if (XGetPixel(draw_ximage,x,y) == 0)
        {
          /*
            Set this pixel to the background color.
          */
          SetPixelViaPixelInfo(draw_image,&draw_image->background_color,q);
          SetPixelAlpha(draw_image,(Quantum) (draw_info->stencil ==
            OpaqueStencil ? TransparentAlpha : OpaqueAlpha),q);
        }
      else
        {
          /*
            Set this pixel to the pen color.
          */
          SetPixelRed(draw_image,ScaleShortToQuantum(
            pixel->pen_color.red),q);
          SetPixelGreen(draw_image,ScaleShortToQuantum(
            pixel->pen_color.green),q);
          SetPixelBlue(draw_image,ScaleShortToQuantum(
            pixel->pen_color.blue),q);
          SetPixelAlpha(draw_image,(Quantum) (draw_info->stencil ==
            OpaqueStencil ? OpaqueAlpha : TransparentAlpha),q);
        }
      q+=GetPixelChannels(draw_image);
    }
    if (SyncCacheViewAuthenticPixels(draw_view,exception) == MagickFalse)
      break;
  }
  draw_view=DestroyCacheView(draw_view);
  XDestroyImage(draw_ximage);
  /*
    Determine draw geometry.
  */
  (void) XParseGeometry(draw_info->geometry,&x,&y,&width,&height);
  if ((width != (unsigned int) draw_image->columns) ||
      (height != (unsigned int) draw_image->rows))
    {
      char
        image_geometry[MagickPathExtent];

      /*
        Scale image.
      */
      (void) FormatLocaleString(image_geometry,MagickPathExtent,"%ux%u",
        width,height);
      (void) TransformImage(&draw_image,(char *) NULL,image_geometry,
        exception);
    }
  if (draw_info->degrees != 0.0)
    {
      Image
        *rotate_image;

      int
        rotations;

      double
        normalized_degrees;

      /*
        Rotate image.
      */
      rotate_image=RotateImage(draw_image,draw_info->degrees,exception);
      if (rotate_image == (Image *) NULL)
        return(MagickFalse);
      draw_image=DestroyImage(draw_image);
      draw_image=rotate_image;
      /*
        Annotation is relative to the degree of rotation.
      */
      normalized_degrees=draw_info->degrees;
      while (normalized_degrees < -45.0)
        normalized_degrees+=360.0;
      for (rotations=0; normalized_degrees > 45.0; rotations++)
        normalized_degrees-=90.0;
      switch (rotations % 4)
      {
        default:
        case 0:
          break;
        case 1:
        {
          /*
            Rotate 90 degrees.
          */
          x=x-(int) draw_image->columns/2;
          y=y+(int) draw_image->columns/2;
          break;
        }
        case 2:
        {
          /*
            Rotate 180 degrees.
          */
          x=x-(int) draw_image->columns;
          break;
        }
        case 3:
        {
          /*
            Rotate 270 degrees.
          */
          x=x-(int) draw_image->columns/2;
          y=y-(int) (draw_image->rows-(draw_image->columns/2));
          break;
        }
      }
    }
  /*
    Composite text onto the image.
  */
  draw_view=AcquireAuthenticCacheView(draw_image,exception);
  for (y=0; y < (int) draw_image->rows; y++)
  {
    register int
      x;

    register Quantum
      *magick_restrict q;

    q=GetCacheViewAuthenticPixels(draw_view,0,(ssize_t) y,draw_image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (int) draw_image->columns; x++)
    {
      if (GetPixelAlpha(image,q) != TransparentAlpha)
        SetPixelAlpha(draw_image,OpaqueAlpha,q);
      q+=GetPixelChannels(draw_image);
    }
    if (SyncCacheViewAuthenticPixels(draw_view,exception) == MagickFalse)
      break;
  }
  draw_view=DestroyCacheView(draw_view);
  (void) XParseGeometry(draw_info->geometry,&x,&y,&width,&height);
  if (draw_info->stencil == TransparentStencil)
    (void) CompositeImage(image,draw_image,CopyAlphaCompositeOp,MagickTrue,
      (ssize_t) x,(ssize_t) y,exception);
  else
    {
      alpha_trait=image->alpha_trait;
      (void) CompositeImage(image,draw_image,OverCompositeOp,MagickTrue,
        (ssize_t) x,(ssize_t) y,exception);
      image->alpha_trait=alpha_trait;
    }
  draw_image=DestroyImage(draw_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X E r r o r                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XError() ignores BadWindow errors for XQueryTree and XGetWindowAttributes,
%  and ignores BadDrawable errors for XGetGeometry, and ignores BadValue errors
%  for XQueryColor.  It returns MagickFalse in those cases.  Otherwise it
%  returns True.
%
%  The format of the XError function is:
%
%      int XError(display,error)
%
%  A description of each parameter follows:
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o error: Specifies the error event.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickExport int XError(Display *display,XErrorEvent *error)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(error != (XErrorEvent *) NULL);
  xerror_alert=MagickTrue;
  switch (error->request_code)
  {
    case X_GetGeometry:
    {
      if ((int) error->error_code == BadDrawable)
        return(MagickFalse);
      break;
    }
    case X_GetWindowAttributes:
    case X_QueryTree:
    {
      if ((int) error->error_code == BadWindow)
        return(MagickFalse);
      break;
    }
    case X_QueryColors:
    {
      if ((int) error->error_code == BadValue)
        return(MagickFalse);
      break;
    }
  }
  return(MagickTrue);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X F r e e R e s o u r c e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XFreeResources() frees X11 resources.
%
%  The format of the XFreeResources method is:
%
%      void XFreeResources(Display *display,XVisualInfo *visual_info,
%        XStandardColormap *map_info,XPixelInfo *pixel,XFontStruct *font_info,
%        XResourceInfo *resource_info,XWindowInfo *window_info)
%        resource_info,window_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o visual_info: Specifies a pointer to a X11 XVisualInfo structure;
%      returned from XGetVisualInfo.
%
%    o map_info: If map_type is specified, this structure is initialized
%      with info from the Standard Colormap.
%
%    o pixel: Specifies a pointer to a XPixelInfo structure.
%
%    o font_info: Specifies a pointer to a XFontStruct structure.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o window_info: Specifies a pointer to a X11 XWindowInfo structure.
%
*/
MagickPrivate void XFreeResources(Display *display,XVisualInfo *visual_info,
  XStandardColormap *map_info,XPixelInfo *pixel,XFontStruct *font_info,
  XResourceInfo *resource_info,XWindowInfo *window_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(resource_info != (XResourceInfo *) NULL);
  if (window_info != (XWindowInfo *) NULL)
    {
      /*
        Free X image.
      */
      if (window_info->ximage != (XImage *) NULL)
        XDestroyImage(window_info->ximage);
      if (window_info->id != (Window) NULL)
        {
          /*
            Free destroy window and free cursors.
          */
          if (window_info->id != XRootWindow(display,visual_info->screen))
            (void) XDestroyWindow(display,window_info->id);
          if (window_info->annotate_context != (GC) NULL)
            (void) XFreeGC(display,window_info->annotate_context);
          if (window_info->highlight_context != (GC) NULL)
            (void) XFreeGC(display,window_info->highlight_context);
          if (window_info->widget_context != (GC) NULL)
            (void) XFreeGC(display,window_info->widget_context);
          if (window_info->cursor != (Cursor) NULL)
            (void) XFreeCursor(display,window_info->cursor);
          window_info->cursor=(Cursor) NULL;
          if (window_info->busy_cursor != (Cursor) NULL)
            (void) XFreeCursor(display,window_info->busy_cursor);
          window_info->busy_cursor=(Cursor) NULL;
        }
    }
  /*
    Free font.
  */
  if (font_info != (XFontStruct *) NULL)
    {
      (void) XFreeFont(display,font_info);
      font_info=(XFontStruct *) NULL;
    }
  if (map_info != (XStandardColormap *) NULL)
    {
      /*
        Free X Standard Colormap.
      */
      if (resource_info->map_type == (char *) NULL)
        (void) XFreeStandardColormap(display,visual_info,map_info,pixel);
      (void) XFree((void *) map_info);
    }
  /*
    Free X visual info.
  */
  if (visual_info != (XVisualInfo *) NULL)
    (void) XFree((void *) visual_info);
  if (resource_info->close_server != MagickFalse)
    (void) XCloseDisplay(display);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X F r e e S t a n d a r d C o l o r m a p                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XFreeStandardColormap() frees an X11 colormap.
%
%  The format of the XFreeStandardColormap method is:
%
%      void XFreeStandardColormap(Display *display,
%        const XVisualInfo *visual_info,XStandardColormap *map_info,
%        XPixelInfo *pixel)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o visual_info: Specifies a pointer to a X11 XVisualInfo structure;
%      returned from XGetVisualInfo.
%
%    o map_info: If map_type is specified, this structure is initialized
%      with info from the Standard Colormap.
%
%    o pixel: Specifies a pointer to a XPixelInfo structure.
%
*/
MagickPrivate void XFreeStandardColormap(Display *display,
  const XVisualInfo *visual_info,XStandardColormap *map_info,XPixelInfo *pixel)
{
  /*
    Free colormap.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(visual_info != (XVisualInfo *) NULL);
  assert(map_info != (XStandardColormap *) NULL);
  (void) XFlush(display);
  if (map_info->colormap != (Colormap) NULL)
    {
      if (map_info->colormap != XDefaultColormap(display,visual_info->screen))
        (void) XFreeColormap(display,map_info->colormap);
      else
        if (pixel != (XPixelInfo *) NULL)
          if ((visual_info->klass != TrueColor) &&
              (visual_info->klass != DirectColor))
            (void) XFreeColors(display,map_info->colormap,pixel->pixels,
              (int) pixel->colors,0);
    }
  map_info->colormap=(Colormap) NULL;
  if (pixel != (XPixelInfo *) NULL)
    {
      if (pixel->pixels != (unsigned long *) NULL)
        pixel->pixels=(unsigned long *) RelinquishMagickMemory(pixel->pixels);
      pixel->pixels=(unsigned long *) NULL;
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t A n n o t a t e I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XGetAnnotateInfo() initializes the AnnotateInfo structure.
%
%  The format of the XGetAnnotateInfo method is:
%
%      void XGetAnnotateInfo(XAnnotateInfo *annotate_info)
%
%  A description of each parameter follows:
%
%    o annotate_info: Specifies a pointer to a XAnnotateInfo structure.
%
*/
MagickPrivate void XGetAnnotateInfo(XAnnotateInfo *annotate_info)
{
  /*
    Initialize annotate structure.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(annotate_info != (XAnnotateInfo *) NULL);
  annotate_info->x=0;
  annotate_info->y=0;
  annotate_info->width=0;
  annotate_info->height=0;
  annotate_info->stencil=ForegroundStencil;
  annotate_info->degrees=0.0;
  annotate_info->font_info=(XFontStruct *) NULL;
  annotate_info->text=(char *) NULL;
  *annotate_info->geometry='\0';
  annotate_info->previous=(XAnnotateInfo *) NULL;
  annotate_info->next=(XAnnotateInfo *) NULL;
  (void) XSupportsLocale();
  (void) XSetLocaleModifiers("");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t M a p I n f o                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XGetMapInfo() initializes the XStandardColormap structure.
%
%  The format of the XStandardColormap method is:
%
%      void XGetMapInfo(const XVisualInfo *visual_info,const Colormap colormap,
%        XStandardColormap *map_info)
%
%  A description of each parameter follows:
%
%    o colormap: Specifies the ID of the X server colormap.
%
%    o visual_info: Specifies a pointer to a X11 XVisualInfo structure;
%      returned from XGetVisualInfo.
%
%    o map_info: Specifies a pointer to a X11 XStandardColormap structure.
%
*/
MagickPrivate void XGetMapInfo(const XVisualInfo *visual_info,
  const Colormap colormap,XStandardColormap *map_info)
{
  /*
    Initialize map info.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(visual_info != (XVisualInfo *) NULL);
  assert(map_info != (XStandardColormap *) NULL);
  map_info->colormap=colormap;
  map_info->red_max=visual_info->red_mask;
  map_info->red_mult=(size_t) (map_info->red_max != 0 ? 1 : 0);
  if (map_info->red_max != 0)
    while ((map_info->red_max & 0x01) == 0)
    {
      map_info->red_max>>=1;
      map_info->red_mult<<=1;
    }
  map_info->green_max=visual_info->green_mask;
  map_info->green_mult=(size_t) (map_info->green_max != 0 ? 1 : 0);
  if (map_info->green_max != 0)
    while ((map_info->green_max & 0x01) == 0)
    {
      map_info->green_max>>=1;
      map_info->green_mult<<=1;
    }
  map_info->blue_max=visual_info->blue_mask;
  map_info->blue_mult=(size_t) (map_info->blue_max != 0 ? 1 : 0);
  if (map_info->blue_max != 0)
    while ((map_info->blue_max & 0x01) == 0)
    {
      map_info->blue_max>>=1;
      map_info->blue_mult<<=1;
    }
  map_info->base_pixel=0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t P i x e l I n f o                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XGetPixelInfo() initializes the PixelInfo structure.
%
%  The format of the XGetPixelInfo method is:
%
%      void XGetPixelInfo(Display *display,const XVisualInfo *visual_info,
%        const XStandardColormap *map_info,const XResourceInfo *resource_info,
%        Image *image,XPixelInfo *pixel)
%        pixel)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o visual_info: Specifies a pointer to a X11 XVisualInfo structure;
%      returned from XGetVisualInfo.
%
%    o map_info: If map_type is specified, this structure is initialized
%      with info from the Standard Colormap.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o image: the image.
%
%    o pixel: Specifies a pointer to a XPixelInfo structure.
%
*/
MagickPrivate void XGetPixelInfo(Display *display,
  const XVisualInfo *visual_info,const XStandardColormap *map_info,
  const XResourceInfo *resource_info,Image *image,XPixelInfo *pixel)
{
  static const char
    *PenColors[MaxNumberPens]=
    {
      "#000000000000",  /* black */
      "#00000000ffff",  /* blue */
      "#0000ffffffff",  /* cyan */
      "#0000ffff0000",  /* green */
      "#bdbdbdbdbdbd",  /* gray */
      "#ffff00000000",  /* red */
      "#ffff0000ffff",  /* magenta */
      "#ffffffff0000",  /* yellow */
      "#ffffffffffff",  /* white */
      "#bdbdbdbdbdbd",  /* gray */
      "#bdbdbdbdbdbd"   /* gray */
    };

  Colormap
    colormap;

  register ssize_t
    i;

  Status
    status;

  unsigned int
    packets;

  /*
    Initialize pixel info.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(visual_info != (XVisualInfo *) NULL);
  assert(map_info != (XStandardColormap *) NULL);
  assert(resource_info != (XResourceInfo *) NULL);
  assert(pixel != (XPixelInfo *) NULL);
  pixel->colors=0;
  if (image != (Image *) NULL)
    if (image->storage_class == PseudoClass)
      pixel->colors=(ssize_t) image->colors;
  packets=(unsigned int)
    MagickMax((int) pixel->colors,visual_info->colormap_size)+MaxNumberPens;
  if (pixel->pixels != (unsigned long *) NULL)
    pixel->pixels=(unsigned long *) RelinquishMagickMemory(pixel->pixels);
  pixel->pixels=(unsigned long *) AcquireQuantumMemory(packets,
    sizeof(*pixel->pixels));
  if (pixel->pixels == (unsigned long *) NULL)
    ThrowXWindowFatalException(ResourceLimitFatalError,"UnableToGetPixelInfo",
      image->filename);
  /*
    Set foreground color.
  */
  colormap=map_info->colormap;
  (void) XParseColor(display,colormap,(char *) ForegroundColor,
    &pixel->foreground_color);
  status=XParseColor(display,colormap,resource_info->foreground_color,
    &pixel->foreground_color);
  if (status == False)
    ThrowXWindowException(XServerError,"ColorIsNotKnownToServer",
      resource_info->foreground_color);
  pixel->foreground_color.pixel=
    XStandardPixel(map_info,&pixel->foreground_color);
  pixel->foreground_color.flags=(char) (DoRed | DoGreen | DoBlue);
  /*
    Set background color.
  */
  (void) XParseColor(display,colormap,"#d6d6d6d6d6d6",&pixel->background_color);
  status=XParseColor(display,colormap,resource_info->background_color,
    &pixel->background_color);
  if (status == False)
    ThrowXWindowException(XServerError,"ColorIsNotKnownToServer",
      resource_info->background_color);
  pixel->background_color.pixel=
    XStandardPixel(map_info,&pixel->background_color);
  pixel->background_color.flags=(char) (DoRed | DoGreen | DoBlue);
  /*
    Set border color.
  */
  (void) XParseColor(display,colormap,(char *) BorderColor,
    &pixel->border_color);
  status=XParseColor(display,colormap,resource_info->border_color,
    &pixel->border_color);
  if (status == False)
    ThrowXWindowException(XServerError,"ColorIsNotKnownToServer",
      resource_info->border_color);
  pixel->border_color.pixel=XStandardPixel(map_info,&pixel->border_color);
  pixel->border_color.flags=(char) (DoRed | DoGreen | DoBlue);
  /*
    Set matte color.
  */
  pixel->matte_color=pixel->background_color;
  if (resource_info->matte_color != (char *) NULL)
    {
      /*
        Matte color is specified as a X resource or command line argument.
      */
      status=XParseColor(display,colormap,resource_info->matte_color,
        &pixel->matte_color);
      if (status == False)
        ThrowXWindowException(XServerError,"ColorIsNotKnownToServer",
          resource_info->matte_color);
      pixel->matte_color.pixel=XStandardPixel(map_info,&pixel->matte_color);
      pixel->matte_color.flags=(char) (DoRed | DoGreen | DoBlue);
    }
  /*
    Set highlight color.
  */
  pixel->highlight_color.red=(unsigned short) (((double)
    pixel->matte_color.red*ScaleQuantumToShort(HighlightModulate))/65535L+
    (ScaleQuantumToShort((Quantum) (QuantumRange-HighlightModulate))));
  pixel->highlight_color.green=(unsigned short) (((double)
    pixel->matte_color.green*ScaleQuantumToShort(HighlightModulate))/65535L+
    (ScaleQuantumToShort((Quantum) (QuantumRange-HighlightModulate))));
  pixel->highlight_color.blue=(unsigned short) (((double)
    pixel->matte_color.blue*ScaleQuantumToShort(HighlightModulate))/65535L+
    (ScaleQuantumToShort((Quantum) (QuantumRange-HighlightModulate))));
  pixel->highlight_color.pixel=XStandardPixel(map_info,&pixel->highlight_color);
  pixel->highlight_color.flags=(char) (DoRed | DoGreen | DoBlue);
  /*
    Set shadow color.
  */
  pixel->shadow_color.red=(unsigned short) (((double)
    pixel->matte_color.red*ScaleQuantumToShort(ShadowModulate))/65535L);
  pixel->shadow_color.green=(unsigned short) (((double)
    pixel->matte_color.green*ScaleQuantumToShort(ShadowModulate))/65535L);
  pixel->shadow_color.blue=(unsigned short) (((double)
    pixel->matte_color.blue*ScaleQuantumToShort(ShadowModulate))/65535L);
  pixel->shadow_color.pixel=XStandardPixel(map_info,&pixel->shadow_color);
  pixel->shadow_color.flags=(char) (DoRed | DoGreen | DoBlue);
  /*
    Set depth color.
  */
  pixel->depth_color.red=(unsigned short) (((double)
    pixel->matte_color.red*ScaleQuantumToShort(DepthModulate))/65535L);
  pixel->depth_color.green=(unsigned short) (((double)
    pixel->matte_color.green*ScaleQuantumToShort(DepthModulate))/65535L);
  pixel->depth_color.blue=(unsigned short) (((double)
    pixel->matte_color.blue*ScaleQuantumToShort(DepthModulate))/65535L);
  pixel->depth_color.pixel=XStandardPixel(map_info,&pixel->depth_color);
  pixel->depth_color.flags=(char) (DoRed | DoGreen | DoBlue);
  /*
    Set trough color.
  */
  pixel->trough_color.red=(unsigned short) (((double)
    pixel->matte_color.red*ScaleQuantumToShort(TroughModulate))/65535L);
  pixel->trough_color.green=(unsigned short) (((double)
    pixel->matte_color.green*ScaleQuantumToShort(TroughModulate))/65535L);
  pixel->trough_color.blue=(unsigned short) (((double)
    pixel->matte_color.blue*ScaleQuantumToShort(TroughModulate))/65535L);
  pixel->trough_color.pixel=XStandardPixel(map_info,&pixel->trough_color);
  pixel->trough_color.flags=(char) (DoRed | DoGreen | DoBlue);
  /*
    Set pen color.
  */
  for (i=0; i < MaxNumberPens; i++)
  {
    (void) XParseColor(display,colormap,(char *) PenColors[i],
      &pixel->pen_colors[i]);
    status=XParseColor(display,colormap,resource_info->pen_colors[i],
      &pixel->pen_colors[i]);
    if (status == False)
      ThrowXWindowException(XServerError,"ColorIsNotKnownToServer",
        resource_info->pen_colors[i]);
    pixel->pen_colors[i].pixel=XStandardPixel(map_info,&pixel->pen_colors[i]);
    pixel->pen_colors[i].flags=(char) (DoRed | DoGreen | DoBlue);
  }
  pixel->box_color=pixel->background_color;
  pixel->pen_color=pixel->foreground_color;
  pixel->box_index=0;
  pixel->pen_index=1;
  if (image != (Image *) NULL)
    {
      if ((resource_info->gamma_correct != MagickFalse) &&
          (image->gamma != 0.0))
        {
          GeometryInfo
            geometry_info;

          MagickStatusType
            flags;

          /*
            Initialize map relative to display and image gamma.
          */
          flags=ParseGeometry(resource_info->display_gamma,&geometry_info);
          red_gamma=geometry_info.rho;
          green_gamma=geometry_info.sigma;
          if ((flags & SigmaValue) == 0)
            green_gamma=red_gamma;
          blue_gamma=geometry_info.xi;
          if ((flags & XiValue) == 0)
            blue_gamma=red_gamma;
          red_gamma*=image->gamma;
          green_gamma*=image->gamma;
          blue_gamma*=image->gamma;
        }
      if (image->storage_class == PseudoClass)
        {
          /*
            Initialize pixel array for images of type PseudoClass.
          */
          for (i=0; i < (ssize_t) image->colors; i++)
            pixel->pixels[i]=XGammaPacket(map_info,image->colormap+i);
          for (i=0; i < MaxNumberPens; i++)
            pixel->pixels[image->colors+i]=pixel->pen_colors[i].pixel;
          pixel->colors+=MaxNumberPens;
        }
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t R e s o u r c e C l a s s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XGetResourceClass() queries the X server for the specified resource name or
%  class.  If the resource name or class is not defined in the database, the
%  supplied default value is returned.
%
%  The format of the XGetResourceClass method is:
%
%      char *XGetResourceClass(XrmDatabase database,const char *client_name,
%        const char *keyword,char *resource_default)
%
%  A description of each parameter follows:
%
%    o database: Specifies a resource database; returned from
%      XrmGetStringDatabase.
%
%    o client_name:  Specifies the application name used to retrieve resource
%      info from the X server database.
%
%    o keyword: Specifies the keyword of the value being retrieved.
%
%    o resource_default: Specifies the default value to return if the query
%      fails to find the specified keyword/class.
%
*/
MagickExport char *XGetResourceClass(XrmDatabase database,
  const char *client_name,const char *keyword,char *resource_default)
{
  char
    resource_class[MagickPathExtent],
    resource_name[MagickPathExtent];

  static char
    *resource_type;

  Status
    status;

  XrmValue
    resource_value;

  if (database == (XrmDatabase) NULL)
    return(resource_default);
  *resource_name='\0';
  *resource_class='\0';
  if (keyword != (char *) NULL)
    {
      int
        c,
        k;

      /*
        Initialize resource keyword and class.
      */
      (void) FormatLocaleString(resource_name,MagickPathExtent,"%s.%s",
        client_name,keyword);
      c=(int) (*client_name);
      if ((c >= XK_a) && (c <= XK_z))
        c-=(XK_a-XK_A);
      else
        if ((c >= XK_agrave) && (c <= XK_odiaeresis))
          c-=(XK_agrave-XK_Agrave);
        else
          if ((c >= XK_oslash) && (c <= XK_thorn))
            c-=(XK_oslash-XK_Ooblique);
      k=(int) (*keyword);
      if ((k >= XK_a) && (k <= XK_z))
        k-=(XK_a-XK_A);
      else
        if ((k >= XK_agrave) && (k <= XK_odiaeresis))
          k-=(XK_agrave-XK_Agrave);
        else
          if ((k >= XK_oslash) && (k <= XK_thorn))
            k-=(XK_oslash-XK_Ooblique);
      (void) FormatLocaleString(resource_class,MagickPathExtent,"%c%s.%c%s",c,
        client_name+1,k,keyword+1);
    }
  status=XrmGetResource(database,resource_name,resource_class,&resource_type,
    &resource_value);
  if (status == False)
    return(resource_default);
  return(resource_value.addr);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t R e s o u r c e D a t a b a s e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XGetResourceDatabase() creates a new resource database and initializes it.
%
%  The format of the XGetResourceDatabase method is:
%
%      XrmDatabase XGetResourceDatabase(Display *display,
%        const char *client_name)
%
%  A description of each parameter follows:
%
%    o database: XGetResourceDatabase() returns the database after it is
%      initialized.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o client_name:  Specifies the application name used to retrieve resource
%      info from the X server database.
%
*/
MagickExport XrmDatabase XGetResourceDatabase(Display *display,
  const char *client_name)
{
  char
    filename[MagickPathExtent];

  int
    c;

  register const char
    *p;

  XrmDatabase
    resource_database,
    server_database;

  if (display == (Display *) NULL)
    return((XrmDatabase) NULL);
  assert(client_name != (char *) NULL);
  /*
    Initialize resource database.
  */
  XrmInitialize();
  (void) XGetDefault(display,(char *) client_name,"dummy");
  resource_database=XrmGetDatabase(display);
  /*
    Combine application database.
  */
  p=client_name+(strlen(client_name)-1);
  while ((p > client_name) && (*p != '/'))
    p--;
  if (*p == '/')
    client_name=p+1;
  c=(int) (*client_name);
  if ((c >= XK_a) && (c <= XK_z))
    c-=(XK_a-XK_A);
  else
    if ((c >= XK_agrave) && (c <= XK_odiaeresis))
      c-=(XK_agrave-XK_Agrave);
    else
      if ((c >= XK_oslash) && (c <= XK_thorn))
        c-=(XK_oslash-XK_Ooblique);
#if defined(X11_APPLICATION_PATH)
  (void) FormatLocaleString(filename,MagickPathExtent,"%s%c%s",
    X11_APPLICATION_PATH,c,client_name+1);
  (void) XrmCombineFileDatabase(filename,&resource_database,MagickFalse);
#endif
  if (XResourceManagerString(display) != (char *) NULL)
    {
      /*
        Combine server database.
      */
      server_database=XrmGetStringDatabase(XResourceManagerString(display));
      XrmCombineDatabase(server_database,&resource_database,MagickFalse);
    }
  /*
    Merge user preferences database.
  */
#if defined(X11_PREFERENCES_PATH)
  (void) FormatLocaleString(filename,MagickPathExtent,"%s%src",
    X11_PREFERENCES_PATH,client_name);
  ExpandFilename(filename);
  (void) XrmCombineFileDatabase(filename,&resource_database,MagickFalse);
#endif
  return(resource_database);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t R e s o u r c e I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XGetResourceInfo(image_info,) initializes the ResourceInfo structure.
%
%  The format of the XGetResourceInfo method is:
%
%      void XGetResourceInfo(const ImageInfo *image_info,XrmDatabase database,
%        const char *client_name,XResourceInfo *resource_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o database: Specifies a resource database; returned from
%      XrmGetStringDatabase.
%
%    o client_name:  Specifies the application name used to retrieve
%      resource info from the X server database.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
*/
MagickExport void XGetResourceInfo(const ImageInfo *image_info,
  XrmDatabase database,const char *client_name,XResourceInfo *resource_info)
{
  char
    *directory,
    *resource_value;

  /*
    Initialize resource info fields.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(resource_info != (XResourceInfo *) NULL);
  (void) memset(resource_info,0,sizeof(*resource_info));
  resource_info->resource_database=database;
  resource_info->image_info=(ImageInfo *) image_info;
  (void) SetImageInfoProgressMonitor(resource_info->image_info,
    XMagickProgressMonitor,(void *) NULL);
  resource_info->quantize_info=CloneQuantizeInfo((QuantizeInfo *) NULL);
  resource_info->close_server=MagickTrue;
  resource_info->client_name=AcquireString(client_name);
  resource_value=XGetResourceClass(database,client_name,"backdrop",
    (char *) "False");
  resource_info->backdrop=IsStringTrue(resource_value);
  resource_info->background_color=XGetResourceInstance(database,client_name,
    "background",(char *) "#d6d6d6d6d6d6");
  resource_info->border_color=XGetResourceInstance(database,client_name,
    "borderColor",BorderColor);
  resource_value=XGetResourceClass(database,client_name,"borderWidth",
    (char *) "2");
  resource_info->border_width=(unsigned int) StringToUnsignedLong(
    resource_value);
  resource_value=XGetResourceClass(database,client_name,"colormap",
    (char *) "shared");
  resource_info->colormap=UndefinedColormap;
  if (LocaleCompare("private",resource_value) == 0)
    resource_info->colormap=PrivateColormap;
  if (LocaleCompare("shared",resource_value) == 0)
    resource_info->colormap=SharedColormap;
  if (resource_info->colormap == UndefinedColormap)
    ThrowXWindowException(OptionError,"UnrecognizedColormapType",
      resource_value);
  resource_value=XGetResourceClass(database,client_name,
    "colorRecovery",(char *) "False");
  resource_info->color_recovery=IsStringTrue(resource_value);
  resource_value=XGetResourceClass(database,client_name,"confirmExit",
    (char *) "False");
  resource_info->confirm_exit=IsStringTrue(resource_value);
  resource_value=XGetResourceClass(database,client_name,"confirmEdit",
    (char *) "False");
  resource_info->confirm_edit=IsStringTrue(resource_value);
  resource_value=XGetResourceClass(database,client_name,"delay",(char *) "1");
  resource_info->delay=(unsigned int) StringToUnsignedLong(resource_value);
  resource_info->display_gamma=XGetResourceClass(database,client_name,
    "displayGamma",(char *) "2.2");
  resource_value=XGetResourceClass(database,client_name,"displayWarnings",
    (char *) "True");
  resource_info->display_warnings=IsStringTrue(resource_value);
  resource_info->font=XGetResourceClass(database,client_name,"font",
    (char *) NULL);
  resource_info->font=XGetResourceClass(database,client_name,"fontList",
    resource_info->font);
  resource_info->font_name[0]=XGetResourceClass(database,client_name,"font1",
    (char *) "fixed");
  resource_info->font_name[1]=XGetResourceClass(database,client_name,"font2",
    (char *) "variable");
  resource_info->font_name[2]=XGetResourceClass(database,client_name,"font3",
    (char *) "5x8");
  resource_info->font_name[3]=XGetResourceClass(database,client_name,"font4",
    (char *) "6x10");
  resource_info->font_name[4]=XGetResourceClass(database,client_name,"font5",
    (char *) "7x13bold");
  resource_info->font_name[5]=XGetResourceClass(database,client_name,"font6",
    (char *) "8x13bold");
  resource_info->font_name[6]=XGetResourceClass(database,client_name,"font7",
    (char *) "9x15bold");
  resource_info->font_name[7]=XGetResourceClass(database,client_name,"font8",
    (char *) "10x20");
  resource_info->font_name[8]=XGetResourceClass(database,client_name,"font9",
    (char *) "12x24");
  resource_info->font_name[9]=XGetResourceClass(database,client_name,"font0",
    (char *) "fixed");
  resource_info->font_name[10]=XGetResourceClass(database,client_name,"font0",
    (char *) "fixed");
  resource_info->foreground_color=XGetResourceInstance(database,client_name,
    "foreground",ForegroundColor);
  resource_value=XGetResourceClass(database,client_name,"gammaCorrect",
    (char *) "False");
  resource_info->gamma_correct=IsStringTrue(resource_value);
  resource_info->image_geometry=ConstantString(XGetResourceClass(database,
    client_name,"geometry",(char *) NULL));
  resource_value=XGetResourceClass(database,client_name,"gravity",
    (char *) "Center");
  resource_info->gravity=(GravityType) ParseCommandOption(MagickGravityOptions,
    MagickFalse,resource_value);
  directory=getcwd(resource_info->home_directory,MagickPathExtent);
  (void) directory;
  resource_info->icon_geometry=XGetResourceClass(database,client_name,
    "iconGeometry",(char *) NULL);
  resource_value=XGetResourceClass(database,client_name,"iconic",
    (char *) "False");
  resource_info->iconic=IsStringTrue(resource_value);
  resource_value=XGetResourceClass(database,client_name,"immutable",
    LocaleCompare(client_name,"PerlMagick") == 0 ? (char *) "True" :
    (char *) "False");
  resource_info->immutable=IsStringTrue(resource_value);
  resource_value=XGetResourceClass(database,client_name,"magnify",
    (char *) "3");
  resource_info->magnify=(unsigned int) StringToUnsignedLong(resource_value);
  resource_info->map_type=XGetResourceClass(database,client_name,"map",
    (char *) NULL);
  resource_info->matte_color=XGetResourceInstance(database,client_name,
    "mattecolor",(char *) NULL);
  resource_info->name=ConstantString(XGetResourceClass(database,client_name,
    "name",(char *) NULL));
  resource_info->pen_colors[0]=XGetResourceClass(database,client_name,"pen1",
    (char *) "black");
  resource_info->pen_colors[1]=XGetResourceClass(database,client_name,"pen2",
    (char *) "blue");
  resource_info->pen_colors[2]=XGetResourceClass(database,client_name,"pen3",
    (char *) "cyan");
  resource_info->pen_colors[3]=XGetResourceClass(database,client_name,"pen4",
    (char *) "green");
  resource_info->pen_colors[4]=XGetResourceClass(database,client_name,"pen5",
    (char *) "gray");
  resource_info->pen_colors[5]=XGetResourceClass(database,client_name,"pen6",
    (char *) "red");
  resource_info->pen_colors[6]=XGetResourceClass(database,client_name,"pen7",
    (char *) "magenta");
  resource_info->pen_colors[7]=XGetResourceClass(database,client_name,"pen8",
    (char *) "yellow");
  resource_info->pen_colors[8]=XGetResourceClass(database,client_name,"pen9",
    (char *) "white");
  resource_info->pen_colors[9]=XGetResourceClass(database,client_name,"pen0",
    (char *) "gray");
  resource_info->pen_colors[10]=XGetResourceClass(database,client_name,"pen0",
    (char *) "gray");
  resource_value=XGetResourceClass(database,client_name,"pause",(char *) "0");
  resource_info->pause=(unsigned int) StringToUnsignedLong(resource_value);
  resource_value=XGetResourceClass(database,client_name,"quantum",(char *) "1");
  resource_info->quantum=StringToLong(resource_value);
  resource_info->text_font=XGetResourceClass(database,client_name,(char *)
    "font",(char *) "fixed");
  resource_info->text_font=XGetResourceClass(database,client_name,
    "textFontList",resource_info->text_font);
  resource_info->title=XGetResourceClass(database,client_name,"title",
    (char *) NULL);
  resource_value=XGetResourceClass(database,client_name,"undoCache",
    (char *) "256");
  resource_info->undo_cache=(unsigned int) StringToUnsignedLong(resource_value);
  resource_value=XGetResourceClass(database,client_name,"update",
    (char *) "False");
  resource_info->update=IsStringTrue(resource_value);
  resource_value=XGetResourceClass(database,client_name,"usePixmap",
    (char *) "True");
  resource_info->use_pixmap=IsStringTrue(resource_value);
  resource_value=XGetResourceClass(database,client_name,"sharedMemory",
    (char *) "True");
  resource_info->use_shared_memory=IsStringTrue(resource_value);
  resource_info->visual_type=XGetResourceClass(database,client_name,"visual",
    (char *) NULL);
  resource_info->window_group=XGetResourceClass(database,client_name,
    "windowGroup",(char *) NULL);
  resource_info->window_id=XGetResourceClass(database,client_name,"window",
    (char *) NULL);
  resource_info->write_filename=XGetResourceClass(database,client_name,
    "writeFilename",(char *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t R e s o u r c e I n s t a n c e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XGetResourceInstance() queries the X server for the specified resource name.
%  If the resource name is not defined in the database, the supplied default
%  value is returned.
%
%  The format of the XGetResourceInstance method is:
%
%      char *XGetResourceInstance(XrmDatabase database,const char *client_name,
%        const char *keyword,const char *resource_default)
%
%  A description of each parameter follows:
%
%    o database: Specifies a resource database; returned from
%      XrmGetStringDatabase.
%
%    o client_name:  Specifies the application name used to retrieve
%      resource info from the X server database.
%
%    o keyword: Specifies the keyword of the value being retrieved.
%
%    o resource_default: Specifies the default value to return if the query
%      fails to find the specified keyword/class.
%
*/
MagickExport char *XGetResourceInstance(XrmDatabase database,
  const char *client_name,const char *keyword,const char *resource_default)
{
  char
    *resource_type,
    resource_name[MagickPathExtent];

  Status
    status;

  XrmValue
    resource_value;

  if (database == (XrmDatabase) NULL)
    return((char *) resource_default);
  *resource_name='\0';
  if (keyword != (char *) NULL)
    (void) FormatLocaleString(resource_name,MagickPathExtent,"%s.%s",client_name,
      keyword);
  status=XrmGetResource(database,resource_name,"ImageMagick",&resource_type,
    &resource_value);
  if (status == False)
    return((char *) resource_default);
  return(resource_value.addr);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t S c r e e n D e n s i t y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XGetScreenDensity() returns the density of the X server screen in
%  dots-per-inch.
%
%  The format of the XGetScreenDensity method is:
%
%      char *XGetScreenDensity(Display *display)
%
%  A description of each parameter follows:
%
%    o density: XGetScreenDensity() returns the density of the X screen in
%      dots-per-inch.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
*/
MagickExport char *XGetScreenDensity(Display *display)
{
  char
    density[MagickPathExtent];

  double
    x_density,
    y_density;

  /*
    Set density as determined by screen size.
  */
  x_density=((((double) DisplayWidth(display,XDefaultScreen(display)))*25.4)/
    ((double) DisplayWidthMM(display,XDefaultScreen(display))));
  y_density=((((double) DisplayHeight(display,XDefaultScreen(display)))*25.4)/
    ((double) DisplayHeightMM(display,XDefaultScreen(display))));
  (void) FormatLocaleString(density,MagickPathExtent,"%gx%g",x_density,
    y_density);
  return(GetPageGeometry(density));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X G e t S u b w i n d o w                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XGetSubwindow() returns the subwindow of a window chosen the user with the
%  pointer and a button press.
%
%  The format of the XGetSubwindow method is:
%
%      Window XGetSubwindow(Display *display,Window window,int x,int y)
%
%  A description of each parameter follows:
%
%    o subwindow: XGetSubwindow() returns NULL if no subwindow is found
%      otherwise the subwindow is returned.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window.
%
%    o x: the x coordinate of the pointer relative to the origin of the
%      window.
%
%    o y: the y coordinate of the pointer relative to the origin of the
%      window.
%
*/
static Window XGetSubwindow(Display *display,Window window,int x,int y)
{
  int
    x_offset,
    y_offset;

  Status
    status;

  Window
    source_window,
    target_window;

  assert(display != (Display *) NULL);
  source_window=XRootWindow(display,XDefaultScreen(display));
  if (window == (Window) NULL)
    return(source_window);
  target_window=window;
  for ( ; ; )
  {
    status=XTranslateCoordinates(display,source_window,window,x,y,
      &x_offset,&y_offset,&target_window);
    if (status != True)
      break;
    if (target_window == (Window) NULL)
      break;
    source_window=window;
    window=target_window;
    x=x_offset;
    y=y_offset;
  }
  if (target_window == (Window) NULL)
    target_window=window;
  return(target_window);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t W i n d o w C o l o r                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XGetWindowColor() returns the color of a pixel interactively chosen from the
%  X server.
%
%  The format of the XGetWindowColor method is:
%
%      MagickBooleanType XGetWindowColor(Display *display,XWindows *windows,
%        char *name,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o name: the name of the color if found in the X Color Database is
%      returned in this character string.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate MagickBooleanType XGetWindowColor(Display *display,
  XWindows *windows,char *name,ExceptionInfo *exception)
{
  int
    x,
    y;

  PixelInfo
    pixel;

  RectangleInfo
    crop_info;

  Status
    status;

  Window
    child,
    client_window,
    root_window,
    target_window;

  XColor
    color;

  XImage
    *ximage;

  XWindowAttributes
    window_attributes;

  /*
    Choose a pixel from the X server.
  */
  assert(display != (Display *) NULL);
  assert(name != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",name);
  *name='\0';
  target_window=XSelectWindow(display,&crop_info);
  if (target_window == (Window) NULL)
    return(MagickFalse);
  root_window=XRootWindow(display,XDefaultScreen(display));
  client_window=target_window;
  if (target_window != root_window)
    {
      unsigned int
        d;

      /*
        Get client window.
      */
      status=XGetGeometry(display,target_window,&root_window,&x,&x,&d,&d,&d,&d);
      if (status != False)
        {
          client_window=XClientWindow(display,target_window);
          target_window=client_window;
        }
    }
  /*
    Verify window is viewable.
  */
  status=XGetWindowAttributes(display,target_window,&window_attributes);
  if ((status == False) || (window_attributes.map_state != IsViewable))
    return(MagickFalse);
  /*
    Get window X image.
  */
  (void) XTranslateCoordinates(display,root_window,target_window,
    (int) crop_info.x,(int) crop_info.y,&x,&y,&child);
  ximage=XGetImage(display,target_window,x,y,1,1,AllPlanes,ZPixmap);
  if (ximage == (XImage *) NULL)
    return(MagickFalse);
  color.pixel=XGetPixel(ximage,0,0);
  XDestroyImage(ximage);
  /*
    Match color against the color database.
  */
  (void) XQueryColor(display,window_attributes.colormap,&color);
  pixel.red=(double) ScaleShortToQuantum(color.red);
  pixel.green=(double) ScaleShortToQuantum(color.green);
  pixel.blue=(double) ScaleShortToQuantum(color.blue);
  pixel.alpha=(MagickRealType) OpaqueAlpha;
  (void) QueryColorname(windows->image.image,&pixel,X11Compliance,name,
    exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X G e t W i n d o w I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XGetWindowImage() reads an image from the target X window and returns it.
%  XGetWindowImage() optionally descends the window hierarchy and overlays the
%  target image with each child image in an optimized fashion.  Any child
%  window that have the same visual, colormap, and are contained by its parent
%  are exempted.
%
%  The format of the XGetWindowImage method is:
%
%      Image *XGetWindowImage(Display *display,const Window window,
%        const unsigned int borders,const unsigned int level,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o window: Specifies the window to obtain the image from.
%
%    o borders: Specifies whether borders pixels are to be saved with
%      the image.
%
%    o level: Specifies an unsigned integer representing the level of
%      decent in the window hierarchy.  This value must be zero or one on
%      the initial call to XGetWindowImage.  A value of zero returns after
%      one call.  A value of one causes the function to descend the window
%      hierarchy and overlay the target image with each subwindow image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *XGetWindowImage(Display *display,const Window window,
  const unsigned int borders,const unsigned int level,ExceptionInfo *exception)
{
  typedef struct _ColormapInfo
  {
    Colormap
      colormap;

    XColor
      *colors;

    struct _ColormapInfo
      *next;
  } ColormapInfo;

  typedef struct _WindowInfo
  {
    Window
      window,
      parent;

    Visual
      *visual;

    Colormap
      colormap;

    XSegment
      bounds;

    RectangleInfo
      crop_info;
  } WindowInfo;

  int
    display_height,
    display_width,
    id,
    x_offset,
    y_offset;

  Quantum
    index;

  RectangleInfo
    crop_info;

  register int
    i;

  static ColormapInfo
    *colormap_info = (ColormapInfo *) NULL;

  static int
    max_windows = 0,
    number_windows = 0;

  static WindowInfo
    *window_info;

  Status
    status;

  Window
    child,
    root_window;

  XWindowAttributes
    window_attributes;

  /*
    Verify window is viewable.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  status=XGetWindowAttributes(display,window,&window_attributes);
  if ((status == False) || (window_attributes.map_state != IsViewable))
    return((Image *) NULL);
  /*
    Cropping rectangle is relative to root window.
  */
  root_window=XRootWindow(display,XDefaultScreen(display));
  (void) XTranslateCoordinates(display,window,root_window,0,0,&x_offset,
    &y_offset,&child);
  crop_info.x=(ssize_t) x_offset;
  crop_info.y=(ssize_t) y_offset;
  crop_info.width=(size_t) window_attributes.width;
  crop_info.height=(size_t) window_attributes.height;
  if (borders != MagickFalse)
    {
      /*
        Include border in image.
      */
      crop_info.x-=(ssize_t) window_attributes.border_width;
      crop_info.y-=(ssize_t) window_attributes.border_width;
      crop_info.width+=(size_t) (window_attributes.border_width << 1);
      crop_info.height+=(size_t) (window_attributes.border_width << 1);
    }
  /*
    Crop to root window.
  */
  if (crop_info.x < 0)
    {
      crop_info.width+=crop_info.x;
      crop_info.x=0;
    }
  if (crop_info.y < 0)
    {
      crop_info.height+=crop_info.y;
      crop_info.y=0;
    }
  display_width=XDisplayWidth(display,XDefaultScreen(display));
  if ((int) (crop_info.x+crop_info.width) > display_width)
    crop_info.width=(size_t) (display_width-crop_info.x);
  display_height=XDisplayHeight(display,XDefaultScreen(display));
  if ((int) (crop_info.y+crop_info.height) > display_height)
    crop_info.height=(size_t) (display_height-crop_info.y);
  /*
    Initialize window info attributes.
  */
  if (number_windows >= max_windows)
    {
      /*
        Allocate or resize window info buffer.
      */
      max_windows+=1024;
      if (window_info == (WindowInfo *) NULL)
        window_info=(WindowInfo *) AcquireQuantumMemory((size_t) max_windows,
          sizeof(*window_info));
      else
        window_info=(WindowInfo *) ResizeQuantumMemory(window_info,(size_t)
          max_windows,sizeof(*window_info));
    }
  if (window_info == (WindowInfo *) NULL)
    {
      ThrowXWindowException(ResourceLimitError,"MemoryAllocationFailed","...");
      return((Image *) NULL);
    }
  id=number_windows++;
  window_info[id].window=window;
  window_info[id].visual=window_attributes.visual;
  window_info[id].colormap=window_attributes.colormap;
  window_info[id].bounds.x1=(short) crop_info.x;
  window_info[id].bounds.y1=(short) crop_info.y;
  window_info[id].bounds.x2=(short) (crop_info.x+(int) crop_info.width-1);
  window_info[id].bounds.y2=(short) (crop_info.y+(int) crop_info.height-1);
  crop_info.x-=x_offset;
  crop_info.y-=y_offset;
  window_info[id].crop_info=crop_info;
  if (level != 0)
    {
      unsigned int
        number_children;

      Window
        *children;

      /*
        Descend the window hierarchy.
      */
      status=XQueryTree(display,window,&root_window,&window_info[id].parent,
        &children,&number_children);
      for (i=0; i < id; i++)
        if ((window_info[i].window == window_info[id].parent) &&
            (window_info[i].visual == window_info[id].visual) &&
            (window_info[i].colormap == window_info[id].colormap))
          {
            if ((window_info[id].bounds.x1 < window_info[i].bounds.x1) ||
                (window_info[id].bounds.x2 > window_info[i].bounds.x2) ||
                (window_info[id].bounds.y1 < window_info[i].bounds.y1) ||
                (window_info[id].bounds.y2 > window_info[i].bounds.y2))
              {
                /*
                  Eliminate windows not circumscribed by their parent.
                */
                number_windows--;
                break;
              }
          }
      if ((status == True) && (number_children != 0))
        {
          for (i=0; i < (int) number_children; i++)
            (void) XGetWindowImage(display,children[i],MagickFalse,level+1,
              exception);
          (void) XFree((void *) children);
        }
    }
  if (level <= 1)
    {
      CacheView
        *composite_view;

      ColormapInfo
        *next;

      Image
        *composite_image,
        *image;

      int
        y;

      MagickBooleanType
        import;

      register int
        j,
        x;

      register Quantum
        *magick_restrict q;

      register size_t
        pixel;

      unsigned int
        number_colors;

      XColor
        *colors;

      XImage
        *ximage;

      /*
        Get X image for each window in the list.
      */
      image=NewImageList();
      for (id=0; id < number_windows; id++)
      {
        /*
          Does target window intersect top level window?
        */
        import=((window_info[id].bounds.x2 >= window_info[0].bounds.x1) &&
           (window_info[id].bounds.x1 <= window_info[0].bounds.x2) &&
           (window_info[id].bounds.y2 >= window_info[0].bounds.y1) &&
           (window_info[id].bounds.y1 <= window_info[0].bounds.y2)) ?
          MagickTrue : MagickFalse;
        /*
          Is target window contained by another window with the same colormap?
        */
        for (j=0; j < id; j++)
          if ((window_info[id].visual == window_info[j].visual) &&
              (window_info[id].colormap == window_info[j].colormap))
            {
              if ((window_info[id].bounds.x1 >= window_info[j].bounds.x1) &&
                  (window_info[id].bounds.x2 <= window_info[j].bounds.x2) &&
                  (window_info[id].bounds.y1 >= window_info[j].bounds.y1) &&
                  (window_info[id].bounds.y2 <= window_info[j].bounds.y2))
                import=MagickFalse;
            }
        if (import == MagickFalse)
          continue;
        /*
          Get X image.
        */
        ximage=XGetImage(display,window_info[id].window,(int)
          window_info[id].crop_info.x,(int) window_info[id].crop_info.y,
          (unsigned int) window_info[id].crop_info.width,(unsigned int)
          window_info[id].crop_info.height,AllPlanes,ZPixmap);
        if (ximage == (XImage *) NULL)
          continue;
        /*
          Initialize window colormap.
        */
        number_colors=0;
        colors=(XColor *) NULL;
        if (window_info[id].colormap != (Colormap) NULL)
          {
            ColormapInfo
              *p;

            /*
              Search colormap list for window colormap.
            */
            number_colors=(unsigned int) window_info[id].visual->map_entries;
            for (p=colormap_info; p != (ColormapInfo *) NULL; p=p->next)
              if (p->colormap == window_info[id].colormap)
                break;
            if (p == (ColormapInfo *) NULL)
              {
                /*
                  Get the window colormap.
                */
                colors=(XColor *) AcquireQuantumMemory(number_colors,
                  sizeof(*colors));
                if (colors == (XColor *) NULL)
                  {
                    XDestroyImage(ximage);
                    return((Image *) NULL);
                  }
                if ((window_info[id].visual->klass != DirectColor) &&
                    (window_info[id].visual->klass != TrueColor))
                  for (i=0; i < (int) number_colors; i++)
                  {
                    colors[i].pixel=(size_t) i;
                    colors[i].pad='\0';
                  }
                else
                  {
                    size_t
                      blue,
                      blue_bit,
                      green,
                      green_bit,
                      red,
                      red_bit;

                    /*
                      DirectColor or TrueColor visual.
                    */
                    red=0;
                    green=0;
                    blue=0;
                    red_bit=window_info[id].visual->red_mask &
                      (~(window_info[id].visual->red_mask)+1);
                    green_bit=window_info[id].visual->green_mask &
                      (~(window_info[id].visual->green_mask)+1);
                    blue_bit=window_info[id].visual->blue_mask &
                      (~(window_info[id].visual->blue_mask)+1);
                    for (i=0; i < (int) number_colors; i++)
                    {
                      colors[i].pixel=(unsigned long) (red | green | blue);
                      colors[i].pad='\0';
                      red+=red_bit;
                      if (red > window_info[id].visual->red_mask)
                        red=0;
                      green+=green_bit;
                      if (green > window_info[id].visual->green_mask)
                        green=0;
                      blue+=blue_bit;
                      if (blue > window_info[id].visual->blue_mask)
                        blue=0;
                    }
                  }
                (void) XQueryColors(display,window_info[id].colormap,colors,
                  (int) number_colors);
                /*
                  Append colormap to colormap list.
                */
                p=(ColormapInfo *) AcquireMagickMemory(sizeof(*p));
                if (p == (ColormapInfo *) NULL)
                  return((Image *) NULL);
                p->colormap=window_info[id].colormap;
                p->colors=colors;
                p->next=colormap_info;
                colormap_info=p;
              }
            colors=p->colors;
          }
        /*
          Allocate image structure.
        */
        composite_image=AcquireImage((ImageInfo *) NULL,exception);
        if (composite_image == (Image *) NULL)
          {
            XDestroyImage(ximage);
            return((Image *) NULL);
          }
        /*
          Convert X image to MIFF format.
        */
        if ((window_info[id].visual->klass != TrueColor) &&
            (window_info[id].visual->klass != DirectColor))
          composite_image->storage_class=PseudoClass;
        composite_image->columns=(size_t) ximage->width;
        composite_image->rows=(size_t) ximage->height;
        composite_view=AcquireAuthenticCacheView(composite_image,exception);
        switch (composite_image->storage_class)
        {
          case DirectClass:
          default:
          {
            register size_t
              color,
              index;

            size_t
              blue_mask,
              blue_shift,
              green_mask,
              green_shift,
              red_mask,
              red_shift;

            /*
              Determine shift and mask for red, green, and blue.
            */
            red_mask=window_info[id].visual->red_mask;
            red_shift=0;
            while ((red_mask != 0) && ((red_mask & 0x01) == 0))
            {
              red_mask>>=1;
              red_shift++;
            }
            green_mask=window_info[id].visual->green_mask;
            green_shift=0;
            while ((green_mask != 0) && ((green_mask & 0x01) == 0))
            {
              green_mask>>=1;
              green_shift++;
            }
            blue_mask=window_info[id].visual->blue_mask;
            blue_shift=0;
            while ((blue_mask != 0) && ((blue_mask & 0x01) == 0))
            {
              blue_mask>>=1;
              blue_shift++;
            }
            /*
              Convert X image to DirectClass packets.
            */
            if ((number_colors != 0) &&
                (window_info[id].visual->klass == DirectColor))
              for (y=0; y < (int) composite_image->rows; y++)
              {
                q=QueueCacheViewAuthenticPixels(composite_view,0,(ssize_t) y,
                  composite_image->columns,1,exception);
                if (q == (Quantum *) NULL)
                  break;
                for (x=0; x < (int) composite_image->columns; x++)
                {
                  pixel=XGetPixel(ximage,x,y);
                  index=(pixel >> red_shift) & red_mask;
                  SetPixelRed(composite_image,
                    ScaleShortToQuantum(colors[index].red),q);
                  index=(pixel >> green_shift) & green_mask;
                  SetPixelGreen(composite_image,
                    ScaleShortToQuantum(colors[index].green),q);
                  index=(pixel >> blue_shift) & blue_mask;
                  SetPixelBlue(composite_image,
                    ScaleShortToQuantum(colors[index].blue),q);
                  q+=GetPixelChannels(composite_image);
                }
                status=SyncCacheViewAuthenticPixels(composite_view,exception);
                if (status == MagickFalse)
                  break;
              }
            else
              for (y=0; y < (int) composite_image->rows; y++)
              {
                q=QueueCacheViewAuthenticPixels(composite_view,0,(ssize_t) y,
                  composite_image->columns,1,exception);
                if (q == (Quantum *) NULL)
                  break;
                for (x=0; x < (int) composite_image->columns; x++)
                {
                  pixel=XGetPixel(ximage,x,y);
                  color=(pixel >> red_shift) & red_mask;
                  if (red_mask != 0)
                    color=(65535UL*color)/red_mask;
                  SetPixelRed(composite_image,ScaleShortToQuantum(
                    (unsigned short) color),q);
                  color=(pixel >> green_shift) & green_mask;
                  if (green_mask != 0)
                    color=(65535UL*color)/green_mask;
                  SetPixelGreen(composite_image,ScaleShortToQuantum(
                    (unsigned short) color),q);
                  color=(pixel >> blue_shift) & blue_mask;
                  if (blue_mask != 0)
                    color=(65535UL*color)/blue_mask;
                  SetPixelBlue(composite_image,ScaleShortToQuantum(
                    (unsigned short) color),q);
                  q+=GetPixelChannels(composite_image);
                }
                status=SyncCacheViewAuthenticPixels(composite_view,exception);
                if (status == MagickFalse)
                  break;
              }
            break;
          }
          case PseudoClass:
          {
            /*
              Create colormap.
            */
            status=AcquireImageColormap(composite_image,number_colors,
              exception);
            if (status == MagickFalse)
              {
                XDestroyImage(ximage);
                composite_image=DestroyImage(composite_image);
                return((Image *) NULL);
              }
            for (i=0; i < (int) composite_image->colors; i++)
            {
              composite_image->colormap[colors[i].pixel].red=(double)
                ScaleShortToQuantum(colors[i].red);
              composite_image->colormap[colors[i].pixel].green=(double)
                ScaleShortToQuantum(colors[i].green);
              composite_image->colormap[colors[i].pixel].blue=(double)
                ScaleShortToQuantum(colors[i].blue);
            }
            /*
              Convert X image to PseudoClass packets.
            */
            for (y=0; y < (int) composite_image->rows; y++)
            {
              q=QueueCacheViewAuthenticPixels(composite_view,0,(ssize_t) y,
                composite_image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (int) composite_image->columns; x++)
              {
                index=(Quantum) XGetPixel(ximage,x,y);
                SetPixelIndex(composite_image,index,q);
                SetPixelViaPixelInfo(composite_image,
                  composite_image->colormap+(ssize_t) index,q);
                q+=GetPixelChannels(composite_image);
              }
              status=SyncCacheViewAuthenticPixels(composite_view,exception);
              if (status == MagickFalse)
                break;
            }
            break;
          }
        }
        composite_view=DestroyCacheView(composite_view);
        XDestroyImage(ximage);
        if (image == (Image *) NULL)
          {
            image=composite_image;
            continue;
          }
        /*
          Composite any children in back-to-front order.
        */
        (void) XTranslateCoordinates(display,window_info[id].window,window,0,0,
          &x_offset,&y_offset,&child);
        x_offset-=(int) crop_info.x;
        if (x_offset < 0)
          x_offset=0;
        y_offset-=(int) crop_info.y;
        if (y_offset < 0)
          y_offset=0;
        (void) CompositeImage(image,composite_image,CopyCompositeOp,MagickTrue,
          (ssize_t) x_offset,(ssize_t) y_offset,exception);
        composite_image=DestroyImage(composite_image);
      }
      /*
        Relinquish resources.
      */
      while (colormap_info != (ColormapInfo *) NULL)
      {
        next=colormap_info->next;
        colormap_info->colors=(XColor *) RelinquishMagickMemory(
          colormap_info->colors);
        colormap_info=(ColormapInfo *) RelinquishMagickMemory(colormap_info);
        colormap_info=next;
      }
      /*
        Relinquish resources and restore initial state.
      */
      window_info=(WindowInfo *) RelinquishMagickMemory(window_info);
      max_windows=0;
      number_windows=0;
      colormap_info=(ColormapInfo *) NULL;
      return(image);
    }
  return((Image *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t W i n d o w I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XGetWindowInfo() initializes the XWindowInfo structure.
%
%  The format of the XGetWindowInfo method is:
%
%      void XGetWindowInfo(Display *display,XVisualInfo *visual_info,
%        XStandardColormap *map_info,XPixelInfo *pixel,XFontStruct *font_info,
%        XResourceInfo *resource_info,XWindowInfo *window)
%        resource_info,window)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o visual_info: Specifies a pointer to a X11 XVisualInfo structure;
%      returned from XGetVisualInfo.
%
%    o map_info: If map_type is specified, this structure is initialized
%      with info from the Standard Colormap.
%
%    o pixel: Specifies a pointer to a XPixelInfo structure.
%
%    o font_info: Specifies a pointer to a XFontStruct structure.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
*/
MagickPrivate void XGetWindowInfo(Display *display,XVisualInfo *visual_info,
  XStandardColormap *map_info,XPixelInfo *pixel,XFontStruct *font_info,
  XResourceInfo *resource_info,XWindowInfo *window)
{
  /*
    Initialize window info.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(visual_info != (XVisualInfo *) NULL);
  assert(map_info != (XStandardColormap *) NULL);
  assert(pixel != (XPixelInfo *) NULL);
  assert(resource_info != (XResourceInfo *) NULL);
  assert(window != (XWindowInfo *) NULL);
  if (window->id != (Window) NULL)
    {
      if (window->cursor != (Cursor) NULL)
        (void) XFreeCursor(display,window->cursor);
      if (window->busy_cursor != (Cursor) NULL)
        (void) XFreeCursor(display,window->busy_cursor);
      if (window->highlight_stipple != (Pixmap) NULL)
        (void) XFreePixmap(display,window->highlight_stipple);
      if (window->shadow_stipple != (Pixmap) NULL)
        (void) XFreePixmap(display,window->shadow_stipple);
      if (window->name == (char *) NULL)
        window->name=AcquireString("");
      if (window->icon_name == (char *) NULL)
        window->icon_name=AcquireString("");
    }
  else
    {
      /*
        Initialize these attributes just once.
      */
      window->id=(Window) NULL;
      if (window->name == (char *) NULL)
        window->name=AcquireString("");
      if (window->icon_name == (char *) NULL)
        window->icon_name=AcquireString("");
      window->x=XDisplayWidth(display,visual_info->screen) >> 1;
      window->y=XDisplayWidth(display,visual_info->screen) >> 1;
      window->ximage=(XImage *) NULL;
      window->matte_image=(XImage *) NULL;
      window->pixmap=(Pixmap) NULL;
      window->matte_pixmap=(Pixmap) NULL;
      window->mapped=MagickFalse;
      window->stasis=MagickFalse;
      window->shared_memory=MagickTrue;
      window->segment_info=(void *) NULL;
#if defined(MAGICKCORE_HAVE_SHARED_MEMORY)
      {
        XShmSegmentInfo
          *segment_info;

        if (window->segment_info == (void *) NULL)
          window->segment_info=AcquireCriticalMemory(2*sizeof(*segment_info));
        segment_info=(XShmSegmentInfo *) window->segment_info;
        segment_info[0].shmid=(-1);
        segment_info[0].shmaddr=(char *) NULL;
        segment_info[1].shmid=(-1);
        segment_info[1].shmaddr=(char *) NULL;
      }
#endif
    }
  /*
    Initialize these attributes every time function is called.
  */
  window->screen=visual_info->screen;
  window->root=XRootWindow(display,visual_info->screen);
  window->visual=visual_info->visual;
  window->storage_class=(unsigned int) visual_info->klass;
  window->depth=(unsigned int) visual_info->depth;
  window->visual_info=visual_info;
  window->map_info=map_info;
  window->pixel_info=pixel;
  window->font_info=font_info;
  window->cursor=XCreateFontCursor(display,XC_left_ptr);
  window->busy_cursor=XCreateFontCursor(display,XC_watch);
  window->geometry=(char *) NULL;
  window->icon_geometry=(char *) NULL;
  if (resource_info->icon_geometry != (char *) NULL)
    (void) CloneString(&window->icon_geometry,resource_info->icon_geometry);
  window->crop_geometry=(char *) NULL;
  window->flags=(size_t) PSize;
  window->width=1;
  window->height=1;
  window->min_width=1;
  window->min_height=1;
  window->width_inc=1;
  window->height_inc=1;
  window->border_width=resource_info->border_width;
  window->annotate_context=pixel->annotate_context;
  window->highlight_context=pixel->highlight_context;
  window->widget_context=pixel->widget_context;
  window->shadow_stipple=(Pixmap) NULL;
  window->highlight_stipple=(Pixmap) NULL;
  window->use_pixmap=MagickTrue;
  window->immutable=MagickFalse;
  window->shape=MagickFalse;
  window->data=0;
  window->mask=(int) (CWBackingStore | CWBackPixel | CWBackPixmap |
    CWBitGravity | CWBorderPixel | CWColormap | CWCursor | CWDontPropagate |
    CWEventMask | CWOverrideRedirect | CWSaveUnder | CWWinGravity);
  window->attributes.background_pixel=pixel->background_color.pixel;
  window->attributes.background_pixmap=(Pixmap) NULL;
  window->attributes.bit_gravity=ForgetGravity;
  window->attributes.backing_store=WhenMapped;
  window->attributes.save_under=MagickTrue;
  window->attributes.border_pixel=pixel->border_color.pixel;
  window->attributes.colormap=map_info->colormap;
  window->attributes.cursor=window->cursor;
  window->attributes.do_not_propagate_mask=NoEventMask;
  window->attributes.event_mask=NoEventMask;
  window->attributes.override_redirect=MagickFalse;
  window->attributes.win_gravity=NorthWestGravity;
  window->orphan=MagickFalse;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X H i g h l i g h t E l l i p s e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XHighlightEllipse() puts a border on the X server around a region defined by
%  highlight_info.
%
%  The format of the XHighlightEllipse method is:
%
%      void XHighlightEllipse(Display *display,Window window,
%        GC annotate_context,const RectangleInfo *highlight_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window structure.
%
%    o annotate_context: Specifies a pointer to a GC structure.
%
%    o highlight_info: Specifies a pointer to a RectangleInfo structure.  It
%      contains the extents of any highlighting rectangle.
%
*/
MagickPrivate void XHighlightEllipse(Display *display,Window window,
  GC annotate_context,const RectangleInfo *highlight_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(window != (Window) NULL);
  assert(annotate_context != (GC) NULL);
  assert(highlight_info != (RectangleInfo *) NULL);
  if ((highlight_info->width < 4) || (highlight_info->height < 4))
    return;
  (void) XDrawArc(display,window,annotate_context,(int) highlight_info->x,
    (int) highlight_info->y,(unsigned int) highlight_info->width-1,
    (unsigned int) highlight_info->height-1,0,360*64);
  (void) XDrawArc(display,window,annotate_context,(int) highlight_info->x+1,
    (int) highlight_info->y+1,(unsigned int) highlight_info->width-3,
    (unsigned int) highlight_info->height-3,0,360*64);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X H i g h l i g h t L i n e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XHighlightLine() puts a border on the X server around a region defined by
%  highlight_info.
%
%  The format of the XHighlightLine method is:
%
%      void XHighlightLine(Display *display,Window window,GC annotate_context,
%        const XSegment *highlight_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window structure.
%
%    o annotate_context: Specifies a pointer to a GC structure.
%
%    o highlight_info: Specifies a pointer to a RectangleInfo structure.  It
%      contains the extents of any highlighting rectangle.
%
*/
MagickPrivate void XHighlightLine(Display *display,Window window,
  GC annotate_context,const XSegment *highlight_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(window != (Window) NULL);
  assert(annotate_context != (GC) NULL);
  assert(highlight_info != (XSegment *) NULL);
  (void) XDrawLine(display,window,annotate_context,highlight_info->x1,
    highlight_info->y1,highlight_info->x2,highlight_info->y2);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X H i g h l i g h t R e c t a n g l e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XHighlightRectangle() puts a border on the X server around a region defined
%  by highlight_info.
%
%  The format of the XHighlightRectangle method is:
%
%      void XHighlightRectangle(Display *display,Window window,
%        GC annotate_context,const RectangleInfo *highlight_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window structure.
%
%    o annotate_context: Specifies a pointer to a GC structure.
%
%    o highlight_info: Specifies a pointer to a RectangleInfo structure.  It
%      contains the extents of any highlighting rectangle.
%
*/
MagickPrivate void XHighlightRectangle(Display *display,Window window,
  GC annotate_context,const RectangleInfo *highlight_info)
{
  assert(display != (Display *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(window != (Window) NULL);
  assert(annotate_context != (GC) NULL);
  assert(highlight_info != (RectangleInfo *) NULL);
  if ((highlight_info->width < 4) || (highlight_info->height < 4))
    return;
  (void) XDrawRectangle(display,window,annotate_context,(int) highlight_info->x,
    (int) highlight_info->y,(unsigned int) highlight_info->width-1,
    (unsigned int) highlight_info->height-1);
  (void) XDrawRectangle(display,window,annotate_context,(int) highlight_info->x+
    1,(int) highlight_info->y+1,(unsigned int) highlight_info->width-3,
    (unsigned int) highlight_info->height-3);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X I m p o r t I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XImportImage() reads an image from an X window.
%
%  The format of the XImportImage method is:
%
%      Image *XImportImage(const ImageInfo *image_info,XImportInfo *ximage_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o ximage_info: Specifies a pointer to an XImportInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *XImportImage(const ImageInfo *image_info,
  XImportInfo *ximage_info,ExceptionInfo *exception)
{
  Colormap
    *colormaps;

  Display
    *display;

  Image
    *image;

  int
    number_colormaps,
    number_windows,
    x;

  RectangleInfo
    crop_info;

  Status
    status;

  Window
    *children,
    client,
    prior_target,
    root,
    target;

  XTextProperty
    window_name;

  /*
    Open X server connection.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(ximage_info != (XImportInfo *) NULL);
  display=XOpenDisplay(image_info->server_name);
  if (display == (Display *) NULL)
    {
      ThrowXWindowException(XServerError,"UnableToOpenXServer",
        XDisplayName(image_info->server_name));
      return((Image *) NULL);
    }
  /*
    Set our forgiving exception handler.
  */
  (void) XSetErrorHandler(XError);
  /*
    Select target window.
  */
  crop_info.x=0;
  crop_info.y=0;
  crop_info.width=0;
  crop_info.height=0;
  root=XRootWindow(display,XDefaultScreen(display));
  target=(Window) NULL;
  if (*image_info->filename != '\0')
    {
      if (LocaleCompare(image_info->filename,"root") == 0)
        target=root;
      else
        {
          /*
            Select window by ID or name.
          */
          if (isdigit((int) ((unsigned char) *image_info->filename)) != 0)
            target=XWindowByID(display,root,(Window)
              strtol(image_info->filename,(char **) NULL,0));
          if (target == (Window) NULL)
            target=XWindowByName(display,root,image_info->filename);
          if (target == (Window) NULL)
            ThrowXWindowException(XServerError,"NoWindowWithSpecifiedIDExists",
              image_info->filename);
        }
    }
  /*
    If target window is not defined, interactively select one.
  */
  prior_target=target;
  if (target == (Window) NULL)
    target=XSelectWindow(display,&crop_info);
  if (target == (Window) NULL)
    ThrowXWindowException(XServerError,"UnableToReadXWindowImage",
      image_info->filename);
  client=target;   /* obsolete */
  if (target != root)
    {
      unsigned int
        d;

      status=XGetGeometry(display,target,&root,&x,&x,&d,&d,&d,&d);
      if (status != False)
        {
          for ( ; ; )
          {
            Window
              parent;

            /*
              Find window manager frame.
            */
            status=XQueryTree(display,target,&root,&parent,&children,&d);
            if ((status != False) && (children != (Window *) NULL))
              (void) XFree((char *) children);
            if ((status == False) || (parent == (Window) NULL) ||
                (parent == root))
              break;
            target=parent;
          }
          /*
            Get client window.
          */
          client=XClientWindow(display,target);
          if (ximage_info->frame == MagickFalse)
            target=client;
          if ((ximage_info->frame == MagickFalse) &&
              (prior_target != MagickFalse))
            target=prior_target;
        }
    }
  if (ximage_info->screen)
    {
      int
        y;

      Window
        child;

      XWindowAttributes
        window_attributes;

      /*
        Obtain window image directly from screen.
      */
      status=XGetWindowAttributes(display,target,&window_attributes);
      if (status == False)
        {
          ThrowXWindowException(XServerError,"UnableToReadXWindowAttributes",
            image_info->filename);
          (void) XCloseDisplay(display);
          return((Image *) NULL);
        }
      (void) XTranslateCoordinates(display,target,root,0,0,&x,&y,&child);
      crop_info.x=(ssize_t) x;
      crop_info.y=(ssize_t) y;
      crop_info.width=(size_t) window_attributes.width;
      crop_info.height=(size_t) window_attributes.height;
      if (ximage_info->borders != 0)
        {
          /*
            Include border in image.
          */
          crop_info.x-=window_attributes.border_width;
          crop_info.y-=window_attributes.border_width;
          crop_info.width+=window_attributes.border_width << 1;
          crop_info.height+=window_attributes.border_width << 1;
        }
      target=root;
    }
  /*
    If WM_COLORMAP_WINDOWS property is set or multiple colormaps, descend.
  */
  number_windows=0;
  status=XGetWMColormapWindows(display,target,&children,&number_windows);
  if ((status == True) && (number_windows > 0))
    {
      ximage_info->descend=MagickTrue;
      (void) XFree ((char *) children);
    }
  colormaps=XListInstalledColormaps(display,target,&number_colormaps);
  if (number_colormaps > 0)
    {
      if (number_colormaps > 1)
        ximage_info->descend=MagickTrue;
      (void) XFree((char *) colormaps);
    }
  /*
    Alert the user not to alter the screen.
  */
  if (ximage_info->silent == MagickFalse)
    (void) XBell(display,0);
  /*
    Get image by window id.
  */
  (void) XGrabServer(display);
  image=XGetWindowImage(display,target,ximage_info->borders,
    ximage_info->descend ? 1U : 0U,exception);
  (void) XUngrabServer(display);
  if (image == (Image *) NULL)
    ThrowXWindowException(XServerError,"UnableToReadXWindowImage",
      image_info->filename)
  else
    {
      (void) CopyMagickString(image->filename,image_info->filename,
        MagickPathExtent);
      if ((crop_info.width != 0) && (crop_info.height != 0))
        {
          Image
            *crop_image;

          /*
            Crop image as defined by the cropping rectangle.
          */
          crop_image=CropImage(image,&crop_info,exception);
          if (crop_image != (Image *) NULL)
            {
              image=DestroyImage(image);
              image=crop_image;
            }
        }
      status=XGetWMName(display,target,&window_name);
      if (status == True)
        {
          if (*image_info->filename == '\0')
            (void) CopyMagickString(image->filename,(char *) window_name.value,
              (size_t) window_name.nitems+1);
          (void) XFree((void *) window_name.value);
        }
    }
  if (ximage_info->silent == MagickFalse)
    {
      /*
        Alert the user we're done.
      */
      (void) XBell(display,0);
      (void) XBell(display,0);
    }
  (void) XCloseDisplay(display);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X I n i t i a l i z e W i n d o w s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XInitializeWindows() initializes the XWindows structure.
%
%  The format of the XInitializeWindows method is:
%
%      XWindows *XInitializeWindows(Display *display,
%        XResourceInfo *resource_info)
%
%  A description of each parameter follows:
%
%    o windows: XInitializeWindows returns a pointer to a XWindows structure.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
*/
MagickPrivate XWindows *XInitializeWindows(Display *display,
  XResourceInfo *resource_info)
{
  Window
    root_window;

  XWindows
    *windows;

  /*
    Allocate windows structure.
  */
  windows=(XWindows *) AcquireMagickMemory(sizeof(*windows));
  if (windows == (XWindows *) NULL)
    {
      ThrowXWindowFatalException(XServerFatalError,"MemoryAllocationFailed",
        "...");
      return((XWindows *) NULL);
    }
  (void) memset(windows,0,sizeof(*windows));
  windows->pixel_info=(XPixelInfo *) AcquireMagickMemory(
    sizeof(*windows->pixel_info));
  windows->icon_pixel=(XPixelInfo *) AcquireMagickMemory(
    sizeof(*windows->icon_pixel));
  windows->icon_resources=(XResourceInfo *) AcquireMagickMemory(
    sizeof(*windows->icon_resources));
  if ((windows->pixel_info == (XPixelInfo *) NULL) ||
      (windows->icon_pixel == (XPixelInfo *) NULL) ||
      (windows->icon_resources == (XResourceInfo *) NULL))
    {
      ThrowXWindowFatalException(XServerFatalError,"MemoryAllocationFailed",
        "...");
      return((XWindows *) NULL);
    }
  /*
    Initialize windows structure.
  */
  windows->display=display;
  windows->wm_protocols=XInternAtom(display,"WM_PROTOCOLS",MagickFalse);
  windows->wm_delete_window=XInternAtom(display,"WM_DELETE_WINDOW",MagickFalse);
  windows->wm_take_focus=XInternAtom(display,"WM_TAKE_FOCUS",MagickFalse);
  windows->im_protocols=XInternAtom(display,"IM_PROTOCOLS",MagickFalse);
  windows->im_remote_command=
    XInternAtom(display,"IM_REMOTE_COMMAND",MagickFalse);
  windows->im_update_widget=XInternAtom(display,"IM_UPDATE_WIDGET",MagickFalse);
  windows->im_update_colormap=
    XInternAtom(display,"IM_UPDATE_COLORMAP",MagickFalse);
  windows->im_former_image=XInternAtom(display,"IM_FORMER_IMAGE",MagickFalse);
  windows->im_next_image=XInternAtom(display,"IM_NEXT_IMAGE",MagickFalse);
  windows->im_retain_colors=XInternAtom(display,"IM_RETAIN_COLORS",MagickFalse);
  windows->im_exit=XInternAtom(display,"IM_EXIT",MagickFalse);
  windows->dnd_protocols=XInternAtom(display,"DndProtocol",MagickFalse);
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  (void) XSynchronize(display,IsWindows95());
#endif
  if (IsEventLogging())
    {
      (void) XSynchronize(display,MagickTrue);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"Version: %s",
        GetMagickVersion((size_t *) NULL));
      (void) LogMagickEvent(X11Event,GetMagickModule(),"Protocols:");
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "  Window Manager: 0x%lx",windows->wm_protocols);
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "    delete window: 0x%lx",windows->wm_delete_window);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"    take focus: 0x%lx",
        windows->wm_take_focus);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"  ImageMagick: 0x%lx",
        windows->im_protocols);
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "    remote command: 0x%lx",windows->im_remote_command);
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "    update widget: 0x%lx",windows->im_update_widget);
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "    update colormap: 0x%lx",windows->im_update_colormap);
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "    former image: 0x%lx",windows->im_former_image);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"    next image: 0x%lx",
        windows->im_next_image);
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "    retain colors: 0x%lx",windows->im_retain_colors);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"    exit: 0x%lx",
        windows->im_exit);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"  Drag and Drop: 0x%lx",
        windows->dnd_protocols);
    }
  /*
    Allocate standard colormap.
  */
  windows->map_info=XAllocStandardColormap();
  windows->icon_map=XAllocStandardColormap();
  if ((windows->map_info == (XStandardColormap *) NULL) ||
      (windows->icon_map == (XStandardColormap *) NULL))
    ThrowXWindowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      "...");
  windows->map_info->colormap=(Colormap) NULL;
  windows->icon_map->colormap=(Colormap) NULL;
  windows->pixel_info->pixels=(unsigned long *) NULL;
  windows->pixel_info->annotate_context=(GC) NULL;
  windows->pixel_info->highlight_context=(GC) NULL;
  windows->pixel_info->widget_context=(GC) NULL;
  windows->font_info=(XFontStruct *) NULL;
  windows->icon_pixel->annotate_context=(GC) NULL;
  windows->icon_pixel->pixels=(unsigned long *) NULL;
  /*
    Allocate visual.
  */
  *windows->icon_resources=(*resource_info);
  windows->icon_resources->visual_type=(char *) "default";
  windows->icon_resources->colormap=SharedColormap;
  windows->visual_info=
    XBestVisualInfo(display,windows->map_info,resource_info);
  windows->icon_visual=
    XBestVisualInfo(display,windows->icon_map,windows->icon_resources);
  if ((windows->visual_info == (XVisualInfo *) NULL) ||
      (windows->icon_visual == (XVisualInfo *) NULL))
    ThrowXWindowFatalException(XServerFatalError,"UnableToGetVisual",
      resource_info->visual_type);
  if (IsEventLogging())
    {
      (void) LogMagickEvent(X11Event,GetMagickModule(),"Visual:");
      (void) LogMagickEvent(X11Event,GetMagickModule(),"  visual id: 0x%lx",
        windows->visual_info->visualid);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"  class: %s",
        XVisualClassName(windows->visual_info->klass));
      (void) LogMagickEvent(X11Event,GetMagickModule(),"  depth: %d planes",
        windows->visual_info->depth);
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "  size of colormap: %d entries",windows->visual_info->colormap_size);
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "  red, green, blue masks: 0x%lx 0x%lx 0x%lx",
        windows->visual_info->red_mask,windows->visual_info->green_mask,
        windows->visual_info->blue_mask);
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "  significant bits in color: %d bits",
        windows->visual_info->bits_per_rgb);
    }
  /*
    Allocate class and manager hints.
  */
  windows->class_hints=XAllocClassHint();
  windows->manager_hints=XAllocWMHints();
  if ((windows->class_hints == (XClassHint *) NULL) ||
      (windows->manager_hints == (XWMHints *) NULL))
    ThrowXWindowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      "...");
  /*
    Determine group leader if we have one.
  */
  root_window=XRootWindow(display,windows->visual_info->screen);
  windows->group_leader.id=(Window) NULL;
  if (resource_info->window_group != (char *) NULL)
    {
      if (isdigit((int) ((unsigned char) *resource_info->window_group)) != 0)
        windows->group_leader.id=XWindowByID(display,root_window,(Window)
          strtol((char *) resource_info->window_group,(char **) NULL,0));
      if (windows->group_leader.id == (Window) NULL)
        windows->group_leader.id=
          XWindowByName(display,root_window,resource_info->window_group);
    }
  return(windows);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e C u r s o r                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMakeCursor() creates a crosshairs X11 cursor.
%
%  The format of the XMakeCursor method is:
%
%      Cursor XMakeCursor(Display *display,Window window,Colormap colormap,
%        char *background_color,char *foreground_color)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o window: Specifies the ID of the window for which the cursor is
%      assigned.
%
%    o colormap: Specifies the ID of the colormap from which the background
%      and foreground color will be retrieved.
%
%    o background_color: Specifies the color to use for the cursor background.
%
%    o foreground_color: Specifies the color to use for the cursor foreground.
%
*/
MagickPrivate Cursor XMakeCursor(Display *display,Window window,
  Colormap colormap,char *background_color,char *foreground_color)
{
#define scope_height 17
#define scope_x_hot 8
#define scope_y_hot 8
#define scope_width 17

  static const unsigned char
    scope_bits[] =
    {
      0x80, 0x03, 0x00, 0x80, 0x02, 0x00, 0x80, 0x02, 0x00, 0x80, 0x02,
      0x00, 0x80, 0x02, 0x00, 0x80, 0x02, 0x00, 0x80, 0x02, 0x00, 0x7f,
      0xfc, 0x01, 0x01, 0x00, 0x01, 0x7f, 0xfc, 0x01, 0x80, 0x02, 0x00,
      0x80, 0x02, 0x00, 0x80, 0x02, 0x00, 0x80, 0x02, 0x00, 0x80, 0x02,
      0x00, 0x80, 0x02, 0x00, 0x80, 0x03, 0x00
    },
    scope_mask_bits[] =
    {
      0xc0, 0x07, 0x00, 0xc0, 0x07, 0x00, 0xc0, 0x06, 0x00, 0xc0, 0x06,
      0x00, 0xc0, 0x06, 0x00, 0xc0, 0x06, 0x00, 0xff, 0xfe, 0x01, 0x7f,
      0xfc, 0x01, 0x03, 0x80, 0x01, 0x7f, 0xfc, 0x01, 0xff, 0xfe, 0x01,
      0xc0, 0x06, 0x00, 0xc0, 0x06, 0x00, 0xc0, 0x06, 0x00, 0xc0, 0x06,
      0x00, 0xc0, 0x07, 0x00, 0xc0, 0x07, 0x00
    };

  Cursor
    cursor;

  Pixmap
    mask,
    source;

  XColor
    background,
    foreground;

  assert(display != (Display *) NULL);
  assert(window != (Window) NULL);
  assert(colormap != (Colormap) NULL);
  assert(background_color != (char *) NULL);
  assert(foreground_color != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",background_color);
  source=XCreateBitmapFromData(display,window,(char *) scope_bits,scope_width,
    scope_height);
  mask=XCreateBitmapFromData(display,window,(char *) scope_mask_bits,
    scope_width,scope_height);
  if ((source == (Pixmap) NULL) || (mask == (Pixmap) NULL))
    {
      ThrowXWindowException(XServerError,"UnableToCreatePixmap","...");
      return((Cursor) NULL);
    }
  (void) XParseColor(display,colormap,background_color,&background);
  (void) XParseColor(display,colormap,foreground_color,&foreground);
  cursor=XCreatePixmapCursor(display,source,mask,&foreground,&background,
    scope_x_hot,scope_y_hot);
  (void) XFreePixmap(display,source);
  (void) XFreePixmap(display,mask);
  return(cursor);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMakeImage() creates an X11 image.  If the image size differs from the X11
%  image size, the image is first resized.
%
%  The format of the XMakeImage method is:
%
%      MagickBooleanType XMakeImage(Display *display,
%        const XResourceInfo *resource_info,XWindowInfo *window,Image *image,
%        unsigned int width,unsigned int height,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o window: Specifies a pointer to a XWindowInfo structure.
%
%    o image: the image.
%
%    o width: Specifies the width in pixels of the rectangular area to
%      display.
%
%    o height: Specifies the height in pixels of the rectangular area to
%      display.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate MagickBooleanType XMakeImage(Display *display,
  const XResourceInfo *resource_info,XWindowInfo *window,Image *image,
  unsigned int width,unsigned int height,ExceptionInfo *exception)
{
#define CheckOverflowException(length,width,height) \
  (((height) != 0) && ((length)/((size_t) height) != ((size_t) width)))

  int
    depth,
    format;

  size_t
    length;

  XImage
    *matte_image,
    *ximage;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(resource_info != (XResourceInfo *) NULL);
  assert(window != (XWindowInfo *) NULL);
  assert(width != 0);
  assert(height != 0);
  if ((window->width == 0) || (window->height == 0))
    return(MagickFalse);
  /*
    Apply user transforms to the image.
  */
  (void) XCheckDefineCursor(display,window->id,window->busy_cursor);
  (void) XFlush(display);
  depth=(int) window->depth;
  if (window->destroy)
    window->image=DestroyImage(window->image);
  window->image=image;
  window->destroy=MagickFalse;
  if (window->image != (Image *) NULL)
    {
      if (window->crop_geometry != (char *) NULL)
        {
          Image
            *crop_image;

          RectangleInfo
            crop_info;

          /*
            Crop image.
          */
          window->image->page.x=0;
          window->image->page.y=0;
          (void) ParsePageGeometry(window->image,window->crop_geometry,
            &crop_info,exception);
          crop_image=CropImage(window->image,&crop_info,exception);
          if (crop_image != (Image *) NULL)
            {
              if (window->image != image)
                window->image=DestroyImage(window->image);
              window->image=crop_image;
              window->destroy=MagickTrue;
            }
        }
      if ((width != (unsigned int) window->image->columns) ||
          (height != (unsigned int) window->image->rows))
        {
          Image
            *resize_image;

          /*
            Resize image.
          */
          resize_image=NewImageList();
          if ((window->pixel_info->colors == 0) &&
              (window->image->rows > (unsigned long) XDisplayHeight(display,window->screen)) &&
              (window->image->columns > (unsigned long) XDisplayWidth(display,window->screen)))
              resize_image=ResizeImage(window->image,width,height,
                image->filter,exception);
          else
            {
              if (window->image->storage_class == PseudoClass)
                resize_image=SampleImage(window->image,width,height,
                  exception);
              else
                resize_image=ThumbnailImage(window->image,width,height,
                  exception);
            }
          if (resize_image != (Image *) NULL)
            {
              if (window->image != image)
                window->image=DestroyImage(window->image);
              window->image=resize_image;
              window->destroy=MagickTrue;
            }
        }
      width=(unsigned int) window->image->columns;
      assert((size_t) width == window->image->columns);
      height=(unsigned int) window->image->rows;
      assert((size_t) height == window->image->rows);
    }
  /*
    Create X image.
  */
  ximage=(XImage *) NULL;
  format=(depth == 1) ? XYBitmap : ZPixmap;
#if defined(MAGICKCORE_HAVE_SHARED_MEMORY)
  if (window->shared_memory != MagickFalse)
    {
      XShmSegmentInfo
        *segment_info;

      segment_info=(XShmSegmentInfo *) window->segment_info;
      segment_info[1].shmid=(-1);
      segment_info[1].shmaddr=(char *) NULL;
      ximage=XShmCreateImage(display,window->visual,(unsigned int) depth,format,
        (char *) NULL,&segment_info[1],width,height);
      length=0;
      if (ximage == (XImage *) NULL)
        window->shared_memory=MagickFalse;
      else
        {
          length=(size_t) ximage->bytes_per_line*ximage->height;
          if (CheckOverflowException(length,ximage->bytes_per_line,ximage->height))
            window->shared_memory=MagickFalse;
        }
      if (window->shared_memory != MagickFalse)
        segment_info[1].shmid=shmget(IPC_PRIVATE,length,IPC_CREAT | 0777);
      if (window->shared_memory != MagickFalse)
        segment_info[1].shmaddr=(char *) shmat(segment_info[1].shmid,0,0);
      if (segment_info[1].shmid < 0)
        window->shared_memory=MagickFalse;
      if (window->shared_memory != MagickFalse)
        (void) shmctl(segment_info[1].shmid,IPC_RMID,0);
      else
        {
          if (ximage != (XImage *) NULL)
            XDestroyImage(ximage);
          ximage=(XImage *) NULL;
          if (segment_info[1].shmaddr)
            {
              (void) shmdt(segment_info[1].shmaddr);
              segment_info[1].shmaddr=(char *) NULL;
            }
          if (segment_info[1].shmid >= 0)
            {
              (void) shmctl(segment_info[1].shmid,IPC_RMID,0);
              segment_info[1].shmid=(-1);
            }
        }
    }
#endif
  /*
    Allocate X image pixel data.
  */
#if defined(MAGICKCORE_HAVE_SHARED_MEMORY)
  if (window->shared_memory)
    {
      Status
        status;

      XShmSegmentInfo
        *segment_info;

      (void) XSync(display,MagickFalse);
      xerror_alert=MagickFalse;
      segment_info=(XShmSegmentInfo *) window->segment_info;
      ximage->data=segment_info[1].shmaddr;
      segment_info[1].readOnly=MagickFalse;
      status=XShmAttach(display,&segment_info[1]);
      if (status != False)
        (void) XSync(display,MagickFalse);
      if ((status == False) || (xerror_alert != MagickFalse))
        {
          window->shared_memory=MagickFalse;
          if (status != False)
            XShmDetach(display,&segment_info[1]);
          ximage->data=NULL;
          XDestroyImage(ximage);
          ximage=(XImage *) NULL;
          if (segment_info[1].shmid >= 0)
            {
              if (segment_info[1].shmaddr != NULL)
                (void) shmdt(segment_info[1].shmaddr);
              (void) shmctl(segment_info[1].shmid,IPC_RMID,0);
              segment_info[1].shmid=(-1);
              segment_info[1].shmaddr=(char *) NULL;
            }
        }
    }
#endif
  if (window->shared_memory == MagickFalse)
    ximage=XCreateImage(display,window->visual,(unsigned int) depth,format,0,
      (char *) NULL,width,height,XBitmapPad(display),0);
  if (ximage == (XImage *) NULL)
    {
      /*
        Unable to create X image.
      */
      (void) XCheckDefineCursor(display,window->id,window->cursor);
      return(MagickFalse);
    }
  length=(size_t) ximage->bytes_per_line*ximage->height;
  if (IsEventLogging())
    {
      (void) LogMagickEvent(X11Event,GetMagickModule(),"XImage:");
      (void) LogMagickEvent(X11Event,GetMagickModule(),"  width, height: %dx%d",
        ximage->width,ximage->height);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"  format: %d",
        ximage->format);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"  byte order: %d",
        ximage->byte_order);
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "  bitmap unit, bit order, pad: %d %d %d",ximage->bitmap_unit,
        ximage->bitmap_bit_order,ximage->bitmap_pad);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"  depth: %d",
        ximage->depth);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"  bytes per line: %d",
        ximage->bytes_per_line);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"  bits per pixel: %d",
        ximage->bits_per_pixel);
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "  red, green, blue masks: 0x%lx 0x%lx 0x%lx",ximage->red_mask,
        ximage->green_mask,ximage->blue_mask);
    }
  if (window->shared_memory == MagickFalse)
    {
      if (ximage->format == XYBitmap)
        {
          ximage->data=(char *) AcquireQuantumMemory((size_t)
            ximage->bytes_per_line,(size_t) ximage->depth*ximage->height);
          if (ximage->data != (char *) NULL)
            (void) memset(ximage->data,0,(size_t)
              ximage->bytes_per_line*ximage->depth*ximage->height);
        }
      else
        {
          ximage->data=(char *) AcquireQuantumMemory((size_t)
            ximage->bytes_per_line,(size_t) ximage->height);
          if (ximage->data != (char *) NULL)
            (void) memset(ximage->data,0,(size_t)
              ximage->bytes_per_line*ximage->height);
        }
    }
  if (ximage->data == (char *) NULL)
    {
      /*
        Unable to allocate pixel data.
      */
      XDestroyImage(ximage);
      ximage=(XImage *) NULL;
      (void) XCheckDefineCursor(display,window->id,window->cursor);
      return(MagickFalse);
    }
  if (window->ximage != (XImage *) NULL)
    {
      /*
        Destroy previous X image.
      */
      length=(size_t) window->ximage->bytes_per_line*window->ximage->height;
#if defined(MAGICKCORE_HAVE_SHARED_MEMORY)
      if (window->segment_info != (XShmSegmentInfo *) NULL)
        {
          XShmSegmentInfo
            *segment_info;

          segment_info=(XShmSegmentInfo *) window->segment_info;
          if (segment_info[0].shmid >= 0)
            {
              (void) XSync(display,MagickFalse);
              (void) XShmDetach(display,&segment_info[0]);
              (void) XSync(display,MagickFalse);
              if (segment_info[0].shmaddr != (char *) NULL)
                (void) shmdt(segment_info[0].shmaddr);
              (void) shmctl(segment_info[0].shmid,IPC_RMID,0);
              segment_info[0].shmid=(-1);
              segment_info[0].shmaddr=(char *) NULL;
              window->ximage->data=(char *) NULL;
          }
        }
#endif
      if (window->ximage->data != (char *) NULL)
        free(window->ximage->data);
      window->ximage->data=(char *) NULL;
      XDestroyImage(window->ximage);
      window->ximage=(XImage *) NULL;
    }
#if defined(MAGICKCORE_HAVE_SHARED_MEMORY)
  if (window->segment_info != (XShmSegmentInfo *) NULL)
    {
      XShmSegmentInfo
        *segment_info;

      segment_info=(XShmSegmentInfo *) window->segment_info;
      segment_info[0]=segment_info[1];
    }
#endif
  window->ximage=ximage;
  matte_image=(XImage *) NULL;
  if ((window->shape != MagickFalse) && (window->image != (Image *) NULL))
    if ((window->image->alpha_trait != UndefinedPixelTrait) &&
        ((int) width <= XDisplayWidth(display,window->screen)) &&
        ((int) height <= XDisplayHeight(display,window->screen)))
      {
        /*
          Create matte image.
        */
        matte_image=XCreateImage(display,window->visual,1,XYBitmap,0,
          (char *) NULL,width,height,XBitmapPad(display),0);
        if (IsEventLogging())
          {
            (void) LogMagickEvent(X11Event,GetMagickModule(),"Matte Image:");
            (void) LogMagickEvent(X11Event,GetMagickModule(),
              "  width, height: %dx%d",matte_image->width,matte_image->height);
          }
        if (matte_image != (XImage *) NULL)
          {
            /*
              Allocate matte image pixel data.
            */
            matte_image->data=(char *) malloc((size_t)
              matte_image->bytes_per_line*matte_image->depth*
              matte_image->height);
            if (matte_image->data == (char *) NULL)
              {
                XDestroyImage(matte_image);
                matte_image=(XImage *) NULL;
              }
          }
      }
  if (window->matte_image != (XImage *) NULL)
    {
      /*
        Free matte image.
      */
      if (window->matte_image->data != (char *) NULL)
        free(window->matte_image->data);
      window->matte_image->data=(char *) NULL;
      XDestroyImage(window->matte_image);
      window->matte_image=(XImage *) NULL;
    }
  window->matte_image=matte_image;
  if (window->matte_pixmap != (Pixmap) NULL)
    {
      (void) XFreePixmap(display,window->matte_pixmap);
      window->matte_pixmap=(Pixmap) NULL;
#if defined(MAGICKCORE_HAVE_SHAPE)
      if (window->shape != MagickFalse)
        XShapeCombineMask(display,window->id,ShapeBounding,0,0,None,ShapeSet);
#endif
    }
  window->stasis=MagickFalse;
  /*
    Convert pixels to X image data.
  */
  if (window->image != (Image *) NULL)
    {
      if ((ximage->byte_order == LSBFirst) || ((ximage->format == XYBitmap) &&
          (ximage->bitmap_bit_order == LSBFirst)))
        XMakeImageLSBFirst(resource_info,window,window->image,ximage,
          matte_image,exception);
      else
        XMakeImageMSBFirst(resource_info,window,window->image,ximage,
          matte_image,exception);
    }
  if (window->matte_image != (XImage *) NULL)
    {
      /*
        Create matte pixmap.
      */
      window->matte_pixmap=XCreatePixmap(display,window->id,width,height,1);
      if (window->matte_pixmap != (Pixmap) NULL)
        {
          GC
            graphics_context;

          XGCValues
            context_values;

          /*
            Copy matte image to matte pixmap.
          */
          context_values.background=0;
          context_values.foreground=1;
          graphics_context=XCreateGC(display,window->matte_pixmap,
            (size_t) (GCBackground | GCForeground),&context_values);
          (void) XPutImage(display,window->matte_pixmap,graphics_context,
            window->matte_image,0,0,0,0,width,height);
          (void) XFreeGC(display,graphics_context);
#if defined(MAGICKCORE_HAVE_SHAPE)
          if (window->shape != MagickFalse)
            XShapeCombineMask(display,window->id,ShapeBounding,0,0,
              window->matte_pixmap,ShapeSet);
#endif
        }
      }
  (void) XMakePixmap(display,resource_info,window);
  /*
    Restore cursor.
  */
  (void) XCheckDefineCursor(display,window->id,window->cursor);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X M a k e I m a g e L S B F i r s t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMakeImageLSBFirst() initializes the pixel data of an X11 Image. The X image
%  pixels are copied in least-significant bit and byte first order.  The
%  server's scanline pad is respected.  Rather than using one or two general
%  cases, many special cases are found here to help speed up the image
%  conversion.
%
%  The format of the XMakeImageLSBFirst method is:
%
%      void XMakeImageLSBFirst(Display *display,XWindows *windows,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o window: Specifies a pointer to a XWindowInfo structure.
%
%    o image: the image.
%
%    o ximage: Specifies a pointer to a XImage structure;  returned from
%      XCreateImage.
%
%    o matte_image: Specifies a pointer to a XImage structure;  returned from
%      XCreateImage.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static void XMakeImageLSBFirst(const XResourceInfo *resource_info,
  const XWindowInfo *window,Image *image,XImage *ximage,XImage *matte_image,
  ExceptionInfo *exception)
{
  CacheView
    *canvas_view;

  Image
    *canvas;

  int
    y;

  register const Quantum
    *p;

  register int
    x;

  register unsigned char
    *q;

  unsigned char
    bit,
    byte;

  unsigned int
    scanline_pad;

  unsigned long
    pixel,
    *pixels;

  XStandardColormap
    *map_info;

  assert(resource_info != (XResourceInfo *) NULL);
  assert(window != (XWindowInfo *) NULL);
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  canvas=image;
  if ((window->immutable == MagickFalse) &&
      (image->storage_class == DirectClass) &&
      (image->alpha_trait != UndefinedPixelTrait))
    {
      char
        size[MagickPathExtent];

      Image
        *pattern;

      ImageInfo
        *image_info;

      image_info=AcquireImageInfo();
      (void) CopyMagickString(image_info->filename,
        resource_info->image_info->texture != (char *) NULL ?
        resource_info->image_info->texture : "pattern:checkerboard",
        MagickPathExtent);
      (void) FormatLocaleString(size,MagickPathExtent,"%.20gx%.20g",(double)
        image->columns,(double) image->rows);
      image_info->size=ConstantString(size);
      pattern=ReadImage(image_info,exception);
      image_info=DestroyImageInfo(image_info);
      if (pattern != (Image *) NULL)
        {
          canvas=CloneImage(image,0,0,MagickTrue,exception);
          if (canvas != (Image *) NULL)
            (void) CompositeImage(canvas,pattern,DstOverCompositeOp,MagickTrue,
              0,0,exception);
          pattern=DestroyImage(pattern);
        }
    }
  scanline_pad=(unsigned int) (ximage->bytes_per_line-((ximage->width*
    ximage->bits_per_pixel) >> 3));
  map_info=window->map_info;
  pixels=window->pixel_info->pixels;
  q=(unsigned char *) ximage->data;
  x=0;
  canvas_view=AcquireVirtualCacheView(canvas,exception);
  if (ximage->format == XYBitmap)
    {
      register unsigned short
        polarity;

      unsigned char
        background,
        foreground;

      /*
        Convert canvas to big-endian bitmap.
      */
      background=(unsigned char)
        (XPixelIntensity(&window->pixel_info->foreground_color) <
         XPixelIntensity(&window->pixel_info->background_color) ? 0x80 : 0x00);
      foreground=(unsigned char)
        (XPixelIntensity(&window->pixel_info->background_color) <
         XPixelIntensity(&window->pixel_info->foreground_color) ? 0x80 : 0x00);
      polarity=(unsigned short) ((GetPixelInfoIntensity(image,
        &canvas->colormap[0])) < (QuantumRange/2.0) ? 1 : 0);
      if (canvas->colors == 2)
        polarity=GetPixelInfoIntensity(image,&canvas->colormap[0]) <
          GetPixelInfoIntensity(image,&canvas->colormap[1]);
      for (y=0; y < (int) canvas->rows; y++)
      {
        p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,canvas->columns,1,
          exception);
        if (p == (const Quantum *) NULL)
          break;
        bit=0;
        byte=0;
        for (x=0; x < (int) canvas->columns; x++)
        {
          byte>>=1;
          if (GetPixelIndex(canvas,p) == (Quantum) polarity)
            byte|=foreground;
          else
            byte|=background;
          bit++;
          if (bit == 8)
            {
              *q++=byte;
              bit=0;
              byte=0;
            }
          p+=GetPixelChannels(canvas);
        }
        if (bit != 0)
          *q=byte >> (8-bit);
        q+=scanline_pad;
      }
    }
  else
    if (window->pixel_info->colors != 0)
      switch (ximage->bits_per_pixel)
      {
        case 2:
        {
          register unsigned int
            nibble;

          /*
            Convert to 2 bit color-mapped X canvas.
          */
          for (y=0; y < (int) canvas->rows; y++)
          {
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            nibble=0;
            for (x=0; x < (int) canvas->columns; x++)
            {
              pixel=pixels[(ssize_t) GetPixelIndex(canvas,p)] & 0x0f;
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) pixel;
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) (pixel << 2);
                  nibble++;
                  break;
                }
                case 2:
                {
                  *q|=(unsigned char) (pixel << 4);
                  nibble++;
                  break;
                }
                case 3:
                {
                  *q|=(unsigned char) (pixel << 6);
                  q++;
                  nibble=0;
                  break;
                }
              }
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
        case 4:
        {
          register unsigned int
            nibble;

          /*
            Convert to 4 bit color-mapped X canvas.
          */
          for (y=0; y < (int) canvas->rows; y++)
          {
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            nibble=0;
            for (x=0; x < (int) canvas->columns; x++)
            {
              pixel=pixels[(ssize_t) GetPixelIndex(canvas,p)] & 0xf;
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) pixel;
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) (pixel << 4);
                  q++;
                  nibble=0;
                  break;
                }
              }
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
        case 6:
        case 8:
        {
          /*
            Convert to 8 bit color-mapped X canvas.
          */
          if (resource_info->color_recovery &&
              resource_info->quantize_info->dither_method != NoDitherMethod)
            {
              XDitherImage(canvas,ximage,exception);
              break;
            }
          for (y=0; y < (int) canvas->rows; y++)
          {
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (int) canvas->columns; x++)
            {
              pixel=pixels[(ssize_t) GetPixelIndex(canvas,p)];
              *q++=(unsigned char) pixel;
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
        default:
        {
          register int
            k;

          register unsigned int
            bytes_per_pixel;

          /*
            Convert to multi-byte color-mapped X canvas.
          */
          bytes_per_pixel=(unsigned int) (ximage->bits_per_pixel >> 3);
          for (y=0; y < (int) canvas->rows; y++)
          {
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (int) canvas->columns; x++)
            {
              pixel=pixels[(ssize_t) GetPixelIndex(canvas,p)];
              for (k=0; k < (int) bytes_per_pixel; k++)
              {
                *q++=(unsigned char) (pixel & 0xff);
                pixel>>=8;
              }
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
      }
    else
      switch (ximage->bits_per_pixel)
      {
        case 2:
        {
          register unsigned int
            nibble;

          /*
            Convert to contiguous 2 bit continuous-tone X canvas.
          */
          for (y=0; y < (int) canvas->rows; y++)
          {
            nibble=0;
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (int) canvas->columns; x++)
            {
              pixel=XGammaPixel(canvas,map_info,p);
              pixel&=0xf;
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) pixel;
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) (pixel << 2);
                  nibble++;
                  break;
                }
                case 2:
                {
                  *q|=(unsigned char) (pixel << 4);
                  nibble++;
                  break;
                }
                case 3:
                {
                  *q|=(unsigned char) (pixel << 6);
                  q++;
                  nibble=0;
                  break;
                }
              }
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
        case 4:
        {
          register unsigned int
            nibble;

          /*
            Convert to contiguous 4 bit continuous-tone X canvas.
          */
          for (y=0; y < (int) canvas->rows; y++)
          {
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            nibble=0;
            for (x=0; x < (int) canvas->columns; x++)
            {
              pixel=XGammaPixel(canvas,map_info,p);
              pixel&=0xf;
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) pixel;
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) (pixel << 4);
                  q++;
                  nibble=0;
                  break;
                }
              }
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
        case 6:
        case 8:
        {
          /*
            Convert to contiguous 8 bit continuous-tone X canvas.
          */
          if (resource_info->color_recovery &&
              resource_info->quantize_info->dither_method != NoDitherMethod)
            {
              XDitherImage(canvas,ximage,exception);
              break;
            }
          for (y=0; y < (int) canvas->rows; y++)
          {
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (int) canvas->columns; x++)
            {
              pixel=XGammaPixel(canvas,map_info,p);
              *q++=(unsigned char) pixel;
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
        default:
        {
          if ((ximage->bits_per_pixel == 32) && (map_info->red_max == 255) &&
              (map_info->green_max == 255) && (map_info->blue_max == 255) &&
              (map_info->red_mult == 65536L) && (map_info->green_mult == 256) &&
              (map_info->blue_mult == 1))
            {
              /*
                Convert to 32 bit continuous-tone X canvas.
              */
              for (y=0; y < (int) canvas->rows; y++)
              {
                p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
                  canvas->columns,1,exception);
                if (p == (const Quantum *) NULL)
                  break;
                if ((red_gamma != 1.0) || (green_gamma != 1.0) ||
                    (blue_gamma != 1.0))
                  {
                    /*
                      Gamma correct canvas.
                    */
                    for (x=(int) canvas->columns-1; x >= 0; x--)
                    {
                      *q++=ScaleQuantumToChar(XBlueGamma(
                        GetPixelBlue(canvas,p)));
                      *q++=ScaleQuantumToChar(XGreenGamma(
                        GetPixelGreen(canvas,p)));
                      *q++=ScaleQuantumToChar(XRedGamma(
                        GetPixelRed(canvas,p)));
                      *q++=0;
                      p+=GetPixelChannels(canvas);
                    }
                    continue;
                  }
                for (x=(int) canvas->columns-1; x >= 0; x--)
                {
                  *q++=ScaleQuantumToChar((Quantum) GetPixelBlue(canvas,p));
                  *q++=ScaleQuantumToChar((Quantum) GetPixelGreen(canvas,p));
                  *q++=ScaleQuantumToChar((Quantum) GetPixelRed(canvas,p));
                  *q++=0;
                  p+=GetPixelChannels(canvas);
                }
              }
            }
          else
            if ((ximage->bits_per_pixel == 32) && (map_info->red_max == 255) &&
                (map_info->green_max == 255) && (map_info->blue_max == 255) &&
                (map_info->red_mult == 1) && (map_info->green_mult == 256) &&
                (map_info->blue_mult == 65536L))
              {
                /*
                  Convert to 32 bit continuous-tone X canvas.
                */
                for (y=0; y < (int) canvas->rows; y++)
                {
                  p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
                    canvas->columns,1,exception);
                  if (p == (const Quantum *) NULL)
                    break;
                  if ((red_gamma != 1.0) || (green_gamma != 1.0) ||
                      (blue_gamma != 1.0))
                    {
                      /*
                        Gamma correct canvas.
                      */
                      for (x=(int) canvas->columns-1; x >= 0; x--)
                      {
                        *q++=ScaleQuantumToChar(XRedGamma(
                          GetPixelRed(canvas,p)));
                        *q++=ScaleQuantumToChar(XGreenGamma(
                          GetPixelGreen(canvas,p)));
                        *q++=ScaleQuantumToChar(XBlueGamma(
                          GetPixelBlue(canvas,p)));
                        *q++=0;
                        p+=GetPixelChannels(canvas);
                      }
                      continue;
                    }
                  for (x=(int) canvas->columns-1; x >= 0; x--)
                  {
                    *q++=ScaleQuantumToChar((Quantum) GetPixelRed(canvas,p));
                    *q++=ScaleQuantumToChar((Quantum) GetPixelGreen(canvas,p));
                    *q++=ScaleQuantumToChar((Quantum) GetPixelBlue(canvas,p));
                    *q++=0;
                    p+=GetPixelChannels(canvas);
                  }
                }
              }
            else
              {
                register int
                  k;

                register unsigned int
                  bytes_per_pixel;

                /*
                  Convert to multi-byte continuous-tone X canvas.
                */
                bytes_per_pixel=(unsigned int) (ximage->bits_per_pixel >> 3);
                for (y=0; y < (int) canvas->rows; y++)
                {
                  p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
                    canvas->columns,1,exception);
                  if (p == (const Quantum *) NULL)
                    break;
                  for (x=0; x < (int) canvas->columns; x++)
                  {
                    pixel=XGammaPixel(canvas,map_info,p);
                    for (k=0; k < (int) bytes_per_pixel; k++)
                    {
                      *q++=(unsigned char) (pixel & 0xff);
                      pixel>>=8;
                    }
                    p+=GetPixelChannels(canvas);
                  }
                  q+=scanline_pad;
                }
              }
          break;
        }
      }
  if (matte_image != (XImage *) NULL)
    {
      /*
        Initialize matte canvas.
      */
      scanline_pad=(unsigned int) (matte_image->bytes_per_line-
        ((matte_image->width*matte_image->bits_per_pixel) >> 3));
      q=(unsigned char *) matte_image->data;
      for (y=0; y < (int) canvas->rows; y++)
      {
        p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,canvas->columns,1,
          exception);
        if (p == (const Quantum *) NULL)
          break;
        bit=0;
        byte=0;
        for (x=(int) canvas->columns-1; x >= 0; x--)
        {
          byte>>=1;
          if (GetPixelAlpha(canvas,p) > (QuantumRange/2))
            byte|=0x80;
          bit++;
          if (bit == 8)
            {
              *q++=byte;
              bit=0;
              byte=0;
            }
          p+=GetPixelChannels(canvas);
        }
        if (bit != 0)
          *q=byte >> (8-bit);
        q+=scanline_pad;
      }
    }
  canvas_view=DestroyCacheView(canvas_view);
  if (canvas != image)
    canvas=DestroyImage(canvas);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X M a k e I m a g e M S B F i r s t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMakeImageMSBFirst() initializes the pixel data of an X11 Image.  The X
%  image pixels are copied in most-significant bit and byte first order.  The
%  server's scanline pad is also respected. Rather than using one or two
%  general cases, many special cases are found here to help speed up the image
%  conversion.
%
%  The format of the XMakeImageMSBFirst method is:
%
%      XMakeImageMSBFirst(resource_info,window,image,ximage,matte_image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o window: Specifies a pointer to a XWindowInfo structure.
%
%    o image: the image.
%
%    o ximage: Specifies a pointer to a XImage structure;  returned from
%      XCreateImage.
%
%    o matte_image: Specifies a pointer to a XImage structure;  returned from
%      XCreateImage.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static void XMakeImageMSBFirst(const XResourceInfo *resource_info,
  const XWindowInfo *window,Image *image,XImage *ximage,XImage *matte_image,
  ExceptionInfo *exception)
{
  CacheView
    *canvas_view;

  Image
    *canvas;

  int
    y;

  register int
    x;

  register const Quantum
    *p;

  register unsigned char
    *q;

  unsigned char
    bit,
    byte;

  unsigned int
    scanline_pad;

  unsigned long
    pixel,
    *pixels;

  XStandardColormap
    *map_info;

  assert(resource_info != (XResourceInfo *) NULL);
  assert(window != (XWindowInfo *) NULL);
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  canvas=image;
  if ((window->immutable != MagickFalse) &&
      (image->storage_class == DirectClass) &&
      (image->alpha_trait != UndefinedPixelTrait))
    {
      char
        size[MagickPathExtent];

      Image
        *pattern;

      ImageInfo
        *image_info;

      image_info=AcquireImageInfo();
      (void) CopyMagickString(image_info->filename,
        resource_info->image_info->texture != (char *) NULL ?
        resource_info->image_info->texture : "pattern:checkerboard",
        MagickPathExtent);
      (void) FormatLocaleString(size,MagickPathExtent,"%.20gx%.20g",(double)
        image->columns,(double) image->rows);
      image_info->size=ConstantString(size);
      pattern=ReadImage(image_info,exception);
      image_info=DestroyImageInfo(image_info);
      if (pattern != (Image *) NULL)
        {
          canvas=CloneImage(image,0,0,MagickTrue,exception);
          if (canvas != (Image *) NULL)
            (void) CompositeImage(canvas,pattern,DstOverCompositeOp,MagickFalse,
              0,0,exception);
          pattern=DestroyImage(pattern);
        }
    }
  scanline_pad=(unsigned int) (ximage->bytes_per_line-((ximage->width*
    ximage->bits_per_pixel) >> 3));
  map_info=window->map_info;
  pixels=window->pixel_info->pixels;
  q=(unsigned char *) ximage->data;
  x=0;
  canvas_view=AcquireVirtualCacheView(canvas,exception);
  if (ximage->format == XYBitmap)
    {
      register unsigned short
        polarity;

      unsigned char
        background,
        foreground;

      /*
        Convert canvas to big-endian bitmap.
      */
      background=(unsigned char)
        (XPixelIntensity(&window->pixel_info->foreground_color) <
         XPixelIntensity(&window->pixel_info->background_color) ?  0x01 : 0x00);
      foreground=(unsigned char)
        (XPixelIntensity(&window->pixel_info->background_color) <
         XPixelIntensity(&window->pixel_info->foreground_color) ?  0x01 : 0x00);
      polarity=(unsigned short) ((GetPixelInfoIntensity(image,
        &canvas->colormap[0])) < (QuantumRange/2.0) ? 1 : 0);
      if (canvas->colors == 2)
        polarity=GetPixelInfoIntensity(image,&canvas->colormap[0]) <
          GetPixelInfoIntensity(image,&canvas->colormap[1]);
      for (y=0; y < (int) canvas->rows; y++)
      {
        p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,canvas->columns,1,
          exception);
        if (p == (const Quantum *) NULL)
          break;
        bit=0;
        byte=0;
        for (x=(int) canvas->columns-1; x >= 0; x--)
        {
          byte<<=1;
          if (GetPixelIndex(canvas,p) == (Quantum) polarity)
            byte|=foreground;
          else
            byte|=background;
          bit++;
          if (bit == 8)
            {
              *q++=byte;
              bit=0;
              byte=0;
            }
          p+=GetPixelChannels(canvas);
        }
        if (bit != 0)
          *q=byte << (8-bit);
        q+=scanline_pad;
      }
    }
  else
    if (window->pixel_info->colors != 0)
      switch (ximage->bits_per_pixel)
      {
        case 2:
        {
          register unsigned int
            nibble;

          /*
            Convert to 2 bit color-mapped X canvas.
          */
          for (y=0; y < (int) canvas->rows; y++)
          {
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            nibble=0;
            for (x=0; x < (int) canvas->columns; x++)
            {
              pixel=pixels[(ssize_t)
                GetPixelIndex(canvas,p)] & 0xf;
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) (pixel << 6);
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) (pixel << 4);
                  nibble++;
                  break;
                }
                case 2:
                {
                  *q|=(unsigned char) (pixel << 2);
                  nibble++;
                  break;
                }
                case 3:
                {
                  *q|=(unsigned char) pixel;
                  q++;
                  nibble=0;
                  break;
                }
              }
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
        case 4:
        {
          register unsigned int
            nibble;

          /*
            Convert to 4 bit color-mapped X canvas.
          */
          for (y=0; y < (int) canvas->rows; y++)
          {
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            nibble=0;
            for (x=0; x < (int) canvas->columns; x++)
            {
              pixel=pixels[(ssize_t)
                GetPixelIndex(canvas,p)] & 0xf;
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) (pixel << 4);
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) pixel;
                  q++;
                  nibble=0;
                  break;
                }
              }
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
        case 6:
        case 8:
        {
          /*
            Convert to 8 bit color-mapped X canvas.
          */
          if ((resource_info->color_recovery != MagickFalse) &&
              (resource_info->quantize_info->dither_method != NoDitherMethod))
            {
              XDitherImage(canvas,ximage,exception);
              break;
            }
          for (y=0; y < (int) canvas->rows; y++)
          {
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (int) canvas->columns; x++)
            {
              pixel=pixels[(ssize_t) GetPixelIndex(canvas,p)];
              *q++=(unsigned char) pixel;
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
        default:
        {
          register int
            k;

          register unsigned int
            bytes_per_pixel;

          unsigned char
            channel[sizeof(size_t)];

          /*
            Convert to 8 bit color-mapped X canvas.
          */
          bytes_per_pixel=(unsigned int) (ximage->bits_per_pixel >> 3);
          for (y=0; y < (int) canvas->rows; y++)
          {
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (int) canvas->columns; x++)
            {
              pixel=pixels[(ssize_t)
                GetPixelIndex(canvas,p)];
              for (k=(int) bytes_per_pixel-1; k >= 0; k--)
              {
                channel[k]=(unsigned char) pixel;
                pixel>>=8;
              }
              for (k=0; k < (int) bytes_per_pixel; k++)
                *q++=channel[k];
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
      }
    else
      switch (ximage->bits_per_pixel)
      {
        case 2:
        {
          register unsigned int
            nibble;

          /*
            Convert to 4 bit continuous-tone X canvas.
          */
          for (y=0; y < (int) canvas->rows; y++)
          {
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            nibble=0;
            for (x=(int) canvas->columns-1; x >= 0; x--)
            {
              pixel=XGammaPixel(canvas,map_info,p);
              pixel&=0xf;
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) (pixel << 6);
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) (pixel << 4);
                  nibble++;
                  break;
                }
                case 2:
                {
                  *q|=(unsigned char) (pixel << 2);
                  nibble++;
                  break;
                }
                case 3:
                {
                  *q|=(unsigned char) pixel;
                  q++;
                  nibble=0;
                  break;
                }
              }
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
        case 4:
        {
          register unsigned int
            nibble;

          /*
            Convert to 4 bit continuous-tone X canvas.
          */
          for (y=0; y < (int) canvas->rows; y++)
          {
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            nibble=0;
            for (x=(int) canvas->columns-1; x >= 0; x--)
            {
              pixel=XGammaPixel(canvas,map_info,p);
              pixel&=0xf;
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) (pixel << 4);
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) pixel;
                  q++;
                  nibble=0;
                  break;
                }
              }
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
        case 6:
        case 8:
        {
          /*
            Convert to 8 bit continuous-tone X canvas.
          */
          if ((resource_info->color_recovery != MagickFalse) &&
              (resource_info->quantize_info->dither_method != NoDitherMethod))
            {
              XDitherImage(canvas,ximage,exception);
              break;
            }
          for (y=0; y < (int) canvas->rows; y++)
          {
            p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
              canvas->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=(int) canvas->columns-1; x >= 0; x--)
            {
              pixel=XGammaPixel(canvas,map_info,p);
              *q++=(unsigned char) pixel;
              p+=GetPixelChannels(canvas);
            }
            q+=scanline_pad;
          }
          break;
        }
        default:
        {
          if ((ximage->bits_per_pixel == 32) && (map_info->red_max == 255) &&
              (map_info->green_max == 255) && (map_info->blue_max == 255) &&
              (map_info->red_mult == 65536L) && (map_info->green_mult == 256) &&
              (map_info->blue_mult == 1))
            {
              /*
                Convert to 32 bit continuous-tone X canvas.
              */
              for (y=0; y < (int) canvas->rows; y++)
              {
                p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
                  canvas->columns,1,exception);
                if (p == (const Quantum *) NULL)
                  break;
                if ((red_gamma != 1.0) || (green_gamma != 1.0) ||
                    (blue_gamma != 1.0))
                  {
                    /*
                      Gamma correct canvas.
                    */
                    for (x=(int) canvas->columns-1; x >= 0; x--)
                    {
                      *q++=0;
                      *q++=ScaleQuantumToChar(XRedGamma(
                        GetPixelRed(canvas,p)));
                      *q++=ScaleQuantumToChar(XGreenGamma(
                        GetPixelGreen(canvas,p)));
                      *q++=ScaleQuantumToChar(XBlueGamma(
                        GetPixelBlue(canvas,p)));
                      p+=GetPixelChannels(canvas);
                    }
                    continue;
                  }
                for (x=(int) canvas->columns-1; x >= 0; x--)
                {
                  *q++=0;
                  *q++=ScaleQuantumToChar((Quantum) GetPixelRed(canvas,p));
                  *q++=ScaleQuantumToChar((Quantum) GetPixelGreen(canvas,p));
                  *q++=ScaleQuantumToChar((Quantum) GetPixelBlue(canvas,p));
                  p+=GetPixelChannels(canvas);
                }
              }
            }
          else
            if ((ximage->bits_per_pixel == 32) && (map_info->red_max == 255) &&
                (map_info->green_max == 255) && (map_info->blue_max == 255) &&
                (map_info->red_mult == 1) && (map_info->green_mult == 256) &&
                (map_info->blue_mult == 65536L))
              {
                /*
                  Convert to 32 bit continuous-tone X canvas.
                */
                for (y=0; y < (int) canvas->rows; y++)
                {
                  p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
                    canvas->columns,1,exception);
                  if (p == (const Quantum *) NULL)
                    break;
                  if ((red_gamma != 1.0) || (green_gamma != 1.0) ||
                      (blue_gamma != 1.0))
                    {
                      /*
                        Gamma correct canvas.
                      */
                      for (x=(int) canvas->columns-1; x >= 0; x--)
                      {
                        *q++=0;
                        *q++=ScaleQuantumToChar(XBlueGamma(
                          GetPixelBlue(canvas,p)));
                        *q++=ScaleQuantumToChar(XGreenGamma(
                          GetPixelGreen(canvas,p)));
                        *q++=ScaleQuantumToChar(XRedGamma(
                          GetPixelRed(canvas,p)));
                        p+=GetPixelChannels(canvas);
                      }
                      continue;
                    }
                  for (x=(int) canvas->columns-1; x >= 0; x--)
                  {
                    *q++=0;
                    *q++=ScaleQuantumToChar((Quantum) GetPixelBlue(canvas,p));
                    *q++=ScaleQuantumToChar((Quantum) GetPixelGreen(canvas,p));
                    *q++=ScaleQuantumToChar((Quantum) GetPixelRed(canvas,p));
                    p+=GetPixelChannels(canvas);
                  }
                }
              }
            else
              {
                register int
                  k;

                register unsigned int
                  bytes_per_pixel;

                unsigned char
                  channel[sizeof(size_t)];

                /*
                  Convert to multi-byte continuous-tone X canvas.
                */
                bytes_per_pixel=(unsigned int) (ximage->bits_per_pixel >> 3);
                for (y=0; y < (int) canvas->rows; y++)
                {
                  p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,
                    canvas->columns,1,exception);
                  if (p == (const Quantum *) NULL)
                    break;
                  for (x=(int) canvas->columns-1; x >= 0; x--)
                  {
                    pixel=XGammaPixel(canvas,map_info,p);
                    for (k=(int) bytes_per_pixel-1; k >= 0; k--)
                    {
                      channel[k]=(unsigned char) pixel;
                      pixel>>=8;
                    }
                    for (k=0; k < (int) bytes_per_pixel; k++)
                      *q++=channel[k];
                    p+=GetPixelChannels(canvas);
                  }
                  q+=scanline_pad;
                }
              }
          break;
        }
      }
  if (matte_image != (XImage *) NULL)
    {
      /*
        Initialize matte canvas.
      */
      scanline_pad=(unsigned int) (matte_image->bytes_per_line-
        ((matte_image->width*matte_image->bits_per_pixel) >> 3));
      q=(unsigned char *) matte_image->data;
      for (y=0; y < (int) canvas->rows; y++)
      {
        p=GetCacheViewVirtualPixels(canvas_view,0,(ssize_t) y,canvas->columns,1,
          exception);
        if (p == (const Quantum *) NULL)
          break;
        bit=0;
        byte=0;
        for (x=(int) canvas->columns-1; x >= 0; x--)
        {
          byte<<=1;
          if (GetPixelAlpha(canvas,p) > (QuantumRange/2))
            byte|=0x01;
          bit++;
          if (bit == 8)
            {
              *q++=byte;
              bit=0;
              byte=0;
            }
          p+=GetPixelChannels(canvas);
        }
        if (bit != 0)
          *q=byte << (8-bit);
        q+=scanline_pad;
      }
    }
  canvas_view=DestroyCacheView(canvas_view);
  if (canvas != image)
    canvas=DestroyImage(canvas);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e M a g n i f y I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMakeMagnifyImage() magnifies a region of an X image and displays it.
%
%  The format of the XMakeMagnifyImage method is:
%
%      void XMakeMagnifyImage(Display *display,XWindows *windows,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate void XMakeMagnifyImage(Display *display,XWindows *windows,
  ExceptionInfo *exception)
{
  char
    tuple[MagickPathExtent];

  int
    y;

  PixelInfo
    pixel;

  register int
    x;

  register ssize_t
    i;

  register unsigned char
    *p,
    *q;

  ssize_t
    n;

  static unsigned int
    previous_magnify = 0;

  static XWindowInfo
    magnify_window;

  unsigned int
    height,
    j,
    k,
    l,
    magnify,
    scanline_pad,
    width;

  XImage
    *ximage;

  /*
    Check boundary conditions.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(windows != (XWindows *) NULL);
  magnify=1;
  for (n=1; n < (ssize_t) windows->magnify.data; n++)
    magnify<<=1;
  while ((magnify*windows->image.ximage->width) < windows->magnify.width)
    magnify<<=1;
  while ((magnify*windows->image.ximage->height) < windows->magnify.height)
    magnify<<=1;
  while (magnify > windows->magnify.width)
    magnify>>=1;
  while (magnify > windows->magnify.height)
    magnify>>=1;
  if (magnify == 0)
    magnify=1;
  if (magnify != previous_magnify)
    {
      Status
        status;

      XTextProperty
        window_name;

      /*
        New magnify factor:  update magnify window name.
      */
      i=0;
      while ((1 << i) <= (int) magnify)
        i++;
      (void) FormatLocaleString(windows->magnify.name,MagickPathExtent,
        "Magnify %.20gX",(double) i);
      status=XStringListToTextProperty(&windows->magnify.name,1,&window_name);
      if (status != False)
        {
          XSetWMName(display,windows->magnify.id,&window_name);
          XSetWMIconName(display,windows->magnify.id,&window_name);
          (void) XFree((void *) window_name.value);
        }
    }
  previous_magnify=magnify;
  ximage=windows->image.ximage;
  width=(unsigned int) windows->magnify.ximage->width;
  height=(unsigned int) windows->magnify.ximage->height;
  if ((windows->magnify.x < 0) ||
      (windows->magnify.x >= windows->image.ximage->width))
    windows->magnify.x=windows->image.ximage->width >> 1;
  x=windows->magnify.x-((width/magnify) >> 1);
  if (x < 0)
    x=0;
  else
    if (x > (int) (ximage->width-(width/magnify)))
      x=ximage->width-width/magnify;
  if ((windows->magnify.y < 0) ||
      (windows->magnify.y >= windows->image.ximage->height))
    windows->magnify.y=windows->image.ximage->height >> 1;
  y=windows->magnify.y-((height/magnify) >> 1);
  if (y < 0)
    y=0;
  else
    if (y > (int) (ximage->height-(height/magnify)))
      y=ximage->height-height/magnify;
  q=(unsigned char *) windows->magnify.ximage->data;
  scanline_pad=(unsigned int) (windows->magnify.ximage->bytes_per_line-
    ((width*windows->magnify.ximage->bits_per_pixel) >> 3));
  if (ximage->bits_per_pixel < 8)
    {
      register unsigned char
        background,
        byte,
        foreground,
        p_bit,
        q_bit;

      register unsigned int
        plane;

      XPixelInfo
        *pixel_info;

      pixel_info=windows->magnify.pixel_info;
      switch (ximage->bitmap_bit_order)
      {
        case LSBFirst:
        {
          /*
            Magnify little-endian bitmap.
          */
          background=0x00;
          foreground=0x80;
          if (ximage->format == XYBitmap)
            {
              background=(unsigned char)
                (XPixelIntensity(&pixel_info->foreground_color) <
                 XPixelIntensity(&pixel_info->background_color) ?  0x80 : 0x00);
              foreground=(unsigned char)
                (XPixelIntensity(&pixel_info->background_color) <
                 XPixelIntensity(&pixel_info->foreground_color) ?  0x80 : 0x00);
              if (windows->magnify.depth > 1)
                Swap(background,foreground);
            }
          for (i=0; i < (ssize_t) height; i+=magnify)
          {
            /*
              Propogate pixel magnify rows.
            */
            for (j=0; j < magnify; j++)
            {
              p=(unsigned char *) ximage->data+y*ximage->bytes_per_line+
                ((x*ximage->bits_per_pixel) >> 3);
              p_bit=(unsigned char) (x*ximage->bits_per_pixel) & 0x07;
              q_bit=0;
              byte=0;
              for (k=0; k < width; k+=magnify)
              {
                /*
                  Propogate pixel magnify columns.
                */
                for (l=0; l < magnify; l++)
                {
                  /*
                    Propogate each bit plane.
                  */
                  for (plane=0; (int) plane < ximage->bits_per_pixel; plane++)
                  {
                    byte>>=1;
                    if (*p & (0x01 << (p_bit+plane)))
                      byte|=foreground;
                    else
                      byte|=background;
                    q_bit++;
                    if (q_bit == 8)
                      {
                        *q++=byte;
                        q_bit=0;
                        byte=0;
                      }
                  }
                }
                p_bit+=ximage->bits_per_pixel;
                if (p_bit == 8)
                  {
                    p++;
                    p_bit=0;
                  }
                if (q_bit != 0)
                  *q=byte >> (8-q_bit);
                q+=scanline_pad;
              }
            }
            y++;
          }
          break;
        }
        case MSBFirst:
        default:
        {
          /*
            Magnify big-endian bitmap.
          */
          background=0x00;
          foreground=0x01;
          if (ximage->format == XYBitmap)
            {
              background=(unsigned char)
                (XPixelIntensity(&pixel_info->foreground_color) <
                 XPixelIntensity(&pixel_info->background_color) ?  0x01 : 0x00);
              foreground=(unsigned char)
                (XPixelIntensity(&pixel_info->background_color) <
                 XPixelIntensity(&pixel_info->foreground_color) ?  0x01 : 0x00);
              if (windows->magnify.depth > 1)
                Swap(background,foreground);
            }
          for (i=0; i < (ssize_t) height; i+=magnify)
          {
            /*
              Propogate pixel magnify rows.
            */
            for (j=0; j < magnify; j++)
            {
              p=(unsigned char *) ximage->data+y*ximage->bytes_per_line+
                ((x*ximage->bits_per_pixel) >> 3);
              p_bit=(unsigned char) (x*ximage->bits_per_pixel) & 0x07;
              q_bit=0;
              byte=0;
              for (k=0; k < width; k+=magnify)
              {
                /*
                  Propogate pixel magnify columns.
                */
                for (l=0; l < magnify; l++)
                {
                  /*
                    Propogate each bit plane.
                  */
                  for (plane=0; (int) plane < ximage->bits_per_pixel; plane++)
                  {
                    byte<<=1;
                    if (*p & (0x80 >> (p_bit+plane)))
                      byte|=foreground;
                    else
                      byte|=background;
                    q_bit++;
                    if (q_bit == 8)
                      {
                        *q++=byte;
                        q_bit=0;
                        byte=0;
                      }
                  }
                }
                p_bit+=ximage->bits_per_pixel;
                if (p_bit == 8)
                  {
                    p++;
                    p_bit=0;
                  }
                if (q_bit != 0)
                  *q=byte << (8-q_bit);
                q+=scanline_pad;
              }
            }
            y++;
          }
          break;
        }
      }
    }
  else
    switch (ximage->bits_per_pixel)
    {
      case 6:
      case 8:
      {
        /*
          Magnify 8 bit X image.
        */
        for (i=0; i < (ssize_t) height; i+=magnify)
        {
          /*
            Propogate pixel magnify rows.
          */
          for (j=0; j < magnify; j++)
          {
            p=(unsigned char *) ximage->data+y*ximage->bytes_per_line+
              ((x*ximage->bits_per_pixel) >> 3);
            for (k=0; k < width; k+=magnify)
            {
              /*
                Propogate pixel magnify columns.
              */
              for (l=0; l < magnify; l++)
                *q++=(*p);
              p++;
            }
            q+=scanline_pad;
          }
          y++;
        }
        break;
      }
      default:
      {
        register unsigned int
          bytes_per_pixel,
          m;

        /*
          Magnify multi-byte X image.
        */
        bytes_per_pixel=(unsigned int) ximage->bits_per_pixel >> 3;
        for (i=0; i < (ssize_t) height; i+=magnify)
        {
          /*
            Propogate pixel magnify rows.
          */
          for (j=0; j < magnify; j++)
          {
            p=(unsigned char *) ximage->data+y*ximage->bytes_per_line+
              ((x*ximage->bits_per_pixel) >> 3);
            for (k=0; k < width; k+=magnify)
            {
              /*
                Propogate pixel magnify columns.
              */
              for (l=0; l < magnify; l++)
                for (m=0; m < bytes_per_pixel; m++)
                  *q++=(*(p+m));
              p+=bytes_per_pixel;
            }
            q+=scanline_pad;
          }
          y++;
        }
        break;
      }
    }
  /*
    Copy X image to magnify pixmap.
  */
  x=windows->magnify.x-((width/magnify) >> 1);
  if (x < 0)
    x=(int) ((width >> 1)-windows->magnify.x*magnify);
  else
    if (x > (int) (ximage->width-(width/magnify)))
      x=(int) ((ximage->width-windows->magnify.x)*magnify-(width >> 1));
    else
      x=0;
  y=windows->magnify.y-((height/magnify) >> 1);
  if (y < 0)
    y=(int) ((height >> 1)-windows->magnify.y*magnify);
  else
    if (y > (int) (ximage->height-(height/magnify)))
      y=(int) ((ximage->height-windows->magnify.y)*magnify-(height >> 1));
    else
      y=0;
  if ((x != 0) || (y != 0))
    (void) XFillRectangle(display,windows->magnify.pixmap,
      windows->magnify.annotate_context,0,0,width,height);
  (void) XPutImage(display,windows->magnify.pixmap,
    windows->magnify.annotate_context,windows->magnify.ximage,0,0,x,y,width-x,
    height-y);
  if ((magnify > 1) && ((magnify <= (width >> 1)) &&
      (magnify <= (height >> 1))))
    {
      RectangleInfo
        highlight_info;

      /*
        Highlight center pixel.
      */
      highlight_info.x=(ssize_t) windows->magnify.width >> 1;
      highlight_info.y=(ssize_t) windows->magnify.height >> 1;
      highlight_info.width=magnify;
      highlight_info.height=magnify;
      (void) XDrawRectangle(display,windows->magnify.pixmap,
        windows->magnify.highlight_context,(int) highlight_info.x,
        (int) highlight_info.y,(unsigned int) highlight_info.width-1,
        (unsigned int) highlight_info.height-1);
      if (magnify > 2)
        (void) XDrawRectangle(display,windows->magnify.pixmap,
          windows->magnify.annotate_context,(int) highlight_info.x+1,
          (int) highlight_info.y+1,(unsigned int) highlight_info.width-3,
          (unsigned int) highlight_info.height-3);
    }
  /*
    Show center pixel color.
  */
  (void) GetOneVirtualPixelInfo(windows->image.image,TileVirtualPixelMethod,
    (ssize_t) windows->magnify.x,(ssize_t) windows->magnify.y,&pixel,exception);
  (void) FormatLocaleString(tuple,MagickPathExtent,"%d,%d: ",
    windows->magnify.x,windows->magnify.y);
  (void) ConcatenateMagickString(tuple,"(",MagickPathExtent);
  ConcatenateColorComponent(&pixel,RedPixelChannel,X11Compliance,tuple);
  (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
  ConcatenateColorComponent(&pixel,GreenPixelChannel,X11Compliance,tuple);
  (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
  ConcatenateColorComponent(&pixel,BluePixelChannel,X11Compliance,tuple);
  if (pixel.colorspace == CMYKColorspace)
    {
      (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
      ConcatenateColorComponent(&pixel,BlackPixelChannel,X11Compliance,tuple);
    }
  if (pixel.alpha_trait != UndefinedPixelTrait)
    {
      (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
      ConcatenateColorComponent(&pixel,AlphaPixelChannel,X11Compliance,tuple);
    }
  (void) ConcatenateMagickString(tuple,")",MagickPathExtent);
  height=(unsigned int) windows->magnify.font_info->ascent+
    windows->magnify.font_info->descent;
  x=windows->magnify.font_info->max_bounds.width >> 1;
  y=windows->magnify.font_info->ascent+(height >> 2);
  (void) XDrawImageString(display,windows->magnify.pixmap,
    windows->magnify.annotate_context,x,y,tuple,(int) strlen(tuple));
  GetColorTuple(&pixel,MagickTrue,tuple);
  y+=height;
  (void) XDrawImageString(display,windows->magnify.pixmap,
    windows->magnify.annotate_context,x,y,tuple,(int) strlen(tuple));
  (void) QueryColorname(windows->image.image,&pixel,SVGCompliance,tuple,
    exception);
  y+=height;
  (void) XDrawImageString(display,windows->magnify.pixmap,
    windows->magnify.annotate_context,x,y,tuple,(int) strlen(tuple));
  /*
    Refresh magnify window.
  */
  magnify_window=windows->magnify;
  magnify_window.x=0;
  magnify_window.y=0;
  XRefreshWindow(display,&magnify_window,(XEvent *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e P i x m a p                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMakePixmap() creates an X11 pixmap.
%
%  The format of the XMakePixmap method is:
%
%      void XMakeStandardColormap(Display *display,XVisualInfo *visual_info,
%        XResourceInfo *resource_info,Image *image,XStandardColormap *map_info,
%        XPixelInfo *pixel)
%
%  A description of each parameter follows:
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a XWindowInfo structure.
%
*/
static MagickBooleanType XMakePixmap(Display *display,
  const XResourceInfo *resource_info,XWindowInfo *window)
{
  unsigned int
    height,
    width;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(resource_info != (XResourceInfo *) NULL);
  assert(window != (XWindowInfo  *) NULL);
  if (window->pixmap != (Pixmap) NULL)
    {
      /*
        Destroy previous X pixmap.
      */
      (void) XFreePixmap(display,window->pixmap);
      window->pixmap=(Pixmap) NULL;
    }
  if (window->use_pixmap == MagickFalse)
    return(MagickFalse);
  if (window->ximage == (XImage *) NULL)
    return(MagickFalse);
  /*
    Display busy cursor.
  */
  (void) XCheckDefineCursor(display,window->id,window->busy_cursor);
  (void) XFlush(display);
  /*
    Create pixmap.
  */
  width=(unsigned int) window->ximage->width;
  height=(unsigned int) window->ximage->height;
  window->pixmap=XCreatePixmap(display,window->id,width,height,window->depth);
  if (window->pixmap == (Pixmap) NULL)
    {
      /*
        Unable to allocate pixmap.
      */
      (void) XCheckDefineCursor(display,window->id,window->cursor);
      return(MagickFalse);
    }
  /*
    Copy X image to pixmap.
  */
#if defined(MAGICKCORE_HAVE_SHARED_MEMORY)
  if (window->shared_memory)
    (void) XShmPutImage(display,window->pixmap,window->annotate_context,
      window->ximage,0,0,0,0,width,height,MagickTrue);
#endif
  if (window->shared_memory == MagickFalse)
    (void) XPutImage(display,window->pixmap,window->annotate_context,
      window->ximage,0,0,0,0,width,height);
  if (IsEventLogging())
    {
      (void) LogMagickEvent(X11Event,GetMagickModule(),"Pixmap:");
      (void) LogMagickEvent(X11Event,GetMagickModule(),"  width, height: %ux%u",
        width,height);
    }
  /*
    Restore cursor.
  */
  (void) XCheckDefineCursor(display,window->id,window->cursor);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e S t a n d a r d C o l o r m a p                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMakeStandardColormap() creates an X11 Standard Colormap.
%
%  The format of the XMakeStandardColormap method is:
%
%      void XMakeStandardColormap(Display *display,XVisualInfo *visual_info,
%        XResourceInfo *resource_info,Image *image,XStandardColormap *map_info,
%        XPixelInfo *pixel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o visual_info: Specifies a pointer to a X11 XVisualInfo structure;
%      returned from XGetVisualInfo.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o image: the image.
%
%    o map_info: If a Standard Colormap type is specified, this structure is
%      initialized with info from the Standard Colormap.
%
%    o pixel: Specifies a pointer to a XPixelInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static inline double DiversityPixelIntensity(
  const DiversityPacket *pixel)
{
  double
    intensity;

  intensity=0.212656*pixel->red+0.715158*pixel->green+0.072186*pixel->blue;
  return(intensity);
}

static int IntensityCompare(const void *x,const void *y)
{
  DiversityPacket
    *color_1,
    *color_2;

  int
    diversity;

  color_1=(DiversityPacket *) x;
  color_2=(DiversityPacket *) y;
  diversity=(int) (DiversityPixelIntensity(color_2)-
    DiversityPixelIntensity(color_1));
  return(diversity);
}

static int PopularityCompare(const void *x,const void *y)
{
  DiversityPacket
    *color_1,
    *color_2;

  color_1=(DiversityPacket *) x;
  color_2=(DiversityPacket *) y;
  return((int) color_2->count-(int) color_1->count);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static inline Quantum ScaleXToQuantum(const size_t x,
  const size_t scale)
{
  return((Quantum) (((double) QuantumRange*x)/scale+0.5));
}

MagickPrivate void XMakeStandardColormap(Display *display,
  XVisualInfo *visual_info,XResourceInfo *resource_info,Image *image,
  XStandardColormap *map_info,XPixelInfo *pixel,ExceptionInfo *exception)
{
  Colormap
    colormap;

  register ssize_t
    i;

  Status
    status;

  size_t
    number_colors,
    retain_colors;

  unsigned short
    gray_value;

  XColor
    color,
    *colors,
    *p;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(visual_info != (XVisualInfo *) NULL);
  assert(map_info != (XStandardColormap *) NULL);
  assert(resource_info != (XResourceInfo *) NULL);
  assert(pixel != (XPixelInfo *) NULL);
  if (resource_info->map_type != (char *) NULL)
    {
      /*
        Standard Colormap is already defined (i.e. xstdcmap).
      */
      XGetPixelInfo(display,visual_info,map_info,resource_info,image,
        pixel);
      number_colors=(unsigned int) (map_info->base_pixel+
        (map_info->red_max+1)*(map_info->green_max+1)*(map_info->blue_max+1));
      if ((map_info->red_max*map_info->green_max*map_info->blue_max) != 0)
        if ((image->alpha_trait == UndefinedPixelTrait) &&
            (resource_info->color_recovery == MagickFalse) &&
            (resource_info->quantize_info->dither_method != NoDitherMethod) &&
            (number_colors < MaxColormapSize))
          {
            Image
              *affinity_image;

            register Quantum
              *magick_restrict q;

            /*
              Improve image appearance with error diffusion.
            */
            affinity_image=AcquireImage((ImageInfo *) NULL,exception);
            if (affinity_image == (Image *) NULL)
              ThrowXWindowFatalException(ResourceLimitFatalError,
                "UnableToDitherImage",image->filename);
            affinity_image->columns=number_colors;
            affinity_image->rows=1;
            /*
              Initialize colormap image.
            */
            q=QueueAuthenticPixels(affinity_image,0,0,affinity_image->columns,
              1,exception);
            if (q != (Quantum *) NULL)
              {
                for (i=0; i < (ssize_t) number_colors; i++)
                {
                  SetPixelRed(affinity_image,0,q);
                  if (map_info->red_max != 0)
                    SetPixelRed(affinity_image,ScaleXToQuantum((size_t)
                      (i/map_info->red_mult),map_info->red_max),q);
                  SetPixelGreen(affinity_image,0,q);
                  if (map_info->green_max != 0)
                    SetPixelGreen(affinity_image,ScaleXToQuantum((size_t)
                      ((i/map_info->green_mult) % (map_info->green_max+1)),
                      map_info->green_max),q);
                  SetPixelBlue(affinity_image,0,q);
                  if (map_info->blue_max != 0)
                    SetPixelBlue(affinity_image,ScaleXToQuantum((size_t)
                      (i % map_info->green_mult),map_info->blue_max),q);
                  SetPixelAlpha(affinity_image,
                    TransparentAlpha,q);
                  q+=GetPixelChannels(affinity_image);
                }
                (void) SyncAuthenticPixels(affinity_image,exception);
                (void) RemapImage(resource_info->quantize_info,image,
                  affinity_image,exception);
              }
            XGetPixelInfo(display,visual_info,map_info,resource_info,image,
              pixel);
            (void) SetImageStorageClass(image,DirectClass,exception);
            affinity_image=DestroyImage(affinity_image);
          }
      if (IsEventLogging())
        {
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Standard Colormap:");
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "  colormap id: 0x%lx",map_info->colormap);
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "  red, green, blue max: %lu %lu %lu",map_info->red_max,
            map_info->green_max,map_info->blue_max);
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "  red, green, blue mult: %lu %lu %lu",map_info->red_mult,
            map_info->green_mult,map_info->blue_mult);
        }
      return;
    }
  if ((visual_info->klass != DirectColor) &&
      (visual_info->klass != TrueColor))
    if ((image->storage_class == DirectClass) ||
        ((int) image->colors > visual_info->colormap_size))
      {
        QuantizeInfo
          quantize_info;

        /*
          Image has more colors than the visual supports.
        */
        quantize_info=(*resource_info->quantize_info);
        quantize_info.number_colors=(size_t) visual_info->colormap_size;
        (void) QuantizeImage(&quantize_info,image,exception);
      }
  /*
    Free previous and create new colormap.
  */
  (void) XFreeStandardColormap(display,visual_info,map_info,pixel);
  colormap=XDefaultColormap(display,visual_info->screen);
  if (visual_info->visual != XDefaultVisual(display,visual_info->screen))
    colormap=XCreateColormap(display,XRootWindow(display,visual_info->screen),
      visual_info->visual,visual_info->klass == DirectColor ?
      AllocAll : AllocNone);
  if (colormap == (Colormap) NULL)
    ThrowXWindowFatalException(ResourceLimitFatalError,"UnableToCreateColormap",
      image->filename);
  /*
    Initialize the map and pixel info structures.
  */
  XGetMapInfo(visual_info,colormap,map_info);
  XGetPixelInfo(display,visual_info,map_info,resource_info,image,pixel);
  /*
    Allocating colors in server colormap is based on visual class.
  */
  switch (visual_info->klass)
  {
    case StaticGray:
    case StaticColor:
    {
      /*
        Define Standard Colormap for StaticGray or StaticColor visual.
      */
      number_colors=image->colors;
      colors=(XColor *) AcquireQuantumMemory((size_t)
        visual_info->colormap_size,sizeof(*colors));
      if (colors == (XColor *) NULL)
        ThrowXWindowFatalException(ResourceLimitFatalError,
          "UnableToCreateColormap",image->filename);
      p=colors;
      color.flags=(char) (DoRed | DoGreen | DoBlue);
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        color.red=ScaleQuantumToShort(XRedGamma(image->colormap[i].red));
        color.green=ScaleQuantumToShort(XGreenGamma(image->colormap[i].green));
        color.blue=ScaleQuantumToShort(XBlueGamma(image->colormap[i].blue));
        if (visual_info->klass != StaticColor)
          {
            gray_value=(unsigned short) XPixelIntensity(&color);
            color.red=gray_value;
            color.green=gray_value;
            color.blue=gray_value;
          }
        status=XAllocColor(display,colormap,&color);
        if (status == False)
          {
            colormap=XCopyColormapAndFree(display,colormap);
            (void) XAllocColor(display,colormap,&color);
          }
        pixel->pixels[i]=color.pixel;
        *p++=color;
      }
      break;
    }
    case GrayScale:
    case PseudoColor:
    {
      unsigned int
        colormap_type;

      /*
        Define Standard Colormap for GrayScale or PseudoColor visual.
      */
      number_colors=image->colors;
      colors=(XColor *) AcquireQuantumMemory((size_t)
        visual_info->colormap_size,sizeof(*colors));
      if (colors == (XColor *) NULL)
        ThrowXWindowFatalException(ResourceLimitFatalError,
          "UnableToCreateColormap",image->filename);
      /*
        Preallocate our GUI colors.
      */
      (void) XAllocColor(display,colormap,&pixel->foreground_color);
      (void) XAllocColor(display,colormap,&pixel->background_color);
      (void) XAllocColor(display,colormap,&pixel->border_color);
      (void) XAllocColor(display,colormap,&pixel->matte_color);
      (void) XAllocColor(display,colormap,&pixel->highlight_color);
      (void) XAllocColor(display,colormap,&pixel->shadow_color);
      (void) XAllocColor(display,colormap,&pixel->depth_color);
      (void) XAllocColor(display,colormap,&pixel->trough_color);
      for (i=0; i < MaxNumberPens; i++)
        (void) XAllocColor(display,colormap,&pixel->pen_colors[i]);
      /*
        Determine if image colors will "fit" into X server colormap.
      */
      colormap_type=resource_info->colormap;
      status=XAllocColorCells(display,colormap,MagickFalse,(unsigned long *)
        NULL,0,pixel->pixels,(unsigned int) image->colors);
      if (status != False)
        colormap_type=PrivateColormap;
      if (colormap_type == SharedColormap)
        {
          CacheView
            *image_view;

          DiversityPacket
            *diversity;

          int
            y;

          register int
            x;

          unsigned short
            index;

          XColor
            *server_colors;

          /*
            Define Standard colormap for shared GrayScale or PseudoColor visual.
          */
          diversity=(DiversityPacket *) AcquireQuantumMemory(image->colors,
            sizeof(*diversity));
          if (diversity == (DiversityPacket *) NULL)
            ThrowXWindowFatalException(ResourceLimitFatalError,
              "UnableToCreateColormap",image->filename);
          for (i=0; i < (ssize_t) image->colors; i++)
          {
            diversity[i].red=ClampToQuantum(image->colormap[i].red);
            diversity[i].green=ClampToQuantum(image->colormap[i].green);
            diversity[i].blue=ClampToQuantum(image->colormap[i].blue);
            diversity[i].index=(unsigned short) i;
            diversity[i].count=0;
          }
          image_view=AcquireAuthenticCacheView(image,exception);
          for (y=0; y < (int) image->rows; y++)
          {
            register int
              x;

            register const Quantum
              *magick_restrict p;

            p=GetCacheViewAuthenticPixels(image_view,0,(ssize_t) y,
              image->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=(int) image->columns-1; x >= 0; x--)
            {
              diversity[(ssize_t) GetPixelIndex(image,p)].count++;
              p+=GetPixelChannels(image);
            }
          }
          image_view=DestroyCacheView(image_view);
          /*
            Sort colors by decreasing intensity.
          */
          qsort((void *) diversity,image->colors,sizeof(*diversity),
            IntensityCompare);
          for (i=0; i < (ssize_t) image->colors; )
          {
            diversity[i].count<<=4;  /* increase this colors popularity */
            i+=MagickMax((int) (image->colors >> 4),2);
          }
          diversity[image->colors-1].count<<=4;
          qsort((void *) diversity,image->colors,sizeof(*diversity),
            PopularityCompare);
          /*
            Allocate colors.
          */
          p=colors;
          color.flags=(char) (DoRed | DoGreen | DoBlue);
          for (i=0; i < (ssize_t) image->colors; i++)
          {
            index=diversity[i].index;
            color.red=
              ScaleQuantumToShort(XRedGamma(image->colormap[index].red));
            color.green=
              ScaleQuantumToShort(XGreenGamma(image->colormap[index].green));
            color.blue=
              ScaleQuantumToShort(XBlueGamma(image->colormap[index].blue));
            if (visual_info->klass != PseudoColor)
              {
                gray_value=(unsigned short) XPixelIntensity(&color);
                color.red=gray_value;
                color.green=gray_value;
                color.blue=gray_value;
              }
            status=XAllocColor(display,colormap,&color);
            if (status == False)
              break;
            pixel->pixels[index]=color.pixel;
            *p++=color;
          }
          /*
            Read X server colormap.
          */
          server_colors=(XColor *) AcquireQuantumMemory((size_t)
            visual_info->colormap_size,sizeof(*server_colors));
          if (server_colors == (XColor *) NULL)
            ThrowXWindowFatalException(ResourceLimitFatalError,
              "UnableToCreateColormap",image->filename);
          for (x=visual_info->colormap_size-1; x >= 0; x--)
            server_colors[x].pixel=(size_t) x;
          (void) XQueryColors(display,colormap,server_colors,
            (int) MagickMin((unsigned int) visual_info->colormap_size,256));
          /*
            Select remaining colors from X server colormap.
          */
          for (; i < (ssize_t) image->colors; i++)
          {
            index=diversity[i].index;
            color.red=ScaleQuantumToShort(
              XRedGamma(image->colormap[index].red));
            color.green=ScaleQuantumToShort(
              XGreenGamma(image->colormap[index].green));
            color.blue=ScaleQuantumToShort(
              XBlueGamma(image->colormap[index].blue));
            if (visual_info->klass != PseudoColor)
              {
                gray_value=(unsigned short) XPixelIntensity(&color);
                color.red=gray_value;
                color.green=gray_value;
                color.blue=gray_value;
              }
            XBestPixel(display,colormap,server_colors,(unsigned int)
              visual_info->colormap_size,&color);
            pixel->pixels[index]=color.pixel;
            *p++=color;
          }
          if ((int) image->colors < visual_info->colormap_size)
            {
              /*
                Fill up colors array-- more choices for pen colors.
              */
              retain_colors=MagickMin((unsigned int)
               (visual_info->colormap_size-image->colors),256);
              for (i=0; i < (ssize_t) retain_colors; i++)
                *p++=server_colors[i];
              number_colors+=retain_colors;
            }
          server_colors=(XColor *) RelinquishMagickMemory(server_colors);
          diversity=(DiversityPacket *) RelinquishMagickMemory(diversity);
          break;
        }
      /*
        Define Standard colormap for private GrayScale or PseudoColor visual.
      */
      if (status == False)
        {
          /*
            Not enough colormap entries in the colormap-- Create a new colormap.
          */
          colormap=XCreateColormap(display,
            XRootWindow(display,visual_info->screen),visual_info->visual,
            AllocNone);
          if (colormap == (Colormap) NULL)
            ThrowXWindowFatalException(ResourceLimitFatalError,
              "UnableToCreateColormap",image->filename);
          map_info->colormap=colormap;
          if ((int) image->colors < visual_info->colormap_size)
            {
              /*
                Retain colors from the default colormap to help lessens the
                effects of colormap flashing.
              */
              retain_colors=MagickMin((unsigned int)
                (visual_info->colormap_size-image->colors),256);
              p=colors+image->colors;
              for (i=0; i < (ssize_t) retain_colors; i++)
              {
                p->pixel=(unsigned long) i;
                p++;
              }
              (void) XQueryColors(display,
                XDefaultColormap(display,visual_info->screen),
                colors+image->colors,(int) retain_colors);
              /*
                Transfer colors from default to private colormap.
              */
              (void) XAllocColorCells(display,colormap,MagickFalse,
                (unsigned long *) NULL,0,pixel->pixels,(unsigned int)
                retain_colors);
              p=colors+image->colors;
              for (i=0; i < (ssize_t) retain_colors; i++)
              {
                p->pixel=pixel->pixels[i];
                p++;
              }
              (void) XStoreColors(display,colormap,colors+image->colors,
                (int) retain_colors);
              number_colors+=retain_colors;
            }
          (void) XAllocColorCells(display,colormap,MagickFalse,
            (unsigned long *) NULL,0,pixel->pixels,(unsigned int)
            image->colors);
        }
      /*
        Store the image colormap.
      */
      p=colors;
      color.flags=(char) (DoRed | DoGreen | DoBlue);
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        color.red=ScaleQuantumToShort(XRedGamma(image->colormap[i].red));
        color.green=ScaleQuantumToShort(XGreenGamma(image->colormap[i].green));
        color.blue=ScaleQuantumToShort(XBlueGamma(image->colormap[i].blue));
        if (visual_info->klass != PseudoColor)
          {
            gray_value=(unsigned short) XPixelIntensity(&color);
            color.red=gray_value;
            color.green=gray_value;
            color.blue=gray_value;
          }
        color.pixel=pixel->pixels[i];
        *p++=color;
      }
      (void) XStoreColors(display,colormap,colors,(int) image->colors);
      break;
    }
    case TrueColor:
    case DirectColor:
    default:
    {
      MagickBooleanType
        linear_colormap;

      /*
        Define Standard Colormap for TrueColor or DirectColor visual.
      */
      number_colors=(unsigned int) ((map_info->red_max*map_info->red_mult)+
        (map_info->green_max*map_info->green_mult)+
        (map_info->blue_max*map_info->blue_mult)+1);
      linear_colormap=(number_colors > 4096) ||
        (((int) (map_info->red_max+1) == visual_info->colormap_size) &&
         ((int) (map_info->green_max+1) == visual_info->colormap_size) &&
         ((int) (map_info->blue_max+1) == visual_info->colormap_size)) ?
         MagickTrue : MagickFalse;
      if (linear_colormap != MagickFalse)
        number_colors=(size_t) visual_info->colormap_size;
      /*
        Allocate color array.
      */
      colors=(XColor *) AcquireQuantumMemory(number_colors,sizeof(*colors));
      if (colors == (XColor *) NULL)
        ThrowXWindowFatalException(ResourceLimitFatalError,
          "UnableToCreateColormap",image->filename);
      /*
        Initialize linear color ramp.
      */
      p=colors;
      color.flags=(char) (DoRed | DoGreen | DoBlue);
      if (linear_colormap != MagickFalse)
        for (i=0; i < (ssize_t) number_colors; i++)
        {
          color.blue=(unsigned short) 0;
          if (map_info->blue_max != 0)
            color.blue=(unsigned short) ((size_t)
              ((65535L*(i % map_info->green_mult))/map_info->blue_max));
          color.green=color.blue;
          color.red=color.blue;
          color.pixel=XStandardPixel(map_info,&color);
          *p++=color;
        }
      else
        for (i=0; i < (ssize_t) number_colors; i++)
        {
          color.red=(unsigned short) 0;
          if (map_info->red_max != 0)
            color.red=(unsigned short) ((size_t)
              ((65535L*(i/map_info->red_mult))/map_info->red_max));
          color.green=(unsigned int) 0;
          if (map_info->green_max != 0)
            color.green=(unsigned short) ((size_t)
              ((65535L*((i/map_info->green_mult) % (map_info->green_max+1)))/
                map_info->green_max));
          color.blue=(unsigned short) 0;
          if (map_info->blue_max != 0)
            color.blue=(unsigned short) ((size_t)
              ((65535L*(i % map_info->green_mult))/map_info->blue_max));
          color.pixel=XStandardPixel(map_info,&color);
          *p++=color;
        }
      if ((visual_info->klass == DirectColor) &&
          (colormap != XDefaultColormap(display,visual_info->screen)))
        (void) XStoreColors(display,colormap,colors,(int) number_colors);
      else
        for (i=0; i < (ssize_t) number_colors; i++)
          (void) XAllocColor(display,colormap,&colors[i]);
      break;
    }
  }
  if ((visual_info->klass != DirectColor) &&
      (visual_info->klass != TrueColor))
    {
      /*
        Set foreground, background, border, etc. pixels.
      */
      XBestPixel(display,colormap,colors,(unsigned int) number_colors,
        &pixel->foreground_color);
      XBestPixel(display,colormap,colors,(unsigned int) number_colors,
        &pixel->background_color);
      if (pixel->background_color.pixel == pixel->foreground_color.pixel)
        {
          /*
            Foreground and background colors must differ.
          */
          pixel->background_color.red=(~pixel->foreground_color.red);
          pixel->background_color.green=
            (~pixel->foreground_color.green);
          pixel->background_color.blue=
            (~pixel->foreground_color.blue);
          XBestPixel(display,colormap,colors,(unsigned int) number_colors,
            &pixel->background_color);
        }
      XBestPixel(display,colormap,colors,(unsigned int) number_colors,
        &pixel->border_color);
      XBestPixel(display,colormap,colors,(unsigned int) number_colors,
        &pixel->matte_color);
      XBestPixel(display,colormap,colors,(unsigned int) number_colors,
        &pixel->highlight_color);
      XBestPixel(display,colormap,colors,(unsigned int) number_colors,
        &pixel->shadow_color);
      XBestPixel(display,colormap,colors,(unsigned int) number_colors,
        &pixel->depth_color);
      XBestPixel(display,colormap,colors,(unsigned int) number_colors,
        &pixel->trough_color);
      for (i=0; i < MaxNumberPens; i++)
      {
        XBestPixel(display,colormap,colors,(unsigned int) number_colors,
          &pixel->pen_colors[i]);
        pixel->pixels[image->colors+i]=pixel->pen_colors[i].pixel;
      }
      pixel->colors=(ssize_t) (image->colors+MaxNumberPens);
    }
  colors=(XColor *) RelinquishMagickMemory(colors);
  if (IsEventLogging())
    {
      (void) LogMagickEvent(X11Event,GetMagickModule(),"Standard Colormap:");
      (void) LogMagickEvent(X11Event,GetMagickModule(),"  colormap id: 0x%lx",
        map_info->colormap);
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "  red, green, blue max: %lu %lu %lu",map_info->red_max,
        map_info->green_max,map_info->blue_max);
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "  red, green, blue mult: %lu %lu %lu",map_info->red_mult,
        map_info->green_mult,map_info->blue_mult);
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e W i n d o w                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMakeWindow() creates an X11 window.
%
%  The format of the XMakeWindow method is:
%
%      void XMakeWindow(Display *display,Window parent,char **argv,int argc,
%        XClassHint *class_hint,XWMHints *manager_hints,
%        XWindowInfo *window_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o parent: Specifies the parent window_info.
%
%    o argv: Specifies the application's argument list.
%
%    o argc: Specifies the number of arguments.
%
%    o class_hint: Specifies a pointer to a X11 XClassHint structure.
%
%    o manager_hints: Specifies a pointer to a X11 XWMHints structure.
%
%    o window_info: Specifies a pointer to a X11 XWindowInfo structure.
%
*/
MagickPrivate void XMakeWindow(Display *display,Window parent,char **argv,
  int argc,XClassHint *class_hint,XWMHints *manager_hints,
  XWindowInfo *window_info)
{
#define MinWindowSize  64

  Atom
    atom_list[2];

  int
    gravity;

  static XTextProperty
    icon_name,
    window_name;

  Status
    status;

  XSizeHints
    *size_hints;

  /*
    Set window info hints.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(window_info != (XWindowInfo *) NULL);
  size_hints=XAllocSizeHints();
  if (size_hints == (XSizeHints *) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToMakeXWindow",argv[0]);
  size_hints->flags=(int) window_info->flags;
  size_hints->x=window_info->x;
  size_hints->y=window_info->y;
  size_hints->width=(int) window_info->width;
  size_hints->height=(int) window_info->height;
  if (window_info->immutable != MagickFalse)
    {
      /*
        Window size cannot be changed.
      */
      size_hints->min_width=size_hints->width;
      size_hints->min_height=size_hints->height;
      size_hints->max_width=size_hints->width;
      size_hints->max_height=size_hints->height;
      size_hints->flags|=PMinSize;
      size_hints->flags|=PMaxSize;
    }
  else
    {
      /*
        Window size can be changed.
      */
      size_hints->min_width=(int) window_info->min_width;
      size_hints->min_height=(int) window_info->min_height;
      size_hints->flags|=PResizeInc;
      size_hints->width_inc=(int) window_info->width_inc;
      size_hints->height_inc=(int) window_info->height_inc;
#if !defined(PRE_R4_ICCCM)
      size_hints->flags|=PBaseSize;
      size_hints->base_width=size_hints->width_inc;
      size_hints->base_height=size_hints->height_inc;
#endif
    }
  gravity=NorthWestGravity;
  if (window_info->geometry != (char *) NULL)
    {
      char
        default_geometry[MagickPathExtent],
        geometry[MagickPathExtent];

      int
        flags;

      register char
        *p;

      /*
        User specified geometry.
      */
      (void) FormatLocaleString(default_geometry,MagickPathExtent,"%dx%d",
        size_hints->width,size_hints->height);
      (void) CopyMagickString(geometry,window_info->geometry,MagickPathExtent);
      p=geometry;
      while (strlen(p) != 0)
      {
        if ((isspace((int) ((unsigned char) *p)) == 0) && (*p != '%'))
          p++;
        else
          (void) memmove(p,p+1,MagickPathExtent-(p-geometry));
      }
      flags=XWMGeometry(display,window_info->screen,geometry,default_geometry,
        window_info->border_width,size_hints,&size_hints->x,&size_hints->y,
        &size_hints->width,&size_hints->height,&gravity);
      if ((flags & WidthValue) && (flags & HeightValue))
        size_hints->flags|=USSize;
      if ((flags & XValue) && (flags & YValue))
        {
          size_hints->flags|=USPosition;
          window_info->x=size_hints->x;
          window_info->y=size_hints->y;
        }
    }
#if !defined(PRE_R4_ICCCM)
  size_hints->win_gravity=gravity;
  size_hints->flags|=PWinGravity;
#endif
  if (window_info->id == (Window) NULL)
    window_info->id=XCreateWindow(display,parent,window_info->x,window_info->y,
      (unsigned int) size_hints->width,(unsigned int) size_hints->height,
      window_info->border_width,(int) window_info->depth,InputOutput,
      window_info->visual,(unsigned long) window_info->mask,
      &window_info->attributes);
  else
    {
      MagickStatusType
        mask;

      XEvent
        sans_event;

      XWindowChanges
        window_changes;

      /*
        Window already exists;  change relevant attributes.
      */
      (void) XChangeWindowAttributes(display,window_info->id,(unsigned long)
        window_info->mask,&window_info->attributes);
      mask=ConfigureNotify;
      while (XCheckTypedWindowEvent(display,window_info->id,(int) mask,&sans_event)) ;
      window_changes.x=window_info->x;
      window_changes.y=window_info->y;
      window_changes.width=(int) window_info->width;
      window_changes.height=(int) window_info->height;
      mask=(MagickStatusType) (CWWidth | CWHeight);
      if (window_info->flags & USPosition)
        mask|=CWX | CWY;
      (void) XReconfigureWMWindow(display,window_info->id,window_info->screen,
        mask,&window_changes);
    }
  if (window_info->id == (Window) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateWindow",
      window_info->name);
  status=XStringListToTextProperty(&window_info->name,1,&window_name);
  if (status == False)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateTextProperty",
      window_info->name);
  status=XStringListToTextProperty(&window_info->icon_name,1,&icon_name);
  if (status == False)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateTextProperty",
      window_info->icon_name);
  if (window_info->icon_geometry != (char *) NULL)
    {
      int
        flags,
        height,
        width;

      /*
        User specified icon geometry.
      */
      size_hints->flags|=USPosition;
      flags=XWMGeometry(display,window_info->screen,window_info->icon_geometry,
        (char *) NULL,0,size_hints,&manager_hints->icon_x,
        &manager_hints->icon_y,&width,&height,&gravity);
      if ((flags & XValue) && (flags & YValue))
        manager_hints->flags|=IconPositionHint;
    }
  XSetWMProperties(display,window_info->id,&window_name,&icon_name,argv,argc,
    size_hints,manager_hints,class_hint);
  if (window_name.value != (void *) NULL)
    {
      (void) XFree((void *) window_name.value);
      window_name.value=(unsigned char *) NULL;
      window_name.nitems=0;
    }
  if (icon_name.value != (void *) NULL)
    {
      (void) XFree((void *) icon_name.value);
      icon_name.value=(unsigned char *) NULL;
      icon_name.nitems=0;
    }
  atom_list[0]=XInternAtom(display,"WM_DELETE_WINDOW",MagickFalse);
  atom_list[1]=XInternAtom(display,"WM_TAKE_FOCUS",MagickFalse);
  (void) XSetWMProtocols(display,window_info->id,atom_list,2);
  (void) XFree((void *) size_hints);
  if (window_info->shape != MagickFalse)
    {
#if defined(MAGICKCORE_HAVE_SHAPE)
      int
        error_base,
        event_base;

      /*
        Can we apply a non-rectangular shaping mask?
      */
      error_base=0;
      event_base=0;
      if (XShapeQueryExtension(display,&error_base,&event_base) == 0)
        window_info->shape=MagickFalse;
#else
      window_info->shape=MagickFalse;
#endif
    }
  window_info->shape=MagickFalse;  /* Fedora 30 has a broken shape extention */
  if (window_info->shared_memory != MagickFalse)
    {
#if defined(MAGICKCORE_HAVE_SHARED_MEMORY)
      /*
        Can we use shared memory with this window?
      */
      if (XShmQueryExtension(display) == 0)
        window_info->shared_memory=MagickFalse;
#else
      window_info->shared_memory=MagickFalse;
#endif
    }
  window_info->image=NewImageList();
  window_info->destroy=MagickFalse;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a g i c k P r o g r e s s M o n i t o r                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMagickProgressMonitor() displays the progress a task is making in
%  completing a task.
%
%  The format of the XMagickProgressMonitor method is:
%
%      void XMagickProgressMonitor(const char *task,
%        const MagickOffsetType quantum,const MagickSizeType span,
%        void *client_data)
%
%  A description of each parameter follows:
%
%    o task: Identifies the task in progress.
%
%    o quantum: Specifies the quantum position within the span which represents
%      how much progress has been made in completing a task.
%
%    o span: Specifies the span relative to completing a task.
%
%    o client_data: Pointer to any client data.
%
*/

static const char *GetLocaleMonitorMessage(const char *text)
{
  char
    message[MagickPathExtent],
    tag[MagickPathExtent];

  const char
    *locale_message;

  register char
    *p;

  (void) CopyMagickString(tag,text,MagickPathExtent);
  p=strrchr(tag,'/');
  if (p != (char *) NULL)
    *p='\0';
  (void) FormatLocaleString(message,MagickPathExtent,"Monitor/%s",tag);
  locale_message=GetLocaleMessage(message);
  if (locale_message == message)
    return(text);
  return(locale_message);
}

MagickPrivate MagickBooleanType XMagickProgressMonitor(const char *tag,
  const MagickOffsetType quantum,const MagickSizeType span,
  void *magick_unused(client_data))
{
  XWindows
    *windows;

  windows=XSetWindows((XWindows *) ~0);
  if (windows == (XWindows *) NULL)
    return(MagickTrue);
  if (windows->info.mapped != MagickFalse)
    XProgressMonitorWidget(windows->display,windows,
      GetLocaleMonitorMessage(tag),quantum,span);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X Q u e r y C o l o r D a t a b a s e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XQueryColorCompliance() looks up a RGB values for a color given in the target
%  string.
%
%  The format of the XQueryColorDatabase method is:
%
%      MagickBooleanType XQueryColorCompliance(const char *target,XColor *color)
%
%  A description of each parameter follows:
%
%    o target: Specifies the color to lookup in the X color database.
%
%    o color: A pointer to an PixelInfo structure.  The RGB value of the target
%      color is returned as this value.
%
*/
MagickPrivate MagickBooleanType XQueryColorCompliance(const char *target,
  XColor *color)
{
  Colormap
    colormap;

  static Display
    *display = (Display *) NULL;

  Status
    status;

  XColor
    xcolor;

  /*
    Initialize color return value.
  */
  assert(color != (XColor *) NULL);
  color->red=0;
  color->green=0;
  color->blue=0;
  color->flags=(char) (DoRed | DoGreen | DoBlue);
  if ((target == (char *) NULL) || (*target == '\0'))
    target="#ffffffffffff";
  /*
    Let the X server define the color for us.
  */
  if (display == (Display *) NULL)
    display=XOpenDisplay((char *) NULL);
  if (display == (Display *) NULL)
    {
      ThrowXWindowException(XServerError,"ColorIsNotKnownToServer",target);
      return(MagickFalse);
    }
  colormap=XDefaultColormap(display,XDefaultScreen(display));
  status=XParseColor(display,colormap,(char *) target,&xcolor);
  if (status == False)
    ThrowXWindowException(XServerError,"ColorIsNotKnownToServer",target)
  else
    {
      color->red=xcolor.red;
      color->green=xcolor.green;
      color->blue=xcolor.blue;
      color->flags=xcolor.flags;
    }
  return(status != False ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X Q u e r y P o s i t i o n                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XQueryPosition() gets the pointer coordinates relative to a window.
%
%  The format of the XQueryPosition method is:
%
%      void XQueryPosition(Display *display,const Window window,int *x,int *y)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window.
%
%    o x: Return the x coordinate of the pointer relative to the origin of the
%      window.
%
%    o y: Return the y coordinate of the pointer relative to the origin of the
%      window.
%
*/
MagickPrivate void XQueryPosition(Display *display,const Window window,int *x,
  int *y)
{
  int
    x_root,
    y_root;

  unsigned int
    mask;

  Window
    root_window;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(window != (Window) NULL);
  assert(x != (int *) NULL);
  assert(y != (int *) NULL);
  (void) XQueryPointer(display,window,&root_window,&root_window,&x_root,&y_root,
    x,y,&mask);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X R e f r e s h W i n d o w                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XRefreshWindow() refreshes an image in a X window.
%
%  The format of the XRefreshWindow method is:
%
%      void XRefreshWindow(Display *display,const XWindowInfo *window,
%        const XEvent *event)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a XWindowInfo structure.
%
%    o event: Specifies a pointer to a XEvent structure.  If it is NULL,
%      the entire image is refreshed.
%
*/
MagickPrivate void XRefreshWindow(Display *display,const XWindowInfo *window,
  const XEvent *event)
{
  int
    x,
    y;

  unsigned int
    height,
    width;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(window != (XWindowInfo *) NULL);
  if (window->ximage == (XImage *) NULL)
    return;
  if (event != (XEvent *) NULL)
    {
      /*
        Determine geometry from expose event.
      */
      x=event->xexpose.x;
      y=event->xexpose.y;
      width=(unsigned int) event->xexpose.width;
      height=(unsigned int) event->xexpose.height;
    }
  else
    {
      XEvent
        sans_event;

      /*
        Refresh entire window; discard outstanding expose events.
      */
      x=0;
      y=0;
      width=window->width;
      height=window->height;
      while (XCheckTypedWindowEvent(display,window->id,Expose,&sans_event)) ;
      if (window->matte_pixmap != (Pixmap) NULL)
        {
#if defined(MAGICKCORE_HAVE_SHAPE)
          if (window->shape != MagickFalse)
            XShapeCombineMask(display,window->id,ShapeBounding,0,0,
              window->matte_pixmap,ShapeSet);
#endif
        }
    }
  /*
    Check boundary conditions.
  */
  if ((window->ximage->width-(x+window->x)) < (int) width)
    width=(unsigned int) (window->ximage->width-(x+window->x));
  if ((window->ximage->height-(y+window->y)) < (int) height)
    height=(unsigned int) (window->ximage->height-(y+window->y));
  /*
    Refresh image.
  */
  if (window->matte_pixmap != (Pixmap) NULL)
    (void) XSetClipMask(display,window->annotate_context,window->matte_pixmap);
  if (window->pixmap != (Pixmap) NULL)
    {
      if (window->depth > 1)
        (void) XCopyArea(display,window->pixmap,window->id,
          window->annotate_context,x+window->x,y+window->y,width,height,x,y);
      else
        (void) XCopyPlane(display,window->pixmap,window->id,
          window->highlight_context,x+window->x,y+window->y,width,height,x,y,
          1L);
    }
  else
    {
#if defined(MAGICKCORE_HAVE_SHARED_MEMORY)
      if (window->shared_memory)
        (void) XShmPutImage(display,window->id,window->annotate_context,
          window->ximage,x+window->x,y+window->y,x,y,width,height,MagickTrue);
#endif
      if (window->shared_memory == MagickFalse)
        (void) XPutImage(display,window->id,window->annotate_context,
          window->ximage,x+window->x,y+window->y,x,y,width,height);
    }
  if (window->matte_pixmap != (Pixmap) NULL)
    (void) XSetClipMask(display,window->annotate_context,None);
  (void) XFlush(display);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X R e m o t e C o m m a n d                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XRemoteCommand() forces a remote display(1) to display the specified
%  image filename.
%
%  The format of the XRemoteCommand method is:
%
%      MagickBooleanType XRemoteCommand(Display *display,const char *window,
%        const char *filename)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o window: Specifies the name or id of an X window.
%
%    o filename: the name of the image filename to display.
%
*/
MagickExport MagickBooleanType XRemoteCommand(Display *display,
  const char *window,const char *filename)
{
  Atom
    remote_atom;

  Window
    remote_window,
    root_window;

  assert(filename != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  if (display == (Display *) NULL)
    display=XOpenDisplay((char *) NULL);
  if (display == (Display *) NULL)
    {
      ThrowXWindowException(XServerError,"UnableToOpenXServer",filename);
      return(MagickFalse);
    }
  remote_atom=XInternAtom(display,"IM_PROTOCOLS",MagickFalse);
  remote_window=(Window) NULL;
  root_window=XRootWindow(display,XDefaultScreen(display));
  if (window != (char *) NULL)
    {
      /*
        Search window hierarchy and identify any clients by name or ID.
      */
      if (isdigit((int) ((unsigned char) *window)) != 0)
        remote_window=XWindowByID(display,root_window,(Window)
          strtol((char *) window,(char **) NULL,0));
      if (remote_window == (Window) NULL)
        remote_window=XWindowByName(display,root_window,window);
    }
  if (remote_window == (Window) NULL)
    remote_window=XWindowByProperty(display,root_window,remote_atom);
  if (remote_window == (Window) NULL)
    {
      ThrowXWindowException(XServerError,"UnableToConnectToRemoteDisplay",
        filename);
      return(MagickFalse);
    }
  /*
    Send remote command.
  */
  remote_atom=XInternAtom(display,"IM_REMOTE_COMMAND",MagickFalse);
  (void) XChangeProperty(display,remote_window,remote_atom,XA_STRING,8,
    PropModeReplace,(unsigned char *) filename,(int) strlen(filename));
  (void) XSync(display,MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X R e n d e r I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XRenderImage() renders text on the image with an X11 font.  It also returns
%  the bounding box of the text relative to the image.
%
%  The format of the XRenderImage method is:
%
%      MagickBooleanType XRenderImage(Image *image,DrawInfo *draw_info,
%        const PointInfo *offset,TypeMetric *metrics,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o draw_info: the draw info.
%
%    o offset: (x,y) location of text relative to image.
%
%    o metrics: bounding box of text.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate MagickBooleanType XRenderImage(Image *image,
  const DrawInfo *draw_info,const PointInfo *offset,TypeMetric *metrics,
  ExceptionInfo *exception)
{
  const char
    *client_name;

  DrawInfo
    cache_info;

  Display
    *display;

  ImageInfo
    *image_info;

  MagickBooleanType
    status;

  size_t
    height,
    width;

  XAnnotateInfo
    annotate_info;

  XFontStruct
    *font_info;

  XPixelInfo
    pixel;

  XResourceInfo
    resource_info;

  XrmDatabase
    resource_database;

  XStandardColormap
    *map_info;

  XVisualInfo
    *visual_info;

  /*
    Open X server connection.
  */
  display=XOpenDisplay(draw_info->server_name);
  if (display == (Display *) NULL)
    {
      ThrowXWindowException(XServerError,"UnableToOpenXServer",
        draw_info->server_name);
      return(MagickFalse);
    }
  /*
    Get user defaults from X resource database.
  */
  (void) XSetErrorHandler(XError);
  image_info=AcquireImageInfo();
  client_name=GetClientName();
  resource_database=XGetResourceDatabase(display,client_name);
  XGetResourceInfo(image_info,resource_database,client_name,&resource_info);
  resource_info.close_server=MagickFalse;
  resource_info.colormap=PrivateColormap;
  resource_info.font=AcquireString(draw_info->font);
  resource_info.background_color=AcquireString("#ffffffffffff");
  resource_info.foreground_color=AcquireString("#000000000000");
  map_info=XAllocStandardColormap();
  visual_info=(XVisualInfo *) NULL;
  font_info=(XFontStruct *) NULL;
  pixel.pixels=(unsigned long *) NULL;
  if (map_info == (XStandardColormap *) NULL)
    {
      ThrowXWindowException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
      return(MagickFalse);
    }
  /*
    Initialize visual info.
  */
  visual_info=XBestVisualInfo(display,map_info,&resource_info);
  if (visual_info == (XVisualInfo *) NULL)
    {
      XFreeResources(display,visual_info,map_info,&pixel,font_info,
        &resource_info,(XWindowInfo *) NULL);
      ThrowXWindowException(XServerError,"UnableToGetVisual",image->filename);
      return(MagickFalse);
    }
  map_info->colormap=(Colormap) NULL;
  /*
    Initialize Standard Colormap info.
  */
  XGetMapInfo(visual_info,XDefaultColormap(display,visual_info->screen),
    map_info);
  XGetPixelInfo(display,visual_info,map_info,&resource_info,(Image *) NULL,
    &pixel);
  pixel.annotate_context=XDefaultGC(display,visual_info->screen);
  /*
    Initialize font info.
  */
  font_info=XBestFont(display,&resource_info,MagickFalse);
  if (font_info == (XFontStruct *) NULL)
    {
      XFreeResources(display,visual_info,map_info,&pixel,font_info,
        &resource_info,(XWindowInfo *) NULL);
      ThrowXWindowException(XServerError,"UnableToLoadFont",draw_info->font);
      return(MagickFalse);
    }
  cache_info=(*draw_info);
  /*
    Initialize annotate info.
  */
  XGetAnnotateInfo(&annotate_info);
  annotate_info.stencil=ForegroundStencil;
  if (cache_info.font != draw_info->font)
    {
      /*
        Type name has changed.
      */
      (void) XFreeFont(display,font_info);
      (void) CloneString(&resource_info.font,draw_info->font);
      font_info=XBestFont(display,&resource_info,MagickFalse);
      if (font_info == (XFontStruct *) NULL)
        {
          ThrowXWindowException(XServerError,"UnableToLoadFont",
            draw_info->font);
          return(MagickFalse);
        }
    }
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(AnnotateEvent,GetMagickModule(),
      "Font %s; pointsize %g",draw_info->font != (char *) NULL ?
      draw_info->font : "none",draw_info->pointsize);
  cache_info=(*draw_info);
  annotate_info.font_info=font_info;
  annotate_info.text=(char *) draw_info->text;
  annotate_info.width=(unsigned int) XTextWidth(font_info,draw_info->text,(int)
    strlen(draw_info->text));
  annotate_info.height=(unsigned int) font_info->ascent+font_info->descent;
  metrics->pixels_per_em.x=(double) font_info->max_bounds.width;
  metrics->pixels_per_em.y=(double) font_info->ascent+font_info->descent;
  metrics->ascent=(double) font_info->ascent+4;
  metrics->descent=(double) (-font_info->descent);
  metrics->width=annotate_info.width/ExpandAffine(&draw_info->affine);
  metrics->height=(double) font_info->ascent+font_info->descent;
  metrics->max_advance=(double) font_info->max_bounds.width;
  metrics->bounds.x1=0.0;
  metrics->bounds.y1=metrics->descent;
  metrics->bounds.x2=metrics->ascent+metrics->descent;
  metrics->bounds.y2=metrics->ascent+metrics->descent;
  metrics->underline_position=(-2.0);
  metrics->underline_thickness=1.0;
  if (draw_info->render == MagickFalse)
    return(MagickTrue);
  if (draw_info->fill.alpha == TransparentAlpha)
    return(MagickTrue);
  /*
    Render fill color.
  */
  width=annotate_info.width;
  height=annotate_info.height;
  if ((fabs(draw_info->affine.rx) >= MagickEpsilon) ||
      (fabs(draw_info->affine.ry) >= MagickEpsilon))
    {
      if ((fabs(draw_info->affine.sx-draw_info->affine.sy) < MagickEpsilon) &&
          (fabs(draw_info->affine.rx+draw_info->affine.ry) < MagickEpsilon))
        annotate_info.degrees=(double) (180.0/MagickPI)*
          atan2(draw_info->affine.rx,draw_info->affine.sx);
    }
  (void) FormatLocaleString(annotate_info.geometry,MagickPathExtent,
    "%.20gx%.20g%+.20g%+.20g",(double) width,(double) height,
    ceil(offset->x-0.5),ceil(offset->y-metrics->ascent-metrics->descent+
    draw_info->interline_spacing-0.5));
  pixel.pen_color.red=ScaleQuantumToShort(
    ClampToQuantum(draw_info->fill.red));
  pixel.pen_color.green=ScaleQuantumToShort(
    ClampToQuantum(draw_info->fill.green));
  pixel.pen_color.blue=ScaleQuantumToShort(
    ClampToQuantum(draw_info->fill.blue));
  status=XAnnotateImage(display,&pixel,&annotate_info,image,exception);
  if (status == 0)
    {
      ThrowXWindowException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
      return(MagickFalse);
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X R e t a i n W i n d o w C o l o r s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XRetainWindowColors() sets X11 color resources on a window.  This preserves
%  the colors associated with an image displayed on the window.
%
%  The format of the XRetainWindowColors method is:
%
%      void XRetainWindowColors(Display *display,const Window window)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a XWindowInfo structure.
%
*/
MagickExport void XRetainWindowColors(Display *display,const Window window)
{
  Atom
    property;

  Pixmap
    pixmap;

  /*
    Put property on the window.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(window != (Window) NULL);
  property=XInternAtom(display,"_XSETROOT_ID",MagickFalse);
  if (property == (Atom) NULL)
    {
      ThrowXWindowException(XServerError,"UnableToCreateProperty",
        "_XSETROOT_ID");
      return;
    }
  pixmap=XCreatePixmap(display,window,1,1,1);
  if (pixmap == (Pixmap) NULL)
    {
      ThrowXWindowException(XServerError,"UnableToCreateBitmap","");
      return;
    }
  (void) XChangeProperty(display,window,property,XA_PIXMAP,32,PropModeReplace,
    (unsigned char *) &pixmap,1);
  (void) XSetCloseDownMode(display,RetainPermanent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X S e l e c t W i n d o w                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XSelectWindow() allows a user to select a window using the mouse.  If the
%  mouse moves, a cropping rectangle is drawn and the extents of the rectangle
%  is returned in the crop_info structure.
%
%  The format of the XSelectWindow function is:
%
%      target_window=XSelectWindow(display,crop_info)
%
%  A description of each parameter follows:
%
%    o window: XSelectWindow returns the window id.
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o crop_info: Specifies a pointer to a RectangleInfo structure.  It
%      contains the extents of any cropping rectangle.
%
*/
static Window XSelectWindow(Display *display,RectangleInfo *crop_info)
{
#define MinimumCropArea  (unsigned int) 9

  Cursor
    target_cursor;

  GC
    annotate_context;

  int
    presses,
    x_offset,
    y_offset;

  Status
    status;

  Window
    root_window,
    target_window;

  XEvent
    event;

  XGCValues
    context_values;

  /*
    Initialize graphic context.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(crop_info != (RectangleInfo *) NULL);
  root_window=XRootWindow(display,XDefaultScreen(display));
  context_values.background=XBlackPixel(display,XDefaultScreen(display));
  context_values.foreground=XWhitePixel(display,XDefaultScreen(display));
  context_values.function=GXinvert;
  context_values.plane_mask=
    context_values.background ^ context_values.foreground;
  context_values.subwindow_mode=IncludeInferiors;
  annotate_context=XCreateGC(display,root_window,(size_t) (GCBackground |
    GCForeground | GCFunction | GCSubwindowMode),&context_values);
  if (annotate_context == (GC) NULL)
    return(MagickFalse);
  /*
    Grab the pointer using target cursor.
  */
  target_cursor=XMakeCursor(display,root_window,XDefaultColormap(display,
    XDefaultScreen(display)),(char * ) "white",(char * ) "black");
  status=XGrabPointer(display,root_window,MagickFalse,(unsigned int)
    (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask),GrabModeSync,
    GrabModeAsync,root_window,target_cursor,CurrentTime);
  if (status != GrabSuccess)
    {
      ThrowXWindowException(XServerError,"UnableToGrabMouse","");
      return((Window) NULL);
    }
  /*
    Select a window.
  */
  crop_info->width=0;
  crop_info->height=0;
  presses=0;
  target_window=(Window) NULL;
  x_offset=0;
  y_offset=0;
  (void) XGrabServer(display);
  do
  {
    if ((crop_info->width*crop_info->height) >= MinimumCropArea)
      (void) XDrawRectangle(display,root_window,annotate_context,
        (int) crop_info->x,(int) crop_info->y,(unsigned int) crop_info->width-1,
        (unsigned int) crop_info->height-1);
    /*
      Allow another event.
    */
    (void) XAllowEvents(display,SyncPointer,CurrentTime);
    (void) XWindowEvent(display,root_window,ButtonPressMask |
      ButtonReleaseMask | ButtonMotionMask,&event);
    if ((crop_info->width*crop_info->height) >= MinimumCropArea)
      (void) XDrawRectangle(display,root_window,annotate_context,
        (int) crop_info->x,(int) crop_info->y,(unsigned int) crop_info->width-1,
        (unsigned int) crop_info->height-1);
    switch (event.type)
    {
      case ButtonPress:
      {
        target_window=XGetSubwindow(display,event.xbutton.subwindow,
          event.xbutton.x,event.xbutton.y);
        if (target_window == (Window) NULL)
          target_window=root_window;
        x_offset=event.xbutton.x_root;
        y_offset=event.xbutton.y_root;
        crop_info->x=(ssize_t) x_offset;
        crop_info->y=(ssize_t) y_offset;
        crop_info->width=0;
        crop_info->height=0;
        presses++;
        break;
      }
      case ButtonRelease:
      {
        presses--;
        break;
      }
      case MotionNotify:
      {
        /*
          Discard pending button motion events.
        */
        while (XCheckMaskEvent(display,ButtonMotionMask,&event)) ;
        crop_info->x=(ssize_t) event.xmotion.x;
        crop_info->y=(ssize_t) event.xmotion.y;
        /*
          Check boundary conditions.
        */
        if ((int) crop_info->x < x_offset)
          crop_info->width=(size_t) (x_offset-crop_info->x);
        else
          {
            crop_info->width=(size_t) (crop_info->x-x_offset);
            crop_info->x=(ssize_t) x_offset;
          }
        if ((int) crop_info->y < y_offset)
          crop_info->height=(size_t) (y_offset-crop_info->y);
        else
          {
            crop_info->height=(size_t) (crop_info->y-y_offset);
            crop_info->y=(ssize_t) y_offset;
          }
      }
      default:
        break;
    }
  } while ((target_window == (Window) NULL) || (presses > 0));
  (void) XUngrabServer(display);
  (void) XUngrabPointer(display,CurrentTime);
  (void) XFreeCursor(display,target_cursor);
  (void) XFreeGC(display,annotate_context);
  if ((crop_info->width*crop_info->height) < MinimumCropArea)
    {
      crop_info->width=0;
      crop_info->height=0;
    }
  if ((crop_info->width != 0) && (crop_info->height != 0))
    target_window=root_window;
  return(target_window);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X S e t C u r s o r S t a t e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XSetCursorState() sets the cursor state to busy, otherwise the cursor are
%  reset to their default.
%
%  The format of the XXSetCursorState method is:
%
%      XSetCursorState(display,windows,const MagickStatusType state)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o state: An unsigned integer greater than 0 sets the cursor state
%      to busy, otherwise the cursor are reset to their default.
%
*/
MagickPrivate void XSetCursorState(Display *display,XWindows *windows,
  const MagickStatusType state)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(windows != (XWindows *) NULL);
  if (state)
    {
      (void) XCheckDefineCursor(display,windows->image.id,
        windows->image.busy_cursor);
      (void) XCheckDefineCursor(display,windows->pan.id,
        windows->pan.busy_cursor);
      (void) XCheckDefineCursor(display,windows->magnify.id,
        windows->magnify.busy_cursor);
      (void) XCheckDefineCursor(display,windows->command.id,
        windows->command.busy_cursor);
    }
  else
    {
      (void) XCheckDefineCursor(display,windows->image.id,
        windows->image.cursor);
      (void) XCheckDefineCursor(display,windows->pan.id,windows->pan.cursor);
      (void) XCheckDefineCursor(display,windows->magnify.id,
        windows->magnify.cursor);
      (void) XCheckDefineCursor(display,windows->command.id,
        windows->command.cursor);
      (void) XCheckDefineCursor(display,windows->command.id,
        windows->widget.cursor);
      (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
    }
  windows->info.mapped=MagickFalse;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X S e t W i n d o w s                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XSetWindows() sets the X windows structure if the windows info is specified.
%  Otherwise the current windows structure is returned.
%
%  The format of the XSetWindows method is:
%
%      XWindows *XSetWindows(XWindows *windows_info)
%
%  A description of each parameter follows:
%
%    o windows_info: Initialize the Windows structure with this information.
%
*/
MagickPrivate XWindows *XSetWindows(XWindows *windows_info)
{
  static XWindows
    *windows = (XWindows *) NULL;

  if (windows_info != (XWindows *) ~0)
    {
      windows=(XWindows *) RelinquishMagickMemory(windows);
      windows=windows_info;
    }
  return(windows);
}
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X U s e r P r e f e r e n c e s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XUserPreferences() saves the preferences in a configuration file in the
%  users' home directory.
%
%  The format of the XUserPreferences method is:
%
%      void XUserPreferences(XResourceInfo *resource_info)
%
%  A description of each parameter follows:
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
*/
MagickPrivate void XUserPreferences(XResourceInfo *resource_info)
{
#if defined(X11_PREFERENCES_PATH)
  char
    cache[MagickPathExtent],
    filename[MagickPathExtent],
    specifier[MagickPathExtent];

  const char
    *client_name,
    *value;

  XrmDatabase
    preferences_database;

  /*
    Save user preferences to the client configuration file.
  */
  assert(resource_info != (XResourceInfo *) NULL);
  client_name=GetClientName();
  preferences_database=XrmGetStringDatabase("");
  (void) FormatLocaleString(specifier,MagickPathExtent,"%s.backdrop",client_name);
  value=resource_info->backdrop ? "True" : "False";
  XrmPutStringResource(&preferences_database,specifier,(char *) value);
  (void) FormatLocaleString(specifier,MagickPathExtent,"%s.colormap",client_name);
  value=resource_info->colormap == SharedColormap ? "Shared" : "Private";
  XrmPutStringResource(&preferences_database,specifier,(char *) value);
  (void) FormatLocaleString(specifier,MagickPathExtent,"%s.confirmExit",
    client_name);
  value=resource_info->confirm_exit ? "True" : "False";
  XrmPutStringResource(&preferences_database,specifier,(char *) value);
  (void) FormatLocaleString(specifier,MagickPathExtent,"%s.confirmEdit",
    client_name);
  value=resource_info->confirm_edit ? "True" : "False";
  XrmPutStringResource(&preferences_database,specifier,(char *) value);
  (void) FormatLocaleString(specifier,MagickPathExtent,"%s.displayWarnings",
    client_name);
  value=resource_info->display_warnings ? "True" : "False";
  XrmPutStringResource(&preferences_database,specifier,(char *) value);
  (void) FormatLocaleString(specifier,MagickPathExtent,"%s.dither",client_name);
  value=resource_info->quantize_info->dither_method != NoDitherMethod ?
    "True" : "False";
  XrmPutStringResource(&preferences_database,specifier,(char *) value);
  (void) FormatLocaleString(specifier,MagickPathExtent,"%s.gammaCorrect",
    client_name);
  value=resource_info->gamma_correct ? "True" : "False";
  XrmPutStringResource(&preferences_database,specifier,(char *) value);
  (void) FormatLocaleString(specifier,MagickPathExtent,"%s.undoCache",client_name);
  (void) FormatLocaleString(cache,MagickPathExtent,"%.20g",(double)
    resource_info->undo_cache);
  XrmPutStringResource(&preferences_database,specifier,cache);
  (void) FormatLocaleString(specifier,MagickPathExtent,"%s.usePixmap",client_name);
  value=resource_info->use_pixmap ? "True" : "False";
  XrmPutStringResource(&preferences_database,specifier,(char *) value);
  (void) FormatLocaleString(filename,MagickPathExtent,"%s%src",
    X11_PREFERENCES_PATH,client_name);
  ExpandFilename(filename);
  XrmPutFileDatabase(preferences_database,filename);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X V i s u a l C l a s s N a m e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XVisualClassName() returns the visual class name as a character string.
%
%  The format of the XVisualClassName method is:
%
%      char *XVisualClassName(const int visual_class)
%
%  A description of each parameter follows:
%
%    o visual_type: XVisualClassName returns the visual class as a character
%      string.
%
%    o class: Specifies the visual class.
%
*/
static const char *XVisualClassName(const int visual_class)
{
  switch (visual_class)
  {
    case StaticGray: return("StaticGray");
    case GrayScale: return("GrayScale");
    case StaticColor: return("StaticColor");
    case PseudoColor: return("PseudoColor");
    case TrueColor: return("TrueColor");
    case DirectColor: return("DirectColor");
  }
  return("unknown visual class");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X W a r n i n g                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XWarning() displays a warning reason in a Notice widget.
%
%  The format of the XWarning method is:
%
%      void XWarning(const unsigned int warning,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o warning: Specifies the numeric warning category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
*/
MagickPrivate void XWarning(const ExceptionType magick_unused(warning),
  const char *reason,const char *description)
{
  char
    text[MagickPathExtent];

  XWindows
    *windows;

  if (reason == (char *) NULL)
    return;
  (void) CopyMagickString(text,reason,MagickPathExtent);
  (void) ConcatenateMagickString(text,":",MagickPathExtent);
  windows=XSetWindows((XWindows *) ~0);
  XNoticeWidget(windows->display,windows,text,(char *) description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X W i n d o w B y I D                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XWindowByID() locates a child window with a given ID.  If not window with
%  the given name is found, 0 is returned.   Only the window specified and its
%  subwindows are searched.
%
%  The format of the XWindowByID function is:
%
%      child=XWindowByID(display,window,id)
%
%  A description of each parameter follows:
%
%    o child: XWindowByID returns the window with the specified
%      id.  If no windows are found, XWindowByID returns 0.
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o id: Specifies the id of the window to locate.
%
*/
MagickPrivate Window XWindowByID(Display *display,const Window root_window,
  const size_t id)
{
  RectangleInfo
    rectangle_info;

  register int
    i;

  Status
    status;

  unsigned int
    number_children;

  Window
    child,
    *children,
    window;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(root_window != (Window) NULL);
  if (id == 0)
    return(XSelectWindow(display,&rectangle_info));
  if (root_window == id)
    return(root_window);
  status=XQueryTree(display,root_window,&child,&child,&children,
    &number_children);
  if (status == False)
    return((Window) NULL);
  window=(Window) NULL;
  for (i=0; i < (int) number_children; i++)
  {
    /*
      Search each child and their children.
    */
    window=XWindowByID(display,children[i],id);
    if (window != (Window) NULL)
      break;
  }
  if (children != (Window *) NULL)
    (void) XFree((void *) children);
  return(window);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X W i n d o w B y N a m e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XWindowByName() locates a window with a given name on a display.  If no
%  window with the given name is found, 0 is returned. If more than one window
%  has the given name, the first one is returned.  Only root and its children
%  are searched.
%
%  The format of the XWindowByName function is:
%
%      window=XWindowByName(display,root_window,name)
%
%  A description of each parameter follows:
%
%    o window: XWindowByName returns the window id.
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o root_window: Specifies the id of the root window.
%
%    o name: Specifies the name of the window to locate.
%
*/
MagickPrivate Window XWindowByName(Display *display,const Window root_window,
  const char *name)
{
  register int
    i;

  Status
    status;

  unsigned int
    number_children;

  Window
    *children,
    child,
    window;

  XTextProperty
    window_name;

  assert(display != (Display *) NULL);
  assert(root_window != (Window) NULL);
  assert(name != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",name);
  if (XGetWMName(display,root_window,&window_name) != 0)
    if (LocaleCompare((char *) window_name.value,name) == 0)
      return(root_window);
  status=XQueryTree(display,root_window,&child,&child,&children,
    &number_children);
  if (status == False)
    return((Window) NULL);
  window=(Window) NULL;
  for (i=0; i < (int) number_children; i++)
  {
    /*
      Search each child and their children.
    */
    window=XWindowByName(display,children[i],name);
    if (window != (Window) NULL)
      break;
  }
  if (children != (Window *) NULL)
    (void) XFree((void *) children);
  return(window);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X W i n d o w B y P r o p e r y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XWindowByProperty() locates a child window with a given property. If not
%  window with the given name is found, 0 is returned.  If more than one window
%  has the given property, the first one is returned.  Only the window
%  specified and its subwindows are searched.
%
%  The format of the XWindowByProperty function is:
%
%      child=XWindowByProperty(display,window,property)
%
%  A description of each parameter follows:
%
%    o child: XWindowByProperty returns the window id with the specified
%      property.  If no windows are found, XWindowByProperty returns 0.
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o property: Specifies the property of the window to locate.
%
*/
MagickPrivate Window XWindowByProperty(Display *display,const Window window,
  const Atom property)
{
  Atom
    type;

  int
    format;

  Status
    status;

  unsigned char
    *data;

  unsigned int
    i,
    number_children;

  unsigned long
    after,
    number_items;

  Window
    child,
    *children,
    parent,
    root;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(display != (Display *) NULL);
  assert(window != (Window) NULL);
  assert(property != (Atom) NULL);
  status=XQueryTree(display,window,&root,&parent,&children,&number_children);
  if (status == False)
    return((Window) NULL);
  type=(Atom) NULL;
  child=(Window) NULL;
  for (i=0; (i < number_children) && (child == (Window) NULL); i++)
  {
    status=XGetWindowProperty(display,children[i],property,0L,0L,MagickFalse,
      (Atom) AnyPropertyType,&type,&format,&number_items,&after,&data);
    if (data != NULL)
      (void) XFree((void *) data);
    if ((status == Success) && (type != (Atom) NULL))
      child=children[i];
  }
  for (i=0; (i < number_children) && (child == (Window) NULL); i++)
    child=XWindowByProperty(display,children[i],property);
  if (children != (Window *) NULL)
    (void) XFree((void *) children);
  return(child);
}
#else

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X I m p o r t I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XImportImage() reads an image from an X window.
%
%  The format of the XImportImage method is:
%
%      Image *XImportImage(const ImageInfo *image_info,XImportInfo *ximage_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info..
%
%    o ximage_info: Specifies a pointer to an XImportInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *XImportImage(const ImageInfo *image_info,
  XImportInfo *ximage_info,ExceptionInfo *exception)
{
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(ximage_info != (XImportInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  (void) ximage_info;
  (void) exception;
  return((Image *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X R e n d e r X 1 1                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XRenderImage() renders text on the image with an X11 font.  It also returns
%  the bounding box of the text relative to the image.
%
%  The format of the XRenderImage method is:
%
%      MagickBooleanType XRenderImage(Image *image,DrawInfo *draw_info,
%        const PointInfo *offset,TypeMetric *metrics,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o draw_info: the draw info.
%
%    o offset: (x,y) location of text relative to image.
%
%    o metrics: bounding box of text.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate MagickBooleanType XRenderImage(Image *image,
  const DrawInfo *draw_info,const PointInfo *offset,TypeMetric *metrics,
  ExceptionInfo *exception)
{
  (void) draw_info;
  (void) offset;
  (void) metrics;
  (void) ThrowMagickException(exception,GetMagickModule(),
    MissingDelegateError,"DelegateLibrarySupportNotBuiltIn","'%s' (X11)",
    image->filename);
  return(MagickFalse);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X C o m p o n e n t G e n e s i s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XComponentGenesis() instantiates the X component.
%
%  The format of the XComponentGenesis method is:
%
%      MagickBooleanType XComponentGenesis(void)
%
*/
MagickPrivate MagickBooleanType XComponentGenesis(void)
{
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t I m p o r t I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XGetImportInfo() initializes the XImportInfo structure.
%
%  The format of the XGetImportInfo method is:
%
%      void XGetImportInfo(XImportInfo *ximage_info)
%
%  A description of each parameter follows:
%
%    o ximage_info: Specifies a pointer to an ImageInfo structure.
%
*/
MagickExport void XGetImportInfo(XImportInfo *ximage_info)
{
  assert(ximage_info != (XImportInfo *) NULL);
  ximage_info->frame=MagickFalse;
  ximage_info->borders=MagickFalse;
  ximage_info->screen=MagickFalse;
  ximage_info->descend=MagickTrue;
  ximage_info->silent=MagickFalse;
}

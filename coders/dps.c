/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            DDDD   PPPP   SSSSS                              %
%                            D   D  P   P  SS                                 %
%                            D   D  PPPP    SSS                               %
%                            D   D  P         SS                              %
%                            DDDD   P      SSSSS                              %
%                                                                             %
%                                                                             %
%            Read Postscript Using the Display Postscript System.             %
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
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/client.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/utility.h"
#include "magick/xwindow-private.h"
#if defined(MAGICKCORE_DPS_DELEGATE)
#include <DPS/dpsXclient.h>
#include <DPS/dpsXpreview.h>
#endif

#if defined(MAGICKCORE_DPS_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d D P S I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDPSImage() reads a Adobe Postscript image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadDPSImage method is:
%
%      Image *ReadDPSImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline double MagickMin(const double x,const double y)
{
  if (x < y)
    return(x);
  return(y);
}

static Image *ReadDPSImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Display
    *display;

  float
    pixels_per_point;

  Image
    *image;

  int
    sans,
    status;

  long
    x,
    y;

  Pixmap
    pixmap;

  register IndexPacket
    *indexes;

  register long
    i;

  register PixelPacket
    *q;

  register unsigned long
    pixel;

  Screen
    *screen;

  XColor
    *colors;

  XImage
    *dps_image;

  XRectangle
    page,
    bits_per_pixel;

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
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  display=XOpenDisplay(image_info->server_name);
  if (display == (Display *) NULL)
    return((Image *) NULL);
  /*
    Set our forgiving exception handler.
  */
  (void) XSetErrorHandler(XError);
  /*
    Open image file.
  */
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    return((Image *) NULL);
  /*
    Get user defaults from X resource database.
  */
  resource_database=XGetResourceDatabase(display,GetClientName());
  XGetResourceInfo(image_info,resource_database,GetClientName(),&resource_info);
  /*
    Allocate standard colormap.
  */
  map_info=XAllocStandardColormap();
  visual_info=(XVisualInfo *) NULL;
  if (map_info == (XStandardColormap *) NULL)
    ThrowReaderException(ResourceLimitError,"UnableToCreateStandardColormap")
  else
    {
      /*
        Initialize visual info.
      */
      (void) CloneString(&resource_info.visual_type,"default");
      visual_info=XBestVisualInfo(display,map_info,&resource_info);
      map_info->colormap=(Colormap) NULL;
    }
  if ((map_info == (XStandardColormap *) NULL) ||
      (visual_info == (XVisualInfo *) NULL))
    {
      image=DestroyImage(image);
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      return((Image *) NULL);
    }
  /*
    Create a pixmap the appropriate size for the image.
  */
  screen=ScreenOfDisplay(display,visual_info->screen);
  pixels_per_point=XDPSPixelsPerPoint(screen);
  if ((image->x_resolution != 0.0) && (image->y_resolution != 0.0))
    pixels_per_point=MagickMin(image->x_resolution,image->y_resolution)/
      DefaultResolution;
  status=XDPSCreatePixmapForEPSF((DPSContext) NULL,screen,
    GetBlobFileHandle(image),visual_info->depth,pixels_per_point,&pixmap,
    &bits_per_pixel,&page);
  if ((status == dps_status_failure) || (status == dps_status_no_extension))
    {
      image=DestroyImage(image);
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      return((Image *) NULL);
    }
  /*
    Rasterize the file into the pixmap.
  */
  status=XDPSImageFileIntoDrawable((DPSContext) NULL,screen,pixmap,
    GetBlobFileHandle(image),(int) bits_per_pixel.height,visual_info->depth,
    &page,-page.x,-page.y,pixels_per_point,MagickTrue,MagickFalse,MagickTrue,
    &sans);
  if (status != dps_status_success)
    {
      image=DestroyImage(image);
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      return((Image *) NULL);
    }
  /*
    Initialize DPS X image.
  */
  dps_image=XGetImage(display,pixmap,0,0,bits_per_pixel.width,
    bits_per_pixel.height,AllPlanes,ZPixmap);
  (void) XFreePixmap(display,pixmap);
  if (dps_image == (XImage *) NULL)
    {
      image=DestroyImage(image);
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      return((Image *) NULL);
    }
  /*
    Get the colormap colors.
  */
  colors=(XColor *) AcquireQuantumMemory(visual_info->colormap_size,
    sizeof(*colors));
  if (colors == (XColor *) NULL)
    {
      image=DestroyImage(image);
      XDestroyImage(dps_image);
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      return((Image *) NULL);
    }
  if ((visual_info->klass != DirectColor) && (visual_info->klass != TrueColor))
    for (i=0; i < visual_info->colormap_size; i++)
    {
      colors[i].pixel=(unsigned long) i;
      colors[i].pad=0;
    }
  else
    {
      unsigned long
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
      red_bit=visual_info->red_mask & (~(visual_info->red_mask)+1);
      green_bit=visual_info->green_mask & (~(visual_info->green_mask)+1);
      blue_bit=visual_info->blue_mask & (~(visual_info->blue_mask)+1);
      for (i=0; i < visual_info->colormap_size; i++)
      {
        colors[i].pixel=red | green | blue;
        colors[i].pad=0;
        red+=red_bit;
        if (red > visual_info->red_mask)
          red=0;
        green+=green_bit;
        if (green > visual_info->green_mask)
          green=0;
        blue+=blue_bit;
        if (blue > visual_info->blue_mask)
          blue=0;
      }
    }
  (void) XQueryColors(display,XDefaultColormap(display,visual_info->screen),
    colors,visual_info->colormap_size);
  /*
    Convert X image to MIFF format.
  */
  if ((visual_info->klass != TrueColor) && (visual_info->klass != DirectColor))
    image->storage_class=PseudoClass;
  image->columns=(unsigned long) dps_image->width;
  image->rows=(unsigned long) dps_image->height;
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  switch (image->storage_class)
  {
    case DirectClass:
    default:
    {
      register unsigned long
        color,
        index;

      unsigned long
        blue_mask,
        blue_shift,
        green_mask,
        green_shift,
        red_mask,
        red_shift;

      /*
        Determine shift and mask for red, green, and blue.
      */
      red_mask=visual_info->red_mask;
      red_shift=0;
      while ((red_mask != 0) && ((red_mask & 0x01) == 0))
      {
        red_mask>>=1;
        red_shift++;
      }
      green_mask=visual_info->green_mask;
      green_shift=0;
      while ((green_mask != 0) && ((green_mask & 0x01) == 0))
      {
        green_mask>>=1;
        green_shift++;
      }
      blue_mask=visual_info->blue_mask;
      blue_shift=0;
      while ((blue_mask != 0) && ((blue_mask & 0x01) == 0))
      {
        blue_mask>>=1;
        blue_shift++;
      }
      /*
        Convert X image to DirectClass packets.
      */
      if ((visual_info->colormap_size > 0) &&
          (visual_info->klass == DirectColor))
        for (y=0; y < (long) image->rows; y++)
        {
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < (long) image->columns; x++)
          {
            pixel=XGetPixel(dps_image,x,y);
            index=(pixel >> red_shift) & red_mask;
            q->red=ScaleShortToQuantum(colors[index].red);
            index=(pixel >> green_shift) & green_mask;
            q->green=ScaleShortToQuantum(colors[index].green);
            index=(pixel >> blue_shift) & blue_mask;
            q->blue=ScaleShortToQuantum(colors[index].blue);
            q++;
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
            break;
        }
      else
        for (y=0; y < (long) image->rows; y++)
        {
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < (long) image->columns; x++)
          {
            pixel=XGetPixel(dps_image,x,y);
            color=(pixel >> red_shift) & red_mask;
            color=(color*65535L)/red_mask;
            q->red=ScaleShortToQuantum((unsigned short) color);
            color=(pixel >> green_shift) & green_mask;
            color=(color*65535L)/green_mask;
            q->green=ScaleShortToQuantum((unsigned short) color);
            color=(pixel >> blue_shift) & blue_mask;
            color=(color*65535L)/blue_mask;
            q->blue=ScaleShortToQuantum((unsigned short) color);
            q++;
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
            break;
        }
      break;
    }
    case PseudoClass:
    {
      /*
        Create colormap.
      */
      if (AcquireImageColormap(image,(unsigned long) visual_info->colormap_size) == MagickFalse)
        {
          image=DestroyImage(image);
          colors=(XColor *) RelinquishMagickMemory(colors);
          XDestroyImage(dps_image);
          XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
            (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
          return((Image *) NULL);
        }
      for (i=0; i < (long) image->colors; i++)
      {
        image->colormap[colors[i].pixel].red=ScaleShortToQuantum(colors[i].red);
        image->colormap[colors[i].pixel].green=
          ScaleShortToQuantum(colors[i].green);
        image->colormap[colors[i].pixel].blue=
          ScaleShortToQuantum(colors[i].blue);
      }
      /*
        Convert X image to PseudoClass packets.
      */
      for (y=0; y < (long) image->rows; y++)
      {
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        indexes=GetAuthenticIndexQueue(image);
        for (x=0; x < (long) image->columns; x++)
          indexes[x]=(unsigned short) XGetPixel(dps_image,x,y);
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
          break;
      }
      break;
    }
  }
  colors=(XColor *) RelinquishMagickMemory(colors);
  XDestroyImage(dps_image);
  if (image->storage_class == PseudoClass)
    (void) SyncImage(image);
  /*
    Rasterize matte image.
  */
  status=XDPSCreatePixmapForEPSF((DPSContext) NULL,screen,
    GetBlobFileHandle(image),1,pixels_per_point,&pixmap,&bits_per_pixel,&page);
  if ((status != dps_status_failure) && (status != dps_status_no_extension))
    {
      status=XDPSImageFileIntoDrawable((DPSContext) NULL,screen,pixmap,
        GetBlobFileHandle(image),(int) bits_per_pixel.height,1,&page,-page.x,
        -page.y,pixels_per_point,MagickTrue,MagickTrue,MagickTrue,&sans);
      if (status == dps_status_success)
        {
          XImage
            *matte_image;

          /*
            Initialize image matte.
          */
          matte_image=XGetImage(display,pixmap,0,0,bits_per_pixel.width,
            bits_per_pixel.height,AllPlanes,ZPixmap);
          (void) XFreePixmap(display,pixmap);
          if (matte_image != (XImage *) NULL)
            {
              image->storage_class=DirectClass;
              image->matte=MagickTrue;
              for (y=0; y < (long) image->rows; y++)
              {
                q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
                if (q == (PixelPacket *) NULL)
                  break;
                for (x=0; x < (long) image->columns; x++)
                {
                  q->opacity=OpaqueOpacity;
                  if (XGetPixel(matte_image,x,y) == 0)
                    q->opacity=TransparentOpacity;
                  q++;
                }
                if (SyncAuthenticPixels(image,exception) == MagickFalse)
                  break;
              }
              XDestroyImage(matte_image);
            }
        }
    }
  /*
    Relinquish resources.
  */
  XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
    (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r D P S I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterDPSImage() adds attributes for the Display Postscript image
%  format to the list of supported formats.  The attributes include the image
%  format tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterDPSImage method is:
%
%      unsigned long RegisterDPSImage(void)
%
*/
ModuleExport unsigned long RegisterDPSImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("DPS");
#if defined(MAGICKCORE_DPS_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadDPSImage;
#endif
  entry->blob_support=MagickFalse;
  entry->description=ConstantString("Display Postscript Interpreter");
  entry->module=ConstantString("DPS");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r D P S I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterDPSImage() removes format registrations made by the
%  DPS module from the list of supported formats.
%
%  The format of the UnregisterDPSImage method is:
%
%      UnregisterDPSImage(void)
%
*/
ModuleExport void UnregisterDPSImage(void)
{
  (void) UnregisterMagickInfo("DPS");
}

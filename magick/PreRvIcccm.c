/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%                      IIIII   CCCC   CCCC   CCCC  M   M                      %
%                        I    C      C      C      MM MM                      %
%                        I    C      C      C      M M M                      %
%                        I    C      C      C      M   M                      %
%                      IIIII   CCCC   CCCC   CCCC  M   M                      %
%                                                                             %
%                     MagickCore X11 Compatibility Methods                    %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                December 1994                                %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
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

#include "magick/studio.h"
#if defined(MAGICKCORE_X11_DELEGATE)
#include "magick/xwindow-private.h"

#if defined(PRE_R6_ICCCM)
/*
  Compatibility methods for pre X11R6 ICCCM.
*/
Status XInitImage(XImage *ximage)
{
  Display
    display;

  ScreenFormat
    screen_format;

  XImage
    *created_ximage,
    target_ximage;

  /*
    Initialize the X image.
  */
  screen_format.depth=ximage->depth;
  screen_format.bits_per_pixel=(int) ximage->bits_per_pixel;
  display.byte_order=ximage->byte_order;
  display.bitmap_unit=ximage->bitmap_unit;
  display.bitmap_bit_order=ximage->bitmap_bit_order;
  display.pixmap_format=(&screen_format);
  display.nformats=1;
  created_ximage=XCreateImage(&display,(Visual *) NULL,ximage->depth,
    ximage->format,ximage->xoffset,(char *) NULL,ximage->width,ximage->height,
    ximage->bitmap_pad,ximage->bytes_per_line);
  if (created_ximage == (XImage *) NULL)
    return(0);
  target_ximage=(*ximage);
  *ximage=(*created_ximage);
  created_ximage->data=(char *) NULL;
  XDestroyImage(created_ximage);
  ximage->red_mask=target_ximage.red_mask;
  ximage->green_mask=target_ximage.green_mask;
  ximage->blue_mask=target_ximage.blue_mask;
  return(1);
}
#endif

#if defined(PRE_R5_ICCCM)
/*
  Compatibility methods for pre X11R5 ICCCM.
*/
void XrmCombineDatabase(XrmDatabase source,XrmDatabase *target,
  Bool override)
{
  XrmMergeDatabases(source,target);
}

Status XrmCombineFileDatabase(const char *filename,XrmDatabase *target,
  Bool override)
{
  XrmDatabase
    *combined_database,
    source;

  source=XrmGetFileDatabase(filename);
  if (override == MagickFalse)
    XrmMergeDatabases(source,target);
  return(1);
}

XrmDatabase XrmGetDatabase(Display *display)
{
  return(display->db);
}

char *XSetLocaleModifiers(char *modifiers)
{
  return((char *) NULL);
}

Bool XSupportsLocale()
{
  return(0);
}
#endif

#if defined(PRE_R4_ICCCM)
/*
  Compatibility methods for pre X11R4 ICCCM.
*/
XClassHint *XAllocClassHint)
{
  return((XClassHint *) AcquireMagickMemory(sizeof(XClassHint)));
}

XIconSize *XAllocIconSize)
{
  return((XIconSize *) AcquireMagickMemory(sizeof(XIconSize)));
}

XSizeHints *XAllocSizeHints)
{
  return((XSizeHints *) AcquireMagickMemory(sizeof(XSizeHints)));
}

Status XReconfigureWMWindow(Display *display,Window window,int screen_number,
  unsigned int value_mask,XWindowChanges *values)
{
  return(XConfigureWindow(display,window,value_mask,values));
}

XStandardColormap *XAllocStandardColormap)
{
  return((XStandardColormap *) AcquireMagickMemory(sizeof(XStandardColormap)));
}

XWMHints *XAllocWMHints)
{
  return((XWMHints *) AcquireMagickMemory(sizeof(XWMHints)));
}

Status XGetGCValues(Display *display,GC gc,size_t mask,
  XGCValues *values)
{
  return(MagickTrue);
}

Status XGetRGBColormaps(Display *display,Window window,
  XStandardColormap **colormap,int *count,Atom property)
{
  *count=1;
  return(XGetStandardColormap(display,window,*colormap,property));
}

Status XGetWMColormapWindows(Display *display,Window window,
  Window **colormap_windows,int *number_windows)
{
  Atom
    actual_type,
    *data,
    property;

  int
    actual_format,
    status;

  size_t
    leftover,
    number_items;

  property=XInternAtom(display,"WM_COLORMAP_WINDOWS",MagickFalse);
  if (property == None)
    return(MagickFalse);
  /*
    Get the window property.
  */
  *data=(Atom) NULL;
  status=XGetWindowProperty(display,window,property,0L,1000000L,MagickFalse,
    XA_WINDOW,&actual_type,&actual_format,&number_items,&leftover,
    (unsigned char **) &data);
  if (status != Success)
    return(MagickFalse);
  if ((actual_type != XA_WINDOW) || (actual_format != 32))
    {
      if (data != (Atom *) NULL)
        XFree((char *) data);
      return(MagickFalse);
    }
  *colormap_windows=(Window *) data;
  *number_windows=(int) number_items;
  return(MagickTrue);
}

Status XGetWMName(Display *display,Window window,XTextProperty *text_property)
{
  char
    *window_name;

  if (XFetchName(display,window,&window_name) == 0)
    return(MagickFalse);
  text_property->value=(unsigned char *) window_name;
  text_property->encoding=XA_STRING;
  text_property->format=8;
  text_property->nitems=strlen(window_name);
  return(MagickTrue);
}

char *XResourceManagerString(Display *display)
{
  return(display->xdefaults);
}

void XrmDestroyDatabase(XrmDatabase database)
{
}

void XSetWMIconName(Display *display,Window window,XTextProperty *property)
{
  XSetIconName(display,window,property->value);
}

void XSetWMName(Display *display,Window window,XTextProperty *property)
{
  XStoreName(display,window,property->value);
}

void XSetWMProperties(Display *display,Window window,
  XTextProperty *window_name,XTextProperty *icon_name,char **argv,
  int argc,XSizeHints *size_hints,XWMHints *manager_hints,
  XClassHint *class_hint)
{
  XSetStandardProperties(display,window,window_name->value,icon_name->value,
    None,argv,argc,size_hints);
  XSetWMHints(display,window,manager_hints);
  XSetClassHint(display,window,class_hint);
}

Status XSetWMProtocols(Display *display,Window window,Atom *protocols,
  int count)
{
  Atom
    wm_protocols;

  wm_protocols=XInternAtom(display,"WM_PROTOCOLS",MagickFalse);
  XChangeProperty(display,window,wm_protocols,XA_ATOM,32,PropModeReplace,
    (unsigned char *) protocols,count);
  return(MagickTrue);
}

int XStringListToTextProperty(char **argv,int argc,XTextProperty *property)
{
  register int
    i;

  register unsigned int
    number_bytes;

  XTextProperty
     protocol;

  number_bytes=0;
  for (i=0; i < (ssize_t) argc; i++)
    number_bytes+=(unsigned int) ((argv[i] ? strlen(argv[i]) : 0)+1);
  protocol.encoding=XA_STRING;
  protocol.format=8;
  protocol.nitems=0;
  if (number_bytes)
    protocol.nitems=number_bytes-1;
  protocol.value=NULL;
  if (number_bytes <= 0)
    {
      protocol.value=(unsigned char *) AcquireQuantumMemory(1UL,
        sizeof(*protocol.value));
      if (protocol.value == MagickFalse)
        return(MagickFalse);
      *protocol.value='\0';
    }
  else
    {
      register char
        *buffer;

      buffer=(char *) AcquireQuantumMemory(number_bytes,sizeof(*buffer));
      if (buffer == (char *) NULL)
        return(MagickFalse);
      protocol.value=(unsigned char *) buffer;
      for (i=0; i < (ssize_t) argc; i++)
      {
        char
          *argument;

        argument=argv[i];
        if (argument == MagickFalse)
          *buffer++='\0';
        else
          {
            (void) CopyMagickString(buffer,argument,MaxTextExtent);
            buffer+=(strlen(argument)+1);
          }
      }
    }
  *property=protocol;
  return(MagickTrue);
}

VisualID XVisualIDFromVisual(Visual *visual)
{
  return(visual->visualid);
}

Status XWithdrawWindow(Display *display,Window window,int screen)
{
  return(XUnmapWindow(display,window));
}

int XWMGeometry(Display *display,int screen,char *user_geometry,
  char *default_geometry,unsigned int border_width,XSizeHints *size_hints,
  int *x,int *y,int *width,int *height,int *gravity)
{
  int
    status;

  status=XGeometry(display,screen,user_geometry,default_geometry,border_width,
    0,0,0,0,x,y,width,height);
  *gravity=NorthWestGravity;
  return(status);
}
#endif

#endif

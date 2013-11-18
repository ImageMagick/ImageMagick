/*
  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore X11 compatibility methods.
*/
#ifndef _MAGICKCORE_PRER5ICCCM_H
#define _MAGICKCORE_PRER5ICCCM_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(PRE_R6_ICCCM)
/*
  Compatability defines for pre X11R6 ICCCM.
*/
#define XK_KP_Home              0xFF95
#define XK_KP_Left              0xFF96
#define XK_KP_Up                0xFF97
#define XK_KP_Right             0xFF98
#define XK_KP_Down              0xFF99
#define XK_KP_Prior             0xFF9A
#define XK_KP_Page_Up           0xFF9A
#define XK_KP_Next              0xFF9B
#define XK_KP_Page_Down         0xFF9B
#define XK_KP_End               0xFF9C
#define XK_KP_Delete            0xFF9F

extern MagickExport Status
  XInitImage(XImage *ximage);
#endif

#if defined(PRE_R5_ICCCM)
extern MagickExport XrmDatabase
  XrmGetDatabase();
#endif

#if defined(PRE_R4_ICCCM)
#if defined(vms)
#define XMaxRequestSize(display)  16384
#endif

#define WithdrawnState  0

typedef struct _XTextProperty
{
  unsigned char
    *value;

  Atom
    encoding;

  int
    format;

  size_t
    nitems;
} XTextProperty;

char
  *XResourceManagerString();

extern MagickExport int
  XWMGeometry();

extern MagickExport Status
  XGetRGBColormaps(),
  XGetWMName(),
  XReconfigureWMWindow(),
  XSetWMProtocols(),
  XWithdrawWindow();

extern MagickExport XClassHint
  *XAllocClassHint();

extern MagickExport XIconSize
  *XAllocIconSize();

extern MagickExport XSizeHints
  *XAllocSizeHints();

extern MagickExport XStandardColormap
  *XAllocStandardColormap();

extern MagickExport XWMHints
  *XAllocWMHints();

extern MagickExport VisualID
  XVisualIDFromVisual();

extern MagickExport void
  XrmDestroyDatabase(),
  XSetWMIconName(),
  XSetWMName(),
  XSetWMProperties();
#else
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

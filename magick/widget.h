/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore X11 widget methods.
*/
#ifndef _MAGICKCORE_WIDGET_H
#define _MAGICKCORE_WIDGET_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(MAGICKCORE_X11_DELEGATE)

#include "magick/xwindow-private.h"

extern MagickExport int
  XCommandWidget(Display *,XWindows *,const char **,XEvent *),
  XConfirmWidget(Display *,XWindows *,const char *,const char *),
  XDialogWidget(Display *,XWindows *,const char *,const char *,char *),
  XMenuWidget(Display *,XWindows *,const char *,const char **,char *);

extern MagickExport MagickBooleanType
  XPreferencesWidget(Display *,XResourceInfo *,XWindows *);

extern MagickExport void
  DestroyXWidget(void),
  XColorBrowserWidget(Display *,XWindows *,const char *,char *),
  XFileBrowserWidget(Display *,XWindows *,const char *,char *),
  XFontBrowserWidget(Display *,XWindows *,const char *,char *),
  XInfoWidget(Display *,XWindows *,const char *),
  XListBrowserWidget(Display *,XWindows *,XWindowInfo *,const char **,
    const char *,const char *,char *),
  XNoticeWidget(Display *,XWindows *,const char *,const char *),
  XProgressMonitorWidget(Display *,XWindows *,const char *,
    const MagickOffsetType,const MagickSizeType),
  XTextViewWidget(Display *,const XResourceInfo *,XWindows *,
    const MagickBooleanType,const char *,const char **);

#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

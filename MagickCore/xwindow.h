/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
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
#ifndef _MAGICKCORE_XWINDOW_H
#define _MAGICKCORE_XWINDOW_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(MAGICKCORE_X11_DELEGATE)
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>

typedef struct _XImportInfo
  XImportInfo;

typedef struct _XResourceInfo
  XResourceInfo;

extern MagickExport char
  *XGetResourceClass(XrmDatabase,const char *,const char *,char *),
  *XGetResourceInstance(XrmDatabase,const char *,const char *,const char *),
  *XGetScreenDensity(Display *);

extern MagickExport int
  XError(Display *,XErrorEvent *);

extern MagickExport XrmDatabase
  XGetResourceDatabase(Display *,const char *);

extern MagickExport void
  XGetResourceInfo(const ImageInfo *,XrmDatabase,const char *,XResourceInfo *),
  XRetainWindowColors(Display *,const Window);
#endif

extern MagickExport Image
  *XImportImage(const ImageInfo *,XImportInfo *);

extern MagickExport MagickBooleanType
  XRemoteCommand(Display *,const char *,const char *);

extern MagickExport void
  DestroyXResources(void),
  XDestroyResourceInfo(XResourceInfo *),
  XGetImportInfo(XImportInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

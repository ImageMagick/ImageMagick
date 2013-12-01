/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            DDDD    OOO   TTTTT                              %
%                            D   D  O   O    T                                %
%                            D   D  O   O    T                                %
%                            D   D  O   O    T                                %
%                            DDDD    OOO     T                                %
%                                                                             %
%                                                                             %
%                      Read/Write Graphviz DOT Format                         %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
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
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/utility.h"
#include "magick/xwindow-private.h"
#if defined(MAGICKCORE_GVC_DELEGATE)
#undef HAVE_CONFIG_H
#include <gvc.h>
static GVC_t
  *graphic_context;
#endif

#if defined(MAGICKCORE_GVC_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d D O T I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDOTImage() reads a Graphviz image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadDOTImage method is:
%
%      Image *ReadDOTImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadDOTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    command[MaxTextExtent];

  const char
    *option;

  graph_t
    *graph;

  Image
    *image;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    return((Image *) NULL);
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) CopyMagickString(read_info->magick,"SVG",MaxTextExtent);
  (void) AcquireUniqueFilename(read_info->filename);
  (void) FormatLocaleString(command,MaxTextExtent,"-Tsvg -o%s %s",
    read_info->filename,image_info->filename);
#if !defined(WITH_CGRAPH)
  graph=agread(GetBlobFileHandle(image));
#else
  graph=agread(GetBlobFileHandle(image),(Agdisc_t *) NULL);
#endif
  if (graph == (graph_t *) NULL)
    return ((Image *) NULL);
  option=GetImageOption(image_info,"dot:layout-engine");
  if (option == (const char *) NULL)
    gvLayout(graphic_context,graph,(char *) "dot");
  else
    gvLayout(graphic_context,graph,(char *) option);
  gvRenderFilename(graphic_context,graph,(char *) "svg",read_info->filename);
  gvFreeLayout(graphic_context,graph);
  agclose(graph);
  /*
    Read SVG graph.
  */
  image=ReadImage(read_info,exception);
  (void) RelinquishUniqueFileResource(read_info->filename);
  read_info=DestroyImageInfo(read_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r D O T I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterDOTImage() adds attributes for the Display Postscript image
%  format to the list of supported formats.  The attributes include the image
%  format tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterDOTImage method is:
%
%      size_t RegisterDOTImage(void)
%
*/
ModuleExport size_t RegisterDOTImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("DOT");
#if defined(MAGICKCORE_GVC_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadDOTImage;
#endif
  entry->blob_support=MagickFalse;
  entry->description=ConstantString("Graphviz");
  entry->module=ConstantString("DOT");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("GV");
#if defined(MAGICKCORE_GVC_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadDOTImage;
#endif
  entry->blob_support=MagickFalse;
  entry->description=ConstantString("Graphviz");
  entry->module=ConstantString("DOT");
  (void) RegisterMagickInfo(entry);
#if defined(MAGICKCORE_GVC_DELEGATE)
  graphic_context=gvContext();
#endif
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r D O T I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterDOTImage() removes format registrations made by the
%  DOT module from the list of supported formats.
%
%  The format of the UnregisterDOTImage method is:
%
%      UnregisterDOTImage(void)
%
*/
ModuleExport void UnregisterDOTImage(void)
{
  (void) UnregisterMagickInfo("GV");
  (void) UnregisterMagickInfo("DOT");
#if defined(MAGICKCORE_GVC_DELEGATE)
  gvFreeContext(graphic_context);
#endif
}

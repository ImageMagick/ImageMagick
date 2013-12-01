/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        H   H   AAA   L      DDDD                            %
%                        H   H  A   A  L      D   D                           %
%                        HHHHH  AAAAA  L      D   D                           %
%                        H   H  A   A  L      D   D                           %
%                        H   H  A   A  LLLLL  DDDD                            %
%                                                                             %
%                                                                             %
%                   Create Identity Hald CLUT Image Format                    %
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
#include "magick/cache.h"
#include "magick/colorspace.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/thread-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d H A L D I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadHALDImage() creates a Hald color lookup table image and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadHALDImage method is:
%
%      Image *ReadHALDImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadHALDImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  size_t
    cube_size,
    level;

  ssize_t
    y;

  /*
    Create HALD color lookup table image.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  level=0;
  if (*image_info->filename != '\0')
    level=StringToUnsignedLong(image_info->filename);
  if (level < 2)
    level=8;
  status=MagickTrue;
  cube_size=level*level;
  image->columns=(size_t) (level*cube_size);
  image->rows=(size_t) (level*cube_size);
  for (y=0; y < (ssize_t) image->rows; y+=(ssize_t) level)
  {
    ssize_t
      blue,
      green,
      red;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=QueueAuthenticPixels(image,0,y,image->columns,(size_t) level,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    blue=y/(ssize_t) level;
    for (green=0; green < (ssize_t) cube_size; green++)
    {
      for (red=0; red < (ssize_t) cube_size; red++)
      {
        SetPixelRed(q,ClampToQuantum((MagickRealType)
          (QuantumRange*red/(cube_size-1.0))));
        SetPixelGreen(q,ClampToQuantum((MagickRealType)
          (QuantumRange*green/(cube_size-1.0))));
        SetPixelBlue(q,ClampToQuantum((MagickRealType)
          (QuantumRange*blue/(cube_size-1.0))));
        SetPixelOpacity(q,OpaqueOpacity);
        q++;
      }
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      status=MagickFalse;
  }
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r H A L D I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterHALDImage() adds attributes for the Hald color lookup table image
%  format to the list of supported formats.  The attributes include the image
%  format tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob, whether
%  the format supports native in-memory I/O, and a brief description of the
%  format.
%
%  The format of the RegisterHALDImage method is:
%
%      size_t RegisterHALDImage(void)
%
*/
ModuleExport size_t RegisterHALDImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("HALD");
  entry->decoder=(DecodeImageHandler *) ReadHALDImage;
  entry->adjoin=MagickFalse;
  entry->format_type=ImplicitFormatType;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->description=ConstantString("Identity Hald color lookup table image");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r H A L D I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterHALDImage() removes format registrations made by the
%  HALD module from the list of supported formats.
%
%  The format of the UnregisterHALDImage method is:
%
%      UnregisterHALDImage(void)
%
*/
ModuleExport void UnregisterHALDImage(void)
{
  (void) UnregisterMagickInfo("HALD");
}

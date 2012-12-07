/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%              PPPP   RRRR   EEEEE  V   V  IIIII  EEEEE  W   W                %
%              P   P  R   R  E      V   V    I    E      W   W                %
%              PPPP   RRRR   EEE    V   V    I    EEE    W   W                %
%              P      R R    E       V V     I    E      W W W                %
%              P      R  R   EEEEE    V    IIIII  EEEEE   W W                 %
%                                                                             %
%                                                                             %
%                           Write A Preview Image.                            %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
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

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/client.h"
#include "magick/constitute.h"
#include "magick/effect.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/resize.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/transform.h"
#include "magick/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePreviewImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P R E V I E W I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPREVIEWImage() adds attributes for the Preview image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPREVIEWImage method is:
%
%      size_t RegisterPREVIEWImage(void)
%
*/
ModuleExport size_t RegisterPREVIEWImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("PREVIEW");
  entry->encoder=(EncodeImageHandler *) WritePreviewImage;
  entry->adjoin=MagickFalse;
  entry->format_type=ImplicitFormatType;
  entry->description=ConstantString(
    "Show a preview an image enhancement, effect, or f/x");
  entry->module=ConstantString("PREVIEW");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P R E V I E W I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPREVIEWImage() removes format registrations made by the
%  PREVIEW module from the list of supported formats.
%
%  The format of the UnregisterPREVIEWImage method is:
%
%      UnregisterPREVIEWImage(void)
%
*/
ModuleExport void UnregisterPREVIEWImage(void)
{
  (void) UnregisterMagickInfo("PREVIEW");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P R E V I E W I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePreviewImage creates several tiles each with a varying
%  stength of an image enhancement function (e.g. gamma).  The image is written
%  in the MIFF format.
%
%  The format of the WritePreviewImage method is:
%
%      MagickBooleanType WritePreviewImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WritePreviewImage(const ImageInfo *image_info,
  Image *image)
{
  Image
    *preview_image;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  preview_image=PreviewImage(image,image_info->preview_type,&image->exception);
  if (preview_image == (Image *) NULL)
    return(MagickFalse);
  (void) CopyMagickString(preview_image->filename,image_info->filename,
    MaxTextExtent);
  write_info=CloneImageInfo(image_info);
  (void) SetImageInfo(write_info,1,&image->exception);
  if (LocaleCompare(write_info->magick,"PREVIEW") == 0)
    (void) FormatLocaleString(preview_image->filename,MaxTextExtent,
      "miff:%s",image_info->filename);
  status=WriteImage(write_info,preview_image);
  preview_image=DestroyImage(preview_image);
  write_info=DestroyImageInfo(write_info);
  return(status);
}

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
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/studio.h"
#include "MagickCore/property.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/client.h"
#include "MagickCore/constitute.h"
#include "MagickCore/effect.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePreviewImage(const ImageInfo *,Image *,ExceptionInfo *);

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
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType WritePreviewImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
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
  preview_image=PreviewImage(image,image_info->preview_type,exception);
  if (preview_image == (Image *) NULL)
    return(MagickFalse);
  (void) CopyMagickString(preview_image->filename,image_info->filename,
    MaxTextExtent);
  write_info=CloneImageInfo(image_info);
  (void) SetImageInfo(write_info,1,exception);
  if (LocaleCompare(write_info->magick,"PREVIEW") == 0)
    (void) FormatLocaleString(preview_image->filename,MaxTextExtent,
      "miff:%s",image_info->filename);
  status=WriteImage(write_info,preview_image,exception);
  preview_image=DestroyImage(preview_image);
  write_info=DestroyImageInfo(write_info);
  return(status);
}

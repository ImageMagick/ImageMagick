/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%       TTTTT  H   H  U   U  M   M  BBBB   N   N   AAA   IIIII  L             %
%         T    H   H  U   U  MM MM  B   B  NN  N  A   A    I    L             %
%         T    HHHHH  U   U  M M M  BBBB   N N N  AAAAA    I    L             %
%         T    H   H  U   U  M   M  B   B  N  NN  A   A    I    L             %
%         T    H   H   UUU   M   M  BBBB   N   N  A   A  IIIII  LLLLL         %
%                                                                             %
%                                                                             %
%                        Write EXIF Thumbnail To File.                        %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/profile.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteTHUMBNAILImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r T H U M B N A I L I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterTHUMBNAILImage() adds attributes for the THUMBNAIL image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterTHUMBNAILImage method is:
%
%      size_t RegisterTHUMBNAILImage(void)
%
*/
ModuleExport size_t RegisterTHUMBNAILImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("THUMBNAIL","THUMBNAIL","EXIF Profile Thumbnail");
  entry->encoder=(EncodeImageHandler *) WriteTHUMBNAILImage;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r T H U M B N A I L I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterTHUMBNAILImage() removes format registrations made by the
%  THUMBNAIL module from the list of supported formats.
%
%  The format of the UnregisterTHUMBNAILImage method is:
%
%      UnregisterTHUMBNAILImage(void)
%
*/
ModuleExport void UnregisterTHUMBNAILImage(void)
{
  (void) UnregisterMagickInfo("THUMBNAIL");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e T H U M B N A I L I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteTHUMBNAILImage() extracts the EXIF thumbnail image and writes it.
%
%  The format of the WriteTHUMBNAILImage method is:
%
%      MagickBooleanType WriteTHUMBNAILImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteTHUMBNAILImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  const char
    *property;

  const StringInfo
    *profile;

  Image
    *thumbnail_image;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  register ssize_t
    i;

  size_t
    length;

  ssize_t
    offset;

  unsigned char
    *q;

  profile=GetImageProfile(image,"exif");
  if (profile == (const StringInfo *) NULL)
    ThrowWriterException(CoderError,"ImageDoesNotHaveAThumbnail");
  property=GetImageProperty(image,"exif:JPEGInterchangeFormat",exception);
  if (property == (const char *) NULL)
    ThrowWriterException(CoderError,"ImageDoesNotHaveAThumbnail");
  offset=(ssize_t) StringToLong(property);
  if (offset < 0)
    ThrowWriterException(CoderError,"ImageDoesNotHaveAThumbnail");
  property=GetImageProperty(image,"exif:JPEGInterchangeFormatLength",exception);
  if (property == (const char *) NULL)
    ThrowWriterException(CoderError,"ImageDoesNotHaveAThumbnail");
  length=(size_t) StringToLong(property);
  q=GetStringInfoDatum(profile)+offset;
  for (i=offset; i < (ssize_t) GetStringInfoLength(profile) - 3; i++)
  {
    if (memcmp(q,"\377\330\377",3) == 0)
      break;
    q++;
  }
  if ((q+length) > (GetStringInfoDatum(profile)+GetStringInfoLength(profile)))
    ThrowWriterException(CoderError,"ImageDoesNotHaveAThumbnail");
  thumbnail_image=BlobToImage(image_info,q,length,exception);
  if (thumbnail_image == (Image *) NULL)
    return(MagickFalse);
  (void) SetImageType(thumbnail_image,thumbnail_image->alpha_trait ==
    UndefinedPixelTrait ? TrueColorType : TrueColorAlphaType,exception);
  (void) CopyMagickString(thumbnail_image->filename,image->filename,
    MagickPathExtent);
  write_info=CloneImageInfo(image_info);
  *write_info->magick='\0';
  (void) SetImageInfo(write_info,1,exception);
  if ((*write_info->magick == '\0') ||
      (LocaleCompare(write_info->magick,"THUMBNAIL") == 0))
    (void) FormatLocaleString(thumbnail_image->filename,MagickPathExtent,
      "miff:%s",write_info->filename);
  status=WriteImage(write_info,thumbnail_image,exception);
  thumbnail_image=DestroyImage(thumbnail_image);
  write_info=DestroyImageInfo(write_info);
  return(status);
}

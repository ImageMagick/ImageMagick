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
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/string-private.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteTHUMBNAILImage(const ImageInfo *,Image *);

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

  entry=SetMagickInfo("THUMBNAIL");
  entry->encoder=(EncodeImageHandler *) WriteTHUMBNAILImage;
  entry->description=ConstantString("EXIF Profile Thumbnail");
  entry->module=ConstantString("THUMBNAIL");
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
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteTHUMBNAILImage(const ImageInfo *image_info,
  Image *image)
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
    magick[MaxTextExtent];

  profile=GetImageProfile(image,"exif");
  if (profile == (const StringInfo *) NULL)
    ThrowWriterException(CoderError,"ImageDoesNotHaveAThumbnail");
  property=GetImageProperty(image,"exif:JPEGInterchangeFormat");
  if (property == (const char *) NULL)
    ThrowWriterException(CoderError,"ImageDoesNotHaveAThumbnail");
  offset=(ssize_t) StringToLong(property);
  property=GetImageProperty(image,"exif:JPEGInterchangeFormatLength");
  if (property == (const char *) NULL)
    ThrowWriterException(CoderError,"ImageDoesNotHaveAThumbnail");
  length=(size_t) StringToLong(property);
  (void) ResetMagickMemory(magick,0,sizeof(magick));
  for (i=0; i < (ssize_t) length; i++)
  {
    magick[0]=magick[1];
    magick[1]=magick[2];
    magick[2]=GetStringInfoDatum(profile)[offset+i];
    if (memcmp(magick,"\377\330\377",3) == 0)
      break;
  }
  thumbnail_image=BlobToImage(image_info,GetStringInfoDatum(profile)+offset+i-2,
    length,&image->exception);
  if (thumbnail_image == (Image *) NULL)
    return(MagickFalse);
  (void) SetImageType(thumbnail_image,thumbnail_image->matte == MagickFalse ?
    TrueColorType : TrueColorMatteType);
  (void) CopyMagickString(thumbnail_image->filename,image->filename,
    MaxTextExtent);
  write_info=CloneImageInfo(image_info);
  (void) SetImageInfo(write_info,1,&image->exception);
  if (LocaleCompare(write_info->magick,"THUMBNAIL") == 0)
    (void) FormatLocaleString(thumbnail_image->filename,MaxTextExtent,
      "miff:%s",write_info->filename);
  status=WriteImage(write_info,thumbnail_image);
  thumbnail_image=DestroyImage(thumbnail_image);
  write_info=DestroyImageInfo(write_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     M   M   AAA   TTTTT  TTTTT  EEEEE                       %
%                     MM MM  A   A    T      T    E                           %
%                     M M M  AAAAA    T      T    EEE                         %
%                     M   M  A   A    T      T    E                           %
%                     M   M  A   A    T      T    EEEEE                       %
%                                                                             %
%                                                                             %
%                     Write Matte Channel To MIFF File.                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
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
#include "MagickCore/cache.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteMATTEImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r M A T T E I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterMATTEImage() adds attributes for the MATTE image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterMATTEImage method is:
%
%      size_t RegisterMATTEImage(void)
%
*/
ModuleExport size_t RegisterMATTEImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("MATTE","MATTE","MATTE format");
  entry->encoder=(EncodeImageHandler *) WriteMATTEImage;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r M A T T E I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterMATTEImage() removes format registrations made by the
%  MATTE module from the list of supported formats.
%
%  The format of the UnregisterMATTEImage method is:
%
%      UnregisterMATTEImage(void)
%
*/
ModuleExport void UnregisterMATTEImage(void)
{
  (void) UnregisterMagickInfo("MATTE");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M A T T E I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteMATTEImage() writes an image of matte bytes to a file.  It consists of
%  data from the matte component of the image [0..255].
%
%  The format of the WriteMATTEImage method is:
%
%      MagickBooleanType WriteMATTEImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteMATTEImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  Image
    *matte_image;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  const Quantum
    *p;

  ssize_t
    x;

  Quantum
    *q;

  ssize_t
    y;

  if ((image->alpha_trait & BlendPixelTrait) == 0)
    ThrowWriterException(CoderError,"ImageDoesNotHaveAnAlphaChannel");
  matte_image=CloneImage(image,0,0,MagickTrue,exception);
  if (matte_image == (Image *) NULL)
    return(MagickFalse);
  (void) SetImageType(matte_image,TrueColorAlphaType,exception);
  matte_image->alpha_trait=UndefinedPixelTrait;
  /*
    Convert image to matte pixels.
  */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    q=QueueAuthenticPixels(matte_image,0,y,matte_image->columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelRed(matte_image,GetPixelAlpha(image,p),q);
      SetPixelGreen(matte_image,GetPixelAlpha(image,p),q);
      SetPixelBlue(matte_image,GetPixelAlpha(image,p),q);
      SetPixelAlpha(matte_image,OpaqueAlpha,q);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(matte_image);
    }
    if (SyncAuthenticPixels(matte_image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  write_info=CloneImageInfo(image_info);
  if ((*write_info->magick == '\0') ||
      (LocaleCompare(write_info->magick,"MATTE") == 0))
    (void) FormatLocaleString(matte_image->filename,MagickPathExtent,
      "MIFF:%s",image->filename);
  status=WriteImage(write_info,matte_image,exception);
  write_info=DestroyImageInfo(write_info);
  matte_image=DestroyImage(matte_image);
  return(status);
}

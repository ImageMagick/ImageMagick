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
#include "magick/cache.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteMATTEImage(const ImageInfo *,Image *);

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

  entry=SetMagickInfo("MATTE");
  entry->encoder=(EncodeImageHandler *) WriteMATTEImage;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("MATTE format");
  entry->module=ConstantString("MATTE");
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
%  Function WriteMATTEImage() writes an image of matte bytes to a file.  It
%  consists of data from the matte component of the image [0..255].
%
%  The format of the WriteMATTEImage method is:
%
%      MagickBooleanType WriteMATTEImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteMATTEImage(const ImageInfo *image_info,
  Image *image)
{
  ExceptionInfo
    *exception;

  Image
    *matte_image;

  MagickBooleanType
    status;

  register const PixelPacket
    *p;

  register ssize_t
    x;

  register PixelPacket
    *q;

  ssize_t
    y;

  if (image->matte == MagickFalse)
    ThrowWriterException(CoderError,"ImageDoesNotHaveAAlphaChannel");
  matte_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    &image->exception);
  if (matte_image == (Image *) NULL)
    return(MagickFalse);
  (void) SetImageType(matte_image,TrueColorMatteType);
  matte_image->matte=MagickFalse;
  /*
    Convert image to matte pixels.
  */
  exception=(&image->exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    q=QueueAuthenticPixels(matte_image,0,y,matte_image->columns,1,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelRed(q,GetPixelOpacity(p));
      SetPixelGreen(q,GetPixelOpacity(p));
      SetPixelBlue(q,GetPixelOpacity(p));
      SetPixelOpacity(q,OpaqueOpacity);
      p++;
      q++;
    }
    if (SyncAuthenticPixels(matte_image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  (void) FormatLocaleString(matte_image->filename,MaxTextExtent,
    "MIFF:%s",image->filename);
  status=WriteImage(image_info,matte_image);
  matte_image=DestroyImage(matte_image);
  return(status);
}

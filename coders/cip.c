/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                             CCCC  IIIII  PPPP                               %
%                            C        I    P   P                              %
%                            C        I    PPPP                               %
%                            C        I    P                                  %
%                             CCCC  IIIII  P                                  %
%                                                                             %
%                                                                             %
%                  Read/Write Cisco IP Phone Image Format                     %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                April 2004                                   %
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
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
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
#include "magick/pixel-accessor.h"
#include "magick/property.h"
#include "magick/quantize.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteCIPImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r C I P I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterCIPImage() adds properties for the CIP IP phone image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterCIPImage method is:
%
%      size_t RegisterCIPImage(void)
%
*/
ModuleExport size_t RegisterCIPImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("CIP");
  entry->encoder=(EncodeImageHandler *) WriteCIPImage;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("Cisco IP phone image format");
  entry->module=ConstantString("CIP");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r C I P I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterCIPImage() removes format registrations made by the
%  CIP module from the list of supported formats.
%
%  The format of the UnregisterCIPImage method is:
%
%      UnregisterCIPImage(void)
%
*/
ModuleExport void UnregisterCIPImage(void)
{
  (void) UnregisterMagickInfo("CIP");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e C I P I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure WriteCIPImage() writes an image to a file in the Cisco IP phone
%  image format.
%
%  The format of the WriteCIPImage method is:
%
%      MagickBooleanType WriteCIPImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static inline ssize_t MagickMin(const ssize_t x,const ssize_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static MagickBooleanType WriteCIPImage(const ImageInfo *image_info,Image *image)
{
  char
    buffer[MaxTextExtent];

  const char
    *value;

  MagickBooleanType
    status;

  register const PixelPacket
    *p;

  register ssize_t
    i,
    x;

  ssize_t
    y;

  unsigned char
    byte;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  (void) WriteBlobString(image,"<CiscoIPPhoneImage>\n");
  value=GetImageProperty(image,"label");
  if (value != (const char *) NULL)
    (void) FormatLocaleString(buffer,MaxTextExtent,"<Title>%s</Title>\n",value);
  else
    {
      char
        basename[MaxTextExtent];

      GetPathComponent(image->filename,BasePath,basename);
      (void) FormatLocaleString(buffer,MaxTextExtent,"<Title>%s</Title>\n",
        basename);
    }
  (void) WriteBlobString(image,buffer);
  (void) FormatLocaleString(buffer,MaxTextExtent,
    "<LocationX>%.20g</LocationX>\n",(double) image->page.x);
  (void) WriteBlobString(image,buffer);
  (void) FormatLocaleString(buffer,MaxTextExtent,
    "<LocationY>%.20g</LocationY>\n",(double) image->page.y);
  (void) WriteBlobString(image,buffer);
  (void) FormatLocaleString(buffer,MaxTextExtent,"<Width>%.20g</Width>\n",
    (double) (image->columns+(image->columns % 2)));
  (void) WriteBlobString(image,buffer);
  (void) FormatLocaleString(buffer,MaxTextExtent,"<Height>%.20g</Height>\n",
    (double) image->rows);
  (void) WriteBlobString(image,buffer);
  (void) FormatLocaleString(buffer,MaxTextExtent,"<Depth>2</Depth>\n");
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,"<Data>");
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    for (x=0; x < ((ssize_t) image->columns-3); x+=4)
    {
      byte=(unsigned char)
        ((((size_t) (3*ClampToQuantum(GetPixelLuma(image,p+3))/QuantumRange) & 0x03) << 6) |
         (((size_t) (3*ClampToQuantum(GetPixelLuma(image,p+2))/QuantumRange) & 0x03) << 4) |
         (((size_t) (3*ClampToQuantum(GetPixelLuma(image,p+1))/QuantumRange) & 0x03) << 2) |
         (((size_t) (3*ClampToQuantum(GetPixelLuma(image,p+0))/QuantumRange) & 0x03) << 0));
      (void) FormatLocaleString(buffer,MaxTextExtent,"%02x",byte);
      (void) WriteBlobString(image,buffer);
      p+=4;
    }
    if ((image->columns % 4) != 0)
      {
        i=(ssize_t) image->columns % 4;
        byte=(unsigned char)
          ((((size_t) (3*ClampToQuantum(GetPixelLuma(image,p+MagickMin(i,3)))/QuantumRange) & 0x03) << 6) |
           (((size_t) (3*ClampToQuantum(GetPixelLuma(image,p+MagickMin(i,2)))/QuantumRange) & 0x03) << 4) |
           (((size_t) (3*ClampToQuantum(GetPixelLuma(image,p+MagickMin(i,1)))/QuantumRange) & 0x03) << 2) |
           (((size_t) (3*ClampToQuantum(GetPixelLuma(image,p+MagickMin(i,0)))/QuantumRange) & 0x03) << 0));
        (void) FormatLocaleString(buffer,MaxTextExtent,"%02x",~byte);
        (void) WriteBlobString(image,buffer);
      }
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  (void) WriteBlobString(image,"</Data>\n");
  (void) WriteBlobString(image,"</CiscoIPPhoneImage>\n");
  (void) CloseBlob(image);
  return(MagickTrue);
}

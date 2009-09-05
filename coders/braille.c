/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%               BBBB   RRRR    AAA   IIIII  L      L      EEEEE               %
%               B   B  R   R  A   A    I    L      L      E                   %
%               BBBB   RRRR   AAAAA    I    L      L      EEE                 %
%               B   B  R R    A   A    I    L      L      E                   %
%               BBBB   R  R   A   A  IIIII  LLLLL  LLLLL  EEEEE               %
%                                                                             %
%                                                                             %
%                          Read/Write Braille Format                          %
%                                                                             %
%                               Samuel Thibault                               %
%                                February 2008                                %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/constitute.h"
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
#include "magick/property.h"
#include "magick/quantize.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteBRAILLEImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r B R A I L L E I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterBRAILLEImage() adds values for the Braille format to
%  the list of supported formats.  The values include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterBRAILLEImage method is:
%
%      unsigned long RegisterBRAILLEImage(void)
%
*/
ModuleExport unsigned long RegisterBRAILLEImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("BRF");
  entry->encoder=(EncodeImageHandler *) WriteBRAILLEImage;
  entry->adjoin=MagickFalse;
  entry->description=AcquireString("BRF ASCII Braille format");
  entry->module=AcquireString("BRAILLE");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("UBRL");
  entry->encoder=(EncodeImageHandler *) WriteBRAILLEImage;
  entry->adjoin=MagickFalse;
  entry->description=AcquireString("Unicode Text format");
  entry->module=AcquireString("BRAILLE");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("ISOBRL");
  entry->encoder=(EncodeImageHandler *) WriteBRAILLEImage;
  entry->adjoin=MagickFalse;
  entry->description=AcquireString("ISO/TR 11548-1 format");
  entry->module=AcquireString("BRAILLE");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r B R A I L L E I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterBRAILLEImage() removes format registrations made by the
%  BRAILLE module from the list of supported formats.
%
%  The format of the UnregisterBRAILLEImage method is:
%
%      UnregisterBRAILLEImage(void)
%
*/
ModuleExport void UnregisterBRAILLEImage(void)
{
  (void) UnregisterMagickInfo("BRF");
  (void) UnregisterMagickInfo("UBRL");
  (void) UnregisterMagickInfo("ISOBRL");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e B R A I L L E I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBRAILLEImage() writes an image to a file in the Braille format.
%
%  The format of the WriteBRAILLEImage method is:
%
%      MagickBooleanType WriteBRAILLEImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: The image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteBRAILLEImage(const ImageInfo *image_info,
  Image *image)
{
  char
    buffer[MaxTextExtent];

  const char
    *value;

  long
    y;

  MagickBooleanType
    status;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  IndexPacket
    polarity;

  register long
    x;

  unsigned long
    cell_height = 4;
  int
    unicode = 0, iso_11548_1 = 0;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);

  if (LocaleCompare(image_info->magick, "UBRL") == 0)
    unicode = 1;
  else
    if (LocaleCompare(image_info->magick, "ISOBRL") == 0)
      iso_11548_1 = 1;
    else
      cell_height = 3;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);

  if (!iso_11548_1)
    {
      value=GetImageProperty(image,"Label");
      if (value != (const char *) NULL) {
        (void) FormatMagickString(buffer,MaxTextExtent,"Title: %s\n", value);
        (void) WriteBlobString(image,buffer);
      }
      if (image->page.x)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"X: %ld\n", image->page.x);
        (void) WriteBlobString(image,buffer);
      }
      if (image->page.y)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"Y: %ld\n", image->page.y);
        (void) WriteBlobString(image,buffer);
      }
      (void) FormatMagickString(buffer,MaxTextExtent,"Width: %lu\n",
        image->columns+(image->columns % 2));
      (void) WriteBlobString(image,buffer);
      (void) FormatMagickString(buffer,MaxTextExtent,"Height: %lu\n", image->rows);
      (void) WriteBlobString(image,buffer);

      (void) WriteBlobString(image,"\n");
    }

  (void) SetImageType(image,BilevelType);
  polarity=(IndexPacket) (PixelIntensityToQuantum(&image->colormap[0]) >=
    (Quantum) (QuantumRange/2));
  if (image->colors == 2)
    polarity=(IndexPacket)
      (PixelIntensityToQuantum(&image->colormap[0]) >=
       PixelIntensityToQuantum(&image->colormap[1]));
  for (y=0; y < (long) image->rows; y+=cell_height)
  {
    if ((y+cell_height) > image->rows)
      cell_height = (unsigned long) (image->rows-y);

    p=GetVirtualPixels(image,0,y,image->columns,cell_height,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);

    for (x=0; x < (long) image->columns; x+=2)
    {
      unsigned char cell = 0;
      int two_columns = x+1 < (long) image->columns;

      do
      {
#define do_cell(dx,dy,bit) \
        cell |= (indexes[x+dx+dy*image->columns]==polarity)<<bit;

        do_cell(0,0,0);
        if (two_columns)
          do_cell(1,0,3);
        if (cell_height < 2)
          break;

        do_cell(0,1,1);
        if (two_columns)
          do_cell(1,1,4);
        if (cell_height < 3)
          break;

        do_cell(0,2,2);
        if (two_columns)
          do_cell(1,2,5);
        if (cell_height < 4)
          break;

        do_cell(0,3,6);
        if (two_columns)
          do_cell(1,3,7);
      } while(0);

      if (unicode)
        {
          unsigned char utf8[3];
          /* Unicode text */
          utf8[0] = (unsigned char) (0xe0|((0x28>>4)&0x0f));
          utf8[1] = 0x80|((0x28<<2)&0x3f)|(cell>>6);
          utf8[2] = 0x80|(cell&0x3f);
          (void) WriteBlob(image,3,utf8);
        }
      else if (iso_11548_1)
        {
          /* ISO/TR 11548-1 binary */
          (void) WriteBlobByte(image,cell);
        }
      else
        {
          /* BRF */
          static const unsigned char iso_to_brf[64] = {
            ' ', 'A', '1', 'B', '\'', 'K', '2', 'L',
            '@', 'C', 'I', 'F', '/', 'M', 'S', 'P',
            '"', 'E', '3', 'H', '9', 'O', '6', 'R',
            '^', 'D', 'J', 'G', '>', 'N', 'T', 'Q',
            ',', '*', '5', '<', '-', 'U', '8', 'V',
            '.', '%', '[', '$', '+', 'X', '!', '&',
            ';', ':', '4', '\\', '0', 'Z', '7', '(',
            '_', '?', 'W', ']', '#', 'Y', ')', '='
          };
          (void) WriteBlobByte(image,iso_to_brf[cell]);
        }
    }
    if (!iso_11548_1)
      (void) WriteBlobByte(image,'\n');
    status=SetImageProgress(image,SaveImageTag,y,image->rows);
    if (status == MagickFalse)
      break;
  }
  (void) CloseBlob(image);
  return(MagickTrue);
}

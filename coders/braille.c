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
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantize.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteBRAILLEImage(const ImageInfo *,Image *,ExceptionInfo *);

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
%      size_t RegisterBRAILLEImage(void)
%
*/
ModuleExport size_t RegisterBRAILLEImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("BRAILLE","BRF","BRF ASCII Braille format");
  entry->encoder=(EncodeImageHandler *) WriteBRAILLEImage;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("BRAILLE","UBRL","Unicode Text format");
  entry->encoder=(EncodeImageHandler *) WriteBRAILLEImage;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("BRAILLE","UBRL6","Unicode Text format 6dot");
  entry->encoder=(EncodeImageHandler *) WriteBRAILLEImage;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("BRAILLE","ISOBRL","ISO/TR 11548-1 format");
  entry->encoder=(EncodeImageHandler *) WriteBRAILLEImage;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("BRAILLE","ISOBRL6","ISO/TR 11548-1 format 6dot");
  entry->encoder=(EncodeImageHandler *) WriteBRAILLEImage;
  entry->flags^=CoderAdjoinFlag;
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
  (void) UnregisterMagickInfo("UBRL6");
  (void) UnregisterMagickInfo("ISOBRL");
  (void) UnregisterMagickInfo("ISOBRL6");
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
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: The image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType WriteBRAILLEImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  char
    buffer[MagickPathExtent];

  const char
    *value;

  int
    unicode = 0,
    iso_11548_1 = 0;

  MagickBooleanType
    status;

  Quantum
    polarity;

  register const Quantum
    *p;

  register ssize_t
    x;

  size_t
    cell_height = 4;

  ssize_t
    y;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (LocaleCompare(image_info->magick, "UBRL") == 0)
    unicode=1;
  else if (LocaleCompare(image_info->magick, "UBRL6") == 0)
    {
      unicode=1;
      cell_height=3;
    }
  else if (LocaleCompare(image_info->magick, "ISOBRL") == 0)
    iso_11548_1=1;
  else if (LocaleCompare(image_info->magick, "ISOBRL6") == 0)
    {
      iso_11548_1=1;
      cell_height=3;
    }
  else
    cell_height=3;
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  if (!iso_11548_1)
    {
      value=GetImageProperty(image,"label",exception);
      if (value != (const char *) NULL)
        {
          (void) FormatLocaleString(buffer,MagickPathExtent,"Title: %s\n", value);
          (void) WriteBlobString(image,buffer);
        }
      if (image->page.x != 0)
        {
          (void) FormatLocaleString(buffer,MagickPathExtent,"X: %.20g\n",(double) 
            image->page.x);
          (void) WriteBlobString(image,buffer);
        }
      if (image->page.y != 0)
        {
          (void) FormatLocaleString(buffer,MagickPathExtent,"Y: %.20g\n",(double) 
            image->page.y);
          (void) WriteBlobString(image,buffer);
        }
      (void) FormatLocaleString(buffer,MagickPathExtent,"Width: %.20g\n",(double)
        (image->columns+(image->columns % 2)));
      (void) WriteBlobString(image,buffer);
      (void) FormatLocaleString(buffer,MagickPathExtent,"Height: %.20g\n",(double)
        image->rows);
      (void) WriteBlobString(image,buffer);
      (void) WriteBlobString(image,"\n");
    }
  (void) SetImageType(image,BilevelType,exception);
  polarity = 0;
  if (image->storage_class == PseudoClass) {
    polarity=(Quantum) (GetPixelInfoIntensity(image,&image->colormap[0]) >=
      (QuantumRange/2.0));
    if (image->colors == 2)
      polarity=(Quantum) (GetPixelInfoIntensity(image,&image->colormap[0]) >=
        GetPixelInfoIntensity(image,&image->colormap[1]));
  }
  for (y=0; y < (ssize_t) image->rows; y+=(ssize_t) cell_height)
  {
    if ((y+cell_height) > image->rows)
      cell_height = (size_t) (image->rows-y);

    p=GetVirtualPixels(image,0,y,image->columns,cell_height,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x+=2)
    {
      unsigned char cell = 0;
      int two_columns = x+1 < (ssize_t) image->columns;

      do
      {
#define do_cell(dx,dy,bit) do { \
        if (image->storage_class == PseudoClass) \
          cell |= (GetPixelIndex(image,p+x+dx+dy*image->columns) == polarity) << bit; \
        else \
          cell |= (GetPixelGreen(image,p+x+dx+dy*image->columns) == 0) << bit; \
DisableMSCWarning(4127) \
} while (0) \
RestoreMSCWarning

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
DisableMSCWarning(4127)
      } while(0);
RestoreMSCWarning

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
    if (iso_11548_1 == 0)
      (void) WriteBlobByte(image,'\n');
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  (void) CloseBlob(image);
  return(MagickTrue);
}

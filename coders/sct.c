/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            SSSSS   CCCC  TTTTT                              %
%                            SS     C        T                                %
%                             SSS   C        T                                %
%                               SS  C        T                                %
%                            SSSSS   CCCC    T                                %
%                                                                             %
%                                                                             %
%                    Read Scitex HandShake Image Format                       %
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
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
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
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/string-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s S C T                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsSCT() returns MagickTrue if the image format type, identified by the
%  magick string, is SCT.
%
%  The format of the IsSCT method is:
%
%      MagickBooleanType IsSCT(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsSCT(const unsigned char *magick,const size_t length)
{
  if (length < 2)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick,"CT",2) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S C T I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadSCTImage() reads a Scitex image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadSCTImage method is:
%
%      Image *ReadSCTImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadSCTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    magick[2];

  Image
    *image;

  MagickBooleanType
    status;

  MagickRealType
    height,
    width;

  Quantum
    pixel;

  register IndexPacket
    *indexes;

  register ssize_t
    i,
    x;

  register PixelPacket
    *q;

  ssize_t
    count,
    y;

  unsigned char
    buffer[768];

  size_t
    separations,
    separations_mask,
    units;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Read control block.
  */
  count=ReadBlob(image,80,buffer);
  (void) count;
  count=ReadBlob(image,2,(unsigned char *) magick);
  if ((LocaleNCompare((char *) magick,"CT",2) != 0) &&
      (LocaleNCompare((char *) magick,"LW",2) != 0) &&
      (LocaleNCompare((char *) magick,"BM",2) != 0) &&
      (LocaleNCompare((char *) magick,"PG",2) != 0) &&
      (LocaleNCompare((char *) magick,"TX",2) != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if ((LocaleNCompare((char *) magick,"LW",2) == 0) ||
      (LocaleNCompare((char *) magick,"BM",2) == 0) ||
      (LocaleNCompare((char *) magick,"PG",2) == 0) ||
      (LocaleNCompare((char *) magick,"TX",2) == 0))
    ThrowReaderException(CoderError,"OnlyContinuousTonePictureSupported");
  count=ReadBlob(image,174,buffer);
  count=ReadBlob(image,768,buffer);
  /*
    Read paramter block.
  */
  units=1UL*ReadBlobByte(image);
  if (units == 0)
    image->units=PixelsPerCentimeterResolution;
  separations=1UL*ReadBlobByte(image);
  separations_mask=ReadBlobMSBShort(image);
  count=ReadBlob(image,14,buffer);
  buffer[14]='\0';
  height=StringToDouble((char *) buffer,(char **) NULL);
  count=ReadBlob(image,14,buffer);
  width=StringToDouble((char *) buffer,(char **) NULL);
  count=ReadBlob(image,12,buffer);
  buffer[12]='\0';
  image->rows=StringToUnsignedLong((char *) buffer);
  count=ReadBlob(image,12,buffer);
  image->columns=StringToUnsignedLong((char *) buffer);
  count=ReadBlob(image,200,buffer);
  count=ReadBlob(image,768,buffer);
  if (separations_mask == 0x0f)
    SetImageColorspace(image,CMYKColorspace);
  image->x_resolution=1.0*image->columns/width;
  image->y_resolution=1.0*image->rows/height;
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  /*
    Convert SCT raster image to pixel packets.
  */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    for (i=0; i < (ssize_t) separations; i++)
    {
      q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (PixelPacket *) NULL)
        break;
      indexes=GetAuthenticIndexQueue(image);
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        pixel=(Quantum) ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
        if (image->colorspace == CMYKColorspace)
          pixel=(Quantum) (QuantumRange-pixel);
        switch (i)
        {
          case 0:
          {
            SetPixelRed(q,pixel);
            SetPixelGreen(q,pixel);
            SetPixelBlue(q,pixel);
            break;
          }
          case 1:
          {
            SetPixelGreen(q,pixel);
            break;
          }
          case 2:
          {
            SetPixelBlue(q,pixel);
            break;
          }
          case 3:
          {
            if (image->colorspace == CMYKColorspace)
              SetPixelBlack(indexes+x,pixel);
            break;
          }
        }
        q++;
      }
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        break;
      if ((image->columns % 2) != 0)
        (void) ReadBlobByte(image);  /* pad */
    }
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S C T I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterSCTImage() adds attributes for the SCT image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterSCTImage method is:
%
%      size_t RegisterSCTImage(void)
%
*/
ModuleExport size_t RegisterSCTImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("SCT");
  entry->decoder=(DecodeImageHandler *) ReadSCTImage;
  entry->magick=(IsImageFormatHandler *) IsSCT;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("Scitex HandShake");
  entry->module=ConstantString("SCT");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S C T I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterSCTImage() removes format registrations made by the
%  SCT module from the list of supported formats.
%
%  The format of the UnregisterSCTImage method is:
%
%      UnregisterSCTImage(void)
%
*/
ModuleExport void UnregisterSCTImage(void)
{
  (void) UnregisterMagickInfo("SCT");
}

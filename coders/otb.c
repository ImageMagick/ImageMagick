/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                              OOO   TTTTT  BBBB                              %
%                             O   O    T    B   B                             %
%                             O   O    T    BBBB                              %
%                             O   O    T    B   B                             %
%                              OOO     T    BBBB                              %
%                                                                             %
%                                                                             %
%                      Read/Write On-The-Air Image Format                     %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                               January 2000                                  %
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
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
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
  WriteOTBImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d O T B I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadOTBImage() reads a on-the-air (level 0) bitmap and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadOTBImage method is:
%
%      Image *ReadOTBImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadOTBImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define GetBit(a,i) (((a) >> (i)) & 1L)

  Image
    *image;

  int
    byte;

  MagickBooleanType
    status;

  register ssize_t
    x;

  register Quantum
    *q;

  ssize_t
    y;

  unsigned char
    bit,
    info,
    depth;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Initialize image structure.
  */
  info=(unsigned char) ReadBlobByte(image);
  if (GetBit(info,4) == 0)
    {
      image->columns=(size_t) ReadBlobByte(image);
      image->rows=(size_t) ReadBlobByte(image);
    }
  else
    {
      image->columns=(size_t) ReadBlobMSBShort(image);
      image->rows=(size_t) ReadBlobMSBShort(image);
    }
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  depth=(unsigned char) ReadBlobByte(image);
  if (depth != 1)
    ThrowReaderException(CoderError,"OnlyLevelZerofilesSupported");
  if (AcquireImageColormap(image,2,exception) == MagickFalse)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  /*
    Convert bi-level image to pixel packets.
  */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    bit=0;
    byte=0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (bit == 0)
        {
          byte=ReadBlobByte(image);
          if (byte == EOF)
            ThrowReaderException(CorruptImageError,"CorruptImage");
        }
      SetPixelIndex(image,(byte & (0x01 << (7-bit))) ? 0x00 : 0x01,q);
      bit++;
      if (bit == 8)
        bit=0;
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    if (image->previous == (Image *) NULL)
      {
        status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
        if (status == MagickFalse)
          break;
      }
  }
  (void) SyncImage(image,exception);
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
%   R e g i s t e r O T B I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterOTBImage() adds attributes for the OTB image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterOTBImage method is:
%
%      size_t RegisterOTBImage(void)
%
*/
ModuleExport size_t RegisterOTBImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("OTB","OTB","On-the-air bitmap");
  entry->decoder=(DecodeImageHandler *) ReadOTBImage;
  entry->encoder=(EncodeImageHandler *) WriteOTBImage;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r O T B I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterOTBImage() removes format registrations made by the
%  OTB module from the list of supported formats.
%
%  The format of the UnregisterOTBImage method is:
%
%      UnregisterOTBImage(void)
%
*/
ModuleExport void UnregisterOTBImage(void)
{
  (void) UnregisterMagickInfo("OTB");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e O T B I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteOTBImage() writes an image to a file in the On-the-air Bitmap
%  (level 0) image format.
%
%  The format of the WriteOTBImage method is:
%
%      MagickBooleanType WriteOTBImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteOTBImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
#define SetBit(a,i,set) \
  a=(unsigned char) ((set) ? (a) | (1L << (i)) : (a) & ~(1L << (i)))

  MagickBooleanType
    status;

  register const Quantum
    *p;

  register ssize_t
    x;

  ssize_t
    y;

  unsigned char
    bit,
    byte,
    info;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  (void) TransformImageColorspace(image,sRGBColorspace,exception);
  /*
    Convert image to a bi-level image.
  */
  (void) SetImageType(image,BilevelType,exception);
  info=0;
  if ((image->columns >= 256) || (image->rows >= 256))
    SetBit(info,4,1);
  (void) WriteBlobByte(image,info);
  if ((image->columns >= 256) || (image->rows >= 256))
    {
      (void) WriteBlobMSBShort(image,(unsigned short) image->columns);
      (void) WriteBlobMSBShort(image,(unsigned short) image->rows);
    }
  else
    {
      (void) WriteBlobByte(image,(unsigned char) image->columns);
      (void) WriteBlobByte(image,(unsigned char) image->rows);
    }
  (void) WriteBlobByte(image,1);  /* depth */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    bit=0;
    byte=0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (GetPixelLuma(image,p) < (QuantumRange/2.0))
        byte|=0x1 << (7-bit);
      bit++;
      if (bit == 8)
        {
          (void) WriteBlobByte(image,byte);
          bit=0;
          byte=0;
        }
      p+=GetPixelChannels(image);
    }
    if (bit != 0)
      (void) WriteBlobByte(image,byte);
    if (image->previous == (Image *) NULL)
      {
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
  }
  (void) CloseBlob(image);
  return(MagickTrue);
}

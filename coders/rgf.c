/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            RRRR    GGG   FFFFF                              %
%                            R   R  G      F                                  %
%                            RRRR   G  GG  FFF                                %
%                            R  R   G   G  F                                  %
%                            R   R   GGG   F                                  %
%                                                                             %
%                                                                             %
%                  Read/Write LEGO Mindstorms EV3 Robot Graphics File         %
%                                                                             %
%                              Software Design                                %
%                               Brian Wheeler                                 %
%                               August 2013                                   %
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
#include "MagickCore/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteRGFImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d X B M I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadRGFImage() reads an RGF bitmap image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadRGFImage method is:
%
%      Image *ReadRGFImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadRGFImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  int
    bit;

  MagickBooleanType
    status;

  ssize_t
    i,
    x;

  Quantum
    *q;

  unsigned char
    *p;

  ssize_t
    y;

  unsigned char
    byte,
    *data;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Read RGF header.
  */
  image->columns = (unsigned long) ReadBlobByte(image);
  image->rows = (unsigned long) ReadBlobByte(image);
  image->depth=8;
  image->storage_class=PseudoClass;
  image->colors=2;
  /*
    Initialize image structure.
  */
  if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Initialize colormap.
  */
  image->colormap[0].red=QuantumRange;
  image->colormap[0].green=QuantumRange;
  image->colormap[0].blue=QuantumRange;
  image->colormap[1].red=(Quantum) 0;
  image->colormap[1].green=(Quantum) 0;
  image->colormap[1].blue=(Quantum) 0;
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  /*
    Read hex image data.
  */
  data=(unsigned char *) AcquireQuantumMemory(image->rows,image->columns*
    sizeof(*data));
  if (data == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  p=data;
  for (i=0; i < (ssize_t) (image->columns * image->rows); i++) 
    {
      *p++=(char) ReadBlobByte(image);
    }

  /*
    Convert RGF image to pixel packets.
  */
  p=data;
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
        byte=(size_t) (*p++);
      SetPixelIndex(image,(Quantum) ((byte & 0x01) != 0 ? 0x01 : 0x00),q);
      bit++;
      byte>>=1;
      if (bit == 8)
        bit=0;
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  data=(unsigned char *) RelinquishMagickMemory(data);
  (void) SyncImage(image,exception);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r R G F I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterRGFImage() adds attributes for the RGF image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterRGFImage method is:
%
%      size_t RegisterRGFImage(void)
%
*/
ModuleExport size_t RegisterRGFImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("RGF","RGF",
    "LEGO Mindstorms EV3 Robot Graphic Format (black and white)");
  entry->decoder=(DecodeImageHandler *) ReadRGFImage;
  entry->encoder=(EncodeImageHandler *) WriteRGFImage;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r R G F I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterRGFImage() removes format registrations made by the
%  RGF module from the list of supported formats.
%
%  The format of the UnregisterRGFImage method is:
%
%      UnregisterRGFImage(void)
%
*/
ModuleExport void UnregisterRGFImage(void)
{
  (void) UnregisterMagickInfo("RGF");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e R G F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteRGFImage() writes an image to a file in the X bitmap format.
%
%  The format of the WriteRGFImage method is:
%
%      MagickBooleanType WriteRGFImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteRGFImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  const Quantum
    *p;

  ssize_t
    x;

  size_t
    bit,
    byte;

  ssize_t
    y;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
  return(status);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
  if ((image->columns > 255L) || (image->rows > 255L))
    ThrowWriterException(ImageError,"Dimensions must be less than 255x255");
  /*
    Write header (just the image dimensions)
   */
  (void) WriteBlobByte(image,image->columns & 0xff);
  (void) WriteBlobByte(image,image->rows & 0xff);
  /*
    Convert MIFF to bit pixels.
  */
  if (IsImageMonochrome(image) == MagickFalse)
    (void) SetImageType(image,BilevelType,exception);
  x=0;
  y=0;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    bit=0;
    byte=0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      byte>>=1;
      if (GetPixelLuma(image,p) < ((double) QuantumRange/2.0))
        byte|=0x80;
      bit++;
      if (bit == 8)
        {
          /*
            Write a bitmap byte to the image file.
          */
          (void) WriteBlobByte(image,(unsigned char) byte);
          bit=0;
          byte=0;
        }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (bit != 0)
    {
      byte >>= 8 - bit;
      (void) WriteBlobByte(image,(unsigned char) byte);
    }
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

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
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
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
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/utility.h"

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

  MagickBooleanType
    status;

  register IndexPacket
    *indexes;

  register PixelPacket
    *q;

  register ssize_t
    i,
    x;

  register unsigned char
    *p;

  size_t
    bit,
    byte;

  ssize_t
    y;

  unsigned char
    *data;


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
  if (AcquireImageColormap(image,image->colors) == MagickFalse)
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
      *p++=ReadBlobByte(image);
    }

  /*
    Convert RGF image to pixel packets.
  */
  p=data;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    bit=0;
    byte=0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (bit == 0)
        byte=(size_t) (*p++);
      SetPixelIndex(indexes+x,(Quantum) ((byte & 0x01) != 0 ? 0x01 : 0x00));
      bit++;
      byte>>=1;
      if (bit == 8)
        bit=0;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  data=(unsigned char *) RelinquishMagickMemory(data);
  (void) SyncImage(image);
  (void) CloseBlob(image);
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

  entry=SetMagickInfo("RGF");
  entry->decoder=(DecodeImageHandler *) ReadRGFImage;
  entry->encoder=(EncodeImageHandler *) WriteRGFImage;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString(
    "LEGO Mindstorms EV3 Robot Graphic Format (black and white)");
  entry->module=ConstantString("RGF");
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

  register const PixelPacket
    *p;

  register ssize_t
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
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace);
  if((image->columns > 255L) || (image->rows > 255L))
    ThrowWriterException(ImageError,"Dimensions must be less than 255x255");

  /*
    Write header (just the image dimensions)
   */
  (void) WriteBlobByte(image,image->columns & 0xff);
  (void) WriteBlobByte(image,image->rows & 0xff);

  /*
    Convert MIFF to bit pixels.
  */
  (void) SetImageType(image,BilevelType);
  x=0;
  y=0;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    bit=0;
    byte=0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      byte>>=1;
      if (GetPixelLuma(image,p) < (QuantumRange/2))
        byte|=0x80;
      bit++;
      if (bit == 8)
        {
          /*
            Write a bitmap byte to the image file.
          */
       	  (void) WriteBlobByte(image,byte);
          bit=0;
          byte=0;
        }
      p++;
    }
    if (bit != 0)
      (void) WriteBlobByte(image,byte);
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  (void) CloseBlob(image);
  return(MagickTrue);
}

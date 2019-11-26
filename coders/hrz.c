/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            H   H  RRRR   ZZZZZ                              %
%                            H   H  R   R     ZZ                              %
%                            HHHHH  RRRR     Z                                %
%                            H   H  R R    ZZ                                 %
%                            H   H  R  R   ZZZZZ                              %
%                                                                             %
%                                                                             %
%                Read/Write Slow Scan TeleVision Image Format                 %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
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
  WriteHRZImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d H R Z I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadHRZImage() reads a Slow Scan TeleVision image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadHRZImage method is:
%
%      Image *ReadHRZImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadHRZImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  register ssize_t
    x;

  register Quantum
    *q;

  register unsigned char
    *p;

  ssize_t
    count,
    y;

  size_t
    length;

  unsigned char
    *pixels;

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
    Convert HRZ raster image to pixel packets.
  */
  image->columns=256;
  image->rows=240;
  image->depth=8;
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  pixels=(unsigned char *) AcquireQuantumMemory(image->columns,3*
    sizeof(*pixels));
  if (pixels == (unsigned char *) NULL) 
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  length=(size_t) (3*image->columns);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    count=ReadBlob(image,length,pixels);
    if ((size_t) count != length)
      {
        pixels=(unsigned char *) RelinquishMagickMemory(pixels);
        ThrowReaderException(CorruptImageError,"UnableToReadImageData");
      }
    p=pixels;
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelRed(image,ScaleCharToQuantum(4**p++),q);
      SetPixelGreen(image,ScaleCharToQuantum(4**p++),q);
      SetPixelBlue(image,ScaleCharToQuantum(4**p++),q);
      SetPixelAlpha(image,OpaqueAlpha,q);
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
      break;
  }
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
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
%   R e g i s t e r H R Z I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterHRZImage() adds attributes for the HRZ X image format to the list
%  of supported formats.  The attributes include the image format tag, a
%  method to read and/or write the format, whether the format supports the
%  saving of more than one frame to the same file or blob, whether the format
%  supports native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterHRZImage method is:
%
%      size_t RegisterHRZImage(void)
%
*/
ModuleExport size_t RegisterHRZImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("HRZ","HRZ","Slow Scan TeleVision");
  entry->decoder=(DecodeImageHandler *) ReadHRZImage;
  entry->encoder=(EncodeImageHandler *) WriteHRZImage;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r H R Z I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterHRZImage() removes format registrations made by the
%  HRZ module from the list of supported formats.
%
%  The format of the UnregisterHRZImage method is:
%
%      UnregisterHRZImage(void)
%
*/
ModuleExport void UnregisterHRZImage(void)
{
  (void) UnregisterMagickInfo("HRZ");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e H R Z I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteHRZImage() writes an image to a file in HRZ X image format.
%
%  The format of the WriteHRZImage method is:
%
%      MagickBooleanType WriteHRZImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteHRZImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  Image
    *hrz_image;

  MagickBooleanType
    status;

  register const Quantum
    *p;

  register ssize_t
    x,
    y;

  register unsigned char
    *q;

  ssize_t
    count;

  unsigned char
    *pixels;

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
  hrz_image=ResizeImage(image,256,240,image->filter,exception);
  if (hrz_image == (Image *) NULL)
    return(MagickFalse);
  (void) TransformImageColorspace(hrz_image,sRGBColorspace,exception);
  /*
    Allocate memory for pixels.
  */
  pixels=(unsigned char *) AcquireQuantumMemory((size_t) hrz_image->columns,
    3*sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    {
      hrz_image=DestroyImage(hrz_image);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Convert MIFF to HRZ raster pixels.
  */
  for (y=0; y < (ssize_t) hrz_image->rows; y++)
  {
    p=GetVirtualPixels(hrz_image,0,y,hrz_image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    q=pixels;
    for (x=0; x < (ssize_t) hrz_image->columns; x++)
    {
      *q++=ScaleQuantumToChar(GetPixelRed(hrz_image,p)/4);
      *q++=ScaleQuantumToChar(GetPixelGreen(hrz_image,p)/4);
      *q++=ScaleQuantumToChar(GetPixelBlue(hrz_image,p)/4);
      p+=GetPixelChannels(hrz_image);
    }
    count=WriteBlob(image,(size_t) (q-pixels),pixels);
    if (count != (ssize_t) (q-pixels))
      break;
    status=SetImageProgress(image,SaveImageTag,y,hrz_image->rows);
    if (status == MagickFalse)
      break;
  }
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  hrz_image=DestroyImage(hrz_image);
  (void) CloseBlob(image);
  return(MagickTrue);
}

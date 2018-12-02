/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        CCCC  L      IIIII  PPPP   BBBB    OOO    AAA   RRRR   DDDD          %
%       C      L        I    P   P  B   B  O   O  A   A  R   R  D   D         %
%       C      L        I    PPP    BBBB   O   O  AAAAA  RRRR   D   D         %
%       C      L        I    P      B   B  O   O  A   A  R R    D   D         %
%        CCCC  LLLLL  IIIII  P      BBBB    OOO   A   A  R  R   DDDD          %
%                                                                             %
%                                                                             %
%                        Read/Write Windows Clipboard.                        %
%                                                                             %
%                              Software Design                                %
%                             Leonard Rosenthol                               %
%                                 May 2002                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
#  if defined(__CYGWIN__)
#    include <windows.h>
#  else
     /* All MinGW needs ... */
#    include "MagickCore/nt-base-private.h"
#    include <wingdi.h>
#  endif
#endif
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/nt-feature.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

#define BMP_HEADER_SIZE 14

/*
  Forward declarations.
*/
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
static MagickBooleanType
  WriteCLIPBOARDImage(const ImageInfo *,Image *,ExceptionInfo *);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d C L I P B O A R D I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadCLIPBOARDImage() reads an image from the system clipboard and returns
%  it.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadCLIPBOARDImage method is:
%
%      Image *ReadCLIPBOARDImage(const ImageInfo *image_info,
%        ExceptionInfo exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
static Image *ReadCLIPBOARDImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  unsigned char
    *p;

  HANDLE
    clip_handle;

  Image
    *image;

  ImageInfo
    *read_info;

  LPVOID
    clip_mem;

  MagickBooleanType
    status;

  register ssize_t
    x;

  register Quantum
    *q;

  size_t
    clip_size,
    total_size;

  ssize_t
    y;

  unsigned int
    offset;

  void
    *clip_data;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  if (!IsClipboardFormatAvailable(CF_BITMAP) &&
      !IsClipboardFormatAvailable(CF_DIB) &&
      !IsClipboardFormatAvailable(CF_DIBV5))
    ThrowReaderException(CoderError,"NoBitmapOnClipboard");
  if (!OpenClipboard(NULL))
    ThrowReaderException(CoderError,"UnableToReadImageData");
  clip_handle=GetClipboardData(CF_DIBV5);
  if (!clip_handle)
    clip_handle=GetClipboardData(CF_DIB);
  if ((clip_handle == NULL) || (clip_handle == INVALID_HANDLE_VALUE))
    {
      CloseClipboard();
      ThrowReaderException(CoderError,"UnableToReadImageData");
    }
  clip_size=(size_t) GlobalSize(clip_handle);
  total_size=clip_size+BMP_HEADER_SIZE;
  clip_data=AcquireMagickMemory(total_size);
  if (clip_data == (void *) NULL)
    {
      CloseClipboard();
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  clip_mem=GlobalLock(clip_handle);
  if (clip_mem == (LPVOID) NULL)
    {
      CloseClipboard();
      ThrowReaderException(CoderError,"UnableToReadImageData");
    }
  p=(unsigned char *) clip_data;
  p+=BMP_HEADER_SIZE;
  (void) memcpy(p,clip_mem,clip_size);
  (void) GlobalUnlock(clip_mem);
  (void) CloseClipboard();
  memset(clip_data,0,BMP_HEADER_SIZE);
  offset=((unsigned int) p[0])+BMP_HEADER_SIZE;
  p-=BMP_HEADER_SIZE;
  p[0]='B';
  p[1]='M';
  p[2]=(unsigned char) total_size;
  p[3]=(unsigned char) (total_size >> 8);
  p[4]=(unsigned char) (total_size >> 16);
  p[5]=(unsigned char) (total_size >> 24);
  p[10]=offset;
  read_info=CloneImageInfo(image_info);
  (void) CopyMagickString(read_info->magick,"BMP",MaxTextExtent);
  image=BlobToImage(read_info,clip_data,total_size,exception);
  read_info=DestroyImageInfo(read_info);
  clip_data=RelinquishMagickMemory(clip_data);
  return(image);
}
#endif /* MAGICKCORE_WINGDI32_DELEGATE */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r C L I P B O A R D I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterCLIPBOARDImage() adds attributes for the clipboard "image format" to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterCLIPBOARDImage method is:
%
%      size_t RegisterCLIPBOARDImage(void)
%
*/
ModuleExport size_t RegisterCLIPBOARDImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("CLIPBOARD","CLIPBOARD","The system clipboard");
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadCLIPBOARDImage;
  entry->encoder=(EncodeImageHandler *) WriteCLIPBOARDImage;
#endif
  entry->flags^=CoderAdjoinFlag;
  entry->format_type=ImplicitFormatType;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r C L I P B O A R D I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterCLIPBOARDImage() removes format registrations made by the
%  RGB module from the list of supported formats.
%
%  The format of the UnregisterCLIPBOARDImage method is:
%
%      UnregisterCLIPBOARDImage(void)
%
*/
ModuleExport void UnregisterCLIPBOARDImage(void)
{
  (void) UnregisterMagickInfo("CLIPBOARD");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e C L I P B O A R D I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteCLIPBOARDImage() writes an image to the system clipboard.
%
%  The format of the WriteCLIPBOARDImage method is:
%
%      MagickBooleanType WriteCLIPBOARDImage(const ImageInfo *image_info,
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
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
static MagickBooleanType WriteCLIPBOARDImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  HANDLE
    clip_handle;

  ImageInfo
    *write_info;

  LPVOID
    clip_mem;

  size_t
    length;

  unsigned char
    *p;

  void
    *clip_data;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    ThrowWriterException(CoderError,"UnableToWriteImageData");
  write_info=CloneImageInfo(image_info);
  if (image->alpha_trait == UndefinedPixelTrait)
    (void) CopyMagickString(write_info->magick,"BMP3",MaxTextExtent);
  else
    (void) CopyMagickString(write_info->magick,"BMP",MaxTextExtent);
  clip_data=ImageToBlob(write_info,image,&length,exception);
  write_info=DestroyImageInfo(write_info);
  if (clip_data == (void *) NULL)
    ThrowWriterException(CoderError,"UnableToWriteImageData");
  clip_handle=(HANDLE) GlobalAlloc(GMEM_MOVEABLE,length-BMP_HEADER_SIZE);
  if (clip_handle == (HANDLE) NULL)
    {
      clip_data=RelinquishMagickMemory(clip_data);
      ThrowWriterException(CoderError,"UnableToWriteImageData");
    }
  clip_mem=GlobalLock(clip_handle);
  if (clip_handle == (LPVOID) NULL)
    {
      (void) GlobalFree((HGLOBAL) clip_handle);
      clip_data=RelinquishMagickMemory(clip_data);
      ThrowWriterException(CoderError,"UnableToWriteImageData");
    }
  p=(unsigned char *) clip_data;
  p+=BMP_HEADER_SIZE;
  (void) memcpy(clip_mem,p,length-BMP_HEADER_SIZE);
  (void) GlobalUnlock(clip_mem);
  clip_data=RelinquishMagickMemory(clip_data);
  if (!OpenClipboard(NULL))
    {
      (void) GlobalFree((HGLOBAL) clip_handle);
      ThrowWriterException(CoderError,"UnableToWriteImageData");
    }
  (void) EmptyClipboard();
  if (image->alpha_trait == UndefinedPixelTrait)
    SetClipboardData(CF_DIB,clip_handle);
  else
    SetClipboardData(CF_DIBV5,clip_handle);
  (void) CloseClipboard();
  return(MagickTrue);
}
#endif /* MAGICKCORE_WINGDI32_DELEGATE */

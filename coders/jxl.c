/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                              JJJ  X   X  L                                  %
%                               J    X X   L                                  %
%                               J     X    L                                  %
%                            J  J    X X   L                                  %
%                             JJ    X   X  LLLLL                              %
%                                                                             %
%                                                                             %
%               Read/Write JPEG XL Lossless JPEG1 Recompression               %
%                                                                             %
%                            The JPEG XL Project                              %
%                               September 2019                                %
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
#include "MagickCore/attribute.h"
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
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#if defined(MAGICKCORE_JXL_DELEGATE)
#include <brunsli/decode.h>
#include <brunsli/encode.h>

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteJXLImage(const ImageInfo *,Image *);
#endif

#if defined(MAGICKCORE_JXL_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d J X L I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadJXLImage() reads a JXL image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadJXLImage method is:
%
%      Image *ReadJXLImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

/*
  Typedef declarations.
*/
typedef struct OutBuffer
{
  unsigned char
    *data;

  size_t
    size;
} OutBuffer;

static size_t AllocOutput(void* data, const uint8_t* buf, size_t count)
{
  OutBuffer *buffer=(OutBuffer *) data;
  buffer->data=ResizeMagickMemory(buffer->data,buffer->size+count);
  if (!buffer->data) return 0;
  memcpy(buffer->data+buffer->size, buf, count);
  buffer->size+=count;
  return(count);
}

static Image *ReadJXLImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *temp_image,
    *result;

  MagickBooleanType
    status;

  OutBuffer
    b;

  unsigned char
    *jxl;

  /*
    TODO: do we need an Image here? No pixels are needed, but OpenBlob
    needs an Image.
  */
  temp_image=AcquireImage(image_info, exception);
  status=OpenBlob(image_info,temp_image,ReadBinaryBlobMode,exception);
  jxl=NULL;
  size_t jxlsize = 0;
  if (status == MagickTrue)
  {
    jxlsize=(size_t) GetBlobSize(temp_image);
    jxl=(unsigned char *) AcquireMagickMemory(jxlsize);
    size_t num_read=ReadBlob(temp_image,jxlsize,jxl);
    if (num_read != jxlsize) status=MagickFalse;
  }
  (void) DestroyImage(temp_image);

  b.data=NULL;
  b.size=0;

  if (status == MagickTrue)
  {
    status=DecodeBrunsli(jxlsize,jxl,&b,AllocOutput) == 1 ?
        MagickTrue : MagickFalse;
  }
  (void) RelinquishMagickMemory(jxl);

  result=NULL;

  if (status == MagickTrue)
  {
    ImageInfo* temp_info=AcquireImageInfo();
    SetImageInfoBlob(temp_info,b.data,b.size);
    result=BlobToImage(temp_info,b.data,b.size,exception);
    (void) DestroyImageInfo(temp_info);
  }
  (void) RelinquishMagickMemory(b.data);

  return(result);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r J X L I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterJXLImage() adds properties for the JXL image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterJXLImage method is:
%
%      size_t RegisterJXLImage(void)
%
*/
ModuleExport size_t RegisterJXLImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("JXL", "JXL", "JPEG XL Lossless JPEG1 Recompression");
#if defined(MAGICKCORE_JXL_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJXLImage;
  entry->encoder=(EncodeImageHandler *) WriteJXLImage;
#endif
  entry->note=ConstantString(
    "JPEG1 recompression as specified in https://arxiv.org/pdf/1908.03565.pdf"
    " page 135. Full JPEG XL support will be implemented in this coder later.");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r J X L I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterJXLImage() removes format registrations made by the
%  JXL module from the list of supported formats.
%
%  The format of the UnregisterJXLImage method is:
%
%      UnregisterJXLImage(void)
%
*/
ModuleExport void UnregisterJXLImage(void)
{
  (void) UnregisterMagickInfo("JXL");
}

#if defined(MAGICKCORE_JXL_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  W r i t e J X L I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteJXLImage() writes a JXL image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the WriteJXLImage method is:
%
%      MagickBooleanType WriteJXLImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%
*/
static MagickBooleanType WriteJXLImage(const ImageInfo *image_info,
    Image *image)
{
  ExceptionInfo
    *exception;

  Image
    *temp_image;

  ImageInfo
    *temp_info;

  MagickBooleanType
    status;

  OutBuffer
    b;

  size_t
    jpegsize;

  unsigned char
    *jpeg;

  exception=AcquireExceptionInfo();

  /*
    TODO: can cloning the image be avoided? The pixels don't need to be cloned,
    only filename or blob information. ImageToBlob overwrites this information.
  */
  temp_image=CloneImage(image, 0, 0, MagickTrue, exception);
  temp_info=AcquireImageInfo();
  (void) CopyMagickString(temp_image->magick,"JPG",MaxTextExtent);
  jpeg=ImageToBlob(temp_info,temp_image,&jpegsize,exception);
  (void) DestroyImage(temp_image);
  (void) DestroyImageInfo(temp_info);

  b.data=NULL;
  b.size=0;
  status=EncodeBrunsli(jpegsize,jpeg,&b,AllocOutput) == 1 ?
      MagickTrue : MagickFalse;
  if (status == MagickTrue)
  {
    status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  }

  if (status == MagickTrue)
  {
    WriteBlob(image,b.size,b.data);
    CloseBlob(image);
  }
  (void) RelinquishMagickMemory(b.data);

  if(exception) exception=DestroyExceptionInfo(exception);
  return(status);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                              JJJ  N   N  X   X                              %
%                               J   NN  N   X X                               %
%                               J   N N N    X                                %
%                            J  J   N  NN   X X                               %
%                             JJ    N   N  X   X                              %
%                                                                             %
%                                                                             %
%                       Read/Write Garmin Image Format                        %
%                                                                             %
%                                   Cristy                                    %
%                                 July 2012                                   %
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
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/draw.h"
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
#include "MagickCore/property.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"

typedef struct _JNXInfo
{
  int
    version,
    serial;

  PointInfo
    northeast,
    southwest;

  int
    levels,
    expire,
    id,
    crc,
    signature;

  unsigned int
    offset;

  int
    order;
} JNXInfo;

typedef struct _JNXLevelInfo
{
  int
    count,
    offset;
} JNXLevelInfo;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d J N X I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadJNXImage() reads an image in the Garmin tile storage format and returns
%  it.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadJNXImage method is:
%
%      Image *ReadJNXImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadJNXImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define JNXMaxLevels  20

  Image
    *image,
    *images;

  JNXInfo
    jnx_info;

  JNXLevelInfo
    jnx_level_info[JNXMaxLevels];

  MagickBooleanType
    status;

  MagickSizeType
    num_images;

  ssize_t
    i;

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
  image->columns=0;
  image->rows=0;
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Read JNX header.
  */
  (void) memset(&jnx_info,0,sizeof(jnx_info));
  jnx_info.version=ReadBlobLSBSignedLong(image);
  if ((jnx_info.version != 3) && (jnx_info.version != 4))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  jnx_info.serial=ReadBlobLSBSignedLong(image);
  jnx_info.northeast.x=180.0*ReadBlobLSBSignedLong(image)/0x7fffffff;
  jnx_info.northeast.y=180.0*ReadBlobLSBSignedLong(image)/0x7fffffff;
  jnx_info.southwest.x=180.0*ReadBlobLSBSignedLong(image)/0x7fffffff;
  jnx_info.southwest.y=180.0*ReadBlobLSBSignedLong(image)/0x7fffffff;
  jnx_info.levels=ReadBlobLSBSignedLong(image);
  if (jnx_info.levels > JNXMaxLevels)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  jnx_info.expire=ReadBlobLSBSignedLong(image);
  jnx_info.id=ReadBlobLSBSignedLong(image);
  jnx_info.crc=ReadBlobLSBSignedLong(image);
  jnx_info.signature=ReadBlobLSBSignedLong(image);
  jnx_info.offset=ReadBlobLSBLong(image);
  if (jnx_info.version > 3)
    jnx_info.order=ReadBlobLSBSignedLong(image);
  else
    if (jnx_info.version == 3)
      jnx_info.order=30;
  if (EOFBlob(image) != MagickFalse)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
    Read JNX levels.
  */
  num_images=0;
  (void) memset(&jnx_level_info,0,sizeof(jnx_level_info));
  for (i=0; i < (ssize_t) jnx_info.levels; i++)
  {
    jnx_level_info[i].count=ReadBlobLSBSignedLong(image);
    if (jnx_level_info[i].count > 50000)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    jnx_level_info[i].offset=ReadBlobLSBSignedLong(image);
    /* scale */
    (void) ReadBlobLSBLong(image);
    if (jnx_info.version > 3)
      {
        /* copyright */
        (void) ReadBlobLSBLong(image);
        while (ReadBlobLSBShort(image) != 0);
      }
    if (EOFBlob(image) != MagickFalse)
      ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
    num_images+=(MagickSizeType) jnx_level_info[i].count;
  }
  if (AcquireMagickResource(ListLengthResource,num_images) == MagickFalse)
    ThrowReaderException(ResourceLimitError,"ListLengthExceedsLimit");
  /*
    Read JNX tiles.
  */
  images=NewImageList();
  for (i=0; i < (ssize_t) jnx_info.levels; i++)
  {
    MagickOffsetType
      offset;

    ssize_t
      j;

    offset=SeekBlob(image,(MagickOffsetType) jnx_level_info[i].offset,SEEK_SET);
    if (offset != (MagickOffsetType) jnx_level_info[i].offset)
      continue;
    for (j=0; j < (ssize_t) jnx_level_info[i].count; j++)
    {
      Image
        *tile_image;

      ImageInfo
        *read_info;

      int
        tile_offset;

      MagickOffsetType
        restore_offset;

      PointInfo
        northeast,
        southwest;

      ssize_t
        count;

      unsigned char
        *blob;

      unsigned int
        tile_length;

      northeast.x=180.0*ReadBlobLSBSignedLong(image)/0x7fffffff;
      northeast.y=180.0*ReadBlobLSBSignedLong(image)/0x7fffffff;
      southwest.x=180.0*ReadBlobLSBSignedLong(image)/0x7fffffff;
      southwest.y=180.0*ReadBlobLSBSignedLong(image)/0x7fffffff;
      (void) ReadBlobLSBShort(image); /* width */
      (void) ReadBlobLSBShort(image); /* height */
      if (EOFBlob(image) != MagickFalse)
        {
          images=DestroyImageList(images);
          ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        }
      tile_length=ReadBlobLSBLong(image);
      tile_offset=ReadBlobLSBSignedLong(image);
      if (tile_offset == -1)
        continue;
      restore_offset=TellBlob(image);
      if (restore_offset < 0)
        continue;
      offset=SeekBlob(image,(MagickOffsetType) tile_offset,SEEK_SET);
      if (offset != (MagickOffsetType) tile_offset)
        continue;
      /*
        Read a tile.
      */
      if (((MagickSizeType) tile_length) > GetBlobSize(image))
        {
          images=DestroyImageList(images);
          ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
        }
      blob=(unsigned char *) AcquireQuantumMemory((size_t) tile_length+2,
        sizeof(*blob));
      if (blob == (unsigned char *) NULL)
        {
          images=DestroyImageList(images);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      blob[0]=0xFF;
      blob[1]=0xD8;
      count=ReadBlob(image,tile_length,blob+2);
      if (count != (ssize_t) tile_length)
        {
          images=DestroyImageList(images);
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        }
      read_info=CloneImageInfo(image_info);
      (void) CopyMagickString(read_info->magick,"JPEG",MagickPathExtent);
      tile_image=BlobToImage(read_info,blob,tile_length+2,exception);
      read_info=DestroyImageInfo(read_info);
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      offset=SeekBlob(image,restore_offset,SEEK_SET);
      if (tile_image == (Image *) NULL)
        continue;
      tile_image->depth=8;
      (void) CopyMagickString(tile_image->magick,image->magick,
        MagickPathExtent);
      (void) FormatImageProperty(tile_image,"jnx:northeast","%.20g,%.20g",
        northeast.x,northeast.y);
      (void) FormatImageProperty(tile_image,"jnx:southwest","%.20g,%.20g",
        southwest.x,southwest.y);
      AppendImageToList(&images,tile_image);
    }
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType) i,
          (MagickSizeType) jnx_info.levels);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  (void) CloseBlob(image);
  image=DestroyImage(image);
  return(GetFirstImageInList(images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r J N X I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterJNXImage() adds attributes for the JNX image format to the list
%  of supported formats.  The attributes include the image format tag, a
%  method to read and/or write the format, whether the format supports the
%  saving of more than one frame to the same file or blob, whether the format
%  supports native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterJNXImage method is:
%
%      size_t RegisterJNXImage(void)
%
*/
ModuleExport size_t RegisterJNXImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("JNX","JNX","Garmin tile format");
  entry->decoder=(DecodeImageHandler *) ReadJNXImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r J N X I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterJNXImage() removes format registrations made by the
%  JNX module from the list of supported formats.
%
%  The format of the UnregisterJNXImage method is:
%
%      UnregisterJNXImage(void)
%
*/
ModuleExport void UnregisterJNXImage(void)
{
  (void) UnregisterMagickInfo("JNX");
}

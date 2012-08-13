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
%                                John Cristy                                  %
%                                 July 2012                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/static.h"
#include "MagickCore/string_.h"

typedef struct _JNXInfo
{
  unsigned int
    version,
    serial;

  PointInfo
    northeast,
    southwest;

  unsigned int
    levels,
    expire,
    id,
    crc,
    signature;

  int
    offset;

  unsigned int
    order;
} JNXInfo;

typedef struct _JNXLevelInfo
{
  unsigned int
    count;

  int
    offset;

  unsigned int
    scale;

  char
    copyright[MaxTextExtent];
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

  register ssize_t
    i;

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
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Read JNX header.
  */
  (void) ResetMagickMemory(&jnx_info,0,sizeof(jnx_info));
  jnx_info.version=ReadBlobLSBLong(image);
  if (jnx_info.version > 4)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  jnx_info.serial=ReadBlobLSBLong(image);
  jnx_info.northeast.x=(double) ReadBlobLSBLong(image);
  jnx_info.northeast.y=(double) ReadBlobLSBLong(image);
  jnx_info.southwest.x=(double) ReadBlobLSBLong(image);
  jnx_info.southwest.y=(double) ReadBlobLSBLong(image);
  jnx_info.levels=ReadBlobLSBLong(image);
  if (jnx_info.levels > JNXMaxLevels)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  jnx_info.expire=ReadBlobLSBLong(image);
  jnx_info.id=ReadBlobLSBLong(image);
  jnx_info.crc=ReadBlobLSBLong(image);
  jnx_info.signature=ReadBlobLSBLong(image);
  jnx_info.offset=ReadBlobLSBLong(image);
  if (jnx_info.version > 3)
    jnx_info.order=(int) ReadBlobLSBLong(image);
  /*
    Read JNX levels.
  */
  (void) ResetMagickMemory(&jnx_level_info,0,sizeof(jnx_level_info));
  for (i=0; i < (ssize_t) jnx_info.levels; i++)
  {
    jnx_level_info[i].count=ReadBlobLSBLong(image);
    jnx_level_info[i].offset=(int) ReadBlobLSBLong(image);
    jnx_level_info[i].scale=(int) ReadBlobLSBLong(image);
    if (jnx_info.version > 3)
      {
        int
          c;

        register ssize_t
          j;

        (void) ReadBlobLSBLong(image);
        j=0;
        while ((c=ReadBlobLSBShort(image)) != 0)
          if (j < (MaxTextExtent-1))
            jnx_level_info[i].copyright[j++]=c;
        jnx_level_info[i].copyright[j]='\0';
      }
  }
  /*
    Read JNX tiles.
  */
  images=NewImageList();
  for (i=0; i < (ssize_t) jnx_info.levels; i++)
  {
    register ssize_t
      j;

    (void) SeekBlob(image,jnx_level_info[i].offset,SEEK_SET);
    for (j=0; j < (ssize_t) jnx_level_info[i].count; j++)
    {
      Image
        *tile_image;

      ImageInfo
        *read_info;

      int
        tile_offset;

      MagickOffsetType
        offset;

      PointInfo
        northeast,
        southwest;

      ssize_t
        count;

      unsigned char
        *blob;

      unsigned int
        tile_length;

      northeast.x=(double) ReadBlobLSBLong(image);
      northeast.y=(double) ReadBlobLSBLong(image);
      southwest.x=(double) ReadBlobLSBLong(image);
      southwest.y=(double) ReadBlobLSBLong(image);
      (void) ReadBlobLSBShort(image); /* width */
      (void) ReadBlobLSBShort(image); /* height */
      tile_length=ReadBlobLSBLong(image);
      tile_offset=(int) ReadBlobLSBLong(image);
      offset=TellBlob(image);
      /*
        Read a tile.
      */
      blob=(unsigned char *) AcquireMagickMemory((tile_length+2)*sizeof(*blob));
      if (blob == (unsigned char *) NULL)
        {
          if (images != (Image *) NULL)
            images=DestroyImageList(images);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      (void) SeekBlob(image,tile_offset,SEEK_SET);
      blob[0]=0xFF;
      blob[1]=0xD8;
      count=ReadBlob(image,tile_length,blob+2);
      if (count != tile_length)
        {
          if (images != (Image *) NULL)
            images=DestroyImageList(images);
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        }
      read_info=CloneImageInfo(image_info);
      (void) CopyMagickString(read_info->magick,"JPEG",MaxTextExtent);
      tile_image=BlobToImage(read_info,blob,tile_length+2,exception);
      read_info=DestroyImageInfo(read_info);
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      (void) SeekBlob(image,offset,SEEK_SET);
      if (tile_image == (Image *) NULL)
        continue;
      if (*jnx_level_info[i].copyright != '\0')
        (void) SetImageProperty(tile_image,"jnx:copyright",
          jnx_level_info[i].copyright,exception);
      (void) FormatImageProperty(tile_image,"jnx:northeast","%g,%g",northeast.x,
        northeast.y);
      (void) FormatImageProperty(tile_image,"jnx:southwest","%g,%g",southwest.x,
        southwest.y);
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
  if (images == (Image *) NULL)
    return((Image *) NULL);
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

  entry=SetMagickInfo("JNX");
  entry->decoder=(DecodeImageHandler *) ReadJNXImage;
  entry->description=ConstantString("Garmin tile storage format");
  entry->seekable_stream=MagickTrue;
  entry->module=ConstantString("JNX");
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            EEEEE  PPPP   TTTTT                              %
%                            E      P   P    T                                %
%                            EEE    PPPP     T                                %
%                            E      P        T                                %
%                            EEEEE  P        T                                %
%                                                                             %
%                                                                             %
%           Read/Write Encapsulated Postscript Format (with preview).         %
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
#include "MagickCore/color.h"
#include "MagickCore/constitute.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/delegate.h"
#include "MagickCore/geometry.h"
#include "MagickCore/histogram.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/quantize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/resize.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"

/*
  Typedef declarations.
*/
typedef struct _EPTInfo
{
  size_t
    magick;

  MagickOffsetType
    postscript_offset,
    tiff_offset;

  size_t
    postscript_length,
    tiff_length;

  unsigned char
    *postscript,
    *tiff;
} EPTInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteEPTImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s E P T                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsEPT() returns MagickTrue if the image format type, identified by the
%  magick string, is EPT.
%
%  The format of the IsEPT method is:
%
%      MagickBooleanType IsEPT(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsEPT(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\305\320\323\306",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d E P T I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadEPTImage() reads a binary Postscript image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadEPTImage method is:
%
%      Image *ReadEPTImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadEPTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  const void
    *postscript_data,
    *tiff_data;

  EPTInfo
    ept_info;

  Image
    *image,
    *tiff_image;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  ssize_t
    count;

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
  ept_info.magick=ReadBlobLSBLong(image);
  if (ept_info.magick != 0xc6d3d0c5ul)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  ept_info.postscript_offset=(MagickOffsetType) ReadBlobLSBLong(image);
  ept_info.postscript_length=ReadBlobLSBLong(image);
  if ((MagickSizeType) ept_info.postscript_length > GetBlobSize(image))
    ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
  (void) ReadBlobLSBLong(image);
  (void) ReadBlobLSBLong(image);
  ept_info.tiff_offset=(MagickOffsetType) ReadBlobLSBLong(image);
  ept_info.tiff_length=ReadBlobLSBLong(image);
  if ((MagickSizeType) ept_info.tiff_length > GetBlobSize(image))
    ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
  (void) ReadBlobLSBShort(image);
  ept_info.postscript=(unsigned char *) AcquireQuantumMemory(
    ept_info.postscript_length+1,sizeof(*ept_info.postscript));
  if (ept_info.postscript == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  (void) memset(ept_info.postscript,0,(ept_info.postscript_length+1)*
    sizeof(*ept_info.postscript));
  ept_info.tiff=(unsigned char *) AcquireQuantumMemory(ept_info.tiff_length+1,
    sizeof(*ept_info.tiff));
  if (ept_info.tiff == (unsigned char *) NULL)
    {
      ept_info.postscript=(unsigned char *) RelinquishMagickMemory(
        ept_info.postscript);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  (void) memset(ept_info.tiff,0,(ept_info.tiff_length+1)*
    sizeof(*ept_info.tiff));
  offset=SeekBlob(image,ept_info.tiff_offset,SEEK_SET);
  if ((ept_info.tiff_length != 0) && (offset < 30))
    {
      ept_info.tiff=(unsigned char *) RelinquishMagickMemory(ept_info.tiff);
      ept_info.postscript=(unsigned char *) RelinquishMagickMemory(
        ept_info.postscript);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  tiff_data=ReadBlobStream(image,ept_info.tiff_length,ept_info.tiff,&count);
  if (count != (ssize_t) (ept_info.tiff_length))
    (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageWarning,
      "InsufficientImageDataInFile","`%s'",image->filename);
  offset=SeekBlob(image,ept_info.postscript_offset,SEEK_SET);
  if ((ept_info.postscript_length != 0) && (offset < 30))
    {
      ept_info.tiff=(unsigned char *) RelinquishMagickMemory(ept_info.tiff);
      ept_info.postscript=(unsigned char *) RelinquishMagickMemory(
        ept_info.postscript);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  postscript_data=ReadBlobStream(image,ept_info.postscript_length,
    ept_info.postscript,&count);
  if (count != (ssize_t) (ept_info.postscript_length))
    (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageWarning,
      "InsufficientImageDataInFile","`%s'",image->filename);
  (void) CloseBlob(image);
  image=DestroyImage(image);
  read_info=CloneImageInfo(image_info);
  read_info->number_scenes=1;
  read_info->scene=0;
  (void) CopyMagickString(read_info->magick,"EPS",MagickPathExtent);
  image=BlobToImage(read_info,postscript_data,ept_info.postscript_length,
    exception);
  if (image != (Image *) NULL)
    {
      (void) CopyMagickString(image->filename,image_info->filename,
        MagickPathExtent);
      (void) CopyMagickString(image->magick,"EPT",MagickPathExtent);
    }
  (void) CopyMagickString(read_info->magick,"TIFF",MagickPathExtent);
  tiff_image=BlobToImage(read_info,tiff_data,ept_info.tiff_length,exception);
  if (tiff_image != (Image *) NULL)
    {
      if (image == (Image *) NULL)
        image=tiff_image;
      else
        AppendImageToList(&image,tiff_image);
    }
  read_info=DestroyImageInfo(read_info);
  ept_info.tiff=(unsigned char *) RelinquishMagickMemory(ept_info.tiff);
  ept_info.postscript=(unsigned char *) RelinquishMagickMemory(
    ept_info.postscript);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r E P T I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterEPTImage() adds attributes for the EPT image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterEPTImage method is:
%
%      size_t RegisterEPTImage(void)
%
*/
ModuleExport size_t RegisterEPTImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("EPT","EPT",
    "Encapsulated PostScript with TIFF preview");
  entry->decoder=(DecodeImageHandler *) ReadEPTImage;
  entry->encoder=(EncodeImageHandler *) WriteEPTImage;
  entry->magick=(IsImageFormatHandler *) IsEPT;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("EPT","EPT2",
    "Encapsulated PostScript Level II with TIFF preview");
  entry->decoder=(DecodeImageHandler *) ReadEPTImage;
  entry->encoder=(EncodeImageHandler *) WriteEPTImage;
  entry->magick=(IsImageFormatHandler *) IsEPT;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("EPT","EPT3",
    "Encapsulated PostScript Level III with TIFF preview");
  entry->decoder=(DecodeImageHandler *) ReadEPTImage;
  entry->encoder=(EncodeImageHandler *) WriteEPTImage;
  entry->magick=(IsImageFormatHandler *) IsEPT;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r E P T I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterEPTImage() removes format registrations made by the
%  EPT module from the list of supported formats.
%
%  The format of the UnregisterEPTImage method is:
%
%      UnregisterEPTImage(void)
%
*/
ModuleExport void UnregisterEPTImage(void)
{
  (void) UnregisterMagickInfo("EPT");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e E P T I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteEPTImage() writes an image in the Encapsulated Postscript format
%  with a TIFF preview.
%
%  The format of the WriteEPTImage method is:
%
%      MagickBooleanType WriteEPTImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteEPTImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent];

  EPTInfo
    ept_info;

  Image
    *write_image;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  /*
    Write EPT image.
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
  write_image=CloneImage(image,0,0,MagickTrue,exception);
  if (write_image == (Image *) NULL)
    return(MagickFalse);
  write_info=CloneImageInfo(image_info);
  (void) CopyMagickString(write_info->filename,"EPS:",MagickPathExtent);
  (void) CopyMagickString(write_info->magick,"EPS",MagickPathExtent);
  if (LocaleCompare(image_info->magick,"EPT2") == 0)
    {
      (void) CopyMagickString(write_info->filename,"EPS2:",MagickPathExtent);
      (void) CopyMagickString(write_info->magick,"EPS2",MagickPathExtent);
    }
  if (LocaleCompare(image_info->magick,"EPT3") == 0)
    {
      (void) CopyMagickString(write_info->filename,"EPS3:",MagickPathExtent);
      (void) CopyMagickString(write_info->magick,"EPS3",MagickPathExtent);
    }
  (void) memset(&ept_info,0,sizeof(ept_info));
  ept_info.magick=0xc6d3d0c5ul;
  ept_info.postscript=(unsigned char *) ImageToBlob(write_info,write_image,
    &ept_info.postscript_length,exception);
  write_image=DestroyImage(write_image);
  write_info=DestroyImageInfo(write_info);
  if (ept_info.postscript == (void *) NULL)
    return(MagickFalse);
  write_image=CloneImage(image,0,0,MagickTrue,exception);
  if (write_image == (Image *) NULL)
    return(MagickFalse);
  write_info=CloneImageInfo(image_info);
  (void) CopyMagickString(write_info->magick,"TIFF",MagickPathExtent);
  (void) FormatLocaleString(filename,MagickPathExtent,"tiff:%s",
    write_info->filename);
  (void) CopyMagickString(write_info->filename,filename,MagickPathExtent);
  if ((write_image->columns > 512) || (write_image->rows > 512))
    {
      Image
        *resize_image;

      resize_image=ResizeImage(write_image,512,512,write_image->filter,
        exception);
      if (resize_image != (Image *) NULL)
        {
          write_image=DestroyImage(write_image);
          write_image=resize_image;
        }
    }
  if ((write_image->storage_class == DirectClass) ||
      (write_image->colors > 256))
    {
      QuantizeInfo
        quantize_info;

      /*
        EPT preview requires that the image is colormapped.
      */
      GetQuantizeInfo(&quantize_info);
      quantize_info.dither_method=IdentifyPaletteImage(write_image,
        exception) == MagickFalse ? RiemersmaDitherMethod : NoDitherMethod;
      (void) QuantizeImage(&quantize_info,write_image,exception);
    }
  write_info->compression=NoCompression;
  ept_info.tiff=(unsigned char *) ImageToBlob(write_info,write_image,
    &ept_info.tiff_length,exception);
  write_image=DestroyImage(write_image);
  write_info=DestroyImageInfo(write_info);
  if (ept_info.tiff == (void *) NULL)
    {
      ept_info.postscript=(unsigned char *) RelinquishMagickMemory(
        ept_info.postscript);
      return(MagickFalse);
    }
  /*
    Write EPT image.
  */
  (void) WriteBlobLSBLong(image,(unsigned int) ept_info.magick);
  (void) WriteBlobLSBLong(image,30);
  (void) WriteBlobLSBLong(image,(unsigned int) ept_info.postscript_length);
  (void) WriteBlobLSBLong(image,0);
  (void) WriteBlobLSBLong(image,0);
  (void) WriteBlobLSBLong(image,(unsigned int) ept_info.postscript_length+30);
  (void) WriteBlobLSBLong(image,(unsigned int) ept_info.tiff_length);
  (void) WriteBlobLSBShort(image,0xffff);
  (void) WriteBlob(image,ept_info.postscript_length,ept_info.postscript);
  (void) WriteBlob(image,ept_info.tiff_length,ept_info.tiff);
  /*
    Relinquish resources.
  */
  ept_info.postscript=(unsigned char *) RelinquishMagickMemory(
    ept_info.postscript);
  ept_info.tiff=(unsigned char *) RelinquishMagickMemory(ept_info.tiff);
  (void) CloseBlob(image);
  return(MagickTrue);
}

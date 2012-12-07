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
#include "magick/color.h"
#include "magick/constitute.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/delegate.h"
#include "magick/geometry.h"
#include "magick/histogram.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/quantize.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/transform.h"
#include "magick/utility.h"

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
  WriteEPTImage(const ImageInfo *,Image *);

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
  EPTInfo
    ept_info;

  Image
    *image;

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
  ept_info.magick=ReadBlobLSBLong(image);
  if (ept_info.magick != 0xc6d3d0c5ul)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  ept_info.postscript_offset=(MagickOffsetType) ReadBlobLSBLong(image);
  ept_info.postscript_length=ReadBlobLSBLong(image);
  (void) ReadBlobLSBLong(image);
  (void) ReadBlobLSBLong(image);
  ept_info.tiff_offset=(MagickOffsetType) ReadBlobLSBLong(image);
  ept_info.tiff_length=ReadBlobLSBLong(image);
  (void) ReadBlobLSBShort(image);
  ept_info.postscript=(unsigned char *) AcquireQuantumMemory(
    ept_info.postscript_length+1,sizeof(*ept_info.postscript));
  if (ept_info.postscript == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(ept_info.postscript,0,(ept_info.postscript_length+1)*
    sizeof(*ept_info.postscript));
  ept_info.tiff=(unsigned char *) AcquireQuantumMemory(ept_info.tiff_length+1,
    sizeof(*ept_info.tiff));
  if (ept_info.tiff == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(ept_info.tiff,0,(ept_info.tiff_length+1)*
    sizeof(*ept_info.tiff));
  offset=SeekBlob(image,ept_info.tiff_offset,SEEK_SET);
  if (offset < 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  count=ReadBlob(image,ept_info.tiff_length,ept_info.tiff);
  if (count != (ssize_t) (ept_info.tiff_length))
    (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageWarning,
      "InsufficientImageDataInFile","`%s'",image->filename);
  offset=SeekBlob(image,ept_info.postscript_offset,SEEK_SET);
  if (offset < 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  count=ReadBlob(image,ept_info.postscript_length,ept_info.postscript);
  if (count != (ssize_t) (ept_info.postscript_length))
    (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageWarning,
      "InsufficientImageDataInFile","`%s'",image->filename);
  (void) CloseBlob(image);
  image=DestroyImage(image);
  read_info=CloneImageInfo(image_info);
  (void) CopyMagickString(read_info->magick,"EPS",MaxTextExtent);
  image=BlobToImage(read_info,ept_info.postscript,ept_info.postscript_length,
    exception);
  if (image == (Image *) NULL)
    {
      (void) CopyMagickString(read_info->magick,"TIFF",MaxTextExtent);
      image=BlobToImage(read_info,ept_info.tiff,ept_info.tiff_length,exception);
    }
  read_info=DestroyImageInfo(read_info);
  if (image != (Image *) NULL)
    (void) CopyMagickString(image->filename,image_info->filename,MaxTextExtent);
  ept_info.tiff=(unsigned char *) RelinquishMagickMemory(ept_info.tiff);
  ept_info.postscript=(unsigned char *) RelinquishMagickMemory(
    ept_info.postscript);
  return(image);
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

  entry=SetMagickInfo("EPT");
  entry->decoder=(DecodeImageHandler *) ReadEPTImage;
  entry->encoder=(EncodeImageHandler *) WriteEPTImage;
  entry->magick=(IsImageFormatHandler *) IsEPT;
  entry->seekable_stream=MagickTrue;
  entry->adjoin=MagickFalse;
  entry->blob_support=MagickFalse;
  entry->description=ConstantString(
    "Encapsulated PostScript with TIFF preview");
  entry->module=ConstantString("EPT");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("EPT2");
  entry->decoder=(DecodeImageHandler *) ReadEPTImage;
  entry->encoder=(EncodeImageHandler *) WriteEPTImage;
  entry->magick=(IsImageFormatHandler *) IsEPT;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->blob_support=MagickFalse;
  entry->description=ConstantString(
    "Encapsulated PostScript Level II with TIFF preview");
  entry->module=ConstantString("EPT");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("EPT3");
  entry->decoder=(DecodeImageHandler *) ReadEPTImage;
  entry->encoder=(EncodeImageHandler *) WriteEPTImage;
  entry->magick=(IsImageFormatHandler *) IsEPT;
  entry->seekable_stream=MagickTrue;
  entry->blob_support=MagickFalse;
  entry->description=ConstantString(
    "Encapsulated PostScript Level III with TIFF preview");
  entry->module=ConstantString("EPT");
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
%      MagickBooleanType WriteEPTImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteEPTImage(const ImageInfo *image_info,Image *image)
{
  char
     filename[MaxTextExtent];

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
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  write_image=CloneImage(image,0,0,MagickTrue,&image->exception);
  if (write_image == (Image *) NULL)
    return(MagickFalse);
  write_info=CloneImageInfo(image_info);
  (void) CopyMagickString(write_info->magick,"EPS",MaxTextExtent);
  if (LocaleCompare(image_info->magick,"EPT2") == 0)
    (void) CopyMagickString(write_info->magick,"EPS2",MaxTextExtent);
  if (LocaleCompare(image_info->magick,"EPT3") == 0)
    (void) CopyMagickString(write_info->magick,"EPS3",MaxTextExtent);
  (void) ResetMagickMemory(&ept_info,0,sizeof(ept_info));
  ept_info.magick=0xc6d3d0c5ul;
  ept_info.postscript=(unsigned char *) ImageToBlob(write_info,write_image,
    &ept_info.postscript_length,&image->exception);
  write_image=DestroyImage(write_image);
  write_info=DestroyImageInfo(write_info);
  if (ept_info.postscript == (void *) NULL)
    return(MagickFalse);
  write_image=CloneImage(image,0,0,MagickTrue,&image->exception);
  if (write_image == (Image *) NULL)
    return(MagickFalse);
  write_info=CloneImageInfo(image_info);
  (void) CopyMagickString(write_info->magick,"TIFF",MaxTextExtent);
  (void) FormatLocaleString(filename,MaxTextExtent,"tiff:%s",
    write_info->filename);
  (void) CopyMagickString(write_info->filename,filename,MaxTextExtent);
  (void) TransformImage(&write_image,(char *) NULL,"512x512>");
  if ((write_image->storage_class == DirectClass) ||
      (write_image->colors > 256))
    {
      QuantizeInfo
        quantize_info;

      /*
        EPT preview requires that the image is colormapped.
      */
      GetQuantizeInfo(&quantize_info);
      quantize_info.dither=IsPaletteImage(write_image,&image->exception) ==
        MagickFalse ? MagickTrue : MagickFalse;
      (void) QuantizeImage(&quantize_info,write_image);
    }
  write_info->compression=NoCompression;
  ept_info.tiff=(unsigned char *) ImageToBlob(write_info,write_image,
    &ept_info.tiff_length,&image->exception);
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

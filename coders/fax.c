/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            FFFFF   AAA   X   X                              %
%                            F      A   A   X X                               %
%                            FFF    AAAAA    X                                %
%                            F      A   A   X X                               %
%                            F      A   A  X   X                              %
%                                                                             %
%                                                                             %
%                   Read/Write Group 3 Fax Image Format                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
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
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/compress.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteFAXImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s F A X                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsFAX() returns MagickTrue if the image format type, identified by the
%  magick string, is FAX.
%
%  The format of the IsFAX method is:
%
%      MagickBooleanType IsFAX(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static MagickBooleanType IsFAX(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (LocaleNCompare((char *) magick,"DFAX",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d F A X I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadFAXImage() reads a Group 3 FAX image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadFAXImage method is:
%
%      Image *ReadFAXImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image* FaxReadG3(Image *image,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=HuffmanDecodeImage(image,exception);
  if (status == MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnableToReadImageData",
      image->filename);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

static Image* FaxReadG4(Image *image,const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent];

  ImageInfo
    *read_info;

  filename[0]='\0';
  if (ImageToFile(image,filename,exception) == MagickFalse)
    ThrowImageException(FileOpenError,"UnableToCreateTemporaryFile");
  (void) CloseBlob(image);
  image=DestroyImage(image);
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) FormatLocaleString(read_info->filename,MagickPathExtent,"group4:%s",
    filename);
  read_info->orientation=TopLeftOrientation;
  image=ReadImage(read_info,exception);
  if (image != (Image *) NULL)
    {
      (void) CopyMagickString(image->filename,image_info->filename,
        MagickPathExtent);
      (void) CopyMagickString(image->magick_filename,image_info->filename,
        MagickPathExtent);
      (void) CopyMagickString(image->magick,"G4",MagickPathExtent);
    }
  read_info=DestroyImageInfo(read_info);
  (void) RelinquishUniqueFileResource(filename);
  return(GetFirstImageInList(image));
}

static Image *ReadFAXImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

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
  image->storage_class=PseudoClass;
  if (image->columns == 0)
    image->columns=2592;
  if (image->rows == 0)
    image->rows=3508;
  image->depth=8;
  if (AcquireImageColormap(image,2,exception) == MagickFalse)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Monochrome colormap.
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
  if (LocaleCompare(image_info->magick,"G4") == 0)
    return(FaxReadG4(image,image_info,exception));
  else
    return(FaxReadG3(image,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r F A X I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterFAXImage() adds attributes for the FAX image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterFAXImage method is:
%
%      size_t RegisterFAXImage(void)
%
*/
ModuleExport size_t RegisterFAXImage(void)
{
  MagickInfo
    *entry;

  static const char
    *Note=
    {
      "FAX machines use non-square pixels which are 1.5 times wider than\n"
      "they are tall but computer displays use square pixels, therefore\n"
      "FAX images may appear to be narrow unless they are explicitly\n"
      "resized using a geometry of \"150x100%\".\n"
    };

  entry=AcquireMagickInfo("FAX","FAX","Group 3 FAX");
  entry->decoder=(DecodeImageHandler *) ReadFAXImage;
  entry->encoder=(EncodeImageHandler *) WriteFAXImage;
  entry->magick=(IsImageFormatHandler *) IsFAX;
  entry->note=ConstantString(Note);
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("FAX","G3","Group 3 FAX");
  entry->decoder=(DecodeImageHandler *) ReadFAXImage;
  entry->encoder=(EncodeImageHandler *) WriteFAXImage;
  entry->magick=(IsImageFormatHandler *) IsFAX;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("FAX","G4","Group 4 FAX");
  entry->decoder=(DecodeImageHandler *) ReadFAXImage;
  entry->encoder=(EncodeImageHandler *) WriteFAXImage;
  entry->magick=(IsImageFormatHandler *) IsFAX;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r F A X I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterFAXImage() removes format registrations made by the
%  FAX module from the list of supported formats.
%
%  The format of the UnregisterFAXImage method is:
%
%      UnregisterFAXImage(void)
%
*/
ModuleExport void UnregisterFAXImage(void)
{
  (void) UnregisterMagickInfo("FAX");
  (void) UnregisterMagickInfo("G3");
  (void) UnregisterMagickInfo("G4");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e F A X I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteFAXImage() writes an image to a file in 1 dimensional Huffman encoded
%  format.
%
%  The format of the WriteFAXImage method is:
%
%      MagickBooleanType WriteFAXImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteFAXImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  size_t
    imageListLength;

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
  write_info=CloneImageInfo(image_info);
  (void) CopyMagickString(write_info->magick,"FAX",MagickPathExtent);
  scene=0;
  imageListLength=GetImageListLength(image);
  do
  {
    /*
      Convert MIFF to monochrome.
    */
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
    status=HuffmanEncodeImage(write_info,image,image,exception);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,imageListLength);
    if (status == MagickFalse)
      break;
  } while (write_info->adjoin != MagickFalse);
  write_info=DestroyImageInfo(write_info);
  (void) CloseBlob(image);
  return(status);
}

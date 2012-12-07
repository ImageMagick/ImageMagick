/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            EEEEE  X   X  RRRR                               %
%                            E       X X   R   R                              %
%                            EEE      X    RRRR                               %
%                            E       X X   R R                                %
%                            EEEEE  X   X  R  R                               %
%                                                                             %
%                                                                             %
%            Read/Write High Dynamic-Range (HDR) Image File Format            %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 April 2007                                  %
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
#include "magick/cache.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/pixel-accessor.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/resource_.h"
#include "magick/utility.h"
#if defined(MAGICKCORE_OPENEXR_DELEGATE)
#include <ImfCRgbaFile.h>

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteEXRImage(const ImageInfo *,Image *);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s E X R                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsEXR() returns MagickTrue if the image format type, identified by the
%  magick string, is EXR.
%
%  The format of the IsEXR method is:
%
%      MagickBooleanType IsEXR(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsEXR(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\166\057\061\001",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

#if defined(MAGICKCORE_OPENEXR_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d E X R I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadEXRImage reads an image in the high dynamic-range (HDR) file format
%  developed by Industrial Light & Magic.  It allocates the memory necessary
%  for the new Image structure and returns a pointer to the new image.
%
%  The format of the ReadEXRImage method is:
%
%      Image *ReadEXRImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadEXRImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  const ImfHeader
    *hdr_info;

  Image
    *image;

  ImageInfo
    *read_info;

  ImfInputFile
    *file;

  ImfRgba
    *scanline;

  int
    max_x,
    max_y,
    min_x,
    min_y;

  MagickBooleanType
    status;

  register ssize_t
    x;

  register PixelPacket
    *q;

  ssize_t
    y;

  /*
    Open image.
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
  read_info=CloneImageInfo(image_info);
  if (IsPathAccessible(read_info->filename) == MagickFalse)
    {
      (void) AcquireUniqueFilename(read_info->filename);
      (void) ImageToFile(image,read_info->filename,exception);
    }
  file=ImfOpenInputFile(read_info->filename);
  if (file == (ImfInputFile *) NULL)
    {
      ThrowFileException(exception,BlobError,"UnableToOpenBlob",
        ImfErrorMessage());
      read_info=DestroyImageInfo(read_info);
      return((Image *) NULL);
    }
  hdr_info=ImfInputHeader(file);
  ImfHeaderDisplayWindow(hdr_info,&min_x,&min_y,&max_x,&max_y);
  image->columns=max_x-min_x+1UL;
  image->rows=max_y-min_y+1UL;
  image->matte=MagickTrue;
  SetImageColorspace(image,RGBColorspace);
  if (image_info->ping != MagickFalse)
    {
      (void) ImfCloseInputFile(file);
      if (LocaleCompare(image_info->filename,read_info->filename) != 0)
        (void) RelinquishUniqueFileResource(read_info->filename);
      read_info=DestroyImageInfo(read_info);
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  scanline=(ImfRgba *) AcquireQuantumMemory(image->columns,sizeof(*scanline));
  if (scanline == (ImfRgba *) NULL)
    {
      (void) ImfCloseInputFile(file);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    ImfInputSetFrameBuffer(file,scanline-min_x-image->columns*(min_y+y),1,
      image->columns);
    ImfInputReadPixels(file,min_y+y,min_y+y);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelRed(q,ClampToQuantum((MagickRealType) QuantumRange*
        ImfHalfToFloat(scanline[x].r)));
      SetPixelGreen(q,ClampToQuantum((MagickRealType) QuantumRange*
        ImfHalfToFloat(scanline[x].g)));
      SetPixelBlue(q,ClampToQuantum((MagickRealType) QuantumRange*
        ImfHalfToFloat(scanline[x].b)));
      SetPixelAlpha(q,ClampToQuantum((MagickRealType) QuantumRange*
        ImfHalfToFloat(scanline[x].a)));
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  scanline=(ImfRgba *) RelinquishMagickMemory(scanline);
  (void) ImfCloseInputFile(file);
  if (LocaleCompare(image_info->filename,read_info->filename) != 0)
    (void) RelinquishUniqueFileResource(read_info->filename);
  read_info=DestroyImageInfo(read_info);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r E X R I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterEXRImage() adds properties for the EXR image format
%  to the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterEXRImage method is:
%
%      size_t RegisterEXRImage(void)
%
*/
ModuleExport size_t RegisterEXRImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("EXR");
#if defined(MAGICKCORE_OPENEXR_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadEXRImage;
  entry->encoder=(EncodeImageHandler *) WriteEXRImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsEXR;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("High Dynamic-range (HDR)");
  entry->blob_support=MagickFalse;
  entry->module=ConstantString("EXR");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r E X R I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterEXRImage() removes format registrations made by the
%  EXR module from the list of supported formats.
%
%  The format of the UnregisterEXRImage method is:
%
%      UnregisterEXRImage(void)
%
*/
ModuleExport void UnregisterEXRImage(void)
{
  (void) UnregisterMagickInfo("EXR");
}

#if defined(MAGICKCORE_OPENEXR_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e E X R I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteEXRImage() writes an image to a file the in the high dynamic-range
%  (HDR) file format developed by Industrial Light & Magic.
%
%  The format of the WriteEXRImage method is:
%
%      MagickBooleanType WriteEXRImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteEXRImage(const ImageInfo *image_info,Image *image)
{
  ImageInfo
    *write_info;

  ImfHalf
    half_quantum;

  ImfHeader
    *hdr_info;

  ImfOutputFile
    *file;

  ImfRgba
    *scanline;

  int
    compression;

  MagickBooleanType
    status;

  register const PixelPacket
    *p;

  register ssize_t
    x;

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
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  (void) SetImageColorspace(image,RGBColorspace);
  write_info=CloneImageInfo(image_info);
  (void) AcquireUniqueFilename(write_info->filename);
  hdr_info=ImfNewHeader();
  ImfHeaderSetDataWindow(hdr_info,0,0,(int) image->columns-1,(int)
    image->rows-1);
  ImfHeaderSetDisplayWindow(hdr_info,0,0,(int) image->columns-1,(int)
    image->rows-1);
  compression=IMF_NO_COMPRESSION;
  if (write_info->compression == ZipSCompression)
    compression=IMF_ZIPS_COMPRESSION;
  if (write_info->compression == ZipCompression)
    compression=IMF_ZIP_COMPRESSION;
  if (write_info->compression == PizCompression)
    compression=IMF_PIZ_COMPRESSION;
  if (write_info->compression == Pxr24Compression)
    compression=IMF_PXR24_COMPRESSION;
#if defined(B44Compression)
  if (write_info->compression == B44Compression)
    compression=IMF_B44_COMPRESSION;
#endif
#if defined(B44ACompression)
  if (write_info->compression == B44ACompression)
    compression=IMF_B44A_COMPRESSION;
#endif
  ImfHeaderSetCompression(hdr_info,compression);
  ImfHeaderSetLineOrder(hdr_info,IMF_INCREASING_Y);
  file=ImfOpenOutputFile(write_info->filename,hdr_info,IMF_WRITE_RGBA);
  ImfDeleteHeader(hdr_info);
  if (file == (ImfOutputFile *) NULL)
    {
      ThrowFileException(&image->exception,BlobError,"UnableToOpenBlob",
        ImfErrorMessage());
      write_info=DestroyImageInfo(write_info);
      return(MagickFalse);
    }
  scanline=(ImfRgba *) AcquireQuantumMemory(image->columns,sizeof(*scanline));
  if (scanline == (ImfRgba *) NULL)
    {
      (void) ImfCloseOutputFile(file);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ImfFloatToHalf(QuantumScale*GetPixelRed(p),&half_quantum);
      scanline[x].r=half_quantum;
      ImfFloatToHalf(QuantumScale*GetPixelGreen(p),&half_quantum);
      scanline[x].g=half_quantum;
      ImfFloatToHalf(QuantumScale*GetPixelBlue(p),&half_quantum);
      scanline[x].b=half_quantum;
      if (image->matte == MagickFalse)
        ImfFloatToHalf(1.0,&half_quantum);
      else
        ImfFloatToHalf(1.0-QuantumScale*GetPixelOpacity(p),
          &half_quantum);
      scanline[x].a=half_quantum;
      p++;
    }
    ImfOutputSetFrameBuffer(file,scanline-(y*image->columns),1,image->columns);
    ImfOutputWritePixels(file,1);
  }
  (void) ImfCloseOutputFile(file);
  scanline=(ImfRgba *) RelinquishMagickMemory(scanline);
  (void) FileToImage(image,write_info->filename);
  (void) RelinquishUniqueFileResource(write_info->filename);
  write_info=DestroyImageInfo(write_info);
  (void) CloseBlob(image);
  return(MagickTrue);
}
#endif

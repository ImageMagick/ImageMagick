/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     V   V  IIIII   CCCC   AAA   RRRR                        %
%                     V   V    I    C      A   A  R   R                       %
%                     V   V    I    C      AAAAA  RRRR                        %
%                      V V     I    C      A   A  R R                         %
%                       V    IIIII   CCCC  A   A  R  R                        %
%                                                                             %
%                                                                             %
%                    Read/Write VICAR Rasterfile Format                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
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
#include "MagickCore/quantum-private.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteVICARImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s V I C A R                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsVICAR() returns MagickTrue if the image format type, identified by the
%  magick string, is VICAR.
%
%  The format of the IsVICAR method is:
%
%      MagickBooleanType IsVICAR(const unsigned char *magick,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsVICAR(const unsigned char *magick,
  const size_t length)
{
  if (length < 14)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick,"LBLSIZE",7) == 0)
    return(MagickTrue);
  if (LocaleNCompare((const char *) magick,"NJPL1I",6) == 0)
    return(MagickTrue);
  if (LocaleNCompare((const char *) magick,"PDS_VERSION_ID",14) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d V I C A R I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadVICARImage() reads a VICAR image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadVICARImage method is:
%
%      Image *ReadVICARImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: Method ReadVICARImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadVICARImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    keyword[MagickPathExtent],
    value[MagickPathExtent];

  const unsigned char
    *pixels;

  Image
    *image;

  int
    c;

  MagickBooleanType
    status,
    value_expected;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register Quantum
    *q;

  size_t
    length;

  ssize_t
    count,
    y;

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
    Decode image header.
  */
  c=ReadBlobByte(image);
  count=1;
  if (c == EOF)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  length=0;
  image->columns=0;
  image->rows=0;
  while (isgraph(c) && ((image->columns == 0) || (image->rows == 0)))
  {
    if (isalnum(c) == MagickFalse)
      {
        c=ReadBlobByte(image);
        count++;
      }
    else
      {
        register char
          *p;

        /*
          Determine a keyword and its value.
        */
        p=keyword;
        do
        {
          if ((size_t) (p-keyword) < (MagickPathExtent-1))
            *p++=c;
          c=ReadBlobByte(image);
          count++;
        } while (isalnum(c) || (c == '_'));
        *p='\0';
        value_expected=MagickFalse;
        while ((isspace((int) ((unsigned char) c)) != 0) || (c == '='))
        {
          if (c == '=')
            value_expected=MagickTrue;
          c=ReadBlobByte(image);
          count++;
        }
        if (value_expected == MagickFalse)
          continue;
        p=value;
        while (isalnum(c))
        {
          if ((size_t) (p-value) < (MagickPathExtent-1))
            *p++=c;
          c=ReadBlobByte(image);
          count++;
        }
        *p='\0';
        /*
          Assign a value to the specified keyword.
        */
        if (LocaleCompare(keyword,"Label_RECORDS") == 0)
          length=(ssize_t) StringToLong(value);
        if (LocaleCompare(keyword,"LBLSIZE") == 0)
          length=(ssize_t) StringToLong(value);
        if (LocaleCompare(keyword,"RECORD_BYTES") == 0)
          image->columns=StringToUnsignedLong(value);
        if (LocaleCompare(keyword,"NS") == 0)
          image->columns=StringToUnsignedLong(value);
        if (LocaleCompare(keyword,"LINES") == 0)
          image->rows=StringToUnsignedLong(value);
        if (LocaleCompare(keyword,"NL") == 0)
          image->rows=StringToUnsignedLong(value);
      }
    while (isspace((int) ((unsigned char) c)) != 0)
    {
      c=ReadBlobByte(image);
      count++;
    }
  }
  while (count < (ssize_t) length)
  {
    c=ReadBlobByte(image);
    if (c == EOF)
      break;
    count++;
  }
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
  image->depth=8;
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  /*
    Read VICAR pixels.
  */
  (void) SetImageColorspace(image,GRAYColorspace,exception);
  quantum_type=GrayQuantum;
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  length=GetQuantumExtent(image,quantum_info,quantum_type);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    pixels=(const unsigned char *) ReadBlobStream(image,length,
      GetQuantumPixels(quantum_info),&count);
    if (count != (ssize_t) length)
      break;
    (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      quantum_type,pixels,exception);
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  SetQuantumImageType(image,quantum_type);
  quantum_info=DestroyQuantumInfo(quantum_info);
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
%   R e g i s t e r V I C A R I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterVICARImage() adds attributes for the VICAR image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterVICARImage method is:
%
%      size_t RegisterVICARImage(void)
%
*/
ModuleExport size_t RegisterVICARImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("VICAR","VICAR","VICAR rasterfile format");
  entry->decoder=(DecodeImageHandler *) ReadVICARImage;
  entry->encoder=(EncodeImageHandler *) WriteVICARImage;
  entry->magick=(IsImageFormatHandler *) IsVICAR;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r V I C A R I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterVICARImage() removes format registrations made by the
%  VICAR module from the list of supported formats.
%
%  The format of the UnregisterVICARImage method is:
%
%      UnregisterVICARImage(void)
%
*/
ModuleExport void UnregisterVICARImage(void)
{
  (void) UnregisterMagickInfo("VICAR");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e V I C A R I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteVICARImage() writes an image in the VICAR rasterfile format.
%  Vicar files contain a text header, followed by one or more planes of binary
%  grayscale image data.  Vicar files are designed to allow many planes to be
%  stacked together to form image cubes.  This method only writes a single
%  grayscale plane.
%
%  WriteVICARImage was written contributed by gorelick@esther.la.asu.edu.
%
%  The format of the WriteVICARImage method is:
%
%      MagickBooleanType WriteVICARImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteVICARImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  char
    header[MagickPathExtent];

  int
    y;

  MagickBooleanType
    status;

  QuantumInfo
    *quantum_info;

  register const Quantum
    *p;

  size_t
    length;

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
  (void) TransformImageColorspace(image,sRGBColorspace,exception);
  /*
    Write header.
  */
  (void) ResetMagickMemory(header,' ',MagickPathExtent);
  (void) FormatLocaleString(header,MagickPathExtent,
    "LBLSIZE=%.20g FORMAT='BYTE' TYPE='IMAGE' BUFSIZE=20000 DIM=2 EOL=0 "
    "RECSIZE=%.20g ORG='BSQ' NL=%.20g NS=%.20g NB=1 N1=0 N2=0 N3=0 N4=0 NBB=0 "
    "NLB=0 TASK='ImageMagick'",(double) MagickPathExtent,(double) image->columns,
    (double) image->rows,(double) image->columns);
  (void) WriteBlob(image,MagickPathExtent,(unsigned char *) header);
  /*
    Write VICAR pixels.
  */
  image->depth=8;
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  pixels=(unsigned char *) GetQuantumPixels(quantum_info);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      GrayQuantum,pixels,exception);
    count=WriteBlob(image,length,pixels);
    if (count != (ssize_t) length)
      break;
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  quantum_info=DestroyQuantumInfo(quantum_info);
  (void) CloseBlob(image);
  return(MagickTrue);
}

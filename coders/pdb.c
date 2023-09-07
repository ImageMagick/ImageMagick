/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP   DDDD   BBBB                               %
%                            P   P  D   D  B   B                              %
%                            PPPP   D   D  BBBB                               %
%                            P      D   D  B   B                              %
%                            P      DDDD   BBBB                               %
%                                                                             %
%                                                                             %
%               Read/Write Palm Database ImageViewer Image Format             %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
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
%   20071202 TS * rewrote RLE decoder - old version could cause buffer overflows
%               * failure of RLE decoding now throws error RLEDecoderError
%               * fixed bug in RLE decoding - now all rows are decoded, not just
%     the first one
%   * fixed bug in reader - record offsets now handled correctly
%   * fixed bug in reader - only bits 0..2 indicate compression type
%               * in writer: now using image color count instead of depth
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/color-private.h"
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
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/timer-private.h"
#include "MagickCore/utility.h"

/*
  Typedef declarations.
*/
typedef struct _PDBInfo
{
  char
    name[32];

  short int
    attributes,
    version;

  ssize_t
    create_time,
    modify_time,
    archive_time;

  size_t
    modify_number,
    application_info,
    sort_info;

  char
    type[4],  /* database type identifier "vIMG" */
    id[4];    /* database creator identifier "View" */

  size_t
    seed,
    next_record;

  short int
    number_records;
} PDBInfo;

typedef struct _PDBImage
{
  char
    name[32],
    version;

  size_t
    reserved_1,
    note;

  short int
    x_last,
    y_last;

  size_t
    reserved_2;

  short int
    width,
    height;

  unsigned char
    type;

  unsigned short
    x_anchor,
    y_anchor;
} PDBImage;
/*
  Forward declarations.
*/
static MagickBooleanType
  WritePDBImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e c o d e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecodeImage unpacks the packed image pixels into runlength-encoded
%  pixel packets.
%
%  The format of the DecodeImage method is:
%
%      MagickBooleanType DecodeImage(Image *image,unsigned char *pixels,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o image: the address of a structure of type Image.
%
%    o pixels:  The address of a byte (8 bits) array of pixel data created by
%      the decoding process.
%
%    o length:  Number of bytes to read into buffer 'pixels'.
%
*/
static MagickBooleanType DecodeImage(Image *image, unsigned char *pixels,
  const size_t length)
{
#define RLE_MODE_NONE -1
#define RLE_MODE_COPY  0
#define RLE_MODE_RUN   1

  int           data = 0, count = 0;
  unsigned char *p;
  int           mode = RLE_MODE_NONE;

  for (p = pixels; p < pixels + length; p++) {
    if (0 == count) {
      data = ReadBlobByte( image );
      if (-1 == data) return MagickFalse;
      if (data > 128) {
        mode  = RLE_MODE_RUN;
        count = data - 128 + 1;
        data  = ReadBlobByte( image );
        if (-1 == data) return MagickFalse;
      } else {
        mode  = RLE_MODE_COPY;
        count = data + 1;
      }
    }

    if (RLE_MODE_COPY == mode) {
      data = ReadBlobByte( image );
      if (-1 == data) return MagickFalse;
    }
    *p = (unsigned char)data;
    --count;
  }
  return MagickTrue;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P D B                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPDB() returns MagickTrue if the image format type, identified by the
%  magick string, is PDB.
%
%  The format of the ReadPDBImage method is:
%
%      MagickBooleanType IsPDB(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsPDB(const unsigned char *magick,const size_t length)
{
  if (length < 68)
    return(MagickFalse);
  if (memcmp(magick+60,"vIMGView",8) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P D B I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPDBImage() reads an Pilot image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadPDBImage method is:
%
%      Image *ReadPDBImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadPDBImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  unsigned char
    attributes,
    tag[3];

  Image
    *image;

  MagickBooleanType
    status;

  PDBImage
    pdb_image;

  PDBInfo
    pdb_info;

  Quantum
    index;

  ssize_t
    x;

  Quantum
    *q;

  unsigned char
    *p;

  size_t
    bits_per_pixel,
    num_pad_bytes,
    one,
    packets;

  ssize_t
    count,
    img_offset,
    comment_offset = 0,
    y;

  unsigned char
    *pixels;

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
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Determine if this a PDB image file.
  */
  (void) memset(&pdb_info,0,sizeof(pdb_info));
  count=ReadBlob(image,sizeof(pdb_info.name),(unsigned char *) pdb_info.name);
  if (count != sizeof(pdb_info.name))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  pdb_info.attributes=(short) ReadBlobMSBShort(image);
  pdb_info.version=(short) ReadBlobMSBShort(image);
  pdb_info.create_time=ReadBlobMSBLong(image);
  pdb_info.modify_time=ReadBlobMSBLong(image);
  pdb_info.archive_time=ReadBlobMSBLong(image);
  pdb_info.modify_number=ReadBlobMSBLong(image);
  pdb_info.application_info=ReadBlobMSBLong(image);
  pdb_info.sort_info=ReadBlobMSBLong(image);
  (void) ReadBlob(image,4,(unsigned char *) pdb_info.type);
  (void) ReadBlob(image,4,(unsigned char *) pdb_info.id);
  pdb_info.seed=ReadBlobMSBLong(image);
  pdb_info.next_record=ReadBlobMSBLong(image);
  pdb_info.number_records=(short) ReadBlobMSBShort(image);
  if ((memcmp(pdb_info.type,"vIMG",4) != 0) ||
      (memcmp(pdb_info.id,"View",4) != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if (pdb_info.next_record != 0)
    ThrowReaderException(CoderError,"MultipleRecordListNotSupported");
  /*
    Read record header.
  */
  img_offset=(ssize_t) ReadBlobMSBSignedLong(image);
  attributes=(unsigned char) ReadBlobByte(image);
  (void) attributes;
  count=ReadBlob(image,3,(unsigned char *) tag);
  if (count != 3  ||  memcmp(tag,"\x6f\x80\x00",3) != 0)
    ThrowReaderException(CorruptImageError,"CorruptImage");
  if (pdb_info.number_records > 1)
    {
      comment_offset=(ssize_t) ReadBlobMSBSignedLong(image);
      attributes=(unsigned char) ReadBlobByte(image);
      count=ReadBlob(image,3,(unsigned char *) tag);
      if (count != 3  ||  memcmp(tag,"\x6f\x80\x01",3) != 0)
        ThrowReaderException(CorruptImageError,"CorruptImage");
    }
  num_pad_bytes = (size_t) (img_offset - TellBlob( image ));
  while (num_pad_bytes-- != 0)
  {
    int
      c;

    c=ReadBlobByte(image);
    if (c == EOF)
      break;
  }
  /*
    Read image header.
  */
  count=ReadBlob(image,sizeof(pdb_image.name),(unsigned char *) pdb_image.name);
  if (count != sizeof(pdb_image.name))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  pdb_image.version=ReadBlobByte(image);
  pdb_image.type=(unsigned char) (ReadBlobByte(image));
  pdb_image.reserved_1=ReadBlobMSBLong(image);
  pdb_image.note=ReadBlobMSBLong(image);
  pdb_image.x_last=(short) ReadBlobMSBShort(image);
  pdb_image.y_last=(short) ReadBlobMSBShort(image);
  pdb_image.reserved_2=ReadBlobMSBLong(image);
  pdb_image.x_anchor=ReadBlobMSBShort(image);
  pdb_image.y_anchor=ReadBlobMSBShort(image);
  pdb_image.width=(short) ReadBlobMSBShort(image);
  pdb_image.height=(short) ReadBlobMSBShort(image);
  /*
    Initialize image structure.
  */
  image->columns=(size_t) pdb_image.width;
  image->rows=(size_t) pdb_image.height;
  image->depth=8;
  image->storage_class=PseudoClass;
  bits_per_pixel=pdb_image.type == 0 ? 2UL : pdb_image.type == 2 ? 4UL : 1UL;
  one=1;
  if (AcquireImageColormap(image,one << bits_per_pixel,exception) == MagickFalse)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status != MagickFalse)
    status=ResetImagePixels(image,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  packets=(bits_per_pixel*image->columns+7)/8;
  pixels=(unsigned char *) AcquireQuantumMemory(packets+257UL,image->rows*
    sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  (void) memset(pixels,0,(packets+257UL)*image->rows*sizeof(*pixels));
  switch (pdb_image.version & 0x07)
  {
    case 0:
    {
      image->compression=NoCompression;
      count=(ssize_t) ReadBlob(image,packets*image->rows,pixels);
      if (count != (ssize_t) (packets*image->rows))
        {
          pixels=(unsigned char *) RelinquishMagickMemory(pixels);
          ThrowReaderException(CorruptImageError,"RLEDecoderError");
        }
      break;
    }
    case 1:
    {
      image->compression=RLECompression;
      if (DecodeImage(image,pixels,packets*image->rows) == MagickFalse)
        {
          pixels=(unsigned char *) RelinquishMagickMemory(pixels);
          ThrowReaderException(CorruptImageError,"RLEDecoderError");
        }
      break;
    }
    default:
    {
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      ThrowReaderException(CorruptImageError,
        "UnrecognizedImageCompressionType");
    }
  }
  p=pixels;
  switch (bits_per_pixel)
  {
    case 1:
    {
      int
        bit;

      /*
        Read 1-bit PDB image.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (Quantum *) NULL)
          break;
        bit=0;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          index=(Quantum) (*p & (0x80 >> bit) ? 0x00 : 0x01);
          SetPixelIndex(image,index,q);
          q+=GetPixelChannels(image);
          bit++;
          if (bit == 8)
            {
              p++;
              bit=0;
            }
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
      (void) SyncImage(image,exception);
      break;
    }
    case 2:
    {
      unsigned int
        shift;

      /*
        Read 2-bit PDB image.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (Quantum *) NULL)
          break;
        shift=8;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          shift-=2;
          index=ConstrainColormapIndex(image,3UL-((*p >> shift) & 0x03),
            exception);
          SetPixelIndex(image,index,q);
          q+=GetPixelChannels(image);
          if (shift == 0)
            {
              shift=8;
              p++;
            }
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
      (void) SyncImage(image,exception);
      break;
    }
    case 4:
    {
      unsigned int
        shift;

      /*
        Read 4-bit PDB image.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (Quantum *) NULL)
          break;
        shift=8;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          shift-=4;
          index=ConstrainColormapIndex(image,15UL-((*p >> shift) & 0x0f),
            exception);
          SetPixelIndex(image,index,q);
          q+=GetPixelChannels(image);
          if (shift == 0)
            {
              shift=8;
              p++;
            }
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
      (void) SyncImage(image,exception);
      break;
    }
    default:
    {
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  }
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  if (pdb_info.number_records > 1)
    {
      char
        *comment;

      int
        c;

      char
        *r;

      size_t
        length;

      num_pad_bytes = (size_t) (comment_offset - TellBlob( image ));
      while (num_pad_bytes-- != 0)
      {
        c=ReadBlobByte(image);
        if (c == EOF)
          break;
      }

      /*
        Read comment.
      */
      c=ReadBlobByte(image);
      length=MagickPathExtent;
      comment=AcquireString((char *) NULL);
      for (r=comment; c != EOF; r++)
      {
        if ((size_t) (r-comment+MagickPathExtent) >= length)
          {
            *r='\0';
            length<<=1;
            length+=MagickPathExtent;
            comment=(char *) ResizeQuantumMemory(comment,length+
              MagickPathExtent,sizeof(*comment));
            if (comment == (char *) NULL)
              break;
            r=comment+strlen(comment);
          }
        *r=c;
        c=ReadBlobByte(image);
      }
      *r='\0';
      if (comment == (char *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      (void) SetImageProperty(image,"comment",comment,exception);
      comment=DestroyString(comment);
    }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P D B I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPDBImage() adds properties for the PDB image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPDBImage method is:
%
%      size_t RegisterPDBImage(void)
%
*/
ModuleExport size_t RegisterPDBImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("PDB","PDB","Palm Database ImageViewer Format");
  entry->decoder=(DecodeImageHandler *) ReadPDBImage;
  entry->encoder=(EncodeImageHandler *) WritePDBImage;
  entry->magick=(IsImageFormatHandler *) IsPDB;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P D B I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPDBImage() removes format registrations made by the
%  PDB module from the list of supported formats.
%
%  The format of the UnregisterPDBImage method is:
%
%      UnregisterPDBImage(void)
%
*/
ModuleExport void UnregisterPDBImage(void)
{
  (void) UnregisterMagickInfo("PDB");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P D B I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePDBImage() writes an image
%
%  The format of the WritePDBImage method is:
%
%      MagickBooleanType WritePDBImage(const ImageInfo *image_info,
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

static unsigned char *EncodeRLE(unsigned char *destination,
  unsigned char *source,size_t literal,size_t repeat)
{
  if (literal > 0)
    *destination++=(unsigned char) (literal-1);
  (void) memcpy(destination,source,literal);
  destination+=literal;
  if (repeat > 0)
    {
      *destination++=(unsigned char) (0x80 | (repeat-1));
      *destination++=source[literal];
    }
  return(destination);
}

static MagickBooleanType WritePDBImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent];

  const char
    *comment;

  int
    bits;

  MagickBooleanType
    status;

  PDBImage
    pdb_image;

  PDBInfo
    pdb_info;

  QuantumInfo
    *quantum_info;

  const Quantum
    *p;

  ssize_t
    x;

  unsigned char
    *q;

  size_t
    bits_per_pixel,
    literal,
    packets,
    packet_size,
    repeat;

  ssize_t
    y;

  unsigned char
    *buffer,
    *runlength,
    *scanline;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
  if (SetImageMonochrome(image,exception) != MagickFalse) {
    bits_per_pixel=1;
  } else if (image->colors <= 4) {
    bits_per_pixel=2;
  } else if (image->colors <= 8) {
    bits_per_pixel=3;
  } else {
    bits_per_pixel=4;
  }
  (void) memset(&pdb_info,0,sizeof(pdb_info));
  (void) memset(&pdb_image,0,sizeof(pdb_image));
  GetPathComponent(image_info->filename,TailPath,filename);
  (void) CopyMagickString(pdb_info.name,filename,sizeof(pdb_info.name));
  pdb_info.attributes=0;
  pdb_info.version=0;
  pdb_info.create_time=GetMagickTime();
  pdb_info.modify_time=pdb_info.create_time;
  pdb_info.archive_time=0;
  pdb_info.modify_number=0;
  pdb_info.application_info=0;
  pdb_info.sort_info=0;
  (void) memcpy(pdb_info.type,"vIMG",4);
  (void) memcpy(pdb_info.id,"View",4);
  pdb_info.seed=0;
  pdb_info.next_record=0;
  comment=GetImageProperty(image,"comment",exception);
  pdb_info.number_records=(comment == (const char *) NULL ? 1 : 2);
  (void) WriteBlob(image,sizeof(pdb_info.name),(unsigned char *) pdb_info.name);
  (void) WriteBlobMSBShort(image,(unsigned short) pdb_info.attributes);
  (void) WriteBlobMSBShort(image,(unsigned short) pdb_info.version);
  (void) WriteBlobMSBLong(image,(unsigned int) pdb_info.create_time);
  (void) WriteBlobMSBLong(image,(unsigned int) pdb_info.modify_time);
  (void) WriteBlobMSBLong(image,(unsigned int) pdb_info.archive_time);
  (void) WriteBlobMSBLong(image,(unsigned int) pdb_info.modify_number);
  (void) WriteBlobMSBLong(image,(unsigned int) pdb_info.application_info);
  (void) WriteBlobMSBLong(image,(unsigned int) pdb_info.sort_info);
  (void) WriteBlob(image,4,(unsigned char *) pdb_info.type);
  (void) WriteBlob(image,4,(unsigned char *) pdb_info.id);
  (void) WriteBlobMSBLong(image,(unsigned int) pdb_info.seed);
  (void) WriteBlobMSBLong(image,(unsigned int) pdb_info.next_record);
  (void) WriteBlobMSBShort(image,(unsigned short) pdb_info.number_records);
  (void) CopyMagickString(pdb_image.name,pdb_info.name,sizeof(pdb_image.name));
  pdb_image.version=1;  /* RLE Compressed */
  switch (bits_per_pixel)
  {
    case 1: pdb_image.type=(unsigned char) 0xff; break;  /* monochrome */
    case 2: pdb_image.type=(unsigned char) 0x00; break;  /* 2 bit gray */
    default: pdb_image.type=(unsigned char) 0x02;  /* 4 bit gray */
  }
  pdb_image.reserved_1=0;
  pdb_image.note=0;
  pdb_image.x_last=0;
  pdb_image.y_last=0;
  pdb_image.reserved_2=0;
  pdb_image.x_anchor=(unsigned short) 0xffff;
  pdb_image.y_anchor=(unsigned short) 0xffff;
  pdb_image.width=(short) image->columns;
  if (image->columns % 16)
    pdb_image.width=(short) (16*(image->columns/16+1));
  pdb_image.height=(short) image->rows;
  packets=((bits_per_pixel*image->columns+7)/8);
  packet_size=(size_t) (bits_per_pixel > 8 ? 2 : 1);
  runlength=(unsigned char *) AcquireQuantumMemory(9UL*packets,
    image->rows*sizeof(*runlength));
  buffer=(unsigned char *) AcquireQuantumMemory(512,sizeof(*buffer));
  scanline=(unsigned char *) AcquireQuantumMemory(image->columns,packet_size*
    sizeof(*scanline));
  if ((runlength == (unsigned char *) NULL) ||
      (buffer == (unsigned char *) NULL) ||
      (scanline == (unsigned char *) NULL))
    {
      if (runlength != (unsigned char *) NULL)
        runlength=(unsigned char *) RelinquishMagickMemory(runlength);
      if (buffer != (unsigned char *) NULL)
        buffer=(unsigned char *) RelinquishMagickMemory(buffer);
      if (scanline != (unsigned char *) NULL)
        scanline=(unsigned char *) RelinquishMagickMemory(scanline);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  (void) memset(buffer,0,512*sizeof(*buffer));
  (void) memset(scanline,0,image->columns*packet_size*sizeof(*scanline));
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
      (void) TransformImageColorspace(image,sRGBColorspace,exception);
  /*
    Convert to GRAY raster scanline.
  */
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    {
      if (runlength != (unsigned char *) NULL)
        runlength=(unsigned char *) RelinquishMagickMemory(runlength);
      if (buffer != (unsigned char *) NULL)
        buffer=(unsigned char *) RelinquishMagickMemory(buffer);
      if (scanline != (unsigned char *) NULL)
        scanline=(unsigned char *) RelinquishMagickMemory(scanline);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  status=SetQuantumDepth(image,quantum_info,bits_per_pixel > 8 ? 16 : 8);
  bits=8/(int) bits_per_pixel-1;  /* start at most significant bits */
  literal=0;
  repeat=0;
  q=runlength;
  buffer[0]=0x00;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    (void) ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      GrayQuantum,scanline,exception);
    for (x=0; x < (ssize_t) pdb_image.width; x++)
    {
      if (x < (ssize_t) image->columns)
        buffer[literal+repeat]|=(0xff-scanline[x*(ssize_t) packet_size]) >>
          (8-bits_per_pixel) << bits*(ssize_t) bits_per_pixel;
      bits--;
      if (bits < 0)
        {
          if (((literal+repeat) > 0) &&
              (buffer[literal+repeat] == buffer[literal+repeat-1]))
            {
              if (repeat == 0)
                {
                  literal--;
                  repeat++;
                }
              repeat++;
              if (0x7f < repeat)
                {
                  q=EncodeRLE(q,buffer,literal,repeat);
                  literal=0;
                  repeat=0;
                }
            }
          else
            {
              if (repeat >= 2)
                literal+=repeat;
              else
                {
                  q=EncodeRLE(q,buffer,literal,repeat);
                  buffer[0]=buffer[literal+repeat];
                  literal=0;
                }
              literal++;
              repeat=0;
              if (0x7f < literal)
                {
                  q=EncodeRLE(q,buffer,(literal < 0x80 ? literal : 0x80),0);
                  (void) memmove(buffer,buffer+literal+repeat,0x80);
                  literal-=0x80;
                }
            }
        bits=8/(int) bits_per_pixel-1;
        buffer[literal+repeat]=0x00;
      }
    }
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  q=EncodeRLE(q,buffer,literal,repeat);
  scanline=(unsigned char *) RelinquishMagickMemory(scanline);
  buffer=(unsigned char *) RelinquishMagickMemory(buffer);
  quantum_info=DestroyQuantumInfo(quantum_info);
  /*
    Write the Image record header.
  */
  (void) WriteBlobMSBLong(image,(unsigned int)
    (TellBlob(image)+8*pdb_info.number_records));
  (void) WriteBlobByte(image,0x40);
  (void) WriteBlobByte(image,0x6f);
  (void) WriteBlobByte(image,0x80);
  (void) WriteBlobByte(image,0);
  if (pdb_info.number_records > 1)
    {
      /*
        Write the comment record header.
      */
      (void) WriteBlobMSBLong(image,(unsigned int) (TellBlob(image)+8+58+q-
        runlength));
      (void) WriteBlobByte(image,0x40);
      (void) WriteBlobByte(image,0x6f);
      (void) WriteBlobByte(image,0x80);
      (void) WriteBlobByte(image,1);
    }
  /*
    Write the Image data.
  */
  (void) WriteBlob(image,sizeof(pdb_image.name),(unsigned char *)
    pdb_image.name);
  (void) WriteBlobByte(image,(unsigned char) pdb_image.version);
  (void) WriteBlobByte(image,pdb_image.type);
  (void) WriteBlobMSBLong(image,(unsigned int) pdb_image.reserved_1);
  (void) WriteBlobMSBLong(image,(unsigned int) pdb_image.note);
  (void) WriteBlobMSBShort(image,(unsigned short) pdb_image.x_last);
  (void) WriteBlobMSBShort(image,(unsigned short) pdb_image.y_last);
  (void) WriteBlobMSBLong(image,(unsigned int) pdb_image.reserved_2);
  (void) WriteBlobMSBShort(image,pdb_image.x_anchor);
  (void) WriteBlobMSBShort(image,pdb_image.y_anchor);
  (void) WriteBlobMSBShort(image,(unsigned short) pdb_image.width);
  (void) WriteBlobMSBShort(image,(unsigned short) pdb_image.height);
  (void) WriteBlob(image,(size_t) (q-runlength),runlength);
  runlength=(unsigned char *) RelinquishMagickMemory(runlength);
  if (comment != (const char *) NULL)
    (void) WriteBlobString(image,comment);
  (void) CloseBlob(image);
  return(MagickTrue);
}

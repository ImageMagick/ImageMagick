/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP    CCCC  X   X                              %
%                            P   P  C       X X                               %
%                            PPPP   C        X                                %
%                            P      C       X X                               %
%                            P       CCCC  X   X                              %
%                                                                             %
%                                                                             %
%                Read/Write ZSoft IBM PC Paintbrush Image Format              %
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
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

/*
  Typedef declarations.
*/
typedef struct _PCXInfo
{
  unsigned char
    identifier,
    version,
    encoding,
    bits_per_pixel;

  unsigned short
    left,
    top,
    right,
    bottom,
    horizontal_resolution,
    vertical_resolution;

  unsigned char
    reserved,
    planes;

  unsigned short
    bytes_per_line,
    palette_info;

  unsigned char
    colormap_signature;
} PCXInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePCXImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s D C X                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsDCX() returns MagickTrue if the image format type, identified by the
%  magick string, is DCX.
%
%  The format of the IsDCX method is:
%
%      MagickBooleanType IsDCX(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsDCX(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\261\150\336\72",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P C X                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPCX() returns MagickTrue if the image format type, identified by the
%  magick string, is PCX.
%
%  The format of the IsPCX method is:
%
%      MagickBooleanType IsPCX(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsPCX(const unsigned char *magick,const size_t length)
{
  if (length < 2)
    return(MagickFalse);
  if (memcmp(magick,"\012\002",2) == 0)
    return(MagickTrue);
  if (memcmp(magick,"\012\005",2) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P C X I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPCXImage() reads a ZSoft IBM PC Paintbrush file and returns it.
%  It allocates the memory necessary for the new Image structure and returns
%  a pointer to the new image.
%
%  The format of the ReadPCXImage method is:
%
%      Image *ReadPCXImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline ssize_t MagickAbsoluteValue(const ssize_t x)
{
  if (x < 0)
    return(-x);
  return(x);
}

static inline size_t MagickMax(const size_t x,const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static Image *ReadPCXImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  int
    bits,
    id,
    mask;

  MagickBooleanType
    status;

  MagickOffsetType
    offset,
    *page_table;

  PCXInfo
    pcx_info;

  register IndexPacket
    *indexes;

  register ssize_t
    x;

  register PixelPacket
    *q;

  register ssize_t
    i;

  register unsigned char
    *p,
    *r;

  size_t
    one,
    pcx_packets;

  ssize_t
    count,
    y;

  unsigned char
    packet,
    *pcx_colormap,
    *pcx_pixels,
    *scanline;

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
  /*
    Determine if this a PCX file.
  */
  page_table=(MagickOffsetType *) NULL;
  if (LocaleCompare(image_info->magick,"DCX") == 0)
    {
      size_t
        magic;

      /*
        Read the DCX page table.
      */
      magic=ReadBlobLSBLong(image);
      if (magic != 987654321)
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      page_table=(MagickOffsetType *) AcquireQuantumMemory(1024UL,
        sizeof(*page_table));
      if (page_table == (MagickOffsetType *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      for (id=0; id < 1024; id++)
      {
        page_table[id]=(MagickOffsetType) ReadBlobLSBLong(image);
        if (page_table[id] == 0)
          break;
      }
    }
  if (page_table != (MagickOffsetType *) NULL)
    {
      offset=SeekBlob(image,(MagickOffsetType) page_table[0],SEEK_SET);
      if (offset < 0)
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  pcx_colormap=(unsigned char *) NULL;
  count=ReadBlob(image,1,&pcx_info.identifier);
  for (id=1; id < 1024; id++)
  {
    /*
      Verify PCX identifier.
    */
    pcx_info.version=(unsigned char) ReadBlobByte(image);
    if ((count == 0) || (pcx_info.identifier != 0x0a))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    pcx_info.encoding=(unsigned char) ReadBlobByte(image);
    pcx_info.bits_per_pixel=(unsigned char) ReadBlobByte(image);
    pcx_info.left=ReadBlobLSBShort(image);
    pcx_info.top=ReadBlobLSBShort(image);
    pcx_info.right=ReadBlobLSBShort(image);
    pcx_info.bottom=ReadBlobLSBShort(image);
    pcx_info.horizontal_resolution=ReadBlobLSBShort(image);
    pcx_info.vertical_resolution=ReadBlobLSBShort(image);
    /*
      Read PCX raster colormap.
    */
    image->columns=(size_t) MagickAbsoluteValue((ssize_t) pcx_info.right-
      pcx_info.left)+1UL;
    image->rows=(size_t) MagickAbsoluteValue((ssize_t) pcx_info.bottom-
      pcx_info.top)+1UL;
    if ((image->columns == 0) || (image->rows == 0) ||
        (pcx_info.bits_per_pixel == 0))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    image->depth=pcx_info.bits_per_pixel <= 8 ? 8U : MAGICKCORE_QUANTUM_DEPTH;
    image->units=PixelsPerInchResolution;
    image->x_resolution=(double) pcx_info.horizontal_resolution;
    image->y_resolution=(double) pcx_info.vertical_resolution;
    image->colors=16;
    pcx_colormap=(unsigned char *) AcquireQuantumMemory(256UL,
      3*sizeof(*pcx_colormap));
    if (pcx_colormap == (unsigned char *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    count=ReadBlob(image,3*image->colors,pcx_colormap);
    pcx_info.reserved=(unsigned char) ReadBlobByte(image);
    pcx_info.planes=(unsigned char) ReadBlobByte(image);
    one=1;
    if ((pcx_info.bits_per_pixel != 8) || (pcx_info.planes == 1))
      if ((pcx_info.version == 3) || (pcx_info.version == 5) ||
          ((pcx_info.bits_per_pixel*pcx_info.planes) == 1))
        image->colors=(size_t) MagickMin(one << (1UL*
          (pcx_info.bits_per_pixel*pcx_info.planes)),256UL);
    if (AcquireImageColormap(image,image->colors) == MagickFalse)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    if ((pcx_info.bits_per_pixel >= 8) && (pcx_info.planes != 1))
      image->storage_class=DirectClass;
    p=pcx_colormap;
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      image->colormap[i].red=ScaleCharToQuantum(*p++);
      image->colormap[i].green=ScaleCharToQuantum(*p++);
      image->colormap[i].blue=ScaleCharToQuantum(*p++);
    }
    pcx_info.bytes_per_line=ReadBlobLSBShort(image);
    pcx_info.palette_info=ReadBlobLSBShort(image);
    for (i=0; i < 58; i++)
      (void) ReadBlobByte(image);
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    /*
      Read image data.
    */
    pcx_packets=(size_t) image->rows*pcx_info.bytes_per_line*
      pcx_info.planes;
    pcx_pixels=(unsigned char *) AcquireQuantumMemory(pcx_packets,
      sizeof(*pcx_pixels));
    scanline=(unsigned char *) AcquireQuantumMemory(MagickMax(image->columns,
      pcx_info.bytes_per_line),MagickMax(8,pcx_info.planes)*sizeof(*scanline));
    if ((pcx_pixels == (unsigned char *) NULL) ||
        (scanline == (unsigned char *) NULL))
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    /*
      Uncompress image data.
    */
    p=pcx_pixels;
    if (pcx_info.encoding == 0)
      while (pcx_packets != 0)
      {
        packet=(unsigned char) ReadBlobByte(image);
        if (EOFBlob(image) != MagickFalse)
          ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        *p++=packet;
        pcx_packets--;
      }
    else
      while (pcx_packets != 0)
      {
        packet=(unsigned char) ReadBlobByte(image);
        if (EOFBlob(image) != MagickFalse)
          ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        if ((packet & 0xc0) != 0xc0)
          {
            *p++=packet;
            pcx_packets--;
            continue;
          }
        count=(ssize_t) (packet & 0x3f);
        packet=(unsigned char) ReadBlobByte(image);
        if (EOFBlob(image) != MagickFalse)
          ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        for ( ; count != 0; count--)
        {
          *p++=packet;
          pcx_packets--;
          if (pcx_packets == 0)
            break;
        }
      }
    if (image->storage_class == DirectClass)
      image->matte=pcx_info.planes > 3 ? MagickTrue : MagickFalse;
    else
      if ((pcx_info.version == 5) ||
          ((pcx_info.bits_per_pixel*pcx_info.planes) == 1))
        {
          /*
            Initialize image colormap.
          */
          if (image->colors > 256)
            ThrowReaderException(CorruptImageError,"ColormapExceeds256Colors");
          if ((pcx_info.bits_per_pixel*pcx_info.planes) == 1)
            {
              /*
                Monochrome colormap.
              */
              image->colormap[0].red=(Quantum) 0;
              image->colormap[0].green=(Quantum) 0;
              image->colormap[0].blue=(Quantum) 0;
              image->colormap[1].red=QuantumRange;
              image->colormap[1].green=QuantumRange;
              image->colormap[1].blue=QuantumRange;
            }
          else
            if (image->colors > 16)
              {
                /*
                  256 color images have their color map at the end of the file.
                */
                pcx_info.colormap_signature=(unsigned char) ReadBlobByte(image);
                count=ReadBlob(image,3*image->colors,pcx_colormap);
                p=pcx_colormap;
                for (i=0; i < (ssize_t) image->colors; i++)
                {
                  image->colormap[i].red=ScaleCharToQuantum(*p++);
                  image->colormap[i].green=ScaleCharToQuantum(*p++);
                  image->colormap[i].blue=ScaleCharToQuantum(*p++);
                }
            }
          pcx_colormap=(unsigned char *) RelinquishMagickMemory(pcx_colormap);
        }
    /*
      Convert PCX raster image to pixel packets.
    */
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=pcx_pixels+(y*pcx_info.bytes_per_line*pcx_info.planes);
      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (PixelPacket *) NULL)
        break;
      indexes=GetAuthenticIndexQueue(image);
      r=scanline;
      if (image->storage_class == DirectClass)
        for (i=0; i < pcx_info.planes; i++)
        {
          r=scanline+i;
          for (x=0; x < (ssize_t) pcx_info.bytes_per_line; x++)
          {
            switch (i)
            {
              case 0:
              {
                *r=(*p++);
                break;
              }
              case 1:
              {
                *r=(*p++);
                break;
              }
              case 2:
              {
                *r=(*p++);
                break;
              }
              case 3:
              default:
              {
                *r=(*p++);
                break;
              }
            }
            r+=pcx_info.planes;
          }
        }
      else
        if (pcx_info.planes > 1)
          {
            for (x=0; x < (ssize_t) image->columns; x++)
              *r++=0;
            for (i=0; i < pcx_info.planes; i++)
            {
              r=scanline;
              for (x=0; x < (ssize_t) pcx_info.bytes_per_line; x++)
              {
                 bits=(*p++);
                 for (mask=0x80; mask != 0; mask>>=1)
                 {
                   if (bits & mask)
                     *r|=1 << i;
                   r++;
                 }
               }
            }
          }
        else
          switch (pcx_info.bits_per_pixel)
          {
            case 1:
            {
              register ssize_t
                bit;

              for (x=0; x < ((ssize_t) image->columns-7); x+=8)
              {
                for (bit=7; bit >= 0; bit--)
                  *r++=(unsigned char) ((*p) & (0x01 << bit) ? 0x01 : 0x00);
                p++;
              }
              if ((image->columns % 8) != 0)
                {
                  for (bit=7; bit >= (ssize_t) (8-(image->columns % 8)); bit--)
                    *r++=(unsigned char) ((*p) & (0x01 << bit) ? 0x01 : 0x00);
                  p++;
                }
              break;
            }
            case 2:
            {
              for (x=0; x < ((ssize_t) image->columns-3); x+=4)
              {
                *r++=(*p >> 6) & 0x3;
                *r++=(*p >> 4) & 0x3;
                *r++=(*p >> 2) & 0x3;
                *r++=(*p) & 0x3;
                p++;
              }
              if ((image->columns % 4) != 0)
                {
                  for (i=3; i >= (ssize_t) (4-(image->columns % 4)); i--)
                    *r++=(unsigned char) ((*p >> (i*2)) & 0x03);
                  p++;
                }
              break;
            }
            case 4:
            {
              for (x=0; x < ((ssize_t) image->columns-1); x+=2)
              {
                *r++=(*p >> 4) & 0xf;
                *r++=(*p) & 0xf;
                p++;
              }
              if ((image->columns % 2) != 0)
                *r++=(*p++ >> 4) & 0xf;
              break;
            }
            case 8:
            {
              (void) CopyMagickMemory(r,p,image->columns);
              break;
            }
            default:
              break;
          }
      /*
        Transfer image scanline.
      */
      r=scanline;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        if (image->storage_class == PseudoClass)
          SetPixelIndex(indexes+x,*r++);
        else
          {
            SetPixelRed(q,ScaleCharToQuantum(*r++));
            SetPixelGreen(q,ScaleCharToQuantum(*r++));
            SetPixelBlue(q,ScaleCharToQuantum(*r++));
            if (image->matte != MagickFalse)
              SetPixelAlpha(q,ScaleCharToQuantum(*r++));
          }
        q++;
      }
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        break;
      if (image->previous == (Image *) NULL)
        {
          status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
    }
    if (image->storage_class == PseudoClass)
      (void) SyncImage(image);
    scanline=(unsigned char *) RelinquishMagickMemory(scanline);
    if (pcx_colormap != (unsigned char *) NULL)
      pcx_colormap=(unsigned char *) RelinquishMagickMemory(pcx_colormap);
    pcx_pixels=(unsigned char *) RelinquishMagickMemory(pcx_pixels);
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if (page_table == (MagickOffsetType *) NULL)
      break;
    if (page_table[id] == 0)
      break;
    offset=SeekBlob(image,(MagickOffsetType) page_table[id],SEEK_SET);
    if (offset < 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    count=ReadBlob(image,1,&pcx_info.identifier);
    if ((count != 0) && (pcx_info.identifier == 0x0a))
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
  }
  if (page_table != (MagickOffsetType *) NULL)
    page_table=(MagickOffsetType *) RelinquishMagickMemory(page_table);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P C X I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPCXImage() adds attributes for the PCX image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPCXImage method is:
%
%      size_t RegisterPCXImage(void)
%
*/
ModuleExport size_t RegisterPCXImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("DCX");
  entry->decoder=(DecodeImageHandler *) ReadPCXImage;
  entry->encoder=(EncodeImageHandler *) WritePCXImage;
  entry->seekable_stream=MagickTrue;
  entry->magick=(IsImageFormatHandler *) IsDCX;
  entry->description=ConstantString("ZSoft IBM PC multi-page Paintbrush");
  entry->module=ConstantString("PCX");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PCX");
  entry->decoder=(DecodeImageHandler *) ReadPCXImage;
  entry->encoder=(EncodeImageHandler *) WritePCXImage;
  entry->magick=(IsImageFormatHandler *) IsPCX;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("ZSoft IBM PC Paintbrush");
  entry->module=ConstantString("PCX");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P C X I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPCXImage() removes format registrations made by the
%  PCX module from the list of supported formats.
%
%  The format of the UnregisterPCXImage method is:
%
%      UnregisterPCXImage(void)
%
*/
ModuleExport void UnregisterPCXImage(void)
{
  (void) UnregisterMagickInfo("DCX");
  (void) UnregisterMagickInfo("PCX");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P C X I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePCXImage() writes an image in the ZSoft IBM PC Paintbrush file
%  format.
%
%  The format of the WritePCXImage method is:
%
%      MagickBooleanType WritePCXImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%
*/
static MagickBooleanType PCXWritePixels(PCXInfo *pcx_info,
  const unsigned char *pixels,Image *image)
{
  register const unsigned char
    *q;

  register ssize_t
    i,
    x;

  ssize_t
    count;

  unsigned char
    packet,
    previous;

  q=pixels;
  for (i=0; i < (ssize_t) pcx_info->planes; i++)
  {
    if (pcx_info->encoding == 0)
      {
        for (x=0; x < (ssize_t) pcx_info->bytes_per_line; x++)
          (void) WriteBlobByte(image,(unsigned char) (*q++));
      }
    else
      {
        previous=(*q++);
        count=1;
        for (x=0; x < (ssize_t) (pcx_info->bytes_per_line-1); x++)
        {
          packet=(*q++);
          if ((packet == previous) && (count < 63))
            {
              count++;
              continue;
            }
          if ((count > 1) || ((previous & 0xc0) == 0xc0))
            {
              count|=0xc0;
              (void) WriteBlobByte(image,(unsigned char) count);
            }
          (void) WriteBlobByte(image,previous);
          previous=packet;
          count=1;
        }
        if ((count > 1) || ((previous & 0xc0) == 0xc0))
          {
            count|=0xc0;
            (void) WriteBlobByte(image,(unsigned char) count);
          }
        (void) WriteBlobByte(image,previous);
      }
  }
  return (MagickTrue);
}

static MagickBooleanType WritePCXImage(const ImageInfo *image_info,Image *image)
{
  MagickBooleanType
    status;

  MagickOffsetType
    offset,
    *page_table,
    scene;

  PCXInfo
    pcx_info;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register ssize_t
    i,
    x;

  register unsigned char
    *q;

  size_t
    length;

  ssize_t
    y;

  unsigned char
    *pcx_colormap,
    *pcx_pixels;

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
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace);
  page_table=(MagickOffsetType *) NULL;
  if ((LocaleCompare(image_info->magick,"DCX") == 0) ||
      ((GetNextImageInList(image) != (Image *) NULL) &&
       (image_info->adjoin != MagickFalse)))
    {
      /*
        Write the DCX page table.
      */
      (void) WriteBlobLSBLong(image,0x3ADE68B1L);
      page_table=(MagickOffsetType *) AcquireQuantumMemory(1024UL,
        sizeof(*page_table));
      if (page_table == (MagickOffsetType *) NULL)
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      for (scene=0; scene < 1024; scene++)
        (void) WriteBlobLSBLong(image,0x00000000L);
    }
  scene=0;
  do
  {
    if (page_table != (MagickOffsetType *) NULL)
      page_table[scene]=TellBlob(image);
    /*
      Initialize PCX raster file header.
    */
    pcx_info.identifier=0x0a;
    pcx_info.version=5;
    pcx_info.encoding=image_info->compression == NoCompression ? 0 : 1;
    pcx_info.bits_per_pixel=8;
    if ((image->storage_class == PseudoClass) &&
        (IsMonochromeImage(image,&image->exception) != MagickFalse))
      pcx_info.bits_per_pixel=1;
    pcx_info.left=0;
    pcx_info.top=0;
    pcx_info.right=(unsigned short) (image->columns-1);
    pcx_info.bottom=(unsigned short) (image->rows-1);
    switch (image->units)
    {
      case UndefinedResolution:
      case PixelsPerInchResolution:
      default:
      {
        pcx_info.horizontal_resolution=(unsigned short) image->x_resolution;
        pcx_info.vertical_resolution=(unsigned short) image->y_resolution;
        break;
      }
      case PixelsPerCentimeterResolution:
      {
        pcx_info.horizontal_resolution=(unsigned short)
          (2.54*image->x_resolution+0.5);
        pcx_info.vertical_resolution=(unsigned short)
          (2.54*image->y_resolution+0.5);
        break;
      }
    }
    pcx_info.reserved=0;
    pcx_info.planes=1;
    if ((image->storage_class == DirectClass) || (image->colors > 256))
      {
        pcx_info.planes=3;
        if (image->matte != MagickFalse)
          pcx_info.planes++;
      }
    pcx_info.bytes_per_line=(unsigned short) (((size_t) image->columns*
      pcx_info.bits_per_pixel+7)/8);
    pcx_info.palette_info=1;
    pcx_info.colormap_signature=0x0c;
    /*
      Write PCX header.
    */
    (void) WriteBlobByte(image,pcx_info.identifier);
    (void) WriteBlobByte(image,pcx_info.version);
    (void) WriteBlobByte(image,pcx_info.encoding);
    (void) WriteBlobByte(image,pcx_info.bits_per_pixel);
    (void) WriteBlobLSBShort(image,pcx_info.left);
    (void) WriteBlobLSBShort(image,pcx_info.top);
    (void) WriteBlobLSBShort(image,pcx_info.right);
    (void) WriteBlobLSBShort(image,pcx_info.bottom);
    (void) WriteBlobLSBShort(image,pcx_info.horizontal_resolution);
    (void) WriteBlobLSBShort(image,pcx_info.vertical_resolution);
    /*
      Dump colormap to file.
    */
    pcx_colormap=(unsigned char *) AcquireQuantumMemory(256UL,
      3*sizeof(*pcx_colormap));
    if (pcx_colormap == (unsigned char *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    (void) memset(pcx_colormap,0,3*256*sizeof(*pcx_colormap));
    q=pcx_colormap;
    if ((image->storage_class == PseudoClass) && (image->colors <= 256))
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        *q++=ScaleQuantumToChar(image->colormap[i].red);
        *q++=ScaleQuantumToChar(image->colormap[i].green);
        *q++=ScaleQuantumToChar(image->colormap[i].blue);
      }
    (void) WriteBlob(image,3*16,(const unsigned char *) pcx_colormap);
    (void) WriteBlobByte(image,pcx_info.reserved);
    (void) WriteBlobByte(image,pcx_info.planes);
    (void) WriteBlobLSBShort(image,pcx_info.bytes_per_line);
    (void) WriteBlobLSBShort(image,pcx_info.palette_info);
    for (i=0; i < 58; i++)
      (void) WriteBlobByte(image,'\0');
    length=(size_t) pcx_info.bytes_per_line;
    pcx_pixels=(unsigned char *) AcquireQuantumMemory(length,pcx_info.planes*
      sizeof(*pcx_pixels));
    if (pcx_pixels == (unsigned char *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    q=pcx_pixels;
    if ((image->storage_class == DirectClass) || (image->colors > 256))
      {
        const PixelPacket
          *pixels;

        /*
          Convert DirectClass image to PCX raster pixels.
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          pixels=GetVirtualPixels(image,0,y,image->columns,1,
            &image->exception);
          if (pixels == (const PixelPacket *) NULL)
            break;
          q=pcx_pixels;
          for (i=0; i < pcx_info.planes; i++)
          {
            p=pixels;
            switch ((int) i)
            {
              case 0:
              {
                for (x=0; x < (ssize_t) pcx_info.bytes_per_line; x++)
                {
                  *q++=ScaleQuantumToChar(GetPixelRed(p));
                  p++;
                }
                break;
              }
              case 1:
              {
                for (x=0; x < (ssize_t) pcx_info.bytes_per_line; x++)
                {
                  *q++=ScaleQuantumToChar(GetPixelGreen(p));
                  p++;
                }
                break;
              }
              case 2:
              {
                for (x=0; x < (ssize_t) pcx_info.bytes_per_line; x++)
                {
                  *q++=ScaleQuantumToChar(GetPixelBlue(p));
                  p++;
                }
                break;
              }
              case 3:
              default:
              {
                for (x=(ssize_t) pcx_info.bytes_per_line; x != 0; x--)
                {
                  *q++=ScaleQuantumToChar((Quantum)
                    (GetPixelAlpha(p)));
                  p++;
                }
                break;
              }
            }
          }
          if (PCXWritePixels(&pcx_info,pcx_pixels,image) == MagickFalse)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
      }
    else
      {
        if (pcx_info.bits_per_pixel > 1)
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
            if (p == (const PixelPacket *) NULL)
              break;
            indexes=GetVirtualIndexQueue(image);
            q=pcx_pixels;
            for (x=0; x < (ssize_t) image->columns; x++)
              *q++=(unsigned char) GetPixelIndex(indexes+x);
            if (PCXWritePixels(&pcx_info,pcx_pixels,image) == MagickFalse)
              break;
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
        else
          {
            IndexPacket
              polarity;

            register unsigned char
              bit,
              byte;

            /*
              Convert PseudoClass image to a PCX monochrome image.
            */
            polarity=(IndexPacket) (PixelIntensityToQuantum(image,
              &image->colormap[0]) < (QuantumRange/2) ? 1 : 0);
            if (image->colors == 2)
              polarity=(IndexPacket) (
                PixelIntensityToQuantum(image,&image->colormap[0]) <
                PixelIntensityToQuantum(image,&image->colormap[1]) ? 1 : 0);
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,
                &image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              indexes=GetVirtualIndexQueue(image);
              bit=0;
              byte=0;
              q=pcx_pixels;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                byte<<=1;
                if (GetPixelIndex(indexes+x) == polarity)
                  byte|=0x01;
                bit++;
                if (bit == 8)
                  {
                    *q++=byte;
                    bit=0;
                    byte=0;
                  }
                p++;
              }
              if (bit != 0)
                *q++=byte << (8-bit);
              if (PCXWritePixels(&pcx_info,pcx_pixels,image) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
          }
        (void) WriteBlobByte(image,pcx_info.colormap_signature);
        (void) WriteBlob(image,3*256,pcx_colormap);
      }
    pcx_pixels=(unsigned char *) RelinquishMagickMemory(pcx_pixels);
    pcx_colormap=(unsigned char *) RelinquishMagickMemory(pcx_colormap);
    if (page_table == (MagickOffsetType *) NULL)
      break;
    if (scene >= 1023)
      break;
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  if (page_table != (MagickOffsetType *) NULL)
    {
      /*
        Write the DCX page table.
      */
      page_table[scene+1]=0;
      offset=SeekBlob(image,0L,SEEK_SET);
      if (offset < 0)
        ThrowWriterException(CorruptImageError,"ImproperImageHeader");
      (void) WriteBlobLSBLong(image,0x3ADE68B1L);
      for (i=0; i <= (ssize_t) scene; i++)
        (void) WriteBlobLSBLong(image,(unsigned int) page_table[i]);
      page_table=(MagickOffsetType *) RelinquishMagickMemory(page_table);
    }
  if (status == MagickFalse)
    {
      char
        *message;

      message=GetExceptionMessage(errno);
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        FileOpenError,"UnableToWriteFile","`%s': %s",image->filename,message);
      message=DestroyString(message);
    }
  (void) CloseBlob(image);
  return(MagickTrue);
}

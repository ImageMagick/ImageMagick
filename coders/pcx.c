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
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

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
    palette_info,
    horizontal_screensize,
    vertical_screensize;

  unsigned char
    colormap_signature;
} PCXInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePCXImage(const ImageInfo *,Image *,ExceptionInfo *);

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
static Image *ReadPCXImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define MaxNumberScenes  1024
#define ThrowPCXException(severity,tag) \
{ \
  if (scanline != (unsigned char *) NULL) \
    scanline=(unsigned char *) RelinquishMagickMemory(scanline); \
  if (pixel_info != (MemoryInfo *) NULL) \
    pixel_info=RelinquishVirtualMemory(pixel_info); \
  if (page_table != (MagickOffsetType *) NULL) \
    page_table=(MagickOffsetType *) RelinquishMagickMemory(page_table); \
  ThrowReaderException(severity,tag); \
}

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

  MemoryInfo
    *pixel_info;

  PCXInfo
    pcx_info;

  Quantum
    *q;

  size_t
    one,
    pcx_packets;

  ssize_t
    count,
    i,
    x,
    y;

  unsigned char
    *p,
    packet,
    pcx_colormap[768],
    *pixels,
    *r,
    *scanline;

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
    Determine if this a PCX file.
  */
  page_table=(MagickOffsetType *) NULL;
  scanline=(unsigned char *) NULL;
  pixel_info=(MemoryInfo *) NULL;
  if (LocaleCompare(image_info->magick,"DCX") == 0)
    {
      size_t
        magic;

      /*
        Read the DCX page table.
      */
      magic=ReadBlobLSBLong(image);
      if (magic != 987654321)
        ThrowPCXException(CorruptImageError,"ImproperImageHeader");
      page_table=(MagickOffsetType *) AcquireQuantumMemory(MaxNumberScenes,
        sizeof(*page_table));
      if (page_table == (MagickOffsetType *) NULL)
        ThrowPCXException(ResourceLimitError,"MemoryAllocationFailed");
      for (id=0; id < MaxNumberScenes; id++)
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
        ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    }
  count=ReadBlob(image,1,&pcx_info.identifier);
  for (id=1; id < MaxNumberScenes; id++)
  {
    int
      bits_per_pixel;

    /*
      Verify PCX identifier.
    */
    pcx_info.version=(unsigned char) ReadBlobByte(image);
    if ((count != 1) || (pcx_info.identifier != 0x0a))
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    pcx_info.encoding=(unsigned char) ReadBlobByte(image);
    bits_per_pixel=ReadBlobByte(image);
    if (bits_per_pixel == -1)
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    pcx_info.bits_per_pixel=(unsigned char) bits_per_pixel;
    pcx_info.left=ReadBlobLSBShort(image);
    pcx_info.top=ReadBlobLSBShort(image);
    pcx_info.right=ReadBlobLSBShort(image);
    pcx_info.bottom=ReadBlobLSBShort(image);
    pcx_info.horizontal_resolution=ReadBlobLSBShort(image);
    pcx_info.vertical_resolution=ReadBlobLSBShort(image);
    if (EOFBlob(image) != MagickFalse)
      ThrowPCXException(CorruptImageError,"UnexpectedEndOfFile");
    /*
      Read PCX raster colormap.
    */
    if ((pcx_info.right < pcx_info.left) || (pcx_info.bottom < pcx_info.top) ||
        ((pcx_info.bits_per_pixel != 1) && (pcx_info.bits_per_pixel != 2) &&
         (pcx_info.bits_per_pixel != 4) && (pcx_info.bits_per_pixel != 8)))
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    image->columns=(size_t) (pcx_info.right-pcx_info.left)+1UL;
    image->rows=(size_t) (pcx_info.bottom-pcx_info.top)+1UL;
    image->depth=pcx_info.bits_per_pixel;
    image->units=PixelsPerInchResolution;
    image->resolution.x=(double) pcx_info.horizontal_resolution;
    image->resolution.y=(double) pcx_info.vertical_resolution;
    image->colors=16;
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if ((MagickSizeType) (image->columns*image->rows/255) > GetBlobSize(image))
      ThrowPCXException(CorruptImageError,"InsufficientImageDataInFile");
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      ThrowPCXException(exception->severity,exception->reason);
    (void) SetImageBackgroundColor(image,exception);
    (void) memset(pcx_colormap,0,sizeof(pcx_colormap));
    count=ReadBlob(image,3*image->colors,pcx_colormap);
    if (count != (ssize_t) (3*image->colors))
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    pcx_info.reserved=(unsigned char) ReadBlobByte(image);
    pcx_info.planes=(unsigned char) ReadBlobByte(image);
    if (pcx_info.planes == 0)
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    if (pcx_info.planes > 6)
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    if ((pcx_info.bits_per_pixel*pcx_info.planes) >= 64)
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    one=1;
    if ((pcx_info.bits_per_pixel != 8) || (pcx_info.planes == 1))
      if ((pcx_info.version == 3) || (pcx_info.version == 5) ||
          ((pcx_info.bits_per_pixel*pcx_info.planes) == 1))
        image->colors=(size_t) MagickMin(one << (1UL*
          (pcx_info.bits_per_pixel*pcx_info.planes)),256UL);
    if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
      ThrowPCXException(ResourceLimitError,"MemoryAllocationFailed");
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
    pcx_info.horizontal_screensize=ReadBlobLSBShort(image);
    pcx_info.vertical_screensize=ReadBlobLSBShort(image);
    for (i=0; i < 54; i++)
      (void) ReadBlobByte(image);
    /*
      Read image data.
    */
    if (HeapOverflowSanityCheckGetSize(image->rows,(size_t) pcx_info.bytes_per_line,&pcx_packets) != MagickFalse)
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    if (HeapOverflowSanityCheckGetSize(pcx_packets,(size_t) pcx_info.planes,&pcx_packets) != MagickFalse)
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    if ((size_t) (pcx_info.bits_per_pixel*pcx_info.planes*image->columns) > (pcx_packets*8U))
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    if ((MagickSizeType) (pcx_packets/32+128) > GetBlobSize(image))
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    scanline=(unsigned char *) AcquireQuantumMemory(MagickMax(image->columns,
      pcx_info.bytes_per_line),MagickMax(pcx_info.planes,8)*sizeof(*scanline));
    pixel_info=AcquireVirtualMemory(pcx_packets,2*sizeof(*pixels));
    if ((scanline == (unsigned char *) NULL) ||
        (pixel_info == (MemoryInfo *) NULL))
      {
        if (scanline != (unsigned char *) NULL)
          scanline=(unsigned char *) RelinquishMagickMemory(scanline);
        if (pixel_info != (MemoryInfo *) NULL)
          pixel_info=RelinquishVirtualMemory(pixel_info);
        ThrowPCXException(ResourceLimitError,"MemoryAllocationFailed");
      }
    (void) memset(scanline,0,(size_t) MagickMax(image->columns,
      pcx_info.bytes_per_line)*MagickMax(pcx_info.planes,8)*sizeof(*scanline));
    pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
    (void) memset(pixels,0,(size_t) pcx_packets*(2*sizeof(*pixels)));
    /*
      Uncompress image data.
    */
    p=pixels;
    if (pcx_info.encoding == 0)
      while (pcx_packets != 0)
      {
        packet=(unsigned char) ReadBlobByte(image);
        if (EOFBlob(image) != MagickFalse)
          ThrowPCXException(CorruptImageError,"UnexpectedEndOfFile");
        *p++=packet;
        pcx_packets--;
      }
    else
      while (pcx_packets != 0)
      {
        packet=(unsigned char) ReadBlobByte(image);
        if (EOFBlob(image) != MagickFalse)
          ThrowPCXException(CorruptImageError,"UnexpectedEndOfFile");
        if ((packet & 0xc0) != 0xc0)
          {
            *p++=packet;
            pcx_packets--;
            continue;
          }
        count=(ssize_t) (packet & 0x3f);
        packet=(unsigned char) ReadBlobByte(image);
        if (EOFBlob(image) != MagickFalse)
          ThrowPCXException(CorruptImageError,"UnexpectedEndOfFile");
        for ( ; count != 0; count--)
        {
          *p++=packet;
          pcx_packets--;
          if (pcx_packets == 0)
            break;
        }
      }
    if (image->storage_class == DirectClass)
      image->alpha_trait=pcx_info.planes > 3 ? BlendPixelTrait :
        UndefinedPixelTrait;
    else
      if ((pcx_info.version == 5) ||
          ((pcx_info.bits_per_pixel*pcx_info.planes) == 1))
        {
          /*
            Initialize image colormap.
          */
          if (image->colors > 256)
            ThrowPCXException(CorruptImageError,"ColormapExceeds256Colors");
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
                offset=SeekBlob(image,(MagickOffsetType) GetBlobSize(image)-3*
                  image->colors-1,SEEK_SET);
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
        }
    /*
      Convert PCX raster image to pixel packets.
    */
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=pixels+(y*pcx_info.bytes_per_line*pcx_info.planes);
      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
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
            r+=(ptrdiff_t) pcx_info.planes;
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
              ssize_t
                bit;

              for (x=0; x < ((ssize_t) image->columns-7); x+=8)
              {
                for (bit=7; bit >= 0; bit--)
                  *r++=(unsigned char) ((*p) & (0x01 << bit) ? 0x00 : 0x01);
                p++;
              }
              if ((image->columns % 8) != 0)
                {
                  for (bit=7; bit >= (ssize_t) (8-(image->columns % 8)); bit--)
                    *r++=(unsigned char) ((*p) & (0x01 << bit) ? 0x00 : 0x01);
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
              (void) memcpy(r,p,image->columns);
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
          SetPixelIndex(image,*r++,q);
        else
          {
            SetPixelRed(image,ScaleCharToQuantum(*r++),q);
            SetPixelGreen(image,ScaleCharToQuantum(*r++),q);
            SetPixelBlue(image,ScaleCharToQuantum(*r++),q);
            if (image->alpha_trait != UndefinedPixelTrait)
              SetPixelAlpha(image,ScaleCharToQuantum(*r++),q);
          }
        q+=(ptrdiff_t) GetPixelChannels(image);
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
      (void) SyncImage(image,exception);
    scanline=(unsigned char *) RelinquishMagickMemory(scanline);
    pixel_info=RelinquishVirtualMemory(pixel_info);
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
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    count=ReadBlob(image,1,&pcx_info.identifier);
    if ((count != 0) && (pcx_info.identifier == 0x0a))
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            status=MagickFalse;
            break;
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
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  if (status == MagickFalse)
    return(DestroyImageList(image));
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

  entry=AcquireMagickInfo("PCX","DCX","ZSoft IBM PC multi-page Paintbrush");
  entry->decoder=(DecodeImageHandler *) ReadPCXImage;
  entry->encoder=(EncodeImageHandler *) WritePCXImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->magick=(IsImageFormatHandler *) IsDCX;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PCX","PCX","ZSoft IBM PC Paintbrush");
  entry->decoder=(DecodeImageHandler *) ReadPCXImage;
  entry->encoder=(EncodeImageHandler *) WritePCXImage;
  entry->magick=(IsImageFormatHandler *) IsPCX;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
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
%      MagickBooleanType WritePCXImage(const ImageInfo *image_info,
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

static MagickBooleanType PCXWritePixels(PCXInfo *pcx_info,
  const unsigned char *pixels,Image *image)
{
  const unsigned char
    *q;

  ssize_t
    count,
    i,
    x;

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

static MagickBooleanType WritePCXImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  const Quantum
    *p;

  MagickBooleanType
    status;

  MagickOffsetType
    offset,
    *page_table,
    scene;

  MemoryInfo
    *pixel_info;

  PCXInfo
    pcx_info;

  size_t
    number_scenes,
    length;

  ssize_t
    i,
    x,
    y;

  unsigned char
    *pcx_colormap,
    *pixels,
    *q;

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
  number_scenes=GetImageListLength(image);
  if (number_scenes > MaxNumberScenes)
    ThrowWriterException(ResourceLimitError,"ListLengthExceedsLimit");
  page_table=(MagickOffsetType *) NULL;
  if ((LocaleCompare(image_info->magick,"DCX") == 0) ||
      ((GetNextImageInList(image) != (Image *) NULL) &&
       (image_info->adjoin != MagickFalse)))
    {
      /*
        Write the DCX page table.
      */
      (void) WriteBlobLSBLong(image,0x3ADE68B1L);
      page_table=(MagickOffsetType *) AcquireQuantumMemory(MaxNumberScenes+1,
        sizeof(*page_table));
      if (page_table == (MagickOffsetType *) NULL)
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      for (scene=0; scene < MaxNumberScenes; scene++)
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
        (SetImageMonochrome(image,exception) != MagickFalse))
      pcx_info.bits_per_pixel=1;
    else
      if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
        (void) TransformImageColorspace(image,sRGBColorspace,exception);
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
        pcx_info.horizontal_resolution=CastDoubleToUShort(image->resolution.x);
        pcx_info.vertical_resolution=CastDoubleToUShort(image->resolution.y);
        break;
      }
      case PixelsPerCentimeterResolution:
      {
        pcx_info.horizontal_resolution=CastDoubleToUShort(2.54*image->resolution.x+0.5);
        pcx_info.vertical_resolution=CastDoubleToUShort(2.54*image->resolution.y+0.5);
        break;
      }
    }
    pcx_info.reserved=0;
    pcx_info.planes=1;
    if ((image->storage_class == DirectClass) || (image->colors > 256))
      {
        pcx_info.planes=3;
        if (image->alpha_trait != UndefinedPixelTrait)
          pcx_info.planes++;
      }
    length=(((size_t) image->columns*pcx_info.bits_per_pixel+7)/8);
    if ((image->columns > 65535UL) || (image->rows > 65535UL) ||
        (length > 65535UL))
      {
        if (page_table != (MagickOffsetType *) NULL)
          page_table=(MagickOffsetType *) RelinquishMagickMemory(page_table);
        ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
      }
    pcx_info.bytes_per_line=(unsigned short) length;
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
      {
        if (page_table != (MagickOffsetType *) NULL)
          page_table=(MagickOffsetType *) RelinquishMagickMemory(page_table);
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      }
    (void) memset(pcx_colormap,0,3*256*sizeof(*pcx_colormap));
    q=pcx_colormap;
    if ((image->storage_class == PseudoClass) && (image->colors <= 256))
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        *q++=ScaleQuantumToChar((Quantum) image->colormap[i].red);
        *q++=ScaleQuantumToChar((Quantum) image->colormap[i].green);
        *q++=ScaleQuantumToChar((Quantum) image->colormap[i].blue);
      }
    (void) WriteBlob(image,3*16,(const unsigned char *) pcx_colormap);
    (void) WriteBlobByte(image,pcx_info.reserved);
    (void) WriteBlobByte(image,pcx_info.planes);
    (void) WriteBlobLSBShort(image,pcx_info.bytes_per_line);
    (void) WriteBlobLSBShort(image,pcx_info.palette_info);
    for (i=0; i < 58; i++)
      (void) WriteBlobByte(image,'\0');
    length=(size_t) pcx_info.bytes_per_line;
    pixel_info=AcquireVirtualMemory(length,pcx_info.planes*sizeof(*pixels));
    if (pixel_info == (MemoryInfo *) NULL)
      {
        pcx_colormap=(unsigned char *) RelinquishMagickMemory(pcx_colormap);
        if (page_table != (MagickOffsetType *) NULL)
          page_table=(MagickOffsetType *) RelinquishMagickMemory(page_table);
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      }
    pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
    q=pixels;
    if ((image->storage_class == DirectClass) || (image->colors > 256))
      {
        /*
          Convert DirectClass image to PCX raster pixels.
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          q=pixels;
          for (i=0; i < pcx_info.planes; i++)
          {
            p=GetVirtualPixels(image,0,y,image->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            switch ((int) i)
            {
              case 0:
              {
                for (x=0; x < (ssize_t) pcx_info.bytes_per_line; x++)
                {
                  *q++=ScaleQuantumToChar(GetPixelRed(image,p));
                  p+=(ptrdiff_t) GetPixelChannels(image);
                }
                break;
              }
              case 1:
              {
                for (x=0; x < (ssize_t) pcx_info.bytes_per_line; x++)
                {
                  *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
                  p+=(ptrdiff_t) GetPixelChannels(image);
                }
                break;
              }
              case 2:
              {
                for (x=0; x < (ssize_t) pcx_info.bytes_per_line; x++)
                {
                  *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
                  p+=(ptrdiff_t) GetPixelChannels(image);
                }
                break;
              }
              case 3:
              default:
              {
                for (x=(ssize_t) pcx_info.bytes_per_line; x != 0; x--)
                {
                  *q++=ScaleQuantumToChar((Quantum) (GetPixelAlpha(image,p)));
                  p+=(ptrdiff_t) GetPixelChannels(image);
                }
                break;
              }
            }
          }
          if (PCXWritePixels(&pcx_info,pixels,image) == MagickFalse)
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
            p=GetVirtualPixels(image,0,y,image->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            q=pixels;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              *q++=(unsigned char) ((ssize_t) GetPixelIndex(image,p));
              p+=(ptrdiff_t) GetPixelChannels(image);
            }
            if (PCXWritePixels(&pcx_info,pixels,image) == MagickFalse)
              break;
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                  y,image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
        else
          {
            unsigned char
              bit,
              byte;

            /*
              Convert PseudoClass image to a PCX monochrome image.
            */
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              bit=0;
              byte=0;
              q=pixels;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                byte<<=1;
                if (GetPixelLuma(image,p) < ((double) QuantumRange/2.0))
                  byte|=0x01;
                bit++;
                if (bit == 8)
                  {
                    *q++=byte;
                    bit=0;
                    byte=0;
                  }
                p+=(ptrdiff_t) GetPixelChannels(image);
              }
              if (bit != 0)
                *q++=byte << (8-bit);
              if (PCXWritePixels(&pcx_info,pixels,image) == MagickFalse)
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
    pixel_info=RelinquishVirtualMemory(pixel_info);
    pcx_colormap=(unsigned char *) RelinquishMagickMemory(pcx_colormap);
    if (page_table == (MagickOffsetType *) NULL)
      break;
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,number_scenes);
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
        {
          page_table=(MagickOffsetType *) RelinquishMagickMemory(page_table);
          ThrowWriterException(CorruptImageError,"ImproperImageHeader");
        }
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
      (void) ThrowMagickException(exception,GetMagickModule(),FileOpenError,
        "UnableToWriteFile","`%s': %s",image->filename,message);
      message=DestroyString(message);
    }
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

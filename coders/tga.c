/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            TTTTT   GGGG   AAA                               %
%                              T    G      A   A                              %
%                              T    G  GG  AAAAA                              %
%                              T    G   G  A   A                              %
%                              T     GGG   A   A                              %
%                                                                             %
%                                                                             %
%                    Read/Write Truevision Targa Image Format                 %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colormap-private.h"
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
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteTGAImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d T G A I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadTGAImage() reads a Truevision TGA image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns
%  a pointer to the new image.
%
%  The format of the ReadTGAImage method is:
%
%      Image *ReadTGAImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadTGAImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define TGAColormap 1
#define TGARGB 2
#define TGAMonochrome 3
#define TGARLEColormap  9
#define TGARLERGB  10
#define TGARLEMonochrome  11

  typedef struct _TGAInfo
  {
    unsigned char
      id_length,
      colormap_type,
      image_type;

    unsigned short
      colormap_index,
      colormap_length;

    unsigned char
      colormap_size;

    unsigned short
      x_origin,
      y_origin,
      width,
      height;

    unsigned char
      bits_per_pixel,
      attributes;
  } TGAInfo;

  Image
    *image;

  IndexPacket
    index;

  MagickBooleanType
    status;

  PixelPacket
    pixel;

  register IndexPacket
    *indexes;

  register PixelPacket
    *q;

  register ssize_t
    i,
    x;

  size_t
    base,
    flag,
    offset,
    real,
    skip;

  ssize_t
    count,
    y;

  TGAInfo
    tga_info;

  unsigned char
    j,
    k,
    pixels[4],
    runlength;

  unsigned int
    alpha_bits;

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
    Read TGA header information.
  */
  count=ReadBlob(image,1,&tga_info.id_length);
  tga_info.colormap_type=(unsigned char) ReadBlobByte(image);
  tga_info.image_type=(unsigned char) ReadBlobByte(image);
  if ((count != 1) ||
      ((tga_info.image_type != TGAColormap) &&
       (tga_info.image_type != TGARGB) &&
       (tga_info.image_type != TGAMonochrome) &&
       (tga_info.image_type != TGARLEColormap) &&
       (tga_info.image_type != TGARLERGB) &&
       (tga_info.image_type != TGARLEMonochrome)) ||
      (((tga_info.image_type == TGAColormap) ||
       (tga_info.image_type == TGARLEColormap)) &&
       (tga_info.colormap_type == 0)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  tga_info.colormap_index=ReadBlobLSBShort(image);
  tga_info.colormap_length=ReadBlobLSBShort(image);
  tga_info.colormap_size=(unsigned char) ReadBlobByte(image);
  tga_info.x_origin=ReadBlobLSBShort(image);
  tga_info.y_origin=ReadBlobLSBShort(image);
  tga_info.width=(unsigned short) ReadBlobLSBShort(image);
  tga_info.height=(unsigned short) ReadBlobLSBShort(image);
  tga_info.bits_per_pixel=(unsigned char) ReadBlobByte(image);
  tga_info.attributes=(unsigned char) ReadBlobByte(image);
  if (EOFBlob(image) != MagickFalse)
    ThrowReaderException(CorruptImageError,"UnableToReadImageData");
  if ((((tga_info.bits_per_pixel <= 1) || (tga_info.bits_per_pixel >= 17)) &&
       (tga_info.bits_per_pixel != 24) && (tga_info.bits_per_pixel != 32)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
    Initialize image structure.
  */
  image->columns=tga_info.width;
  image->rows=tga_info.height;
  alpha_bits=(tga_info.attributes & 0x0FU);
  image->matte=(alpha_bits > 0) || (tga_info.bits_per_pixel == 32) ||
    (tga_info.colormap_size == 32) ?  MagickTrue : MagickFalse;
  if ((tga_info.image_type != TGAColormap) &&
      (tga_info.image_type != TGARLEColormap))
    image->depth=(size_t) ((tga_info.bits_per_pixel <= 8) ? 8 :
      (tga_info.bits_per_pixel <= 16) ? 5 :
      (tga_info.bits_per_pixel == 24) ? 8 :
      (tga_info.bits_per_pixel == 32) ? 8 : 8);
  else
    image->depth=(size_t) ((tga_info.colormap_size <= 8) ? 8 :
      (tga_info.colormap_size <= 16) ? 5 :
      (tga_info.colormap_size == 24) ? 8 :
      (tga_info.colormap_size == 32) ? 8 : 8);
  if ((tga_info.image_type == TGAColormap) ||
      (tga_info.image_type == TGAMonochrome) ||
      (tga_info.image_type == TGARLEColormap) ||
      (tga_info.image_type == TGARLEMonochrome))
    image->storage_class=PseudoClass;
  image->compression=NoCompression;
  if ((tga_info.image_type == TGARLEColormap) ||
      (tga_info.image_type == TGARLEMonochrome))
    image->compression=RLECompression;
  if (image->storage_class == PseudoClass)
    {
      if (tga_info.colormap_type != 0)
        image->colors=tga_info.colormap_length;
      else
        {
          size_t
            one;

          one=1;
          image->colors=one << tga_info.bits_per_pixel;
          if (AcquireImageColormap(image,image->colors) == MagickFalse)
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
    }
  if (tga_info.id_length != 0)
    {
      char
        *comment;

      size_t
        length;

      /*
        TGA image comment.
      */
      length=(size_t) tga_info.id_length;
      comment=(char *) NULL;
      if (~length >= (MaxTextExtent-1))
        comment=(char *) AcquireQuantumMemory(length+MaxTextExtent,
          sizeof(*comment));
      if (comment == (char *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      count=ReadBlob(image,tga_info.id_length,(unsigned char *) comment);
      comment[tga_info.id_length]='\0';
      (void) SetImageProperty(image,"comment",comment);
      comment=DestroyString(comment);
    }
  (void) ResetMagickMemory(&pixel,0,sizeof(pixel));
  pixel.opacity=(Quantum) OpaqueOpacity;
  if (tga_info.colormap_type != 0)
    {
      /*
        Read TGA raster colormap.
      */
      if (AcquireImageColormap(image,image->colors) == MagickFalse)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        switch (tga_info.colormap_size)
        {
          case 8:
          default:
          {
            /*
              Gray scale.
            */
            pixel.red=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
            pixel.green=pixel.red;
            pixel.blue=pixel.red;
            break;
          }
          case 15:
          case 16:
          {
            QuantumAny
              range;

            /*
              5 bits each of red green and blue.
            */
            j=(unsigned char) ReadBlobByte(image);
            k=(unsigned char) ReadBlobByte(image);
            range=GetQuantumRange(5UL);
            pixel.red=ScaleAnyToQuantum(1UL*(k & 0x7c) >> 2,range);
            pixel.green=ScaleAnyToQuantum((1UL*(k & 0x03) << 3)+
              (1UL*(j & 0xe0) >> 5),range);
            pixel.blue=ScaleAnyToQuantum(1UL*(j & 0x1f),range);
            break;
          }
          case 24:
          {
            /*
              8 bits each of blue, green and red.
            */
            pixel.blue=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
            pixel.green=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
            pixel.red=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
            break;
          }
          case 32:
          {
            /*
              8 bits each of blue, green, red, and alpha.
            */
            pixel.blue=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
            pixel.green=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
            pixel.red=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
            pixel.opacity=(Quantum) (QuantumRange-ScaleCharToQuantum(
              (unsigned char) ReadBlobByte(image)));
            break;
          }
        }
        image->colormap[i]=pixel;
      }
    }
  /*
    Convert TGA pixels to pixel packets.
  */
  base=0;
  flag=0;
  skip=MagickFalse;
  real=0;
  index=(IndexPacket) 0;
  runlength=0;
  offset=0;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    real=offset;
    if (((unsigned char) (tga_info.attributes & 0x20) >> 5) == 0)
      real=image->rows-real-1;
    q=QueueAuthenticPixels(image,0,(ssize_t) real,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((tga_info.image_type == TGARLEColormap) ||
          (tga_info.image_type == TGARLERGB) ||
          (tga_info.image_type == TGARLEMonochrome))
        {
          if (runlength != 0)
            {
              runlength--;
              skip=flag != 0;
            }
          else
            {
              count=ReadBlob(image,1,&runlength);
              if (count == 0)
                ThrowReaderException(CorruptImageError,"UnableToReadImageData");
              flag=runlength & 0x80;
              if (flag != 0)
                runlength-=128;
              skip=MagickFalse;
            }
        }
      if (skip == MagickFalse)
        switch (tga_info.bits_per_pixel)
        {
          case 8:
          default:
          {
            /*
              Gray scale.
            */
            index=(IndexPacket) ReadBlobByte(image);
            if (tga_info.colormap_type != 0)
              pixel=image->colormap[(ssize_t) ConstrainColormapIndex(image,
                1UL*index)];
            else
              {
                pixel.red=ScaleCharToQuantum((unsigned char) index);
                pixel.green=ScaleCharToQuantum((unsigned char) index);
                pixel.blue=ScaleCharToQuantum((unsigned char) index);
              }
            break;
          }
          case 15:
          case 16:
          {
            QuantumAny
              range;

            /*
              5 bits each of RGB;
            */
            if (ReadBlob(image,2,pixels) != 2)
              ThrowReaderException(CorruptImageError,"UnableToReadImageData");
            j=pixels[0];
            k=pixels[1];
            range=GetQuantumRange(5UL);
            pixel.red=ScaleAnyToQuantum(1UL*(k & 0x7c) >> 2,range);
            pixel.green=ScaleAnyToQuantum((1UL*(k & 0x03) << 3)+
              (1UL*(j & 0xe0) >> 5),range);
            pixel.blue=ScaleAnyToQuantum(1UL*(j & 0x1f),range);
            if (image->matte != MagickFalse)
              pixel.opacity=(k & 0x80) == 0 ? (Quantum) OpaqueOpacity :
                (Quantum) TransparentOpacity;
            if (image->storage_class == PseudoClass)
              index=ConstrainColormapIndex(image,((size_t) k << 8)+j);
            break;
          }
          case 24:
          {
            /*
              BGR pixels.
            */
            if (ReadBlob(image,3,pixels) != 3)
              ThrowReaderException(CorruptImageError,"UnableToReadImageData");
            pixel.blue=ScaleCharToQuantum(pixels[0]);
            pixel.green=ScaleCharToQuantum(pixels[1]);
            pixel.red=ScaleCharToQuantum(pixels[2]);
            break;
          }
          case 32:
          {
            /*
              BGRA pixels.
            */
            if (ReadBlob(image,4,pixels) != 4)
              ThrowReaderException(CorruptImageError,"UnableToReadImageData");
            pixel.blue=ScaleCharToQuantum(pixels[0]);
            pixel.green=ScaleCharToQuantum(pixels[1]);
            pixel.red=ScaleCharToQuantum(pixels[2]);
            pixel.opacity=(Quantum) (QuantumRange-ScaleCharToQuantum(
              pixels[3]));
            break;
          }
        }
      if (status == MagickFalse)
        ThrowReaderException(CorruptImageError,"UnableToReadImageData");
      if (image->storage_class == PseudoClass)
        SetPixelIndex(indexes+x,index);
      SetPixelRed(q,pixel.red);
      SetPixelGreen(q,pixel.green);
      SetPixelBlue(q,pixel.blue);
      if (image->matte != MagickFalse)
        SetPixelOpacity(q,pixel.opacity);
      q++;
    }
    if (((unsigned char) (tga_info.attributes & 0xc0) >> 6) == 4)
      offset+=4;
    else
      if (((unsigned char) (tga_info.attributes & 0xc0) >> 6) == 2)
        offset+=2;
      else
        offset++;
    if (offset >= image->rows)
      {
        base++;
        offset=base;
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
%   R e g i s t e r T G A I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterTGAImage() adds properties for the TGA image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterTGAImage method is:
%
%      size_t RegisterTGAImage(void)
%
*/
ModuleExport size_t RegisterTGAImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("ICB");
  entry->decoder=(DecodeImageHandler *) ReadTGAImage;
  entry->encoder=(EncodeImageHandler *) WriteTGAImage;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("Truevision Targa image");
  entry->module=ConstantString("TGA");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("TGA");
  entry->decoder=(DecodeImageHandler *) ReadTGAImage;
  entry->encoder=(EncodeImageHandler *) WriteTGAImage;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("Truevision Targa image");
  entry->module=ConstantString("TGA");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("VDA");
  entry->decoder=(DecodeImageHandler *) ReadTGAImage;
  entry->encoder=(EncodeImageHandler *) WriteTGAImage;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("Truevision Targa image");
  entry->module=ConstantString("TGA");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("VST");
  entry->decoder=(DecodeImageHandler *) ReadTGAImage;
  entry->encoder=(EncodeImageHandler *) WriteTGAImage;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("Truevision Targa image");
  entry->module=ConstantString("TGA");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r T G A I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterTGAImage() removes format registrations made by the
%  TGA module from the list of supported formats.
%
%  The format of the UnregisterTGAImage method is:
%
%      UnregisterTGAImage(void)
%
*/
ModuleExport void UnregisterTGAImage(void)
{
  (void) UnregisterMagickInfo("ICB");
  (void) UnregisterMagickInfo("TGA");
  (void) UnregisterMagickInfo("VDA");
  (void) UnregisterMagickInfo("VST");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e T G A I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteTGAImage() writes a image in the Truevision Targa rasterfile
%  format.
%
%  The format of the WriteTGAImage method is:
%
%      MagickBooleanType WriteTGAImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static MagickBooleanType WriteTGAImage(const ImageInfo *image_info,Image *image)
{
#define TargaColormap 1
#define TargaRGB 2
#define TargaMonochrome 3
#define TargaRLEColormap  9
#define TargaRLERGB  10
#define TargaRLEMonochrome  11

  typedef struct _TargaInfo
  {
    unsigned char
      id_length,
      colormap_type,
      image_type;

    unsigned short
      colormap_index,
      colormap_length;

    unsigned char
      colormap_size;

    unsigned short
      x_origin,
      y_origin,
      width,
      height;

    unsigned char
      bits_per_pixel,
      attributes;
  } TargaInfo;

  const char
    *value;

  MagickBooleanType
    status;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register ssize_t
    x;

  register ssize_t
    i;

  register unsigned char
    *q;

  ssize_t
    count,
    y;

  TargaInfo
    targa_info;

  unsigned char
    *targa_pixels;

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
  /*
    Initialize TGA raster file header.
  */
  if ((image->columns > 65535L) || (image->rows > 65535L))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace);
  targa_info.id_length=0;
  value=GetImageProperty(image,"comment");
  if (value != (const char *) NULL)
    targa_info.id_length=(unsigned char) MagickMin(strlen(value),255);
  targa_info.colormap_type=0;
  targa_info.colormap_index=0;
  targa_info.colormap_length=0;
  targa_info.colormap_size=0;
  targa_info.x_origin=0;
  targa_info.y_origin=0;
  targa_info.width=(unsigned short) image->columns;
  targa_info.height=(unsigned short) image->rows;
  targa_info.bits_per_pixel=8;
  targa_info.attributes=0;
  if ((image_info->type != TrueColorType) &&
      (image_info->type != TrueColorMatteType) &&
      (image_info->type != PaletteType) &&
      (image->matte == MagickFalse) &&
      (IsGrayImage(image,&image->exception) != MagickFalse))
    targa_info.image_type=TargaMonochrome;
  else
    if ((image->storage_class == DirectClass) || (image->colors > 256))
      {
        /*
          Full color TGA raster.
        */
        targa_info.image_type=TargaRGB;
        targa_info.bits_per_pixel=24;
        if (image->matte != MagickFalse)
          {
            targa_info.bits_per_pixel=32;
            targa_info.attributes=8;  /* # of alpha bits */
          }
      }
    else
      {
        /*
          Colormapped TGA raster.
        */
        targa_info.image_type=TargaColormap;
        targa_info.colormap_type=1;
        targa_info.colormap_length=(unsigned short) image->colors;
        targa_info.colormap_size=24;
      }
  /*
    Write TGA header.
  */
  (void) WriteBlobByte(image,targa_info.id_length);
  (void) WriteBlobByte(image,targa_info.colormap_type);
  (void) WriteBlobByte(image,targa_info.image_type);
  (void) WriteBlobLSBShort(image,targa_info.colormap_index);
  (void) WriteBlobLSBShort(image,targa_info.colormap_length);
  (void) WriteBlobByte(image,targa_info.colormap_size);
  (void) WriteBlobLSBShort(image,targa_info.x_origin);
  (void) WriteBlobLSBShort(image,targa_info.y_origin);
  (void) WriteBlobLSBShort(image,targa_info.width);
  (void) WriteBlobLSBShort(image,targa_info.height);
  (void) WriteBlobByte(image,targa_info.bits_per_pixel);
  (void) WriteBlobByte(image,targa_info.attributes);
  if (targa_info.id_length != 0)
    (void) WriteBlob(image,targa_info.id_length,(unsigned char *)
      value);
  if (targa_info.image_type == TargaColormap)
    {
      unsigned char
        *targa_colormap;

      /*
        Dump colormap to file (blue, green, red byte order).
      */
      targa_colormap=(unsigned char *) AcquireQuantumMemory((size_t)
        targa_info.colormap_length,3UL*sizeof(*targa_colormap));
      if (targa_colormap == (unsigned char *) NULL)
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      q=targa_colormap;
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        *q++=ScaleQuantumToChar(image->colormap[i].blue);
        *q++=ScaleQuantumToChar(image->colormap[i].green);
        *q++=ScaleQuantumToChar(image->colormap[i].red);
      }
      (void) WriteBlob(image,(size_t) (3*targa_info.colormap_length),
        targa_colormap);
      targa_colormap=(unsigned char *) RelinquishMagickMemory(targa_colormap);
    }
  /*
    Convert MIFF to TGA raster pixels.
  */
  count=(ssize_t) (targa_info.bits_per_pixel*targa_info.width)/8;
  targa_pixels=(unsigned char *) AcquireQuantumMemory((size_t) count,
    sizeof(*targa_pixels));
  if (targa_pixels == (unsigned char *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  for (y=(ssize_t) (image->rows-1); y >= 0; y--)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    q=targa_pixels;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (targa_info.image_type == TargaColormap)
        *q++=(unsigned char) GetPixelIndex(indexes+x);
      else
        if (targa_info.image_type == TargaMonochrome)
          *q++=(unsigned char) ScaleQuantumToChar(ClampToQuantum(
            GetPixelLuma(image,p)));
        else
          {
            *q++=ScaleQuantumToChar(GetPixelBlue(p));
            *q++=ScaleQuantumToChar(GetPixelGreen(p));
            *q++=ScaleQuantumToChar(GetPixelRed(p));
            if (image->matte != MagickFalse)
              *q++=(unsigned char) ScaleQuantumToChar(GetPixelAlpha(p));
            if (image->colorspace == CMYKColorspace)
              *q++=ScaleQuantumToChar(GetPixelIndex(indexes+x));
          }
      p++;
    }
    (void) WriteBlob(image,(size_t) (q-targa_pixels),targa_pixels);
    if (image->previous == (Image *) NULL)
      {
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
  }
  targa_pixels=(unsigned char *) RelinquishMagickMemory(targa_pixels);
  (void) CloseBlob(image);
  return(MagickTrue);
}

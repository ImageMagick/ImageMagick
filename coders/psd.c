/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP   SSSSS  DDDD                               %
%                            P   P  SS     D   D                              %
%                            PPPP    SSS   D   D                              %
%                            P         SS  D   D                              %
%                            P      SSSSS  DDDD                               %
%                                                                             %
%                                                                             %
%                   Read/Write Adobe Photoshop Image Format                   %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                              Leonard Rosenthol                              %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/artifact.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/colorspace.h"
#include "magick/constitute.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor-private.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"

/*
  Define declaractions.
*/
#define MaxPSDChannels  56
#define PSDQuantum(x) (((long) (x)+1) & -2)

/*
  Enumerated declaractions.
*/
typedef enum
{
  BitmapMode = 0,
  GrayscaleMode = 1,
  IndexedMode = 2,
  RGBMode = 3,
  CMYKMode = 4,
  MultichannelMode = 7,
  DuotoneMode = 8,
  LabMode = 9
} PSDImageType;

/*
  Typedef declaractions.
*/
typedef struct _ChannelInfo
{
  short int
    type;

  unsigned long
    size;
} ChannelInfo;

typedef struct _LayerInfo
{
  RectangleInfo
    page,
    mask;

  unsigned short
    channels;

  ChannelInfo
    channel_info[MaxPSDChannels];

  char
    blendkey[4];

  Quantum
    opacity;

  unsigned char
    clipping,
    visible,
    flags;

  unsigned long
    offset_x,
    offset_y;

  unsigned char
    name[256];

  Image
    *image;
} LayerInfo;

typedef struct _PSDInfo
{
  char
    signature[4];

  unsigned short
    channels,
    version;

  unsigned char
    reserved[6];

  unsigned long
    rows,
    columns;

  unsigned short
    depth,
    mode;
} PSDInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePSDImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P S D                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPSD()() returns MagickTrue if the image format type, identified by the
%  magick string, is PSD.
%
%  The format of the IsPSD method is:
%
%      MagickBooleanType IsPSD(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsPSD(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick,"8BPS",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P S D I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPSDImage() reads an Adobe Photoshop image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadPSDImage method is:
%
%      Image *ReadPSDImage(image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static const char *CompositeOperatorToPSDBlendMode(CompositeOperator operator)
{
  const char
    *blend_mode;

  switch (operator)
  {
    case OverCompositeOp:    blend_mode = "norm";  break;
    case MultiplyCompositeOp:  blend_mode = "mul ";  break;
    case DissolveCompositeOp:  blend_mode = "diss";  break;
    case DifferenceCompositeOp:  blend_mode = "diff";  break;
    case DarkenCompositeOp:    blend_mode = "dark";  break;
    case LightenCompositeOp:  blend_mode = "lite";  break;
    case HueCompositeOp:    blend_mode = "hue ";  break;
    case SaturateCompositeOp:  blend_mode = "sat ";  break;
    case ColorizeCompositeOp:  blend_mode = "colr";  break;
    case LuminizeCompositeOp:  blend_mode = "lum ";  break;
    case ScreenCompositeOp:    blend_mode = "scrn";  break;
    case OverlayCompositeOp:  blend_mode = "over";  break;
    default:
      blend_mode = "norm";
  }
  return(blend_mode);
}

static ssize_t DecodePSDPixels(const size_t number_compact_pixels,
  const unsigned char *compact_pixels,const long depth,
  const size_t number_pixels,unsigned char *pixels)
{
  int
    pixel;

  register ssize_t
    i,
    j;

  ssize_t
    packets;

  size_t
    length;

  packets=(ssize_t) number_compact_pixels;
  for (i=0; (packets > 1) && (i < (ssize_t) number_pixels); )
  {
    length=(*compact_pixels++);
    packets--;
    if (length == 128)
      continue;
    if (length > 128)
      {
        length=256-length+1;
        pixel=(*compact_pixels++);
        packets--;
        for (j=0; j < (long) length; j++)
        {
          switch (depth)
          {
            case 1:
            {
              *pixels++=(pixel >> 7) & 0x01 ? 0 : 255;
              *pixels++=(pixel >> 6) & 0x01 ? 0 : 255;
              *pixels++=(pixel >> 5) & 0x01 ? 0 : 255;
              *pixels++=(pixel >> 4) & 0x01 ? 0 : 255;
              *pixels++=(pixel >> 3) & 0x01 ? 0 : 255;
              *pixels++=(pixel >> 2) & 0x01 ? 0 : 255;
              *pixels++=(pixel >> 1) & 0x01 ? 0 : 255;
              *pixels++=(pixel >> 0) & 0x01 ? 0 : 255;
              i+=8;
              break;
            }
            case 4:
            {
              *pixels++=(pixel >> 4) & 0xff;
              *pixels++=(pixel & 0x0f) & 0xff;
              i+=2;
              break;
            }
            case 2:
            {
              *pixels++=(pixel >> 6) & 0x03;
              *pixels++=(pixel >> 4) & 0x03;
              *pixels++=(pixel >> 2) & 0x03;
              *pixels++=(pixel & 0x03) & 0x03;
              i+=4;
              break;
            }
            default:
            {
              *pixels++=pixel;
              i++;
              break;
            }
          }
        }
        continue;
      }
    length++;
    for (j=0; j < (long) length; j++)
    {
      switch (depth)
      {
        case 1:
        {
          *pixels++=(*compact_pixels >> 7) & 0x01 ? 0 : 255;
          *pixels++=(*compact_pixels >> 6) & 0x01 ? 0 : 255;
          *pixels++=(*compact_pixels >> 5) & 0x01 ? 0 : 255;
          *pixels++=(*compact_pixels >> 4) & 0x01 ? 0 : 255;
          *pixels++=(*compact_pixels >> 3) & 0x01 ? 0 : 255;
          *pixels++=(*compact_pixels >> 2) & 0x01 ? 0 : 255;
          *pixels++=(*compact_pixels >> 1) & 0x01 ? 0 : 255;
          *pixels++=(*compact_pixels >> 0) & 0x01 ? 0 : 255;
          i+=8;
          break;
        }
        case 4:
        {
          *pixels++=(*compact_pixels >> 4) & 0xff;
          *pixels++=(*compact_pixels & 0x0f) & 0xff;
          i+=2;
          break;
        }
        case 2:
        {
          *pixels++=(*compact_pixels >> 6) & 0x03;
          *pixels++=(*compact_pixels >> 4) & 0x03;
          *pixels++=(*compact_pixels >> 2) & 0x03;
          *pixels++=(*compact_pixels & 0x03) & 0x03;
          i+=4;
          break;
        }
        default:
        {
          *pixels++=(*compact_pixels);
          i++;
          break;
        }
      }
      compact_pixels++;
    }
  }
  return(i);
}

static inline MagickOffsetType GetPSDOffset(PSDInfo *psd_info,Image *image)
{
  if (psd_info->version == 1)
    return((MagickOffsetType) ReadBlobMSBShort(image));
  return((MagickOffsetType) ReadBlobMSBLong(image));
}

static inline MagickSizeType GetPSDSize(PSDInfo *psd_info,Image *image)
{
  if (psd_info->version == 1)
    return((MagickSizeType) ReadBlobMSBLong(image));
  return((MagickSizeType) ReadBlobMSBLongLong(image));
}

static inline long MagickAbsoluteValue(const long x)
{
  if (x < 0)
    return(-x);
  return(x);
}

static const char *ModeToString(PSDImageType type)
{
  switch (type)
  {
    case BitmapMode: return "Bitmap";
    case GrayscaleMode: return "Grayscale";
    case IndexedMode: return "Indexed";
    case RGBMode: return "RGB";
    case CMYKMode:  return "CMYK";
    case MultichannelMode: return "Multichannel";
    case DuotoneMode: return "Duotone";
    case LabMode: return "L*A*B";
    default: return "unknown";
  }
}

static MagickBooleanType ParseImageResourceBlocks(Image *image,
  const unsigned char *blocks,size_t length)
{
  const unsigned char
    *p;

  StringInfo
    *profile;

  unsigned long
    count,
    long_sans;

  unsigned short
    id,
    short_sans;

  if (length < 16)
    return(MagickFalse);
  profile=AcquireStringInfo(length);
  SetStringInfoDatum(profile,blocks);
  (void) SetImageProfile(image,"8bim",profile);
  profile=DestroyStringInfo(profile);
  for (p=blocks; (p >= blocks) && (p < (blocks+length-16)); )
  {
    if (LocaleNCompare((const char *) p,"8BIM",4) != 0)
      break;
    p=PushLongPixel(LSBEndian,p,&long_sans);
    p=PushShortPixel(LSBEndian,p,&id);
    p=PushShortPixel(LSBEndian,p,&short_sans);
    p=PushLongPixel(LSBEndian,p,&count);
    switch (id)
    {
      case 0x03ed:
      {
        unsigned short
          resolution;

        /*
          Resolution info.
        */
        p=PushShortPixel(LSBEndian,p,&resolution);
        image->x_resolution=(double) resolution;
        p=PushShortPixel(LSBEndian,p,&short_sans);
        p=PushShortPixel(LSBEndian,p,&short_sans);
        p=PushShortPixel(LSBEndian,p,&short_sans);
        p=PushShortPixel(LSBEndian,p,&resolution);
        image->y_resolution=(double) resolution;
        p=PushShortPixel(LSBEndian,p,&short_sans);
        p=PushShortPixel(LSBEndian,p,&short_sans);
        p=PushShortPixel(LSBEndian,p,&short_sans);
        break;
      }
      default:
      {
        p+=count;
        break;
      }
    }
    if ((count & 0x01) != 0)
      p++;
  }
  return(MagickTrue);
}

static CompositeOperator PSDBlendModeToCompositeOperator(const char *mode)
{
  if (mode == (const char *) NULL)
    return(OverCompositeOp);
  if (LocaleNCompare(mode,"norm",4) == 0)
    return(OverCompositeOp);
  if (LocaleNCompare(mode,"mul ",4) == 0)
    return(MultiplyCompositeOp);
  if (LocaleNCompare(mode,"diss",4) == 0)
    return(DissolveCompositeOp);
  if (LocaleNCompare(mode,"diff",4) == 0)
    return(DifferenceCompositeOp);
  if (LocaleNCompare(mode,"dark",4) == 0)
    return(DarkenCompositeOp);
  if (LocaleNCompare(mode,"lite",4) == 0)
    return(LightenCompositeOp);
  if (LocaleNCompare(mode,"hue ",4) == 0)
    return(HueCompositeOp);
  if (LocaleNCompare(mode,"sat ",4) == 0)
    return(SaturateCompositeOp);
  if (LocaleNCompare(mode,"colr",4) == 0)
    return(ColorizeCompositeOp);
  if (LocaleNCompare(mode,"lum ",4) == 0)
    return(LuminizeCompositeOp);
  if (LocaleNCompare(mode,"scrn",4) == 0)
    return(ScreenCompositeOp);
  if (LocaleNCompare(mode,"over",4) == 0)
    return(OverlayCompositeOp);
  if (LocaleNCompare(mode,"hLit",4) == 0)
    return(OverCompositeOp);
  if (LocaleNCompare(mode,"sLit",4) == 0)
    return(OverCompositeOp);
  if (LocaleNCompare(mode,"smud",4) == 0)
    return(OverCompositeOp);
  if (LocaleNCompare(mode,"div ",4) == 0)
    return(OverCompositeOp);
  if (LocaleNCompare(mode,"idiv",4) == 0)
    return(OverCompositeOp);
  return(OverCompositeOp);
}

static MagickBooleanType ReadPSDLayer(Image *image,
  const unsigned long channels,const unsigned long type,
  const MagickOffsetType *offsets,ExceptionInfo *exception)
{
  long
    y;

  Quantum
    pixel;

  register IndexPacket
    *indexes;

  register long
    x;

  register PixelPacket
    *q;

  register const unsigned char
    *p;

  size_t
    packet_size;

  ssize_t
    count;

  unsigned char
    *compact_pixels,
    *pixels;

  unsigned short
    nibble;

  packet_size=1;
  if (image->storage_class == PseudoClass)
    {
      if (image->colors > 256)
        packet_size++;
      else
        if (image->depth > 8)
          packet_size++;
    }
  else
    if (image->depth > 8)
      packet_size++;
  pixels=(unsigned char *) AcquireQuantumMemory(image->columns+256,packet_size*
    sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  (void) ResetMagickMemory(pixels,0,image->columns*packet_size*sizeof(*pixels));
  compact_pixels=(unsigned char *) NULL;
  if (image->compression == RLECompression)
    {
      size_t
        length;

      length=0;
      for (y=0; y < (long) image->rows; y++)
        if ((MagickOffsetType) length < offsets[y])
          length=offsets[y];
      compact_pixels=(unsigned char *) AcquireQuantumMemory(length,
        sizeof(*pixels));
      if (compact_pixels == (unsigned char *) NULL)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
      (void) ResetMagickMemory(compact_pixels,0,length*sizeof(*compact_pixels));
    }
  for (y=0; y < (long) image->rows; y++)
  {
    if (image->compression != RLECompression)
      count=ReadBlob(image,packet_size*image->columns,pixels);
    else
      {
        count=ReadBlob(image,(size_t) offsets[y],compact_pixels);
        if (count != (ssize_t) offsets[y])
          break;
        count=DecodePSDPixels((size_t) offsets[y],compact_pixels,
          image->depth,packet_size*image->columns,pixels);
      }
    if (count < (ssize_t) (packet_size*image->columns))
      break;
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    p=pixels;
    for (x=0; x < (long) image->columns; x++)
    {
      if (packet_size == 1)
        pixel=ScaleCharToQuantum(*p++);
      else
        {
          p=PushShortPixel(MSBEndian,p,&nibble);
          pixel=ScaleShortToQuantum(nibble);
        }
      switch (type)
      {
        case -1:
        {
          q->opacity=(Quantum) (QuantumRange-pixel);
          break;
        }
        case 0:
        {
          q->red=pixel;
          if (channels == 1)
            {
              q->green=q->red;
              q->blue=q->red;
            }
          if (image->storage_class == PseudoClass)
            {
              if (packet_size == 1)
                indexes[x]=(IndexPacket) ScaleQuantumToChar(pixel);
              else
                indexes[x]=(IndexPacket) ScaleQuantumToShort(pixel);
              *q=image->colormap[(long) indexes[x]];
              q->red=image->colormap[(long) indexes[x]].red;
              q->green=image->colormap[(long) indexes[x]].green;
              q->blue=image->colormap[(long) indexes[x]].blue;
            }
          break;
        }
        case 1:
        {
          if (image->storage_class == PseudoClass)
            q->opacity=(Quantum) (QuantumRange-pixel);
          else
            q->green=pixel;
          break;
        }
        case 2:
        {
          q->blue=pixel;
          break;
        }
        case 3:
        {
          if (image->colorspace == CMYKColorspace)
            indexes[x]=(IndexPacket) pixel;
          else
            q->opacity=(Quantum) (QuantumRange-pixel);
          break;
        }
        case 4:
        {
          q->opacity=(Quantum) (QuantumRange-pixel);
          break;
        }
        default:
          break;
      }
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  if (image->compression == RLECompression)
    compact_pixels=(unsigned char *) RelinquishMagickMemory(compact_pixels);
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  return(MagickTrue);
}

static Image *ReadPSDImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    message[MaxTextExtent],
    type[4];

  Image
    *image;

  IndexPacket
    *indexes;

  LayerInfo
    *layer_info;

  long
    j,
    number_layers,
    y;

  MagickBooleanType
    status;

  MagickOffsetType
    offset,
    *offsets;

  MagickSizeType
    combinedlength,
    length,
    size;

  PSDInfo
    psd_info;

  register long
    i,
    x;

  register PixelPacket
    *q;

  ssize_t
    count;

  unsigned char
    *data;

  unsigned short
    compression;

  unsigned long
    mask_size,
    skip_first_alpha = 0;

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
    Read image header.
  */
  count=ReadBlob(image,4,(unsigned char *) psd_info.signature);
  psd_info.version=ReadBlobMSBShort(image);
  if ((count == 0) || (LocaleNCompare(psd_info.signature,"8BPS",4) != 0) ||
      ((psd_info.version != 1) && (psd_info.version != 2)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  count=ReadBlob(image,6,psd_info.reserved);
  psd_info.channels=ReadBlobMSBShort(image);
  if (psd_info.channels > MaxPSDChannels)
    ThrowReaderException(CorruptImageError,"MaximumChannelsExceeded");
  psd_info.rows=ReadBlobMSBLong(image);
  psd_info.columns=ReadBlobMSBLong(image);
  if ((psd_info.version == 1) && ((psd_info.rows > 30000) ||
      (psd_info.columns > 30000)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  psd_info.depth=ReadBlobMSBShort(image);
  if ((psd_info.depth != 1) && (psd_info.depth != 8) && (psd_info.depth != 16))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  psd_info.mode=ReadBlobMSBShort(image);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Image is %ld x %ld with channels=%d, depth=%d, mode=%s",
      psd_info.columns,psd_info.rows,psd_info.channels,psd_info.depth,
      ModeToString((PSDImageType) psd_info.mode));
  /*
    Initialize image.
  */
  image->depth=psd_info.depth;
  image->columns=psd_info.columns;
  image->rows=psd_info.rows;
  if (SetImageBackgroundColor(image) == MagickFalse)
    {
      InheritException(exception,&image->exception);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  image->matte=psd_info.channels >= 4 ? MagickTrue : MagickFalse;
  if (psd_info.mode == LabMode)
    image->colorspace=LabColorspace;
  if (psd_info.mode == CMYKMode)
    {
      image->colorspace=CMYKColorspace;
      image->matte=psd_info.channels >= 5 ? MagickTrue : MagickFalse;
    }
  if ((psd_info.mode == BitmapMode) || (psd_info.mode == GrayscaleMode) ||
      (psd_info.mode == DuotoneMode))
    {
      if (AcquireImageColormap(image,256) == MagickFalse)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      image->matte=psd_info.channels >= 2 ? MagickTrue : MagickFalse;
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  ImageColorMap allocated");
      image->colorspace=GRAYColorspace;
    }
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      image->matte ? "  image has matte" : "  image has no matte");
  /*
    Read PSD raster colormap only present for indexed and duotone images.
  */
  length=ReadBlobMSBLong(image);
  if (length != 0)
    {
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  reading colormap");
      if (psd_info.mode == DuotoneMode)
        {
          /*
            Duotone image data;  the format of this data is undocumented.
          */
          data=(unsigned char *) AcquireQuantumMemory((size_t) length,
            sizeof(*data));
          if (data == (unsigned char *) NULL)
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          count=ReadBlob(image,(size_t) length,data);
          data=(unsigned char *) RelinquishMagickMemory(data);
        }
      else
        {
          /*
            Read PSD raster colormap.
          */
          if (AcquireImageColormap(image,(unsigned long) (length/3)) == MagickFalse)
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          for (i=0; i < (long) image->colors; i++)
            image->colormap[i].red=ScaleCharToQuantum((unsigned char)
              ReadBlobByte(image));
          for (i=0; i < (long) image->colors; i++)
            image->colormap[i].green=ScaleCharToQuantum((unsigned char)
              ReadBlobByte(image));
          for (i=0; i < (long) image->colors; i++)
            image->colormap[i].blue=ScaleCharToQuantum((unsigned char)
              ReadBlobByte(image));
          image->matte=psd_info.channels >= 2 ? MagickTrue : MagickFalse;
        }
    }
  length=ReadBlobMSBLong(image);
  if (length != 0)
    {
      unsigned char
        *blocks;

      /*
        Image resources block.
      */
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  reading image resource blocks - %ld bytes",(long) length);
      blocks=(unsigned char *) AcquireQuantumMemory((size_t) length,
        sizeof(*blocks));
      if (blocks == (unsigned char *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      count=ReadBlob(image,(size_t) length,blocks);
      if ((count != (ssize_t) length) ||
          (LocaleNCompare((char *) blocks,"8BIM",4) != 0))
        {
          blocks=(unsigned char *) RelinquishMagickMemory(blocks);
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        }
      (void) ParseImageResourceBlocks(image,blocks,(size_t) length);
      blocks=(unsigned char *) RelinquishMagickMemory(blocks);
    }
  /*
    If we are only "pinging" the image, then we're done - so return.
  */
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  /*
    Layer and mask block.
  */
  layer_info=(LayerInfo *) NULL;
  number_layers=1;
  length=GetPSDSize(&psd_info,image);
  if (length == 8)
    {
      length=ReadBlobMSBLong(image);
      length=ReadBlobMSBLong(image);
    }
  if ((image_info->number_scenes == 1) && (image_info->scene == 0))
    for ( ; length != 0; length--)
      if (ReadBlobByte(image) == EOF)
        {
          ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
            image->filename);
          break;
        }
  if (length == 0)
    {
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  image has no layers");
    }
  else
    {
      offset=TellBlob(image);
      size=GetPSDSize(&psd_info,image);
      if (size == 0)
        {
          unsigned long
            quantum;

          /*
            Skip layers & masks.
          */
          quantum=psd_info.version == 1 ? 4UL : 8UL;
          for (j=0; j < (long) (length-quantum); j++)
            (void) ReadBlobByte(image);
        }
      else
        {
          MagickOffsetType
            layer_offset;

          layer_offset=offset+length;
          number_layers=(short) ReadBlobMSBShort(image);
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  image contains %ld layers", number_layers);
          if (number_layers < 0)
            {
              /*
                Weird hack in PSD format to ignore first alpha channel.
              */
              skip_first_alpha=1;
              number_layers=MagickAbsoluteValue(number_layers);
              if (image->debug != MagickFalse)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  negative layer count corrected for");
            }
          layer_info=(LayerInfo *) AcquireQuantumMemory((size_t) number_layers,
            sizeof(*layer_info));
          if (layer_info == (LayerInfo *) NULL)
            {
              if (image->debug != MagickFalse)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  allocation of LayerInfo failed");
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            }
          (void) ResetMagickMemory(layer_info,0,(size_t) number_layers*
            sizeof(*layer_info));
          for (i=0; i < number_layers; i++)
          {
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  reading layer #%ld",i+1);
            layer_info[i].page.y=(long) ReadBlobMSBLong(image);
            layer_info[i].page.x=(long) ReadBlobMSBLong(image);
            layer_info[i].page.height=(unsigned long)
              ReadBlobMSBLong(image)-layer_info[i].page.y;
            layer_info[i].page.width=(unsigned long)
              ReadBlobMSBLong(image)-layer_info[i].page.x;
            layer_info[i].channels=ReadBlobMSBShort(image);
            if (layer_info[i].channels > MaxPSDChannels)
              ThrowReaderException(CorruptImageError,"MaximumChannelsExceeded");
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    offset(%ld,%ld), size(%ld,%ld), channels=%d",
                layer_info[i].page.x, layer_info[i].page.y,
                layer_info[i].page.height,layer_info[i].page.width,
                layer_info[i].channels);
            for (j=0; j < (long) layer_info[i].channels; j++)
            {
              layer_info[i].channel_info[j].type=(short)
                ReadBlobMSBShort(image);
              layer_info[i].channel_info[j].size=(unsigned long)
                GetPSDSize(&psd_info,image);
              if (image->debug != MagickFalse)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    channel[%ld]: type=%d, size=%ld",j,
                  layer_info[i].channel_info[j].type,
              (long) layer_info[i].channel_info[j].size);
            }
            count=ReadBlob(image,4,(unsigned char *) type);
            if ((count == 0) || (LocaleNCompare(type,"8BIM",4) != 0))
              {
                if (image->debug != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  layer type was %.4s instead of 8BIM", type);
                 ThrowReaderException(CorruptImageError,"ImproperImageHeader");
              }
            count=ReadBlob(image,4,(unsigned char *) layer_info[i].blendkey);
            layer_info[i].opacity=(Quantum) (QuantumRange-ScaleCharToQuantum(
              (unsigned char) ReadBlobByte(image)));
            layer_info[i].clipping=(unsigned char) ReadBlobByte(image);
            layer_info[i].flags=(unsigned char) ReadBlobByte(image);
            layer_info[i].visible=!(layer_info[i].flags & 0x02);
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "   blend=%.4s, opacity=%lu, clipping=%s, flags=%d, visible=%s",
                layer_info[i].blendkey,(unsigned long) layer_info[i].opacity,
                layer_info[i].clipping ? "true" : "false",layer_info[i].flags,
                layer_info[i].visible ? "true" : "false");
            (void) ReadBlobByte(image);  /* filler */
            combinedlength=0;
            size=ReadBlobMSBLong(image);
            if (size != 0)
              {
                if (image->debug != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "    layer contains additional info");
                length=ReadBlobMSBLong(image);
                if (length != 0)
                  {
                    /*
                      Layer mask info.
                    */
                    layer_info[i].mask.y=(long) ReadBlobMSBLong(image);
                    layer_info[i].mask.x=(long) ReadBlobMSBLong(image);
                    layer_info[i].mask.height=(unsigned long)
                      (ReadBlobMSBLong(image)-layer_info[i].mask.y);
                    layer_info[i].mask.width=(unsigned long)
                      (ReadBlobMSBLong(image)-layer_info[i].mask.x);
                    if (image->debug != MagickFalse)
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "      layer mask: offset(%ld,%ld), size(%ld,%ld), length=%ld",
                        layer_info[i].mask.x,layer_info[i].mask.y,
                        layer_info[i].mask.width, layer_info[i].mask.height,
                        (long) length-16);
                    /*
                      Skip over the rest of the layer mask information.
                    */
                    for (j=0; j < (long) (length-16); j++)
                      (void) ReadBlobByte(image);
                  }
                combinedlength+=length+4;  /* +4 for length */
                length=ReadBlobMSBLong(image);
                if (length != 0)
                  {
                    /*
                      Layer blending ranges info.
                    */
                    if (image->debug != MagickFalse)
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "      layer blending ranges: length=%ld",(long) length);
                    /*
                      We read it, but don't use it...
                    */
                    for (j=0; j < (long) (length); j+=8)
                    {
                      size_t blend_source=ReadBlobMSBLong(image);
                      size_t blend_dest=ReadBlobMSBLong(image);
                      if (image->debug != MagickFalse)
                        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "        source(%x), dest(%x)",(unsigned int)
                          blend_source,(unsigned int) blend_dest);
                    }
                  }
                combinedlength+=length+4;
                /*
                  Layer name.
                */
                length=(size_t) ReadBlobByte(image);
                for (j=0; j < (long) length; j++)
                  layer_info[i].name[j]=(unsigned char) ReadBlobByte(image);
                layer_info[i].name[j]='\0';
                if (image->debug != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "      layer name: %s",layer_info[i].name);
                combinedlength+=length+1;

#if     0  /* still in development */
          /*
            Adjustment layers and other stuff...
          */
          {
            char  alsig[4],
                alkey[4];

            count=ReadBlob(image,4,alsig);
            if ((count == 0) || (LocaleNCompare(alsig,"8BIM",4) != 0)) {
              if (debug != MagickFalse)
              {
      if (image->debug != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  adjustment layer type was %.4s instead of 8BIM", alsig);
              }
              ThrowReaderException(CorruptImageError,"ImproperImageHeader");
            }
            count=ReadBlob(image,4,alkey);
            length=ReadBlobMSBLong(image);
              if (debug != MagickFalse)
              {
      if (image->debug != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "      adjustment layer key: %.4s, data length=%ld",
                            alkey, length);
              }

              if ( length ) {
              for (j=0; j < (long) (length); j++)
                (void) ReadBlobByte(image);
              }

          }
          combinedlength += 12 + length;  /* sig, key, length + the actual length*/
#endif

               /*
                  Skip the rest of the variable data until we support it.
                */
                if (image->debug != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "      unsupported data: length=%ld",(long)
                    (size-combinedlength));
                for (j=0; j < (long) (size-combinedlength); j++)
                  (void) ReadBlobByte(image);
              }
            /*
              Allocate layered image.
            */
            layer_info[i].image=CloneImage(image,layer_info[i].page.width,
              layer_info[i].page.height,MagickFalse,&image->exception);
            if (layer_info[i].image == (Image *) NULL)
              {
                for (j=0; j < i; j++)
                  layer_info[j].image=DestroyImage(layer_info[j].image);
                if (image->debug != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  allocation of image for layer %ld failed", i);
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    setting up new layer image");
            (void) SetImageBackgroundColor(layer_info[i].image);
            layer_info[i].image->compose=
              PSDBlendModeToCompositeOperator(layer_info[i].blendkey);
            if (layer_info[i].visible == MagickFalse)
              layer_info[i].image->compose=NoCompositeOp;
            if (psd_info.mode == CMYKMode)
              layer_info[i].image->colorspace=CMYKColorspace;
            if ((psd_info.mode == BitmapMode) ||
                (psd_info.mode == GrayscaleMode) ||
                (psd_info.mode == DuotoneMode))
              layer_info[i].image->colorspace=GRAYColorspace;
            for (j=0; j < (long) layer_info[i].channels; j++)
              if (layer_info[i].channel_info[j].type == -1)
                layer_info[i].image->matte=MagickTrue;
            /*
              Set up some hidden attributes for folks that need them.
            */
            (void) FormatMagickString(message,MaxTextExtent,"%ld",
              layer_info[i].page.x);
            (void) SetImageArtifact(layer_info[i].image,"psd:layer.x",message);
            (void) FormatMagickString(message,MaxTextExtent,"%ld",
              layer_info[i].page.y);
            (void) SetImageArtifact(layer_info[i].image,"psd:layer.y",message);
            (void) FormatMagickString(message,MaxTextExtent,"%lu",
              (unsigned long) layer_info[i].opacity);
            (void) SetImageArtifact(layer_info[i].image,"psd:layer.opacity",
              message);
            (void) SetImageProperty(layer_info[i].image,"label",(char *)
              layer_info[i].name);
          }
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  reading image data for layers");
        /*
          Read pixel data for each layer.
        */
        for (i=0; i < number_layers; i++)
        {
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  reading data for layer %ld",i);
            for (j=0; j < (long) layer_info[i].channels; j++)
            {
              if (image->debug != MagickFalse)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    reading data for channel %ld", j);
#if     1
              if (layer_info[i].channel_info[j].size <= (2*layer_info[i].image->rows))
                {
                  long
                    k;

                  if (image->debug != MagickFalse)
                    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                      "      layer data is empty");
                  /*
                    A layer without data.
                  */
                  for (k=0; k < (long) layer_info[i].channel_info[j].size; k++)
                    (void) ReadBlobByte(layer_info[i].image);
                  continue;
                }
#endif
              offsets=(MagickOffsetType *) NULL;
              layer_info[i].image->compression=NoCompression;
              compression=ReadBlobMSBShort(layer_info[i].image);
              if ((layer_info[i].page.height != 0) &&
                  (layer_info[i].page.width != 0))
                {
                  if (compression == 1)
                    {
                      /*
                        Read RLE compressed data.
                      */
                      layer_info[i].image->compression=RLECompression;
                      if (image->debug != MagickFalse)
                        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "      layer data is RLE compressed");
                      offsets=(MagickOffsetType *) AcquireQuantumMemory(
                        layer_info[i].image->rows,sizeof(*offsets));
                      if (offsets == (MagickOffsetType *) NULL)
                        ThrowReaderException(ResourceLimitError,
                          "MemoryAllocationFailed");
                      for (y=0; y < (long) layer_info[i].image->rows; y++)
                        offsets[y]=GetPSDOffset(&psd_info,layer_info[i].image);
                    }
                  status=ReadPSDLayer(layer_info[i].image,
                    layer_info[i].channels,(unsigned long)
                    layer_info[i].channel_info[j].type,offsets,exception);
                  if (compression == 1)
                    offsets=(MagickOffsetType *) RelinquishMagickMemory(
                      offsets);
                  if (status == MagickFalse)
                    break;
                }
              }
            if (layer_info[i].opacity != OpaqueOpacity)
              {
                /*
                  Correct for opacity level.
                */
                for (y=0; y < (long) layer_info[i].image->rows; y++)
                {
                  q=GetAuthenticPixels(layer_info[i].image,0,y,
                    layer_info[i].image->columns,1,exception);
                  if (q == (PixelPacket *) NULL)
                    break;
                  indexes=GetAuthenticIndexQueue(layer_info[i].image);
                  for (x=0; x < (long) layer_info[i].image->columns; x++)
                  {
                    q->opacity=(Quantum) (QuantumRange-(Quantum) (QuantumScale*
                      ((QuantumRange-q->opacity)*(QuantumRange-
                      layer_info[i].opacity))));
                    q++;
                  }
                  if (SyncAuthenticPixels(layer_info[i].image,exception) == MagickFalse)
                    break;
                }
              }
            if (layer_info[i].image->colorspace == CMYKColorspace)
              (void) NegateImage(layer_info[i].image,MagickFalse);
            status=SetImageProgress(image,LoadImagesTag,i,(MagickSizeType)
              number_layers);
            if (status == MagickFalse)
              break;
          }
        /* added by palf -> invisible group layer make layer of this group
           invisible I consider that all layer with width and height null are
           layer for group layer */
       {
         short inside_layer = 0;
         short layer_visible = 0;
         for (i=number_layers-1; i >=0; i--)
         {
           if ((layer_info[i].page.width == 0) ||
               (layer_info[i].page.height == 0))
             {
               if (inside_layer == 0)
                 {
                   inside_layer=1;
                   layer_visible=(short int) layer_info[i].visible;
                 }
               else
                 {
                   inside_layer = 0;
                 }
             }
           else
             if ((inside_layer == 1) && (layer_visible == 0))
               {
                 layer_info[i].visible=(unsigned char) layer_visible;
                 layer_info[i].image->compose=NoCompositeOp;
               }
         }
       }
       /* added by palf -> suppression of empty layer */
       /* I consider that all layer with width and height null are layer for group layer */
       for (i=0; i < number_layers; i++)
       {
         if ((layer_info[i].page.width == 0) ||
             (layer_info[i].page.height == 0))
           {
             if (layer_info[i].image != (Image *) NULL)
               layer_info[i].image=DestroyImage(layer_info[i].image);
             for (j=i; j < number_layers - 1; j++)
               layer_info[j] = layer_info[j+1];
             number_layers--;
             i--;
           }
        }
        mask_size = ReadBlobMSBLong(image);  /* global mask size: currently ignored */
        if (number_layers > 0)
          {
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  putting layers into image list");
            for (i=0; i < number_layers; i++)
            {
              if (i > 0)
                layer_info[i].image->previous=layer_info[i-1].image;
              if (i < (number_layers-1))
                layer_info[i].image->next=layer_info[i+1].image;
              layer_info[i].image->page=layer_info[i].page;
            }
            image->next=layer_info[0].image;
            layer_info[0].image->previous=image;
            layer_info=(LayerInfo *) RelinquishMagickMemory(layer_info);
          }
        layer_offset-=TellBlob(image);
        offset=SeekBlob(image,layer_offset,SEEK_CUR);
      }
    }
  /*
    Read the precombined layer, present for PSD < 4 compatibility
  */
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  reading the precombined layer");
  offsets=(MagickOffsetType *) NULL;
  image->compression=NoCompression;
  compression=ReadBlobMSBShort(image);
  if (compression == 1)
    {
      /*
        Read Packbit encoded pixel data as separate planes.
      */
      image->compression=RLECompression;
      offsets=(MagickOffsetType *) AcquireQuantumMemory(image->rows,
        psd_info.channels*sizeof(*offsets));
      if (offsets == (MagickOffsetType *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      for (i=0; i < (long) (image->rows*psd_info.channels); i++)
        offsets[i]=GetPSDOffset(&psd_info,image);
    }
  for (i=0; i < (long) psd_info.channels; i++)
  {
    status=ReadPSDLayer(image,psd_info.channels,(unsigned long) i,offsets+i*
      image->rows,exception);
    if (status == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImagesTag,i,psd_info.channels);
    if (status == MagickFalse)
      break;
  }
  if (compression == 1)
    offsets=(MagickOffsetType *) RelinquishMagickMemory(offsets);
  if (image->colorspace == CMYKColorspace)
    (void) NegateImage(image,MagickFalse);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P S D I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPSDImage() adds properties for the PSD image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPSDImage method is:
%
%      unsigned long RegisterPSDImage(void)
%
*/
ModuleExport unsigned long RegisterPSDImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("PSB");
  entry->decoder=(DecodeImageHandler *) ReadPSDImage;
  entry->encoder=(EncodeImageHandler *) WritePSDImage;
  entry->magick=(IsImageFormatHandler *) IsPSD;
  entry->description=ConstantString("Adobe Large Document Format");
  entry->module=ConstantString("PSD");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PSD");
  entry->decoder=(DecodeImageHandler *) ReadPSDImage;
  entry->encoder=(EncodeImageHandler *) WritePSDImage;
  entry->magick=(IsImageFormatHandler *) IsPSD;
  entry->description=ConstantString("Adobe Photoshop bitmap");
  entry->module=ConstantString("PSD");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P S D I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPSDImage() removes format registrations made by the
%  PSD module from the list of supported formats.
%
%  The format of the UnregisterPSDImage method is:
%
%      UnregisterPSDImage(void)
%
*/
ModuleExport void UnregisterPSDImage(void)
{
  (void) UnregisterMagickInfo("PSB");
  (void) UnregisterMagickInfo("PSD");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P S D I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePSDImage() writes an image in the Adobe Photoshop encoded image format.
%
%  The format of the WritePSDImage method is:
%
%      MagickBooleanType WritePSDImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%
*/

static inline ssize_t SetPSDOffset(const PSDInfo *psd_info,Image *image,
  const size_t offset)
{
  if (psd_info->version == 1)
    return(WriteBlobMSBShort(image,(unsigned short) offset));
  return(WriteBlobMSBLong(image,(unsigned short) offset));
}

static inline ssize_t SetPSDSize(const PSDInfo *psd_info,Image *image,
  const MagickSizeType size)
{
  if (psd_info->version == 1)
    return(WriteBlobMSBLong(image,(unsigned int) size));
  return(WriteBlobMSBLongLong(image,size));
}

static size_t PSDPackbitsEncodeImage(Image *image,const size_t length,
  const unsigned char *pixels,unsigned char *compact_pixels)
{
  int
    count;

  register long
    i,
    j;

  register unsigned char
    *q;

  unsigned char
    *packbits;

  /*
    Compress pixels with Packbits encoding.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(pixels != (unsigned char *) NULL);
  packbits=(unsigned char *) AcquireQuantumMemory(128UL,sizeof(*packbits));
  if (packbits == (unsigned char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  q=compact_pixels;
  for (i=(long) length; i != 0; )
  {
    switch (i)
    {
      case 1:
      {
        i--;
        *q++=(unsigned char) 0;
        *q++=(*pixels);
        break;
      }
      case 2:
      {
        i-=2;
        *q++=(unsigned char) 1;
        *q++=(*pixels);
        *q++=pixels[1];
        break;
      }
      case 3:
      {
        i-=3;
        if ((*pixels == *(pixels+1)) && (*(pixels+1) == *(pixels+2)))
          {
            *q++=(unsigned char) ((256-3)+1);
            *q++=(*pixels);
            break;
          }
        *q++=(unsigned char) 2;
        *q++=(*pixels);
        *q++=pixels[1];
        *q++=pixels[2];
        break;
      }
      default:
      {
        if ((*pixels == *(pixels+1)) && (*(pixels+1) == *(pixels+2)))
          {
            /*
              Packed run.
            */
            count=3;
            while (((long) count < i) && (*pixels == *(pixels+count)))
            {
              count++;
              if (count >= 127)
                break;
            }
            i-=count;
            *q++=(unsigned char) ((256-count)+1);
            *q++=(*pixels);
            pixels+=count;
            break;
          }
        /*
          Literal run.
        */
        count=0;
        while ((*(pixels+count) != *(pixels+count+1)) ||
               (*(pixels+count+1) != *(pixels+count+2)))
        {
          packbits[count+1]=pixels[count];
          count++;
          if (((long) count >= (i-3)) || (count >= 127))
            break;
        }
        i-=count;
        *packbits=(unsigned char) (count-1);
        for (j=0; j <= (long) count; j++)
          *q++=packbits[j];
        pixels+=count;
        break;
      }
    }
  }
  *q++=(unsigned char) 128;  /* EOD marker */
  packbits=(unsigned char *) RelinquishMagickMemory(packbits);
  return((size_t) (q-compact_pixels));
}

static void WritePackbitsLength(const PSDInfo *psd_info,
  const ImageInfo *image_info,Image *image,Image *tmp_image,
  unsigned char *pixels,unsigned char *compact_pixels,
  const QuantumType quantum_type)
{
  int
    y;

  QuantumInfo
    *quantum_info;

  register const PixelPacket
    *p;

  size_t
    length,
    packet_size;

  if (tmp_image->depth > 8)
    tmp_image->depth=16;
  packet_size=tmp_image->depth > 8UL ? 2UL : 1UL;
  quantum_info=AcquireQuantumInfo(image_info,image);
  for (y=0; y < (long) tmp_image->rows; y++)
  {
    p=GetVirtualPixels(tmp_image,0,y,tmp_image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    length=ExportQuantumPixels(tmp_image,(CacheView *) NULL,quantum_info,
      quantum_type,pixels,&image->exception);
    length=PSDPackbitsEncodeImage(image,length,pixels,compact_pixels);
    (void) SetPSDOffset(psd_info,image,length);
  }
  quantum_info=DestroyQuantumInfo(quantum_info);
}

static void WriteOneChannel(const PSDInfo *psd_info,const ImageInfo *image_info,
  Image *image,Image *tmp_image,unsigned char *pixels,
  unsigned char *compact_pixels,const QuantumType quantum_type,
  const MagickBooleanType compression_flag)
{
  int
    y;

  MagickBooleanType
    monochrome;

  QuantumInfo
    *quantum_info;

  register const PixelPacket
    *p;

  register long
    i;

  size_t
    length,
    packet_size;

  (void) psd_info;
  if ((compression_flag != MagickFalse) &&
      (tmp_image->compression != RLECompression))
    (void) WriteBlobMSBShort(image,0);
  if (tmp_image->depth > 8)
    tmp_image->depth=16;
  monochrome=IsMonochromeImage(image,&image->exception);
  packet_size=tmp_image->depth > 8UL ? 2UL : 1UL;
  quantum_info=AcquireQuantumInfo(image_info,image);
  for (y=0; y < (long) tmp_image->rows; y++)
  {
    p=GetVirtualPixels(tmp_image,0,y,tmp_image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    length=ExportQuantumPixels(tmp_image,(CacheView *) NULL,quantum_info,
      quantum_type,pixels,&image->exception);
    if (monochrome != MagickFalse)
      for (i=0; i < (long) length; i++)
        pixels[i]=(~pixels[i]);
    if (tmp_image->compression != RLECompression)
      (void) WriteBlob(image,length,pixels);
    else
      {
        length=PSDPackbitsEncodeImage(image,length,pixels,compact_pixels);
        (void) WriteBlob(image,length,compact_pixels);
      }
  }
  quantum_info=DestroyQuantumInfo(quantum_info);
}

static MagickBooleanType WriteImageChannels(const PSDInfo *psd_info,
  const ImageInfo *image_info,Image *image,Image *tmp_image,
  const MagickBooleanType separate)
{
  int
    i;

  size_t
    channels,
    packet_size;

  unsigned char
    *compact_pixels,
    *pixels;

  /*
    Write uncompressed pixels as separate planes.
  */
  channels=1;
  if ((tmp_image->storage_class == PseudoClass) &&
      (tmp_image->matte == MagickFalse))
    channels++;
  packet_size=tmp_image->depth > 8UL ? 2UL : 1UL;
  pixels=(unsigned char *) AcquireQuantumMemory(channels*tmp_image->columns,
    packet_size*sizeof(*pixels));
  compact_pixels=(unsigned char *) AcquireQuantumMemory(2*channels*
    tmp_image->columns,packet_size*sizeof(*pixels));
  if ((pixels == (unsigned char *) NULL) ||
      (compact_pixels == (unsigned char *) NULL))
    {
      if (pixels != (unsigned char *) NULL)
        pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      if (compact_pixels != (unsigned char *) NULL)
        compact_pixels=(unsigned char *)
          RelinquishMagickMemory(compact_pixels);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  i=0;
  if (IsGrayImage(tmp_image,&tmp_image->exception) != MagickFalse)
    {
      tmp_image->compression=RLECompression;
      if (tmp_image->compression == RLECompression)
        {
          /*
            Packbits compression.
          */
          (void) WriteBlobMSBShort(image,1);
          if (tmp_image->matte != MagickFalse)
            WritePackbitsLength(psd_info,image_info,image,tmp_image,pixels,
              compact_pixels,AlphaQuantum);
          WritePackbitsLength(psd_info,image_info,image,tmp_image,pixels,
           compact_pixels,GrayQuantum);
        }
      WriteOneChannel(psd_info,image_info,image,tmp_image,pixels,
        compact_pixels,GrayQuantum,(i++ == 0) || (separate != MagickFalse) ?
        MagickTrue : MagickFalse);
    }
  else
    if (tmp_image->storage_class == PseudoClass)
      {
        if (tmp_image->compression == RLECompression)
          {
            /*
              Packbits compression.
            */
            (void) WriteBlobMSBShort(image,1);
            if (tmp_image->matte == MagickFalse)
              WritePackbitsLength(psd_info,image_info,image,tmp_image,pixels,
                compact_pixels,IndexQuantum);
            else
              WritePackbitsLength(psd_info,image_info,image,tmp_image,pixels,
                compact_pixels,IndexAlphaQuantum);
          }
        if (tmp_image->matte == MagickFalse)
          WriteOneChannel(psd_info,image_info,image,tmp_image,pixels,
            compact_pixels,IndexQuantum,(i++ == 0) ||
            (separate != MagickFalse) ? MagickTrue : MagickFalse);
        else
          WriteOneChannel(psd_info,image_info,image,tmp_image,pixels,
            compact_pixels,IndexAlphaQuantum,(i++ == 0) ||
            (separate != MagickFalse) ?  MagickTrue : MagickFalse);
        (void) SetImageProgress(image,SaveImagesTag,0,1);
      }
    else
      {
        if (tmp_image->colorspace == CMYKColorspace)
          {
            tmp_image->compression=NoCompression;
            (void) NegateImage(tmp_image,MagickFalse);
          }
        if (tmp_image->compression == RLECompression)
          {
            /*
              Packbits compression.
            */
            (void) WriteBlobMSBShort(image,1);
            if (tmp_image->matte != MagickFalse)
              WritePackbitsLength(psd_info,image_info,image,tmp_image,pixels,
                compact_pixels,AlphaQuantum);
            WritePackbitsLength(psd_info,image_info,image,tmp_image,pixels,
              compact_pixels,RedQuantum);
            WritePackbitsLength(psd_info,image_info,image,tmp_image,pixels,
              compact_pixels,GreenQuantum);
            WritePackbitsLength(psd_info,image_info,image,tmp_image,pixels,
              compact_pixels,BlueQuantum);
            if (tmp_image->colorspace == CMYKColorspace)
              WritePackbitsLength(psd_info,image_info,image,tmp_image,pixels,
                compact_pixels,BlackQuantum);
          }
        (void) SetImageProgress(image,SaveImagesTag,0,6);
        if (tmp_image->matte != MagickFalse)
          WriteOneChannel(psd_info,image_info,image,tmp_image,pixels,
            compact_pixels,AlphaQuantum,(i++ == 0) ||
            (separate != MagickFalse) ? MagickTrue : MagickFalse);
        (void) SetImageProgress(image,SaveImagesTag,1,6);
        WriteOneChannel(psd_info,image_info,image,tmp_image,pixels,
          compact_pixels,RedQuantum,(i++ == 0) || (separate != MagickFalse) ?
          MagickTrue : MagickFalse);
        (void) SetImageProgress(image,SaveImagesTag,2,6);
        WriteOneChannel(psd_info,image_info,image,tmp_image,pixels,
          compact_pixels,GreenQuantum,(i++ == 0) || (separate != MagickFalse) ?
          MagickTrue : MagickFalse);
        (void) SetImageProgress(image,SaveImagesTag,3,6);
        WriteOneChannel(psd_info,image_info,image,tmp_image,pixels,
          compact_pixels,BlueQuantum,(i++ == 0) || (separate != MagickFalse) ?
          MagickTrue : MagickFalse);
        (void) SetImageProgress(image,SaveImagesTag,4,6);
        if (tmp_image->colorspace == CMYKColorspace)
          {
            WriteOneChannel(psd_info,image_info,image,tmp_image,pixels,
              compact_pixels,BlackQuantum,(i++ == 0) ||
              (separate != MagickFalse) ? MagickTrue : MagickFalse);
            (void) NegateImage(tmp_image,MagickFalse);
          }
        (void) SetImageProgress(image,SaveImagesTag,5,6);
      }
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  return(MagickTrue);
}

static void WritePascalString(Image* inImage,const char *inString,int inPad)
{
  size_t
    strLength;
  int i;

  /* max length is 255 */

  strLength = (strlen(inString) > 255UL ) ? 255UL : strlen(inString);

  if ( strLength !=  0 )
  {
  (void) WriteBlobByte(inImage,(unsigned char) strLength);
  (void) WriteBlob(inImage, strLength, (const unsigned char *) inString);
  }
  else
  (void) WriteBlobByte(inImage, 0);

  strLength ++;

  if ( (strLength % inPad) == 0 )
    return;
  for (i=0; i < (long) (inPad-(strLength % inPad)); i++)
    (void) WriteBlobByte(inImage,0);
}

static void WriteResolutionResourceBlock(Image *image)
{
  double
    x_resolution,
    y_resolution;

  unsigned short
    units;

  x_resolution=65536.0*image->x_resolution+0.5;
  y_resolution=65536.0*image->y_resolution+0.5;
  units=1;
  if (image->units == PixelsPerCentimeterResolution)
    {
      x_resolution=2.54*65536.0*image->x_resolution*0.5;
      y_resolution=2.54*65536.0*image->y_resolution+0.5;
      units=2;
    }
  (void) WriteBlob(image,4,(const unsigned char *) "8BIM");
  (void) WriteBlobMSBShort(image,0x03ED);
  (void) WriteBlobMSBShort(image,0);
  (void) WriteBlobMSBLong(image,16); /* resource size */
  (void) WriteBlobMSBLong(image,(unsigned int) (x_resolution+0.5));
  (void) WriteBlobMSBShort(image,units); /* horizontal resolution unit */
  (void) WriteBlobMSBShort(image,units); /* width unit */
  (void) WriteBlobMSBLong(image,(unsigned int) (y_resolution+0.5));
  (void) WriteBlobMSBShort(image,units); /* vertical resolution unit */
  (void) WriteBlobMSBShort(image,units); /* height unit */
}

static MagickBooleanType WritePSDImage(const ImageInfo *image_info,Image *image)
{
  const char
    *theAttr;

  const StringInfo
    *bim_profile,
    *icc_profile;

  MagickBooleanType
    invert_layer_count = MagickFalse,
    status;

  PSDInfo
    psd_info;

  register long
    i;

  size_t
    length,
    num_channels,
    packet_size;

  unsigned char
    layer_name[4];

  unsigned long
    channel_size,
    channelLength,
    layer_count,
    layer_info_size,
    rounded_layer_info_size;

  Image
    * tmp_image = (Image *) NULL,
    * base_image = GetNextImageInList(image);

  /*
    Open image file.
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
  packet_size=(size_t) (image->depth > 8 ? 6 : 3);
  if (image->matte != MagickFalse)
    packet_size+=image->depth > 8 ? 2 : 1;
  psd_info.version=1;
  if ((LocaleCompare(image_info->magick,"PSB") == 0) ||
      (image->columns > 30000) || (image->rows > 30000))
    psd_info.version=2;
  (void) WriteBlob(image,4,(const unsigned char *) "8BPS");
  (void) WriteBlobMSBShort(image,psd_info.version);  /* version */
  for (i=1; i <= 6; i++)
    (void) WriteBlobByte(image, 0);  /* 6 bytes of reserved */
  if (IsGrayImage(image,&image->exception) != MagickFalse)
    num_channels=1;
  else
    if (image->storage_class == PseudoClass)
      num_channels=(image->matte ? 2UL : 1UL);
    else
      {
        if (image->colorspace != CMYKColorspace)
          num_channels=(image->matte ? 4UL : 3UL);
        else
          num_channels=(image->matte ? 5UL : 4UL);
      }
  (void) WriteBlobMSBShort(image,(unsigned short) num_channels);
  (void) WriteBlobMSBLong(image,(unsigned int) image->rows);
  (void) WriteBlobMSBLong(image,(unsigned int) image->columns);
  if (IsGrayImage(image,&image->exception) != MagickFalse)
    {
      MagickBooleanType
        monochrome;

      /*
        Write depth & mode.
      */
      monochrome=IsMonochromeImage(image,&image->exception);
      (void) WriteBlobMSBShort(image,monochrome != MagickFalse ? 1 :
        image->depth > 8 ? 16 : 8);
      (void) WriteBlobMSBShort(image,monochrome != MagickFalse ?
        BitmapMode : GrayscaleMode);
    }
  else
    {
      (void) WriteBlobMSBShort(image,(unsigned short)
        (image->storage_class == PseudoClass ? 8 : image->depth > 8 ? 16 : 8));
      if (((image->colorspace != UndefinedColorspace) ||
           (image->colorspace != CMYKColorspace)) &&
           (image->colorspace != CMYKColorspace))
        {
          if (image->colorspace != RGBColorspace)
            (void) TransformImageColorspace(image,RGBColorspace);
          (void) WriteBlobMSBShort(image,(unsigned short)
            (image->storage_class == PseudoClass ? IndexedMode : RGBMode));
        }
      else
        {
          if (image->colorspace != RGBColorspace)
            (void) TransformImageColorspace(image,CMYKColorspace);
          (void) WriteBlobMSBShort(image,CMYKMode);
        }
    }
  if ((IsGrayImage(image,&image->exception) != MagickFalse) ||
      (image->storage_class == DirectClass) || (image->colors > 256))
    (void) WriteBlobMSBLong(image,0);
  else
    {
      /*
        Write PSD raster colormap.
      */
      (void) WriteBlobMSBLong(image,768);
      for (i=0; i < (long) image->colors; i++)
        (void) WriteBlobByte(image,ScaleQuantumToChar(image->colormap[i].red));
      for ( ; i < 256; i++)
        (void) WriteBlobByte(image,0);
      for (i=0; i < (long) image->colors; i++)
        (void) WriteBlobByte(image,ScaleQuantumToChar(image->colormap[i].green));
      for ( ; i < 256; i++)
        (void) WriteBlobByte(image,0);
      for (i=0; i < (long) image->colors; i++)
        (void) WriteBlobByte(image,ScaleQuantumToChar(image->colormap[i].blue));
      for ( ; i < 256; i++)
        (void) WriteBlobByte(image,0);
    }
  /*
    Image resource block.
  */
  length=28; /* 0x03EB */
  bim_profile=(StringInfo *) NULL;
  icc_profile=GetImageProfile(image,"icc");
  if (icc_profile != (StringInfo *) NULL)
    length+=PSDQuantum(GetStringInfoLength(icc_profile))+12;
  else
    {
      bim_profile=GetImageProfile(image,"8bim");
      if (bim_profile != (StringInfo *) NULL)
        length+=GetStringInfoLength(bim_profile);
    }
  (void) WriteBlobMSBLong(image,length);
  WriteResolutionResourceBlock(image);
  if (bim_profile != (StringInfo *) NULL)
    (void) WriteBlob(image,GetStringInfoLength(bim_profile),GetStringInfoDatum(
      bim_profile));
  if (icc_profile != (StringInfo *) NULL)
    {
      (void) WriteBlob(image,4,(const unsigned char *) "8BIM");
      (void) WriteBlobMSBShort(image,0x040F);
      (void) WriteBlobMSBShort(image,0);
      (void) WriteBlobMSBLong(image,GetStringInfoLength(icc_profile));
      (void) WriteBlob(image,GetStringInfoLength(icc_profile),
        GetStringInfoDatum(icc_profile));
      if ((MagickOffsetType) GetStringInfoLength(icc_profile) !=
          PSDQuantum(GetStringInfoLength(icc_profile)))
        (void) WriteBlobByte(image,0);
    }

compute_layer_info:
  layer_count = 0;
  layer_info_size = 2;
  tmp_image = base_image;
  while ( tmp_image != NULL ) {
    packet_size=tmp_image->depth > 8 ? 2UL : 1UL;

    if (IsGrayImage(image,&image->exception) != MagickFalse)
      num_channels=1;
    else
      if (tmp_image->storage_class == PseudoClass)
       num_channels = (tmp_image->matte != MagickFalse ? 2UL : 1UL);
      else
        if (tmp_image->colorspace != CMYKColorspace)
          num_channels = (tmp_image->matte != MagickFalse ? 4UL : 3UL);
        else
        num_channels = (tmp_image->matte != MagickFalse ? 5UL : 4UL);

    channelLength=(unsigned long) (tmp_image->columns * tmp_image->rows *
      packet_size + 2);
    layer_info_size += (unsigned long) (4*4 + 2 + num_channels * 6 +
      (psd_info.version == 1 ? 8 : 16) + 4 * 1 + 4 + num_channels *
      channelLength);
    theAttr=(const char *) GetImageProperty(tmp_image,"label");
    if (!theAttr)
      layer_info_size += 16;
    else
      {
        size_t length=strlen(theAttr);
        layer_info_size += 8+length+(4-(length % 4))+1;
      }
    layer_count++;
    tmp_image = GetNextImageInList(tmp_image);
  }

  /* if the image has a matte, then we need to use layers */
  if ( layer_count == 0 && image->matte == MagickTrue )
  {
  invert_layer_count = MagickTrue;
  base_image = image;
  goto compute_layer_info;  /* yes, goto's suck, but it keeps the code cleaner! */
  }

  if ( layer_count == 0 )
    (void) SetPSDSize(&psd_info,image,0);
  else
  {
    (void) SetPSDSize(&psd_info,image,layer_info_size+
      (psd_info.version == 1 ? 8 : 16));
    if ( layer_info_size/2 != (layer_info_size+1)/2 ) /* odd */
      rounded_layer_info_size = layer_info_size + 1;
    else
      rounded_layer_info_size = layer_info_size;
    (void) SetPSDSize(&psd_info,image,rounded_layer_info_size);

    if ( invert_layer_count )
      layer_count *= -1;  /* if we have a matte, then use negative count! */
    (void) WriteBlobMSBShort(image,(unsigned short) layer_count);

    layer_count = 1;
    tmp_image = base_image;
    tmp_image->compression=NoCompression;
    while ( tmp_image != NULL ) {
      (void) WriteBlobMSBLong(image,0);
      (void) WriteBlobMSBLong(image,0);
      (void) WriteBlobMSBLong(image,(unsigned int) tmp_image->rows);
      (void) WriteBlobMSBLong(image,(unsigned int) tmp_image->columns);

      packet_size=tmp_image->depth > 8 ? 2UL : 1UL;
      channel_size=(unsigned int) ((packet_size*tmp_image->rows*
        tmp_image->columns)+2);
      if (tmp_image->storage_class == PseudoClass) {
       (void) WriteBlobMSBShort(image,(unsigned short)
         (tmp_image->matte ? 2 : 1));
       if (tmp_image->matte) {
         (void) WriteBlobMSBShort(image,(unsigned short) -1);
         (void) SetPSDSize(&psd_info,image,channel_size);
       }
       (void) WriteBlobMSBShort(image, 0);
       (void) SetPSDSize(&psd_info,image,channel_size);
      } else
      if (tmp_image->colorspace != CMYKColorspace)
      {
        (void) WriteBlobMSBShort(image,(unsigned short)
          (tmp_image->matte ? 4 : 3));
       if (tmp_image->matte) {
         (void) WriteBlobMSBShort(image,(unsigned short) -1);
         (void) SetPSDSize(&psd_info,image,channel_size);
       }
       (void) WriteBlobMSBShort(image, 0);
       (void) SetPSDSize(&psd_info,image,channel_size);
       (void) WriteBlobMSBShort(image, 1);
       (void) SetPSDSize(&psd_info,image,channel_size);
       (void) WriteBlobMSBShort(image, 2);
       (void) SetPSDSize(&psd_info,image,channel_size);
      }
      else
      {
        (void) WriteBlobMSBShort(image,(unsigned short)
          (tmp_image->matte ? 5 : 4));
       if (tmp_image->matte) {
         (void) WriteBlobMSBShort(image,(unsigned short) -1);
         (void) SetPSDSize(&psd_info,image,channel_size);
       }
       (void) WriteBlobMSBShort(image, 0);
       (void) SetPSDSize(&psd_info,image,channel_size);
       (void) WriteBlobMSBShort(image, 1);
       (void) SetPSDSize(&psd_info,image,channel_size);
       (void) WriteBlobMSBShort(image, 2);
       (void) SetPSDSize(&psd_info,image,channel_size);
       (void) WriteBlobMSBShort(image, 3);
       (void) SetPSDSize(&psd_info,image,channel_size);
      }

      (void) WriteBlob(image,4,(const unsigned char *) "8BIM");
      (void) WriteBlob(image,4,(const unsigned char *)
        CompositeOperatorToPSDBlendMode(tmp_image->compose));
      (void) WriteBlobByte(image, 255); /* BOGUS: layer opacity */
      (void) WriteBlobByte(image, 0);
      (void) WriteBlobByte(image, 1); /* BOGUS: layer attributes - visible, etc. */
      (void) WriteBlobByte(image, 0);


      theAttr=(const char *) GetImageProperty(tmp_image,"label");
      if (!theAttr) {
        (void) WriteBlobMSBLong(image, 16);
        (void) WriteBlobMSBLong(image, 0);
        (void) WriteBlobMSBLong(image, 0);
        (void) FormatMagickString((char *) layer_name,MaxTextExtent,"L%06ld",
          layer_count++ );
        WritePascalString( image, (char*)layer_name, 4 );
      } else {
        size_t length=strlen(theAttr);
        (void) WriteBlobMSBLong(image,(unsigned int) (length+(4-(length % 4))+
          1+8));
        (void) WriteBlobMSBLong(image,0);
        (void) WriteBlobMSBLong(image,0);
        WritePascalString( image, theAttr, 4 );
      }
      tmp_image = GetNextImageInList(tmp_image);
    };

     /* now the image data! */
    tmp_image = base_image;
    while ( tmp_image != NULL ) {
      status=WriteImageChannels(&psd_info,image_info,image,tmp_image,MagickTrue);

      /* add in the pad! */
       if ( rounded_layer_info_size != layer_info_size )
         (void) WriteBlobByte(image,'\0');

      tmp_image = GetNextImageInList(tmp_image);
    };

    /* user mask data */
     (void) WriteBlobMSBLong(image, 0);

  }

  /* now the background image data! */
  status=WriteImageChannels(&psd_info,image_info,image,image,MagickFalse);

  (void) CloseBlob(image);
  return(status);
}

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
#include "magick/artifact.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/colormap.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
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
#include "magick/pixel.h"
#include "magick/pixel-accessor.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"

/*
  Define declaractions.
*/
#define MaxPSDChannels  56
#define PSDQuantum(x) (((ssize_t) (x)+1) & -2)

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

  size_t
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

  size_t
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
    color_channels,
    version;

  unsigned char
    reserved[6];

  size_t
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

static const char *CompositeOperatorToPSDBlendMode(CompositeOperator op)
{
  const char
    *blend_mode;

  switch (op)
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
  const unsigned char *compact_pixels,const ssize_t depth,
  const size_t number_pixels,unsigned char *pixels)
{
  int
    pixel;

  register ssize_t
    i,
    j;

  size_t
    length;

  ssize_t
    packets;

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
        for (j=0; j < (ssize_t) length; j++)
        {
          switch (depth)
          {
            case 1:
            {
              *pixels++=(pixel >> 7) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 6) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 5) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 4) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 3) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 2) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 1) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 0) & 0x01 ? 0U : 255U;
              i+=8;
              break;
            }
            case 4:
            {
              *pixels++=(unsigned char) ((pixel >> 4) & 0xff);
              *pixels++=(unsigned char) ((pixel & 0x0f) & 0xff);
              i+=2;
              break;
            }
            case 2:
            {
              *pixels++=(unsigned char) ((pixel >> 6) & 0x03);
              *pixels++=(unsigned char) ((pixel >> 4) & 0x03);
              *pixels++=(unsigned char) ((pixel >> 2) & 0x03);
              *pixels++=(unsigned char) ((pixel & 0x03) & 0x03);
              i+=4;
              break;
            }
            default:
            {
              *pixels++=(unsigned char) pixel;
              i++;
              break;
            }
          }
        }
        continue;
      }
    length++;
    for (j=0; j < (ssize_t) length; j++)
    {
      switch (depth)
      {
        case 1:
        {
          *pixels++=(*compact_pixels >> 7) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 6) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 5) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 4) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 3) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 2) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 1) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 0) & 0x01 ? 0U : 255U;
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

static inline ssize_t MagickAbsoluteValue(const ssize_t x)
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

  unsigned int
    count,
    long_sans;

  unsigned short
    id,
    short_sans;

  if (length < 16)
    return(MagickFalse);
  profile=BlobToStringInfo((const void *) NULL,length);
  SetStringInfoDatum(profile,blocks);
  (void) SetImageProfile(image,"8bim",profile);
  profile=DestroyStringInfo(profile);
  for (p=blocks; (p >= blocks) && (p < (blocks+length-16)); )
  {
    if (LocaleNCompare((const char *) p,"8BIM",4) != 0)
      break;
    p=PushLongPixel(MSBEndian,p,&long_sans);
    p=PushShortPixel(MSBEndian,p,&id);
    p=PushShortPixel(MSBEndian,p,&short_sans);
    p=PushLongPixel(MSBEndian,p,&count);
    switch (id)
    {
      case 0x03ed:
      {
        char
          value[MaxTextExtent];

        unsigned short
          resolution;

        /*
          Resolution info.
        */
        p=PushShortPixel(MSBEndian,p,&resolution);
        image->x_resolution=(double) resolution;
        (void) FormatLocaleString(value,MaxTextExtent,"%g",image->x_resolution);
        (void) SetImageProperty(image,"tiff:XResolution",value);
        p=PushShortPixel(MSBEndian,p,&short_sans);
        p=PushShortPixel(MSBEndian,p,&short_sans);
        p=PushShortPixel(MSBEndian,p,&short_sans);
        p=PushShortPixel(MSBEndian,p,&resolution);
        image->y_resolution=(double) resolution;
        (void) FormatLocaleString(value,MaxTextExtent,"%g",image->y_resolution);
        (void) SetImageProperty(image,"tiff:YResolution",value);
        p=PushShortPixel(MSBEndian,p,&short_sans);
        p=PushShortPixel(MSBEndian,p,&short_sans);
        p=PushShortPixel(MSBEndian,p,&short_sans);
        image->units=PixelsPerInchResolution;
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

static MagickBooleanType ReadPSDLayer(Image *image,const size_t channels,
  const ssize_t type,const MagickOffsetType *offsets,ExceptionInfo *exception)
{
  ColorspaceType
    colorspace;

  Quantum
    pixel;

  register const unsigned char
    *p;

  register IndexPacket
    *indexes;

  register PixelPacket
    *q;

  register ssize_t
    x;

  size_t
    packet_size;

  ssize_t
    count,
    y;

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
      for (y=0; y < (ssize_t) image->rows; y++)
        if ((MagickOffsetType) length < offsets[y])
          length=(size_t) offsets[y];
      compact_pixels=(unsigned char *) AcquireQuantumMemory(length,
        sizeof(*pixels));
      if (compact_pixels == (unsigned char *) NULL)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
      (void) ResetMagickMemory(compact_pixels,0,length*sizeof(*compact_pixels));
    }
  colorspace=image->colorspace;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    if (image->depth == 1)
      {
        if (image->compression != RLECompression)
          count=ReadBlob(image,(image->columns+7)/8,pixels);
        else
          {
            count=ReadBlob(image,(size_t) offsets[y],compact_pixels);
            if (count != (ssize_t) offsets[y])
              break;
            count=DecodePSDPixels((size_t) offsets[y],compact_pixels,
              (ssize_t) 123456,(size_t) ((image->columns+7)/8),pixels);
          }
        if (count < (ssize_t) ((image->columns+7)/8))
          break;
      }
    else
      {
        if (image->compression != RLECompression)
          count=ReadBlob(image,packet_size*image->columns,pixels);
        else
          {
            count=ReadBlob(image,(size_t) offsets[y],compact_pixels);
            if (count != (ssize_t) offsets[y])
              break;
            count=DecodePSDPixels((size_t) offsets[y],compact_pixels,
              (ssize_t) image->depth,packet_size*image->columns,pixels);
          }
        if (count < (ssize_t) (packet_size*image->columns))
          break;
      }
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    p=pixels;
    for (x=0; x < (ssize_t) image->columns; x++)
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
          SetPixelAlpha(q,pixel);
          break;
        }
        case 0:
        {
          SetPixelRed(q,pixel);
          if (channels == 1)
            {
              SetPixelGreen(q,GetPixelRed(q));
              SetPixelBlue(q,GetPixelRed(q));
            }
          if (image->storage_class == PseudoClass)
            {
              if (packet_size == 1)
                SetPixelIndex(indexes+x,ScaleQuantumToChar(pixel));
              else
                SetPixelIndex(indexes+x,ScaleQuantumToShort(pixel));
              SetPixelRGBO(q,image->colormap+(ssize_t)
                GetPixelIndex(indexes+x));
              if (image->depth == 1)
                {
                  ssize_t
                    bit,
                    number_bits;

                  number_bits=image->columns-x;
                  if (number_bits > 8)
                    number_bits=8;
                  for (bit=0; bit < number_bits; bit++)
                  {
                    SetPixelIndex(indexes+x,(((unsigned char) pixel) &
                      (0x01 << (7-bit))) != 0 ? 0 : 255);
                    SetPixelRGBO(q,image->colormap+(ssize_t)
                      GetPixelIndex(indexes+x));
                    q++;
                    x++;
                  }
                }
            }
          break;
        }
        case 1:
        {
          if (image->storage_class == PseudoClass)
            SetPixelAlpha(q,pixel);
          else
            SetPixelGreen(q,pixel);
          break;
        }
        case 2:
        {
          if (image->storage_class == PseudoClass)
            SetPixelAlpha(q,pixel);
          else
            SetPixelBlue(q,pixel);
          break;
        }
        case 3:
        {
          if (image->colorspace == CMYKColorspace)
            SetPixelIndex(indexes+x,pixel);
          else
            if (image->matte != MagickFalse)
              SetPixelAlpha(q,pixel);
          break;
        }
        case 4:
        {
          if ((IssRGBCompatibleColorspace(image->colorspace) != MagickFalse) &&
              (channels > 3))
            break;
          if (image->matte != MagickFalse)
            SetPixelAlpha(q,pixel);
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
  image->colorspace=colorspace;
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

  LayerInfo
    *layer_info;

  MagickBooleanType
    check_background,
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

  register PixelPacket
    *q;

  register ssize_t
    i,
    x;

  size_t
    mask_size,
    skip_first_alpha = 0;

  ssize_t
    count,
    j,
    number_layers,
    y;

  unsigned char
    *data;

  unsigned short
    compression;

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
  psd_info.color_channels=psd_info.channels;
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
      "  Image is %.20g x %.20g with channels=%.20g, depth=%.20g, mode=%s",
      (double) psd_info.columns,(double) psd_info.rows,(double)
      psd_info.channels,(double) psd_info.depth,ModeToString((PSDImageType)
      psd_info.mode));
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
  if (psd_info.mode == LabMode)
    SetImageColorspace(image,LabColorspace);
  psd_info.color_channels=3;
  if (psd_info.mode == CMYKMode)
    {
      psd_info.color_channels=4;
      SetImageColorspace(image,CMYKColorspace);
    }
  if ((psd_info.mode == BitmapMode) || (psd_info.mode == GrayscaleMode) ||
      (psd_info.mode == DuotoneMode))
    {
      psd_info.color_channels=1;
      if (AcquireImageColormap(image,256) == MagickFalse)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  Image colormap allocated");
      SetImageColorspace(image,GRAYColorspace);
    }
  image->matte=MagickFalse;
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
          if (AcquireImageColormap(image,(size_t) (length/3)) == MagickFalse)
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          for (i=0; i < (ssize_t) image->colors; i++)
            image->colormap[i].red=ScaleCharToQuantum((unsigned char)
              ReadBlobByte(image));
          for (i=0; i < (ssize_t) image->colors; i++)
            image->colormap[i].green=ScaleCharToQuantum((unsigned char)
              ReadBlobByte(image));
          for (i=0; i < (ssize_t) image->colors; i++)
            image->colormap[i].blue=ScaleCharToQuantum((unsigned char)
              ReadBlobByte(image));
          image->matte=MagickFalse;
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
          "  reading image resource blocks - %.20g bytes",(double)
          ((MagickOffsetType) length));
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
  check_background=MagickFalse;
  if ((image_info->number_scenes == 1) && (image_info->scene == 0))
    {
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  read composite only");
      check_background=MagickTrue;
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
          size_t
            quantum;

          unsigned long
            tag;

          /*
            Skip layers & masks.
          */
          quantum=psd_info.version == 1 ? 4UL : 8UL;
          tag=ReadBlobMSBLong(image);
          (void) tag;
          count=ReadBlob(image,4,(unsigned char *) type);
          if ((count == 0) || (LocaleNCompare(type,"8BIM",4) != 0))
            {
              if (DiscardBlobBytes(image,length-quantum-8) == MagickFalse)
                ThrowFileException(exception,CorruptImageError,
                  "UnexpectedEndOfFile",image->filename);
            }
          else
            {
              count=ReadBlob(image,4,(unsigned char *) type);
              if ((count != 0) && (LocaleNCompare(type,"Lr16",4) == 0))
                size=GetPSDSize(&psd_info,image);
              else
                if (DiscardBlobBytes(image,length-quantum-12) == MagickFalse)
                  ThrowFileException(exception,CorruptImageError,
                    "UnexpectedEndOfFile",image->filename);
            }
        }
      if (size != 0)
        {
          MagickOffsetType
            layer_offset;

          image->matte=psd_info.channels > psd_info.color_channels ?
            MagickTrue : MagickFalse;
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              image->matte ? "  image has matte" : "  image has no matte");
          layer_offset=offset+length;
          number_layers=(short) ReadBlobMSBShort(image);
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  image contains %.20g layers",(double) number_layers);
          if (number_layers < 0)
            {
              /*
                Weird hack in PSD format to ignore first alpha channel.
              */
              skip_first_alpha=1;
              (void) skip_first_alpha;
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
            int
              x,
              y;

            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  reading layer #%.20g",(double) i+1);
            layer_info[i].page.y=(int) ReadBlobMSBLong(image);
            layer_info[i].page.x=(int) ReadBlobMSBLong(image);
            y=(int) ReadBlobMSBLong(image);
            x=(int) ReadBlobMSBLong(image);
            layer_info[i].page.width=(ssize_t) (x-layer_info[i].page.x);
            layer_info[i].page.height=(ssize_t) (y-layer_info[i].page.y);
            layer_info[i].channels=ReadBlobMSBShort(image);
            if (check_background == MagickTrue)
              {
                size_t
                  quantum;

                if (layer_info[i].channels == psd_info.color_channels)
                  image->matte=MagickFalse;
                quantum=psd_info.version == 1 ? 4UL : 8UL;
                if (DiscardBlobBytes(image,length-20-quantum) == MagickFalse)
                   ThrowFileException(exception,CorruptImageError,
                     "UnexpectedEndOfFile",image->filename);
                break;
              }
            if (layer_info[i].channels > MaxPSDChannels)
              ThrowReaderException(CorruptImageError,"MaximumChannelsExceeded");
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    offset(%.20g,%.20g), size(%.20g,%.20g), channels=%.20g",
                (double) layer_info[i].page.x,(double) layer_info[i].page.y,
                (double) layer_info[i].page.height,(double)
                layer_info[i].page.width,(double) layer_info[i].channels);
            for (j=0; j < (ssize_t) layer_info[i].channels; j++)
            {
              layer_info[i].channel_info[j].type=(short)
                ReadBlobMSBShort(image);
              layer_info[i].channel_info[j].size=(size_t)
                GetPSDSize(&psd_info,image);
              if (image->debug != MagickFalse)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    channel[%.20g]: type=%.20g, size=%.20g",(double) j,
                  (double) layer_info[i].channel_info[j].type,
                  (double) layer_info[i].channel_info[j].size);
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
                "   blend=%.4s, opacity=%.20g, clipping=%s, flags=%d, visible=%s",
                layer_info[i].blendkey,(double) layer_info[i].opacity,
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
                    layer_info[i].mask.y=(int) ReadBlobMSBLong(image);
                    layer_info[i].mask.x=(int) ReadBlobMSBLong(image);
                    layer_info[i].mask.height=(size_t)
                      (ReadBlobMSBLong(image)-layer_info[i].mask.y);
                    layer_info[i].mask.width=(size_t)
                      (ReadBlobMSBLong(image)-layer_info[i].mask.x);
                    if (image->debug != MagickFalse)
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "      layer mask: offset(%.20g,%.20g), size(%.20g,%.20g), length=%.20g",
                        (double) layer_info[i].mask.x,(double) layer_info[i].mask.y,
                        (double) layer_info[i].mask.width,(double)
                        layer_info[i].mask.height,(double)
                        ((MagickOffsetType) length)-16);
                    /*
                      Skip over the rest of the layer mask information.
                    */
                    if (DiscardBlobBytes(image,length-16) == MagickFalse)
                      ThrowFileException(exception,CorruptImageError,
                        "UnexpectedEndOfFile",image->filename);
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
                        "      layer blending ranges: length=%.20g",(double)
                        ((MagickOffsetType) length));
                    /*
                      We read it, but don't use it...
                    */
                    for (j=0; j < (ssize_t) (length); j+=8)
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
                for (j=0; j < (ssize_t) length; j++)
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
            char  alsig[4], alkey[4];

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
                            "      adjustment layer key: %.4s, data length=%.20g",
                            alkey, (double) length);
              }

              if ( length ) {
              for (j=0; j < (ssize_t) (length); j++)
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
                    "      unsupported data: length=%.20g",(double)
                    ((MagickOffsetType) (size-combinedlength)));
                if (DiscardBlobBytes(image,size-combinedlength) == MagickFalse)
                  ThrowFileException(exception,CorruptImageError,
                    "UnexpectedEndOfFile",image->filename);
              }
            /*
              Allocate layered image.
            */
            layer_info[i].image=CloneImage(image,layer_info[i].page.width,
              layer_info[i].page.height == ~0U ? 1 : layer_info[i].page.height,
              MagickFalse,&image->exception);
            if (layer_info[i].image == (Image *) NULL)
              {
                for (j=0; j < i; j++)
                  layer_info[j].image=DestroyImage(layer_info[j].image);
                if (image->debug != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  allocation of image for layer %.20g failed",(double) i);
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    setting up new layer image");
            if (image_info->ping != MagickFalse)
              (void) SetImageBackgroundColor(layer_info[i].image);
            layer_info[i].image->compose=
              PSDBlendModeToCompositeOperator(layer_info[i].blendkey);
            if (layer_info[i].visible == MagickFalse)
              layer_info[i].image->compose=NoCompositeOp;
            if (psd_info.mode == CMYKMode)
              SetImageColorspace(layer_info[i].image,CMYKColorspace);
            if ((psd_info.mode == BitmapMode) ||
                (psd_info.mode == GrayscaleMode) ||
                (psd_info.mode == DuotoneMode))
              SetImageColorspace(layer_info[i].image,GRAYColorspace);
            for (j=0; j < (ssize_t) layer_info[i].channels; j++)
              if (layer_info[i].channel_info[j].type == -1)
                layer_info[i].image->matte=MagickTrue;
            /*
              Set up some hidden attributes for folks that need them.
            */
            (void) FormatLocaleString(message,MaxTextExtent,"%.20gld",
              (double) layer_info[i].page.x);
            (void) SetImageArtifact(layer_info[i].image,"psd:layer.x",message);
            (void) FormatLocaleString(message,MaxTextExtent,"%.20g",
              (double) layer_info[i].page.y);
            (void) SetImageArtifact(layer_info[i].image,"psd:layer.y",message);
            (void) FormatLocaleString(message,MaxTextExtent,"%.20g",
              (double) layer_info[i].opacity);
            (void) SetImageArtifact(layer_info[i].image,"psd:layer.opacity",
              message);
            (void) SetImageProperty(layer_info[i].image,"label",(char *)
              layer_info[i].name);
          }
        if (check_background == MagickFalse)
          {
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
                  "  reading data for layer %.20g",(double) i);
                for (j=0; j < (ssize_t) layer_info[i].channels; j++)
                {
                  if (image->debug != MagickFalse)
                    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                      "    reading data for channel %.20g",(double) j);
#if 1
                  if (layer_info[i].channel_info[j].size <= (2*layer_info[i].image->rows))
                    {
                      ssize_t
                        k;

                      if (image->debug != MagickFalse)
                        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "      layer data is empty");
                      /*
                        A layer without data.
                      */
                      for (k=0; k < (ssize_t) layer_info[i].channel_info[j].size; k++)
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
                          for (y=0; y < (ssize_t) layer_info[i].image->rows; y++)
                            offsets[y]=GetPSDOffset(&psd_info,
                              layer_info[i].image);
                        }
                      status=ReadPSDLayer(layer_info[i].image,
                        layer_info[i].channels,
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
                    for (y=0; y < (ssize_t) layer_info[i].image->rows; y++)
                    {
                      q=GetAuthenticPixels(layer_info[i].image,0,y,
                        layer_info[i].image->columns,1,exception);
                      if (q == (PixelPacket *) NULL)
                        break;
                      for (x=0; x < (ssize_t) layer_info[i].image->columns; x++)
                      {
                        q->opacity=(Quantum) (QuantumRange-(Quantum)
                          (QuantumScale*((QuantumRange-q->opacity)*
                          (QuantumRange-layer_info[i].opacity))));
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
               invisible I consider that all layer with width and height null
               are layer for group layer */
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
           /* I consider that all layer with width and height null are layer
              for group layer */
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
            mask_size = ReadBlobMSBLong(image);  /* ignore global mask size */
            (void) mask_size;
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
      for (i=0; i < (ssize_t) (image->rows*psd_info.channels); i++)
        offsets[i]=GetPSDOffset(&psd_info,image);
    }
  for (i=0; i < (ssize_t) psd_info.channels; i++)
  {
    status=ReadPSDLayer(image,psd_info.channels,i,offsets+i*image->rows,
      exception);
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
%      size_t RegisterPSDImage(void)
%
*/
ModuleExport size_t RegisterPSDImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("PSB");
  entry->decoder=(DecodeImageHandler *) ReadPSDImage;
  entry->encoder=(EncodeImageHandler *) WritePSDImage;
  entry->magick=(IsImageFormatHandler *) IsPSD;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Adobe Large Document Format");
  entry->module=ConstantString("PSD");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PSD");
  entry->decoder=(DecodeImageHandler *) ReadPSDImage;
  entry->encoder=(EncodeImageHandler *) WritePSDImage;
  entry->magick=(IsImageFormatHandler *) IsPSD;
  entry->seekable_stream=MagickTrue;
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

  register ssize_t
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
  for (i=(ssize_t) length; i != 0; )
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
            while (((ssize_t) count < i) && (*pixels == *(pixels+count)))
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
          if (((ssize_t) count >= (i-3)) || (count >= 127))
            break;
        }
        i-=count;
        *packbits=(unsigned char) (count-1);
        for (j=0; j <= (ssize_t) count; j++)
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
  const ImageInfo *image_info,Image *image,Image *next_image,
  unsigned char *compact_pixels,const QuantumType quantum_type)
{
  QuantumInfo
    *quantum_info;

  register const PixelPacket
    *p;

  size_t
    length,
    packet_size;

  ssize_t
    y;

  unsigned char
    *pixels;

  if (next_image->depth > 8)
    next_image->depth=16;
  packet_size=next_image->depth > 8UL ? 2UL : 1UL;
  (void) packet_size;
  quantum_info=AcquireQuantumInfo(image_info,image);
  pixels=GetQuantumPixels(quantum_info);
  for (y=0; y < (ssize_t) next_image->rows; y++)
  {
    p=GetVirtualPixels(next_image,0,y,next_image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    length=ExportQuantumPixels(next_image,(CacheView *) NULL,quantum_info,
      quantum_type,pixels,&image->exception);
    length=PSDPackbitsEncodeImage(image,length,pixels,compact_pixels);
    (void) SetPSDOffset(psd_info,image,length);
  }
  quantum_info=DestroyQuantumInfo(quantum_info);
}

static void WriteOneChannel(const PSDInfo *psd_info,const ImageInfo *image_info,
  Image *image,Image *next_image,unsigned char *compact_pixels,
  const QuantumType quantum_type,const MagickBooleanType compression_flag)
{
  int
    y;

  MagickBooleanType
    monochrome;

  QuantumInfo
    *quantum_info;

  register const PixelPacket
    *p;

  register ssize_t
    i;

  size_t
    length,
    packet_size;

  unsigned char
    *pixels;

  (void) psd_info;
  if ((compression_flag != MagickFalse) &&
      (next_image->compression != RLECompression))
    (void) WriteBlobMSBShort(image,0);
  if (next_image->depth > 8)
    next_image->depth=16;
  monochrome=IsMonochromeImage(image,&image->exception) && (image->depth == 1)
    ? MagickTrue : MagickFalse;
  packet_size=next_image->depth > 8UL ? 2UL : 1UL;
  (void) packet_size;
  quantum_info=AcquireQuantumInfo(image_info,image);
  pixels=GetQuantumPixels(quantum_info);
  for (y=0; y < (ssize_t) next_image->rows; y++)
  {
    p=GetVirtualPixels(next_image,0,y,next_image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    length=ExportQuantumPixels(next_image,(CacheView *) NULL,quantum_info,
      quantum_type,pixels,&image->exception);
    if (monochrome != MagickFalse)
      for (i=0; i < (ssize_t) length; i++)
        pixels[i]=(~pixels[i]);
    if (next_image->compression != RLECompression)
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
  const ImageInfo *image_info,Image *image,Image *next_image,
  const MagickBooleanType separate)
{
  int
    i;

  size_t
    channels,
    packet_size;

  unsigned char
    *compact_pixels;

  /*
    Write uncompressed pixels as separate planes.
  */
  channels=1;
  packet_size=next_image->depth > 8UL ? 2UL : 1UL;
  compact_pixels=(unsigned char *) NULL;
  if (next_image->compression == RLECompression)
    {
      compact_pixels=(unsigned char *) AcquireQuantumMemory(2*channels*
        next_image->columns,packet_size*sizeof(*compact_pixels));
      if (compact_pixels == (unsigned char *) NULL)
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  i=0;
  if (IsGrayImage(next_image,&next_image->exception) != MagickFalse)
    {
      if (next_image->compression == RLECompression)
        {
          /*
            Packbits compression.
          */
          (void) WriteBlobMSBShort(image,1);
          WritePackbitsLength(psd_info,image_info,image,next_image,
            compact_pixels,GrayQuantum);
          if (next_image->matte != MagickFalse)
            WritePackbitsLength(psd_info,image_info,image,next_image,
              compact_pixels,AlphaQuantum);
        }
      WriteOneChannel(psd_info,image_info,image,next_image,compact_pixels,
        GrayQuantum,(i++ == 0) || (separate != MagickFalse) ? MagickTrue :
        MagickFalse);
      if (next_image->matte != MagickFalse)
        WriteOneChannel(psd_info,image_info,image,next_image,compact_pixels,
          AlphaQuantum,(i++ == 0) || (separate != MagickFalse) ? MagickTrue :
          MagickFalse);
      (void) SetImageProgress(image,SaveImagesTag,0,1);
    }
  else
    if (next_image->storage_class == PseudoClass)
      {
        if (next_image->compression == RLECompression)
          {
            /*
              Packbits compression.
            */
            (void) WriteBlobMSBShort(image,1);
            WritePackbitsLength(psd_info,image_info,image,next_image,
              compact_pixels,IndexQuantum);
            if (next_image->matte != MagickFalse)
              WritePackbitsLength(psd_info,image_info,image,next_image,
                compact_pixels,AlphaQuantum);
          }
        WriteOneChannel(psd_info,image_info,image,next_image,compact_pixels,
          IndexQuantum,(i++ == 0) || (separate != MagickFalse) ? MagickTrue :
          MagickFalse);
        if (next_image->matte != MagickFalse)
          WriteOneChannel(psd_info,image_info,image,next_image,compact_pixels,
            AlphaQuantum,(i++ == 0) || (separate != MagickFalse) ? MagickTrue :
            MagickFalse);
        (void) SetImageProgress(image,SaveImagesTag,0,1);
      }
    else
      {
        if (next_image->colorspace == CMYKColorspace)
          (void) NegateImage(next_image,MagickFalse);
        if (next_image->compression == RLECompression)
          {
            /*
              Packbits compression.
            */
            (void) WriteBlobMSBShort(image,1);
            WritePackbitsLength(psd_info,image_info,image,next_image,
              compact_pixels,RedQuantum);
            WritePackbitsLength(psd_info,image_info,image,next_image,
              compact_pixels,GreenQuantum);
            WritePackbitsLength(psd_info,image_info,image,next_image,
              compact_pixels,BlueQuantum);
            if (next_image->colorspace == CMYKColorspace)
              WritePackbitsLength(psd_info,image_info,image,next_image,
                compact_pixels,BlackQuantum);
            if (next_image->matte != MagickFalse)
              WritePackbitsLength(psd_info,image_info,image,next_image,
                compact_pixels,AlphaQuantum);
          }
        (void) SetImageProgress(image,SaveImagesTag,0,6);
        WriteOneChannel(psd_info,image_info,image,next_image,compact_pixels,
          RedQuantum,(i++ == 0) || (separate != MagickFalse) ? MagickTrue :
          MagickFalse);
        (void) SetImageProgress(image,SaveImagesTag,1,6);
        WriteOneChannel(psd_info,image_info,image,next_image,compact_pixels,
          GreenQuantum,(i++ == 0) || (separate != MagickFalse) ? MagickTrue :
          MagickFalse);
        (void) SetImageProgress(image,SaveImagesTag,2,6);
        WriteOneChannel(psd_info,image_info,image,next_image,compact_pixels,
          BlueQuantum,(i++ == 0) || (separate != MagickFalse) ? MagickTrue :
          MagickFalse);
        (void) SetImageProgress(image,SaveImagesTag,3,6);
        if (next_image->colorspace == CMYKColorspace)
          WriteOneChannel(psd_info,image_info,image,next_image,compact_pixels,
            BlackQuantum,(i++ == 0) || (separate != MagickFalse) ? MagickTrue :
            MagickFalse);
        (void) SetImageProgress(image,SaveImagesTag,4,6);
        if (next_image->matte != MagickFalse)
          WriteOneChannel(psd_info,image_info,image,next_image,compact_pixels,
            AlphaQuantum,(i++ == 0) || (separate != MagickFalse) ? MagickTrue :
            MagickFalse);
        (void) SetImageProgress(image,SaveImagesTag,5,6);
        if (next_image->colorspace == CMYKColorspace)
          (void) NegateImage(next_image,MagickFalse);
      }
  if (next_image->compression == RLECompression)
    compact_pixels=(unsigned char *) RelinquishMagickMemory(compact_pixels);
  return(MagickTrue);
}

static void WritePascalString(Image* inImage,const char *inString,int inPad)
{
  size_t
    length;

  register ssize_t
    i;

  /*
    Max length is 255.
  */
  length=(strlen(inString) > 255UL ) ? 255UL : strlen(inString);
  if (length ==  0)
    (void) WriteBlobByte(inImage,0);
  else
    {
      (void) WriteBlobByte(inImage,(unsigned char) length);
      (void) WriteBlob(inImage, length, (const unsigned char *) inString);
    }
  length++;
  if ((length % inPad) == 0)
    return;
  for (i=0; i < (ssize_t) (inPad-(length % inPad)); i++)
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

static void RemoveICCProfileFromResourceBlock(StringInfo *bim_profile)
{
  register const unsigned char
    *p;

  size_t
    length;

  unsigned char
    *datum;

  unsigned int
    count,
    long_sans;

  unsigned short
    id,
    short_sans;

  length=GetStringInfoLength(bim_profile);
  if (length < 16)
    return;
  datum=GetStringInfoDatum(bim_profile);
  for (p=datum; (p >= datum) && (p < (datum+length-16)); )
  {
    register unsigned char
      *q;

    q=(unsigned char *) p;
    if (LocaleNCompare((const char *) p,"8BIM",4) != 0)
      break;
    p=PushLongPixel(MSBEndian,p,&long_sans);
    p=PushShortPixel(MSBEndian,p,&id);
    p=PushShortPixel(MSBEndian,p,&short_sans);
    p=PushLongPixel(MSBEndian,p,&count);
    if (id == 0x0000040f)
      {
        (void) CopyMagickMemory(q,q+PSDQuantum(count)+12,length-
          (PSDQuantum(count)+12)-(q-datum));
        SetStringInfoLength(bim_profile,length-(PSDQuantum(count)+12));
        break;
      }
    p+=count;
    if ((count & 0x01) != 0)
      p++;
  }
}

static void RemoveResolutionFromResourceBlock(StringInfo *bim_profile)
{
  register const unsigned char
    *p;

  size_t
    length;

  unsigned char
    *datum;

  unsigned int
    count,
    long_sans;

  unsigned short
    id,
    short_sans;

  length=GetStringInfoLength(bim_profile);
  if (length < 16)
    return;
  datum=GetStringInfoDatum(bim_profile);
  for (p=datum; (p >= datum) && (p < (datum+length-16)); )
  {
    register unsigned char
      *q;

    q=(unsigned char *) p;
    if (LocaleNCompare((const char *) p,"8BIM",4) != 0)
      break;
    p=PushLongPixel(MSBEndian,p,&long_sans);
    p=PushShortPixel(MSBEndian,p,&id);
    p=PushShortPixel(MSBEndian,p,&short_sans);
    p=PushLongPixel(MSBEndian,p,&count);
    if ((id == 0x000003ed) && (PSDQuantum(count) < (ssize_t) (length-12)))
      {
        (void) CopyMagickMemory(q,q+PSDQuantum(count)+12,length-
          (PSDQuantum(count)+12)-(q-datum));
        SetStringInfoLength(bim_profile,length-(PSDQuantum(count)+12));
        break;
      }
    p+=count;
    if ((count & 0x01) != 0)
      p++;
  }
}

static MagickBooleanType WritePSDImage(const ImageInfo *image_info,Image *image)
{
  const char
    *property;

  const StringInfo
    *icc_profile;

  Image
    *base_image,
    *next_image;

  MagickBooleanType
    status;

  PSDInfo
    psd_info;

  register ssize_t
    i;

  size_t
    channel_size,
    channelLength,
    layer_count,
    layer_info_size,
    length,
    num_channels,
    packet_size,
    rounded_layer_info_size;

  StringInfo
    *bim_profile;

  unsigned char
    layer_name[4];

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
    num_channels=(image->matte != MagickFalse ? 2UL : 1UL);
  else
    if (image->storage_class == PseudoClass)
      num_channels=(image->matte != MagickFalse ? 2UL : 1UL);
    else
      {
        if (image->colorspace != CMYKColorspace)
          num_channels=(image->matte != MagickFalse ? 4UL : 3UL);
        else
          num_channels=(image->matte != MagickFalse ? 5UL : 4UL);
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
      monochrome=IsMonochromeImage(image,&image->exception) &&
        (image->depth == 1) ? MagickTrue : MagickFalse;
      (void) WriteBlobMSBShort(image,(unsigned short)
        (monochrome != MagickFalse ? 1 : image->depth > 8 ? 16 : 8));
      (void) WriteBlobMSBShort(image,(unsigned short)
        (monochrome != MagickFalse ? BitmapMode : GrayscaleMode));
    }
  else
    {
      (void) WriteBlobMSBShort(image,(unsigned short) (image->storage_class ==
        PseudoClass ? 8 : image->depth > 8 ? 16 : 8));
      if (((image_info->colorspace != UndefinedColorspace) ||
           (image->colorspace != CMYKColorspace)) &&
          (image_info->colorspace != CMYKColorspace))
        {
          if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
            (void) TransformImageColorspace(image,sRGBColorspace);
          (void) WriteBlobMSBShort(image,(unsigned short)
            (image->storage_class == PseudoClass ? IndexedMode : RGBMode));
        }
      else
        {
          if (image->colorspace != CMYKColorspace)
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
      for (i=0; i < (ssize_t) image->colors; i++)
        (void) WriteBlobByte(image,ScaleQuantumToChar(image->colormap[i].red));
      for ( ; i < 256; i++)
        (void) WriteBlobByte(image,0);
      for (i=0; i < (ssize_t) image->colors; i++)
        (void) WriteBlobByte(image,ScaleQuantumToChar(
          image->colormap[i].green));
      for ( ; i < 256; i++)
        (void) WriteBlobByte(image,0);
      for (i=0; i < (ssize_t) image->colors; i++)
        (void) WriteBlobByte(image,ScaleQuantumToChar(image->colormap[i].blue));
      for ( ; i < 256; i++)
        (void) WriteBlobByte(image,0);
    }
  /*
    Image resource block.
  */
  length=28; /* 0x03EB */
  bim_profile=(StringInfo *) GetImageProfile(image,"8bim");
  icc_profile=GetImageProfile(image,"icc");
  if (bim_profile != (StringInfo *) NULL)
    {
      bim_profile=CloneStringInfo(bim_profile);
      if (icc_profile != (StringInfo *) NULL)
        RemoveICCProfileFromResourceBlock(bim_profile);
      RemoveResolutionFromResourceBlock(bim_profile);
      length+=PSDQuantum(GetStringInfoLength(bim_profile));
    }
  if (icc_profile != (const StringInfo *) NULL)
    length+=PSDQuantum(GetStringInfoLength(icc_profile))+12;
  (void) WriteBlobMSBLong(image,(unsigned int) length);
  WriteResolutionResourceBlock(image);
  if (bim_profile != (StringInfo *) NULL)
    {
      (void) WriteBlob(image,GetStringInfoLength(bim_profile),
        GetStringInfoDatum(bim_profile));
      bim_profile=DestroyStringInfo(bim_profile);
    }
  if (icc_profile != (StringInfo *) NULL)
    {
      (void) WriteBlob(image,4,(const unsigned char *) "8BIM");
      (void) WriteBlobMSBShort(image,0x0000040F);
      (void) WriteBlobMSBShort(image,0);
      (void) WriteBlobMSBLong(image,(unsigned int) GetStringInfoLength(
        icc_profile));
      (void) WriteBlob(image,GetStringInfoLength(icc_profile),
        GetStringInfoDatum(icc_profile));
      if ((MagickOffsetType) GetStringInfoLength(icc_profile) !=
          PSDQuantum(GetStringInfoLength(icc_profile)))
        (void) WriteBlobByte(image,0);
    }
  layer_count=0;
  layer_info_size=2;
  base_image=GetNextImageInList(image);
  if ((image->matte != MagickFalse) && (base_image == (Image *) NULL))
    base_image=image;
  next_image=base_image;
  while ( next_image != NULL )
  {
    packet_size=next_image->depth > 8 ? 2UL : 1UL;
    if (IsGrayImage(next_image,&image->exception) != MagickFalse)
      num_channels=next_image->matte != MagickFalse ? 2UL : 1UL;
    else
      if (next_image->storage_class == PseudoClass)
        num_channels=next_image->matte != MagickFalse ? 2UL : 1UL;
      else
        if (next_image->colorspace != CMYKColorspace)
          num_channels=next_image->matte != MagickFalse ? 4UL : 3UL;
        else
          num_channels=next_image->matte != MagickFalse ? 5UL : 4UL;
    channelLength=(size_t) (next_image->columns*next_image->rows*packet_size+2);
    layer_info_size+=(size_t) (4*4+2+num_channels*6+(psd_info.version == 1 ? 8 :
      16)+4*1+4+num_channels*channelLength);
    property=(const char *) GetImageProperty(next_image,"label");
    if (property == (const char *) NULL)
      layer_info_size+=16;
    else
      {
        size_t
          length;

        length=strlen(property);
        layer_info_size+=8+length+(4-(length % 4));
      }
    layer_count++;
    next_image=GetNextImageInList(next_image);
  }
  if (layer_count == 0)
    (void) SetPSDSize(&psd_info,image,0);
  else
    {
      CompressionType
        compression;

      (void) SetPSDSize(&psd_info,image,layer_info_size+
        (psd_info.version == 1 ? 8 : 16));
      if ((layer_info_size/2) != ((layer_info_size+1)/2))
        rounded_layer_info_size=layer_info_size+1;
      else
        rounded_layer_info_size=layer_info_size;
      (void) SetPSDSize(&psd_info,image,rounded_layer_info_size);
      (void) WriteBlobMSBShort(image,(unsigned short) layer_count);
      layer_count=1;
      compression=base_image->compression;
      next_image=base_image;
      while (next_image != NULL)
      {
        next_image->compression=NoCompression;
        (void) WriteBlobMSBLong(image,(unsigned int) next_image->page.y);
        (void) WriteBlobMSBLong(image,(unsigned int) next_image->page.x);
        (void) WriteBlobMSBLong(image,(unsigned int) next_image->page.y+
          next_image->rows);
        (void) WriteBlobMSBLong(image,(unsigned int) next_image->page.x+
          next_image->columns);
        packet_size=next_image->depth > 8 ? 2UL : 1UL;
        channel_size=(unsigned int) ((packet_size*next_image->rows*
          next_image->columns)+2);
        if ((IsGrayImage(next_image,&image->exception) != MagickFalse) ||
            (next_image->storage_class == PseudoClass))
          {
             (void) WriteBlobMSBShort(image,(unsigned short)
               (next_image->matte != MagickFalse ? 2 : 1));
             (void) WriteBlobMSBShort(image,0);
             (void) SetPSDSize(&psd_info,image,channel_size);
             if (next_image->matte != MagickFalse)
               {
                 (void) WriteBlobMSBShort(image,(unsigned short) -1);
                 (void) SetPSDSize(&psd_info,image,channel_size);
               }
           }
          else
            if (next_image->colorspace != CMYKColorspace)
              {
                (void) WriteBlobMSBShort(image,(unsigned short)
                  (next_image->matte != MagickFalse ? 4 : 3));
               (void) WriteBlobMSBShort(image,0);
               (void) SetPSDSize(&psd_info,image,channel_size);
               (void) WriteBlobMSBShort(image,1);
               (void) SetPSDSize(&psd_info,image,channel_size);
               (void) WriteBlobMSBShort(image,2);
               (void) SetPSDSize(&psd_info,image,channel_size);
               if (next_image->matte!= MagickFalse )
                 {
                   (void) WriteBlobMSBShort(image,(unsigned short) -1);
                   (void) SetPSDSize(&psd_info,image,channel_size);
                 }
             }
           else
             {
               (void) WriteBlobMSBShort(image,(unsigned short)
                 (next_image->matte ? 5 : 4));
               (void) WriteBlobMSBShort(image,0);
               (void) SetPSDSize(&psd_info,image,channel_size);
               (void) WriteBlobMSBShort(image,1);
               (void) SetPSDSize(&psd_info,image,channel_size);
               (void) WriteBlobMSBShort(image,2);
               (void) SetPSDSize(&psd_info,image,channel_size);
               (void) WriteBlobMSBShort(image,3);
               (void) SetPSDSize(&psd_info,image,channel_size);
               if (next_image->matte)
                 {
                   (void) WriteBlobMSBShort(image,(unsigned short) -1);
                   (void) SetPSDSize(&psd_info,image,channel_size);
                 }
             }
        (void) WriteBlob(image,4,(const unsigned char *) "8BIM");
        (void) WriteBlob(image,4,(const unsigned char *)
          CompositeOperatorToPSDBlendMode(next_image->compose));
        (void) WriteBlobByte(image,255); /* layer opacity */
        (void) WriteBlobByte(image,0);
        (void) WriteBlobByte(image,1); /* layer propertys - visible, etc. */
        (void) WriteBlobByte(image,0);
        property=(const char *) GetImageProperty(next_image,"label");
        if (property == (const char *) NULL)
          {
            (void) WriteBlobMSBLong(image,16);
            (void) WriteBlobMSBLong(image,0);
            (void) WriteBlobMSBLong(image,0);
            (void) FormatLocaleString((char *) layer_name,MaxTextExtent,
              "L%06ld",(long) layer_count++);
            WritePascalString( image, (char*)layer_name, 4 );
          }
        else
          {
            size_t
              length;

            length=strlen(property);
            (void) WriteBlobMSBLong(image,(unsigned int) (length+(4-
              (length % 4))+8));
            (void) WriteBlobMSBLong(image,0);
            (void) WriteBlobMSBLong(image,0);
            WritePascalString(image,property,4);
          }
        next_image=GetNextImageInList(next_image);
      }
      /*
        Now the image data!
      */
      next_image=base_image;
      while (next_image != NULL)
      {
        status=WriteImageChannels(&psd_info,image_info,image,next_image,
          MagickTrue);
        next_image=GetNextImageInList(next_image);
      }
      (void) WriteBlobMSBLong(image,0);  /* user mask data */
      base_image->compression=compression;
    }
  /*
    Write composite image.
  */
  status=WriteImageChannels(&psd_info,image_info,image,image,MagickFalse);
  (void) CloseBlob(image);
  return(status);
}

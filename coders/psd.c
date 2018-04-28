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
%                                   Cristy                                    %
%                              Leonard Rosenthol                              %
%                                 July 1992                                   %
%                                Dirk Lemstra                                 %
%                                December 2013                                %
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
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/channel.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/policy.h"
#include "MagickCore/profile.h"
#include "MagickCore/property.h"
#include "MagickCore/registry.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#ifdef MAGICKCORE_ZLIB_DELEGATE
#include <zlib.h>
#endif
#include "psd-private.h"

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
  Raw = 0,
  RLE = 1,
  ZipWithoutPrediction = 2,
  ZipWithPrediction = 3
} PSDCompressionType;

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
  short
    type;

  size_t
    size;
} ChannelInfo;

typedef struct _MaskInfo
{
  Image
    *image;

  RectangleInfo
    page;

  unsigned char
    background,
    flags;
} MaskInfo;

typedef struct _LayerInfo
{
  ChannelInfo
    channel_info[MaxPSDChannels];

  char
    blendkey[4];

  Image
    *image;

  MaskInfo
    mask;

  Quantum
    opacity;

  RectangleInfo
    page;

  size_t
    offset_x,
    offset_y;

  unsigned char
    clipping,
    flags,
    name[257],
    visible;

  unsigned short
    channels;

  StringInfo
    *info;
} LayerInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePSDImage(const ImageInfo *,Image *,ExceptionInfo *);

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
%      Image *ReadPSDImage(image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static const char *CompositeOperatorToPSDBlendMode(Image *image)
{
  switch (image->compose)
  {
    case ColorBurnCompositeOp:
      return(image->endian == LSBEndian ? "vidi" : "idiv");
    case ColorDodgeCompositeOp:
      return(image->endian == LSBEndian ? " vid" : "div ");
    case ColorizeCompositeOp:
      return(image->endian == LSBEndian ? "rloc" : "colr");
    case DarkenCompositeOp:
      return(image->endian == LSBEndian ? "krad" : "dark");
    case DifferenceCompositeOp:
      return(image->endian == LSBEndian ? "ffid" : "diff");
    case DissolveCompositeOp:
      return(image->endian == LSBEndian ? "ssid" : "diss");
    case ExclusionCompositeOp:
      return(image->endian == LSBEndian ? "dums" : "smud");
    case HardLightCompositeOp:
      return(image->endian == LSBEndian ? "tiLh" : "hLit");
    case HardMixCompositeOp:
      return(image->endian == LSBEndian ? "xiMh" : "hMix");
    case HueCompositeOp:
      return(image->endian == LSBEndian ? " euh" : "hue ");
    case LightenCompositeOp:
      return(image->endian == LSBEndian ? "etil" : "lite");
    case LinearBurnCompositeOp:
      return(image->endian == LSBEndian ? "nrbl" : "lbrn");
    case LinearDodgeCompositeOp:
      return(image->endian == LSBEndian ? "gddl" : "lddg");
    case LinearLightCompositeOp:
      return(image->endian == LSBEndian ? "tiLl" : "lLit");
    case LuminizeCompositeOp:
      return(image->endian == LSBEndian ? " mul" : "lum ");
    case MultiplyCompositeOp:
      return(image->endian == LSBEndian ? " lum" : "mul ");
    case OverlayCompositeOp:
      return(image->endian == LSBEndian ? "revo" : "over");
    case PinLightCompositeOp:
      return(image->endian == LSBEndian ? "tiLp" : "pLit");
    case SaturateCompositeOp:
      return(image->endian == LSBEndian ? " tas" : "sat ");
    case ScreenCompositeOp:
      return(image->endian == LSBEndian ? "nrcs" : "scrn");
    case SoftLightCompositeOp:
      return(image->endian == LSBEndian ? "tiLs" : "sLit");
    case VividLightCompositeOp:
      return(image->endian == LSBEndian ? "tiLv" : "vLit");
    case OverCompositeOp:
    default:
      return(image->endian == LSBEndian ? "mron" : "norm");
  }
}

/*
  For some reason Photoshop seems to blend semi-transparent pixels with white.
  This method reverts the blending. This can be disabled by setting the
  option 'psd:alpha-unblend' to off.
*/
static MagickBooleanType CorrectPSDAlphaBlend(const ImageInfo *image_info,
  Image *image,ExceptionInfo* exception)
{
  const char
    *option;

  MagickBooleanType
    status;

  ssize_t
    y;

  if (image->alpha_trait != BlendPixelTrait || image->colorspace != sRGBColorspace)
    return(MagickTrue);
  option=GetImageOption(image_info,"psd:alpha-unblend");
  if (IsStringFalse(option) != MagickFalse)
    return(MagickTrue);
  status=MagickTrue;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
#pragma omp parallel for schedule(static) shared(status) \
  magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
    {
      status=MagickFalse;
      continue;
    }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        gamma;

      register ssize_t
        i;

      gamma=QuantumScale*GetPixelAlpha(image, q);
      if (gamma != 0.0 && gamma != 1.0)
        {
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            if (channel != AlphaPixelChannel)
              q[i]=ClampToQuantum((q[i]-((1.0-gamma)*QuantumRange))/gamma);
          }
        }
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      status=MagickFalse;
  }

  return(status);
}

static inline CompressionType ConvertPSDCompression(
  PSDCompressionType compression)
{
  switch (compression)
  {
    case RLE:
      return RLECompression;
    case ZipWithPrediction:
    case ZipWithoutPrediction:
      return ZipCompression;
    default:
      return NoCompression;
  }
}

static MagickBooleanType ApplyPSDLayerOpacity(Image *image,Quantum opacity,
  MagickBooleanType revert,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  ssize_t
    y;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  applying layer opacity %.20g", (double) opacity);
  if (opacity == OpaqueAlpha)
    return(MagickTrue);
  if (image->alpha_trait != BlendPixelTrait)
    (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
  status=MagickTrue;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
#pragma omp parallel for schedule(static) shared(status) \
  magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (revert == MagickFalse)
        SetPixelAlpha(image,(Quantum) (QuantumScale*(GetPixelAlpha(image,q))*
          opacity),q);
      else if (opacity > 0)
        SetPixelAlpha(image,(Quantum) (QuantumRange*(GetPixelAlpha(image,q)/
          (MagickRealType) opacity)),q);
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      status=MagickFalse;
  }

  return(status);
}

static MagickBooleanType ApplyPSDOpacityMask(Image *image,const Image *mask,
  Quantum background,MagickBooleanType revert,ExceptionInfo *exception)
{
  Image
    *complete_mask;

  MagickBooleanType
    status;

  PixelInfo
    color;

  ssize_t
    y;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  applying opacity mask");
  complete_mask=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (complete_mask == (Image *) NULL)
    return(MagickFalse);
  complete_mask->alpha_trait=BlendPixelTrait;
  GetPixelInfo(complete_mask,&color);
  color.red=background;
  SetImageColor(complete_mask,&color,exception);
  status=CompositeImage(complete_mask,mask,OverCompositeOp,MagickTrue,
    mask->page.x-image->page.x,mask->page.y-image->page.y,exception);
  if (status == MagickFalse)
    {
      complete_mask=DestroyImage(complete_mask);
      return(status);
    }
  image->alpha_trait=BlendPixelTrait;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
#pragma omp parallel for schedule(static) shared(status) \
  magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *magick_restrict q;

    register Quantum
      *p;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    p=GetAuthenticPixels(complete_mask,0,y,complete_mask->columns,1,exception);
    if ((q == (Quantum *) NULL) || (p == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickRealType
        alpha,
        intensity;

      alpha=GetPixelAlpha(image,q);
      intensity=GetPixelIntensity(complete_mask,p);
      if (revert == MagickFalse)
        SetPixelAlpha(image,ClampToQuantum(intensity*(QuantumScale*alpha)),q);
      else if (intensity > 0)
        SetPixelAlpha(image,ClampToQuantum((alpha/intensity)*QuantumRange),q);
      q+=GetPixelChannels(image);
      p+=GetPixelChannels(complete_mask);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      status=MagickFalse;
  }
  complete_mask=DestroyImage(complete_mask);
  return(status);
}

static void PreservePSDOpacityMask(Image *image,LayerInfo* layer_info,
  ExceptionInfo *exception)
{
  char
    *key;

  RandomInfo
    *random_info;

  StringInfo
    *key_info;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  preserving opacity mask");
  random_info=AcquireRandomInfo();
  key_info=GetRandomKey(random_info,2+1);
  key=(char *) GetStringInfoDatum(key_info);
  key[8]=layer_info->mask.background;
  key[9]='\0';
  layer_info->mask.image->page.x+=layer_info->page.x;
  layer_info->mask.image->page.y+=layer_info->page.y;
  (void) SetImageRegistry(ImageRegistryType,(const char *) key,
    layer_info->mask.image,exception);
  (void) SetImageArtifact(layer_info->image,"psd:opacity-mask",
    (const char *) key);
  key_info=DestroyStringInfo(key_info);
  random_info=DestroyRandomInfo(random_info);
}

static ssize_t DecodePSDPixels(const size_t number_compact_pixels,
  const unsigned char *compact_pixels,const ssize_t depth,
  const size_t number_pixels,unsigned char *pixels)
{
#define CheckNumberCompactPixels \
  if (packets == 0) \
    return(i); \
  packets--

#define CheckNumberPixels(count) \
  if (((ssize_t) i + count) > (ssize_t) number_pixels) \
    return(i); \
  i+=count

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
    packets--;
    length=(size_t) (*compact_pixels++);
    if (length == 128)
      continue;
    if (length > 128)
      {
        length=256-length+1;
        CheckNumberCompactPixels;
        pixel=(*compact_pixels++);
        for (j=0; j < (ssize_t) length; j++)
        {
          switch (depth)
          {
            case 1:
            {
              CheckNumberPixels(8);
              *pixels++=(pixel >> 7) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 6) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 5) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 4) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 3) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 2) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 1) & 0x01 ? 0U : 255U;
              *pixels++=(pixel >> 0) & 0x01 ? 0U : 255U;
              break;
            }
            case 2:
            {
              CheckNumberPixels(4);
              *pixels++=(unsigned char) ((pixel >> 6) & 0x03);
              *pixels++=(unsigned char) ((pixel >> 4) & 0x03);
              *pixels++=(unsigned char) ((pixel >> 2) & 0x03);
              *pixels++=(unsigned char) ((pixel & 0x03) & 0x03);
              break;
            }
            case 4:
            {
              CheckNumberPixels(2);
              *pixels++=(unsigned char) ((pixel >> 4) & 0xff);
              *pixels++=(unsigned char) ((pixel & 0x0f) & 0xff);
              break;
            }
            default:
            {
              CheckNumberPixels(1);
              *pixels++=(unsigned char) pixel;
              break;
            }
          }
        }
        continue;
      }
    length++;
    for (j=0; j < (ssize_t) length; j++)
    {
      CheckNumberCompactPixels;
      switch (depth)
      {
        case 1:
        {
          CheckNumberPixels(8);
          *pixels++=(*compact_pixels >> 7) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 6) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 5) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 4) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 3) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 2) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 1) & 0x01 ? 0U : 255U;
          *pixels++=(*compact_pixels >> 0) & 0x01 ? 0U : 255U;
          break;
        }
        case 2:
        {
          CheckNumberPixels(4);
          *pixels++=(*compact_pixels >> 6) & 0x03;
          *pixels++=(*compact_pixels >> 4) & 0x03;
          *pixels++=(*compact_pixels >> 2) & 0x03;
          *pixels++=(*compact_pixels & 0x03) & 0x03;
          break;
        }
        case 4:
        {
          CheckNumberPixels(2);
          *pixels++=(*compact_pixels >> 4) & 0xff;
          *pixels++=(*compact_pixels & 0x0f) & 0xff;
          break;
        }
        default:
        {
          CheckNumberPixels(1);
          *pixels++=(*compact_pixels);
          break;
        }
      }
      compact_pixels++;
    }
  }
  return(i);
}

static inline LayerInfo *DestroyLayerInfo(LayerInfo *layer_info,
  const ssize_t number_layers)
{
  ssize_t
    i;

  for (i=0; i<number_layers; i++)
  {
    if (layer_info[i].image != (Image *) NULL)
      layer_info[i].image=DestroyImage(layer_info[i].image);
    if (layer_info[i].mask.image != (Image *) NULL)
      layer_info[i].mask.image=DestroyImage(layer_info[i].mask.image);
    if (layer_info[i].info != (StringInfo *) NULL)
      layer_info[i].info=DestroyStringInfo(layer_info[i].info);
  }

  return (LayerInfo *) RelinquishMagickMemory(layer_info);
}

static inline size_t GetPSDPacketSize(const Image *image)
{
  if (image->storage_class == PseudoClass)
    {
      if (image->colors > 256)
        return(2);
    }
  if (image->depth > 16)
    return(4);
  if (image->depth > 8)
    return(2);

  return(1);
}

static inline MagickSizeType GetPSDSize(const PSDInfo *psd_info,Image *image)
{
  if (psd_info->version == 1)
    return((MagickSizeType) ReadBlobLong(image));
  return((MagickSizeType) ReadBlobLongLong(image));
}

static inline size_t GetPSDRowSize(Image *image)
{
  if (image->depth == 1)
    return(((image->columns+7)/8)*GetPSDPacketSize(image));
  else
    return(image->columns*GetPSDPacketSize(image));
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

static MagickBooleanType NegateCMYK(Image *image,ExceptionInfo *exception)
{
  ChannelType
    channel_mask;

  MagickBooleanType
    status;

  channel_mask=SetImageChannelMask(image,(ChannelType)(AllChannels &~
    AlphaChannel));
  status=NegateImage(image,MagickFalse,exception);
  (void) SetImageChannelMask(image,channel_mask);
  return(status);
}

static StringInfo *ParseImageResourceBlocks(Image *image,
  const unsigned char *blocks,size_t length,
  MagickBooleanType *has_merged_image,ExceptionInfo *exception)
{
  const unsigned char
    *p;

  StringInfo
    *profile;

  unsigned char
    name_length;

  unsigned int
    count;

  unsigned short
    id,
    short_sans;

  if (length < 16)
    return((StringInfo *) NULL);
  profile=BlobToStringInfo((const unsigned char *) NULL,length);
  SetStringInfoDatum(profile,blocks);
  SetStringInfoName(profile,"8bim");
  for (p=blocks; (p >= blocks) && (p < (blocks+length-7)); )
  {
    if (LocaleNCompare((const char *) p,"8BIM",4) != 0)
      break;
    p+=4;
    p=PushShortPixel(MSBEndian,p,&id);
    p=PushCharPixel(p,&name_length);
    if ((name_length % 2) == 0)
      name_length++;
    p+=name_length;
    if (p > (blocks+length-4))
      break;
    p=PushLongPixel(MSBEndian,p,&count);
    if ((p+count) > (blocks+length))
      break;
    switch (id)
    {
      case 0x03ed:
      {
        char
          value[MagickPathExtent];

        unsigned short
          resolution;

        /*
          Resolution info.
        */
        if (count < 16)
          break;
        p=PushShortPixel(MSBEndian,p,&resolution);
        image->resolution.x=(double) resolution;
        (void) FormatLocaleString(value,MagickPathExtent,"%g",
          image->resolution.x);
        (void) SetImageProperty(image,"tiff:XResolution",value,exception);
        p=PushShortPixel(MSBEndian,p,&short_sans);
        p=PushShortPixel(MSBEndian,p,&short_sans);
        p=PushShortPixel(MSBEndian,p,&short_sans);
        p=PushShortPixel(MSBEndian,p,&resolution);
        image->resolution.y=(double) resolution;
        (void) FormatLocaleString(value,MagickPathExtent,"%g",
          image->resolution.y);
        (void) SetImageProperty(image,"tiff:YResolution",value,exception);
        p=PushShortPixel(MSBEndian,p,&short_sans);
        p=PushShortPixel(MSBEndian,p,&short_sans);
        p=PushShortPixel(MSBEndian,p,&short_sans);
        image->units=PixelsPerInchResolution;
        break;
      }
      case 0x0421:
      {
        if ((count > 4) && (*(p+4) == 0))
          *has_merged_image=MagickFalse;
        p+=count;
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
  return(profile);
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
    return(HardLightCompositeOp);
  if (LocaleNCompare(mode,"sLit",4) == 0)
    return(SoftLightCompositeOp);
  if (LocaleNCompare(mode,"smud",4) == 0)
    return(ExclusionCompositeOp);
  if (LocaleNCompare(mode,"div ",4) == 0)
    return(ColorDodgeCompositeOp);
  if (LocaleNCompare(mode,"idiv",4) == 0)
    return(ColorBurnCompositeOp);
  if (LocaleNCompare(mode,"lbrn",4) == 0)
    return(LinearBurnCompositeOp);
  if (LocaleNCompare(mode,"lddg",4) == 0)
    return(LinearDodgeCompositeOp);
  if (LocaleNCompare(mode,"lLit",4) == 0)
    return(LinearLightCompositeOp);
  if (LocaleNCompare(mode,"vLit",4) == 0)
    return(VividLightCompositeOp);
  if (LocaleNCompare(mode,"pLit",4) == 0)
    return(PinLightCompositeOp);
  if (LocaleNCompare(mode,"hMix",4) == 0)
    return(HardMixCompositeOp);
  return(OverCompositeOp);
}

static inline void ReversePSDString(Image *image,char *p,size_t length)
{
  char
    *q;

  if (image->endian == MSBEndian)
    return;

  q=p+length;
  for(--q; p < q; ++p, --q)
  {
    *p = *p ^ *q,
    *q = *p ^ *q,
    *p = *p ^ *q;
  }
}

static inline void SetPSDPixel(Image *image,const size_t channels,
  const ssize_t type,const size_t packet_size,const Quantum pixel,Quantum *q,
  ExceptionInfo *exception)
{
  if (image->storage_class == PseudoClass)
    {
      PixelInfo
        *color;

      if (type == 0)
        {
          if (packet_size == 1)
            SetPixelIndex(image,ScaleQuantumToChar(pixel),q);
          else
            SetPixelIndex(image,ScaleQuantumToShort(pixel),q);
        }
      color=image->colormap+(ssize_t) ConstrainColormapIndex(image,
        GetPixelIndex(image,q),exception);
      if ((type == 0) && (channels > 1))
        return;
      else
        color->alpha=(MagickRealType) pixel;
      SetPixelViaPixelInfo(image,color,q);
      return;
    }
  switch (type)
  {
    case -1:
    {
      SetPixelAlpha(image,pixel,q);
      break;
    }
    case -2:
    case 0:
    {
      SetPixelRed(image,pixel,q);
      break;
    }
    case -3:
    case 1:
    {
      SetPixelGreen(image,pixel,q);
      break;
    }
    case -4:
    case 2:
    {
      SetPixelBlue(image,pixel,q);
      break;
    }
    case 3:
    {
      if (image->colorspace == CMYKColorspace)
        SetPixelBlack(image,pixel,q);
      else
        if (image->alpha_trait != UndefinedPixelTrait)
          SetPixelAlpha(image,pixel,q);
      break;
    }
    case 4:
    {
      if ((IssRGBCompatibleColorspace(image->colorspace) != MagickFalse) &&
          (channels > 3))
        break;
      if (image->alpha_trait != UndefinedPixelTrait)
        SetPixelAlpha(image,pixel,q);
      break;
    }
  }
}

static MagickBooleanType ReadPSDChannelPixels(Image *image,
  const size_t channels,const size_t row,const ssize_t type,
  const unsigned char *pixels,ExceptionInfo *exception)
{
  Quantum
    pixel;

  register const unsigned char
    *p;

  register Quantum
    *q;

  register ssize_t
    x;

  size_t
    packet_size;

  p=pixels;
  q=GetAuthenticPixels(image,0,row,image->columns,1,exception);
  if (q == (Quantum *) NULL)
    return MagickFalse;
  packet_size=GetPSDPacketSize(image);
  for (x=0; x < (ssize_t) image->columns; x++)
  {
    if (packet_size == 1)
      pixel=ScaleCharToQuantum(*p++);
    else if (packet_size == 2)
      {
        unsigned short
          nibble;

        p=PushShortPixel(MSBEndian,p,&nibble);
        pixel=ScaleShortToQuantum(nibble);
      }
    else
      {
        MagickFloatType
          nibble;

        p=PushFloatPixel(MSBEndian,p,&nibble);
        pixel=ClampToQuantum((MagickRealType)QuantumRange*nibble);
      }
    if (image->depth > 1)
      {
        SetPSDPixel(image,channels,type,packet_size,pixel,q,exception);
        q+=GetPixelChannels(image);
      }
    else
      {
        ssize_t
          bit,
          number_bits;

        number_bits=image->columns-x;
        if (number_bits > 8)
          number_bits=8;
        for (bit = 0; bit < number_bits; bit++)
        {
          SetPSDPixel(image,channels,type,packet_size,(((unsigned char) pixel)
            & (0x01 << (7-bit))) != 0 ? 0 : QuantumRange,q,exception);
          q+=GetPixelChannels(image);
          x++;
        }
        if (x != (ssize_t) image->columns)
          x--;
        continue;
      }
  }
  return(SyncAuthenticPixels(image,exception));
}

static MagickBooleanType ReadPSDChannelRaw(Image *image,const size_t channels,
  const ssize_t type,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  size_t
    count,
    row_size;

  ssize_t
    y;

  unsigned char
    *pixels;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
       "      layer data is RAW");

  row_size=GetPSDRowSize(image);
  pixels=(unsigned char *) AcquireQuantumMemory(row_size,sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);

  status=MagickTrue;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    status=MagickFalse;

    count=ReadBlob(image,row_size,pixels);
    if (count != row_size)
      {
        status=MagickFalse;
        break;
      }

    status=ReadPSDChannelPixels(image,channels,y,type,pixels,exception);
    if (status == MagickFalse)
      break;
  }

  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  return(status);
}

static inline MagickOffsetType *ReadPSDRLESizes(Image *image,
  const PSDInfo *psd_info,const size_t size)
{
  MagickOffsetType
    *sizes;

  ssize_t
    y;

  sizes=(MagickOffsetType *) AcquireQuantumMemory(size,sizeof(*sizes));
  if(sizes != (MagickOffsetType *) NULL)
    {
      for (y=0; y < (ssize_t) size; y++)
      {
        if (psd_info->version == 1)
          sizes[y]=(MagickOffsetType) ReadBlobShort(image);
        else
          sizes[y]=(MagickOffsetType) ReadBlobLong(image);
      }
    }
  return sizes;
}

static MagickBooleanType ReadPSDChannelRLE(Image *image,const PSDInfo *psd_info,
  const ssize_t type,MagickOffsetType *sizes,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  size_t
    length,
    row_size;

  ssize_t
    count,
    y;

  unsigned char
    *compact_pixels,
    *pixels;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
       "      layer data is RLE compressed");

  row_size=GetPSDRowSize(image);
  pixels=(unsigned char *) AcquireQuantumMemory(row_size,sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);

  length=0;
  for (y=0; y < (ssize_t) image->rows; y++)
    if ((MagickOffsetType) length < sizes[y])
      length=(size_t) sizes[y];

  if (length > (row_size+512)) // arbitrary number
    {
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      ThrowBinaryException(ResourceLimitError,"InvalidLength",image->filename);
    }

  compact_pixels=(unsigned char *) AcquireQuantumMemory(length,sizeof(*pixels));
  if (compact_pixels == (unsigned char *) NULL)
    {
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }

  (void) memset(compact_pixels,0,length*sizeof(*compact_pixels));

  status=MagickTrue;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    status=MagickFalse;

    count=ReadBlob(image,(size_t) sizes[y],compact_pixels);
    if (count != (ssize_t) sizes[y])
      break;

    count=DecodePSDPixels((size_t) sizes[y],compact_pixels,
      (ssize_t) (image->depth == 1 ? 123456 : image->depth),row_size,pixels);
    if (count != (ssize_t) row_size)
      break;

    status=ReadPSDChannelPixels(image,psd_info->channels,y,type,pixels,
      exception);
    if (status == MagickFalse)
      break;
  }

  compact_pixels=(unsigned char *) RelinquishMagickMemory(compact_pixels);
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  return(status);
}

#ifdef MAGICKCORE_ZLIB_DELEGATE
static MagickBooleanType ReadPSDChannelZip(Image *image,const size_t channels,
  const ssize_t type,const PSDCompressionType compression,
  const size_t compact_size,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  register unsigned char
    *p;

  size_t
    count,
    length,
    packet_size,
    row_size;

  ssize_t
    y;

  unsigned char
    *compact_pixels,
    *pixels;

  z_stream
    stream;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
       "      layer data is ZIP compressed");

  if ((MagickSizeType) compact_size > GetBlobSize(image))
    ThrowBinaryException(CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  compact_pixels=(unsigned char *) AcquireQuantumMemory(compact_size,
    sizeof(*compact_pixels));
  if (compact_pixels == (unsigned char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);

  packet_size=GetPSDPacketSize(image);
  row_size=image->columns*packet_size;
  count=image->rows*row_size;

  pixels=(unsigned char *) AcquireQuantumMemory(count,sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    {
      compact_pixels=(unsigned char *) RelinquishMagickMemory(compact_pixels);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  if (ReadBlob(image,compact_size,compact_pixels) != (ssize_t) compact_size)
    {
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      compact_pixels=(unsigned char *) RelinquishMagickMemory(compact_pixels);
      ThrowBinaryException(CorruptImageError,"UnexpectedEndOfFile",
        image->filename);
    }

  memset(&stream,0,sizeof(stream));
  stream.data_type=Z_BINARY;
  stream.next_in=(Bytef *)compact_pixels;
  stream.avail_in=(uInt) compact_size;
  stream.next_out=(Bytef *)pixels;
  stream.avail_out=(uInt) count;

  if (inflateInit(&stream) == Z_OK)
    {
      int
        ret;

      while (stream.avail_out > 0)
      {
        ret=inflate(&stream,Z_SYNC_FLUSH);
        if ((ret != Z_OK) && (ret != Z_STREAM_END))
          {
            (void) inflateEnd(&stream);
            compact_pixels=(unsigned char *) RelinquishMagickMemory(
              compact_pixels);
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
            return(MagickFalse);
          }
        if (ret == Z_STREAM_END)
          break;
      }
      (void) inflateEnd(&stream);
    }

  if (compression == ZipWithPrediction)
    {
      p=pixels;
      while (count > 0)
      {
        length=image->columns;
        while (--length)
        {
          if (packet_size == 2)
            {
              p[2]+=p[0]+((p[1]+p[3]) >> 8);
              p[3]+=p[1];
            }
          // else if (packet_size == 4)
          //   {
          //     TODO: Figure out what to do there.
          //   }
          else
            *(p+1)+=*p;
          p+=packet_size;
        }
        p+=packet_size;
        count-=row_size;
      }
    }

  status=MagickTrue;
  p=pixels;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    status=ReadPSDChannelPixels(image,channels,y,type,p,exception);
    if (status == MagickFalse)
      break;

    p+=row_size;
  }

  compact_pixels=(unsigned char *) RelinquishMagickMemory(compact_pixels);
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  return(status);
}
#endif

static MagickBooleanType ReadPSDChannel(Image *image,
  const ImageInfo *image_info,const PSDInfo *psd_info,LayerInfo* layer_info,
  const size_t channel,const PSDCompressionType compression,
  ExceptionInfo *exception)
{
  Image
    *channel_image,
    *mask;

  MagickOffsetType
    offset;

  MagickBooleanType
    status;

  channel_image=image;
  mask=(Image *) NULL;
  if ((layer_info->channel_info[channel].type < -1) &&
      (layer_info->mask.page.width > 0) && (layer_info->mask.page.height > 0))
    {
      const char
        *option;

      /*
        Ignore mask that is not a user supplied layer mask, if the mask is
        disabled or if the flags have unsupported values.
      */
      option=GetImageOption(image_info,"psd:preserve-opacity-mask");
      if ((layer_info->channel_info[channel].type != -2) ||
          (layer_info->mask.flags > 2) || ((layer_info->mask.flags & 0x02) &&
           (IsStringTrue(option) == MagickFalse)))
        {
          SeekBlob(image,layer_info->channel_info[channel].size-2,SEEK_CUR);
          return(MagickTrue);
        }
      mask=CloneImage(image,layer_info->mask.page.width,
        layer_info->mask.page.height,MagickFalse,exception);
      if (mask != (Image *) NULL)
        {
          SetImageType(mask,GrayscaleType,exception);
          channel_image=mask;
        }
    }

  offset=TellBlob(image);
  status=MagickFalse;
  switch(compression)
  {
    case Raw:
      status=ReadPSDChannelRaw(channel_image,psd_info->channels,
        layer_info->channel_info[channel].type,exception);
      break;
    case RLE:
      {
        MagickOffsetType
          *sizes;

        sizes=ReadPSDRLESizes(channel_image,psd_info,channel_image->rows);
        if (sizes == (MagickOffsetType *) NULL)
          ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
            image->filename);
        status=ReadPSDChannelRLE(channel_image,psd_info,
          layer_info->channel_info[channel].type,sizes,exception);
        sizes=(MagickOffsetType *) RelinquishMagickMemory(sizes);
      }
      break;
    case ZipWithPrediction:
    case ZipWithoutPrediction:
#ifdef MAGICKCORE_ZLIB_DELEGATE
      status=ReadPSDChannelZip(channel_image,layer_info->channels,
        layer_info->channel_info[channel].type,compression,
        layer_info->channel_info[channel].size-2,exception);
#else
      (void) ThrowMagickException(exception,GetMagickModule(),
          MissingDelegateWarning,"DelegateLibrarySupportNotBuiltIn",
            "'%s' (ZLIB)",image->filename);
#endif
      break;
    default:
      (void) ThrowMagickException(exception,GetMagickModule(),TypeWarning,
        "CompressionNotSupported","'%.20g'",(double) compression);
      break;
  }

  SeekBlob(image,offset+layer_info->channel_info[channel].size-2,SEEK_SET);
  if (status == MagickFalse)
    {
      if (mask != (Image *) NULL)
        DestroyImage(mask);
      ThrowBinaryException(CoderError,"UnableToDecompressImage",
        image->filename);
    }
  if (mask != (Image *) NULL)
    {
      if (layer_info->mask.image != (Image *) NULL)
        layer_info->mask.image=DestroyImage(layer_info->mask.image);
      layer_info->mask.image=mask;
    }
  return(status);
}

static MagickBooleanType ReadPSDLayer(Image *image,const ImageInfo *image_info,
  const PSDInfo *psd_info,LayerInfo* layer_info,ExceptionInfo *exception)
{
  char
    message[MagickPathExtent];

  MagickBooleanType
    status;

  PSDCompressionType
    compression;

  ssize_t
    j;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    setting up new layer image");
  if (psd_info->mode != IndexedMode)
    (void) SetImageBackgroundColor(layer_info->image,exception);
  layer_info->image->compose=PSDBlendModeToCompositeOperator(
    layer_info->blendkey);
  if (layer_info->visible == MagickFalse)
    layer_info->image->compose=NoCompositeOp;
  /*
    Set up some hidden attributes for folks that need them.
  */
  (void) FormatLocaleString(message,MagickPathExtent,"%.20g",
    (double) layer_info->page.x);
  (void) SetImageArtifact(layer_info->image,"psd:layer.x",message);
  (void) FormatLocaleString(message,MagickPathExtent,"%.20g",
    (double) layer_info->page.y);
  (void) SetImageArtifact(layer_info->image,"psd:layer.y",message);
  (void) FormatLocaleString(message,MagickPathExtent,"%.20g",(double)
    layer_info->opacity);
  (void) SetImageArtifact(layer_info->image,"psd:layer.opacity",message);
  (void) SetImageProperty(layer_info->image,"label",(char *) layer_info->name,
    exception);

  status=MagickTrue;
  for (j=0; j < (ssize_t) layer_info->channels; j++)
  {
    if (image->debug != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    reading data for channel %.20g",(double) j);

    compression=(PSDCompressionType) ReadBlobShort(layer_info->image);

    /* TODO: Remove this when we figure out how to support this */
    if ((compression == ZipWithPrediction) && (image->depth == 32))
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          TypeError,"CompressionNotSupported","ZipWithPrediction(32 bit)");
        return(MagickFalse);
      }

    layer_info->image->compression=ConvertPSDCompression(compression);
    if (layer_info->channel_info[j].type == -1)
      layer_info->image->alpha_trait=BlendPixelTrait;

    status=ReadPSDChannel(layer_info->image,image_info,psd_info,layer_info,j,
      compression,exception);

    if (status == MagickFalse)
      break;
  }

  if (status != MagickFalse)
    status=ApplyPSDLayerOpacity(layer_info->image,layer_info->opacity,
      MagickFalse,exception);

  if ((status != MagickFalse) &&
      (layer_info->image->colorspace == CMYKColorspace))
    status=NegateCMYK(layer_info->image,exception);

  if ((status != MagickFalse) && (layer_info->mask.image != (Image *) NULL))
    {
      const char
        *option;

      layer_info->mask.image->page.x=layer_info->mask.page.x;
      layer_info->mask.image->page.y=layer_info->mask.page.y;
      /* Do not composite the mask when it is disabled */
      if ((layer_info->mask.flags & 0x02) == 0x02)
        layer_info->mask.image->compose=NoCompositeOp;
      else
        status=ApplyPSDOpacityMask(layer_info->image,layer_info->mask.image,
          layer_info->mask.background == 0 ? 0 : QuantumRange,MagickFalse,
          exception);
      option=GetImageOption(image_info,"psd:preserve-opacity-mask");
      if (IsStringTrue(option) != MagickFalse)
        PreservePSDOpacityMask(image,layer_info,exception);
      layer_info->mask.image=DestroyImage(layer_info->mask.image);
    }

  return(status);
}

static MagickBooleanType CheckPSDChannels(const PSDInfo *psd_info,
  LayerInfo *layer_info)
{
  int
    channel_type;

  register ssize_t
    i;

  if (layer_info->channels < psd_info->min_channels)
    return(MagickFalse);
  channel_type=RedChannel;
  if (psd_info->min_channels >= 3)
    channel_type|=(GreenChannel | BlueChannel);
  if (psd_info->min_channels >= 4)
    channel_type|=BlackChannel;
  for (i=0; i < layer_info->channels; i++)
  {
    short
      type;

    type=layer_info->channel_info[i].type;
    if (type == -1)
      {
        channel_type|=AlphaChannel;
        continue;
      }
    if (type < -1)
      continue;
    if (type == 0)
      channel_type&=~RedChannel;
    else if (type == 1)
      channel_type&=~GreenChannel;
    else if (type == 2)
      channel_type&=~BlueChannel;
    else if (type == 3)
      channel_type&=~BlackChannel;
  }
  if (channel_type == 0)
    return(MagickTrue);
  if ((channel_type == AlphaChannel) &&
      (layer_info->channels >= psd_info->min_channels + 1))
    return(MagickTrue);
  return(MagickFalse);
}

static MagickBooleanType ReadPSDLayersInternal(Image *image,
  const ImageInfo *image_info,const PSDInfo *psd_info,
  const MagickBooleanType skip_layers,ExceptionInfo *exception)
{
  char
    type[4];

  LayerInfo
    *layer_info;

  MagickSizeType
    size;

  MagickBooleanType
    status;

  register ssize_t
    i;

  ssize_t
    count,
    j,
    number_layers;

  size=GetPSDSize(psd_info,image);
  if (size == 0)
    {
      /*
        Skip layers & masks.
      */
      (void) ReadBlobLong(image);
      count=ReadBlob(image,4,(unsigned char *) type);
      if (count == 4)
        ReversePSDString(image,type,count);
      if ((count != 4) || (LocaleNCompare(type,"8BIM",4) != 0))
        return(MagickTrue);
      else
        {
          count=ReadBlob(image,4,(unsigned char *) type);
          if (count == 4)
            ReversePSDString(image,type,4);
          if ((count == 4) && ((LocaleNCompare(type,"Lr16",4) == 0) ||
              (LocaleNCompare(type,"Lr32",4) == 0)))
            size=GetPSDSize(psd_info,image);
          else
            return(MagickTrue);
        }
    }
  status=MagickTrue;
  if (size != 0)
    {
      layer_info=(LayerInfo *) NULL;
      number_layers=(short) ReadBlobShort(image);

      if (number_layers < 0)
        {
          /*
            The first alpha channel in the merged result contains the
            transparency data for the merged result.
          */
          number_layers=MagickAbsoluteValue(number_layers);
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  negative layer count corrected for");
          image->alpha_trait=BlendPixelTrait;
        }

      /*
        We only need to know if the image has an alpha channel
      */
      if (skip_layers != MagickFalse)
        return(MagickTrue);

      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  image contains %.20g layers",(double) number_layers);

      if (number_layers == 0)
        ThrowBinaryException(CorruptImageError,"InvalidNumberOfLayers",
          image->filename);

      layer_info=(LayerInfo *) AcquireQuantumMemory((size_t) number_layers,
        sizeof(*layer_info));
      if (layer_info == (LayerInfo *) NULL)
        {
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  allocation of LayerInfo failed");
          ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
            image->filename);
        }
      (void) memset(layer_info,0,(size_t) number_layers*
        sizeof(*layer_info));

      for (i=0; i < number_layers; i++)
      {
        ssize_t
          x,
          y;

        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  reading layer #%.20g",(double) i+1);
        layer_info[i].page.y=ReadBlobSignedLong(image);
        layer_info[i].page.x=ReadBlobSignedLong(image);
        y=ReadBlobSignedLong(image);
        x=ReadBlobSignedLong(image);
        layer_info[i].page.width=(size_t) (x-layer_info[i].page.x);
        layer_info[i].page.height=(size_t) (y-layer_info[i].page.y);
        layer_info[i].channels=ReadBlobShort(image);
        if (layer_info[i].channels > MaxPSDChannels)
          {
            layer_info=DestroyLayerInfo(layer_info,number_layers);
            ThrowBinaryException(CorruptImageError,"MaximumChannelsExceeded",
              image->filename);
          }
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    offset(%.20g,%.20g), size(%.20g,%.20g), channels=%.20g",
            (double) layer_info[i].page.x,(double) layer_info[i].page.y,
            (double) layer_info[i].page.height,(double)
            layer_info[i].page.width,(double) layer_info[i].channels);
        for (j=0; j < (ssize_t) layer_info[i].channels; j++)
        {
          layer_info[i].channel_info[j].type=(short) ReadBlobShort(image);
          if ((layer_info[i].channel_info[j].type < -4) ||
              (layer_info[i].channel_info[j].type > 4))
            {
              layer_info=DestroyLayerInfo(layer_info,number_layers);
              ThrowBinaryException(CorruptImageError,"NoSuchImageChannel",
                image->filename);
            }
          layer_info[i].channel_info[j].size=(size_t) GetPSDSize(psd_info,
            image);
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    channel[%.20g]: type=%.20g, size=%.20g",(double) j,
              (double) layer_info[i].channel_info[j].type,
              (double) layer_info[i].channel_info[j].size);
        }
        if (CheckPSDChannels(psd_info,&layer_info[i]) == MagickFalse)
          {
            layer_info=DestroyLayerInfo(layer_info,number_layers);
            ThrowBinaryException(CorruptImageError,"ImproperImageHeader",
              image->filename);
          }
        count=ReadBlob(image,4,(unsigned char *) type);
        if (count == 4)
          ReversePSDString(image,type,4);
        if ((count != 4) || (LocaleNCompare(type,"8BIM",4) != 0))
          {
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  layer type was %.4s instead of 8BIM", type);
            layer_info=DestroyLayerInfo(layer_info,number_layers);
            ThrowBinaryException(CorruptImageError,"ImproperImageHeader",
              image->filename);
          }
        count=ReadBlob(image,4,(unsigned char *) layer_info[i].blendkey);
        if (count != 4)
          {
            layer_info=DestroyLayerInfo(layer_info,number_layers);
            ThrowBinaryException(CorruptImageError,"ImproperImageHeader",
              image->filename);
          }
        ReversePSDString(image,layer_info[i].blendkey,4);
        layer_info[i].opacity=(Quantum) ScaleCharToQuantum((unsigned char)
          ReadBlobByte(image));
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

        size=ReadBlobLong(image);
        if (size != 0)
          {
            MagickSizeType
              combined_length,
              length;

            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    layer contains additional info");
            length=ReadBlobLong(image);
            combined_length=length+4;
            if (length != 0)
              {
                /*
                  Layer mask info.
                */
                layer_info[i].mask.page.y=ReadBlobSignedLong(image);
                layer_info[i].mask.page.x=ReadBlobSignedLong(image);
                layer_info[i].mask.page.height=(size_t) (ReadBlobSignedLong(image)-
                  layer_info[i].mask.page.y);
                layer_info[i].mask.page.width=(size_t) (ReadBlobSignedLong(image)-
                  layer_info[i].mask.page.x);
                layer_info[i].mask.background=(unsigned char) ReadBlobByte(
                  image);
                layer_info[i].mask.flags=(unsigned char) ReadBlobByte(image);
                if (!(layer_info[i].mask.flags & 0x01))
                  {
                    layer_info[i].mask.page.y=layer_info[i].mask.page.y-
                      layer_info[i].page.y;
                    layer_info[i].mask.page.x=layer_info[i].mask.page.x-
                      layer_info[i].page.x;
                  }
                if (image->debug != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "      layer mask: offset(%.20g,%.20g), size(%.20g,%.20g), length=%.20g",
                    (double) layer_info[i].mask.page.x,(double)
                    layer_info[i].mask.page.y,(double)
                    layer_info[i].mask.page.width,(double)
                    layer_info[i].mask.page.height,(double) ((MagickOffsetType)
                    length)-18);
                /*
                  Skip over the rest of the layer mask information.
                */
                if (DiscardBlobBytes(image,(MagickSizeType) (length-18)) == MagickFalse)
                  {
                    layer_info=DestroyLayerInfo(layer_info,number_layers);
                    ThrowBinaryException(CorruptImageError,
                      "UnexpectedEndOfFile",image->filename);
                  }
              }
            length=ReadBlobLong(image);
            combined_length+=length+4;
            if (length != 0)
              {
                /*
                  Layer blending ranges info.
                */
                if (image->debug != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "      layer blending ranges: length=%.20g",(double)
                    ((MagickOffsetType) length));
                if (DiscardBlobBytes(image,length) == MagickFalse)
                  {
                    layer_info=DestroyLayerInfo(layer_info,number_layers);
                    ThrowBinaryException(CorruptImageError,
                      "UnexpectedEndOfFile",image->filename);
                  }
              }
            /*
              Layer name.
            */
            length=(MagickSizeType) (unsigned char) ReadBlobByte(image);
            combined_length+=length+1;
            if (length > 0)
              (void) ReadBlob(image,(size_t) length++,layer_info[i].name);
            layer_info[i].name[length]='\0';
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "      layer name: %s",layer_info[i].name);
            if ((length % 4) != 0)
              {
                length=4-(length % 4);
                combined_length+=length;
                /* Skip over the padding of the layer name */
                if (DiscardBlobBytes(image,length) == MagickFalse)
                  {
                    layer_info=DestroyLayerInfo(layer_info,number_layers);
                    ThrowBinaryException(CorruptImageError,
                      "UnexpectedEndOfFile",image->filename);
                  }
              }
            length=(MagickSizeType) size-combined_length;
            if (length > 0)
              {
                unsigned char
                  *info;

                if (length > GetBlobSize(image))
                  {
                    layer_info=DestroyLayerInfo(layer_info,number_layers);
                    ThrowBinaryException(CorruptImageError,
                      "InsufficientImageDataInFile",image->filename);
                  }
                layer_info[i].info=AcquireStringInfo((const size_t) length);
                info=GetStringInfoDatum(layer_info[i].info);
                (void) ReadBlob(image,(const size_t) length,info);
              }
          }
      }

      for (i=0; i < number_layers; i++)
      {
        if ((layer_info[i].page.width == 0) || (layer_info[i].page.height == 0))
          {
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "      layer data is empty");
            if (layer_info[i].info != (StringInfo *) NULL)
              layer_info[i].info=DestroyStringInfo(layer_info[i].info);
            continue;
          }

        /*
          Allocate layered image.
        */
        layer_info[i].image=CloneImage(image,layer_info[i].page.width,
          layer_info[i].page.height,MagickFalse,exception);
        if (layer_info[i].image == (Image *) NULL)
          {
            layer_info=DestroyLayerInfo(layer_info,number_layers);
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  allocation of image for layer %.20g failed",(double) i);
            ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
              image->filename);
          }
        if (layer_info[i].info != (StringInfo *) NULL)
          {
            (void) SetImageProfile(layer_info[i].image,"psd:additional-info",
              layer_info[i].info,exception);
            layer_info[i].info=DestroyStringInfo(layer_info[i].info);
          }
      }

      if (image_info->ping == MagickFalse)
        {
          for (i=0; i < number_layers; i++)
          {
            if (layer_info[i].image == (Image *) NULL)
              {
                for (j=0; j < layer_info[i].channels; j++)
                {
                  if (DiscardBlobBytes(image,(MagickSizeType)
                      layer_info[i].channel_info[j].size) == MagickFalse)
                    {
                      layer_info=DestroyLayerInfo(layer_info,number_layers);
                      ThrowBinaryException(CorruptImageError,
                        "UnexpectedEndOfFile",image->filename);
                    }
                }
                continue;
              }

            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  reading data for layer %.20g",(double) i);

            status=ReadPSDLayer(image,image_info,psd_info,&layer_info[i],
              exception);
            if (status == MagickFalse)
              break;

            status=SetImageProgress(image,LoadImagesTag,i,(MagickSizeType)
              number_layers);
            if (status == MagickFalse)
              break;
          }
        }

      if (status != MagickFalse)
        {
          for (i=0; i < number_layers; i++)
          {
            if (layer_info[i].image == (Image *) NULL)
              {
                for (j=i; j < number_layers - 1; j++)
                  layer_info[j] = layer_info[j+1];
                number_layers--;
                i--;
              }
          }

          if (number_layers > 0)
            {
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
            }
          layer_info=(LayerInfo *) RelinquishMagickMemory(layer_info);
        }
      else
        layer_info=DestroyLayerInfo(layer_info,number_layers);
    }

  return(status);
}

ModuleExport MagickBooleanType ReadPSDLayers(Image *image,
  const ImageInfo *image_info,const PSDInfo *psd_info,ExceptionInfo *exception)
{
  PolicyDomain
    domain;

  PolicyRights
    rights;

  domain=CoderPolicyDomain;
  rights=ReadPolicyRights;
  if (IsRightsAuthorized(domain,rights,"PSD") == MagickFalse)
    return(MagickTrue);
  return(ReadPSDLayersInternal(image,image_info,psd_info,MagickFalse,
    exception));
}

static MagickBooleanType ReadPSDMergedImage(const ImageInfo *image_info,
  Image *image,const PSDInfo *psd_info,ExceptionInfo *exception)
{
  MagickOffsetType
    *sizes;

  MagickBooleanType
    status;

  PSDCompressionType
    compression;

  register ssize_t
    i;

  compression=(PSDCompressionType) ReadBlobMSBShort(image);
  image->compression=ConvertPSDCompression(compression);

  if (compression != Raw && compression != RLE)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        TypeWarning,"CompressionNotSupported","'%.20g'",(double) compression);
      return(MagickFalse);
    }

  sizes=(MagickOffsetType *) NULL;
  if (compression == RLE)
    {
      sizes=ReadPSDRLESizes(image,psd_info,image->rows*psd_info->channels);
      if (sizes == (MagickOffsetType *) NULL)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
    }

  status=MagickTrue;
  for (i=0; i < (ssize_t) psd_info->channels; i++)
  {
    ssize_t
      type;

    type=i;
    if ((type == 1) && (psd_info->channels == 2))
      type=-1;

    if (compression == RLE)
      status=ReadPSDChannelRLE(image,psd_info,type,sizes+(i*image->rows),
        exception);
    else
      status=ReadPSDChannelRaw(image,psd_info->channels,type,exception);

    if (status != MagickFalse)
      status=SetImageProgress(image,LoadImagesTag,i,psd_info->channels);

    if (status == MagickFalse)
      break;
  }

  if ((status != MagickFalse) && (image->colorspace == CMYKColorspace))
    status=NegateCMYK(image,exception);

  if (status != MagickFalse)
    status=CorrectPSDAlphaBlend(image_info,image,exception);

  sizes=(MagickOffsetType *) RelinquishMagickMemory(sizes);

  return(status);
}

static Image *ReadPSDImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    has_merged_image,
    skip_layers;

  MagickOffsetType
    offset;

  MagickSizeType
    length;

  MagickBooleanType
    status;

  PSDInfo
    psd_info;

  register ssize_t
    i;

  size_t
    imageListLength;

  ssize_t
    count;

  StringInfo
    *profile;

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
    Read image header.
  */
  image->endian=MSBEndian;
  count=ReadBlob(image,4,(unsigned char *) psd_info.signature);
  psd_info.version=ReadBlobMSBShort(image);
  if ((count != 4) || (LocaleNCompare(psd_info.signature,"8BPS",4) != 0) ||
      ((psd_info.version != 1) && (psd_info.version != 2)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  (void) ReadBlob(image,6,psd_info.reserved);
  psd_info.channels=ReadBlobMSBShort(image);
  if (psd_info.channels < 1)
    ThrowReaderException(CorruptImageError,"MissingImageChannel");
  if (psd_info.channels > MaxPSDChannels)
    ThrowReaderException(CorruptImageError,"MaximumChannelsExceeded");
  psd_info.rows=ReadBlobMSBLong(image);
  psd_info.columns=ReadBlobMSBLong(image);
  if ((psd_info.version == 1) && ((psd_info.rows > 30000) ||
      (psd_info.columns > 30000)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  psd_info.depth=ReadBlobMSBShort(image);
  if ((psd_info.depth != 1) && (psd_info.depth != 8) &&
      (psd_info.depth != 16) && (psd_info.depth != 32))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  psd_info.mode=ReadBlobMSBShort(image);
  if ((psd_info.mode == IndexedMode) && (psd_info.channels > 3))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Image is %.20g x %.20g with channels=%.20g, depth=%.20g, mode=%s",
      (double) psd_info.columns,(double) psd_info.rows,(double)
      psd_info.channels,(double) psd_info.depth,ModeToString((PSDImageType)
      psd_info.mode));
  if (EOFBlob(image) != MagickFalse)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
    Initialize image.
  */
  image->depth=psd_info.depth;
  image->columns=psd_info.columns;
  image->rows=psd_info.rows;
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  status=ResetImagePixels(image,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  psd_info.min_channels=3;
  if (psd_info.mode == LabMode)
    SetImageColorspace(image,LabColorspace,exception);
  if (psd_info.mode == CMYKMode)
    {
      psd_info.min_channels=4;
      SetImageColorspace(image,CMYKColorspace,exception);
      if (psd_info.channels > 4)
        SetImageAlphaChannel(image,ActivateAlphaChannel,exception);
    }
  else if ((psd_info.mode == BitmapMode) || (psd_info.mode == GrayscaleMode) ||
           (psd_info.mode == DuotoneMode))
    {
      if (psd_info.depth != 32)
        {
          status=AcquireImageColormap(image,psd_info.depth < 16 ? 256 : 65536,
            exception);
          if (status == MagickFalse)
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Image colormap allocated");
        }
      psd_info.min_channels=1;
      SetImageColorspace(image,GRAYColorspace,exception);
      if (psd_info.channels > 1)
        SetImageAlphaChannel(image,ActivateAlphaChannel,exception);
    }
  else
    if (psd_info.channels > 3)
      SetImageAlphaChannel(image,ActivateAlphaChannel,exception);
  if (psd_info.channels < psd_info.min_channels)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
    Read PSD raster colormap only present for indexed and duotone images.
  */
  length=ReadBlobMSBLong(image);
  if ((psd_info.mode == IndexedMode) && (length < 3))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if (length != 0)
    {
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  reading colormap");
      if ((psd_info.mode == DuotoneMode) || (psd_info.depth == 32))
        {
          /*
            Duotone image data;  the format of this data is undocumented.
            32 bits per pixel;  the colormap is ignored.
          */
          (void) SeekBlob(image,(const MagickOffsetType) length,SEEK_CUR);
        }
      else
        {
          size_t
            number_colors;

          /*
            Read PSD raster colormap.
          */
          number_colors=length/3;
          if (number_colors > 65536)
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
          if (AcquireImageColormap(image,number_colors,exception) == MagickFalse)
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
          image->alpha_trait=UndefinedPixelTrait;
        }
    }
  if ((image->depth == 1) && (image->storage_class != PseudoClass))
    ThrowReaderException(CorruptImageError, "ImproperImageHeader");
  has_merged_image=MagickTrue;
  profile=(StringInfo *) NULL;
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
      if (length > GetBlobSize(image))
        ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
      blocks=(unsigned char *) AcquireQuantumMemory((size_t) length,
        sizeof(*blocks));
      if (blocks == (unsigned char *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      count=ReadBlob(image,(size_t) length,blocks);
      if ((count != (ssize_t) length) || (length < 4) ||
          (LocaleNCompare((char *) blocks,"8BIM",4) != 0))
        {
          blocks=(unsigned char *) RelinquishMagickMemory(blocks);
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        }
      profile=ParseImageResourceBlocks(image,blocks,(size_t) length,
        &has_merged_image,exception);
      blocks=(unsigned char *) RelinquishMagickMemory(blocks);
    }
  /*
    Layer and mask block.
  */
  length=GetPSDSize(&psd_info,image);
  if (length == 8)
    {
      length=ReadBlobMSBLong(image);
      length=ReadBlobMSBLong(image);
    }
  offset=TellBlob(image);
  skip_layers=MagickFalse;
  if ((image_info->number_scenes == 1) && (image_info->scene == 0) &&
      (has_merged_image != MagickFalse))
    {
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  read composite only");
      skip_layers=MagickTrue;
    }
  if (length == 0)
    {
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  image has no layers");
    }
  else
    {
      if (ReadPSDLayersInternal(image,image_info,&psd_info,skip_layers,
            exception) != MagickTrue)
        {
          if (profile != (StringInfo *) NULL)
            profile=DestroyStringInfo(profile);
          (void) CloseBlob(image);
          image=DestroyImageList(image);
          return((Image *) NULL);
        }

      /*
         Skip the rest of the layer and mask information.
      */
      SeekBlob(image,offset+length,SEEK_SET);
    }
  /*
    If we are only "pinging" the image, then we're done - so return.
  */
  if (EOFBlob(image) != MagickFalse)
    {
      if (profile != (StringInfo *) NULL)
        profile=DestroyStringInfo(profile);
      ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
    }
  if (image_info->ping != MagickFalse)
    {
      if (profile != (StringInfo *) NULL)
        profile=DestroyStringInfo(profile);
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  /*
    Read the precombined layer, present for PSD < 4 compatibility.
  */
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  reading the precombined layer");
  imageListLength=GetImageListLength(image);
  if ((has_merged_image != MagickFalse) || (imageListLength == 1))
    has_merged_image=(MagickBooleanType) ReadPSDMergedImage(image_info,image,
      &psd_info,exception);
  if ((has_merged_image == MagickFalse) && (imageListLength == 1) &&
      (length != 0))
    {
      SeekBlob(image,offset,SEEK_SET);
      status=ReadPSDLayersInternal(image,image_info,&psd_info,MagickFalse,
        exception);
      if (status != MagickTrue)
        {
          if (profile != (StringInfo *) NULL)
            profile=DestroyStringInfo(profile);
          (void) CloseBlob(image);
          image=DestroyImageList(image);
          return((Image *) NULL);
        }
    }
  if (has_merged_image == MagickFalse)
    {
      Image
        *merged;

      if (imageListLength == 1)
        {
          if (profile != (StringInfo *) NULL)
            profile=DestroyStringInfo(profile);
          ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
        }
      image->background_color.alpha=TransparentAlpha;
      image->background_color.alpha_trait=BlendPixelTrait;
      (void) SetImageBackgroundColor(image,exception);
      merged=MergeImageLayers(image,FlattenLayer,exception);
      ReplaceImageInList(&image,merged);
    }
  if (profile != (StringInfo *) NULL)
    {
      (void) SetImageProfile(image,GetStringInfoName(profile),profile,
        exception);
      profile=DestroyStringInfo(profile);
    }
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

  entry=AcquireMagickInfo("PSD","PSB","Adobe Large Document Format");
  entry->decoder=(DecodeImageHandler *) ReadPSDImage;
  entry->encoder=(EncodeImageHandler *) WritePSDImage;
  entry->magick=(IsImageFormatHandler *) IsPSD;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PSD","PSD","Adobe Photoshop bitmap");
  entry->decoder=(DecodeImageHandler *) ReadPSDImage;
  entry->encoder=(EncodeImageHandler *) WritePSDImage;
  entry->magick=(IsImageFormatHandler *) IsPSD;
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
%      MagickBooleanType WritePSDImage(const ImageInfo *image_info,Image *image,
%        ExceptionInfo *exception)
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

static inline ssize_t SetPSDOffset(const PSDInfo *psd_info,Image *image,
  const size_t offset)
{
  if (psd_info->version == 1)
    return(WriteBlobMSBShort(image,(unsigned short) offset));
  return(WriteBlobMSBLong(image,(unsigned int) offset));
}

static inline ssize_t WritePSDOffset(const PSDInfo *psd_info,Image *image,
  const MagickSizeType size,const MagickSizeType offset)
{
  MagickSizeType
    current_offset;

  ssize_t
    result;

  current_offset=TellBlob(image);
  SeekBlob(image,offset,SEEK_SET);
  if (psd_info->version == 1)
    result=WriteBlobMSBShort(image,(unsigned short) size);
  else
    result=WriteBlobMSBLong(image,(unsigned int) size);
  SeekBlob(image,current_offset,SEEK_SET);
  return(result);
}

static inline ssize_t SetPSDSize(const PSDInfo *psd_info,Image *image,
  const MagickSizeType size)
{
  if (psd_info->version == 1)
    return(WriteBlobLong(image,(unsigned int) size));
  return(WriteBlobLongLong(image,size));
}

static inline ssize_t WritePSDSize(const PSDInfo *psd_info,Image *image,
  const MagickSizeType size,const MagickSizeType offset)
{
  MagickSizeType
    current_offset;

  ssize_t
    result;

  current_offset=TellBlob(image);
  SeekBlob(image,offset,SEEK_SET);
  result=SetPSDSize(psd_info, image, size);
  SeekBlob(image,current_offset,SEEK_SET);
  return(result);
}

static size_t PSDPackbitsEncodeImage(Image *image,const size_t length,
  const unsigned char *pixels,unsigned char *compact_pixels,
  ExceptionInfo *exception)
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
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(pixels != (unsigned char *) NULL);
  assert(compact_pixels != (unsigned char *) NULL);
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

static size_t WriteCompressionStart(const PSDInfo *psd_info,Image *image,
  const Image *next_image,const CompressionType compression,
  const ssize_t channels)
{
  size_t
    length;

  ssize_t
    i,
    y;

  if (compression == RLECompression)
    {
      length=WriteBlobShort(image,RLE);
      for (i=0; i < channels; i++)
        for (y=0; y < (ssize_t) next_image->rows; y++)
          length+=SetPSDOffset(psd_info,image,0);
    }
#ifdef MAGICKCORE_ZLIB_DELEGATE
  else if (compression == ZipCompression)
    length=WriteBlobShort(image,ZipWithoutPrediction);
#endif
  else
    length=WriteBlobShort(image,Raw);
  return(length);
}

static size_t WritePSDChannel(const PSDInfo *psd_info,
  const ImageInfo *image_info,Image *image,Image *next_image,
  const QuantumType quantum_type, unsigned char *compact_pixels,
  MagickOffsetType size_offset,const MagickBooleanType separate,
  const CompressionType compression,ExceptionInfo *exception)
{
  int
    y;

  MagickBooleanType
    monochrome;

  QuantumInfo
    *quantum_info;

  register const Quantum
    *p;

  register ssize_t
    i;

  size_t
    count,
    length;

  unsigned char
    *pixels;

#ifdef MAGICKCORE_ZLIB_DELEGATE

#define CHUNK 16384

  int
    flush,
    level;

  unsigned char
    *compressed_pixels;

  z_stream
    stream;

  compressed_pixels=(unsigned char *) NULL;
  flush=Z_NO_FLUSH;
#endif
  count=0;
  if (separate != MagickFalse)
    {
      size_offset=TellBlob(image)+2;
      count+=WriteCompressionStart(psd_info,image,next_image,compression,1);
    }
  if (next_image->depth > 8)
    next_image->depth=16;
  monochrome=IsImageMonochrome(image) && (image->depth == 1) ?
    MagickTrue : MagickFalse;
  quantum_info=AcquireQuantumInfo(image_info,next_image);
  if (quantum_info == (QuantumInfo *) NULL)
    return(0);
  pixels=(unsigned char *) GetQuantumPixels(quantum_info);
#ifdef MAGICKCORE_ZLIB_DELEGATE
  if (compression == ZipCompression)
    {
      compressed_pixels=(unsigned char *) AcquireQuantumMemory(CHUNK,
        sizeof(*compressed_pixels));
      if (compressed_pixels == (unsigned char *) NULL)
        {
          quantum_info=DestroyQuantumInfo(quantum_info);
          return(0);
        }
      memset(&stream,0,sizeof(stream));
      stream.data_type=Z_BINARY;
      level=Z_DEFAULT_COMPRESSION;
      if ((image_info->quality > 0 && image_info->quality < 10))
        level=(int) image_info->quality;
      if (deflateInit(&stream,level) != Z_OK)
        {
          quantum_info=DestroyQuantumInfo(quantum_info);
          return(0);
        }
    }
#endif
  for (y=0; y < (ssize_t) next_image->rows; y++)
  {
    p=GetVirtualPixels(next_image,0,y,next_image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    length=ExportQuantumPixels(next_image,(CacheView *) NULL,quantum_info,
      quantum_type,pixels,exception);
    if (monochrome != MagickFalse)
      for (i=0; i < (ssize_t) length; i++)
        pixels[i]=(~pixels[i]);
    if (compression == RLECompression)
      {
        length=PSDPackbitsEncodeImage(image,length,pixels,compact_pixels,
          exception);
        count+=WriteBlob(image,length,compact_pixels);
        size_offset+=WritePSDOffset(psd_info,image,length,size_offset);
      }
#ifdef MAGICKCORE_ZLIB_DELEGATE
    else if (compression == ZipCompression)
      {
        stream.avail_in=(uInt) length;
        stream.next_in=(Bytef *) pixels;
        if (y == (ssize_t) next_image->rows-1)
          flush=Z_FINISH;
        do {
            stream.avail_out=(uInt) CHUNK;
            stream.next_out=(Bytef *) compressed_pixels;
            if (deflate(&stream,flush) == Z_STREAM_ERROR)
              break;
            length=(size_t) CHUNK-stream.avail_out;
            if (length > 0)
              count+=WriteBlob(image,length,compressed_pixels);
        } while (stream.avail_out == 0);
      }
#endif
    else
      count+=WriteBlob(image,length,pixels);
  }
#ifdef MAGICKCORE_ZLIB_DELEGATE
  if (compression == ZipCompression)
    {
      (void) deflateEnd(&stream);
      compressed_pixels=(unsigned char *) RelinquishMagickMemory(
        compressed_pixels);
    }
#endif
  quantum_info=DestroyQuantumInfo(quantum_info);
  return(count);
}

static unsigned char *AcquireCompactPixels(const Image *image,
  ExceptionInfo *exception)
{
  size_t
    packet_size;

  unsigned char
    *compact_pixels;

  packet_size=image->depth > 8UL ? 2UL : 1UL;
  compact_pixels=(unsigned char *) AcquireQuantumMemory((9*
    image->columns)+1,packet_size*sizeof(*compact_pixels));
  if (compact_pixels == (unsigned char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
    }
  return(compact_pixels);
}

static size_t WritePSDChannels(const PSDInfo *psd_info,
  const ImageInfo *image_info,Image *image,Image *next_image,
  MagickOffsetType size_offset,const MagickBooleanType separate,
  ExceptionInfo *exception)
{
  CompressionType
    compression;

  Image
    *mask;

  MagickOffsetType
    rows_offset;

  size_t
    channels,
    count,
    length,
    offset_length;

  unsigned char
    *compact_pixels;

  count=0;
  offset_length=0;
  rows_offset=0;
  compact_pixels=(unsigned char *) NULL;
  compression=next_image->compression;
  if (image_info->compression != UndefinedCompression)
    compression=image_info->compression;
  if (compression == RLECompression)
    {
      compact_pixels=AcquireCompactPixels(next_image,exception);
      if (compact_pixels == (unsigned char *) NULL)
        return(0);
    }
  channels=1;
  if (separate == MagickFalse)
    {
      if (next_image->storage_class != PseudoClass)
        {
          if (IsImageGray(next_image) == MagickFalse)
            channels=next_image->colorspace == CMYKColorspace ? 4 : 3;
          if (next_image->alpha_trait != UndefinedPixelTrait)
            channels++;
        }
      rows_offset=TellBlob(image)+2;
      count+=WriteCompressionStart(psd_info,image,next_image,compression,
        channels);
      offset_length=(next_image->rows*(psd_info->version == 1 ? 2 : 4));
    }
  size_offset+=2;
  if (next_image->storage_class == PseudoClass)
    {
      length=WritePSDChannel(psd_info,image_info,image,next_image,
        IndexQuantum,compact_pixels,rows_offset,separate,compression,
        exception);
      if (separate != MagickFalse)
        size_offset+=WritePSDSize(psd_info,image,length,size_offset)+2;
      else
        rows_offset+=offset_length;
      count+=length;
    }
  else
    {
      if (IsImageGray(next_image) != MagickFalse)
        {
          length=WritePSDChannel(psd_info,image_info,image,next_image,
            GrayQuantum,compact_pixels,rows_offset,separate,compression,
            exception);
          if (separate != MagickFalse)
            size_offset+=WritePSDSize(psd_info,image,length,size_offset)+2;
          else
            rows_offset+=offset_length;
          count+=length;
        }
      else
        {
          if (next_image->colorspace == CMYKColorspace)
            (void) NegateCMYK(next_image,exception);

          length=WritePSDChannel(psd_info,image_info,image,next_image,
            RedQuantum,compact_pixels,rows_offset,separate,compression,
            exception);
          if (separate != MagickFalse)
            size_offset+=WritePSDSize(psd_info,image,length,size_offset)+2;
          else
            rows_offset+=offset_length;
          count+=length;

          length=WritePSDChannel(psd_info,image_info,image,next_image,
            GreenQuantum,compact_pixels,rows_offset,separate,compression,
            exception);
          if (separate != MagickFalse)
            size_offset+=WritePSDSize(psd_info,image,length,size_offset)+2;
          else
            rows_offset+=offset_length;
          count+=length;

          length=WritePSDChannel(psd_info,image_info,image,next_image,
            BlueQuantum,compact_pixels,rows_offset,separate,compression,
            exception);
          if (separate != MagickFalse)
            size_offset+=WritePSDSize(psd_info,image,length,size_offset)+2;
          else
            rows_offset+=offset_length;
          count+=length;

          if (next_image->colorspace == CMYKColorspace)
            {
              length=WritePSDChannel(psd_info,image_info,image,next_image,
                BlackQuantum,compact_pixels,rows_offset,separate,compression,
                exception);
              if (separate != MagickFalse)
                size_offset+=WritePSDSize(psd_info,image,length,size_offset)+2;
              else
                rows_offset+=offset_length;
              count+=length;
            }
        }
      if (next_image->alpha_trait != UndefinedPixelTrait)
        {
          length=WritePSDChannel(psd_info,image_info,image,next_image,
            AlphaQuantum,compact_pixels,rows_offset,separate,compression,
            exception);
          if (separate != MagickFalse)
            size_offset+=WritePSDSize(psd_info,image,length,size_offset)+2;
          else
            rows_offset+=offset_length;
          count+=length;
        }
    }
  compact_pixels=(unsigned char *) RelinquishMagickMemory(compact_pixels);
  if (next_image->colorspace == CMYKColorspace)
    (void) NegateCMYK(next_image,exception);
  if (separate != MagickFalse)
    {
      const char
        *property;

      property=GetImageArtifact(next_image,"psd:opacity-mask");
      if (property != (const char *) NULL)
        {
          mask=(Image *) GetImageRegistry(ImageRegistryType,property,
            exception);
          if (mask != (Image *) NULL)
            {
              if (compression == RLECompression)
                {
                  compact_pixels=AcquireCompactPixels(mask,exception);
                  if (compact_pixels == (unsigned char *) NULL)
                    return(0);
                }
              length=WritePSDChannel(psd_info,image_info,image,mask,
                RedQuantum,compact_pixels,rows_offset,MagickTrue,compression,
                exception);
              (void) WritePSDSize(psd_info,image,length,size_offset);
              count+=length;
              compact_pixels=(unsigned char *) RelinquishMagickMemory(
                compact_pixels);
            }
        }
    }
  return(count);
}

static size_t WritePascalString(Image *image,const char *value,size_t padding)
{
  size_t
    count,
    length;

  register ssize_t
    i;

  /*
    Max length is 255.
  */
  count=0;
  length=(strlen(value) > 255UL ) ? 255UL : strlen(value);
  if (length ==  0)
    count+=WriteBlobByte(image,0);
  else
    {
      count+=WriteBlobByte(image,(unsigned char) length);
      count+=WriteBlob(image,length,(const unsigned char *) value);
    }
  length++;
  if ((length % padding) == 0)
    return(count);
  for (i=0; i < (ssize_t) (padding-(length % padding)); i++)
    count+=WriteBlobByte(image,0);
  return(count);
}

static void WriteResolutionResourceBlock(Image *image)
{
  double
    x_resolution,
    y_resolution;

  unsigned short
    units;

  if (image->units == PixelsPerCentimeterResolution)
    {
      x_resolution=2.54*65536.0*image->resolution.x+0.5;
      y_resolution=2.54*65536.0*image->resolution.y+0.5;
      units=2;
    }
  else
    {
      x_resolution=65536.0*image->resolution.x+0.5;
      y_resolution=65536.0*image->resolution.y+0.5;
      units=1;
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

static inline size_t WriteChannelSize(const PSDInfo *psd_info,Image *image,
  const signed short channel)
{
  size_t
    count;

  count=(size_t) WriteBlobShort(image,channel);
  count+=SetPSDSize(psd_info,image,0);
  return(count);
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
        ssize_t
          quantum;

        quantum=PSDQuantum(count)+12;
        if ((quantum >= 12) && (quantum < (ssize_t) length))
          {
            if ((q+quantum < (datum+length-16)))
              (void) memmove(q,q+quantum,length-quantum-(q-datum));
            SetStringInfoLength(bim_profile,length-quantum);
          }
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

    ssize_t
      cnt;

    q=(unsigned char *) p;
    if (LocaleNCompare((const char *) p,"8BIM",4) != 0)
      return;
    p=PushLongPixel(MSBEndian,p,&long_sans);
    p=PushShortPixel(MSBEndian,p,&id);
    p=PushShortPixel(MSBEndian,p,&short_sans);
    p=PushLongPixel(MSBEndian,p,&count);
    cnt=PSDQuantum(count);
    if (cnt < 0)
      return;
    if ((id == 0x000003ed) && (cnt < (ssize_t) (length-12)) &&
        ((ssize_t) length-(cnt+12)-(q-datum)) > 0)
      {
        (void) memmove(q,q+cnt+12,length-(cnt+12)-(q-datum));
        SetStringInfoLength(bim_profile,length-(cnt+12));
        break;
      }
    p+=count;
    if ((count & 0x01) != 0)
      p++;
  }
}

static const StringInfo *GetAdditionalInformation(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
#define PSDKeySize 5
#define PSDAllowedLength 36

  char
    key[PSDKeySize];

  /* Whitelist of keys from: https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/ */
  const char
    allowed[PSDAllowedLength][PSDKeySize] = {
      "blnc", "blwh", "brit", "brst", "clbl", "clrL", "curv", "expA", "FMsk",
      "GdFl", "grdm", "hue ", "hue2", "infx", "knko", "lclr", "levl", "lnsr",
      "lfx2", "luni", "lrFX", "lspf", "lyid", "lyvr", "mixr", "nvrt", "phfl",
      "post", "PtFl", "selc", "shpa", "sn2P", "SoCo", "thrs", "tsly", "vibA"
    },
    *option;

  const StringInfo
    *info;

  MagickBooleanType
    found;

  register size_t
    i;

  size_t
    remaining_length,
    length;

  StringInfo
    *profile;

  unsigned char
    *p;

  unsigned int
    size;

  info=GetImageProfile(image,"psd:additional-info");
  if (info == (const StringInfo *) NULL)
    return((const StringInfo *) NULL);
  option=GetImageOption(image_info,"psd:additional-info");
  if (LocaleCompare(option,"all") == 0)
    return(info);
  if (LocaleCompare(option,"selective") != 0)
    {
      profile=RemoveImageProfile(image,"psd:additional-info");
      return(DestroyStringInfo(profile));
    }
  length=GetStringInfoLength(info);
  p=GetStringInfoDatum(info);
  remaining_length=length;
  length=0;
  while (remaining_length >= 12)
  {
    /* skip over signature */
    p+=4;
    key[0]=(*p++);
    key[1]=(*p++);
    key[2]=(*p++);
    key[3]=(*p++);
    key[4]='\0';
    size=(unsigned int) (*p++) << 24;
    size|=(unsigned int) (*p++) << 16;
    size|=(unsigned int) (*p++) << 8;
    size|=(unsigned int) (*p++);
    size=size & 0xffffffff;
    remaining_length-=12;
    if ((size_t) size > remaining_length)
      return((const StringInfo *) NULL);
    found=MagickFalse;
    for (i=0; i < PSDAllowedLength; i++)
    {
      if (LocaleNCompare(key,allowed[i],PSDKeySize) != 0)
        continue;

      found=MagickTrue;
      break;
    }
    remaining_length-=(size_t) size;
    if (found == MagickFalse)
      {
        if (remaining_length > 0)
          p=(unsigned char *) memmove(p-12,p+size,remaining_length);
        continue;
      }
    length+=(size_t) size+12;
    p+=size;
  }
  profile=RemoveImageProfile(image,"psd:additional-info");
  if (length == 0)
    return(DestroyStringInfo(profile));
  SetStringInfoLength(profile,(const size_t) length);
  SetImageProfile(image,"psd:additional-info",info,exception);
  return(profile);
}

static MagickBooleanType WritePSDLayersInternal(Image *image,
  const ImageInfo *image_info,const PSDInfo *psd_info,size_t *layers_size,
  ExceptionInfo *exception)
{
  char
    layer_name[MagickPathExtent];

  const char
    *property;

  const StringInfo
    *info;

  Image
    *base_image,
    *next_image;

  MagickBooleanType
    status;

  MagickOffsetType
    *layer_size_offsets,
    size_offset;

  register ssize_t
    i;

  size_t
    layer_count,
    layer_index,
    length,
    name_length,
    rounded_size,
    size;

  status=MagickTrue;
  base_image=GetNextImageInList(image);
  if (base_image == (Image *) NULL)
    base_image=image;
  size=0;
  size_offset=TellBlob(image);
  SetPSDSize(psd_info,image,0);
  layer_count=0;
  for (next_image=base_image; next_image != NULL; )
  {
    layer_count++;
    next_image=GetNextImageInList(next_image);
  }
  if (image->alpha_trait != UndefinedPixelTrait)
    size+=WriteBlobShort(image,-(unsigned short) layer_count);
  else
    size+=WriteBlobShort(image,(unsigned short) layer_count);
  layer_size_offsets=(MagickOffsetType *) AcquireQuantumMemory(
    (size_t) layer_count,sizeof(MagickOffsetType));
  if (layer_size_offsets == (MagickOffsetType *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  layer_index=0;
  for (next_image=base_image; next_image != NULL; )
  {
    Image
      *mask;

    unsigned char
      default_color;

    unsigned short
      channels,
      total_channels;

    mask=(Image *) NULL;
    property=GetImageArtifact(next_image,"psd:opacity-mask");
    default_color=0;
    if (property != (const char *) NULL)
      {
        mask=(Image *) GetImageRegistry(ImageRegistryType,property,exception);
        default_color=strlen(property) == 9 ? 255 : 0;
      }
    size+=WriteBlobSignedLong(image,(signed int) next_image->page.y);
    size+=WriteBlobSignedLong(image,(signed int) next_image->page.x);
    size+=WriteBlobSignedLong(image,(signed int) (next_image->page.y+
      next_image->rows));
    size+=WriteBlobSignedLong(image,(signed int) (next_image->page.x+
      next_image->columns));
    channels=1U;
    if ((next_image->storage_class != PseudoClass) &&
        (IsImageGray(next_image) == MagickFalse))
      channels=next_image->colorspace == CMYKColorspace ? 4U : 3U;
    total_channels=channels;
    if (next_image->alpha_trait != UndefinedPixelTrait)
      total_channels++;
    if (mask != (Image *) NULL)
      total_channels++;
    size+=WriteBlobShort(image,total_channels);
    layer_size_offsets[layer_index++]=TellBlob(image);
    for (i=0; i < (ssize_t) channels; i++)
      size+=WriteChannelSize(psd_info,image,(signed short) i);
    if (next_image->alpha_trait != UndefinedPixelTrait)
      size+=WriteChannelSize(psd_info,image,-1);
    if (mask != (Image *) NULL)
      size+=WriteChannelSize(psd_info,image,-2);
    size+=WriteBlobString(image,image->endian == LSBEndian ? "MIB8" :"8BIM");
    size+=WriteBlobString(image,CompositeOperatorToPSDBlendMode(next_image));
    property=GetImageArtifact(next_image,"psd:layer.opacity");
    if (property != (const char *) NULL)
      {
        Quantum
          opacity;

        opacity=(Quantum) StringToInteger(property);
        size+=WriteBlobByte(image,ScaleQuantumToChar(opacity));
        (void) ApplyPSDLayerOpacity(next_image,opacity,MagickTrue,exception);
      }
    else
      size+=WriteBlobByte(image,255);
    size+=WriteBlobByte(image,0);
    size+=WriteBlobByte(image,next_image->compose==NoCompositeOp ?
      1 << 0x02 : 1); /* layer properties - visible, etc. */
    size+=WriteBlobByte(image,0);
    info=GetAdditionalInformation(image_info,next_image,exception);
    property=(const char *) GetImageProperty(next_image,"label",exception);
    if (property == (const char *) NULL)
      {
        (void) FormatLocaleString(layer_name,MagickPathExtent,"L%.20g",
          (double) layer_index);
        property=layer_name;
      }
    name_length=strlen(property)+1;
    if ((name_length % 4) != 0)
      name_length+=(4-(name_length % 4));
    if (info != (const StringInfo *) NULL)
      name_length+=GetStringInfoLength(info);
    name_length+=8;
    if (mask != (Image *) NULL)
      name_length+=20;
    size+=WriteBlobLong(image,(unsigned int) name_length);
    if (mask == (Image *) NULL)
      size+=WriteBlobLong(image,0);
    else
      {
        if (mask->compose != NoCompositeOp)
          (void) ApplyPSDOpacityMask(next_image,mask,ScaleCharToQuantum(
            default_color),MagickTrue,exception);
        mask->page.y+=image->page.y;
        mask->page.x+=image->page.x;
        size+=WriteBlobLong(image,20);
        size+=WriteBlobSignedLong(image,mask->page.y);
        size+=WriteBlobSignedLong(image,mask->page.x);
        size+=WriteBlobSignedLong(image,(const signed int) mask->rows+
          mask->page.y);
        size+=WriteBlobSignedLong(image,(const signed int) mask->columns+
          mask->page.x);
        size+=WriteBlobByte(image,default_color);
        size+=WriteBlobByte(image,mask->compose == NoCompositeOp ? 2 : 0);
        size+=WriteBlobMSBShort(image,0);
      }
    size+=WriteBlobLong(image,0);
    size+=WritePascalString(image,property,4);
    if (info != (const StringInfo *) NULL)
      size+=WriteBlob(image,GetStringInfoLength(info),
        GetStringInfoDatum(info));
    next_image=GetNextImageInList(next_image);
  }
  /*
    Now the image data!
  */
  next_image=base_image;
  layer_index=0;
  while (next_image != NULL)
  {
    length=WritePSDChannels(psd_info,image_info,image,next_image,
      layer_size_offsets[layer_index++],MagickTrue,exception);
    if (length == 0)
      {
        status=MagickFalse;
        break;
      }
    size+=length;
    next_image=GetNextImageInList(next_image);
  }
  /*
    Write the total size
  */
  if (layers_size != (size_t*) NULL)
    *layers_size=size;
  if ((size/2) != ((size+1)/2))
    rounded_size=size+1;
  else
    rounded_size=size;
  (void) WritePSDSize(psd_info,image,rounded_size,size_offset);
  layer_size_offsets=(MagickOffsetType *) RelinquishMagickMemory(
    layer_size_offsets);
  /*
    Remove the opacity mask from the registry
  */
  next_image=base_image;
  while (next_image != (Image *) NULL)
  {
    property=GetImageArtifact(next_image,"psd:opacity-mask");
    if (property != (const char *) NULL)
      DeleteImageRegistry(property);
    next_image=GetNextImageInList(next_image);
  }

  return(status);
}

ModuleExport MagickBooleanType WritePSDLayers(Image * image,
  const ImageInfo *image_info,const PSDInfo *psd_info,ExceptionInfo *exception)
{
  PolicyDomain
    domain;

  PolicyRights
    rights;

  domain=CoderPolicyDomain;
  rights=WritePolicyRights;
  if (IsRightsAuthorized(domain,rights,"PSD") == MagickFalse)
    return(MagickTrue);
  return WritePSDLayersInternal(image,image_info,psd_info,(size_t*) NULL,
    exception);
}

static MagickBooleanType WritePSDImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  const StringInfo
    *icc_profile;

  MagickBooleanType
    status;

  PSDInfo
    psd_info;

  register ssize_t
    i;

  size_t
    length,
    num_channels,
    packet_size;

  StringInfo
    *bim_profile;

  /*
    Open image file.
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
  packet_size=(size_t) (image->depth > 8 ? 6 : 3);
  if (image->alpha_trait != UndefinedPixelTrait)
    packet_size+=image->depth > 8 ? 2 : 1;
  psd_info.version=1;
  if ((LocaleCompare(image_info->magick,"PSB") == 0) ||
      (image->columns > 30000) || (image->rows > 30000))
    psd_info.version=2;
  (void) WriteBlob(image,4,(const unsigned char *) "8BPS");
  (void) WriteBlobMSBShort(image,psd_info.version);  /* version */
  for (i=1; i <= 6; i++)
    (void) WriteBlobByte(image, 0);  /* 6 bytes of reserved */
  /* When the image has a color profile it won't be converted to gray scale */
  if ((GetImageProfile(image,"icc") == (StringInfo *) NULL) &&
      (SetImageGray(image,exception) != MagickFalse))
    num_channels=(image->alpha_trait != UndefinedPixelTrait ? 2UL : 1UL);
  else
    if ((image_info->type != TrueColorType) && (image_info->type !=
         TrueColorAlphaType) && (image->storage_class == PseudoClass))
      num_channels=(image->alpha_trait != UndefinedPixelTrait ? 2UL : 1UL);
    else
      {
        if (image->storage_class == PseudoClass)
          (void) SetImageStorageClass(image,DirectClass,exception);
        if (image->colorspace != CMYKColorspace)
          num_channels=(image->alpha_trait != UndefinedPixelTrait ? 4UL : 3UL);
        else
          num_channels=(image->alpha_trait != UndefinedPixelTrait ? 5UL : 4UL);
      }
  (void) WriteBlobMSBShort(image,(unsigned short) num_channels);
  (void) WriteBlobMSBLong(image,(unsigned int) image->rows);
  (void) WriteBlobMSBLong(image,(unsigned int) image->columns);
  if (IsImageGray(image) != MagickFalse)
    {
      MagickBooleanType
        monochrome;

      /*
        Write depth & mode.
      */
      monochrome=IsImageMonochrome(image) && (image->depth == 1) ?
        MagickTrue : MagickFalse;
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
          (void) TransformImageColorspace(image,sRGBColorspace,exception);
          (void) WriteBlobMSBShort(image,(unsigned short)
            (image->storage_class == PseudoClass ? IndexedMode : RGBMode));
        }
      else
        {
          if (image->colorspace != CMYKColorspace)
            (void) TransformImageColorspace(image,CMYKColorspace,exception);
          (void) WriteBlobMSBShort(image,CMYKMode);
        }
    }
  if ((IsImageGray(image) != MagickFalse) ||
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
  if (status != MagickFalse)
    {
      MagickOffsetType
        size_offset;

      size_t
        size;

      size_offset=TellBlob(image);
      SetPSDSize(&psd_info,image,0);
      status=WritePSDLayersInternal(image,image_info,&psd_info,&size,
        exception);
      size_offset+=WritePSDSize(&psd_info,image,size+
        (psd_info.version == 1 ? 8 : 12),size_offset);
    }
  (void) WriteBlobMSBLong(image,0);  /* user mask data */
  /*
    Write composite image.
  */
  if (status != MagickFalse)
    {
      CompressionType
        compression;

      compression=image->compression;
      if (image->compression == ZipCompression)
        image->compression=RLECompression;
      if (image_info->compression != UndefinedCompression)
        image->compression=image_info->compression;
      if (WritePSDChannels(&psd_info,image_info,image,image,0,MagickFalse,
          exception) == 0)
        status=MagickFalse;
      image->compression=compression;
    }
  (void) CloseBlob(image);
  return(status);
}

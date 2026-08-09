/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%            AAA    SSSSS  EEEEE  PPPP   RRRR  IIIII  TTTTT  EEEEE            %
%           A   A   S      E      P   P  R   R   I      T    E                %
%           AAAAA   SSSSS  EEEEE  PPPP   RRRR    I      T    EEEEE            %
%           A   A       S  E      P      R R     I      T    E                %
%           A   A   SSSSS  EEEEE  P      R  R  IIIII    T    EEEEE            %
%                                                                             %
%                                                                             %
%                      Read/Write Aseprite Sprite Format                      %
%                                                                             %
%                              Software Design                                %
%                                Dirk Lemstra                                 %
%                                 August 2026                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
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
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#if defined(MAGICKCORE_ZLIB_DELEGATE)
#include <zlib.h>
#endif

/*
  Define declarations.
*/
#define ASE_MAGIC 0xA5E0
#define ASE_FRAME_MAGIC 0xF1FA

/*
  Typedef declarations.
*/
typedef struct _AsepriteHeader
{
  uint32_t file_size;
  uint16_t frames;
  uint16_t width;
  uint16_t height;
  uint16_t color_depth;
  uint32_t flags;
  uint16_t speed;
  uint32_t padding1;
  uint32_t padding2;
  uint8_t palette_index;
  uint8_t ignore1;
  uint8_t ignore2;
  uint8_t ignore3;
  uint16_t pixel_width;
  uint16_t pixel_height;
} AsepriteHeader;

typedef struct _AsepriteFrame
{
  uint32_t frame_size;
  uint16_t frame_magic;
  uint16_t chunk_count;
  uint16_t duration;
  uint8_t padding1;
  uint8_t padding2;
  uint32_t next_frame;
} AsepriteFrame;

typedef struct _AsepriteChunk
{
  uint32_t chunk_size;
  uint16_t chunk_type;
} AsepriteChunk;

typedef struct _AsepriteLayer
{
  MagickBooleanType visible;
  uint16_t blend_mode;
  uint8_t opacity;
} AsepriteLayer;

/*
  AsepriteLayerList is a growable array of AsepriteLayer entries indexed by
  layer index, used to resolve a cel's referencing layer's visibility,
  blend mode and opacity.
*/
typedef struct _AsepriteLayerList
{
  AsepriteLayer *layers;
  size_t count;
  size_t capacity;
} AsepriteLayerList;

/*
  AsepriteCelCache caches the decompressed pixels (and placement/opacity) of
  every Raw/Compressed cel keyed by (frame_index,layer_index), so a later
  Linked Cel (cel_type==1) referencing an earlier frame's cel for the same
  layer can be resolved and composited identically to a direct cel.
*/
typedef struct _AsepriteCelCacheEntry
{
  size_t frame_index;
  uint16_t layer_index;
  int16_t cel_x;
  int16_t cel_y;
  uint16_t cel_width;
  uint16_t cel_height;
  uint8_t cel_opacity;
  uint8_t *pixels;
} AsepriteCelCacheEntry;

typedef struct _AsepriteCelCache
{
  AsepriteCelCacheEntry *entries;
  size_t count;
  size_t capacity;
} AsepriteCelCache;

/*
  AsepriteCanvas bundles the frame's decoded pixel buffer with its
  dimensions, color depth and transparent-index, so compositing helpers
  only need a single parameter instead of five.
*/
typedef struct _AsepriteCanvas
{
  uint8_t *pixels;
  size_t width;
  size_t height;
  size_t color_depth;
  uint8_t transparent_index;
} AsepriteCanvas;

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteASEImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d A S E I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadASEImage() reads an Aseprite sprite file and returns it.
%
%  The format of the ReadASEImage method is:
%
%      Image *ReadASEImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static void DestroyASELayerList(AsepriteLayerList *layers)
{
  layers->layers=(AsepriteLayer *) RelinquishMagickMemory(layers->layers);
  layers->count=0;
  layers->capacity=0;
}

static void DestroyASECelCache(AsepriteCelCache *cel_cache)
{
  size_t
    i;

  if (cel_cache->entries == (AsepriteCelCacheEntry *) NULL)
    return;
  for (i=0; i < cel_cache->count; i++)
    cel_cache->entries[i].pixels=(uint8_t *) RelinquishMagickMemory(
      cel_cache->entries[i].pixels);
  cel_cache->entries=(AsepriteCelCacheEntry *) RelinquishMagickMemory(
    cel_cache->entries);
  cel_cache->count=0;
  cel_cache->capacity=0;
}

static void ReadASEPaletteChunk(Image *image,const uint8_t *chunk_data,
  size_t payload_size)
{
  size_t
    first_index,
    i,
    last_index,
    new_size,
    offset;

  if (payload_size < 20)
    return;
  new_size=(size_t) (chunk_data[0] | (chunk_data[1] << 8) |
    (chunk_data[2] << 16) | (chunk_data[3] << 24));
  first_index=(size_t) (chunk_data[4] | (chunk_data[5] << 8) |
    (chunk_data[6] << 16) | (chunk_data[7] << 24));
  last_index=(size_t) (chunk_data[8] | (chunk_data[9] << 8) |
    (chunk_data[10] << 16) | (chunk_data[11] << 24));
  if ((last_index < 256) && (last_index+1 > new_size))
    new_size=last_index+1;
  if ((new_size > 0) && (new_size <= 256))
    image->colors=new_size;
  offset=20;
  for (i=first_index; (i <= last_index) && (i < 256); i++)
  {
    uint16_t
      flags;

    if ((offset+6) > payload_size)
      break;
    flags=(uint16_t) (chunk_data[offset] | (chunk_data[offset+1] << 8));
    image->colormap[i].red=(MagickRealType) ScaleCharToQuantum(
      chunk_data[offset+2]);
    image->colormap[i].green=(MagickRealType) ScaleCharToQuantum(
      chunk_data[offset+3]);
    image->colormap[i].blue=(MagickRealType) ScaleCharToQuantum(
      chunk_data[offset+4]);
    image->colormap[i].alpha=(MagickRealType) ScaleCharToQuantum(
      chunk_data[offset+5]);
    offset+=6;
    if ((flags & 0x01) != 0)
      {
        uint16_t
          name_length;

        if ((offset+2) > payload_size)
          break;
        name_length=(uint16_t) (chunk_data[offset] |
          (chunk_data[offset+1] << 8));
        offset+=2+name_length;
      }
  }
}

/*
  ReadASELayerChunk() reads a layer chunk (0x2004) and appends its
  visibility, blend mode and opacity to the growable layers array, so cels
  can later be composited according to their referencing layer's index.
  Per the header's flags field, the opacity value is only meaningful if
  bit 0 ("Layer opacity has valid value") is set; otherwise the layer is
  treated as fully opaque.
*/
static MagickBooleanType ReadASELayerChunk(const Image *image,
  const uint8_t *chunk_data,size_t payload_size,
  MagickBooleanType opacity_is_valid,AsepriteLayerList *layers,
  ExceptionInfo *exception)
{
  AsepriteLayer
    *layer;

  if (payload_size < 18)
    return(MagickTrue);
  if (layers->count == layers->capacity)
    {
      size_t
        new_capacity;

      AsepriteLayer
        *new_layers;

      new_capacity=(layers->capacity == 0) ? 16 : (layers->capacity*2);
      new_layers=(AsepriteLayer *) ResizeQuantumMemory(layers->layers,
        new_capacity,sizeof(*new_layers));
      if (new_layers == (AsepriteLayer *) NULL)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            image->filename);
          return(MagickFalse);
        }
      layers->layers=new_layers;
      layers->capacity=new_capacity;
    }
  layer=layers->layers+layers->count;
  layer->visible=((chunk_data[0] & 0x01) != 0) ? MagickTrue : MagickFalse;
  layer->blend_mode=(uint16_t) (chunk_data[10] | (chunk_data[11] << 8));
  layer->opacity=(opacity_is_valid != MagickFalse) ? chunk_data[12] : 255;
  layers->count++;
  return(MagickTrue);
}

/*
  CompositeASECel() alpha-blends a single decompressed cel's pixel buffer
  onto the frame canvas at the given position/opacity ("over" compositing).
  Used for both direct cels (Raw/Compressed) and cels resolved via a Linked
  Cel reference, so both paths render identically.
*/
static void CompositeASECel(const uint8_t *cel_pixels,uint16_t cel_width,
  uint16_t cel_height,int16_t cel_x,int16_t cel_y,double combined_opacity,
  AsepriteCanvas *canvas)
{
  size_t
    bytes_per_pixel,
    x,
    y;

  bytes_per_pixel=canvas->color_depth/8;
  for (y=0; y < (size_t) cel_height; y++)
  {
    ssize_t
      canvas_y;

    canvas_y=(ssize_t) cel_y+(ssize_t) y;
    if ((canvas_y < 0) || (canvas_y >= (ssize_t) canvas->height))
      continue;
    for (x=0; x < (size_t) cel_width; x++)
    {
      ssize_t
        canvas_x;

      const uint8_t
        *src;

      uint8_t
        *dst;

      canvas_x=(ssize_t) cel_x+(ssize_t) x;
      if ((canvas_x < 0) || (canvas_x >= (ssize_t) canvas->width))
        continue;
      src=cel_pixels+(y*cel_width+x)*bytes_per_pixel;
      dst=canvas->pixels+((size_t) canvas_y*canvas->width+(size_t) canvas_x)*
        bytes_per_pixel;
      switch (canvas->color_depth)
      {
        /* RGBA: alpha blend the cel over the canvas ("over" compositing). */
        case 32:
        {
          double
            dst_alpha,
            out_alpha,
            src_alpha;

          size_t
            channel;

          src_alpha=((double) src[3]/255.0)*combined_opacity;
          dst_alpha=(double) dst[3]/255.0;
          out_alpha=src_alpha+dst_alpha*(1.0-src_alpha);
          for (channel=0; channel < 3; channel++)
          {
            double
              value;

            value=out_alpha > 0.0 ? (src[channel]*src_alpha+dst[channel]*
              dst_alpha*(1.0-src_alpha))/out_alpha : 0.0;
            dst[channel]=(uint8_t) (value+0.5);
          }
          dst[3]=(uint8_t) (out_alpha*255.0+0.5);
          break;
        }
        /* Grayscale+alpha: same "over" compositing on a single channel. */
        case 16:
        {
          double
            dst_alpha,
            out_alpha,
            src_alpha;

          src_alpha=((double) src[1]/255.0)*combined_opacity;
          dst_alpha=(double) dst[1]/255.0;
          out_alpha=src_alpha+dst_alpha*(1.0-src_alpha);
          if (out_alpha > 0.0)
            dst[0]=(uint8_t) ((src[0]*src_alpha+dst[0]*dst_alpha*
              (1.0-src_alpha))/out_alpha+0.5);
          dst[1]=(uint8_t) (out_alpha*255.0+0.5);
          break;
        }
        /* Indexed: no partial blending is possible; visible, non-transparent pixels 
           replace the destination. */
        default:
        {
          if ((src[0] != canvas->transparent_index) &&
              (combined_opacity >= 0.5))
            dst[0]=src[0];
          break;
        }
      }
    }
  }
}

/*
  CacheASECel() stores a copy of a decoded direct cel's pixels/placement so
  a later frame's Linked Cel (cel_type==1) referencing this (frame,layer)
  can be resolved. Returns MagickFalse on allocation failure.
*/
static MagickBooleanType CacheASECel(const Image *image,
  const uint8_t *cel_pixels,size_t cel_pixels_size,uint16_t cel_width,
  uint16_t cel_height,int16_t cel_x,int16_t cel_y,uint8_t cel_opacity,
  size_t frame_index,uint16_t layer_index,AsepriteCelCache *cel_cache,
  ExceptionInfo *exception)
{
  AsepriteCelCacheEntry
    *entry;

  if (cel_cache->count == cel_cache->capacity)
    {
      AsepriteCelCacheEntry
        *new_entries;

      size_t
        new_capacity;

      new_capacity=(cel_cache->capacity == 0) ? 16 : (cel_cache->capacity*2);
      new_entries=(AsepriteCelCacheEntry *) ResizeQuantumMemory(
        cel_cache->entries,new_capacity,sizeof(*new_entries));
      if (new_entries == (AsepriteCelCacheEntry *) NULL)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            image->filename);
          return(MagickFalse);
        }
      cel_cache->entries=new_entries;
      cel_cache->capacity=new_capacity;
    }
  entry=cel_cache->entries+cel_cache->count;
  entry->pixels=(uint8_t *) AcquireQuantumMemory(cel_pixels_size,
    sizeof(uint8_t));
  if (entry->pixels == (uint8_t *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  (void) memcpy(entry->pixels,cel_pixels,cel_pixels_size);
  entry->frame_index=frame_index;
  entry->layer_index=layer_index;
  entry->cel_x=cel_x;
  entry->cel_y=cel_y;
  entry->cel_width=cel_width;
  entry->cel_height=cel_height;
  entry->cel_opacity=cel_opacity;
  cel_cache->count++;
  return(MagickTrue);
}

static MagickBooleanType ReadASECelChunk(const Image *image,
  const uint8_t *chunk_data,size_t payload_size,AsepriteCanvas *canvas,
  const AsepriteLayerList *layers,size_t frame_index,
  AsepriteCelCache *cel_cache,ExceptionInfo *exception)
{
  double
    combined_opacity;

  int16_t
    cel_x,
    cel_y;

  MagickBooleanType
    visible;

  size_t
    bytes_per_pixel,
    cel_pixels_size,
    i;

  uint16_t
    cel_height,
    cel_type,
    cel_width,
    layer_index;

  uint8_t
    cel_opacity,
    *cel_pixels,
    layer_opacity;

  if (payload_size < 16)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        CorruptImageError,"InvalidChunkSize","`%s'",image->filename);
      return(MagickFalse);
    }
  layer_index=(uint16_t) (chunk_data[0] | (chunk_data[1] << 8));
  cel_x=(int16_t) (chunk_data[2] | (chunk_data[3] << 8));
  cel_y=(int16_t) (chunk_data[4] | (chunk_data[5] << 8));
  cel_opacity=chunk_data[6];
  cel_type=(uint16_t) (chunk_data[7] | (chunk_data[8] << 8));
  visible=MagickTrue;
  layer_opacity=255;
  if (layer_index < layers->count)
    {
      visible=layers->layers[layer_index].visible;
      layer_opacity=layers->layers[layer_index].opacity;
    }
  if ((visible != MagickFalse) && (layer_index < layers->count) &&
      (layers->layers[layer_index].blend_mode != 0))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        CoderError,"UnsupportedBlendMode","`%s'",image->filename);
      return(MagickFalse);
    }
  if (cel_type == 1)
    {
      /* Linked Cel: resolve the referenced frame's cel for this layer from
         the cache and composite it exactly like a direct cel. */
      size_t
        link_frame;

      if (payload_size < 18)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            CorruptImageError,"InvalidChunkSize","`%s'",image->filename);
          return(MagickFalse);
        }
      if (visible == MagickFalse)
        return(MagickTrue);
      link_frame=(size_t) (chunk_data[16] | (chunk_data[17] << 8));
      for (i=0; i < cel_cache->count; i++)
      {
        AsepriteCelCacheEntry
          *entry;

        entry=cel_cache->entries+i;
        if ((entry->frame_index == link_frame) &&
            (entry->layer_index == layer_index))
          {
            combined_opacity=((double) layer_opacity/255.0)*
              ((double) entry->cel_opacity/255.0);
            CompositeASECel(entry->pixels,entry->cel_width,
              entry->cel_height,entry->cel_x,entry->cel_y,combined_opacity,
              canvas);
            break;
          }
      }
      return(MagickTrue);
    }
  if (cel_type != 0 && cel_type != 2)
    return(MagickTrue);  /* Tilemap cels are not supported. */
  if (payload_size < 20)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        CorruptImageError,"InvalidChunkSize","`%s'",image->filename);
      return(MagickFalse);
    }
  cel_width=(uint16_t) (chunk_data[16] | (chunk_data[17] << 8));
  cel_height=(uint16_t) (chunk_data[18] | (chunk_data[19] << 8));
  bytes_per_pixel=canvas->color_depth/8;
  cel_pixels_size=(size_t) cel_width*cel_height*bytes_per_pixel;
  if (cel_pixels_size == 0)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        CorruptImageError,"InvalidChunkSize","`%s'",image->filename);
      return(MagickFalse);
    }
  cel_pixels=(uint8_t *) AcquireQuantumMemory(cel_pixels_size,
    sizeof(uint8_t));
  if (cel_pixels == (uint8_t *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  if (cel_type == 0)
    {
      if ((size_t) (payload_size-20) < cel_pixels_size)
        {
          cel_pixels=(uint8_t *) RelinquishMagickMemory(cel_pixels);
          (void) ThrowMagickException(exception,GetMagickModule(),
            CorruptImageError,"InvalidChunkSize","`%s'",image->filename);
          return(MagickFalse);
        }
      (void) memcpy(cel_pixels,chunk_data+20,cel_pixels_size);
    }
  else
    {
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      int
        zlib_status;

      uLongf
        uncompressed_size;

      uncompressed_size=(uLongf) cel_pixels_size;
      zlib_status=uncompress(cel_pixels,&uncompressed_size,chunk_data+20,
        (uLongf) (payload_size-20));
      if ((zlib_status != Z_OK) || 
          (uncompressed_size != (uLongf) cel_pixels_size))
        {
          cel_pixels=(uint8_t *) RelinquishMagickMemory(cel_pixels);
          (void) ThrowMagickException(exception,GetMagickModule(),
            CorruptImageError,"UnableToDecompressImage","`%s'",
            image->filename);
          return(MagickFalse);
        }
#else
      cel_pixels=(uint8_t *) RelinquishMagickMemory(cel_pixels);
      (void) ThrowMagickException(exception,GetMagickModule(),
        MissingDelegateError,"DelegateLibrarySupportNotBuiltIn","`%s' (ZLIB)",
        image->filename);
      return(MagickFalse);
#endif
    }
  /* Cache this direct cel's pixels so a later frame can link to it, even
     if the current layer is invisible or fully transparent. */
  if (CacheASECel(image,cel_pixels,cel_pixels_size,cel_width,cel_height,
      cel_x,cel_y,cel_opacity,frame_index,layer_index,cel_cache,
      exception) == MagickFalse)
    {
      cel_pixels=(uint8_t *) RelinquishMagickMemory(cel_pixels);
      return(MagickFalse);
    }
  if (visible == MagickFalse)
    {
      cel_pixels=(uint8_t *) RelinquishMagickMemory(cel_pixels);
      return(MagickTrue);
    }
  combined_opacity=((double) layer_opacity/255.0)*((double) cel_opacity/
    255.0);
  CompositeASECel(cel_pixels,cel_width,cel_height,cel_x,cel_y,
    combined_opacity,canvas);
  cel_pixels=(uint8_t *) RelinquishMagickMemory(cel_pixels);
  return(MagickTrue);
}

static Image *ReadASEImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  AsepriteCanvas
    canvas;

  AsepriteCelCache
    cel_cache;

  AsepriteChunk
    ase_chunk;

  AsepriteFrame
    ase_frame;

  AsepriteHeader
    ase_header;

  AsepriteLayerList
    layers;

  Image
    *image,
    *frame_image;

  MagickBooleanType
    status;

  Quantum
    *q;

  size_t
    chunk_idx,
    count,
    frame_idx,
    pixels_size;

  ssize_t
    x;

  ssize_t
    y;

  uint16_t
    magic;

  uint8_t
    *chunk_data,
    *pixel_data,
    *pixels;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  layers.layers=(AsepriteLayer *) NULL;
  layers.count=0;
  layers.capacity=0;
  cel_cache.entries=(AsepriteCelCacheEntry *) NULL;
  cel_cache.count=0;
  cel_cache.capacity=0;
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  ase_header.file_size=ReadBlobLSBLong(image);
  magic=ReadBlobLSBShort(image);
  if (magic != ASE_MAGIC)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  ase_header.frames=ReadBlobLSBShort(image);
  ase_header.width=ReadBlobLSBShort(image);
  ase_header.height=ReadBlobLSBShort(image);
  ase_header.color_depth=ReadBlobLSBShort(image);
  ase_header.flags=ReadBlobLSBLong(image);
  ase_header.speed=ReadBlobLSBShort(image);
  ase_header.padding1=ReadBlobLSBLong(image);
  ase_header.padding2=ReadBlobLSBLong(image);
  ase_header.palette_index=(uint8_t) ReadBlobByte(image);
  ase_header.ignore1=(uint8_t) ReadBlobByte(image);
  ase_header.ignore2=(uint8_t) ReadBlobByte(image);
  ase_header.ignore3=(uint8_t) ReadBlobByte(image);
  ase_header.pixel_width=ReadBlobLSBShort(image);
  ase_header.pixel_height=ReadBlobLSBShort(image);
  (void) SeekBlob(image,92,SEEK_CUR);
  if ((ase_header.width == 0) || (ase_header.height == 0))
    ThrowReaderException(CorruptImageError,"InvalidImageDimensions");
  if ((ase_header.width > 16384) || (ase_header.height > 16384))
    ThrowReaderException(CorruptImageError,"InvalidImageDimensions");
  if ((ase_header.color_depth != 32) && (ase_header.color_depth != 8) &&
      (ase_header.color_depth != 16))
    ThrowReaderException(CorruptImageError,"UnsupportedColorDepth");
  if ((ase_header.frames == 0) || (ase_header.frames > 4096))
    ThrowReaderException(CorruptImageError,"InvalidNumberOfFrames");
  if (ase_header.file_size > 512*1024*1024)
    ThrowReaderException(CorruptImageError,"FileSizeTooLarge");
  image->columns=ase_header.width;
  image->rows=ase_header.height;
  image->alpha_trait=BlendPixelTrait;
  image->ticks_per_second=1000;
  image->dispose=BackgroundDispose;
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  if (ase_header.color_depth == 8)
    {
      if (AcquireImageColormap(image,256,exception) == MagickFalse)
        {
          DestroyASELayerList(&layers);
          DestroyASECelCache(&cel_cache);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
    }
  if (AcquireMagickResource(ListLengthResource,ase_header.frames) == MagickFalse)
    ThrowFileException(exception,ResourceLimitError,"ListLengthExceedsLimit",
      image->filename);
  frame_image=image;
  for (frame_idx=0; frame_idx < (size_t) ase_header.frames; frame_idx++)
  {
    if (frame_idx > 0)
      {
        AcquireNextImage(image_info,frame_image,exception);
        if (GetNextImageInList(frame_image) == (Image *) NULL)
          {
            DestroyASELayerList(&layers);
            DestroyASECelCache(&cel_cache);
            return(DestroyImageList(image));
          }
        frame_image=SyncNextImageInList(frame_image);
        frame_image->columns=ase_header.width;
        frame_image->rows=ase_header.height;
        frame_image->alpha_trait=BlendPixelTrait;
        frame_image->ticks_per_second=1000;
        frame_image->dispose=BackgroundDispose;
        status=SetImageExtent(frame_image,frame_image->columns,
          frame_image->rows,exception);
        if (status == MagickFalse)
          {
            DestroyASELayerList(&layers);
            DestroyASECelCache(&cel_cache);
            return(DestroyImageList(image));
          }
        if (ase_header.color_depth == 8)
          {
            if (AcquireImageColormap(frame_image,256,exception) ==
                MagickFalse)
              {
                DestroyASELayerList(&layers);
                DestroyASECelCache(&cel_cache);
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }
            (void) memcpy(frame_image->colormap,
              frame_image->previous->colormap,256*
              sizeof(*frame_image->colormap));
          }
      }
    ase_frame.frame_size=ReadBlobLSBLong(image);
    ase_frame.frame_magic=ReadBlobLSBShort(image);
    if (ase_frame.frame_magic != ASE_FRAME_MAGIC)
      {
        DestroyASELayerList(&layers);
        DestroyASECelCache(&cel_cache);
        ThrowReaderException(CorruptImageError,"InvalidFrameHeader");
      }
    ase_frame.chunk_count=ReadBlobLSBShort(image);
    ase_frame.duration=ReadBlobLSBShort(image);
    ase_frame.padding1=(uint8_t) ReadBlobByte(image);
    ase_frame.padding2=(uint8_t) ReadBlobByte(image);
    ase_frame.next_frame=ReadBlobLSBLong(image);
    if (ase_frame.duration > 0)
      frame_image->delay=ase_frame.duration;
    if (image_info->ping != MagickFalse)
      {
        (void) SeekBlob(image,(MagickOffsetType) ase_frame.frame_size-16,
          SEEK_CUR);
        continue;
      }
    switch (ase_header.color_depth)
    {
      case 32:
        pixels_size=(size_t) ase_header.width*ase_header.height*4;
        break;
      case 16:
        pixels_size=(size_t) ase_header.width*ase_header.height*2;
        break;
      case 8:
        pixels_size=(size_t) ase_header.width*ase_header.height;
        break;
      default:
        pixels_size=0;
        break;
    }
    pixel_data=(uint8_t *) AcquireQuantumMemory(pixels_size,sizeof(uint8_t));
    if (pixel_data == (uint8_t *) NULL)
      {
        DestroyASELayerList(&layers);
        DestroyASECelCache(&cel_cache);
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      }
    if (ase_header.color_depth == 8)
      (void) memset(pixel_data,ase_header.palette_index,pixels_size);
    else
      (void) memset(pixel_data,0,pixels_size);
    canvas.pixels=pixel_data;
    canvas.width=ase_header.width;
    canvas.height=ase_header.height;
    canvas.color_depth=ase_header.color_depth;
    canvas.transparent_index=ase_header.palette_index;
    for (chunk_idx=0; chunk_idx < (size_t) ase_frame.chunk_count; chunk_idx++)
    {
      size_t
        payload_size;

      ase_chunk.chunk_size=ReadBlobLSBLong(image);
      ase_chunk.chunk_type=ReadBlobLSBShort(image);
      if ((ase_chunk.chunk_size < 6) || (ase_chunk.chunk_size > 100*1024*1024))
        {
          pixel_data=(uint8_t *) RelinquishMagickMemory(pixel_data);
          DestroyASELayerList(&layers);
          DestroyASECelCache(&cel_cache);
          ThrowReaderException(CorruptImageError,"InvalidChunkSize");
        }
      payload_size=ase_chunk.chunk_size-6;
      chunk_data=(uint8_t *) AcquireQuantumMemory(payload_size,
        sizeof(uint8_t));
      if (chunk_data == (uint8_t *) NULL)
        {
          pixel_data=(uint8_t *) RelinquishMagickMemory(pixel_data);
          DestroyASELayerList(&layers);
          DestroyASECelCache(&cel_cache);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      count=ReadBlob(image,payload_size,chunk_data);
      if (count != payload_size)
        {
          chunk_data=(uint8_t *) RelinquishMagickMemory(chunk_data);
          pixel_data=(uint8_t *) RelinquishMagickMemory(pixel_data);
          DestroyASELayerList(&layers);
          DestroyASECelCache(&cel_cache);
          ThrowReaderException(CorruptImageError,"UnableToReadImageData");
        }
      switch (ase_chunk.chunk_type)
      {
        /* CEL chunk (0x2005): composite the raw or compressed cel image 
           payload onto the frame canvas. */
        case 0x2005:
        {
          MagickBooleanType
            cel_status;

          cel_status=ReadASECelChunk(image,chunk_data,payload_size,&canvas,
            &layers,frame_idx,&cel_cache,exception);
          chunk_data=(uint8_t *) RelinquishMagickMemory(chunk_data);
          if (cel_status == MagickFalse)
            {
              pixel_data=(uint8_t *) RelinquishMagickMemory(pixel_data);
              DestroyASELayerList(&layers);
              DestroyASECelCache(&cel_cache);
              return(DestroyImageList(image));
            }
          continue;
        }
        /* New palette chunk: populate the RGBA color table. */
        case 0x2019:
        {
          if (frame_image->storage_class == PseudoClass)
            ReadASEPaletteChunk(frame_image,chunk_data,payload_size);
          break;
        }
        /* Layer chunk: track visibility/blend mode/opacity for later cel 
           compositing. */
        case 0x2004:
        {
          if (ReadASELayerChunk(image,chunk_data,payload_size,
              (ase_header.flags & 0x01) != 0 ? MagickTrue : MagickFalse,
              &layers,exception) == MagickFalse)
            {
              chunk_data=(uint8_t *) RelinquishMagickMemory(chunk_data);
              pixel_data=(uint8_t *) RelinquishMagickMemory(pixel_data);
              DestroyASELayerList(&layers);
              DestroyASECelCache(&cel_cache);
              return(DestroyImageList(image));
            }
          break;
        }
        default:
          break;
      }
      chunk_data=(uint8_t *) RelinquishMagickMemory(chunk_data);
    }
    pixels=pixel_data;
    switch (ase_header.color_depth)
    {
      case 32:
      {
        unsigned char
          *p;

        p=pixels;
        for (y=0; y < (ssize_t) frame_image->rows; y++)
        {
          q=QueueAuthenticPixels(frame_image,0,y,frame_image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) frame_image->columns; x++)
          {
            SetPixelRed(frame_image,ScaleCharToQuantum(*p++),q);
            SetPixelGreen(frame_image,ScaleCharToQuantum(*p++),q);
            SetPixelBlue(frame_image,ScaleCharToQuantum(*p++),q);
            SetPixelAlpha(frame_image,ScaleCharToQuantum(*p++),q);
            q+=GetPixelChannels(frame_image);
          }
          if (SyncAuthenticPixels(frame_image,exception) == MagickFalse)
            break;
          if (SetImageProgress(frame_image,LoadImageTag,y,frame_image->rows) == MagickFalse)
            break;
        }
        break;
      }
      case 16:
      {
        unsigned char
          *p;

        p=pixels;
        for (y=0; y < (ssize_t) frame_image->rows; y++)
        {
          q=QueueAuthenticPixels(frame_image,0,y,frame_image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) frame_image->columns; x++)
          {
            Quantum
              gray;

            gray=ScaleCharToQuantum(*p++);
            SetPixelRed(frame_image,gray,q);
            SetPixelGreen(frame_image,gray,q);
            SetPixelBlue(frame_image,gray,q);
            SetPixelAlpha(frame_image,ScaleCharToQuantum(*p++),q);
            q+=GetPixelChannels(frame_image);
          }
          if (SyncAuthenticPixels(frame_image,exception) == MagickFalse)
            break;
          if (SetImageProgress(frame_image,LoadImageTag,y,frame_image->rows) == MagickFalse)
            break;
        }
        break;
      }
      case 8:
      {
        unsigned char
          *p;

        p=pixels;
        for (y=0; y < (ssize_t) frame_image->rows; y++)
        {
          q=QueueAuthenticPixels(frame_image,0,y,frame_image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) frame_image->columns; x++)
          {
            SetPixelIndex(frame_image,*p,q);
            p++;
            q+=GetPixelChannels(frame_image);
          }
          if (SyncAuthenticPixels(frame_image,exception) == MagickFalse)
            break;
          if (SetImageProgress(frame_image,LoadImageTag,y,frame_image->rows) == MagickFalse)
            break;
        }
        break;
      }
    }
    pixel_data=(uint8_t *) RelinquishMagickMemory(pixel_data);
  }
  DestroyASELayerList(&layers);
  DestroyASECelCache(&cel_cache);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
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
%   R e g i s t e r A S E I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterASEImage() adds properties for the ASE image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterASEImage method is:
%
%      size_t RegisterASEImage(void)
%
*/
ModuleExport size_t RegisterASEImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("ASE","ASE","Aseprite Sprite Format");
  entry->decoder=(DecodeImageHandler *) ReadASEImage;
  entry->encoder=(EncodeImageHandler *) WriteASEImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->mime_type=ConstantString("image/x-aseprite");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("ASE","ASEPRITE","Aseprite Sprite Format");
  entry->decoder=(DecodeImageHandler *) ReadASEImage;
  entry->encoder=(EncodeImageHandler *) WriteASEImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->mime_type=ConstantString("image/x-aseprite");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r A S E I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterASEImage() removes format registrations made by the
%  ASE module from the list of supported formats.
%
%  The format of the UnregisterASEImage method is:
%
%      UnregisterASEImage(void)
%
*/
ModuleExport void UnregisterASEImage(void)
{
  (void) UnregisterMagickInfo("ASE");
  (void) UnregisterMagickInfo("ASEPRITE");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e A S E I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteASEImage() writes an image to an Aseprite sprite file.
%
%  The format of the WriteASEImage method is:
%
%      MagickBooleanType WriteASEImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteASEImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  const Quantum
    *q;

  double
    delay_ms;

  Image
    *p;

  MagickBooleanType
    is_indexed,
    status;

  MagickOffsetType
    chunk_count_offset,
    end_offset,
    file_size_offset,
    frame_size_offset;

  size_t
    bytes_per_pixel,
    chunk_count,
    color_depth,
    count,
    frame_count,
    frame_x,
    frame_y,
    pixel_count,
    ticks_per_second;

  ssize_t
    i;

  uint8_t
    *pixels,
    transparent_index;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  frame_count=0;
  for (p=image; p != (Image *) NULL; p=p->next)
    frame_count++;
  /*
    Indexed (PseudoClass) images with 256 colors or fewer are written as
    8bpp Indexed sprites with a real Palette Chunk; everything else is
    written as 32bpp RGBA, as before.
  */
  is_indexed=((image->storage_class == PseudoClass) &&
    (image->colors > 0) && (image->colors <= 256)) ? MagickTrue :
    MagickFalse;
  color_depth=(is_indexed != MagickFalse) ? 8 : 32;
  bytes_per_pixel=color_depth/8;
  transparent_index=0;
  if (is_indexed != MagickFalse)
    for (i=0; i < (ssize_t) image->colors; i++)
      if (image->colormap[i].alpha == (MagickRealType) TransparentAlpha)
        {
          transparent_index=(uint8_t) i;
          break;
        }
  file_size_offset=TellBlob(image);
  (void) WriteBlobLSBLong(image,0);
  (void) WriteBlobLSBShort(image,ASE_MAGIC);
  (void) WriteBlobLSBShort(image,(unsigned short) frame_count);
  (void) WriteBlobLSBShort(image,(unsigned short) image->columns);
  (void) WriteBlobLSBShort(image,(unsigned short) image->rows);
  (void) WriteBlobLSBShort(image,(unsigned short) color_depth);
  (void) WriteBlobLSBLong(image,1);  /* flags: bit 0 = layer opacity has valid value */
  ticks_per_second=(image->ticks_per_second > 0) ? image->ticks_per_second :
    100;
  (void) WriteBlobLSBShort(image,(unsigned short) (((double) image->delay*1000.0)/
    (double) ticks_per_second+0.5));
  (void) WriteBlobLSBLong(image,0);
  (void) WriteBlobLSBLong(image,0);
  (void) WriteBlobByte(image,transparent_index);
  (void) WriteBlobByte(image,0);
  (void) WriteBlobByte(image,0);
  (void) WriteBlobByte(image,0);
  (void) WriteBlobLSBShort(image,1);
  (void) WriteBlobLSBShort(image,1);
  for (i=0; i < 46; i++)
    (void) WriteBlobLSBShort(image,0);  /* grid x/y/width/height + reserved[84] */
  pixel_count=image->columns*image->rows;
  pixels=(uint8_t *) AcquireQuantumMemory(pixel_count,bytes_per_pixel);
  if (pixels == (uint8_t *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  for (p=image; p != (Image *) NULL; p=p->next)
  {
    chunk_count=0;
    frame_size_offset=TellBlob(image);
    (void) WriteBlobLSBLong(image,0);
    (void) WriteBlobLSBShort(image,ASE_FRAME_MAGIC);
    chunk_count_offset=TellBlob(image);
    (void) WriteBlobLSBShort(image,0);
    ticks_per_second=(p->ticks_per_second > 0) ? p->ticks_per_second : 100;
    delay_ms=((double) p->delay*1000.0)/(double) ticks_per_second;
    (void) WriteBlobLSBShort(image,(unsigned short) (delay_ms+0.5));
    (void) WriteBlobByte(image,0);
    (void) WriteBlobByte(image,0);
    (void) WriteBlobLSBLong(image,0);
    if (p == image)
      {
        /*
          Write a single Layer Chunk (0x2004) describing the one layer all
          cels reference, so the file is well-formed for any Aseprite-
          compatible reader (not just our own).
        */
        (void) WriteBlobLSBLong(image,24);
        (void) WriteBlobLSBShort(image,0x2004);
        (void) WriteBlobLSBShort(image,1);  /* flags: visible */
        (void) WriteBlobLSBShort(image,0);  /* layer type: normal */
        (void) WriteBlobLSBShort(image,0);  /* child level */
        (void) WriteBlobLSBShort(image,0);  /* default width (ignored) */
        (void) WriteBlobLSBShort(image,0);  /* default height (ignored) */
        (void) WriteBlobLSBShort(image,0);  /* blend mode: normal */
        (void) WriteBlobByte(image,255);    /* opacity */
        (void) WriteBlobByte(image,0);
        (void) WriteBlobByte(image,0);
        (void) WriteBlobByte(image,0);
        (void) WriteBlobLSBShort(image,0);  /* layer name length */
        chunk_count++;
        if (is_indexed != MagickFalse)
          {
            /*
              Write the New Palette Chunk (0x2019) with the real colormap
              (including per-entry alpha), so the sprite's colors and
              transparency survive the round trip.
            */
            (void) WriteBlobLSBLong(image,(unsigned int) (20+image->colors*6+6));
            (void) WriteBlobLSBShort(image,0x2019);
            (void) WriteBlobLSBLong(image,(unsigned int) image->colors);
            (void) WriteBlobLSBLong(image,0);
            (void) WriteBlobLSBLong(image,(unsigned int) (image->colors-1));
            (void) WriteBlobLSBLong(image,0);
            (void) WriteBlobLSBLong(image,0);
            for (i=0; i < (ssize_t) image->colors; i++)
            {
              (void) WriteBlobLSBShort(image,0);
              (void) WriteBlobByte(image,ScaleQuantumToChar(
                ClampToQuantum(image->colormap[i].red)));
              (void) WriteBlobByte(image,ScaleQuantumToChar(
                ClampToQuantum(image->colormap[i].green)));
              (void) WriteBlobByte(image,ScaleQuantumToChar(
                ClampToQuantum(image->colormap[i].blue)));
              (void) WriteBlobByte(image,ScaleQuantumToChar(
                ClampToQuantum(image->colormap[i].alpha)));
            }
            chunk_count++;
          }
      }
    (void) memset(pixels,0,pixel_count*bytes_per_pixel);
    for (frame_y=0; frame_y < p->rows; frame_y++)
    {
      q=GetVirtualPixels(p,0,(ssize_t)frame_y,p->columns,1,exception);
      if (q == (const Quantum *) NULL)
        break;
      for (frame_x=0; frame_x < p->columns; frame_x++)
      {
        size_t
          offset;
        offset=(frame_y*p->columns+frame_x)*bytes_per_pixel;
        if (offset+bytes_per_pixel <= pixel_count*bytes_per_pixel)
          {
            if (is_indexed != MagickFalse)
              pixels[offset]=(uint8_t) GetPixelIndex(p,q);
            else
              {
                pixels[offset]=ScaleQuantumToChar(GetPixelRed(p,q));
                pixels[offset+1]=ScaleQuantumToChar(GetPixelGreen(p,q));
                pixels[offset+2]=ScaleQuantumToChar(GetPixelBlue(p,q));
                pixels[offset+3]=ScaleQuantumToChar(GetPixelAlpha(p,q));
              }
          }
        q+=GetPixelChannels(p);
      }
    }
    if (image_info->compression == NoCompression)
      {
        (void) WriteBlobLSBLong(image,(unsigned int) (pixel_count*bytes_per_pixel+26));
        (void) WriteBlobLSBShort(image,0x2005);
        (void) WriteBlobLSBShort(image,0); /* layer index */
        (void) WriteBlobLSBShort(image,0); /* x */
        (void) WriteBlobLSBShort(image,0); /* y */
        (void) WriteBlobByte(image,255);   /* opacity */
        (void) WriteBlobLSBShort(image,0);  /* raw image */
        (void) WriteBlobLSBShort(image,0);  /* z-index */
        (void) WriteBlobByte(image,0);
        (void) WriteBlobByte(image,0);
        (void) WriteBlobByte(image,0);
        (void) WriteBlobByte(image,0);
        (void) WriteBlobByte(image,0);
        (void) WriteBlobLSBShort(image,(unsigned short) p->columns);
        (void) WriteBlobLSBShort(image,(unsigned short) p->rows);
        count=WriteBlob(image,pixel_count*bytes_per_pixel,pixels);
        chunk_count++;
        if (count != pixel_count*bytes_per_pixel)
          {
            pixels=(uint8_t *) RelinquishMagickMemory(pixels);
            ThrowWriterException(FileOpenError,"UnableToWriteBlob");
          }
      }
    else
#if defined(MAGICKCORE_ZLIB_DELEGATE)
    {
      uLongf
        compressed_size,
        payload_size;
      uint8_t
        *compressed;

      payload_size=(uLongf) (pixel_count*bytes_per_pixel);
      compressed_size=compressBound((uLong) payload_size);
      compressed=(uint8_t *) AcquireQuantumMemory((size_t) compressed_size,
        sizeof(uint8_t));
      if (compressed == (uint8_t *) NULL)
        {
          pixels=(uint8_t *) RelinquishMagickMemory(pixels);
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        }
      if (compress2(compressed,&compressed_size,pixels,payload_size,
          Z_BEST_COMPRESSION) != Z_OK)
        {
          compressed=(uint8_t *) RelinquishMagickMemory(compressed);
          pixels=(uint8_t *) RelinquishMagickMemory(pixels);
          ThrowWriterException(CorruptImageError,"UnableToCompressImage");
        }
      (void) WriteBlobLSBLong(image,(unsigned int) (compressed_size+26));
      (void) WriteBlobLSBShort(image,0x2005);
      (void) WriteBlobLSBShort(image,0); /* layer index */
      (void) WriteBlobLSBShort(image,0); /* x */
      (void) WriteBlobLSBShort(image,0); /* y */
      (void) WriteBlobByte(image,255);   /* opacity */
      (void) WriteBlobLSBShort(image,2);  /* compressed image */
      (void) WriteBlobLSBShort(image,0);  /* z-index */
      (void) WriteBlobByte(image,0);
      (void) WriteBlobByte(image,0);
      (void) WriteBlobByte(image,0);
      (void) WriteBlobByte(image,0);
      (void) WriteBlobByte(image,0);
      (void) WriteBlobLSBShort(image,(unsigned short) p->columns);
      (void) WriteBlobLSBShort(image,(unsigned short) p->rows);
      count=WriteBlob(image,(size_t) compressed_size,compressed);
      compressed=(uint8_t *) RelinquishMagickMemory(compressed);
      chunk_count++;
      if (count != (size_t) compressed_size)
        {
          pixels=(uint8_t *) RelinquishMagickMemory(pixels);
          ThrowWriterException(FileOpenError,"UnableToWriteBlob");
        }
    }
#else
    {
      pixels=(uint8_t *) RelinquishMagickMemory(pixels);
      (void) ThrowMagickException(exception,GetMagickModule(),
      MissingDelegateError,"DelegateLibrarySupportNotBuiltIn","`%s' (ZLIB)",
      image->filename);
      return(MagickFalse);
    }
#endif
    end_offset=TellBlob(image);
    SeekBlob(image,chunk_count_offset,SEEK_SET);
    (void) WriteBlobLSBShort(image,(unsigned short) chunk_count);
    SeekBlob(image,frame_size_offset,SEEK_SET);
    (void) WriteBlobLSBLong(image,(unsigned int) (end_offset-frame_size_offset));
    SeekBlob(image,0,SEEK_END);
  }
  pixels=(uint8_t *) RelinquishMagickMemory(pixels);
  end_offset=TellBlob(image);
  SeekBlob(image,file_size_offset,SEEK_SET);
  (void) WriteBlobLSBLong(image,(unsigned int) end_offset);
  SeekBlob(image,0,SEEK_END);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

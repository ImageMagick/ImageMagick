/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     W   W  BBBB   IIIII  N   N  FFFFF   OOO                                %
%     W   W  B   B    I    NN  N  F      O   O                               %
%     W W W  BBBB     I    N N N  FFF    O   O                               %
%     WW WW  B   B    I    N  NN  F      O   O                               %
%     W   W  BBBB   IIIII  N   N  F       OOO                                %
%                                                                             %
%                                                                             %
%                   Read Amiga Workbench Icon Image Format                    %
%                                                                             %
%                              Software Design                                %
%                            Gareth Davidson                                  %
%                              March 2026                                     %
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
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#if defined(MAGICKCORE_ZLIB_DELEGATE)
#include <zlib.h>
#endif

/*
  Define declarations.
*/
#define AmigaIconMagic  0xE310
#define MaxAmigaIconDimension  1024

/*
  Amiga Workbench system palettes.
*/
static const unsigned char AmigaWB1x[4][3] =
{
  { 85, 170, 255 }, { 255, 255, 255 }, { 0, 0, 0 }, { 255, 136, 0 }
};

static const unsigned char AmigaWB2x[8][3] =
{
  { 149, 149, 149 }, { 0, 0, 0 }, { 255, 255, 255 }, { 59, 103, 162 },
  { 123, 123, 123 }, { 175, 175, 175 }, { 170, 144, 124 }, { 255, 169, 151 }
};

/*
  Helper: extract an unsigned integer from a bit array.
*/
static unsigned int BitsToUInt(const unsigned char *data, size_t total_bits,
  size_t *bit_pos, size_t count)
{
  unsigned int
    val;

  size_t
    i;

  val=0;
  for (i=0; i < count; i++)
  {
    val<<=1;
    if (*bit_pos < total_bits)
    {
      size_t
        byte_idx,
        bit_idx;

      byte_idx=(*bit_pos) >> 3;
      bit_idx=7-((*bit_pos) & 7);
      val|=(unsigned int) ((data[byte_idx] >> bit_idx) & 1);
    }
    (*bit_pos)++;
  }
  return(val);
}

/*
  Count expanded bits from a NewIcon encoded line (for buffer sizing).
*/
static size_t NewIconLineBitCount(const char *src, size_t src_len)
{
  size_t
    bit_count,
    ch_idx;

  bit_count=0;
  for (ch_idx=0; ch_idx < src_len; ch_idx++)
  {
    unsigned int
      byte;

    byte=(unsigned int) (unsigned char) src[ch_idx];
    if (byte < 0x20)
      continue;
    if (byte <= 0xD0)
      bit_count+=7;
    else
      bit_count+=(size_t) (byte-0xD0)*7;
  }
  return(bit_count);
}

/*
  Decode one NewIcon 7-bit encoded line into a bit buffer.

  Each character maps to 7 bits; characters 0xD1+ are RLE zero runs.
  Returns the number of bits written.
*/
static size_t DecodeNewIconLine(const char *src, size_t src_len,
  unsigned char *bits, size_t bits_alloc, size_t bit_offset)
{
  size_t
    bit_count,
    ch_idx;

  bit_count=bit_offset;
  for (ch_idx=0; ch_idx < src_len; ch_idx++)
  {
    unsigned int
      byte,
      val;

    int
      bit_i;

    byte=(unsigned int) (unsigned char) src[ch_idx];
    if (byte < 0x20)
      continue;
    if (byte <= 0x9F)
      val=byte-0x20;
    else if (byte <= 0xD0)
      val=byte-0x51;
    else if (byte >= 0xD1)
    {
      bit_count+=(size_t) (byte-0xD0)*7;
      continue;
    }
    else
      continue;
    for (bit_i=6; bit_i >= 0; bit_i--)
    {
      if (bit_count < bits_alloc*8)
      {
        if ((val >> bit_i) & 1)
          bits[bit_count >> 3]|=
            (unsigned char) (1 << (7-(bit_count & 7)));
      }
      bit_count++;
    }
  }
  return(bit_count);
}

/*
  Decode GlowIcon/ColorIcon RLE data.

  The RLE operates on a bit stream where entries are `depth` bits wide,
  but control bytes are always 8 bits.

  Returns allocated buffer with decoded bytes (one byte per entry).
  Caller must free with RelinquishMagickMemory().
*/
static unsigned char *DecodeAmigaRLE(const unsigned char *data,
  size_t data_size, size_t depth, size_t expected_count, size_t *decoded_count)
{
  unsigned char
    *result;

  size_t
    bit_pos,
    count,
    total_bits;

  result=(unsigned char *) AcquireQuantumMemory(expected_count,
    sizeof(*result));
  if (result == (unsigned char *) NULL)
    return((unsigned char *) NULL);
  total_bits=data_size*8;
  bit_pos=0;
  count=0;
  while ((bit_pos+8 <= total_bits) && (count < expected_count))
  {
    unsigned int
      control;

    control=BitsToUInt(data,total_bits,&bit_pos,8);
    if (control == 0x80)
      continue;
    if (control <= 0x7F)
    {
      size_t
        i,
        n;

      n=(size_t) control+1;
      for (i=0; i < n; i++)
      {
        if (count >= expected_count)
          break;
        result[count++]=(unsigned char) BitsToUInt(data,total_bits,&bit_pos,
          depth);
      }
    }
    else
    {
      unsigned int
        val;

      size_t
        i,
        n;

      val=BitsToUInt(data,total_bits,&bit_pos,depth);
      n=257-(size_t) control;
      for (i=0; i < n; i++)
      {
        if (count >= expected_count)
          break;
        result[count++]=(unsigned char) val;
      }
    }
  }
  *decoded_count=count;
  return(result);
}

/*
  Helper: set one RGBA pixel in a DirectClass image row.
*/
static inline void SetAmigaPixelRGBA(const Image *image, Quantum *q,
  unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  SetPixelRed(image,ScaleCharToQuantum(r),q);
  SetPixelGreen(image,ScaleCharToQuantum(g),q);
  SetPixelBlue(image,ScaleCharToQuantum(b),q);
  SetPixelAlpha(image,ScaleCharToQuantum(a),q);
}

/*
  Append a new image to the list and set its dimensions.
  Returns MagickFalse on failure.
*/
static MagickBooleanType AppendAmigaImage(Image **image, size_t width,
  size_t height, size_t scene, const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  if (scene > 0)
  {
    AcquireNextImage(image_info,*image,exception);
    *image=SyncNextImageInList(*image);
    if (*image == (Image *) NULL)
      return(MagickFalse);
  }
  (*image)->columns=width;
  (*image)->rows=height;
  (*image)->depth=8;
  (*image)->alpha_trait=BlendPixelTrait;
  (*image)->scene=scene;
  if (SetImageExtent(*image,width,height,exception) == MagickFalse)
    return(MagickFalse);
  return(MagickTrue);
}

/*
  Render a paletted image (NewIcon or GlowIcon) to DirectClass RGBA.
*/
static void RenderPalettedAmigaImage(Image *image, size_t width, size_t height,
  const unsigned char *pixels, size_t pixel_count,
  const unsigned char *palette, size_t pal_entries,
  MagickBooleanType has_transparent, unsigned char transparent_idx,
  ExceptionInfo *exception)
{
  ssize_t
    y;

  for (y=0; y < (ssize_t) height; y++)
  {
    Quantum
      *q;

    ssize_t
      x;

    q=QueueAuthenticPixels(image,0,y,width,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) width; x++)
    {
      size_t
        pi;

      unsigned char
        idx;

      pi=(size_t) y*width+(size_t) x;
      idx=pi < pixel_count ? pixels[pi] : 0;
      if (has_transparent && (idx == transparent_idx))
        SetAmigaPixelRGBA(image,q,0,0,0,0);
      else if ((palette != (const unsigned char *) NULL) &&
               ((size_t) idx < pal_entries))
        SetAmigaPixelRGBA(image,q,palette[idx*3],palette[idx*3+1],
          palette[idx*3+2],255);
      else
        SetAmigaPixelRGBA(image,q,0,0,0,255);
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
}

/*
  Read and render a classic planar Amiga icon image from the blob.

  Reads the 20-byte Image header, bitplane data, selects the system palette,
  renders to a new scene, and increments *scene.  Returns MagickTrue on
  success.  On failure the blob position is indeterminate.
*/
static MagickBooleanType ReadAndRenderClassicImage(Image **image,
  size_t *scene, unsigned int user_data, const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  unsigned short
    img_width,
    img_height,
    img_depth;

  unsigned char
    plane_pick,
    plane_on_off,
    **planes;

  size_t
    words_per_row,
    bytes_per_row,
    plane_size,
    num_colors,
    p_idx;

  const unsigned char
    (*palette)[3];

  ssize_t
    y;

  (void) ReadBlobMSBShort(*image);  /* left_edge */
  (void) ReadBlobMSBShort(*image);  /* top_edge */
  img_width=ReadBlobMSBShort(*image);
  img_height=ReadBlobMSBShort(*image);
  img_depth=ReadBlobMSBShort(*image);
  (void) ReadBlobMSBLong(*image);   /* image_data ptr */
  plane_pick=(unsigned char) ReadBlobByte(*image);
  plane_on_off=(unsigned char) ReadBlobByte(*image);
  (void) ReadBlobMSBLong(*image);   /* next ptr */
  if (EOFBlob(*image) != MagickFalse)
    return(MagickFalse);
  if ((img_width == 0) || (img_height == 0) || (img_depth == 0) ||
      (img_width > MaxAmigaIconDimension) ||
      (img_height > MaxAmigaIconDimension) || (img_depth > 8))
    return(MagickFalse);
  words_per_row=((size_t) img_width+15)/16;
  bytes_per_row=words_per_row*2;
  plane_size=bytes_per_row*(size_t) img_height;
  if (plane_size * (size_t) img_depth > GetBlobSize(*image))
    return(MagickFalse);
  planes=(unsigned char **) AcquireQuantumMemory((size_t) img_depth,
    sizeof(*planes));
  if (planes == (unsigned char **) NULL)
    return(MagickFalse);
  (void) memset(planes,0,(size_t) img_depth*sizeof(*planes));
  for (p_idx=0; p_idx < (size_t) img_depth; p_idx++)
  {
    planes[p_idx]=(unsigned char *) AcquireQuantumMemory(plane_size,
      sizeof(*planes[p_idx]));
    if (planes[p_idx] == (unsigned char *) NULL)
    {
      size_t j;
      for (j=0; j < p_idx; j++)
        planes[j]=(unsigned char *) RelinquishMagickMemory(planes[j]);
      planes=(unsigned char **) RelinquishMagickMemory(planes);
      return(MagickFalse);
    }
    if (ReadBlob(*image,plane_size,planes[p_idx]) != (ssize_t) plane_size)
    {
      size_t j;
      for (j=0; j <= p_idx; j++)
        planes[j]=(unsigned char *) RelinquishMagickMemory(planes[j]);
      planes=(unsigned char **) RelinquishMagickMemory(planes);
      return(MagickFalse);
    }
  }
  if (user_data & 0xFF)
  {
    palette=AmigaWB2x;
    num_colors=8;
  }
  else
  {
    palette=AmigaWB1x;
    num_colors=4;
  }
  if (AppendAmigaImage(image,(size_t) img_width,(size_t) img_height,
      *scene,image_info,exception) == MagickFalse)
  {
    size_t j;
    for (j=0; j < (size_t) img_depth; j++)
      planes[j]=(unsigned char *) RelinquishMagickMemory(planes[j]);
    planes=(unsigned char **) RelinquishMagickMemory(planes);
    return(MagickFalse);
  }
  for (y=0; y < (ssize_t) img_height; y++)
  {
    Quantum
      *q;

    ssize_t
      x;

    q=QueueAuthenticPixels(*image,0,y,(size_t) img_width,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) img_width; x++)
    {
      size_t
        byte_idx,
        bit_idx,
        plane_num,
        data_plane;

      unsigned int
        color_index;

      byte_idx=(size_t) y*bytes_per_row+((size_t) x >> 3);
      bit_idx=7-((size_t) x & 7);
      color_index=0;
      data_plane=0;
      for (plane_num=0; plane_num < (size_t) img_depth; plane_num++)
      {
        if (plane_pick & (1 << plane_num))
        {
          if (data_plane < (size_t) img_depth)
            color_index|=(unsigned int)
              (((planes[data_plane][byte_idx] >> bit_idx) & 1) << plane_num);
          data_plane++;
        }
        else if (plane_on_off & (1 << plane_num))
          color_index|=(1U << plane_num);
      }
      if (color_index == 0)
        SetAmigaPixelRGBA(*image,q,0,0,0,0);
      else if (color_index < num_colors)
        SetAmigaPixelRGBA(*image,q,palette[color_index][0],
          palette[color_index][1],palette[color_index][2],255);
      else
        SetAmigaPixelRGBA(*image,q,0,0,0,255);
      q+=(ptrdiff_t) GetPixelChannels(*image);
    }
    if (SyncAuthenticPixels(*image,exception) == MagickFalse)
      break;
  }
  for (p_idx=0; p_idx < (size_t) img_depth; p_idx++)
    planes[p_idx]=(unsigned char *) RelinquishMagickMemory(planes[p_idx]);
  planes=(unsigned char **) RelinquishMagickMemory(planes);
  (*scene)++;
  return(MagickTrue);
}

/*
  Decode and render a NewIcon image from collected IM1=/IM2= lines.

  Returns MagickTrue if an image was successfully decoded and rendered.
*/
static MagickBooleanType DecodeAndRenderNewIcon(Image **image, size_t *scene,
  char **im_lines, size_t im_count, const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    *first_line;

  size_t
    ni_width,
    ni_height,
    ni_num_colors,
    ni_depth,
    ni_transparent,
    bit_count,
    bit_pos,
    pal_idx,
    pix_count,
    line_idx,
    line_bits_alloc,
    line_bits_bytes;

  unsigned char
    *ni_palette,
    *ni_pixels,
    *line_bits;

  if ((im_count == 0) || (im_lines == (char **) NULL))
    return(MagickFalse);
  first_line=im_lines[0];
  if (strlen(first_line) < 5)
    return(MagickFalse);
  ni_transparent=((unsigned char) first_line[0] == 0x42) ? 1 : 0;
  if (((unsigned char) first_line[1] < 0x21) ||
      ((unsigned char) first_line[2] < 0x21))
    return(MagickFalse);
  ni_width=(size_t) ((unsigned char) first_line[1]-0x21);
  ni_height=(size_t) ((unsigned char) first_line[2]-0x21);
  ni_num_colors=(size_t)
    (((unsigned char) first_line[3]-0x21) << 6) +
    (size_t) ((unsigned char) first_line[4]-0x21);
  if ((ni_width == 0) || (ni_height == 0) || (ni_num_colors == 0) ||
      (ni_width > MaxAmigaIconDimension) ||
      (ni_height > MaxAmigaIconDimension) ||
      (ni_num_colors > 256))
    return(MagickFalse);
  ni_depth=1;
  while ((1UL << ni_depth) < ni_num_colors)
    ni_depth++;
  /*
    Find max expanded bit count to size the reusable decode buffer.
  */
  line_bits_alloc=NewIconLineBitCount(first_line+5,strlen(first_line+5));
  for (line_idx=1; line_idx < im_count; line_idx++)
  {
    size_t
      ll;

    ll=NewIconLineBitCount(im_lines[line_idx],strlen(im_lines[line_idx]));
    if (ll > line_bits_alloc)
      line_bits_alloc=ll;
  }
  line_bits_alloc+=7;
  line_bits_bytes=(line_bits_alloc+7)/8+1;
  line_bits=(unsigned char *) AcquireQuantumMemory(line_bits_bytes,
    sizeof(*line_bits));
  if (line_bits == (unsigned char *) NULL)
    return(MagickFalse);
  /*
    First line: extract palette only, discard remaining bits.
  */
  (void) memset(line_bits,0,line_bits_bytes*sizeof(*line_bits));
  bit_count=DecodeNewIconLine(first_line+5,strlen(first_line+5),
    line_bits,line_bits_bytes,0);
  ni_palette=(unsigned char *) AcquireQuantumMemory(ni_num_colors*3+3,
    sizeof(*ni_palette));
  if (ni_palette == (unsigned char *) NULL)
  {
    line_bits=(unsigned char *) RelinquishMagickMemory(line_bits);
    return(MagickFalse);
  }
  (void) memset(ni_palette,0,(ni_num_colors*3+3)*sizeof(*ni_palette));
  bit_pos=0;
  for (pal_idx=0; pal_idx < ni_num_colors; pal_idx++)
  {
    ni_palette[pal_idx*3]=(unsigned char) BitsToUInt(line_bits,
      bit_count,&bit_pos,8);
    ni_palette[pal_idx*3+1]=(unsigned char) BitsToUInt(line_bits,
      bit_count,&bit_pos,8);
    ni_palette[pal_idx*3+2]=(unsigned char) BitsToUInt(line_bits,
      bit_count,&bit_pos,8);
  }
  /*
    Subsequent lines: extract pixels per-line, discard leftover bits.
  */
  ni_pixels=(unsigned char *) AcquireQuantumMemory(ni_width*ni_height+1,
    sizeof(*ni_pixels));
  if (ni_pixels == (unsigned char *) NULL)
  {
    ni_palette=(unsigned char *) RelinquishMagickMemory(ni_palette);
    line_bits=(unsigned char *) RelinquishMagickMemory(line_bits);
    return(MagickFalse);
  }
  (void) memset(ni_pixels,0,(ni_width*ni_height+1)*sizeof(*ni_pixels));
  pix_count=0;
  for (line_idx=1; line_idx < im_count &&
       pix_count < ni_width*ni_height; line_idx++)
  {
    size_t
      lbp;

    (void) memset(line_bits,0,line_bits_bytes*sizeof(*line_bits));
    bit_count=DecodeNewIconLine(im_lines[line_idx],
      strlen(im_lines[line_idx]),line_bits,line_bits_bytes,0);
    lbp=0;
    while ((lbp+ni_depth <= bit_count) &&
           (pix_count < ni_width*ni_height))
    {
      ni_pixels[pix_count++]=(unsigned char) BitsToUInt(
        line_bits,bit_count,&lbp,ni_depth);
    }
  }
  line_bits=(unsigned char *) RelinquishMagickMemory(line_bits);
  /*
    Render NewIcon to image.
  */
  if (AppendAmigaImage(image,ni_width,ni_height,*scene,image_info,
      exception) == MagickFalse)
  {
    ni_pixels=(unsigned char *) RelinquishMagickMemory(ni_pixels);
    ni_palette=(unsigned char *) RelinquishMagickMemory(ni_palette);
    return(MagickFalse);
  }
  RenderPalettedAmigaImage(*image,ni_width,ni_height,
    ni_pixels,pix_count,ni_palette,ni_num_colors,
    ni_transparent ? MagickTrue : MagickFalse,0,exception);
  ni_pixels=(unsigned char *) RelinquishMagickMemory(ni_pixels);
  ni_palette=(unsigned char *) RelinquishMagickMemory(ni_palette);
  (*scene)++;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s A M I G A I C O N                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsWBINFO() returns MagickTrue if the image format type, identified by
%  the magick string, is an Amiga Workbench icon file.
%
%  The format of the IsWBINFO method is:
%
%      MagickBooleanType IsWBINFO(const unsigned char *magick,
%        const size_t length)
%
*/
static MagickBooleanType IsWBINFO(const unsigned char *magick,
  const size_t length)
{
  if (length < 2)
    return(MagickFalse);
  if ((magick[0] == 0xE3) && (magick[1] == 0x10))
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d A M I G A I C O N I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadWBINFOImage() reads an Amiga Workbench .info icon file and returns
%  it as one or more images.  Each icon generation (Classic, NewIcon,
%  GlowIcon, ARGB) becomes a separate image in the returned list.
%
%  The format of the ReadWBINFOImage method is:
%
%      Image *ReadWBINFOImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadWBINFOImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  size_t
    scene;

  unsigned short
    magic;

  unsigned int
    gadget_render,
    select_render,
    user_data,
    has_default_tool,
    has_tooltypes,
    has_drawer_data,
    has_tool_window;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"%s",
    image_info->filename);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
  {
    image=DestroyImageList(image);
    return((Image *) NULL);
  }
  /*
    Read 78-byte DiskObject header (big-endian).
  */
  magic=ReadBlobMSBShort(image);
  if (magic != AmigaIconMagic)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  (void) ReadBlobMSBShort(image);  /* version */
  (void) ReadBlobMSBLong(image);   /* gadget next */
  (void) ReadBlobMSBShort(image);  /* left_edge */
  (void) ReadBlobMSBShort(image);  /* top_edge */
  (void) ReadBlobMSBShort(image);  /* width */
  (void) ReadBlobMSBShort(image);  /* height */
  (void) ReadBlobMSBShort(image);  /* flags */
  (void) ReadBlobMSBShort(image);  /* activation */
  (void) ReadBlobMSBShort(image);  /* gadget_type */
  gadget_render=ReadBlobMSBLong(image);
  select_render=ReadBlobMSBLong(image);
  (void) ReadBlobMSBLong(image);   /* gadget_text */
  (void) ReadBlobMSBLong(image);   /* mutual_exclude */
  (void) ReadBlobMSBLong(image);   /* special_info */
  (void) ReadBlobMSBShort(image);  /* gadget_id */
  user_data=ReadBlobMSBLong(image);
  /* End of embedded Gadget (44 bytes from offset 4), now at offset 48 */
  (void) ReadBlobByte(image);      /* icon_type */
  (void) ReadBlobByte(image);      /* padding */
  has_default_tool=ReadBlobMSBLong(image);
  has_tooltypes=ReadBlobMSBLong(image);
  (void) ReadBlobMSBLong(image);   /* current_x */
  (void) ReadBlobMSBLong(image);   /* current_y */
  has_drawer_data=ReadBlobMSBLong(image);
  has_tool_window=ReadBlobMSBLong(image);
  (void) ReadBlobMSBLong(image);   /* stack_size */
  /* Now at offset 78 */
  if (EOFBlob(image) != MagickFalse)
    ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");

  scene=0;

  /*
    Skip DrawerData (56 bytes) if present.
  */
  if (has_drawer_data)
  {
    if (SeekBlob(image,56,SEEK_CUR) < 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  }

  /*
    Read classic planar image(s).
  */
  if (gadget_render)
  {
    if (ReadAndRenderClassicImage(&image,&scene,user_data,image_info,
        exception) == MagickFalse)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  }
  if (select_render)
  {
    if (ReadAndRenderClassicImage(&image,&scene,user_data,image_info,
        exception) == MagickFalse)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  }

  /*
    Skip DefaultTool if present.
  */
  if (has_default_tool)
  {
    unsigned int
      text_len;

    text_len=ReadBlobMSBLong(image);
    if (text_len > GetBlobSize(image))
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    if (SeekBlob(image,(MagickOffsetType) text_len,SEEK_CUR) < 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  }

  /*
    Read ToolTypes and decode NewIcons if present.
  */
  if (has_tooltypes)
  {
    unsigned int
      count_field;

    size_t
      num_entries = 0,
      tt_idx,
      im1_count,
      im1_alloc,
      im2_count,
      im2_alloc;

    char
      **tooltypes = (char **) NULL,
      **im1_lines = (char **) NULL,
      **im2_lines = (char **) NULL;

    if (EOFBlob(image) != MagickFalse)
      goto newicon_cleanup;
    count_field=ReadBlobMSBLong(image);
    if (count_field < 8)
      num_entries=0;
    else
      num_entries=(size_t) (count_field/4-1);
    if (num_entries > GetBlobSize(image) / 4)
      num_entries=0;
    /*
      Read all tooltype strings, collect IM1=/IM2= lines.
    */
    tooltypes=(char **) NULL;
    im1_lines=(char **) NULL;
    im2_lines=(char **) NULL;
    im1_count=0;
    im1_alloc=0;
    im2_count=0;
    im2_alloc=0;
    if (num_entries > 0)
    {
      tooltypes=(char **) AcquireQuantumMemory(num_entries,sizeof(*tooltypes));
      if (tooltypes == (char **) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      (void) memset(tooltypes,0,num_entries*sizeof(*tooltypes));
    }
    for (tt_idx=0; tt_idx < num_entries; tt_idx++)
    {
      unsigned int
        text_len;

      if (EOFBlob(image) != MagickFalse)
        break;
      text_len=ReadBlobMSBLong(image);
      if ((text_len == 0) || (text_len > GetBlobSize(image)))
        break;
      tooltypes[tt_idx]=(char *) AcquireQuantumMemory(text_len+1,
        sizeof(char));
      if (tooltypes[tt_idx] == (char *) NULL)
        break;
      if (ReadBlob(image,text_len,(unsigned char *) tooltypes[tt_idx]) !=
          (ssize_t) text_len)
      {
        tooltypes[tt_idx]=(char *) RelinquishMagickMemory(tooltypes[tt_idx]);
        tooltypes[tt_idx]=(char *) NULL;
        break;
      }
      tooltypes[tt_idx][text_len]='\0';
      if (strncmp(tooltypes[tt_idx],"IM1=",4) == 0)
      {
        if (im1_count >= im1_alloc)
        {
          im1_alloc=im1_alloc == 0 ? 16 : im1_alloc*2;
          im1_lines=(char **) ResizeQuantumMemory(im1_lines,im1_alloc,
            sizeof(*im1_lines));
          if (im1_lines == (char **) NULL)
            break;
        }
        im1_lines[im1_count++]=tooltypes[tt_idx]+4;
      }
      else if (strncmp(tooltypes[tt_idx],"IM2=",4) == 0)
      {
        if (im2_count >= im2_alloc)
        {
          im2_alloc=im2_alloc == 0 ? 16 : im2_alloc*2;
          im2_lines=(char **) ResizeQuantumMemory(im2_lines,im2_alloc,
            sizeof(*im2_lines));
          if (im2_lines == (char **) NULL)
            break;
        }
        im2_lines[im2_count++]=tooltypes[tt_idx]+4;
      }
    }
    /*
      Decode NewIcon from IM1= and IM2= lines.
    */
    (void) DecodeAndRenderNewIcon(&image,&scene,im1_lines,im1_count,
      image_info,exception);
    (void) DecodeAndRenderNewIcon(&image,&scene,im2_lines,im2_count,
      image_info,exception);
newicon_cleanup:
    if (im1_lines != (char **) NULL)
      im1_lines=(char **) RelinquishMagickMemory(im1_lines);
    if (im2_lines != (char **) NULL)
      im2_lines=(char **) RelinquishMagickMemory(im2_lines);
    if (tooltypes != (char **) NULL)
    {
      for (tt_idx=0; tt_idx < num_entries; tt_idx++)
        if (tooltypes[tt_idx] != (char *) NULL)
          tooltypes[tt_idx]=(char *) RelinquishMagickMemory(tooltypes[tt_idx]);
      tooltypes=(char **) RelinquishMagickMemory(tooltypes);
    }
  }

  /*
    Skip ToolWindow if present.
  */
  if (has_tool_window)
  {
    unsigned int
      text_len;

    text_len=ReadBlobMSBLong(image);
    if (text_len > GetBlobSize(image))
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    if (SeekBlob(image,(MagickOffsetType) text_len,SEEK_CUR) < 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  }

  /*
    Skip DrawerData2 (6 bytes) if present.
  */
  if (has_drawer_data && (user_data & 0xFF))
  {
    if (SeekBlob(image,6,SEEK_CUR) < 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  }

  /*
    Scan for FORM ICON (GlowIcons and ARGB images).
    Read all remaining data into a buffer and search for "FORM" + "ICON".
  */
  {
    MagickOffsetType
      current_offset;

    MagickSizeType
      blob_size;

    size_t
      remaining;

    unsigned char
      *rest_data;

    current_offset=TellBlob(image);
    blob_size=GetBlobSize(image);
    if ((current_offset >= 0) && ((MagickSizeType) current_offset < blob_size))
    {
      remaining=(size_t) (blob_size-(MagickSizeType) current_offset);
      if (remaining > 16)
      {
        rest_data=(unsigned char *) AcquireQuantumMemory(remaining,
          sizeof(*rest_data));
        if (rest_data != (unsigned char *) NULL)
        {
          ssize_t
            bytes_read;

          bytes_read=ReadBlob(image,remaining,rest_data);
          if (bytes_read > 12)
          {
            size_t
              search_pos;

            /*
              Search for FORM....ICON signature.
            */
            for (search_pos=0; search_pos+12 <= (size_t) bytes_read;
                 search_pos++)
            {
              if ((rest_data[search_pos] == 'F') &&
                  (rest_data[search_pos+1] == 'O') &&
                  (rest_data[search_pos+2] == 'R') &&
                  (rest_data[search_pos+3] == 'M') &&
                  (rest_data[search_pos+8] == 'I') &&
                  (rest_data[search_pos+9] == 'C') &&
                  (rest_data[search_pos+10] == 'O') &&
                  (rest_data[search_pos+11] == 'N'))
              {
                size_t
                  form_size,
                  form_end,
                  chunk_pos,
                  icon_width,
                  icon_height;

                form_size=((size_t) rest_data[search_pos+4] << 24) |
                  ((size_t) rest_data[search_pos+5] << 16) |
                  ((size_t) rest_data[search_pos+6] << 8) |
                  (size_t) rest_data[search_pos+7];
                if (form_size > (size_t) bytes_read-search_pos-8)
                  form_end=(size_t) bytes_read;
                else
                  form_end=search_pos+8+form_size;
                chunk_pos=search_pos+12;  /* past FORM+size+ICON */
                icon_width=0;
                icon_height=0;
                /*
                  Parse IFF chunks: FACE, IMAG, ARGB.
                */
                while (chunk_pos+8 <= form_end)
                {
                  unsigned int
                    chunk_id;

                  size_t
                    chunk_size,
                    chunk_data;

                  chunk_id=((unsigned int) rest_data[chunk_pos] << 24) |
                    ((unsigned int) rest_data[chunk_pos+1] << 16) |
                    ((unsigned int) rest_data[chunk_pos+2] << 8) |
                    (unsigned int) rest_data[chunk_pos+3];
                  chunk_size=((size_t) rest_data[chunk_pos+4] << 24) |
                    ((size_t) rest_data[chunk_pos+5] << 16) |
                    ((size_t) rest_data[chunk_pos+6] << 8) |
                    (size_t) rest_data[chunk_pos+7];
                  chunk_data=chunk_pos+8;
                  if ((chunk_size == 0) ||
                      (chunk_size > form_end-chunk_data))
                    break;
                  chunk_pos=chunk_data+chunk_size;
                  if (chunk_size & 1)
                    chunk_pos++;  /* IFF padding */

                  if (chunk_id == 0x46414345)  /* FACE */
                  {
                    if (chunk_size >= 4)
                    {
                      icon_width=(size_t) rest_data[chunk_data]+1;
                      icon_height=(size_t) rest_data[chunk_data+1]+1;
                    }
                  }
                  else if (chunk_id == 0x494D4147)  /* IMAG */
                  {
                    if ((chunk_size >= 10) && (icon_width > 0) &&
                        (icon_height > 0) &&
                        (icon_width <= MaxAmigaIconDimension) &&
                        (icon_height <= MaxAmigaIconDimension))
                    {
                      unsigned char
                        transparent_color,
                        num_colors_m1,
                        imag_flags,
                        image_format,
                        palette_format,
                        imag_depth;

                      size_t
                        image_size,
                        palette_size;

                      unsigned char
                        has_transparent,
                        has_palette;

                      unsigned char
                        *pixel_data,
                        *pal_data;

                      size_t
                        pixel_count,
                        pal_count,
                        num_pal_colors;

                      transparent_color=rest_data[chunk_data];
                      num_colors_m1=rest_data[chunk_data+1];
                      imag_flags=rest_data[chunk_data+2];
                      image_format=rest_data[chunk_data+3];
                      palette_format=rest_data[chunk_data+4];
                      imag_depth=rest_data[chunk_data+5];
                      image_size=
                        ((size_t) rest_data[chunk_data+6] << 8 |
                        (size_t) rest_data[chunk_data+7])+1;
                      palette_size=
                        ((size_t) rest_data[chunk_data+8] << 8 |
                        (size_t) rest_data[chunk_data+9])+1;
                      has_transparent=(imag_flags & 1) ? 1 : 0;
                      has_palette=(imag_flags & 2) ? 1 : 0;
                      num_pal_colors=(size_t) num_colors_m1+1;
                      if ((imag_depth == 0) || (imag_depth > 8))
                        break;
                      if ((form_end < chunk_data+10) ||
                          (image_size > form_end-chunk_data-10))
                        break;
                      /*
                        Decode image pixels.
                      */
                      if (image_format == 1)
                      {
                        pixel_data=DecodeAmigaRLE(
                          rest_data+chunk_data+10,image_size,
                          (size_t) imag_depth,
                          icon_width*icon_height,&pixel_count);
                      }
                      else
                      {
                        pixel_count=image_size;
                        if (pixel_count > icon_width*icon_height)
                          pixel_count=icon_width*icon_height;
                        pixel_data=(unsigned char *) AcquireQuantumMemory(
                          pixel_count+1,sizeof(*pixel_data));
                        if (pixel_data != (unsigned char *) NULL)
                          (void) memcpy(pixel_data,rest_data+chunk_data+10,
                            pixel_count);
                      }
                      if (pixel_data == (unsigned char *) NULL)
                        break;
                      /*
                        Decode palette.
                      */
                      pal_data=(unsigned char *) NULL;
                      pal_count=0;
                      if (has_palette)
                      {
                        if (palette_size <= form_end-chunk_data-10-image_size)
                        {
                          if (palette_format == 1)
                          {
                            pal_data=DecodeAmigaRLE(
                              rest_data+chunk_data+10+image_size,
                              palette_size,8,num_pal_colors*3,&pal_count);
                          }
                          else
                          {
                            pal_count=palette_size;
                            if (pal_count > num_pal_colors*3)
                              pal_count=num_pal_colors*3;
                            pal_data=(unsigned char *) AcquireQuantumMemory(
                              pal_count+1,sizeof(*pal_data));
                            if (pal_data != (unsigned char *) NULL)
                              (void) memcpy(pal_data,
                                rest_data+chunk_data+10+image_size,pal_count);
                          }
                        }
                      }
                      /*
                        Render GlowIcon to image.
                      */
                      if (AppendAmigaImage(&image,icon_width,icon_height,
                          scene,image_info,exception) == MagickFalse)
                      {
                        pixel_data=(unsigned char *)
                          RelinquishMagickMemory(pixel_data);
                        if (pal_data != (unsigned char *) NULL)
                          pal_data=(unsigned char *)
                            RelinquishMagickMemory(pal_data);
                        rest_data=(unsigned char *)
                          RelinquishMagickMemory(rest_data);
                        return(DestroyImageList(image));
                      }
                      RenderPalettedAmigaImage(image,icon_width,icon_height,
                        pixel_data,pixel_count,pal_data,pal_count/3,
                        has_transparent ? MagickTrue : MagickFalse,
                        transparent_color,exception);
                      pixel_data=(unsigned char *)
                        RelinquishMagickMemory(pixel_data);
                      if (pal_data != (unsigned char *) NULL)
                        pal_data=(unsigned char *)
                          RelinquishMagickMemory(pal_data);
                      scene++;
                      /*
                        Skip the selected IMAG (second one).
                      */
                    }
                  }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
                  else if (chunk_id == 0x41524742)  /* ARGB */
                  {
                    if ((chunk_size > 10) && (icon_width > 0) &&
                        (icon_height > 0) &&
                        (icon_width <= MaxAmigaIconDimension) &&
                        (icon_height <= MaxAmigaIconDimension))
                    {
                      unsigned char
                        *compressed,
                        *argb_raw;

                      size_t
                        comp_size;

                      uLongf
                        raw_size;

                      comp_size=chunk_size-10;
                      compressed=rest_data+chunk_data+10;
                      raw_size=(uLongf) (icon_width*icon_height*4);
                      argb_raw=(unsigned char *) AcquireQuantumMemory(
                        raw_size+1,sizeof(*argb_raw));
                      if (argb_raw != (unsigned char *) NULL)
                      {
                        if (uncompress(argb_raw,&raw_size,compressed,
                            (uLong) comp_size) == Z_OK)
                        {
                          if (AppendAmigaImage(&image,icon_width,
                              icon_height,scene,image_info,exception) !=
                              MagickFalse)
                          {
                            ssize_t
                              y;

                            for (y=0; y < (ssize_t) icon_height; y++)
                            {
                              Quantum
                                *q;

                              ssize_t
                                x;

                              q=QueueAuthenticPixels(image,0,y,icon_width,1,
                                exception);
                              if (q == (Quantum *) NULL)
                                break;
                              for (x=0; x < (ssize_t) icon_width; x++)
                              {
                                size_t
                                  pi;

                                pi=((size_t) y*icon_width+(size_t) x)*4;
                                if (pi+3 < (size_t) raw_size)
                                  SetAmigaPixelRGBA(image,q,argb_raw[pi+1],
                                    argb_raw[pi+2],argb_raw[pi+3],
                                    argb_raw[pi]);
                                else
                                  SetAmigaPixelRGBA(image,q,0,0,0,0);
                                q+=(ptrdiff_t) GetPixelChannels(image);
                              }
                              if (SyncAuthenticPixels(image,exception) ==
                                  MagickFalse)
                                break;
                            }
                            scene++;
                          }
                        }
                        argb_raw=(unsigned char *)
                          RelinquishMagickMemory(argb_raw);
                      }
                    }
                  }
#endif
                }
                break;  /* only process first FORM ICON */
              }
            }
          }
          rest_data=(unsigned char *) RelinquishMagickMemory(rest_data);
        }
      }
    }
  }

  if (CloseBlob(image) == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r A M I G A I C O N I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterWBINFOImage() adds attributes for the Amiga Workbench Icon format to
%  the list of supported formats.
%
%  The format of the RegisterWBINFOImage method is:
%
%      size_t RegisterWBINFOImage(void)
%
*/
ModuleExport size_t RegisterWBINFOImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("WBINFO","WBINFO",
    "Amiga Workbench Icon");
  entry->decoder=(DecodeImageHandler *) ReadWBINFOImage;
  entry->magick=(IsImageFormatHandler *) IsWBINFO;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->mime_type=ConstantString("image/x-amiga-icon");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r A M I G A I C O N I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterWBINFOImage() removes format registrations made by the
%  WBINFO module from the list of supported formats.
%
%  The format of the UnregisterWBINFOImage method is:
%
%      UnregisterWBINFOImage(void)
%
*/
ModuleExport void UnregisterWBINFOImage(void)
{
  (void) UnregisterMagickInfo("WBINFO");
}

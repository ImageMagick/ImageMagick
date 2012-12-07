/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            DDDD   DDDD   SSSSS                              %
%                            D   D  D   D  SS                                 %
%                            D   D  D   D   SSS                               %
%                            D   D  D   D     SS                              %
%                            DDDD   DDDD   SSSSS                              %
%                                                                             %
%                                                                             %
%              Read Microsoft Direct Draw Surface Image Format                %
%                                                                             %
%                              Software Design                                %
%                             Bianca van Schaik                               %
%                                March 2008                                   %
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
#include "magick/colorspace.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/profile.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/transform.h"
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/colorspace.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/compress.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum.h"
#include "magick/static.h"
#include "magick/string_.h"

/*
  Definitions
*/
#define DDSD_CAPS         0x00000001
#define DDSD_HEIGHT       0x00000002
#define DDSD_WIDTH        0x00000004
#define DDSD_PITCH        0x00000008
#define DDSD_PIXELFORMAT  0x00001000
#define DDSD_MIPMAPCOUNT  0x00020000
#define DDSD_LINEARSIZE   0x00080000
#define DDSD_DEPTH        0x00800000

#define DDPF_ALPHAPIXELS  0x00000001
#define DDPF_FOURCC       0x00000004
#define DDPF_RGB          0x00000040

#define FOURCC_DXT1       0x31545844
#define FOURCC_DXT3       0x33545844
#define FOURCC_DXT5       0x35545844

#define DDSCAPS_COMPLEX   0x00000008
#define DDSCAPS_TEXTURE   0x00001000
#define DDSCAPS_MIPMAP    0x00400000

#define DDSCAPS2_CUBEMAP  0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX  0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX  0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY  0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY  0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ  0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ  0x00008000
#define DDSCAPS2_VOLUME   0x00200000

/*
  Structure declarations.
*/
typedef struct _DDSPixelFormat
{
  size_t
    flags,
    fourcc,
    rgb_bitcount,
    r_bitmask,
    g_bitmask,
    b_bitmask,
    alpha_bitmask;
} DDSPixelFormat;

typedef struct _DDSInfo
{
  size_t
    flags,
    height,
    width,
    pitchOrLinearSize,
    depth,
    mipmapcount,
    ddscaps1,
    ddscaps2;

  DDSPixelFormat
    pixelformat;
} DDSInfo;

typedef struct _DDSColors
{
  unsigned char
    r[4],
    g[4],
    b[4],
    a[4];
} DDSColors;

typedef MagickBooleanType
  DDSDecoder(Image *,DDSInfo *);

/*
  Macros
*/
#define C565_r(x) (((x) & 0xF800) >> 11)
#define C565_g(x) (((x) & 0x07E0) >> 5)
#define C565_b(x)  ((x) & 0x001F)

#define C565_red(x)   ( (C565_r(x) << 3 | C565_r(x) >> 2))
#define C565_green(x) ( (C565_g(x) << 2 | C565_g(x) >> 4))
#define C565_blue(x)  ( (C565_b(x) << 3 | C565_b(x) >> 2))

#define DIV2(x)  ((x) > 1 ? ((x) >> 1) : 1)

/*
  Forward declarations
*/
static MagickBooleanType
  ReadDDSInfo(Image *image, DDSInfo *dds_info);

static void
  CalculateColors(unsigned short c0, unsigned short c1,
    DDSColors *c, MagickBooleanType ignoreAlpha);

static MagickBooleanType
  ReadDXT1(Image *image, DDSInfo *dds_info);

static MagickBooleanType
  ReadDXT3(Image *image, DDSInfo *dds_info);

static MagickBooleanType
  ReadDXT5(Image *image, DDSInfo *dds_info);

static MagickBooleanType
  ReadUncompressedRGB(Image *image, DDSInfo *dds_info);

static MagickBooleanType
  ReadUncompressedRGBA(Image *image, DDSInfo *dds_info);

static void
  SkipDXTMipmaps(Image *image, DDSInfo *dds_info, int texel_size);

static void
  SkipRGBMipmaps(Image *image, DDSInfo *dds_info, int pixel_size);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d D D S I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDDSImage() reads a DirectDraw Surface image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadDDSImage method is:
%
%      Image *ReadDDSImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: The image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t Min(size_t one, size_t two)
{
  if (one < two)
    return one;
  return two;
}

static Image *ReadDDSImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status,
    cubemap = MagickFalse,
    volume = MagickFalse,
    matte;

  CompressionType
    compression;

  DDSInfo
    dds_info;

  DDSDecoder
    *decoder;

  size_t
    n, num_images;

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
    Initialize image structure.
  */
  if (ReadDDSInfo(image, &dds_info) != MagickTrue) {
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  }

  if (dds_info.ddscaps2 & DDSCAPS2_CUBEMAP)
    cubemap = MagickTrue;

  if (dds_info.ddscaps2 & DDSCAPS2_VOLUME && dds_info.depth > 0)
    volume = MagickTrue;

  (void) SeekBlob(image, 128, SEEK_SET);

  /*
    Determine pixel format
  */
  if (dds_info.pixelformat.flags & DDPF_RGB)
    {
      compression = NoCompression;
      if (dds_info.pixelformat.flags & DDPF_ALPHAPIXELS)
        {
          matte = MagickTrue;
          decoder = ReadUncompressedRGBA;
        }
      else
        {
          matte = MagickTrue;
          decoder = ReadUncompressedRGB;
        }
    }
  else if (dds_info.pixelformat.flags & DDPF_FOURCC)
    {
      switch (dds_info.pixelformat.fourcc)
      {
        case FOURCC_DXT1:
        {
          matte = MagickFalse;
          compression = DXT1Compression;
          decoder = ReadDXT1;
          break;
        }

        case FOURCC_DXT3:
        {
          matte = MagickTrue;
          compression = DXT3Compression;
          decoder = ReadDXT3;
          break;
        }

        case FOURCC_DXT5:
        {
          matte = MagickTrue;
          compression = DXT5Compression;
          decoder = ReadDXT5;
          break;
        }

        default:
        {
          /* Unknown FOURCC */
          ThrowReaderException(CorruptImageError, "ImageTypeNotSupported");
        }
      }
    }
  else
    {
      /* Neither compressed nor uncompressed... thus unsupported */
      ThrowReaderException(CorruptImageError, "ImageTypeNotSupported");
    }

  num_images = 1;
  if (cubemap)
    {
      /*
        Determine number of faces defined in the cubemap
      */
      num_images = 0;
      if (dds_info.ddscaps2 & DDSCAPS2_CUBEMAP_POSITIVEX) num_images++;
      if (dds_info.ddscaps2 & DDSCAPS2_CUBEMAP_NEGATIVEX) num_images++;
      if (dds_info.ddscaps2 & DDSCAPS2_CUBEMAP_POSITIVEY) num_images++;
      if (dds_info.ddscaps2 & DDSCAPS2_CUBEMAP_NEGATIVEY) num_images++;
      if (dds_info.ddscaps2 & DDSCAPS2_CUBEMAP_POSITIVEZ) num_images++;
      if (dds_info.ddscaps2 & DDSCAPS2_CUBEMAP_NEGATIVEZ) num_images++;
    }

  if (volume)
    num_images = dds_info.depth;

  for (n = 0; n < num_images; n++)
  {
    if (n != 0)
      {
        /* Start a new image */
        AcquireNextImage(image_info,image);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image = DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
      }
 
    image->matte = matte;
    image->compression = compression;
    image->columns = dds_info.width;
    image->rows = dds_info.height;
    image->storage_class = DirectClass;
    image->endian = LSBEndian;
    image->depth = 8;
    if (image_info->ping != MagickFalse)
      {
        (void) CloseBlob(image);
        return(GetFirstImageInList(image));
      }

    if ((decoder)(image, &dds_info) != MagickTrue)
      {
        (void) CloseBlob(image);
        return(GetFirstImageInList(image));
      }
  }

  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);

  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

static MagickBooleanType ReadDDSInfo(Image *image, DDSInfo *dds_info)
{
  size_t
    hdr_size,
    required;

  /* Seek to start of header */
  (void) SeekBlob(image, 4, SEEK_SET);

  /* Check header field */
  hdr_size = ReadBlobLSBLong(image);
  if (hdr_size != 124)
    return MagickFalse;

  /* Fill in DDS info struct */
  dds_info->flags = ReadBlobLSBLong(image);

  /* Check required flags */
  required=(size_t) (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT);
  if ((dds_info->flags & required) != required)
    return MagickFalse;

  dds_info->height = ReadBlobLSBLong(image);
  dds_info->width = ReadBlobLSBLong(image);
  dds_info->pitchOrLinearSize = ReadBlobLSBLong(image);
  dds_info->depth = ReadBlobLSBLong(image);
  dds_info->mipmapcount = ReadBlobLSBLong(image);

  (void) SeekBlob(image, 44, SEEK_CUR);   /* reserved region of 11 DWORDs */

  /* Read pixel format structure */
  hdr_size = ReadBlobLSBLong(image);
  if (hdr_size != 32)
    return MagickFalse;

  dds_info->pixelformat.flags = ReadBlobLSBLong(image);
  dds_info->pixelformat.fourcc = ReadBlobLSBLong(image);
  dds_info->pixelformat.rgb_bitcount = ReadBlobLSBLong(image);
  dds_info->pixelformat.r_bitmask = ReadBlobLSBLong(image);
  dds_info->pixelformat.g_bitmask = ReadBlobLSBLong(image);
  dds_info->pixelformat.b_bitmask = ReadBlobLSBLong(image);
  dds_info->pixelformat.alpha_bitmask = ReadBlobLSBLong(image);

  dds_info->ddscaps1 = ReadBlobLSBLong(image);
  dds_info->ddscaps2 = ReadBlobLSBLong(image);
  (void) SeekBlob(image, 12, SEEK_CUR); /* 3 reserved DWORDs */

  return MagickTrue;
}

static void CalculateColors(unsigned short c0, unsigned short c1,
  DDSColors *c, MagickBooleanType ignoreAlpha)
{
  c->a[0] = c->a[1] = c->a[2] = c->a[3] = 0;

  c->r[0] = (unsigned char) C565_red(c0);
  c->g[0] = (unsigned char) C565_green(c0);
  c->b[0] = (unsigned char) C565_blue(c0);

  c->r[1] = (unsigned char) C565_red(c1);
  c->g[1] = (unsigned char) C565_green(c1);
  c->b[1] = (unsigned char) C565_blue(c1);

  if (ignoreAlpha == MagickTrue || c0 > c1)
    {
      c->r[2] = (unsigned char) ((2 * c->r[0] + c->r[1]) / 3);
      c->g[2] = (unsigned char) ((2 * c->g[0] + c->g[1]) / 3);
      c->b[2] = (unsigned char) ((2 * c->b[0] + c->b[1]) / 3);

      c->r[3] = (unsigned char) ((c->r[0] + 2 * c->r[1]) / 3);
      c->g[3] = (unsigned char) ((c->g[0] + 2 * c->g[1]) / 3);
      c->b[3] = (unsigned char) ((c->b[0] + 2 * c->b[1]) / 3);
    }
  else
    {
      c->r[2] = (unsigned char) ((c->r[0] + c->r[1]) / 2);
      c->g[2] = (unsigned char) ((c->g[0] + c->g[1]) / 2);
      c->b[2] = (unsigned char) ((c->b[0] + c->b[1]) / 2);

      c->r[3] = c->g[3] = c->b[3] = 0;
      c->a[3] = 255;
    }
}

static MagickBooleanType ReadDXT1(Image *image, DDSInfo *dds_info)
{
  DDSColors
    colors;

  ExceptionInfo
    *exception;

  PixelPacket
    *q;

  register ssize_t
    i,
    x;

  size_t
    bits;

  ssize_t
    j,
    y;

  unsigned char
    code;

  unsigned short
    c0,
    c1;

  exception=(&image->exception);
  for (y = 0; y < (ssize_t) dds_info->height; y += 4)
  {
    for (x = 0; x < (ssize_t) dds_info->width; x += 4)
    {
      /* Get 4x4 patch of pixels to write on */
      q = QueueAuthenticPixels(image, x, y, Min(4, dds_info->width - x),
        Min(4, dds_info->height - y),exception);

      if (q == (PixelPacket *) NULL)
        return MagickFalse;

      /* Read 8 bytes of data from the image */
      c0 = ReadBlobLSBShort(image);
      c1 = ReadBlobLSBShort(image);
      bits = ReadBlobLSBLong(image);

      CalculateColors(c0, c1, &colors, MagickFalse);

      /* Write the pixels */
      for (j = 0; j < 4; j++)
      {
        for (i = 0; i < 4; i++)
        {
          if ((x + i) < (ssize_t) dds_info->width && (y + j) < (ssize_t) dds_info->height)
            {
              code = (unsigned char) ((bits >> ((j*4+i)*2)) & 0x3);
              SetPixelRed(q,ScaleCharToQuantum(colors.r[code]));
              SetPixelGreen(q,ScaleCharToQuantum(colors.g[code]));
              SetPixelBlue(q,ScaleCharToQuantum(colors.b[code]));
              SetPixelOpacity(q,ScaleCharToQuantum(colors.a[code]));
              if (colors.a[code] && image->matte == MagickFalse)
                /* Correct matte */
                image->matte = MagickTrue;
              q++;
            }
        }
      }

      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        return MagickFalse;
    }
  }

  SkipDXTMipmaps(image, dds_info, 8);

  return MagickTrue;
}

static MagickBooleanType ReadDXT3(Image *image, DDSInfo *dds_info)
{
  DDSColors
    colors;

  ExceptionInfo
    *exception;

  ssize_t
    j,
    y;

  PixelPacket
    *q;

  register ssize_t
    i,
    x;

  unsigned char
    alpha;

  size_t
    a0,
    a1,
    bits,
    code;

  unsigned short
    c0,
    c1;

  exception=(&image->exception);
  for (y = 0; y < (ssize_t) dds_info->height; y += 4)
  {
    for (x = 0; x < (ssize_t) dds_info->width; x += 4)
    {
      /* Get 4x4 patch of pixels to write on */
      q = QueueAuthenticPixels(image, x, y, Min(4, dds_info->width - x),
                         Min(4, dds_info->height - y),exception);

      if (q == (PixelPacket *) NULL)
        return MagickFalse;

      /* Read alpha values (8 bytes) */
      a0 = ReadBlobLSBLong(image);
      a1 = ReadBlobLSBLong(image);

      /* Read 8 bytes of data from the image */
      c0 = ReadBlobLSBShort(image);
      c1 = ReadBlobLSBShort(image);
      bits = ReadBlobLSBLong(image);

      CalculateColors(c0, c1, &colors, MagickTrue);

      /* Write the pixels */
      for (j = 0; j < 4; j++)
      {
        for (i = 0; i < 4; i++)
        {
          if ((x + i) < (ssize_t) dds_info->width && (y + j) < (ssize_t) dds_info->height)
            {
              code = (bits >> ((4*j+i)*2)) & 0x3;
              SetPixelRed(q,ScaleCharToQuantum(colors.r[code]));
              SetPixelGreen(q,ScaleCharToQuantum(colors.g[code]));
              SetPixelBlue(q,ScaleCharToQuantum(colors.b[code]));
              /*
                Extract alpha value: multiply 0..15 by 17 to get range 0..255
              */
              if (j < 2)
                alpha = 17U * (unsigned char) ((a0 >> (4*(4*j+i))) & 0xf);
              else
                alpha = 17U * (unsigned char) ((a1 >> (4*(4*(j-2)+i))) & 0xf);
              SetPixelAlpha(q,ScaleCharToQuantum((unsigned char)
                alpha));
              q++;
            }
        }
      }

      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        return MagickFalse;
    }
  }

  SkipDXTMipmaps(image, dds_info, 16);

  return MagickTrue;
}

static MagickBooleanType ReadDXT5(Image *image, DDSInfo *dds_info)
{
  DDSColors
    colors;

  ExceptionInfo
    *exception;

  ssize_t
    j,
    y;

  MagickSizeType
    alpha_bits;

  PixelPacket
    *q;

  register ssize_t
    i,
    x;

  unsigned char
    a0,
    a1;

  size_t
    alpha,
    bits,
    code,
    alpha_code;

  unsigned short
    c0,
    c1;

  exception=(&image->exception);
  for (y = 0; y < (ssize_t) dds_info->height; y += 4)
  {
    for (x = 0; x < (ssize_t) dds_info->width; x += 4)
    {
      /* Get 4x4 patch of pixels to write on */
      q = QueueAuthenticPixels(image, x, y, Min(4, dds_info->width - x),
                         Min(4, dds_info->height - y),exception);

      if (q == (PixelPacket *) NULL)
        return MagickFalse;

      /* Read alpha values (8 bytes) */
      a0 = (unsigned char) ReadBlobByte(image);
      a1 = (unsigned char) ReadBlobByte(image);

      alpha_bits = (MagickSizeType)ReadBlobLSBLong(image)
                 | ((MagickSizeType)ReadBlobLSBShort(image) << 32);

      /* Read 8 bytes of data from the image */
      c0 = ReadBlobLSBShort(image);
      c1 = ReadBlobLSBShort(image);
      bits = ReadBlobLSBLong(image);

      CalculateColors(c0, c1, &colors, MagickTrue);

      /* Write the pixels */
      for (j = 0; j < 4; j++)
      {
        for (i = 0; i < 4; i++)
        {
          if ((x + i) < (ssize_t) dds_info->width && (y + j) < (ssize_t) dds_info->height)
            {
              code = (bits >> ((4*j+i)*2)) & 0x3;
              SetPixelRed(q,ScaleCharToQuantum(colors.r[code]));
              SetPixelGreen(q,ScaleCharToQuantum(colors.g[code]));
              SetPixelBlue(q,ScaleCharToQuantum(colors.b[code]));
              /* Extract alpha value */
              alpha_code = (size_t) (alpha_bits >> (3*(4*j+i))) & 0x7;
              if (alpha_code == 0)
                alpha = a0;
              else if (alpha_code == 1)
                alpha = a1;
              else if (a0 > a1)
                alpha = ((8-alpha_code) * a0 + (alpha_code-1) * a1) / 7;
              else if (alpha_code == 6)
                alpha = alpha_code;
              else if (alpha_code == 7)
                alpha = 255;
              else
                alpha = (((6-alpha_code) * a0 + (alpha_code-1) * a1) / 5);
              SetPixelAlpha(q,ScaleCharToQuantum((unsigned char)
                alpha));
              q++;
            }
        }
      }

      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        return MagickFalse;
    }
  }

  SkipDXTMipmaps(image, dds_info, 16);

  return MagickTrue;
}

static MagickBooleanType ReadUncompressedRGB(Image *image, DDSInfo *dds_info)
{
  ExceptionInfo
    *exception;

  ssize_t
    x, y;

  PixelPacket
    *q;

  exception=(&image->exception);
  for (y = 0; y < (ssize_t) dds_info->height; y++)
  {
    q = QueueAuthenticPixels(image, 0, y, dds_info->width, 1,exception);

    if (q == (PixelPacket *) NULL)
      return MagickFalse;

    for (x = 0; x < (ssize_t) dds_info->width; x++)
    {
      SetPixelBlue(q,ScaleCharToQuantum((unsigned char)
        ReadBlobByte(image)));
      SetPixelGreen(q,ScaleCharToQuantum((unsigned char)
        ReadBlobByte(image)));
      SetPixelRed(q,ScaleCharToQuantum((unsigned char)
        ReadBlobByte(image)));
      if (dds_info->pixelformat.rgb_bitcount == 32)
        (void) ReadBlobByte(image);
      q++;
    }
 
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      return MagickFalse;
  }

  SkipRGBMipmaps(image, dds_info, 3);

  return MagickTrue;
}

static MagickBooleanType ReadUncompressedRGBA(Image *image, DDSInfo *dds_info)
{
  ExceptionInfo
    *exception;

  ssize_t
    x, y;

  PixelPacket
    *q;

  exception=(&image->exception);
  for (y = 0; y < (ssize_t) dds_info->height; y++)
  {
    q = QueueAuthenticPixels(image, 0, y, dds_info->width, 1,exception);

    if (q == (PixelPacket *) NULL)
      return MagickFalse;

    for (x = 0; x < (ssize_t) dds_info->width; x++)
    {
      SetPixelBlue(q,ScaleCharToQuantum((unsigned char)
        ReadBlobByte(image)));
      SetPixelGreen(q,ScaleCharToQuantum((unsigned char)
        ReadBlobByte(image)));
      SetPixelRed(q,ScaleCharToQuantum((unsigned char)
        ReadBlobByte(image)));
      SetPixelAlpha(q,ScaleCharToQuantum((unsigned char)
        ReadBlobByte(image)));
      q++;
    }

    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      return MagickFalse;
  }

  SkipRGBMipmaps(image, dds_info, 4);

  return MagickTrue;
}

/*
  Skip the mipmap images for compressed (DXTn) dds files
*/
static void SkipDXTMipmaps(Image *image, DDSInfo *dds_info, int texel_size)
{
  register ssize_t
    i;

  MagickOffsetType
    offset;

  size_t
    h,
    w;

  /*
    Only skip mipmaps for textures and cube maps
  */
  if (dds_info->ddscaps1 & DDSCAPS_MIPMAP
      && (dds_info->ddscaps1 & DDSCAPS_TEXTURE
          || dds_info->ddscaps2 & DDSCAPS2_CUBEMAP))
    {
      w = DIV2(dds_info->width);
      h = DIV2(dds_info->height);

      /*
        Mipmapcount includes the main image, so start from one
      */
      for (i = 1; (i < (ssize_t) dds_info->mipmapcount) && w && h; i++)
      {
        offset = (MagickOffsetType) ((w + 3) / 4) * ((h + 3) / 4) * texel_size;
        (void) SeekBlob(image, offset, SEEK_CUR);

        w = DIV2(w);
        h = DIV2(h);
      }
    }
}

/*
  Skip the mipmap images for uncompressed (RGB or RGBA) dds files
*/
static void SkipRGBMipmaps(Image *image, DDSInfo *dds_info, int pixel_size)
{
  MagickOffsetType
    offset;

  register ssize_t
    i;

  size_t
    h,
    w;

  /*
    Only skip mipmaps for textures and cube maps
  */
  if (dds_info->ddscaps1 & DDSCAPS_MIPMAP
      && (dds_info->ddscaps1 & DDSCAPS_TEXTURE
          || dds_info->ddscaps2 & DDSCAPS2_CUBEMAP))
    {
      w = DIV2(dds_info->width);
      h = DIV2(dds_info->height);

      /*
        Mipmapcount includes the main image, so start from one
      */
      for (i=1; (i < (ssize_t) dds_info->mipmapcount) && w && h; i++)
      {
        offset = (MagickOffsetType) w * h * pixel_size;
        (void) SeekBlob(image, offset, SEEK_CUR);

        w = DIV2(w);
        h = DIV2(h);
      }
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s D D S                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsDDS() returns MagickTrue if the image format type, identified by the
%  magick string, is DDS.
%
%  The format of the IsDDS method is:
%
%      MagickBooleanType IsDDS(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsDDS(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (LocaleNCompare((char *) magick,"DDS ", 4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r D D S I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterDDSImage() adds attributes for the DDS image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterDDSImage method is:
%
%      RegisterDDSImage(void)
%
*/
ModuleExport size_t RegisterDDSImage(void)
{
  MagickInfo
    *entry;

  entry = SetMagickInfo("DDS");
  entry->decoder = (DecodeImageHandler *) ReadDDSImage;
  entry->magick = (IsImageFormatHandler *) IsDDS;
  entry->seekable_stream=MagickTrue;
  entry->description = ConstantString("Microsoft DirectDraw Surface");
  entry->module = ConstantString("DDS");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r D D S I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterDDSImage() removes format registrations made by the
%  DDS module from the list of supported formats.
%
%  The format of the UnregisterDDSImage method is:
%
%      UnregisterDDSImage(void)
%
*/
ModuleExport void UnregisterDDSImage(void)
{
  (void) UnregisterMagickInfo("DDS");
}


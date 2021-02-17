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
%           Read/Write Microsoft Direct Draw Surface Image Format             %
%                                                                             %
%                              Software Design                                %
%                             Bianca van Schaik                               %
%                                March 2008                                   %
%                               Dirk Lemstra                                  %
%                              September 2013                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
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
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/module.h"
#include "MagickCore/transform.h"

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
#define DDPF_LUMINANCE    0x00020000

#define FOURCC_DXT1       0x31545844
#define FOURCC_DXT3       0x33545844
#define FOURCC_DXT5       0x35545844
#define FOURCC_DX10       0x30315844

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

#define DDSEXT_DIMENSION_TEX2D      0x00000003
#define DDSEXTFLAGS_CUBEMAP         0x00000004

typedef enum DXGI_FORMAT 
{
  DXGI_FORMAT_UNKNOWN,
  DXGI_FORMAT_R32G32B32A32_TYPELESS,
  DXGI_FORMAT_R32G32B32A32_FLOAT,
  DXGI_FORMAT_R32G32B32A32_UINT,
  DXGI_FORMAT_R32G32B32A32_SINT,
  DXGI_FORMAT_R32G32B32_TYPELESS,
  DXGI_FORMAT_R32G32B32_FLOAT,
  DXGI_FORMAT_R32G32B32_UINT,
  DXGI_FORMAT_R32G32B32_SINT,
  DXGI_FORMAT_R16G16B16A16_TYPELESS,
  DXGI_FORMAT_R16G16B16A16_FLOAT,
  DXGI_FORMAT_R16G16B16A16_UNORM,
  DXGI_FORMAT_R16G16B16A16_UINT,
  DXGI_FORMAT_R16G16B16A16_SNORM,
  DXGI_FORMAT_R16G16B16A16_SINT,
  DXGI_FORMAT_R32G32_TYPELESS,
  DXGI_FORMAT_R32G32_FLOAT,
  DXGI_FORMAT_R32G32_UINT,
  DXGI_FORMAT_R32G32_SINT,
  DXGI_FORMAT_R32G8X24_TYPELESS,
  DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
  DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
  DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
  DXGI_FORMAT_R10G10B10A2_TYPELESS,
  DXGI_FORMAT_R10G10B10A2_UNORM,
  DXGI_FORMAT_R10G10B10A2_UINT,
  DXGI_FORMAT_R11G11B10_FLOAT,
  DXGI_FORMAT_R8G8B8A8_TYPELESS,
  DXGI_FORMAT_R8G8B8A8_UNORM,
  DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
  DXGI_FORMAT_R8G8B8A8_UINT,
  DXGI_FORMAT_R8G8B8A8_SNORM,
  DXGI_FORMAT_R8G8B8A8_SINT,
  DXGI_FORMAT_R16G16_TYPELESS,
  DXGI_FORMAT_R16G16_FLOAT,
  DXGI_FORMAT_R16G16_UNORM,
  DXGI_FORMAT_R16G16_UINT,
  DXGI_FORMAT_R16G16_SNORM,
  DXGI_FORMAT_R16G16_SINT,
  DXGI_FORMAT_R32_TYPELESS,
  DXGI_FORMAT_D32_FLOAT,
  DXGI_FORMAT_R32_FLOAT,
  DXGI_FORMAT_R32_UINT,
  DXGI_FORMAT_R32_SINT,
  DXGI_FORMAT_R24G8_TYPELESS,
  DXGI_FORMAT_D24_UNORM_S8_UINT,
  DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
  DXGI_FORMAT_X24_TYPELESS_G8_UINT,
  DXGI_FORMAT_R8G8_TYPELESS,
  DXGI_FORMAT_R8G8_UNORM,
  DXGI_FORMAT_R8G8_UINT,
  DXGI_FORMAT_R8G8_SNORM,
  DXGI_FORMAT_R8G8_SINT,
  DXGI_FORMAT_R16_TYPELESS,
  DXGI_FORMAT_R16_FLOAT,
  DXGI_FORMAT_D16_UNORM,
  DXGI_FORMAT_R16_UNORM,
  DXGI_FORMAT_R16_UINT,
  DXGI_FORMAT_R16_SNORM,
  DXGI_FORMAT_R16_SINT,
  DXGI_FORMAT_R8_TYPELESS,
  DXGI_FORMAT_R8_UNORM,
  DXGI_FORMAT_R8_UINT,
  DXGI_FORMAT_R8_SNORM,
  DXGI_FORMAT_R8_SINT,
  DXGI_FORMAT_A8_UNORM,
  DXGI_FORMAT_R1_UNORM,
  DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
  DXGI_FORMAT_R8G8_B8G8_UNORM,
  DXGI_FORMAT_G8R8_G8B8_UNORM,
  DXGI_FORMAT_BC1_TYPELESS,
  DXGI_FORMAT_BC1_UNORM,
  DXGI_FORMAT_BC1_UNORM_SRGB,
  DXGI_FORMAT_BC2_TYPELESS,
  DXGI_FORMAT_BC2_UNORM,
  DXGI_FORMAT_BC2_UNORM_SRGB,
  DXGI_FORMAT_BC3_TYPELESS,
  DXGI_FORMAT_BC3_UNORM,
  DXGI_FORMAT_BC3_UNORM_SRGB,
  DXGI_FORMAT_BC4_TYPELESS,
  DXGI_FORMAT_BC4_UNORM,
  DXGI_FORMAT_BC4_SNORM,
  DXGI_FORMAT_BC5_TYPELESS,
  DXGI_FORMAT_BC5_UNORM,
  DXGI_FORMAT_BC5_SNORM,
  DXGI_FORMAT_B5G6R5_UNORM,
  DXGI_FORMAT_B5G5R5A1_UNORM,
  DXGI_FORMAT_B8G8R8A8_UNORM,
  DXGI_FORMAT_B8G8R8X8_UNORM,
  DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
  DXGI_FORMAT_B8G8R8A8_TYPELESS,
  DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
  DXGI_FORMAT_B8G8R8X8_TYPELESS,
  DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
  DXGI_FORMAT_BC6H_TYPELESS,
  DXGI_FORMAT_BC6H_UF16,
  DXGI_FORMAT_BC6H_SF16,
  DXGI_FORMAT_BC7_TYPELESS,
  DXGI_FORMAT_BC7_UNORM,
  DXGI_FORMAT_BC7_UNORM_SRGB,
  DXGI_FORMAT_AYUV,
  DXGI_FORMAT_Y410,
  DXGI_FORMAT_Y416,
  DXGI_FORMAT_NV12,
  DXGI_FORMAT_P010,
  DXGI_FORMAT_P016,
  DXGI_FORMAT_420_OPAQUE,
  DXGI_FORMAT_YUY2,
  DXGI_FORMAT_Y210,
  DXGI_FORMAT_Y216,
  DXGI_FORMAT_NV11,
  DXGI_FORMAT_AI44,
  DXGI_FORMAT_IA44,
  DXGI_FORMAT_P8,
  DXGI_FORMAT_A8P8,
  DXGI_FORMAT_B4G4R4A4_UNORM,
  DXGI_FORMAT_P208,
  DXGI_FORMAT_V208,
  DXGI_FORMAT_V408,
  DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE,
  DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE,
  DXGI_FORMAT_FORCE_UINT
} DXGI_FORMAT;


#ifndef SIZE_MAX
#define SIZE_MAX ((size_t) -1)
#endif

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
    ddscaps2,
    extFormat,
    extDimension,
    extFlags,
    extArraySize,
    extFlags2;
  
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

typedef struct _DDSVector4
{
  float
    x,
    y,
    z,
    w;
} DDSVector4;

typedef struct _DDSVector3
{
  float
    x,
    y,
    z;
} DDSVector3;

typedef struct _DDSSourceBlock
{
  unsigned char
    start,
    end,
    error;
} DDSSourceBlock;

typedef struct _DDSSingleColorLookup
{
  DDSSourceBlock sources[2];
} DDSSingleColorLookup;

typedef MagickBooleanType
  DDSDecoder(const ImageInfo *,Image *,DDSInfo *,const MagickBooleanType,
    ExceptionInfo *);

typedef MagickBooleanType
  DDSPixelDecoder(Image *,DDSInfo *,ExceptionInfo *);

static const DDSSingleColorLookup DDSLookup_5_4[] =
{
  { { { 0, 0, 0 }, { 0, 0, 0 } } },
  { { { 0, 0, 1 }, { 0, 1, 1 } } },
  { { { 0, 0, 2 }, { 0, 1, 0 } } },
  { { { 0, 0, 3 }, { 0, 1, 1 } } },
  { { { 0, 0, 4 }, { 0, 2, 1 } } },
  { { { 1, 0, 3 }, { 0, 2, 0 } } },
  { { { 1, 0, 2 }, { 0, 2, 1 } } },
  { { { 1, 0, 1 }, { 0, 3, 1 } } },
  { { { 1, 0, 0 }, { 0, 3, 0 } } },
  { { { 1, 0, 1 }, { 1, 2, 1 } } },
  { { { 1, 0, 2 }, { 1, 2, 0 } } },
  { { { 1, 0, 3 }, { 0, 4, 0 } } },
  { { { 1, 0, 4 }, { 0, 5, 1 } } },
  { { { 2, 0, 3 }, { 0, 5, 0 } } },
  { { { 2, 0, 2 }, { 0, 5, 1 } } },
  { { { 2, 0, 1 }, { 0, 6, 1 } } },
  { { { 2, 0, 0 }, { 0, 6, 0 } } },
  { { { 2, 0, 1 }, { 2, 3, 1 } } },
  { { { 2, 0, 2 }, { 2, 3, 0 } } },
  { { { 2, 0, 3 }, { 0, 7, 0 } } },
  { { { 2, 0, 4 }, { 1, 6, 1 } } },
  { { { 3, 0, 3 }, { 1, 6, 0 } } },
  { { { 3, 0, 2 }, { 0, 8, 0 } } },
  { { { 3, 0, 1 }, { 0, 9, 1 } } },
  { { { 3, 0, 0 }, { 0, 9, 0 } } },
  { { { 3, 0, 1 }, { 0, 9, 1 } } },
  { { { 3, 0, 2 }, { 0, 10, 1 } } },
  { { { 3, 0, 3 }, { 0, 10, 0 } } },
  { { { 3, 0, 4 }, { 2, 7, 1 } } },
  { { { 4, 0, 4 }, { 2, 7, 0 } } },
  { { { 4, 0, 3 }, { 0, 11, 0 } } },
  { { { 4, 0, 2 }, { 1, 10, 1 } } },
  { { { 4, 0, 1 }, { 1, 10, 0 } } },
  { { { 4, 0, 0 }, { 0, 12, 0 } } },
  { { { 4, 0, 1 }, { 0, 13, 1 } } },
  { { { 4, 0, 2 }, { 0, 13, 0 } } },
  { { { 4, 0, 3 }, { 0, 13, 1 } } },
  { { { 4, 0, 4 }, { 0, 14, 1 } } },
  { { { 5, 0, 3 }, { 0, 14, 0 } } },
  { { { 5, 0, 2 }, { 2, 11, 1 } } },
  { { { 5, 0, 1 }, { 2, 11, 0 } } },
  { { { 5, 0, 0 }, { 0, 15, 0 } } },
  { { { 5, 0, 1 }, { 1, 14, 1 } } },
  { { { 5, 0, 2 }, { 1, 14, 0 } } },
  { { { 5, 0, 3 }, { 0, 16, 0 } } },
  { { { 5, 0, 4 }, { 0, 17, 1 } } },
  { { { 6, 0, 3 }, { 0, 17, 0 } } },
  { { { 6, 0, 2 }, { 0, 17, 1 } } },
  { { { 6, 0, 1 }, { 0, 18, 1 } } },
  { { { 6, 0, 0 }, { 0, 18, 0 } } },
  { { { 6, 0, 1 }, { 2, 15, 1 } } },
  { { { 6, 0, 2 }, { 2, 15, 0 } } },
  { { { 6, 0, 3 }, { 0, 19, 0 } } },
  { { { 6, 0, 4 }, { 1, 18, 1 } } },
  { { { 7, 0, 3 }, { 1, 18, 0 } } },
  { { { 7, 0, 2 }, { 0, 20, 0 } } },
  { { { 7, 0, 1 }, { 0, 21, 1 } } },
  { { { 7, 0, 0 }, { 0, 21, 0 } } },
  { { { 7, 0, 1 }, { 0, 21, 1 } } },
  { { { 7, 0, 2 }, { 0, 22, 1 } } },
  { { { 7, 0, 3 }, { 0, 22, 0 } } },
  { { { 7, 0, 4 }, { 2, 19, 1 } } },
  { { { 8, 0, 4 }, { 2, 19, 0 } } },
  { { { 8, 0, 3 }, { 0, 23, 0 } } },
  { { { 8, 0, 2 }, { 1, 22, 1 } } },
  { { { 8, 0, 1 }, { 1, 22, 0 } } },
  { { { 8, 0, 0 }, { 0, 24, 0 } } },
  { { { 8, 0, 1 }, { 0, 25, 1 } } },
  { { { 8, 0, 2 }, { 0, 25, 0 } } },
  { { { 8, 0, 3 }, { 0, 25, 1 } } },
  { { { 8, 0, 4 }, { 0, 26, 1 } } },
  { { { 9, 0, 3 }, { 0, 26, 0 } } },
  { { { 9, 0, 2 }, { 2, 23, 1 } } },
  { { { 9, 0, 1 }, { 2, 23, 0 } } },
  { { { 9, 0, 0 }, { 0, 27, 0 } } },
  { { { 9, 0, 1 }, { 1, 26, 1 } } },
  { { { 9, 0, 2 }, { 1, 26, 0 } } },
  { { { 9, 0, 3 }, { 0, 28, 0 } } },
  { { { 9, 0, 4 }, { 0, 29, 1 } } },
  { { { 10, 0, 3 }, { 0, 29, 0 } } },
  { { { 10, 0, 2 }, { 0, 29, 1 } } },
  { { { 10, 0, 1 }, { 0, 30, 1 } } },
  { { { 10, 0, 0 }, { 0, 30, 0 } } },
  { { { 10, 0, 1 }, { 2, 27, 1 } } },
  { { { 10, 0, 2 }, { 2, 27, 0 } } },
  { { { 10, 0, 3 }, { 0, 31, 0 } } },
  { { { 10, 0, 4 }, { 1, 30, 1 } } },
  { { { 11, 0, 3 }, { 1, 30, 0 } } },
  { { { 11, 0, 2 }, { 4, 24, 0 } } },
  { { { 11, 0, 1 }, { 1, 31, 1 } } },
  { { { 11, 0, 0 }, { 1, 31, 0 } } },
  { { { 11, 0, 1 }, { 1, 31, 1 } } },
  { { { 11, 0, 2 }, { 2, 30, 1 } } },
  { { { 11, 0, 3 }, { 2, 30, 0 } } },
  { { { 11, 0, 4 }, { 2, 31, 1 } } },
  { { { 12, 0, 4 }, { 2, 31, 0 } } },
  { { { 12, 0, 3 }, { 4, 27, 0 } } },
  { { { 12, 0, 2 }, { 3, 30, 1 } } },
  { { { 12, 0, 1 }, { 3, 30, 0 } } },
  { { { 12, 0, 0 }, { 4, 28, 0 } } },
  { { { 12, 0, 1 }, { 3, 31, 1 } } },
  { { { 12, 0, 2 }, { 3, 31, 0 } } },
  { { { 12, 0, 3 }, { 3, 31, 1 } } },
  { { { 12, 0, 4 }, { 4, 30, 1 } } },
  { { { 13, 0, 3 }, { 4, 30, 0 } } },
  { { { 13, 0, 2 }, { 6, 27, 1 } } },
  { { { 13, 0, 1 }, { 6, 27, 0 } } },
  { { { 13, 0, 0 }, { 4, 31, 0 } } },
  { { { 13, 0, 1 }, { 5, 30, 1 } } },
  { { { 13, 0, 2 }, { 5, 30, 0 } } },
  { { { 13, 0, 3 }, { 8, 24, 0 } } },
  { { { 13, 0, 4 }, { 5, 31, 1 } } },
  { { { 14, 0, 3 }, { 5, 31, 0 } } },
  { { { 14, 0, 2 }, { 5, 31, 1 } } },
  { { { 14, 0, 1 }, { 6, 30, 1 } } },
  { { { 14, 0, 0 }, { 6, 30, 0 } } },
  { { { 14, 0, 1 }, { 6, 31, 1 } } },
  { { { 14, 0, 2 }, { 6, 31, 0 } } },
  { { { 14, 0, 3 }, { 8, 27, 0 } } },
  { { { 14, 0, 4 }, { 7, 30, 1 } } },
  { { { 15, 0, 3 }, { 7, 30, 0 } } },
  { { { 15, 0, 2 }, { 8, 28, 0 } } },
  { { { 15, 0, 1 }, { 7, 31, 1 } } },
  { { { 15, 0, 0 }, { 7, 31, 0 } } },
  { { { 15, 0, 1 }, { 7, 31, 1 } } },
  { { { 15, 0, 2 }, { 8, 30, 1 } } },
  { { { 15, 0, 3 }, { 8, 30, 0 } } },
  { { { 15, 0, 4 }, { 10, 27, 1 } } },
  { { { 16, 0, 4 }, { 10, 27, 0 } } },
  { { { 16, 0, 3 }, { 8, 31, 0 } } },
  { { { 16, 0, 2 }, { 9, 30, 1 } } },
  { { { 16, 0, 1 }, { 9, 30, 0 } } },
  { { { 16, 0, 0 }, { 12, 24, 0 } } },
  { { { 16, 0, 1 }, { 9, 31, 1 } } },
  { { { 16, 0, 2 }, { 9, 31, 0 } } },
  { { { 16, 0, 3 }, { 9, 31, 1 } } },
  { { { 16, 0, 4 }, { 10, 30, 1 } } },
  { { { 17, 0, 3 }, { 10, 30, 0 } } },
  { { { 17, 0, 2 }, { 10, 31, 1 } } },
  { { { 17, 0, 1 }, { 10, 31, 0 } } },
  { { { 17, 0, 0 }, { 12, 27, 0 } } },
  { { { 17, 0, 1 }, { 11, 30, 1 } } },
  { { { 17, 0, 2 }, { 11, 30, 0 } } },
  { { { 17, 0, 3 }, { 12, 28, 0 } } },
  { { { 17, 0, 4 }, { 11, 31, 1 } } },
  { { { 18, 0, 3 }, { 11, 31, 0 } } },
  { { { 18, 0, 2 }, { 11, 31, 1 } } },
  { { { 18, 0, 1 }, { 12, 30, 1 } } },
  { { { 18, 0, 0 }, { 12, 30, 0 } } },
  { { { 18, 0, 1 }, { 14, 27, 1 } } },
  { { { 18, 0, 2 }, { 14, 27, 0 } } },
  { { { 18, 0, 3 }, { 12, 31, 0 } } },
  { { { 18, 0, 4 }, { 13, 30, 1 } } },
  { { { 19, 0, 3 }, { 13, 30, 0 } } },
  { { { 19, 0, 2 }, { 16, 24, 0 } } },
  { { { 19, 0, 1 }, { 13, 31, 1 } } },
  { { { 19, 0, 0 }, { 13, 31, 0 } } },
  { { { 19, 0, 1 }, { 13, 31, 1 } } },
  { { { 19, 0, 2 }, { 14, 30, 1 } } },
  { { { 19, 0, 3 }, { 14, 30, 0 } } },
  { { { 19, 0, 4 }, { 14, 31, 1 } } },
  { { { 20, 0, 4 }, { 14, 31, 0 } } },
  { { { 20, 0, 3 }, { 16, 27, 0 } } },
  { { { 20, 0, 2 }, { 15, 30, 1 } } },
  { { { 20, 0, 1 }, { 15, 30, 0 } } },
  { { { 20, 0, 0 }, { 16, 28, 0 } } },
  { { { 20, 0, 1 }, { 15, 31, 1 } } },
  { { { 20, 0, 2 }, { 15, 31, 0 } } },
  { { { 20, 0, 3 }, { 15, 31, 1 } } },
  { { { 20, 0, 4 }, { 16, 30, 1 } } },
  { { { 21, 0, 3 }, { 16, 30, 0 } } },
  { { { 21, 0, 2 }, { 18, 27, 1 } } },
  { { { 21, 0, 1 }, { 18, 27, 0 } } },
  { { { 21, 0, 0 }, { 16, 31, 0 } } },
  { { { 21, 0, 1 }, { 17, 30, 1 } } },
  { { { 21, 0, 2 }, { 17, 30, 0 } } },
  { { { 21, 0, 3 }, { 20, 24, 0 } } },
  { { { 21, 0, 4 }, { 17, 31, 1 } } },
  { { { 22, 0, 3 }, { 17, 31, 0 } } },
  { { { 22, 0, 2 }, { 17, 31, 1 } } },
  { { { 22, 0, 1 }, { 18, 30, 1 } } },
  { { { 22, 0, 0 }, { 18, 30, 0 } } },
  { { { 22, 0, 1 }, { 18, 31, 1 } } },
  { { { 22, 0, 2 }, { 18, 31, 0 } } },
  { { { 22, 0, 3 }, { 20, 27, 0 } } },
  { { { 22, 0, 4 }, { 19, 30, 1 } } },
  { { { 23, 0, 3 }, { 19, 30, 0 } } },
  { { { 23, 0, 2 }, { 20, 28, 0 } } },
  { { { 23, 0, 1 }, { 19, 31, 1 } } },
  { { { 23, 0, 0 }, { 19, 31, 0 } } },
  { { { 23, 0, 1 }, { 19, 31, 1 } } },
  { { { 23, 0, 2 }, { 20, 30, 1 } } },
  { { { 23, 0, 3 }, { 20, 30, 0 } } },
  { { { 23, 0, 4 }, { 22, 27, 1 } } },
  { { { 24, 0, 4 }, { 22, 27, 0 } } },
  { { { 24, 0, 3 }, { 20, 31, 0 } } },
  { { { 24, 0, 2 }, { 21, 30, 1 } } },
  { { { 24, 0, 1 }, { 21, 30, 0 } } },
  { { { 24, 0, 0 }, { 24, 24, 0 } } },
  { { { 24, 0, 1 }, { 21, 31, 1 } } },
  { { { 24, 0, 2 }, { 21, 31, 0 } } },
  { { { 24, 0, 3 }, { 21, 31, 1 } } },
  { { { 24, 0, 4 }, { 22, 30, 1 } } },
  { { { 25, 0, 3 }, { 22, 30, 0 } } },
  { { { 25, 0, 2 }, { 22, 31, 1 } } },
  { { { 25, 0, 1 }, { 22, 31, 0 } } },
  { { { 25, 0, 0 }, { 24, 27, 0 } } },
  { { { 25, 0, 1 }, { 23, 30, 1 } } },
  { { { 25, 0, 2 }, { 23, 30, 0 } } },
  { { { 25, 0, 3 }, { 24, 28, 0 } } },
  { { { 25, 0, 4 }, { 23, 31, 1 } } },
  { { { 26, 0, 3 }, { 23, 31, 0 } } },
  { { { 26, 0, 2 }, { 23, 31, 1 } } },
  { { { 26, 0, 1 }, { 24, 30, 1 } } },
  { { { 26, 0, 0 }, { 24, 30, 0 } } },
  { { { 26, 0, 1 }, { 26, 27, 1 } } },
  { { { 26, 0, 2 }, { 26, 27, 0 } } },
  { { { 26, 0, 3 }, { 24, 31, 0 } } },
  { { { 26, 0, 4 }, { 25, 30, 1 } } },
  { { { 27, 0, 3 }, { 25, 30, 0 } } },
  { { { 27, 0, 2 }, { 28, 24, 0 } } },
  { { { 27, 0, 1 }, { 25, 31, 1 } } },
  { { { 27, 0, 0 }, { 25, 31, 0 } } },
  { { { 27, 0, 1 }, { 25, 31, 1 } } },
  { { { 27, 0, 2 }, { 26, 30, 1 } } },
  { { { 27, 0, 3 }, { 26, 30, 0 } } },
  { { { 27, 0, 4 }, { 26, 31, 1 } } },
  { { { 28, 0, 4 }, { 26, 31, 0 } } },
  { { { 28, 0, 3 }, { 28, 27, 0 } } },
  { { { 28, 0, 2 }, { 27, 30, 1 } } },
  { { { 28, 0, 1 }, { 27, 30, 0 } } },
  { { { 28, 0, 0 }, { 28, 28, 0 } } },
  { { { 28, 0, 1 }, { 27, 31, 1 } } },
  { { { 28, 0, 2 }, { 27, 31, 0 } } },
  { { { 28, 0, 3 }, { 27, 31, 1 } } },
  { { { 28, 0, 4 }, { 28, 30, 1 } } },
  { { { 29, 0, 3 }, { 28, 30, 0 } } },
  { { { 29, 0, 2 }, { 30, 27, 1 } } },
  { { { 29, 0, 1 }, { 30, 27, 0 } } },
  { { { 29, 0, 0 }, { 28, 31, 0 } } },
  { { { 29, 0, 1 }, { 29, 30, 1 } } },
  { { { 29, 0, 2 }, { 29, 30, 0 } } },
  { { { 29, 0, 3 }, { 29, 30, 1 } } },
  { { { 29, 0, 4 }, { 29, 31, 1 } } },
  { { { 30, 0, 3 }, { 29, 31, 0 } } },
  { { { 30, 0, 2 }, { 29, 31, 1 } } },
  { { { 30, 0, 1 }, { 30, 30, 1 } } },
  { { { 30, 0, 0 }, { 30, 30, 0 } } },
  { { { 30, 0, 1 }, { 30, 31, 1 } } },
  { { { 30, 0, 2 }, { 30, 31, 0 } } },
  { { { 30, 0, 3 }, { 30, 31, 1 } } },
  { { { 30, 0, 4 }, { 31, 30, 1 } } },
  { { { 31, 0, 3 }, { 31, 30, 0 } } },
  { { { 31, 0, 2 }, { 31, 30, 1 } } },
  { { { 31, 0, 1 }, { 31, 31, 1 } } },
  { { { 31, 0, 0 }, { 31, 31, 0 } } }
};

static const DDSSingleColorLookup DDSLookup_6_4[] =
{
  { { { 0, 0, 0 }, { 0, 0, 0 } } },
  { { { 0, 0, 1 }, { 0, 1, 0 } } },
  { { { 0, 0, 2 }, { 0, 2, 0 } } },
  { { { 1, 0, 1 }, { 0, 3, 1 } } },
  { { { 1, 0, 0 }, { 0, 3, 0 } } },
  { { { 1, 0, 1 }, { 0, 4, 0 } } },
  { { { 1, 0, 2 }, { 0, 5, 0 } } },
  { { { 2, 0, 1 }, { 0, 6, 1 } } },
  { { { 2, 0, 0 }, { 0, 6, 0 } } },
  { { { 2, 0, 1 }, { 0, 7, 0 } } },
  { { { 2, 0, 2 }, { 0, 8, 0 } } },
  { { { 3, 0, 1 }, { 0, 9, 1 } } },
  { { { 3, 0, 0 }, { 0, 9, 0 } } },
  { { { 3, 0, 1 }, { 0, 10, 0 } } },
  { { { 3, 0, 2 }, { 0, 11, 0 } } },
  { { { 4, 0, 1 }, { 0, 12, 1 } } },
  { { { 4, 0, 0 }, { 0, 12, 0 } } },
  { { { 4, 0, 1 }, { 0, 13, 0 } } },
  { { { 4, 0, 2 }, { 0, 14, 0 } } },
  { { { 5, 0, 1 }, { 0, 15, 1 } } },
  { { { 5, 0, 0 }, { 0, 15, 0 } } },
  { { { 5, 0, 1 }, { 0, 16, 0 } } },
  { { { 5, 0, 2 }, { 1, 15, 0 } } },
  { { { 6, 0, 1 }, { 0, 17, 0 } } },
  { { { 6, 0, 0 }, { 0, 18, 0 } } },
  { { { 6, 0, 1 }, { 0, 19, 0 } } },
  { { { 6, 0, 2 }, { 3, 14, 0 } } },
  { { { 7, 0, 1 }, { 0, 20, 0 } } },
  { { { 7, 0, 0 }, { 0, 21, 0 } } },
  { { { 7, 0, 1 }, { 0, 22, 0 } } },
  { { { 7, 0, 2 }, { 4, 15, 0 } } },
  { { { 8, 0, 1 }, { 0, 23, 0 } } },
  { { { 8, 0, 0 }, { 0, 24, 0 } } },
  { { { 8, 0, 1 }, { 0, 25, 0 } } },
  { { { 8, 0, 2 }, { 6, 14, 0 } } },
  { { { 9, 0, 1 }, { 0, 26, 0 } } },
  { { { 9, 0, 0 }, { 0, 27, 0 } } },
  { { { 9, 0, 1 }, { 0, 28, 0 } } },
  { { { 9, 0, 2 }, { 7, 15, 0 } } },
  { { { 10, 0, 1 }, { 0, 29, 0 } } },
  { { { 10, 0, 0 }, { 0, 30, 0 } } },
  { { { 10, 0, 1 }, { 0, 31, 0 } } },
  { { { 10, 0, 2 }, { 9, 14, 0 } } },
  { { { 11, 0, 1 }, { 0, 32, 0 } } },
  { { { 11, 0, 0 }, { 0, 33, 0 } } },
  { { { 11, 0, 1 }, { 2, 30, 0 } } },
  { { { 11, 0, 2 }, { 0, 34, 0 } } },
  { { { 12, 0, 1 }, { 0, 35, 0 } } },
  { { { 12, 0, 0 }, { 0, 36, 0 } } },
  { { { 12, 0, 1 }, { 3, 31, 0 } } },
  { { { 12, 0, 2 }, { 0, 37, 0 } } },
  { { { 13, 0, 1 }, { 0, 38, 0 } } },
  { { { 13, 0, 0 }, { 0, 39, 0 } } },
  { { { 13, 0, 1 }, { 5, 30, 0 } } },
  { { { 13, 0, 2 }, { 0, 40, 0 } } },
  { { { 14, 0, 1 }, { 0, 41, 0 } } },
  { { { 14, 0, 0 }, { 0, 42, 0 } } },
  { { { 14, 0, 1 }, { 6, 31, 0 } } },
  { { { 14, 0, 2 }, { 0, 43, 0 } } },
  { { { 15, 0, 1 }, { 0, 44, 0 } } },
  { { { 15, 0, 0 }, { 0, 45, 0 } } },
  { { { 15, 0, 1 }, { 8, 30, 0 } } },
  { { { 15, 0, 2 }, { 0, 46, 0 } } },
  { { { 16, 0, 2 }, { 0, 47, 0 } } },
  { { { 16, 0, 1 }, { 1, 46, 0 } } },
  { { { 16, 0, 0 }, { 0, 48, 0 } } },
  { { { 16, 0, 1 }, { 0, 49, 0 } } },
  { { { 16, 0, 2 }, { 0, 50, 0 } } },
  { { { 17, 0, 1 }, { 2, 47, 0 } } },
  { { { 17, 0, 0 }, { 0, 51, 0 } } },
  { { { 17, 0, 1 }, { 0, 52, 0 } } },
  { { { 17, 0, 2 }, { 0, 53, 0 } } },
  { { { 18, 0, 1 }, { 4, 46, 0 } } },
  { { { 18, 0, 0 }, { 0, 54, 0 } } },
  { { { 18, 0, 1 }, { 0, 55, 0 } } },
  { { { 18, 0, 2 }, { 0, 56, 0 } } },
  { { { 19, 0, 1 }, { 5, 47, 0 } } },
  { { { 19, 0, 0 }, { 0, 57, 0 } } },
  { { { 19, 0, 1 }, { 0, 58, 0 } } },
  { { { 19, 0, 2 }, { 0, 59, 0 } } },
  { { { 20, 0, 1 }, { 7, 46, 0 } } },
  { { { 20, 0, 0 }, { 0, 60, 0 } } },
  { { { 20, 0, 1 }, { 0, 61, 0 } } },
  { { { 20, 0, 2 }, { 0, 62, 0 } } },
  { { { 21, 0, 1 }, { 8, 47, 0 } } },
  { { { 21, 0, 0 }, { 0, 63, 0 } } },
  { { { 21, 0, 1 }, { 1, 62, 0 } } },
  { { { 21, 0, 2 }, { 1, 63, 0 } } },
  { { { 22, 0, 1 }, { 10, 46, 0 } } },
  { { { 22, 0, 0 }, { 2, 62, 0 } } },
  { { { 22, 0, 1 }, { 2, 63, 0 } } },
  { { { 22, 0, 2 }, { 3, 62, 0 } } },
  { { { 23, 0, 1 }, { 11, 47, 0 } } },
  { { { 23, 0, 0 }, { 3, 63, 0 } } },
  { { { 23, 0, 1 }, { 4, 62, 0 } } },
  { { { 23, 0, 2 }, { 4, 63, 0 } } },
  { { { 24, 0, 1 }, { 13, 46, 0 } } },
  { { { 24, 0, 0 }, { 5, 62, 0 } } },
  { { { 24, 0, 1 }, { 5, 63, 0 } } },
  { { { 24, 0, 2 }, { 6, 62, 0 } } },
  { { { 25, 0, 1 }, { 14, 47, 0 } } },
  { { { 25, 0, 0 }, { 6, 63, 0 } } },
  { { { 25, 0, 1 }, { 7, 62, 0 } } },
  { { { 25, 0, 2 }, { 7, 63, 0 } } },
  { { { 26, 0, 1 }, { 16, 45, 0 } } },
  { { { 26, 0, 0 }, { 8, 62, 0 } } },
  { { { 26, 0, 1 }, { 8, 63, 0 } } },
  { { { 26, 0, 2 }, { 9, 62, 0 } } },
  { { { 27, 0, 1 }, { 16, 48, 0 } } },
  { { { 27, 0, 0 }, { 9, 63, 0 } } },
  { { { 27, 0, 1 }, { 10, 62, 0 } } },
  { { { 27, 0, 2 }, { 10, 63, 0 } } },
  { { { 28, 0, 1 }, { 16, 51, 0 } } },
  { { { 28, 0, 0 }, { 11, 62, 0 } } },
  { { { 28, 0, 1 }, { 11, 63, 0 } } },
  { { { 28, 0, 2 }, { 12, 62, 0 } } },
  { { { 29, 0, 1 }, { 16, 54, 0 } } },
  { { { 29, 0, 0 }, { 12, 63, 0 } } },
  { { { 29, 0, 1 }, { 13, 62, 0 } } },
  { { { 29, 0, 2 }, { 13, 63, 0 } } },
  { { { 30, 0, 1 }, { 16, 57, 0 } } },
  { { { 30, 0, 0 }, { 14, 62, 0 } } },
  { { { 30, 0, 1 }, { 14, 63, 0 } } },
  { { { 30, 0, 2 }, { 15, 62, 0 } } },
  { { { 31, 0, 1 }, { 16, 60, 0 } } },
  { { { 31, 0, 0 }, { 15, 63, 0 } } },
  { { { 31, 0, 1 }, { 24, 46, 0 } } },
  { { { 31, 0, 2 }, { 16, 62, 0 } } },
  { { { 32, 0, 2 }, { 16, 63, 0 } } },
  { { { 32, 0, 1 }, { 17, 62, 0 } } },
  { { { 32, 0, 0 }, { 25, 47, 0 } } },
  { { { 32, 0, 1 }, { 17, 63, 0 } } },
  { { { 32, 0, 2 }, { 18, 62, 0 } } },
  { { { 33, 0, 1 }, { 18, 63, 0 } } },
  { { { 33, 0, 0 }, { 27, 46, 0 } } },
  { { { 33, 0, 1 }, { 19, 62, 0 } } },
  { { { 33, 0, 2 }, { 19, 63, 0 } } },
  { { { 34, 0, 1 }, { 20, 62, 0 } } },
  { { { 34, 0, 0 }, { 28, 47, 0 } } },
  { { { 34, 0, 1 }, { 20, 63, 0 } } },
  { { { 34, 0, 2 }, { 21, 62, 0 } } },
  { { { 35, 0, 1 }, { 21, 63, 0 } } },
  { { { 35, 0, 0 }, { 30, 46, 0 } } },
  { { { 35, 0, 1 }, { 22, 62, 0 } } },
  { { { 35, 0, 2 }, { 22, 63, 0 } } },
  { { { 36, 0, 1 }, { 23, 62, 0 } } },
  { { { 36, 0, 0 }, { 31, 47, 0 } } },
  { { { 36, 0, 1 }, { 23, 63, 0 } } },
  { { { 36, 0, 2 }, { 24, 62, 0 } } },
  { { { 37, 0, 1 }, { 24, 63, 0 } } },
  { { { 37, 0, 0 }, { 32, 47, 0 } } },
  { { { 37, 0, 1 }, { 25, 62, 0 } } },
  { { { 37, 0, 2 }, { 25, 63, 0 } } },
  { { { 38, 0, 1 }, { 26, 62, 0 } } },
  { { { 38, 0, 0 }, { 32, 50, 0 } } },
  { { { 38, 0, 1 }, { 26, 63, 0 } } },
  { { { 38, 0, 2 }, { 27, 62, 0 } } },
  { { { 39, 0, 1 }, { 27, 63, 0 } } },
  { { { 39, 0, 0 }, { 32, 53, 0 } } },
  { { { 39, 0, 1 }, { 28, 62, 0 } } },
  { { { 39, 0, 2 }, { 28, 63, 0 } } },
  { { { 40, 0, 1 }, { 29, 62, 0 } } },
  { { { 40, 0, 0 }, { 32, 56, 0 } } },
  { { { 40, 0, 1 }, { 29, 63, 0 } } },
  { { { 40, 0, 2 }, { 30, 62, 0 } } },
  { { { 41, 0, 1 }, { 30, 63, 0 } } },
  { { { 41, 0, 0 }, { 32, 59, 0 } } },
  { { { 41, 0, 1 }, { 31, 62, 0 } } },
  { { { 41, 0, 2 }, { 31, 63, 0 } } },
  { { { 42, 0, 1 }, { 32, 61, 0 } } },
  { { { 42, 0, 0 }, { 32, 62, 0 } } },
  { { { 42, 0, 1 }, { 32, 63, 0 } } },
  { { { 42, 0, 2 }, { 41, 46, 0 } } },
  { { { 43, 0, 1 }, { 33, 62, 0 } } },
  { { { 43, 0, 0 }, { 33, 63, 0 } } },
  { { { 43, 0, 1 }, { 34, 62, 0 } } },
  { { { 43, 0, 2 }, { 42, 47, 0 } } },
  { { { 44, 0, 1 }, { 34, 63, 0 } } },
  { { { 44, 0, 0 }, { 35, 62, 0 } } },
  { { { 44, 0, 1 }, { 35, 63, 0 } } },
  { { { 44, 0, 2 }, { 44, 46, 0 } } },
  { { { 45, 0, 1 }, { 36, 62, 0 } } },
  { { { 45, 0, 0 }, { 36, 63, 0 } } },
  { { { 45, 0, 1 }, { 37, 62, 0 } } },
  { { { 45, 0, 2 }, { 45, 47, 0 } } },
  { { { 46, 0, 1 }, { 37, 63, 0 } } },
  { { { 46, 0, 0 }, { 38, 62, 0 } } },
  { { { 46, 0, 1 }, { 38, 63, 0 } } },
  { { { 46, 0, 2 }, { 47, 46, 0 } } },
  { { { 47, 0, 1 }, { 39, 62, 0 } } },
  { { { 47, 0, 0 }, { 39, 63, 0 } } },
  { { { 47, 0, 1 }, { 40, 62, 0 } } },
  { { { 47, 0, 2 }, { 48, 46, 0 } } },
  { { { 48, 0, 2 }, { 40, 63, 0 } } },
  { { { 48, 0, 1 }, { 41, 62, 0 } } },
  { { { 48, 0, 0 }, { 41, 63, 0 } } },
  { { { 48, 0, 1 }, { 48, 49, 0 } } },
  { { { 48, 0, 2 }, { 42, 62, 0 } } },
  { { { 49, 0, 1 }, { 42, 63, 0 } } },
  { { { 49, 0, 0 }, { 43, 62, 0 } } },
  { { { 49, 0, 1 }, { 48, 52, 0 } } },
  { { { 49, 0, 2 }, { 43, 63, 0 } } },
  { { { 50, 0, 1 }, { 44, 62, 0 } } },
  { { { 50, 0, 0 }, { 44, 63, 0 } } },
  { { { 50, 0, 1 }, { 48, 55, 0 } } },
  { { { 50, 0, 2 }, { 45, 62, 0 } } },
  { { { 51, 0, 1 }, { 45, 63, 0 } } },
  { { { 51, 0, 0 }, { 46, 62, 0 } } },
  { { { 51, 0, 1 }, { 48, 58, 0 } } },
  { { { 51, 0, 2 }, { 46, 63, 0 } } },
  { { { 52, 0, 1 }, { 47, 62, 0 } } },
  { { { 52, 0, 0 }, { 47, 63, 0 } } },
  { { { 52, 0, 1 }, { 48, 61, 0 } } },
  { { { 52, 0, 2 }, { 48, 62, 0 } } },
  { { { 53, 0, 1 }, { 56, 47, 0 } } },
  { { { 53, 0, 0 }, { 48, 63, 0 } } },
  { { { 53, 0, 1 }, { 49, 62, 0 } } },
  { { { 53, 0, 2 }, { 49, 63, 0 } } },
  { { { 54, 0, 1 }, { 58, 46, 0 } } },
  { { { 54, 0, 0 }, { 50, 62, 0 } } },
  { { { 54, 0, 1 }, { 50, 63, 0 } } },
  { { { 54, 0, 2 }, { 51, 62, 0 } } },
  { { { 55, 0, 1 }, { 59, 47, 0 } } },
  { { { 55, 0, 0 }, { 51, 63, 0 } } },
  { { { 55, 0, 1 }, { 52, 62, 0 } } },
  { { { 55, 0, 2 }, { 52, 63, 0 } } },
  { { { 56, 0, 1 }, { 61, 46, 0 } } },
  { { { 56, 0, 0 }, { 53, 62, 0 } } },
  { { { 56, 0, 1 }, { 53, 63, 0 } } },
  { { { 56, 0, 2 }, { 54, 62, 0 } } },
  { { { 57, 0, 1 }, { 62, 47, 0 } } },
  { { { 57, 0, 0 }, { 54, 63, 0 } } },
  { { { 57, 0, 1 }, { 55, 62, 0 } } },
  { { { 57, 0, 2 }, { 55, 63, 0 } } },
  { { { 58, 0, 1 }, { 56, 62, 1 } } },
  { { { 58, 0, 0 }, { 56, 62, 0 } } },
  { { { 58, 0, 1 }, { 56, 63, 0 } } },
  { { { 58, 0, 2 }, { 57, 62, 0 } } },
  { { { 59, 0, 1 }, { 57, 63, 1 } } },
  { { { 59, 0, 0 }, { 57, 63, 0 } } },
  { { { 59, 0, 1 }, { 58, 62, 0 } } },
  { { { 59, 0, 2 }, { 58, 63, 0 } } },
  { { { 60, 0, 1 }, { 59, 62, 1 } } },
  { { { 60, 0, 0 }, { 59, 62, 0 } } },
  { { { 60, 0, 1 }, { 59, 63, 0 } } },
  { { { 60, 0, 2 }, { 60, 62, 0 } } },
  { { { 61, 0, 1 }, { 60, 63, 1 } } },
  { { { 61, 0, 0 }, { 60, 63, 0 } } },
  { { { 61, 0, 1 }, { 61, 62, 0 } } },
  { { { 61, 0, 2 }, { 61, 63, 0 } } },
  { { { 62, 0, 1 }, { 62, 62, 1 } } },
  { { { 62, 0, 0 }, { 62, 62, 0 } } },
  { { { 62, 0, 1 }, { 62, 63, 0 } } },
  { { { 62, 0, 2 }, { 63, 62, 0 } } },
  { { { 63, 0, 1 }, { 63, 63, 1 } } },
  { { { 63, 0, 0 }, { 63, 63, 0 } } }
};

static const DDSSingleColorLookup*
  DDS_LOOKUP[] =
{
  DDSLookup_5_4,
  DDSLookup_6_4,
  DDSLookup_5_4
};

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

#define FixRange(min, max, steps) \
if (min > max) \
  min = max; \
if ((ssize_t) max - min < steps) \
  max = MagickMin(min + steps, 255); \
if ((ssize_t) max - min < steps) \
  min = MagickMax(0, (ssize_t) max - steps)

#define Dot(left, right) (left.x*right.x) + (left.y*right.y) + (left.z*right.z)

#define VectorInit(vector, value) vector.x = vector.y = vector.z = vector.w \
  = value
#define VectorInit3(vector, value) vector.x = vector.y = vector.z = value

#define IsBitMask(mask, r, g, b, a) (mask.r_bitmask == r && mask.g_bitmask == \
  g && mask.b_bitmask == b && mask.alpha_bitmask == a)

/*
  Forward declarations
*/
static MagickBooleanType
  WriteDDSImage(const ImageInfo *,Image *,ExceptionInfo *);

static inline void VectorAdd(const DDSVector4 left, const DDSVector4 right,
  DDSVector4 *destination)
{
  destination->x = left.x + right.x;
  destination->y = left.y + right.y;
  destination->z = left.z + right.z;
  destination->w = left.w + right.w;
}

static inline void VectorClamp(DDSVector4 *value)
{
  value->x = MagickMin(1.0f,MagickMax(0.0f,value->x));
  value->y = MagickMin(1.0f,MagickMax(0.0f,value->y));
  value->z = MagickMin(1.0f,MagickMax(0.0f,value->z));
  value->w = MagickMin(1.0f,MagickMax(0.0f,value->w));
}

static inline void VectorClamp3(DDSVector3 *value)
{
  value->x = MagickMin(1.0f,MagickMax(0.0f,value->x));
  value->y = MagickMin(1.0f,MagickMax(0.0f,value->y));
  value->z = MagickMin(1.0f,MagickMax(0.0f,value->z));
}

static inline void VectorCopy43(const DDSVector4 source,
  DDSVector3 *destination)
{
  destination->x = source.x;
  destination->y = source.y;
  destination->z = source.z;
}

static inline void VectorCopy44(const DDSVector4 source,
  DDSVector4 *destination)
{
  destination->x = source.x;
  destination->y = source.y;
  destination->z = source.z;
  destination->w = source.w;
}

static inline void VectorNegativeMultiplySubtract(const DDSVector4 a,
  const DDSVector4 b, const DDSVector4 c, DDSVector4 *destination)
{
  destination->x = c.x - (a.x * b.x);
  destination->y = c.y - (a.y * b.y);
  destination->z = c.z - (a.z * b.z);
  destination->w = c.w - (a.w * b.w);
}

static inline void VectorMultiply(const DDSVector4 left,
  const DDSVector4 right, DDSVector4 *destination)
{
  destination->x = left.x * right.x;
  destination->y = left.y * right.y;
  destination->z = left.z * right.z;
  destination->w = left.w * right.w;
}

static inline void VectorMultiply3(const DDSVector3 left,
  const DDSVector3 right, DDSVector3 *destination)
{
  destination->x = left.x * right.x;
  destination->y = left.y * right.y;
  destination->z = left.z * right.z;
}

static inline void VectorMultiplyAdd(const DDSVector4 a, const DDSVector4 b,
  const DDSVector4 c, DDSVector4 *destination)
{
  destination->x = (a.x * b.x) + c.x;
  destination->y = (a.y * b.y) + c.y;
  destination->z = (a.z * b.z) + c.z;
  destination->w = (a.w * b.w) + c.w;
}

static inline void VectorMultiplyAdd3(const DDSVector3 a, const DDSVector3 b,
  const DDSVector3 c, DDSVector3 *destination)
{
  destination->x = (a.x * b.x) + c.x;
  destination->y = (a.y * b.y) + c.y;
  destination->z = (a.z * b.z) + c.z;
}

static inline void VectorReciprocal(const DDSVector4 value,
  DDSVector4 *destination)
{
  destination->x = 1.0f / value.x;
  destination->y = 1.0f / value.y;
  destination->z = 1.0f / value.z;
  destination->w = 1.0f / value.w;
}

static inline void VectorSubtract(const DDSVector4 left,
  const DDSVector4 right, DDSVector4 *destination)
{
  destination->x = left.x - right.x;
  destination->y = left.y - right.y;
  destination->z = left.z - right.z;
  destination->w = left.w - right.w;
}

static inline void VectorSubtract3(const DDSVector3 left,
  const DDSVector3 right, DDSVector3 *destination)
{
  destination->x = left.x - right.x;
  destination->y = left.y - right.y;
  destination->z = left.z - right.z;
}

static inline void VectorTruncate(DDSVector4 *value)
{
  value->x = value->x > 0.0f ? floor(value->x) : ceil(value->x);
  value->y = value->y > 0.0f ? floor(value->y) : ceil(value->y);
  value->z = value->z > 0.0f ? floor(value->z) : ceil(value->z);
  value->w = value->w > 0.0f ? floor(value->w) : ceil(value->w);
}

static inline void VectorTruncate3(DDSVector3 *value)
{
  value->x = value->x > 0.0f ? floor(value->x) : ceil(value->x);
  value->y = value->y > 0.0f ? floor(value->y) : ceil(value->y);
  value->z = value->z > 0.0f ? floor(value->z) : ceil(value->z);
}

static inline size_t ClampToLimit(const float value, const size_t limit)
{
  size_t
    result = (int) (value + 0.5f);

  if (result < 0.0f)
    return(0);
  if (result > limit)
    return(limit);
  return result;
}

static inline size_t ColorTo565(const DDSVector3 point)
{
  size_t r = ClampToLimit(31.0f*point.x,31);
  size_t g = ClampToLimit(63.0f*point.y,63);
  size_t b = ClampToLimit(31.0f*point.z,31);

  return (r << 11) | (g << 5) | b;
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
static MagickBooleanType IsDDS(const unsigned char *magick, const size_t length)
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
  
  /* Read optional DX10 header if available */
  if ((dds_info->pixelformat.flags & DDPF_FOURCC) &&
      (dds_info->pixelformat.fourcc == FOURCC_DX10))
    {
      dds_info->extFormat = ReadBlobLSBLong(image);
      dds_info->extDimension = ReadBlobLSBLong(image);
      dds_info->extFlags = ReadBlobLSBLong(image);
      dds_info->extArraySize = ReadBlobLSBLong(image);
      dds_info->extFlags2 = ReadBlobLSBLong(image);
    }
  else
    {
      dds_info->extFormat = 0;
      dds_info->extDimension = 0;
      dds_info->extFlags = 0;
      dds_info->extArraySize = 0;
      dds_info->extFlags2 = 0;
    }
  
  return MagickTrue;
}

static MagickBooleanType SetDXT1Pixels(Image *image,ssize_t x,ssize_t y,
  DDSColors colors,size_t bits,Quantum *q)
{
  ssize_t
    i;

  ssize_t
    j;

  unsigned char
    code;

  for (j = 0; j < 4; j++)
  {
    for (i = 0; i < 4; i++)
    {
      if ((x + i) < (ssize_t) image->columns &&
          (y + j) < (ssize_t) image->rows)
        {
          code=(unsigned char) ((bits >> ((j*4+i)*2)) & 0x3);
          SetPixelRed(image,ScaleCharToQuantum(colors.r[code]),q);
          SetPixelGreen(image,ScaleCharToQuantum(colors.g[code]),q);
          SetPixelBlue(image,ScaleCharToQuantum(colors.b[code]),q);
          SetPixelOpacity(image,ScaleCharToQuantum(colors.a[code]),q);
          if ((colors.a[code] != 0) &&
              (image->alpha_trait == UndefinedPixelTrait))
            return(MagickFalse);
          q+=GetPixelChannels(image);
        }
    }
  }
  return(MagickTrue);
}

static MagickBooleanType ReadMipmaps(const ImageInfo *image_info,Image *image,
  DDSInfo *dds_info,DDSPixelDecoder decoder,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  /*
    Only skip mipmaps for textures and cube maps
  */
  if (EOFBlob(image) != MagickFalse)
    {
      ThrowFileException(exception,CorruptImageWarning,"UnexpectedEndOfFile",
        image->filename);
      return(MagickFalse);
    }
  status=MagickTrue;
  if (dds_info->ddscaps1 & DDSCAPS_MIPMAP
      && (dds_info->ddscaps1 & DDSCAPS_TEXTURE
          || dds_info->ddscaps2 & DDSCAPS2_CUBEMAP))
    {
      ssize_t
        i;

      size_t
        h,
        w;

      w=DIV2(dds_info->width);
      h=DIV2(dds_info->height);

      /*
        Mipmapcount includes the main image, so start from one
      */
      for (i = 1; (i < (ssize_t) dds_info->mipmapcount) && w && h; i++)
      {
        AcquireNextImage(image_info,image,exception);
        if (image->next == (Image *) NULL)
          return(MagickFalse);
        image->next->alpha_trait=image->alpha_trait;
        image=SyncNextImageInList(image);
        status=SetImageExtent(image,w,h,exception);
        if (status == MagickFalse)
          break;
        status=decoder(image,dds_info,exception);
        if (status == MagickFalse)
          break;
        if ((w == 1) && (h == 1))
          break;

        w=DIV2(w);
        h=DIV2(h);
      }
    }
  return(status);
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

  if (ignoreAlpha != MagickFalse || c0 > c1)
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

static MagickBooleanType ReadDXT1Pixels(Image *image,
  DDSInfo *magick_unused(dds_info),ExceptionInfo *exception)
{
  DDSColors
    colors;

  Quantum
    *q;

  ssize_t
    x;

  size_t
    bits;

  ssize_t
    y;

  unsigned short
    c0,
    c1;

  magick_unreferenced(dds_info);
  for (y = 0; y < (ssize_t) image->rows; y += 4)
  {
    for (x = 0; x < (ssize_t) image->columns; x += 4)
    {
      /* Get 4x4 patch of pixels to write on */
      q=QueueAuthenticPixels(image,x,y,MagickMin(4,image->columns-x),
        MagickMin(4,image->rows-y),exception);

      if (q == (Quantum *) NULL)
        return(MagickFalse);

      /* Read 8 bytes of data from the image */
      c0=ReadBlobLSBShort(image);
      c1=ReadBlobLSBShort(image);
      bits=ReadBlobLSBLong(image);

      CalculateColors(c0,c1,&colors,MagickFalse);
      if (EOFBlob(image) != MagickFalse)
        return(MagickFalse);

      /* Write the pixels */
      if (SetDXT1Pixels(image,x,y,colors,bits,q) == MagickFalse)
        {
          /* Correct alpha */
          SetImageAlpha(image,QuantumRange,exception);
          q=QueueAuthenticPixels(image,x,y,MagickMin(4,image->columns-x),
            MagickMin(4,image->rows-y),exception);
          if (q != (Quantum *) NULL)
            SetDXT1Pixels(image,x,y,colors,bits,q);
        }
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        return(MagickFalse);
    }
    if (EOFBlob(image) != MagickFalse)
      return(MagickFalse);
  }
  return(MagickTrue);
}

/*
  Skip the mipmap images for compressed (DXTn) dds files
*/
static MagickBooleanType SkipDXTMipmaps(Image *image,DDSInfo *dds_info,
  int texel_size,ExceptionInfo *exception)
{
  /*
    Only skip mipmaps for textures and cube maps
  */
  if (EOFBlob(image) != MagickFalse)
    {
      ThrowFileException(exception,CorruptImageWarning,"UnexpectedEndOfFile",
        image->filename);
      return(MagickFalse);
    }
  if (dds_info->ddscaps1 & DDSCAPS_MIPMAP
      && (dds_info->ddscaps1 & DDSCAPS_TEXTURE
          || dds_info->ddscaps2 & DDSCAPS2_CUBEMAP))
    {
      MagickOffsetType
        offset;

      ssize_t
        i;

      size_t
        h,
        w;

      w=DIV2(dds_info->width);
      h=DIV2(dds_info->height);

      /*
        Mipmapcount includes the main image, so start from one
      */
      for (i = 1; (i < (ssize_t) dds_info->mipmapcount) && w && h; i++)
      {
        offset=(MagickOffsetType)((w+3)/4)*((h+3)/4)*texel_size;
        if (SeekBlob(image,offset,SEEK_CUR) < 0)
          break;
        w=DIV2(w);
        h=DIV2(h);
        if ((w == 1) && (h == 1))
          break;
      }
    }
  return(MagickTrue);
}

static MagickBooleanType ReadDXT1(const ImageInfo *image_info,Image *image,
  DDSInfo *dds_info,const MagickBooleanType read_mipmaps,
  ExceptionInfo *exception)
{
  if (ReadDXT1Pixels(image,dds_info,exception) == MagickFalse)
    return(MagickFalse);

  if (read_mipmaps != MagickFalse)
    return(ReadMipmaps(image_info,image,dds_info,ReadDXT1Pixels,exception));
  else
    return(SkipDXTMipmaps(image,dds_info,8,exception));
}

static MagickBooleanType ReadDXT3Pixels(Image *image,
  DDSInfo *magick_unused(dds_info),ExceptionInfo *exception)
{
  DDSColors
    colors;

  Quantum
    *q;

  ssize_t
    i,
    x;

  unsigned char
    alpha;

  size_t
    a0,
    a1,
    bits,
    code;

  ssize_t
    j,
    y;

  unsigned short
    c0,
    c1;

  magick_unreferenced(dds_info);
  for (y = 0; y < (ssize_t) image->rows; y += 4)
  {
    for (x = 0; x < (ssize_t) image->columns; x += 4)
    {
      /* Get 4x4 patch of pixels to write on */
      q = QueueAuthenticPixels(image, x, y, MagickMin(4, image->columns - x),
                         MagickMin(4, image->rows - y),exception);

      if (q == (Quantum *) NULL)
        return(MagickFalse);

      /* Read alpha values (8 bytes) */
      a0 = ReadBlobLSBLong(image);
      a1 = ReadBlobLSBLong(image);

      /* Read 8 bytes of data from the image */
      c0 = ReadBlobLSBShort(image);
      c1 = ReadBlobLSBShort(image);
      bits = ReadBlobLSBLong(image);

      CalculateColors(c0, c1, &colors, MagickTrue);

      if (EOFBlob(image) != MagickFalse)
        return(MagickFalse);

      /* Write the pixels */
      for (j = 0; j < 4; j++)
      {
        for (i = 0; i < 4; i++)
        {
          if ((x + i) < (ssize_t) image->columns && (y + j) < (ssize_t) image->rows)
            {
              code = (bits >> ((4*j+i)*2)) & 0x3;
              SetPixelRed(image,ScaleCharToQuantum(colors.r[code]),q);
              SetPixelGreen(image,ScaleCharToQuantum(colors.g[code]),q);
              SetPixelBlue(image,ScaleCharToQuantum(colors.b[code]),q);
              /*
                Extract alpha value: multiply 0..15 by 17 to get range 0..255
              */
              if (j < 2)
                alpha = 17U * (unsigned char) ((a0 >> (4*(4*j+i))) & 0xf);
              else
                alpha = 17U * (unsigned char) ((a1 >> (4*(4*(j-2)+i))) & 0xf);
              SetPixelAlpha(image,ScaleCharToQuantum((unsigned char) alpha),q);
              q+=GetPixelChannels(image);
            }
        }
      }
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        return(MagickFalse);
    }
    if (EOFBlob(image) != MagickFalse)
      return(MagickFalse);
  }
  return(MagickTrue);
}

static MagickBooleanType ReadDXT3(const ImageInfo *image_info,Image *image,
  DDSInfo *dds_info,const MagickBooleanType read_mipmaps,
  ExceptionInfo *exception)
{
  if (ReadDXT3Pixels(image,dds_info,exception) == MagickFalse)
    return(MagickFalse);

  if (read_mipmaps != MagickFalse)
    return(ReadMipmaps(image_info,image,dds_info,ReadDXT3Pixels,exception));
  else
    return(SkipDXTMipmaps(image,dds_info,16,exception));
}

static MagickBooleanType ReadDXT5Pixels(Image *image,
  DDSInfo *magick_unused(dds_info),ExceptionInfo *exception)
{
  DDSColors
    colors;

  MagickSizeType
    alpha_bits;

  Quantum
    *q;

  ssize_t
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

  ssize_t
    j,
    y;

  unsigned short
    c0,
    c1;

  magick_unreferenced(dds_info);
  for (y = 0; y < (ssize_t) image->rows; y += 4)
  {
    for (x = 0; x < (ssize_t) image->columns; x += 4)
    {
      /* Get 4x4 patch of pixels to write on */
      q = QueueAuthenticPixels(image, x, y, MagickMin(4, image->columns - x),
                         MagickMin(4, image->rows - y),exception);

      if (q == (Quantum *) NULL)
        return(MagickFalse);

      /* Read alpha values (8 bytes) */
      a0 = (unsigned char) ReadBlobByte(image);
      a1 = (unsigned char) ReadBlobByte(image);

      alpha_bits = (MagickSizeType)ReadBlobLSBLong(image);
      alpha_bits = alpha_bits | ((MagickSizeType)ReadBlobLSBShort(image) << 32);

      /* Read 8 bytes of data from the image */
      c0 = ReadBlobLSBShort(image);
      c1 = ReadBlobLSBShort(image);
      bits = ReadBlobLSBLong(image);

      CalculateColors(c0, c1, &colors, MagickTrue);
      if (EOFBlob(image) != MagickFalse)
        return(MagickFalse);

      /* Write the pixels */
      for (j = 0; j < 4; j++)
      {
        for (i = 0; i < 4; i++)
        {
          if ((x + i) < (ssize_t) image->columns &&
              (y + j) < (ssize_t) image->rows)
            {
              code = (bits >> ((4*j+i)*2)) & 0x3;
              SetPixelRed(image,ScaleCharToQuantum(colors.r[code]),q);
              SetPixelGreen(image,ScaleCharToQuantum(colors.g[code]),q);
              SetPixelBlue(image,ScaleCharToQuantum(colors.b[code]),q);
              /* Extract alpha value */
              alpha_code = (size_t) (alpha_bits >> (3*(4*j+i))) & 0x7;
              if (alpha_code == 0)
                alpha = a0;
              else if (alpha_code == 1)
                alpha = a1;
              else if (a0 > a1)
                alpha = ((8-alpha_code) * a0 + (alpha_code-1) * a1) / 7;
              else if (alpha_code == 6)
                alpha = 0;
              else if (alpha_code == 7)
                alpha = 255;
              else
                alpha = (((6-alpha_code) * a0 + (alpha_code-1) * a1) / 5);
              SetPixelAlpha(image,ScaleCharToQuantum((unsigned char) alpha),q);
              q+=GetPixelChannels(image);
            }
        }
      }
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        return(MagickFalse);
    }
    if (EOFBlob(image) != MagickFalse)
      return(MagickFalse);
  }
  return(MagickTrue);
}

static MagickBooleanType ReadDXT5(const ImageInfo *image_info,Image *image,
  DDSInfo *dds_info,const MagickBooleanType read_mipmaps,
  ExceptionInfo *exception)
{
  if (ReadDXT5Pixels(image,dds_info,exception) == MagickFalse)
    return(MagickFalse);

  if (read_mipmaps != MagickFalse)
    return(ReadMipmaps(image_info,image,dds_info,ReadDXT5Pixels,exception));
  else
    return(SkipDXTMipmaps(image,dds_info,16,exception));
}

static MagickBooleanType ReadUncompressedRGBPixels(Image *image,
  DDSInfo *dds_info,ExceptionInfo *exception)
{
  Quantum
    *q;

  ssize_t
    x, y;

  unsigned short
    color;

  for (y = 0; y < (ssize_t) image->rows; y++)
  {
    q = QueueAuthenticPixels(image, 0, y, image->columns, 1,exception);

    if (q == (Quantum *) NULL)
      return(MagickFalse);

    for (x = 0; x < (ssize_t) image->columns; x++)
    {
      if (dds_info->pixelformat.rgb_bitcount == 8 ||
          dds_info->extFormat == DXGI_FORMAT_R8_UNORM)
        SetPixelGray(image,ScaleCharToQuantum(ReadBlobByte(image)),q);
      else if (dds_info->pixelformat.rgb_bitcount == 16 ||
          dds_info->extFormat == DXGI_FORMAT_B5G6R5_UNORM)
        {
           color=ReadBlobShort(image);
           SetPixelRed(image,ScaleCharToQuantum((unsigned char)
             (((color >> 11)/31.0)*255)),q);
           SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
             ((((unsigned short)(color << 5) >> 10)/63.0)*255)),q);
           SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
             ((((unsigned short)(color << 11) >> 11)/31.0)*255)),q);
        }
      else
        {
          SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
            ReadBlobByte(image)),q);
          SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
            ReadBlobByte(image)),q);
          SetPixelRed(image,ScaleCharToQuantum((unsigned char)
            ReadBlobByte(image)),q);
          if (dds_info->pixelformat.rgb_bitcount == 32 || 
              dds_info->extFormat == DXGI_FORMAT_B8G8R8X8_UNORM)
            (void) ReadBlobByte(image);
        }
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      return(MagickFalse);
    if (EOFBlob(image) != MagickFalse)
      return(MagickFalse);
  }
  return(MagickTrue);
}

/*
  Skip the mipmap images for uncompressed (RGB or RGBA) dds files
*/
static MagickBooleanType SkipRGBMipmaps(Image *image,DDSInfo *dds_info,
  int pixel_size,ExceptionInfo *exception)
{
  /*
    Only skip mipmaps for textures and cube maps
  */
  if (EOFBlob(image) != MagickFalse)
    {
      ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
        image->filename);
      return(MagickFalse);
    }
  if (dds_info->ddscaps1 & DDSCAPS_MIPMAP
      && (dds_info->ddscaps1 & DDSCAPS_TEXTURE
          || dds_info->ddscaps2 & DDSCAPS2_CUBEMAP))
    {
      MagickOffsetType
        offset;
  
      ssize_t
        i;

      size_t
        h,
        w;

      w=DIV2(dds_info->width);
      h=DIV2(dds_info->height);

      /*
        Mipmapcount includes the main image, so start from one
      */
      for (i=1; (i < (ssize_t) dds_info->mipmapcount) && w && h; i++)
      {
        offset=(MagickOffsetType)w*h*pixel_size;
        if (SeekBlob(image,offset,SEEK_CUR) < 0)
          break;
        w=DIV2(w);
        h=DIV2(h);
        if ((w == 1) && (h == 1))
          break;
      }
    }
  return(MagickTrue);
}

static MagickBooleanType ReadUncompressedRGB(const ImageInfo *image_info,
  Image *image,DDSInfo *dds_info,const MagickBooleanType read_mipmaps,
  ExceptionInfo *exception)
{
  if (dds_info->pixelformat.rgb_bitcount == 8 || 
      dds_info->extFormat == DXGI_FORMAT_R8_UNORM)
    (void) SetImageType(image,GrayscaleType,exception);
  else if (dds_info->pixelformat.rgb_bitcount == 16 && !IsBitMask(
    dds_info->pixelformat,0xf800,0x07e0,0x001f,0x0000))
    ThrowBinaryException(CorruptImageError,"ImageTypeNotSupported",
      image->filename);

  if (ReadUncompressedRGBPixels(image,dds_info,exception) == MagickFalse)
    return(MagickFalse);

  if (read_mipmaps != MagickFalse)
    return(ReadMipmaps(image_info,image,dds_info,ReadUncompressedRGBPixels,
      exception));
  else
    return(SkipRGBMipmaps(image,dds_info,3,exception));
}

static MagickBooleanType ReadUncompressedRGBAPixels(Image *image,
  DDSInfo *dds_info,ExceptionInfo *exception)
{
  Quantum
    *q;

  ssize_t
    alphaBits,
    x,
    y;

  unsigned short
    color;

  alphaBits=0;
  if (dds_info->pixelformat.rgb_bitcount == 16)
    {
      if (IsBitMask(dds_info->pixelformat,0x7c00,0x03e0,0x001f,0x8000))
        alphaBits=1;
      else if (IsBitMask(dds_info->pixelformat,0x00ff,0x00ff,0x00ff,0xff00))
        {
          alphaBits=2;
          (void) SetImageType(image,GrayscaleAlphaType,exception);
        }
      else if (IsBitMask(dds_info->pixelformat,0x0f00,0x00f0,0x000f,0xf000))
        alphaBits=4;
      else
        ThrowBinaryException(CorruptImageError,"ImageTypeNotSupported",
          image->filename);
    }

  if (dds_info->extFormat == DXGI_FORMAT_B5G5R5A1_UNORM)
    alphaBits=1;

  for (y = 0; y < (ssize_t) image->rows; y++)
  {
    q = QueueAuthenticPixels(image, 0, y, image->columns, 1,exception);

    if (q == (Quantum *) NULL)
      return(MagickFalse);

    for (x = 0; x < (ssize_t) image->columns; x++)
    {
      if (dds_info->pixelformat.rgb_bitcount == 16 ||
          dds_info->extFormat == DXGI_FORMAT_B5G5R5A1_UNORM)
        {
           color=ReadBlobShort(image);
           if (alphaBits == 1)
             {
               SetPixelAlpha(image,(color & (1 << 15)) ? QuantumRange : 0,q);
               SetPixelRed(image,ScaleCharToQuantum((unsigned char)
                 ((((unsigned short)(color << 1) >> 11)/31.0)*255)),q);
               SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
                 ((((unsigned short)(color << 6) >> 11)/31.0)*255)),q);
               SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
                 ((((unsigned short)(color << 11) >> 11)/31.0)*255)),q);
             }
          else if (alphaBits == 2)
            {
               SetPixelAlpha(image,ScaleCharToQuantum((unsigned char)
                 (color >> 8)),q);
               SetPixelGray(image,ScaleCharToQuantum((unsigned char)color),q);
            }
          else
            {
               SetPixelAlpha(image,ScaleCharToQuantum((unsigned char)
                 (((color >> 12)/15.0)*255)),q);
               SetPixelRed(image,ScaleCharToQuantum((unsigned char)
                 ((((unsigned short)(color << 4) >> 12)/15.0)*255)),q);
               SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
                 ((((unsigned short)(color << 8) >> 12)/15.0)*255)),q);
               SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
                 ((((unsigned short)(color << 12) >> 12)/15.0)*255)),q);
            }
        }
      else if (dds_info->extFormat == DXGI_FORMAT_R8G8B8A8_UNORM ||
          IsBitMask(dds_info->pixelformat,0x000000ff,0x0000ff00,0x00ff0000,0xff000000))
        {
          SetPixelRed(image,ScaleCharToQuantum((unsigned char)
            ReadBlobByte(image)),q);
          SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
            ReadBlobByte(image)),q);
          SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
            ReadBlobByte(image)),q);
          SetPixelAlpha(image,ScaleCharToQuantum((unsigned char)
            ReadBlobByte(image)),q);
        }
      else
        {
          SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
            ReadBlobByte(image)),q);
          SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
            ReadBlobByte(image)),q);
          SetPixelRed(image,ScaleCharToQuantum((unsigned char)
            ReadBlobByte(image)),q);
          SetPixelAlpha(image,ScaleCharToQuantum((unsigned char)
            ReadBlobByte(image)),q);
        }
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      return(MagickFalse);
    if (EOFBlob(image) != MagickFalse)
      return(MagickFalse);
  }
  return(MagickTrue);
}

static MagickBooleanType ReadUncompressedRGBA(const ImageInfo *image_info,
  Image *image,DDSInfo *dds_info,const MagickBooleanType read_mipmaps,
  ExceptionInfo *exception)
{
  if (ReadUncompressedRGBAPixels(image,dds_info,exception) == MagickFalse)
    return(MagickFalse);

  if (read_mipmaps != MagickFalse)
    return(ReadMipmaps(image_info,image,dds_info,ReadUncompressedRGBAPixels,
      exception));
  else
    return(SkipRGBMipmaps(image,dds_info,4,exception));
}

static Image *ReadDDSImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  const char
    *option;

  CompressionType
    compression;

  DDSInfo
    dds_info;

  DDSDecoder
    *decoder;

  Image
    *image;

  MagickBooleanType
    status,
    cubemap,
    volume,
    read_mipmaps;

  PixelTrait
    alpha_trait;

  size_t
    n,
    num_images;

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
  cubemap=MagickFalse,
  volume=MagickFalse,
  read_mipmaps=MagickFalse;
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  
  /*
    Initialize image structure.
  */
  if (ReadDDSInfo(image, &dds_info) != MagickTrue)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  if (dds_info.ddscaps2 & DDSCAPS2_CUBEMAP)
    cubemap = MagickTrue;

  if (dds_info.ddscaps2 & DDSCAPS2_VOLUME && dds_info.depth > 0)
    volume = MagickTrue;

  /*
    Determine pixel format
  */
  if (dds_info.pixelformat.flags & DDPF_RGB)
    {
      compression = NoCompression;
      if (dds_info.pixelformat.flags & DDPF_ALPHAPIXELS)
        {
          alpha_trait = BlendPixelTrait;
          decoder = ReadUncompressedRGBA;
        }
      else
        {
          alpha_trait = UndefinedPixelTrait;
          decoder = ReadUncompressedRGB;
        }
    }
  else if (dds_info.pixelformat.flags & DDPF_LUMINANCE)
   {
      compression = NoCompression;
      if (dds_info.pixelformat.flags & DDPF_ALPHAPIXELS)
        {
          /* Not sure how to handle this */
          ThrowReaderException(CorruptImageError, "ImageTypeNotSupported");
        }
      else
        {
          alpha_trait = UndefinedPixelTrait;
          decoder = ReadUncompressedRGB;
        }
    }
  else if (dds_info.pixelformat.flags & DDPF_FOURCC)
    {
      switch (dds_info.pixelformat.fourcc)
      {
        case FOURCC_DXT1:
        {
          alpha_trait = UndefinedPixelTrait;
          compression = DXT1Compression;
          decoder = ReadDXT1;
          break;
        }
        case FOURCC_DXT3:
        {
          alpha_trait = BlendPixelTrait;
          compression = DXT3Compression;
          decoder = ReadDXT3;
          break;
        }
        case FOURCC_DXT5:
        {
          alpha_trait = BlendPixelTrait;
          compression = DXT5Compression;
          decoder = ReadDXT5;
          break;
        }
        case FOURCC_DX10:
        {
          if (dds_info.extDimension != DDSEXT_DIMENSION_TEX2D)
            {
              ThrowReaderException(CorruptImageError, "ImageTypeNotSupported");
            }

          switch (dds_info.extFormat)
          {
            case DXGI_FORMAT_R8_UNORM:
            {
              compression = NoCompression;
              alpha_trait = UndefinedPixelTrait;
              decoder = ReadUncompressedRGB;
              break;
            }
            case DXGI_FORMAT_B5G6R5_UNORM:
            {
              compression = NoCompression;
              alpha_trait = UndefinedPixelTrait;
              decoder = ReadUncompressedRGB;
              break;
            }
            case DXGI_FORMAT_B5G5R5A1_UNORM:
            {
              compression = NoCompression;
              alpha_trait = BlendPixelTrait;
              decoder = ReadUncompressedRGBA;
              break;
            }
            case DXGI_FORMAT_B8G8R8A8_UNORM:
            {
              compression = NoCompression;
              alpha_trait = BlendPixelTrait;
              decoder = ReadUncompressedRGBA;
              break;
            }
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            {
              compression = NoCompression;
              alpha_trait = BlendPixelTrait;
              decoder = ReadUncompressedRGBA;
              break;
            }
            case DXGI_FORMAT_B8G8R8X8_UNORM:
            {
              compression = NoCompression;
              alpha_trait = UndefinedPixelTrait;
              decoder = ReadUncompressedRGB;
              break;
            }
            case DXGI_FORMAT_BC1_UNORM:
            {
              alpha_trait = UndefinedPixelTrait;
              compression = DXT1Compression;
              decoder = ReadDXT1;
              break;
            }
            case DXGI_FORMAT_BC2_UNORM:
            {
              alpha_trait = BlendPixelTrait;
              compression = DXT3Compression;
              decoder = ReadDXT3;
              break;
            }
            case DXGI_FORMAT_BC3_UNORM:
            {
              alpha_trait = BlendPixelTrait;
              compression = DXT5Compression;
              decoder = ReadDXT5;
              break;
            }
            default:
            {
              /* Unknown format */
              ThrowReaderException(CorruptImageError, "ImageTypeNotSupported");
            }
          }

          if (dds_info.extFlags & DDSEXTFLAGS_CUBEMAP)
            cubemap = MagickTrue;

          num_images = dds_info.extArraySize;
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

  if ((num_images == 0) || (num_images > GetBlobSize(image)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  if (AcquireMagickResource(ListLengthResource,num_images) == MagickFalse)
    ThrowReaderException(ResourceLimitError,"ListLengthExceedsLimit");

  option=GetImageOption(image_info,"dds:skip-mipmaps");
  if (IsStringFalse(option) != MagickFalse)
    read_mipmaps=MagickTrue;

  for (n = 0; n < num_images; n++)
  {
    if (n != 0)
      {
        /* Start a new image */
        if (EOFBlob(image) != MagickFalse)
          ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          return(DestroyImageList(image));
        image=SyncNextImageInList(image);
      }
    
    image->alpha_trait=alpha_trait;
    image->compression=compression;
    image->columns=dds_info.width;
    image->rows=dds_info.height;
    image->storage_class=DirectClass;
    image->endian=LSBEndian;
    image->depth=8;
    if (image_info->ping != MagickFalse)
      {
        (void) CloseBlob(image);
        return(GetFirstImageInList(image));
      }
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    (void) SetImageBackgroundColor(image,exception);
    status=(decoder)(image_info,image,&dds_info,read_mipmaps,exception);
    if (status == MagickFalse)
      {
        (void) CloseBlob(image);
        if (n == 0)
          return(DestroyImageList(image));
        return(GetFirstImageInList(image));
      }
  }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
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

  entry = AcquireMagickInfo("DDS","DDS","Microsoft DirectDraw Surface");
  entry->decoder = (DecodeImageHandler *) ReadDDSImage;
  entry->encoder = (EncodeImageHandler *) WriteDDSImage;
  entry->magick = (IsImageFormatHandler *) IsDDS;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry = AcquireMagickInfo("DDS","DXT1","Microsoft DirectDraw Surface");
  entry->decoder = (DecodeImageHandler *) ReadDDSImage;
  entry->encoder = (EncodeImageHandler *) WriteDDSImage;
  entry->magick = (IsImageFormatHandler *) IsDDS;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry = AcquireMagickInfo("DDS","DXT5","Microsoft DirectDraw Surface");
  entry->decoder = (DecodeImageHandler *) ReadDDSImage;
  entry->encoder = (EncodeImageHandler *) WriteDDSImage;
  entry->magick = (IsImageFormatHandler *) IsDDS;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

static void RemapIndices(const ssize_t *map, const unsigned char *source,
  unsigned char *target)
{
  ssize_t
    i;

  for (i = 0; i < 16; i++)
  {
    if (map[i] == -1)
      target[i] = 3;
    else
      target[i] = source[map[i]];
  }
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
  (void) UnregisterMagickInfo("DXT1");
  (void) UnregisterMagickInfo("DXT5");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e D D S I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteDDSImage() writes a DirectDraw Surface image file in the DXT5 format.
%
%  The format of the WriteBMPImage method is:
%
%     MagickBooleanType WriteDDSImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static size_t CompressAlpha(const size_t min, const size_t max,
  const size_t steps, const ssize_t *alphas, unsigned char* indices)
{
  unsigned char
    codes[8];

  ssize_t
    i;

  size_t
    error,
    index,
    j,
    least,
    value;

  codes[0] = (unsigned char) min;
  codes[1] = (unsigned char) max;
  codes[6] = 0;
  codes[7] = 255;

  for (i=1; i <  (ssize_t) steps; i++)
    codes[i+1] = (unsigned char) (((steps-i)*min + i*max) / steps);

  error = 0;
  for (i=0; i<16; i++)
  {
    if (alphas[i] == -1)
      {
        indices[i] = 0;
        continue;
      }

    value = alphas[i];
    least = SIZE_MAX;
    index = 0;
    for (j=0; j<8; j++)
    {
      size_t
        dist;

      dist = value - (size_t)codes[j];
      dist *= dist;

      if (dist < least)
        {
          least = dist;
          index = j;
        }
    }

    indices[i] = (unsigned char)index;
    error += least;
  }

  return error;
}

static MagickBooleanType ConstructOrdering(const size_t count,
  const DDSVector4 *points, const DDSVector3 axis, DDSVector4 *pointsWeights,
  DDSVector4 *xSumwSum, unsigned char *order, size_t iteration)
{
  float
     dps[16],
     f;

  ssize_t
    i;

  size_t
    j;

  unsigned char
    c,
    *o,
    *p;

  o = order + (16*iteration);

  for (i=0; i < (ssize_t) count; i++)
  {
    dps[i] = Dot(points[i],axis);
    o[i] = (unsigned char)i;
  }

  for (i=0; i < (ssize_t) count; i++)
  {
    for (j=i; j > 0 && dps[j] < dps[j - 1]; j--)
    {
      f = dps[j];
      dps[j] = dps[j - 1];
      dps[j - 1] = f;

      c = o[j];
      o[j] = o[j - 1];
      o[j - 1] = c;
    }
  }

  for (i=0; i < (ssize_t) iteration; i++)
  {
    MagickBooleanType
      same;

    p = order + (16*i);
    same = MagickTrue;

    for (j=0; j < count; j++)
    {
      if (o[j] != p[j])
        {
          same = MagickFalse;
          break;
        }
    }

    if (same != MagickFalse)
      return MagickFalse;
  }

  xSumwSum->x = 0;
  xSumwSum->y = 0;
  xSumwSum->z = 0;
  xSumwSum->w = 0;

  for (i=0; i < (ssize_t) count; i++)
  {
    DDSVector4
      v;

    j = (size_t) o[i];

    v.x = points[j].w * points[j].x;
    v.y = points[j].w * points[j].y;
    v.z = points[j].w * points[j].z;
    v.w = points[j].w * 1.0f;

    VectorCopy44(v,&pointsWeights[i]);
    VectorAdd(*xSumwSum,v,xSumwSum);
  }

  return MagickTrue;
}

static void CompressClusterFit(const size_t count,
  const DDSVector4 *points, const ssize_t *map, const DDSVector3 principle,
  const DDSVector4 metric, DDSVector3 *start, DDSVector3* end,
  unsigned char *indices)
{
  DDSVector3
    axis;

  DDSVector4
    grid,
    gridrcp,
    half,
    onethird_onethird2,
    pointsWeights[16],
    two,
    twonineths,
    twothirds_twothirds2,
    xSumwSum;

  float
    bestError = 1e+37f;

  size_t
    bestIteration = 0,
    besti = 0,
    bestj = 0,
    bestk = 0,
    iterationIndex;

  ssize_t
    i;

  unsigned char
    *o,
    order[128],
    unordered[16];

  VectorInit(half,0.5f);
  VectorInit(two,2.0f);

  VectorInit(onethird_onethird2,1.0f/3.0f);
  onethird_onethird2.w = 1.0f/9.0f;
  VectorInit(twothirds_twothirds2,2.0f/3.0f);
  twothirds_twothirds2.w = 4.0f/9.0f;
  VectorInit(twonineths,2.0f/9.0f);

  grid.x = 31.0f;
  grid.y = 63.0f;
  grid.z = 31.0f;
  grid.w = 0.0f;

  gridrcp.x = 1.0f/31.0f;
  gridrcp.y = 1.0f/63.0f;
  gridrcp.z = 1.0f/31.0f;
  gridrcp.w = 0.0f;

  xSumwSum.x = 0.0f;
  xSumwSum.y = 0.0f;
  xSumwSum.z = 0.0f;
  xSumwSum.w = 0.0f;

  ConstructOrdering(count,points,principle,pointsWeights,&xSumwSum,order,0);

  for (iterationIndex = 0;;)
  {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,1) \
    num_threads(GetMagickResourceLimit(ThreadResource))
#endif
    for (i=0; i < (ssize_t) count; i++)
    {
      DDSVector4
        part0,
        part1,
        part2;

      size_t
        ii,
        j,
        k,
        kmin;

      VectorInit(part0,0.0f);
      for(ii=0; ii < (size_t) i; ii++)
        VectorAdd(pointsWeights[ii],part0,&part0);

      VectorInit(part1,0.0f);
      for (j=(size_t) i;;)
      {
        if (j == 0)
          {
            VectorCopy44(pointsWeights[0],&part2);
            kmin = 1;
          }
          else
          {
            VectorInit(part2,0.0f);
            kmin = j;
          }

        for (k=kmin;;)
        {
          DDSVector4
            a,
            alpha2_sum,
            alphax_sum,
            alphabeta_sum,
            b,
            beta2_sum,
            betax_sum,
            e1,
            e2,
            factor,
            part3;

          float
            error;

          VectorSubtract(xSumwSum,part2,&part3);
          VectorSubtract(part3,part1,&part3);
          VectorSubtract(part3,part0,&part3);

          VectorMultiplyAdd(part1,twothirds_twothirds2,part0,&alphax_sum);
          VectorMultiplyAdd(part2,onethird_onethird2,alphax_sum,&alphax_sum);
          VectorInit(alpha2_sum,alphax_sum.w);

          VectorMultiplyAdd(part2,twothirds_twothirds2,part3,&betax_sum);
          VectorMultiplyAdd(part1,onethird_onethird2,betax_sum,&betax_sum);
          VectorInit(beta2_sum,betax_sum.w);

          VectorAdd(part1,part2,&alphabeta_sum);
          VectorInit(alphabeta_sum,alphabeta_sum.w);
          VectorMultiply(twonineths,alphabeta_sum,&alphabeta_sum);

          VectorMultiply(alpha2_sum,beta2_sum,&factor);
          VectorNegativeMultiplySubtract(alphabeta_sum,alphabeta_sum,factor,
            &factor);
          VectorReciprocal(factor,&factor);

          VectorMultiply(alphax_sum,beta2_sum,&a);
          VectorNegativeMultiplySubtract(betax_sum,alphabeta_sum,a,&a);
          VectorMultiply(a,factor,&a);

          VectorMultiply(betax_sum,alpha2_sum,&b);
          VectorNegativeMultiplySubtract(alphax_sum,alphabeta_sum,b,&b);
          VectorMultiply(b,factor,&b);

          VectorClamp(&a);
          VectorMultiplyAdd(grid,a,half,&a);
          VectorTruncate(&a);
          VectorMultiply(a,gridrcp,&a);

          VectorClamp(&b);
          VectorMultiplyAdd(grid,b,half,&b);
          VectorTruncate(&b);
          VectorMultiply(b,gridrcp,&b);

          VectorMultiply(b,b,&e1);
          VectorMultiply(e1,beta2_sum,&e1);
          VectorMultiply(a,a,&e2);
          VectorMultiplyAdd(e2,alpha2_sum,e1,&e1);

          VectorMultiply(a,b,&e2);
          VectorMultiply(e2,alphabeta_sum,&e2);
          VectorNegativeMultiplySubtract(a,alphax_sum,e2,&e2);
          VectorNegativeMultiplySubtract(b,betax_sum,e2,&e2);
          VectorMultiplyAdd(two,e2,e1,&e2);
          VectorMultiply(e2,metric,&e2);

          error = e2.x + e2.y + e2.z;

          if (error < bestError)
            {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
              #pragma omp critical (DDS_CompressClusterFit)
#endif
              {
                if (error < bestError)
                  {
                    VectorCopy43(a,start);
                    VectorCopy43(b,end);
                    bestError = error;
                    besti = i;
                    bestj = j;
                    bestk = k;
                    bestIteration = iterationIndex;
                  }
              }
            }

          if (k == count)
            break;

          VectorAdd(pointsWeights[k],part2,&part2);
          k++;
        }

        if (j == count)
          break;

        VectorAdd(pointsWeights[j],part1,&part1);
        j++;
      }
    }

    if (bestIteration != iterationIndex)
      break;

    iterationIndex++;
    if (iterationIndex == 8)
      break;

    VectorSubtract3(*end,*start,&axis);
    if (ConstructOrdering(count,points,axis,pointsWeights,&xSumwSum,order,
      iterationIndex) == MagickFalse)
      break;
  }

  o = order + (16*bestIteration);

  for (i=0; i < (ssize_t) besti; i++)
    unordered[o[i]] = 0;
  for (i=besti; i < (ssize_t) bestj; i++)
    unordered[o[i]] = 2;
  for (i=bestj; i < (ssize_t) bestk; i++)
    unordered[o[i]] = 3;
  for (i=bestk; i < (ssize_t) count; i++)
    unordered[o[i]] = 1;

  RemapIndices(map,unordered,indices);
}

static void CompressRangeFit(const size_t count,
  const DDSVector4* points, const ssize_t *map, const DDSVector3 principle,
  const DDSVector4 metric, DDSVector3 *start, DDSVector3 *end,
  unsigned char *indices)
{
  float
    d,
    bestDist,
    max,
    min,
    val;

  DDSVector3
    codes[4],
    grid,
    gridrcp,
    half,
    dist;

  ssize_t
    i;

  size_t
    bestj,
    j;

  unsigned char
    closest[16];

  VectorInit3(half,0.5f);

  grid.x = 31.0f;
  grid.y = 63.0f;
  grid.z = 31.0f;

  gridrcp.x = 1.0f/31.0f;
  gridrcp.y = 1.0f/63.0f;
  gridrcp.z = 1.0f/31.0f;

  if (count > 0)
    {
      VectorCopy43(points[0],start);
      VectorCopy43(points[0],end);

      min = max = Dot(points[0],principle);
      for (i=1; i < (ssize_t) count; i++)
      {
        val = Dot(points[i],principle);
        if (val < min)
        {
          VectorCopy43(points[i],start);
          min = val;
        }
        else if (val > max)
        {
          VectorCopy43(points[i],end);
          max = val;
        }
      }
    }

  VectorClamp3(start);
  VectorMultiplyAdd3(grid,*start,half,start);
  VectorTruncate3(start);
  VectorMultiply3(*start,gridrcp,start);

  VectorClamp3(end);
  VectorMultiplyAdd3(grid,*end,half,end);
  VectorTruncate3(end);
  VectorMultiply3(*end,gridrcp,end);

  codes[0] = *start;
  codes[1] = *end;
  codes[2].x = (start->x * (2.0f/3.0f)) + (end->x * (1.0f/3.0f));
  codes[2].y = (start->y * (2.0f/3.0f)) + (end->y * (1.0f/3.0f));
  codes[2].z = (start->z * (2.0f/3.0f)) + (end->z * (1.0f/3.0f));
  codes[3].x = (start->x * (1.0f/3.0f)) + (end->x * (2.0f/3.0f));
  codes[3].y = (start->y * (1.0f/3.0f)) + (end->y * (2.0f/3.0f));
  codes[3].z = (start->z * (1.0f/3.0f)) + (end->z * (2.0f/3.0f));

  for (i=0; i < (ssize_t) count; i++)
  {
    bestDist = 1e+37f;
    bestj = 0;
    for (j=0; j < 4; j++)
    {
      dist.x = (points[i].x - codes[j].x) * metric.x;
      dist.y = (points[i].y - codes[j].y) * metric.y;
      dist.z = (points[i].z - codes[j].z) * metric.z;

      d = Dot(dist,dist);
      if (d < bestDist)
        {
          bestDist = d;
          bestj = j;
        }
    }

    closest[i] = (unsigned char) bestj;
  }

  RemapIndices(map, closest, indices);
}

static void ComputeEndPoints(const DDSSingleColorLookup *lookup[],
  const unsigned char *color, DDSVector3 *start, DDSVector3 *end,
  unsigned char *index)
{
  ssize_t
    i;

  size_t
    c,
    maxError = SIZE_MAX;

  for (i=0; i < 2; i++)
  {
    const DDSSourceBlock*
      sources[3];

      size_t
        error = 0;

    for (c=0; c < 3; c++)
    {
      sources[c] = &lookup[c][color[c]].sources[i];
      error += ((size_t) sources[c]->error) * ((size_t) sources[c]->error);
    }

    if (error > maxError)
      continue;

    start->x = (float) sources[0]->start / 31.0f;
    start->y = (float) sources[1]->start / 63.0f;
    start->z = (float) sources[2]->start / 31.0f;

    end->x = (float) sources[0]->end / 31.0f;
    end->y = (float) sources[1]->end / 63.0f;
    end->z = (float) sources[2]->end / 31.0f;

    *index = (unsigned char) (2*i);
    maxError = error;
  }
}

static void ComputePrincipleComponent(const float *covariance,
  DDSVector3 *principle)
{
  DDSVector4
    row0,
    row1,
    row2,
    v;

  ssize_t
    i;

  row0.x = covariance[0];
  row0.y = covariance[1];
  row0.z = covariance[2];
  row0.w = 0.0f;

  row1.x = covariance[1];
  row1.y = covariance[3];
  row1.z = covariance[4];
  row1.w = 0.0f;

  row2.x = covariance[2];
  row2.y = covariance[4];
  row2.z = covariance[5];
  row2.w = 0.0f;

  VectorInit(v,1.0f);

  for (i=0; i < 8; i++)
  {
    DDSVector4
      w;

    float
      a;

    w.x = row0.x * v.x;
    w.y = row0.y * v.x;
    w.z = row0.z * v.x;
    w.w = row0.w * v.x;

    w.x = (row1.x * v.y) + w.x;
    w.y = (row1.y * v.y) + w.y;
    w.z = (row1.z * v.y) + w.z;
    w.w = (row1.w * v.y) + w.w;

    w.x = (row2.x * v.z) + w.x;
    w.y = (row2.y * v.z) + w.y;
    w.z = (row2.z * v.z) + w.z;
    w.w = (row2.w * v.z) + w.w;

    a = (float) PerceptibleReciprocal(MagickMax(w.x,MagickMax(w.y,w.z)));

    v.x = w.x * a;
    v.y = w.y * a;
    v.z = w.z * a;
    v.w = w.w * a;
  }

  VectorCopy43(v,principle);
}

static void ComputeWeightedCovariance(const size_t count,
  const DDSVector4 *points, float *covariance)
{
  DDSVector3
    centroid;

  float
    total;

  size_t
    i;

  total = 0.0f;
  VectorInit3(centroid,0.0f);

  for (i=0; i < count; i++)
  {
    total += points[i].w;
    centroid.x += (points[i].x * points[i].w);
    centroid.y += (points[i].y * points[i].w);
    centroid.z += (points[i].z * points[i].w);
  }

  if( total > 1.192092896e-07F)
    {
      centroid.x /= total;
      centroid.y /= total;
      centroid.z /= total;
    }

  for (i=0; i < 6; i++)
    covariance[i] = 0.0f;

  for (i = 0; i < count; i++)
  {
    DDSVector3
      a,
      b;

    a.x = points[i].x - centroid.x;
    a.y = points[i].y - centroid.y;
    a.z = points[i].z - centroid.z;

    b.x = points[i].w * a.x;
    b.y = points[i].w * a.y;
    b.z = points[i].w * a.z;

    covariance[0] += a.x*b.x;
    covariance[1] += a.x*b.y;
    covariance[2] += a.x*b.z;
    covariance[3] += a.y*b.y;
    covariance[4] += a.y*b.z;
    covariance[5] += a.z*b.z;
  }
}

static void WriteAlphas(Image *image, const ssize_t *alphas, size_t min5,
  size_t max5, size_t min7, size_t max7)
{
  ssize_t
    i;

  size_t
    err5,
    err7,
    j;

  unsigned char
    indices5[16],
    indices7[16];

  FixRange(min5,max5,5);
  err5 = CompressAlpha(min5,max5,5,alphas,indices5);

  FixRange(min7,max7,7);
  err7 = CompressAlpha(min7,max7,7,alphas,indices7);

  if (err7 < err5)
  {
    for (i=0; i < 16; i++)
    {
      unsigned char
        index;

      index = indices7[i];
      if( index == 0 )
        indices5[i] = 1;
      else if (index == 1)
        indices5[i] = 0;
      else
        indices5[i] = 9 - index;
    }

    min5 = max7;
    max5 = min7;
  }
  
  (void) WriteBlobByte(image,(unsigned char) min5);
  (void) WriteBlobByte(image,(unsigned char) max5);
  
  for(i=0; i < 2; i++)
  {
    size_t
      value = 0;

    for (j=0; j < 8; j++)
    {
      size_t index = (size_t) indices5[j + i*8];
      value |= ( index << 3*j );
    }

    for (j=0; j < 3; j++)
    {
      size_t byte = (value >> 8*j) & 0xff;
      (void) WriteBlobByte(image,(unsigned char) byte);
    }
  }
}

static void WriteIndices(Image *image, const DDSVector3 start,
  const DDSVector3 end, unsigned char *indices)
{
  ssize_t
    i;

  size_t
    a,
    b;

  unsigned char
    remapped[16];

  const unsigned char
    *ind;

  a = ColorTo565(start);
  b = ColorTo565(end);

  for (i=0; i<16; i++)
  {
    if( a < b )
      remapped[i] = (indices[i] ^ 0x1) & 0x3;
    else if( a == b )
      remapped[i] = 0;
    else
      remapped[i] = indices[i];
  }

  if( a < b )
    Swap(a,b);

  (void) WriteBlobByte(image,(unsigned char) (a & 0xff));
  (void) WriteBlobByte(image,(unsigned char) (a >> 8));
  (void) WriteBlobByte(image,(unsigned char) (b & 0xff));
  (void) WriteBlobByte(image,(unsigned char) (b >> 8));

  for (i=0; i<4; i++)
  {
     ind = remapped + 4*i;
     (void) WriteBlobByte(image,ind[0] | (ind[1] << 2) | (ind[2] << 4) |
       (ind[3] << 6));
  }
}

static void WriteCompressed(Image *image, const size_t count,
  DDSVector4 *points, const ssize_t *map, const MagickBooleanType clusterFit)
{
  float
    covariance[16];

  DDSVector3
    end,
    principle,
    start;

  DDSVector4
    metric;

  unsigned char
    indices[16];

  VectorInit(metric,1.0f);
  VectorInit3(start,0.0f);
  VectorInit3(end,0.0f);

  ComputeWeightedCovariance(count,points,covariance);
  ComputePrincipleComponent(covariance,&principle);

  if ((clusterFit == MagickFalse) || (count == 0))
    CompressRangeFit(count,points,map,principle,metric,&start,&end,indices);
  else
    CompressClusterFit(count,points,map,principle,metric,&start,&end,indices);

  WriteIndices(image,start,end,indices);
}

static void WriteSingleColorFit(Image *image, const DDSVector4 *points,
  const ssize_t *map)
{
  DDSVector3
    start,
    end;

  ssize_t
    i;

  unsigned char
    color[3],
    index,
    indexes[16],
    indices[16];

  color[0] = (unsigned char) ClampToLimit(255.0f*points->x,255);
  color[1] = (unsigned char) ClampToLimit(255.0f*points->y,255);
  color[2] = (unsigned char) ClampToLimit(255.0f*points->z,255);

  index=0;
  ComputeEndPoints(DDS_LOOKUP,color,&start,&end,&index);

  for (i=0; i< 16; i++)
    indexes[i]=index;
  RemapIndices(map,indexes,indices);
  WriteIndices(image,start,end,indices);
}

static void WriteFourCC(Image *image, const size_t compression,
  const MagickBooleanType clusterFit, const MagickBooleanType weightByAlpha,
  ExceptionInfo *exception)
{
  ssize_t
    x;

  ssize_t
    i,
    y,
    bx,
    by;

  const Quantum
    *p;

  for (y=0; y < (ssize_t) image->rows; y+=4)
  {
    for (x=0; x < (ssize_t) image->columns; x+=4)
    {
      MagickBooleanType
        match;

      DDSVector4
        point,
        points[16];

      size_t
        count = 0,
        max5 = 0,
        max7 = 0,
        min5 = 255,
        min7 = 255,
        columns = 4,
        rows = 4;

      ssize_t
        alphas[16],
        map[16];

      unsigned char
        alpha;

      if (x + columns >= image->columns)
        columns = image->columns - x;

      if (y + rows >= image->rows)
        rows = image->rows - y;

      p=GetVirtualPixels(image,x,y,columns,rows,exception);
      if (p == (const Quantum *) NULL)
        break;

      for (i=0; i<16; i++)
      {
        map[i] = -1;
        alphas[i] = -1;
      }

      for (by=0; by < (ssize_t) rows; by++)
      {
        for (bx=0; bx < (ssize_t) columns; bx++)
        {
          if (compression == FOURCC_DXT5)
            alpha = ScaleQuantumToChar(GetPixelAlpha(image,p));
          else
            alpha = 255;

          if (compression == FOURCC_DXT5)
            {
              if (alpha < min7)
                min7 = alpha;
              if (alpha > max7)
                max7 = alpha;
              if (alpha != 0 && alpha < min5)
                min5 = alpha;
              if (alpha != 255 && alpha > max5)
                max5 = alpha;
            }
          
          alphas[4*by + bx] = (size_t)alpha;

          point.x = (float)ScaleQuantumToChar(GetPixelRed(image,p)) / 255.0f;
          point.y = (float)ScaleQuantumToChar(GetPixelGreen(image,p)) / 255.0f;
          point.z = (float)ScaleQuantumToChar(GetPixelBlue(image,p)) / 255.0f;
          point.w = weightByAlpha ? (float)(alpha + 1) / 256.0f : 1.0f;
          p+=GetPixelChannels(image);

          match = MagickFalse;
          for (i=0; i < (ssize_t) count; i++)
          {
            if ((points[i].x == point.x) &&
                (points[i].y == point.y) &&
                (points[i].z == point.z) &&
                (alpha       >= 128 || compression == FOURCC_DXT5))
              {
                points[i].w += point.w;
                map[4*by + bx] = i;
                match = MagickTrue;
                break;
              }
          }

          if (match != MagickFalse)
            continue;

          points[count].x = point.x;
          points[count].y = point.y;
          points[count].z = point.z;
          points[count].w = point.w;
          map[4*by + bx] = count;
          count++;
        }
      }

      for (i=0; i < (ssize_t) count; i++)
        points[i].w = sqrt(points[i].w);

      if (compression == FOURCC_DXT5)
        WriteAlphas(image,alphas,min5,max5,min7,max7);

      if (count == 1)
        WriteSingleColorFit(image,points,map);
      else
        WriteCompressed(image,count,points,map,clusterFit);
    }
  }
}

static void WriteUncompressed(Image *image, ExceptionInfo *exception)
{
  const Quantum
    *p;

  ssize_t
    x;

  ssize_t
    y;

  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;

    for (x=0; x < (ssize_t) image->columns; x++)
    {
      (void) WriteBlobByte(image,ScaleQuantumToChar(GetPixelBlue(image,p)));
      (void) WriteBlobByte(image,ScaleQuantumToChar(GetPixelGreen(image,p)));
      (void) WriteBlobByte(image,ScaleQuantumToChar(GetPixelRed(image,p)));
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) WriteBlobByte(image,ScaleQuantumToChar(GetPixelAlpha(image,p)));
      p+=GetPixelChannels(image);
    }
  }
}

static void WriteImageData(Image *image, const size_t pixelFormat,
  const size_t compression,const MagickBooleanType clusterFit,
  const MagickBooleanType weightByAlpha, ExceptionInfo *exception)
{
  if (pixelFormat == DDPF_FOURCC)
    WriteFourCC(image,compression,clusterFit,weightByAlpha,exception);
  else
    WriteUncompressed(image,exception);
}

static MagickBooleanType WriteMipmaps(Image *image,const ImageInfo *image_info,
  const size_t pixelFormat,const size_t compression,const size_t mipmaps,
  const MagickBooleanType fromlist,const MagickBooleanType clusterFit,
  const MagickBooleanType weightByAlpha,ExceptionInfo *exception)
{
  const char
    *option;

  Image
    *mipmap_image,
    *resize_image;

  MagickBooleanType
    fast_mipmaps,
    status;

  ssize_t
    i;

  size_t
    columns,
    rows;

  columns=DIV2(image->columns);
  rows=DIV2(image->rows);

  option=GetImageOption(image_info,"dds:fast-mipmaps");
  fast_mipmaps=IsStringTrue(option);
  mipmap_image=image;
  resize_image=image;
  status=MagickTrue;
  for (i=0; i < (ssize_t) mipmaps; i++)
  {
    if (fromlist == MagickFalse)
      {
        mipmap_image=ResizeImage(resize_image,columns,rows,TriangleFilter,
          exception);

        if (mipmap_image == (Image *) NULL)
          {
            status=MagickFalse;
            break;
          }
      }
    else
      {
        mipmap_image=mipmap_image->next;
        if ((mipmap_image->columns != columns) || (mipmap_image->rows != rows))
          ThrowBinaryException(CoderError,"ImageColumnOrRowSizeIsNotSupported",
            image->filename);
      }

    DestroyBlob(mipmap_image);
    mipmap_image->blob=ReferenceBlob(image->blob);

    WriteImageData(mipmap_image,pixelFormat,compression,weightByAlpha,
      clusterFit,exception);

    if (fromlist == MagickFalse)
      {
        if (fast_mipmaps == MagickFalse)
          mipmap_image=DestroyImage(mipmap_image);
        else
          {
            if (resize_image != image)
              resize_image=DestroyImage(resize_image);
            resize_image=mipmap_image;
          }
      }

    columns=DIV2(columns);
    rows=DIV2(rows);
  }

  if (resize_image != image)
    resize_image=DestroyImage(resize_image);

  return(status);
}

static void WriteDDSInfo(Image *image, const size_t pixelFormat,
  const size_t compression, const size_t mipmaps)
{
  char
    software[MagickPathExtent];

  ssize_t
    i;

  unsigned int
    format,
    caps,
    flags;

  flags=(unsigned int) (DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT |
    DDSD_PIXELFORMAT);
  caps=(unsigned int) DDSCAPS_TEXTURE;
  format=(unsigned int) pixelFormat;

  if (format == DDPF_FOURCC)
      flags=flags | DDSD_LINEARSIZE;
  else
      flags=flags | DDSD_PITCH;

  if (mipmaps > 0)
    {
      flags=flags | (unsigned int) DDSD_MIPMAPCOUNT;
      caps=caps | (unsigned int) (DDSCAPS_MIPMAP | DDSCAPS_COMPLEX);
    }

  if (format != DDPF_FOURCC && image->alpha_trait != UndefinedPixelTrait)
    format=format | DDPF_ALPHAPIXELS;

  (void) WriteBlob(image,4,(unsigned char *) "DDS ");
  (void) WriteBlobLSBLong(image,124);
  (void) WriteBlobLSBLong(image,flags);
  (void) WriteBlobLSBLong(image,(unsigned int) image->rows);
  (void) WriteBlobLSBLong(image,(unsigned int) image->columns);

  if (pixelFormat == DDPF_FOURCC)
    {
      /* Compressed DDS requires linear compressed size of first image */
      if (compression == FOURCC_DXT1)
        (void) WriteBlobLSBLong(image,(unsigned int) (MagickMax(1,
          (image->columns+3)/4)*MagickMax(1,(image->rows+3)/4)*8));
      else /* DXT5 */
        (void) WriteBlobLSBLong(image,(unsigned int) (MagickMax(1,
          (image->columns+3)/4)*MagickMax(1,(image->rows+3)/4)*16));
    }
  else
    {
      /* Uncompressed DDS requires byte pitch of first image */
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) WriteBlobLSBLong(image,(unsigned int) (image->columns * 4));
      else
        (void) WriteBlobLSBLong(image,(unsigned int) (image->columns * 3));
    }

  (void) WriteBlobLSBLong(image,0x00);
  (void) WriteBlobLSBLong(image,(unsigned int) mipmaps+1);
  (void) memset(software,0,sizeof(software));
  (void) CopyMagickString(software,"IMAGEMAGICK",MagickPathExtent);
  (void) WriteBlob(image,44,(unsigned char *) software);

  (void) WriteBlobLSBLong(image,32);
  (void) WriteBlobLSBLong(image,format);

  if (pixelFormat == DDPF_FOURCC)
    {
      (void) WriteBlobLSBLong(image,(unsigned int) compression);
      for(i=0;i < 5;i++)  /* bitcount / masks */
        (void) WriteBlobLSBLong(image,0x00);
    }
  else
    {
      (void) WriteBlobLSBLong(image,0x00);
      if (image->alpha_trait != UndefinedPixelTrait)
        {
          (void) WriteBlobLSBLong(image,32);
          (void) WriteBlobLSBLong(image,0xff0000);
          (void) WriteBlobLSBLong(image,0xff00);
          (void) WriteBlobLSBLong(image,0xff);
          (void) WriteBlobLSBLong(image,0xff000000);
        }
      else
        {
          (void) WriteBlobLSBLong(image,24);
          (void) WriteBlobLSBLong(image,0xff0000);
          (void) WriteBlobLSBLong(image,0xff00);
          (void) WriteBlobLSBLong(image,0xff);
          (void) WriteBlobLSBLong(image,0x00);
        }
    }
  
  (void) WriteBlobLSBLong(image,caps);
  for(i=0;i < 4;i++)   /* ddscaps2 + reserved region */
    (void) WriteBlobLSBLong(image,0x00);
}

static MagickBooleanType WriteDDSImage(const ImageInfo *image_info,
  Image *image, ExceptionInfo *exception)
{
  const char
    *option;

  size_t
    compression,
    columns,
    maxMipmaps,
    mipmaps,
    pixelFormat,
    rows;

  MagickBooleanType
    clusterFit,
    fromlist,
    status,
    weightByAlpha;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  (void) TransformImageColorspace(image,sRGBColorspace,exception);
  pixelFormat=DDPF_FOURCC;
  compression=FOURCC_DXT5;
  if (image->alpha_trait == UndefinedPixelTrait)
    compression=FOURCC_DXT1;
  if (LocaleCompare(image_info->magick,"dxt1") == 0)
    compression=FOURCC_DXT1;
  if (image_info->compression == DXT1Compression)
    compression=FOURCC_DXT1;
  else if (image_info->compression == NoCompression)
    pixelFormat=DDPF_RGB;
  option=GetImageOption(image_info,"dds:compression");
  if (option != (char *) NULL)
    {
       if (LocaleCompare(option,"dxt1") == 0)
         compression=FOURCC_DXT1;
       if (LocaleCompare(option,"none") == 0)
         pixelFormat=DDPF_RGB;
    }
  clusterFit=MagickFalse;
  weightByAlpha=MagickFalse;
  if (pixelFormat == DDPF_FOURCC)
    {
      option=GetImageOption(image_info,"dds:cluster-fit");
      if (IsStringTrue(option) != MagickFalse)
        {
          clusterFit=MagickTrue;
          if (compression != FOURCC_DXT1)
            {
              option=GetImageOption(image_info,"dds:weight-by-alpha");
              if (IsStringTrue(option) != MagickFalse)
                weightByAlpha=MagickTrue;
            }
        }
    }
  mipmaps=0;
  fromlist=MagickFalse;
  option=GetImageOption(image_info,"dds:mipmaps");
  if (option != (char *) NULL)
    {
      if (LocaleNCompare(option,"fromlist",8) == 0)
        {
          Image
            *next;

          fromlist=MagickTrue;
          next=image->next;
          while(next != (Image *) NULL)
          {
            mipmaps++;
            next=next->next;
          }
        }
    }
  if ((mipmaps == 0) &&
      ((image->columns & (image->columns - 1)) == 0) &&
      ((image->rows & (image->rows - 1)) == 0))
    {
      maxMipmaps=SIZE_MAX;
      if (option != (char *) NULL)
        maxMipmaps=StringToUnsignedLong(option);

      if (maxMipmaps != 0)
        {
          columns=image->columns;
          rows=image->rows;
          while ((columns != 1 || rows != 1) && mipmaps != maxMipmaps)
          {
            columns=DIV2(columns);
            rows=DIV2(rows);
            mipmaps++;
          }
        }
    }
  option=GetImageOption(image_info,"dds:raw");
  if (IsStringTrue(option) == MagickFalse)
    WriteDDSInfo(image,pixelFormat,compression,mipmaps);
  else
    mipmaps=0;
  WriteImageData(image,pixelFormat,compression,clusterFit,weightByAlpha,
    exception);
  if ((mipmaps > 0) && (WriteMipmaps(image,image_info,pixelFormat,compression,
       mipmaps,fromlist,clusterFit,weightByAlpha,exception) == MagickFalse))
    return(MagickFalse);
  (void) CloseBlob(image);
  return(MagickTrue);
}

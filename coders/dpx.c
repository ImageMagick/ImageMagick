/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            DDDD   PPPP   X   X                              %
%                            D   D  P   P   X X                               %
%                            D   D  PPPP    XXX                               %
%                            D   D  P       X X                               %
%                            DDDD   P      X   X                              %
%                                                                             %
%                                                                             %
%                     Read/Write SMTPE DPX Image Format                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                March 2001                                   %
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
#include "magick/artifact.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/colorspace.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel-accessor.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/string-private.h"

/*
  Typedef declaration.
*/
typedef enum
{
  UserDefinedColorimetric = 0,
  PrintingDensityColorimetric = 1,
  LinearColorimetric = 2,
  LogarithmicColorimetric = 3,
  UnspecifiedVideoColorimetric = 4,
  SMTPE_274MColorimetric = 5,
  ITU_R709Colorimetric = 6,
  ITU_R601_625LColorimetric = 7,
  ITU_R601_525LColorimetric = 8,
  NTSCCompositeVideoColorimetric = 9,
  PALCompositeVideoColorimetric = 10,
  ZDepthLinearColorimetric = 11,
  DepthHomogeneousColorimetric = 12
} DPXColorimetric;

typedef enum
{
  UndefinedComponentType = 0,
  RedComponentType = 1,
  GreenComponentType = 2,
  BlueComponentType = 3,
  AlphaComponentType = 4,
  LumaComponentType = 6,
  ColorDifferenceCbCrComponentType = 7,
  DepthComponentType = 8,
  CompositeVideoComponentType = 9,
  RGBComponentType = 50,
  RGBAComponentType = 51,
  ABGRComponentType = 52,
  CbYCrY422ComponentType = 100,
  CbYACrYA4224ComponentType = 101,
  CbYCr444ComponentType = 102,
  CbYCrA4444ComponentType = 103,
  UserDef2ElementComponentType = 150,
  UserDef3ElementComponentType = 151,
  UserDef4ElementComponentType = 152,
  UserDef5ElementComponentType = 153,
  UserDef6ElementComponentType = 154,
  UserDef7ElementComponentType = 155,
  UserDef8ElementComponentType = 156
} DPXComponentType;

typedef enum
{
  TransferCharacteristicUserDefined = 0,
  TransferCharacteristicPrintingDensity = 1,
  TransferCharacteristicLinear = 2,
  TransferCharacteristicLogarithmic = 3,
  TransferCharacteristicUnspecifiedVideo = 4,
  TransferCharacteristicSMTPE274M = 5,     /* 1920x1080 TV */
  TransferCharacteristicITU_R709 = 6,      /* ITU R709 */
  TransferCharacteristicITU_R601_625L = 7, /* 625 Line */
  TransferCharacteristicITU_R601_525L = 8, /* 525 Line */
  TransferCharacteristicNTSCCompositeVideo = 9,
  TransferCharacteristicPALCompositeVideo = 10,
  TransferCharacteristicZDepthLinear = 11,
  TransferCharacteristicZDepthHomogeneous = 12
} DPXTransferCharacteristic;

typedef struct _DPXFileInfo
{
  unsigned int
    magic,
    image_offset;

  char
    version[8];

  unsigned int
    file_size,
    ditto_key,
    generic_size,
    industry_size,
    user_size;

  char
    filename[100],
    timestamp[24],
    creator[100],
    project[200],
    copyright[200];

  unsigned int
    encrypt_key;

  char
    reserve[104];
} DPXFileInfo;

typedef struct _DPXFilmInfo
{
  char
    id[2],
    type[2],
    offset[2],
    prefix[6],
    count[4],
    format[32];

  unsigned int
    frame_position,
    sequence_extent,
    held_count;

  float
    frame_rate,
    shutter_angle;

  char
    frame_id[32],
    slate[100],
    reserve[56];
} DPXFilmInfo;

typedef struct _DPXImageElement
{
  unsigned int
    data_sign,
    low_data;

  float
    low_quantity;

  unsigned int
    high_data;

  float
    high_quantity;

  unsigned char
    descriptor,
    transfer_characteristic,
    colorimetric,
    bit_size;

  unsigned short
    packing,
    encoding;

  unsigned int
    data_offset,
    end_of_line_padding,
    end_of_image_padding;

  unsigned char
    description[32];
} DPXImageElement;

typedef struct _DPXImageInfo
{
  unsigned short
    orientation,
    number_elements;

  unsigned int
    pixels_per_line,
    lines_per_element;

  DPXImageElement
    image_element[8];

  unsigned char
    reserve[52];
} DPXImageInfo;

typedef struct _DPXOrientationInfo
{
  unsigned int
    x_offset,
    y_offset;

  float
    x_center,
    y_center;

  unsigned int
    x_size,
    y_size;

  char
    filename[100],
    timestamp[24],
    device[32],
    serial[32];

  unsigned short
    border[4];

  unsigned int
    aspect_ratio[2];

  unsigned char
    reserve[28];
} DPXOrientationInfo;

typedef struct _DPXTelevisionInfo
{
  unsigned int
    time_code,
    user_bits;

  unsigned char
    interlace,
    field_number,
    video_signal,
    padding;

  float
    horizontal_sample_rate,
    vertical_sample_rate,
    frame_rate,
    time_offset,
    gamma,
    black_level,
    black_gain,
    break_point,
    white_level,
    integration_times;

  char
    reserve[76];
} DPXTelevisionInfo;

typedef struct _DPXUserInfo
{
  char
    id[32];
} DPXUserInfo;

typedef struct DPXInfo
{
  DPXFileInfo
    file;

  DPXImageInfo
    image;

  DPXOrientationInfo
    orientation;

  DPXFilmInfo
    film;

  DPXTelevisionInfo
    television;

  DPXUserInfo
    user;
} DPXInfo;

/*
  Forward declaractions.
*/
static MagickBooleanType
  WriteDPXImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s D P X                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsDPX() returns MagickTrue if the image format type, identified by the
%  magick string, is DPX.
%
%  The format of the IsDPX method is:
%
%      MagickBooleanType IsDPX(const unsigned char *magick,const size_t extent)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o extent: Specifies the extent of the magick string.
%
*/
static MagickBooleanType IsDPX(const unsigned char *magick,const size_t extent)
{
  if (extent < 4)
    return(MagickFalse);
  if (memcmp(magick,"SDPX",4) == 0)
    return(MagickTrue);
  if (memcmp(magick,"XPDS",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d D P X I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDPXImage() reads an DPX X image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadDPXImage method is:
%
%      Image *ReadDPXImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static size_t GetBytesPerRow(const size_t columns,
  const size_t samples_per_pixel,const size_t bits_per_pixel,
  const MagickBooleanType pad)
{
  size_t
    bytes_per_row;

  switch (bits_per_pixel)
  {
    case 1:
    {
      bytes_per_row=4*(((size_t) samples_per_pixel*columns*bits_per_pixel+31)/
        32);
      break;
    }
    case 8:
    default:
    {
      bytes_per_row=4*(((size_t) samples_per_pixel*columns*bits_per_pixel+31)/
        32);
      break;
    }
    case 10:
    {
      if (pad == MagickFalse)
        {
          bytes_per_row=4*(((size_t) samples_per_pixel*columns*bits_per_pixel+
            31)/32);
          break;
        }
      bytes_per_row=4*(((size_t) (32*((samples_per_pixel*columns+2)/3))+31)/32);
      break;
    }
    case 12:
    {
      if (pad == MagickFalse)
        {
          bytes_per_row=4*(((size_t) samples_per_pixel*columns*bits_per_pixel+
            31)/32);
          break;
        }
      bytes_per_row=2*(((size_t) (16*samples_per_pixel*columns)+15)/16);
      break;
    }
    case 16:
    {
      bytes_per_row=2*(((size_t) samples_per_pixel*columns*bits_per_pixel+8)/
        16);
      break;
    }
    case 32:
    {
      bytes_per_row=4*(((size_t) samples_per_pixel*columns*bits_per_pixel+31)/
        32);
      break;
    }
    case 64:
    {
      bytes_per_row=8*(((size_t) samples_per_pixel*columns*bits_per_pixel+63)/
        64);
      break;
    }
  }
  return(bytes_per_row);
}

static const char *GetImageTransferCharacteristic(
  const DPXTransferCharacteristic characteristic)
{
  const char
    *transfer;

  /*
    Get the element transfer characteristic.
  */
  switch(characteristic)
  {
    case TransferCharacteristicUserDefined:
    {
      transfer="UserDefined";
      break;
    }
    case TransferCharacteristicPrintingDensity:
    {
      transfer="PrintingDensity";
      break;
    }
    case TransferCharacteristicLinear:
    {
      transfer="Linear";
      break;
    }
    case TransferCharacteristicLogarithmic:
    {
      transfer="Logarithmic";
      break;
    }
    case TransferCharacteristicUnspecifiedVideo:
    {
      transfer="UnspecifiedVideo";
      break;
    }
    case TransferCharacteristicSMTPE274M:
    {
      transfer="SMTPE274M";
      break;
    }
    case TransferCharacteristicITU_R709:
    {
      transfer="ITU-R709";
      break;
    }
    case TransferCharacteristicITU_R601_625L:
    {
      transfer="ITU-R601-625L";
      break;
    }
    case TransferCharacteristicITU_R601_525L:
    {
      transfer="ITU-R601-525L";
      break;
    }
    case TransferCharacteristicNTSCCompositeVideo:
    {
      transfer="NTSCCompositeVideo";
      break;
    }
    case TransferCharacteristicPALCompositeVideo:
    {
      transfer="PALCompositeVideo";
      break;
    }
    case TransferCharacteristicZDepthLinear:
    {
      transfer="ZDepthLinear";
      break;
    }
    case TransferCharacteristicZDepthHomogeneous:
    {
      transfer="ZDepthHomogeneous";
      break;
    }
    default:
      transfer="Reserved";
  }
  return(transfer);
}

static inline MagickBooleanType IsFloatDefined(const float value)
{
  union
  {
    unsigned int
      unsigned_value;

    float
      float_value;
  } quantum;

  quantum.unsigned_value=0U;
  quantum.float_value=value;
  if (quantum.unsigned_value == 0U)
    return(MagickFalse);
  return(MagickTrue);
}

static void SetPrimaryChromaticity(const DPXColorimetric colorimetric,
  ChromaticityInfo *chromaticity_info)
{
  switch(colorimetric)
  {
    case SMTPE_274MColorimetric:
    case ITU_R709Colorimetric:
    {
      chromaticity_info->red_primary.x=0.640;
      chromaticity_info->red_primary.y=0.330;
      chromaticity_info->red_primary.z=0.030;
      chromaticity_info->green_primary.x=0.300;
      chromaticity_info->green_primary.y=0.600;
      chromaticity_info->green_primary.z=0.100;
      chromaticity_info->blue_primary.x=0.150;
      chromaticity_info->blue_primary.y=0.060;
      chromaticity_info->blue_primary.z=0.790;
      chromaticity_info->white_point.x=0.3127;
      chromaticity_info->white_point.y=0.3290;
      chromaticity_info->white_point.z=0.3582;
      break;
    }
    case NTSCCompositeVideoColorimetric:
    {
      chromaticity_info->red_primary.x=0.67;
      chromaticity_info->red_primary.y=0.33;
      chromaticity_info->red_primary.z=0.00;
      chromaticity_info->green_primary.x=0.21;
      chromaticity_info->green_primary.y=0.71;
      chromaticity_info->green_primary.z=0.08;
      chromaticity_info->blue_primary.x=0.14;
      chromaticity_info->blue_primary.y=0.08;
      chromaticity_info->blue_primary.z=0.78;
      chromaticity_info->white_point.x=0.310;
      chromaticity_info->white_point.y=0.316;
      chromaticity_info->white_point.z=0.374;
      break;
    }
    case PALCompositeVideoColorimetric:
    {
      chromaticity_info->red_primary.x=0.640;
      chromaticity_info->red_primary.y=0.330;
      chromaticity_info->red_primary.z=0.030;
      chromaticity_info->green_primary.x=0.290;
      chromaticity_info->green_primary.y=0.600;
      chromaticity_info->green_primary.z=0.110;
      chromaticity_info->blue_primary.x=0.150;
      chromaticity_info->blue_primary.y=0.060;
      chromaticity_info->blue_primary.z=0.790;
      chromaticity_info->white_point.x=0.3127;
      chromaticity_info->white_point.y=0.3290;
      chromaticity_info->white_point.z=0.3582;
      break;
    }
    default:
      break;
  }
}

static void TimeCodeToString(const size_t timestamp,char *code)
{
#define TimeFields  7

  unsigned int
    shift;

  register ssize_t
    i;

  *code='\0';
  shift=4*TimeFields;
  for (i=0; i <= TimeFields; i++)
  {
    (void) FormatLocaleString(code,MaxTextExtent-strlen(code),"%x",
      (unsigned int) ((timestamp >> shift) & 0x0fU));
    code++;
    if (((i % 2) != 0) && (i < TimeFields))
      *code++=':';
    shift-=4;
    *code='\0';
  }
}

static Image *ReadDPXImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    magick[4],
    value[MaxTextExtent];

  DPXInfo
    dpx;

  Image
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register ssize_t
    i;

  size_t
    extent,
    samples_per_pixel;

  ssize_t
    count,
    n,
    row,
    y;

  unsigned char
    component_type;

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
    Read DPX file header.
  */
  offset=0;
  count=ReadBlob(image,4,(unsigned char *) magick);
  offset+=count;
  if ((count != 4) || ((LocaleNCompare(magick,"SDPX",4) != 0) &&
      (LocaleNCompare((char *) magick,"XPDS",4) != 0)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  image->endian=LSBEndian;
  if (LocaleNCompare(magick,"SDPX",4) == 0)
    image->endian=MSBEndian;
  (void) ResetMagickMemory(&dpx,0,sizeof(dpx));
  dpx.file.image_offset=ReadBlobLong(image);
  offset+=4;
  offset+=ReadBlob(image,sizeof(dpx.file.version),(unsigned char *)
    dpx.file.version);
  (void) FormatImageProperty(image,"dpx:file.version","%.8s",dpx.file.version);
  dpx.file.file_size=ReadBlobLong(image);
  offset+=4;
  dpx.file.ditto_key=ReadBlobLong(image);
  offset+=4;
  if (dpx.file.ditto_key != ~0U)
    (void) FormatImageProperty(image,"dpx:file.ditto.key","%u",
      dpx.file.ditto_key);
  dpx.file.generic_size=ReadBlobLong(image);
  offset+=4;
  dpx.file.industry_size=ReadBlobLong(image);
  offset+=4;
  dpx.file.user_size=ReadBlobLong(image);
  offset+=4;
  offset+=ReadBlob(image,sizeof(dpx.file.filename),(unsigned char *)
    dpx.file.filename);
  (void) FormatImageProperty(image,"dpx:file.filename","%.100s",
    dpx.file.filename);
  (void) FormatImageProperty(image,"document","%.100s",dpx.file.filename);
  offset+=ReadBlob(image,sizeof(dpx.file.timestamp),(unsigned char *)
    dpx.file.timestamp);
  if (*dpx.file.timestamp != '\0')
    (void) FormatImageProperty(image,"dpx:file.timestamp","%.24s",
      dpx.file.timestamp);
  offset+=ReadBlob(image,sizeof(dpx.file.creator),(unsigned char *)
    dpx.file.creator);
  if (*dpx.file.creator == '\0')
    {
      (void) FormatImageProperty(image,"dpx:file.creator","%.100s",
        GetMagickVersion((size_t *) NULL));
      (void) FormatImageProperty(image,"software","%.100s",
        GetMagickVersion((size_t *) NULL));
    }
  else
    {
      (void) FormatImageProperty(image,"dpx:file.creator","%.100s",
        dpx.file.creator);
      (void) FormatImageProperty(image,"software","%.100s",dpx.file.creator);
    }
  offset+=ReadBlob(image,sizeof(dpx.file.project),(unsigned char *)
    dpx.file.project);
  if (*dpx.file.project != '\0')
    {
      (void) FormatImageProperty(image,"dpx:file.project","%.200s",
        dpx.file.project);
      (void) FormatImageProperty(image,"comment","%.100s",dpx.file.project);
    }
  offset+=ReadBlob(image,sizeof(dpx.file.copyright),(unsigned char *)
    dpx.file.copyright);
  if (*dpx.file.copyright != '\0')
    {
      (void) FormatImageProperty(image,"dpx:file.copyright","%.200s",
        dpx.file.copyright);
      (void) FormatImageProperty(image,"copyright","%.100s",
        dpx.file.copyright);
    }
  dpx.file.encrypt_key=ReadBlobLong(image);
  offset+=4;
  if (dpx.file.encrypt_key != ~0U)
    (void) FormatImageProperty(image,"dpx:file.encrypt_key","%u",
      dpx.file.encrypt_key);
  offset+=ReadBlob(image,sizeof(dpx.file.reserve),(unsigned char *)
    dpx.file.reserve);
  /*
    Read DPX image header.
  */
  dpx.image.orientation=ReadBlobShort(image);
  offset+=2;
  if (dpx.image.orientation != (unsigned short) ~0)
    (void) FormatImageProperty(image,"dpx:image.orientation","%d",
      dpx.image.orientation);
  switch (dpx.image.orientation)
  {
    default:
    case 0: image->orientation=TopLeftOrientation; break;
    case 1: image->orientation=TopRightOrientation; break;
    case 2: image->orientation=BottomLeftOrientation; break;
    case 3: image->orientation=BottomRightOrientation; break;
    case 4: image->orientation=LeftTopOrientation; break;
    case 5: image->orientation=RightTopOrientation; break;
    case 6: image->orientation=LeftBottomOrientation; break;
    case 7: image->orientation=RightBottomOrientation; break;
  }
  dpx.image.number_elements=ReadBlobShort(image);
  offset+=2;
  dpx.image.pixels_per_line=ReadBlobLong(image);
  offset+=4;
  image->columns=dpx.image.pixels_per_line;
  dpx.image.lines_per_element=ReadBlobLong(image);
  offset+=4;
  image->rows=dpx.image.lines_per_element;
  for (i=0; i < 8; i++)
  {
    char
      property[MaxTextExtent];

    dpx.image.image_element[i].data_sign=ReadBlobLong(image);
    offset+=4;
    dpx.image.image_element[i].low_data=ReadBlobLong(image);
    offset+=4;
    dpx.image.image_element[i].low_quantity=ReadBlobFloat(image);
    offset+=4;
    dpx.image.image_element[i].high_data=ReadBlobLong(image);
    offset+=4;
    dpx.image.image_element[i].high_quantity=ReadBlobFloat(image);
    offset+=4;
    dpx.image.image_element[i].descriptor=(unsigned char) ReadBlobByte(image);
    offset++;
    dpx.image.image_element[i].transfer_characteristic=(unsigned char)
      ReadBlobByte(image);
    (void) FormatLocaleString(property,MaxTextExtent,
      "dpx:image.element[%lu].transfer-characteristic",(long) i);
    (void) FormatImageProperty(image,property,"%s",
      GetImageTransferCharacteristic((DPXTransferCharacteristic)
      dpx.image.image_element[i].transfer_characteristic));
    offset++;
    dpx.image.image_element[i].colorimetric=(unsigned char) ReadBlobByte(image);
    offset++;
    dpx.image.image_element[i].bit_size=(unsigned char) ReadBlobByte(image);
    offset++;
    dpx.image.image_element[i].packing=ReadBlobShort(image);
    offset+=2;
    dpx.image.image_element[i].encoding=ReadBlobShort(image);
    offset+=2;
    dpx.image.image_element[i].data_offset=ReadBlobLong(image);
    offset+=4;
    dpx.image.image_element[i].end_of_line_padding=ReadBlobLong(image);
    offset+=4;
    dpx.image.image_element[i].end_of_image_padding=ReadBlobLong(image);
    offset+=4;
    offset+=ReadBlob(image,sizeof(dpx.image.image_element[i].description),
      (unsigned char *) dpx.image.image_element[i].description);
  }
  (void) SetImageColorspace(image,RGBColorspace);
  offset+=ReadBlob(image,sizeof(dpx.image.reserve),(unsigned char *)
    dpx.image.reserve);
  if (dpx.file.image_offset >= 1664U)
    {
      /*
        Read DPX orientation header.
      */
      dpx.orientation.x_offset=ReadBlobLong(image);
      offset+=4;
      if (dpx.orientation.x_offset != ~0U)
        (void) FormatImageProperty(image,"dpx:orientation.x_offset","%u",
          dpx.orientation.x_offset);
      dpx.orientation.y_offset=ReadBlobLong(image);
      offset+=4;
      if (dpx.orientation.y_offset != ~0U)
        (void) FormatImageProperty(image,"dpx:orientation.y_offset","%u",
          dpx.orientation.y_offset);
      dpx.orientation.x_center=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.orientation.x_center) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:orientation.x_center","%g",
          dpx.orientation.x_center);
      dpx.orientation.y_center=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.orientation.y_center) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:orientation.y_center","%g",
          dpx.orientation.y_center);
      dpx.orientation.x_size=ReadBlobLong(image);
      offset+=4;
      if (dpx.orientation.x_size != ~0U)
        (void) FormatImageProperty(image,"dpx:orientation.x_size","%u",
          dpx.orientation.x_size);
      dpx.orientation.y_size=ReadBlobLong(image);
      offset+=4;
      if (dpx.orientation.y_size != ~0U)
        (void) FormatImageProperty(image,"dpx:orientation.y_size","%u",
          dpx.orientation.y_size);
      offset+=ReadBlob(image,sizeof(dpx.orientation.filename),(unsigned char *)
        dpx.orientation.filename);
      if (*dpx.orientation.filename != '\0')
        (void) FormatImageProperty(image,"dpx:orientation.filename","%.100s",
          dpx.orientation.filename);
      offset+=ReadBlob(image,sizeof(dpx.orientation.timestamp),(unsigned char *)
        dpx.orientation.timestamp);
      if (*dpx.orientation.timestamp != '\0')
        (void) FormatImageProperty(image,"dpx:orientation.timestamp","%.24s",
          dpx.orientation.timestamp);
      offset+=ReadBlob(image,sizeof(dpx.orientation.device),(unsigned char *)
        dpx.orientation.device);
      if (*dpx.orientation.device != '\0')
        (void) FormatImageProperty(image,"dpx:orientation.device","%.32s",
          dpx.orientation.device);
      offset+=ReadBlob(image,sizeof(dpx.orientation.serial),(unsigned char *)
        dpx.orientation.serial);
      if (*dpx.orientation.serial != '\0')
        (void) FormatImageProperty(image,"dpx:orientation.serial","%.32s",
          dpx.orientation.serial);
      for (i=0; i < 4; i++)
      {
        dpx.orientation.border[i]=ReadBlobShort(image);
        offset+=2;
      }
      if ((dpx.orientation.border[0] != (unsigned short) (~0)) &&
          (dpx.orientation.border[1] != (unsigned short) (~0)))
        (void) FormatImageProperty(image,"dpx:orientation.border","%dx%d%+d%+d",          dpx.orientation.border[0],dpx.orientation.border[1],
          dpx.orientation.border[2],dpx.orientation.border[3]);
      for (i=0; i < 2; i++)
      {
        dpx.orientation.aspect_ratio[i]=ReadBlobLong(image);
        offset+=4;
      }
      if ((dpx.orientation.aspect_ratio[0] != ~0U) &&
          (dpx.orientation.aspect_ratio[1] != ~0U))
        (void) FormatImageProperty(image,"dpx:orientation.aspect_ratio",
          "%ux%u",dpx.orientation.aspect_ratio[0],
          dpx.orientation.aspect_ratio[1]);
      offset+=ReadBlob(image,sizeof(dpx.orientation.reserve),(unsigned char *)
        dpx.orientation.reserve);
    }
  if (dpx.file.image_offset >= 1920U)
    {
      /*
        Read DPX film header.
      */
      offset+=ReadBlob(image,sizeof(dpx.film.id),(unsigned char *) dpx.film.id);
      if (*dpx.film.type != '\0')
        (void) FormatImageProperty(image,"dpx:film.id","%.2s",dpx.film.id);
      offset+=ReadBlob(image,sizeof(dpx.film.type),(unsigned char *)
        dpx.film.type);
      if (*dpx.film.type != '\0')
        (void) FormatImageProperty(image,"dpx:film.type","%.2s",dpx.film.type);
      offset+=ReadBlob(image,sizeof(dpx.film.offset),(unsigned char *)
        dpx.film.offset);
      if (*dpx.film.offset != '\0')
        (void) FormatImageProperty(image,"dpx:film.offset","%.2s",
          dpx.film.offset);
      offset+=ReadBlob(image,sizeof(dpx.film.prefix),(unsigned char *)
        dpx.film.prefix);
      if (*dpx.film.prefix != '\0')
        (void) FormatImageProperty(image,"dpx:film.prefix","%.6s",
          dpx.film.prefix);
      offset+=ReadBlob(image,sizeof(dpx.film.count),(unsigned char *)
        dpx.film.count);
      if (*dpx.film.count != '\0')
        (void) FormatImageProperty(image,"dpx:film.count","%.4s",
          dpx.film.count);
      offset+=ReadBlob(image,sizeof(dpx.film.format),(unsigned char *)
        dpx.film.format);
      if (*dpx.film.format != '\0')
        (void) FormatImageProperty(image,"dpx:film.format","%.4s",
          dpx.film.format);
      dpx.film.frame_position=ReadBlobLong(image);
      offset+=4;
      if (dpx.film.frame_position != ~0U)
        (void) FormatImageProperty(image,"dpx:film.frame_position","%u",
          dpx.film.frame_position);
      dpx.film.sequence_extent=ReadBlobLong(image);
      offset+=4;
      if (dpx.film.sequence_extent != ~0U)
        (void) FormatImageProperty(image,"dpx:film.sequence_extent","%u",
          dpx.film.sequence_extent);
      dpx.film.held_count=ReadBlobLong(image);
      offset+=4;
      if (dpx.film.held_count != ~0U)
        (void) FormatImageProperty(image,"dpx:film.held_count","%u",
          dpx.film.held_count);
      dpx.film.frame_rate=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.film.frame_rate) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:film.frame_rate","%g",
          dpx.film.frame_rate);
      dpx.film.shutter_angle=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.film.shutter_angle) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:film.shutter_angle","%g",
          dpx.film.shutter_angle);
      offset+=ReadBlob(image,sizeof(dpx.film.frame_id),(unsigned char *)
        dpx.film.frame_id);
      if (*dpx.film.frame_id != '\0')
        (void) FormatImageProperty(image,"dpx:film.frame_id","%.32s",
          dpx.film.frame_id);
      offset+=ReadBlob(image,sizeof(dpx.film.slate),(unsigned char *)
        dpx.film.slate);
      if (*dpx.film.slate != '\0')
        (void) FormatImageProperty(image,"dpx:film.slate","%.100s",
          dpx.film.slate);
      offset+=ReadBlob(image,sizeof(dpx.film.reserve),(unsigned char *)
        dpx.film.reserve);
    }
  if (dpx.file.image_offset >= 2048U)
    {
      /*
        Read DPX television header.
      */
      dpx.television.time_code=(unsigned int) ReadBlobLong(image);
      offset+=4;
      TimeCodeToString(dpx.television.time_code,value);
      (void) SetImageProperty(image,"dpx:television.time.code",value);
      dpx.television.user_bits=(unsigned int) ReadBlobLong(image);
      offset+=4;
      TimeCodeToString(dpx.television.user_bits,value);
      (void) SetImageProperty(image,"dpx:television.user.bits",value);
      dpx.television.interlace=(unsigned char) ReadBlobByte(image);
      offset++;
      if (dpx.television.interlace != 0)
        (void) FormatImageProperty(image,"dpx:television.interlace","%.20g",
          (double) dpx.television.interlace);
      dpx.television.field_number=(unsigned char) ReadBlobByte(image);
      offset++;
      if (dpx.television.field_number != 0)
        (void) FormatImageProperty(image,"dpx:television.field_number","%.20g",
          (double) dpx.television.field_number);
      dpx.television.video_signal=(unsigned char) ReadBlobByte(image);
      offset++;
      if (dpx.television.video_signal != 0)
        (void) FormatImageProperty(image,"dpx:television.video_signal","%.20g",
          (double) dpx.television.video_signal);
      dpx.television.padding=(unsigned char) ReadBlobByte(image);
      offset++;
      if (dpx.television.padding != 0)
        (void) FormatImageProperty(image,"dpx:television.padding","%d",
          dpx.television.padding);
      dpx.television.horizontal_sample_rate=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.television.horizontal_sample_rate) != MagickFalse)
        (void) FormatImageProperty(image,
          "dpx:television.horizontal_sample_rate","%g",
          dpx.television.horizontal_sample_rate);
      dpx.television.vertical_sample_rate=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.television.vertical_sample_rate) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.vertical_sample_rate",
          "%g",dpx.television.vertical_sample_rate);
      dpx.television.frame_rate=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.television.frame_rate) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.frame_rate","%g",
          dpx.television.frame_rate);
      dpx.television.time_offset=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.television.time_offset) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.time_offset","%g",
          dpx.television.time_offset);
      dpx.television.gamma=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.television.gamma) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.gamma","%g",
          dpx.television.gamma);
      dpx.television.black_level=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.television.black_level) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.black_level","%g",
          dpx.television.black_level);
      dpx.television.black_gain=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.television.black_gain) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.black_gain","%g",
          dpx.television.black_gain);
      dpx.television.break_point=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.television.break_point) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.break_point","%g",
          dpx.television.break_point);
      dpx.television.white_level=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.television.white_level) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.white_level","%g",
          dpx.television.white_level);
      dpx.television.integration_times=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpx.television.integration_times) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.integration_times",
          "%g",dpx.television.integration_times);
      offset+=ReadBlob(image,sizeof(dpx.television.reserve),(unsigned char *)
        dpx.television.reserve);
    }
  if (dpx.file.image_offset > 2080U)
    {
      /*
        Read DPX user header.
      */
      offset+=ReadBlob(image,sizeof(dpx.user.id),(unsigned char *) dpx.user.id);
      if (*dpx.user.id != '\0')
        (void) FormatImageProperty(image,"dpx:user.id","%.32s",dpx.user.id);
      if ((dpx.file.user_size != ~0U) &&
          ((size_t) dpx.file.user_size > sizeof(dpx.user.id)))
        {
          StringInfo
            *profile;

           profile=BlobToStringInfo((const void *) NULL,
             dpx.file.user_size-sizeof(dpx.user.id));
           if (profile == (StringInfo *) NULL)
             ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
           offset+=ReadBlob(image,GetStringInfoLength(profile),
             GetStringInfoDatum(profile));
           (void) SetImageProfile(image,"dpx",profile);
           profile=DestroyStringInfo(profile);
        }
    }
  for ( ; offset < (MagickOffsetType) dpx.file.image_offset; offset++)
    (void) ReadBlobByte(image);
  /*
    Read DPX image header.
  */
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  for (n=0; n < (ssize_t) dpx.image.number_elements; n++)
  {
    /*
      Convert DPX raster image to pixel packets.
    */
    if ((dpx.image.image_element[n].data_offset != ~0U) &&
        (dpx.image.image_element[n].data_offset != 0U))
      {
         MagickOffsetType
           data_offset;

         data_offset=(MagickOffsetType) dpx.image.image_element[n].data_offset;
         if (data_offset < offset)
           offset=SeekBlob(image,data_offset,SEEK_SET);
         else
           for ( ; offset < data_offset; offset++)
             (void) ReadBlobByte(image);
          if (offset != data_offset)
            ThrowReaderException(CorruptImageError,"UnableToReadImageData");
       }
    SetPrimaryChromaticity((DPXColorimetric)
      dpx.image.image_element[n].colorimetric,&image->chromaticity);
    image->depth=dpx.image.image_element[n].bit_size;
    samples_per_pixel=1;
    quantum_type=GrayQuantum;
    component_type=dpx.image.image_element[n].descriptor;
    switch (component_type)
    {
      case CbYCrY422ComponentType:
      {
        samples_per_pixel=2;
        quantum_type=CbYCrYQuantum;
        break;
      }
      case CbYACrYA4224ComponentType:
      case CbYCr444ComponentType:
      {
        samples_per_pixel=3;
        quantum_type=CbYCrQuantum;
        break;
      }
      case RGBComponentType:
      {
        samples_per_pixel=3;
        quantum_type=RGBQuantum;
        break;
      }
      case ABGRComponentType:
      case RGBAComponentType:
      {
        image->matte=MagickTrue;
        samples_per_pixel=4;
        quantum_type=RGBAQuantum;
        break;
      }
      default:
        break;
    }
    switch (component_type)
    {
      case CbYCrY422ComponentType:
      case CbYACrYA4224ComponentType:
      case CbYCr444ComponentType:
      {
        (void) SetImageColorspace(image,Rec709YCbCrColorspace);
        break;
      }
      case LumaComponentType:
      {
        (void) SetImageColorspace(image,GRAYColorspace);
        break;
      }
      default:
      {
        (void) SetImageColorspace(image,RGBColorspace);
        if (dpx.image.image_element[n].transfer_characteristic == LogarithmicColorimetric)
          (void) SetImageColorspace(image,LogColorspace);
        if (dpx.image.image_element[n].transfer_characteristic == PrintingDensityColorimetric)
          (void) SetImageColorspace(image,LogColorspace);
        break;
      }
    }
    extent=GetBytesPerRow(image->columns,samples_per_pixel,image->depth,
      dpx.image.image_element[n].packing == 0 ? MagickFalse : MagickTrue);
    /*
      DPX any-bit pixel format.
    */
    status=MagickTrue;
    row=0;
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    SetQuantumQuantum(quantum_info,32);
    SetQuantumPack(quantum_info,dpx.image.image_element[n].packing == 0 ?
      MagickTrue : MagickFalse);
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      MagickBooleanType
        sync;

      register PixelPacket
        *q;

      size_t
        length;

      ssize_t
        count,
        offset;

      unsigned char
        *pixels;

      if (status == MagickFalse)
        continue;
      pixels=GetQuantumPixels(quantum_info);
      {
        count=ReadBlob(image,extent,pixels);
        if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
            (image->previous == (Image *) NULL))
          {
            MagickBooleanType
              proceed;

            proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType) row,
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
        offset=row++;
      }
      if (count != (ssize_t) extent)
        status=MagickFalse;
      q=QueueAuthenticPixels(image,0,offset,image->columns,1,exception);
      if (q == (PixelPacket *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      length=ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
        quantum_type,pixels,exception);
      (void) length;
      sync=SyncAuthenticPixels(image,exception);
      if (sync == MagickFalse)
        status=MagickFalse;
    }
    quantum_info=DestroyQuantumInfo(quantum_info);
    if (status == MagickFalse)
      ThrowReaderException(CorruptImageError,"UnableToReadImageData");
    SetQuantumImageType(image,quantum_type);
    if (EOFBlob(image) != MagickFalse)
      ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
        image->filename);
  }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r D P X I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterDPXImage() adds properties for the DPX image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterDPXImage method is:
%
%      size_t RegisterDPXImage(void)
%
*/
ModuleExport size_t RegisterDPXImage(void)
{
  MagickInfo
    *entry;

  static const char
    *DPXNote =
    {
      "Digital Moving Picture Exchange Bitmap, Version 2.0.\n"
      "See SMPTE 268M-2003 specification at http://www.smtpe.org\n"
    };

  entry=SetMagickInfo("DPX");
  entry->decoder=(DecodeImageHandler *) ReadDPXImage;
  entry->encoder=(EncodeImageHandler *) WriteDPXImage;
  entry->magick=(IsImageFormatHandler *) IsDPX;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("SMPTE 268M-2003 (DPX 2.0)");
  entry->note=ConstantString(DPXNote);
  entry->module=ConstantString("DPX");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r D P X I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterDPXImage() removes format registrations made by the
%  DPX module from the list of supported formats.
%
%  The format of the UnregisterDPXImage method is:
%
%      UnregisterDPXImage(void)
%
*/
ModuleExport void UnregisterDPXImage(void)
{
  (void) UnregisterMagickInfo("DPX");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e D P X I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteDPXImage() writes an image in DPX encoded image format.
%
%  The format of the WriteDPXImage method is:
%
%      MagickBooleanType WriteDPXImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static inline const char *GetDPXProperty(const ImageInfo *image_info,
  const Image *image,const char *property)
{
  const char
    *value;

  value=GetImageOption(image_info,property);
  if (value != (const char *) NULL)
    return(value);
  return(GetImageProperty(image,property));
}

static unsigned int StringToTimeCode(const char *key)
{
  char
    buffer[2];

  register ssize_t
    i;

  unsigned int
    shift,
    value;

  value=0;
  shift=28;
  buffer[1]='\0';
  for (i=0; (*key != 0) && (i < 11); i++)
  {
    if (isxdigit((int) ((unsigned char) *key)) == 0)
      {
        key++;
        continue;
      }
    buffer[0]=(*key++);
    value|=(unsigned int) ((strtol(buffer,(char **) NULL,16)) << shift);
    shift-=4;
  }
  return(value);
}

static MagickBooleanType WriteDPXImage(const ImageInfo *image_info,Image *image)
{
  const char
    *value;

  const StringInfo
    *profile;

  DPXInfo
    dpx;

  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  MagickStatusType
    flags;

  GeometryInfo
    geometry_info;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register const PixelPacket
    *p;

  register ssize_t
    i;

  ssize_t
    count,
    horizontal_factor,
    vertical_factor,
    y;

  size_t
    extent;

  time_t
    seconds;

  unsigned char
    *pixels;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  horizontal_factor=4;
  vertical_factor=4;
  if (image_info->sampling_factor != (char *) NULL)
    {
      GeometryInfo
        geometry_info;

      MagickStatusType
        flags;

      flags=ParseGeometry(image_info->sampling_factor,&geometry_info);
      horizontal_factor=(ssize_t) geometry_info.rho;
      vertical_factor=(ssize_t) geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        vertical_factor=horizontal_factor;
      if ((horizontal_factor != 1) && (horizontal_factor != 2) &&
          (horizontal_factor != 4) && (vertical_factor != 1) &&
          (vertical_factor != 2) && (vertical_factor != 4))
        ThrowWriterException(CorruptImageError,"UnexpectedSamplingFactor");
    }
  if ((image->colorspace == YCbCrColorspace) &&
      ((horizontal_factor == 2) || (vertical_factor == 2)))
    if ((image->columns % 2) != 0)
      image->columns++;
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  /*
    Write file header.
  */
  (void) ResetMagickMemory(&dpx,0,sizeof(dpx));
  offset=0;
  dpx.file.magic=0x53445058U;
  offset+=WriteBlobLong(image,dpx.file.magic);
  dpx.file.image_offset=0x2000U;
  profile=GetImageProfile(image,"dpx");
  if (profile != (StringInfo *) NULL)
    {
      if (GetStringInfoLength(profile) > 1048576)
        ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
      dpx.file.image_offset+=(unsigned int) GetStringInfoLength(profile);
      dpx.file.image_offset=(((dpx.file.image_offset+0x2000-1)/0x2000)*0x2000);
    }
  offset+=WriteBlobLong(image,dpx.file.image_offset);
  (void) strncpy(dpx.file.version,"V2.0",sizeof(dpx.file.version));
  offset+=WriteBlob(image,8,(unsigned char *) &dpx.file.version);
  dpx.file.file_size=(unsigned int) (4U*image->columns*image->rows+
    dpx.file.image_offset);
  offset+=WriteBlobLong(image,dpx.file.file_size);
  dpx.file.ditto_key=1U;  /* new frame */
  offset+=WriteBlobLong(image,dpx.file.ditto_key);
  dpx.file.generic_size=0x00000680U;
  offset+=WriteBlobLong(image,dpx.file.generic_size);
  dpx.file.industry_size=0x00000180U;
  offset+=WriteBlobLong(image,dpx.file.industry_size);
  dpx.file.user_size=0;
  if (profile != (StringInfo *) NULL)
    {
      dpx.file.user_size+=(unsigned int) GetStringInfoLength(profile);
      dpx.file.user_size=(((dpx.file.user_size+0x2000-1)/0x2000)*0x2000);
    }
  offset+=WriteBlobLong(image,dpx.file.user_size);
  value=GetDPXProperty(image_info,image,"dpx:file.filename");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.file.filename,value,sizeof(dpx.file.filename));
  offset+=WriteBlob(image,sizeof(dpx.file.filename),(unsigned char *)
    dpx.file.filename);
  seconds=time((time_t *) NULL);
  (void) FormatMagickTime(seconds,sizeof(dpx.file.timestamp),
    dpx.file.timestamp);
  offset+=WriteBlob(image,sizeof(dpx.file.timestamp),(unsigned char *)
    dpx.file.timestamp);
  (void) strncpy(dpx.file.creator,GetMagickVersion((size_t *) NULL),
    sizeof(dpx.file.creator));
  value=GetDPXProperty(image_info,image,"dpx:file.creator");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.file.creator,value,sizeof(dpx.file.creator));
  offset+=WriteBlob(image,sizeof(dpx.file.creator),(unsigned char *)
    dpx.file.creator);
  value=GetDPXProperty(image_info,image,"dpx:file.project");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.file.project,value,sizeof(dpx.file.project));
  offset+=WriteBlob(image,sizeof(dpx.file.project),(unsigned char *)
    dpx.file.project);
  value=GetDPXProperty(image_info,image,"dpx:file.copyright");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.file.copyright,value,
      sizeof(dpx.file.copyright));
  offset+=WriteBlob(image,sizeof(dpx.file.copyright),(unsigned char *)
    dpx.file.copyright);
  dpx.file.encrypt_key=(~0U);
  offset+=WriteBlobLong(image,dpx.file.encrypt_key);
  offset+=WriteBlob(image,sizeof(dpx.file.reserve),(unsigned char *)
    dpx.file.reserve);
  /*
    Write image header.
  */
  switch (image->orientation)
  {
    default:
    case TopLeftOrientation: dpx.image.orientation=0; break;
    case TopRightOrientation: dpx.image.orientation=1; break;
    case BottomLeftOrientation: dpx.image.orientation=2; break;
    case BottomRightOrientation: dpx.image.orientation=3; break;
    case LeftTopOrientation: dpx.image.orientation=4; break;
    case RightTopOrientation: dpx.image.orientation=5; break;
    case LeftBottomOrientation: dpx.image.orientation=6; break;
    case RightBottomOrientation: dpx.image.orientation=7; break;
  }
  offset+=WriteBlobShort(image,dpx.image.orientation);
  dpx.image.number_elements=1;
  offset+=WriteBlobShort(image,dpx.image.number_elements);
  if ((image->columns != (unsigned int) image->columns) ||
      (image->rows != (unsigned int) image->rows))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  offset+=WriteBlobLong(image,(unsigned int) image->columns);
  offset+=WriteBlobLong(image,(unsigned int) image->rows);
  for (i=0; i < 8; i++)
  {
    dpx.image.image_element[i].data_sign=0U;
    offset+=WriteBlobLong(image,dpx.image.image_element[i].data_sign);
    dpx.image.image_element[i].low_data=0U;
    offset+=WriteBlobLong(image,dpx.image.image_element[i].low_data);
    dpx.image.image_element[i].low_quantity=0.0f;
    offset+=WriteBlobFloat(image,dpx.image.image_element[i].low_quantity);
    dpx.image.image_element[i].high_data=0U;
    offset+=WriteBlobLong(image,dpx.image.image_element[i].high_data);
    dpx.image.image_element[i].high_quantity=0.0f;
    offset+=WriteBlobFloat(image,dpx.image.image_element[i].high_quantity);
    dpx.image.image_element[i].descriptor=0;
    if (i == 0)
      switch (image->colorspace)
      {
        case Rec601YCbCrColorspace:
        case Rec709YCbCrColorspace:
        case YCbCrColorspace:
        {
          dpx.image.image_element[i].descriptor=CbYCr444ComponentType;
          if (image->matte != MagickFalse)
            dpx.image.image_element[i].descriptor=CbYCrA4444ComponentType;
          break;
        }
        default:
        {
          dpx.image.image_element[i].descriptor=RGBComponentType;
          if (image->matte != MagickFalse)
            dpx.image.image_element[i].descriptor=RGBAComponentType;
          if ((image_info->type != TrueColorType) &&
              (image->matte == MagickFalse) &&
              (IsGrayImage(image,&image->exception) != MagickFalse))
            dpx.image.image_element[i].descriptor=LumaComponentType;
          break;
        }
      }
    offset+=WriteBlobByte(image,dpx.image.image_element[i].descriptor);
    dpx.image.image_element[i].transfer_characteristic=0;
    if (image->colorspace == LogColorspace)
      dpx.image.image_element[0].transfer_characteristic=
        PrintingDensityColorimetric;
    offset+=WriteBlobByte(image,
      dpx.image.image_element[i].transfer_characteristic);
    dpx.image.image_element[i].colorimetric=0;
    offset+=WriteBlobByte(image,dpx.image.image_element[i].colorimetric);
    dpx.image.image_element[i].bit_size=0;
    if (i == 0)
      dpx.image.image_element[i].bit_size=(unsigned char) image->depth;
    offset+=WriteBlobByte(image,dpx.image.image_element[i].bit_size);
    dpx.image.image_element[i].packing=0;
    if ((image->depth == 10) || (image->depth == 12))
      dpx.image.image_element[i].packing=1;
    offset+=WriteBlobShort(image,dpx.image.image_element[i].packing);
    dpx.image.image_element[i].encoding=0;
    offset+=WriteBlobShort(image,dpx.image.image_element[i].encoding);
    dpx.image.image_element[i].data_offset=0U;
    if (i == 0)
      dpx.image.image_element[i].data_offset=dpx.file.image_offset;
    offset+=WriteBlobLong(image,dpx.image.image_element[i].data_offset);
    dpx.image.image_element[i].end_of_line_padding=0U;
    offset+=WriteBlobLong(image,dpx.image.image_element[i].end_of_line_padding);
    offset+=WriteBlobLong(image,
      dpx.image.image_element[i].end_of_image_padding);
    offset+=WriteBlob(image,sizeof(dpx.image.image_element[i].description),
      (unsigned char *) dpx.image.image_element[i].description);
  }
  offset+=WriteBlob(image,sizeof(dpx.image.reserve),(unsigned char *)
    dpx.image.reserve);
  /*
    Write orientation header.
  */
  if ((image->rows != image->magick_rows) ||
      (image->columns != image->magick_columns))
    {
      /*
        These properties are not valid if image size changed.
      */
      (void) DeleteImageProperty(image,"dpx:orientation.x_offset");
      (void) DeleteImageProperty(image,"dpx:orientation.y_offset");
      (void) DeleteImageProperty(image,"dpx:orientation.x_center");
      (void) DeleteImageProperty(image,"dpx:orientation.y_center");
      (void) DeleteImageProperty(image,"dpx:orientation.x_size");
      (void) DeleteImageProperty(image,"dpx:orientation.y_size");
    }
  dpx.orientation.x_offset=0U;
  value=GetDPXProperty(image_info,image,"dpx:orientation.x_offset");
  if (value != (const char *) NULL)
    dpx.orientation.x_offset=(unsigned int) StringToUnsignedLong(value);
  offset+=WriteBlobLong(image,dpx.orientation.x_offset);
  dpx.orientation.y_offset=0U;
  value=GetDPXProperty(image_info,image,"dpx:orientation.y_offset");
  if (value != (const char *) NULL)
    dpx.orientation.y_offset=(unsigned int) StringToUnsignedLong(value);
  offset+=WriteBlobLong(image,dpx.orientation.y_offset);
  dpx.orientation.x_center=0.0f;
  value=GetDPXProperty(image_info,image,"dpx:orientation.x_center");
  if (value != (const char *) NULL)
    dpx.orientation.x_center=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,dpx.orientation.x_center);
  dpx.orientation.y_center=0.0f;
  value=GetDPXProperty(image_info,image,"dpx:orientation.y_center");
  if (value != (const char *) NULL)
    dpx.orientation.y_center=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,dpx.orientation.y_center);
  dpx.orientation.x_size=0U;
  value=GetDPXProperty(image_info,image,"dpx:orientation.x_size");
  if (value != (const char *) NULL)
    dpx.orientation.x_size=(unsigned int) StringToUnsignedLong(value);
  offset+=WriteBlobLong(image,dpx.orientation.x_size);
  dpx.orientation.y_size=0U;
  value=GetDPXProperty(image_info,image,"dpx:orientation.y_size");
  if (value != (const char *) NULL)
    dpx.orientation.y_size=(unsigned int) StringToUnsignedLong(value);
  offset+=WriteBlobLong(image,dpx.orientation.y_size);
  value=GetDPXProperty(image_info,image,"dpx:orientation.filename");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.orientation.filename,value,
      sizeof(dpx.orientation.filename));
  offset+=WriteBlob(image,sizeof(dpx.orientation.filename),(unsigned char *)
    dpx.orientation.filename);
  offset+=WriteBlob(image,sizeof(dpx.orientation.timestamp),(unsigned char *)
    dpx.orientation.timestamp);
  value=GetDPXProperty(image_info,image,"dpx:orientation.device");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.orientation.device,value,sizeof(dpx.orientation.device));
  offset+=WriteBlob(image,sizeof(dpx.orientation.device),(unsigned char *)
    dpx.orientation.device);
  value=GetDPXProperty(image_info,image,"dpx:orientation.serial");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.orientation.serial,value,sizeof(dpx.orientation.serial));
  offset+=WriteBlob(image,sizeof(dpx.orientation.serial),(unsigned char *)
    dpx.orientation.serial);
  for (i=0; i < 4; i++)
    dpx.orientation.border[i]=0;
  value=GetDPXProperty(image_info,image,"dpx:orientation.border");
  if (value != (const char *) NULL)
    {
      flags=ParseGeometry(value,&geometry_info);
      if ((flags & SigmaValue) == 0)
        geometry_info.sigma=geometry_info.rho;
      dpx.orientation.border[0]=(unsigned short) (geometry_info.rho+0.5);
      dpx.orientation.border[1]=(unsigned short) (geometry_info.sigma+0.5);
      dpx.orientation.border[2]=(unsigned short) (geometry_info.xi+0.5);
      dpx.orientation.border[3]=(unsigned short) (geometry_info.psi+0.5);
    }
  for (i=0; i < 4; i++)
    offset+=WriteBlobShort(image,dpx.orientation.border[i]);
  for (i=0; i < 2; i++)
    dpx.orientation.aspect_ratio[i]=0U;
  value=GetDPXProperty(image_info,image,"dpx:orientation.aspect_ratio");
  if (value != (const char *) NULL)
    {
      flags=ParseGeometry(value,&geometry_info);
      if ((flags & SigmaValue) == 0)
        geometry_info.sigma=geometry_info.rho;
      dpx.orientation.aspect_ratio[0]=(unsigned int) (geometry_info.rho+0.5);
      dpx.orientation.aspect_ratio[1]=(unsigned int) (geometry_info.sigma+0.5);
    }
  for (i=0; i < 2; i++)
    offset+=WriteBlobLong(image,dpx.orientation.aspect_ratio[i]);
  offset+=WriteBlob(image,sizeof(dpx.orientation.reserve),(unsigned char *)
    dpx.orientation.reserve);
  /*
    Write film header.
  */
  *dpx.film.id='\0';
  value=GetDPXProperty(image_info,image,"dpx:film.id");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.film.id,value,sizeof(dpx.film.id));
  offset+=WriteBlob(image,sizeof(dpx.film.id),(unsigned char *) dpx.film.id);
  *dpx.film.type='\0';
  value=GetDPXProperty(image_info,image,"dpx:film.type");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.film.type,value,sizeof(dpx.film.type));
  offset+=WriteBlob(image,sizeof(dpx.film.type),(unsigned char *)
    dpx.film.type);
  *dpx.film.offset='\0';
  value=GetDPXProperty(image_info,image,"dpx:film.offset");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.film.offset,value,sizeof(dpx.film.offset));
  offset+=WriteBlob(image,sizeof(dpx.film.offset),(unsigned char *)
    dpx.film.offset);
  *dpx.film.prefix='\0';
  value=GetDPXProperty(image_info,image,"dpx:film.prefix");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.film.prefix,value,sizeof(dpx.film.prefix));
  offset+=WriteBlob(image,sizeof(dpx.film.prefix),(unsigned char *)
    dpx.film.prefix);
  *dpx.film.count='\0';
  value=GetDPXProperty(image_info,image,"dpx:film.count");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.film.count,value,sizeof(dpx.film.count));
  offset+=WriteBlob(image,sizeof(dpx.film.count),(unsigned char *)
    dpx.film.count);
  *dpx.film.format='\0';
  value=GetDPXProperty(image_info,image,"dpx:film.format");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.film.format,value,sizeof(dpx.film.format));
  offset+=WriteBlob(image,sizeof(dpx.film.format),(unsigned char *)
    dpx.film.format);
  dpx.film.frame_position=0U;
  value=GetDPXProperty(image_info,image,"dpx:film.frame_position");
  if (value != (const char *) NULL)
    dpx.film.frame_position=(unsigned int) StringToUnsignedLong(value);
  offset+=WriteBlobLong(image,dpx.film.frame_position);
  dpx.film.sequence_extent=0U;
  value=GetDPXProperty(image_info,image,"dpx:film.sequence_extent");
  if (value != (const char *) NULL)
    dpx.film.sequence_extent=(unsigned int) StringToUnsignedLong(value);
  offset+=WriteBlobLong(image,dpx.film.sequence_extent);
  dpx.film.held_count=0U;
  value=GetDPXProperty(image_info,image,"dpx:film.held_count");
  if (value != (const char *) NULL)
    dpx.film.held_count=(unsigned int) StringToUnsignedLong(value);
  offset+=WriteBlobLong(image,dpx.film.held_count);
  dpx.film.frame_rate=0.0f;
  value=GetDPXProperty(image_info,image,"dpx:film.frame_rate");
  if (value != (const char *) NULL)
    dpx.film.frame_rate=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,dpx.film.frame_rate);
  dpx.film.shutter_angle=0.0f;
  value=GetDPXProperty(image_info,image,"dpx:film.shutter_angle");
  if (value != (const char *) NULL)
    dpx.film.shutter_angle=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,dpx.film.shutter_angle);
  *dpx.film.frame_id='\0';
  value=GetDPXProperty(image_info,image,"dpx:film.frame_id");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.film.frame_id,value,sizeof(dpx.film.frame_id));
  offset+=WriteBlob(image,sizeof(dpx.film.frame_id),(unsigned char *)
    dpx.film.frame_id);
  value=GetDPXProperty(image_info,image,"dpx:film.slate");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.film.slate,value,sizeof(dpx.film.slate));
  offset+=WriteBlob(image,sizeof(dpx.film.slate),(unsigned char *)
    dpx.film.slate);
  offset+=WriteBlob(image,sizeof(dpx.film.reserve),(unsigned char *)
    dpx.film.reserve);
  /*
    Write television header.
  */
  value=GetDPXProperty(image_info,image,"dpx:television.time.code");
  if (value != (const char *) NULL)
    dpx.television.time_code=StringToTimeCode(value);
  offset+=WriteBlobLong(image,dpx.television.time_code);
  value=GetDPXProperty(image_info,image,"dpx:television.user.bits");
  if (value != (const char *) NULL)
    dpx.television.user_bits=StringToTimeCode(value);
  offset+=WriteBlobLong(image,dpx.television.user_bits);
  value=GetDPXProperty(image_info,image,"dpx:television.interlace");
  if (value != (const char *) NULL)
    dpx.television.interlace=(unsigned char) StringToLong(value);
  offset+=WriteBlobByte(image,dpx.television.interlace);
  value=GetDPXProperty(image_info,image,"dpx:television.field_number");
  if (value != (const char *) NULL)
    dpx.television.field_number=(unsigned char) StringToLong(value);
  offset+=WriteBlobByte(image,dpx.television.field_number);
  dpx.television.video_signal=0;
  value=GetDPXProperty(image_info,image,"dpx:television.video_signal");
  if (value != (const char *) NULL)
    dpx.television.video_signal=(unsigned char) StringToLong(value);
  offset+=WriteBlobByte(image,dpx.television.video_signal);
  dpx.television.padding=0;
  value=GetDPXProperty(image_info,image,"dpx:television.padding");
  if (value != (const char *) NULL)
    dpx.television.padding=(unsigned char) StringToLong(value);
  offset+=WriteBlobByte(image,dpx.television.padding);
  dpx.television.horizontal_sample_rate=0.0f;
  value=GetDPXProperty(image_info,image,
    "dpx:television.horizontal_sample_rate");
  if (value != (const char *) NULL)
    dpx.television.horizontal_sample_rate=StringToDouble(value,
      (char **) NULL);
  offset+=WriteBlobFloat(image,dpx.television.horizontal_sample_rate);
  dpx.television.vertical_sample_rate=0.0f;
  value=GetDPXProperty(image_info,image,"dpx:television.vertical_sample_rate");
  if (value != (const char *) NULL)
    dpx.television.vertical_sample_rate=StringToDouble(value,
      (char **) NULL);
  offset+=WriteBlobFloat(image,dpx.television.vertical_sample_rate);
  dpx.television.frame_rate=0.0f;
  value=GetDPXProperty(image_info,image,"dpx:television.frame_rate");
  if (value != (const char *) NULL)
    dpx.television.frame_rate=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,dpx.television.frame_rate);
  dpx.television.time_offset=0.0f;
  value=GetDPXProperty(image_info,image,"dpx:television.time_offset");
  if (value != (const char *) NULL)
    dpx.television.time_offset=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,dpx.television.time_offset);
  dpx.television.gamma=0.0f;
  value=GetDPXProperty(image_info,image,"dpx:television.gamma");
  if (value != (const char *) NULL)
    dpx.television.gamma=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,dpx.television.gamma);
  dpx.television.black_level=0.0f;
  value=GetDPXProperty(image_info,image,"dpx:television.black_level");
  if (value != (const char *) NULL)
    dpx.television.black_level=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,dpx.television.black_level);
  dpx.television.black_gain=0.0f;
  value=GetDPXProperty(image_info,image,"dpx:television.black_gain");
  if (value != (const char *) NULL)
    dpx.television.black_gain=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,dpx.television.black_gain);
  dpx.television.break_point=0.0f;
  value=GetDPXProperty(image_info,image,"dpx:television.break_point");
  if (value != (const char *) NULL)
    dpx.television.break_point=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,dpx.television.break_point);
  dpx.television.white_level=0.0f;
  value=GetDPXProperty(image_info,image,"dpx:television.white_level");
  if (value != (const char *) NULL)
    dpx.television.white_level=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,dpx.television.white_level);
  dpx.television.integration_times=0.0f;
  value=GetDPXProperty(image_info,image,"dpx:television.integration_times");
  if (value != (const char *) NULL)
    dpx.television.integration_times=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,dpx.television.integration_times);
  offset+=WriteBlob(image,sizeof(dpx.television.reserve),(unsigned char *)
    dpx.television.reserve);
  /*
    Write user header.
  */
  value=GetDPXProperty(image_info,image,"dpx:user.id");
  if (value != (const char *) NULL)
    (void) strncpy(dpx.user.id,value,sizeof(dpx.user.id));
  offset+=WriteBlob(image,sizeof(dpx.user.id),(unsigned char *) dpx.user.id);
  if (profile != (StringInfo *) NULL)
    offset+=WriteBlob(image,GetStringInfoLength(profile),
      GetStringInfoDatum(profile));
  while (offset < (MagickOffsetType) dpx.image.image_element[0].data_offset)
  {
    count=WriteBlobByte(image,0x00);
    if (count != 1)
      {
        ThrowFileException(&image->exception,FileOpenError,"UnableToWriteFile",
          image->filename);
        break;
      }
    offset+=count;
  }
  /*
    Convert pixel packets to DPX raster image.
  */
  quantum_info=AcquireQuantumInfo(image_info,image);
  SetQuantumQuantum(quantum_info,32);
  SetQuantumPack(quantum_info,dpx.image.image_element[0].packing == 0 ?
    MagickTrue : MagickFalse);
  quantum_type=RGBQuantum;
  if (image->matte != MagickFalse)
    quantum_type=RGBAQuantum;
  if (image->colorspace == YCbCrColorspace)
    {
      quantum_type=CbYCrQuantum;
      if (image->matte != MagickFalse)
        quantum_type=CbYCrAQuantum;
      if ((horizontal_factor == 2) || (vertical_factor == 2))
        quantum_type=CbYCrYQuantum;
    }
  extent=GetBytesPerRow(image->columns,image->matte != MagickFalse ? 4UL : 3UL,
    image->depth,MagickTrue);
  if ((image_info->type != TrueColorType) && (image->matte == MagickFalse) &&
      (IsGrayImage(image,&image->exception) != MagickFalse))
    {
      quantum_type=GrayQuantum;
      extent=GetBytesPerRow(image->columns,1UL,image->depth,MagickTrue);
    }
  pixels=GetQuantumPixels(quantum_info);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    (void) ExportQuantumPixels(image,(const CacheView *) NULL,quantum_info,
      quantum_type,pixels,&image->exception);
    count=WriteBlob(image,extent,pixels);
    if (count != (ssize_t) extent)
      break;
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  quantum_info=DestroyQuantumInfo(quantum_info);
  (void) CloseBlob(image);
  return(status);
}

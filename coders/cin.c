/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             CCCC  IIIII  N   N                              %
%                            C        I    NN  N                              %
%                            C        I    N N N                              %
%                            C        I    N  NN                              %
%                             CCCC  IIIII  N   N                              %
%                                                                             %
%                                                                             %
%                    Read/Write Kodak Cineon Image Format                     %
%                Cineon Image Format is a subset of SMTPE CIN                 %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                             Kelly Bergougnoux                               %
%                               October 2003                                  %
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
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Cineon image file format draft is available at
%  http://www.cineon.com/ff_draft.php.
%
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/profile-private.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/timer-private.h"

/*
  Typedef declaration.
*/
typedef struct _CINDataFormatInfo
{
  unsigned char
    interleave,
    packing,
    sign,
    sense;

  size_t
    line_pad,
    channel_pad;

  unsigned char
    reserve[20];
} CINDataFormatInfo;

typedef struct _CINFileInfo
{
  size_t
    magic,
    image_offset,
    generic_length,
    industry_length,
    user_length,
    file_size;

  char
    version[8],
    filename[100],
    create_date[12],
    create_time[12],
    reserve[36];
} CINFileInfo;

typedef struct _CINFilmInfo
{
  char
    id,
    type,
    offset,
    reserve1;

  size_t
    prefix,
    count;

  char
    format[32];

  size_t
    frame_position;

  float
    frame_rate;

  char
    frame_id[32],
    slate_info[200],
    reserve[740];
} CINFilmInfo;

typedef struct _CINImageChannel
{
  unsigned char
    designator[2],
    bits_per_pixel,
    reserve;

  size_t
    pixels_per_line,
    lines_per_image;

  float
    min_data,
    min_quantity,
    max_data,
    max_quantity;
} CINImageChannel;

typedef struct _CINImageInfo
{
  unsigned char
    orientation,
    number_channels,
    reserve1[2];

  CINImageChannel
    channel[8];

  float
    white_point[2],
    red_primary_chromaticity[2],
    green_primary_chromaticity[2],
    blue_primary_chromaticity[2];

  char
    label[200],
    reserve[28];
} CINImageInfo;

typedef struct _CINOriginationInfo
{
  ssize_t
    x_offset,
    y_offset;

  char
    filename[100],
    create_date[12],
    create_time[12],
    device[64],
    model[32],
    serial[32];

  float
    x_pitch,
    y_pitch,
    gamma;

  char
    reserve[40];
} CINOriginationInfo;

typedef struct _CINUserInfo
{
  char
    id[32];
} CINUserInfo;

typedef struct CINInfo
{
  CINFileInfo
    file;

  CINImageInfo
    image;

  CINDataFormatInfo
    data_format;

  CINOriginationInfo
    origination;

  CINFilmInfo
    film;

  CINUserInfo
    user;
} CINInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteCINImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s C I N E O N                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsCIN() returns MagickTrue if the image format type, identified by the magick
%  string, is CIN.
%
%  The format of the IsCIN method is:
%
%      MagickBooleanType IsCIN(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsCIN(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\200\052\137\327",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d C I N E O N I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadCINImage() reads an CIN X image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a point to the
%  new image.
%
%  The format of the ReadCINImage method is:
%
%      Image *ReadCINImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static size_t GetBytesPerRow(size_t columns,
  size_t samples_per_pixel,size_t bits_per_pixel,
  MagickBooleanType pad)
{
  size_t
    bytes_per_row;

  switch (bits_per_pixel)
  {
    case 1:
    {
      bytes_per_row=4*(((size_t) samples_per_pixel*columns*
        bits_per_pixel+31)/32);
      break;
    }
    case 8:
    default:
    {
      bytes_per_row=4*(((size_t) samples_per_pixel*columns*
        bits_per_pixel+31)/32);
      break;
    }
    case 10:
    {
      if (pad == MagickFalse)
        {
          bytes_per_row=4*(((size_t) samples_per_pixel*columns*
            bits_per_pixel+31)/32);
          break;
        }
      bytes_per_row=4*(((size_t) (32*((samples_per_pixel*columns+2)/3))+31)/32);
      break;
    }
    case 12:
    {
      if (pad == MagickFalse)
        {
          bytes_per_row=4*(((size_t) samples_per_pixel*columns*
            bits_per_pixel+31)/32);
          break;
        }
      bytes_per_row=2*(((size_t) (16*samples_per_pixel*columns)+15)/16);
      break;
    }
    case 16:
    {
      bytes_per_row=2*(((size_t) samples_per_pixel*columns*
        bits_per_pixel+8)/16);
      break;
    }
    case 32:
    {
      bytes_per_row=4*(((size_t) samples_per_pixel*columns*
        bits_per_pixel+31)/32);
      break;
    }
    case 64:
    {
      bytes_per_row=8*(((size_t) samples_per_pixel*columns*
        bits_per_pixel+63)/64);
      break;
    }
  }
  return(bytes_per_row);
}

static inline MagickBooleanType IsFloatDefined(const float value)
{
  union
  {
    unsigned int
      unsigned_value;

    double
      float_value;
  } quantum;

  quantum.unsigned_value=0U;
  quantum.float_value=value;
  if (quantum.unsigned_value == 0U)
    return(MagickFalse);
  return(MagickTrue);
}

static Image *ReadCINImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define MonoColorType  1
#define RGBColorType  3

  char
    property[MagickPathExtent];

  CINInfo
    cin;

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

  ssize_t
    i;

  Quantum
    *q;

  size_t
    length;

  ssize_t
    count,
    y;

  unsigned char
    magick[4],
    *pixels;


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
    File information.
  */
  offset=0;
  count=ReadBlob(image,4,magick);
  offset+=count;
  if ((count != 4) ||
      ((LocaleNCompare((char *) magick,"\200\052\137\327",4) != 0)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  memset(&cin,0,sizeof(cin));
  image->endian=(magick[0] == 0x80) && (magick[1] == 0x2a) &&
    (magick[2] == 0x5f) && (magick[3] == 0xd7) ? MSBEndian : LSBEndian;
  cin.file.image_offset=ReadBlobLong(image);
  if (cin.file.image_offset < 712)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  offset+=4;
  cin.file.generic_length=ReadBlobLong(image);
  offset+=4;
  cin.file.industry_length=ReadBlobLong(image);
  offset+=4;
  cin.file.user_length=ReadBlobLong(image);
  offset+=4;
  cin.file.file_size=ReadBlobLong(image);
  offset+=4;
  offset+=ReadBlob(image,sizeof(cin.file.version),(unsigned char *)
    cin.file.version);
  (void) CopyMagickString(property,cin.file.version,sizeof(cin.file.version));
  (void) SetImageProperty(image,"dpx:file.version",property,exception);
  offset+=ReadBlob(image,sizeof(cin.file.filename),(unsigned char *)
    cin.file.filename);
  (void) CopyMagickString(property,cin.file.filename,sizeof(cin.file.filename));
  (void) SetImageProperty(image,"dpx:file.filename",property,exception);
  offset+=ReadBlob(image,sizeof(cin.file.create_date),(unsigned char *)
    cin.file.create_date);
  (void) CopyMagickString(property,cin.file.create_date,
    sizeof(cin.file.create_date));
  (void) SetImageProperty(image,"dpx:file.create_date",property,exception);
  offset+=ReadBlob(image,sizeof(cin.file.create_time),(unsigned char *)
    cin.file.create_time);
  (void) CopyMagickString(property,cin.file.create_time,
    sizeof(cin.file.create_time));
  (void) SetImageProperty(image,"dpx:file.create_time",property,exception);
  offset+=ReadBlob(image,sizeof(cin.file.reserve),(unsigned char *)
    cin.file.reserve);
  /*
    Image information.
  */
  cin.image.orientation=(unsigned char) ReadBlobByte(image);
  offset++;
  if (cin.image.orientation != (unsigned char) (~0))
    (void) FormatImageProperty(image,"dpx:image.orientation","%d",
      cin.image.orientation);
  switch (cin.image.orientation)
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
  cin.image.number_channels=(unsigned char) ReadBlobByte(image);
  offset++;
  offset+=ReadBlob(image,sizeof(cin.image.reserve1),(unsigned char *)
    cin.image.reserve1);
  for (i=0; i < 8; i++)
  {
    cin.image.channel[i].designator[0]=(unsigned char) ReadBlobByte(image);
    offset++;
    cin.image.channel[i].designator[1]=(unsigned char) ReadBlobByte(image);
    offset++;
    cin.image.channel[i].bits_per_pixel=(unsigned char) ReadBlobByte(image);
    offset++;
    cin.image.channel[i].reserve=(unsigned char) ReadBlobByte(image);
    offset++;
    cin.image.channel[i].pixels_per_line=ReadBlobLong(image);
    offset+=4;
    cin.image.channel[i].lines_per_image=ReadBlobLong(image);
    offset+=4;
    cin.image.channel[i].min_data=ReadBlobFloat(image);
    offset+=4;
    cin.image.channel[i].min_quantity=ReadBlobFloat(image);
    offset+=4;
    cin.image.channel[i].max_data=ReadBlobFloat(image);
    offset+=4;
    cin.image.channel[i].max_quantity=ReadBlobFloat(image);
    offset+=4;
  }
  cin.image.white_point[0]=ReadBlobFloat(image);
  offset+=4;
  if (IsFloatDefined(cin.image.white_point[0]) != MagickFalse)
    image->chromaticity.white_point.x=cin.image.white_point[0];
  cin.image.white_point[1]=ReadBlobFloat(image);
  offset+=4;
  if (IsFloatDefined(cin.image.white_point[1]) != MagickFalse)
    image->chromaticity.white_point.y=cin.image.white_point[1];
  cin.image.red_primary_chromaticity[0]=ReadBlobFloat(image);
  offset+=4;
  if (IsFloatDefined(cin.image.red_primary_chromaticity[0]) != MagickFalse)
    image->chromaticity.red_primary.x=cin.image.red_primary_chromaticity[0];
  cin.image.red_primary_chromaticity[1]=ReadBlobFloat(image);
  offset+=4;
  if (IsFloatDefined(cin.image.red_primary_chromaticity[1]) != MagickFalse)
    image->chromaticity.red_primary.y=cin.image.red_primary_chromaticity[1];
  cin.image.green_primary_chromaticity[0]=ReadBlobFloat(image);
  offset+=4;
  if (IsFloatDefined(cin.image.green_primary_chromaticity[0]) != MagickFalse)
    image->chromaticity.red_primary.x=cin.image.green_primary_chromaticity[0];
  cin.image.green_primary_chromaticity[1]=ReadBlobFloat(image);
  offset+=4;
  if (IsFloatDefined(cin.image.green_primary_chromaticity[1]) != MagickFalse)
    image->chromaticity.green_primary.y=cin.image.green_primary_chromaticity[1];
  cin.image.blue_primary_chromaticity[0]=ReadBlobFloat(image);
  offset+=4;
  if (IsFloatDefined(cin.image.blue_primary_chromaticity[0]) != MagickFalse)
    image->chromaticity.blue_primary.x=cin.image.blue_primary_chromaticity[0];
  cin.image.blue_primary_chromaticity[1]=ReadBlobFloat(image);
  offset+=4;
  if (IsFloatDefined(cin.image.blue_primary_chromaticity[1]) != MagickFalse)
    image->chromaticity.blue_primary.y=cin.image.blue_primary_chromaticity[1];
  offset+=ReadBlob(image,sizeof(cin.image.label),(unsigned char *)
    cin.image.label);
  (void) CopyMagickString(property,cin.image.label,sizeof(cin.image.label));
  (void) SetImageProperty(image,"dpx:image.label",property,exception);
  offset+=ReadBlob(image,sizeof(cin.image.reserve),(unsigned char *)
    cin.image.reserve);
  /*
    Image data format information.
  */
  cin.data_format.interleave=(unsigned char) ReadBlobByte(image);
  offset++;
  cin.data_format.packing=(unsigned char) ReadBlobByte(image);
  offset++;
  cin.data_format.sign=(unsigned char) ReadBlobByte(image);
  offset++;
  cin.data_format.sense=(unsigned char) ReadBlobByte(image);
  offset++;
  cin.data_format.line_pad=ReadBlobLong(image);
  offset+=4;
  cin.data_format.channel_pad=ReadBlobLong(image);
  offset+=4;
  offset+=ReadBlob(image,sizeof(cin.data_format.reserve),(unsigned char *)
    cin.data_format.reserve);
  /*
    Image origination information.
  */
  cin.origination.x_offset=ReadBlobSignedLong(image);
  offset+=4;
  if ((size_t) cin.origination.x_offset != ~0UL)
    (void) FormatImageProperty(image,"dpx:origination.x_offset","%.20g",
      (double) cin.origination.x_offset);
  cin.origination.y_offset=(ssize_t) ReadBlobLong(image);
  offset+=4;
  if ((size_t) cin.origination.y_offset != ~0UL)
    (void) FormatImageProperty(image,"dpx:origination.y_offset","%.20g",
      (double) cin.origination.y_offset);
  offset+=ReadBlob(image,sizeof(cin.origination.filename),(unsigned char *)
    cin.origination.filename);
  (void) CopyMagickString(property,cin.origination.filename,
    sizeof(cin.origination.filename));
  (void) SetImageProperty(image,"dpx:origination.filename",property,exception);
  offset+=ReadBlob(image,sizeof(cin.origination.create_date),(unsigned char *)
    cin.origination.create_date);
  (void) CopyMagickString(property,cin.origination.create_date,
    sizeof(cin.origination.create_date));
  (void) SetImageProperty(image,"dpx:origination.create_date",property,
    exception);
  offset+=ReadBlob(image,sizeof(cin.origination.create_time),(unsigned char *)
    cin.origination.create_time);
  (void) CopyMagickString(property,cin.origination.create_time,
    sizeof(cin.origination.create_time));
  (void) SetImageProperty(image,"dpx:origination.create_time",property,
    exception);
  offset+=ReadBlob(image,sizeof(cin.origination.device),(unsigned char *)
    cin.origination.device);
  (void) CopyMagickString(property,cin.origination.device,
    sizeof(cin.origination.device));
  (void) SetImageProperty(image,"dpx:origination.device",property,exception);
  offset+=ReadBlob(image,sizeof(cin.origination.model),(unsigned char *)
    cin.origination.model);
  (void) CopyMagickString(property,cin.origination.model,
    sizeof(cin.origination.model));
  (void) SetImageProperty(image,"dpx:origination.model",property,exception);
  (void) memset(cin.origination.serial,0,
    sizeof(cin.origination.serial));
  offset+=ReadBlob(image,sizeof(cin.origination.serial),(unsigned char *)
    cin.origination.serial);
  (void) CopyMagickString(property,cin.origination.serial,
    sizeof(cin.origination.serial));
  (void) SetImageProperty(image,"dpx:origination.serial",property,exception);
  cin.origination.x_pitch=ReadBlobFloat(image);
  offset+=4;
  cin.origination.y_pitch=ReadBlobFloat(image);
  offset+=4;
  cin.origination.gamma=ReadBlobFloat(image);
  offset+=4;
  if (IsFloatDefined(cin.origination.gamma) != MagickFalse)
    image->gamma=cin.origination.gamma;
  offset+=ReadBlob(image,sizeof(cin.origination.reserve),(unsigned char *)
    cin.origination.reserve);
  if ((cin.file.image_offset > 2048) && (cin.file.user_length != 0))
    {
      int
        c;

      /*
        Image film information.
      */
      cin.film.id=(char) ReadBlobByte(image);
      offset++;
      c=cin.film.id;
      if (c != ~0)
        (void) FormatImageProperty(image,"dpx:film.id","%d",cin.film.id);
      cin.film.type=(char) ReadBlobByte(image);
      offset++;
      c=cin.film.type;
      if (c != ~0)
        (void) FormatImageProperty(image,"dpx:film.type","%d",cin.film.type);
      cin.film.offset=(char) ReadBlobByte(image);
      offset++;
      c=cin.film.offset;
      if (c != ~0)
        (void) FormatImageProperty(image,"dpx:film.offset","%d",
          cin.film.offset);
      cin.film.reserve1=(char) ReadBlobByte(image);
      offset++;
      cin.film.prefix=ReadBlobLong(image);
      offset+=4;
      if (cin.film.prefix != ~0UL)
        (void) FormatImageProperty(image,"dpx:film.prefix","%.20g",(double)
          cin.film.prefix);
      cin.film.count=ReadBlobLong(image);
      offset+=4;
      offset+=ReadBlob(image,sizeof(cin.film.format),(unsigned char *)
        cin.film.format);
      (void) CopyMagickString(property,cin.film.format,sizeof(cin.film.format));
      (void) SetImageProperty(image,"dpx:film.format",property,exception);
      cin.film.frame_position=ReadBlobLong(image);
      offset+=4;
      if (cin.film.frame_position != ~0UL)
        (void) FormatImageProperty(image,"dpx:film.frame_position","%.20g",
          (double) cin.film.frame_position);
      cin.film.frame_rate=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(cin.film.frame_rate) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:film.frame_rate","%g",
          (double) cin.film.frame_rate);
      offset+=ReadBlob(image,sizeof(cin.film.frame_id),(unsigned char *)
        cin.film.frame_id);
      (void) CopyMagickString(property,cin.film.frame_id,
        sizeof(cin.film.frame_id));
      (void) SetImageProperty(image,"dpx:film.frame_id",property,exception);
      offset+=ReadBlob(image,sizeof(cin.film.slate_info),(unsigned char *)
        cin.film.slate_info);
      (void) CopyMagickString(property,cin.film.slate_info,
        sizeof(cin.film.slate_info));
      (void) SetImageProperty(image,"dpx:film.slate_info",property,exception);
      offset+=ReadBlob(image,sizeof(cin.film.reserve),(unsigned char *)
        cin.film.reserve);
    }
  if ((cin.file.image_offset > 2048) && (cin.file.user_length != 0))
    {
      StringInfo
        *profile;

      /*
        User defined data.
      */
      if (cin.file.user_length > GetBlobSize(image))
        ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
      profile=AcquireProfileStringInfo("dpx:user.data",cin.file.user_length,
        exception);
      if (profile == (StringInfo *) NULL)
        offset=SeekBlob(image,(MagickOffsetType) cin.file.user_length,SEEK_CUR);
      else
        {
          offset+=ReadBlob(image,cin.file.user_length,
            GetStringInfoDatum(profile));
          (void) SetImageProfilePrivate(image,profile,exception);
        }
    }
  image->depth=cin.image.channel[0].bits_per_pixel;
  image->columns=cin.image.channel[0].pixels_per_line;
  image->rows=cin.image.channel[0].lines_per_image;
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(image);
    }
  if (((MagickSizeType) image->columns*image->rows/8) > GetBlobSize(image))
    ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
  for ( ; offset < (MagickOffsetType) cin.file.image_offset; offset++)
  {
    int
      c;

    c=ReadBlobByte(image);
    if (c == EOF)
      break;
  }
  if (offset < (MagickOffsetType) cin.file.image_offset)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  (void) SetImageBackgroundColor(image,exception);
  /*
    Convert CIN raster image to pixel packets.
  */
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  SetQuantumQuantum(quantum_info,32);
  SetQuantumPack(quantum_info,MagickFalse);
  quantum_type=RGBQuantum;
  length=GetBytesPerRow(image->columns,3,image->depth,MagickTrue);
  if (cin.image.number_channels == 1)
    {
      quantum_type=GrayQuantum;
      length=GetBytesPerRow(image->columns,1,image->depth,MagickTrue);
    }
  pixels=GetQuantumPixels(quantum_info);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const void
      *stream;

    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    stream=ReadBlobStream(image,length,pixels,&count);
    if ((size_t) count != length)
      break;
    (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      quantum_type,(unsigned char *) stream,exception);
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
  SetQuantumImageType(image,quantum_type);
  quantum_info=DestroyQuantumInfo(quantum_info);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  SetImageColorspace(image,LogColorspace,exception);
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
%   R e g i s t e r C I N E O N I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterCINImage() adds attributes for the CIN image format to the list of
%  of supported formats.  The attributes include the image format tag, a method
%  to read and/or write the format, whether the format supports the saving of
%  more than one frame to the same file or blob, whether the format supports
%  native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterCINImage method is:
%
%      size_t RegisterCINImage(void)
%
*/
ModuleExport size_t RegisterCINImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("CIN","CIN","Cineon Image File");
  entry->decoder=(DecodeImageHandler *) ReadCINImage;
  entry->encoder=(EncodeImageHandler *) WriteCINImage;
  entry->magick=(IsImageFormatHandler *) IsCIN;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r C I N E O N I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterCINImage() removes format registrations made by the CIN module
%  from the list of supported formats.
%
%  The format of the UnregisterCINImage method is:
%
%      UnregisterCINImage(void)
%
*/
ModuleExport void UnregisterCINImage(void)
{
  (void) UnregisterMagickInfo("CINEON");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e C I N E O N I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteCINImage() writes an image in CIN encoded image format.
%
%  The format of the WriteCINImage method is:
%
%      MagickBooleanType WriteCINImage(const ImageInfo *image_info,
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

static inline const char *GetCINProperty(const ImageInfo *image_info,
  const Image *image,const char *property,ExceptionInfo *exception)
{
  const char
    *value;

  value=GetImageOption(image_info,property);
  if (value != (const char *) NULL)
    return(value);
  return(GetImageProperty(image,property,exception));
}

static MagickBooleanType WriteCINImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  char
    timestamp[MagickPathExtent];

  const char
    *value;

  CINInfo
    cin;

  const StringInfo
    *profile;

  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  const Quantum
    *p;

  ssize_t
    i;

  size_t
    length;

  ssize_t
    count,
    y;

  struct tm
    utc_time;

  time_t
    seconds;

  unsigned char
    *pixels;

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
  if (image->colorspace != LogColorspace)
    (void) TransformImageColorspace(image,LogColorspace,exception);
  /*
    Write image information.
  */
  (void) memset(&cin,0,sizeof(cin));
  offset=0;
  cin.file.magic=0x802A5FD7UL;
  offset+=WriteBlobLong(image,(unsigned int) cin.file.magic);
  cin.file.image_offset=0x800;
  offset+=WriteBlobLong(image,(unsigned int) cin.file.image_offset);
  cin.file.generic_length=0x400;
  offset+=WriteBlobLong(image,(unsigned int) cin.file.generic_length);
  cin.file.industry_length=0x400;
  offset+=WriteBlobLong(image,(unsigned int) cin.file.industry_length);
  cin.file.user_length=0x00;
  profile=GetImageProfile(image,"dpx:user.data");
  if (profile != (StringInfo *) NULL)
    {
      cin.file.user_length+=(size_t) GetStringInfoLength(profile);
      cin.file.user_length=(((cin.file.user_length+0x2000-1)/0x2000)*0x2000);
    }
  offset+=WriteBlobLong(image,(unsigned int) cin.file.user_length);
  cin.file.file_size=4*image->columns*image->rows+0x2000;
  offset+=WriteBlobLong(image,(unsigned int) cin.file.file_size);
  (void) CopyMagickString(cin.file.version,"V4.5",sizeof(cin.file.version));
  offset+=WriteBlob(image,sizeof(cin.file.version),(unsigned char *)
    cin.file.version);
  value=GetCINProperty(image_info,image,"dpx:file.filename",exception);
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.file.filename,value,sizeof(cin.file.filename));
  else
    (void) CopyMagickString(cin.file.filename,image->filename,
      sizeof(cin.file.filename));
  offset+=WriteBlob(image,sizeof(cin.file.filename),(unsigned char *)
    cin.file.filename);
  seconds=GetMagickTime();
  GetMagickUTCTime(&seconds,&utc_time);
  (void) memset(timestamp,0,sizeof(timestamp));
  (void) strftime(timestamp,MagickPathExtent,"%Y:%m:%d:%H:%M:%SUTC",&utc_time);
  (void) memset(cin.file.create_date,0,sizeof(cin.file.create_date));
  (void) CopyMagickString(cin.file.create_date,timestamp,11);
  offset+=WriteBlob(image,sizeof(cin.file.create_date),(unsigned char *)
    cin.file.create_date);
  (void) memset(cin.file.create_time,0,sizeof(cin.file.create_time));
  (void) CopyMagickString(cin.file.create_time,timestamp+11,11);
  offset+=WriteBlob(image,sizeof(cin.file.create_time),(unsigned char *)
    cin.file.create_time);
  offset+=WriteBlob(image,sizeof(cin.file.reserve),(unsigned char *)
    cin.file.reserve);
  cin.image.orientation=0x00;
  offset+=WriteBlobByte(image,cin.image.orientation);
  cin.image.number_channels=3;
  offset+=WriteBlobByte(image,cin.image.number_channels);
  offset+=WriteBlob(image,sizeof(cin.image.reserve1),(unsigned char *)
    cin.image.reserve1);
  for (i=0; i < 8; i++)
  {
    cin.image.channel[i].designator[0]=0; /* universal metric */
    offset+=WriteBlobByte(image,cin.image.channel[0].designator[0]);
    cin.image.channel[i].designator[1]=(unsigned char) (i > 3 ? 0 : i+1); /* channel color */;
    offset+=WriteBlobByte(image,cin.image.channel[1].designator[0]);
    cin.image.channel[i].bits_per_pixel=(unsigned char) image->depth;
    offset+=WriteBlobByte(image,cin.image.channel[0].bits_per_pixel);
    offset+=WriteBlobByte(image,cin.image.channel[0].reserve);
    cin.image.channel[i].pixels_per_line=image->columns;
    offset+=WriteBlobLong(image,(unsigned int)
      cin.image.channel[0].pixels_per_line);
    cin.image.channel[i].lines_per_image=image->rows;
    offset+=WriteBlobLong(image,(unsigned int)
      cin.image.channel[0].lines_per_image);
    cin.image.channel[i].min_data=0;
    offset+=WriteBlobFloat(image,cin.image.channel[0].min_data);
    cin.image.channel[i].min_quantity=0.0;
    offset+=WriteBlobFloat(image,cin.image.channel[0].min_quantity);
    cin.image.channel[i].max_data=(float) ((MagickOffsetType)
      GetQuantumRange(image->depth));
    offset+=WriteBlobFloat(image,cin.image.channel[0].max_data);
    cin.image.channel[i].max_quantity=2.048f;
    offset+=WriteBlobFloat(image,cin.image.channel[0].max_quantity);
  }
  offset+=WriteBlobFloat(image,(float) image->chromaticity.white_point.x);
  offset+=WriteBlobFloat(image,(float) image->chromaticity.white_point.y);
  offset+=WriteBlobFloat(image,(float) image->chromaticity.red_primary.x);
  offset+=WriteBlobFloat(image,(float) image->chromaticity.red_primary.y);
  offset+=WriteBlobFloat(image,(float) image->chromaticity.green_primary.x);
  offset+=WriteBlobFloat(image,(float) image->chromaticity.green_primary.y);
  offset+=WriteBlobFloat(image,(float) image->chromaticity.blue_primary.x);
  offset+=WriteBlobFloat(image,(float) image->chromaticity.blue_primary.y);
  value=GetCINProperty(image_info,image,"dpx:image.label",exception);
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.image.label,value,sizeof(cin.image.label));
  offset+=WriteBlob(image,sizeof(cin.image.label),(unsigned char *)
    cin.image.label);
  offset+=WriteBlob(image,sizeof(cin.image.reserve),(unsigned char *)
    cin.image.reserve);
  /*
    Write data format information.
  */
  cin.data_format.interleave=0; /* pixel interleave (rgbrgbr...) */
  offset+=WriteBlobByte(image,cin.data_format.interleave);
  cin.data_format.packing=5; /* packing ssize_tword (32bit) boundaries */
  offset+=WriteBlobByte(image,cin.data_format.packing);
  cin.data_format.sign=0; /* unsigned data */
  offset+=WriteBlobByte(image,cin.data_format.sign);
  cin.data_format.sense=0; /* image sense: positive image */
  offset+=WriteBlobByte(image,cin.data_format.sense);
  cin.data_format.line_pad=0;
  offset+=WriteBlobLong(image,(unsigned int) cin.data_format.line_pad);
  cin.data_format.channel_pad=0;
  offset+=WriteBlobLong(image,(unsigned int) cin.data_format.channel_pad);
  offset+=WriteBlob(image,sizeof(cin.data_format.reserve),(unsigned char *)
    cin.data_format.reserve);
  /*
    Write origination information.
  */
  cin.origination.x_offset=0UL;
  value=GetCINProperty(image_info,image,"dpx:origination.x_offset",exception);
  if (value != (const char *) NULL)
    cin.origination.x_offset=(ssize_t) StringToLong(value);
  offset+=WriteBlobLong(image,(unsigned int) cin.origination.x_offset);
  cin.origination.y_offset=0UL;
  value=GetCINProperty(image_info,image,"dpx:origination.y_offset",exception);
  if (value != (const char *) NULL)
    cin.origination.y_offset=(ssize_t) StringToLong(value);
  offset+=WriteBlobLong(image,(unsigned int) cin.origination.y_offset);
  value=GetCINProperty(image_info,image,"dpx:origination.filename",exception);
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.origination.filename,value,
      sizeof(cin.origination.filename));
  else
    (void) CopyMagickString(cin.origination.filename,image->filename,
      sizeof(cin.origination.filename));
  offset+=WriteBlob(image,sizeof(cin.origination.filename),(unsigned char *)
    cin.origination.filename);
  (void) memset(timestamp,0,sizeof(timestamp));
  (void) strftime(timestamp,MagickPathExtent,"%Y:%m:%d:%H:%M:%SUTC",&utc_time);
  (void) memset(cin.origination.create_date,0,
    sizeof(cin.origination.create_date));
  (void) CopyMagickString(cin.origination.create_date,timestamp,11);
  offset+=WriteBlob(image,sizeof(cin.origination.create_date),(unsigned char *)
    cin.origination.create_date);
  (void) memset(cin.origination.create_time,0,
     sizeof(cin.origination.create_time));
  (void) CopyMagickString(cin.origination.create_time,timestamp+11,15);
  offset+=WriteBlob(image,sizeof(cin.origination.create_time),(unsigned char *)
    cin.origination.create_time);
  value=GetCINProperty(image_info,image,"dpx:origination.device",exception);
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.origination.device,value,
      sizeof(cin.origination.device));
  offset+=WriteBlob(image,sizeof(cin.origination.device),(unsigned char *)
    cin.origination.device);
  value=GetCINProperty(image_info,image,"dpx:origination.model",exception);
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.origination.model,value,
      sizeof(cin.origination.model));
  offset+=WriteBlob(image,sizeof(cin.origination.model),(unsigned char *)
    cin.origination.model);
  value=GetCINProperty(image_info,image,"dpx:origination.serial",exception);
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.origination.serial,value,
      sizeof(cin.origination.serial));
  offset+=WriteBlob(image,sizeof(cin.origination.serial),(unsigned char *)
    cin.origination.serial);
  cin.origination.x_pitch=0.0f;
  value=GetCINProperty(image_info,image,"dpx:origination.x_pitch",exception);
  if (value != (const char *) NULL)
    cin.origination.x_pitch=StringToFloat(value,(char **) NULL);
  offset+=WriteBlobFloat(image,cin.origination.x_pitch);
  cin.origination.y_pitch=0.0f;
  value=GetCINProperty(image_info,image,"dpx:origination.y_pitch",exception);
  if (value != (const char *) NULL)
    cin.origination.y_pitch=StringToFloat(value,(char **) NULL);
  offset+=WriteBlobFloat(image,cin.origination.y_pitch);
  cin.origination.gamma=(float) image->gamma;
  offset+=WriteBlobFloat(image,cin.origination.gamma);
  offset+=WriteBlob(image,sizeof(cin.origination.reserve),(unsigned char *)
    cin.origination.reserve);
  /*
    Image film information.
  */
  cin.film.id=0;
  value=GetCINProperty(image_info,image,"dpx:film.id",exception);
  if (value != (const char *) NULL)
    cin.film.id=(char) StringToLong(value);
  offset+=WriteBlobByte(image,(unsigned char) cin.film.id);
  cin.film.type=0;
  value=GetCINProperty(image_info,image,"dpx:film.type",exception);
  if (value != (const char *) NULL)
    cin.film.type=(char) StringToLong(value);
  offset+=WriteBlobByte(image,(unsigned char) cin.film.type);
  cin.film.offset=0;
  value=GetCINProperty(image_info,image,"dpx:film.offset",exception);
  if (value != (const char *) NULL)
    cin.film.offset=(char) StringToLong(value);
  offset+=WriteBlobByte(image,(unsigned char) cin.film.offset);
  offset+=WriteBlobByte(image,(unsigned char) cin.film.reserve1);
  cin.film.prefix=0UL;
  value=GetCINProperty(image_info,image,"dpx:film.prefix",exception);
  if (value != (const char *) NULL)
    cin.film.prefix=StringToUnsignedLong(value);
  offset+=WriteBlobLong(image,(unsigned int) cin.film.prefix);
  cin.film.count=0UL;
  value=GetCINProperty(image_info,image,"dpx:film.count",exception);
  if (value != (const char *) NULL)
    cin.film.count=StringToUnsignedLong(value);
  offset+=WriteBlobLong(image,(unsigned int) cin.film.count);
  value=GetCINProperty(image_info,image,"dpx:film.format",exception);
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.film.format,value,sizeof(cin.film.format));
  offset+=WriteBlob(image,sizeof(cin.film.format),(unsigned char *)
    cin.film.format);
  cin.film.frame_position=0UL;
  value=GetCINProperty(image_info,image,"dpx:film.frame_position",exception);
  if (value != (const char *) NULL)
    cin.film.frame_position=StringToUnsignedLong(value);
  offset+=WriteBlobLong(image,(unsigned int) cin.film.frame_position);
  cin.film.frame_rate=0.0f;
  value=GetCINProperty(image_info,image,"dpx:film.frame_rate",exception);
  if (value != (const char *) NULL)
    cin.film.frame_rate=StringToFloat(value,(char **) NULL);
  offset+=WriteBlobFloat(image,cin.film.frame_rate);
  value=GetCINProperty(image_info,image,"dpx:film.frame_id",exception);
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.film.frame_id,value,sizeof(cin.film.frame_id));
  offset+=WriteBlob(image,sizeof(cin.film.frame_id),(unsigned char *)
    cin.film.frame_id);
  value=GetCINProperty(image_info,image,"dpx:film.slate_info",exception);
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.film.slate_info,value,
      sizeof(cin.film.slate_info));
  offset+=WriteBlob(image,sizeof(cin.film.slate_info),(unsigned char *)
    cin.film.slate_info);
  offset+=WriteBlob(image,sizeof(cin.film.reserve),(unsigned char *)
    cin.film.reserve);
  if (profile != (StringInfo *) NULL)
    offset+=WriteBlob(image,GetStringInfoLength(profile),
      GetStringInfoDatum(profile));
  while (offset < (MagickOffsetType) cin.file.image_offset)
    offset+=WriteBlobByte(image,0x00);
  /*
    Convert pixel packets to CIN raster image.
  */
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  SetQuantumQuantum(quantum_info,32);
  SetQuantumPack(quantum_info,MagickFalse);
  quantum_type=RGBQuantum;
  pixels=(unsigned char *) GetQuantumPixels(quantum_info);
  length=GetBytesPerRow(image->columns,3,image->depth,MagickTrue);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    (void) ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      quantum_type,pixels,exception);
    count=WriteBlob(image,length,pixels);
    if (count != (ssize_t) length)
      break;
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  quantum_info=DestroyQuantumInfo(quantum_info);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

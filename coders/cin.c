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
#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/colorspace.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/module.h"


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
  Forward declaractions.
*/
static MagickBooleanType
  WriteCINImage(const ImageInfo *,Image *);


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

  register ssize_t
    i;

  register PixelPacket
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
    File information.
  */
  offset=0;
  count=ReadBlob(image,4,magick);
  offset+=count;
  if ((count != 4) ||
      ((LocaleNCompare((char *) magick,"\200\052\137\327",4) != 0)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  image->endian=(magick[0] == 0x80) && (magick[1] == 0x2a) &&
    (magick[2] == 0x5f) && (magick[3] == 0xd7) ? MSBEndian : LSBEndian;
  cin.file.image_offset=ReadBlobLong(image);
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
  (void) SetImageProperty(image,"dpx:file.version",cin.file.version);
  offset+=ReadBlob(image,sizeof(cin.file.filename),(unsigned char *)
    cin.file.filename);
  (void) SetImageProperty(image,"dpx:file.filename",cin.file.filename);
  offset+=ReadBlob(image,sizeof(cin.file.create_date),(unsigned char *)
    cin.file.create_date);
  (void) SetImageProperty(image,"dpx:file.create_date",cin.file.create_date);
  offset+=ReadBlob(image,sizeof(cin.file.create_time),(unsigned char *)
    cin.file.create_time);
  (void) SetImageProperty(image,"dpx:file.create_time",cin.file.create_time);
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
  (void) SetImageProperty(image,"dpx:image.label",cin.image.label);
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
  cin.origination.x_offset=(int) ReadBlobLong(image);
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
  (void) SetImageProperty(image,"dpx:origination.filename",
    cin.origination.filename);
  offset+=ReadBlob(image,sizeof(cin.origination.create_date),(unsigned char *)
    cin.origination.create_date);
  (void) SetImageProperty(image,"dpx:origination.create_date",
    cin.origination.create_date);
  offset+=ReadBlob(image,sizeof(cin.origination.create_time),(unsigned char *)
    cin.origination.create_time);
  (void) SetImageProperty(image,"dpx:origination.create_time",
    cin.origination.create_time);
  offset+=ReadBlob(image,sizeof(cin.origination.device),(unsigned char *)
    cin.origination.device);
  (void) SetImageProperty(image,"dpx:origination.device",
    cin.origination.device);
  offset+=ReadBlob(image,sizeof(cin.origination.model),(unsigned char *)
    cin.origination.model);
  (void) SetImageProperty(image,"dpx:origination.model",cin.origination.model);
  offset+=ReadBlob(image,sizeof(cin.origination.serial),(unsigned char *)
    cin.origination.serial);
  (void) SetImageProperty(image,"dpx:origination.serial",
    cin.origination.serial);
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
      cin.film.id=ReadBlobByte(image);
      offset++;
      c=cin.film.id;
      if (c != ~0)
        (void) FormatImageProperty(image,"dpx:film.id","%d",cin.film.id);
      cin.film.type=ReadBlobByte(image);
      offset++;
      c=cin.film.type;
      if (c != ~0)
        (void) FormatImageProperty(image,"dpx:film.type","%d",cin.film.type);
      cin.film.offset=ReadBlobByte(image);
      offset++;
      c=cin.film.offset;
      if (c != ~0)
        (void) FormatImageProperty(image,"dpx:film.offset","%d",
          cin.film.offset);
      cin.film.reserve1=ReadBlobByte(image);
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
      (void) SetImageProperty(image,"dpx:film.format",cin.film.format);
      cin.film.frame_position=ReadBlobLong(image);
      offset+=4;
      if (cin.film.frame_position != ~0UL)
        (void) FormatImageProperty(image,"dpx:film.frame_position","%.20g",
          (double) cin.film.frame_position);
      cin.film.frame_rate=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(cin.film.frame_rate) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:film.frame_rate","%g",
          cin.film.frame_rate);
      offset+=ReadBlob(image,sizeof(cin.film.frame_id),(unsigned char *)
        cin.film.frame_id);
      (void) SetImageProperty(image,"dpx:film.frame_id",cin.film.frame_id);
      offset+=ReadBlob(image,sizeof(cin.film.slate_info),(unsigned char *)
        cin.film.slate_info);
      (void) SetImageProperty(image,"dpx:film.slate_info",cin.film.slate_info);
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
      profile=BlobToStringInfo((const void *) NULL,cin.file.user_length);
      if (profile == (StringInfo *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      offset+=ReadBlob(image,GetStringInfoLength(profile),
        GetStringInfoDatum(profile));
      (void) SetImageProfile(image,"dpx:user.data",profile);
      profile=DestroyStringInfo(profile);
    }
  for ( ; offset < (MagickOffsetType) cin.file.image_offset; offset++)
    (void) ReadBlobByte(image);
  image->depth=cin.image.channel[0].bits_per_pixel;
  image->columns=cin.image.channel[0].pixels_per_line;
  image->rows=cin.image.channel[0].lines_per_image;
  if (image_info->ping)
    {
      (void) CloseBlob(image);
      return(image);
    }
  /*
    Convert CIN raster image to pixel packets.
  */
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  quantum_info->quantum=32;
  quantum_info->pack=MagickFalse;
  quantum_type=RGBQuantum;
  pixels=GetQuantumPixels(quantum_info);
  length=GetQuantumExtent(image,quantum_info,quantum_type);
  length=GetBytesPerRow(image->columns,3,image->depth,MagickTrue);
  if (cin.image.number_channels == 1)
    {
      quantum_type=GrayQuantum;
      length=GetBytesPerRow(image->columns,1,image->depth,MagickTrue);
    }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    count=ReadBlob(image,length,pixels);
    if ((size_t) count != length)
      break;
    (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      quantum_type,pixels,exception);
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
  SetImageColorspace(image,LogColorspace);
  (void) CloseBlob(image);
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

  entry=SetMagickInfo("CIN");
  entry->decoder=(DecodeImageHandler *) ReadCINImage;
  entry->encoder=(EncodeImageHandler *) WriteCINImage;
  entry->magick=(IsImageFormatHandler *) IsCIN;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("Cineon Image File");
  entry->module=ConstantString("CIN");
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
%      MagickBooleanType WriteCINImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static inline const char *GetCINProperty(const ImageInfo *image_info,
  const Image *image,const char *property)
{
  const char
    *value;

  value=GetImageOption(image_info,property);
  if (value != (const char *) NULL)
    return(value);
  return(GetImageProperty(image,property));
}

static MagickBooleanType WriteCINImage(const ImageInfo *image_info,Image *image)
{
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

  register const PixelPacket
    *p;

  register ssize_t
    i;

  size_t
    length;

  ssize_t
    count,
    y;

  struct tm
    local_time;

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
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  if (image->colorspace != LogColorspace)
    (void) TransformImageColorspace(image,LogColorspace);
  /*
    Write image information.
  */
  (void) ResetMagickMemory(&cin,0,sizeof(cin));
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
  value=GetCINProperty(image_info,image,"dpx:file.filename");
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.file.filename,value,sizeof(cin.file.filename));
  else
    (void) CopyMagickString(cin.file.filename,image->filename,
      sizeof(cin.file.filename));
  offset+=WriteBlob(image,sizeof(cin.file.filename),(unsigned char *)
    cin.file.filename);
  seconds=time((time_t *) NULL);
#if defined(MAGICKCORE_HAVE_LOCALTIME_R)
  (void) localtime_r(&seconds,&local_time);
#else
  (void) memcpy(&local_time,localtime(&seconds),sizeof(local_time));
#endif
  (void) strftime(cin.file.create_date,sizeof(cin.file.create_date),"%Y:%m:%d",
    &local_time);
  offset+=WriteBlob(image,sizeof(cin.file.create_date),(unsigned char *)
    cin.file.create_date);
  (void) strftime(cin.file.create_time,sizeof(cin.file.create_time),
    "%H:%M:%S%Z",&local_time);
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
  offset+=WriteBlobFloat(image,image->chromaticity.white_point.x);
  offset+=WriteBlobFloat(image,image->chromaticity.white_point.y);
  offset+=WriteBlobFloat(image,image->chromaticity.red_primary.x);
  offset+=WriteBlobFloat(image,image->chromaticity.red_primary.y);
  offset+=WriteBlobFloat(image,image->chromaticity.green_primary.x);
  offset+=WriteBlobFloat(image,image->chromaticity.green_primary.y);
  offset+=WriteBlobFloat(image,image->chromaticity.blue_primary.x);
  offset+=WriteBlobFloat(image,image->chromaticity.blue_primary.y);
  value=GetCINProperty(image_info,image,"dpx:image.label");
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
  value=GetCINProperty(image_info,image,"dpx:origination.x_offset");
  if (value != (const char *) NULL)
    cin.origination.x_offset=(ssize_t) StringToLong(value);
  offset+=WriteBlobLong(image,(unsigned int) cin.origination.x_offset);
  cin.origination.y_offset=0UL;
  value=GetCINProperty(image_info,image,"dpx:origination.y_offset");
  if (value != (const char *) NULL)
    cin.origination.y_offset=(ssize_t) StringToLong(value);
  offset+=WriteBlobLong(image,(unsigned int) cin.origination.y_offset);
  value=GetCINProperty(image_info,image,"dpx:origination.filename");
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.origination.filename,value,
      sizeof(cin.origination.filename));
  else
    (void) CopyMagickString(cin.origination.filename,image->filename,
      sizeof(cin.origination.filename));
  offset+=WriteBlob(image,sizeof(cin.origination.filename),(unsigned char *)
    cin.origination.filename);
  seconds=time((time_t *) NULL);
  (void) strftime(cin.origination.create_date,
    sizeof(cin.origination.create_date),"%Y:%m:%d",&local_time);
  offset+=WriteBlob(image,sizeof(cin.origination.create_date),(unsigned char *)
    cin.origination.create_date);
  (void) strftime(cin.origination.create_time,
    sizeof(cin.origination.create_time),"%H:%M:%S%Z",&local_time);
  offset+=WriteBlob(image,sizeof(cin.origination.create_time),(unsigned char *)
    cin.origination.create_time);
  value=GetCINProperty(image_info,image,"dpx:origination.device");
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.origination.device,value,
      sizeof(cin.origination.device));
  offset+=WriteBlob(image,sizeof(cin.origination.device),(unsigned char *)
    cin.origination.device);
  value=GetCINProperty(image_info,image,"dpx:origination.model");
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.origination.model,value,
      sizeof(cin.origination.model));
  offset+=WriteBlob(image,sizeof(cin.origination.model),(unsigned char *)
    cin.origination.model);
  value=GetCINProperty(image_info,image,"dpx:origination.serial");
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.origination.serial,value,
      sizeof(cin.origination.serial));
  offset+=WriteBlob(image,sizeof(cin.origination.serial),(unsigned char *)
    cin.origination.serial);
  cin.origination.x_pitch=0.0f;
  value=GetCINProperty(image_info,image,"dpx:origination.x_pitch");
  if (value != (const char *) NULL)
    cin.origination.x_pitch=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,cin.origination.x_pitch);
  cin.origination.y_pitch=0.0f;
  value=GetCINProperty(image_info,image,"dpx:origination.y_pitch");
  if (value != (const char *) NULL)
    cin.origination.y_pitch=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,cin.origination.y_pitch);
  cin.origination.gamma=image->gamma;
  offset+=WriteBlobFloat(image,cin.origination.gamma);
  offset+=WriteBlob(image,sizeof(cin.origination.reserve),(unsigned char *)
    cin.origination.reserve);
  /*
    Image film information.
  */
  cin.film.id=0;
  value=GetCINProperty(image_info,image,"dpx:film.id");
  if (value != (const char *) NULL)
    cin.film.id=(char) StringToLong(value);
  offset+=WriteBlobByte(image,(unsigned char) cin.film.id);
  cin.film.type=0;
  value=GetCINProperty(image_info,image,"dpx:film.type");
  if (value != (const char *) NULL)
    cin.film.type=(char) StringToLong(value);
  offset+=WriteBlobByte(image,(unsigned char) cin.film.type);
  cin.film.offset=0;
  value=GetCINProperty(image_info,image,"dpx:film.offset");
  if (value != (const char *) NULL)
    cin.film.offset=(char) StringToLong(value);
  offset+=WriteBlobByte(image,(unsigned char) cin.film.offset);
  offset+=WriteBlobByte(image,(unsigned char) cin.film.reserve1);
  cin.film.prefix=0UL;
  value=GetCINProperty(image_info,image,"dpx:film.prefix");
  if (value != (const char *) NULL)
    cin.film.prefix=StringToUnsignedLong(value);
  offset+=WriteBlobLong(image,(unsigned int) cin.film.prefix);
  cin.film.count=0UL;
  value=GetCINProperty(image_info,image,"dpx:film.count");
  if (value != (const char *) NULL)
    cin.film.count=StringToUnsignedLong(value);
  offset+=WriteBlobLong(image,(unsigned int) cin.film.count);
  value=GetCINProperty(image_info,image,"dpx:film.format");
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.film.format,value,sizeof(cin.film.format));
  offset+=WriteBlob(image,sizeof(cin.film.format),(unsigned char *)
    cin.film.format);
  cin.film.frame_position=0UL;
  value=GetCINProperty(image_info,image,"dpx:film.frame_position");
  if (value != (const char *) NULL)
    cin.film.frame_position=StringToUnsignedLong(value);
  offset+=WriteBlobLong(image,(unsigned int) cin.film.frame_position);
  cin.film.frame_rate=0.0f;
  value=GetCINProperty(image_info,image,"dpx:film.frame_rate");
  if (value != (const char *) NULL)
    cin.film.frame_rate=StringToDouble(value,(char **) NULL);
  offset+=WriteBlobFloat(image,cin.film.frame_rate);
  value=GetCINProperty(image_info,image,"dpx:film.frame_id");
  if (value != (const char *) NULL)
    (void) CopyMagickString(cin.film.frame_id,value,sizeof(cin.film.frame_id));
  offset+=WriteBlob(image,sizeof(cin.film.frame_id),(unsigned char *)
    cin.film.frame_id);
  value=GetCINProperty(image_info,image,"dpx:film.slate_info");
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
  quantum_info->quantum=32;
  quantum_info->pack=MagickFalse;
  quantum_type=RGBQuantum;
  pixels=GetQuantumPixels(quantum_info);
  length=GetBytesPerRow(image->columns,3,image->depth,MagickTrue);
DisableMSCWarning(4127)
  if (0)
RestoreMSCWarning
    {
      quantum_type=GrayQuantum;
      length=GetBytesPerRow(image->columns,1UL,image->depth,MagickTrue);
    }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    (void) ExportQuantumPixels(image,(const CacheView *) NULL,quantum_info,
      quantum_type,pixels,&image->exception);
    count=WriteBlob(image,length,pixels);
    if (count != (ssize_t) length)
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
